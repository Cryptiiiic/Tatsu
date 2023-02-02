//
// Created by cryptic on 2/1/23.
//

#include <Modern.hpp>
#include <plist/plist++.h>
#include <fmt/color.h>

namespace PList {
    static std::shared_ptr<Structure> ImportStruct(plist_t root) {
        std::shared_ptr<Structure> ret = nullptr;
        plist_type type = plist_get_node_type(root);

        if (PLIST_ARRAY == type || PLIST_DICT == type) {
            ret = std::reinterpret_pointer_cast<Structure>(ModernNode::FromPlist(root));
        } else {
            plist_free(root);
        }

        return ret;
    }

    std::shared_ptr<Structure> ModernStructure::FromXml(const std::string &xml) {
        plist_t root = nullptr;
        plist_from_xml(xml.c_str(), xml.size(), &root);
        return ImportStruct(root);
    }

    std::shared_ptr<Structure> ModernStructure::FromBin(const std::string &bin) {
        plist_t root = nullptr;
        plist_from_bin(bin.c_str(), bin.size(), &root);
        return ImportStruct(root);
    }

    std::shared_ptr<Structure> ModernStructure::FromBin(const std::vector<char> &bin) {
        plist_t root = nullptr;
        plist_from_bin(&bin[0], bin.size(), &root);
        return ImportStruct(root);
    }

    std::shared_ptr<ModernNode> ModernNode::FromPlist(plist_t node, ModernNode *parent) {
        std::shared_ptr<ModernNode> ret = nullptr;
        if (node)
        {
            plist_type type = plist_get_node_type(node);
            switch (type)
            {
                case PLIST_DICT:
                    ret = std::reinterpret_pointer_cast<ModernNode>(std::make_shared<Dictionary>(Dictionary(node, parent)));
                    break;
                case PLIST_ARRAY:
                    ret = std::reinterpret_pointer_cast<ModernNode>(std::make_shared<Array>(node, parent));
                    break;
                case PLIST_BOOLEAN:
                    ret = std::reinterpret_pointer_cast<ModernNode>(std::make_shared<Boolean>(node, parent));
                    break;
                case PLIST_UINT:
                    ret = std::reinterpret_pointer_cast<ModernNode>(std::make_shared<Integer>(node, parent));
                    break;
                case PLIST_REAL:
                    ret = std::reinterpret_pointer_cast<ModernNode>(std::make_shared<Real>(node, parent));
                    break;
                case PLIST_STRING:
                    ret = std::reinterpret_pointer_cast<ModernNode>(std::make_shared<String>(node, parent));
                    break;
                case PLIST_KEY:
                    ret = std::reinterpret_pointer_cast<ModernNode>(std::make_shared<Key>(node, parent));
                    break;
                case PLIST_UID:
                    ret = std::reinterpret_pointer_cast<ModernNode>(std::make_shared<Uid>(node, parent));
                    break;
                case PLIST_DATE:
                    ret = std::reinterpret_pointer_cast<ModernNode>(std::make_shared<Date>(node, parent));
                    break;
                case PLIST_DATA:
                    ret = std::reinterpret_pointer_cast<ModernNode>(std::make_shared<Data>(node, parent));
                    break;
                default:
                    plist_free(node);
                    break;
            }
        }
        return ret;
    }
}
