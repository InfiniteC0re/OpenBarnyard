#pragma once

TOSHI_NAMESPACE_START

//-----------------------------------------------------------------------------
// Purpose: manager of the effects of the GUI.
//
// NOTE: empty in JPOG and Nicktoons (Nicktoons creates its TClass in the
// static initialiser of TGUIFunction).
//-----------------------------------------------------------------------------
class TGUIEffectManager : public TObject
{
public:
	TDECLARE_CLASS( TGUIEffectManager, TObject );

public:
	TGUIEffectManager();
	~TGUIEffectManager();
};

TOSHI_NAMESPACE_END
