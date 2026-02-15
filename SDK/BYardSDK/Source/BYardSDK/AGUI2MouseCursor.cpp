#include "pch.h"
#include "AGUI2MouseCursor.h"
#include "SDK_T2GUITextureSectionManager.h"
#include "AGUI2.h"
#include "SDKHooks.h"

TOSHI_NAMESPACE_USING

AGUI2MouseCursor::AGUI2MouseCursor()
{
	CALL_THIS( 0x00646360, AGUI2MouseCursor*, AGUI2MouseCursor*, this );
}

TBOOL AGUI2MouseCursor::Create( const char* a_szPointerUpTexture, const char* a_szPointerDownTexture, TFLOAT a_fScalar )
{
	return CALL_THIS( 0x00646360, AGUI2MouseCursor*, TBOOL, this, const char*, a_szPointerUpTexture, const char*, a_szPointerDownTexture, TFLOAT, a_fScalar );
}

void AGUI2MouseCursor::Update()
{
	CALL_THIS( 0x00646740, AGUI2MouseCursor*, AGUI2MouseCursor*, this );
}

void AGUI2MouseCursor::MoveCursor( TFLOAT a_fDeltaX, TFLOAT a_fDeltaY )
{
	CALL_THIS( 0x00646400, AGUI2MouseCursor*, AGUI2MouseCursor*, this, TFLOAT, a_fDeltaX, TFLOAT, a_fDeltaY );
}
