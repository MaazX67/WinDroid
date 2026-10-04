#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <cerrno>
#include <spawn.h>
#include <sys/wait.h>
#include <unistd.h>
extern char** environ;
#endif

namespace {

constexpr int usageError = 2;
constexpr int launchError = 10;

void printUsage(std::ostream& output) {
    output << "APKRunner Fusion 2.0 - Android guest ADB bridge\n"
           << "Usage:\n"
           << "  APKRunner-Fusion install <APK>\n"
           << "  APKRunner-Fusion run <package.name>\n"
           << "  APKRunner-Fusion devices\n";
}

bool isPackageName(const std::string& value) {
    if (value.empty() || value.front() == '.' || value.back() == '.') {
        return false;
    }

    bool hasSeparator = false;
    bool previousWasSeparator = true;
    for (const unsigned char character : value) {
        if (character == '.') {
            if (previousWasSeparator) {
                return false;
            }
            hasSeparator = true;
            previousWasSeparator = true;
        } else if ((character >= 'a' && character <= 'z') ||
                   (character >= 'A' && character <= 'Z') ||
                   (character >= '0' && character <= '9') || character == '_') {
            previousWasSeparator = false;
        } else {
            return false;
        }
    }
    return hasSeparator && !previousWasSeparator;
}

#ifdef _WIN32

std::wstring toWide(const std::string& value) {
    const int length = MultiByteToWideChar(CP_ACP, 0, value.c_str(), -1, nullptr, 0);
    if (length <= 0) {
        return {};
    }
    std::wstring result(static_cast<std::size_t>(length), L'\0');
    MultiByteToWideChar(CP_ACP, 0, value.c_str(), -1, result.data(), length);
    result.pop_back();
    return result;
}

std::wstring quoteArgument(const std::wstring& argument) {
    std::wstring quoted = L"\"";
    std::size_t backslashes = 0;
    for (const wchar_t character : argument) {
        if (character == L'\\') {
            ++backslashes;
        } else if (character == L'"') {
            quoted.append(backslashes * 2 + 1, L'\\');
            quoted.push_back(character);
            backslashes = 0;
        } else {
            quoted.append(backslashes, L'\\');
            quoted.push_back(character);
            backslashes = 0;
        }
    }
    quoted.append(backslashes * 2, L'\\');
    quoted.push_back(L'"');
    return quoted;
}

int runAdb(const std::vector<std::string>& arguments) {
    std::wstring commandLine = quoteArgument(L"adb.exe");
    for (const auto& argument : arguments) {
        const std::wstring wideArgument = toWide(argument);
        if (wideArgument.empty() && !argument.empty()) {
            std::cerr << "Could not convert an argument to the Windows character set.\n";
            return launchError;
        }
        commandLine.push_back(L' ');
        commandLine += quoteArgument(wideArgument);
    }

    std::vector<wchar_t> mutableCommandLine(commandLine.begin(), commandLine.end());
    mutableCommandLine.push_back(L'\0');

    STARTUPINFOW startup{};
    startup.cb = sizeof(startup);
    startup.dwFlags = STARTF_USESTDHANDLES;
    startup.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
    startup.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    startup.hStdError = GetStdHandle(STD_ERROR_HANDLE);
    PROCESS_INFORMATION process{};

    if (!CreateProcessW(L"adb.exe", mutableCommandLine.data(), nullptr, nullptr, TRUE,
                        0, nullptr, nullptr, &startup, &process)) {
        std::cerr << "Could not start adb.exe (Windows error " << GetLastError()
                  << "). Install Android platform-tools and ensure adb.exe is on PATH.\n";
        return launchError;
    }

    WaitForSingleObject(process.hProcess, INFINITE);
    DWORD exitCode = 1;
    GetExitCodeProcess(process.hProcess, &exitCode);
    CloseHandle(process.hThread);
    CloseHandle(process.hProcess);
    return static_cast<int>(exitCode);
}

#else

int runAdb(const std::vector<std::string>& arguments) {
    std::vector<char*> argv;
    argv.reserve(arguments.size() + 2);
    argv.push_back(const_cast<char*>("adb"));
    for (const auto& argument : arguments) {
        argv.push_back(const_cast<char*>(argument.c_str()));
    }
    argv.push_back(nullptr);

    pid_t process = 0;
    const int spawnError = posix_spawnp(&process, "adb", nullptr, nullptr, argv.data(), environ);
    if (spawnError != 0) {
        std::cerr << "Could not start adb: " << std::strerror(spawnError)
                  << ". Install Android platform-tools and ensure adb is on PATH.\n";
        return launchError;
    }

    int status = 0;
    if (waitpid(process, &status, 0) < 0) {
        std::cerr << "Could not wait for adb: " << std::strerror(errno) << '\n';
        return launchError;
    }
    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }
    return launchError;
}

#endif

}  // namespace

int main(int argc, char** argv) {
    if (argc == 2 && (std::string(argv[1]) == "--help" || std::string(argv[1]) == "-h")) {
        printUsage(std::cout);
        return 0;
    }
    if (argc == 2 && std::string(argv[1]) == "devices") {
        return runAdb({"devices", "-l"});
    }
    if (argc == 3 && std::string(argv[1]) == "install") {
        std::error_code error;
        if (!std::filesystem::is_regular_file(argv[2], error)) {
            std::cerr << "APK file does not exist or is not a regular file: " << argv[2] << '\n';
            return usageError;
        }
        return runAdb({"install", "-r", argv[2]});
    }
    if (argc == 3 && std::string(argv[1]) == "run") {
        const std::string packageName(argv[2]);
        if (!isPackageName(packageName)) {
            std::cerr << "Invalid Android package name: " << packageName << '\n';
            return usageError;
        }
        return runAdb({"shell", "monkey", "-p", packageName,
                       "-c", "android.intent.category.LAUNCHER", "1"});
    }

    printUsage(std::cerr);
    return usageError;
}