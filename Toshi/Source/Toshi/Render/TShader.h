#pragma once
#include "TOrderTable.h"

namespace Toshi {
	
	class TShader :
		public TGenericClassDerived<TShader, TObject, "TShader", TMAKEVERSION(1, 0), false>
	{
	public:
		enum class State : uint32_t
		{
			None      = 0,
			Created   = BITFIELD(0),
			Validated = BITFIELD(1),
		};

		class TShaderList
		{
		public:
			TShaderList()
			{
				m_pRoot = TNULL;
			}

			void AddShader(TShader* pShader)
			{
				TShader* pLastShader = m_pRoot;

				while (pLastShader && pLastShader->m_pNextShader != TNULL)
					pLastShader = pLastShader->m_pNextShader;

				if (pLastShader != TNULL)
					pLastShader->m_pNextShader = pShader;
				else
					m_pRoot = pShader;

				pShader->m_pNextShader = TNULL;
			}

			void RemoveShader(TShader* pShader)
			{
				TShader* pCurrShader = m_pRoot;
				TShader* pPrevShader = TNULL;
				TBOOL bFound = TFALSE;

				while (pCurrShader)
				{
					if (pCurrShader == pShader)
					{
						bFound = TTRUE;
						break;
					}

					pPrevShader = pCurrShader;
					pCurrShader = pCurrShader->m_pNextShader;
				}

				if (bFound)
				{
					if (pPrevShader)
					{
						TASSERT(m_pRoot != pShader);
						pPrevShader->m_pNextShader = pShader->m_pNextShader;
					}
					else
					{
						TASSERT(m_pRoot == pShader);
						m_pRoot = pShader->m_pNextShader;
					}
				}
			}

		private:
			TShader* m_pRoot;
		};

	public:
		TShader();
		virtual ~TShader();

		virtual void OnDestroy();
		virtual void Flush();
		virtual void StartFlush() = 0;
		virtual void EndFlush() = 0;
		virtual TBOOL Create();
		virtual TBOOL Validate();
		virtual void Invalidate();
		virtual const char* GetShaderPrefix() = 0;
		virtual bool Unk1() { return TTRUE; }
		virtual bool Unk2() { return TTRUE; }
		virtual void Render(TRenderPacket* pPacket) = 0;

		TBOOL IsCreated() const { return m_State.IsSet(State::Created); }
		TBOOL IsValidated() const { return m_State.IsSet(State::Validated); }

	public:
		inline static TShaderList sm_oShaderList;

	private:
		T2Flags<State> m_State;
		TShader* m_pNextShader;
	};

	DEFINE_T2FLAGS(TShader::State)
}