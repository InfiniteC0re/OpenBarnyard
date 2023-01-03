#include "pch.h"
#include "AExampleClass.h"
#include "./Memory/AMemory.h"

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
			unsigned char* buf = (unsigned char*)TMalloc(file->GetSize());//-12);
			//file->Seek(12, TFile::TSEEK_CUR);
			file->Read(buf, file->GetSize());//-12);

			// This is only for testing purposes
			*(int*)(&buf[8]) = 0; // Setting the CRC to 0 so the new CRC doesn't take the old one and CRCs it

			TUtil::CRCInitialise();
			uint32_t crc = TUtil::CRC32(buf, file->GetSize());

			TFree(buf);
			file->Destroy();
		}
	}
	/*
	TFile* file = TFile::Create("C:\\Program Files (x86)\\Steam\\steamapps\\common\\de Blob\\Data\\XUI\\DE\\common.trb", TFile::OpenFlags_Read);
	TTRB trb;
	trb.Load(file);
	uint8_t* buf = (uint8_t*)trb.GetSymbolAddress("txui");
	buf += 8;
	buf = (uint8_t*)(*(int*)buf);
	TXUIResource* res = new TXUIResource();
	res->Load(buf);
	*/
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

	class NodeTreeTest : public TNodeTree<NodeTreeTest>::TNode
	{
	public:
		NodeTreeTest(float value) : m_Value(value) { }

		float GetValue() { return m_Value; }

	private:
		float m_Value;
	};

	TNodeTree<NodeTreeTest> tree;

	struct DummyShit
	{
		TNodeTree<NodeTreeTest>* fuckthisshit1;
		NodeTreeTest* fuckthisshit2 = 0;
		NodeTreeTest* fuckthisshit3 = 0;
		NodeTreeTest* fuckthisshit4 = 0;
		NodeTreeTest* fuckthisshit5 = 0;
	};

	DummyShit dummyparent;
	tree.InsertNode((TNodeTree<NodeTreeTest>::TNode*)&dummyparent, new NodeTreeTest(5.0f));
	tree.InsertNode((TNodeTree<NodeTreeTest>::TNode*)&dummyparent, new NodeTreeTest(7.0f));
	tree.InsertNode((TNodeTree<NodeTreeTest>::TNode*)&dummyparent, new NodeTreeTest(9.0f));
	//TOSHI_INFO(tree.Root()-> ->GetValue());

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
	list.InsertHead(new Test(1.0f));
	list.InsertHead(new Test(5.0f));
	
	list.Head()->Log();         // 5.0
	list.Head()->Next()->Log(); // 1.0

	delete list.Head()->Next();
	delete list.Head();

	auto kernel = new TKernelInterface();
	
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