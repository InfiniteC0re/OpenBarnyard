#pragma once
#include "TModel.h"
#include "Toshi/T2DList.h"
#include "Toshi/T2SimpleArray.h"

namespace Toshi {

	class T2ModelInstance;

	class TModelRegistryEntry :
		public T2DList<TModelRegistryEntry>::Node
	{
	public:
		TModelRegistryEntry()
		{
			m_CRC32 = 0;
			m_uiRefCount = 0;
			m_pModel = TNULL;
		}

		TModelRegistryEntry(TUINT32 a_CRC32, TModel* a_pModel)
		{
			Create(a_CRC32, a_pModel);
		}

		TUINT IncRefCount()
		{
			return ++m_uiRefCount;
		}

		TUINT DecRefCount()
		{
			// TODO: Add something more?
			TASSERT(m_uiRefCount > 0);
			return --m_uiRefCount;
		}

		void Create(TUINT32 a_CRC32, TModel* a_pModel)
		{
			m_CRC32 = a_CRC32;
			m_uiRefCount = 0;
			m_pModel = a_pModel;
		}

		TUINT32 GetCRC32() const { return m_CRC32; }
		TUINT GetRefCount() const { return m_uiRefCount; }
		TModel* GetModel() const { return m_pModel; }

		TModelRegistryEntry& operator=(const TModelRegistryEntry& a_rOther)
		{
			m_CRC32 = a_rOther.m_CRC32;
			m_uiRefCount = a_rOther.m_uiRefCount;
			m_pModel = a_rOther.m_pModel;
		}

	private:
		TUINT32 m_CRC32;
		TUINT m_uiRefCount;
		TModel* m_pModel;
	};

	class TModelPtr;

	class TModelRegistry
	{
	public:
		static constexpr TUINT MAX_NUM_MODELS = 500;

	public:
		static void Initialise();
		static void Uninitialise();

		static TModelRegistryEntry* CreateModel(const TCHAR* a_szFileName, TModelPtr& a_rModelRef, TTRB* a_pAssetTRB);

	public:
		inline static T2SimpleArray<TModelRegistryEntry> ms_pEntries;
		inline static T2DList<TModelRegistryEntry> ms_oUsedList;
		inline static T2DList<TModelRegistryEntry> ms_oFreeList;
	};

	class TModelPtr
	{
	public:
		friend TModelRegistry;

	public:
		TModelPtr() = default;
		~TModelPtr();

		TBOOL Create(const TCHAR* a_szFileName, TTRB* a_pTRB);
		T2ModelInstance* CreateInstance();

		void SetModel(TModel* a_pModel) { m_pModel = a_pModel; }

		TModel* GetModel() const { return m_pModel; }
		TModelRegistryEntry* GetRegistryEntry() const { return m_pEntry; }

	private:
		TModel* m_pModel = TNULL;
		TModelRegistryEntry* m_pEntry = TNULL;
	};

}
