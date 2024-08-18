#pragma once
#include "TModel.h"
#include "Toshi/T2DList.h"

TOSHI_NAMESPACE_START

class T2ModelInstance;
class TModelPtr;

class TModelManager
{
public:
	class ModelNode :
	    public T2DList<ModelNode>::Node
	{
	public:
		ModelNode()
		{
			m_CRC32      = 0;
			m_uiRefCount = 0;
			m_pModel     = TNULL;
		}

		ModelNode( TUINT32 a_CRC32, TModel* a_pModel )
		{
			Create( a_CRC32, a_pModel );
		}

		TUINT IncRefCount()
		{
			return ++m_uiRefCount;
		}

		TUINT DecRefCount()
		{
			// TODO: Add something more?
			TASSERT( m_uiRefCount > 0 );
			return --m_uiRefCount;
		}

		void Create( TUINT32 a_CRC32, TModel* a_pModel )
		{
			m_CRC32      = a_CRC32;
			m_uiRefCount = 0;
			m_pModel     = a_pModel;
		}

		TUINT32 GetCRC32() const { return m_CRC32; }
		TUINT   GetRefCount() const { return m_uiRefCount; }
		TModel* GetModel() const { return m_pModel; }

		ModelNode& operator=( const ModelNode& a_rOther )
		{
			m_CRC32      = a_rOther.m_CRC32;
			m_uiRefCount = a_rOther.m_uiRefCount;
			m_pModel     = a_rOther.m_pModel;
		}

	private:
		TUINT32 m_CRC32;
		TUINT   m_uiRefCount;
		TModel* m_pModel;
	};

	static constexpr TUINT MAX_NUM_MODELS = 500;

public:
	static void Initialise();
	static void Uninitialise();

	static ModelNode* CreateModel( const TCHAR* a_szFileName, TModelPtr& a_rModelRef, TTRB* a_pAssetTRB );

public:
	inline static ModelNode*         ms_pEntries;
	inline static T2DList<ModelNode> ms_oUsedList;
	inline static T2DList<ModelNode> ms_oFreeList;
};

class TModelPtr
{
public:
	friend TModelManager;

public:
	TModelPtr() = default;
	~TModelPtr();

	TBOOL            Create( const TCHAR* a_szFileName, TTRB* a_pTRB );
	T2ModelInstance* CreateInstance();

	void SetModel( TModel* a_pModel ) { m_pModel = a_pModel; }

	TModel*                   GetModel() const { return m_pModel; }
	TModelManager::ModelNode* GetRegistryEntry() const { return m_pEntry; }

private:
	TModel*                   m_pModel = TNULL;
	TModelManager::ModelNode* m_pEntry = TNULL;
};

TOSHI_NAMESPACE_END
