//
// Created by cryptic on 1/28/23.
//

#include <Manifest.hpp>
#include <Lib.hpp>
#include <Timer.hpp>
#include <fmt/core.h>
#include <fmt/color.h>
#include <filesystem>
#include <fstream>
#include <utility>
#include <iostream>


Manifest::Manifest(std::string path) : mManifestPath(std::move(path)) {
    if(!this->readManifest()) {
        fmt::print(fg(fmt::color::crimson), "{0}: Failed to parse manifest!\n", __PRETTY_FUNCTION__);
        return;
    }
//    this->mManifest = this->mManifestStructure->GetPlist();
    if(!reinterpret_cast<PList::Dictionary *>(this->mManifestStructure.get())->size()) {
        fmt::print(fg(fmt::color::crimson), "{0}: manifest plist is empty!\n", __PRETTY_FUNCTION__);
        return;
    }
    this->mBuildIdentitiesArray = reinterpret_cast<PList::Array *>(reinterpret_cast<PList::Dictionary*>(this->mManifestStructure.get())->Find("BuildIdentities")->second);
    this->mManifestVersion = reinterpret_cast<PList::Dictionary*>(this->mManifestStructure.get())->Find("ManifestVersion")->second;
    this->mProductBuildVersion = reinterpret_cast<PList::Dictionary*>(this->mManifestStructure.get())->Find("ProductBuildVersion")->second;
    this->mProductVersion = reinterpret_cast<PList::Dictionary*>(this->mManifestStructure.get())->Find("ProductVersion")->second;
    this->mSupportedProductTypesArray = reinterpret_cast<PList::Dictionary*>(this->mManifestStructure.get())->Find("SupportedProductTypes")->second;
    if(!this->mBuildIdentitiesArray || !this->mBuildIdentitiesArray->GetSize() || !this->mBuildIdentitiesArray->size() || this->mBuildIdentitiesArray->GetType() != PLIST_ARRAY) {
        fmt::print(fg(fmt::color::crimson), "{0}: manifest plist is missing BuildIdentities array!\n", __PRETTY_FUNCTION__);
        return;
    }
    if(this->mManifestVersion->GetType() != PLIST_INT) {
        fmt::print(fg(fmt::color::crimson), "{0}: manifest plist is missing ManifestVersion!\n", __PRETTY_FUNCTION__);
        return;
    }
    if(this->mProductBuildVersion->GetType() != PLIST_STRING) {
        fmt::print(fg(fmt::color::crimson), "{0}: manifest plist is missing ProductBuildVersion!\n", __PRETTY_FUNCTION__);
        return;
    }
    if(this->mProductVersion->GetType() != PLIST_STRING) {
        fmt::print(fg(fmt::color::crimson), "{0}: manifest plist is missing ProductVersion!\n", __PRETTY_FUNCTION__);
        return;
    }
    if(this->mSupportedProductTypesArray->GetType() != PLIST_ARRAY) {
        fmt::print(fg(fmt::color::crimson), "{0}: manifest plist is missing SupportedProductTypes array!\n", __PRETTY_FUNCTION__);
        return;
    }
}

bool Manifest::matchIdentity(int chipID, std::string deviceClass, int variant, bool overwrite) {
    if(this->pIdentity && !overwrite) {
        return true;
    }
    if(!this->mBuildIdentitiesArray->size()) {
        fmt::print(fg(fmt::color::crimson), "{0}: build identities array is empty!\n", __PRETTY_FUNCTION__);
        return false;
    }
    for(auto & it : *reinterpret_cast<PList::Array *>(this->mBuildIdentitiesArray))
    {
        if(!it) {
            continue;
        }
        auto dict = reinterpret_cast<PList::Dictionary *>(it);
        auto chipidPair = *dict->Find("ApChipID");
        if(!chipidPair.first.empty()) {
            auto chipIDString = reinterpret_cast<PList::String *>(chipidPair.second)->GetValue();
            auto chipid = std::stoul(chipIDString, nullptr, 16);
            if(chipid != chipID) {
                continue;
            }
        }
        auto infoPair = *dict->Find("Info");
        if(!infoPair.first.empty()) {
            auto infoDict = reinterpret_cast<PList::Dictionary *>(infoPair.second);
            auto restoreBehaviorPair = *infoDict->Find("RestoreBehavior");
            if(!restoreBehaviorPair.first.empty()) {
                auto restoreBehaviorString = reinterpret_cast<PList::String *>(restoreBehaviorPair.second)->GetValue();
                switch(variant) {
                    case ERASE:
                        if (!std::equal(this->mEraseString.begin(), this->mEraseString.end(), restoreBehaviorString.begin())) {
                            continue;
                        }
                        break;
                    case UPDATE:
                        if (!std::equal(this->mUpdateString.begin(), this->mUpdateString.end(), restoreBehaviorString.begin())) {
                            continue;
                        }
                        break;
                    default:
                        return false;
                }
            }
            auto deviceClassPair = *infoDict->Find("DeviceClass");
            if(!deviceClassPair.first.empty()) {
                auto deviceClassString = reinterpret_cast<PList::String *>(deviceClassPair.second)->GetValue();
                if (!std::equal(deviceClass.begin(), deviceClass.end(), deviceClassString.begin())) {
                    continue;
                }
                this->pIdentity = it;
                return true;
            }
        }
    }
    return false;
}

bool Manifest::readManifest() {
    if(this->mManifestPath.empty()) {
        this->mManifestPath = fmt::format("{0}/{1}", std::string(std::filesystem::current_path()), std::string("BuildManifest.plist"));
    }
    std::ifstream manifestFileStream(this->mManifestPath, std::ios::in | std::ios::binary | std::ios::ate);
    if(!manifestFileStream.good()) {
        fmt::print(fg(fmt::color::crimson), "{0}: Failed to open {1} ({2})!\n", __PRETTY_FUNCTION__, this->mManifestPath, strerror(errno));
        return false;
    }
    std::streamsize manifestFileSize = manifestFileStream.tellg();
    if(!manifestFileSize) {
        fmt::print(fg(fmt::color::crimson), "{0}: Manifest is empty?\n", __PRETTY_FUNCTION__, this->mManifestPath);
        return false;
    }
    manifestFileStream.seekg(0, std::ios_base::beg);
    auto manifestBuffer = new std::string;
    manifestBuffer->assign(manifestFileSize, '\0');
    manifestFileStream.read(reinterpret_cast<char *>(&manifestBuffer->at(0)), manifestFileSize);
    if(!manifestFileStream.good()) {
        fmt::print(fg(fmt::color::crimson), "{0}: Failed to read {1} ({2})!\n", __PRETTY_FUNCTION__, this->mManifestPath, strerror(errno));
        return false;
    }
    if(plist_is_binary(manifestBuffer->c_str(), static_cast<uint32_t>(manifestFileSize))) {
        this->mManifestStructure = PList::ModernStructure::FromBin(*manifestBuffer);
    } else {
        this->mManifestStructure = PList::ModernStructure::FromXml(*manifestBuffer);
    }
    return true;
}

Manifest::~Manifest() = default;
