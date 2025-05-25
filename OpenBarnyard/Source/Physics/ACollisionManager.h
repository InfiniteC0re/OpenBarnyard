#pragma once
#include "Collision/AObjectHashMain.h"

#include <Toshi/T2DList.h>

class ACollisionManager
    : public Toshi::TTask
    , public Toshi::TSingleton<ACollisionManager>
{
public:
	TDECLARE_CLASS( ACollisionManager, Toshi::TTask );

	struct Node
	{
		TINT            field0_0x0   = 0;
		TINT            field1_0x4   = 0;
		Toshi::TVector4 field2_0x8   = Toshi::TVector4::VEC_ZERO;
		Toshi::TVector4 field3_0x18  = Toshi::TVector4::VEC_ZERO;
		Toshi::TVector4 field4_0x28  = Toshi::TVector4::VEC_ZERO;
		Toshi::TVector4 field5_0x38  = Toshi::TVector4::VEC_ZERO;
		TINT            field6_0x48  = 0;
		TINT            field7_0x4c  = 0;
		TINT            field8_0x50  = -1;
		Toshi::TVector4 field9_0x54  = Toshi::TVector4::VEC_ZERO;
		TINT            field10_0x64 = 0;
	};

	enum HashType : TUINT
	{
		HashType_UNK1,
		HashType_UNK2,
		HashType_UNK3,
		HashType_UNK4
	};

public:
	ACollisionManager();
	~ACollisionManager();

	//-----------------------------------------------------------------------------
	// Toshi::TTask
	//-----------------------------------------------------------------------------
	virtual TBOOL Reset() OVERRIDE;
	virtual TBOOL OnCreate() OVERRIDE;
	virtual TBOOL OnUpdate( TFLOAT a_fDeltaTime ) OVERRIDE;
	virtual void  OnDestroy() OVERRIDE;
	virtual void  OnActivate() OVERRIDE;
	virtual void  OnDeactivate() OVERRIDE;

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------
	AObjectHashMain* CreateObjectHashMain( HashType a_eHashType );

private:
	void CreateStatic();
	void DestroyStatic();

private:
	TINT field2_0x28;
	// ...
	TFLOAT field403_0x1bc;
	TFLOAT field404_0x1c0;
	TBOOL  field405_0x1c4;
};
