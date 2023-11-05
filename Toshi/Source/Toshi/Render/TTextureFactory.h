#pragma once
#include "TTextureResourceFormat.h"

#include "Toshi/Core/TNodeList.h"
#include "Toshi/Render/TResource.h"

namespace Toshi {

	class TTexture;
	class T2Texture;

	class TTextureFactory : 
		public TGenericClassDerived<TTextureFactory, TResource, "TTextureFactory", TMAKEVERSION(1, 0), TFALSE>
	{
	public:
		class NameEntry : public TNodeList<NameEntry>::TNode
		{
		public:
			constexpr static TUINT32 MAX_TEXTURENAMELEN = 50;

		public:
			NameEntry(const char* a_szName, TTexture* a_pTexture);
			~NameEntry();

			const char* GetName() const { return m_szName; }
			TTexture* GetTexture() { return m_pTexture; }

		private:
			char m_szName[MAX_TEXTURENAMELEN];
			TTexture* m_pTexture;
		};

		constexpr static TUINT32 NUM_LISTS = 1024;

	public:
		TTextureFactory() = default;
		~TTextureFactory();

		virtual void Dump() const;
		virtual TTexture* CreateTextureFromFile(const char* a_szFilePath, TUINT32 a_eTextureFlags) = 0;
		virtual TTexture* CreateTextureFromMemory(void* a_pData, TUINT a_uiDataSize, TUINT a_eTextureFlags, TUINT a_uiWidth, TUINT a_uiHeight) = 0;
		virtual TTexture* CreateEx(void* a_pData, TUINT a_uiDataSize, TUINT a_uiWidth, TUINT a_uiHeight, TUINT a_uiMipLevels, TTEXTURERESOURCEFORMAT a_eFormat, BOOL a_bNoMipLevels) = 0;
		virtual TTexture* CreateFromT2Texture(T2Texture* a_pTexture) = 0;

		TTexture* FindTexture(const char* a_szName) const;
		NameEntry* RegisterTexture(const char* a_szName, TTexture* a_pTexture);
		void DeregisterTexture(TTexture* a_pTexture);

		static TUINT32 HashName(const char* a_szName);

	private:
		TNodeList<NameEntry> m_aLists[NUM_LISTS];
	};

}