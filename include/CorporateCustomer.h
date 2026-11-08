#pragma once

#include "Customer.h"
#include <optional>

class CorporateCustomer final : public Customer {
private:
    double annualOperatingCashFlow_;
    double annualDebtService_;

public:
    CorporateCustomer(const std::string& id, const std::string& name,
                      double pd, double annualOperatingCashFlow,
                      double annualDebtService);

    double getAnnualOperatingCashFlow() const noexcept;
    double getAnnualDebtService() const noexcept;
    std::optional<double> calculateDSCR() const noexcept;
    CustomerType getType() const noexcept override;
};
