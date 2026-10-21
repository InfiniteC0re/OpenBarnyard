#pragma once
#include "Math/TVector4.h"
#include "Math/TMatrix44.h"

#include <d3d11.h>

TOSHI_NAMESPACE_START

//-----------------------------------------------------------------------------
// Purpose: A constant buffer (cbuffer) with a copy of its contents in memory, uploaded
// before the next draw that uses it when it changed.
// Bound with TRenderD3DInterface::SetVSConstantBuffer/SetPSConstantBuffer; slot 0 is the
// shared buffer of the shader HALs (TRenderD3DInterface::SetVec4InVSBuffer).
//-----------------------------------------------------------------------------
class MODAPI TConstantBufferGeneric
{
public:
	TConstantBufferGeneric();
	~TConstantBufferGeneric();

	TConstantBufferGeneric( const TConstantBufferGeneric& )            = delete;
	TConstantBufferGeneric& operator=( const TConstantBufferGeneric& ) = delete;

	// a_uiSize is rounded up to 16 bytes, the size of a register
	TBOOL Create( TUINT a_uiSize );
	void  Destroy();

	// Writes a_uiSize bytes at a_uiOffset, the buffer only becomes dirty when they changed
	void SetData( TUINT a_uiOffset, const void* a_pData, TUINT a_uiSize );
	void SetVec4( TUINT a_uiRegister, const TVector4& a_rcValue ) { SetData( a_uiRegister * sizeof( TVector4 ), &a_rcValue, sizeof( TVector4 ) ); }
	void SetMatrix( TUINT a_uiRegister, const TMatrix44& a_rcValue ) { SetData( a_uiRegister * sizeof( TVector4 ), &a_rcValue, sizeof( TMatrix44 ) ); }

	// Direct access to the contents, the buffer is uploaded again
	void*       Edit();
	const void* GetData() const { return m_pData; }

	// Sends the contents to the GPU if they changed, the renderer does it before draws
	void Upload();

	TBOOL         IsCreated() const { return m_pBuffer != TNULL; }
	TBOOL         IsDirty() const { return m_bDirty; }
	TUINT         GetSize() const { return m_uiSize; }
	ID3D11Buffer* GetBuffer() const { return m_pBuffer; }

private:
	ID3D11Buffer* m_pBuffer;
	TUINT8*       m_pData;
	TUINT         m_uiSize;
	TBOOL         m_bDirty;
};

//-----------------------------------------------------------------------------
// Purpose: Typed constant buffer, T mirrors the HLSL cbuffer (16 byte registers, no
// variable crossing a register boundary).
//-----------------------------------------------------------------------------
template <class T>
class TConstantBuffer : public TConstantBufferGeneric
{
public:
	TSTATICASSERT( sizeof( T ) % 16 == 0 );

	TBOOL Create() { return TConstantBufferGeneric::Create( sizeof( T ) ); }

	// Copies a_rcData in, the buffer only becomes dirty when it changed
	void Set( const T& a_rcData ) { SetData( 0, &a_rcData, sizeof( T ) ); }

	T&       Edit() { return *TSTATICCAST( T, TConstantBufferGeneric::Edit() ); }
	const T& Get() const { return *TSTATICCAST( const T, GetData() ); }
};

TOSHI_NAMESPACE_END
