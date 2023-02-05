//
// Created by cryptic on 2/1/23.
//

#include <Modern.hpp>
#include <Timer.hpp>
#include <plist/plist++.h>
#include <fmt/color.h>

namespace PList {
    static ModernStructure *ImportStruct(plist_t root) {
        ModernStructure *ret = nullptr;
        plist_type type = plist_get_node_type(root);
        if (PLIST_ARRAY == type || PLIST_DICT == type) {
            ret = static_cast<ModernStructure *>(ModernNode::FromPlist(root));
        } else {
            plist_free(root);
        }
        return ret;
    }

    ModernStructure *ModernStructure::FromXml(const std::string &xml) {
        plist_t root = nullptr;
        TIMER();
        plist_from_xml(xml.c_str(), xml.size(), &root);
        TIMER1();
        return ImportStruct(root);
    }

    ModernStructure * ModernStructure::FromBin(const std::string &bin) {
        plist_t root = nullptr;
        plist_from_bin(bin.c_str(), bin.size(), &root);
        return ImportStruct(root);
    }

    ModernStructure * ModernStructure::FromBin(const std::vector<char> &bin) {
        plist_t root = nullptr;
        plist_from_bin(&bin[0], bin.size(), &root);
        return ImportStruct(root);
    }

    std::shared_ptr<Dictionary> dict_;
    std::shared_ptr<Array> arr_;
    std::shared_ptr<Boolean> bool_;
    std::shared_ptr<Integer> int_;
    std::shared_ptr<Real> real;
    std::shared_ptr<String> string_;
    std::shared_ptr<Key> key_;
    std::shared_ptr<Uid> uid_;
    std::shared_ptr<Date> date_;
    std::shared_ptr<Data> data_;

    ModernNode *ModernNode::FromPlist(plist_t node, ModernNode *parent) {
        ModernNode *ret = nullptr;
        if (node)
        {
            plist_type type = plist_get_node_type(node);
            switch (type)
            {
                case PLIST_DICT:
                    dict_ = std::make_shared<Dictionary>(node, parent);
                    if(!dict_ || !dict_->size()) {
                        plist_free(node);
                        break;
                    }
                    return (ModernNode *)dict_.get();
                case PLIST_ARRAY:
                    arr_ = std::make_shared<Array>(node, parent);
                    if(!arr_ || !arr_->size()) {
                        plist_free(node);
                        break;
                    }
                    return (ModernNode *)arr_.get();
                case PLIST_BOOLEAN:
                    bool_ = std::make_shared<Boolean>(node, parent);
                    if(!bool_) {
                        plist_free(node);
                        break;
                    }
                    return (ModernNode *)bool_.get();
                case PLIST_INT:
                    int_ = std::make_shared<Integer>(node, parent);
                    if(!int_) {
                        plist_free(node);
                        break;
                    }
                    return (ModernNode *)int_.get();
                case PLIST_REAL:
                    real = std::make_shared<Real>(node, parent);
                    if(!real) {
                        plist_free(node);
                        break;
                    }
                    return (ModernNode *)real.get();
                case PLIST_STRING:
                    string_ = std::make_shared<String>(node, parent);
                    if(!string_) {
                        plist_free(node);
                        break;
                    }
                    return (ModernNode *)string_.get();
                case PLIST_KEY:
                    key_ = std::make_shared<Key>(node, parent);
                    if(!key_) {
                        plist_free(node);
                        break;
                    }
                    return (ModernNode *)key_.get();
                case PLIST_UID:
                    uid_ = std::make_shared<Uid>(node, parent);
                    if(!uid_) {
                        plist_free(node);
                        break;
                    }
                    return (ModernNode *)uid_.get();
                case PLIST_DATE:
                    date_ = std::make_shared<Date>(node, parent);
                    if(!date_) {
                        plist_free(node);
                        break;
                    }
                    return (ModernNode *)date_.get();
                case PLIST_DATA:
                    data_ = std::make_shared<Data>(node, parent);
                    if(!data_) {
                        plist_free(node);
                        break;
                    }
                    return (ModernNode *)data_.get();
                default:
                    plist_free(node);
                    return nullptr;
            }
        }
        return nullptr;
    }
}
