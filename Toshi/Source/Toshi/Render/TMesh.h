#pragma once
#include "TMaterial.h"

namespace Toshi
{
	class TShader;

	class TMesh :
		public TGenericClassDerived<TMesh, TObject, "TMesh", TMAKEVERSION(1, 0), false>
	{
	public:
		enum class State : uint32_t
		{
			None = 0,
			Created = BITFIELD(0),
			Validated = BITFIELD(1),
		};

	public:
		TMesh()
		{
			m_pOwnerShader = TNULL;
			m_pMaterial = TNULL;
			m_State = State::None;
		}

		virtual TBOOL Validate()
		{
			m_State.Set(State::Validated);
			return true;
		}

		virtual void Invalidate()
		{
			m_State.Unset(State::Validated);
		}

		virtual TBOOL Create()
		{
			m_State.Set(State::Created);
			return true;
		}

		virtual TBOOL Render()
		{
			return true;
		}

		virtual void OnDestroy()
		{
			m_State.Unset(State::Created);
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
			return m_State.IsSet(State::Created);
		}

	protected:
		TMaterial* m_pMaterial;  // 0x04
		TShader* m_pOwnerShader; // 0x08
		T2Flags<State> m_State;  // 0x0C
	};

	DEFINE_T2FLAGS(TMesh::State);
}