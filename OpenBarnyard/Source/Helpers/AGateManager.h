#pragma once
#include "FSM/AFunctionTable.h"
#include "GameInterface/AHackedQuickSave.h"
#include "Physics/ACollisionObjectMesh.h"

#include <Toshi/TObject.h>
#include <Toshi/TSingleton.h>
#include <Toshi/TSceneObject.h>
#include <Toshi/TPString8.h>
#include <Toshi/T2Vector.h>
#include <Math/TMatrix44.h>
#include <Math/TVector4.h>

class AModelInstance;

//-----------------------------------------------------------------------------
// Purpose: The gates of the farm ("wgate_<type>_NN..." locators of the
// terrain) that open once the player has the key of their area (or the quests
// open them), the padlocks of the gates and the fence of the tomato launcher.
// The gates are rendered by the manager and swing open over time (Update).
// TClass 0x0078de7c (version 1.0, size 0x310), singleton at 0x0078de44,
// vtable 0x007324d0.
//-----------------------------------------------------------------------------
class AGateManager
    : public Toshi::TObject
    , public Toshi::TSingleton<AGateManager>
{
public:
	TDECLARE_CLASS( AGateManager, Toshi::TObject );

	// The model of a gate
	using MODEL = TINT;
	enum MODEL_ : MODEL
	{
		MODEL_WHITEGATE = 0,
		MODEL_BROWNGATE = 1, // uses the model of the white gate
		MODEL_JUNKGATE  = 2,
		MODEL_GOLFGATE  = 3,
		MODEL_BARNDOOR  = 4,
		MODEL_PADLOCK   = 5, // blinks and disappears when opened
		MODEL_NUMOF,
	};

	// The area a gate leads to (the keys of the inventory open them)
	using GATETYPE = TINT;
	enum GATETYPE_ : GATETYPE
	{
		GATETYPE_BARN      = 0,
		GATETYPE_SOUTHPATH = 1,
		GATETYPE_WOODS     = 2,
		GATETYPE_GOLF      = 3,
		GATETYPE_HONEY     = 4,
		GATETYPE_JUNKYARD  = 5,
		GATETYPE_OLDMILL   = 6,
		GATETYPE_GULCH     = 7,
		GATETYPE_NIGHTBARN = 8,
		GATETYPE_NUMOF,
	};

	static constexpr TINT MAX_NUM_OPENING_GATES = 8;
	static constexpr TINT NUM_LOADED_MODELS     = 5;

	struct Gate
	    : public Toshi::TSceneObject
	{
		Gate();
		~Gate();

		// Closes the gate (the padlock appears again)
		void Close();

		// The position of the gate
		const Toshi::TVector3& GetPosition() { return GetTransform().GetTranslation(); }

		ACollisionObjectMesh oCollision; // 0x088
		MODEL                eModel;     // 0x0f0
		GATETYPE             eType;      // 0x0f4
		TFLOAT               fAngle;     // 0x0f8, in degrees, the gate swings by it
		TBOOL                bOpen;      // 0x0fc
		TBOOL                bInverted;  // 0x0fd, the model is mirrored
		TINT                 iIndex;     // 0x100
		const TCHAR*         szName;     // 0x104, name of the locator
	};

	struct OpeningGate
	{
		Toshi::TMatrix44 matStart; // 0x00
		Gate*            pGate;    // 0x40
		TFLOAT           fTime;    // 0x44
	};

public:
	AGateManager();
	~AGateManager();

	virtual AFunctionTable* GetFunctionTable(); // +0x0c

	void  Update( TFLOAT a_fDeltaTime );
	void  Render();
	TBOOL Save( AHackedQuickSave::Buffer* a_pBuffer );
	TBOOL Load( AHackedQuickSave::Buffer* a_pBuffer );

	// Opens a_pGate, a_bAnimate swings it open over time (the padlocks blink)
	void OpenGate( Gate* a_pGate, TBOOL a_bAnimate );

	// Opens / closes the closed (open) gates of a_eType.
	// NOTE: CloseGatesOfType does nothing in retail, it calls Close on the closed gates.
	void OpenGatesOfType( GATETYPE a_eType, TBOOL a_bAnimate );
	void CloseGatesOfType( GATETYPE a_eType );

	TBOOL IsGateOpening( Gate* a_pGate );

	Gate* FindGate( GATETYPE a_eType, TINT a_iIndex );
	Gate* FindGateOfType( GATETYPE a_eType );

	// The closest padlock of a_eType whose open state isn't a_bOpen (a_rfDistanceSq is the max distance and receives the distance)
	Gate* FindClosestPadlock( GATETYPE a_eType, const Toshi::TVector4& a_rcPosition, TFLOAT& a_rfDistanceSq, TBOOL a_bOpen );

	// The first padlock within sqrt( a_fMaxDistanceSq ) of the player that can be opened
	Gate* FindNearbyPadlock( TFLOAT a_fMaxDistanceSq );

	TINT  GetNumGates() const { return m_iNumGates; }
	Gate* GetGate( TINT a_iIndex ) const { return &m_pGates[ a_iIndex ]; }

	// The padlock the player stands next to (it's opened by the key)
	Gate* GetNearbyPadlock() const { return m_pNearbyPadlock; }

	void SetTomatoGateOpen( TBOOL a_bOpen );

	// FSM natives
	void  OpenClosestGate();
	void  CloseGateType( Toshi::TPString8 a_strType );
	TBOOL IsGateOpen( Toshi::TPString8 a_strType );
	void  SetGate( Toshi::TPString8 a_strType, TBOOL a_bOpen );
	void  OpenGateType( Toshi::TPString8 a_strType );

	static GATETYPE FindGateType( const Toshi::TPString8& a_rcName );

private:
	// Creates the gates from the locators of the terrain
	void Create();

public:
	// DAT_0078de4c
	static AFunctionTable ms_oFunctionTable;

private:
	Gate*                                               m_pGates;           // 0x004
	TINT                                                m_iNumGates;        // 0x008
	Toshi::TManagedModel*                               m_pModels;          // 0x00c
	Toshi::T2Vector<OpeningGate, MAX_NUM_OPENING_GATES> m_vecOpeningGates;  // 0x010
	Gate*                                               m_pNearbyPadlock;   // 0x254
	Toshi::TMatrix44                                    m_oMirrorMatrix;    // 0x258
	AModelInstance*                                     m_pTomatoTarp;      // 0x298
	AModelInstance*                                     m_pTomatoFence1;    // 0x29c
	AModelInstance*                                     m_pTomatoFence2;    // 0x2a0
	ACollisionObjectMesh                                m_oTomatoCollision; // 0x2a4
	TBOOL                                               m_bTomatoGateOpen;  // 0x30c
};
