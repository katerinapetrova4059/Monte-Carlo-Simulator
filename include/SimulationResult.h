#pragma once

#include <cstddef>
#include <ostream>
#include <vector>

class SimulationResult {
private:
    std::vector<double> losses_;
    double mean_;
    double stdDev_;

public:
    explicit SimulationResult(std::vector<double> losses);

    std::size_t getSimulationCount() const noexcept;
    double getMean() const noexcept;
    double getStdDev() const noexcept;
    double getMinLoss() const noexcept;
    double getMaxLoss() const noexcept;

    double getVaR(double confidence) const;
    double getExpectedShortfall(double confidence) const;

    const std::vector<double>& getLosses() const noexcept;

    friend std::ostream& operator<<(std::ostream& os, const SimulationResult& result);
};
