#pragma once
#include <Toshi/TPString8.h>

TOSHI_NAMESPACE_START

//-----------------------------------------------------------------------------
// Purpose: interface of the renderer side texture store of the GUI. Texture
// sections refer to textures with the IDs it gives to the texture names.
//-----------------------------------------------------------------------------
class TGUITextureFactory : public TObject
{
public:
	TDECLARE_CLASS( TGUITextureFactory, TObject );

	// NOTE: the values are unknown
	enum TextureFlag : TINT
	{
	};

public:
	TGUITextureFactory();
	TGUITextureFactory( const TGUITextureFactory& a_rcOther );
	~TGUITextureFactory();

	// Returns the ID of the texture a_rcName (implementations load it if needed), -1 if it can't be found
	virtual TINT16 GetTextureID( const TPString8& a_rcName ) = 0;

	// Returns the ID of the texture a_rcName if it's already known, -1 otherwise
	virtual TINT16 FindTextureID( const TPString8& a_rcName ) = 0;

	virtual const TPString8& GetTextureName( TINT16 a_iID ) const                                     = 0;
	virtual TINT16           GetTextureWidth( TINT16 a_iID ) const                                    = 0;
	virtual TINT16           GetTextureHeight( TINT16 a_iID ) const                                   = 0;
	virtual TBOOL            GetTextureFlag( TINT16 a_iID, TextureFlag a_eFlag ) const                = 0;
	virtual void             SetTextureFlag( TINT16 a_iID, TextureFlag a_eFlag, TBOOL a_bValue )      = 0;
	virtual TINT16           GetFirstTextureID() const                                                = 0;
	virtual TINT16           GetNextTextureID( TINT16 a_iID ) const                                   = 0;
};

TOSHI_NAMESPACE_END
