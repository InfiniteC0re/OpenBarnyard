#pragma once
#include <TGUI/TGUIFontFactory.h>
#include <Toshi/TArray.h>

class PGUITRDisplayContext;

namespace Toshi
{
class TGUIFont;
class TGUIManager;
} // namespace Toshi

//-----------------------------------------------------------------------------
// Purpose: font store of the TRender GUI renderer. It only gives IDs to the
// font names, the glyphs of a TGUIFont refer to textures of the texture
// factory (see PGUITRDisplayContext::DrawLabel).
//-----------------------------------------------------------------------------
class PGUITRFontFactory : public Toshi::TGUIFontFactory
{
public:
	TDECLARE_CLASS( PGUITRFontFactory, Toshi::TGUIFontFactory );

	class Font
	{
	public:
		// $JPOG: FUNCTION PGUIRenderer.dll+000060f0
		Font() {}

		// $JPOG: FUNCTION PGUIRenderer.dll+000060d0
		Font( const Toshi::TPString8& a_rcName )
		    : m_strName( a_rcName )
		{}

		// $JPOG: FUNCTION PGUIRenderer.dll+00005b10
		~Font() {}

		// $JPOG: FUNCTION PGUIRenderer.dll+00006090
		Font& operator=( const Font& a_rcOther )
		{
			m_strName = a_rcOther.m_strName;
			return *this;
		}

		// $JPOG: FUNCTION PGUIRenderer.dll+00005b40
		const Toshi::TPString8& GetName() const { return m_strName; }

	private:
		Toshi::TPString8 m_strName; // 0x0
	};

public:
	PGUITRFontFactory();
	~PGUITRFontFactory();

	//-----------------------------------------------------------------------------
	// Toshi::TGUIFontFactory
	//-----------------------------------------------------------------------------
	virtual TINT                    GetFontID( const Toshi::TPString8& a_rcName ) override;
	virtual TINT                    FindFontID( const Toshi::TPString8& a_rcName ) override;
	virtual const Toshi::TPString8& GetFontName( TINT a_iID ) override;

	//-----------------------------------------------------------------------------
	// PGUITRFontFactory
	//-----------------------------------------------------------------------------
	virtual TBOOL IsFontIDValid( TINT a_iID ) const;

	void Create( PGUITRDisplayContext* a_pDisplayContext );

	// NOTE: Barnyard route added on top (TGUIManager::SetFontFallback): makes a TGUIFont of a font of
	// T2GUIFontManager (Barnyard's Data/GUI/fonts.trb), the glyphs refer to the textures of the font
	static Toshi::TGUIFont* CreateFontFromT2GUIFont( Toshi::TGUIManager* a_pManager, const Toshi::TPString8& a_rcName );

	// $JPOG: FUNCTION PGUIRenderer.dll+00006100
	Font* GetFont( TINT a_iID ) const { return m_aFonts[ a_iID ]; }

	// $JPOG: FUNCTION PGUIRenderer.dll+00005b50
	TINT GetFontCount() const { return m_aFonts.Size(); }

private:
	Toshi::TArray<Font*>  m_aFonts;          // 0x04
	PGUITRDisplayContext* m_pDisplayContext; // 0x14
};
