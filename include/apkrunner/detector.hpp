#pragma once

#include <string>
#include <vector>

namespace apkrunner {

enum class Engine {
    Unity,
    Unreal,
    Godot,
    MinecraftBedrock,
    Cocos,
    GenericJava,
    Unknown
};

Engine detectEngine(const std::vector<std::string>& entries, bool hasDex);
const char* engineName(Engine engine);

}  // namespace apkrunner