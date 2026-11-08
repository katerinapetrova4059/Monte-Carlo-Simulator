#pragma once

#include "DomainTypes.h"

#include <cstddef>
#include <optional>
#include <unordered_set>

// Empty category set means "include all values".
// Bounds are inclusive. Loan count is based on the original portfolio.
struct FilterCriteria {
    std::unordered_set<CustomerType> customerTypes;
    std::unordered_set<LoanType> loanTypes;
    std::unordered_set<FacilityType> facilityTypes;

    std::optional<double> minPD;
    std::optional<double> maxPD;
    std::optional<double> minEAD;
    std::optional<double> maxEAD;
    std::optional<double> minLGD;
    std::optional<double> maxLGD;

    std::optional<std::size_t> minLoansPerCustomer;
    std::optional<std::size_t> maxLoansPerCustomer;
};
