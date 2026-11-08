#include "Loan.h"
#include <cmath>
#include <stdexcept>

Loan::Loan(const std::string& id, const std::string& customerId,
           double ead, double lgd)
    : id_(id), customerId_(customerId), ead_(ead), lgd_(lgd) {
    if (id.find_first_not_of(" \t\r\n") == std::string::npos)
        throw std::invalid_argument("Loan ID cannot be blank");
    if (customerId.find_first_not_of(" \t\r\n") == std::string::npos)
        throw std::invalid_argument("Customer ID cannot be blank");
    if (!std::isfinite(ead) || ead < 0.0)
        throw std::invalid_argument("EAD must be nonnegative and finite");
    if (!std::isfinite(lgd) || lgd < 0.0 || lgd > 1.0)
        throw std::invalid_argument("LGD must be between 0 and 1");
}

const std::string& Loan::getId() const noexcept { return id_; }
const std::string& Loan::getCustomerId() const noexcept { return customerId_; }
double Loan::getEAD() const noexcept { return ead_; }
double Loan::getLGD() const noexcept { return lgd_; }

double Loan::calculateExpectedLoss(double customerPD) const {
    if (!std::isfinite(customerPD) || customerPD < 0.0 || customerPD > 1.0)
        throw std::invalid_argument("Customer PD must be between 0 and 1");
    return ead_ * lgd_ * customerPD;
}
