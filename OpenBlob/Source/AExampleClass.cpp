#include "AExampleClass.h"
#include "Toshi/File/TNativeFile.h"
#include "Toshi/Core/TSystem.h"

TOSHI_CLASS_DERIVED_INITIALIZE(AExampleClass, Toshi::TObject, 1, 0)

AExampleClass::AExampleClass()
{
	TOSHI_INFO("Called AExampleClass");

	Toshi::TNativeFile f;
	Toshi::TCString str = Toshi::TCString("C:\\Program Files (x86)\\Steam\\steamapps\\common\\de Blob\\Data\\BlobChar\\AssetPack.trb");
	//f.Open(&str, 1);
	//f.Seek(5, Toshi::TFile::TSEEK_SET);
	//TOSHI_INFO(f.Tell());
	//f.Close();
}

AExampleClass::~AExampleClass()
{
	TOSHI_INFO("Called ~AExampleClass");
}