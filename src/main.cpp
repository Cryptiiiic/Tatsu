#include <Manifest.hpp>
#include <Timer.hpp>
#include <Tatsu.hpp>
#include <filesystem>
#include <fmt/color.h>

int main() {
    std::vector<std::shared_ptr<Manifest>> manifestList{};
    std::vector<std::shared_ptr<Tatsu>> tatsuList{};
    auto tmp = std::make_shared<Manifest>(fmt::format("{0}/{1}", std::string(std::filesystem::current_path()), std::string("BuildManifest.plist")));
    if(!tmp->isValid()) {
        return -1;
    }
    manifestList.emplace_back(tmp);
    auto tmp2 = std::make_shared<Tatsu>(tmp, 0x8120, "d74ap", 0x69, ERASE, 0x1111111111111111, "", "", std::vector<std::string>{ "NeRDEpoch", "iBEC" });
    tatsuList.emplace_back(tmp2);
    return 0;
}
