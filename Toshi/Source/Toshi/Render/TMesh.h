#pragma once
#include "TMaterial.h"

namespace Toshi
{
	class TShader;

	class TMesh :
		public TGenericClassDerived<TMesh, TObject, "TMesh", TMAKEVERSION(1, 0), false>
	{
	public:
		typedef uint32_t State;
		enum State_ : State
		{
			State_Created = BITFIELD(0),
			State_Validated = BITFIELD(1),
		};

	public:
		virtual bool Validate()
		{
			m_State |= State_Validated;
			return true;
		}

		virtual void Invalidate()
		{
			m_State &= ~State_Validated;
		}

		virtual bool Create()
		{
			m_State |= State_Created;
			return true;
		}

		virtual bool Render()
		{
			return true;
		}

		virtual void OnDestroy()
		{
			m_State &= State_Created;
		}

		void DestroyResource()
		{
			Invalidate();
			OnDestroy();
			delete this;
		}

		TMaterial* GetMaterial() const
		{
			return m_pMaterial;
		}

		void SetOwnerShader(TShader* pShader)
		{
			m_pOwnerShader = pShader;
		}

		TShader* GetOwnerShader()
		{
			return m_pOwnerShader;
		}

	private:
		TMaterial* m_pMaterial;  // 0x04
		TShader* m_pOwnerShader; // 0x08
		State m_State;           // 0x0C
	};
}