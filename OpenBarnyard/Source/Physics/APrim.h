#pragma once
#include <Toshi/TObject.h>
#include <Toshi/TArray.h>
#include <Math/TVector3.h>
#include <Math/TVector4.h>
#include <Math/TMatrix44.h>

class AGeomContactList;
class APrimCapsule;

//-----------------------------------------------------------------------------
// Purpose: Base of the simple collision primitives (sphere, box, capsule and a
// composite of other prims) that can be tested against each other and against
// rays. The tests are picked from tables by the types of the prims (filled by
// InitialiseClass). Nothing in retail creates or tests prims.
// NOTE: TClass version 1.0, size 0x8, no create functions, vtable 0x00727ad8.
//-----------------------------------------------------------------------------
class APrim : public Toshi::TObject
{
public:
	TDECLARE_CLASS( APrim, Toshi::TObject );

	enum TYPE : TINT
	{
		TYPE_SPHERE,
		TYPE_BOX,
		TYPE_CAPSULE,
		TYPE_COMPOSITE,
		TYPE_CUSTOM, // APrimCustom
		TYPE_NUMOF
	};

	// Tests two prims, adds the contacts to a_pContacts if it isn't TNULL
	using IntersectFunc = TBOOL ( * )( APrim* a_pPrim1, const Toshi::TMatrix44& a_rcTransform1, APrim* a_pPrim2, const Toshi::TMatrix44& a_rcTransform2, AGeomContactList* a_pContacts );

	// Tests a ray against a prim, adds the hit distances to a_pHits and returns the closest one (-1.0f if there's none)
	using IntersectRayFunc = TFLOAT ( * )( APrim* a_pPrim, const Toshi::TMatrix44& a_rcTransform, const Toshi::TVector3& a_rcOrigin, const Toshi::TVector3& a_rcDirection, Toshi::TArray<TFLOAT>* a_pHits, TFLOAT a_fMaxDistance, TINT a_iUnused );

public:
	APrim( TYPE a_eType );
	~APrim();

	// The sphere around the prim (xyz is the centre, w the radius)
	virtual Toshi::TVector4 GetBoundingSphere() = 0; // +0x0c

	TYPE GetType() const { return m_eType; }

	// Tests two prims, without contacts
	static TBOOL Intersect( APrim* a_pPrim1, const Toshi::TMatrix44& a_rcTransform1, APrim* a_pPrim2, const Toshi::TMatrix44& a_rcTransform2 );

	static TFLOAT IntersectRay( APrim* a_pPrim, const Toshi::TMatrix44& a_rcTransform, const Toshi::TVector3& a_rcOrigin, const Toshi::TVector3& a_rcDirection, Toshi::TArray<TFLOAT>* a_pHits, TFLOAT a_fMaxDistance, TINT a_iUnused );

	// The tests (not members in retail)
	static TBOOL  IntersectSphereSphere( const Toshi::TVector3& a_rcCenter1, TFLOAT a_fRadius1, const Toshi::TVector3& a_rcCenter2, TFLOAT a_fRadius2, AGeomContactList* a_pContacts );
	static TBOOL  IntersectSphereSphere( APrim* a_pPrim1, const Toshi::TMatrix44& a_rcTransform1, APrim* a_pPrim2, const Toshi::TMatrix44& a_rcTransform2, AGeomContactList* a_pContacts );
	static TBOOL  IntersectSphereBox( APrim* a_pPrim1, const Toshi::TMatrix44& a_rcTransform1, APrim* a_pPrim2, const Toshi::TMatrix44& a_rcTransform2, AGeomContactList* a_pContacts );
	static TBOOL  IntersectSphereCapsule( APrim* a_pPrim1, const Toshi::TMatrix44& a_rcTransform1, APrim* a_pPrim2, const Toshi::TMatrix44& a_rcTransform2, AGeomContactList* a_pContacts );
	static TBOOL  IntersectComposite( APrim* a_pPrim1, const Toshi::TMatrix44& a_rcTransform1, APrim* a_pPrim2, const Toshi::TMatrix44& a_rcTransform2, AGeomContactList* a_pContacts );
	static TBOOL  IntersectCustom( APrim* a_pPrim1, const Toshi::TMatrix44& a_rcTransform1, APrim* a_pPrim2, const Toshi::TMatrix44& a_rcTransform2, AGeomContactList* a_pContacts );
	static TFLOAT IntersectRaySphere( APrim* a_pPrim, const Toshi::TMatrix44& a_rcTransform, const Toshi::TVector3& a_rcOrigin, const Toshi::TVector3& a_rcDirection, Toshi::TArray<TFLOAT>* a_pHits, TFLOAT a_fMaxDistance, TINT a_iUnused );
	static TFLOAT IntersectRayBox( APrim* a_pPrim, const Toshi::TMatrix44& a_rcTransform, const Toshi::TVector3& a_rcOrigin, const Toshi::TVector3& a_rcDirection, Toshi::TArray<TFLOAT>* a_pHits, TFLOAT a_fMaxDistance, TINT a_iUnused );
	static TFLOAT IntersectRayCapsule( APrim* a_pPrim, const Toshi::TMatrix44& a_rcTransform, const Toshi::TVector3& a_rcOrigin, const Toshi::TVector3& a_rcDirection, Toshi::TArray<TFLOAT>* a_pHits, TFLOAT a_fMaxDistance, TINT a_iUnused );
	static TFLOAT IntersectRayComposite( APrim* a_pPrim, const Toshi::TMatrix44& a_rcTransform, const Toshi::TVector3& a_rcOrigin, const Toshi::TVector3& a_rcDirection, Toshi::TArray<TFLOAT>* a_pHits, TFLOAT a_fMaxDistance, TINT a_iUnused );
	// Intersects the ray (a_rcRay is the direction scaled by the length) with the capsule.
	// Outputs up to 2 ray parameters (divided by the length of a_rcRay) and returns their number.
	static TINT RayVsCapsule( APrimCapsule* a_pCapsule, const Toshi::TMatrix44& a_rcTransform, const Toshi::TVector3& a_rcOrigin, const Toshi::TVector3& a_rcRay, TFLOAT* a_pOutDistances );

	static TFLOAT IntersectRayCustom( APrim* a_pPrim, const Toshi::TMatrix44& a_rcTransform, const Toshi::TVector3& a_rcOrigin, const Toshi::TVector3& a_rcDirection, Toshi::TArray<TFLOAT>* a_pHits, TFLOAT a_fMaxDistance, TINT a_iUnused );

public:
	static IntersectRayFunc ms_aIntersectRayFuncs[ TYPE_NUMOF ];            // 0x00789778
	static IntersectFunc    ms_aIntersectFuncs[ TYPE_NUMOF ][ TYPE_NUMOF ]; // 0x00789790, [ type of prim 1 ][ type of prim 2 ]

protected:
	TYPE m_eType; // 0x04
};

//-----------------------------------------------------------------------------
// Purpose: Prim of TYPE_CUSTOM, it implements the tests itself.
// NOTE: made up, no retail class has this type. The virtuals are the ones
// called by IntersectCustom/IntersectRayCustom (vtable +0x10 to +0x18).
//-----------------------------------------------------------------------------
class APrimCustom : public APrim
{
public:
	APrimCustom()
	    : APrim( TYPE_CUSTOM )
	{}

	virtual TBOOL  IntersectPrim( const Toshi::TMatrix44& a_rcTransform, APrim* a_pOther, const Toshi::TMatrix44& a_rcOtherTransform, AGeomContactList* a_pContacts )                                                    = 0; // +0x10
	virtual TFLOAT IntersectRay( const Toshi::TMatrix44& a_rcTransform, const Toshi::TVector3& a_rcOrigin, const Toshi::TVector3& a_rcDirection, Toshi::TArray<TFLOAT>* a_pHits, TFLOAT a_fMaxDistance, TINT a_iUnused ) = 0; // +0x14
	virtual void   PrepareIntersect( APrim* a_pOther )                                                                                                                                                                   = 0; // +0x18
};

//-----------------------------------------------------------------------------
// NOTE: TClass version 1.0, size 0xc, vtable 0x00727af4.
//-----------------------------------------------------------------------------
class APrimSphere : public APrim
{
public:
	TDECLARE_CLASS( APrimSphere, APrim );

public:
	APrimSphere();
	APrimSphere( TFLOAT a_fRadius );
	~APrimSphere();

	virtual Toshi::TVector4 GetBoundingSphere() OVERRIDE; // +0x0c

	TFLOAT GetRadius() const { return m_fRadius; }

private:
	TFLOAT m_fRadius; // 0x08

	friend class APrim;
};

//-----------------------------------------------------------------------------
// NOTE: TClass version 1.0, size 0x20, vtable 0x00727b04.
//-----------------------------------------------------------------------------
class APrimBox : public APrim
{
public:
	TDECLARE_CLASS( APrimBox, APrim );

public:
	APrimBox();
	~APrimBox();

	virtual Toshi::TVector4 GetBoundingSphere() OVERRIDE; // +0x0c

private:
	Toshi::TVector3 m_vUnk08;   // 0x08, TVector3::VEC_ZERO, unused
	Toshi::TVector3 m_vExtents; // 0x14, half size

	friend class APrim;
};

//-----------------------------------------------------------------------------
// NOTE: TClass version 1.0, size 0x10, vtable 0x00727b14. The sphere test puts
// the axis from -m_fLength / 2 to m_fLength / 2 along Z, the ray test from 0 to
// the length of the ray (see APrim::RayVsCapsule).
//-----------------------------------------------------------------------------
class APrimCapsule : public APrim
{
public:
	TDECLARE_CLASS( APrimCapsule, APrim );

public:
	APrimCapsule();
	~APrimCapsule();

	virtual Toshi::TVector4 GetBoundingSphere() OVERRIDE; // +0x0c

private:
	TFLOAT m_fRadius; // 0x08
	TFLOAT m_fLength; // 0x0C

	friend class APrim;
};

//-----------------------------------------------------------------------------
// NOTE: TClass version 1.0, size 0x18, vtable 0x00727b24.
//-----------------------------------------------------------------------------
class APrimComposite : public APrim
{
public:
	TDECLARE_CLASS( APrimComposite, APrim );

	struct Element
	{
		APrim*           pPrim;      // 0x00
		Toshi::TMatrix44 oTransform; // 0x04, relative to the composite
	};

public:
	APrimComposite();
	~APrimComposite();

	virtual Toshi::TVector4 GetBoundingSphere() OVERRIDE; // +0x0c

private:
	Toshi::TArray<Element> m_aElements; // 0x08

	friend class APrim;
};

TSTATICASSERT( sizeof( APrim ) == 0x8 );
TSTATICASSERT( sizeof( APrimSphere ) == 0xc );
TSTATICASSERT( sizeof( APrimBox ) == 0x20 );
TSTATICASSERT( sizeof( APrimCapsule ) == 0x10 );
TSTATICASSERT( sizeof( APrimComposite::Element ) == 0x44 );
TSTATICASSERT( sizeof( APrimComposite ) == 0x18 );
