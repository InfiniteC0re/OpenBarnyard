#pragma once
#include <Render/TViewport.h>

class PGUITRRenderObject
{
public:
	// $Barnyard: FUNCTION 0059d610
	PGUITRRenderObject( TUINT a_uiIndex )
	{
		m_uiIndex = a_uiIndex;
	}

	// $Barnyard: FUNCTION 0059d600
	~PGUITRRenderObject()
	{
		if ( m_pViewport )
		{
			delete m_pViewport;
		}
	}

	// $Barnyard: FUNCTION 0066a1c0
	void SetViewport( Toshi::TViewport* a_pViewport )
	{
		m_pViewport = a_pViewport;
	}

	Toshi::TViewport* GetViewport()
	{
		return m_pViewport;
	}

	TUINT GetIndex() const
	{
		return m_uiIndex;
	}

private:
	TUINT               m_Unk1;
	PGUITRRenderObject* m_pUnk2;
	Toshi::TViewport*   m_pViewport;
	TUINT               m_uiIndex;
};
