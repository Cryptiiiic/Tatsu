//
// Created by cryptic on 1/30/23.
//

#include <Tatsu.hpp>

#include <utility>
#include <random>
#include <iostream>
#include <fmt/color.h>
#include <digestpp/digestpp.hpp>
#include <chrono>
#include <unordered_map>
#include <filesystem>
#include <fstream>
#include <Timer.hpp>
#include <Requests.hpp>
#include <Lib.hpp>

Tatsu::Tatsu(std::shared_ptr<Manifest> manifest, int chipID, std::string deviceClass, uint64_t ECID, int variant, uint64_t generator, std::string apNonce, std::string sepNonce, std::vector<std::string> componentList)
: mManifest(std::move(manifest)), mChipID(chipID), mDeviceClass(std::move(deviceClass)), mECID(ECID), mVariant(variant), mGenerator(generator), mAPNonce(std::move(apNonce)), mSEPNonce(std::move(sepNonce)), mComponentList(std::move(componentList)) {
    if(!this->mManifest || !this->mManifest.get()) {
        return;
    }
    if(this->mChipID <= 0) {
        return;
    }
    if(this->mDeviceClass.empty()) {
        return;
    }
    if(this->mECID == -1) {
        return;
    }
    if(this->mVariant < 0) {
        this->mVariant = ERASE;
    }
    if(this->mGenerator == -1) {
        std::random_device rngDevice;
        std::mt19937 rng(rngDevice());
        std::uniform_int_distribution<std::mt19937_64::result_type> distributionRNG(0, UINT64_MAX);
        this->mGenerator = distributionRNG(rng);
    }
    if(this->mAPNonce.empty()) {
        std::string apnonce;
        if(this->mChipID < 0x8010) {
            digestpp::sha1().reset();
            apnonce = digestpp::sha1().absorb(reinterpret_cast<const char *>(&this->mGenerator), 8).hexdigest();
            digestpp::sha1().reset();
            digestpp::sha1().absorb(reinterpret_cast<const char *>(&this->mGenerator), 8).digest<unsigned char>(this->mAPNonceDGST, 32);
            this->mAPNonce = apnonce;
            this->mAPNonceDGSTVector.insert(this->mAPNonceDGSTVector.end(), this->mAPNonceDGST, this->mAPNonceDGST + 20);
        } else {
            digestpp::sha384().reset();
            apnonce = digestpp::sha384().absorb(reinterpret_cast<const char *>(&this->mGenerator), 8).hexdigest();
            apnonce.resize(64);
            digestpp::sha384().reset();
            digestpp::sha384().absorb(reinterpret_cast<const char *>(&this->mGenerator), 8).digest<unsigned char>(this->mAPNonceDGST, 64);
            this->mAPNonceDGSTVector.insert(this->mAPNonceDGSTVector.end(), this->mAPNonceDGST, this->mAPNonceDGST + 40);
            this->mAPNonce = apnonce;
        }
        if (apnonce.empty()) {
            return;
        }
    }
    if(this->mSEPNonce.empty()) {
        std::random_device rngDevice;
        std::mt19937 rng(rngDevice());
        std::uniform_int_distribution<std::mt19937_64::result_type> distributionRNG(0, UINT64_MAX);
        this->mTMPGenerator = distributionRNG(rng);
        digestpp::sha1().reset();
        this->mSEPNonce = digestpp::sha1().absorb(reinterpret_cast<const char *>(&this->mTMPGenerator), 8).hexdigest();
        digestpp::sha1().reset();
        digestpp::sha1().absorb(reinterpret_cast<const char *>(&this->mTMPGenerator), 8).digest<unsigned char>(this->mSEPNonceDGST, 32);
        this->mSEPNonceDGSTVector.insert(this->mSEPNonceDGSTVector.end(), this->mSEPNonceDGST, this->mSEPNonceDGST + 20);
    }
    this->initParameters();
    uint32_t size = 0;
    char* data = nullptr;
//    debug_plist(this->mParameters->GetPlist());
    int ret = plist_to_xml(this->mParameters->GetPlist(), &data, &size);
    std::string body(data);
    auto *rq = new Requests();
    if(ret == PLIST_ERR_SUCCESS) {
        this->writeBlob(rq->sendPOST(body));
    } else {
        fmt::print(fmt::fg(fmt::color::crimson), "{0}: Failed to convert plist to xml error: {1}\n", __PRETTY_FUNCTION__, ret);
    }
}

bool Tatsu::initParameters() {
    this->mParameters = std::make_unique<PList::Dictionary>(new PList::Dictionary);
    this->mParameters->Set("@BBTicket", std::make_unique<PList::Boolean>(false).get());
    this->mParameters->Set("@Locality", std::make_unique<PList::String>(std::string("en_US")).get());
    this->mParameters->Set("@HostPlatformInfo", std::make_unique<PList::String>(std::string("mac")).get());
    this->mParameters->Set("@VersionInfo", std::make_unique<PList::String>(std::string(TSS_VERSION_STRING)).get());
    this->mParameters->Set("@UUID", std::make_unique<PList::String>(std::string(generateUUID())).get());
    this->initFromIdentity();
    this->initIMG4();
    this->initComponents();
    return true;
}

bool Tatsu::initFromIdentity() {
    if(!this->mManifest->matchIdentity(this->mChipID, this->mDeviceClass, this->mVariant)) {
        return false;
    }
    for(const std::string& entry: entries) {
        auto ident = reinterpret_cast<PList::Dictionary *>(this->mManifest->pIdentity);
        if(!ident) {
            continue;
        }
        auto identIter = ident->Find(entry);
        auto find = *identIter;
        if(!validateString(find.first)) {
            continue;
        }
        if(!find.first.empty() && find.second->GetType() != PLIST_NULL && find.second->GetType() != PLIST_NONE) {
            this->mParameters->Set(entry, find.second);
        }
    }
    return true;
}

bool Tatsu::initIMG4() {
    if(!this->mBasebandSerialNumber.empty() && this->mBasebandGoldCertID > 0) {
        this->mParameters->Set("@BBTicket", std::make_unique<PList::Boolean>(true).get());
        this->mParameters->Set("BbSNUM", std::make_unique<PList::String>(std::string(this->mBasebandSerialNumber)).get());
        this->mParameters->Set("@BBTicket", std::make_unique<PList::Integer>(static_cast<int64_t>(this->mBasebandGoldCertID)).get());
    }
    this->mParameters->Set("@ApImg4Ticket", std::make_unique<PList::Boolean>(true).get());
    this->mParameters->Set("ApSecurityMode", std::make_unique<PList::Boolean>(true).get());
    this->mParameters->Set("ApProductionMode", std::make_unique<PList::Boolean>(true).get());
    this->mParameters->Set("ApNonce", std::make_unique<PList::Data>(this->mAPNonceDGSTVector).get());
    this->mParameters->Set("SepNonce", std::make_unique<PList::Data>(this->mAPNonceDGSTVector).get());
    this->mParameters->Set("ApECID", std::make_unique<PList::Integer>(this->mECID).get());
    return true;
}

bool Tatsu::initComponents() {
    auto identDict = reinterpret_cast<PList::Dictionary *>(this->mManifest->pIdentity);
    if(!identDict) {
        return false;
    }
    auto manifestDict = identDict->Find("Manifest");
    PList::Node *manifestOrig = manifestDict->second;
    if(!manifestOrig) {
        return false;
    }
    auto manifestSmart = std::make_unique<PList::Dictionary>(manifestOrig);
    auto manifest = manifestSmart.get();
    manifest = reinterpret_cast<PList::Dictionary *>(manifestOrig);
    std::array<std::string, 4> keys = { "BasebandFirmware", "SE,UpdatePayload", "BaseSystem", "Diags" };
    for(auto &k : keys) {
        auto find = *manifest->Find(k);
        if(!validateString(find.first)) {
            continue;
        }
        if(!find.first.empty() && find.second->GetType() != PLIST_NULL && find.second->GetType() != PLIST_NONE) {
            manifest->Remove(manifest->Find(k)->second);
        }
    }
    uint32_t count = manifest->size();
    if(!count) {
        return false;
    }
    for(auto &it : *manifest) {
        auto dict = reinterpret_cast<PList::Dictionary *>(it.second);
        if(!it.first.empty() && it.second->GetType() != PLIST_NULL && it.second->GetType() != PLIST_NONE) {
            // code not needed
        } else {
            continue;
        }
        auto find = *dict->Find("Info");
        if(!find.first.empty() && find.second->GetType() != PLIST_NULL && find.second->GetType() != PLIST_NONE) {
            auto find2 = *reinterpret_cast<PList::Dictionary *>(find.second)->Find("RestoreRequestRules");
            if(!find2.first.empty() && find2.second->GetType() != PLIST_NULL && find2.second->GetType() != PLIST_NONE) {
                // no code needed
            } else {
                dict->Remove(find2.second);
                continue;
            }
        } else {
            dict->Remove(find.second);
            continue;
        }

        bool addComponent = false;
        addComponent = this->mComponentList.empty();
        if(!addComponent) {
            for(auto idx : this->mComponentList) {
                if(std::equal(it.first.begin(), it.first.end(), idx.begin())) {
                    addComponent = true;
                    break;
                } else {
                    addComponent = false;
                }
            }
        }
        if(addComponent) {
            auto find3 = *dict->Find("Trusted");
            if(!find3.first.empty() && find3.second->GetType() != PLIST_NULL && find3.second->GetType() != PLIST_NONE) {
                auto find4 = *dict->Find("Digest");
                if(!validateString(find4.first)) {
                    continue;
                }
                if(!find4.first.empty() && find4.second->GetType() != PLIST_NULL && find4.second->GetType() != PLIST_NONE) {
                    auto vec = reinterpret_cast<PList::Data *>(find4.second)->GetValue();
                    if(reinterpret_cast<PList::Data *>(find4.second)->GetValue().empty()) {
                        dict->Set("Digest", std::make_unique<PList::Data>().get());
                    }
                } else {
                    dict->Set("Digest", std::make_unique<PList::Data>().get());
                }
            }
            this->initRestoreRequestRules(it.second);
            this->mParameters->Set(it.first, it.second);
        }
    }
    manifest->Remove("Manifest");
    return true;
}

bool Tatsu::initRestoreRequestRules(PList::Node *entry) {
    auto entryDict = reinterpret_cast<PList::Dictionary *>(entry);
    auto info = *entryDict->Find("Info");
    if(!info.first.empty() && info.second->GetType() != PLIST_NULL && info.second->GetType() != PLIST_NONE) {
        auto restoreRequestRules = *reinterpret_cast<PList::Dictionary *>(info.second)->Find("RestoreRequestRules");
        if(!restoreRequestRules.first.empty() && restoreRequestRules.second->GetType() != PLIST_NULL && restoreRequestRules.second->GetType() != PLIST_NONE) {
            // no code needed
        } else {
            return false;
        }
    } else {
        return false;
    }
    auto restoreRequestRules = *reinterpret_cast<PList::Dictionary *>(info.second)->Find("RestoreRequestRules");
    auto array = reinterpret_cast<PList::Array *>(restoreRequestRules.second);
    if(!array->size()) {
        return false;
    }
    for(auto & it : *array) {
        if(!it) {
            break;
        }
        auto dict = reinterpret_cast<PList::Dictionary *>(it);
        auto conditions = *dict->Find("Conditions");
        if(!conditions.first.empty() && conditions.second->GetType() != PLIST_NULL && conditions.second->GetType() != PLIST_NONE) {
            // no code needed
        } else {
            break;
        }
        auto actions = *dict->Find("Actions");
        if(!actions.first.empty() && actions.second->GetType() != PLIST_NULL && actions.second->GetType() != PLIST_NONE) {
            // no code needed
        } else {
            break;
        }
        std::unordered_map<std::string, std::string> keyMap = {
            {"ApRawProductionMode", "ApProductionMode"},
            {"ApCurrentProductionMode", "ApProductionMode"},
            {"ApRawSecurityMode", "ApSecurityMode"},
            {"ApRequiresImage4", "ApSupportsImg4"},
            {"ApDemotionPolicyOverride", "DemotionPolicy"},
            {"ApInRomDFU", "ApInRomDFU"},
        };
        auto conditionsKey = reinterpret_cast<PList::Dictionary *>(conditions.second)->begin()->first;
        auto conditionsValue = reinterpret_cast<PList::Dictionary *>(conditions.second)->begin()->second;
        auto actionsKey = reinterpret_cast<PList::Dictionary *>(actions.second)->begin()->first;
        auto actionsValue = reinterpret_cast<PList::Dictionary *>(actions.second)->begin()->second;
        bool paramValue = reinterpret_cast<PList::Boolean *>(this->mParameters->Find(keyMap.at(conditionsKey))->second)->GetValue();
        if(paramValue && conditionsValue) {
            entryDict->Set(actionsKey, std::make_unique<PList::Boolean>(paramValue).get());
        }
    }
    entryDict->Remove("Info");
    return true;
}

bool Tatsu::writeBlob(const std::string &blob) {
    if(blob.empty()) {
        return false;
    }
    auto blobOut = std::string(blob);
    if(blob.find("STATUS=0&MESSAGE=SUCCESS") != std::string::npos) {
        blobOut.erase(0, 40);
    } else {
        fmt::print(fg(fmt::color::crimson), "{0}: Failed to save SHSH blobs!", __PRETTY_FUNCTION__);
        return false;
    }
//    auto blobPlist = PList::ModernStructure::FromXml(blobOut);
//    debug_plist(blobPlist->GetPlist());
    auto blobPath = fmt::format("{0}/{1}", std::string(std::filesystem::current_path()), std::string("blob.shsh2"));
    std::ofstream blobFileStream(blobPath, std::ios::out | std::ios::binary | std::ios::beg);
    if(!blobFileStream.good()) {
        fmt::print(fg(fmt::color::crimson), "{0}: Failed to open {1} for writing ({2})!\n", __PRETTY_FUNCTION__, blobPath, strerror(errno));
        return false;
    }
    blobFileStream.write(reinterpret_cast<char *>(&blobOut.at(0)), blobOut.size());
    if(!blobFileStream.good()) {
        fmt::print(fg(fmt::color::crimson), "{0}: Failed to write {1} ({2})!\n", __PRETTY_FUNCTION__, blobPath, strerror(errno));
        return false;
    }
    fmt::print(fg((fmt::color)0x00c200), "{0}: Successfully saved SHSH blobs!\n", __PRETTY_FUNCTION__);
    return true;
}

Tatsu::~Tatsu() = default;
