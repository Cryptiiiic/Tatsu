import pytatsu
import os
import pybmtool

def cli():
    # ipsw_list = [
    #     "https://updates.cdn-apple.com/2022FallFCS/fullrestores/012-73356/4A4B72C4-F77C-46FE-BCAE-0BDAC844F75E/iPhone14,2_16.0.3_20A392_Restore.ipsw",
    #     "https://updates.cdn-apple.com/2022FallFCS/fullrestores/012-72652/FBAEB0D8-C414-4CBA-97A8-555D8367B5CB/iPhone14,2_16.0.2_20A380_Restore.ipsw",
    #     "https://updates.cdn-apple.com/2022FallFCS/fullrestores/012-65747/591A05A2-E535-4228-8F2B-ADA395C72D77/iPhone14,2_16.0_20A362_Restore.ipsw",
    #     "https://updates.cdn-apple.com/2022FallFCS/fullrestores/012-39023/4003229C-E504-4FEC-BD31-130A5D569CA7/iPhone14,2_15.7_19H12_Restore.ipsw",
    #     "https://updates.cdn-apple.com/2022SummerFCS/fullrestores/012-52415/04B2D458-9F17-4937-8ECB-60BB805C3BBB/iPhone14,2_15.6.1_19G82_Restore.ipsw",
    #     "https://updates.cdn-apple.com/2022SummerFCS/fullrestores/012-41767/A2EF8631-6FDA-4304-A66A-B516E9890DEB/iPhone14,2_15.6_19G71_Restore.ipsw",
    #     "https://updates.cdn-apple.com/2022SummerFCS/fullrestores/012-40402/AAF3D2F1-CACA-4070-A09B-D428A92C4B4D/iPhone14,2_15.6_19G69_Restore.ipsw",
    #     "https://updates.cdn-apple.com/2022SpringSeed/fullrestores/012-36356/88120729-4787-4343-9964-488687AC55AE/iPhone14,2_15.6_19G5063a_Restore.ipsw",
    #     "https://updates.cdn-apple.com/2022SpringSeed/fullrestores/012-33018/A7674C2A-55B9-4A5A-AD46-CA4CF1EFADA7/iPhone14,2_15.6_19G5056c_Restore.ipsw",
    #     "https://updates.cdn-apple.com/2022SpringSeed/fullrestores/012-17897/AA7F668E-C14D-40E7-B0BF-082F1BCEF277/iPhone14,2_15.6_19G5046d_Restore.ipsw",
    #     "https://updates.cdn-apple.com/2022SpringSeed/fullrestores/012-10589/6EE345C8-95EB-44A2-AF11-67DD30C53A64/iPhone14,2_15.6_19G5037d_Restore.ipsw",
    #     "https://updates.cdn-apple.com/2022SpringSeed/fullrestores/002-94118/43A7C79C-2FBE-48E1-A11E-977B31366739/iPhone14,2_15.6_19G5027e_Restore.ipsw",
    #     "https://updates.cdn-apple.com/2022SpringFCS/fullrestores/012-07470/447321BB-DF11-4900-8209-B6D86300039D/iPhone14,2_15.5_19F77_Restore.ipsw",
    #     "https://updates.cdn-apple.com/2022FCSWinter/fullrestores/002-81181/9D4DDBE6-0FDE-4925-A8E8-4EE02D1A35F5/iPhone14,2_15.4.1_19E258_Restore.ipsw",
    #     "https://updates.cdn-apple.com/2022FCSWinter/fullrestores/071-09884/114A42ED-D46C-4C3E-85FF-74F33C19AC11/iPhone14,2_15.4_19E241_Restore.ipsw",
    #     "https://updates.cdn-apple.com/2022FCSWinter/fullrestores/002-64299/5491C1B0-210A-4E1E-A47A-3BA783A3C09B/iPhone14,2_15.3.1_19D52_Restore.ipsw",
    #     "https://updates.cdn-apple.com/2022FCSWinter/fullrestores/002-57207/8B0145D3-8E98-4B5F-987C-A8709F2069FB/iPhone14,2_15.3_19D50_Restore.ipsw",
    #     "https://updates.cdn-apple.com/2021FCSWinter/fullrestores/002-51507/47593DCE-6CD6-4CF2-BC18-4C1ECB9570DD/iPhone14,2_15.2.1_19C63_Restore.ipsw",
    #     "https://updates.cdn-apple.com/2021FCSWinter/fullrestores/002-42790/6B7F4F0F-560D-4BCE-BE82-441A216EE1A6/iPhone14,2_15.2_19C57_Restore.ipsw",
    #     "https://updates.cdn-apple.com/2021FallFCS/fullrestores/002-34722/1500217D-0C23-48B5-89B0-3214949BCC3F/iPhone14,2_15.1.1_19B81_Restore.ipsw",
    #     "https://updates.cdn-apple.com/2021FallFCS/fullrestores/071-63953/950EC737-84E9-4255-9A12-059D9020D198/iPhone14,2_15.1_19B74_Restore.ipsw",
    #     "https://updates.cdn-apple.com/2021FCSFall/fullrestores/002-14579/C7B407E9-5B94-45B4-8688-5C91A3416665/iPhone14,2_15.0.2_19A404_Restore.ipsw",
    #     "https://updates.cdn-apple.com/2021FallFCS/fullrestores/002-09363/F489360E-C0A9-4A6F-A2F8-961BDEB2F3C9/iPhone14,2_15.0.1_19A348_Restore.ipsw",
    #     "https://updates.cdn-apple.com/2021FallFCS/fullrestores/002-02399/F59733EA-9918-42A6-ACC8-DE5AA24C74AD/iPhone14,2_15.0_19A346_Restore.ipsw",
    # ]
    # for ipsw in ipsw_list:
    #     bm = pybmtool.BMParse(url=ipsw)
    #     tss = pytatsu.TSS(
    #         board="d63ap",
    #         ecid=0x69,
    #         update=False,
    #         generator="0x1111111111111111",
    #         apnonce="",
    #         sepnonce="",
    #         build_manifest_path=f"{os.getcwd()}/BuildManifest.plist",
    #         component_list=[
    #             "Rap,RTKitOS",
    #         ],
    #     )
    #     tss.send_request()
        bm = pybmtool.BMParse(url="https://updates.cdn-apple.com/2023WinterFCS/fullrestores/032-18276/9B728687-DCC9-45BB-B7F8-9D321EB84390/iPhone_4.7_15.7.3_19H307_Restore.ipsw")
        tss = pytatsu.TSS(
            board="n71ap",
            ecid=0x69,
            update=False,
            generator="0x1111111111111111",
            apnonce="",
            sepnonce="",
            build_manifest_path=f"{os.getcwd()}/BuildManifest.plist",
            component_list=[],
        )
        tss.send_request()


if __name__ == "__main__":
    cli()
