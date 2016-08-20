#pragma once

#include <cstdint>

class RegisterPair final
{
public:
	RegisterPair() = default;
	~RegisterPair() = default;

	uint8_t ReadLowByte() const { return low_; }
	uint8_t ReadHighByte() const { return high_; }
	void WriteLowByte(uint8_t value) { low_ = value; }
	void WriteHighByte(uint8_t value) { high_ = value; }

	uint16_t ReadWord() const { return (high_ << 8) + low_; }
	void WriteWord(uint16_t value) { high_ = static_cast<uint8_t>((value >> 8) & 0xFF); low_ = static_cast<uint8_t>(value & 0xFF); }

private:
	uint8_t low_{ 0 };
	uint8_t high_{ 0 };
};
