#pragma once

#include "Portfolio.h"

#include <cstddef>
#include <vector>

class PortfolioFilter;

// Read-only, non-owning selection of loans and their customers.
// The original Portfolio must outlive the view and must not be changed
// while this view is being used (especially no loan/customer removals).
class PortfolioView {
private:
    const Portfolio& portfolio_;
    std::vector<const Loan*> loans_;
    std::vector<const Customer*> customers_;

    PortfolioView(const Portfolio& portfolio, std::vector<const Loan*> loans);
    friend class PortfolioFilter;

public:
    const std::vector<const Loan*>& getLoans() const noexcept;
    const std::vector<const Customer*>& getCustomers() const noexcept;

    std::size_t getLoanCount() const noexcept;
    std::size_t getCustomerCount() const noexcept;
    double getTotalEAD() const noexcept;
    double getTotalExpectedLoss() const;
};
