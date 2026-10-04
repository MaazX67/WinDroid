#pragma once

#include <string>

namespace apkrunner {

struct NativeExecutionSupport {
    bool supported = false;
    std::string explanation;
};

NativeExecutionSupport checkNativeExecutionSupport(const std::string& abi);

}  // namespace apkrunner