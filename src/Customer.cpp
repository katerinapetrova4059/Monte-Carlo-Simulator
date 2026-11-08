#include "Customer.h"
#include <cmath>
#include <stdexcept>

Customer::Customer(const std::string& id, const std::string& name, double pd)
    : id_(id), name_(name), pd_(pd) {
    if (id.find_first_not_of(" \t\r\n") == std::string::npos)
        throw std::invalid_argument("Customer ID cannot be blank");
    if (name.find_first_not_of(" \t\r\n") == std::string::npos)
        throw std::invalid_argument("Customer name cannot be blank");
    if (!std::isfinite(pd) || pd < 0.0 || pd > 1.0)
        throw std::invalid_argument("Customer PD must be between 0 and 1");
}

const std::string& Customer::getId() const noexcept { return id_; }
const std::string& Customer::getName() const noexcept { return name_; }
double Customer::getPD() const noexcept { return pd_; }
