#pragma once
#include "TTexture.h"
#include "Toshi/Core/TNodeList.h"
#include "Toshi/Render/TResource.h"

namespace Toshi {

	class TTextureFactory : 
		public TGenericClassDerived<TTextureFactory, TResource, "TTextureFactory", TMAKEVERSION(1, 0), TTRUE>
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

		constexpr static TUINT32 NUM_LISTS = 50;

	public:
		TTextureFactory() = default;
		~TTextureFactory();

		virtual void Dump() const;

	private:
		TNodeList<NameEntry> m_aLists[NUM_LISTS];
	};

}