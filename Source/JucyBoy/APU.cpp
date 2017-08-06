#include "APU.h"
#include <string>

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

// MMU mapped memory read/write functions
uint8_t APU::OnIoMemoryRead(const Memory::Address &address) const
{
	if (!apu_enabled_ && (address < Memory::NR52 || address > 0xFF3F)) return 0x00;

	switch (static_cast<uint16_t>(address))
	{
	case Memory::NR10:
		return channel_1_.ReadNR10();
	case Memory::NR11:
		return channel_1_.ReadNRx1();
	case Memory::NR12:
		return channel_1_.ReadNRx2();
	case Memory::NR13:
		return channel_1_.ReadNRx3();
	case Memory::NR14:
		return channel_1_.ReadNRx4();
	case 0xFF15:
		return 0xFF;
	case Memory::NR21:
		return channel_2_.ReadNRx1();
	case Memory::NR22:
		return channel_2_.ReadNRx2();
	case Memory::NR23:
		return channel_2_.ReadNRx3();
	case Memory::NR24:
		return channel_2_.ReadNRx4();
	case Memory::NR30:
		return 0xFF;
		//return channel_3_.ReadNR30();
	case Memory::NR31:
		return 0xFF;
		//return channel_3_.ReadNR31();
	case Memory::NR32:
		return 0xFF;
		//return channel_3_.ReadNR32();
	case Memory::NR33:
		return 0xFF;
		//return channel_3_.ReadNR33();
	case Memory::NR34:
		return 0xFF;
		//return channel_3_.ReadNR34();
	case 0xFF1F:
		return 0xFF;
	case Memory::NR41:
		return 0xFF;
		//return channel_4_.ReadNR41();
	case Memory::NR42:
		return 0xFF;
		//return channel_4_.ReadNR42();
	case Memory::NR43:
		return 0xFF;
		//return channel_4_.ReadNR43();
	case Memory::NR44:
		return 0xFF;
		//return channel_4_.ReadNR44();
	case Memory::NR50:
		return static_cast<uint8_t>(right_volume_) | static_cast<uint8_t>(left_volume_ << 4);
	case Memory::NR51:
		return (right_channels_enabled_) | (left_channels_enabled_ << 4);
	case Memory::NR52:
		return 0x70 | static_cast<uint8_t>(channel_1_.IsChannelOn()) | static_cast<uint8_t>((channel_2_.IsChannelOn()) << 1)
			| /*(static_cast<uint8_t>(channel_3_.IsChannelOn()) << 2) | (static_cast<uint8_t>(channel_4_.IsChannelOn()) << 3) |*/ (apu_enabled_ << 7);
	default:
		throw std::invalid_argument{ "Reading from invalid memory address in APU: " + static_cast<uint16_t>(address) };
	}
}

void APU::OnIoMemoryWritten(const Memory::Address &address, uint8_t value)
{
	if (!apu_enabled_ && (address < Memory::NR52 || address > 0xFF3F)) return;

	switch (static_cast<uint16_t>(address))
	{
	case Memory::NR10:
		channel_1_.OnNR10Written(value);
		break;
	case Memory::NR11:
		channel_1_.OnNRx1Written(value);
		break;
	case Memory::NR12:
		channel_1_.OnNRx2Written(value);
		break;
	case Memory::NR13:
		channel_1_.OnNRx3Written(value);
		break;
	case Memory::NR14:
		channel_1_.OnNRx4Written(value);
		break;
	case 0xFF15:
		break;
	case Memory::NR21:
		channel_2_.OnNRx1Written(value);
		break;
	case Memory::NR22:
		channel_2_.OnNRx2Written(value);
		break;
	case Memory::NR23:
		channel_2_.OnNRx3Written(value);
		break;
	case Memory::NR24:
		channel_2_.OnNRx4Written(value);
		break;
	case Memory::NR30:
		break;
	case Memory::NR31:
		break;
	case Memory::NR32:
		break;
	case Memory::NR33:
		break;
	case Memory::NR34:
		break;
	case 0xFF1F:
		break;
	case Memory::NR41:
		break;
	case Memory::NR42:
		break;
	case Memory::NR43:
		break;
	case Memory::NR44:
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
		break;
	default:
		throw std::invalid_argument{ "Writing to invalid memory address in APU: " + static_cast<uint16_t>(address) };
	}
}

void APU::ClockLengthCounters()
{
	channel_1_.ClockLengthCounter();
	channel_2_.ClockLengthCounter();
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
