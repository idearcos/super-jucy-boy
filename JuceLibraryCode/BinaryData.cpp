/* ==================================== JUCER_BINARY_RESOURCE ====================================

   This is an auto-generated file: Any edits you make may be overwritten!

*/

namespace BinaryData
{

//================== README.md ==================
static const unsigned char temp_binary_data_0[] =
"# Super Jucy Boy\r\n"
"Super Jucy Boy is a Game Boy emulator written in C++.\r\n"
"\r\n"
"The main goal of this project is to achieve great emulation accuracy, while offering a rich set of debugging features and user interface.\r\n"
"\r\n"
"## Build instructions\r\n"
"0) Clone repository: `https://github.com/idearcos/super-jucy-boy.git`, then `cd super-jucy-boy`\r\n"
"1) Get project dependencies: `git submodule update --init --remote --depth 1`\r\n"
"2) If the project files for your platform are in super-jucy-boy/Builds, you can proceed to building the project as usual. Visual Studio 2017 and XCode project files are currently available. If you are using a different platform, continue to step 3.\r"
"\n"
"3) Super Jucy Boy is managed with a tool called Projucer. `cd Dependencies/JUCE/extras/Projucer/Builds` and build Projucer for your environment.\r\n"
"4) Open SuperJucyBoy.jucer with the Projucer application. Add a new exporter for your target platform in the Exporters menu, and copy the relevant options (mainly Header search paths), from one of the existing exporters.\r\n"
"5) Save the .jucer file by pressing Ctrl + S. This will create the project files for your target platform in the Builds directory.\r\n"
"6) Proceed to build the project as usual.\r\n"
"\r\n"
"## Usage\r\n"
"Open the SuperJucyBoy application. Right click in the window to show the popup menu where you can load a ROM file, open the debugging window, and more.\r\n"
"\r\n"
"## Built with\r\n"
"- [JUCE](https://www.juce.com/) - Framework for GUI and audio, and project management with Projucer\r\n"
"- [GLEW](http://glew.sourceforge.net/) - OpenGL extension loading\r\n"
"- [cereal](http://uscilab.github.io/cereal/) - A C++11 library for serialization\r\n"
"\r\n"
"## License\r\n"
"Super Jucy Boy is licensed under GPLv3.\r\n";

const char* README_md = (const char*) temp_binary_data_0;


const char* getNamedResource (const char*, int&) throw();
const char* getNamedResource (const char* resourceNameUTF8, int& numBytes) throw()
{
    unsigned int hash = 0;
    if (resourceNameUTF8 != 0)
        while (*resourceNameUTF8 != 0)
            hash = 31 * hash + (unsigned int) *resourceNameUTF8++;

    switch (hash)
    {
        case 0x64791dc8:  numBytes = 1695; return README_md;
        default: break;
    }

    numBytes = 0;
    return 0;
}

const char* namedResourceList[] =
{
    "README_md"
};

}
