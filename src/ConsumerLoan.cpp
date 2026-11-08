#include "ConsumerLoan.h"
#include <cmath>
#include <stdexcept>

ConsumerLoan::ConsumerLoan(
    const std::string& id, const std::string& customerId,
    double ead, double lgd, int termMonths, double monthlyInstallment)
    : Loan(id, customerId, ead, lgd),
      termMonths_(termMonths), monthlyInstallment_(monthlyInstallment) {
    if (termMonths <= 0)
        throw std::invalid_argument("Loan term must be positive");
    if (!std::isfinite(monthlyInstallment) || monthlyInstallment < 0.0)
        throw std::invalid_argument("Monthly installment must be nonnegative and finite");
}

int ConsumerLoan::getTermMonths() const noexcept { return termMonths_; }
double ConsumerLoan::getMonthlyInstallment() const noexcept { return monthlyInstallment_; }
LoanType ConsumerLoan::getType() const noexcept { return LoanType::Consumer; }
CustomerType ConsumerLoan::requiredCustomerType() const noexcept { return CustomerType::Individual; }
