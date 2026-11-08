#include "PortfolioView.h"

#include <algorithm>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <utility>

PortfolioView::PortfolioView(
    const Portfolio& portfolio, std::vector<const Loan*> loans)
    : portfolio_(portfolio), loans_(std::move(loans)) {

    std::unordered_set<std::string> customerIds;
    for (const Loan* loan : loans_) {
        if (!loan) {
            throw std::invalid_argument("Portfolio view cannot contain null loans");
        }

        const Customer* customer = portfolio_.getCustomer(loan->getCustomerId());
        if (!customer) {
            throw std::logic_error("Selected loan has no matching customer");
        }

        if (customerIds.insert(customer->getId()).second) {
            customers_.push_back(customer);
        }
    }

    std::sort(customers_.begin(), customers_.end(),
              [](const Customer* a, const Customer* b) {
                  return a->getId() < b->getId();
              });
}

const std::vector<const Loan*>& PortfolioView::getLoans() const noexcept {
    return loans_;
}

const std::vector<const Customer*>& PortfolioView::getCustomers() const noexcept {
    return customers_;
}

std::size_t PortfolioView::getLoanCount() const noexcept {
    return loans_.size();
}

std::size_t PortfolioView::getCustomerCount() const noexcept {
    return customers_.size();
}

double PortfolioView::getTotalEAD() const noexcept {
    double total = 0.0;
    for (const Loan* loan : loans_) {
        total += loan->getEAD();
    }
    return total;
}

double PortfolioView::getTotalExpectedLoss() const {
    double total = 0.0;
    for (const Loan* loan : loans_) {
        const Customer* customer = portfolio_.getCustomer(loan->getCustomerId());
        if (!customer) {
            throw std::logic_error("Selected loan has no matching customer");
        }
        total += loan->calculateExpectedLoss(customer->getPD());
    }
    return total;
}
