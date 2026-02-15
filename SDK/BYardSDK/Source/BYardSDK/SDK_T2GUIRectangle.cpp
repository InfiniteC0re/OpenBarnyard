#include "pch.h"
#include "SDK_T2GUIRectangle.h"
#include "AGUI2.h"
#include "SDKHooks.h"

SDK_T2GUIRectangle::SDK_T2GUIRectangle()
{
}

void SDK_T2GUIRectangle::SetTextureSection( Toshi::T2GUITextureSection* a_pTexSection )
{
	if ( a_pTexSection )
	{
		m_pMaterial = (SDK_T2GUIMaterial*)a_pTexSection->GetMaterial();
		m_UV1       = a_pTexSection->GetUVPoint1();
		m_UV2       = a_pTexSection->GetUVPoint2();
	}
	else
	{
		m_pMaterial = TNULL;
	}
}

SDK_T2GUIRectangle* SDK_T2GUIRectangle::CreateFromEngine()
{
	return CALL_THIS( 0x006c3ae0, SDK_T2GUIRectangle*, SDK_T2GUIRectangle*, new SDK_T2GUIRectangle );
}
