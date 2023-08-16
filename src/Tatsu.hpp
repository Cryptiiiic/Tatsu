//
// Created by cryptic on 1/30/23.
//

#ifndef TATSU_TATSU_HPP
#define TATSU_TATSU_HPP

#pragma once

#include <array>
#include <Manifest.hpp>

#define TSS_VERSION "914.40.5"
#define TSS_VERSION_STRING "libauthinstall-" TSS_VERSION

#define ENTRIES_COUNT 54
static std::array<std::string, ENTRIES_COUNT> entries = {
        "UniqueBuildID",
        "ApChipID",
        "ApBoardID",
        "ApSecurityDomain",
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
        "NeRDEpoch",
};

class Tatsu {
private:
    std::shared_ptr<Manifest> mManifest = nullptr;
    int mChipID = -1;
    std::string mDeviceClass;
    uint64_t mECID = -1;
    int mVariant = -1;
    uint64_t mGenerator = -1;
    uint64_t mTMPGenerator = -1;
    std::string mAPNonce;
    unsigned char mAPNonceDGST[64]{};
    std::vector<char> mAPNonceDGSTVector;
    std::string mSEPNonce;
    unsigned char mSEPNonceDGST[32]{};
    std::vector<char> mSEPNonceDGSTVector;
    std::string mBasebandSerialNumber;
    int mBasebandGoldCertID = -1;
    std::vector<std::string> mComponentList{};
    std::pair<std::vector<std::string>, std::vector<std::string>> mCustomComponentList{};

    std::shared_ptr<PList::Dictionary> mParameters = nullptr;
//    plist_t mRequest = nullptr;

public:
    explicit Tatsu(std::shared_ptr<Manifest> manifest = nullptr, int chipID = -1, std::string deviceClass = "", uint64_t ECID = -1, int variant = -1, uint64_t generator = -1, std::string apNonce = "", std::string sepNonce = "", std::vector<std::string> componentList = {}, std::pair<std::vector<std::string>, std::vector<std::string>> customComponentList = {});
    bool initParameters();
    bool initFromIdentity();
    bool initIMG4();
    bool initComponents();
    bool initRestoreRequestRules(PList::Node *entry);
    bool writeBlob(const std::string &blob);
    virtual ~Tatsu();
};


#endif //TATSU_TATSU_HPP
