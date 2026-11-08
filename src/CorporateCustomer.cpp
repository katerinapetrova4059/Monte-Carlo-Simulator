#include "CorporateCustomer.h"
#include <cmath>
#include <stdexcept>

CorporateCustomer::CorporateCustomer(
    const std::string& id, const std::string& name, double pd,
    double annualOperatingCashFlow, double annualDebtService)
    : Customer(id, name, pd),
      annualOperatingCashFlow_(annualOperatingCashFlow),
      annualDebtService_(annualDebtService) {
    if (!std::isfinite(annualOperatingCashFlow))
        throw std::invalid_argument("Operating cash flow must be finite");
    if (!std::isfinite(annualDebtService) || annualDebtService < 0.0)
        throw std::invalid_argument("Annual debt service must be nonnegative and finite");
}

double CorporateCustomer::getAnnualOperatingCashFlow() const noexcept { return annualOperatingCashFlow_; }
double CorporateCustomer::getAnnualDebtService() const noexcept { return annualDebtService_; }
std::optional<double> CorporateCustomer::calculateDSCR() const noexcept {
    if (annualDebtService_ == 0.0) return std::nullopt;
    return annualOperatingCashFlow_ / annualDebtService_;
}
CustomerType CorporateCustomer::getType() const noexcept { return CustomerType::Corporate; }
