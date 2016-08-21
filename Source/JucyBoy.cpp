#include "JucyBoy.h"
#include <sstream>

JucyBoy::JucyBoy() :
	cpu_{ mmu_ },
	is_running_{ false },
	exit_loop_{ false }
{
	setSize (600, 400);
	setWantsKeyboardFocus(true);
	addAndMakeVisible(cpu_status_component_);

	cpu_status_component_.SetCpuState(cpu_);
}

JucyBoy::~JucyBoy()
{
	Stop();
}

void JucyBoy::Reset()
{
	Stop();

	cpu_.Reset();
	mmu_.Reset();
}

void JucyBoy::LoadRom(const juce::File &file)
{
	Stop();

	// Convert the juce file to std string
	mmu_.LoadRom(file.getFullPathName().toStdString());

	Run();
}

void JucyBoy::Run()
{
	if (loop_function_thread_.joinable()) { return; }

	exit_loop_.store(false);
	loop_function_thread_ = std::thread{ &JucyBoy::RunningLoopFunction, this };
	is_running_ = true;
}

void JucyBoy::Stop()
{
	if (!loop_function_thread_.joinable()) { return; }

	exit_loop_.store(true);
	loop_function_thread_.join();
	is_running_ = false;
}

bool JucyBoy::IsRunning() const
{
	return is_running_;
}

void JucyBoy::RunningLoopFunction()
{
	while (!exit_loop_.load())
	{
		ExecuteNextInstruction();
	}
}

void JucyBoy::ExecuteNextInstruction()
{
	auto cycles = cpu_.ExecuteInstruction(cpu_.FetchOpcode());
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
		if (is_running_)
		{
			Stop();
			cpu_status_component_.SetCpuState(cpu_);
		}
		else
		{
			Run();
		}
	}
	else if (key.getKeyCode() == KeyPress::rightKey)
	{
		if (!is_running_)
		{
			try {
				ExecuteNextInstruction();
				cpu_status_component_.SetCpuState(cpu_);
			}
			catch (std::exception &e)
			{
				AlertWindow::showMessageBox(AlertWindow::WarningIcon, "Exception caught: ", e.what());
			}
		}
	}

	return true;
}
