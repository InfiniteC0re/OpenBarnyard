#pragma once
#include <Plugins/PPropertyParser/PPropertyBlock.h>

TOSHI_NAMESPACE_START

//-----------------------------------------------------------------------------
// Purpose: definition block of the "[textureanimations]" block: one animation per line
// (name texture frequency x y width height rotation wrap mirrorx mirrory
// { { x, y, width, height }, ... };).
//-----------------------------------------------------------------------------
class TGUITextureAnimationBlock : public PPropertyBlock
{
public:
	TDECLARE_CLASS( TGUITextureAnimationBlock, PPropertyBlock );

public:
	TGUITextureAnimationBlock();
	~TGUITextureAnimationBlock();

	virtual TBOOL Load( PPropertyReader* a_pReader ) override;
	virtual TBOOL Save( PPropertyWriter* a_pWriter ) override;

};

TOSHI_NAMESPACE_END
