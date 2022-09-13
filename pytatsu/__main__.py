from pytatsu.tss import TSS
import os


def cli():
    tss = TSS(board="d63ap", update=False, ecid=0x69, generator="0x1111111111111111",
              apnonce="",
              sepnonce="",
              buildManifestPath=f"{os.getcwd()}/BuildManifest.plist")
    tss.tatsuRequestSend()


if __name__ == '__main__':
    cli()
