#pragma once
#include "T2GUIElement.h"
#include "T2GUIMaterial.h"
#include "T2GUITextureSection.h"

#include <Math/TVector4.h>

TOSHI_NAMESPACE_START

class T2GUIRectangle : public T2GUIElement
{
public:
	T2GUIRectangle();

	virtual void Render() override;

	/**
	 * Sets material and UVs from AGUI2TextureSection.
	 * 
	 * @param a_pTexSection can be TNULL
	 */
	void SetTextureSection( T2GUITextureSection* a_pTexSection );
	void SetTextureSection( const T2GUITextureSection& a_rTexSection );

	/**
	 * Initialises rectangle with it's width and height.
	 */
	void Create( TFLOAT a_fWidth, TFLOAT a_fHeight )
	{
		T2GUIElement::SetDimensions( a_fWidth, a_fHeight );
	}

protected:
	T2GUIMaterial* m_pMaterial;
	TVector4       m_UV1;
	TVector4       m_UV2;
};

TOSHI_NAMESPACE_END
