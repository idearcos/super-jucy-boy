/* ==================================== JUCER_BINARY_RESOURCE ====================================

   This is an auto-generated file: Any edits you make may be overwritten!

*/

namespace BinaryData
{

//================== README.md ==================
static const unsigned char temp_binary_data_0[] =
"# Super Jucy Boy\r\n"
"Super Jucy Boy is a Game Boy emulator written in C++. It is developed for educational purpose, therefore the main objective is to emulate the real hardware as accurately as possible.\r\n"
"\r\n"
"The project is built using the JUCE framework, a great toolkit to create multi-platform GUI applications in C++.\r\n"
"\r\n"
"## How to build\r\n"
"JUCE framework is used to manage the project in a platform-independent way.\r\n"
"\r\n"
"- [Download JUCE](https://www.juce.com/get-juce)\r\n"
"- Open SuperJucyBoy.jucer in the Projucer application (bundled with JUCE).\r\n"
"- If your target platform is not in the list of existing ones, create a new exporter.\r\n"
"- Save the Projucer project in order to generate the project files of the various platforms.\r\n"
"- Select your target platform in the list in Projucer, and click \"Open in IDE...\" to open the project in your desired IDE.\r\n"
"- Build the application as usual in your IDE.";

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
        case 0x64791dc8:  numBytes = 892; return README_md;
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
