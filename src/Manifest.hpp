//
// Created by cryptic on 1/28/23.
//

#ifndef TATSU_MANIFEST_HPP
#define TATSU_MANIFEST_HPP

#pragma once
#include <plist/plist++.h>
#include <Modern.hpp>

enum restoreVariants {
    ERASE = 0,
    UPDATE,
    DEFAULT,
};

class Manifest {
private:
    PList::Dictionary *mManifestStructure = nullptr;
    PList::Array *mBuildIdentitiesArray = nullptr;
    PList::Integer *mManifestVersion = nullptr;
    PList::String *mProductBuildVersion = nullptr;
    PList::String *mProductVersion = nullptr;
    PList::Array *mSupportedProductTypesArray = nullptr;
//    plist_t mManifest = nullptr;
    std::string mManifestPath;

public:
    const std::string eraseString = "Erase";
    const std::string updateString = "Update";
    PList::Dictionary *pIdentity = nullptr;
    bool requiresUIDMode = false;
    explicit Manifest(std::string path = "");
    bool matchIdentity(int chipID, std::string deviceClass, int variant, bool overwrite = false);
    bool readManifest();
    bool isValid();
    ~Manifest();

};



#endif //TATSU_MANIFEST_HPP
