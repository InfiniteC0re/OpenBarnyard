#pragma once
#include <Toshi/T2DList.h>
#include <Plugins/PPropertyParser/PBProperties.h>

//-----------------------------------------------------------------------------
// Purpose: Describes an animation controller of an AAnimatableObjectType.
// Created by name from the "Type" of an "AnimControllerType" block
// (AAnimatableObjectType::SetAnimController) and reads the rest of the block.
//-----------------------------------------------------------------------------
class AAnimControllerType
    : public Toshi::TObject
    , public Toshi::T2DList<AAnimControllerType>::Node
{
public:
	TDECLARE_CLASS( AAnimControllerType, Toshi::TObject );

public:
	AAnimControllerType();
	virtual ~AAnimControllerType();

	// RETURNS_TRUE 00413920
	virtual TBOOL Create( const PBProperties* a_pProperties ) { return TTRUE; } // +0x0c

	// Class of the AAnimController the objects of the type create
	virtual Toshi::TClass* GetControllerClass() = 0; // +0x10
};
