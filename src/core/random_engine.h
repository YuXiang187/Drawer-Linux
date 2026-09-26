#pragma once

#include <memory>

class RandomEngine
{
public:
    RandomEngine();
    ~RandomEngine();

    // Returns a random integer in [min, max] (inclusive)
    int nextInt(int min, int max);

    // Returns a value sampled from a normal (Gaussian) distribution with the
    // given mean and standard deviation. This is the C++ counterpart of
    // rand_distr::Normal::new(mean, stddev).sample(rng) used by rpick.
    double nextGaussian(double mean, double stddev);

private:
    class Impl;
    std::unique_ptr<Impl> d;
};
