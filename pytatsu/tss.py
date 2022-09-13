from pybmtool.BMParse import BMParse
import plistlib
import uuid
import random
import hashlib
import binascii
import aiohttp
import asyncio


class TSS:
    def __init__(self, board: str, ecid: int, update: bool = False, generator: str = "", apnonce: str = "",
                 sepnonce: str = "", buildManifestPath: str = ""):
        self.bm = BMParse(buildManifestPath=buildManifestPath)
        self.manifest = self.bm.manifest
        self.identity = self.bm.getBoardIdentity(board=board, update=update)
        self.chipid = int(self.identity.get("ApChipID", None), 16)
        self.libAuthInstallVersion = "850.0.2"
        self.tssVersionString = f"libauthinstall-{self.libAuthInstallVersion}"
        self.doGenSHA = False
        self.generatorString = ""
        self.sepGeneratorString = ""
        self.generator = b""
        self.apnonce = b""
        self.sepnonce = b""
        self.ecid = ecid
        self.bbsnum = b""
        self.bbgoldcertid = 0
        self.parameters = {}
        self.request = {}
        self.tatsuURLS = [
            "https://gs.apple.com/TSS/controller?action=2",
            "https://17.111.103.65/TSS/controller?action=2",
            "https://17.111.103.15/TSS/controller?action=2",
            "http://gs.apple.com/TSS/controller?action=2",
            "http://17.111.103.65/TSS/controller?action=2",
            "http://17.111.103.15/TSS/controller?action=2",
        ]
        if self.chipid < 1:
            raise AssertionError("Could not get ChipID!")
        if len(generator) < 1:
            self.generatorString = binascii.hexlify(bytearray(random.getrandbits(8) for _ in range(8))).decode("utf-8")
            self.generator = binascii.unhexlify(self.generatorString)
            self.doGenSHA = True
        else:
            self.generatorString = generator
            self.generator = binascii.unhexlify(bytes(generator.strip("0x").encode('iso-8859-15')))
        if len(apnonce) < 1:
            self.doGenSHA = True
        else:
            if not self.doGenSHA:
                self.apnonce = binascii.unhexlify(bytes(apnonce.encode('iso-8859-15')))
        if self.doGenSHA:
            if self.chipid >= 0x8010:
                apsha = hashlib.sha384()
                apsha.update(self.generator)
                self.apnonce = apsha.digest()[:64]
            else:
                apsha = hashlib.sha1()
                apsha.update(self.generator)
                self.apnonce = apsha.digest()
        if len(sepnonce) < 1:
            self.sepGeneratorString = binascii.hexlify(bytearray(random.getrandbits(8) for _ in range(8))).decode("utf-8")
            sepgenerator = binascii.unhexlify(self.sepGeneratorString)
            if self.chipid >= 0x8010:
                sepsha = hashlib.sha384()
                sepsha.update(sepgenerator)
                self.sepnonce = sepsha.digest()[:64]
            else:
                sepsha = hashlib.sha1()
                sepsha.update(sepgenerator)
                self.sepnonce = sepsha.digest()
        else:
            self.sepnonce = binascii.unhexlify(bytes(sepnonce.encode('iso-8859-15')))
        self.tatsuInitParameters()
        self.tatsuInitRequest()

    def tatsuInitFromIdentity(self):
        if self.parameters is None:
            self.parameters = {}
        if len(self.identity) < 1:
            raise AssertionError("Identity is empty!")
        if not self.identity.get("UniqueBuildID", None):
            raise AssertionError("Could not find UniqueBuildID!")
        if not self.identity.get("ApChipID", None):
            raise AssertionError("Could not find ApChipID!")
        if not self.identity.get("ApBoardID", None):
            raise AssertionError("Could not find ApBoardID!")
        if not self.identity.get("ApSecurityDomain", None):
            raise AssertionError("Could not find ApSecurityDomain!")
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
            "Ap,OSLongVersion"
        ]
        for key, value in self.identity.items():
            if key in entries:
                self.parameters[key] = value

    def tatsuInitIMG4(self):
        if self.parameters.get("BbChipID", None) and len(self.bbsnum) > 0 and self.bbgoldcertid > 0:
            parameters = {
                "@BBTicket": True,
                "BbSNUM": self.bbsnum,
                "BbGoldCertId": self.bbgoldcertid
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

    def tatsuInitRestoreRequestRules(self, manifestEntry: dict):
        if len(manifestEntry) < 1:
            raise AssertionError("Manifest entry is empty!")
        if len(self.parameters) < 1:
            raise AssertionError("Parameters are empty!")
        if not manifestEntry.get("Info", None):
            raise AssertionError("Manifest entry is missing info!")
        rules = manifestEntry["Info"]["RestoreRequestRules"]
        if rules:
            for entry in rules:
                if not entry.get("Conditions") or not entry.get("Actions"):
                    break
                else:
                    conditionsKey, conditionsValue = next(iter(entry["Conditions"].items()), (None, None))
                    actionsKey, actionsValue = next(iter(entry["Actions"].items()), (None, None))
                    key_dict = {
                        "ApRawProductionMode": "ApProductionMode",
                        "ApCurrentProductionMode": "ApProductionMode",
                        "ApRawSecurityMode": "ApSecurityMode",
                        "ApRequiresImage4": "ApSupportsImg4",
                        "ApDemotionPolicyOverride": "DemotionPolicy",
                        "ApInRomDFU": "ApInRomDFU",
                    }
                    paramValue = self.parameters.get(key_dict[conditionsKey], False)
                    if paramValue and conditionsValue:
                        if not manifestEntry.get(actionsKey, None):
                            manifestEntry[actionsKey] = actionsValue
        if manifestEntry.get("Info", None):
            del manifestEntry["Info"]

    def tatsuInitComponents(self):
        for key, value in list(self.parameters["Manifest"].items()):
            if not value.get("Info", None):
                del self.parameters["Manifest"][key]
            elif not value["Info"].get("RestoreRequestRules", None):
                del self.parameters["Manifest"][key]
                continue
            if key in ["BasebandFirmware", "SE,UpdatePayload", "BaseSystem", "Diags"]:
                if self.parameters["Manifest"].get(key, None):
                    del self.parameters["Manifest"][key]
                    continue
            if self.parameters.get("_OnlyFWComponents", None):
                if not value.get("Trusted", None):
                    if self.parameters.get(key, None):
                        del self.parameters["Manifest"][key]
                        continue
                if not value["Info"].get("IsFirmwarePayload") and\
                        not value["Info"].get("IsSecondaryFirmwarePayload") and\
                        not value["Info"].get("IsFUDFirmware"):
                    if self.parameters["Manifest"].get(key, None):
                        del self.parameters["Manifest"][key]
                        continue
            self.parameters[key] = value
            if value.get("Trusted", None) and not value.get("Digest", None):
                self.parameters[key]["Digest"] = b""
            self.tatsuInitRestoreRequestRules(manifestEntry=self.parameters[key])
        del self.parameters["Manifest"]


    def tatsuInitParameters(self):
        self.parameters = {
            "@BBTicket": bool(False),
            "@Locality": str("en_US"),
            "@HostPlatformInfo": str("mac"),
            "@VersionInfo": str(self.tssVersionString),
            "@UUID": str(uuid.uuid1()).upper()
        }
        self.tatsuInitFromIdentity()
        self.tatsuInitIMG4()
        self.tatsuInitComponents()

    def tatsuInitRequest(self):
        self.request = plistlib.dumps(self.parameters)

    async def tatsuRequestSend_(self):
        async with aiohttp.ClientSession() as session:
            if "https" in self.tatsuURLS[0]:
                ssl = True
            else:
                ssl = False
            headers = {
                "Cache-Control": "no-cache",
                "Content-Type": "text/xml; charset=\"utf-8\"",
                "User-Agent": "InetURL/1.0"
                # "Expect": b"",
            }
            # proxy="http://127.0.0.1:8888",
            async with session.post(url=self.tatsuURLS[0], headers=headers, data=self.request, ssl=ssl) as response:
                blobResponse = await response.text()
                if blobResponse[:9] == "STATUS=94":
                    print("This device isn't eligible for the requested build.")
                    return
                if blobResponse[:8] != "STATUS=0":
                    print("Invalid tatsu response!")
                    return
                print("Successfully saved blobs!")
                blobResponse = blobResponse.strip("STATUS=0&MESSAGE=SUCCESS&REQUEST_STRING=")
                blobResponse = plistlib.loads(blobResponse.encode("utf-8"))
                if len(self.generatorString) > 0:
                    blobResponse["generator"] = f"0x{self.generatorString.strip('0x')}"
                if len(self.sepGeneratorString) > 0:
                    blobResponse["sepgenerator"] = f"0x{self.sepGeneratorString.strip('0x')}"
                blobResponse = plistlib.dumps(blobResponse)
                with open("blob.shsh2", "wb+") as blob:
                    blob.write(blobResponse)


    def tatsuRequestSend(self):
        if len(self.request) < 1:
            raise AssertionError("Request is empty!")
        # print(str(self.request).replace("\\n", "\n").replace("\\t", "\t"))
        loop = asyncio.get_event_loop()
        loop.run_until_complete(self.tatsuRequestSend_())
