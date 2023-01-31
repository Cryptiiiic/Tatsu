import pybmtool
import plistlib
import uuid
import random
import hashlib
import binascii
import aiohttp
import asyncio
from typing import Optional, List


class TSS:
    def __init__(
        self,
        board: str,
        ecid: int,
        update: bool = False,
        generator: str = "",
        apnonce: str = "",
        sepnonce: str = "",
        build_manifest_path: str = "",
        component_list: Optional[List[str]] = None,
    ):
        self.bm = pybmtool.BMParse(build_manifest_path=build_manifest_path)
        self.manifest = self.bm.manifest
        self.product_build_version = self.manifest.get("ProductBuildVersion", "Unknown")
        self.product_version = self.manifest.get("ProductVersion", "Unknown")
        self.identity = self.bm.get_board_identity(board=board, update=update)
        self.chipid = int(self.identity.get("ApChipID", None), 16)
        self.libauthinstallversion = "850.0.2"
        self.tss_version = f"libauthinstall-{self.libauthinstallversion}"
        self.do_gen_sha = False
        self.generator_string = ""
        self.sep_generator_string = ""
        self.generator = b""
        self.apnonce = b""
        self.sepnonce = b""
        self.ecid = ecid
        self.bbsnum = b""
        self.bbgoldcertid = 0
        self.parameters = {}
        self.request = {}
        self.component_list = []
        self.tatsu_urls = [
            "https://gs.apple.com/TSS/controller?action=2",
            "https://17.111.103.65/TSS/controller?action=2",
            "https://17.111.103.15/TSS/controller?action=2",
            "http://gs.apple.com/TSS/controller?action=2",
            "http://17.111.103.65/TSS/controller?action=2",
            "http://17.111.103.15/TSS/controller?action=2",
        ]
        if self.chipid < 1:
            raise ValueError("Could not get ChipID!")
        if component_list is None:
            self.component_list = []
        else:
            self.component_list = component_list
        print(f"Selected device: {board}")
        print(f"Selected version: {self.product_version}({self.product_build_version})")
        self.setup_nonces(generator, apnonce, sepnonce)
        self.init_parameters()
        self.init_request()

    def setup_nonces(self, generator: str, apnonce: str, sepnonce: str):
        if len(generator) < 1:
            self.generator_string = binascii.hexlify(
                bytearray(random.getrandbits(8) for _ in range(8))
            ).decode("utf-8")
            self.generator = binascii.unhexlify(self.generator_string)
            self.do_gen_sha = True
        else:
            self.generator_string = generator.strip("0x")
            self.generator = binascii.unhexlify(
                bytes(self.generator_string.encode("iso-8859-15"))
            )
        if len(apnonce) < 1:
            self.do_gen_sha = True
        else:
            if not self.do_gen_sha:
                self.apnonce = binascii.unhexlify(bytes(apnonce.encode("iso-8859-15")))
        if self.do_gen_sha:
            if self.chipid >= 0x8010:
                apsha = hashlib.sha384()
                apsha.update(self.generator)
                self.apnonce = apsha.digest()[:32]
            else:
                apsha = hashlib.sha1()
                apsha.update(self.generator)
                self.apnonce = apsha.digest()
        if len(sepnonce) < 1:
            self.sep_generator_string = binascii.hexlify(
                bytearray(random.getrandbits(8) for _ in range(8))
            ).decode("utf-8")
            sepgenerator = binascii.unhexlify(self.sep_generator_string)
            if self.chipid >= 0x8010:
                sepsha = hashlib.sha384()
                sepsha.update(sepgenerator)
                self.sepnonce = sepsha.digest()[:32]
            else:
                sepsha = hashlib.sha1()
                sepsha.update(sepgenerator)
                self.sepnonce = sepsha.digest()
        else:
            self.sepnonce = binascii.unhexlify(bytes(sepnonce.encode("iso-8859-15")))

    def init_from_identity(self):
        if self.parameters is None:
            self.parameters = {}
        if len(self.identity) < 1:
            raise ValueError("Identity is empty!")
        parameters = {
            "UniqueBuildID": self.identity["UniqueBuildID"],
            "ApChipID": self.identity["ApChipID"],
            "ApBoardID": self.identity["ApBoardID"],
            "ApSecurityDomain": self.identity["ApSecurityDomain"],
        }
        self.parameters.update(parameters)
        entries = [
            "BMU,BoardID",
            "BMU,ChipID",
            "BbChipID",
            "SE,ChipID",
            "Savage,ChipID",
            "Savage,PatchEpoch",
            "Yonkers,BoardID",
            "Yonkers,ChipID",
            "Rap,BoardID",
            "Rap,ChipID",
            "Rap,SecurityDomain",
            "eUICC,ChipID",
            "Baobab,BoardID",
            "Baobab,ChipID",
            "Baobab,ManifestEpoch",
            "Baobab,SecurityDomain",
            "Timer,BoardID,1",
            "Timer,BoardID,2",
            "Timer,ChipID,1",
            "Timer,ChipID,2",
            "Timer,SecurityDomain,1",
            "Timer,SecurityDomain,2",
            "BbProvisioningManifestKeyHash",
            "BbActivationManifestKeyHash",
            "BbFDRSecurityKeyHash",
            "BbCalibrationManifestKeyHash",
            "BbFactoryActivationManifestKeyHash",
            "BbSkeyId",
            "Yonkers,PatchEpoch",
            "PearlCertificationRootPub",
            "Manifest",
            "Cryptex1,ChipID",
            "Cryptex1,Type",
            "Cryptex1,SubType",
            "Cryptex1,ProductClass",
            "Cryptex1,UseProductClass",
            "Cryptex1,NonceDomain",
            "Cryptex1,Version",
            "Cryptex1,PreauthorizationVersion",
            "Cryptex1,FakeRoot",
            "Cryptex1,SystemOS",
            "Cryptex1,SystemVolume",
            "Cryptex1,SystemTrustCache",
            "Cryptex1,AppOS",
            "Cryptex1,AppVolume",
            "Cryptex1,AppTrustCache",
            "Cryptex1,MobileAssetBrainVolume",
            "Cryptex1,MobileAssetBrainTrustCache",
            "Ap,OSLongVersion",
        ]
        for key, value in self.identity.items():
            if key in entries:
                self.parameters[key] = value

    def init_img4(self):
        if (
            self.parameters.get("BbChipID", None)
            and len(self.bbsnum) > 0
            and self.bbgoldcertid > 0
        ):
            parameters = {
                "@BBTicket": True,
                "BbSNUM": self.bbsnum,
                "BbGoldCertId": self.bbgoldcertid,
            }
            self.parameters.update(parameters)
        parameters = {
            "@ApImg4Ticket": True,
            "ApSecurityMode": True,
            "ApProductionMode": True,
            "ApNonce": self.apnonce,
            "SepNonce": self.sepnonce,
            "ApECID": self.ecid,
        }
        self.parameters.update(parameters)

    def init_restore_request_rules(self, manifest_entry: dict):
        if len(manifest_entry) < 1:
            raise ValueError("Manifest entry is empty!")
        if len(self.parameters) < 1:
            raise ValueError("Parameters are empty!")
        rules = manifest_entry["Info"]["RestoreRequestRules"]
        if rules:
            for entry in rules:
                if not entry.get("Conditions") or not entry.get("Actions"):
                    break
                else:
                    conditions_key, conditions_value = next(
                        iter(entry["Conditions"].items()), (None, None)
                    )
                    actions_key, actions_value = next(
                        iter(entry["Actions"].items()), (None, None)
                    )
                    key_dict = {
                        "ApRawProductionMode": "ApProductionMode",
                        "ApCurrentProductionMode": "ApProductionMode",
                        "ApRawSecurityMode": "ApSecurityMode",
                        "ApRequiresImage4": "ApSupportsImg4",
                        "ApDemotionPolicyOverride": "DemotionPolicy",
                        "ApInRomDFU": "ApInRomDFU",
                    }
                    param_value = self.parameters.get(key_dict[conditions_key], False)
                    if param_value and conditions_value:
                        if not manifest_entry.get(actions_key, None):
                            manifest_entry[actions_key] = actions_value
        if manifest_entry.get("Info", None):
            manifest_entry.pop("Info")

    def init_components(self):
        manifest = self.parameters["Manifest"]
        for key, value in list(manifest.items()):
            if not value.get("Info", None):
                manifest.pop(key)
            elif not value["Info"].get("RestoreRequestRules", None):
                manifest.pop(key)
                continue
            if key in ["BasebandFirmware", "SE,UpdatePayload", "BaseSystem", "Diags"]:
                if manifest.get(key, None):
                    manifest.pop(key)
                    continue
            if self.parameters.get("_OnlyFWComponents", None):
                if not value.get("Trusted", None):
                    if self.parameters.get(key, None):
                        manifest.pop(key)
                        continue
                if (
                    not value["Info"].get("IsFirmwarePayload")
                    and not value["Info"].get("IsSecondaryFirmwarePayload")
                    and not value["Info"].get("IsFUDFirmware")
                ):
                    if manifest.get(key, None):
                        manifest.pop(key)
                        continue
            add = False
            if len(self.component_list) < 1:
                add = True
            elif key in self.component_list:
                add = True
            if add:
                self.parameters[key] = value
                if value.get("Trusted", None) and not value.get("Digest", None):
                    self.parameters[key]["Digest"] = b""
                self.init_restore_request_rules(manifest_entry=self.parameters[key])
        del self.parameters["Manifest"]

    def init_parameters(self):
        self.parameters = {
            "@BBTicket": bool(False),
            "@Locality": str("en_US"),
            "@HostPlatformInfo": str("mac"),
            "@VersionInfo": str(self.tss_version),
            "@UUID": str(uuid.uuid1()).upper(),
        }
        self.init_from_identity()
        self.init_img4()
        self.init_components()

    def init_request(self):
        self.request = plistlib.dumps(self.parameters)

    async def __send_request(self):
        async with aiohttp.ClientSession() as session:
            if "https" in self.tatsu_urls[0]:
                ssl = True
            else:
                ssl = False
            headers = {
                "Cache-Control": "no-cache",
                "Content-Type": 'text/xml; charset="utf-8"',
                "User-Agent": "InetURL/1.0",
            }
            # proxy="http://127.0.0.1:8888",
            async with session.post(
                url=self.tatsu_urls[0], headers=headers, data=self.request, ssl=ssl
            ) as response:
                blob_response = await response.text()
                if blob_response[:9] == "STATUS=94":
                    print("This device isn't eligible for the requested build.")
                    return
                if blob_response[:8] != "STATUS=0":
                    print("Invalid tatsu response!")
                    return
                print("Successfully saved blobs!")
                blob_response = blob_response.strip(
                    "STATUS=0&MESSAGE=SUCCESS&REQUEST_STRING="
                )
                blob_response = plistlib.loads(blob_response.encode("utf-8"))
                if len(self.generator_string) > 0:
                    blob_response["generator"] = f"0x{self.generator_string}"
                if len(self.sep_generator_string) > 0:
                    blob_response["sepgenerator"] = f"0x{self.sep_generator_string}"
                blob_response = plistlib.dumps(blob_response)
                with open("blob.shsh2", "wb+") as blob:
                    blob.write(blob_response)

    def send_request(self):
        if len(self.request) < 1:
            raise ValueError("Request is empty!")
        # print(str(self.request).replace("\\n", "\n").replace("\\t", "\t"))
        loop = asyncio.get_event_loop()
        loop.run_until_complete(self.__send_request())
