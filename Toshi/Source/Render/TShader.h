#pragma once
#include "TOrderTable.h"

namespace Toshi {
	
	class TShader : public TObject
	{
	public:
		TDECLARE_CLASS(TObject);

	public:
		using State = TUINT32;
		enum State_ : State
		{
			State_None      = 0,
			State_Created   = BITFIELD(0),
			State_Validated = BITFIELD(1),
		};

		class TShaderList
		{
		public:
			TShaderList() : m_pRoot(TNULL) { }

			void AddShader(TShader* pShader);
			void RemoveShader(TShader* pShader);

			TShader* GetRootShader() { return m_pRoot; }

		private:
			TShader* m_pRoot;
		};

	public:
		TShader();
		~TShader();

		//-----------------------------------------------------------------------------
		// Own methods
		//-----------------------------------------------------------------------------
		virtual void OnDestroy();
		virtual void Flush();
		virtual void StartFlush() = 0;
		virtual void EndFlush() = 0;
		virtual TBOOL Create();
		virtual TBOOL Validate();
		virtual void Invalidate();
		virtual TBOOL TryInvalidate() { return TTRUE; }
		virtual TBOOL TryValidate() { return TTRUE; }
		virtual void Render(TRenderPacket* pPacket) = 0;

		TBOOL IsCreated() const { return m_State & State_Created; }
		TBOOL IsValidated() const { return m_State & State_Validated; }

		TShader* GetNextShader() { return m_pNextShader; }

	public:
		inline static TShaderList sm_oShaderList;

	private:
		State m_State;
		TShader* m_pNextShader;
	};

}
