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
    };

    std::size_t simulationCount_;
    std::uint64_t seed_;
    unsigned threadCount_;

    static std::vector<Exposure> buildExposures(const PortfolioView& view);

    // Симулира една част от итерациите със собствен генератор.
    // static => не пипа член-данни, затова е безопасна за изпълнение в нишка.
    static std::vector<double> simulateChunk(const std::vector<Exposure>& exposures,
                                             std::size_t iterations,
                                             std::uint64_t seed);

public:
    MonteCarloSimulator(std::size_t simulationCount, std::uint64_t seed,
                        unsigned threadCount = defaultThreadCount());

    // Брой ядра на процесора (поне 1).
    static unsigned defaultThreadCount() noexcept;

    std::size_t getSimulationCount() const noexcept;
    std::uint64_t getSeed() const noexcept;
    unsigned getThreadCount() const noexcept;
    void setSimulationCount(std::size_t simulationCount);
    void setSeed(std::uint64_t seed) noexcept;
    void setThreadCount(unsigned threadCount);

    [[nodiscard]] SimulationResult run(const PortfolioView& view) const;
};
