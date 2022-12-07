#include "pch.h"
#include "ARootTask.h"
#include <Toshi/File/TTRB.h>
#include <Toshi/Core/TSystem.h>

#define TEST_STL 1
#define TEST_LOCALE_BARNYARD 1

bool ARootTask::OnUpdate(float deltaTime)
{
	m_Seconds += deltaTime;

	if (m_Seconds >= 5)
	{
		m_Seconds = 0;
		Activate(false);

		TOSHI_INFO("ARootTask: Five seconds has passed. Deactivating ARootTask");
		TOSHI_INFO("Average FPS is {0:f}", m_Kernel->GetAverageFPS());

#if TEST_STL
		TOSHI_INFO("===================STL Test===================");
		TOSHI_INFO("----------------------------------------------");
		TOSHI_INFO("Toshi::STL::Vector");
		TOSHI_INFO("----------------------------------------------");
		Toshi::STL::Vector<float> vec;
		vec.push_back(0.1f);
		vec.push_back(0.5f);
		vec.push_back(0.9f);

		TOSHI_INFO("Vector's size: {0}", vec.size());
		TOSHI_INFO("Vector's elements:");
		for (auto& f : vec) { TOSHI_INFO(f); }
		TOSHI_INFO("----------------------------------------------");
		TOSHI_INFO("Toshi::STL::Map");
		TOSHI_INFO("----------------------------------------------");
		Toshi::STL::Map<int, float> map;
		map[0] = 0.2f;
		map[98] = 987.0f;
		map[154] = 24.6f;
		TOSHI_INFO("Map Size: {0}", map.size());
		TOSHI_INFO("Map contains 0: {0}", map.contains(0));
		TOSHI_INFO("Map contains 1: {0}", map.contains(1));
		TOSHI_INFO("Map contains 97: {0}", map.contains(97));
		TOSHI_INFO("Map contains 98: {0}", map.contains(98));
		TOSHI_INFO("Map contains 154: {0}", map.contains(154));
		TOSHI_INFO("----------------------------------------------");
		TOSHI_INFO("Toshi::STL::UnorderedMap (HashTable)");
		TOSHI_INFO("----------------------------------------------");
		Toshi::STL::UnorderedMap<int, float> umap;
		umap[0] = 0.2f;
		umap[98] = 987.0f;
		umap[154] = 24.6f;
		TOSHI_INFO("UnorderedMap has {0} buckets", umap.bucket_count());
		TOSHI_INFO("==============================================");
#endif

#if TEST_LOCALE_BARNYARD
		struct LocaleStrings
		{
			uint32_t Count;
			wchar_t** Strings;
		};

		Toshi::TTRB localeFile;
		auto result = localeFile.Load("Data/Barnyard_Locale/eng-uk.trb");
		TASSERT(result == Toshi::TTRB::ERROR_OK, "Unable to open file");

		LocaleStrings* table = localeFile.CastSymbol<LocaleStrings>("LocaleStrings");
		TASSERT(table != TNULL, "LocaleStrings doesn't exist");

		TOSHI_TRACE("Loaded Barnyard locale file");
		TOSHI_TRACE("String count: {0}", table->Count);
		TOSHI_TRACE(L"First string: {0}", table->Strings[0]);
#endif
	}

	return true;
}
