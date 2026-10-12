#pragma once
#include <Toshi/TObject.h>
#include <Math/TVector4.h>

//-----------------------------------------------------------------------------
// A position units can (re)spawn at. APrecisionSquirt keeps one for each
// "respawn" goal of its network.
// TClass 00709640 (parent TObject, size 0x38), vtable 00722e7c.
//-----------------------------------------------------------------------------
class ASpawnPoint
    : public Toshi::TObject
{
public:
	TDECLARE_CLASS( ASpawnPoint, Toshi::TObject );

public:
	ASpawnPoint();
	~ASpawnPoint();

	ASpawnPoint& operator=( const ASpawnPoint& a_rcOther );

	//-----------------------------------------------------------------------------
	// Own virtuals
	//-----------------------------------------------------------------------------

	// TTRUE if a_rcPosition is closer than a_fDistance
	virtual TBOOL                  IsWithinDistance( const Toshi::TVector4& a_rcPosition, TFLOAT a_fDistance ); // +0x0c
	virtual void                   Unk_0x10() {}                                                                // +0x10, DOES_NOTHING 005acd80
	virtual const Toshi::TVector4* SetPosition( const Toshi::TVector4& a_rcPosition );                          // +0x14

	// NOTE: folded with other setters of a field at +0x2c (0066b160)
	void SetArea( TINT a_iArea ) { m_iArea = a_iArea; }

	const Toshi::TVector4& GetPosition() const { return m_vPosition; }
	TINT                   GetArea() const { return m_iArea; }

private:
	Toshi::TVector4 m_vPosition;    // 0x04
	Toshi::TVector4 m_vDirection;   // 0x14
	TFLOAT          m_fRadius;      // 0x24
	TUINT32         m_uiColour;     // 0x28
	TINT            m_iArea;        // 0x2c, area of the goal
	TBOOL           m_bPositionSet; // 0x30
	TINT            m_iUnk34;       // 0x34
};

TSTATICASSERT( sizeof( ASpawnPoint ) == 0x38 );
