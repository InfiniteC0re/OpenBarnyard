# OpenBlob
Decompilation of de Blob and Toshi engine

# **SUPPORT THE DEVELOPERS OF THE TOSHI ENGINE**
We do not own the Toshi Engine in any way, shape or form, and this project would not have been possible had they not developed TOSHI in the first place. The Toshi Engine is currently owned by [THQ Nordic](https://www.thqnordic.com); we highly urge you to follow & support their projects if you like this project!

## **DO NOT USE THIS DECOMPILATION PROJECT AS A MEANS TO PIRATE DE BLOB OR ANY OTHER TOSHI GAMES.**
We do not condone using this project as a means for piracy in any form. This project was made with love and care for the source material and was created for purely educational purposes.

## Building

### Windows

Building on Windows requires

- Visual Studio 2019/2022

Clone the repository with the following commands:

	git clone https://github.com/AdventureT/OpenBlob.git
	cd OpenBlob
	git submodule update --init --recursive
	./GenerateProjects-VS2019.bat (if you use Visual Studio 2019)
	./GenerateProjects-VS2022.bat (if you use Visual Studio 2022)

Then compile the generated solution and that's it

## Thirdparty libraries
- [premake](https://github.com/premake/premake-core)
- [spdlog](https://github.com/gabime/spdlog)
