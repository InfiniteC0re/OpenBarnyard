#pragma once
#include "TOrderTable.h"

namespace Toshi
{
	class TShader :
		public TGenericClassDerived<TShader, TObject, "TShader", TMAKEVERSION(1, 0), false>
	{
	public:
		typedef uint32_t State;
		enum State_ : State
		{
			State_Created = BITFIELD(0),
			State_Validated = BITFIELD(1),
		};

	public:
		TShader() = default;
		virtual ~TShader();

		virtual void OnDestroy();
		virtual void Flush();
		virtual void StartFlush() = 0;
		virtual void EndFlush() = 0;
		virtual void Create();
		virtual void Validate();
		virtual void Invalidate();
		virtual const char* GetShaderPrefix() = 0;
		virtual bool Unk1() { return TTRUE; }
		virtual bool Unk2() { return TTRUE; }
		virtual void Render(TRenderPacket* pPacket) = 0;

	private:
		State m_State;
	};
}