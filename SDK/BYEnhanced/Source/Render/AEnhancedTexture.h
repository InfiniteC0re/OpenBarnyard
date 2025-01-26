#pragma once
#include <Toshi/TSingleton.h>
#include <Toshi/T2Map.h>

#include <Platform/GL/T2GLTexture_GL.h>

struct IDirect3DTexture8;

//-----------------------------------------------------------------------------
// Purpose: converts D3D texture into an OpenGL texture.
//-----------------------------------------------------------------------------
class AEnhancedTexture
{
public:
};

class AEnhancedTextureManager
    : public Toshi::TSingleton<AEnhancedTextureManager>
{
public:
	AEnhancedTextureManager();
	~AEnhancedTextureManager();

	Toshi::T2GLTexture* CreateTexture( IDirect3DTexture8* a_pD3DTexture, const void* a_pData, TSIZE a_uiDataSize );
	void       DestroyTexture( Toshi::T2GLTexture* a_pTexture );

	static Toshi::T2GLTexture* GetAssociation( IDirect3DTexture8* a_pD3DTexture );

private:
	void AddAssociation( IDirect3DTexture8* a_pD3DTexture, Toshi::T2GLTexture* a_pGLTexture );
	void RemoveAssociation( IDirect3DTexture8* a_pD3DTexture );
	void RemoveAssociation( Toshi::T2GLTexture* a_pGLTexture );
	
private:
	Toshi::T2Map<IDirect3DTexture8*, Toshi::T2GLTexture*> m_mapAssociations;
};
