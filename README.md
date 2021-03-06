# NES-Emulator
NES Emulator written from scratch in C.  
  
Currently can be built for Linux and Windows, with plans for building to Web Assembley (emscripten) in the future

## Screenshots
![Demo Gif](media/demo.gif)

## Audio Waveform visualisation
<img src="media/oscilloscope.png" alt="oscilloscope" width="400"/>

This emulator has a real time waveform visualisation of each channel, (square 1 and 2, triangle and noise channel).
Video demo can be found [here](https://youtu.be/fevGlhVMHI8)

## Installation 
- Clone repo with `git clone --recursive https://github.com/junnys6018/NES-Emulator.git`
- If the repository was cloned non-recursively run `git submodule update --init` to clone required submodules
- This project uses **premake** to build. For convenience, scripts are provided to generate visual studio project files for Windows and make files for Linux
- Binarys are generated in the `bin` directory.
- Make sure you build with optimisations turned on, otherwise the emulation will not run in real time. On visual studio, build in release. If your building with make, run `make config=release`

## Dependencies
The only dependency this project has is SDL. On windows the library is prebuilt in the repo, so you dont have to download and install SDL yourself.  
On linux run `sudo apt-get install libsdl2-dev`

## Running 
The emulator can be run from the command line with `NES-Emulator.exe [rom.nes]`  
Alternatively you can just run the program without arguments and goto `Settings -> Load Rom...` 

**On Windows** make sure the executable can find sdl.dll, this is provided in the root directroy of the repo, if you try to run the executable from the `bin` directory the program will crash. Building in release will automatically copy the executable from the `bin` folder into the root directory of the repository. **Make sure you run the executable in the same directory as sdl.dll**

## Mappers
NES cartidges have thier own circuitry that can modify the behaviour of the console, many cartridges provide bank switching, and have the ability to generate hardware interrupts. Some cartridges even have thier own sound systems to expand the audio capabilities of the NES. The nes emulation community have assigned numbers to each variant of cartridge, called a "mapper number". 

Currently the following mappers have been implemented

 \#  | Name  | Some Games
-----|-------|--------------------------------------------------
 000 | NROM  | Super Mario Bros. 1, Donkey Kong, Balloon Fight
 001 | MMC1  | Legend of Zelda, Dr. Mario, Metroid
 002 | UxROM | Mega Man, Duck Tales, Castlevania
 003 | CNROM | Arkanoid, Cybernoid, Solomon's Key
 004 | MMC3  | Super Mario Bros 2 and 3, Silver surfer  
 
There are plans to implement mappers 7 and 9 in the future, which covers most of the games on the NES

## Controls
Button | Key         
-------|-------------
A      | X           
B      | Z           
Start  | Enter       
Select | Tab 
Up     | Up arrow    
Down   | Down arrow  
Left   | Left arrow  
Right  | Right arrow 

There are plans to support controller input in the future.  
  
This emulator has the abilty to pause the emulation and step through it, here are the controls 
Key    | Action         
-------|-----------------------------
Space  | Emulate one CPU instruction           
F      | Emulate one frame           
P      | Emulate one PPU cycle
