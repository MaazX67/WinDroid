#include "api_layer.h"
#include <android/log.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <cstring>
#include <algorithm>

#define LOG_TAG "WinCore-API"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

// Windows message box return codes
#define IDOK 1
#define IDCANCEL 2
#define IDYES 6
#define IDNO 7

/**
 * APILayer::APILayer
 * 
 * Constructor - Initializes API layer with empty handle table
 * Sets lastError to 0 and nextHandle starting at 1 (0 is invalid).
 */
APILayer::APILayer()
    : lastError_(0),
      exitCode_(0),
      nextHandle_(1) {
    LOGI("APILayer initialized");
}

/**
 * APILayer::~APILayer
 * 
 * Destructor - Closes all open handles and cleans up resources
 */
APILayer::~APILayer() {
    closeAllHandles();
    LOGI("APILayer destroyed");
}

/**
 * APILayer::MessageBoxA
 * 
 * Emulates Windows MessageBoxA by logging to Android logcat.
 * Returns IDOK immediately (non-blocking behavior for headless environment).
 */
int32_t APILayer::MessageBoxA(HANDLE hwnd, const char* text,
                             const char* caption, uint32_t type) {
    if (!text || !caption) {
        setError(ERROR_INVALID_HANDLE);
        return IDCANCEL;
    }

    // Log message box to Android logcat (simulates display)
    LOGI("MessageBox - Caption: %s", caption);
    LOGI("MessageBox - Text: %s", text);
    
    // Return IDOK (user clicked OK) by default
    return IDOK;
}

/**
 * APILayer::CreateFileA
 * 
 * Creates or opens a file. Maps Windows path to Android filesystem,
 * then opens using POSIX open() call.
 */
APILayer::HANDLE APILayer::CreateFileA(const char* filename, DWORD desiredAccess,
                                      DWORD shareMode, void* securityAttributes,
                                      DWORD creationDisposition, DWORD flagsAndAttributes,
                                      HANDLE templateFile) {
    if (!filename) {
        setError(ERROR_INVALID_HANDLE);
        return INVALID_HANDLE_VALUE;
    }

    // Map Windows path to Android path
    std::string androidPath = mapWindowsPathToAndroid(filename);
    LOGI("CreateFileA: Mapped '%s' -> '%s'", filename, androidPath.c_str());

    // Determine POSIX open flags
    int openFlags = 0;
    
    // Access mode
    if (desiredAccess & GENERIC_WRITE) {
        openFlags |= O_WRONLY;
    } else if (desiredAccess & GENERIC_READ) {
        openFlags |= O_RDONLY;
    } else {
        openFlags |= O_RDONLY;  // Default to read
    }

    // Creation disposition
    switch (creationDisposition) {
        case CREATE_NEW:
            openFlags |= (O_CREAT | O_EXCL);
            break;
        case CREATE_ALWAYS:
            openFlags |= (O_CREAT | O_TRUNC);
            break;
        case OPEN_EXISTING:
            // No flags, just open existing
            break;
        default:
            openFlags |= O_CREAT;
            break;
    }

    // Open file with permissions rw-rw-rw-
    int fd = open(androidPath.c_str(), openFlags, 0666);
    
    if (fd < 0) {
        LOGE("Failed to open file '%s': errno=%d", androidPath.c_str(), errno);
        setError(ERROR_FILE_NOT_FOUND);
        return INVALID_HANDLE_VALUE;
    }

    // Create Windows-style handle and map to file descriptor
    HANDLE handle = static_cast<HANDLE>(nextHandle_++);
    handleToFDMap_[handle] = fd;
    
    LOGI("CreateFileA: Opened file with handle %zu, fd=%d", handle, fd);
    return handle;
}

/**
 * APILayer::ReadFile
 * 
 * Reads data from file. Translates Windows handle to file descriptor,
 * then calls POSIX read().
 */
bool APILayer::ReadFile(HANDLE handle, void* buffer, DWORD bytesToRead,
                       DWORD* bytesRead, void* overlapped) {
    if (!buffer || !bytesRead) {
        setError(ERROR_INVALID_HANDLE);
        return false;
    }

    // Find file descriptor for handle
    auto it = handleToFDMap_.find(handle);
    if (it == handleToFDMap_.end()) {
        LOGE("ReadFile: Invalid handle %zu", handle);
        setError(ERROR_INVALID_HANDLE);
        return false;
    }

    int fd = it->second;
    
    // Read from file
    ssize_t result = read(fd, buffer, bytesToRead);
    
    if (result < 0) {
        LOGE("ReadFile: read() failed, errno=%d", errno);
        setError(ERROR_ACCESS_DENIED);
        *bytesRead = 0;
        return false;
    }

    *bytesRead = static_cast<DWORD>(result);
    LOGI("ReadFile: Read %u bytes from fd=%d", *bytesRead, fd);
    return true;
}

/**
 * APILayer::WriteFile
 * 
 * Writes data to file. Translates Windows handle to file descriptor,
 * then calls POSIX write().
 */
bool APILayer::WriteFile(HANDLE handle, const void* buffer, DWORD bytesToWrite,
                        DWORD* bytesWritten, void* overlapped) {
    if (!buffer || !bytesWritten) {
        setError(ERROR_INVALID_HANDLE);
        return false;
    }

    // Find file descriptor for handle
    auto it = handleToFDMap_.find(handle);
    if (it == handleToFDMap_.end()) {
        LOGE("WriteFile: Invalid handle %zu", handle);
        setError(ERROR_INVALID_HANDLE);
        return false;
    }

    int fd = it->second;
    
    // Write to file
    ssize_t result = write(fd, buffer, bytesToWrite);
    
    if (result < 0) {
        LOGE("WriteFile: write() failed, errno=%d", errno);
        setError(ERROR_ACCESS_DENIED);
        *bytesWritten = 0;
        return false;
    }

    *bytesWritten = static_cast<DWORD>(result);
    LOGI("WriteFile: Wrote %u bytes to fd=%d", *bytesWritten, fd);
    return true;
}

/**
 * APILayer::ExitProcess
 * 
 * Sets exit code to signal process termination.
 * Causes main execution loop to exit cleanly.
 */
void APILayer::ExitProcess(uint32_t exitCode) {
    exitCode_ = exitCode;
    LOGI("ExitProcess called with exit code %u", exitCode);
}

/**
 * APILayer::getLastError
 * 
 * Returns last Windows error code set by API calls.
 */
APILayer::DWORD APILayer::getLastError() const {
    return lastError_;
}

/**
 * APILayer::closeHandle
 * 
 * Closes a file handle and removes from handle mapping.
 * Calls POSIX close() on underlying file descriptor.
 */
bool APILayer::closeHandle(HANDLE handle) {
    auto it = handleToFDMap_.find(handle);
    if (it == handleToFDMap_.end()) {
        LOGE("closeHandle: Invalid handle %zu", handle);
        setError(ERROR_INVALID_HANDLE);
        return false;
    }

    int fd = it->second;
    int result = close(fd);
    
    handleToFDMap_.erase(it);
    
    if (result != 0) {
        LOGE("closeHandle: close() failed for fd=%d, errno=%d", fd, errno);
        setError(ERROR_ACCESS_DENIED);
        return false;
    }

    LOGI("closeHandle: Closed handle %zu (fd=%d)", handle, fd);
    return true;
}

/**
 * APILayer::closeAllHandles
 * 
 * Closes all open file handles during cleanup.
 */
void APILayer::closeAllHandles() {
    LOGI("Closing %zu handles", handleToFDMap_.size());
    
    // Create copy of handles to avoid iterator invalidation
    auto handles = handleToFDMap_;
    
    for (auto& pair : handles) {
        closeHandle(pair.first);
    }
}

/**
 * APILayer::getExitCode
 * 
 * Returns exit code from last ExitProcess call.
 */
uint32_t APILayer::getExitCode() const {
    return exitCode_;
}

/**
 * APILayer::mapWindowsPathToAndroid
 * 
 * Converts Windows-style paths to Android-accessible paths.
 * 
 * Examples:
 * - "C:\\file.txt" -> "/sdcard/file.txt"
 * - "D:\\folder\\file.txt" -> "/data/file.txt"
 * - "notepad.exe" -> "/sdcard/notepad.exe"
 */
std::string APILayer::mapWindowsPathToAndroid(const char* windowsPath) {
    if (!windowsPath) {
        return "/sdcard/";
    }

    std::string path(windowsPath);
    
    // Convert backslashes to forward slashes
    std::replace(path.begin(), path.end(), '\\', '/');

    // Handle drive letters (C:, D:, etc.)
    if (path.length() >= 2 && path[1] == ':') {
        // Remove drive letter
        path = path.substr(2);
    }

    // Remove leading slashes if any and add /sdcard
    while (!path.empty() && path[0] == '/') {
        path = path.substr(1);
    }

    // Return path on Android storage
    return "/sdcard/" + path;
}

/**
 * APILayer::setError
 * 
 * Sets last Windows error code.
 */
void APILayer::setError(DWORD errorCode) {
    lastError_ = errorCode;
}
