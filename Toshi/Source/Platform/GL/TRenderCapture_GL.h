#pragma once
#include "Render/TRenderCapture.h"

#include "Platform/GL/TGLHeaders.h"

TOSHI_NAMESPACE_START

class TRenderCaptureGL : public TRenderCapture
{
public:
	TRenderCaptureGL();
	~TRenderCaptureGL();

	virtual TBOOL Create( FORMAT a_eFormat, TINT a_iWidth, TINT a_iHeight ) override;
	virtual void  Destroy() override;
	virtual TBOOL Request() override;

	// RETURNS_TRUE
	virtual TBOOL Wait() override { return TTRUE; }

	virtual TBOOL Poll() override { return m_pBuffer != TNULL; }

	virtual void* ObtainBuffer() override { return m_pBuffer; }

	virtual void ReleaseBuffer() override;

	// Copies the finished frame into the buffer, called by TRenderGLInterface::EndScene before it's presented
	TBOOL CaptureBackBuffer( TUINT a_uiBackBufferWidth, TUINT a_uiBackBufferHeight );

private:
	TUINT8* m_pBuffer;
	FORMAT  m_eFormat;
	TINT    m_iWidth;
	TINT    m_iHeight;
	TINT    m_iUnk1;
	TBOOL   m_bUnk2;
};

TOSHI_NAMESPACE_END
