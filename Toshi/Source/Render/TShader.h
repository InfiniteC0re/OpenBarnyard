#pragma once
#include "TOrderTable.h"

namespace Toshi {
	
	class TShader : public TObject
	{
	public:
		TDECLARE_CLASS(TShader, TObject);

	public:
		using State = TUINT32;
		enum State_ : State
		{
			State_None      = 0,
			State_Created   = BITFLAG(0),
			State_Validated = BITFLAG(1),
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

		/* Called before shader is destroyed. */
		virtual void OnDestroy();

		/* Additional call to flush something more? */
		virtual void Flush();

		/* Setup render state before rendering meshes. */
		virtual void StartFlush() = 0;

		/* Restore render states after all rendering is done. */
		virtual void EndFlush() = 0;

		/* Do some initial setups, f.e. create order tables. */
		virtual TBOOL Create();

		/* Make sure the shader is ready to use. */
		virtual TBOOL Validate();

		/* We don't need the shader, at least for now. */
		virtual void Invalidate();

		/* Why? I don't know */
		virtual TBOOL TryInvalidate() { return TTRUE; }
		
		/* Why? I don't know */
		virtual TBOOL TryValidate() { return TTRUE; }
		
		/* Renders the mesh stored in the render packet containing other useful info. */
		virtual void Render(TRenderPacket* a_pRenderPacket) = 0;

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
