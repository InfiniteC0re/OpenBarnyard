#pragma once
#include "AInteractionActivityMessage.h"

#include <Math/TVector3.h>

//-----------------------------------------------------------------------------
// Activity message that keeps a position and a direction for the other
// activities of the sequence ("manualpoint" of ASetManualPointActivity, read by
// AWalkActivity). Vtable 0x0072cc08.
//-----------------------------------------------------------------------------
class AWorldPositionActivityMessage
    : public AInteractionActivityMessage
{
public:
	TDECLARE_CLASS( AWorldPositionActivityMessage, AInteractionActivityMessage );

public:
	AWorldPositionActivityMessage();
	~AWorldPositionActivityMessage();

	virtual void CreateGroup( AInteractionActivityMessageGroups* a_pGroups ) OVERRIDE; // +0x20

	void SetPosition( const Toshi::TVector3& a_rcPosition ) { m_vPosition = a_rcPosition; }
	void SetDirection( const Toshi::TVector3& a_rcDirection ) { m_vDirection = a_rcDirection; }

	const Toshi::TVector3& GetPosition() const { return m_vPosition; }
	const Toshi::TVector3& GetDirection() const { return m_vDirection; }

private:
	Toshi::TVector3 m_vPosition;  // 0x18
	Toshi::TVector3 m_vDirection; // 0x24
};

TSTATICASSERT( sizeof( AWorldPositionActivityMessage ) == 0x30 );
