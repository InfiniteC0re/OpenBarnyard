#pragma once
#include <Plugins/PPropertyParser/PPropertyBlock.h>

TOSHI_NAMESPACE_START

//-----------------------------------------------------------------------------
// Purpose: definition block of the "[font]" block: a TGUIFont ("type = TGUIFont;" and its properties).
//-----------------------------------------------------------------------------
class TGUIFontBlock : public PPropertyBlock
{
public:
	TDECLARE_CLASS( TGUIFontBlock, PPropertyBlock );

public:
	TGUIFontBlock();
	~TGUIFontBlock();

	virtual TBOOL Load( PPropertyReader* a_pReader ) override;
	virtual TBOOL Save( PPropertyWriter* a_pWriter ) override;

};

TOSHI_NAMESPACE_END
