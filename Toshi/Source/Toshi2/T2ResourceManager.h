#pragma once
#include "Toshi/File/TTRB.h"
#include "Toshi/Thread/TThread.h"
#include "Toshi/Utils/TUtil.h"

#include <cstdint>

#ifdef FindResource
#undef FindResource
#endif

namespace Toshi
{
	class T2ResourceData;
	class T2ResourcePtr;

	class T2ResourceManager :
		public TSingleton<T2ResourceManager>
	{
	public:
		using t_CreateDestroyCallbk = void* (*)(void* pCustomData, TTRB* trb, TBOOL bCreated);

	public:
		T2ResourceManager(int a_iMaxNumResources);
		~T2ResourceManager();

		int CreateResource(const char* resourceName, void* pData, t_CreateDestroyCallbk a_fnCreateDestroyCallbk, void* pCallbkData);
		void DestroyResource(int a_iID);
		
		TBOOL ReloadResource(const char* pName, const char* pFilepath);

		void IncRefCount(int a_iID);
		void DecRefCount(int a_iID);
		
		T2ResourcePtr FindResource(int& iOutResource, const char* pName);
		int FindUnusedResource();

		T2ResourceData* GetResourceData(int a_iID);
		void* GetData(int a_iID);

		int GetMaxNumResources() const { return m_iMaxNumResources; }
		int GetNumUsedResources() const { return m_iNumUsedResources; }

	private:
		int m_iMaxNumResources;
		int m_iNumUsedResources;
		int m_iUnk;
		T2ResourceData* m_pData;
	};

	class T2ResourceData
	{
	public:
		static constexpr int MAX_RESOURCE_NAME = 48;

		friend class T2ResourceManager;

	public:
		enum FLAG : uint8_t
		{
			FLAG_LOADED = BITFIELD(0),
			FLAG_LOADING = BITFIELD(1),
			FLAG_INITIALISED = BITFIELD(2),
			FLAG_DESTROYED = BITFIELD(3),
		};

		void Init(const char* a_pName, T2ResourceManager::t_CreateDestroyCallbk a_fnCreateDestroyCallbk, void* a_pCallbkData);
		void DeInit();

		void Load(const char* filepath);
		void Unload();

		void SetLoadedData(void* a_pData);
		void* GetData();

		const char* GetName() const
		{
			return m_pResourceName;
		}

		TBOOL HasFlag(uint8_t flag) const
		{
			return (m_iFlags & flag) == flag;
		}

		TBOOL HasAnyFlag(uint8_t flag) const
		{
			return (m_iFlags & flag) != 0;
		}

	private:
		void* m_pData;
		TTRB* m_pTRB1;
		int m_iFlags;
		int m_iRefCount;
		char m_pResourceName[48];
		void* m_pCreateDestroyCallbkData;
		T2ResourceManager::t_CreateDestroyCallbk m_fnCreateDestroyCallbk;
		TTRB* m_pTRB2;
	};

	class T2ResourcePtr
	{
	public:
		static constexpr int IDINVALID = 0;

	public:
		T2ResourcePtr()
		{
			m_iResourceID = IDINVALID;
			T2ResourceManager::GetSingleton()->IncRefCount(IDINVALID);
		}

		T2ResourcePtr(int a_iID)
		{
			m_iResourceID = a_iID;
			T2ResourceManager::GetSingleton()->IncRefCount(m_iResourceID);
		}

		explicit T2ResourcePtr(const T2ResourcePtr& other)
		{
			m_iResourceID = other.m_iResourceID;
			T2ResourceManager::GetSingletonSafe()->IncRefCount(m_iResourceID);
		}

		~T2ResourcePtr()
		{
			T2ResourceManager::GetSingleton()->DecRefCount(m_iResourceID);
		}

		void operator=(const T2ResourcePtr& other)
		{
			T2ResourceManager::GetSingletonSafe()->DecRefCount(m_iResourceID);
			m_iResourceID = other.m_iResourceID;
			T2ResourceManager::GetSingletonSafe()->IncRefCount(m_iResourceID);
		}

		void* GetData() const
		{
			return T2ResourceManager::GetSingletonSafe()->GetData(m_iResourceID);
		}

		int GetID() const
		{
			return m_iResourceID;
		}

		operator int() const
		{
			return m_iResourceID;
		}

	protected:
		int m_iResourceID;
	};

	template <class T>
	class T2ResPtr : protected T2ResourcePtr
	{
	public:
		T2ResPtr(int a_iID = IDINVALID) : T2ResourcePtr(a_iID) { }
		T2ResPtr(const T2ResPtr& a_rOther) : T2ResourcePtr(a_rOther.m_iResourceID) { }
		T2ResPtr(T2ResPtr&& a_rOther) : T2ResourcePtr(a_rOther.m_iResourceID) { a_rOther.m_iResourceID = IDINVALID; }

		void operator=(const T2ResPtr& a_rOther)
		{
			T2ResourceManager::GetSingletonSafe()->DecRefCount(m_iResourceID);
			m_iResourceID = a_rOther.m_iResourceID;
			T2ResourceManager::GetSingletonSafe()->IncRefCount(m_iResourceID);
		}
		
		int GetID() const { return m_iResourceID; }
		operator int() const { return m_iResourceID; }

		T* GetData() const { return TSTATICCAST(T*, T2ResourcePtr::GetData()); }
		T* operator->() const { return TSTATICCAST(T*, T2ResourcePtr::GetData()); }
	};

	class T2Resource
	{
	public:
		void CreateResource(const char* resourceName, void* pData, T2ResourceManager::t_CreateDestroyCallbk a_fnCreateDestroyCallbk, void* pCallbkData);
		
		void DestroyResource()
		{
			TASSERT(m_iID != T2ResourcePtr::IDINVALID);
			T2ResourceManager::GetSingletonSafe()->DestroyResource(m_iID);
		}

		T2ResourcePtr GetResourcePtr() const
		{
			return T2ResourcePtr(m_iID);
		}

		template <class T>
		T2ResPtr<T> GetResPtr() const
		{
			return T2ResPtr<T>(m_iID);
		}

		int GetResourceId()
		{
			return m_iID;
		}

		operator int() const
		{
			return m_iID;
		}

	private:
		int m_iID = 0;
	};
}