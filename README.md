# Super Jucy Boy
Super Jucy Boy is a Game Boy emulator written in C++.

The main goal of this project is to achieve great emulation accuracy, while offering a rich set of debugging features and user interface.

## Build instructions
0) Clone repository: `https://github.com/idearcos/super-jucy-boy.git`, then `cd super-jucy-boy`
1) Get project dependencies: `git submodule update --init --remote --depth 1`
2) If the project files for your platform are in super-jucy-boy/Builds, you can proceed to building the project as usual. Visual Studio 2017 and XCode project files are currently available. If you are using a different platform, continue to step 3.
3) Super Jucy Boy is managed with a tool called Projucer. `cd Dependencies/JUCE/extras/Projucer/Builds` and build Projucer for your environment.
4) Open SuperJucyBoy.jucer with the Projucer application. Add a new exporter for your target platform in the Exporters menu, and copy the relevant options (mainly Header search paths), from one of the existing exporters.
5) Save the .jucer file by pressing Ctrl + S. This will create the project files for your target platform in the Builds directory.
6) Proceed to build the project as usual.

## Usage
Open the SuperJucyBoy application. Right click in the window to show the popup menu where you can load a ROM file, open the debugging window, and more.

## Built with
- [JUCE](https://www.juce.com/) - Framework for GUI and audio, and project management with Projucer
- [GLEW](http://glew.sourceforge.net/) - OpenGL extension loading
- [cereal](http://uscilab.github.io/cereal/) - A C++11 library for serialization

## License
Super Jucy Boy is licensed under GPLv3.
