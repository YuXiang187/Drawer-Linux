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
