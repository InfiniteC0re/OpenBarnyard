#pragma once
#include "Render/TRenderCapture.h"

TOSHI_NAMESPACE_START

class TRenderCaptureD3D : public TRenderCapture
{
public:
	TRenderCaptureD3D();
	~TRenderCaptureD3D();

	virtual TBOOL Create( FORMAT a_eFormat, TINT a_iWidth, TINT a_iHeight ) override;
	virtual void  Destroy() override;
	virtual TBOOL Request() override;

	// RETURNS_TRUE
	virtual TBOOL Wait() override { return TTRUE; }

	// $Barnyard: FUNCTION 006d9d00
	virtual TBOOL Poll() override { return m_pBuffer != TNULL; }

	// $Barnyard: FUNCTION 006d9d10
	virtual void* ObtainBuffer() override { return m_pBuffer; }

	virtual void ReleaseBuffer() override;

private:
	TUINT8* m_pBuffer; // 0x04
	FORMAT  m_eFormat; // 0x08
	TINT    m_iWidth;  // 0x0C
	TINT    m_iHeight; // 0x10
	TINT    m_iUnk1;   // 0x14
	TBOOL   m_bUnk2;   // 0x18
};

TOSHI_NAMESPACE_END
