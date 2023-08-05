#pragma once

#include "Toshi/Core/TObject.h"
#include "Toshi2/T2DList.h"
#include "Toshi/Math/Math.h"

class ABaseEntity : 
	public Toshi::TGenericClassDerived<ABaseEntity, Toshi::TObject, "ABaseEntity", TMAKEVERSION(1, 0), TTRUE>,
	public Toshi::T2DList<ABaseEntity>::Node
{
	static constexpr int ENTITY_HASH_SIZE = 0x400;

	enum FLAG
	{
		FLAG_Created   = BITFIELD(0),
		FLAG_Visible   = BITFIELD(2),
		FLAG_DontTick  = BITFIELD(8),
		FLAG_LowLOD    = BITFIELD(9),
		FLAG_MediumLOD = BITFIELD(10),
		FLAG_HighLOD   = BITFIELD(11),
		FLAG_LODMask   = FLAG_LowLOD | FLAG_MediumLOD | FLAG_HighLOD,
		FLAG_UPDATEMATRIX  = BITFIELD(15)
	};
public:
	virtual ~ABaseEntity() = default;
	virtual void Tick(float a_fTicks) { m_fTicks += a_fTicks; }
	virtual void OnLODChanging() {};
	virtual void Create()
	{
		TASSERT(0 == (m_Flags & FLAG_Created));
		m_Flags |= FLAG_Created;
	}
	virtual void PostCreate() {};
	virtual void Destroy() {};
	
	virtual float GetAnimationSpeed(int) const { return 1.0f; }
	virtual void SetVisible(bool bEnable)
	{
		bEnable ? m_Flags |= FLAG_Visible : m_Flags &= ~FLAG_Visible;
	}
	virtual void SetPosition(const Toshi::TVector4& a_pos)
	{
		m_mMatrix.m_f41 = a_pos.x;
		m_mMatrix.m_f42 = a_pos.y;
		m_mMatrix.m_f43 = a_pos.z;
		m_mMatrix.m_f44 = a_pos.w;
		m_Flags |= FLAG_UPDATEMATRIX;
	}
	virtual void SetFacing(const Toshi::TVector4& a_facing)
	{
		m_mMatrix.LookAtDirection(a_facing, Toshi::TVector4::VEC_NEGY);
		m_Flags |= FLAG_UPDATEMATRIX;
	}
	virtual void SetMatrix(const Toshi::TMatrix44& a_matrix)
	{
		m_mMatrix = a_matrix;
		m_Flags |= FLAG_UPDATEMATRIX;
	}
	
	virtual void GetReactionForce(Toshi::TVector4& a_reactionForce)
	{
		a_reactionForce = Toshi::TVector4::VEC_ZERO;
	}
	
	virtual float GetStepUpHeight() const { return 0.3f; }
	virtual void SetBBox(Toshi::TVector4 a_bbox)
	{
		m_vBoundingBox = a_bbox;
		m_fBoundingSphereRadius = a_bbox.Magnitude();
	}
	virtual float GetInteractionRadius() const { return 2.5f; }
	virtual void OnLeaveState(int a_State) {};
	virtual void OnEnterState(int a_State) {};

	void SetState(int a_State);


	static int sm_EntityHash[ENTITY_HASH_SIZE];

	short m_State;                                         // 0xC
	short m_iID;                                           // 0xE
	Toshi::TMatrix44 m_mMatrix;                            // 0x10
	Toshi::TVector4 m_vBoundingBox;                        // 0x50
	float m_fBoundingSphereRadius;                         // 0x68
	float m_fTicks;                                        // 0x6C
	uint8_t m_Flags;                                       // 0x70
	Toshi::T2DList<ABaseEntity>::Node* m_pEntityCellPair;  // 0x7C
};

