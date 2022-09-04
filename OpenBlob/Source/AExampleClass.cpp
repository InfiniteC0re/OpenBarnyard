#include "pch.h"
#include "AExampleClass.h"
#include "ARootTask.h"

#include <Toshi/File/TNativeFileSystem.h>
#include <Toshi/Core/TSystem.h>
#include <Toshi/Core/TKernelInterface.h>
#include <Toshi/Core/TScheduler.h>
#include <Toshi/Math/TRandom.h>

TOSHI_CLASS_DERIVED_INITIALIZE(AExampleClass, Toshi::TObject, TMAKEVERSION(1, 0))

AExampleClass::AExampleClass()
{
	TOSHI_INFO("Called AExampleClass");

	Toshi::TCString str = Toshi::TCString("C:\\Program Files (x86)\\Steam\\steamapps\\common\\de Blob\\Data\\BlobChar\\AssetPack.trb");
	Toshi::TNativeFileSystem* fs = Toshi::tnew<Toshi::TNativeFileSystem>(str);
	Toshi::TNativeFile* f = (Toshi::TNativeFile*)fs->CreateFile(str, 1);

	Toshi::TRandom rnd;
	TOSHI_INFO("=================TRandom Test=================");
	TOSHI_INFO("1) Random int (9000, 10000): {0}", rnd.GetInt(9000, 10000));
	TOSHI_INFO("2) Random int (9000, 10000): {0}", rnd.GetInt(9000, 10000));
	TOSHI_INFO("3) Random int (9000, 10000): {0}", rnd.GetInt(9000, 10000));
	TOSHI_INFO("4) Random int (9000, 10000): {0}", rnd.GetInt(9000, 10000));
	TOSHI_INFO("5) Random float: {0}", rnd.GetFloat());
	TOSHI_INFO("6) Random float: {0}", rnd.GetFloat());
	TOSHI_INFO("7) Random float: {0}", rnd.GetFloat());
	TOSHI_INFO("8) Random float: {0}", rnd.GetFloat());
	TOSHI_INFO("9) Random int (500): {0}", rnd.GetInt(500));
	TOSHI_INFO("10) Random int (500): {0}", rnd.GetInt(500));
	TOSHI_INFO("11) Random int (500): {0}", rnd.GetInt(500));
	TOSHI_INFO("12) Random int (500): {0}", rnd.GetInt(500));
	TOSHI_INFO("13) Random int: {0}", rnd.GetInt());
	TOSHI_INFO("14) Random int: {0}", rnd.GetInt());
	TOSHI_INFO("15) Random int: {0}", rnd.GetInt());
	TOSHI_INFO("16) Random int: {0}", rnd.GetInt());
	TOSHI_INFO("==============================================");

	if (f)
	{
		char sos[0x800]{};
		f->Seek(5, Toshi::TNativeFile::TSEEK_SET);
		int read = f->Read(&sos, sizeof(sos));

		//f.Open(str, 1);

		str.Concat(".lol", -1);
		
		fs->SetPrefix("C:/");

		TOSHI_INFO("File size: {0} bytes", f->GetSize());
		f->Seek(5, Toshi::TFile::TSEEK_SET);
		TOSHI_INFO("Seeked to offset: {0}", f->Tell());
		f->Close();
	}
	
	auto kernel = Toshi::tnew<Toshi::TKernelInterface>();
	
	m_RootTask = static_cast<ARootTask*>(
		kernel->GetScheduler().CreateTask(&ARootTask::s_Class, nullptr)
	);

	m_RootTask->SetKernelInterface(kernel);
	m_RootTask->Create();

	TOSHI_INFO("Started ARootTask for 5 seconds...");

	while (m_RootTask->IsActive())
	{
		kernel->Update();
	}

	kernel->Delete();
}

AExampleClass::~AExampleClass()
{
	TOSHI_INFO("Called ~AExampleClass");
}