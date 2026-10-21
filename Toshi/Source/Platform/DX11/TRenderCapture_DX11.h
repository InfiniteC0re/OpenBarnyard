#pragma once
#include "Render/TRenderCapture.h"

#include <d3d11.h>

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

	virtual TBOOL Poll() override { return m_pBuffer != TNULL; }

	virtual void* ObtainBuffer() override { return m_pBuffer; }

	virtual void ReleaseBuffer() override;

	// Copies the finished frame into the buffer, called by TRenderD3DInterface::EndScene before it's presented
	TBOOL CaptureBackBuffer( ID3D11Device* a_pDevice, ID3D11DeviceContext* a_pDeviceContext, ID3D11Texture2D* a_pBackBuffer );

private:
	TUINT8* m_pBuffer;
	FORMAT  m_eFormat;
	TINT    m_iWidth;
	TINT    m_iHeight;
	TINT    m_iUnk1;
	TBOOL   m_bUnk2;
};

TOSHI_NAMESPACE_END
