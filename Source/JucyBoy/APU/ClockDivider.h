#pragma once

#include <cstdint>
#include <functional>

class ClockDivider final
{
public:
	ClockDivider(size_t period, std::function<void()> callback) :
		period_{ period },
		input_clock_cycles_left_{ period },
		callback_{ callback }
	{
	}

	~ClockDivider() = default;

	void OnInputClockCyclesLapsed(size_t num_clock_cycles)
	{
		for (auto remaining_clock_cycles = num_clock_cycles; remaining_clock_cycles > 0; --remaining_clock_cycles)
		{
			if (--input_clock_cycles_left_ == 0)
			{
				input_clock_cycles_left_ = period_;
				callback_();
			}
		}
	}

	void SetPeriod(size_t period) { period_ = period; }

	void Reset() { input_clock_cycles_left_ = period_; }

	template<class Archive>
	void serialize(Archive &archive)
	{
		archive(period_, input_clock_cycles_left_);
	}

private:
	size_t period_;
	size_t input_clock_cycles_left_;

	std::function<void()> callback_;
};
