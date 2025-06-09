#pragma once
#include <Render/TModel.h>
#include <Opcode.h>

class ACollisionModel
{
public:
	ACollisionModel();
	virtual ~ACollisionModel();

	TBOOL Create( TINT a_iMeshIndex, Toshi::TModel* a_pModel );

private:
	Toshi::TVector4       m_vecTreeCenter;
	TFLOAT                m_fAverageExtent;
	Toshi::TModel*        m_pModel;
	TINT                  m_iMeshIndex;
	Opcode::Model         m_OpcodeModel;
	Opcode::MeshInterface m_MeshInterface;
};
