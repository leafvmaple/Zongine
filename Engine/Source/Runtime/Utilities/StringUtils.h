#pragma once

#include <string>
#include <cstdlib>

#include <filesystem>

namespace std {
    template<>
    struct hash<vector<string>> {
        size_t operator()(const vector<string>& v) const {
            size_t seed = v.size();
            for (const auto& s : v) {
                seed ^= hash<string>{}(s)+0x9e3779b9 + (seed << 6) + (seed >> 2);
            }
            return seed;
        }
    };
}

namespace Zongine {
    size_t _GetExtensionPos(const std::string& path);
    std::string replaceFileExtension(const std::string& path, const std::string& newExtension);
    std::string getFileExtension(const std::string& path);

    void ToLower(std::string& str);
    bool TryReplaceExtension(std::filesystem::path& path, const std::filesystem::path& ext);
}