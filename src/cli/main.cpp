#include "apkrunner/apk_parser.hpp"
#include "apkrunner/detector.hpp"
#include "apkrunner/elf_loader.hpp"

#include <exception>
#include <iostream>
#include <string>

namespace {

constexpr int usageError = 2;
constexpr int runtimeUnavailable = 3;

void printUsage(std::ostream& output) {
    output << "APKRunner-Universal 0.1.0\n"
           << "Usage:\n"
           << "  APKRunner detect <APK>\n"
           << "  APKRunner run <APK> [--width N] [--height N] [--fps N]\n"
           << "  APKRunner build <APK> -o <EXE>\n";
}

bool isPositiveInteger(const std::string& value) {
    if (value.empty()) {
        return false;
    }
    for (const char character : value) {
        if (character < '0' || character > '9') {
            return false;
        }
    }
    return value != "0";
}

bool validRunOptions(int argc, char** argv) {
    for (int index = 3; index < argc; index += 2) {
        const std::string option(argv[index]);
        if ((option != "--width" && option != "--height" && option != "--fps") ||
            index + 1 >= argc || !isPositiveInteger(argv[index + 1])) {
            return false;
        }
    }
    return true;
}

int runDetect(const std::string& apkPath) {
    const apkrunner::ApkContents contents = apkrunner::ApkParser::parse(apkPath);
    const auto engine = apkrunner::detectEngine(contents.entries, contents.hasDex);

    std::cout << "Engine: " << apkrunner::engineName(engine) << '\n'
              << "Manifest: " << (contents.hasManifest ? "present" : "missing") << '\n'
              << "DEX: " << (contents.hasDex ? "present" : "missing") << '\n'
              << "Native ABIs:";
    if (contents.nativeAbis.empty()) {
        std::cout << " none";
    } else {
        for (const auto& abi : contents.nativeAbis) {
            std::cout << ' ' << abi;
        }
    }
    std::cout << '\n';
    return 0;
}

int reportUnavailable(const std::string& command) {
    std::cerr << "'" << command << "' is not available: this build only inspects APK metadata. "
              << "Executing APKs requires Android ART/framework services, compatible native "
              << "runtime bridges, and graphics/audio/input backends, none of which are included.\n";
    return runtimeUnavailable;
}

}  // namespace

int main(int argc, char** argv) {
    if (argc < 2) {
        printUsage(std::cerr);
        return usageError;
    }

    const std::string command(argv[1]);
    if (command == "--help" || command == "-h" || command == "help") {
        printUsage(std::cout);
        return 0;
    }

    if (command == "detect") {
        if (argc != 3) {
            printUsage(std::cerr);
            return usageError;
        }
        try {
            return runDetect(argv[2]);
        } catch (const std::exception& error) {
            std::cerr << "APKRunner: " << error.what() << '\n';
            return usageError;
        }
    }

    if (command == "run") {
        if (argc < 3 || !validRunOptions(argc, argv)) {
            printUsage(std::cerr);
            return usageError;
        }
        return reportUnavailable(command);
    }

    if (command == "build") {
        if (argc != 5 || std::string(argv[3]) != "-o" || std::string(argv[4]).empty()) {
            printUsage(std::cerr);
            return usageError;
        }
        return reportUnavailable(command);
    }

    std::cerr << "Unknown command: " << command << '\n';
    printUsage(std::cerr);
    return usageError;
}