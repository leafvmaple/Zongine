#pragma once

#include <string>
#include <cstdlib>

#include <filesystem>

namespace Zongine {
    size_t _GetExtensionPos(const std::string& path);
    std::string replaceFileExtension(const std::string& path, const std::string& newExtension);
    std::string getFileExtension(const std::string& path);

    void ToLower(std::string& str);
    bool TryReplaceExtension(std::filesystem::path& path, const std::filesystem::path& ext);
}