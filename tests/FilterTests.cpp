#include "CSVRepo.h"
#include "PortfolioFilter.h"
#include <cassert>
#include <cmath>
#include <filesystem>
#include <stdexcept>

bool near(double a, double b) { return std::fabs(a - b) < 1e-9; }

int main() {
    const std::filesystem::path dataDir = PROJECT_DATA_DIR;
    Portfolio portfolio = CSVRepo::loadPortfolio(
        dataDir / "customers.csv", dataDir / "loans.csv");
    {
        FilterCriteria f;
        auto v = PortfolioFilter::apply(portfolio, f);
        assert(v.getCustomerCount() == 2 && v.getLoanCount() == 3);
        assert(near(v.getTotalEAD(), 515000) && near(v.getTotalExpectedLoss(), 4240));
    }
    {
        FilterCriteria f;
        f.loanTypes.insert(LoanType::Consumer);
        auto v = PortfolioFilter::apply(portfolio, f);
        assert(v.getCustomerCount() == 1 && v.getLoanCount() == 2);
        assert(near(v.getTotalEAD(), 15000));
    }
    {
        FilterCriteria f;
        f.customerTypes.insert(CustomerType::Individual);
        f.facilityTypes.insert(FacilityType::TermLoan);
        auto v = PortfolioFilter::apply(portfolio, f);
        assert(v.getLoanCount() == 0 && v.getCustomerCount() == 0);
        assert(near(v.getTotalEAD(), 0) && near(v.getTotalExpectedLoss(), 0));
    }
    {
        FilterCriteria f;
        f.minLoansPerCustomer = 2;
        auto v = PortfolioFilter::apply(portfolio, f);
        assert(v.getLoanCount() == 2 && v.getCustomerCount() == 1);
    }
    {
        FilterCriteria f;
        f.maxPD = 0.025;
        auto v = PortfolioFilter::apply(portfolio, f);
        assert(v.getLoanCount() == 1 && near(v.getTotalExpectedLoss(), 4000));
    }
    {
        FilterCriteria f;
        f.minEAD = 6000;
        f.maxLGD = .5;
        auto v = PortfolioFilter::apply(portfolio, f);
        assert(v.getLoanCount() == 1 && near(v.getTotalEAD(), 500000));
    }
    {
        FilterCriteria f;
        f.minPD = 0.10;
        f.maxPD = 0.01;
        bool rejected = false;
        try { (void)PortfolioFilter::apply(portfolio, f); }
        catch (const std::invalid_argument&) { rejected = true; }
        assert(rejected);
    }
    assert(portfolio.getCustomerCount() == 2 && portfolio.getLoanCount() == 3);
}
