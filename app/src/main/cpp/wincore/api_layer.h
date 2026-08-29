#ifndef API_LAYER_H
#define API_LAYER_H

#include <cstdint>
#include <string>
#include <unordered_map>
#include <functional>

/**
 * APILayer - Emulates Windows API calls for running Windows binaries
 * 
 * This class intercepts Windows API calls from the running binary and
 * provides Android-based implementations. It manages:
 * - File I/O operations (CreateFileA, ReadFile, WriteFile)
 * - Message boxes (MessageBoxA)
 * - Process control (ExitProcess)
 * 
 * Features:
 * - API function registry with callback handlers
 * - Windows handle management mapped to Android resources
 * - Type conversion between Windows and Android APIs
 */
class APILayer {
public:
    // Windows compatible handle typedef
    typedef uintptr_t HANDLE;
    typedef uint32_t DWORD;
    typedef uint8_t BYTE;

    // Windows API return codes
    static const HANDLE INVALID_HANDLE_VALUE = static_cast<HANDLE>(-1);
    static const DWORD ERROR_FILE_NOT_FOUND = 2;
    static const DWORD ERROR_ACCESS_DENIED = 5;
    static const DWORD ERROR_INVALID_HANDLE = 6;

    // File operation flags (Windows constants)
    static const DWORD GENERIC_READ = 0x80000000;
    static const DWORD GENERIC_WRITE = 0x40000000;
    static const DWORD CREATE_NEW = 1;
    static const DWORD CREATE_ALWAYS = 2;
    static const DWORD OPEN_EXISTING = 3;

    /**
     * Constructor - Initializes API layer with empty handle table
     */
    APILayer();

    /**
     * Destructor - Cleans up resources and closes open handles
     */
    ~APILayer();

    /**
     * MessageBoxA - Displays a Windows-style message box
     * 
     * @param hwnd - Parent window handle (unused on Android, can be 0)
     * @param text - Message box text content
     * @param caption - Message box window title
     * @param type - Message box type/buttons (MB_OK, MB_YESNO, etc.)
     * @return Result code: 1=OK, 2=Cancel, 6=Yes, 7=No
     * 
     * Android Implementation:
     * Logs to Android logcat with the message content.
     * Returns IDOK (1) by default.
     */
    int32_t MessageBoxA(HANDLE hwnd, const char* text, 
                       const char* caption, uint32_t type);

    /**
     * CreateFileA - Creates or opens a file
     * 
     * @param filename - Path to file
     * @param desiredAccess - File access mode (GENERIC_READ, GENERIC_WRITE)
     * @param shareMode - Share mode (0 = exclusive)
     * @param securityAttributes - Security attributes (unused, nullptr)
     * @param creationDisposition - How to create/open (CREATE_NEW, CREATE_ALWAYS, OPEN_EXISTING)
     * @param flagsAndAttributes - File attributes (unused)
     * @param templateFile - Template file handle (unused)
     * @return File handle on success, INVALID_HANDLE_VALUE on failure
     * 
     * Android Implementation:
     * Maps Windows file paths to Android accessible paths.
     * Opens files using standard POSIX open() call.
     * Returns handle mapped to file descriptor.
     */
    HANDLE CreateFileA(const char* filename, DWORD desiredAccess,
                      DWORD shareMode, void* securityAttributes,
                      DWORD creationDisposition, DWORD flagsAndAttributes,
                      HANDLE templateFile);

    /**
     * ReadFile - Reads data from a file handle
     * 
     * @param handle - File handle from CreateFileA
     * @param buffer - [OUTPUT] Buffer to read data into
     * @param bytesToRead - Number of bytes to read
     * @param bytesRead - [OUTPUT] Actual bytes read
     * @param overlapped - Overlapped I/O structure (unused, nullptr)
     * @return true on success, false on failure
     * 
     * Android Implementation:
     * Uses POSIX read() call on underlying file descriptor.
     * Handles partial reads and error conditions.
     */
    bool ReadFile(HANDLE handle, void* buffer, DWORD bytesToRead,
                 DWORD* bytesRead, void* overlapped);

    /**
     * WriteFile - Writes data to a file handle
     * 
     * @param handle - File handle from CreateFileA
     * @param buffer - Data buffer to write
     * @param bytesToWrite - Number of bytes to write
     * @param bytesWritten - [OUTPUT] Actual bytes written
     * @param overlapped - Overlapped I/O structure (unused, nullptr)
     * @return true on success, false on failure
     * 
     * Android Implementation:
     * Uses POSIX write() call on underlying file descriptor.
     * Handles partial writes and error conditions.
     */
    bool WriteFile(HANDLE handle, const void* buffer, DWORD bytesToWrite,
                  DWORD* bytesWritten, void* overlapped);

    /**
     * ExitProcess - Terminates process execution
     * 
     * @param exitCode - Exit code to return
     * 
     * Android Implementation:
     * Sets execution state and exit code.
     * Causes run() function to exit cleanly.
     * Does not forcefully terminate thread (cooperative).
     */
    void ExitProcess(uint32_t exitCode);

    /**
     * getLastError - Returns last Windows API error code
     * 
     * @return Windows error code (ERROR_* constants)
     */
    DWORD getLastError() const;

    /**
     * closeHandle - Closes a Windows API handle
     * 
     * @param handle - Handle to close
     * @return true if handle was valid and closed, false on error
     */
    bool closeHandle(HANDLE handle);

    /**
     * closeAllHandles - Closes all open handles
     * Used during cleanup
     */
    void closeAllHandles();

    /**
     * getExitCode - Returns exit code from ExitProcess call
     * 
     * @return Exit code, or 0 if not set
     */
    uint32_t getExitCode() const;

private:
    DWORD lastError_;                                   // Last Windows error code
    uint32_t exitCode_;                                 // Process exit code
    std::unordered_map<HANDLE, int> handleToFDMap_;    // Map handles to file descriptors
    HANDLE nextHandle_;                                 // Counter for next handle to assign

    /**
     * mapWindowsPathToAndroid - Converts Windows path to Android accessible path
     * 
     * @param windowsPath - Windows-style path (e.g., "C:\\path\\file.txt")
     * @return Android-accessible path (e.g., "/sdcard/path/file.txt")
     * 
     * Simple implementation maps:
     * - C:\ -> /sdcard/
     * - Backslashes -> forward slashes
     */
    std::string mapWindowsPathToAndroid(const char* windowsPath);

    /**
     * setError - Sets last Windows error code
     * 
     * @param errorCode - Windows error code constant
     */
    void setError(DWORD errorCode);
};

#endif // API_LAYER_H
