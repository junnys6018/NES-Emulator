# NES-Emulator
![ci badge](https://github.com/junnys6018/NES-Emulator/actions/workflows/main-workflow.yml/badge.svg)

NES Emulator written from scratch in C.  
  
Can be built for Linux, Windows and webassembly!

## Demo
https://junnys6018.github.io/nes/

## Screenshots
![Demo Gif](media/demo.gif)

## Audio Waveform visualisation
<img src="media/oscilloscope.png" alt="oscilloscope" width="400"/>

This emulator has a real time waveform visualisation of each channel, (square 1 and 2, triangle and noise channel).
Video demo can be found [here](https://youtu.be/fevGlhVMHI8)

## Building 
- Clone repo with `git clone --recursive https://github.com/junnys6018/NES-Emulator.git`.
- If the repository was cloned non-recursively run `git submodule update --init` to clone required submodules.
- Run `./scripts/GenerateLinuxProjects.sh` or `./scripts/GenerateWindowsProjects.bat`.
- Build files are in the `build` directory.
- Build with make on linux or msvc on windows. 
- Make sure you build in release, otherwise the emulation will not run in real time.

### Webassembly
Here is a rough guide for targeting webassembly. If you just want to get hold of the webassembly module and javascript glue code, it is provided in the `web` directory as emu.js and emu.wasm. Included is an example consuming the webassembly module.

- Install [emscripten](https://emscripten.org/docs/getting_started/downloads.html)
- Download the emscripten toolchain file [here](https://github.com/emscripten-core/emscripten/blob/main/cmake/Modules/Platform/Emscripten.cmake)
- Activate emscripten `source <path/to/emsdk/emsdk_env.sh>`

```
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=<path/to/Emscripten.cmake> -DCMAKE_BUILD_TYPE=Release
cd build
make
```

## Dependencies
The only dependency this project has is SDL. On windows the library is prebuilt in the repo, so you dont have to download and install SDL yourself.  
On linux run `sudo apt-get install libsdl2-dev`

## Running 
The emulator can be run from the command line with `NES-Emulator.exe [rom.nes]`  
Alternatively you can just run the program without arguments and goto `Settings -> Load Rom...` 

**On Windows** make sure the executable can find sdl.dll, this is provided in `application`, if you try to run the executable from the `build` directory the program will crash. Building in release will automatically copy the executable from the `build` folder into `application`. **Make sure you run the executable in the same directory as sdl.dll**

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
### Keyboard
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

### Controller
Other controllers may work but are untested

Button | Key (PS4)   | Key (Xbox)  
-------|-------------|-------------
A      | X           | A           
B      | O           | B           
Start  | Options     | Start       
Select | Share       | Back        
Up     | D-pad Up    | D-pad Up    
Down   | D-pad Down  | D-pad Down  
Left   | D-pad Left  | D-pad Left  
Right  | D-pad Right | D-pad Right 
  
This emulator has the abilty to pause the emulation and step through it, here are the controls 

Key    | Action         
-------|-----------------------------
Space  | Emulate one CPU instruction           
F      | Emulate one frame           
P      | Emulate one PPU cycle
