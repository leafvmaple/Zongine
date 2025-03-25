#include "StringUtils.h"

#include <windows.h>

namespace Zongine {
    size_t _GetExtensionPos(const std::string& path) {
        size_t dotPos = path.find_last_of('.');
        size_t slashPos = path.find_last_of("/\\");
        if (dotPos != std::string::npos && (slashPos == std::string::npos || dotPos > slashPos)) {
            return dotPos;
        }
        return std::string::npos;
    }

    std::string replaceFileExtension(const std::string& path, const std::string& newExtension) {
        size_t dotPos = _GetExtensionPos(path);
        if (dotPos != std::string::npos) {
            return path.substr(0, dotPos) + newExtension;
        }

        return path + newExtension;
    }

    std::string getFileExtension(const std::string& path) {
        size_t dotPos = _GetExtensionPos(path);

        if (dotPos != std::string::npos) {
            return path.substr(dotPos);
        }

        return "";
    }

    void ToLower(std::string& str) {
        for (auto& c : str) {
            c = tolower(c);
        }
    }

    bool TryReplaceExtension(std::filesystem::path& path, const std::filesystem::path& ext)
    {
        std::filesystem::path tmp = path;

        tmp.replace_extension(ext);
        if (std::filesystem::exists(tmp)) {
            path.replace_extension(ext);
            return true;
        }

        return false;
    }
}