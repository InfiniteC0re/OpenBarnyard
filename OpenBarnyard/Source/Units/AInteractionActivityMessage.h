#pragma once
#include <Toshi/TObject.h>
#include <Toshi/TNodeList.h>
#include <Toshi/TPString8.h>

class AInteractionSequence;
class AInteractionActivityMessageGroups;

//-----------------------------------------------------------------------------
// Data the activities of a sequence share by name, see
// AInteractionSequence::GetActivityMessage. Messages are taken from the
// AInteractionActivityMessageGroup of their class (see AInteractionManager)
// and returned to it once the sequence is reset.
//-----------------------------------------------------------------------------
class AInteractionActivityMessage
    : public Toshi::TObject
    , public Toshi::TNodeList<AInteractionActivityMessage>::TNode
{
public:
	TDECLARE_CLASS( AInteractionActivityMessage, Toshi::TObject );

public:
	AInteractionActivityMessage();
	~AInteractionActivityMessage();

	virtual void  Reset();                                                         // +0x0c, called when the message is returned to its group
	virtual void  Update( TFLOAT a_fDeltaTime ) {}                                 // +0x10, DOES_NOTHING
	virtual TBOOL Unk_0x14( void* a_pUnk ) { return TFALSE; }                      // +0x14, RETURNS_FALSE
	virtual void* Unk_0x18() { return TNULL; }                                     // +0x18, RETURNS_ZERO
	virtual TBOOL Unk_0x1c( void* a_pUnk ) { return TFALSE; }                      // +0x1c, RETURNS_FALSE
	virtual void  CreateGroup( AInteractionActivityMessageGroups* a_pGroups ) = 0; // +0x20, adds the group of the class if there is none

	void SetName( const Toshi::TPString8& a_rcName );

	// NOTE: retail calls a folded setter (0054f230)
	void SetSequence( AInteractionSequence* a_pSequence ) { m_pSequence = a_pSequence; }

	const Toshi::TPString8& GetName() const { return m_strName; }
	AInteractionSequence*   GetSequence() const { return m_pSequence; }

protected:
	Toshi::TPString8      m_strName;   // 0x10
	AInteractionSequence* m_pSequence; // 0x14
};

TSTATICASSERT( sizeof( AInteractionActivityMessage ) == 0x18 );
