//
// Created by cryptic on 2/1/23.
//

#ifndef TATSU_MODERN_HPP
#define TATSU_MODERN_HPP

#pragma once
#include <plist/plist++.h>

namespace PList {

    class ModernNode : public Node {
    private:
    public:
        static ModernNode *FromPlist(plist_t node, ModernNode *parent = nullptr);
        friend class ModernStructure;
    };

    class ModernStructure : public ModernNode {
    public:
        static ModernStructure *FromXml(const std::string &xml);
        static ModernStructure *FromBin(const std::string &bin);
        static ModernStructure *FromBin(const std::vector<char> &bin);
    private:
//        ModernStructure& operator=(const ModernStructure& s);
    };

//     class ModernDictionary : Dictionary {
//     public:
//        auto begin () { return this->Begin(); }
//        auto end () { return this->End(); }
//     private:
//    };
}


#endif //TATSU_MODERN_HPP
