#include "pch.h"
// TOSHI_TMEMORY_FLAGS allows you to change the mode TMemory initializes with
// Flags_NativeMethods stands for native memory management methods instead of using dlmalloc
#define TOSHI_TMEMORY_FLAGS Toshi::TMemory::Flags_NativeMethods

// TOSHI_TMEMORY_SIZE allows you to change size of the global memory block
// The value is not used when TMemory initialized with native methods
#define TOSHI_TMEMORY_SIZE 0

// This file includes the entrypoint so set all the settings before including it
#include "Toshi.h"
#include "ttc.h"

using namespace Toshi;
using namespace rapidxml;

static void ShowUsage()
{
	TOSHI_WARN("Usage:");
	TOSHI_WARN("ttc.exe C:/ttc.xml");
}

int TMain(int argc, char** argv)
{
	if (argc != 2)
	{
		//ShowUsage();
		//return;
	}

	//argv++;
	TFile* docFile = TFile::Create("D:\\Barnyard\\Preserved\\SKY.XML");
	size_t docFileSize = docFile->GetSize();

	char* docFileData = new char[docFileSize + 1];
	docFile->Read(docFileData, docFileSize);
	docFileData[docFileSize] = '\0';
	docFile->Destroy();

	xml_document<char> doc;
	doc.parse<0>(docFileData);

	auto ttcNode = doc.first_node("TTC");

	if (ttcNode == TNULL)
	{
		TOSHI_ERROR("Invalid XML file was provided");
		return 1;
	}
	
	TTC ttc;
	ttc.Parse(ttcNode);
	ttc.Write();

	return 0;
}