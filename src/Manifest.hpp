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
    PList::Node *mBuildIdentitiesArray = nullptr;
    PList::Node *mManifestVersion = nullptr;
    PList::Node *mProductBuildVersion = nullptr;
    PList::Node *mProductVersion = nullptr;
    PList::Node *mSupportedProductTypesArray = nullptr;
    std::shared_ptr<PList::Structure> mManifestStructure = nullptr;
    plist_t mManifest = nullptr;
    std::string mManifestPath;

    const std::string mEraseString = "Erase";
    const std::string mUpdateString = "Update";
public:
    PList::Node *pIdentity = nullptr;
    explicit Manifest(std::string path = "");
    bool matchIdentity(int chipID, std::string deviceClass, int variant, bool overwrite = false);
    bool readManifest();
    ~Manifest();

};


#endif //TATSU_MANIFEST_HPP
