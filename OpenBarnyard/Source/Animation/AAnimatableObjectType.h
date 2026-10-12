#pragma once
#include "AModel.h"
#include "ANamedAnimationSet.h"
#include "AAnimControllerType.h"

#include <Toshi/T2DList.h>
#include <Toshi/TPString8.h>
#include <Toshi/T2Vector.h>

#include <Plugins/PPropertyParser/PBProperties.h>

class AAnimatableObject;
class AModelInstance;

class AAnimatableObjectType
    : public Toshi::T2DList<AAnimatableObjectType>::Node
{
public:
	constexpr static TSIZE MAX_ANIMATION_SETS = 6;

	// Built from the "Skins" block: Tex = [ material, skin textures..., skin lights... ]
	struct Skins
	{
		Skins()
		    : iNumSkins( 0 ), szMaterialName( TNULL ), pszTextures( TNULL ), pszSkinLights( TNULL )
		{}

		~Skins();

		TINT    iNumSkins;      // 0x0
		TCHAR*  szMaterialName; // 0x4
		TCHAR** pszTextures;    // 0x8
		TCHAR** pszSkinLights;  // 0xc
	};

public:
	// constructors/destructor
	AAnimatableObjectType();
	virtual ~AAnimatableObjectType();

	virtual TBOOL CreateFromProperties( const PBProperties* a_pProperties, const Toshi::TPString8& a_rcName );
	virtual TBOOL Create( const Toshi::TPString8& a_rcName );
	virtual void  OnAnimatableObjectCreated( AAnimatableObject* a_pAnimatableObject );

	void Destroy();

	ANamedAnimationSetRef FindAnimationSet( const Toshi::TPString8& a_rcName ) const;

	// Returns the animation set called a_rcName or the default (first) one if there's no such set
	ANamedAnimationSetRef FindAnimationSetOrDefault( const Toshi::TPString8& a_rcName ) const;

	ANamedAnimationSetRef GetAnimationSet( TINT a_iIndex ) const;
	ANamedAnimationSetRef GetDefaultAnimationSet() const;
	TINT                  GetNumAnimationSets() const;

	AModelInstance* Instantiate();

	TINT         GetNumSkins() const;
	const TCHAR* GetMaterialName() const;
	const TCHAR* GetSkinTexture( TINT a_iSkin ) const;
	const TCHAR* GetSkinLighting( TINT a_iSkin ) const;

	const Toshi::TPString8& GetName() const { return m_strName; }
	AModel*                 GetModel() const { return m_pAModel; }

	Toshi::T2DList<AAnimControllerType>& GetAnimControllerTypes() { return m_llAnimControllerTypes; }

private:
	TBOOL LoadAnimationSet( const PBProperties* a_pProperties );
	TBOOL SetAnimController( const PBProperties* a_pProperties );
	TBOOL SetSkins( const PBProperties* a_pProperties );

private:
	Toshi::TPString8                                           m_strName;
	Toshi::T2Vector<ANamedAnimationSetRef, MAX_ANIMATION_SETS> m_vecAnimationSets;
	Toshi::T2DList<AAnimControllerType>                        m_llAnimControllerTypes;
	AModel*                                                    m_pAModel; // 0x34
	Skins*                                                     m_pSkins;  // 0x38
};
