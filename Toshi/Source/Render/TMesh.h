#pragma once
#include "TMaterial.h"

namespace Toshi {

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

		// These flags can be used by the client classes
		using FLAG = TUINT32;
		enum FLAG_ : FLAG
		{
			FLAG_LOCKED = 0x8000
		};

	public:
		TMesh();

		//-----------------------------------------------------------------------------
		// Own methods
		//-----------------------------------------------------------------------------
		virtual TBOOL Validate();
		virtual void Invalidate();
		virtual TBOOL Create();
		virtual TBOOL Render();
		virtual void OnDestroy();

		void DestroyResource();

		TMaterial* GetMaterial() const;
		void SetMaterial(TMaterial* pMaterial);

		TShader* GetOwnerShader();
		void SetOwnerShader(TShader* pShader);

		TBOOL IsCreated() const;
		TBOOL IsValidated() const;

	protected:
		~TMesh();

	protected:
		TMaterial* m_pMaterial;  // 0x04
		TShader* m_pOwnerShader; // 0x08
		State m_State;           // 0x0C
	};

}
