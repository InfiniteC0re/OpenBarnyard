#include "pch.h"
#include "AGopherShopItem.h"
#include "Animation/AAnimatableObject.h"
#include "Animation/AAnimatableObjectManager.h"
#include "Animation/AModel.h"
#include "Quests/AQuestManager.h"
#include "Locale/ALocaleManager.h"

#include <Math/TMatrix44.h>
#include <Toshi/TStringManager.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $Barnyard: FUNCTION 0049b5e0
AGopherShopItem::AGopherShopItem()
    : m_fFade( 0.0f )
    , m_fOffset( 0.0f )
    , m_vecRotation( -1.0f, 0.0f, 0.0f, 1.0f )
    , m_iCategory( 0 )
    , m_fScale( 1.0f )
    , m_pObject( TNULL )
    , m_iCount( 0 )
    , m_iPrice( -1 )
    , m_iMaxCount( 1 )
    , m_iNumRequired( 0 )
{
}

// $Barnyard: FUNCTION 0049b150
AGopherShopItem::~AGopherShopItem()
{
	if ( m_pObject )
	{
		AAnimatableObjectManager::GetSingleton()->DestroyAnimatableObject( m_pObject );
		m_pObject = TNULL;
	}
}

// $Barnyard: FUNCTION 00493d30
AGopherShopItem* AGopherShopItem::Create( const AGopherShopPrice& a_rcPrice )
{
	// NOTE: retail formats every name with "%s" into a TString8 first
	TPString8 strName( a_rcPrice.szModel );

	AGopherShopItem* pItem = new AGopherShopItem();

	// NOTE: retail writes the default into the price list itself
	if ( a_rcPrice.iTextId == -1 )
		const_cast<AGopherShopPrice&>( a_rcPrice ).iTextId = 101;

	pItem->SetName( strName, a_rcPrice.iTextId );
	pItem->m_iPrice    = a_rcPrice.iPrice;
	pItem->m_iMaxCount = a_rcPrice.iMaxCount;
	pItem->m_fScale    = a_rcPrice.fScale;
	pItem->m_iCategory = a_rcPrice.iCategory;

	pItem->m_vecRotation.x = a_rcPrice.fRotX * 0.017453292f;
	pItem->m_vecRotation.y = a_rcPrice.fRotY * 0.017453292f;
	pItem->m_vecRotation.z = a_rcPrice.fRotZ * 0.017453292f;
	pItem->m_vecRotation.w = 1.0f;

	if ( a_rcPrice.szInventoryType )
	{
		strName = TPString8( a_rcPrice.szInventoryType );

		// The items of the inventory use the inventory type as their model
		if ( pItem->m_iCategory == 3 )
			pItem->SetModel( strName );
		else
			pItem->SetInventoryType( strName );
	}

	if ( a_rcPrice.szName2 )
	{
		strName = TPString8( a_rcPrice.szName2 );
		pItem->SetName2( strName );
	}

	if ( a_rcPrice.szQuest )
	{
		strName = TPString8( a_rcPrice.szQuest );
		pItem->SetQuest( strName );
	}

	if ( a_rcPrice.szName3 )
	{
		strName           = TPString8( a_rcPrice.szName3 );
		pItem->m_strName3 = strName;
	}

	if ( a_rcPrice.szRequired )
	{
		// Comma separated list of models
		TCHAR szRequired[ 128 ];
		TStringManager::String8Copy( szRequired, a_rcPrice.szRequired, -1 );

		TINT iStart  = 0;
		TINT iLength = TStringManager::String8Length( szRequired );

		for ( TINT i = 0; i < iLength; i++ )
		{
			if ( szRequired[ i ] == ',' )
			{
				if ( i != iLength - 1 )
					szRequired[ i ] = '\0';
			}
			else if ( i != iLength - 1 )
			{
				continue;
			}

			strName                                       = TPString8( &szRequired[ iStart ] );
			pItem->m_aRequired[ pItem->m_iNumRequired++ ] = strName;
			iStart                                        = i + 1;
		}
	}

	AAnimatableObjectType* pType   = AAnimatableObjectManager::GetSingleton()->FindType( pItem->GetModel() );
	AAnimatableObject*     pObject = AAnimatableObjectManager::GetSingleton()->CreateAnimatableObject( pType, TNULL, 0 );
	pObject->SetSkeletonUpdating( TFALSE, TTRUE );
	pObject->SetVisible( TTRUE );
	pObject->GetModelInstance()->DisableDrawWithDefaultMaterials();
	pObject->GetModelInstance()->SetUnlit();
	pItem->m_pObject = pObject;

	return pItem;
}

// $Barnyard: FUNCTION 0049a050
void AGopherShopItem::UpdateTransform( AGopherShopItem* a_pItem, TBOOL a_bSetPosition, TVector4 a_vecPosition, TVector4 a_vecRotation, TFLOAT a_fScale )
{
	if ( !a_pItem )
		return;

	AModelInstance* pModelInstance = a_pItem->GetModelInstance();

	TMatrix44 matTransform = TMatrix44::IDENTITY;
	matTransform.RotateX( 1.5707964f );
	matTransform.Rotate( a_vecRotation.x, a_vecRotation.y, a_vecRotation.z );
	matTransform.Rotate( a_pItem->m_vecRotation.x, a_pItem->m_vecRotation.y, a_pItem->m_vecRotation.z );

	TFLOAT fScale = a_pItem->m_fScale * a_fScale;
	matTransform.Scale( fScale, fScale, fScale );

	if ( a_bSetPosition )
		matTransform.AsBasisVector4( BASISVECTOR_TRANSLATION ) = a_vecPosition;

	if ( a_pItem->GetId() == 310 || a_pItem->GetId() == 312 || a_pItem->GetId() == 313 )
		matTransform.m_f42 += 0.5f;

	// Centre the bounding sphere of the model
	const TVector4& rcSphere = pModelInstance->GetSceneObject()->GetInstance()->GetModel()->GetLOD( 0 ).BoundingSphere.AsVector4();
	TVector3        vecCentre( -rcSphere.x, -rcSphere.y, -rcSphere.z );

	TVector3 vecTranslation;
	TMatrix44::TransformVector( vecTranslation, matTransform, vecCentre );
	matTransform.AsBasisVector4( BASISVECTOR_TRANSLATION ).Set( vecTranslation.x, vecTranslation.y, vecTranslation.z, matTransform.m_f44 );

	pModelInstance->SetTransform( matTransform );
}

// $Barnyard: FUNCTION 0049b020
TBOOL AGopherShopItem::IsUnlocked()
{
	if ( m_strQuest.IsEmpty() )
		return TTRUE;

	AQuestManager* pQuestManager = AQuestManager::GetSingleton();

	TINT iAct;
	TINT iTier;
	pQuestManager->FindQuestTier( m_strQuest.GetString(), &iAct, &iTier );
	TINT iQuestTier = pQuestManager->GetAbsQuestTier( iAct, iTier );

	return pQuestManager->IsQuestComplete( m_strQuest, 1 ) || pQuestManager->GetCurAbsQuestTier() > iQuestTier;
}

// $Barnyard: FUNCTION 0049b0d0
TINT AGopherShopItem::GetNumRemaining()
{
	TINT iRemaining = m_iMaxCount;

	if ( iRemaining != -1 )
		iRemaining -= m_iCount;

	return iRemaining;
}

// $Barnyard: FUNCTION 0049b0e0
TBOOL AGopherShopItem::IsSoldOut()
{
	return m_iMaxCount != -1 && m_iMaxCount == m_iCount;
}

// $Barnyard: FUNCTION 0049b110
void AGopherShopItem::IncreaseCount( TINT a_iUnused )
{
	if ( m_iMaxCount == -1 || m_iMaxCount - m_iCount != 0 )
		m_iCount++;
}

// $Barnyard: FUNCTION 0049b260
void AGopherShopItem::SetName( TPString8 a_strModel, TINT a_iTextId )
{
	m_iTextId  = a_iTextId;
	m_wszName  = ALocaleManager::Interface()->GetString( a_iTextId );
	m_strModel = a_strModel;
}

// $Barnyard: FUNCTION 0049b350
AModelInstance* AGopherShopItem::GetModelInstance()
{
	return m_pObject->GetModelInstance();
}

// $Barnyard: FUNCTION 0049b690
AGopherShopPurchase AGopherShopItem::GetPurchase() const
{
	AGopherShopPurchase oPurchase;
	oPurchase.iId              = m_iTextId;
	oPurchase.iCount           = m_iCount;
	oPurchase.strModel         = GetModel();
	oPurchase.iCategory        = m_iCategory;
	oPurchase.strInventoryType = GetInventoryType();
	oPurchase.strName2         = GetName2();
	oPurchase.strQuest         = GetQuest();
	oPurchase.iMaxCount        = m_iMaxCount;
	oPurchase.strName3         = GetName3();

	return oPurchase;
}
