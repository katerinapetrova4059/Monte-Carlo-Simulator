
#pragma once

#include "Customer.h"
#include "Loan.h"

#include <cstddef>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

class Portfolio {
private:

    // Customers stored by Customer ID
    std::unordered_map<
        std::string,
        std::unique_ptr<Customer>
    > customers_;

    // Loans stored by Loan ID
    std::unordered_map<
        std::string,
        std::unique_ptr<Loan>
    > loans_;

public:

    // Add customers and loans
    void addCustomer(std::unique_ptr<Customer> customer);
    void addLoan(std::unique_ptr<Loan> loan);

    // Remove customers and loans
    bool removeCustomer(const std::string& customerId);
    bool removeLoan(const std::string& loanId);

    // Search by ID
    const Customer* getCustomer(
        const std::string& customerId
    ) const noexcept;

    const Loan* getLoan(
        const std::string& loanId
    ) const noexcept;

    // Retrieve collections
    std::vector<const Customer*> getAllCustomers() const;

    std::vector<const Loan*> getAllLoans() const;

    std::vector<const Loan*> getLoansForCustomer(
        const std::string& customerId
    ) const;

    // Portfolio statistics
    std::size_t getCustomerCount() const noexcept;
    std::size_t getLoanCount() const noexcept;

    double getTotalEAD() const noexcept;
    double getTotalExpectedLoss() const;
};
