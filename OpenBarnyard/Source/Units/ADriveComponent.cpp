#include "pch.h"
#include "ADriveComponent.h"
#include "ADriveSystem.h"
#include "AGeneralUnitComponentManager.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 0054dd00, CreateObject 0054de00, CreateObjectInPlace 0054de60
TDEFINE_CLASS( ADriveComponent );

// $Barnyard: FUNCTION 0054dd10
ADriveComponent::ADriveComponent()
    : m_pDriveSystem( TNULL )
{
}

// $Barnyard: FUNCTION 0054dd60
// $Barnyard: FUNCTION 0054dea0
ADriveComponent::~ADriveComponent()
{
}

// NOTE: folded with AUnitProfileComponent::GetManager (005d2300)
AUnitComponentManager* ADriveComponent::GetManager()
{
	return AGeneralUnitComponentManager::GetSingletonSafe();
}

// $Barnyard: FUNCTION 0054ddc0
AInteractionActivityData* ADriveComponent::Initialise( AUnit* a_pUnit, ADriveSystem* a_pDriveSystem )
{
	m_pDriveSystem = a_pDriveSystem;

	m_oData.AllocateFromSize( a_pDriveSystem->GetDataSize() );

	return &m_oData;
}

// $Barnyard: FUNCTION 0054dde0
ADrive* ADriveComponent::GetCurrentDrive()
{
	return m_pDriveSystem->SelectDrive( GetOwner(), &m_oData );
}
