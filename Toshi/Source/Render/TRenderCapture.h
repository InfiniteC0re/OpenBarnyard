#pragma once

TOSHI_NAMESPACE_START

//-----------------------------------------------------------------------------
// Purpose: Copies the contents of the screen into a buffer.
// Usage: Create, Request, then Poll until the buffer is ready and ObtainBuffer.
//-----------------------------------------------------------------------------
class TRenderCapture
{
public:
	enum FORMAT : TUINT32
	{
		FORMAT_RGBA32 = 0,
	};

public:
	TRenderCapture() = default;

	// $Barnyard: FUNCTION 006d9aa0
	virtual ~TRenderCapture() = default;

	virtual TBOOL Create( FORMAT a_eFormat, TINT a_iWidth, TINT a_iHeight ) = 0; // +0x04
	virtual void  Destroy()                                                 = 0; // +0x08
	virtual TBOOL Request()                                                 = 0; // +0x0C
	virtual TBOOL Wait()                                                    = 0; // +0x10
	virtual TBOOL Poll()                                                    = 0; // +0x14
	virtual void* ObtainBuffer()                                            = 0; // +0x18
	virtual void  ReleaseBuffer()                                           = 0; // +0x1C
};

TOSHI_NAMESPACE_END
