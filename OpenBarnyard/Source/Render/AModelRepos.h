#pragma once
#include "AModel.h"

#include <Toshi/Strings/TPString8.h>
#include <Toshi2/T2Map.h>

TOBJECT(AModelRepos, Toshi::TObject, TTRUE),
	public Toshi::TSingleton<AModelRepos>
{
public:
	AModelRepos();
	~AModelRepos();

	AModel* GetModel(const Toshi::TPString8& a_rName);
	AModel* GetUnusedModel(const Toshi::TPString8& a_rName);

	AModelInstance* InstantiateModel(AModel* a_pModel);
	AModelInstance* InstantiateNewModel(const Toshi::TPString8& a_rName, Toshi::TTRB* a_pTRB);

	void CreateInModels2(const Toshi::TPString8& a_rName, Toshi::TTRB* a_pTRB);

	void Update(TFLOAT a_fDeltaTime);

private:
	AModel* CreateNewModel(const Toshi::TPString8& a_rName, Toshi::TTRB* a_pTRB);

	void UnloadAllModels();

private:
	Toshi::T2DList<AModel> m_UnusedModels;
	Toshi::T2Map<Toshi::TPString8, AModel*, Toshi::TPString8::Comparator> m_UsedModels;
	Toshi::T2Map<Toshi::TPString8, AModel*, Toshi::TPString8::Comparator> m_Models2;
};
