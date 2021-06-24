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

bool error_injector()
{
    return std::rand() % 10 == 0;
}

int return_value_by_reference(double &value)
{
    value = some_task();
    return error_injector() ? E_FAIL : E_OK;
}

double return_value_with_exception()
{
    double value = some_task();
    if (error_injector())
        throw std::logic_error("bla");
    return value;
}

std::optional<double> return_value_with_optional()
{
    double value = some_task();
    if (error_injector())
        return std::nullopt;
    return value;
}

tl::expected<double, int> return_value_with_expected()
{
    double value = some_task();
    if (error_injector())
        return tl::unexpected<int>(E_FAIL);
    return value;
}

void do_expected()
{
    auto val = return_value_with_expected();
    if (!val)
    {
        error_handler();
    }
}

void do_by_ref()
{
    double value = 0;
    if (return_value_by_reference(value) != E_OK)
    {
        error_handler();
    }
}

void do_with_optional()
{
    std::optional<double> value = return_value_with_optional();
    if (!value)
    {
        error_handler();
    }
}

void do_with_exception()
{
    try
    {
        double value = return_value_with_exception();
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
        do_by_ref();
    }
}

static void BM_do_expected(benchmark::State &state)
{
    for (auto _ : state)
    {
        do_expected();
    }
}

static void BM_do_with_optional(benchmark::State &state)
{
    for (auto _ : state)
    {
        do_with_optional();
    }
}

static void BM_do_with_exception(benchmark::State &state)
{
    for (auto _ : state)
    {
        do_with_exception();
    }
}

BENCHMARK(BM_do_with_exception);
BENCHMARK(BM_do_with_optional);
BENCHMARK(BM_do_expected);
BENCHMARK(BM_do_by_ref);

BENCHMARK_MAIN();
