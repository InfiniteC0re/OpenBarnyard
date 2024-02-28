#include "ToshiPCH.h"
#include "TMaterial.h"
#include "TOrderTable.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

namespace Toshi {

	TDEFINE_CLASS_NORUNTIME(TMaterial);

	TMaterial::TMaterial()
	{
		m_Flags = Flags_NULL;
		m_pShader = TNULL;
		m_iNumTex = 0;
		TUtil::MemClear(m_pTextures, sizeof(m_pTextures));
		m_pRegMaterial = TNULL;
		m_Name[0] = '\0';
	}

	TMaterial::~TMaterial()
	{

	}

	void TMaterial::OnDestroy()
	{
		TASSERT(TTRUE == IsCreated());
		m_Flags &= ~Flags_Created;
	}

	TBOOL TMaterial::Create()
	{
		TASSERT(TFALSE == IsCreated());
		m_Flags |= Flags_Created;
		return TTRUE;
	}

	void TMaterial::PreRender()
	{

	}

	void TMaterial::PostRender()
	{

	}

	void TMaterial::Destroy()
	{
		OnDestroy();
		delete this;
	}

	void TMaterial::SetShader(TShader* pShader)
	{
		TASSERT(TNULL == m_pShader);
		m_pShader = pShader;
	}

	TShader* TMaterial::GetShader() const
	{
		return m_pShader;
	}

	void TMaterial::SetTextureNum(TUINT32 a_iNumTextures)
	{
		TASSERT(a_iNumTextures <= MAXTEXTURES);
		m_iNumTex = a_iNumTextures;
	}

	void TMaterial::SetTexture(TUINT32 a_iStage, TTexture* a_pTexture)
	{
		TASSERT(a_iStage < MAXTEXTURES);
		m_pTextures[a_iStage] = a_pTexture;
	}

	TUINT32 TMaterial::GetTextureNum() const
	{
		return m_iNumTex;
	}

	TTexture* TMaterial::GetTexture(TUINT32 a_iStage /*= 0*/) const
	{
		return m_pTextures[a_iStage];
	}

	void TMaterial::SetName(const TCHAR* name)
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

	void TMaterial::SetFlag(Flags flag, TBOOL set /*= TFALSE*/)
	{
		if (set)
		{
			if (flag & (Flags_NO_CULL | Flags_Unk2 | Flags_Unk3))
			{
				m_Flags &= ~(Flags_NO_CULL | Flags_Unk2 | Flags_Unk3);
			}

			m_Flags |= flag;
		}
		else
		{
			m_Flags &= ~flag;
		}
	}

	TBOOL TMaterial::IsCreated() const
	{
		return m_Flags & Flags_Created;
	}

	TRenderPacket* TMaterial::AddRenderPacket(TMesh* a_pMesh)
	{
		TVALIDPTR(a_pMesh);
		TVALIDPTR(m_pRegMaterial);

		return m_pRegMaterial->AddRenderPacket(a_pMesh);
	}

	void TMaterial::SetRegMaterial(TRegMaterial* pRegMaterial)
	{
		m_pRegMaterial = pRegMaterial;
	}

	TRegMaterial* TMaterial::GetRegMaterial() const
	{
		return m_pRegMaterial;
	}

}
