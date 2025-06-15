#pragma once

namespace APhysics
{

extern Toshi::TVector4 g_vecGravityDirection;

class ASweptSphere
{

};

class ASphere
{

};

class ARay
{

};

class ACollisionInterfaceCommon
{

};

template <class T>
class ACollisionInterface
    : public ACollisionInterfaceCommon
{
public:
	virtual TBOOL Unknown1( T* );
	virtual TBOOL Unknown2( T*, void* );
	virtual TBOOL Unknown3( T*, void*, void*, void* ) = 0;
};

} // namespace APhysics
