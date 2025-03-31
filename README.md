![Logo](UI/SquarixLogo.png)

A small C++ game where you have to dodge squares. 

## Downloading

Download the archive with the latest version from [here](https://github.com/MaximKrait/Squarix/releases)
 and run the executable in it. Important, the font file must be in the same folder as the game!!!

At the moment only the Linux version has been compiled!

## Building

If you want to compile the game yourself from the source files, then proceed according to these instructions:

### Linux

1. Establish dependencies

- Fedora
  
 ```bash
 sudo dnf install g++ SDL2-devel SDL2_ttf-devel  
 ```
- Ubuntu/Debian

 ```bash
 sudo apt install g++ libsdl2-dev libsdl2-ttf-dev  
 ```

- Arch Linux

 ```bash
 sudo pacman -S base-devel sdl2 sdl2_ttf  
 ```
1.1. File retrieval

You can get the source code directly from [here](https://github.com/MaximKrait/Squarix/releases)

Or you can do the following:

 ```bash
 git clone https://github.com/MaximKrait/Squarix.git
cd Squarix
 ```

2. Compile the code

 ```bash
 g++ -o Squarix main.cpp -lSDL2 -lSDL2_ttf
 ```

3. Start game
 ```bash
 ./Squarix
 ```
or click on the executable file

### Windows

1. Install MinGW-w64 (if there is no compiler)

2. Install SDL2 and SDL2_ttf

3. Compile the code 
 ```bash
 g++ -o Squarix.exe main.cpp -I"path_to_sdl_include" -L"path_to_sdl_lib" -lSDL2 -lSDL2_ttf
 ```
4. Start game



