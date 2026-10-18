#pragma once
#include <Plugins/PPropertyParser/PPropertyBlock.h>

TOSHI_NAMESPACE_START

//-----------------------------------------------------------------------------
// Purpose: definition block of the "[texturesections]" block: one texture section per line
// (name texture x y width height rotation wrap mirrorx mirrory;).
//-----------------------------------------------------------------------------
class TGUITextureSectionBlock : public PPropertyBlock
{
public:
	TDECLARE_CLASS( TGUITextureSectionBlock, PPropertyBlock );

public:
	TGUITextureSectionBlock();
	~TGUITextureSectionBlock();

	virtual TBOOL Load( PPropertyReader* a_pReader ) override;
	virtual TBOOL Save( PPropertyWriter* a_pWriter ) override;

};

TOSHI_NAMESPACE_END
