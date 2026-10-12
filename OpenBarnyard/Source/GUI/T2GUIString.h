#pragma once
#include <GUI/T2GUIElement.h>
#include <GUI/T2GUIFont.h>

TOSHI_NAMESPACE_START

//-----------------------------------------------------------------------------
// Single line of text drawn with a T2GUIFont. Its size is the size of the text.
// vtable 007412e8.
// NOTE: engine class (Toshi::T2GUIString). It lives in the game project until
// it's moved to Toshi/Source/GUI.
//-----------------------------------------------------------------------------
class T2GUIString
    : public T2GUIElement
{
public:
	T2GUIString();
	~T2GUIString();

	//-----------------------------------------------------------------------------
	// T2GUIElement
	//-----------------------------------------------------------------------------
	virtual void Render() override;
	virtual void GetDimensions( TFLOAT& a_rWidth, TFLOAT& a_rHeight ) override;
	// DOES_NOTHING: the size always follows the text
	virtual void SetDimensions( TFLOAT a_fWidth, TFLOAT a_fHeight ) override {}

	void SetFont( T2GUIFont* a_pFont );

	void SetText( const TWCHAR* a_wszText )
	{
		m_wszText          = a_wszText;
		m_bDimensionsDirty = TTRUE;
	}

	void SetScale( TFLOAT a_fScale )
	{
		m_fScale           = a_fScale;
		m_bDimensionsDirty = TTRUE;
	}

	const TWCHAR* GetText() const { return m_wszText; }
	T2GUIFont*    GetFont() const { return m_pFont; }
	TFLOAT        GetScale() const { return m_fScale; }

private:
	const TWCHAR* m_wszText;          // 0x54
	T2GUIFont*    m_pFont;            // 0x58
	TFLOAT        m_fScale;           // 0x5c
	TBOOL         m_bDimensionsDirty; // 0x60
};

TOSHI_NAMESPACE_END
