#pragma once
#include "TOrderTable.h"

namespace Toshi {
	
	class TShader :
		public TGenericClassDerived<TShader, TObject, "TShader", TMAKEVERSION(1, 0), TFALSE>
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
			TShaderList() : m_pRoot(TNULL) { }

			void AddShader(TShader* pShader);
			void RemoveShader(TShader* pShader);

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
		virtual TBOOL Unk1() { return TTRUE; }
		virtual TBOOL Unk2() { return TTRUE; }
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