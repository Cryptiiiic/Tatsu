import pytatsu
import os


def cli():
    tss = pytatsu.TSS(
        board="d63ap",
        ecid=0x69,
        update=False,
        generator="0x1111111111111111",
        apnonce="",
        sepnonce="",
        build_manifest_path=f"{os.getcwd()}/BuildManifest.plist",
        component_list=[
            "Rap,RTKitOS",
        ],
    )
    tss.send_request()


if __name__ == "__main__":
    cli()
