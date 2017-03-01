#include "IMbc.h"
#include "MMU.h"

IMbc::IMbc(MMU &mmu) :
	mmu_{ &mmu }
{
	mmu_listener_deregister_functions_.emplace_back(mmu_->AddListener([this](Memory::Address address, uint8_t value) { this->OnRomWritten(address, value); }, Memory::Region::ROM_Bank0));
	mmu_listener_deregister_functions_.emplace_back(mmu_->AddListener([this](Memory::Address address, uint8_t value) { this->OnRomWritten(address, value); }, Memory::Region::ROM_OtherBanks));
}

IMbc::~IMbc()
{
	for (auto& deregister_function : mmu_listener_deregister_functions_)
	{
		deregister_function();
	}
}

std::vector<std::vector<uint8_t>> IMbc::GetExternalRamBanks(uint8_t external_ram_size_code)
{
	std::vector<std::vector<uint8_t>> external_ram_banks;

	size_t num_ram_banks{ 0 };
	switch (external_ram_size_code)
	{
	case 0:
		num_ram_banks = 0;
		break;
	case 1:
		// Special case: single RAM bank of 2 kBytes
		external_ram_banks.emplace_back(0x800, uint8_t{ 0 });
		break;
	case 2:
		num_ram_banks = 1;
		break;
	case 3:
		num_ram_banks = 4;
		break;
	case 4:
		num_ram_banks = 16;
		break;
	case 5:
		num_ram_banks = 8;
		break;
	default:
		throw std::invalid_argument{ "Unsupported external RAM size code: " + std::to_string(static_cast<int>(external_ram_size_code)) };
	}

	for (int i = 0; i < num_ram_banks; ++i)
	{
		external_ram_banks.emplace_back(Memory::external_ram_bank_size_, uint8_t{ 0 });
	}

	return external_ram_banks;
}
