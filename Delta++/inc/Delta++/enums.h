#pragma once

namespace DPP
{
	enum class PathSchemeType
	{
		Exact,
		Euler,
		Milstein
	};

    enum class OptionPayoffType
	{
		Call,
		Put,
		//TODO add more
	};

	enum class OptionExerciseType
	{
		European,
		American,
		//TODO add more
	};

    enum class CalculationMethod
	{
		//_NONE = 0,
		Binomial,
		BlackScholes,
		MonteCarlo,
		//TODO add more
	};

    enum class Calculation
	{
		_NONE = 0,
		PV,
        Delta,
        Gamma,
        Vega,
        Rho,
		_SIZE,
		//TODO add more
	};

}