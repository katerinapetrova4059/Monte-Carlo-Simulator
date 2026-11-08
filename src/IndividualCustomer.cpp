#include "IndividualCustomer.h"
#include <cmath>
#include <stdexcept>

IndividualCustomer::IndividualCustomer(
    const std::string& id, const std::string& name, double pd,
    double monthlyIncome, double monthlyDebtPayments)
    : Customer(id, name, pd), monthlyIncome_(monthlyIncome),
      monthlyDebtPayments_(monthlyDebtPayments) {
    if (!std::isfinite(monthlyIncome) || monthlyIncome < 0.0)
        throw std::invalid_argument("Monthly income must be nonnegative and finite");
    if (!std::isfinite(monthlyDebtPayments) || monthlyDebtPayments < 0.0)
        throw std::invalid_argument("Monthly debt payments must be nonnegative and finite");
}

double IndividualCustomer::getMonthlyIncome() const noexcept { return monthlyIncome_; }
double IndividualCustomer::getMonthlyDebtPayments() const noexcept { return monthlyDebtPayments_; }
std::optional<double> IndividualCustomer::calculateDTI() const noexcept {
    if (monthlyIncome_ == 0.0) return std::nullopt;
    return monthlyDebtPayments_ / monthlyIncome_;
}
CustomerType IndividualCustomer::getType() const noexcept { return CustomerType::Individual; }
