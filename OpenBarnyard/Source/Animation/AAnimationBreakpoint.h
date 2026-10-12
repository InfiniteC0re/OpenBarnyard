#pragma once
#include <Plugins/PPropertyParser/PBProperties.h>
#include <Toshi/TPString8.h>

class AAnimatableObject;

class AAnimationBreakpoint
    : public Toshi::TObject
{
public:
	TDECLARE_CLASS( AAnimationBreakpoint, Toshi::TObject );

public:
	AAnimationBreakpoint();
	~AAnimationBreakpoint();

	// Changes the time unless the breakpoint is untimed (time is -1)
	virtual TBOOL SetTime( TFLOAT a_fTime );                                   // +0x0c
	virtual TBOOL Create( const PBProperties* a_pProperties );                 // +0x10
	virtual void  Execute( AAnimatableObject* a_pObject, TFLOAT a_fTime ) = 0; // +0x14

	TFLOAT GetTime() const { return m_fTime; }
	TBOOL  IsTimed() const { return m_fTime != -1.0f; }

	// Marks the breakpoint as untimed
	void SetUntimed() { m_fTime = -1.0f; }

	// Creates a breakpoint of class AAnimationBreakpoint<a_rcType> (untimed unless a_bTimed) without calling Create.
	// Returns TNULL if there's no such class.
	static AAnimationBreakpoint* CreateBreakpoint( const Toshi::TPString8& a_rcType, TBOOL a_bTimed );

protected:
	TFLOAT m_fTime; // 0x4
};
