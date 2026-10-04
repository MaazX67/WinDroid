#include "apkrunner/detector.hpp"

#include <algorithm>
#include <string>

namespace apkrunner {
namespace {

bool hasLibrary(const std::vector<std::string>& entries, const std::string& libraryName) {
    return std::any_of(entries.begin(), entries.end(), [&libraryName](const std::string& entry) {
        const auto separator = entry.find_last_of('/');
        return entry.substr(separator == std::string::npos ? 0 : separator + 1) == libraryName;
    });
}

}  // namespace

Engine detectEngine(const std::vector<std::string>& entries, bool hasDex) {
    if (hasLibrary(entries, "libunity.so")) {
        return Engine::Unity;
    }
    if (hasLibrary(entries, "libUE4.so") || hasLibrary(entries, "libUnreal.so")) {
        return Engine::Unreal;
    }
    if (hasLibrary(entries, "libgodot_android.so") || hasLibrary(entries, "libgodot.so")) {
        return Engine::Godot;
    }
    if (hasLibrary(entries, "libminecraftpe.so")) {
        return Engine::MinecraftBedrock;
    }
    if (hasLibrary(entries, "libcocos.so") || hasLibrary(entries, "libcocos2dcpp.so")) {
        return Engine::Cocos;
    }
    return hasDex ? Engine::GenericJava : Engine::Unknown;
}

const char* engineName(Engine engine) {
    switch (engine) {
        case Engine::Unity: return "Unity";
        case Engine::Unreal: return "Unreal Engine";
        case Engine::Godot: return "Godot";
        case Engine::MinecraftBedrock: return "Minecraft Bedrock";
        case Engine::Cocos: return "Cocos";
        case Engine::GenericJava: return "Generic Android (DEX)";
        case Engine::Unknown: return "Unknown";
    }
    return "Unknown";
}

}  // namespace apkrunner