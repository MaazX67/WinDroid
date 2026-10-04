#include "apkrunner/elf_loader.hpp"

namespace apkrunner {

NativeExecutionSupport checkNativeExecutionSupport(const std::string& abi) {
    if (abi == "arm64-v8a" || abi == "armeabi-v7a") {
        return {false,
                "Android ARM ELF loading is not implemented. Box64 translates Linux x86-64 "
                "programs on Linux; it is not a Windows Android-ELF loader."};
    }
    if (abi == "x86" || abi == "x86_64") {
        return {false,
                "Android ELF loading is not implemented. Matching CPU architecture alone "
                "does not provide Android's Bionic ABI or platform services."};
    }
    return {false, "Unrecognized Android ABI; native library execution is not implemented."};
}

}  // namespace apkrunner