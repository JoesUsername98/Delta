#include <benchmark/benchmark.h>

#include "engine_factory.h"
using namespace DPP;
using namespace std::string_literals;

//static void BM_StringCreation(benchmark::State& state) {
//  for (auto _ : state)
//    std::string empty_string;
//}
//// Register the function as a benchmark
//BENCHMARK(BM_StringCreation);
//
//// Define another benchmark
//static void BM_StringCopy(benchmark::State& state) {
//  std::string x = "hello";
//  for (auto _ : state)
//    std::string copy(x);
//}
//BENCHMARK(BM_StringCopy);

// Define another benchmark
static void BM_BAMP_EuroCall_1000Steps(benchmark::State& state) {
	auto stepsIn = state.range(0);

	int64_t bytes = 0;
	for (auto s : state) 
	{
		auto buildResult = TriMatrixBuilder::create(stepsIn, 1. / stepsIn)
			.withUnderlyingValueAndVolatility(100., 1.2)
			.withInterestRate(0.01)
			.withPayoff(OptionPayoffType::Call, 105.)
			.withRiskNuetralProb()
			.withPremium(OptionExerciseType::European)
			.withDelta()
			.withPsuedoOptimalStoppingTime();

		const auto mat = buildResult.build();
		const auto& vec = mat.getMatrix();

		bytes = sizeof( decltype(vec) ) + (sizeof(vec[0]) * vec.size() );
	}
	std::string byteStr =
		bytes < 1024 ? std::to_string(bytes) + "b" :
		bytes < 1024 * 1024 ? std::to_string(bytes / 1024) + "Kib" :
		bytes < 1024 * 1024 * 1024 ? std::to_string(bytes / (1024 * 1024)) + "Mib" :
		std::to_string(bytes / (1024 * 1024 * 1024)) + "Gib";

	state.SetLabel("timeSteps: "s + std::to_string(stepsIn) + " bytes: " + byteStr );
}
BENCHMARK(BM_BAMP_EuroCall_1000Steps)->Unit( benchmark::TimeUnit::kMillisecond )->RangeMultiplier(2)->Range(8, 1024 * 8);

BENCHMARK_MAIN();