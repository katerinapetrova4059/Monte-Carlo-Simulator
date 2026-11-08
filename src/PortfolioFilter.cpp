#include "PortfolioFilter.h"

#include "CorporateLoan.h"

#include <cmath>
#include <limits>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace {

template <typename T>
bool matches(const std::unordered_set<T>& allowed, T actual) {
    return allowed.empty() || allowed.contains(actual);
}

bool inRange(double value, const std::optional<double>& min,
             const std::optional<double>& max) {
    return (!min || value >= *min) && (!max || value <= *max);
}

template <typename T>
bool inRange(T value, const std::optional<T>& min,
             const std::optional<T>& max) {
    return (!min || value >= *min) && (!max || value <= *max);
}

void checkBounds(const std::optional<double>& min,
                 const std::optional<double>& max,
                 double lowest, double highest,
                 const char* field) {
    const auto valid = [=](const std::optional<double>& value) {
        return !value || (std::isfinite(*value) &&
                          *value >= lowest && *value <= highest);
    };
    if (!valid(min) || !valid(max) || (min && max && *min > *max)) {
        throw std::invalid_argument(std::string("Invalid ") + field + " filter range");
    }
}

void validate(const FilterCriteria& c) {
    checkBounds(c.minPD, c.maxPD, 0.0, 1.0, "PD");
    checkBounds(c.minLGD, c.maxLGD, 0.0, 1.0, "LGD");
    checkBounds(c.minEAD, c.maxEAD, 0.0,
                std::numeric_limits<double>::max(), "EAD");
    if (c.minLoansPerCustomer && c.maxLoansPerCustomer &&
        *c.minLoansPerCustomer > *c.maxLoansPerCustomer) {
        throw std::invalid_argument("Invalid loans-per-customer filter range");
    }
}

} // namespace

PortfolioView PortfolioFilter::apply(
    const Portfolio& portfolio, const FilterCriteria& criteria) {

    validate(criteria);

    // Count ALL loans per customer in the original portfolio.
    const auto allLoans = portfolio.getAllLoans();
    std::unordered_map<std::string, std::size_t> loanCounts;
    for (const Loan* loan : allLoans) {
        ++loanCounts[loan->getCustomerId()];
    }

    std::vector<const Loan*> selected;
    selected.reserve(allLoans.size());

    for (const Loan* loan : allLoans) {
        const Customer* customer = portfolio.getCustomer(loan->getCustomerId());
        if (!customer) {
            throw std::logic_error("Portfolio loan has no matching customer");
        }

        if (!matches(criteria.customerTypes, customer->getType()) ||
            !matches(criteria.loanTypes, loan->getType()) ||
            !inRange(customer->getPD(), criteria.minPD, criteria.maxPD) ||
            !inRange(loan->getEAD(), criteria.minEAD, criteria.maxEAD) ||
            !inRange(loan->getLGD(), criteria.minLGD, criteria.maxLGD) ||
            !inRange(loanCounts.at(customer->getId()),
                     criteria.minLoansPerCustomer,
                     criteria.maxLoansPerCustomer)) {
            continue;
        }

        // FacilityType is defined only for CorporateLoan in our current model.
        // A facility filter therefore excludes ConsumerLoan objects.
        if (!criteria.facilityTypes.empty()) {
            const auto* corporate = dynamic_cast<const CorporateLoan*>(loan);
            if (!corporate ||
                !criteria.facilityTypes.contains(corporate->getFacilityType())) {
                continue;
            }
        }

        selected.push_back(loan);
    }

    return PortfolioView(portfolio, std::move(selected));
}
