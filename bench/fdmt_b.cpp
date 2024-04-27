#include <benchmark/benchmark.h>

#include <algorithm>
#include <random>
#include <vector>
#include <dmt/fdmt.hpp>

class FDMTFixture : public benchmark::Fixture {
public:
    void SetUp(const ::benchmark::State& state) {
        f_min  = 704.0F;
        f_max  = 1216.0F;
        nchans = 4096;
        tsamp  = 0.00008192F;
        dt_max = 2048;
        nsamps = state.range(0);
    }

    void TearDown(const ::benchmark::State&) {}

    template <typename T>
    std::vector<T> generate_vector(size_t size, std::mt19937& gen) {
        std::vector<T> vec(size);
        std::uniform_real_distribution<T> dis(0.0, 1.0);
        std::generate(vec.begin(), vec.end(), [&]() { return dis(gen); });
        return vec;
    }

    float f_min;
    float f_max;
    size_t nchans;
    float tsamp;
    size_t dt_max;
    size_t nsamps;
};

BENCHMARK_DEFINE_F(FDMTFixture, BM_fdmt_plan_seq_cpu)(benchmark::State& state) {
    for (auto _ : state) {
        FDMT fdmt(f_min, f_max, nchans, nsamps, tsamp, dt_max);
    }
}

BENCHMARK_DEFINE_F(FDMTFixture, BM_fdmt_initialise_seq_cpu)(benchmark::State& state) {
    FDMT fdmt(f_min, f_max, nchans, nsamps, tsamp, dt_max);

    std::random_device rd;
    std::mt19937 gen(rd());
    auto waterfall = generate_vector<float>(nchans * nsamps, gen);
    std::vector<float> state_init(
        nchans * fdmt.get_dt_grid_init().size() * nsamps, 0.0F);
    for (auto _ : state) {
        fdmt.initialise(waterfall.data(), state_init.data());
    }
}

BENCHMARK_DEFINE_F(FDMTFixture, BM_fdmt_execute_seq_cpu)(benchmark::State& state) {
    FDMT fdmt(f_min, f_max, nchans, nsamps, tsamp, dt_max);

    std::random_device rd;
    std::mt19937 gen(rd());
    auto waterfall = generate_vector<float>(nchans * nsamps, gen);
    std::vector<float> dmt(fdmt.get_dt_grid_final().size() * nsamps, 0.0F);
    for (auto _ : state) {
        fdmt.execute(waterfall.data(), waterfall.size(), dmt.data(),
                     dmt.size());
    }
}

BENCHMARK_DEFINE_F(FDMTFixture, BM_fdmt_overall_seq_cpu)(benchmark::State& state) {
    std::random_device rd;
    std::mt19937 gen(rd());
    auto waterfall = generate_vector<float>(nchans * nsamps, gen);
    
    for (auto _ : state) {
        FDMT fdmt(f_min, f_max, nchans, nsamps, tsamp, dt_max);
        state.PauseTiming();
        std::vector<float> dmt(fdmt.get_dt_grid_final().size() * nsamps, 0.0F);
        state.ResumeTiming();
        
        fdmt.execute(waterfall.data(), waterfall.size(), dmt.data(),
                     dmt.size());
    }
}

constexpr size_t min_nsamps = 1 << 11;
constexpr size_t max_nsamps = 1 << 16;

BENCHMARK_REGISTER_F(FDMTFixture, BM_fdmt_plan_seq_cpu)
    ->RangeMultiplier(2)
    ->Range(min_nsamps, max_nsamps);
BENCHMARK_REGISTER_F(FDMTFixture, BM_fdmt_initialise_seq_cpu)
    ->RangeMultiplier(2)
    ->Range(min_nsamps, max_nsamps);
BENCHMARK_REGISTER_F(FDMTFixture, BM_fdmt_execute_seq_cpu)
    ->RangeMultiplier(2)
    ->Range(min_nsamps, max_nsamps);
BENCHMARK_REGISTER_F(FDMTFixture, BM_fdmt_overall_seq_cpu)
    ->RangeMultiplier(2)
    ->Range(min_nsamps, max_nsamps);

BENCHMARK_MAIN();
