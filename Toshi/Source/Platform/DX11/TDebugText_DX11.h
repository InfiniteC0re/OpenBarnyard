#pragma once
#include "Render/TDebugText.h"
#include "CD3DFont.h"

TOSHI_NAMESPACE_START

class TRenderD3DInterface;

// NOTE: created by TRenderD3DInterface::InitDebugText, which retail never calls (TRenderD3DInterface::CreateDebugText has no callers)
class TDebugD3DText : public TDebugText
{
public:
	friend TRenderD3DInterface;

	//-----------------------------------------------------------------------------
	// TDebugText
	//-----------------------------------------------------------------------------
	virtual void   Reset() override;
	virtual void   Destroy() override;
	virtual void   Render() override;
	virtual Entry* AllocateEntry( TUINT16 a_uiLength ) override;

	virtual ~TDebugD3DText() {}

	// Releases what lives in the default pool, call when the device is lost
	TBOOL OnDeviceLost();
	// Restores what lives in the default pool, call when the device is found again
	TBOOL OnDeviceFound();

private:
	TINT      m_iUnk;         // 0x14
	CD3DFont* m_pFont;        // 0x18
	TUINT8    m_aBuffer[ 4 ]; // 0x1C, actually m_iBufferSize bytes
};

TOSHI_NAMESPACE_END
