#include "../JuceLibraryCode/JuceHeader.h"
#include "CpuStatusComponent.h"
#include <sstream>
#include <iomanip>
#include "JucyBoy/CPU.h"

//==============================================================================
CpuStatusComponent::CpuStatusComponent()
{

}

CpuStatusComponent::~CpuStatusComponent()
{

}

void CpuStatusComponent::paint (Graphics& g)
{
    g.fillAll (Colours::white);   // clear the background

    g.setColour (Colours::grey);
    g.drawRect (getLocalBounds(), 1);   // draw an outline around the component

    g.setColour (Colours::lightblue);
    g.setFont (14.0f);
	std::stringstream registers_state;
	registers_state << "CPU registers:" << std::endl;
	registers_state << "AF: 0x" << std::uppercase << std::setfill('0') << std::setw(2) << std::hex
		<< static_cast<size_t>(a_) << static_cast<size_t>(f_) << std::endl;
	registers_state << "BC: 0x" << std::uppercase << std::setfill('0') << std::setw(2) << std::hex
		<< static_cast<size_t>(b_) << static_cast<size_t>(c_) << std::endl;
	registers_state << "DE: 0x" << std::uppercase << std::setfill('0') << std::setw(2) << std::hex
		<< static_cast<size_t>(d_) << static_cast<size_t>(e_) << std::endl;
	registers_state << "HL: 0x" << std::uppercase << std::setfill('0') << std::setw(2) << std::hex 
		<< static_cast<size_t>(h_) << static_cast<size_t>(l_) << std::endl;
	registers_state << "PC: 0x" << std::uppercase << std::setfill('0') << std::setw(4) << std::hex << pc_ << std::endl;
	registers_state << "SP: 0x" << std::uppercase << std::setfill('0') << std::setw(4) << std::hex << sp_ << std::endl;
    g.drawMultiLineText (registers_state.str(), getLocalBounds().getX(), getLocalBounds().getY() + 14, getLocalBounds().getWidth());
}

void CpuStatusComponent::resized()
{
    // This method is where you should set the bounds of any child
    // components that your component contains..

}

void CpuStatusComponent::SetCpuState(const CPU &cpu)
{
	const auto &registers = cpu.GetRegisters();
	a_ = registers.af_.ReadHighByte();
	f_ = registers.af_.ReadLowByte();
	b_ = registers.bc_.ReadHighByte();
	c_ = registers.bc_.ReadLowByte();
	d_ = registers.de_.ReadHighByte();
	e_ = registers.de_.ReadLowByte();
	h_ = registers.hl_.ReadHighByte();
	l_ = registers.hl_.ReadLowByte();
	pc_ = registers.pc_;
	sp_ = registers.sp_;
}
