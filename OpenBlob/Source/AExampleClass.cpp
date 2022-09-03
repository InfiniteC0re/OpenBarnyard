#include "AExampleClass.h"
#include "ARootTask.h"

#include <Toshi/File/TNativeFileSystem.h>
#include <Toshi/Core/TSystem.h>
#include <Toshi/Core/TKernelInterface.h>
#include <Toshi/Core/TScheduler.h>
#include <Toshi/Math/TRandom.h>

TOSHI_CLASS_DERIVED_INITIALIZE(AExampleClass, Toshi::TObject, MKVERSION(1, 0))

AExampleClass::AExampleClass()
{
	TOSHI_INFO("Called AExampleClass");

	Toshi::TCString str = Toshi::TCString("C:\\Program Files (x86)\\Steam\\steamapps\\common\\de Blob\\Data\\BlobChar\\AssetPack.trb");
	Toshi::TNativeFileSystem* fs = new Toshi::TNativeFileSystem(str.GetString());
	Toshi::TNativeFile* f = (Toshi::TNativeFile*)fs->CreateFile(str, 1);

	Toshi::TRandom rnd;
	TOSHI_INFO("TRandom test:");
	TOSHI_INFO("Random int 1 (500, 10000): {0}", rnd.GetInt(500, 10000));
	TOSHI_INFO("Random int 2 (500, 10000): {0}", rnd.GetInt(500, 10000));
	TOSHI_INFO("Random int 3 (500, 10000): {0}", rnd.GetInt(500, 10000));
	TOSHI_INFO("Random int 4 (500, 10000): {0}", rnd.GetInt(500, 10000));
	TOSHI_INFO("Random float 1: {0}", rnd.GetFloat());
	TOSHI_INFO("Random float 2: {0}", rnd.GetFloat());
	TOSHI_INFO("Random float 3: {0}", rnd.GetFloat());
	TOSHI_INFO("Random float 4: {0}", rnd.GetFloat());
	TOSHI_INFO("Random int 1 (500): {0}", rnd.GetInt(500));
	TOSHI_INFO("Random int 2 (500): {0}", rnd.GetInt(500));
	TOSHI_INFO("Random int 3 (500): {0}", rnd.GetInt(500));
	TOSHI_INFO("Random int 4 (500): {0}", rnd.GetInt(500));
	TOSHI_INFO("Random int 1: {0}", rnd.GetInt());
	TOSHI_INFO("Random int 2: {0}", rnd.GetInt());
	TOSHI_INFO("Random int 3: {0}", rnd.GetInt());
	TOSHI_INFO("Random int 4: {0}", rnd.GetInt());

	if (f)
	{
		char sos[0x800]{};
		f->Seek(5, Toshi::TNativeFile::TSEEK_SET);
		int read = f->Read(&sos, sizeof(sos));

		//f.Open(str, 1);

		str.Concat(".lol", -1);

		((Toshi::TFileSystem*)fs)->SetPrefix("C:/");

		TOSHI_INFO("File size: {0} bytes", f->GetSize());
		f->Seek(5, Toshi::TFile::TSEEK_SET);
		TOSHI_INFO("Seeked to offset: {0}", f->Tell());
		f->Close();
	}
	
	Toshi::TKernelInterface* kernel = new Toshi::TKernelInterface;
	
	m_RootTask = static_cast<ARootTask*>(
		kernel->GetScheduler().CreateTask(&ARootTask::s_Class, nullptr)
	);

	m_RootTask->Create();

	while (true)
	{
		kernel->Update();
	}
}

AExampleClass::~AExampleClass()
{
	TOSHI_INFO("Called ~AExampleClass");
}