#pragma once

#include "Portfolio.h"
#include <filesystem>

class CSVRepo {
public:
    [[nodiscard]] static Portfolio loadPortfolio(
        const std::filesystem::path& customersFile,
        const std::filesystem::path& loansFile);
};
