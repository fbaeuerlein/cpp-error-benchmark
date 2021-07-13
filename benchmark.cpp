#include <iostream>
#include <time.h>

#include <chrono>
#include <optional>
#include <functional>
#include <ctime>
#include "tl/expected.hpp"
#include "benchmark/benchmark.h"
#include <vector>
#include <numeric>
#include <random>

#define E_FAIL -1
#define E_OK 0

#if 1
#define OPTIMIZE(expr) benchmark::DoNotOptimize(expr);
#else
#define OPTIMIZE(expr) expr;
#endif

class ErrorInjector
{
    public:
        ErrorInjector(unsigned int ratio) : _ratio(ratio), _count(0) {}

        bool error() 
        {
            if ( ++_count == _ratio )
            {
                _count = 0;
                return true;
            }
            return false;
        }

        bool ok() { return !error(); }

    private:
        unsigned int _ratio;
        unsigned int _count;

};

template<typename T>
class MeanCalculation
{
    public:

    MeanCalculation() {}

    void add(T const & value) { _data.push_back(value); }
    void clear() { _data.clear(); }

    int by_ref(T & result ) const
    {
        if ( _data.empty() ) 
        {
            return E_FAIL;
        }

        result = mean();
        return E_OK;
    }

    T with_exception() const
    {
        if ( _data.empty() ) throw std::logic_error("No data for mean calculation");
        return mean();
    }

    std::optional<T> with_optional() const
    {
        if ( _data.empty() ) return std::nullopt;
        return { mean() };
    }

    tl::expected<T, int> with_expected() const
    {
        if (_data.empty()) return tl::unexpected<int>(E_FAIL);
        return mean();
    }

    std::tuple<bool, T> with_tuple() const
    {
        if (_data.empty()) return std::make_tuple(false, T{});
        return std::make_tuple(true, mean());
    }

    private:

    T mean() const 
    {
        return std::accumulate(std::begin(_data), std::end(_data), 0.) / _data.size();
    }

    std::vector<T> _data;
};

void addValues(MeanCalculation<double> & m, ErrorInjector & e, benchmark::State & state)
{
    if (e.ok() )
    {
        for(volatile int i = 0; i < 5; ++i )
        {
            m.add(i);
        }
    }
    else
    {
        state.counters["errors"]++;
    }
}

static void BM_do_with_exception(benchmark::State &state)
{
    ErrorInjector err(state.range(0));
    state.counters["errors"] = 0;
    state.counters["ratio"] = state.range(0);
    auto x = [&](MeanCalculation<double> & m) -> double {
        addValues(m, err, state);
        try
        {
            return m.with_exception();
        }
        catch(const std::exception& e)
        {
            return 0.;
        }
    };

    for (auto _ : state)
    {
        MeanCalculation<double> m;
        OPTIMIZE(x(m));
    }
}

static void BM_do_with_optional(benchmark::State &state)
{
    ErrorInjector err(state.range(0));
    state.counters["errors"] = 0;
    state.counters["ratio"] = state.range(0);
    auto x = [&](MeanCalculation<double> & m) -> double {
        addValues(m, err, state);
        auto v = m.with_optional();
        return v.value_or(0);
    };

    for (auto _ : state)
    {
        MeanCalculation<double> m;
        OPTIMIZE(x(m));
    }
}



static void BM_do_with_ref(benchmark::State &state)
{
    MeanCalculation<double> m;
    ErrorInjector err(state.range(0));
    state.counters["errors"] = 0;
    state.counters["ratio"] = state.range(0);
    auto x = [&](MeanCalculation<double> & m) -> double {
        addValues(m, err, state);
        double v = 0; 
        return m.by_ref(v) == E_OK ? v : 0.;
    };

    for (auto _ : state)
    {
        MeanCalculation<double> m;
        OPTIMIZE(x(m));
    }
}

static void BM_do_with_tuple(benchmark::State &state)
{
    MeanCalculation<double> m;
    ErrorInjector err(state.range(0));
    state.counters["errors"] = 0;
    state.counters["ratio"] = state.range(0);
    auto x = [&](MeanCalculation<double> & m) -> double {
        addValues(m, err, state);
        auto v = m.with_tuple(); 
        return std::get<bool>(v) ? std::get<double>(v) : 0.;
    };

    for (auto _ : state)
    {
        MeanCalculation<double> m;
        OPTIMIZE(x(m));
    }
}

static void BM_do_with_expected(benchmark::State &state)
{
    MeanCalculation<double> m;
    ErrorInjector err(state.range(0));
    state.counters["errors"] = 0;
    state.counters["ratio"] = state.range(0);
    auto x = [&](MeanCalculation<double> & m) -> double {
        addValues(m, err, state);
        auto v = m.with_expected(); 
        return v.value_or(0.);
    };

    for (auto _ : state)
    {
        MeanCalculation<double> m;
        OPTIMIZE(x(m));
    }
}
#define ARGS() Arg(2048)->Arg(1024)->Arg(512)->Arg(256)->Arg(128)->Arg(64)->Arg(16)->Arg(8)->Arg(4)->Arg(2)

BENCHMARK(BM_do_with_exception)->ARGS();
BENCHMARK(BM_do_with_optional)->ARGS();
BENCHMARK(BM_do_with_ref)->ARGS();
BENCHMARK(BM_do_with_tuple)->ARGS();
BENCHMARK(BM_do_with_expected)->ARGS();

BENCHMARK_MAIN();
