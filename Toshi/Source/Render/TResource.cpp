#include "ToshiPCH.h"
#include "TResource.h"
#include "TRenderInterface.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

TDEFINE_CLASS( TResource );

// $Barnyard: FUNCTION 006d8240
TResource::TResource()
{
	m_State     = 0;
	m_UId       = 0;
	m_pRenderer = TNULL;
	m_Name[ 0 ] = 0;
}

// $Barnyard: FUNCTION 006d8220
TResource::~TResource()
{
	m_State |= TResourceState_Dead;
}

// $Barnyard: FUNCTION 006d8160
TBOOL TResource::Create()
{
	TASSERT( TFALSE == IsCreated(), "This resource is already created" );

	m_State |= TResourceState_Created;
	return TTRUE;
}

// $Barnyard: FUNCTION 006d8180
TBOOL TResource::Validate()
{
	TASSERT( IsDying() == TFALSE, "Resource is dying and cannot be validated" );
	TASSERT( Parent() == TNULL || Parent()->IsDying() == TFALSE, "Parent resource is dying" );

	if ( IsDying() ) return TFALSE;

	m_State |= TResourceState_Valid;
	return TTRUE;
}

// $Barnyard: FUNCTION 006bffd0
void TResource::Invalidate()
{
	m_State &= ~TResourceState_Valid;
}

// $Barnyard: FUNCTION 006d8210
void TResource::DestroyResource()
{
	GetRenderer()->DestroyResource( this );
}

TBOOL TResource::TryInvalidate()
{
	return TTRUE;
}

TBOOL TResource::TryValidate()
{
	return TTRUE;
}

// $Barnyard: FUNCTION 006d8170
void TResource::OnDestroy()
{
	TASSERT( TTRUE == IsCreated(), "Tried to destroy resource that wasn't created" );

	m_State &= ~TResourceState_Created;
	m_State |= TResourceState_Dying;
}

// $Barnyard: FUNCTION 006d82b0
void TResource::SetParent( TResource* a_pParent )
{
	auto pTree = TNode::Tree();

	pTree->Remove( this );

	if ( !a_pParent )
	{
		if ( pTree )
		{
			pTree->InsertAtRoot( this );
		}
	}
	else
	{
		pTree->Insert( a_pParent, this );
	}
}

// $Barnyard: FUNCTION 006d81a0
void TResource::SetName( const TCHAR* name )
{
	// 006b5350
	if ( name == TNULL )
	{
		name = "res:";

		TCHAR UIdStr[ 12 ];
		T2String8::IntToString( m_UId, UIdStr, 0xE, 10 );
	}

	TASSERT( TStringManager::String8Length( name ) <= MAXNAMELEN, "Name is too long" );
	TStringManager::String8Copy( m_Name, name, -1 );
}

// $Barnyard: FUNCTION 006d8400
TBOOL TResource::RecurseSimple( t_RecurseCb a_pCallback, TResource* a_pResource, void* a_pUserData )
{
	if ( a_pResource )
		return Recurse( a_pCallback, a_pResource, TFALSE, a_pUserData );
	else
		return Recurse( a_pCallback, Child(), TTRUE, a_pUserData );
}

// $Barnyard: FUNCTION 006beb70
TBOOL TResource::Recurse( t_RecurseCb a_pCallback, TResource* a_pResource, TBOOL a_bFlag, void* a_pUserData )
{
	TResource* pRes = a_pResource;

	if ( TNULL != a_pResource )
	{
		TResource* pNext;

		do
		{
			pNext = pRes->Next();

			if ( pNext == a_pResource || pNext == pRes || !a_bFlag )
				pNext = TNULL;

			if ( !a_pCallback( pRes, a_pUserData ) )
				return TFALSE;

			if ( pRes->Child() && !Recurse( a_pCallback, pRes->Child(), TTRUE, a_pUserData ) )
				return TFALSE;

			pRes = pNext;

		} while ( pNext != TNULL );
	}

	return TTRUE;
}

Toshi::TRenderInterface* TResource::GetRenderer() const
{
	return m_pRenderer;
}

void TResource::SetRenderer( TRenderInterface* pRenderer )
{
	m_pRenderer = pRenderer;
}

TUINT32 TResource::GetUId() const
{
	return m_UId;
}

void TResource::SetUId( TUINT32 uid )
{
	m_UId = uid;
}

TOSHI_NAMESPACE_END
