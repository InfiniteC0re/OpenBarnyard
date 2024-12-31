#pragma once
#include "AToshiAnimationInterface.h"
#include "AAnimatableObjectType.h"
#include "AModel.h"

#include <Toshi/T2DList.h>
#include <Toshi/T2SList.h>
#include <Toshi/T2Vector.h>
#include <Toshi/TPString8.h>
#include <Math/TVector2.h>
#include <Math/TQuaternion.h>

class AAnimatableObject
    : public Toshi::TObject
    , public Toshi::T2DList<AAnimatableObject>::Node
    , public Toshi::T2SList<AAnimatableObject>::Node
{
public:
	TDECLARE_CLASS( AAnimatableObject, Toshi::TObject );

	struct S1
	{
		Toshi::TVector4    vecUnk1  = Toshi::TVector4::VEC_ZERO;
		Toshi::TQuaternion quatUnk2 = Toshi::TQuaternion::IDENTITY;
		TINT               Unk3     = 0;
		TINT               Unk4     = 0;
		TINT8              Unk5     = -1;
		TINT8              Unk6     = -1;
	};

	using FLAGS = TUINT8;
	enum FLAGS_ : FLAGS
	{
		FLAGS_NONE,
		FLAGS_CREATED,
	};

	struct S2
	{};

public:
	AAnimatableObject();
	~AAnimatableObject();

	TBOOL Create( AAnimatableObjectType* a_pObjectType, void* a_Unk1, TUINT a_eFlags = 0 );

private:
	S1                                            m_oS1;
	void*                                         m_pUnk1;
	AAnimatableObjectType*                        m_pObjectType;
	AToshiAnimationInterface                      m_oToshiAnimInterface;
	AModelInstanceRef                             m_pModelInstance;
	Toshi::T2DList<Toshi::T2DListNodeWrapper<S2>> m_UnkList; // type is a placeholder
	Toshi::T2Vector<Toshi::TPString8, 2>          m_vecPlayingAnims;
	Toshi::T2SList<AAnimatableObject>             m_SList;
	FLAGS                                         m_eFlags;
};
