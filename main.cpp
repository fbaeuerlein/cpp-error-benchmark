#include <iostream>
#include <time.h>
#define E_FAIL -1
#define E_OK 0

#define SAMPLES 10000000
#include <chrono>
#include <optional>
#include <functional>
#include <ctime>
#include "tl/expected.hpp"
#include "benchmark/benchmark.h"

volatile int max_sum = 100; // volatile prevents loop optimization

double some_task()
{
    double x = 0;
    for (int i = 1; i < max_sum; ++i)
        x += 1. / static_cast<double>(i);

    return x;
}

void error_handler()
{
    (void)some_task();
}

bool errorInjector(int num)
{
    return std::rand() % num == 0;
}

int return_value_by_reference(double &value, std::function<bool()> const & error_injector)
{
    value = some_task();
    return error_injector() ? E_FAIL : E_OK;
}

double return_value_with_exception(std::function<bool()> const & error_injector)
{
    double value = some_task();
    if (error_injector())
        throw std::logic_error("bla");
    return value;
}

std::optional<double> return_value_with_optional(std::function<bool()> const & error_injector)
{
    double value = some_task();
    if (error_injector())
        return std::nullopt;
    return value;
}

tl::expected<double, int> return_value_with_expected(std::function<bool()> const & error_injector)
{
    double value = some_task();
    if (error_injector())
        return tl::unexpected<int>(E_FAIL);
    return value;
}

void do_expected(int x)
{
    auto error_injector = [=](){ return errorInjector(x); };
    auto val = return_value_with_expected(error_injector);
    if (!val)
    {
        error_handler();
    }
}

void do_by_ref(int x)
{
    double value = 0;
    auto error_injector = [=](){ return errorInjector(x); };
    if (return_value_by_reference(value, error_injector) != E_OK)
    {
        error_handler();
    }
}

void do_with_optional(int x)
{
    auto error_injector = [=](){ return errorInjector(x); };
    std::optional<double> value = return_value_with_optional(error_injector);
    if (!value)
    {
        error_handler();
    }
}

void do_with_exception(int x)
{
    auto error_injector = [=](){ return errorInjector(x); };
    try
    {
        double value = return_value_with_exception(error_injector);
    }
    catch (const std::exception &e)
    {
        error_handler();
    }
}

static void BM_do_by_ref(benchmark::State &state)
{
    for (auto _ : state)
    {
        do_by_ref(state.range(0));
    }
}

static void BM_do_expected(benchmark::State &state)
{
    for (auto _ : state)
    {
        do_expected(state.range(0));
    }
}

static void BM_do_with_optional(benchmark::State &state)
{
    for (auto _ : state)
    {
        do_with_optional(state.range(0));
    }
}

static void BM_do_with_exception(benchmark::State &state)
{
    for (auto _ : state)
    {
        do_with_exception(state.range(0));
    }
}

BENCHMARK(BM_do_with_exception)->Arg(500);
BENCHMARK(BM_do_with_exception)->Arg(100);
BENCHMARK(BM_do_with_exception)->Arg(10);
BENCHMARK(BM_do_with_exception)->Arg(5);
BENCHMARK(BM_do_with_exception)->Arg(2);


BENCHMARK(BM_do_with_optional)->Arg(500);
BENCHMARK(BM_do_with_optional)->Arg(100);
BENCHMARK(BM_do_with_optional)->Arg(10);
BENCHMARK(BM_do_with_optional)->Arg(5);
BENCHMARK(BM_do_with_optional)->Arg(2);

BENCHMARK(BM_do_expected)->Arg(500);
BENCHMARK(BM_do_expected)->Arg(100);
BENCHMARK(BM_do_expected)->Arg(10);
BENCHMARK(BM_do_expected)->Arg(5);
BENCHMARK(BM_do_expected)->Arg(2);

BENCHMARK(BM_do_by_ref)->Arg(500);
BENCHMARK(BM_do_by_ref)->Arg(100);
BENCHMARK(BM_do_by_ref)->Arg(10);
BENCHMARK(BM_do_by_ref)->Arg(5);
BENCHMARK(BM_do_by_ref)->Arg(2);

BENCHMARK_MAIN();
