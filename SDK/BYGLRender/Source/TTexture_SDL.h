#pragma once
#include "Toshi/T2Map.h"

#include <gl/glew.h>

struct IDirect3DTexture8;

namespace Toshi
{

typedef enum TEXTURE_FORMAT
{
	TEXTURE_FORMAT_UNKNOWN                                 = 0,
	TEXTURE_FORMAT_R32G32B32A32_TYPELESS                   = 1,
	TEXTURE_FORMAT_R32G32B32A32_FLOAT                      = 2,
	TEXTURE_FORMAT_R32G32B32A32_UINT                       = 3,
	TEXTURE_FORMAT_R32G32B32A32_SINT                       = 4,
	TEXTURE_FORMAT_R32G32B32_TYPELESS                      = 5,
	TEXTURE_FORMAT_R32G32B32_FLOAT                         = 6,
	TEXTURE_FORMAT_R32G32B32_UINT                          = 7,
	TEXTURE_FORMAT_R32G32B32_SINT                          = 8,
	TEXTURE_FORMAT_R16G16B16A16_TYPELESS                   = 9,
	TEXTURE_FORMAT_R16G16B16A16_FLOAT                      = 10,
	TEXTURE_FORMAT_R16G16B16A16_UNORM                      = 11,
	TEXTURE_FORMAT_R16G16B16A16_UINT                       = 12,
	TEXTURE_FORMAT_R16G16B16A16_SNORM                      = 13,
	TEXTURE_FORMAT_R16G16B16A16_SINT                       = 14,
	TEXTURE_FORMAT_R32G32_TYPELESS                         = 15,
	TEXTURE_FORMAT_R32G32_FLOAT                            = 16,
	TEXTURE_FORMAT_R32G32_UINT                             = 17,
	TEXTURE_FORMAT_R32G32_SINT                             = 18,
	TEXTURE_FORMAT_R32G8X24_TYPELESS                       = 19,
	TEXTURE_FORMAT_D32_FLOAT_S8X24_UINT                    = 20,
	TEXTURE_FORMAT_R32_FLOAT_X8X24_TYPELESS                = 21,
	TEXTURE_FORMAT_X32_TYPELESS_G8X24_UINT                 = 22,
	TEXTURE_FORMAT_R10G10B10A2_TYPELESS                    = 23,
	TEXTURE_FORMAT_R10G10B10A2_UNORM                       = 24,
	TEXTURE_FORMAT_R10G10B10A2_UINT                        = 25,
	TEXTURE_FORMAT_R11G11B10_FLOAT                         = 26,
	TEXTURE_FORMAT_R8G8B8A8_TYPELESS                       = 27,
	TEXTURE_FORMAT_R8G8B8A8_UNORM                          = 28,
	TEXTURE_FORMAT_R8G8B8A8_UNORM_SRGB                     = 29,
	TEXTURE_FORMAT_R8G8B8A8_UINT                           = 30,
	TEXTURE_FORMAT_R8G8B8A8_SNORM                          = 31,
	TEXTURE_FORMAT_R8G8B8A8_SINT                           = 32,
	TEXTURE_FORMAT_R16G16_TYPELESS                         = 33,
	TEXTURE_FORMAT_R16G16_FLOAT                            = 34,
	TEXTURE_FORMAT_R16G16_UNORM                            = 35,
	TEXTURE_FORMAT_R16G16_UINT                             = 36,
	TEXTURE_FORMAT_R16G16_SNORM                            = 37,
	TEXTURE_FORMAT_R16G16_SINT                             = 38,
	TEXTURE_FORMAT_R32_TYPELESS                            = 39,
	TEXTURE_FORMAT_D32_FLOAT                               = 40,
	TEXTURE_FORMAT_R32_FLOAT                               = 41,
	TEXTURE_FORMAT_R32_UINT                                = 42,
	TEXTURE_FORMAT_R32_SINT                                = 43,
	TEXTURE_FORMAT_R24G8_TYPELESS                          = 44,
	TEXTURE_FORMAT_D24_UNORM_S8_UINT                       = 45,
	TEXTURE_FORMAT_R24_UNORM_X8_TYPELESS                   = 46,
	TEXTURE_FORMAT_X24_TYPELESS_G8_UINT                    = 47,
	TEXTURE_FORMAT_R8G8_TYPELESS                           = 48,
	TEXTURE_FORMAT_R8G8_UNORM                              = 49,
	TEXTURE_FORMAT_R8G8_UINT                               = 50,
	TEXTURE_FORMAT_R8G8_SNORM                              = 51,
	TEXTURE_FORMAT_R8G8_SINT                               = 52,
	TEXTURE_FORMAT_R16_TYPELESS                            = 53,
	TEXTURE_FORMAT_R16_FLOAT                               = 54,
	TEXTURE_FORMAT_D16_UNORM                               = 55,
	TEXTURE_FORMAT_R16_UNORM                               = 56,
	TEXTURE_FORMAT_R16_UINT                                = 57,
	TEXTURE_FORMAT_R16_SNORM                               = 58,
	TEXTURE_FORMAT_R16_SINT                                = 59,
	TEXTURE_FORMAT_R8_TYPELESS                             = 60,
	TEXTURE_FORMAT_R8_UNORM                                = 61,
	TEXTURE_FORMAT_R8_UINT                                 = 62,
	TEXTURE_FORMAT_R8_SNORM                                = 63,
	TEXTURE_FORMAT_R8_SINT                                 = 64,
	TEXTURE_FORMAT_A8_UNORM                                = 65,
	TEXTURE_FORMAT_R1_UNORM                                = 66,
	TEXTURE_FORMAT_R9G9B9E5_SHAREDEXP                      = 67,
	TEXTURE_FORMAT_R8G8_B8G8_UNORM                         = 68,
	TEXTURE_FORMAT_G8R8_G8B8_UNORM                         = 69,
	TEXTURE_FORMAT_BC1_TYPELESS                            = 70,
	TEXTURE_FORMAT_BC1_UNORM                               = 71,
	TEXTURE_FORMAT_BC1_UNORM_SRGB                          = 72,
	TEXTURE_FORMAT_BC2_TYPELESS                            = 73,
	TEXTURE_FORMAT_BC2_UNORM                               = 74,
	TEXTURE_FORMAT_BC2_UNORM_SRGB                          = 75,
	TEXTURE_FORMAT_BC3_TYPELESS                            = 76,
	TEXTURE_FORMAT_BC3_UNORM                               = 77,
	TEXTURE_FORMAT_BC3_UNORM_SRGB                          = 78,
	TEXTURE_FORMAT_BC4_TYPELESS                            = 79,
	TEXTURE_FORMAT_BC4_UNORM                               = 80,
	TEXTURE_FORMAT_BC4_SNORM                               = 81,
	TEXTURE_FORMAT_BC5_TYPELESS                            = 82,
	TEXTURE_FORMAT_BC5_UNORM                               = 83,
	TEXTURE_FORMAT_BC5_SNORM                               = 84,
	TEXTURE_FORMAT_B5G6R5_UNORM                            = 85,
	TEXTURE_FORMAT_B5G5R5A1_UNORM                          = 86,
	TEXTURE_FORMAT_B8G8R8A8_UNORM                          = 87,
	TEXTURE_FORMAT_B8G8R8X8_UNORM                          = 88,
	TEXTURE_FORMAT_R10G10B10_XR_BIAS_A2_UNORM              = 89,
	TEXTURE_FORMAT_B8G8R8A8_TYPELESS                       = 90,
	TEXTURE_FORMAT_B8G8R8A8_UNORM_SRGB                     = 91,
	TEXTURE_FORMAT_B8G8R8X8_TYPELESS                       = 92,
	TEXTURE_FORMAT_B8G8R8X8_UNORM_SRGB                     = 93,
	TEXTURE_FORMAT_BC6H_TYPELESS                           = 94,
	TEXTURE_FORMAT_BC6H_UF16                               = 95,
	TEXTURE_FORMAT_BC6H_SF16                               = 96,
	TEXTURE_FORMAT_BC7_TYPELESS                            = 97,
	TEXTURE_FORMAT_BC7_UNORM                               = 98,
	TEXTURE_FORMAT_BC7_UNORM_SRGB                          = 99,
	TEXTURE_FORMAT_AYUV                                    = 100,
	TEXTURE_FORMAT_Y410                                    = 101,
	TEXTURE_FORMAT_Y416                                    = 102,
	TEXTURE_FORMAT_NV12                                    = 103,
	TEXTURE_FORMAT_P010                                    = 104,
	TEXTURE_FORMAT_P016                                    = 105,
	TEXTURE_FORMAT_420_OPAQUE                              = 106,
	TEXTURE_FORMAT_YUY2                                    = 107,
	TEXTURE_FORMAT_Y210                                    = 108,
	TEXTURE_FORMAT_Y216                                    = 109,
	TEXTURE_FORMAT_NV11                                    = 110,
	TEXTURE_FORMAT_AI44                                    = 111,
	TEXTURE_FORMAT_IA44                                    = 112,
	TEXTURE_FORMAT_P8                                      = 113,
	TEXTURE_FORMAT_A8P8                                    = 114,
	TEXTURE_FORMAT_B4G4R4A4_UNORM                          = 115,
	TEXTURE_FORMAT_P208                                    = 130,
	TEXTURE_FORMAT_V208                                    = 131,
	TEXTURE_FORMAT_V408                                    = 132,
	TEXTURE_FORMAT_SAMPLER_FEEDBACK_MIN_MIP_OPAQUE         = 189,
	TEXTURE_FORMAT_SAMPLER_FEEDBACK_MIP_REGION_USED_OPAQUE = 190,
	TEXTURE_FORMAT_FORCE_UINT                              = 0xffffffff
} TEXTURE_FORMAT;

typedef enum TEXTURE_ADDRESS_MODE
{
	TEXTURE_ADDRESS_WRAP        = 1,
	TEXTURE_ADDRESS_MIRROR      = 2,
	TEXTURE_ADDRESS_CLAMP       = 3,
	TEXTURE_ADDRESS_BORDER      = 4,
	TEXTURE_ADDRESS_MIRROR_ONCE = 5
} TEXTURE_ADDRESS_MODE;

typedef enum TEXTURE_FILTER
{
	TEXTURE_FILTER_MIN_MAG_MIP_POINT                          = 0,
	TEXTURE_FILTER_MIN_MAG_POINT_MIP_LINEAR                   = 0x1,
	TEXTURE_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT             = 0x4,
	TEXTURE_FILTER_MIN_POINT_MAG_MIP_LINEAR                   = 0x5,
	TEXTURE_FILTER_MIN_LINEAR_MAG_MIP_POINT                   = 0x10,
	TEXTURE_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR            = 0x11,
	TEXTURE_FILTER_MIN_MAG_LINEAR_MIP_POINT                   = 0x14,
	TEXTURE_FILTER_MIN_MAG_MIP_LINEAR                         = 0x15,
	TEXTURE_FILTER_ANISOTROPIC                                = 0x55,
	TEXTURE_FILTER_COMPARISON_MIN_MAG_MIP_POINT               = 0x80,
	TEXTURE_FILTER_COMPARISON_MIN_MAG_POINT_MIP_LINEAR        = 0x81,
	TEXTURE_FILTER_COMPARISON_MIN_POINT_MAG_LINEAR_MIP_POINT  = 0x84,
	TEXTURE_FILTER_COMPARISON_MIN_POINT_MAG_MIP_LINEAR        = 0x85,
	TEXTURE_FILTER_COMPARISON_MIN_LINEAR_MAG_MIP_POINT        = 0x90,
	TEXTURE_FILTER_COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR = 0x91,
	TEXTURE_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT        = 0x94,
	TEXTURE_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR              = 0x95,
	TEXTURE_FILTER_COMPARISON_ANISOTROPIC                     = 0xd5,
	TEXTURE_FILTER_MINIMUM_MIN_MAG_MIP_POINT                  = 0x100,
	TEXTURE_FILTER_MINIMUM_MIN_MAG_POINT_MIP_LINEAR           = 0x101,
	TEXTURE_FILTER_MINIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT     = 0x104,
	TEXTURE_FILTER_MINIMUM_MIN_POINT_MAG_MIP_LINEAR           = 0x105,
	TEXTURE_FILTER_MINIMUM_MIN_LINEAR_MAG_MIP_POINT           = 0x110,
	TEXTURE_FILTER_MINIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR    = 0x111,
	TEXTURE_FILTER_MINIMUM_MIN_MAG_LINEAR_MIP_POINT           = 0x114,
	TEXTURE_FILTER_MINIMUM_MIN_MAG_MIP_LINEAR                 = 0x115,
	TEXTURE_FILTER_MINIMUM_ANISOTROPIC                        = 0x155,
	TEXTURE_FILTER_MAXIMUM_MIN_MAG_MIP_POINT                  = 0x180,
	TEXTURE_FILTER_MAXIMUM_MIN_MAG_POINT_MIP_LINEAR           = 0x181,
	TEXTURE_FILTER_MAXIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT     = 0x184,
	TEXTURE_FILTER_MAXIMUM_MIN_POINT_MAG_MIP_LINEAR           = 0x185,
	TEXTURE_FILTER_MAXIMUM_MIN_LINEAR_MAG_MIP_POINT           = 0x190,
	TEXTURE_FILTER_MAXIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR    = 0x191,
	TEXTURE_FILTER_MAXIMUM_MIN_MAG_LINEAR_MIP_POINT           = 0x194,
	TEXTURE_FILTER_MAXIMUM_MIN_MAG_MIP_LINEAR                 = 0x195,
	TEXTURE_FILTER_MAXIMUM_ANISOTROPIC                        = 0x1d5
} TEXTURE_FILTER;

class TTextureSDL
{
public:
	friend class TTextureManagerSDL;

	struct Info
	{
		UINT           Width;
		UINT           Height;
		UINT           MipMapCount;
		UINT16         Width2;
		UINT16         Height2;
		TEXTURE_FORMAT Format;
		GLuint         Handle;

		Info()
		{
			Width       = 0;
			Height      = 0;
			MipMapCount = 0;
			Width2      = 0;
			Height2     = 0;
			Format      = TEXTURE_FORMAT_UNKNOWN;
			Handle      = 0;
		}
	};

public:
	TTextureSDL()
	{
		m_Unk1         = TNULL;
		m_TexInfo      = TNULL;
		m_TexName      = TNULL;
		m_DataSize     = 0;
		m_TexData      = 0;
		m_pPrevTexture = TNULL;
		m_pNextTexture = TNULL;
		m_Flags1       = 0;
		m_eAddressU    = TEXTURE_ADDRESS_WRAP;
		m_eAddressV    = TEXTURE_ADDRESS_WRAP;
		m_eFilter      = TEXTURE_FILTER_MIN_MAG_MIP_LINEAR;
		m_SamplerId    = 3;
	}

	void                Init();
	void                Deinit();
	void                Bind( GLenum a_eTarget );
	TTextureSDL*        InitRunTime( Info* pTextureInfo );
	static TTextureSDL* InitRunTime( TEXTURE_FORMAT format, UINT width, UINT height, const void* srcData );
	void                SelectSamplerId();

	const char* GetName() const { return m_TexName; }
	uint8_t     GetFlags1() const { return m_Flags1; }
	void*       GetHandle() const { return TREINTERPRETCAST( void*, m_TexInfo->Handle ); }
	UINT        GetWidth() const { return m_TexInfo->Width; }
	UINT        GetHeight() const { return m_TexInfo->Height; }

	TTextureSDL* GetPrev() const { return m_pPrevTexture; }
	TTextureSDL* GetNext() const { return m_pNextTexture; }

	TBOOL CheckFormat() { return m_TexInfo->Format == TEXTURE_FORMAT_R8G8B8A8_UNORM || m_TexInfo->Format == TEXTURE_FORMAT_BC3_UNORM; }
	void  SetName( const char* name ) { m_TexName = name; }
	void  SetSamplerId( int iSamplerId ) { m_SamplerId = iSamplerId; }
	void  SetWrap( TEXTURE_ADDRESS_MODE eAddressU, TEXTURE_ADDRESS_MODE eAddressV );

	void SetAlphaEnabled( TBOOL bEnabled )
	{
		if ( bEnabled )
			m_Flags1 |= 1;
		else
			m_Flags1 &= ~1;
	}

private:
	void*                m_Unk1;
	const char*          m_TexName;
	Info*                m_TexInfo;
	uint32_t             m_DataSize;
	uint8_t*             m_TexData;
	TTextureSDL*         m_pPrevTexture;
	TTextureSDL*         m_pNextTexture;
	uint8_t              m_Flags1;
	TEXTURE_ADDRESS_MODE m_eAddressU;
	TEXTURE_ADDRESS_MODE m_eAddressV;
	TEXTURE_FILTER       m_eFilter;
	int                  m_SamplerId;
};

class TTextureManagerSDL : public TSingleton<TTextureManagerSDL>
{
public:
	friend class TTextureSDL;

public:
	TTextureManagerSDL();
	~TTextureManagerSDL() = default;

	void AddGLAssociation( IDirect3DTexture8* a_pTexture, TTextureSDL* a_pReplaced )
	{
		auto pResult = m_AssociatedTextures.Find( a_pTexture );

		if ( pResult == m_AssociatedTextures.End() )
		{
			m_AssociatedTextures.Insert( a_pTexture, a_pReplaced );
		}
		else
		{
			TASSERT( TFALSE );
		}
	}

	TTextureSDL* GetGLAssociation( IDirect3DTexture8* a_pTexture )
	{
		auto pResult = m_AssociatedTextures.Find( a_pTexture );

		if ( pResult != m_AssociatedTextures.End() )
		{
			return pResult->GetSecond();
		}

		return m_pInvalidTexture;
	}

	TTextureSDL* RemoveGLAssociation( IDirect3DTexture8* a_pTexture )
	{
		auto pResult = m_AssociatedTextures.Find( a_pTexture );

		if ( pResult != m_AssociatedTextures.End() )
		{
			auto pTexture = pResult->GetSecond();
			m_AssociatedTextures.Remove( pResult );

			return pTexture;
		}

		return TNULL;
	}

	void SetLastTexture( TTextureSDL* pTexture )
	{
		m_pLastTexture = pTexture;
	}

	TTextureSDL* GetLastTexture() const
	{
		return m_pLastTexture;
	}

	TTextureSDL* GetInvalidTexture() const
	{
		return m_pInvalidTexture;
	}

	TTextureSDL* GetWhiteTexture() const
	{
		return m_pWhiteTexture;
	}

	TTextureSDL* FindTexture( const char* texName );

	void AddTexture( TTextureSDL* pTexture );

#ifdef TOSHI_DEBUG
public:
	size_t DEBUG_GetNumTextures() const
	{
		return s_NumTextures;
	}

private:
	inline static size_t s_NumTextures = 0;
#endif // TOSHI_DEBUG

private:
	TTextureSDL*                            m_pLastTexture;
	TTextureSDL*                            m_pInvalidTexture;
	TTextureSDL*                            m_pWhiteTexture;
	T2Map<IDirect3DTexture8*, TTextureSDL*> m_AssociatedTextures;
};
} // namespace Toshi
