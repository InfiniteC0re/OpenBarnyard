#include "pch.h"
#include "AGUI2Rectangle.h"
#include "AGUI2.h"
#include "SDKHooks.h"

AGUI2Rectangle::AGUI2Rectangle()
{
}

void AGUI2Rectangle::SetTextureSection( AGUI2TextureSection* a_pTexSection )
{
	if ( a_pTexSection )
	{
		m_pMaterial = a_pTexSection->GetMaterial();
		m_UV1		= a_pTexSection->GetUVPoint1();
		m_UV2		= a_pTexSection->GetUVPoint2();
	}
	else
	{
		m_pMaterial = TNULL;
	}
}

AGUI2Rectangle* AGUI2Rectangle::CreateFromEngine()
{
	return CALL_THIS( 0x006c3ae0, AGUI2Rectangle*, AGUI2Rectangle*, new AGUI2Rectangle );
}
