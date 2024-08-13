#pragma once
#include <Render/TViewport.h>

class PGUITRRenderObject
{
public:
	PGUITRRenderObject( TUINT a_uiIndex )
	{
		m_uiIndex = a_uiIndex;
	}

	~PGUITRRenderObject()
	{
		if ( m_pViewport )
		{
			delete m_pViewport;
		}
	}

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
	TUINT				m_Unk1;
	PGUITRRenderObject* m_pUnk2;
	Toshi::TViewport*	m_pViewport;
	TUINT				m_uiIndex;
};
