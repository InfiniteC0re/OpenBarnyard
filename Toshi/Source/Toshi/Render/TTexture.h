#pragma once
#include "Toshi/Render/TResource.h"
#include "Toshi/Render/TRenderAdapter.h"

namespace Toshi {
	
	enum class TTEXTURERESOURCEFORMAT
	{

	};

	class TTexture :
		public TGenericClassDerived<TTexture, TResource, "TTexture", TMAKEVERSION(1, 0), TTRUE>
	{
	public:
		struct LOCKSTATE
		{

		};

	public:
		TTexture();
		~TTexture();

		virtual TINT GetWidth() const = 0;
		virtual TINT GetHeight() const = 0;
		virtual TBOOL Lock(LOCKSTATE& a_rLockState);
		virtual void Unlock();
		virtual void Unknown(void* a_pUnknown) = 0;
		virtual TBOOL Create(void*, TUINT, TUINT, TUINT, TUINT) = 0;
		virtual TBOOL Create(const char*, TUINT) = 0;
		virtual TBOOL CreateEx(void*, TUINT, TUINT, TUINT, TUINT, TTEXTURERESOURCEFORMAT, TUINT);

	private:
		int m_Unk1;
		int m_Unk2;
		int m_Unk3;
	};

}