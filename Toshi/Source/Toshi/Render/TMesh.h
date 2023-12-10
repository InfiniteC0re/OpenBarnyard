#pragma once
#include "TMaterial.h"

namespace Toshi
{
	class TShader;

	class TMesh :
		public TGenericClassDerived<TMesh, TObject, "TMesh", TMAKEVERSION(1, 0), TFALSE>
	{
	public:
		using State = TUINT32;
		enum State_ : State
		{
			State_None = 0,
			State_Created = BITFIELD(0),
			State_Validated = BITFIELD(1),
		};

	public:
		TMesh()
		{
			m_pOwnerShader = TNULL;
			m_pMaterial = TNULL;
			m_State = State_None;
		}

		virtual TBOOL Validate()
		{
			m_State |= State_Validated;
			return TTRUE;
		}

		virtual void Invalidate()
		{
			m_State &= ~State_Validated;
		}

		virtual TBOOL Create()
		{
			m_State |= State_Created;
			return TTRUE;
		}

		virtual TBOOL Render()
		{
			return TTRUE;
		}

		virtual void OnDestroy()
		{
			m_State &= ~State_Created;
		}

		void DestroyResource()
		{
			Invalidate();
			OnDestroy();
			delete this;
		}

		void SetMaterial(TMaterial* pMaterial)
		{
			m_pMaterial = pMaterial;
		}

		TMaterial* GetMaterial() const
		{
			return m_pMaterial;
		}

		void SetOwnerShader(TShader* pShader)
		{
			TASSERT(TNULL == m_pOwnerShader);
			m_pOwnerShader = pShader;
		}

		TShader* GetOwnerShader()
		{
			return m_pOwnerShader;
		}

		TBOOL IsCreated() const
		{
			return m_State & State_Created;
		}

	protected:
		~TMesh()
		{

		}

	protected:
		TMaterial* m_pMaterial;  // 0x04
		TShader* m_pOwnerShader; // 0x08
		State m_State;           // 0x0C
	};

}
