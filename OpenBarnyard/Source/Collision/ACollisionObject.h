#pragma once
#include "AObjectHash.h"
#include "AHashedObjectAuto.h"

class ACollisionObject
    : public AHashedObjectAuto
    , public Toshi::TObject
{
public:
	TDECLARE_CLASS( ACollisionObject, Toshi::TObject );

public:
	ACollisionObject();
	~ACollisionObject();

	//-----------------------------------------------------------------------------
	// AHashedObjectAuto
	//-----------------------------------------------------------------------------
	virtual TFLOAT                 GetRadius() OVERRIDE;
	virtual const Toshi::TVector3& GetPosition() OVERRIDE;

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------
	virtual void                   VTable0x1C() = 0; // the signature is unknown
	virtual void                   VTable0x20() = 0; // the signature is unknown
	virtual void                   VTable0x24() = 0; // the signature is unknown
	virtual void                   VTable0x28() = 0; // the signature is unknown
	virtual void                   VTable0x2C() = 0; // the signature is unknown
	virtual TBOOL                  VTable0x30( void*, void*, void*, void*, void*, void* );
	virtual void                   VTable0x34();
	virtual void                   VTable0x38()        = 0; // the signature is unknown
	virtual const Toshi::TVector3& GetObjectPosition() = 0; // VTable: 0x3C
	virtual TFLOAT                 GetObjectRadius()   = 0; // VTable: 0x40
	virtual void                   VTable0x44()        = 0; // the signature is unknown
	virtual void                   VTable0x48()        = 0; // the signature is unknown
	virtual TBOOL                  VTable0x4C( void* );
	virtual void*                  VTable0x50();
	virtual void*                  VTable0x54( void*, void*, void*, void*, void*, void*, void* ); // return value is unknown
	virtual void                   VTable0x58() = 0;                                              // the signature is unknown
	virtual void                   VTable0x5C() = 0;                                              // the signature is unknown

	AObjectHash::CellObject* CreateHash();


protected:
	TINT field3_0x18;
	TINT field4_0x1c;
};
