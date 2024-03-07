# OpenBarnyard [![Toshi](https://github.com/InfiniteC0re/OpenBarnyard/actions/workflows/ci.yaml/badge.svg)](https://github.com/InfiniteC0re/OpenBarnyard/actions/workflows/ci.yaml)
Decompilation of Barnyard the video game and the TOSHI 2.0 game engine.

![alt text](https://repository-images.githubusercontent.com/524925868/e2f3fde2-ece4-4a71-bd81-f4cbce744c9a)

# **SUPPORT THE DEVELOPERS OF THE TOSHI ENGINE**
We do not own the Toshi Engine in any way, shape or form, and this project would not have been possible had they not developed TOSHI in the first place. The Toshi Engine is currently owned by [THQ Nordic](https://www.thqnordic.com); we highly urge you to follow & support their projects if you like this project!

## **DO NOT USE THIS DECOMPILATION PROJECT AS A MEANS TO PIRATE BARNYARD OR ANY OTHER TOSHI GAMES.**
We do not condone using this project as a means for piracy in any form. This project was made with love and care for the source material and was created for purely educational purposes.

## Building

### Windows

Building on Windows requires

- Visual Studio 2022

Clone the repository with the following commands:

	git clone https://github.com/InfiniteC0re/OpenBarnyard.git
	cd OpenBarnyard
	git submodule update --init --recursive
	./GenerateProjects-VS2022.bat

Then compile the generated solution and that's it

## Thirdparty libraries
- [premake](https://github.com/premake/premake-core)
- [tracy](https://github.com/wolfpld/tracy)
- [fmod](https://fmod.com/)
- [Opcode](http://www.codercorner.com/Opcode.htm)
