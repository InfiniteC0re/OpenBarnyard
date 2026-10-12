#pragma once
#include <Toshi/TObject.h>
#include <Toshi/TArray.h>
#include <Toshi/TNodeList.h>
#include <Math/TVector3.h>

//-----------------------------------------------------------------------------
// Purpose: Contacts found by the APrim vs APrim tests (APrim::Intersect).
// NOTE: the name is made up. It's the secondary base of AGeomIntersectResult
// (vtable 0x00727a80, only the destructor).
//-----------------------------------------------------------------------------
class AGeomContactList
{
public:
	struct Contact
	{
		Toshi::TVector3 vPosition; // 0x00
		Toshi::TVector3 vNormal;   // 0x0C
		TFLOAT          fDepth;    // 0x18
	};

public:
	AGeomContactList();
	virtual ~AGeomContactList();

	// Adds a contact, the normal is negated if m_bFlipNormals is set
	void AddContact( const Toshi::TVector3& a_rcPosition, const Toshi::TVector3& a_rcNormal, TFLOAT a_fDepth );

	TINT           GetNumContacts() const { return m_aContacts.Size(); }
	const Contact& GetContact( TINT a_iIndex ) const { return m_aContacts[ a_iIndex ]; }

public:
	TBOOL                  m_bFlipNormals; // 0x04, not initialised
	Toshi::TArray<Contact> m_aContacts;    // 0x08
};

TSTATICASSERT( sizeof( AGeomContactList::Contact ) == 0x1C );
TSTATICASSERT( sizeof( AGeomContactList ) == 0x18 );

//-----------------------------------------------------------------------------
// Purpose: Result of an APrim vs APrim test. Unused in retail.
// NOTE: TClass version 1.0, size 0x30, vtable 0x00727a94 (AGeomContactList 0x00727a90).
//-----------------------------------------------------------------------------
class AGeomIntersectResult
    : public Toshi::TObject
    , public AGeomContactList
    , public Toshi::TNodeList<AGeomIntersectResult>::TNode
{
public:
	TDECLARE_CLASS( AGeomIntersectResult, Toshi::TObject );

public:
	AGeomIntersectResult();
	~AGeomIntersectResult();

private:
	TINT m_iUnk28; // 0x28, not initialised
	TINT m_iUnk2C; // 0x2C, not initialised
};

TSTATICASSERT( sizeof( AGeomIntersectResult ) == 0x30 );

//-----------------------------------------------------------------------------
// Purpose: Result of an APrim ray test. Unused in retail.
// NOTE: TClass version 1.0, size 0x18, vtable 0x00727a84.
//-----------------------------------------------------------------------------
class AGeomRayIntersectResult
    : public Toshi::TObject
    , public Toshi::TNodeList<AGeomRayIntersectResult>::TNode
{
public:
	TDECLARE_CLASS( AGeomRayIntersectResult, Toshi::TObject );

public:
	AGeomRayIntersectResult();

private:
	TINT   m_iUnk10;    // 0x10
	TFLOAT m_fDistance; // 0x14, TMath::MAXFLOAT
};

TSTATICASSERT( sizeof( AGeomRayIntersectResult ) == 0x18 );
