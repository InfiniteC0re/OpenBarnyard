#pragma once
#include "AInteractionRequirement.h"

//-----------------------------------------------------------------------------
// Created by AInteractionManager::FindOrCreateRequirement for a requirement
// name that an interaction uses before (or without) it being defined, replaced
// once the real one is loaded. Evaluate does nothing, so the roles stay as
// AInteraction::EvaluateRequirements reset them: a placeholder that is never
// replaced (unknown name, or a requirement class that isn't reconstructed)
// is ALWAYS satisfied.
//-----------------------------------------------------------------------------
class APlaceholderRequirement
    : public AInteractionRequirement
{
public:
	TDECLARE_CLASS( APlaceholderRequirement, AInteractionRequirement );

public:
	// NOTE: TClass needs a default constructor, retail registers RETURNS_ZERO as the create functions
	APlaceholderRequirement() = default;
	APlaceholderRequirement( Toshi::TPString8 a_strName );
	~APlaceholderRequirement();

	virtual TBOOL Create( const PBProperties* a_pProperties ) OVERRIDE { return TFALSE; } // +0x0c, RETURNS_FALSE
	virtual void  Evaluate( AInteractionParticipants* a_pParticipants ) OVERRIDE {}       // +0x1c, DOES_NOTHING
};
