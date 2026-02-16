#pragma once
#include "ACollisionObject.h"

#include <Render/TTransformObject.h>

class ACollisionObjectModel
	: public ACollisionObject
{
public:
	ACollisionObjectModel();
	~ACollisionObjectModel();

	// STUBS:
	virtual void                   VTable0x1C( void* ) {}
	virtual void                   VTable0x20( void*, void* ) {}
	virtual void                   VTable0x24( void*, void*, void*, void*, void*, void*, void* ) {}
	virtual void                   VTable0x28( void*, void*, void*, void*, void*, void* ) {}
	virtual void                   VTable0x2C( void*, void*, void*, void* ) {}
	virtual const Toshi::TSphere&  GetObjectSphere() { return Toshi::TSphere(); }
	virtual const Toshi::TVector4& GetObjectPosition() { return Toshi::TVector4::VEC_NEGX; }
	virtual TFLOAT                 GetObjectRadius() { return 0.0f; }
	virtual void                   VTable0x44( Toshi::TVector4& a_rOutVector ) {}
	virtual TFLOAT                 GetHeightScale() { return 0.0f; }
	virtual void                   TranslateBy( const Toshi::TVector4& a_rDirection, TFLOAT a_fLength ) {}
	virtual void                   VTable0x5C( void* ) {}

private:
	// ...
	Toshi::TVector4         m_vUnk1;
	TINT                    m_iUnk2;
	Toshi::TTransformObject m_oTransform;
};
