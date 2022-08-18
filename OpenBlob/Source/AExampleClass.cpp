#include "AExampleClass.h"
#include "Toshi/File/TNativeFile.h"
#include "Toshi/Core/TSystem.h"

TOSHI_CLASS_INITIALIZE(AExampleClass, nullptr, 1, 0)

AExampleClass::AExampleClass()
{
	TOSHI_INFO("Called AExampleClass");

	Toshi::TNativeFile f;
	Toshi::TCString str = Toshi::TCString("C:\\Program Files (x86)\\Steam\\steamapps\\common\\de Blob\\Data\\BlobChar\\AssetPack.trb");
	f.Open(&str, 1);
}

AExampleClass::~AExampleClass()
{
	TOSHI_INFO("Called ~AExampleClass");
}