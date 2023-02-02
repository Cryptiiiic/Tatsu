#include <Manifest.hpp>
#include <Timer.hpp>
#include <Tatsu.hpp>
#include <filesystem>
#include <fmt/color.h>

int main() {
    std::vector<std::unique_ptr<Manifest>> manifestList{};
    std::vector<std::unique_ptr<Tatsu>> tatsuList{};
    manifestList.emplace_back(std::move(std::make_unique<Manifest>(fmt::format("{0}/{1}", std::string(std::filesystem::current_path()), std::string("BuildManifest.plist")))));
    tatsuList.emplace_back(std::move(std::make_unique<Tatsu>(manifestList[0].get(), 0x8000, "n71ap", 0x69, ERASE, 0x1111111111111111)));
    return 0;
}
