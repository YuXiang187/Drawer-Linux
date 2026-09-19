#pragma once

#include <memory>

class RandomEngine
{
public:
    RandomEngine();
    ~RandomEngine();

    // Returns a random integer in [min, max] (inclusive)
    int nextInt(int min, int max);

private:
    class Impl;
    std::unique_ptr<Impl> d;
};
