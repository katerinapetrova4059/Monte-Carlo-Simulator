#pragma once

#include "Loan.h"

class ConsumerLoan final : public Loan {
private:
    int termMonths_;
    double monthlyInstallment_;

public:
    ConsumerLoan(const std::string& id, const std::string& customerId,
                 double ead, double lgd, int termMonths,
                 double monthlyInstallment);

    int getTermMonths() const noexcept;
    double getMonthlyInstallment() const noexcept;
    LoanType getType() const noexcept override;
    CustomerType requiredCustomerType() const noexcept override;
};
