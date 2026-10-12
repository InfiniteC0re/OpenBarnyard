#pragma once
#include "AModel.h"

#include <Toshi/TPString8.h>
#include <Toshi/T2Map.h>

class AModelRepos : public Toshi::TObject
    , public Toshi::TSingleton<AModelRepos>
{
public:
	TDECLARE_CLASS( AModelRepos, Toshi::TObject );

	using ModelsMap = Toshi::T2Map<Toshi::TPString8, AModel*, Toshi::TPString8::Comparator>;

	static constexpr TSIZE MAX_UNUSED_MODELS_NUM = 9;

	using RENDERMASK = TUINT;
	enum RENDERMASK_ : RENDERMASK
	{
		RENDERMASK_GLOW   = BITFLAG( 0 ), // instances with AModelInstance::Flags_Glow, rendered after the particles
		RENDERMASK_NORMAL = BITFLAG( 1 ), // all the other instances, rendered before the terrain
	};

public:
	AModelRepos();
	~AModelRepos();

	AModel* GetModel( const Toshi::TPString8& a_rcName );
	AModel* GetUnusedModel( const Toshi::TPString8& a_rName );

	AModelInstance* InstantiateModel( AModel* a_pModel );
	AModelInstance* InstantiateNewModel( const Toshi::TPString8& a_rName, Toshi::TTRB* a_pTRB );
	void            DestroyModelInstance( AModelInstance* a_pInstance );

	void LoadModel( const Toshi::TPString8& a_rcName, Toshi::TTRB* a_pTRB );
	void UnloadModel( const Toshi::TPString8& a_rcName, TBOOL a_bDestroy );

	void Update( TFLOAT a_fDeltaTime );

	// Calls a_fnCallback for every used model and then for every model
	void ForEachModel( void ( *a_fnCallback )( AModel* a_pModel ) );
	void RenderModels( RENDERMASK a_eMask );

	void UnloadAllModels();
	void UnloadUnusedModels();
	void MarkAllModelsUnused();

private:
	AModel* AllocateAModel( const Toshi::TPString8& a_rcName, Toshi::TTRB* a_pTRB );

	void MarkModelUnused( AModel* a_pModel );

protected:
	Toshi::T2DList<AModel> m_UnusedModels;
	ModelsMap              m_UsedModels;
	ModelsMap              m_AllModels;
};

TSINGLETON_DECLARE_ALIAS( AModelRepos, ModelRepos );
