#pragma once

#include "DomainTypes.h"
#include <string>

class Loan {
private:
    std::string id_;
    std::string customerId_;
    double ead_;
    double lgd_;

protected:
    Loan(const std::string& id, const std::string& customerId,
         double ead, double lgd);

public:
    virtual ~Loan() = default;
// trqbva da se napravqt i setter-i!!!
    const std::string& getId() const noexcept;
    const std::string& getCustomerId() const noexcept;
    double getEAD() const noexcept;
    double getLGD() const noexcept;
    double calculateExpectedLoss(double customerPD) const;

    virtual LoanType getType() const noexcept = 0;
    virtual CustomerType requiredCustomerType() const noexcept = 0;
};
