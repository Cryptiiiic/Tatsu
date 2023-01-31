//
// Created by cryptic on 1/28/23.
//

#ifndef TATSU_MANIFEST_HPP
#define TATSU_MANIFEST_HPP

#pragma once
#include <plist/plist++.h>

enum restoreVariants {
    ERASE = 0,
    UPDATE,
    DEFAULT,
};

class Manifest {
private:
    plist_t mBuildIdentitiesArray = nullptr;
    plist_t mManifestVersion = nullptr;
    plist_t mProductBuildVersion = nullptr;
    plist_t mProductVersion = nullptr;
    plist_t mSupportedProductTypesArray = nullptr;
    plist_t mManifest = nullptr;

    std::string mEraseString = "Erase";
    std::string mUpdateString = "Update";
public:
    plist_t pIdentity = nullptr;
    Manifest(plist_t manifest = nullptr);
    bool matchIdentity(int chipID, std::string deviceClass, int variant, bool overwrite = false);
    ~Manifest();

};


#endif //TATSU_MANIFEST_HPP
