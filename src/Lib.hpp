//
// Created by cryptic on 1/30/23.
//

#ifndef TATSU_LIB_HPP
#define TATSU_LIB_HPP

#pragma once
#include <string>
#include <random>
#include <digestpp/hasher.hpp>
#include <digestpp/digestpp.hpp>
#include <plist/plist++.h>
#include <fmt/core.h>
#include <fmt/color.h>

class RND {
public:
    uint64_t tmp = 0;
};

static std::string generateUUID() {
    RND *rnd = new RND();
    std::random_device rngDevice;
    std::mt19937 rng(rngDevice());
    std::uniform_int_distribution<std::mt19937_64::result_type> distributionRNG(0, UINT64_MAX);
    rnd->tmp = distributionRNG(rng);
    digestpp::sha384().reset();
    std::string hash = digestpp::sha384().absorb(reinterpret_cast<const char *>(&rnd->tmp), 8).hexdigest();
    std::transform(hash.begin(), hash.end(), hash.begin(), ::toupper);
    hash.resize(32);
    std::string separator("-");
    hash.insert(8, separator);
    hash.insert(13, separator);
    hash.insert(18, separator);
    hash.insert(23, separator);
    return hash;
}

static void debug_plist(plist_t plistIn) {
    uint32_t size = 0;
    char* data = nullptr;
    int ret = plist_to_xml(plistIn, &data, &size);
    if(ret == PLIST_ERR_SUCCESS) {
        fmt::print("{0}", data);
    } else {
        fmt::print(fmt::fg(fmt::color::crimson), "{0}: error: {1}\n", __PRETTY_FUNCTION__, ret);
    }
}

static bool validateString(const std::string& s)
{
    for(const char c : s) {
        if (!isalpha(c) && !isspace(c))
            return false;
    }

    return true;
}

#endif //TATSU_LIB_HPP
