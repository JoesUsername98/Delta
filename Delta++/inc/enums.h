#pragma once

namespace DPP
{
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
		None = 0,
		Binomial,
		//TODO add more
	};

    enum class Calculation
	{
		None = 0,
		PV,
        Delta,
        Gamma,
        Vega,
        Rho,
		//TODO add more
	};

}