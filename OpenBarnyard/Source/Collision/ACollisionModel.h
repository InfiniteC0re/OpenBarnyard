#pragma once
#include <Render/TModel.h>
#include <Opcode.h>

class ACollisionModel
{
public:
	ACollisionModel();
	virtual ~ACollisionModel();

private:
	// ...
	Toshi::TModel* m_pModel;
	// ...
	Opcode::Model         m_OpcodeModel;
	Opcode::MeshInterface m_MeshInterface;
};
