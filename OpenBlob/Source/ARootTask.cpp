#include "pch.h"
#include "ARootTask.h"
#include <Toshi/File/TTRB.h>
#include <Toshi/Core/TSystem.h>

bool ARootTask::OnUpdate(float deltaTime)
{
	m_Seconds += deltaTime;

	if (m_Seconds >= 5)
	{
		m_Seconds = 0;

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

		TOSHI_INFO("ARootTask: Five seconds has passed. Deactivating ARootTask");
		TOSHI_INFO("Average FPS is {0:f}", m_Kernel->GetAverageFPS());

		Toshi::TTRB trb;
		TOSHI_INFO("Trying to read Data/Barnyard_Locale/eng-uk.trb");
		trb.Open("Data/Barnyard_Locale/eng-uk.trb");
		
		{
			struct LocaleStrings
			{
				uint32_t Count;
				wchar_t** Strings;

				/* the fields below only exist in Barnyard */

				uint16_t Unknown1; // 0
				uint16_t Unknown2; // 57760 (0b1110000110100000)
				uint16_t Unknown3; // 0
				uint16_t Unknown4; // 57760 (0b1110000110100000)
			};

			LocaleStrings* localeStrings = static_cast<LocaleStrings*>(trb.FindSymb("LocaleStrings"));

			for (uint32_t i = 0; i < localeStrings->Count; i++)
			{
				TOSHI_INFO(L"{0}", localeStrings->Strings[i]);
			}
		}
		/*else
		{
			TOSHI_ERROR("Cannot open Data/Barnyard_Locale/eng-uk.trb");
		}*/
		
		Activate(false);
	}

	return true;
}
