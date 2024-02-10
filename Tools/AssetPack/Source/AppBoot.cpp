#include "pch.h"
#include "AArgumentParser.h"
#include "AAssetBuilder.h"
#include "AAssetUnpacker.h"

#include <Toshi/Toshi.h>
#include <Toshi/TPString8.h>

#include <Core/TMemoryInitialiser.h>

#include <filesystem>

TOSHI_NAMESPACE_USING

static TMemoryInitialiser s_MemoryInitialiser;

int main(int argc, char** argv)
{
	TUtil::TOSHIParams toshiParams;
	toshiParams.szCommandLine = GetCommandLineA();
	
	TUtil::ToshiCreate(toshiParams);
	TUtil::SetTPStringPool(new TPString8Pool(1024, 0, &T2Allocator::s_GlobalAllocator, TNULL));

	AArgumentParser args(argv, argc);
	if (args.GetMode() == AArgumentParser::Mode::Unpack)
	{
		TString8 filepath = args.GetInPath();

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
	else if (args.GetMode() == AArgumentParser::Mode::Pack)
	{
		TString8 filepath = args.GetInPath();

		for (TUINT i = 0; i < filepath.Length(); i++)
			if (filepath[i] == '/') filepath[i] = '\\';

		TString8 inputFileName = filepath.GetString(filepath.FindReverse('\\', -1) + 1);
		inputFileName.Truncate(inputFileName.FindReverse('.', -1));

		AAssetBuilder assetBuilder;

		for (const auto& entry : std::filesystem::directory_iterator(args.GetInPath()))
		{
			auto wcsPath = entry.path().native().c_str();

			char szPath[MAX_PATH];
			TStringManager::StringUnicodeToChar(szPath, entry.path().native().c_str(), -1);

			assetBuilder.Add(szPath);
		}

		TString8 outPath;
		TString8 assetName;

		if (args.GetOutPath())
		{
			outPath = TString8::Format("%s\\%s.trb", args.GetOutPath(), args.GetAssetName());
		}
		else
		{
			outPath = TString8::Format("%s.trb", args.GetAssetName());
		}

		assetBuilder.Save(outPath, args.IsUsingBTEC());
	}

	return 0;
}