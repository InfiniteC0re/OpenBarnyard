#pragma once
#include <Plugins/PPropertyParser/PPropertyBlock.h>

TOSHI_NAMESPACE_START

//-----------------------------------------------------------------------------
// Purpose: definition block of the "[textures]" block. The textures are loaded by the texture factory, the block is
// read and ignored.
//-----------------------------------------------------------------------------
class TGUITextureBlock : public PPropertyBlock
{
public:
	TDECLARE_CLASS( TGUITextureBlock, PPropertyBlock );

public:
	TGUITextureBlock();
	~TGUITextureBlock();

	virtual TBOOL Load( PPropertyReader* a_pReader ) override;
	virtual TBOOL Save( PPropertyWriter* a_pWriter ) override;

};

TOSHI_NAMESPACE_END
