#pragma once

#include <array>
#include <optional>
#include <string_view>

enum class CustomerType { Individual, Corporate };
enum class LoanType { Consumer, Corporate };
enum class FacilityType { TermLoan, RevolvingCredit };

inline std::string_view toString(CustomerType type) noexcept {
    switch (type) {
        case CustomerType::Individual: return "Individual";
        case CustomerType::Corporate: return "Corporate";
    }
    return "Unknown";
}

inline std::string_view toString(LoanType type) noexcept {
    switch (type) {
        case LoanType::Consumer: return "Consumer Loan";
        case LoanType::Corporate: return "Corporate Loan";
    }
    return "Unknown";
}

inline std::string_view toString(FacilityType type) noexcept {
    switch (type) {
        case FacilityType::TermLoan: return "Term Loan";
        case FacilityType::RevolvingCredit: return "Revolving Credit";
    }
    return "Unknown";
}

inline constexpr std::array<CustomerType, 2> allCustomerTypes{
    CustomerType::Individual, CustomerType::Corporate
};
inline constexpr std::array<LoanType, 2> allLoanTypes{
    LoanType::Consumer, LoanType::Corporate
};
inline constexpr std::array<FacilityType, 2> allFacilityTypes{
    FacilityType::TermLoan, FacilityType::RevolvingCredit
};

template <typename Enum, std::size_t N>
std::optional<Enum> parseDomainType(
    std::string_view text, const std::array<Enum, N>& values) noexcept {
    for (Enum value : values) {
        if (toString(value) == text) {
            return value;
        }
    }
    return std::nullopt;
}

inline std::optional<CustomerType> parseCustomerType(std::string_view text) noexcept {
    return parseDomainType(text, allCustomerTypes);
}
inline std::optional<LoanType> parseLoanType(std::string_view text) noexcept {
    return parseDomainType(text, allLoanTypes);
}
inline std::optional<FacilityType> parseFacilityType(std::string_view text) noexcept {
    return parseDomainType(text, allFacilityTypes);
}
