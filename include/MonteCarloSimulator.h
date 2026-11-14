#pragma once

#include "PortfolioView.h"
#include "SimulationResult.h"

#include <cstddef>
#include <cstdint>
#include <vector>

class MonteCarloSimulator {
private:
    // Един клиент, сведен до това, което симулацията използва.
    struct Exposure {
        double pd;              // вероятност за дефолт
        double lossIfDefault;   // сумата от EAD × LGD на кредитите му
        double threshold;       // Φ⁻¹(pd): дефолт, ако стойността на активите падне под него
    };

    std::size_t simulationCount_;
    std::uint64_t seed_;
    double rho_;                // корелация с общия фактор Z, в [0, 1)
    unsigned threadCount_;

    static std::vector<Exposure> buildExposures(const PortfolioView& view);

    // Симулира една част от итерациите със собствен генератор.
    // static => не пипа член-данни, затова е безопасна за изпълнение в нишка.
    static std::vector<double> simulateChunk(const std::vector<Exposure>& exposures,
                                             std::size_t iterations,
                                             std::uint64_t seed,
                                             double rho);

public:
    // rho = 0 означава независими дефолти (без общ фактор).
    MonteCarloSimulator(std::size_t simulationCount, std::uint64_t seed,
                        double rho = 0.0,
                        unsigned threadCount = defaultThreadCount());

    // Брой ядра на процесора (поне 1).
    static unsigned defaultThreadCount() noexcept;

    std::size_t getSimulationCount() const noexcept;
    std::uint64_t getSeed() const noexcept;
    double getRho() const noexcept;
    unsigned getThreadCount() const noexcept;
    void setSimulationCount(std::size_t simulationCount);
    void setSeed(std::uint64_t seed) noexcept;
    void setRho(double rho);
    void setThreadCount(unsigned threadCount);

    [[nodiscard]] SimulationResult run(const PortfolioView& view) const;
};
