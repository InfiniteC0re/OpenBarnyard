#pragma once
#include "AQuestElement.h"
#include "Units/AUnitRef.h"

#include <Toshi/TString16.h>

//-----------------------------------------------------------------------------
// Quest element completed by giving "amount" items of "gifttype" to a unit
// ("receivername", or the first unit of "receivertype").
//-----------------------------------------------------------------------------
class AGiftQuestElement : public AQuestElement
{
public:
	TDECLARE_CLASS( AGiftQuestElement, AQuestElement );

public:
	AGiftQuestElement();
	~AGiftQuestElement();

	//-----------------------------------------------------------------------------
	// AQuestElement
	//-----------------------------------------------------------------------------
	virtual TBOOL    Create( const PBProperties* a_pProperties, TINT* a_pDataOffset ) OVERRIDE;               // +0x0c
	virtual TBOOL    InitData( AInteractionActivityData* a_pData ) OVERRIDE;                                  // +0x10
	virtual void     DeinitData( AInteractionActivityData* a_pData ) OVERRIDE;                                // +0x14
	virtual TBOOL    GetDescription( const TWCHAR** a_ppOutText ) OVERRIDE;                                   // +0x18
	virtual void     Save( AHackedQuickSave::Buffer* a_pBuffer, AInteractionActivityData* a_pData ) OVERRIDE; // +0x1c
	virtual TBOOL    Load( AHackedQuickSave::Buffer* a_pBuffer, AInteractionActivityData* a_pData ) OVERRIDE; // +0x20
	virtual PROGRESS CheckProgress( AInteractionActivityData* a_pData ) OVERRIDE;                             // +0x28

private:
	// The receiver of the instance
	AUnitRef* GetReceiver( AInteractionActivityData* a_pData ) const { return TREINTERPRETCAST( AUnitRef*, TSTATICCAST( TUINT8, a_pData->pData ) + m_iReceiverOffset ); }

private:
	Toshi::TPString8 m_strGiftType;     // 0x5c, "gifttype"
	Toshi::TPString8 m_strReceiverType; // 0x60, "receivertype"
	Toshi::TPString8 m_strReceiverName; // 0x64, "receivername"
	Toshi::TString16 m_strDescription;  // 0x68
	TINT             m_iAmount;         // 0x74, "amount"
	TINT             m_iReceiverOffset; // 0x78, offset of the AUnitRef in the instance data
};

TSTATICASSERT( sizeof( AGiftQuestElement ) == 0x7c );
