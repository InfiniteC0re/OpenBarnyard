#pragma once
#include <Plugins/PPropertyParser/PPropertyBlock.h>

TOSHI_NAMESPACE_START

//-----------------------------------------------------------------------------
// Purpose: definition block of the "[strings]" block: name = "string"; pairs added to the strings
// of the TGUIManager.
//-----------------------------------------------------------------------------
class TGUIStringBlock : public PPropertyBlock
{
public:
	TDECLARE_CLASS( TGUIStringBlock, PPropertyBlock );

public:
	TGUIStringBlock();
	~TGUIStringBlock();

	virtual TBOOL Load( PPropertyReader* a_pReader ) override;
	virtual TBOOL Save( PPropertyWriter* a_pWriter ) override;

};

TOSHI_NAMESPACE_END
