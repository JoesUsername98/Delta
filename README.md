# Delta++

This is a clone of Delta in C++ using Dear ImGUI for the (more basic) front end.

## Prequisites

Vulkan

## Getting Started

Once you have installed Vulkan and cloned the repository run the following commands

```bash
$ mkdir build
```
```bash
$ cd build
```
```bash
$ cmake .. -DCMAKE_POLICY_VERSION_MINIMUM="3.5"
```
```bash
$ cmake --build . 
```

To generate the code coverage tool replace step 4 with
```bash
cmake --build . [ --target coverage ]
```

To configure and build for Google Benchmark replace step 3 with a release build
```bash
$ cmake .. -DCMAKE_POLICY_VERSION_MINIMUM="3.5" -DCMAKE_BUILD_TYPE=Release
```

## Most Recent Benchmarks

CPU: AMD Ryzen AI 9 365  
OPTIONS: --benchmark_min_time=10s  
OS: WSL  

| Benchmark                          | Time     | CPU     | Iterations | Bytes       | Val       |
|------------------------------------|----------|---------|------------|-------------|-----------|
| BM_BAMP_EuroCall_Steps/8           | 0.001 ms | 0.001 ms| 18673861   | 2Kib        | 43.221599 |
| BM_BAMP_EuroCall_Steps/16          | 0.003 ms | 0.003 ms| 5563558    | 9Kib        | 43.755127 |
| BM_BAMP_EuroCall_Steps/32          | 0.011 ms | 0.010 ms| 1319385    | 35Kib       | 43.989970 |
| BM_BAMP_EuroCall_Steps/64          | 0.040 ms | 0.037 ms| 319955     | 134Kib      | 44.082850 |
| BM_BAMP_EuroCall_Steps/128         | 0.126 ms | 0.116 ms| 111112     | 524Kib      | 44.111583 |
| BM_BAMP_EuroCall_Steps/256         | 0.416 ms | 0.384 ms| 32318      | 2Mib        | 44.113373 |
| BM_BAMP_EuroCall_Steps/512         | 1.70 ms  | 1.57 ms | 8068       | 8Mib        | 44.105370 |
| BM_BAMP_EuroCall_Steps/1024        | 9.15 ms  | 8.44 ms | 1487       | 32Mib       | 44.095078 |
| BM_BAMP_EuroCall_Steps/2048        | 42.9 ms  | 39.5 ms | 437        | 128Mib      | 44.085486 |
| BM_BAMP_EuroCall_Steps/4096        | 183 ms   | 168 ms  | 88         | 512Mib      | 44.089530 |
| BM_BAMP_EuroCall_Steps/8192        | 539 ms   | 497 ms  | 28         | 2Gib        | 44.088074 |

CPU: AMD Ryzen AI 9 365  
OPTIONS: --benchmark_min_time=10s  
OS: Windows11  

| Benchmark                          | Time     | CPU     | Iterations | Bytes       | Val       |
|------------------------------------|----------|---------|------------|-------------|-----------|
| BM_BAMP_EuroCall_Steps/8           | 0.003 ms | 0.003 ms| 4186916    | 2Kib        | 43.221599 |
| BM_BAMP_EuroCall_Steps/16          | 0.004 ms | 0.004 ms| 3780591    | 9Kib        | 43.755127 |
| BM_BAMP_EuroCall_Steps/32          | 0.009 ms | 0.009 ms| 1357576    | 35Kib       | 43.989970 |
| BM_BAMP_EuroCall_Steps/64          | 0.032 ms | 0.031 ms| 466667     | 134Kib      | 44.082850 |
| BM_BAMP_EuroCall_Steps/128         | 0.164 ms | 0.160 ms| 106667     | 524Kib      | 44.111583 |
| BM_BAMP_EuroCall_Steps/256         | 1.86 ms  | 1.82 ms | 7467       | 2Mib        | 44.113373 |
| BM_BAMP_EuroCall_Steps/512         | 5.35 ms  | 5.23 ms | 2651       | 8Mib        | 44.105370 |
| BM_BAMP_EuroCall_Steps/1024        | 24.4 ms  | 24.0 ms | 467        | 32Mib       | 44.095078 |
| BM_BAMP_EuroCall_Steps/2048        | 82.7 ms  | 81.6 ms | 240        | 128Mib      | 44.085486 |
| BM_BAMP_EuroCall_Steps/4096        | 297 ms   | 287 ms  | 44         | 512Mib      | 44.089530 |
| BM_BAMP_EuroCall_Steps/8192        | 1284 ms  | 1259 ms | 9          | 2Gib        | 44.088074 |

# Delta

This started off as a working example of Shreve Stochastic Calculus I. After about 2 years, I revisited the project and implemented some new features that I understood after reading Implementing Quant Lib, Paul Willmott 1&2 and flicking through Hull. The initail aim was to aid in basic learning of the BAPM. I later learned of DerivaGens existence... thanks Hull.

## Getting Started

I would like to think this works straight out of the box being a .Net program apart from needing the Extended.Wpf.Toolkit by Xceed for a UI element I liked. K.I.S.S, Just use Nuget for this.

## Running the tests

This is a WPF application so I expect you to use a windows based IDE. To that end, you should have a test explorer there.

## Directors Commentary

I may end up writing more characters here than in the entire of my project!
To start, I would like to say that this was designed from inception to be a showcase project with a start and finishing pont. I think I am now at version 1.0. I have a minimum viable "product". It has a UI and it does some maths. The maths was the easy part and the UI was essential to keep your ape brain stimulated. Something this project was never meant to be was perfect.

From inception of this project I had two main themes, to relegiously follow TDD and menaically implement as many design patterns as I had read about in the gang of four book, both had more or less stopped by the projects completion [for worse and better, respectively]. TDD died off because I was being lazy, honestly. To test in a professional way, I would have to revamp almost all my tests that I had written so far. I stopped using design patterns because I realised that they are a tool best used seldomly ( apart from strategy, everyone likes the strategy pattern .) This is an important part about growing up as someone who engineers software you learn that the answer is always 'it depends', and when it comes to design patterns, the answer is almost always 'nah, you ain't gunna need it.'

At work I mainly write C++, so I am starting to develop an unhealthy obsession with performance. I started testing the binomial model I had implemented for larger and larger numbers of time steps and oh boy is there something terrible wrong there. I know the path dependant method is O(2^n) but there must be something horribly wrong with the visitors because this thing chugs. Admittedly, I did write this with the 'I am learning ethos, let me try this' ethos so I was never too fussed about performance but this irks me now. After reading implementing Quant Lib, I wanted to implement a lower triangular matrix version of this model ( at the expense of path dependant results ) This ends up having O(n^2) time complexity and that mades a world of difference. You can actually get a ms result close to analytical black-scholes now!

Again on the performance theme, lets talk about the UI. I enjoyed learning aboout WPF and the MVVM. It seems like a half decent way to make a professional peice of software. However, I hate it. Maybe this is a result of what I was doing by visualising the tree, but boy did I struggle hack and GPT those edges [lines] in that connect the nodes. They are terrible and they contribute to the majority of the slowness of the UI itself. Also, events suck.

A note, I do actually know good git ettiquete. You just don't require it when you are working on a solo project and have stashes that you can use instead of branches.

## Furtherwork

- Asianing and Lookbacks seem to be exclusively an option for the BAPM.
- Adding dividends.
- Fixing that bloody, LayoutUpdated event that slows the UI down to a stand still!
- I would like to work more with rates and ivestigate a boostrapping tool and maybe something to construct a volatility surface (once I have finished reading Willmott 3.) This will likely be a new library and bastardise the UI further.

## Authors

* **Joe Osborne** [GitHub](https://github.com/JoesUsername98)

## License

This project is licensed under the MIT License - see the [LICENSE.md](LICENSE.md) file for details
Good luck making money off this demo project, have at it!

## Acknowledgments

* Thanks to Jack O'Mahoney for giving me my first shot as a quant with no experience.
* Thanks to Julien Hok, Xiaoqing Zhang for taking time to walk me through the maths and UI issues I was having, respectively.
* Thanks to all my other colleagues and friends in the Quant space for motivating me to love what I do and learn more everyday.
