#pragma once

#include "FilterCriteria.h"
#include "PortfolioView.h"

class PortfolioFilter {
public:
    [[nodiscard]] static PortfolioView apply(
        const Portfolio& portfolio,
        const FilterCriteria& criteria
    );
};
