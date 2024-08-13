#include "ToshiPCH.h"
#include "TModel_DX8.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

TBOOL TModelHAL::Create( TTMD* a_pTMD, TBOOL a_bLoad )
{
	m_eFlags |= Flags_Created;

	if ( a_bLoad )
	{
		if ( !LoadTMDHAL( a_pTMD ) )
		{
			m_eFlags &= ~Flags_Created;
			return TFALSE;
		}
	}

	return TTRUE;
}

TBOOL TModelHAL::Create( const TCHAR* a_szFileName, TBOOL a_bLoad )
{
	m_eFlags |= Flags_Created;

	if ( a_bLoad )
	{
		if ( !LoadFromFile( a_szFileName ) )
		{
			m_eFlags &= ~Flags_Created;
			return TFALSE;
		}
	}

	return TTRUE;
}

TBOOL TModelHAL::Create( const TCHAR* a_szFileName, TBOOL a_bLoad, TTRB* a_pAssetTRB, TUINT8 a_ui8FileNameLen )
{
	m_eFlags |= Flags_Created;

	if ( a_bLoad )
	{
		if ( !LoadTMDHAL( a_szFileName, a_pAssetTRB, a_ui8FileNameLen ) )
		{
			m_eFlags &= ~Flags_Created;
			return TFALSE;
		}
	}

	return TTRUE;
}

void TModelHAL::Delete()
{
	Unload();
	delete this;
}

TBOOL TModelHAL::LoadFromFile( const TCHAR* a_szFileName )
{
	if ( !IsLoaded() )
	{
		TASSERT( TFALSE, "Deprecated!" );
		return TFALSE;
	}

	return TTRUE;
}

void TModelHAL::Unload()
{
	if ( m_pSkeleton )
	{
		m_pSkeleton->GetKeyLibraryInstance().Destroy();
		m_pSkeleton = TNULL;
	}

	for ( TINT i = 0; i < m_iLODCount; i++ )
	{
		if ( m_LODs[ i ].ppMeshes )
		{
			for ( TINT k = 0; k < m_LODs[ i ].iNumMeshes; k++ )
			{
				if ( m_LODs[ i ].ppMeshes[ k ] )
				{
					m_LODs[ i ].ppMeshes[ k ]->DestroyResource();
					m_LODs[ i ].ppMeshes[ k ] = TNULL;
				}
			}

			delete[] m_LODs[ i ].ppMeshes;
		}
	}

	UnloadTRB();
}

TBOOL TModelHAL::GetUnknownFlag()
{
	return TTRUE;
}

TBOOL TModelHAL::LoadTMDHAL( TTMD* a_pTMD )
{
	if ( !IsLoaded() )
	{
		TASSERT( TFALSE, "Deprecated!" );
		return TFALSE;
	}

	return TTRUE;
}

TBOOL TModelHAL::LoadTMDHAL( const TCHAR* a_szFileName, TTRB* a_pAssetTRB, TUINT8 a_ui8FileNameLen )
{
	if ( !IsLoaded() )
	{
		if ( !LoadTRB( a_szFileName, a_pAssetTRB, a_ui8FileNameLen ) )
		{
			return TFALSE;
		}

		m_eFlags |= Flags_Loaded;
	}

	return TTRUE;
}

TOSHI_NAMESPACE_END
