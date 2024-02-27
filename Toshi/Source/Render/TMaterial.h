#pragma once
#include "TTexture.h"

namespace Toshi {

	class TRenderPacket;
	class TRegMaterial;
	class TShader;
	class TMesh;

	class TMaterial : public TObject
	{
	public:
		TDECLARE_CLASS(TObject);

	public:
		static constexpr TUINT MAXTEXTURES = 6;
		static constexpr TUINT NAMESIZELIMIT = 31;

		typedef TUINT32 Flags;
		enum Flags_ : Flags
		{
			Flags_NULL         = 0,
			Flags_NO_CULL      = BITFLAG(0),
			Flags_Unk2         = BITFLAG(1),
			Flags_Unk3         = BITFLAG(2),
			Flags_Unk4         = BITFLAG(3),
			Flags_Created      = BITFLAG(4),
			Flags_NoDepthTest  = BITFLAG(27),
			Flags_AlphaUpdate  = BITFLAG(28),
			Flags_AlphaTexture = BITFLAG(31),
		};

	public:
		TMaterial();
		~TMaterial();

		//-----------------------------------------------------------------------------
		// Own methods
		//-----------------------------------------------------------------------------
		virtual void OnDestroy();
		virtual TBOOL Create();
		virtual void PreRender();
		virtual void PostRender();

		TShader* GetShader() const;
		void SetShader(TShader* a_pShader);

		TTexture* GetTexture(TUINT32 a_iStage = 0) const;
		void SetTexture(TUINT32 a_iStage, TTexture* a_pTexture);		
		void SetTextureNum(TUINT32 a_iNumTextures);

		void SetName(const TCHAR* a_szName);
		void SetFlag(Flags a_eFlag, TBOOL a_bEnable);

		TRegMaterial* GetRegMaterial() const;
		void SetRegMaterial(TRegMaterial* a_pRegMaterial);

		TBOOL IsCreated() const;

		TRenderPacket* AddRenderPacket(TMesh* a_pMesh);

	protected:
		Flags m_Flags;                      // 0x04
		TShader* m_pShader;                 // 0x08
		TRegMaterial* m_pRegMaterial;       // 0x0C
		TUINT32 m_iNumTex;                  // 0x10
		TTexture* m_pTextures[MAXTEXTURES]; // 0x14
		TCHAR m_Name[NAMESIZELIMIT + 1];    // 0x2C
	};

}