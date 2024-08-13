#pragma once
#include "AGUI2Element.h"
#include "AGUI2Material.h"
#include "AGUI2TextureSection.h"

#include <Math/TVector4.h>

class AGUI2Rectangle :
	public AGUI2Element
{
public:
	AGUI2Rectangle();

	virtual void Render() override;

	/**
	 * Sets material and UVs from AGUI2TextureSection.
	 * 
	 * @param a_pTexSection can be TNULL
	 */
	void SetTextureSection( AGUI2TextureSection* a_pTexSection );

	/**
	 * Initialises rectangle with it's width and height.
	 */
	void Create( TFLOAT a_fWidth, TFLOAT a_fHeight )
	{
		AGUI2Element::SetDimensions( a_fWidth, a_fHeight );
	}

protected:
	AGUI2Material*	m_pMaterial;
	Toshi::TVector4 m_UV1;
	Toshi::TVector4 m_UV2;
};
