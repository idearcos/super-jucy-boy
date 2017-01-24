#include "OamDma.h"
#include "MMU.h"
#include <string>

OamDma::OamDma(MMU &mmu) :
	mmu_{ &mmu }
{

}

void OamDma::OnMachineCycleLapse()
{
	switch (oam_dma_state_)
	{
	case State::Startup:
		current_oam_dma_byte_index_ = 0;
		oam_dma_state_ = State::Active;
		break;
	case State::Active:
		mmu_->OamDmaActive(true);

		//TODO: improve this workaround
		mmu_->OamDmaActive(false);
		mmu_->WriteByte(Memory::oam_start_ + current_oam_dma_byte_index_, mmu_->ReadByte(oam_dma_source_ + current_oam_dma_byte_index_));
		mmu_->OamDmaActive(true);

		if (++current_oam_dma_byte_index_ == 0xA0)
		{
			oam_dma_state_ = State::Teardown;
			break;
		}

		break;
	case State::Teardown:
		mmu_->OamDmaActive(false);
		oam_dma_state_ = State::Inactive;
		break;
	case State::Inactive:
		break;
	default:
		throw std::logic_error{ "Invalid OAM DMA state: " + std::to_string(static_cast<size_t>(oam_dma_state_)) };
		break;
	}
}

void OamDma::OnIoMemoryWritten(Memory::Address address, uint8_t value)
{
	switch (address)
	{
	case Memory::dma_transfer_source_register_: // DMA
		if (value > 0xF1) throw std::invalid_argument("Invalid DMA transfer source: " + std::to_string(int{ value }));

		oam_dma_state_ = State::Startup;
		oam_dma_source_ = value << 8;
		break;
	default:
		break;
	}
}
