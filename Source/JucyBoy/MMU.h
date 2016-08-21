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

	void Reset();

	uint8_t Read(Address address) const;
	void Write(Address address, uint8_t value);

	void LoadRom(const std::string &rom_file_path);

private:
	std::array<uint8_t, std::numeric_limits<Address>::max() + 1> memory_;
};
