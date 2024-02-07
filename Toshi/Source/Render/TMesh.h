#pragma once
#include "TMaterial.h"

namespace Toshi
{
	class TShader;

	class TMesh : public TObject
	{
	public:
		TDECLARE_CLASS(TObject);

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

		virtual TBOOL Validate();
		virtual void Invalidate();
		virtual TBOOL Create();
		virtual TBOOL Render();
		virtual void OnDestroy();

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

		TBOOL IsValidated() const
		{
			return m_State & State_Validated;
		}

	protected:
		~TMesh() = default;

	protected:
		TMaterial* m_pMaterial;  // 0x04
		TShader* m_pOwnerShader; // 0x08
		State m_State;           // 0x0C
	};

}
