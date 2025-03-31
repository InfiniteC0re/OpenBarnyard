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
	// ...
	TFLOAT                m_fUnk1;
	TFLOAT                m_fUnk2;
	Toshi::TModel*        m_pModel;
	TINT                  m_iMeshIndex;
	Opcode::Model         m_OpcodeModel;
	Opcode::MeshInterface m_MeshInterface;
};
