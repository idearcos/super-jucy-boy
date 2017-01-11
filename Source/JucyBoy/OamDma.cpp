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
	case State::Requested:
		current_oam_dma_byte_index_ = 0;
		oam_dma_state_ = State::Active;
		break;
	case State::Active:
		if (current_oam_dma_byte_index_ == 0xA0)
		{
			oam_dma_state_ = State::Inactive;
			mmu_->OamDmaActive(false);
			break;
		}

		mmu_->WriteByte(Memory::oam_start_ + current_oam_dma_byte_index_, mmu_->ReadByte(oam_dma_source_ + current_oam_dma_byte_index_));
		current_oam_dma_byte_index_ += 1;
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
		oam_dma_state_ = State::Requested;
		oam_dma_source_ = value << 8;
		mmu_->OamDmaActive(true);
		break;
	default:
		break;
	}
}
