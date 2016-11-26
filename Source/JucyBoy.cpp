#include "JucyBoy.h"
#include <sstream>

JucyBoy::JucyBoy() :
	cpu_status_component_{ cpu_ }
{
	setSize (160 * 4 + 150, 144 * 4);
	setWantsKeyboardFocus(true);

	cpu_status_component_.addMouseListener(this, true);
	addAndMakeVisible(cpu_status_component_);

	game_screen_component_.addMouseListener(this, true);
	addAndMakeVisible(game_screen_component_);

	// Add listeners
	listener_deregister_functions_.emplace_back(AddListener(cpu_status_component_, &CpuStatusComponent::OnStatusUpdateRequested));

	cpu_.AddListener(gpu_);
	cpu_.AddListener(cpu_status_component_);
	cpu_.AddListener(*this);

	listener_deregister_functions_.emplace_back(mmu_.AddListener(cpu_, &CPU::OnIoMemoryWritten, Memory::Region::IO));
	listener_deregister_functions_.emplace_back(mmu_.AddListener(cpu_, &CPU::OnInterruptsRegisterWritten, Memory::Region::Interrupts));
	listener_deregister_functions_.emplace_back(mmu_.AddListener(gpu_, &GPU::OnVramWritten, Memory::Region::VRAM));
	listener_deregister_functions_.emplace_back(mmu_.AddListener(gpu_, &GPU::OnOamWritten, Memory::Region::OAM));
	listener_deregister_functions_.emplace_back(mmu_.AddListener(gpu_, &GPU::OnIoMemoryWritten, Memory::Region::IO));

	gpu_.AddListener(game_screen_component_);

	NotifyStatusUpdateRequest();
}

JucyBoy::~JucyBoy()
{
	// Remove listeners
	cpu_.RemoveListener(gpu_);
	cpu_.RemoveListener(cpu_status_component_);
	cpu_.RemoveListener(*this);
	gpu_.RemoveListener(game_screen_component_);

	for (auto deregister_function : listener_deregister_functions_)
	{
		deregister_function();
	}
}

void JucyBoy::Reset()
{
	// Stop execution, then reset all components
	cpu_.Stop();
	cpu_.Reset();
	mmu_.Reset();
}

void JucyBoy::LoadRom(const juce::File &file)
{
	Reset();

	// Convert the juce file to std string
	mmu_.LoadRom(file.getFullPathName().toStdString());

	//cpu_.Run();
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
	auto working_area = getLocalBounds();
	game_screen_component_.setBounds(working_area.removeFromLeft(160 * 4).removeFromTop(144 * 4));
	//usage_instructions_area_ = working_area.removeFromTop(getHeight() / 4);
	cpu_status_component_.setBounds(working_area);
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
		{FileChooser rom_chooser{ "Select a ROM file to load...", File::getSpecialLocation(File::currentExecutableFile), "*.gb" };
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
		if (!mmu_.IsRomLoaded()) { return true; }
		if (cpu_.IsRunning())
		{
			cpu_.Stop();
			NotifyStatusUpdateRequest();
		}
		else
		{
			cpu_.Run();
		}
	}
	else if (key.getKeyCode() == KeyPress::rightKey)
	{
		if (!mmu_.IsRomLoaded()) { return true; }
		if (!cpu_.IsRunning())
		{
			try
			{
				cpu_.StepOver();
			}
			catch (std::exception &e)
			{
				AlertWindow::showMessageBox(AlertWindow::WarningIcon, "Exception caught in CPU: ", e.what());
			}
			NotifyStatusUpdateRequest();
		}
	}

	return true;
}

void JucyBoy::OnExceptionInRunningLoop()
{
	// The listener callback is called from within the CPU's running loop.
	// The call has to be forwarded to the message thread in order to join the running loop thread.
	// Moreover, any update to the GUI components (as the listener callback of Reset) can only be done safely in the message thread.
	triggerAsyncUpdate();
}

void JucyBoy::handleAsyncUpdate()
{
	try
	{
		// If an exception was thrown in the running loop, Stop will rethrow it
		cpu_.Stop();
	}
	catch (std::exception &e)
	{
		AlertWindow::showMessageBox(AlertWindow::WarningIcon, "Exception caught in CPU: ", e.what());
	}

	NotifyStatusUpdateRequest();
}

void JucyBoy::NotifyStatusUpdateRequest()
{
	for (auto& listener : listeners_)
	{
		listener();
	}
}
