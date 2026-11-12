#include "SimulationResult.h"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <numeric>
#include <stdexcept>
#include <utility>

namespace {


std::size_t varIndex(double confidence, std::size_t count) {
    if (!std::isfinite(confidence) || confidence <= 0.0 || confidence >= 1.0)
        throw std::invalid_argument("Confidence level must be between 0 and 1 (exclusive)");

    // Напр. при 1000 итерации и 0.99: ceil(990) - 1 = 989 (990-тата загуба).
    const auto index = static_cast<std::size_t>(
        std::ceil(confidence * static_cast<double>(count))) - 1;
    return std::min(index, count - 1);
}

} // namespace

SimulationResult::SimulationResult(std::vector<double> losses)
    : losses_(std::move(losses)), mean_(0.0), stdDev_(0.0) {

    if (losses_.empty())
        throw std::invalid_argument("Simulation result needs at least one loss value");

    const auto invalid = std::find_if(losses_.begin(), losses_.end(),
        [](double loss) { return !std::isfinite(loss) || loss < 0.0; });
    if (invalid != losses_.end())
        throw std::invalid_argument("Loss values must be nonnegative and finite");

    std::sort(losses_.begin(), losses_.end());

    const double n = static_cast<double>(losses_.size());
    mean_ = std::accumulate(losses_.begin(), losses_.end(), 0.0) / n;

    if (losses_.size() > 1) {
        double sumSquares = 0.0;
        for (double loss : losses_) {
            const double diff = loss - mean_;
            sumSquares += diff * diff;
        }
        stdDev_ = std::sqrt(sumSquares / (n - 1.0));
    }
}

std::size_t SimulationResult::getSimulationCount() const noexcept { return losses_.size(); }
double SimulationResult::getMean() const noexcept { return mean_; }
double SimulationResult::getStdDev() const noexcept { return stdDev_; }
double SimulationResult::getMinLoss() const noexcept { return losses_.front(); }
double SimulationResult::getMaxLoss() const noexcept { return losses_.back(); }
const std::vector<double>& SimulationResult::getLosses() const noexcept { return losses_; }

double SimulationResult::getVaR(double confidence) const {
    return losses_[varIndex(confidence, losses_.size())];
}

double SimulationResult::getExpectedShortfall(double confidence) const {
    const auto start = losses_.begin() +
        static_cast<std::ptrdiff_t>(varIndex(confidence, losses_.size()));
    const auto tailCount = static_cast<double>(losses_.end() - start);
    return std::accumulate(start, losses_.end(), 0.0) / tailCount;
}

std::ostream& operator<<(std::ostream& os, const SimulationResult& result) {
    // Запазваме форматирането на потока, за да не го променим за извикващия.
    const auto oldFlags = os.flags();
    const auto oldPrecision = os.precision();

    os << std::fixed << std::setprecision(2)
       << "Simulations:           " << result.getSimulationCount() << '\n'
       << "Mean loss:             " << result.getMean() << '\n'
       << "Std deviation:         " << result.getStdDev() << '\n'
       << "Min / Max loss:        " << result.getMinLoss() << " / " << result.getMaxLoss() << '\n'
       << "VaR 95%:               " << result.getVaR(0.95) << '\n'
       << "VaR 99%:               " << result.getVaR(0.99) << '\n'
       << "Expected Shortfall 95%: " << result.getExpectedShortfall(0.95) << '\n'
       << "Expected Shortfall 99%: " << result.getExpectedShortfall(0.99) << '\n';

    os.flags(oldFlags);
    os.precision(oldPrecision);
    return os;
}
