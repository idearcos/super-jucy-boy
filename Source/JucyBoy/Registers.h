#pragma once

#include <cstdint>

class Register final
{
public:
	Register() = default;
	~Register() = default;

	uint8_t Read() const { return value_; }
	void Write(uint8_t value) { value_ = value; }

	auto Increment() { return ++value_; }
	auto Decrement() { return --value_; }

private:
	uint8_t value_{ 0 };
};

class RegisterPair final
{
public:
	RegisterPair() = default;
	~RegisterPair() = default;

	const Register& GetLow() const { return low_; }
	Register& GetLow() { return low_; }
	const Register& GetHigh() const { return high_; }
	Register& GetHigh() { return high_; }

	uint16_t Read() const { return (high_.Read() << 8) + low_.Read(); }
	void Write(uint16_t value) { high_.Write(static_cast<uint8_t>((value >> 8) & 0xFF)); low_.Write(static_cast<uint8_t>(value & 0xFF)); }

	void Increment() { auto value = Read(); Write(++value); }
	void Decrement() { auto value = Read(); Write(--value); }

private:
	Register low_;
	Register high_;
};
