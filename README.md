# OpenBarnyard [![Toshi](https://github.com/InfiniteC0re/OpenBarnyard/actions/workflows/ci.yaml/badge.svg)](https://github.com/InfiniteC0re/OpenBarnyard/actions/workflows/ci.yaml)
[OpenJPOG](https://github.com/AdventureT/OpenJPOG) | [Modifications](https://www.youtube.com/@infc0re) | [Discord](https://discord.gg/3bMq8hWvFX)

This is a **WIP** decompilation of Barnyard the video game (for Windows) and the proprietary TOSHI 2.0 game engine developed by Blue Tongue Entertainment.

## Status

<img src="https://opentoshi.net/progress/Barnyard.svg" width="100%"/>

At the moment, the progress bar and percentage don't show the real progress since many methods don't have markers containing their addresses yet so it might take a while to fill the gaps.

## Notes

> [!IMPORTANT]
> **SUPPORT THE DEVELOPERS OF THE TOSHI ENGINE**
> 
> We do not own the Toshi Engine in any way, shape or form, and this project would not have been possible had they not developed TOSHI in the first place. The Toshi Engine is currently owned by [THQ Nordic](https://www.thqnordic.com); we highly urge you to follow & support their projects if you like this project!

> [!WARNING]
> **DO NOT USE THIS DECOMPILATION PROJECT AS A MEANS TO PIRATE BARNYARD OR ANY OTHER TOSHI GAMES!**
> 
> We do not condone using this project as a means for piracy in any form. This project was made with love and care for the source material and was created for purely educational purposes. 

## Building

### Windows

Building on Windows requires:

- Visual Studio 2022

Clone the repository with the following commands:

	git clone https://github.com/InfiniteC0re/OpenBarnyard.git
	cd OpenBarnyard
	git submodule update --init --recursive
	./GenerateProjects-VS2022.bat

Doing that should be enough to build the whole solution in IDE.

## Thirdparty libraries
- [premake](https://github.com/premake/premake-core)
- [tracy](https://github.com/wolfpld/tracy)
- [fmod](https://fmod.com/)
- [Opcode](http://www.codercorner.com/Opcode.htm)
