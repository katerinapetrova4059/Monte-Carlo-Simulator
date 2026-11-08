
#include "Portfolio.h"

#include <algorithm>
#include <stdexcept>
#include <utility>

void Portfolio::addCustomer(std::unique_ptr<Customer> customer) {
    if (!customer) {
        throw std::invalid_argument(
            "Cannot add a null customer"
        );
    }

    const std::string id = customer->getId();

    if (customers_.contains(id)) {
        throw std::invalid_argument(
            "Customer ID already exists: " + id
        );
    }

    customers_.emplace(id, std::move(customer));
}

void Portfolio::addLoan(std::unique_ptr<Loan> loan) {
    if (!loan) {
        throw std::invalid_argument(
            "Cannot add a null loan"
        );
    }

    const std::string id = loan->getId();

    if (loans_.contains(id)) {
        throw std::invalid_argument(
            "Loan ID already exists: " + id
        );
    }

    const Customer* customer =
        getCustomer(loan->getCustomerId());

    if (!customer) {
        throw std::invalid_argument(
            "Customer not found: " + loan->getCustomerId()
        );
    }

    if (customer->getType() !=
        loan->requiredCustomerType()) {

        throw std::invalid_argument(
            "Loan type is incompatible with customer type"
        );
    }

    loans_.emplace(id, std::move(loan));
}

// Remove customer
bool Portfolio::removeCustomer(const std::string& customerId) {
    if (!customers_.contains(customerId)) {
        return false;
    }

    // Prevent removing a customer with active loans
    for (const auto& [loanId, loan] : loans_) {
        if (loan->getCustomerId() == customerId) {
            throw std::logic_error(
                "Cannot remove a customer with existing loans"
            );
        }
    }

    return customers_.erase(customerId) > 0;
}

bool Portfolio::removeLoan(
    const std::string& loanId
) {return loans_.erase(loanId) > 0;}

// Find customer by ID
const Customer* Portfolio::getCustomer(
    const std::string& customerId
) const noexcept {
//konstanten iterator
    const auto it = customers_.find(customerId);

    return it == customers_.end()
        ? nullptr
        : it->second.get();
}

// Find loan by ID
const Loan* Portfolio::getLoan(const std::string& loanId) const noexcept {

    const auto it = loans_.find(loanId);

    return it == loans_.end()
        ? nullptr
        : it->second.get();
}

// Get all customers
std::vector<const Customer*>
Portfolio::getAllCustomers() const {

    std::vector<const Customer*> result;

    result.reserve(customers_.size());

    for (const auto& [id, customer] : customers_) {
        result.push_back(customer.get());
    }

    std::sort(
        result.begin(),
        result.end(),
        [](const Customer* a, const Customer* b) {
            return a->getId() < b->getId();
        }
    );

    return result;
}

// Get all loans
std::vector<const Loan*>
Portfolio::getAllLoans() const {

    std::vector<const Loan*> result;

    result.reserve(loans_.size());

    for (const auto& [id, loan] : loans_) {
        result.push_back(loan.get());
    }

    std::sort(
        result.begin(),
        result.end(),
        [](const Loan* a, const Loan* b) {
            return a->getId() < b->getId();
        }
    );

    return result;
}

// Get loans belonging to a specific customer
std::vector<const Loan*>
Portfolio::getLoansForCustomer(
    const std::string& customerId
) const {

    std::vector<const Loan*> result;

    if (!getCustomer(customerId)) {
        return result;
    }

    for (const auto& [id, loan] : loans_) {
        if (loan->getCustomerId() == customerId) {
            result.push_back(loan.get());
        }
    }

    std::sort(
        result.begin(),
        result.end(),
        [](const Loan* a, const Loan* b) {
            return a->getId() < b->getId();
        }
    );

    return result;
}

// Number of customers
std::size_t Portfolio::getCustomerCount() const noexcept {
    return customers_.size();
}

// Number of loans
std::size_t Portfolio::getLoanCount() const noexcept {
    return loans_.size();
}

// Total Exposure at Default
double Portfolio::getTotalEAD() const noexcept {

    double total = 0.0;

    for (const auto& [id, loan] : loans_) {
        total += loan->getEAD();
    }

    return total;
}

// Total Expected Loss
double Portfolio::getTotalExpectedLoss() const {

    double total = 0.0;

    for (const auto& [id, loan] : loans_) {

        const Customer* customer =
            getCustomer(loan->getCustomerId());

        if (!customer) {
            throw std::logic_error(
                "Portfolio contains a loan without a customer"
            );
        }

        total += loan->calculateExpectedLoss(
            customer->getPD()
        );
    }

    return total;
}
