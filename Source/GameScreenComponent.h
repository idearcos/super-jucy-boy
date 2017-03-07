#pragma once

#include <cstdint>
#include <array>
#include "OpenGl2DComponent.h"
#include "JucyBoy/PPU.h"

class GameScreenComponent final : public OpenGl2DComponent, public PPU::Listener
{
public:
	GameScreenComponent();
	~GameScreenComponent();

	void render() override;

	void paint(Graphics&) override {}
	void resized() override {}

	void OnNewFrame(const PPU::Framebuffer &ppu_framebuffer) override;

private:
	uint8_t PpuColorToIntensity(PPU::Color color);

private:
	std::array<uint8_t, 160 * 144 /** 3*/> framebuffer_;
	std::mutex framebuffer_mutex_;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GameScreenComponent)
};
