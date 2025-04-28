#include "ToshiPCH.h"
#include "TVertexFactoryResource_DX8.h"
#include "TRenderInterface_DX8.h"
#include "TVertexPoolResource_DX8.h"

#include "Toshi/T2Pair.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

TDEFINE_CLASS( TVertexFactoryResource );

// $Barnyard: FUNCTION 006d7910
TVertexPoolResourceInterface* TVertexFactoryResource::CreatePoolResource( TUINT16 a_uiMaxStaticVertices, TUINT16 a_uiFlags )
{
	auto pVertexPool = TSTATICCAST(
	    TVertexPoolResource,
	    GetRenderer()->CreateResource( &TGetClass( TVertexPoolResource ), TNULL, this )
	);

	TVALIDPTR( pVertexPool );

	pVertexPool->Create( this, a_uiMaxStaticVertices, a_uiFlags );
	CreatePool( a_uiMaxStaticVertices, a_uiFlags );

	return pVertexPool;
}

// $Barnyard: FUNCTION 006d78d0
TVertexBlockResource* TVertexFactoryResource::CreateBlockResource( TUINT16 a_uiMaxVertices, TUINT32 a_uiFlags )
{
	auto pVertexBlock = TSTATICCAST(
	    TVertexBlockResource,
	    GetRenderer()->CreateResource( &TGetClass( TVertexBlockResource ), TNULL, this )
	);

	TVALIDPTR( pVertexBlock );

	pVertexBlock->Create( this, a_uiMaxVertices, a_uiFlags );
	return pVertexBlock;
}

// $Barnyard: FUNCTION 006d7890
TVertexBlockResource* TVertexFactoryResource::FindBlockResource( TVertexPoolResource* a_pResource )
{
	if ( a_pResource->GetFlags() & 2 )
	{
		return TNULL;
	}

	using Pair  = T2Pair<TVertexBlockResource*, TVertexPoolResource*>;
	Pair result = { TNULL, a_pResource };

	// $Barnyard: FUNCTION 006d7850
	TResource::RecurseSimple(
	    []( TResource* a_pResource, void* a_pUserData ) {
		    auto pPair = TSTATICCAST( Pair, a_pUserData );

		    if ( a_pResource->IsA( &TGetClass( TVertexBlockResource ) ) )
		    {
			    auto pBlockResource = TSTATICCAST( TVertexBlockResource, a_pResource );

			    if ( pBlockResource->CanFit( pPair->second ) && !pBlockResource->IsDying() )
			    {
				    pPair->first = pBlockResource;
				    return TFALSE;
			    }
		    }

		    return TTRUE;
	    },
	    this,
	    &result
	);

	return result.first;
}

TOSHI_NAMESPACE_END
