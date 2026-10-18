#pragma once
#include <Plugins/PPropertyParser/PPropertyBlock.h>

TOSHI_NAMESPACE_START

//-----------------------------------------------------------------------------
// Purpose: definition block of the "[dialog]" block: a top-level TGUIWidget,
// created from the class named in the block and added to the top-level
// instances of the TGUIManager.
//-----------------------------------------------------------------------------
class TGUIDialogBlock : public PPropertyBlock
{
public:
	TDECLARE_CLASS( TGUIDialogBlock, PPropertyBlock );

public:
	TGUIDialogBlock();
	~TGUIDialogBlock();

	virtual TBOOL Load( PPropertyReader* a_pReader ) override;
	virtual TBOOL Save( PPropertyWriter* a_pWriter ) override;
};

TOSHI_NAMESPACE_END
