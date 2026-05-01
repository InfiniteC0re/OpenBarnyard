#pragma once

#include <Math/TVector4.h>

class AInteriorGoal;

class AInteriorNetworkPath
{
public:
	static constexpr TINT MAX_NUM_PATH_AREAS = 550;

	class BuildPathState
	{
	public:
		BuildPathState();
		~BuildPathState();

		void  Reset();
		void  AdvancePath( TINT a_iNumAreas );
		TINT  GetPathAreaIndex( TINT a_iIndex ) const;
		void  PushPathArea( TINT16 a_iAreaIndex );
		TBOOL ReachedDestination() const;
		void  SetState( const Toshi::TVector4& a_rcTarget, const Toshi::TVector4& a_rcDestination, const Toshi::TVector4& a_rcForward );

	public:
		TINT   m_iPathHead;
		TINT   m_iNumPathAreas;
		TINT16 m_iUnk1;
		TINT16 m_aPathAreas[ MAX_NUM_PATH_AREAS + 1 ];

		Toshi::TVector4 m_vPosition;
		Toshi::TVector4 m_vDestination;
		Toshi::TVector4 m_vForward;

		TINT            m_iUnk2;
		Toshi::TVector4 m_vUnk3;
		TUINT8          m_uiFlags;
	};

public:
	AInteriorNetworkPath();
	~AInteriorNetworkPath();

	void  SetStart( const Toshi::TVector4& a_rcPosition, TINT a_iAreaIndex );
	TBOOL SetPathToGoal( AInteriorGoal* a_pGoal, Toshi::TVector4& a_rOutPosition, Toshi::TVector4& a_rOutForward );

	void OnSerialize( void* );
	void OnDeserialize( void* );

public:
	TFLOAT          m_fUnk1;
	TINT            m_iStartArea;
	Toshi::TVector4 m_vStartPosition;
	BuildPathState  m_oPath;
	TINT            m_Unk3;
};
