#include "APU.h"
#include <string>

void APU::OnMachineCycleLapse()
{
	for (int ii = 0; ii < 2; ++ii)
	{
		if (apu_enabled_)
		{
			channel_1_.OnClockCyclesLapsed(2);
			channel_2_.OnClockCyclesLapsed(2);
			channel_3_.OnClockCyclesLapsed(2);
			channel_4_.OnClockCyclesLapsed(2);

			frame_sequencer_divider_.OnInputClockCyclesLapsed(2);
		}

		SampleBatch sample_batch;
		const auto channel_1_sample = channel_1_.GetSample();
		const auto channel_2_sample = channel_2_.GetSample();
		const auto channel_3_sample = channel_3_.GetSample();
		const auto channel_4_sample = channel_4_.GetSample();

		for (int output_index = 0; output_index < num_outputs_; ++output_index)
		{
			sample_batch[output_index][0] = ((channels_enabled_[output_index] & 0x01) != 0) * channel_1_sample * (master_volumes_[output_index] + 1);
			sample_batch[output_index][1] = ((channels_enabled_[output_index] & 0x02) != 0) * channel_2_sample * (master_volumes_[output_index] + 1);
			sample_batch[output_index][2] = ((channels_enabled_[output_index] & 0x04) != 0) * channel_3_sample * (master_volumes_[output_index] + 1);
			sample_batch[output_index][3] = ((channels_enabled_[output_index] & 0x08) != 0) * channel_4_sample * (master_volumes_[output_index] + 1);
		}

		// Notify listeners
		NotifyNewSample(sample_batch);
	}
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
		channel_4_.ClockVolumeEnvelope();
		break;
	default:
		throw std::logic_error{ "Invalid frame sequencer step: " + std::to_string(frame_sequencer_step_) };
	}

	frame_sequencer_step_ = (frame_sequencer_step_ + 1) & 0x07;
}

// MMU mapped memory read/write functions
uint8_t APU::OnIoMemoryRead(Memory::Address address) const
{
	if (!apu_enabled_ && (address < Memory::NR52)) return 0x00;

	switch (address)
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
		return channel_3_.ReadNR30();
	case Memory::NR31:
		return channel_3_.ReadNR31();
	case Memory::NR32:
		return channel_3_.ReadNR32();
	case Memory::NR33:
		return channel_3_.ReadNR33();
	case Memory::NR34:
		return channel_3_.ReadNR34();
	case 0xFF1F:
		return 0xFF;
	case Memory::NR41:
		return channel_4_.ReadNR41();
	case Memory::NR42:
		return channel_4_.ReadNR42();
	case Memory::NR43:
		return channel_4_.ReadNR43();
	case Memory::NR44:
		return channel_4_.ReadNR44();
	case Memory::NR50:
		return static_cast<uint8_t>(master_volumes_[0]) | static_cast<uint8_t>(master_volumes_[1] << 4);
	case Memory::NR51:
		return (channels_enabled_[0]) | (channels_enabled_[1] << 4);
	case Memory::NR52:
		return 0x70 | static_cast<uint8_t>(channel_1_.IsChannelOn()) | static_cast<uint8_t>((channel_2_.IsChannelOn()) << 1)
			| (static_cast<uint8_t>(channel_3_.IsChannelOn()) << 2) | (static_cast<uint8_t>(channel_4_.IsChannelOn()) << 3) | (apu_enabled_ << 7);
	case 0xFF27:
	case 0xFF28:
	case 0xFF29:
	case 0xFF2A:
	case 0xFF2B:
	case 0xFF2C:
	case 0xFF2D:
	case 0xFF2E:
	case 0xFF2F:
		return 0xFF;
	case Memory::WaveStart:
	case 0xFF31:
	case 0xFF32:
	case 0xFF33:
	case 0xFF34:
	case 0xFF35:
	case 0xFF36:
	case 0xFF37:
	case 0xFF38:
	case 0xFF39:
	case 0xFF3A:
	case 0xFF3B:
	case 0xFF3C:
	case 0xFF3D:
	case 0xFF3E:
	case Memory::WaveEnd:
		return (channel_3_.GetWaveTableSample(2 * (address - Memory::WaveStart)) << 4) | (channel_3_.GetWaveTableSample(2 * (address - Memory::WaveStart) + 1) & 0x0F);
	default:
		throw std::invalid_argument{ "Reading from invalid memory address in APU: " + std::to_string(address) };
	}
}

void APU::OnIoMemoryWritten(Memory::Address address, uint8_t value)
{
	if (!apu_enabled_ && (address < Memory::NR52)) return;

	switch (address)
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
		channel_3_.OnNR30Written(value);
		break;
	case Memory::NR31:
		channel_3_.OnNR31Written(value);
		break;
	case Memory::NR32:
		channel_3_.OnNR32Written(value);
		break;
	case Memory::NR33:
		channel_3_.OnNR33Written(value);
		break;
	case Memory::NR34:
		channel_3_.OnNR34Written(value);
		break;
	case 0xFF1F:
		break;
	case Memory::NR41:
		channel_4_.OnNR41Written(value);
		break;
	case Memory::NR42:
		channel_4_.OnNR42Written(value);
		break;
	case Memory::NR43:
		channel_4_.OnNR43Written(value);
		break;
	case Memory::NR44:
		channel_4_.OnNR44Written(value);
		break;
	case Memory::NR50:
		master_volumes_[Outputs::Right] = value & 0x07;
		master_volumes_[Outputs::Left] = (value & 0x70) >> 4;
		break;
	case Memory::NR51:
		channels_enabled_[Outputs::Right] = value & 0x0F;
		channels_enabled_[Outputs::Left] = (value & 0xF0) >> 4;
		break;
	case Memory::NR52:
		{const auto was_apu_enabled = apu_enabled_;
		apu_enabled_ = (value & 0x80) != 0;
		const auto did_toggle = apu_enabled_ != was_apu_enabled;

		//TODO: what else is initialized?
		if (did_toggle)
		{
			if (apu_enabled_)
			{
				frame_sequencer_divider_.Reset();
			}
			else
			{
				channel_1_.Disable();
				channel_2_.Disable();
				channel_3_.Disable();
				channel_4_.Disable();
				/*for (auto address = Memory::NR10; address <= Memory::NR52; ++address)
				{
					mmu_->WriteByte(address, 0x00, false);
				}*/
			}
		}}
		break;
	case 0xFF27:
	case 0xFF28:
	case 0xFF29:
	case 0xFF2A:
	case 0xFF2B:
	case 0xFF2C:
	case 0xFF2D:
	case 0xFF2E:
	case 0xFF2F:
		break;
	case Memory::WaveStart:
	case 0xFF31:
	case 0xFF32:
	case 0xFF33:
	case 0xFF34:
	case 0xFF35:
	case 0xFF36:
	case 0xFF37:
	case 0xFF38:
	case 0xFF39:
	case 0xFF3A:
	case 0xFF3B:
	case 0xFF3C:
	case 0xFF3D:
	case 0xFF3E:
	case Memory::WaveEnd:
		channel_3_.SetWaveTableSample(2 * (address - Memory::WaveStart), value >> 4);
		channel_3_.SetWaveTableSample(2 * (address - Memory::WaveStart) + 1, value & 0x0F);
		break;
	default:
		throw std::invalid_argument{ "Writing to invalid memory address in APU: " + std::to_string(address) };
	}
}

void APU::ClockLengthCounters()
{
	channel_1_.ClockLengthCounter();
	channel_2_.ClockLengthCounter();
	channel_3_.ClockLengthCounter();
	channel_4_.ClockLengthCounter();
}

std::function<void()> APU::AddListener(Listener listener)
{
	auto it = listeners_.emplace(listeners_.begin(), listener);
	return [it, this]() { listeners_.erase(it); };
}

void APU::NotifyNewSample(SampleBatch &sample_batch)
{
	for (auto& listener : listeners_)
	{
		listener(sample_batch);
	}
}
