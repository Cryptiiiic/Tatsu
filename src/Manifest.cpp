//
// Created by cryptic on 1/28/23.
//

#include "Manifest.hpp"
#include <fmt/core.h>
#include <fmt/color.h>

Manifest::Manifest(plist_t manifest) : mManifest(manifest) {
    if(!this->mManifest) {
        fmt::print(fg(fmt::color::crimson), "{0}: Failed to provide valid manifest plist!\n", __PRETTY_FUNCTION__);
        return;
    }
    this->mBuildIdentitiesArray = plist_dict_get_item(this->mManifest, "BuildIdentities");
    if(!this->mBuildIdentitiesArray) {
        fmt::print(fg(fmt::color::crimson), "{0}: manifest plist is missing BuildIdentities array!\n", __PRETTY_FUNCTION__);
        return;
    }
    this->mManifestVersion = plist_dict_get_item(this->mManifest, "ManifestVersion");
    if(!this->mManifestVersion) {
        fmt::print(fg(fmt::color::crimson), "{0}: manifest plist is missing ManifestVersion!\n", __PRETTY_FUNCTION__);
        return;
    }
    this->mProductBuildVersion = plist_dict_get_item(this->mManifest, "ProductBuildVersion");
    if(!this->mProductBuildVersion) {
        fmt::print(fg(fmt::color::crimson), "{0}: manifest plist is missing ProductBuildVersion!\n", __PRETTY_FUNCTION__);
        return;
    }
    this->mProductVersion = plist_dict_get_item(this->mManifest, "ProductVersion");
    if(!this->mProductVersion) {
        fmt::print(fg(fmt::color::crimson), "{0}: manifest plist is missing ProductVersion!\n", __PRETTY_FUNCTION__);
        return;
    }
    this->mSupportedProductTypesArray = plist_dict_get_item(this->mManifest, "SupportedProductTypes");
    if(!this->mSupportedProductTypesArray) {
        fmt::print(fg(fmt::color::crimson), "{0}: manifest plist is missing SupportedProductTypes array!\n", __PRETTY_FUNCTION__);
        return;
    }
//    fmt::print(fg(fmt::color::forest_green), "{0}: Identified manifest as {1}({2})\n", __PRETTY_FUNCTION__,
//               plist_get_string_ptr(this->mProductVersion, nullptr), plist_get_string_ptr(this->mProductBuildVersion, nullptr));
//    fmt::print(fg(fmt::color::forest_green), "{0}: Identified manifest as {1}({2})\n", __PRETTY_FUNCTION__,
//               plist_get_string_ptr(this->mProductVersion, nullptr), plist_get_string_ptr(this->mProductBuildVersion, nullptr));
}

bool Manifest::matchIdentity(int chipID, std::string deviceClass, int variant, bool overwrite) {
    if(this->pIdentity && !overwrite) {
        return true;
    }
    uint32_t count = plist_array_get_size(this->mBuildIdentitiesArray);
    if(!count) {
        return false;
    }
    plist_array_iter iter = nullptr;
    plist_array_new_iter(this->mBuildIdentitiesArray, &iter);
    if(!iter) {
        return false;
    }
    plist_t subPlist = nullptr;
    do {
        plist_array_next_item(this->mBuildIdentitiesArray, iter, &subPlist);
        if(subPlist) {
            plist_t chipIDEntry = plist_dict_get_item(subPlist, "ApChipID");
            if(!chipIDEntry) {
                continue;
            }
            plist_t infoEntry = plist_dict_get_item(subPlist, "Info");
            if(!infoEntry) {
                continue;
            }
            plist_t deviceClassEntry = plist_dict_get_item(infoEntry, "DeviceClass");
            if(!deviceClassEntry) {
                continue;
            }
            std::string deviceClassString = plist_get_string_ptr(deviceClassEntry, nullptr);
            if(deviceClassString.empty()) {
                continue;
            }
            plist_t restoreBehaviorEntry = plist_dict_get_item(infoEntry, "RestoreBehavior");
            if(!restoreBehaviorEntry) {
                continue;
            }
            std::string restoreBehaviorString = plist_get_string_ptr(restoreBehaviorEntry, nullptr);
            if(restoreBehaviorString.empty()) {
                continue;
            }
            std::string chipIDString = plist_get_string_ptr(chipIDEntry, nullptr);
            if(chipIDString.empty()) {
                continue;
            }
            int chipIDInt = static_cast<int>(std::stoul(chipIDString, nullptr, 16));
            if(!chipIDInt) {
                continue;
            }
            if(chipIDInt == chipID) {
                if(!std::equal(deviceClass.begin(), deviceClass.end(), deviceClassString.begin())) {
                    continue;
                }
                switch(variant) {
                    case ERASE:
                        if(!std::equal(this->mEraseString.begin(), this->mEraseString.end(), restoreBehaviorString.begin())) {
                            continue;
                        }
                        break;
                    case UPDATE:
                        if(!std::equal(this->mUpdateString.begin(), this->mUpdateString.end(), restoreBehaviorString.begin())) {
                            continue;
                        }
                        break;
                    default:
                        return false;
                }
                this->pIdentity = plist_copy(subPlist);
//                fmt::print(fg(fmt::color::forest_green), "{0}: identity ptr: {1}\n", __PRETTY_FUNCTION__, this->pIdentity);
                return true;
            }
        }
    } while(subPlist);
    free(subPlist);
    return false;
}

Manifest::~Manifest() = default;
