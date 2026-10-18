#pragma once
#include <Toshi/TPString8.h>

TOSHI_NAMESPACE_START

//-----------------------------------------------------------------------------
// Purpose: interface of the renderer side font store of the GUI, gives IDs
// to the font names (see TGUIManager::AddFont).
//-----------------------------------------------------------------------------
class TGUIFontFactory : public TObject
{
public:
	TDECLARE_CLASS( TGUIFontFactory, TObject );

public:
	TGUIFontFactory();
	TGUIFontFactory( const TGUIFontFactory& a_rcOther );
	~TGUIFontFactory();

	// Returns the ID of the font a_rcName (implementations create it if needed)
	virtual TINT GetFontID( const TPString8& a_rcName ) = 0;

	// Returns the ID of the font a_rcName if it's already known, -1 otherwise
	virtual TINT FindFontID( const TPString8& a_rcName ) = 0;

	virtual const TPString8& GetFontName( TINT a_iID ) = 0;
};

TOSHI_NAMESPACE_END
