#include "pch.h"
#include "AInventoryItem.h"
#include "ACarryableUnitHelper.h"
#include "AAnimatedUnit.h"
#include "AUnitManager.h"
#include "AUnitFactory.h"
#include "AUnitType.h"
#include "Animation/AModel.h"
#include "Animation/AAnimatableObject.h"

#include <Toshi/TString16.h>
#include <Toshi/TStringManager.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 00626810, CreateObject 00626c40, CreateObjectInPlace 00626cc0
TDEFINE_CLASS( AInventoryItem );

TPSTRING8_DECLARE( apple );

// $Barnyard: FUNCTION 006268e0
AInventoryItem::AInventoryItem()
    : m_iCount( 0 )
{
}

// $Barnyard: FUNCTION 00626910
// $Barnyard: FUNCTION 00626d90
AInventoryItem::~AInventoryItem()
{
	m_oUnit = TNULL;
}

// $Barnyard: FUNCTION 00626db0
TBOOL AInventoryItem::Save( AHackedQuickSave::Buffer* a_pBuffer )
{
	a_pBuffer->WriteInt( m_iCount );

	AUnit* pUnit = m_oUnit->GetUnit();

	// The holder is found by the name of its type when loading
	TPString8 strHolderType;

	strHolderType = pUnit->GetCarryableHelper()->GetHolder()->GetType()->GetName();

	a_pBuffer->WriteString( strHolderType );

	TPString8 strType = pUnit->GetType()->GetName();
	a_pBuffer->WriteString( strType );

	return TTRUE;
}

// $Barnyard: FUNCTION 00626e90
TBOOL AInventoryItem::Load( AHackedQuickSave::Buffer* a_pBuffer )
{
	a_pBuffer->ReadInt( m_iCount );

	TPString8 strHolderType;
	a_pBuffer->ReadString( strHolderType );
	AUnit* pHolder = AUnitManager::GetSingleton()->FindUnitByTypeName( strHolderType );

	m_oUnit = TNULL;

	TPString8 strType;
	a_pBuffer->ReadString( strType );

	if ( m_iCount > MAX_COUNT )
		m_iCount = MAX_COUNT;

	TVector4 vecDirection( 0.0f, 0.0f, 1.0f, 1.0f );
	TVector4 vecPosition( 0.0f, 0.0f, 0.0f, 1.0f );

	AUnitManager* pUnitManager = AUnitManager::GetSingleton();
	AUnit*        pUnit        = pUnitManager->SpawnUnit( pUnitManager->GetUnitFactory()->Find( strType ), vecPosition, vecDirection );

	AModelInstance* pModelInstance = TSTATICCAST( AVisibleUnit, pUnit )->GetModelInstance();
	pModelInstance->DisableDrawWithDefaultMaterials();
	pModelInstance->SetUnlit();

	m_oUnit = pUnit->GetSelfProxy();
	GetUnit()->GetCarryableHelper()->SetHolder( pHolder );

	DisableUnit( TTRUE );
	return TTRUE;
}

// $Barnyard: FUNCTION 00627090
TBOOL AInventoryItem::Set( AUnit* a_pUnit, AUnit* a_pHolder, TINT a_iCount )
{
	m_oUnit = a_pUnit->GetSelfProxy();

	m_iCount = a_iCount;
	if ( a_iCount > MAX_COUNT )
		m_iCount = MAX_COUNT;

	a_pUnit->GetCarryableHelper()->SetHolder( a_pHolder );

	AModelInstance* pModelInstance = TSTATICCAST( AVisibleUnit, a_pUnit )->GetModelInstance();
	pModelInstance->DisableDrawWithDefaultMaterials();
	pModelInstance->SetUnlit();

	DisableUnit( TTRUE );
	return TTRUE;
}

// $Barnyard: FUNCTION 00627180
TBOOL AInventoryItem::Set( const TPString8& a_rcTypeName, AUnit* a_pHolder, TINT a_iCount )
{
	TVector4 vecDirection( 0.0f, 0.0f, 1.0f, 1.0f );
	TVector4 vecPosition( 0.0f, 0.0f, 0.0f, 1.0f );

	AUnitManager* pUnitManager = AUnitManager::GetSingleton();
	AUnit*        pUnit        = TNULL;

	if ( !pUnitManager->GetUnitFactory()->Find( a_rcTypeName ) || ( pUnit = pUnitManager->SpawnUnit( a_rcTypeName, vecPosition, vecDirection ) ) == TNULL )
		pUnit = pUnitManager->SpawnUnit( TPS8( apple ), vecPosition, vecDirection );

	AModelInstance* pModelInstance = TSTATICCAST( AVisibleUnit, pUnit )->GetModelInstance();
	pModelInstance->DisableDrawWithDefaultMaterials();
	pModelInstance->SetUnlit();

	pUnit->GetCarryableHelper()->SetHolder( a_pHolder );
	m_oUnit = pUnit->GetSelfProxy();

	m_iCount = a_iCount;
	if ( a_iCount > MAX_COUNT )
		m_iCount = MAX_COUNT;

	DisableUnit( TTRUE );
	return TTRUE;
}

// $Barnyard: FUNCTION 00626b40
void AInventoryItem::ReplaceUnit( AUnit* a_pUnit, AUnit* a_pHolder )
{
	m_oUnit = a_pUnit->GetSelfProxy();
	a_pUnit->GetCarryableHelper()->SetHolder( a_pHolder );

	// NOTE: inlined
	DisableUnit( TFALSE );
}

// $Barnyard: FUNCTION 006268b0
void AInventoryItem::Reset()
{
	m_oUnit  = TNULL;
	m_iCount = 0;
}

// $Barnyard: FUNCTION 006269e0
void AInventoryItem::DisableUnit( TBOOL a_bHide )
{
	AUnit* pUnit = GetUnit();

	pUnit->SetAIEnabled( TFALSE );
	pUnit->SetMovement( TFALSE );
	pUnit->SetExternalUpdate( TFALSE );
	pUnit->SetHashed( TFALSE );
	pUnit->SetNoQuery( TTRUE );
	AUnitManager::GetSingleton()->SetUnitPoolUsed( pUnit, TFALSE );

	if ( a_bHide && pUnit->GetClass()->IsA( &TGetClass( AVisibleUnit ) ) )
	{
		TSTATICCAST( AVisibleUnit, pUnit )->SetVisible( TFALSE );

		if ( pUnit->GetClass()->IsA( &TGetClass( AAnimatedUnit ) ) )
			TSTATICCAST( AAnimatedUnit, pUnit )->GetAnimatableObject()->SetAnimControllersDisabled( TTRUE );
	}
}

// $Barnyard: FUNCTION 00626850
void AInventoryItem::AddCount( TINT a_iCount )
{
	m_iCount += a_iCount;

	if ( m_iCount > MAX_COUNT )
		m_iCount = MAX_COUNT;
}

// $Barnyard: FUNCTION 00626870
void AInventoryItem::RemoveCount( TINT a_iCount )
{
	m_iCount -= a_iCount;

	if ( m_iCount > MAX_COUNT )
		m_iCount = MAX_COUNT;
}

// $Barnyard: FUNCTION 00626890
void AInventoryItem::SetCount( TINT a_iCount )
{
	m_iCount = a_iCount;

	if ( a_iCount > MAX_COUNT )
		m_iCount = MAX_COUNT;
}

// $Barnyard: FUNCTION 00626a80
const TWCHAR* AInventoryItem::GetScreenName()
{
	if ( m_oUnit->GetUnit()->GetScreenName() )
		return m_oUnit->GetUnit()->GetScreenName();

	TString16 strTypeName;
	strTypeName = m_oUnit->GetUnit()->GetType()->GetName().GetString();
	TStringManager::String16Format( m_wszScreenName, MAX_SCREEN_NAME_LEN, L"%s", strTypeName.GetString() );

	return m_wszScreenName;
}

// $Barnyard: FUNCTION 00627300
TPString8 AInventoryItem::GetTypeName() const
{
	if ( m_oUnit.Get() && m_oUnit->GetUnit() )
		return m_oUnit->GetUnit()->GetType()->GetName();

	return TPString8();
}

// $Barnyard: FUNCTION 006269c0
AUnit* AInventoryItem::GetUnit() const
{
	if ( !m_oUnit.Get() )
		return TNULL;

	return m_oUnit->GetUnit();
}

// $Barnyard: FUNCTION 006269d0
AUnitType* AInventoryItem::GetUnitType() const
{
	return m_oUnit->GetUnit()->GetType();
}
