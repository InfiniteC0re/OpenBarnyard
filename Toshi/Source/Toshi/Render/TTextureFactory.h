#pragma once
#include "TTextureResourceFormat.h"

#include "Toshi/Core/TNodeList.h"
#include "Toshi/Render/TResource.h"

#include TOSHI_MULTIRENDER(TTextureResourceHAL)

namespace Toshi {

	class TTexture;

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
		virtual TTextureResourceHAL* CreateTextureFromFile(const char* a_szFilePath, TUINT32 a_uiFlags) = 0;
		virtual TTextureResourceHAL* CreateTextureFromMemory(void* a_pMem, TUINT32 a_uiMemSize, TUINT32 a_uiWidth, TUINT32 a_uiHeight, TUINT32 a_uiFlags) = 0;
		virtual TTextureResourceHAL* CreateEx(void* a_pMem, TUINT32 a_uiMemSize, TUINT32 a_uiWidth, TUINT32 a_uiHeight, TUINT32 a_uiFlags, TTEXTURERESOURCEFORMAT a_eFormat, TUINT32 a_uiUnknown) = 0;
		virtual TTextureResourceHAL* CreateUnk(void* a_pUnk) = 0; // TODO: Figure out it's name and parameters

		TTexture* FindTexture(const char* a_szName) const;
		NameEntry* RegisterTexture(const char* a_szName, TTexture* a_pTexture);
		void DeregisterTexture(TTexture* a_pTexture);

		static TUINT32 HashName(const char* a_szName);

	private:
		TNodeList<NameEntry> m_aLists[NUM_LISTS];
	};

}