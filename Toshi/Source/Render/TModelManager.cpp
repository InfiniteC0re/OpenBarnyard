#include "ToshiPCH.h"
#include "TModelManager.h"
#include "TRenderInterface.h"

#include "Toshi/TSceneObject.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

// $Barnyard: FUNCTION 006c1c90
TManagedModel::TManagedModel()
    : m_pModel( TNULL )
{
	static TBOOL s_bFilledList = TFALSE;

	if ( !s_bFilledList )
	{
		for ( TUINT i = 0; i < TModelManager::MAX_NUM_MODELS; i++ )
		{
			TModelManager::ms_oFreeList.PushBack( &TModelManager::ms_pEntries[ i ] );
		}

		s_bFilledList = TTRUE;
	}

	m_pEntry = TNULL;
}

// $Barnyard: FUNCTION 006c1ce0
TManagedModel::~TManagedModel()
{
	if ( m_pModel )
	{
		if ( m_pEntry )
		{
			if ( m_pEntry->GetRefCount() == 1 )
			{
				m_pModel->Delete();
				m_pEntry->Remove();
				TModelManager::ms_oFreeList.PushBack( m_pEntry );

				m_pModel = TNULL;
				m_pEntry = TNULL;
			}
			else
			{
				m_pEntry->DecRefCount();
				m_pModel = TNULL;
				m_pEntry = TNULL;
			}
		}
		else
		{
			m_pModel->Delete();
			m_pModel = TNULL;
			m_pEntry = TNULL;
		}
	}
}

// $Barnyard: FUNCTION 006c1c00
void TModelManager::Initialise()
{
	TASSERT( ms_pEntries == TNULL );
	ms_pEntries = new ModelNode[ MAX_NUM_MODELS ];
}

// $Barnyard: FUNCTION 006c1c50
void TModelManager::Uninitialise()
{
	TASSERT( ms_pEntries != TNULL );
	ms_oUsedList.Clear();
	ms_oFreeList.Clear();

	delete[] ms_pEntries;
	ms_pEntries = TNULL;
}

// $Barnyard: FUNCTION 006c1d40
TModelManager::ModelNode* TModelManager::CreateModel( const TCHAR* a_szFileName, TManagedModel& a_rModelRef, TTRB* a_pAssetTRB )
{
	if ( ms_oFreeList.IsEmpty() )
	{
		TASSERT( TFALSE, "Couldn't create new model since the registry is all used" );
		return TNULL;
	}

	TCHAR filepath[ 248 ];
	auto  iFileNameLen = TStringManager::String8Length( a_szFileName );
	TStringManager::String8Copy( filepath, a_szFileName );

	for ( TUINT i = 0; i < iFileNameLen; i++ )
	{
		if ( filepath[ i ] == '/' )
		{
			filepath[ i ] = '\\';
		}
	}

	auto crc32 = TUtil::CRC32( TREINTERPRETCAST( TBYTE*, filepath ), iFileNameLen );

	for ( auto it = ms_oUsedList.Begin(); it != ms_oUsedList.End(); ++it )
	{
		if ( it->GetCRC32() == crc32 )
		{
			a_rModelRef.m_pModel = it->GetModel();
			it->IncRefCount();

			return it;
		}
	}

	auto pEntry = ms_oFreeList.PopBack();
	ms_oUsedList.PushFront( pEntry );

	const TCHAR* szFileName;
	TUINT8       ui8NameLen = 0;

	if ( !a_pAssetTRB || !TModel::GetSkeletonAssetSymbolName( filepath, szFileName, ui8NameLen, a_pAssetTRB ) )
	{
		a_pAssetTRB = TNULL;
		szFileName  = filepath;
		ui8NameLen  = -1;
	}

	auto pModel = TRenderInterface::GetSingleton()->CreateModel( szFileName, TTRUE, a_pAssetTRB, ui8NameLen );
	pEntry->Create( crc32, pModel );
	pEntry->IncRefCount();
	a_rModelRef.m_pModel = pModel;

	return pEntry;
}

// $Barnyard: FUNCTION 006c1fd0
TBOOL TManagedModel::Create( const TCHAR* a_szFileName, TTRB* a_pTRB )
{
	m_pModel = TNULL;
	m_pEntry = TModelManager::CreateModel( a_szFileName, *this, a_pTRB );

	return m_pModel != TNULL;
}

TSceneObject* TManagedModel::CreateSceneObject()
{
	TSceneObject* pSceneObject = new TSceneObject();
	pSceneObject->Create( this );

	return pSceneObject;
}

TOSHI_NAMESPACE_END
