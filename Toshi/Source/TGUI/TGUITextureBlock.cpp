#include "ToshiPCH.h"
#include "TGUITextureBlock.h"

#include <Plugins/PPropertyParser/PPropertyReader.h>
#include <Plugins/PPropertyParser/PPropertyWriter.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

// NOTE: CreateObject returns TNULL in Nicktoons. Its static initialiser (00383f18) also creates the
// pooled strings "texture" and "allow_transparent", which nothing uses
TDEFINE_CLASS_NORUNTIME( TGUITextureBlock );

// $NicktoonsPS2: FUNCTION 00383e28
// $JPOG: FUNCTION TGuiInterface.dll+0002d870
TGUITextureBlock::TGUITextureBlock()
    : PPropertyBlock( TPString8( "textures" ), PPropertyBlock::Priority( 1 ) )
{
}

// $NicktoonsPS2: FUNCTION 00383eb0
// $JPOG: FUNCTION TGuiInterface.dll+0002db10
TGUITextureBlock::~TGUITextureBlock()
{
}

// $NicktoonsPS2: FUNCTION 00383ed8
// $JPOG: FUNCTION TGuiInterface.dll+0002d9e0
TBOOL TGUITextureBlock::Load( PPropertyReader* a_pReader )
{
	PProperties* pProperties = a_pReader->LoadPropertyBlock();

	if ( pProperties )
		delete pProperties;

	return TTRUE;
}

// NOTE: JPOG gets the texture factory and the string pool and writes nothing
// $NicktoonsPS2: FUNCTION 00383f10
// $JPOG: FUNCTION TGuiInterface.dll+0002daf0
TBOOL TGUITextureBlock::Save( PPropertyWriter* a_pWriter )
{
	return TTRUE;
}

TOSHI_NAMESPACE_END
