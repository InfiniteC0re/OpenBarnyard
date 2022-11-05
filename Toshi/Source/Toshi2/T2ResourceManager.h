#pragma once
#include <cstdint>
#include "Toshi/File/TTRB.h"
#include "Toshi/Core/TThread.h"
#include "Toshi/Render/TAssetInit.h"
#include "Toshi/Utils/TUtil.h"

namespace Toshi
{
	using t_CreateDestroyCallbk = void* (*)(void* unk, TTRB* trb, bool unk1);

	class T2ResourceData;

	class T2Resource
	{
		int m_iID;
	public:
		static void CreateResource(const char* resourceName, void* unk, t_CreateDestroyCallbk a_fnCreateDestroyCallbk, void* a_pData);
		void DestroyResource();
	};

	class T2ResourceManager : public TSingleton<T2ResourceManager>
	{
	public:

		T2ResourceManager(int a_iMaxNumResources);
		~T2ResourceManager();

		T2ResourceData& GetResourceData(int a_iID);
		void* GetData(int a_iID);
		void IncRefCount(int a_iID);
		void DecRefCount(int a_iID);
		int CreateResource(const char* resourceName, void* unk, t_CreateDestroyCallbk a_fnCreateDestroyCallbk, void* a_pData);
		void DestroyResource(int a_iID);
		int FindUnusedResource();

		int m_iMaxNumResources;		// 0x0
		int m_iNumUsedResources;	// 0x4
		int m_iUnk;					// 0x8
		T2ResourceData* m_pData;	// 0xC
	};

	class T2ResourcePtr
	{
	public:
		static const int IDINVALID = 0;

		int m_count;

		T2ResourcePtr(const T2ResourcePtr& a_resourcePtr)
		{
			m_count = a_resourcePtr.m_count;
			T2ResourceManager::GetSingleton()->IncRefCount(m_count);
		}
	};

	// sizeof(T2ResourceData) == 0x4C!
	class T2ResourceData
	{
	public:
		enum FLAG : uint8_t
		{
			FLAG_LOADING = BITFIELD(0),
			FLAG_INITIALISED = BITFIELD(1),
			FLAG_DESTROYED = BITFIELD(2),
			FLAG_USED = BITFIELD(3),
		};
		
		void Init(const char* a_pName, t_CreateDestroyCallbk a_fnCreateDestroyCallbk, void* a_pData);
		void DeInit();
		void SetLoadedData(void* a_pData);
		void Unload();
		void* GetData();
		inline bool HasFlag(uint8_t flag) { return (m_iFlags & flag) == flag; }
	public:
		void* m_pData;										// 0x0
		TTRB* m_trb2;										// 0x4
		int m_iFlags;										// 0x8
		int m_iRefCount;									// 0xC
		char* m_pResourceName;								// 0x10
		void* m_unk;										// 0x40
		t_CreateDestroyCallbk m_fnCreateDestroyCallbk;		// 0x44
		TTRB* m_trb;										// 0x48
	};
}