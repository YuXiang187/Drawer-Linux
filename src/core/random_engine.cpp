#include "random_engine.h"
#include <chrono>
#include <random>

class RandomEngine::Impl
{
public:
    std::mt19937 generator;

    Impl()
    {
        try {
            std::random_device device;
            generator.seed(device());
        } catch (const std::exception&) {
            auto now = std::chrono::high_resolution_clock::now();
            generator.seed(static_cast<unsigned>(
                now.time_since_epoch().count()));
        }
    }
};

RandomEngine::RandomEngine()
    : d(std::make_unique<Impl>())
{}

RandomEngine::~RandomEngine() = default;

int RandomEngine::nextInt(int min, int max)
{
    if (min >= max)
        return min;

    std::uniform_int_distribution<int> dist(min, max);
    return dist(d->generator);
}

double RandomEngine::nextGaussian(double mean, double stddev)
{
    // std::normal_distribution requires a positive standard deviation.
    if (stddev <= 0.0)
        return mean;

    std::normal_distribution<double> dist(mean, stddev);
    return dist(d->generator);
}
