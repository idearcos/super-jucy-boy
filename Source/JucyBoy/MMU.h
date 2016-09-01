#pragma once

#include <cstdint>
#include <array>
#include <limits>
#include <string>

class MMU
{
public:
	using Address = uint16_t;

	MMU();
	~MMU();

	// Sets certain memory registers' initial state
	void Reset();

	uint8_t ReadByte(Address address) const;
	uint16_t ReadWord(Address address) const;
	void WriteByte(Address address, uint8_t value);
	void WriteWord(Address address, uint16_t value);

	void LoadRom(const std::string &rom_file_path);
	bool IsRomLoaded() const noexcept { return rom_loaded_; }

private:
	std::array<uint8_t, std::numeric_limits<Address>::max() + 1> memory_{}; // Value-initialize to all-zeroes

	bool rom_loaded_{ false };
};
