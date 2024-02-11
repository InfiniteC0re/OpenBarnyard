#pragma once
#include "TTexture.h"

namespace Toshi {

	class TRegMaterial;
	class TShader;

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
			Flags_NO_CULL      = BITFIELD(0),
			Flags_Unk2         = BITFIELD(1),
			Flags_Unk3         = BITFIELD(2),
			Flags_Unk4         = BITFIELD(3),
			Flags_Created      = BITFIELD(4),
			Flags_NoDepthTest  = BITFIELD(27),
			Flags_AlphaUpdate  = BITFIELD(28),
			Flags_AlphaTexture = BITFIELD(31),
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
		void SetShader(TShader* pShader);

		TTexture* GetTexture(TUINT32 a_iStage = 0) const;
		void SetTextureNum(TUINT32 a_iNumTextures);
		void SetTexture(TUINT32 a_iStage, TTexture* a_pTexture);		

		void SetName(const TCHAR* name);
		void SetFlag(Flags flag, TBOOL set = TFALSE);

		TBOOL IsCreated() const;

		TRegMaterial* GetRegMaterial() const;
		void SetRegMaterial(TRegMaterial* pRegMaterial);

	protected:
		Flags m_Flags;                      // 0x04
		TShader* m_pOwnerShader;            // 0x08
		TRegMaterial* m_pRegMaterial;       // 0x0C
		TUINT32 m_iNumTex;                  // 0x10
		TTexture* m_pTextures[MAXTEXTURES]; // 0x14
		TCHAR m_Name[NAMESIZELIMIT + 1];    // 0x2C
	};

}