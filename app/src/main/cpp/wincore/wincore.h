#ifndef WINCORE_H
#define WINCORE_H

#include <string>
#include <map>
#include <vector>
#include <cstdint>
#include <memory>

// Forward declarations
class JITTranslator;
class APILayer;

/**
 * WinCoreEngine - Main Windows executable emulation engine for Android ARM64
 * 
 * This class manages the loading and execution of Windows PE (x86) binaries
 * on Android using ARM64 JIT translation and Windows API emulation.
 * 
 * Supports:
 * - Loading Windows .exe files
 * - JIT compilation of x86 code to ARM64
 * - Windows API interception and emulation
 * - Memory management for Windows binaries
 */
class WinCoreEngine {
public:
    // Enum for execution state
    enum class ExecutionState {
        IDLE,           // Engine not running
        LOADING,        // Binary being loaded
        RUNNING,        // Binary executing
        SUSPENDED,      // Execution suspended
        COMPLETED,      // Execution finished
        ERROR           // Error occurred
    };

    /**
     * Constructor - Initializes WinCoreEngine with default settings
     * Sets up memory management, JIT translator, and API layer
     */
    WinCoreEngine();

    /**
     * Destructor - Cleans up all resources
     * Frees allocated memory and closes loaded binaries
     */
    ~WinCoreEngine();

    /**
     * loadExe - Loads a Windows PE executable from the given path
     * 
     * @param path - Full file path to Windows .exe binary
     * @return true if successfully loaded, false on error
     *         Sets execution state to LOADING on entry, updates on completion
     * 
     * Process:
     * 1. Validates PE header format
     * 2. Allocates memory for code and data sections
     * 3. Loads sections into memory
     * 4. Resolves imports via API layer
     */
    bool loadExe(const std::string& path);

    /**
     * run - Executes the loaded Windows binary
     * 
     * @return true if execution completed successfully, false on error
     *         Sets execution state to RUNNING, then to COMPLETED or ERROR
     * 
     * Process:
     * 1. Validates that a binary is loaded
     * 2. Initializes execution environment
     * 3. Starts JIT translation and execution
     * 4. Handles Windows API calls via API layer
     * 5. Manages exit codes and cleanup
     */
    bool run();

    /**
     * getExecutionState - Returns current engine execution state
     * 
     * @return Current ExecutionState enum value
     */
    ExecutionState getExecutionState() const;

    /**
     * getLastError - Returns description of last error that occurred
     * 
     * @return Error message string (empty if no error)
     */
    std::string getLastError() const;

    /**
     * getExitCode - Returns exit code from last execution
     * 
     * @return Exit code (0 on success, non-zero on error)
     */
    int32_t getExitCode() const;

    /**
     * unload - Unloads current binary and frees all resources
     * 
     * @return true if successful, false on error
     */
    bool unload();

private:
    // Internal state
    ExecutionState state_;
    std::string lastError_;
    int32_t exitCode_;

    // Memory and binary data
    std::vector<uint8_t> binaryData_;      // Raw binary file contents
    uintptr_t allocatedMemoryBase_;        // Base address of allocated memory
    size_t allocatedMemorySize_;           // Size of allocated memory

    // Component pointers
    std::unique_ptr<JITTranslator> jitTranslator_;
    std::unique_ptr<APILayer> apiLayer_;

    /**
     * setError - Internal helper to set error state and message
     * 
     * @param message - Error description
     */
    void setError(const std::string& message);

    /**
     * validatePEHeader - Validates Windows PE binary header format
     * 
     * @return true if valid PE binary, false otherwise
     */
    bool validatePEHeader() const;

    /**
     * loadPESections - Loads all PE sections into allocated memory
     * 
     * @return true if all sections loaded successfully
     */
    bool loadPESections();

    /**
     * initializeMemory - Allocates and initializes memory for binary execution
     * 
     * @return true if memory allocated successfully
     */
    bool initializeMemory();

    /**
     * cleanupMemory - Frees allocated memory
     */
    void cleanupMemory();
};

#endif // WINCORE_H
