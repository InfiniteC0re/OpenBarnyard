#include "pch.h"

#include <Toshi.h>
#include <Toshi/Strings/TPString8.h>

#include "AArgumentParser.h"
#include "AAssetBuilder.h"
#include "AAssetUnpacker.h"

TOSHI_NAMESPACE_USING

static TMemoryInitialiser s_MemoryInitialiser;

int main(int argc, char** argv)
{
	Toshi::TUtil::ToshiCreate(GetCommandLineA(), 0, 0);
	TUtil::SetTPStringPool(new TPString8Pool(1024, 0, &T2Allocator::s_GlobalAllocator, TNULL));

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
			//assetPack.Save("C:\\Users\\InfiniteC0re\\Desktop\\asset.trb");
			AAssetUnpacker::Unpack(assetPack, outputDir, args.IsUsingBTEC());
		}
	}

	return 0;
}