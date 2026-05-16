#pragma once
#include "AHudElement.h"

#include <Toshi/TSingleton.h>
#include <Toshi/TTask.h>

class AHudManager
    : public Toshi::TTask
    , public Toshi::TSingleton<AHudManager>
{
public:
	TDECLARE_CLASS( AHudManager, Toshi::TTask );

public:
	AHudManager();
	virtual ~AHudManager();

	virtual TBOOL Reset() override;
	virtual TBOOL OnCreate() override;
	virtual TBOOL OnUpdate( TFLOAT a_fDeltaTime ) override;
	virtual void  OnDestroy() override;
	virtual void  OnActivate() override;
	virtual void  OnDeactivate() override;

	TBOOL        AddElement( AHudElement* a_pElement, TBOOL a_bOwnedByManager, TBOOL a_bInsertTail = TTRUE, TBOOL a_bShow = TFALSE );
	AHudElement* FindElement( const Toshi::TClass* a_pClass );

	template <class T>
	T* FindElement()
	{
		return TSTATICCAST( T, FindElement( &TGetClass( T ) ) );
	}

private:
	TUINT32 GetCurrentHUDMask() const;
	void    UpdateElementsVisibility( TBOOL a_bForceShow );

private:
	Toshi::T2DList<AHudElement> m_Elements;
};
