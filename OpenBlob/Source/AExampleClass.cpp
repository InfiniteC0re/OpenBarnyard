#include "AExampleClass.h"
#include "Toshi/File/TNativeFileSystem.h"
#include "Toshi/Core/TSystem.h"

TOSHI_CLASS_DERIVED_INITIALIZE(AExampleClass, Toshi::TObject, MKVERSION(1, 0))
TOSHI_CLASS_DERIVED_INITIALIZE(AExampleClass2, Toshi::TObject, MKVERSION(1, 1))

AExampleClass::AExampleClass()
{
	TOSHI_INFO("Called AExampleClass");

	
	Toshi::TCString str = Toshi::TCString("C:\\Program Files (x86)\\Steam\\steamapps\\common\\de Blob\\Data\\BlobChar\\AssetPack.trb");
	Toshi::TNativeFileSystem* fs = new Toshi::TNativeFileSystem(str.GetString());
	Toshi::TNativeFile* f = (Toshi::TNativeFile*)fs->CreateFile(str, 1);
	char sos[0x800]{};
	f->Seek(5, Toshi::TNativeFile::TSEEK_SET);
	int read = f->Read(&sos, sizeof(sos));

	//f.Open(str, 1);
	
	str.Concat(".lol", -1);

	((Toshi::TFileSystem*)fs)->SetPrefix("C:/");

	if (f)
	{
		TOSHI_INFO("File size: {0} bytes", f->GetSize());
		f->Seek(5, Toshi::TFile::TSEEK_SET);
		TOSHI_INFO("Seeked to offset: {0}", f->Tell());
		f->Close();
	}

}

AExampleClass::~AExampleClass()
{
	TOSHI_INFO("Called ~AExampleClass");
}

AExampleClass2::AExampleClass2()
{
	TOSHI_INFO("Called AExampleClass2");
}

AExampleClass2::~AExampleClass2()
{
	TOSHI_INFO("Called ~AExampleClass2");
}