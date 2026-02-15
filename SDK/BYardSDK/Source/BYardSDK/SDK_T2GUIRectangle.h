#pragma once
#include "SDK_T2GUIElement.h"
#include "SDK_T2GUIMaterial.h"
#include <GUI/T2GUITextureSection.h>

#include <Math/TVector4.h>

class SDK_T2GUIRectangle : public SDK_T2GUIElement
{
public:
	SDK_T2GUIRectangle();

	virtual void Render() override {}

	/**
	 * Sets material and UVs from AGUI2TextureSection.
	 * 
	 * @param a_pTexSection can be TNULL
	 */
	void SetTextureSection( Toshi::T2GUITextureSection* a_pTexSection );

	/**
	 * Initialises rectangle with it's width and height.
	 */
	void Create( TFLOAT a_fWidth, TFLOAT a_fHeight )
	{
		SDK_T2GUIElement::SetDimensions( a_fWidth, a_fHeight );
	}

	static SDK_T2GUIRectangle* CreateFromEngine();

protected:
	SDK_T2GUIMaterial* m_pMaterial;
	Toshi::TVector4    m_UV1;
	Toshi::TVector4    m_UV2;
};
