# NES-Emulator
NES Emulator written from scratch in c

## Installation 
- Clone repo with `git clone --recursive https://github.com/junnys6018/NES-Emulator.git`
- If the repository was cloned non-recursively run `git submodule update --init` to clone required submodules
- Premake files are provided to generate project files for any supported toolset.
- For convenience, scripts are provided to generate visual studio project files for Windows and make files for Linux
- Binarys are generated in the `bin` directory, note that the executable has to be run from the root directory
- Building in release will automatically copy the executable from the `bin` folder into the root directory of the repository 
