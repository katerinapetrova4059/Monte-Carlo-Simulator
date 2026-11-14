#include "MonteCarloSimulator.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <future>
#include <limits>
#include <random>
#include <stdexcept>
#include <string>
#include <thread>
#include <unordered_map>
#include <utility>
//razlika mejdu derived seed i tozi random generator ot simulaciqta!
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

// Φ(x): вероятността стандартна нормална величина да е по-малка от x.
// Изразява се чрез std::erfc (complementary error function) от <cmath>.
double normalCdf(double x) {
    return 0.5 * std::erfc(-x / std::sqrt(2.0));
}

// Φ⁻¹(p): обратното - кой праг x има вероятност p да не бъде надминат.
// Намираме го с бисекция: Φ е растяща, затова разполовяваме интервала,
// докато го стесним около търсения x.
double normalInverseCdf(double p) {
    // Крайни случаи: PD = 0 -> никога не фалира, PD = 1 -> винаги фалира.
    if (p <= 0.0) return -std::numeric_limits<double>::infinity();
    if (p >= 1.0) return  std::numeric_limits<double>::infinity();

    double low = -10.0;    // Φ(-10) ≈ 1e-23, практически 0
    double high = 10.0;    // Φ(10)  ≈ 1, практически 1
    for (int i = 0; i < 100; ++i) {    // 100 разполовявания стигат за пълна double точност
        const double mid = 0.5 * (low + high);
        if (normalCdf(mid) < p)
            low = mid;     // Φ(mid) е твърде малко -> отговорът е вдясно
        else
            high = mid;    // иначе е вляво
    }
    return 0.5 * (low + high);
}

} // namespace

MonteCarloSimulator::MonteCarloSimulator(std::size_t simulationCount, std::uint64_t seed,
                                         double rho, unsigned threadCount)
    : simulationCount_(0), seed_(seed), rho_(0.0), threadCount_(1) {
    setSimulationCount(simulationCount);   // DRY principa
    setRho(rho);
    setThreadCount(threadCount);
}

unsigned MonteCarloSimulator::defaultThreadCount() noexcept {
    // 1u za da se zastrahovam ako vurne 0 hardware_concurrency()
    return std::max(1u, std::thread::hardware_concurrency());
}

std::size_t MonteCarloSimulator::getSimulationCount() const noexcept { return simulationCount_; }
std::uint64_t MonteCarloSimulator::getSeed() const noexcept { return seed_; }
double MonteCarloSimulator::getRho() const noexcept { return rho_; }
unsigned MonteCarloSimulator::getThreadCount() const noexcept { return threadCount_; }

void MonteCarloSimulator::setSimulationCount(std::size_t simulationCount) {
    if (simulationCount == 0)
        throw std::invalid_argument("Simulation count must be positive");
    simulationCount_ = simulationCount;
}

void MonteCarloSimulator::setSeed(std::uint64_t seed) noexcept { seed_ = seed; }

void MonteCarloSimulator::setRho(double rho) {
    // rho = 1 е забранено: тогава √(1−ρ) = 0 и личният фактор ε изчезва.
    if (!std::isfinite(rho) || rho < 0.0 || rho >= 1.0)
        throw std::invalid_argument("Correlation rho must be in [0, 1)");
    rho_ = rho;
}

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
        const double pd = customer->getPD();
        // Прагът се смята веднъж тук, а не при всяка от милионите итерации.
        exposures.push_back({pd, 0.0, normalInverseCdf(pd)});
    }

    // Събираме загубата при дефолт от всички кредити на клиента.
    for (const Loan* loan : view.getLoans()) {
        Exposure& exposure = exposures.at(indexById.at(loan->getCustomerId()));
        exposure.lossIfDefault += loan->getEAD() * loan->getLGD();
    }

    return exposures;
}

std::vector<double> MonteCarloSimulator::simulateChunk
(const std::vector<Exposure>& exposures,
    std::size_t iterations,
    std::uint64_t seed,
    double rho) {

    // Собствен генератор за тази нишка - нищо не се споделя с другите нишки.
    std::mt19937_64 generator(seed);
    std::normal_distribution<double> normal(0.0, 1.0);   // N(0,1) за Z и за ε

    // Теглата не зависят от сценария - смятаме корените веднъж.
    const double systematicWeight = std::sqrt(rho);         // √ρ
    const double idiosyncraticWeight = std::sqrt(1.0 - rho); // √(1−ρ)

    std::vector<double> losses;
    losses.reserve(iterations);

    for (std::size_t i = 0; i < iterations; ++i) {
        // Общият фактор "икономика" - един за всички клиенти в този сценарий.
        const double z = normal(generator);

        double loss = 0.0;
        for (const Exposure& exposure : exposures) {
            const double epsilon = normal(generator);   // личният фактор на клиента
            const double assetValue = systematicWeight * z + idiosyncraticWeight * epsilon;
            if (assetValue < exposure.threshold) {
                loss += exposure.lossIfDefault;
            }
        }
        losses.push_back(loss);
    }
    return losses;
}

SimulationResult MonteCarloSimulator::run(const PortfolioView& view) const {
    const std::vector<Exposure> exposures = buildExposures(view);

    //да не би да има повече нишки отколкото симулации
    const std::size_t chunkCount =
        std::min<std::size_t>(threadCount_, simulationCount_);
    const std::size_t baseSize = simulationCount_ / chunkCount;
    const std::size_t remainder = simulationCount_ % chunkCount;

    std::vector<std::future<std::vector<double>>> futures;
    futures.reserve(chunkCount);

    for (std::size_t chunk = 0; chunk < chunkCount; ++chunk) {
        // Остатъкът се разпределя по 1 в първите части: 10 / 3 -> 4, 3, 3.
        const std::size_t iterations = baseSize + (chunk < remainder ? 1 : 0);
        const std::uint64_t chunkSeed = deriveChunkSeed(seed_, chunk);

        futures.push_back(std::async(std::launch::async,
            [&exposures, iterations, chunkSeed, rho = rho_] {
                return simulateChunk(exposures, iterations, chunkSeed, rho);
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
