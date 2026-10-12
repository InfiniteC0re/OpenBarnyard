#pragma once

#include <Math/TRandom.h>
#include <Math/TVector3.h>
#include <Math/TVector4.h>
#include <Toshi/TNodeList.h>
#include <Toshi/TSingleton.h>

class AMaterialLibrary;
class ASysMaterial;
class ASysMesh;

// Renders the rain drops and the splashes of the rain on the ground.
// NOTE: the original name is unknown (it has no TClass), the original keeps it at ARenderer+0x80.
class ARainRenderer : public Toshi::TSingleton<ARainRenderer>
{
public:
	class RainDrop : public Toshi::TNodeList<RainDrop>::TNode
	{
	public:
		RainDrop( ARainRenderer* a_pRainRenderer );
		~RainDrop();

		void Reset( const Toshi::TVector4& a_rPosition, ARainRenderer* a_pRainRenderer );
		// Returns TFALSE once all of the positions landed, a_bRecycle respawns the landed positions instead
		TBOOL Update( TFLOAT a_fDeltaTime, const Toshi::TVector4& a_rCameraPosition, ARainRenderer* a_pRainRenderer, TBOOL a_bRecycle );
		void  Render( TFLOAT a_fDeltaTime, const Toshi::TVector4& a_rCameraPosition, ARainRenderer* a_pRainRenderer );

	private:
		// Every drop is drawn as a few streaks
		static constexpr TINT NUM_POSITIONS = 10;

	private:
		ASysMesh*        m_pMesh;      // 0x0c
		Toshi::TVector3* m_pPositions; // 0x10
		TUINT32          m_uiColour;   // 0x14
	};

	class RainSplash : public Toshi::TNodeList<RainSplash>::TNode
	{
	public:
		RainSplash( ARainRenderer* a_pRainRenderer );
		~RainSplash();

		void Reset( TFLOAT a_fRatio, ARainRenderer* a_pRainRenderer );
		void Update( TFLOAT a_fDeltaTime, const Toshi::TVector4& a_rCameraPosition, ARainRenderer* a_pRainRenderer );
		void Render( TFLOAT a_fDeltaTime, const Toshi::TVector4& a_rCameraPosition, ARainRenderer* a_pRainRenderer );

	private:
		// Places the splash a_iIndex on the ground somewhere around the rain origin
		void ResetSplash( TINT a_iIndex, ARainRenderer* a_pRainRenderer );

	private:
		TINT             m_iNumSplashes;
		ASysMesh*        m_pMesh;
		Toshi::TVector3* m_pPositions;
		TFLOAT*          m_pTimers;
		TUINT32          m_uiColour;
	};

public:
	ARainRenderer();
	~ARainRenderer();

	void SetRainDropSize( TFLOAT a_fWidth, TFLOAT a_fLength );

	// Sets the rain intensity and the interval of spawning the drops (AGameTimeFX::Apply)
	void SetRainIntensity( TFLOAT a_fIntensity )
	{
		m_fRainIntensity     = a_fIntensity;
		m_fRainSpawnInterval = 1.0f / ( a_fIntensity * m_fMaxRainDrops );
	}

	void SetRainSpeed( TFLOAT a_fFallSpeed, TFLOAT a_fWindStrength )
	{
		m_fRainFallSpeed = a_fFallSpeed;
		m_fWindStrength  = a_fWindStrength;
	}

	void SetLockToCurrentWeatherType( TBOOL a_bLock ) { m_bLockToCurrentWeatherType = a_bLock; }
	void Update( TFLOAT a_fDeltaTime );
	void Render();

private:
	void CreateMaterials();
	void DestroyMaterials();
	void CreateRainDropPool();
	void CreateRainSplashPool();

private:
	TFLOAT m_fDeltaTime;
	TFLOAT m_fElapsedTime;
	TFLOAT m_fRainIntensity;
	TFLOAT m_fRainFallSpeed;
	TFLOAT m_fMaxRainDrops;
	TFLOAT m_fRainDropWidth;
	TFLOAT m_fRainDropLength;
	TFLOAT m_fRainSpawnTimer;
	TFLOAT m_fRainSpawnInterval;
	TUINT  m_uiRandomSeed;

	ASysMaterial*  m_pRainMaterial;
	ASysMaterial*  m_pRainSplashMaterial;
	Toshi::TRandom m_oRandom;

	TINT                       m_iMaxRainDrops;
	Toshi::TNodeList<RainDrop> m_llFreeRainDrops;
	Toshi::TNodeList<RainDrop> m_llActiveRainDrops;

	TINT                         m_iMaxRainSplashes;
	Toshi::TNodeList<RainSplash> m_llFreeRainSplashes;
	Toshi::TNodeList<RainSplash> m_llActiveRainSplashes;

	Toshi::TVector4   m_vRainOrigin;
	Toshi::TVector4   m_vWindDirection;
	TFLOAT            m_fWindStrength;
	AMaterialLibrary* m_pMaterialLibrary;
	TBOOL             m_bLockToCurrentWeatherType;
};
