#pragma once

#include <Math/TRandom.h>
#include <Math/TVector3.h>
#include <Math/TVector4.h>
#include <Toshi/TNodeList.h>
#include <Toshi/TSingleton.h>

class AMaterialLibrary;
class ASysMaterial;
class ASysMesh;

class AWeatherManager : public Toshi::TSingleton<AWeatherManager>
{
public:
	class RainDrop : public Toshi::TNodeList<RainDrop>::TNode
	{
	public:
		RainDrop( AWeatherManager* a_pWeatherManager );
		~RainDrop();

		void  Reset( const Toshi::TVector4& a_rPosition, AWeatherManager* a_pWeatherManager );
		TBOOL Update( TFLOAT a_fDeltaTime, const Toshi::TVector4& a_rCameraPosition, AWeatherManager* a_pWeatherManager, TBOOL a_bForceReset );
		void  Render( TFLOAT a_fDeltaTime, const Toshi::TVector4& a_rCameraPosition, AWeatherManager* a_pWeatherManager );

	private:
		ASysMesh*        m_pMesh;
		Toshi::TVector3* m_pPositions;
		TUINT32          m_uiColour;
	};

	class RainSplash : public Toshi::TNodeList<RainSplash>::TNode
	{
	public:
		RainSplash( AWeatherManager* a_pWeatherManager );
		~RainSplash();

		void Reset( TFLOAT a_fRatio, AWeatherManager* a_pWeatherManager );
		void Update( TFLOAT a_fDeltaTime, const Toshi::TVector4& a_rCameraPosition, AWeatherManager* a_pWeatherManager );
		void Render( TFLOAT a_fDeltaTime, const Toshi::TVector4& a_rCameraPosition, AWeatherManager* a_pWeatherManager );

	private:
		TINT             m_iNumSplashes;
		ASysMesh*        m_pMesh;
		Toshi::TVector3* m_pPositions;
		TFLOAT*          m_pTimers;
		TUINT32          m_uiColour;
	};

public:
	AWeatherManager();
	~AWeatherManager();

	void SetCurrentWeather( TFLOAT a_fWeatherIntensity );
	void SetRainDropSize( TFLOAT a_fWidth, TFLOAT a_fLength );
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
	TFLOAT m_fUnkC;
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
	TFLOAT            m_fUnk12C;
	AMaterialLibrary* m_pMaterialLibrary;
	TBOOL             m_bLockToCurrentWeatherType;
};
