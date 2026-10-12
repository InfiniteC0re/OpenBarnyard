#pragma once
#include <Toshi/TTask.h>
#include <Toshi/TSingleton.h>
#include <Toshi/TPString8.h>
#include <Math/TVector4.h>
#include <Math/TQuaternion.h>
#include <Plugins/PPropertyParser/PBProperties.h>

class AParticleEffectHandle;

// A particle effect placed at a terrain locator (see LocatorLists). It plays
// only while the camera is close enough, either constantly or emitting a new
// one-shot effect every EmitTime seconds.
//
// Size: 0x3c
struct AParticleEffectLocator
{
	using FLAGS = TUINT8;
	enum FLAGS_ : FLAGS
	{
		FLAGS_NONE     = 0,
		FLAGS_CONSTANT = BITFLAG( 0 ),
		FLAGS_UNIFORM  = BITFLAG( 1 ),
		FLAGS_RANDOM   = BITFLAG( 2 ),
		FLAGS_PLAYING  = BITFLAG( 3 ),
		FLAGS_ENABLED  = BITFLAG( 4 ),
		FLAGS_TYPEMASK = FLAGS_CONSTANT | FLAGS_UNIFORM | FLAGS_RANDOM,
	};

	AParticleEffectLocator()
	    : fEmitTime( 1.0f )
	    , fEmitTimePlusMinus( 0.3f )
	    , fEmitDistance( 400.0f )
	    , fTimer( 1.0f )
	    , pHandle( TNULL )
	    , uiFlags( FLAGS_ENABLED )
	{}

	// Reads the settings and places the effect at its locator
	TBOOL Setup( const PBProperties* a_pProperties );

	// Starts the constant effect
	void Play();

	// Stops the constant effect if it's playing
	void Stop();

	// Sets the time until the next one-shot effect
	void ResetTimer();

	void Update( TFLOAT a_fDeltaTime );

	TINT                   iEffect;            // 0x00
	Toshi::TVector4        vecPosition;        // 0x04
	Toshi::TQuaternion     quatRotation;       // 0x14
	TFLOAT                 fEmitTime;          // 0x24
	TFLOAT                 fEmitTimePlusMinus; // 0x28
	TFLOAT                 fEmitDistance;      // 0x2c (squared after Setup)
	TFLOAT                 fTimer;             // 0x30
	AParticleEffectHandle* pHandle;            // 0x34
	FLAGS                  uiFlags;            // 0x38
};

// Plays the particle effects listed in data\Particles\LocatorLists\*.trb at the
// terrain locators.
//
// Singleton at 0x007b5e3c, vtable 0x0073aa7c.
class AParticleEffectLocatorManager
    : public Toshi::TTask
    , public Toshi::TSingleton<AParticleEffectLocatorManager>
{
public:
	TDECLARE_CLASS( AParticleEffectLocatorManager, Toshi::TTask );

	inline static constexpr TINT MAX_NUM_LOCATORS = 40;

public:
	AParticleEffectLocatorManager();
	~AParticleEffectLocatorManager();

	//-----------------------------------------------------------------------------
	// Toshi::TTask
	//-----------------------------------------------------------------------------
	virtual TBOOL Reset() OVERRIDE;
	virtual TBOOL OnUpdate( TFLOAT a_fDeltaTime ) OVERRIDE;
	virtual void  OnDestroy() OVERRIDE;

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------

	// Loads every locator list named in the array a_pList
	void LoadLocatorLists( const PBPropertyValue* a_pList );

	// Loads the locators of a list. Delays the loading until the terrain exists.
	void LoadLocatorList( const Toshi::TPString8& a_rcName );

	// Stops and deletes all of the locators
	void Clear();

	TINT                    GetNumLocators() const { return m_iNumLocators; }
	AParticleEffectLocator* GetLocator( TINT a_iIndex ) const { return m_apLocators[ a_iIndex ]; }

private:
	static AParticleEffectLocator* CreateLocator( const PBProperties* a_pProperties );

private:
	TINT                    m_iNumLocators;                   // 0x20
	AParticleEffectLocator* m_apLocators[ MAX_NUM_LOCATORS ]; // 0x24
	Toshi::TPString8        m_strPendingList;                 // 0xc4
};
