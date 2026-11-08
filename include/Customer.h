#pragma once
#include "DomainTypes.h"
#include <string>

class Customer {
private:
    std::string id_;
    std::string name_;
    double pd_;

protected:
    Customer(const std::string& id, const std::string& name, double pd);

public:
    virtual ~Customer() = default;

    const std::string& getId() const noexcept;
    const std::string& getName() const noexcept;
    double getPD() const noexcept;
    virtual CustomerType getType() const noexcept = 0;
};
