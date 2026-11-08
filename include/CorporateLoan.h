#pragma once

#include "Loan.h"

class CorporateLoan final : public Loan {
private:
    int termMonths_;
    FacilityType facilityType_;

public:
    CorporateLoan(const std::string& id, const std::string& customerId,
                  double ead, double lgd, int termMonths,
                  FacilityType facilityType);

    int getTermMonths() const noexcept;
    FacilityType getFacilityType() const noexcept;
    LoanType getType() const noexcept override;
    CustomerType requiredCustomerType() const noexcept override;
};
