#pragma once
#include "Toshi/Render/TResource.h"
#include "Toshi/Render/TRenderAdapter.h"

#include "TTextureFactory.h"

namespace Toshi {

	class T2Texture;

	class TTexture :
		public TGenericClassDerived<TTexture, TResource, "TTexture", TMAKEVERSION(1, 0), TFALSE>
	{
	public:
		struct LOCKSTATE
		{
			TINT Pitch;
			void* pBits;
		};

		friend TTextureFactory;

	public:
		TTexture();
		~TTexture();

		virtual TUINT GetWidth() = 0;
		virtual TUINT GetHeight() = 0;
		virtual TBOOL Lock(LOCKSTATE& a_rLockState);
		virtual void Unlock();
		virtual void CreateFromT2Texture(T2Texture* a_pTexture) = 0;
		virtual TBOOL Create(void* a_pData, TUINT a_uiDataSize, TUINT a_eTextureFlags, TUINT a_uiWidth, TUINT a_uiHeight) = 0;
		virtual TBOOL Create(const char* a_szFileName, TUINT a_eTextureFlags) = 0;
		virtual TBOOL CreateEx(void* a_pData, TUINT a_uiDataSize, TUINT a_uiWidth, TUINT a_uiHeight, TUINT a_uiMipLevels, TTEXTURERESOURCEFORMAT a_eFormat, BOOL a_bNoMipLevels);

		TUINT GetAddress() const
		{
			return m_eAddress;
		}

		TBOOL CreateResource(void* a_pData, TUINT a_uiDataSize, TUINT a_eTextureFlags, TUINT a_uiWidth, TUINT a_uiHeight)
		{
			return TResource::Create();
		}

		TBOOL CreateResource(const char* a_szFileName, TUINT a_eTextureFlags)
		{
			return TResource::Create();
		}

		TBOOL CreateResource(void* a_pData, TUINT a_uiDataSize, TUINT a_uiWidth, TUINT a_uiHeight, TUINT a_uiMipLevels, TTEXTURERESOURCEFORMAT a_eFormat, BOOL a_bNoMipLevels)
		{
			return TResource::Create();
		}

	protected:
		TUINT m_eAddress;
		TUINT m_eTextureFlags;
		TTextureFactory::NameEntry* m_pNameEntry;
	};

}