#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shlobj.h>
#include <shobjidl.h>
#include <winreg.h>
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
    output << "APKRunner Fusion 2.1 - Android guest ADB bridge\n"
           << "Usage:\n"
           << "  APKRunner-Fusion install <APK>\n"
           << "  APKRunner-Fusion open <APK>\n"
           << "  APKRunner-Fusion run <package.name>\n"
           << "  APKRunner-Fusion devices\n"
           << "  APKRunner-Fusion --register\n";
}

void pauseForConsoleUser() {
#ifdef _WIN32
    std::cout << "Press Enter to exit..." << std::flush;
    std::string line;
    std::getline(std::cin, line);
#endif
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

std::filesystem::path executablePath() {
    std::vector<wchar_t> buffer(32768);
    const DWORD length = GetModuleFileNameW(nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));
    if (length == 0 || length >= buffer.size()) {
        return {};
    }
    return std::filesystem::path(std::wstring(buffer.data(), length));
}

std::filesystem::path resolveAdb() {
    const auto sibling = executablePath().parent_path() / L"adb.exe";
    std::error_code error;
    if (std::filesystem::is_regular_file(sibling, error)) {
        return sibling;
    }

    std::vector<wchar_t> tempPath(32768);
    const DWORD tempLength = GetTempPathW(static_cast<DWORD>(tempPath.size()), tempPath.data());
    if (tempLength > 0 && tempLength < tempPath.size()) {
        const auto cached = std::filesystem::path(tempPath.data()) / L"APKRunnerFusion" / L"adb.exe";
        if (std::filesystem::is_regular_file(cached, error)) {
            return cached;
        }
    }

    std::vector<wchar_t> searchResult(32768);
    const DWORD searchLength = SearchPathW(nullptr, L"adb.exe", nullptr,
                                           static_cast<DWORD>(searchResult.size()),
                                           searchResult.data(), nullptr);
    if (searchLength > 0 && searchLength < searchResult.size()) {
        return std::filesystem::path(std::wstring(searchResult.data(), searchLength));
    }
    return {};
}

int runAdb(const std::vector<std::string>& arguments, std::string* capturedOutput = nullptr) {
    const std::filesystem::path adbPath = resolveAdb();
    if (adbPath.empty()) {
        std::cerr << "No ADB device, please install a supported Android guest or connect a phone. "
                  << "Bundle adb.exe beside this launcher or install Android platform-tools on PATH.\n";
        return launchError;
    }

    std::wstring commandLine = quoteArgument(adbPath.wstring());
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
    PROCESS_INFORMATION process{};
    SECURITY_ATTRIBUTES pipeSecurity{};
    pipeSecurity.nLength = sizeof(pipeSecurity);
    pipeSecurity.bInheritHandle = TRUE;
    HANDLE readPipe = nullptr;
    HANDLE writePipe = nullptr;
    if (capturedOutput != nullptr) {
        if (!CreatePipe(&readPipe, &writePipe, &pipeSecurity, 0) ||
            !SetHandleInformation(readPipe, HANDLE_FLAG_INHERIT, 0)) {
            if (readPipe != nullptr) CloseHandle(readPipe);
            if (writePipe != nullptr) CloseHandle(writePipe);
            std::cerr << "Could not create a pipe to read ADB output.\n";
            return launchError;
        }
        startup.dwFlags = STARTF_USESTDHANDLES;
        startup.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
        startup.hStdOutput = writePipe;
        startup.hStdError = writePipe;
    }

    if (!CreateProcessW(adbPath.c_str(), mutableCommandLine.data(), nullptr, nullptr,
                        capturedOutput != nullptr,
                        0, nullptr, nullptr, &startup, &process)) {
        if (readPipe != nullptr) CloseHandle(readPipe);
        if (writePipe != nullptr) CloseHandle(writePipe);
        std::cerr << "Could not start adb.exe (Windows error " << GetLastError() << ").\n";
        return launchError;
    }

    if (writePipe != nullptr) CloseHandle(writePipe);
    if (capturedOutput != nullptr) {
        capturedOutput->clear();
        char buffer[4096];
        DWORD bytesRead = 0;
        while (ReadFile(readPipe, buffer, sizeof(buffer), &bytesRead, nullptr) && bytesRead > 0) {
            capturedOutput->append(buffer, bytesRead);
        }
        CloseHandle(readPipe);
        std::cout << *capturedOutput;
    }

    WaitForSingleObject(process.hProcess, INFINITE);
    DWORD exitCode = 1;
    GetExitCodeProcess(process.hProcess, &exitCode);
    CloseHandle(process.hThread);
    CloseHandle(process.hProcess);
    return static_cast<int>(exitCode);
}

bool writeRegistryString(const wchar_t* subKey, const wchar_t* valueName,
                         const std::wstring& value) {
    HKEY key = nullptr;
    const LONG openResult = RegCreateKeyExW(HKEY_CURRENT_USER, subKey, 0, nullptr, 0,
                                             KEY_SET_VALUE, nullptr, &key, nullptr);
    if (openResult != ERROR_SUCCESS) {
        return false;
    }
    const DWORD bytes = static_cast<DWORD>((value.size() + 1) * sizeof(wchar_t));
    const LONG writeResult = RegSetValueExW(key, valueName, 0, REG_SZ,
                                             reinterpret_cast<const BYTE*>(value.c_str()), bytes);
    RegCloseKey(key);
    return writeResult == ERROR_SUCCESS;
}

int registerApkOpenWith() {
    const auto exe = executablePath();
    if (exe.empty()) {
        std::cerr << "Could not determine the Fusion executable path.\n";
        return launchError;
    }
    const std::wstring openCommand = quoteArgument(exe.wstring()) + L" open \"%1\"";
    if (!writeRegistryString(L"Software\\Classes\\APKRunnerFusion", nullptr,
                             L"APKRunner Fusion APK Installer") ||
        !writeRegistryString(L"Software\\Classes\\APKRunnerFusion\\shell\\open\\command",
                             nullptr, openCommand)) {
        std::cerr << "Could not register the APK Open With command for this user.\n";
        return launchError;
    }

    HKEY key = nullptr;
    const LONG openResult = RegCreateKeyExW(HKEY_CURRENT_USER,
        L"Software\\Classes\\.apk\\OpenWithProgids", 0, nullptr, 0,
        KEY_SET_VALUE, nullptr, &key, nullptr);
    if (openResult != ERROR_SUCCESS) {
        std::cerr << "Could not add Fusion to the APK Open With list.\n";
        return launchError;
    }
    const LONG writeResult = RegSetValueExW(key, L"APKRunnerFusion", 0, REG_NONE, nullptr, 0);
    RegCloseKey(key);
    if (writeResult != ERROR_SUCCESS) {
        std::cerr << "Could not add Fusion to the APK Open With list.\n";
        return launchError;
    }

    SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, nullptr, nullptr);
    std::cout << "APKRunner Fusion was added to this user's .apk Open With list.\n";
    return 0;
}

bool createDesktopShortcut(const std::string& apkPath, const std::string& packageName) {
    const HRESULT initResult = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    if (FAILED(initResult) && initResult != RPC_E_CHANGED_MODE) {
        return false;
    }

    IShellLinkW* link = nullptr;
    HRESULT result = CoCreateInstance(CLSID_ShellLink, nullptr, CLSCTX_INPROC_SERVER,
                                      IID_IShellLinkW, reinterpret_cast<void**>(&link));
    if (FAILED(result)) {
        if (SUCCEEDED(initResult)) CoUninitialize();
        return false;
    }

    const auto exe = executablePath();
    const std::wstring widePackage = toWide(packageName);
    const std::wstring arguments = L"run " + quoteArgument(widePackage);
    result = link->SetPath(exe.c_str());
    if (SUCCEEDED(result)) result = link->SetArguments(arguments.c_str());
    if (SUCCEEDED(result)) result = link->SetWorkingDirectory(exe.parent_path().c_str());
    if (SUCCEEDED(result)) result = link->SetDescription(L"Launch Android app through APKRunner Fusion");

    IPersistFile* persist = nullptr;
    if (SUCCEEDED(result)) result = link->QueryInterface(IID_IPersistFile,
                                                         reinterpret_cast<void**>(&persist));
    PWSTR desktop = nullptr;
    if (SUCCEEDED(result)) result = SHGetKnownFolderPath(FOLDERID_Desktop, 0, nullptr, &desktop);
    if (SUCCEEDED(result)) {
        const auto shortcut = std::filesystem::path(desktop) /
                              (std::filesystem::path(apkPath).stem().wstring() + L".lnk");
        result = persist->Save(shortcut.c_str(), TRUE);
    }

    if (desktop != nullptr) CoTaskMemFree(desktop);
    if (persist != nullptr) persist->Release();
    link->Release();
    if (SUCCEEDED(initResult)) CoUninitialize();
    return SUCCEEDED(result);
}

#else

int runAdb(const std::vector<std::string>& arguments, std::string* capturedOutput = nullptr) {
    if (capturedOutput != nullptr) {
        int outputPipe[2];
        if (pipe(outputPipe) != 0) {
            std::cerr << "Could not create a pipe to read ADB output.\n";
            return launchError;
        }
        posix_spawn_file_actions_t actions;
        posix_spawn_file_actions_init(&actions);
        posix_spawn_file_actions_adddup2(&actions, outputPipe[1], STDOUT_FILENO);
        posix_spawn_file_actions_adddup2(&actions, outputPipe[1], STDERR_FILENO);
        posix_spawn_file_actions_addclose(&actions, outputPipe[0]);
        posix_spawn_file_actions_addclose(&actions, outputPipe[1]);

        std::vector<char*> argv;
        argv.reserve(arguments.size() + 2);
        argv.push_back(const_cast<char*>("adb"));
        for (const auto& argument : arguments) argv.push_back(const_cast<char*>(argument.c_str()));
        argv.push_back(nullptr);

        pid_t process = 0;
        const int spawnError = posix_spawnp(&process, "adb", &actions, nullptr, argv.data(), environ);
        posix_spawn_file_actions_destroy(&actions);
        close(outputPipe[1]);
        if (spawnError != 0) {
            close(outputPipe[0]);
            std::cerr << "No ADB device, please install a supported Android guest or connect a phone. "
                      << "Install Android platform-tools and ensure adb is on PATH.\n";
            return launchError;
        }

        capturedOutput->clear();
        char buffer[4096];
        ssize_t bytesRead = 0;
        while ((bytesRead = read(outputPipe[0], buffer, sizeof(buffer))) > 0) {
            capturedOutput->append(buffer, static_cast<std::size_t>(bytesRead));
        }
        close(outputPipe[0]);
        int status = 0;
        if (waitpid(process, &status, 0) < 0) return launchError;
        std::cout << *capturedOutput;
        if (WIFEXITED(status)) return WEXITSTATUS(status);
        return launchError;
    }

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
        std::cerr << "No ADB device, please install a supported Android guest or connect a phone. "
                  << "Install Android platform-tools and ensure adb is on PATH. "
                  << std::strerror(spawnError) << '\n';
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

bool hasOnlineDevice(const std::string& output) {
    std::istringstream lines(output);
    std::string line;
    while (std::getline(lines, line)) {
        std::istringstream fields(line);
        std::string serial;
        std::string state;
        if ((fields >> serial >> state) && state == "device") {
            return true;
        }
    }
    return false;
}

int listDevices() {
    std::string output;
    const int result = runAdb({"devices", "-l"}, &output);
    if (result != 0) return result;
    if (!hasOnlineDevice(output)) {
        std::cerr << "No ADB device, please install a supported Android guest or connect a phone.\n";
        return launchError;
    }
    return 0;
}

int installApk(const std::string& apkPath) {
    std::error_code error;
    if (!std::filesystem::is_regular_file(apkPath, error)) {
        std::cerr << "APK file does not exist or is not a regular file: " << apkPath << '\n';
        return usageError;
    }

    const int installResult = runAdb({"install", "-r", apkPath});
    if (installResult != 0) return installResult;

#ifdef _WIN32
    std::cout << "Installed. Enter the app package name to create a desktop launch shortcut "
                 "(or press Enter to skip): " << std::flush;
    std::string packageName;
    std::getline(std::cin, packageName);
    if (!packageName.empty()) {
        if (!isPackageName(packageName)) {
            std::cerr << "Invalid package name; no shortcut was created.\n";
            return usageError;
        }
        if (!createDesktopShortcut(apkPath, packageName)) {
            std::cerr << "App installed, but Fusion could not create the desktop shortcut.\n";
            return launchError;
        }
        std::cout << "Desktop shortcut created.\n";
    }
#endif
    return 0;
}

}  // namespace

int main(int argc, char** argv) {
    if (argc == 1) {
        printUsage(std::cout);
        pauseForConsoleUser();
        return 0;
    }
    if ((argc == 2 || argc == 1) &&
        (std::string(argv[1]) == "--help" || std::string(argv[1]) == "-h")) {
        printUsage(std::cout);
#ifdef _WIN32
        pauseForConsoleUser();
#endif
        return 0;
    }
    if (argc == 2 && std::string(argv[1]) == "devices") {
        return listDevices();
    }
    if (argc == 2 && std::string(argv[1]) == "--register") {
#ifdef _WIN32
        return registerApkOpenWith();
#else
        std::cerr << "APK file association registration is only supported on Windows.\n";
        return usageError;
#endif
    }
    if (argc == 3 && (std::string(argv[1]) == "install" || std::string(argv[1]) == "open")) {
        return installApk(argv[2]);
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