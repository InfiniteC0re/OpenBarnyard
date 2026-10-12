#pragma once
#include "AAnimatedUnit.h"

#include <GUI/T2GUIRectangle.h>
#include <Math/TVector4.h>
#include <Render/TModel.h>
#include <Toshi/TPString8.h>

class ASpline;
class AInteriorNetwork;
class AModelInstance;
class ACollisionObjectMesh;

//-----------------------------------------------------------------------------
// A swarm of bees of the Honey Collecting minigame (unit type "beeswarm").
// TClass 0078292c (parent AAnimatedUnit, size 0x290), vtables 0071e650,
// 0071e644 (AGoalCollisionUpdateInterface).
// Each swarm patrols along a spline of Data\MiniGameData\HoneyCollecting\
// beepatrolN.trb. A cone in front of it ("data\models\pie_fov", m_pFOVCollision)
// is what the bees see: when the player walks into it, they notice him
// (STATE_ALERT), get angry (red, faster) and chase him through the "woods"
// interior network (STATE_ATTACK). Close enough, they sting him (STATE_STING)
// until he swats them away or hides. A squirt of milk disperses them.
//-----------------------------------------------------------------------------
class ABeeSwarm
    : public AAnimatedUnit
{
public:
	TDECLARE_CLASS( ABeeSwarm, AAnimatedUnit );

	enum STATE : TINT
	{
		STATE_IDLE,            // 0, the steer stops
		STATE_PATROL,          // 1, follows the patrol spline (AStationarySteer)
		STATE_RETURN,          // 2, flies back to the closest point of the spline (AInteriorSteer)
		STATE_SEEK_POLLEN,     // 3, set by the game when the swarm brought honey to a hive that isn't full
		STATE_SEEK_HIVE,       // 4, never set
		STATE_AT_HIVE,         // 5, the game gives m_iHoney to the hive
		STATE_POLLEN,          // 6, rests above the flower bed for 5 seconds ("pollen_lp")
		STATE_ATTACK,          // 7, chases the player ("attack_lp")
		STATE_ATTACK_DELAYED,  // 8, attacks after 1 second, never set
		STATE_STING,           // 9, sticks to the player ("sting_lp", "sting1"..."sting4")
		STATE_DISPERSE,        // 10, swatted away by the player ("disperse")
		STATE_DISPERSE_MILKED, // 11, hit by the milk
		STATE_ALERT,           // 12, noticed the player
	};

public:
	ABeeSwarm();
	~ABeeSwarm();

	//-----------------------------------------------------------------------------
	// AUnit
	//-----------------------------------------------------------------------------
	virtual TBOOL          Create( AUnitTransform* a_pTransform ) OVERRIDE; // +0x58
	virtual TBOOL          Think( TFLOAT a_fDeltaTime ) OVERRIDE;           // +0x6c
	virtual Toshi::TClass* GetSteerClass() OVERRIDE;                        // +0x80

	// Sets the patrol, the "woods" network the swarm chases the player through and the
	// two minimap icons (the swarm and its field of view), then puts it at the start of
	// the patrol and creates the field of view cone. a_bUnk is stored and never read.
	TBOOL Setup( AInteriorNetwork* a_pNetwork, TBOOL a_bUnk, ASpline* a_pPatrol, TINT a_iMapIcon, TINT a_iFOVMapIcon );

	// Puts the swarm at the start of its patrol (STATE_IDLE, AStationarySteer)
	void Reset();

	void SetState( STATE a_eState );

	// The angry swarm is drawn pulsing red (RenderCallback), moves faster and buzzes higher
	void SetAngry( TBOOL a_bAngry );

	// The flower bed the swarm rests on (STATE_POLLEN) when its patrol passes by it
	void SetFlowerBed( const Toshi::TVector4& a_rcPosition );

	// Pauses the buzz of the swarm
	void PauseSound( TBOOL a_bPause );

	STATE  GetState() const { return m_eState; }
	TBOOL  IsAngry() const { return m_bAngry; }
	TINT   GetMapIcon() const { return m_iMapIcon; }
	TINT   GetFOVMapIcon() const { return m_iFOVMapIcon; }
	TINT   GetHoney() const { return m_iHoney; }
	void   SetHoney( TINT a_iHoney ) { m_iHoney = a_iHoney; }
	void   SetIgnorePlayer( TBOOL a_bIgnore ) { m_bIgnorePlayer = a_bIgnore; }
	TFLOAT GetHideTime() const { return m_fHideTime; }
	void   SetHideTime( TFLOAT a_fTime ) { m_fHideTime = a_fTime; }

private:
	// Whether the player is inside the field of view cone
	TBOOL CanSeePlayer();

	void UpdatePatrol( TFLOAT a_fDeltaTime );
	void UpdateReturn( TFLOAT a_fDeltaTime );
	void UpdateSeekPollen( TFLOAT a_fDeltaTime );
	void UpdateSeekHive( TFLOAT a_fDeltaTime );
	void UpdatePollen( TFLOAT a_fDeltaTime );
	void UpdateAttack( TFLOAT a_fDeltaTime );
	void UpdateAttackDelayed( TFLOAT a_fDeltaTime );
	void UpdateSting( TFLOAT a_fDeltaTime );
	void UpdateDisperse( TFLOAT a_fDeltaTime );
	void UpdateAlert( TFLOAT a_fDeltaTime );

	// Custom render method of the model of the angry swarm
	static void RenderCallback( Toshi::TModelInstance* a_pInstance, void* a_pUserData );

private:
	TINT                                    m_iUnk98;              // 0x098
	STATE                                   m_eState;              // 0x09c
	Toshi::TPString8                        m_aUnkStrings[ 40 ];   // 0x0a0, never used
	TINT                                    m_iUnk140;             // 0x140
	TINT                                    m_iUnk144;             // 0x144
	Toshi::TVector4                         m_vUnk148;             // 0x148
	Toshi::TVector4                         m_vUnk158;             // 0x158
	TINT                                    m_iUnk168;             // 0x168
	TINT                                    m_iUnk16c;             // 0x16c, not initialised
	TINT                                    m_iSound;              // 0x170, ASoundId of the buzz
	TINT                                    m_iCue;                // 0x174
	TBOOL                                   m_bUnk178;             // 0x178, Setup( a_bUnk )
	TBOOL                                   m_bUnk179;             // 0x179
	TFLOAT                                  m_fPollenTime;         // 0x17c, STATE_POLLEN
	TFLOAT                                  m_fAttackDelay;        // 0x180, STATE_ATTACK_DELAYED
	Toshi::TVector4                         m_vTarget;             // 0x184, the player's position (STATE_ATTACK)
	TINT                                    m_iUnk194;             // 0x194
	TINT                                    m_iUnk198;             // 0x198, not initialised
	TINT                                    m_iHoney;              // 0x19c, brought to the hive (STATE_SEEK_HIVE)
	Toshi::TPString8                        m_strAnimation;        // 0x1a0
	TINT                                    m_iUnk1a4;             // 0x1a4
	TINT                                    m_iUnk1a8;             // 0x1a8
	TFLOAT                                  m_fStateTime;          // 0x1ac, STATE_ATTACK, STATE_DISPERSE*, STATE_ALERT
	TBOOL                                   m_bIgnorePlayer;       // 0x1b0, never set
	TFLOAT                                  m_fAttackCooldown;     // 0x1b4, never set
	Toshi::TVector4                         m_vFlowerBed;          // 0x1b8, also the patrol point flown back to (STATE_RETURN)
	ASpline*                                m_pPatrol;             // 0x1c8
	TFLOAT                                  m_fPatrolProgress;     // 0x1cc, 0.0f..1.0f
	AInteriorNetwork*                       m_pNetwork;            // 0x1d0
	TFLOAT                                  m_fStingTime;          // 0x1d4, time to the next "stingN" animation
	TFLOAT                                  m_fFlowerBedProgress;  // 0x1d8, -1.0f when the patrol doesn't pass by a flower bed
	TFLOAT                                  m_fPollenCooldown;     // 0x1dc
	Toshi::T2GUIRectangle                   m_oUnkRectangle;       // 0x1e0, never used
	TINT                                    m_iMapIcon;            // 0x258
	TINT                                    m_iFOVMapIcon;         // 0x25c
	TBOOL                                   m_bAngry;              // 0x260
	Toshi::TModelInstance::t_CustomRenderCB m_fnRenderCallback;    // 0x264, of the model, replaced while angry
	void*                                   m_pRenderCallbackData; // 0x268
	TFLOAT                                  m_fDisperseTime;       // 0x26c
	TFLOAT                                  m_fHideTime;           // 0x270, AHoneyCollectingGame: time the player has been hiding from the swarm
	AModelInstance*                         m_pFOVModel;           // 0x274, "data\models\pie_fov"
	ACollisionObjectMesh*                   m_pFOVCollision;       // 0x278
	TFLOAT                                  m_fFadeInTime;         // 0x27c, of the next buzz
	TFLOAT                                  m_fFadeOutTime;        // 0x280, of the current buzz
	TFLOAT                                  m_fFrequency;          // 0x284, of the buzz
	TFLOAT                                  m_fCurrentFrequency;   // 0x288
	TFLOAT                                  m_fWobbleTime;         // 0x28c, the angry swarm weaves along the patrol
};

TSTATICASSERT( sizeof( ABeeSwarm ) == 0x290 );
