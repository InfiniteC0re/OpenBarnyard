#pragma once
#include "AModel.h"

#include <Toshi/TPString8.h>
#include <Toshi/T2Map.h>

class AModelRepos :
    public Toshi::TObject,
    public Toshi::TSingleton<AModelRepos>
{
public:
	TDECLARE_CLASS( AModelRepos, Toshi::TObject );

	using ModelsMap = Toshi::T2Map<Toshi::TPString8, AModel*, Toshi::TPString8::Comparator>;

	static constexpr TSIZE MAX_UNUSED_MODELS_NUM = 9;

public:
	AModelRepos();
	~AModelRepos();

	AModel* GetModel( const Toshi::TPString8& a_rcName );
	AModel* GetUnusedModel( const Toshi::TPString8& a_rName );

	AModelInstance* InstantiateModel( AModel* a_pModel );
	AModelInstance* InstantiateNewModel( const Toshi::TPString8& a_rName, Toshi::TTRB* a_pTRB );

	void LoadModel( const Toshi::TPString8& a_rcName, Toshi::TTRB* a_pTRB );
	void UnloadModel( const Toshi::TPString8& a_rcName, TBOOL a_bDestroy );

	void Update( TFLOAT a_fDeltaTime );

private:
	AModel* AllocateAModel( const Toshi::TPString8& a_rcName, Toshi::TTRB* a_pTRB );

	void MarkAllModelsUnused();
	void MarkModelUnused( AModel* a_pModel );

	void UnloadAllModels();
	void UnloadAllUnusedModels();

private:
	Toshi::T2DList<AModel> m_UnusedModels;
	ModelsMap              m_UsedModels;
	ModelsMap              m_AllModels;
};
