# OpenBlob
Decompilation of de Blob and Toshi engine

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