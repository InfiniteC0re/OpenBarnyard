#pragma once
#include "Toshi/Typedefs.h"

#include "Platform/GL/TRenderInterface_GL.h"

TOSHI_NAMESPACE_START

class MODAPI T2Texture
{
public:
	T2Texture();
	~T2Texture();

	void SetData( const void* a_pData, TUINT a_uiDataSize );
	void GetData( const void*& a_rData, TUINT& a_rDataSize )
	{
		a_rData     = m_pData;
		a_rDataSize = m_uiDataSize;
	}

	TBOOL Load();
	void  Unload();

	const TGLTextureInfo& GetImageInfo() const;
	TUINT                 GetWidth() const;
	TUINT                 GetHeight() const;
	TUINT                 GetMipLevels() const;

	GLuint GetGLTexture() { return m_uiTexture; }

protected:
	TUINT                     m_uiDataSize;
	const void*               m_pData;
	TUINT                     m_Unk1;
	TUINT                     m_Unk2;
	TGLTextureInfo m_ImageInfo;
	GLuint         m_uiTexture;
};

TOSHI_NAMESPACE_END
