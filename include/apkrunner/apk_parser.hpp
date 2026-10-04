#pragma once

#include <string>
#include <vector>

namespace apkrunner {

struct ApkContents {
    std::vector<std::string> entries;
    std::vector<std::string> nativeAbis;
    bool hasManifest = false;
    bool hasDex = false;
};

class ApkParser {
public:
    static ApkContents parse(const std::string& apkPath);
};

}  // namespace apkrunner