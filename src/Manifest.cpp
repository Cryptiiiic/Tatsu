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

bool Manifest::isValid() {
    auto a1 = this->mManifestStructure;
    auto a2 = this->mManifestStructure->size();
    auto a3 = this->mBuildIdentitiesArray;
    auto a4 = this->mBuildIdentitiesArray->size();
    return a1 && a2 && a3 && a4;
//    return this->mManifestStructure && this->mManifestStructure->size() && this->mBuildIdentitiesArray && this->mBuildIdentitiesArray->size();
}

Manifest::Manifest(std::string path) : mManifestPath(std::move(path)) {
    if(!this->readManifest()) {
        fmt::print(fg(fmt::color::crimson), "{0}: Failed to parse manifest!\n", __PRETTY_FUNCTION__);
        return;
    }
//    this->mManifest = this->mManifestStructure->GetPlist();
    if(!this->mManifestStructure->size()) {
        fmt::print(fg(fmt::color::crimson), "{0}: manifest plist is empty!\n", __PRETTY_FUNCTION__);
        return;
    }
    PList::Dictionary *structure = this->mManifestStructure;
    PList::Node *prop = nullptr;
    std::string find;
    std::string print;


    auto findProp = [&structure, &prop, &find, &print](PList::Dictionary *, PList::Node *, const std::string&, const std::string&){
        auto tmp = structure->Find(find);
        if(tmp->second) {
            prop = (PList::Node *)tmp->second;
            return;
        } else {
            fmt::print(fg(fmt::color::crimson), "{0}: could not find {1} {2}!\n", __PRETTY_FUNCTION__, find, print);
            return;
        }
    };
    findProp(structure=this->mManifestStructure, prop, find="BuildIdentities", print="array");
    this->mBuildIdentitiesArray = (PList::Array *)prop;
    findProp(structure=this->mManifestStructure, prop, find="ManifestVersion", print="int");
    this->mManifestVersion = (PList::Integer *)prop;
    findProp(structure=this->mManifestStructure, prop, find="ProductBuildVersion", print="string");
    this->mProductBuildVersion = (PList::String *)prop;
    findProp(structure=this->mManifestStructure, prop, find="ProductVersion", print="string");
    this->mProductVersion = (PList::String *)prop;
    findProp(structure=this->mManifestStructure, prop, find="SupportedProductTypes", print="array");
    this->mSupportedProductTypesArray = (PList::Array *)prop;
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
    for(auto & it : *this->mBuildIdentitiesArray)
    {
        if(!it) {
            continue;
        }
        auto dict = (PList::Dictionary *)(it);
        auto chipidPair = *dict->Find("ApChipID");
        if(!chipidPair.first.empty()) {
            auto chipIDString = ((PList::String *)chipidPair.second)->GetValue();
            auto chipid = std::stoul(chipIDString, nullptr, 16);
            if(chipid != chipID) {
                continue;
            }
        }
        auto infoPair = *dict->Find("Info");
        if(!infoPair.first.empty()) {
            auto infoDict = (PList::Dictionary *)(infoPair.second);
            auto infoDictEnd = infoDict->End();
            auto restoreBehaviorPair = *infoDict->Find("RestoreBehavior");
            if(!restoreBehaviorPair.first.empty()) {
                auto restoreBehaviorString = ((PList::String *)restoreBehaviorPair.second)->GetValue();
                switch(variant) {
                    case ERASE:
                        if (!std::equal(this->eraseString.begin(), this->eraseString.end(), restoreBehaviorString.begin())) {
                            continue;
                        }
                        break;
                    case UPDATE:
                        if (!std::equal(this->updateString.begin(), this->updateString.end(), restoreBehaviorString.begin())) {
                            continue;
                        }
                        break;
                    default:
                        return false;
                }
            }
            auto deviceClassPair = *infoDict->Find("DeviceClass");
            auto uidMode = infoDict->Find("RequiresUIDMode");
            if(uidMode != infoDictEnd) {
                if(!uidMode->first.empty()) {
                    auto uidBool = (PList::Boolean *)uidMode->second;
                    if(uidBool->GetValue()) {
                        this->requiresUIDMode = true;
                    }
                }
            }
            if(!deviceClassPair.first.empty()) {
                auto deviceClassString = ((PList::String *)deviceClassPair.second)->GetValue();
                if (!std::equal(deviceClass.begin(), deviceClass.end(), deviceClassString.begin())) {
                    continue;
                }
                this->pIdentity = dict;
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
    manifestFileStream.read(&manifestBuffer->at(0), manifestFileSize);
    if(!manifestFileStream.good()) {
        manifestFileStream.close();
        fmt::print(fg(fmt::color::crimson), "{0}: Failed to read {1} ({2})!\n", __PRETTY_FUNCTION__, this->mManifestPath, strerror(errno));
        return false;
    }
    if(plist_is_binary(manifestBuffer->c_str(), static_cast<uint32_t>(manifestFileSize))) {
        this->mManifestStructure = (PList::Dictionary *)(PList::ModernStructure::FromBin(*manifestBuffer));
    } else {
        this->mManifestStructure = (PList::Dictionary *)(PList::ModernStructure::FromXml(*manifestBuffer));
    }
    manifestFileStream.close();
    return true;
}

Manifest::~Manifest() = default;