#pragma once
#include "TTexture.h"

namespace Toshi
{
	class TRegMaterial;
	class TShader;

	class TMaterial :
		public TGenericClassDerived<TMaterial, TObject, "TMaterial", TMAKEVERSION(1, 0), TFALSE>
	{
	public:
		static constexpr size_t NAMESIZELIMIT = 31;

		typedef uint32_t Flags;
		enum Flags_ : Flags
		{
			Flags_NULL         = 0,
			Flags_Unk1         = BITFIELD(0),
			Flags_Unk2         = BITFIELD(1),
			Flags_Unk3         = BITFIELD(2),
			Flags_Unk4         = BITFIELD(3),
			Flags_Created      = BITFIELD(4),
			Flags_NoDepthTest  = BITFIELD(27),
			Flags_AlphaUpdate  = BITFIELD(28),
			Flags_AlphaTexture = BITFIELD(31),
		};

	public:
		TMaterial()
		{
			m_Flags = Flags_NULL;
			m_Unk = TNULL;
			m_pRegMaterial = TNULL;
			m_Name[0] = '\0';
			m_pOwnerShader = TNULL;
			m_pTexture = TNULL;
		}

		virtual ~TMaterial() = default;

		virtual void OnDestroy()
		{
			TASSERT(TTRUE == IsCreated());
			m_Flags &= ~Flags_Created;
			m_Unk = TNULL;
		}

		virtual TBOOL Create()
		{
			TASSERT(TFALSE == IsCreated());
			m_Flags |= Flags_Created;
			return TTRUE;
		}

		virtual void PreRender()
		{

		}

		virtual void PostRender()
		{

		}

		void SetOwnerShader(TShader* pShader)
		{
			TASSERT(TNULL == m_pOwnerShader);
			m_pOwnerShader = pShader;
		}

		TShader* GetOwnerShader() const
		{
			return m_pOwnerShader;
		}

		void SetTexture(TTexture* pTexture)
		{
			m_pTexture = pTexture;
		}

		TTexture* GetTexture() const
		{
			return m_pTexture;
		}

		void SetName(const char* name)
		{
			if (name == TNULL)
			{
				m_Name[0] = '\0';
			}
			else
			{
				T2String8::Copy(m_Name, name, NAMESIZELIMIT);
			}
		}

		void SetFlag(Flags flag, TBOOL set = TFALSE)
		{
			if (set)
			{
				if (flag & (Flags_Unk1 | Flags_Unk2 | Flags_Unk3))
				{
					m_Flags &= ~(Flags_Unk1 | Flags_Unk2 | Flags_Unk3);
				}

				m_Flags |= flag;
			}
			else
			{
				m_Flags &= ~flag;
			}
		}

		TBOOL IsCreated() const
		{
			return m_Flags & Flags_Created;
		}

		void SetRegMaterial(TRegMaterial* pRegMaterial)
		{
			m_pRegMaterial = pRegMaterial;
		}

		TRegMaterial* GetRegMaterial() const
		{
			return m_pRegMaterial;
		}

	protected:
		Flags m_Flags;                  // 0x04
		TShader* m_pOwnerShader;        // 0x08
		TRegMaterial* m_pRegMaterial;   // 0x0C
		TTexture* m_pTexture;           // 0x14
		char m_Name[NAMESIZELIMIT + 1]; // 0x2C
		void* m_Unk;                    // 0x4C
	};
}