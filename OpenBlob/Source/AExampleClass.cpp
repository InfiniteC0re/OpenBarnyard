#include "pch.h"
#include "AExampleClass.h"
#include "Memory/AMemory.h"
#include "Toshi/Xui/TXUI.h"
#include "Toshi/Memory/TMemory.h"

#include <Toshi/File/TTSF.h>
#include <Toshi/Render/TAssetInit.h>
#include <Toshi/Core/TError.h>

using namespace Toshi;

AExampleClass::AExampleClass()
{
	TOSHI_INFO("Called AExampleClass");

	TRandom rnd;
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

	{
		TFile* file = TFile::Create("C:\\Program Files (x86)\\Steam\\steamapps\\common\\de Blob\\Data\\BlobChar\\AssetPack.trb");

		if (file)
		{
			char sos[0x800]{};
			file->Seek(5, TFile::TSEEK_SET);
			int read = file->Read(&sos, sizeof(sos));

			TOSHI_INFO("File size: {0} bytes", file->GetSize());
			file->Seek(5, TFile::TSEEK_SET);
			TOSHI_INFO("Seeked to offset: {0}", file->Tell());
			file->Destroy();
		}
	}

	{
		TFile* file = TFile::Create("C:\\Program Files (x86)\\Steam\\userdata\\180297931\\532320\\remote\\SaveData.dat.old");

		if (file)
		{
			unsigned char* buf = (unsigned char*)TMalloc(file->GetSize());//-12);
			//file->Seek(12, TFile::TSEEK_CUR);
			file->Read(buf, file->GetSize());//-12);

			// This is only for testing purposes
			*(int*)(&buf[8]) = 0; // Setting the CRC to 0 so the new CRC doesn't take the old one and CRCs it

			uint32_t crc = TUtil::CRC32(buf, file->GetSize());

			TFree(buf);
			file->Destroy();
		}
	}
	
	TFile* file = TFile::Create("Data\\XUI\\DE\\common.trb");
	
	if (file)
	{
		TTRB trb;
		trb.Load(file);
		TAssetInit::InitAssets(trb, true, false);
	}
	

	/*TError* error = TError::GetSingletonWeak();
	error->AddError("Material \'%s\' Couldnt find texture \'%s\'", "test", "debloblol.tga");
	error->AddError("couldnt create resource \'%s\' because we have reached our max (%d)", "Resource1", 5);
	const char* error1 = error->GetError(0);
	const char* error2 = error->GetError(1);
	const char* error3 = error->GetError(2);*/

	/*new Toshi::TLogFile();

	Toshi::TLogFile::GetSingletonWeak()->Log(Toshi::TLogFile::TYPE_Info, "Toshi", "Kernel", "Creating AExampleClass %s", "Yes we are...");*/

	/*TFile* file = TFile::Create("C:\\Program Files (x86)\\Steam\\steamapps\\common\\de Blob\\Data\\BlobChar\\AssetPack.trb");

	if (file)
	{
		TTRB trb;
		trb.Load(file);
		TAssetInit::InitAssets(trb, true, false);
	}*/

	/*
	TFile* file = TFile::Create("C:\\Program Files (x86)\\Steam\\steamapps\\common\\de Blob\\Data\\XUI\\DE\\common.trb", TFile::OpenFlags_Read);
	
	if (file)
	{
		file->Seek(0x6E1 + 2, TFile::TSEEK_SET);

		uint8_t buf[0x30];
		file->Read(buf, 0x30);
		uint8_t* buf2 = buf;
		XURXUISceneData* element = new XURXUISceneData();
		TXUIResource resource;
		element->Load(resource, buf2);
	}
	*/
	/*
	if (error == TTRB::ERROR_OK)
	{
		TXUIResource xuiResource;
		xuiResource.ReadHeader((unsigned char*)trb.GetSymbolAddress("txui"));
		int size = xuiResource.GetTotalSize((unsigned char*)trb.GetSymbolAddress("txui"));
		TOSHI_INFO(size);
	}
	
	trb.Load("C:\\Program Files (x86)\\Steam\\steamapps\\common\\de Blob\\Data\\BlobChar\\AssetPack.trb");
	trb.GetSymbolAddress("SkeletonHeader");
	trb.Load("C:\\Users\\nepel\\Desktop\\BKG_CONCEPT01_NTSC_ENG.TTL");
	*/

	TSystemManager::GetSingleton()->GetScheduler()->CreateTask(TGetClass(ARootTask))->Create();
}

AExampleClass::~AExampleClass()
{
	TOSHI_INFO("Called ~AExampleClass");
}