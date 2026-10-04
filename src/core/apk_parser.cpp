#include "apkrunner/apk_parser.hpp"

#include <algorithm>
#include <array>
#include <memory>
#include <stdexcept>
#include <string>
#include <zip.h>

namespace apkrunner {
namespace {

struct ZipCloser {
    void operator()(zip_t* archive) const {
        if (archive != nullptr) {
            zip_close(archive);
        }
    }
};

bool isDexFile(const std::string& name) {
    if (name == "classes.dex") {
        return true;
    }
    if (name.size() <= 11 || name.compare(0, 7, "classes") != 0 ||
        name.compare(name.size() - 4, 4, ".dex") != 0) {
        return false;
    }
    return std::all_of(name.begin() + 7, name.end() - 4, [](unsigned char value) {
        return value >= '0' && value <= '9';
    });
}

void addAbiIfPresent(const std::string& name, ApkContents& result) {
    static const std::array<const char*, 4> supportedAbis = {
        "arm64-v8a", "armeabi-v7a", "x86_64", "x86"
    };

    constexpr const char* libraryPrefix = "lib/";
    if (name.compare(0, 4, libraryPrefix) != 0) {
        return;
    }

    const auto abiEnd = name.find('/', 4);
    if (abiEnd == std::string::npos || abiEnd + 1 >= name.size()) {
        return;
    }

    const std::string abi = name.substr(4, abiEnd - 4);
    if (name.find('/', abiEnd + 1) == std::string::npos &&
        name.size() >= 3 && name.compare(name.size() - 3, 3, ".so") == 0 &&
        std::any_of(supportedAbis.begin(), supportedAbis.end(), [&abi](const char* candidate) {
            return abi == candidate;
        }) &&
        std::find(result.nativeAbis.begin(), result.nativeAbis.end(), abi) == result.nativeAbis.end()) {
        result.nativeAbis.push_back(abi);
    }
}

}  // namespace

ApkContents ApkParser::parse(const std::string& apkPath) {
    int errorCode = 0;
    std::unique_ptr<zip_t, ZipCloser> archive(zip_open(apkPath.c_str(), ZIP_RDONLY, &errorCode));
    if (!archive) {
        zip_error_t error;
        zip_error_init_with_code(&error, errorCode);
        const std::string message = zip_error_strerror(&error);
        zip_error_fini(&error);
        throw std::runtime_error("cannot open APK '" + apkPath + "': " + message);
    }

    ApkContents result;
    const zip_int64_t entryCount = zip_get_num_entries(archive.get(), 0);
    if (entryCount < 0) {
        throw std::runtime_error("cannot read APK entry table: " + std::string(zip_strerror(archive.get())));
    }

    result.entries.reserve(static_cast<std::size_t>(entryCount));
    for (zip_uint64_t index = 0; index < static_cast<zip_uint64_t>(entryCount); ++index) {
        const char* entryName = zip_get_name(archive.get(), index, ZIP_FL_ENC_GUESS);
        if (entryName == nullptr) {
            throw std::runtime_error("cannot read APK entry name: " + std::string(zip_strerror(archive.get())));
        }

        const std::string name(entryName);
        result.entries.push_back(name);
        result.hasManifest = result.hasManifest || name == "AndroidManifest.xml";
        result.hasDex = result.hasDex || isDexFile(name);
        addAbiIfPresent(name, result);
    }

    return result;
}

}  // namespace apkrunner