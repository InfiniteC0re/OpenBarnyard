#include "pch.h"
#include "AExampleClass.h"
#include "ARootTask.h"

#include <Toshi/File/TFile.h>
#include <Toshi/Core/TSystem.h>
#include <Toshi/Core/TKernelInterface.h>
#include <Toshi/Core/TScheduler.h>
#include <Toshi/Math/TRandom.h>
#include <Toshi/File/TTRB.h>
#include <Toshi/Core/TGenericDList.h>
#include <MemoryCard/EnSaveData.h>
#include <Toshi/Core/TMemory.cpp>
#include <Toshi/Xui/TXUIResource.h>

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
		TFile* file = TFile::Create("C:\\Program Files (x86)\\Steam\\steamapps\\common\\de Blob\\Data\\BlobChar\\AssetPack.trb", TFile::OpenFlags_Read);

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
		TFile* file = TFile::Create("C:\\Program Files (x86)\\Steam\\userdata\\180297931\\532320\\remote\\SaveData.dat.old", TFile::OpenFlags_Read);

		if (file)
		{
			unsigned char* buf = (unsigned char*)tmalloc(file->GetSize());//-12);
			//file->Seek(12, TFile::TSEEK_CUR);
			file->Read(buf, file->GetSize());//-12);

			// This is only for testing purposes
			*(int*)(&buf[8]) = 0; // Setting the CRC to 0 so the new CRC doesn't take the old one and CRCs it

			EnSaveData::GenerateCRC();
			uint32_t crc = EnSaveData::CalculateCRC(buf, file->GetSize());

			tfree(buf);
			file->Destroy();
		}
	}

	TTRB trb;
	TTRB::ERROR error = trb.Load("C:\\Program Files (x86)\\Steam\\steamapps\\common\\de Blob\\Data\\XUI\\UK\\frontend.trb");

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

	class Test : public TDList<Test>::TNode
	{
	public:
		Test(float value) : m_Value(value) { }
		~Test() { TNode::Remove(); }

		inline Test* Next() { return TNode::Next()->As<Test>(); }
		inline float GetValue() const { return m_Value; }

		inline void Log() const { TOSHI_INFO("Value of TDList<Test>::TNode: {0}", m_Value); }
	private:
		float m_Value;
	};

	TDList<Test> list;
	list.InsertHead(tnew<Test>(1.0f));
	list.InsertHead(tnew<Test>(5.0f));
	
	list.Head()->Log();         // 5.0
	list.Head()->Next()->Log(); // 1.0

	tdelete(list.Head()->Next());
	tdelete(list.Head());

	auto kernel = tnew<TKernelInterface>();
	
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