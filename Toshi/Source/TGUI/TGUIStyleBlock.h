#pragma once
#include <Plugins/PPropertyParser/PPropertyBlock.h>

TOSHI_NAMESPACE_START

//-----------------------------------------------------------------------------
// Purpose: definition block of the "[style]" block: a TGUIStyle, added to the
// style instances of the TGUIManager.
//-----------------------------------------------------------------------------
class TGUIStyleBlock : public PPropertyBlock
{
public:
	TDECLARE_CLASS( TGUIStyleBlock, PPropertyBlock );

public:
	TGUIStyleBlock();
	~TGUIStyleBlock();

	virtual TBOOL Load( PPropertyReader* a_pReader ) override;
	virtual TBOOL Save( PPropertyWriter* a_pWriter ) override;
};

TOSHI_NAMESPACE_END
