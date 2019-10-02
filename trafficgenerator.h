#ifndef TRAFFICGENERATOR_TRAFFICGENERATOR_H
#define TRAFFICGENERATOR_TRAFFICGENERATOR_H

#include <random>

template <typename Numeric_t,
        typename = typename std::enable_if<
                std::is_convertible<Numeric_t, long double>::value
        >::type>
std::vector<Numeric_t> generateTraffic(Numeric_t intensity,
        const size_t durationSeconds, const size_t terminalsCount = 1) {
    // Setup randomization engine
    static std::random_device randomDevice;
    static std::mt19937 randomGenerator(randomDevice());

    // Setup calls distribution
    std::poisson_distribution<> poissonianDistribution(intensity);

    // Generate equilibrium traffic for given terminals
    std::vector<double> traffic(durationSeconds, 0);
    for (size_t terminal = 0; terminal < terminalsCount; ++terminal)
    {
        for (size_t second = 0; second < durationSeconds; ++second)
        {
            traffic[second] += poissonianDistribution(randomGenerator);
        }
    }
    return traffic;
}

#endif //TRAFFICGENERATOR_TRAFFICGENERATOR_H
