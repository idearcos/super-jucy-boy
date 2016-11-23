#pragma once

#include <cstdint>

class RegisterPair final
{
public:
	RegisterPair() = default;
	~RegisterPair() = default;

	const uint8_t& GetLow() const { return low_; }
	uint8_t& GetLow() { return low_; }
	const uint8_t& GetHigh() const { return high_; }
	uint8_t& GetHigh() { return high_; }

	operator uint16_t() const { return (high_ << 8) + low_; }
	RegisterPair& operator=(uint16_t value) { high_ = static_cast<uint8_t>((value >> 8) & 0xFF); low_ = static_cast<uint8_t>(value & 0xFF); return *this; }

	RegisterPair& operator+=(uint16_t value) { high_ = static_cast<uint8_t>(((*this + value) >> 8) & 0xFF); low_ = static_cast<uint8_t>((*this + value) & 0xFF); return *this; }
	RegisterPair& operator-=(uint16_t value) { high_ = static_cast<uint8_t>(((*this - value) >> 8) & 0xFF); low_ = static_cast<uint8_t>((*this - value) & 0xFF); return *this; }

	uint16_t operator++() { auto value = uint16_t(*this); *this = ++value; return uint16_t(*this); }
	uint16_t operator--() { auto value = uint16_t(*this); *this = --value; return uint16_t(*this); }
	uint16_t operator++(int) { const auto original_value = uint16_t(*this); auto value = original_value; *this = ++value; return value; }
	uint16_t operator--(int) { const auto original_value = uint16_t(*this); auto value = original_value; *this = --value; return value; }

private:
	uint8_t low_;
	uint8_t high_;
};
