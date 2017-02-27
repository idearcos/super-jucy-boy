#include "APU.h"
#include "MMU.h"

APU::APU(MMU &mmu) :
	mmu_{ &mmu }
{

}

void APU::OnMachineCycleLapse()
{
	if (apu_enabled_)
	{
		channel_1_.OnMachineCycleLapse();
		channel_2_.OnMachineCycleLapse();

		frame_sequencer_divider_.OnInputClockCyclesLapsed(4);
	}

	size_t right_sample{ 0 };
	right_sample += (right_channels_enabled_ & 0x01) * channel_1_.GetSample();
	right_sample += ((right_channels_enabled_ >> 1) & 0x01) * channel_2_.GetSample();
	right_sample *= (right_volume_ + 1);

	size_t left_sample{ 0 };
	left_sample += (left_channels_enabled_ & 0x01) * channel_1_.GetSample();
	left_sample += ((left_channels_enabled_ >> 1) & 0x01) * channel_2_.GetSample();
	left_sample *= (left_volume_ + 1);

	// Notify listeners
	NotifyNewSample(right_sample, left_sample);
}

void APU::OnFrameSequencerClocked()
{
	switch (frame_sequencer_step_)
	{
	case 0:
		ClockLengthCounters();
		break;
	case 1:
		break;
	case 2:
		channel_1_.ClockFrequencySweep();
		ClockLengthCounters();
		break;
	case 3:
		break;
	case 4:
		ClockLengthCounters();
		break;
	case 5:
		break;
	case 6:
		channel_1_.ClockFrequencySweep();
		ClockLengthCounters();
		break;
	case 7:
		channel_1_.ClockVolumeEnvelope();
		channel_2_.ClockVolumeEnvelope();
		break;
	default:
		throw std::logic_error{ "Invalid frame sequencer step: " + std::to_string(frame_sequencer_step_) };
	}

	frame_sequencer_step_ = (frame_sequencer_step_ + 1) & 0x07;
}

void APU::OnIoMemoryWritten(Memory::Address address, uint8_t value)
{
	if (!apu_enabled_ && (address < Memory::NR52 || address > 0xFF3F)) return;

	switch (address)
	{
	case Memory::NR10:
		channel_1_.OnNR10Written(value);
		mmu_->WriteByte(Memory::NR10, 0x80 | mmu_->ReadByte(Memory::NR10), false);
		break;
	case Memory::NR11:
		channel_1_.OnNRx1Written(value);
		mmu_->WriteByte(Memory::NR11, 0x3F | mmu_->ReadByte(Memory::NR11), false);
		break;
	case Memory::NR12:
		channel_1_.OnNRx2Written(value);
		break;
	case Memory::NR13:
		channel_1_.OnNRx3Written(value);
		mmu_->WriteByte(Memory::NR13, 0xFF | mmu_->ReadByte(Memory::NR13), false);
		break;
	case Memory::NR14:
		channel_1_.OnNRx4Written(value);
		mmu_->WriteByte(Memory::NR14, 0xBF | mmu_->ReadByte(Memory::NR14), false);
		if (!channel_1_.IsChannelOn()) { mmu_->SetBit<0>(Memory::NR52, false); }
		break;
	case Memory::NR21:
		channel_2_.OnNRx1Written(value);
		mmu_->WriteByte(Memory::NR21, 0x3F | mmu_->ReadByte(Memory::NR21), false);
		break;
	case Memory::NR22:
		channel_2_.OnNRx2Written(value);
		break;
	case Memory::NR23:
		channel_2_.OnNRx3Written(value);
		mmu_->WriteByte(Memory::NR23, 0xFF | mmu_->ReadByte(Memory::NR23), false);
		break;
	case Memory::NR24:
		channel_2_.OnNRx4Written(value);
		mmu_->WriteByte(Memory::NR24, 0xBF | mmu_->ReadByte(Memory::NR24), false);
		if (!channel_2_.IsChannelOn()) { mmu_->SetBit<1>(Memory::NR52, false); }
		break;
	case Memory::NR30:
		//mmu_->WriteByte(Memory::NR30, 0x7F | mmu_->ReadByte(Memory::NR30), false);
		break;
	case Memory::NR31:
		//mmu_->WriteByte(Memory::NR31, 0xFF | mmu_->ReadByte(Memory::NR31), false);
		break;
	case Memory::NR32:
		//mmu_->WriteByte(Memory::NR32, 0x9F | mmu_->ReadByte(Memory::NR32), false);
		break;
	case Memory::NR33:
		//mmu_->WriteByte(Memory::NR33, 0xFF | mmu_->ReadByte(Memory::NR33), false);
		break;
	case Memory::NR34:
		//mmu_->WriteByte(Memory::NR34, 0xBF | mmu_->ReadByte(Memory::NR34), false);
		//if (!channel_3_.IsChannelOn()) { mmu_->SetBit<2>(Memory::NR52, false); }
		break;
	case Memory::NR41:
		//mmu_->WriteByte(Memory::NR41, 0xFF | mmu_->ReadByte(Memory::NR41), false);
		break;
	case Memory::NR42:
		break;
	case Memory::NR43:
		break;
	case Memory::NR44:
		//mmu_->WriteByte(Memory::NR44, 0xBF | mmu_->ReadByte(Memory::NR44), false);
		//if (!channel_4_.IsChannelOn()) { mmu_->SetBit<3>(Memory::NR52, false); }
		break;
	case Memory::NR50:
		right_volume_ = value & 0x07;
		left_volume_ = (value & 0x70) >> 4;
		break;
	case Memory::NR51:
		right_channels_enabled_ = value & 0x0F;
		left_channels_enabled_ = (value & 0xF0) >> 4;
		break;
	case Memory::NR52:
		apu_enabled_ = (value & 0x80) != 0;

		mmu_->WriteByte(Memory::NR52, 0x7F | mmu_->ReadByte(Memory::NR52), false);
		if (!channel_1_.IsChannelOn()) { mmu_->ClearBit<0>(Memory::NR52, false); }
		if (!channel_2_.IsChannelOn()) { mmu_->ClearBit<1>(Memory::NR52, false); }
		//if (!channel_3_.IsChannelOn()) { mmu_->ClearBit<2>(Memory::NR52, false); }
		//if (!channel_4_.IsChannelOn()) { mmu_->ClearBit<3>(Memory::NR52, false); }

		break;
	default:
		break;
	}
}

void APU::ClockLengthCounters()
{
	channel_1_.ClockLengthCounter();
	channel_2_.ClockLengthCounter();

	if (!channel_1_.IsChannelOn()) { mmu_->ClearBit<0>(Memory::NR52, false); }
	if (!channel_2_.IsChannelOn()) { mmu_->ClearBit<1>(Memory::NR52, false); }
}

std::function<void()> APU::AddListener(Listener listener)
{
	auto it = listeners_.emplace(listeners_.begin(), listener);
	return [it, this]() { listeners_.erase(it); };
}

void APU::NotifyNewSample(size_t right_sample, size_t left_sample)
{
	for (auto& listener : listeners_)
	{
		listener(right_sample, left_sample);
	}
}
