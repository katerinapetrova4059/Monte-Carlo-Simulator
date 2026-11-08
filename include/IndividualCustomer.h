#pragma once

#include "Customer.h"
#include <optional>

class IndividualCustomer final : public Customer {
private:
    double monthlyIncome_;
    double monthlyDebtPayments_;

public:
    IndividualCustomer(const std::string& id, const std::string& name,
                       double pd, double monthlyIncome,
                       double monthlyDebtPayments);

    double getMonthlyIncome() const noexcept;
    double getMonthlyDebtPayments() const noexcept;
    std::optional<double> calculateDTI() const noexcept;
    CustomerType getType() const noexcept override;
};
