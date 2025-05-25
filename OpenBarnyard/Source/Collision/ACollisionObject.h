#pragma once
#include "AObjectHash.h"
#include "AHashedObjectAuto.h"

class ACollisionObject
    : public AHashedObjectAuto
    , public Toshi::TObject
{
public:
	TDECLARE_CLASS( ACollisionObject, Toshi::TObject );
	
	enum Type : TINT
	{
		Type_None,
		Type_Unk1,
		Type_Simple,
		Type_NUMOF,
	};

public:
	ACollisionObject();
	~ACollisionObject();

	AObjectHash::CellObject* CreateHash();

	//-----------------------------------------------------------------------------
	// AHashedObjectAuto
	//-----------------------------------------------------------------------------
	virtual TFLOAT                 GetRadius() OVERRIDE;
	virtual const Toshi::TVector3& GetPosition() OVERRIDE;

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------
	virtual void                   VTable0x1C( void* )                                           = 0;
	virtual void                   VTable0x20( void*, void* )                                    = 0;
	virtual void                   VTable0x24( void*, void*, void*, void*, void*, void*, void* ) = 0;
	virtual void                   VTable0x28( void*, void*, void*, void*, void*, void* )        = 0;
	virtual void                   VTable0x2C( void*, void*, void*, void* )                      = 0;
	virtual TBOOL                  VTable0x30( void*, void*, void*, void*, void*, void* );
	virtual void                   VTable0x34();
	virtual const Toshi::TSphere&  GetObjectSphere()                           = 0; // VTable: 0x38
	virtual const Toshi::TVector3& GetObjectPosition()                         = 0; // VTable: 0x3C
	virtual TFLOAT                 GetObjectRadius()                           = 0; // VTable: 0x40
	virtual void                   VTable0x44( Toshi::TVector4& a_rOutVector ) = 0; // VTable: 0x44
	virtual TFLOAT                 GetHeightScale()                            = 0; // VTable: 0x48
	virtual TBOOL                  Create( TFLOAT a_fRadius );                      // VTable: 0x4Ñ
	virtual void*                  VTable0x50();
	virtual void*                  VTable0x54( void*, void*, void*, void*, void*, void*, void* );            // return value is unknown
	virtual void                   TranslateBy( const Toshi::TVector4& a_rDirection, TFLOAT a_fLength ) = 0; // VTable: 0x58
	virtual void                   VTable0x5C( void* )                                                  = 0; // return value is unknown

protected:
	TINT field3_0x18;
	Type m_eType;
};
