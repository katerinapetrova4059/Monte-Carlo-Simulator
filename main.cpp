#include "CSVRepo.h"
#include "DomainTypes.h"
#include "MonteCarloSimulator.h"
#include "PortfolioFilter.h"

#include <filesystem>
#include <iomanip>
#include <iostream>
#include <stdexcept>

void printScenario(const char* name, const PortfolioView& view) {
    std::cout << name << "\n"
              << "  Customers: " << view.getCustomerCount() << "\n"
              << "  Loans: " << view.getLoanCount() << "\n"
              << "  EAD: " << view.getTotalEAD() << "\n"
              << "  Expected Loss: " << view.getTotalExpectedLoss() << "\n\n";
}

int main() {
    try {
        const std::filesystem::path dataDir = PROJECT_DATA_DIR;
        Portfolio portfolio = CSVRepo::loadPortfolio(
            dataDir / "customers.csv", dataDir / "loans.csv");

        std::cout << std::fixed << std::setprecision(2);
        std::cout << "=== PORTFOLIO FILTER / WHAT-IF ANALYSIS ===\n\n";

        // No restrictions: select the full portfolio.
        FilterCriteria all;
        const auto original = PortfolioFilter::apply(portfolio, all);
        printScenario("Original portfolio", original);

        // Monte Carlo върху целия портфейл: 100 000 сценария, seed 42.
        // Средната симулирана загуба трябва да е близо до Expected Loss отгоре,
        // независимо от rho. Корелацията променя само опашката (VaR / ES).
        for (const double rho : {0.0, 0.2}) {
            const MonteCarloSimulator simulator(100'000, 42, rho);
            const SimulationResult result = simulator.run(original);
            std::cout << "=== MONTE CARLO (rho = " << rho << ", "
                      << simulator.getThreadCount() << " threads) ===\n"
                      << result << "\n\n";
        }

        // First scenario: individuals who hold at least two loans.
        FilterCriteria individuals;
        individuals.customerTypes.insert(CustomerType::Individual);
        individuals.minLoansPerCustomer = 2;
        const auto scenarioA = PortfolioFilter::apply(portfolio, individuals);
        printScenario("Scenario A: individuals with >= 2 loans", scenarioA);

        // Second scenario: corporate term facilities only.
        FilterCriteria termFacilities;
        termFacilities.facilityTypes.insert(FacilityType::TermLoan);
        const auto scenarioB = PortfolioFilter::apply(portfolio, termFacilities);
        printScenario("Scenario B: term facilities only", scenarioB);

        // The source portfolio has not changed.
        std::cout << "Unchanged original: " << portfolio.getCustomerCount()
                  << " customers, " << portfolio.getLoanCount() << " loans\n";

        // DomainTypes remains the single catalog used to populate GUI options.
        std::cout << "\nAvailable loan types: ";
        for (auto type : allLoanTypes) {
            std::cout << toString(type) << "; ";
        }
        std::cout << '\n';
    } catch (const std::exception& error) {
        std::cerr << "Error: " << error.what() << '\n';
        return 1;
    }
    return 0;
}
