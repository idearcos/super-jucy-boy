# Super Jucy Boy
Super Jucy Boy is a Game Boy emulator written in C++. It is developed for educational purpose, therefore the main objective is to emulate the real hardware as accurately as possible.

The project is built using the JUCE framework, a great toolkit to create multi-platform GUI applications in C++.

## How to build
JUCE framework is used to manage the project in a platform-independent way.

- [Download JUCE](https://www.juce.com/get-juce)
- Open SuperJucyBoy.jucer in the Projucer application (bundled with JUCE).
- If your target platform is not in the list of existing ones, create a new exporter.
- Save the Projucer project in order to generate the project files of the various platforms.
- Select your target platform in the list in Projucer, and click "Open in IDE..." to open the project in your desired IDE.
- Build the application as usual in your IDE.