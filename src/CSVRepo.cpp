#include "CSVRepo.h"
#include "IndividualCustomer.h"
#include "CorporateCustomer.h"
#include "ConsumerLoan.h"
#include "CorporateLoan.h"
#include "DomainTypes.h"

#include <cmath>
#include <cctype>
#include <fstream>
#include <initializer_list>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace {

std::string trim(std::string_view text) {
    while (!text.empty() && std::isspace(static_cast<unsigned char>(text.front())))
        text.remove_prefix(1);
    while (!text.empty() && std::isspace(static_cast<unsigned char>(text.back())))
        text.remove_suffix(1);
    return std::string(text);
}

std::vector<std::string> parseCsvLine(const std::string& line) {
    std::vector<std::string> fields;
    std::string field;
    bool inQuotes = false;
    bool afterQuote = false;

    for (std::size_t i = 0; i < line.size(); ++i) {
        const char ch = line[i];
        if (inQuotes) {
            if (ch == '"') {
                if (i + 1 < line.size() && line[i + 1] == '"') {
                    field += '"';
                    ++i;
                } else {
                    inQuotes = false;
                    afterQuote = true;
                }
            } else {
                field += ch;
            }
        } else if (ch == ',') {
            fields.push_back(trim(field));
            field.clear();
            afterQuote = false;
        } else if (ch == '"') {
            if (afterQuote || !trim(field).empty())
                throw std::invalid_argument("Unexpected quote in CSV field");
            field.clear();
            inQuotes = true;
        } else if (afterQuote) {
            if (!std::isspace(static_cast<unsigned char>(ch)))
                throw std::invalid_argument("Unexpected text after closing quote");
        } else {
            field += ch;
        }
    }
    if (inQuotes)
        throw std::invalid_argument("Unclosed quoted CSV field");
    fields.push_back(trim(field));
    return fields;
}

double readDouble(const std::string& value, std::string_view column) {
    if (value.empty())
        throw std::invalid_argument("Missing value for " + std::string(column));
    try {
        std::size_t end = 0;
        const double number = std::stod(value, &end);
        if (end != value.size() || !std::isfinite(number))
            throw std::invalid_argument("invalid");
        return number;
    } catch (const std::exception&) {
        throw std::invalid_argument("Invalid number in " + std::string(column) + ": " + value);
    }
}

int readInt(const std::string& value, std::string_view column) {
    if (value.empty())
        throw std::invalid_argument("Missing value for " + std::string(column));
    try {
        std::size_t end = 0;
        const int number = std::stoi(value, &end);
        if (end != value.size())
            throw std::invalid_argument("invalid");
        return number;
    } catch (const std::exception&) {
        throw std::invalid_argument("Invalid integer in " + std::string(column) + ": " + value);
    }
}

template <typename Callback>
void forEachCsvRow(const std::filesystem::path& path,
                   std::initializer_list<std::string_view> required,
                   Callback onRecord) {
    std::ifstream file(path);
    if (!file)
        throw std::runtime_error("Cannot open CSV file: " + path.string());

    std::string line;
    if (!std::getline(file, line))
        throw std::runtime_error("CSV file is empty: " + path.string());

    // Strip UTF-8 BOM, which some spreadsheet applications export.
    if (line.size() >= 3 &&
        static_cast<unsigned char>(line[0]) == 0xEF &&
        static_cast<unsigned char>(line[1]) == 0xBB &&
        static_cast<unsigned char>(line[2]) == 0xBF) {
        line.erase(0, 3);
    }

    std::vector<std::string> headers;
    try {
        headers = parseCsvLine(line);
    } catch (const std::exception& e) {
        throw std::runtime_error(path.string() + ":1: " + e.what());
    }

    std::unordered_map<std::string, std::size_t> index;
    for (std::size_t i = 0; i < headers.size(); ++i) {
        if (headers[i].empty() || !index.emplace(headers[i], i).second)
            throw std::runtime_error(path.string() + ":1: Duplicate or empty CSV column");
    }
    for (const auto requiredColumn : required) {
        if (!index.contains(std::string(requiredColumn)))
            throw std::runtime_error(path.string() + ":1: Missing column " + std::string(requiredColumn));
    }

    std::size_t lineNumber = 1;
    while (std::getline(file, line)) {
        ++lineNumber;
        if (trim(line).empty())
            continue;
        try {
            const auto fields = parseCsvLine(line);
            if (fields.size() != headers.size())
                throw std::invalid_argument("Wrong number of CSV columns");

            const auto get = [&](std::string_view column) -> const std::string& {
                return fields.at(index.at(std::string(column)));
            };
            onRecord(get);
        } catch (const std::exception& e) {
            throw std::runtime_error(path.string() + ":" +
                                     std::to_string(lineNumber) + ": " + e.what());
        }
    }
    if (file.bad())
        throw std::runtime_error("Read error in CSV file: " + path.string());
}

} // TUK SPIRA NAMESPACE

Portfolio CSVRepo::loadPortfolio(const std::filesystem::path& customersFile,
                                 const std::filesystem::path& loansFile) {
    Portfolio portfolio;

    forEachCsvRow(customersFile,
        {"customer_id", "name", "customer_type", "pd",
         "monthly_income", "monthly_debt_payments",
         "annual_operating_cash_flow", "annual_debt_service"},
        [&](const auto& get) {
            const auto type = parseCustomerType(get("customer_type"));
            if (!type)
                throw std::invalid_argument("Unknown customer type: " + get("customer_type"));

            const auto& id = get("customer_id");
            const auto& name = get("name");
            const double pd = readDouble(get("pd"), "pd");

            switch (*type) {
                case CustomerType::Individual:
                    portfolio.addCustomer(std::make_unique<IndividualCustomer>(
                        id, name, pd,
                        readDouble(get("monthly_income"), "monthly_income"),
                        readDouble(get("monthly_debt_payments"), "monthly_debt_payments")));
                    break;
                case CustomerType::Corporate:
                    portfolio.addCustomer(std::make_unique<CorporateCustomer>(
                        id, name, pd,
                        readDouble(get("annual_operating_cash_flow"), "annual_operating_cash_flow"),
                        readDouble(get("annual_debt_service"), "annual_debt_service")));
                    break;
            }
        });

    forEachCsvRow(loansFile,
        {"loan_id", "customer_id", "loan_type", "ead", "lgd",
         "term_months", "monthly_installment", "facility_type"},
        [&](const auto& get) {
            const auto type = parseLoanType(get("loan_type"));
            if (!type)
                throw std::invalid_argument("Unknown loan type: " + get("loan_type"));

            const auto& id = get("loan_id");
            const auto& customerId = get("customer_id");
            const double ead = readDouble(get("ead"), "ead");
            const double lgd = readDouble(get("lgd"), "lgd");
            const int term = readInt(get("term_months"), "term_months");

            switch (*type) {
                case LoanType::Consumer:
                    if (!get("facility_type").empty())
                        throw std::invalid_argument("Consumer loan cannot specify facility_type");
                    portfolio.addLoan(std::make_unique<ConsumerLoan>(
                        id, customerId, ead, lgd, term,
                        readDouble(get("monthly_installment"), "monthly_installment")));
                    break;
                case LoanType::Corporate: {
                    if (!get("monthly_installment").empty())
                        throw std::invalid_argument("Corporate loan cannot specify monthly_installment");
                    const auto facility = parseFacilityType(get("facility_type"));
                    if (!facility)
                        throw std::invalid_argument("Unknown facility type: " + get("facility_type"));
                    portfolio.addLoan(std::make_unique<CorporateLoan>(
                        id, customerId, ead, lgd, term, *facility));
                    break;
                }
            }
        });

    return portfolio;
}
