#pragma once
#include "Render/TDebugText.h"
#include "TGLFont.h"

TOSHI_NAMESPACE_START

class TRenderGLInterface;

// NOTE: created by TRenderGLInterface::InitDebugText, which retail never calls (TRenderGLInterface::CreateDebugText has no callers)
class TDebugGLText : public TDebugText
{
public:
	friend TRenderGLInterface;

	//-----------------------------------------------------------------------------
	// TDebugText
	//-----------------------------------------------------------------------------
	virtual void   Reset() override;
	virtual void   Destroy() override;
	virtual void   Render() override;
	virtual Entry* AllocateEntry( TUINT16 a_uiLength ) override;

	virtual ~TDebugGLText() {}

	// Releases what lives in the default pool, call when the device is lost
	TBOOL OnDeviceLost();
	// Restores what lives in the default pool, call when the device is found again
	TBOOL OnDeviceFound();

private:
	TINT      m_iUnk;         // 0x14
	TGLFont* m_pFont;        // 0x18
	TUINT8    m_aBuffer[ 4 ]; // 0x1C, actually m_iBufferSize bytes
};

TOSHI_NAMESPACE_END
