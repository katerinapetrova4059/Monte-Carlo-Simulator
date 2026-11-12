#include "MonteCarloSimulator.h"

#include <algorithm>
#include <array>
#include <future>
#include <random>
#include <stdexcept>
#include <string>
#include <thread>
#include <unordered_map>
#include <utility>

namespace {

// Получава различен, но повторим seed за всяка част от основния seed.
// std::seed_seq "разбърква" входа, така че близки входове (42/0, 42/1)
// дават съвсем различни seed-ове.
std::uint64_t deriveChunkSeed(std::uint64_t baseSeed, std::size_t chunkIndex) {
    std::seed_seq sequence{
        static_cast<std::uint32_t>(baseSeed),          // долните 32 бита
        static_cast<std::uint32_t>(baseSeed >> 32),    // горните 32 бита
        static_cast<std::uint32_t>(chunkIndex)
    };
    std::array<std::uint32_t, 2> out{};
    sequence.generate(out.begin(), out.end());
    return (static_cast<std::uint64_t>(out[1]) << 32) | out[0];
}

} // namespace

MonteCarloSimulator::MonteCarloSimulator(std::size_t simulationCount, std::uint64_t seed,
                                         unsigned threadCount)
    : simulationCount_(0), seed_(seed), threadCount_(1) {
    setSimulationCount(simulationCount);   // DRY principa
    setThreadCount(threadCount);
}

unsigned MonteCarloSimulator::defaultThreadCount() noexcept {
    // hardware_concurrency() може да върне 0, ако не може да определи броя ядра.
    return std::max(1u, std::thread::hardware_concurrency());
}

std::size_t MonteCarloSimulator::getSimulationCount() const noexcept { return simulationCount_; }
std::uint64_t MonteCarloSimulator::getSeed() const noexcept { return seed_; }
unsigned MonteCarloSimulator::getThreadCount() const noexcept { return threadCount_; }

void MonteCarloSimulator::setSimulationCount(std::size_t simulationCount) {
    if (simulationCount == 0)
        throw std::invalid_argument("Simulation count must be positive");
    simulationCount_ = simulationCount;
}

void MonteCarloSimulator::setSeed(std::uint64_t seed) noexcept { seed_ = seed; }

void MonteCarloSimulator::setThreadCount(unsigned threadCount) {
    if (threadCount == 0)
        throw std::invalid_argument("Thread count must be positive");
    threadCount_ = threadCount;
}

std::vector<MonteCarloSimulator::Exposure>
MonteCarloSimulator::buildExposures(const PortfolioView& view) {
    std::vector<Exposure> exposures;
    exposures.reserve(view.getCustomerCount());

    // pomoshtna tablica za po-burzo tursene
    std::unordered_map<std::string, std::size_t> indexById;
    for (const Customer* customer : view.getCustomers()) {
        indexById.emplace(customer->getId(), exposures.size());
        exposures.push_back({customer->getPD(), 0.0});
    }

    // Събираме загубата при дефолт от всички кредити на клиента.
    for (const Loan* loan : view.getLoans()) {
        Exposure& exposure = exposures.at(indexById.at(loan->getCustomerId()));
        exposure.lossIfDefault += loan->getEAD() * loan->getLGD();
    }

    return exposures;
}

std::vector<double> MonteCarloSimulator::simulateChunk(
    const std::vector<Exposure>& exposures, std::size_t iterations, std::uint64_t seed) {

    // Собствен генератор за тази нишка - нищо не се споделя с другите нишки.
    std::mt19937_64 generator(seed);
    std::uniform_real_distribution<double> uniform(0.0, 1.0);

    std::vector<double> losses;
    losses.reserve(iterations);

    for (std::size_t i = 0; i < iterations; ++i) {
        double loss = 0.0;
        for (const Exposure& exposure : exposures) {
            if (uniform(generator) < exposure.pd) {
                loss += exposure.lossIfDefault;
            }
        }
        losses.push_back(loss);
    }
    return losses;
}

SimulationResult MonteCarloSimulator::run(const PortfolioView& view) const {
    const std::vector<Exposure> exposures = buildExposures(view);

    // Не пускаме повече нишки от итерациите (напр. 3 итерации, 8 ядра).
    const std::size_t chunkCount =
        std::min<std::size_t>(threadCount_, simulationCount_);
    const std::size_t baseSize = simulationCount_ / chunkCount;
    const std::size_t remainder = simulationCount_ % chunkCount;

    // 1) Пускаме по една задача за всяка част.
    std::vector<std::future<std::vector<double>>> futures;
    futures.reserve(chunkCount);

    for (std::size_t chunk = 0; chunk < chunkCount; ++chunk) {
        // Остатъкът се разпределя по 1 в първите части: 10 / 3 -> 4, 3, 3.
        const std::size_t iterations = baseSize + (chunk < remainder ? 1 : 0);
        const std::uint64_t chunkSeed = deriveChunkSeed(seed_, chunk);

        futures.push_back(std::async(std::launch::async,
            [&exposures, iterations, chunkSeed] {
                return simulateChunk(exposures, iterations, chunkSeed);
            }));
    }

    // 2) Изчакваме всяка задача и сливаме резултатите.
    std::vector<double> losses;
    losses.reserve(simulationCount_);

    for (auto& future : futures) {
        const std::vector<double> part = future.get();   // блокира, докато частта е готова
        losses.insert(losses.end(), part.begin(), part.end());
    }

    return SimulationResult(std::move(losses));
}
