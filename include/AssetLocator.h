#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace AssetLocator {

std::vector<std::filesystem::path> candidates(const std::string& relativePath);
std::optional<std::filesystem::path> find(const std::string& relativePath);

}
