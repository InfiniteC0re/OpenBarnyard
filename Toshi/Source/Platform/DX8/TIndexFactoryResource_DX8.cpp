#include "ToshiPCH.h"
#include "TIndexFactoryResource_DX8.h"
#include "TRenderInterface_DX8.h"
#include "TIndexBlockResource_DX8.h"
#include "Toshi/T2Pair.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

TDEFINE_CLASS( TIndexFactoryResource );

// destructor
// $Barnyard: FUNCTION 006d7b80

// $Barnyard: FUNCTION 006d7a50
TIndexBlockResource* TIndexFactoryResource::FindBlockResource( TIndexPoolResource* a_pResource )
{
	if ( a_pResource->GetFlags() & 2 )
	{
		return TNULL;
	}

	using Pair  = T2Pair<TIndexBlockResource*, TIndexPoolResource*>;
	Pair result = { TNULL, a_pResource };

	TResource::RecurseSimple(
	    []( TResource* a_pResource, void* a_pUserData ) {
		    auto pPair = TSTATICCAST( Pair, a_pUserData );

		    if ( a_pResource->IsA( &TGetClass( TIndexBlockResource ) ) )
		    {
			    auto pBlockResource = TSTATICCAST( TIndexBlockResource, a_pResource );

			    if ( pBlockResource->CanFit( pPair->GetSecond() ) && !pBlockResource->IsDying() )
			    {
				    pPair->m_First = pBlockResource;
				    return TFALSE;
			    }
		    }

		    return TTRUE;
	    },
	    this,
	    &result
	);

	return result.m_First;
}

// $Barnyard: FUNCTION 006d7a90
TIndexBlockResource* TIndexFactoryResource::CreateBlockResource( TUINT16 a_uiMaxIndices, TUINT32 a_uiFlags )
{
	auto pIndexBlock = TSTATICCAST(
	    TIndexBlockResource,
	    GetRenderer()->CreateResource( &TGetClass( TIndexBlockResource ), TNULL, this )
	);

	TVALIDPTR( pIndexBlock );

	pIndexBlock->Create( this, a_uiMaxIndices, a_uiFlags );
	return pIndexBlock;
}

// $Barnyard: FUNCTION 006d7ad0
TIndexPoolResourceInterface* TIndexFactoryResource::CreatePoolResource( TUINT16 a_uiMaxStaticIndices, TUINT16 a_uiFlags )
{
	auto pIndexPool = TSTATICCAST(
	    TIndexPoolResource,
	    GetRenderer()->CreateResource( &TGetClass( TIndexPoolResource ), TNULL, this )
	);

	TVALIDPTR( pIndexPool );

	pIndexPool->Create( this, a_uiMaxStaticIndices, a_uiFlags );
	CreatePool( a_uiMaxStaticIndices, a_uiFlags );

	return pIndexPool;
}

TOSHI_NAMESPACE_END
