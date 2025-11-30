#include <benchmark/benchmark.h>

#include <Delta++/engine_factory.h>
#include <Delta++/tri_matrix_builder.h>

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
static void BM_BAMP_EuroCall_Steps(benchmark::State& state) {
	auto stepsIn = state.range(0);

	int64_t bytes = 0;
	double value = 0.;
	for (auto s : state) 
	{
		auto buildResult = TriMatrixBuilder::create(stepsIn, 1. / stepsIn)
			.withUnderlyingValueAndVolatility(100., 1.2)
			.withInterestRate(0.01)
			.withPayoff(OptionPayoffType::Call, 105.)
			.withRiskNuetralProb()
			.withPremium(OptionExerciseType::European);

		const auto mat = buildResult.build();
		const auto& vec = mat.getMatrix();

		bytes = sizeof( decltype(vec) ) + (sizeof(vec[0]) * vec.size() );
		value = vec[0].m_data.m_optionValue;
	}
	std::string byteStr =
		bytes < 1024 ? std::to_string(bytes) + "b" :
		bytes < 1024 * 1024 ? std::to_string(bytes / 1024) + "Kib" :
		bytes < 1024 * 1024 * 1024 ? std::to_string(bytes / (1024 * 1024)) + "Mib" :
		std::to_string(bytes / (1024 * 1024 * 1024)) + "Gib";

	state.SetLabel("bytes: " + byteStr + "\t val: "+ std::to_string(value));
}
 BENCHMARK(BM_BAMP_EuroCall_Steps)->Unit( benchmark::TimeUnit::kMillisecond )->RangeMultiplier(2)->Range(8, 1024 * 8);

static void BM_MC_AmerPut_TimeSteps(benchmark::State& state) {
	auto stepsIn = state.range(0);

	int64_t bytes = 0;
	double value = 0.;
	for (auto s : state)
	{
		auto buildResult = TriMatrixBuilder::create(stepsIn, 1. / stepsIn)
			.withUnderlyingValueAndVolatility(100., 1.2)
			.withInterestRate(0.01)
			.withPayoff(OptionPayoffType::Call, 105.)
			.withRiskNuetralProb()
			.withPremium(OptionExerciseType::European);

		TradeData trd(OptionExerciseType::American, OptionPayoffType::Put, 100., 1.);
		MarketData mkt(0.2, 100., 0.05);
		CalcData calc(Calculation::PV, stepsIn, 1000);

		auto engine_res = EngineFactory::getEngine<MonteCarloEngine>(mkt, trd, calc);
		auto& engine_mc = engine_res.value();
		engine_mc->run();

		const auto mat = buildResult.build();
		const auto& vec = mat.getMatrix();

		value = engine_mc->m_results.at(Calculation::PV);
	}
	state.SetLabel("val: " + std::to_string(value));
}
BENCHMARK(BM_MC_AmerPut_TimeSteps)->Unit(benchmark::TimeUnit::kMillisecond)->RangeMultiplier(2)->Range(8, 1024 * 8);

BENCHMARK_MAIN();