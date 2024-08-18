#pragma once
#include "Render/TRenderInterface.h"
#include "Render/TRenderContext.h"

TOSHI_NAMESPACE_START

class TOSHI_API TViewport
{
public:
	using ChangeEvent = TUINT32;
	enum ChangeEvent_ : ChangeEvent
	{
		ChangeEvent_BackgroundColor,
		ChangeEvent_MaxZ,
		ChangeEvent_MinZ,
		ChangeEvent_Height,
		ChangeEvent_Width,
		ChangeEvent_X,
		ChangeEvent_Y,
		ChangeEvent_AllocatorBlock,
		ChangeEvent_BackgroundClear,
		ChangeEvent_DepthClear,
	};

public:
	TViewport( TBOOL a_bFlag );
	virtual ~TViewport();

	virtual void BeginSKU();
	virtual void EndSKU();
	virtual void ChangeSKU( ChangeEvent a_eEvent );

	TFLOAT GetX() const;
	TFLOAT GetY() const;
	TFLOAT GetWidth() const;
	TFLOAT GetHeight() const;

	void Begin();
	void End();

	TBOOL AllowBackgroundClear( TBOOL allow );
	TBOOL AllowDepthClear( TBOOL allow );

	void  Enable( TBOOL enable );
	TBOOL EnableDefaultBeginRender( TBOOL enable );

	void SetMemoryAllocatorBlock( TMemory::MemBlock* a_pMemBlock );
	void SetX( TFLOAT value );
	void SetY( TFLOAT value );
	void SetWidth( TFLOAT value );
	void SetHeight( TFLOAT value );
	void SetMinZ( TFLOAT value );
	void SetMaxZ( TFLOAT value );

	TRenderContext* GetRenderContext() const;

	void SetBackgroundColor( TUINT8 r, TUINT8 g, TUINT8 b, TUINT8 a );
	void GetBackgroundColor( TUINT8& r, TUINT8& g, TUINT8& b, TUINT8& a );

private:
	TMemory::MemBlock* m_MemAllocatorBlock;     // 0x00
	TRenderContext*    m_pRenderCtx;            // 0x04
	TUINT8             m_ColorR;                // 0x08
	TUINT8             m_ColorG;                // 0x09
	TUINT8             m_ColorB;                // 0x0A
	TUINT8             m_ColorA;                // 0x0B
	TBOOL              m_bAllowBackgroundClear; // 0x0C
	TBOOL              m_bAllowDepthClear;      // 0x0D
	TBOOL              m_bDefaultBeginRender;   // 0x0E
	TBOOL              m_bEnable;               // 0x0F
	TBOOL              m_bInViewport;           // 0x10
};

TOSHI_NAMESPACE_END
