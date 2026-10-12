#pragma once
#ifdef BARNYARD_LATEST

#  include "AGUI2XUIElement.h"

#  include <GUI/T2GUIMaterial.h>
#  include <Render/TTexture.h>

//-----------------------------------------------------------------------------
// RTTI: AGUI2XUIImage : AGUI2XUIElement
//-----------------------------------------------------------------------------
class AGUI2XUIImage
    : public AGUI2XUIElement
{
public:
	TDECLARE_CLASS( AGUI2XUIImage, AGUI2XUIElement );

	// Values of the XUI SizeMode property that are handled
	enum SIZEMODE : TUINT32
	{
		SIZEMODE_TEXTURE = 1, // the element takes the size of the texture
		SIZEMODE_STRETCH = 4,
	};

public:
	AGUI2XUIImage();
	~AGUI2XUIImage();

	//-----------------------------------------------------------------------------
	// Toshi::T2GUIElement
	//-----------------------------------------------------------------------------
	virtual void Render() override;

	//-----------------------------------------------------------------------------
	// AGUI2XUIElement
	//-----------------------------------------------------------------------------
	virtual TBOOL Create( AGUI2XUIResource& a_rResource, XURXUIElementData* a_pData, TBOOL a_bCreateChildren ) override;
	virtual void  UpdateTimelinePropFloat( AGUI2XUIResource& a_rResource, TUINT32 a_uiProp, TINT a_iIndex, TBOOL a_bKeyframeChanged, TFLOAT a_fFrom, TFLOAT a_fTo, TFLOAT a_fT ) override;
	virtual void  UpdateTimelinePropValue( AGUI2XUIResource& a_rResource, TUINT32 a_uiProp, TUINT32 a_uiFrom, TUINT32 a_uiTo, TINT a_iIndex, TBOOL a_bKeyframeChanged, TFLOAT a_fT ) override;

	void  SetImage( const TWCHAR* a_wszImagePath );
	TBOOL UpdateSize();

private:
	TUINT32               m_eSizeMode; // 0x98
	Toshi::T2GUIMaterial* m_pMaterial; // 0x9C
	Toshi::TVector2       m_vecUV1;    // 0xA0
	Toshi::TVector2       m_vecUV2;    // 0xA8
	Toshi::TTexture*      m_pTexture;  // 0xB0
};

#endif // BARNYARD_LATEST
