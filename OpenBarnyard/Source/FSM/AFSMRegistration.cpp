#include "pch.h"
#include "CVar/AVarType.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

VARTYPE_REGISTER( bool, "bool" );
VARTYPE_REGISTER( int, "int" );
VARTYPE_REGISTER( float, "float" );
VARTYPE_REGISTER( string, "string" );
VARTYPE_REGISTER( unit, "unit" );

VARTYPE_REGISTER( Position, "Position" );
VARTYPE_REGISTER( Rotation, "Rotation" );
VARTYPE_REGISTER( PrimType, "PrimType" );
VARTYPE_REGISTER( ePrimCapsule, "ePrimCapsule" );
VARTYPE_REGISTER( ePrimSphere, "ePrimSphere" );
VARTYPE_REGISTER( ePrimBox, "ePrimBox" );
VARTYPE_REGISTER( Radius, "Radius" );
VARTYPE_REGISTER( Length, "Length" );
VARTYPE_REGISTER( Sides, "Sides" );
VARTYPE_REGISTER( SceneRoot, "Scene Root" );
VARTYPE_REGISTER( BoneName, "BoneName" );
VARTYPE_REGISTER( Geom_Max, "Geom_Max" );
VARTYPE_REGISTER( Animations1, "Animations" ); // one of those two is wrong and cannot be restored, the real name is unknown
VARTYPE_REGISTER( Animations2, "Animations" );
VARTYPE_REGISTER( brth_lp, "brth_lp" );
