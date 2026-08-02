#include "AssetLocator.h"

#include <iostream>
#include <mutex>
#include <sstream>
#include <unordered_set>

namespace {
std::unordered_set<std::string> reportedMissing;
std::mutex reportedMissingMutex;
}

std::vector<std::filesystem::path> AssetLocator::candidates(
    const std::string& relativePath) {
    const std::filesystem::path requested(relativePath);
    return {requested, std::filesystem::path("..") / requested,
            std::filesystem::path("../..") / requested};
}

std::optional<std::filesystem::path> AssetLocator::find(
    const std::string& relativePath) {
    const auto paths = candidates(relativePath);
    std::error_code error;
    for (const auto& path : paths) {
        if (std::filesystem::is_regular_file(path, error)) {
            return path.lexically_normal();
        }
        error.clear();
    }

    std::lock_guard<std::mutex> lock(reportedMissingMutex);
    if (reportedMissing.insert(relativePath).second) {
        std::ostringstream tried;
        for (std::size_t i = 0; i < paths.size(); ++i) {
            if (i > 0) tried << ", ";
            tried << paths[i].generic_string();
        }
        std::cerr << "Asset missing: " << relativePath
                  << " (tried: " << tried.str() << ")\n";
    }
    return std::nullopt;
}
