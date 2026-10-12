#pragma once
#include "GUI/AGUI2TextBox.h"

#include <Toshi/TSingleton.h>
#include <File/TTRB.h>
#include <Math/TVector4.h>
#include <GUI/T2GUIElement.h>
#include <GUI/T2GUIRectangle.h>

namespace Toshi
{
class TManagedModel;
}

// Milk particles of the milk squirt: a pool of small models moving along a
// ballistic trajectory. Settings and the models come from a TRB file
// ("MilkParticleData" symbol). AMilkProjectile owns the particles of its part
// of the stream and gives them back once it dies.
// Also has a debug overlay listing the particle counts and the lighting.
//
// Singleton at 0x007cd1b0 (created by Load, not by the constructor).
class AMilkManager
    : public Toshi::TSingleton<AMilkManager>
{
public:
	// "MilkParticleData" symbol of the TRB file.
	struct Data
	{
		TFLOAT*       pRenderParams;   // 0x00, read by Render
		TFLOAT        fSpeedVariation; // 0x04, launch speed is scaled by a random value in [1 - x, 1 + x]
		TFLOAT        fSideVariation;  // 0x08, random sideways velocity in [-x, x]
		TFLOAT        fUpVariation;    // 0x0c, random vertical velocity in [-x, x]
		TFLOAT        fSpawnRate;      // 0x10, particles per second of each of the squirt's streams
		TINT16        iNumParticles;   // 0x14
		TINT16        iNumTypes;       // 0x16, one model per type
		const TCHAR** pszModels;       // 0x18
	};

	struct Particle
	{
		// $Barnyard: FUNCTION 00666df0
		Particle()
		    : vPosition( Toshi::TVector4::VEC_ZERO )
		    , vVelocity( Toshi::TVector4::VEC_ZERO )
		    , iType( -1 )
		    , pPrev( TNULL )
		    , pNext( TNULL )
		{}

		Toshi::TVector4 vPosition; // 0x00
		Toshi::TVector4 vVelocity; // 0x10
		TINT            iType;     // 0x20
		Particle*       pPrev;     // 0x24
		Particle*       pNext;     // 0x28
	};

public:
	AMilkManager();
	~AMilkManager();

	// Creates the singleton and loads a_szFileName. Returns TNULL if the singleton already exists or loading has failed.
	static AMilkManager* Load( const TCHAR* a_szFileName );

	void Unload();

	// Moves every used particle.
	void Update( TFLOAT a_fDeltaTime );

	void Render();

	// Takes a free particle (of a random type) and places it. Returns its index or -1 if there are no free particles.
	TINT SpawnParticle( const Toshi::TVector4& a_rcPosition, const Toshi::TVector4& a_rcVelocity );

	// Moves the particle along its trajectory by a_fTime seconds.
	void UpdateParticle( TINT a_iParticle, TFLOAT a_fTime );

	// Gives the particles back to the pool of free particles.
	void ReleaseParticles( TINT a_iNumParticles, const TINT* a_pParticles );

	const Data* GetData() const { return m_pData; }

private:
	TBOOL LoadImpl( const TCHAR* a_szFileName );

private:
	Toshi::TTRB            m_oTRB;                  // 0x000
	Data*                  m_pData;                 // 0x140
	TCHAR                  m_szFileName[ 512 ];     // 0x144
	Toshi::TManagedModel** m_ppModels;              // 0x344, one per type
	Particle*              m_pParticles;            // 0x348
	Particle*              m_pFreeParticles;        // 0x34c
	Particle**             m_ppUsedParticles;       // 0x350, list of used particles per type
	TINT                   m_iNumFreeParticles;     // 0x354
	TINT*                  m_pNumUsedParticles;     // 0x358, per type
	TBOOL                  m_bDebugInfo;            // 0x35c
	Toshi::T2GUIElement    m_oStatsElement;         // 0x360
	AGUI2TextBox           m_oLactStats;            // 0x3b4
	AGUI2TextBox           m_oAllParticles;         // 0x42c
	TWCHAR                 m_wszAllParticles[ 48 ]; // 0x4a4
	AGUI2TextBox*          m_pTypeTextBoxes;        // 0x504
	TWCHAR**               m_pTypeStrings;          // 0x508
	AGUI2TextBox           m_oAmbient;              // 0x50c
	TWCHAR                 m_wszAmbient[ 32 ];      // 0x584
	Toshi::T2GUIRectangle  m_oAmbientBackground;    // 0x5c4
	Toshi::T2GUIRectangle  m_oAmbientColour;        // 0x63c
	AGUI2TextBox           m_oDiffuse;              // 0x6b4
	TWCHAR                 m_wszDiffuse[ 32 ];      // 0x72c
	Toshi::T2GUIRectangle  m_oDiffuseBackground;    // 0x76c
	Toshi::T2GUIRectangle  m_oDiffuseColour;        // 0x7e4
};

TSTATICASSERT( sizeof( AMilkManager::Particle ) == 0x2c );
TSTATICASSERT( sizeof( AMilkManager ) == 0x85c );
