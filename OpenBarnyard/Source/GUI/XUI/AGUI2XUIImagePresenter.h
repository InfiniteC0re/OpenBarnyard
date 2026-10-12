#pragma once
#ifdef BARNYARD_LATEST

#  include "AGUI2XUIElement.h"

#  include <GUI/T2GUIMaterial.h>
#  include <Render/TTexture.h>

//-----------------------------------------------------------------------------
// RTTI: AGUI2XUIImagePresenter : AGUI2XUIElement
// Shows the ImagePath of the control that owns the visual it is in.
//-----------------------------------------------------------------------------
class AGUI2XUIImagePresenter
    : public AGUI2XUIElement
{
public:
	TDECLARE_CLASS( AGUI2XUIImagePresenter, AGUI2XUIElement );

public:
	AGUI2XUIImagePresenter();
	~AGUI2XUIImagePresenter();

	//-----------------------------------------------------------------------------
	// Toshi::T2GUIElement
	//-----------------------------------------------------------------------------
	virtual void Render() override;

	//-----------------------------------------------------------------------------
	// AGUI2XUIElement
	//-----------------------------------------------------------------------------
	virtual TBOOL Create( AGUI2XUIResource& a_rResource, XURXUIElementData* a_pData, TBOOL a_bCreateChildren ) override;

	void  SetImage( const TWCHAR* a_wszImagePath );
	TBOOL UpdateSize( Toshi::TTexture* a_pTexture );

private:
	TUINT32               m_eSizeMode; // 0x98
	Toshi::T2GUIMaterial* m_pMaterial; // 0x9C
	Toshi::TVector2       m_vecUV1;    // 0xA0
	Toshi::TVector2       m_vecUV2;    // 0xA8
};

#endif // BARNYARD_LATEST
