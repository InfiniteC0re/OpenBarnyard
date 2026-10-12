#pragma once
#include "ASimAnimModelHelper.h"
#include "ABarnSign.h"

class ACollisionObjectMesh;

//-----------------------------------------------------------------------------
// Purpose: The animated farm objects: the windmills (their heads turn towards
// the wind and the blades spin), the weather vane, the water wheel and the
// barn sign (swings in heavy rain). Size 0x98, vtable 0x007247a4.
//-----------------------------------------------------------------------------
class AWindmillHelper
    : public ASimAnimModelHelper
{
public:
	static constexpr TINT MAX_NUM_WINDMILLS    = 3;
	static constexpr TINT MAX_NUM_WEATHERVANES = 1;
	static constexpr TINT MAX_NUM_WATERWHEELS  = 1;
	static constexpr TINT MAX_NUM_BARNSIGNS    = 1;

	// A windmill: the head (tail model) turns towards the wind, the blades spin around the blades bone of the head
	struct Windmill
	{
		Windmill();

		void Create( const TCHAR* a_szLocator, const Toshi::TPString8& a_rcTailType, const Toshi::TPString8& a_rcBladesType, TFLOAT a_fScale, TFLOAT a_fFadeStart, TFLOAT a_fFadeEnd );
		void Update( TFLOAT a_fDeltaTime, TBOOL a_bDistanceCulling );
		void Reset();

		AModelInstance* pBlades;      // 0x00
		AModelInstance* pTail;        // 0x04
		TFLOAT          fScale;       // 0x08
		TINT            iBladesBone;  // 0x0C
		TFLOAT          fBladesAngle; // 0x10
		TFLOAT          fYaw;         // 0x14
		TFLOAT          fTargetYaw;   // 0x18
	};

	// The weather vane: the pointer turns towards the wind
	struct WeatherVane
	{
		WeatherVane();

		void Create( const TCHAR* a_szLocator );
		void Update( TFLOAT a_fDeltaTime, TBOOL a_bDistanceCulling );

		AModelInstance*       pBase;      // 0x00
		AModelInstance*       pPointer;   // 0x04
		TFLOAT                fYaw;       // 0x08
		TFLOAT                fTargetYaw; // 0x0C
		ACollisionObjectMesh* pCollision; // 0x10
	};

	// The water wheel, spins around its X axis
	struct WaterWheel
	{
		WaterWheel()
		    : pModel( TNULL )
		{}

		void Create( const TCHAR* a_szLocator );
		void Update( TFLOAT a_fDeltaTime, TBOOL a_bDistanceCulling );

		AModelInstance* pModel; // 0x00
	};

public:
	AWindmillHelper();
	virtual ~AWindmillHelper();

	//-----------------------------------------------------------------------------
	// ASimAnimModelHelper
	//-----------------------------------------------------------------------------
	virtual TBOOL OnUpdate( TFLOAT a_fDeltaTime ) OVERRIDE;
	virtual void  OnDestroy() OVERRIDE;

	//-----------------------------------------------------------------------------
	// Own Methods
	//-----------------------------------------------------------------------------
	void CreateBarnSign();

	// Creates the windmills, the weather vane, the water wheel and the barn sign of the farm
	void CreateFarmObjects();

	// Enables or disables the animation (skeleton updating) of the barn sign
	void SetBarnSignAnimated( TBOOL a_bAnimated );
	// Creates the Cow Tipping windmill (wmillcowtip), always animated (no distance culling)
	void CreateCowTippingWindmill();
	// Adds the windmill at locator a_szLocator, fading out between a_fFadeStart and a_fFadeEnd
	void AddWindmill( const TCHAR* a_szLocator, TFLOAT a_fScale, TFLOAT a_fFadeStart, TFLOAT a_fFadeEnd );

	// 00783ed4, the direction of the wind (yaw, radians) the windmills and the weather vane turn towards.
	// Chicken Launch sets it to 0 while it runs.
	inline static TFLOAT ms_fWindDirection = 0.0f;

private:
	void DestroyWindmills();
	void DestroyWeatherVanes();
	void DestroyWaterWheels();

	void PopWindmill();
	void PopWeatherVane();
	void PopWaterWheel();
	void PopBarnSign();

private:
	TINT        m_iNumWindmills;                         // 0x04
	Windmill    m_aWindmills[ MAX_NUM_WINDMILLS ];       // 0x08
	TINT        m_iNumWeatherVanes;                      // 0x5C
	WeatherVane m_aWeatherVanes[ MAX_NUM_WEATHERVANES ]; // 0x60
	TINT        m_iNumWaterWheels;                       // 0x74
	WaterWheel  m_aWaterWheels[ MAX_NUM_WATERWHEELS ];   // 0x78
	TINT        m_iNumBarnSigns;                         // 0x7C
	ABarnSign   m_aBarnSigns[ MAX_NUM_BARNSIGNS ];       // 0x80
	TBOOL       m_bDistanceCulling;                      // 0x94, objects further than the far plane aren't animated
};

TSTATICASSERT( sizeof( AWindmillHelper ) == 0x98 );
