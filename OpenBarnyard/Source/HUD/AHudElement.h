#pragma once
#include <Toshi/T2DList.h>
#include <Toshi/TObject.h>

class AHudElement
    : public Toshi::TObject
    , public Toshi::T2DList<AHudElement>::Node
{
public:
	TDECLARE_CLASS( AHudElement, Toshi::TObject );

	using Mask = TUINT32;
	enum Mask_ : Mask
	{
		MASK_NONE = 0,
		MASK_ALL  = 0xFFFFFFFF
	};

	friend class AHudManager;

public:
	AHudElement( Mask a_eMask = MASK_ALL );
	virtual ~AHudElement();

	virtual void  Unknown1();
	virtual void  Unknown2();
	virtual TBOOL OnAttach();
	virtual void  OnDetach();
	virtual void  Show();
	virtual void  Hide();
	virtual void  Update( TFLOAT a_fDeltaTime );
	virtual void  Reset();

	void Detach();

	void SetMask( Mask a_eMask ) { m_eMask = a_eMask; }
	Mask GetMask() const { return m_eMask; }

	void  SetForceHide( TBOOL a_bForceHide ) { m_bForceHide = a_bForceHide; }
	TBOOL IsForceHidden() const { return m_bForceHide; }
	TBOOL IsVisible() const { return m_bCurrentlyVisible; }
	TBOOL IsOwnedByManager() const { return m_bOwnedByManager; }

protected:
	Mask  m_eMask;
	TBOOL m_bForceHide;
	TBOOL m_bCurrentlyVisible;
	TBOOL m_bOwnedByManager;
};
