#include <plist/plist++.h>
#include <fmt/core.h>
#include <fmt/color.h>
#include <filesystem>
#include <fstream>

#include "Manifest.hpp"
#include "Tatsu.hpp"

int main() {
    plist_t manifest = nullptr;
    std::string manifestPath = fmt::format("{0}/{1}", std::string(std::filesystem::current_path()), std::string("BuildManifest.plist"));
    std::ifstream manifestFileStream(manifestPath, std::ios::in | std::ios::binary | std::ios::ate);
    if(!manifestFileStream.good()) {
        fmt::print(fg(fmt::color::crimson), "Failed to open {0} ({1})!\n", manifestPath, strerror(errno));
        return -1;
    } else {
//        fmt::print(fg(fmt::color::green), "Opened {0}!\n", manifestPath);
    }
    std::streamsize manifestFileSize = manifestFileStream.tellg();
    if(!manifestFileSize) {
        fmt::print(fg(fmt::color::crimson), "Manifest is empty?\n");
        return -1;
    }
    manifestFileStream.seekg(0, std::ios_base::beg);
    std::allocator<uint8_t> alloc;
    char *manifestBuffer = reinterpret_cast<char *>(alloc.allocate(static_cast<size_t>(manifestFileSize)));
    manifestFileStream.read(manifestBuffer, manifestFileSize);
    if(!manifestFileStream.good()) {
        fmt::print(fg(fmt::color::crimson), "Failed to read {0} ({1})!\n", manifestPath, strerror(errno));
        return -1;
    } else {
//        fmt::print(fg(fmt::color::green), "Read {0} to buffer!\n", manifestPath);
    }
    if(plist_is_binary(manifestBuffer, static_cast<uint32_t>(manifestFileSize))) {
        if(plist_from_bin(const_cast<const char*>(manifestBuffer), static_cast<uint32_t>(manifestFileSize), &manifest) != PLIST_ERR_SUCCESS) {
            fmt::print(fg(fmt::color::crimson), "Failed to read manifest {0} as bplist ({1})!\n", manifestPath, strerror(errno));
            return -1;
        }
    } else {
        if(plist_from_xml(const_cast<const char*>(manifestBuffer), static_cast<uint32_t>(manifestFileSize), &manifest) != PLIST_ERR_SUCCESS) {
            fmt::print(fg(fmt::color::crimson), "Failed to read manifest {0} as xml ({1})!\n", manifestPath, strerror(errno));
            return -1;
        }
    }
//    fmt::print(fg(fmt::color::green), "Read {0} to plist buffer!\n", manifestPath);
    Manifest *bm = new Manifest(manifest);
    Tatsu *tss = new Tatsu(bm, 0x8000, "n71ap", 0x69, ERASE, 0x1111111111111111);
//    bm->matchIdentity(0x8000, "n71ap", ERASE);
    delete bm;
    delete tss;
    return 0;
}
