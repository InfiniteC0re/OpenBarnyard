#include "ToshiPCH.h"
#include "T2GUI.h"
#include "T2GUIPolygon.h"
#include "T2GUIElement.h"
#include "Math/TVector2.h"

TOSHI_NAMESPACE_START

// $Barnyard: FUNCTION 006d8730
T2GUIPolygon::T2GUIPolygon()
{
	m_pMaterial    = TNULL;
	m_iNumVertices = 0;
	m_pVertices    = TNULL;
	m_pUVs         = TNULL;
	m_bBoundsValid = TFALSE;
}

// $Barnyard: FUNCTION 006d8760
// $Barnyard: FUNCTION 006d88a0
T2GUIPolygon::~T2GUIPolygon()
{
	delete[] m_pUVs;
	delete[] m_pVertices;
}

// $Barnyard: FUNCTION 006d8850
void T2GUIPolygon::Render()
{
	T2GUIRenderer* pRenderer = T2GUI::GetRenderer();

	pRenderer->SetMaterial( m_pMaterial );
	pRenderer->SetColour( m_uiColour );
	pRenderer->RenderTriStrip( m_pVertices, m_pUVs, m_iNumVertices );

	T2GUIElement::Render();
}

// $Barnyard: FUNCTION 006d8790
void T2GUIPolygon::GetDimensions( TFLOAT& a_rWidth, TFLOAT& a_rHeight )
{
	if ( !m_bBoundsValid )
	{
		TFLOAT fMinX = TMath::MAXFLOAT;
		TFLOAT fMaxX = TMath::MINFLOAT;
		TFLOAT fMinY = TMath::MAXFLOAT;
		TFLOAT fMaxY = TMath::MINFLOAT;

		const TVector2* pVert = m_pVertices;
		for ( TINT i = 0; i < m_iNumVertices; i++, pVert++ )
		{
			if ( pVert->x < fMinX ) fMinX = pVert->x;
			if ( pVert->x > fMaxX ) fMaxX = pVert->x;
			if ( pVert->y < fMinY ) fMinY = pVert->y;
			if ( pVert->y > fMaxY ) fMaxY = pVert->y;
		}

		m_bBoundsValid = TTRUE;
		m_fWidth       = fMaxX - fMinX;
		m_fHeight      = fMaxY - fMinY;
	}

	a_rWidth  = m_fWidth;
	a_rHeight = m_fHeight;
}

// $Barnyard: FUNCTION 006d88e0
void T2GUIPolygon::CreateBuffers( TINT a_iNumVertices )
{
	m_iNumVertices      = a_iNumVertices;
	m_pVertices         = new TVector2[ a_iNumVertices ];
	m_pUVs              = new TVector2[ a_iNumVertices ];
	m_bBoundsValid      = TFALSE;
}

void T2GUIPolygon::InvalidateBounds()
{
	m_bBoundsValid = TFALSE;
}

TOSHI_NAMESPACE_END
