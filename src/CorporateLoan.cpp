#include "CorporateLoan.h"
#include <stdexcept>

CorporateLoan::CorporateLoan(
    const std::string& id, const std::string& customerId,
    double ead, double lgd, int termMonths, FacilityType facilityType)
    : Loan(id, customerId, ead, lgd),
      termMonths_(termMonths), facilityType_(facilityType) {
    if (termMonths <= 0)
        throw std::invalid_argument("Loan term must be positive");
    if (facilityType != FacilityType::TermLoan &&
        facilityType != FacilityType::RevolvingCredit)
        throw std::invalid_argument("Invalid corporate facility type");
}

int CorporateLoan::getTermMonths() const noexcept { return termMonths_; }
FacilityType CorporateLoan::getFacilityType() const noexcept { return facilityType_; }
LoanType CorporateLoan::getType() const noexcept { return LoanType::Corporate; }
CustomerType CorporateLoan::requiredCustomerType() const noexcept { return CustomerType::Corporate; }
