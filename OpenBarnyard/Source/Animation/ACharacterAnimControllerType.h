#pragma once
#include "AAnimControllerType.h"

//-----------------------------------------------------------------------------
// Purpose: Type of ACharacterAnimController. Also the C++ base of the types of
// the other character-like controllers (ABasic, AAlien, AAttached,
// AChasingChicks), which share its Create.
// Properties: UserRender (bool, default false) - render one mesh a frame
// (ACharacterAnimController::UserRenderCallback).
//-----------------------------------------------------------------------------
class ACharacterAnimControllerType
    : public AAnimControllerType
{
public:
	TDECLARE_CLASS( ACharacterAnimControllerType, AAnimControllerType );

public:
	ACharacterAnimControllerType();
	~ACharacterAnimControllerType();

	//-----------------------------------------------------------------------------
	// AAnimControllerType
	//-----------------------------------------------------------------------------
	virtual TBOOL          Create( const PBProperties* a_pProperties ) OVERRIDE; // +0x0c
	virtual Toshi::TClass* GetControllerClass() OVERRIDE;                        // +0x10

	TBOOL IsUserRender() const { return m_bUserRender; }

private:
	TBOOL m_bUserRender; // 0x0c
};
