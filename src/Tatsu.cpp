//
// Created by cryptic on 1/30/23.
//

#include <Tatsu.hpp>

#include <utility>
#include <random>
#include <fmt/color.h>
#include <digestpp/digestpp.hpp>
#include <chrono>
#include <iostream>
#include <unordered_map>
#include <Timer.hpp>
#include <Requests.hpp>
#include <Lib.hpp>

Tatsu::Tatsu(Manifest *manifest, int chipID, std::string deviceClass, uint64_t ECID, int variant, uint64_t generator, std::string apNonce, std::string sepNonce, std::vector<std::string> componentList)
: mManifest(manifest), mChipID(chipID), mDeviceClass(std::move(deviceClass)), mECID(ECID), mVariant(variant), mGenerator(generator), mAPNonce(std::move(apNonce)), mSEPNonce(std::move(sepNonce)), mComponentList(std::move(componentList)) {
    TIMER_START();
    if(!this->mManifest) {
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
//    fmt::print(fg(fmt::color::forest_green), "generator: 0x{0:16X}\n", this->mGenerator);
    if(this->mAPNonce.empty()) {
        std::string apnonce;
        if(this->mChipID < 0x8010) {
            digestpp::sha1().reset();
            apnonce = digestpp::sha1().absorb(reinterpret_cast<const char *>(&this->mGenerator), 8).hexdigest();
            digestpp::sha1().reset();
            digestpp::sha1().absorb(reinterpret_cast<const char *>(&this->mGenerator), 8).digest<unsigned char>(this->mAPNonceDGST, 32);
            this->mAPNonce = apnonce;
        } else {
            digestpp::sha384().reset();
            apnonce = digestpp::sha384().absorb(reinterpret_cast<const char *>(&this->mGenerator), 8).hexdigest();
            apnonce.resize(64);
            digestpp::sha384().reset();
            digestpp::sha384().absorb(reinterpret_cast<const char *>(&this->mGenerator), 8).digest<unsigned char>(this->mAPNonceDGST, 64);
            this->mAPNonce = apnonce;
        }
        if (apnonce.empty()) {
            return;
        }
    }
//    fmt::print(fg(fmt::color::forest_green), "apnonce: {0:s}\n", this->mAPNonce);
    if(this->mSEPNonce.empty()) {
        std::random_device rngDevice;
        std::mt19937 rng(rngDevice());
        std::uniform_int_distribution<std::mt19937_64::result_type> distributionRNG(0, UINT64_MAX);
        this->mTMPGenerator = distributionRNG(rng);
        digestpp::sha1().reset();
        this->mSEPNonce = digestpp::sha1().absorb(reinterpret_cast<const char *>(&this->mTMPGenerator), 8).hexdigest();
        digestpp::sha1().reset();
        digestpp::sha1().absorb(reinterpret_cast<const char *>(&this->mTMPGenerator), 8).digest<unsigned char>(this->mSEPNonceDGST, 32);
//        fmt::print(fg(fmt::color::forest_green), "sepnonce: {0:s}\n", this->mSEPNonce);
    }
    this->initParameters();
//    debug_plist(this->mParameters);
    uint32_t size = 0;
    char* data = nullptr;
    int ret = plist_to_xml(this->mParameters, &data, &size);
    std::string body(data);
    auto *rq = new Requests();
    if(ret == PLIST_ERR_SUCCESS) {
        rq->sendPOST(body);
    } else {
        fmt::print(fmt::fg(fmt::color::crimson), "{0}: Failed to convert plist to xml error: {1}\n", __PRETTY_FUNCTION__, ret);
    }
    TIMER_STOP();
}

bool Tatsu::initParameters() {
    this->mParameters = plist_new_dict();
    plist_dict_set_item(this->mParameters, "@BBTicket", plist_new_bool(false));
    plist_dict_set_item(this->mParameters, "@Locality", plist_new_string("en_US"));
    plist_dict_set_item(this->mParameters, "@HostPlatformInfo", plist_new_string("mac"));
    plist_dict_set_item(this->mParameters, "@VersionInfo", plist_new_string(TSS_VERSION_STRING));
    plist_dict_set_item(this->mParameters, "@UUID", plist_new_string(generateUUID().c_str()));
    this->initFromIdentity();
    this->initIMG4();
    this->initComponents();
    return false;
}

bool Tatsu::initFromIdentity() {
    if(!this->mManifest->matchIdentity(this->mChipID, this->mDeviceClass, this->mVariant)) {
        return false;
    }
    for(const std::string& entry: entries) {
            if(plist_dict_get_item(this->mManifest->pIdentity, entry.c_str())) {
                plist_dict_set_item(this->mParameters, entry.c_str(),
                                    plist_copy(plist_dict_get_item(this->mManifest->pIdentity, entry.c_str())));
            }
    }
    return false;
}

bool Tatsu::initIMG4() {
    if(!this->mBasebandSerialNumber.empty() && this->mBasebandGoldCertID > 0) {
        plist_dict_set_item(this->mParameters, "@BBTicket", plist_new_bool(true));
        plist_dict_set_item(this->mParameters, "BbSNUM", plist_new_string(this->mBasebandSerialNumber.c_str()));
        plist_dict_set_item(this->mParameters, "BbGoldCertId", plist_new_uint(this->mBasebandGoldCertID));
    }
    plist_dict_set_item(this->mParameters, "@ApImg4Ticket", plist_new_bool(true));
    plist_dict_set_item(this->mParameters, "ApSecurityMode", plist_new_bool(true));
    plist_dict_set_item(this->mParameters, "ApProductionMode", plist_new_bool(true));
    plist_dict_set_item(this->mParameters, "ApNonce", plist_new_data(reinterpret_cast<const char *>(this->mAPNonceDGST), this->mAPNonce.length() / 2));
    plist_dict_set_item(this->mParameters, "SepNonce", plist_new_data(reinterpret_cast<const char *>(this->mSEPNonceDGST), this->mSEPNonce.length() / 2));
    plist_dict_set_item(this->mParameters, "ApECID", plist_new_uint(this->mECID));
    return false;
}

bool Tatsu::initComponents() {
    plist_dict_iter iter = nullptr;
    char *key = nullptr;
    plist_t val = nullptr;
    plist_t subPlist = nullptr;
    plist_t manifest = plist_copy(plist_dict_get_item(this->mManifest->pIdentity, "Manifest"));
    if(!manifest) {
        return false;
    }
    std::array<const char *, 4> keys = { "BasebandFirmware", "SE,UpdatePayload", "BaseSystem", "Diags" };
    for(auto k : keys) {
        if(plist_dict_get_item(manifest, k)) {
            plist_dict_remove_item(manifest, k);
        }
    }
    uint32_t count = plist_dict_get_size(manifest);
    if(!count) {
        return false;
    }
    plist_dict_new_iter(manifest, &iter);
    if(!iter) {
        return false;
    }
    do {
        plist_dict_next_item(manifest, iter, &key, &val);
        if(key) {
            std::string tmp(key);
            if(!plist_dict_get_item(val, "Info")) {
                plist_dict_remove_item(manifest, key);
                continue;
            } else {
                if(!plist_dict_get_item(plist_dict_get_item(val, "Info"), "RestoreRequestRules")) {
                    plist_dict_remove_item(manifest, key);
                    continue;
                }
            }
            bool addComponent = false;
            addComponent = this->mComponentList.empty();
            for(auto idx : this->mComponentList) {
                std::cout << idx << std::endl;
                if(std::equal(tmp.begin(), tmp.end(), idx.begin())) {
                    addComponent = true;
                    break;
                }
            }
            if(addComponent) {
                plist_t val2 = plist_copy(val);
                plist_dict_set_item(this->mParameters, key, val2);
                if(plist_dict_get_item(val2, "Trusted")) {
                    char *digestVal = nullptr;
                    uint64_t digestLen = 0;
                    plist_get_data_val(plist_dict_get_item(val2, "Digest"), &digestVal, &digestLen);
                    if(!digestLen && !digestVal) {
                        plist_dict_set_item(val2, "Digest", plist_new_data(nullptr, 0));
                    }
                }
                this->initRestoreRequestRules(val2);
            }
        }
    } while(key);
    if(plist_dict_get_item(manifest, "Manifest")) {
        plist_dict_remove_item(manifest, "Manifest");
    }
    return true;
}

bool Tatsu::initRestoreRequestRules(plist_t entry) {
    char *tmp = nullptr;
    plist_dict_get_item_key(entry, &tmp);
    plist_array_iter iter = nullptr;
    plist_t subPlist = nullptr;
    if(!entry) {
        return false;
    }
    plist_t rules = plist_dict_get_item(plist_dict_get_item(entry, "Info"), "RestoreRequestRules");
    if(!rules) {
        return false;
    }
    uint32_t count = plist_array_get_size(rules);
    if(!count) {
        return false;
    }
    plist_array_new_iter(rules, &iter);
    if(!iter) {
        return false;
    }
    do {
        plist_array_next_item(rules, iter, &subPlist);
        if (subPlist) {
            plist_t conditions = plist_dict_get_item(subPlist, "Conditions");
            plist_t actions = plist_dict_get_item(subPlist, "Actions");
            if(!conditions || !actions) {
                break;
            } else {
                std::unordered_map<std::string, std::string> keyMap = {
                        {"ApRawProductionMode", "ApProductionMode"},
                        {"ApCurrentProductionMode", "ApProductionMode"},
                        {"ApRawSecurityMode", "ApSecurityMode"},
                        {"ApRequiresImage4", "ApSupportsImg4"},
                        {"ApDemotionPolicyOverride", "DemotionPolicy"},
                        {"ApInRomDFU", "ApInRomDFU"},
                };
                plist_dict_iter conditionsIter = nullptr;
                char *conditionsKey = nullptr;
                plist_t conditionsVal = nullptr;
                plist_dict_new_iter(conditions, &conditionsIter);
                plist_dict_next_item(conditions, conditionsIter, &conditionsKey, &conditionsVal);
                plist_dict_iter actionsIter = nullptr;
                char *actionsKey = nullptr;
                plist_t actionsVal = nullptr;
                plist_dict_new_iter(actions, &actionsIter);
                plist_dict_next_item(actions, actionsIter, &actionsKey, &actionsVal);
                tmp = nullptr;
                plist_dict_get_item_key(conditionsVal, &tmp);
                bool paramValue = false;
                plist_get_bool_val(plist_dict_get_item(this->mParameters, keyMap.at(tmp).c_str()),
                                   reinterpret_cast<uint8_t *>(&paramValue));
                if(paramValue && conditionsVal) {
                    plist_dict_set_item(entry, actionsKey, plist_new_bool(paramValue));

                }
            }
        }
    } while(subPlist);
    if(plist_dict_get_item(entry, "Info")) {
        plist_dict_remove_item(entry, "Info");
    }
    return true;
}

Tatsu::~Tatsu() = default;
