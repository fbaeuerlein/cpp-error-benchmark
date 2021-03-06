# Errorhandling benchmark <!-- omit in toc -->

- [Overview](#overview)
- [Scenario](#scenario)
- [Compared approaches](#compared-approaches)
  - [Return value by reference and error by return](#return-value-by-reference-and-error-by-return)
  - [Return value and exception](#return-value-and-exception)
  - [Returning std::tuple<bool, T>](#returning-stdtuplebool-t)
  - [Returning std::optional<T>](#returning-stdoptionalt)
  - [Returning expected<T, E>](#returning-expectedt-e)
- [Run benchmark](#run-benchmark)
  - [Building and running benchmark](#building-and-running-benchmark)
  - [Running with quick-bench.com](#running-with-quick-benchcom)
- [Result](#result)
  - [Optimized execution](#optimized-execution)
  - [Not optimized execution](#not-optimized-execution)
  - [Summary](#summary)

# Overview

There are different styles of error handling/propagation possible within C++, ranging from the ancient C-style that returns an error code up to exceptions and other more modern approaches. This little project should show some of these possibilities of error handling and compares them based on execution time with the (Google benchmark library](https://github.com/google/benchmark).

# Scenario

A common scenario for error handling/propagation is calling a function that returns a value (e.g. read from a device or some calculation) and that might fail due to an error or invalid usage. So this benchmarking is focused on a simple function that returns a double value from a mean calculation, but the calculation might fail because there are no values for the calculation and thus an error should be signalized.

# Compared approaches

## Return value by reference and error by return

This is something you often see in C-style libraries and legacy code bases. Usually the possible return values (i.e. error codes/indicators) are defined globally (E_OK, E_FAIL, etc.) or the return type might have a special semantic (e.g. [memcmp](https://www.cplusplus.com/reference/cstring/memcmp/)). 
If the operation fails, the value of interest `result` might contain an arbitrary value. So checking the error indicator before using it is necessary.

```
    int by_ref(T & result ) const
    {
        if ( _data.empty() ) 
        {
            return E_FAIL;
        }

        result = mean();
        return E_OK;
    }
```

## Return value and exception

The exception based implementation simply returns the calculated value if no error occured. If there's an error, an exception is thrown.

```
    T with_exception() const
    {
        if ( _data.empty() ) throw std::logic_error("No data for mean calculation");
        return mean();
    }
```

## Returning std::tuple<bool, T>

A [std::tuple](https://en.cppreference.com/w/cpp/utility/tuple) is returned that contains a boolean error state (true if no error, false if error has occured) and the calculated value or contains a default constructed value if an error is present. 

```
    std::tuple<bool, T> with_tuple() const
    {
        if (_data.empty()) return std::make_tuple(false, T{});
        return std::make_tuple(true, mean());
    }
```

## Returning std::optional<T>

The [std::optional](https://en.cppreference.com/w/cpp/utility/optional) implementation is returning an optional which has no value if an error occured. If not, it contains the calculated value.

```
    std::optional<T> with_optional() const
    {
        if ( _data.empty() ) return std::nullopt;
        return { mean() };
    }
```

## Returning expected<T, E>

Returning expected is somehow a generalization of optional, besides the fact that you can specify the type of error that could be returned (not only boolean state as in optional). It was proposed in the [std::expected proposal](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2021/p0323r10.html). This benchmark uses an implementation from [Sy Brand](https://github.com/TartanLlama/expected).

```
    tl::expected<T, int> with_expected() const
    {
        if (_data.empty()) return tl::unexpected<int>(E_FAIL);
        return mean();
    }
```

# Run benchmark

## Building and running benchmark

    mkdir build && cd build
    cmake .. && make -j4

    ./bench

To export a CSV with multiple runs that's readable by the jupyter notebook, use:

    bench --benchmark_repetitions=25 --benchmark_report_aggregates_only=true --benchmark_format=csv > data.csv

## Running with quick-bench.com

If you want to run the benchmark on [quick-bench.com](quick-bench.com), simply define `RUN_ON_QUICKBENCH` within the source and copy the code into the browser window.

# Result

There's the posibility of calling the function that should be benchmarked with `benchmark::DoNotOptimize(expr)` to avoid optimization. The following sections show the difference of execution with optimization and without.

## Optimized execution

![Median execution time](images/graph_optimized.png)

## Not optimized execution

![Median execution time](images/graph_not_optimized.png)

## Summary


