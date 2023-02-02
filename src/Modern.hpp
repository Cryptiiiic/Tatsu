//
// Created by cryptic on 2/1/23.
//

#ifndef TATSU_MODERN_HPP
#define TATSU_MODERN_HPP

#pragma once
#include <plist/plist++.h>

namespace PList {

    class ModernStructure : public Structure {
    public:
        static std::shared_ptr<Structure> FromXml(const std::string &xml);
        static std::shared_ptr<Structure> FromBin(const std::string &bin);
        static std::shared_ptr<Structure> FromBin(const std::vector<char> &bin);
    private:
//        ModernStructure& operator=(const ModernStructure& s);
    };

    class ModernNode : public Node {
    private:
    public:
        static std::shared_ptr<ModernNode>FromPlist(plist_t node, ModernNode *parent = nullptr);
    };

     class ModernDictionary : Dictionary {
     public:
        auto begin () { return this->Begin(); }
        auto end () { return this->End(); }
     private:
    };
}


#endif //TATSU_MODERN_HPP
