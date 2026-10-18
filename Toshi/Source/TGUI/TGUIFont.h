#pragma once
#include "TGUIDeserialisable.h"
#include "TGUIFontGlyph.h"
#include "TGUIFormattedText.h"

#include <Toshi/TNodeList.h>
#include <Toshi/TArray.h>
#include <Toshi/TString8.h>
#include <Toshi/TString16.h>

TOSHI_NAMESPACE_START

class TGUIManager;

//-----------------------------------------------------------------------------
// Purpose: bitmap font of the GUI ("[font]" blocks, "type = TGUIFont;"): a
// name, a height, a spacing, the textures of the glyphs and the glyphs, which
// are found by character with a hash table.
//
// NOTE: only in JPOG (Nicktoons has no TGUIFont, its [font] blocks are
// skipped). JPOG's SetInterface/GetInterface keep the TGUIManager here.
//-----------------------------------------------------------------------------
class TGUIFont
    : public TGUIDeserialisable
    , public TNodeList<TGUIFont>::TNode
{
public:
	TDECLARE_CLASS( TGUIFont, TGUIDeserialisable );

	using GlyphList = TNodeList<TGUITextureSection>;

public:
	TGUIFont();
	TGUIFont( const TGUIFont& a_rcOther );
	~TGUIFont();

	virtual TBOOL        Deserialise( const PProperties* a_pProperties ) override;
	virtual TBOOL        Serialise( PProperties* a_pProperties ) override;
	virtual PropertySet* GetPropertyInfoSet() const override;
	virtual PropertySet* GetPropertyInfoOrderedSet() const override;
	virtual TGUIFont*    Clone() const;

	TBOOL Create( const TPString8& a_rcName, TINT a_iFontHeight, TINT a_iSpacing );

	void AddGlyph( TGUIFontGlyph* a_pGlyph );
	void RemoveGlyph( TGUIFontGlyph* a_pGlyph );

	// Adds a texture to the texture list (if it isn't in it yet)
	void AddTexture( TINT16 a_iTexture );

	// Removes a texture and deletes the glyphs that use it
	void RemoveTexture( TINT16 a_iTexture );

	// Rebuilds the texture list from the textures of the glyphs
	void CleanupTextures();

	TGUIFontGlyph* GetGlyph( TUINT16 a_uiCharacter ) const;
	TGUIFontGlyph* GetGlyph( const TPString8& a_rcName ) const;
	TBOOL          IsCharacterDefined( TUINT16 a_uiCharacter ) const;
	TUINT16        GetLowestCharacter() const;
	TUINT16        GetHighestCharacter() const;

	void GetStringDimensions( const TString16& a_rcString, TINT& a_rWidth, TINT& a_rHeight ) const;
	void GetCharacterDimensions( TUINT16 a_uiCharacter, TINT& a_rWidth, TINT& a_rHeight ) const;

	// How the texts are broken into lines
	enum BreakMode : TINT
	{
		BreakMode_European, // at the spaces
		BreakMode_Japanese, // between any two characters, except before/after some punctuation
	};

	// Breaks a_rcString into lines of a_iMaxWidth (no maximum if -1), adds them to a_pText if it isn't TNULL
	// and returns the size of the text in a_rWidth/a_rHeight
	void BreakString( const TString16& a_rcString, TGUIFormattedText* a_pText, TGUIAlignment a_eAlignment, const TGUIColour& a_rcColour, TINT& a_rWidth, TINT& a_rHeight, TINT a_iMaxWidth, TINT a_iMaxHeight ) const;
	void BreakStringEuropean( const TString16& a_rcString, TGUIFormattedText* a_pText, TGUIAlignment a_eAlignment, const TGUIColour& a_rcColour, TINT& a_rWidth, TINT& a_rHeight, TINT a_iMaxWidth, TINT a_iMaxHeight ) const;
	void BreakStringJapanese( const TString16& a_rcString, TGUIFormattedText* a_pText, TGUIAlignment a_eAlignment, const TGUIColour& a_rcColour, TINT& a_rWidth, TINT& a_rHeight, TINT a_iMaxWidth, TINT a_iMaxHeight ) const;

	// NOTE: JPOG reads the break mode from the locale of the system, TOSHI 2.0's T2Locale doesn't have it
	static void      SetBreakMode( BreakMode a_eBreakMode ) { ms_eBreakMode = a_eBreakMode; }
	static BreakMode GetBreakMode() { return ms_eBreakMode; }

	// "c" for printable ASCII characters, "U+xxxx" otherwise
	static void  CharToString( TUINT16 a_uiCharacter, TString8& a_rOutString );
	static TBOOL StringToChar( const TString8& a_rcString, TUINT16& a_rOutCharacter );

	// $JPOG: FUNCTION TGuiInterface.dll+0000e900
	static void SetScale( TFLOAT a_fHorizScale, TFLOAT a_fVertScale )
	{
		ms_fHorizScale = a_fHorizScale;
		ms_fVertScale  = a_fVertScale;
	}

	// Japanese characters that can't start/end a line (BreakStringJapanese)
	static TBOOL IsJapaneseNoPreBreak( TUINT16 a_uiCharacter );
	static TBOOL IsJapaneseNoPostBreak( TUINT16 a_uiCharacter );

	// $JPOG: FUNCTION TGuiInterface.dll+0000e930
	void SetManager( TGUIManager* a_pManager ) { m_pManager = a_pManager; }

	// $JPOG: FUNCTION TGuiInterface.dll+0000e920
	TGUIManager* GetManager() const { return m_pManager; }

	// $JPOG: FUNCTION TGuiInterface.dll+0000e8d0
	const TPString8& GetName() const { return m_strName; }

	// $JPOG: FUNCTION TGuiInterface.dll+0000e890
	void SetName( const TPString8& a_rcName ) { m_strName = a_rcName; }

	// $JPOG: FUNCTION TGuiInterface.dll+0000e8f0
	TINT GetFontID() const { return m_iFontID; }

	// $JPOG: FUNCTION TGuiInterface.dll+0000e8e0
	void SetFontID( TINT a_iFontID ) { m_iFontID = a_iFontID; }

	// $JPOG: FUNCTION TGuiInterface.dll+0000e880
	TINT GetFontHeight() const { return m_iFontHeight; }

	// $JPOG: FUNCTION TGuiInterface.dll+0000e870
	void SetFontHeight( TINT a_iFontHeight ) { m_iFontHeight = a_iFontHeight; }

	// $JPOG: FUNCTION TGuiInterface.dll+0000e860
	TINT GetSpacing() const { return m_iSpacing; }

	// $JPOG: FUNCTION TGuiInterface.dll+0000e850
	void SetSpacing( TINT a_iSpacing ) { m_iSpacing = a_iSpacing; }

	// $JPOG: FUNCTION TGuiInterface.dll+0000e830
	TArray<TINT16>& GetTextureList() { return m_aTextures; }

	// $JPOG: FUNCTION TGuiInterface.dll+0000e840
	GlyphList& GetGlyphList() { return m_oGlyphs; }

private:
	// Node of the glyph hash table
	struct GlyphHashNode
	{
		TUINT16        uiCharacter; // 0x0
		TGUIFontGlyph* pGlyph;      // 0x4
		GlyphHashNode* pNext;       // 0x8
	};

	void           CreateGlyphHash();
	void           DeleteGlyphHash();
	GlyphHashNode* FindGlyphHashNode( TUINT16 a_uiCharacter ) const;
	void           InsertGlyphHash( TUINT16 a_uiCharacter, TGUIFontGlyph* a_pGlyph );
	TBOOL          RemoveGlyphHash( TUINT16 a_uiCharacter );
	TINT           FindTexture( TINT16 a_iTexture ) const;
	void           LinkGlyph( TGUIFontGlyph* a_pGlyph );

public:
	static PropertySet* ms_oPropertySet;
	static PropertySet* ms_oPropertySetOrdered;
	static TFLOAT       ms_fHorizScale;
	static TFLOAT       ms_fVertScale;
	static BreakMode    ms_eBreakMode;

	static TPString8 str_name;
	static TPString8 str_texture;
	static TPString8 str_glyph;
	static TPString8 str_xpos;
	static TPString8 str_ypos;
	static TPString8 str_width;
	static TPString8 str_height;
	static TPString8 str_kernleft;
	static TPString8 str_kernright;
	static TPString8 str_spacing;

private:
	TGUIManager*    m_pManager;    // 0x10
	TPString8       m_strName;     // 0x14
	TINT            m_iFontID;     // 0x18 from the font factory
	TArray<TINT16>  m_aTextures;   // 0x1C
	GlyphList       m_oGlyphs;     // 0x2C
	TINT            m_iNumHashed;  // 0x3C
	TINT            m_iNumBuckets; // 0x40
	GlyphHashNode** m_ppBuckets;   // 0x44
	TINT            m_iFontHeight; // 0x48
	TINT            m_iSpacing;    // 0x4C
};

TOSHI_NAMESPACE_END
