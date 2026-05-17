#pragma once
#include "T2GUIElement.h"

TOSHI_NAMESPACE_START

class T2GUIPolygon : public T2GUIElement
{
public:
	T2GUIPolygon();
	~T2GUIPolygon();

	//-----------------------------------------------------------------------------
	// T2GUIElement
	//-----------------------------------------------------------------------------
	virtual void Render() override;
	virtual void GetDimensions( TFLOAT& a_rWidth, TFLOAT& a_rHeight ) override;

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------
	void CreateBuffers( TINT a_iNumVertices );

	void InvalidateBounds();

private:
	T2GUIMaterial* m_pMaterial;    // 0x54
	TINT           m_iNumVertices; // 0x58
	TVector2*      m_pVertices;    // 0x5C
	TVector2*      m_pUVs;         // 0x60
	TBOOL          m_bBoundsValid; // 0x64
};

TOSHI_NAMESPACE_END
