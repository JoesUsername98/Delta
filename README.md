# Delta++

This is a clone of Delta in C++ using Dear ImGui with OpenGL backend for the front end.

## Prerequisites

- C++23 compatible compiler
- CMake 3.16 or higher
- OpenGL compatible graphics driver

## Getting Started

Once you have cloned the repository, build with CMake:

```bash
# Generate build files
cmake -B build -S .

# Build the project
cmake --build build
```

### Running Tests
```bash
ctest --test-dir build
```

### Code Coverage

To generate the code coverage report:
```bash
cmake -B build -S .
cmake --build build --target coverage
```

### Benchmark Build

For Google Benchmark, use a release build:
```bash
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

## Architecture

The project is structured as follows:

- **Delta++**: Core mathematical library with derivatives pricing models
- **Delta++Math**: Mathematical utilities and algorithms  
- **Delta++UI**: OpenGL-based ImGui interface for interactive pricing
- **walnut_custom**: Custom OpenGL implementation of the Walnut framework

## UI Framework Migration

The project has been migrated from Vulkan to OpenGL for broader compatibility:

- **Custom OpenGL Backend**: Custom implementation in `walnut_custom/` library  
- **CMake Integration**: Full CMake build system with modern dependency management
- **Self-contained Dependencies**: All external dependencies fetched automatically via CMake

## Most Recent Benchmarks

CPU: AMD Ryzen AI 9 365

OPTIONS: --benchmark_min_time=10s

### WSL

| Benchmark                        | Time     | CPU      | Iterations | Allocated | Value     |
|----------------------------------|----------:|---------:|----------:|---------:|----------:|
| BM_BAMP_EuroCall_Steps/8         | 0.001 ms  | 0.001 ms | 20,266,945 | 2 KiB    | 43.221599 |
| BM_BAMP_EuroCall_Steps/16        | 0.002 ms  | 0.002 ms | 7,628,125  | 9 KiB    | 43.755127 |
| BM_BAMP_EuroCall_Steps/32        | 0.006 ms  | 0.006 ms | 2,451,821  | 35 KiB   | 43.989970 |
| BM_BAMP_EuroCall_Steps/64        | 0.019 ms  | 0.020 ms | 703,138    | 134 KiB  | 44.082850 |
| BM_BAMP_EuroCall_Steps/128       | 0.072 ms  | 0.074 ms | 188,749    | 524 KiB  | 44.111583 |
| BM_BAMP_EuroCall_Steps/256       | 0.285 ms  | 0.291 ms | 48,148     | 2 MiB    | 44.113373 |
| BM_BAMP_EuroCall_Steps/512       | 1.11 ms   | 1.13 ms  | 12,314     | 8 MiB    | 44.105370 |
| BM_BAMP_EuroCall_Steps/1024      | 6.25 ms   | 6.43 ms  | 2,208      | 32 MiB   | 44.095078 |
| BM_BAMP_EuroCall_Steps/2048      | 25.1 ms   | 25.6 ms  | 552        | 128 MiB  | 44.085486 |
| BM_BAMP_EuroCall_Steps/4096      | 107 ms    | 110 ms   | 135        | 512 MiB  | 44.089530 |
| BM_BAMP_EuroCall_Steps/8192      | 420 ms    | 431 ms   | 32         | 2 GiB    | 44.088074 |

### Windows 11

| Benchmark                        | Time     | CPU      | Iterations | Allocated | Value     |
|----------------------------------|----------:|---------:|----------:|---------:|----------:|
| BM_BAMP_EuroCall_Steps/8         | 0.001 ms  | 0.001 ms | 15,368,782 | 2 KiB    | 43.221599 |
| BM_BAMP_EuroCall_Steps/16        | 0.002 ms  | 0.002 ms | 7,724,138  | 9 KiB    | 43.755127 |
| BM_BAMP_EuroCall_Steps/32        | 0.005 ms  | 0.005 ms | 2,567,335  | 35 KiB   | 43.989970 |
| BM_BAMP_EuroCall_Steps/64        | 0.018 ms  | 0.018 ms | 779,130    | 134 KiB  | 44.082850 |
| BM_BAMP_EuroCall_Steps/128       | 0.067 ms  | 0.067 ms | 208,858    | 524 KiB  | 44.111583 |
| BM_BAMP_EuroCall_Steps/256       | 0.563 ms  | 0.563 ms | 25,455     | 2 MiB    | 44.113373 |
| BM_BAMP_EuroCall_Steps/512       | 2.18 ms   | 2.17 ms  | 6,446      | 8 MiB    | 44.105370 |
| BM_BAMP_EuroCall_Steps/1024      | 9.41 ms   | 9.38 ms  | 1,491      | 32 MiB   | 44.095078 |
| BM_BAMP_EuroCall_Steps/2048      | 39.4 ms   | 39.3 ms  | 358        | 128 MiB  | 44.085486 |
| BM_BAMP_EuroCall_Steps/4096      | 187 ms    | 187 ms   | 85         | 512 MiB  | 44.089530 |
| BM_BAMP_EuroCall_Steps/8192      | 671 ms    | 670 ms   | 20         | 2 GiB    | 44.088074 |

# Original Delta Project History

This started off as a working example of Shreve Stochastic Calculus I. After about 2 years, I revisited the project and implemented some new features that I understood after reading Implementing QuantLib, Paul Willmott 1&2 and flipping through Hull. The initial aim was to aid in basic learning of the BAPM. I later learned of DerivaGen's existence... thanks Hull.

## Original .NET Version

The original version was a WPF application that worked straight out of the box being a .NET program apart from needing the Extended.Wpf.Toolkit by Xceed for a UI element. 

## Developer Commentary

I may end up writing more characters here than in the entire project!
To start, I would like to say that this was designed from inception to be a showcase project with a start and finishing point. I think I am now at version 1.0. I have a minimum viable "product". It has a UI and it does some math. The math was the easy part and the UI was essential to keep your ape brain stimulated. Something this project was never meant to be was perfect.

From inception of this project I had two main themes: to religiously follow TDD and maniacally implement as many design patterns as I had read about in the Gang of Four book. Both had more or less stopped by the project's completion [for worse and better, respectively]. TDD died off because I was being lazy, honestly. To test in a professional way, I would have to revamp almost all my tests that I had written so far. I stopped using design patterns because I realized that they are a tool best used seldomly (apart from strategy, everyone likes the strategy pattern). This is an important part about growing up as someone who engineers software - you learn that the answer is always "it depends", and when it comes to design patterns, the answer is almost always "nah, you ain't gonna need it."

At work I mainly write C++, so I am starting to develop an unhealthy obsession with performance. I started testing the binomial model I had implemented for larger and larger numbers of time steps and oh boy is there something terribly wrong there. I know the path dependent method is O(2^n) but there must be something horribly wrong with the visitors because this thing chugs. Admittedly, I did write this with the "I am learning, let me try this" ethos so I was never too fussed about performance but this irks me now. After reading Implementing QuantLib, I wanted to implement a lower triangular matrix version of this model (at the expense of path dependent results). This ends up having O(n^2) time complexity and that makes a world of difference. You can actually get a millisecond result close to analytical Black-Scholes now!

Again on the performance theme, let's talk about the UI. I enjoyed learning about WPF and the MVVM. It seems like a half decent way to make a professional piece of software. However, I hate it. Maybe this is a result of what I was doing by visualizing the tree, but boy did I struggle to hack those edges [lines] that connect the nodes. They are terrible and they contribute to the majority of the slowness of the UI itself. Also, events suck.

A note: I do actually know good git etiquette. You just don't require it when you are working on a solo project and have stashes that you can use instead of branches.

## Future Work

- Asian and Lookback options seem to be exclusively an option for the BAPM.
- Adding dividends.
- Fixing that bloody LayoutUpdated event that slows the UI down to a standstill!
- I would like to work more with rates and investigate a bootstrapping tool and maybe something to construct a volatility surface (once I have finished reading Willmott 3). This will likely be a new library and bastardize the UI further.

## Authors

* **Joe Osborne** [GitHub](https://github.com/JoesUsername98)

## License

This project is licensed under the MIT License - see the [LICENSE.md](LICENSE.md) file for details
Good luck making money off this demo project, have at it!

## Acknowledgments

* Thanks to Jack O'Mahoney for giving me my first shot as a quant with no experience.
* Thanks to Julien Hok, Xiaoqing Zhang for taking time to walk me through the maths and UI issues I was having, respectively.
* Thanks to all my other colleagues and friends in the Quant space for motivating me to love what I do and learn more everyday.
