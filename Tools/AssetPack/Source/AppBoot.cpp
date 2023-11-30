#include "pch.h"

// TOSHI_TMEMORY_FLAGS allows you to change the mode TMemory initializes with
// Flags_NativeMethods stands for native memory management methods instead of using dlmalloc
#define TOSHI_TMEMORY_FLAGS Toshi::TMemory::Flags_NativeMethods

// TOSHI_TMEMORY_SIZE allows you to change size of the global memory block
// The value is not used when TMemory initialized with native methods
#define TOSHI_TMEMORY_SIZE 0

// This file includes the entrypoint so set all the settings before including it
#include <Toshi.h>
#include <Toshi/Strings/TPString8.h>

#include "AArgumentParser.h"
#include "AAssetBuilder.h"
#include "AAssetUnpacker.h"

TOSHI_NAMESPACE_USING

int TMain(int argc, char** argv)
{
	TSystemManager::GetSingleton()->SetStringPool(new TPString8Pool(1024, 0, &T2Allocator::s_GlobalAllocator, TNULL));

	AArgumentParser args(argv, argc);
	if (args.GetMode() == AArgumentParser::Mode::Unpack)
	{
		TString8 filepath = args.GetPath1();

		for (TUINT i = 0; i < filepath.Length(); i++)
			if (filepath[i] == '/') filepath[i] = '\\';

		TString8 inputFileName = filepath.GetString(filepath.FindReverse('\\', -1) + 1);
		inputFileName.Truncate(inputFileName.FindReverse('.', -1));

		TString8 outputDir = filepath;
		outputDir.Truncate(outputDir.FindReverse('\\', -1));
		outputDir += "\\";
		outputDir += inputFileName;
		CreateDirectoryA(outputDir, NULL);

		AAssetPack assetPack;
		if (assetPack.Load(filepath))
		{
			AAssetUnpacker::Unpack(assetPack, outputDir, args.IsUsingBTEC());
		}
	}

	return 0;
}