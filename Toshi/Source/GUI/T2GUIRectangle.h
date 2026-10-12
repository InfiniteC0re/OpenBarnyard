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
	// NOTE: 00642950 is a jmp thunk to this destructor
	// $Barnyard: FUNCTION 006c38c0
	// $Barnyard: FUNCTION 00642950
	// NOTE: 00404aa0 is the scalar deleting destructor
	// $Barnyard: FUNCTION 00404aa0
	~T2GUIRectangle() {}

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

	// Sets the texture coordinates of the four corners (only the first and the third are used by Render)
	void SetUVs( const TVector2& a_rcUV1, const TVector2& a_rcUV2, const TVector2& a_rcUV3, const TVector2& a_rcUV4 );
	void GetUVs( TVector2& a_rUV1, TVector2& a_rUV2, TVector2& a_rUV3, TVector2& a_rUV4 ) const;

	// Mirrors the texture horizontally (swaps the first and the second UV of both pairs)
	void FlipU();

	// Mirrors the texture vertically (swaps the first UV with the fourth and the second with the third)
	void FlipV();

	// $Barnyard: FUNCTION 006c3960
	T2GUIMaterial* GetMaterial() const { return m_pMaterial; }

protected:
	T2GUIMaterial* m_pMaterial;
	TVector4       m_UV1;
	TVector4       m_UV2;
};

TOSHI_NAMESPACE_END
