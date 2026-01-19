#include "pch.h"
#include "AFSMKeyWord.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

FSM_REGISTER_KEYWORD( Position, "Position" );
FSM_REGISTER_KEYWORD( Rotation, "Rotation" );
FSM_REGISTER_KEYWORD( PrimType, "PrimType" );
FSM_REGISTER_KEYWORD( ePrimCapsule, "ePrimCapsule" );
FSM_REGISTER_KEYWORD( ePrimSphere, "ePrimSphere" );
FSM_REGISTER_KEYWORD( ePrimBox, "ePrimBox" );
FSM_REGISTER_KEYWORD( Radius, "Radius" );
FSM_REGISTER_KEYWORD( Length, "Length" );
FSM_REGISTER_KEYWORD( Sides, "Sides" );
FSM_REGISTER_KEYWORD( SceneRoot, "Scene Root" );
FSM_REGISTER_KEYWORD( BoneName, "BoneName" );
FSM_REGISTER_KEYWORD( Geom_Max, "Geom_Max" );
FSM_REGISTER_KEYWORD( Animations1, "Animations" ); // one of those two is wrong and cannot be restored, the real name is unknown
FSM_REGISTER_KEYWORD( Animations2, "Animations" );
FSM_REGISTER_KEYWORD( brth_lp, "brth_lp" );
FSM_REGISTER_KEYWORD( bool, "bool" );
FSM_REGISTER_KEYWORD( int, "int" );
FSM_REGISTER_KEYWORD( float, "float" );
FSM_REGISTER_KEYWORD( string, "string" );
FSM_REGISTER_KEYWORD( unit, "unit" );
