#include "JucyBoy.h"
#include <sstream>

JucyBoy::JucyBoy()
{
	setSize (320, 240);
	setWantsKeyboardFocus(true);
	addAndMakeVisible(cpu_status_component_);

	// Add listeners
	cpu_.AddListener(cpu_status_component_);
	cpu_.Reset();
}

JucyBoy::~JucyBoy()
{

}

void JucyBoy::Reset()
{
	// Stop execution, then reset all components
	cpu_.Stop();
	cpu_.Reset();
	mmu_.Reset();
	cpu_.Run();
}

void JucyBoy::LoadRom(const juce::File &file)
{
	cpu_.Stop();
	cpu_.Reset();
	mmu_.Reset();

	// Convert the juce file to std string
	mmu_.LoadRom(file.getFullPathName().toStdString());

	cpu_.Run();
}

void JucyBoy::paint (Graphics& g)
{
	g.fillAll(Colours::white);

	g.setColour(Colours::lightblue);
	g.setFont(14.0f);
	std::stringstream usage_instructions;
	usage_instructions << "Space: run / stop" << std::endl;
	usage_instructions << "Right: step over" << std::endl;
	g.drawMultiLineText(usage_instructions.str(), 0, 14, getWidth());

	cpu_status_component_.repaint();
}

void JucyBoy::resized()
{
    // This is called when the MainContentComponent is resized.
    // If you add any child components, this is where you should
    // update their positions.
	cpu_status_component_.setBounds(getLocalBounds().withTop(getHeight() / 2));
}

void JucyBoy::mouseDown( const MouseEvent &event)
{
 	if (!event.mods.isRightButtonDown()) { return; }

	PopupMenu m;
	m.addItem(1, "Load ROM");
	const int result = m.show();
	
	switch (result)
	{
	case 0:
		// Did not select anything
		break;
	case 1:
		{FileChooser rom_chooser{ "Select a ROM file to load...", File::getSpecialLocation(File::currentExecutableFile), ".gb" };
		if (rom_chooser.browseForFileToOpen()) {
			auto rom_file = rom_chooser.getResult();
			try
			{
				LoadRom(rom_file);
			}
			catch (std::exception &e)
			{
				AlertWindow::showMessageBox(AlertWindow::AlertIconType::WarningIcon, "Failed to open ROM file", String{ "Error: " } +e.what());
			}
		}}
		break;
	}
}

bool JucyBoy::keyPressed(const KeyPress &key)
{
	// Switch statement does not work below because the keys are not compile time constants...
	if (key.getKeyCode() == KeyPress::spaceKey)
	{
		if (cpu_.IsRunning())
		{
			cpu_.Stop();
		}
		else
		{
			cpu_.Run();
		}
	}
	else if (key.getKeyCode() == KeyPress::rightKey)
	{
		if (!cpu_.IsRunning())
		{
			try {
				cpu_.StepOver();
			}
			catch (std::exception &e)
			{
				AlertWindow::showMessageBox(AlertWindow::WarningIcon, "Exception caught: ", e.what());
			}
		}
	}

	return true;
}
