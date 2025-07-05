# chip8
My take on a CHIP-8 Interpreter. This is a practice project to dip my toes into emulator development.

## building
Get yourself a copy of the dependencies
```
mkdir vendored
cd vendored
git clone https://github.com/libsdl-org/SDL.git
git clone https://gitlab.com/iniparser/iniparser.git
```
Use cmake
```
mkdir build
cd build
cmake ..
```
## resources
The chip8 community, as well as the wider EmuDev community is fantastic. There are many resources and tools to help get beginners started.  
[https://tobiasvl.github.io/blog/write-a-chip-8-emulator/](https://tobiasvl.github.io/blog/write-a-chip-8-emulator/)  
[http://devernay.free.fr/hacks/chip8/C8TECH10.HTM](http://devernay.free.fr/hacks/chip8/C8TECH10.HTM)  
[https://github.com/Timendus/chip8-test-suite](https://github.com/Timendus/chip8-test-suite)  

## libraries
I used some fantastic libraries. credits to [SDL3](https://wiki.libsdl.org/SDL3/FrontPage) and [iniparser](https://gitlab.com/iniparser/iniparser)
