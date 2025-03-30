#pragma once
#include "Math/TVector3.h"
#include "Math/TQuaternion.h"
#include "Toshi/TQList.h"
#include "File/TTRB.h"

#ifdef LoadLibrary
#  undef LoadLibrary
#endif LoadLibrary

TOSHI_NAMESPACE_START

constexpr TINT TANIMATION_MAXBONES = 128;

using TAnimVector     = TVector3;
using TAnimQuaternion = TQuaternion;
using TAnimScale      = TFLOAT;
using TAnimationBone  = unsigned short;

class TSkeletonInstance;

class TAnimation : public TQList<TAnimation>::TNode
{
public:
	using Flags = TUINT16;
	enum Flags_ : Flags
	{
		Flags_None                = 0,
		Flags_Active              = BITFLAG( 0 ),
		Flags_Managed             = BITFLAG( 1 ),
		Flags_Unknown2            = BITFLAG( 2 ),
		Flags_UpdateStateOnRemove = BITFLAG( 3 ),
	};

	using State = TUINT32;
	enum State_ : State
	{
		STATE_BLENDING_IN,
		STATE_PLAYING,
		STATE_BLENDING_OUT,
	};

public:
	TAnimation();
	~TAnimation();

	TBOOL UpdateTime( TFLOAT a_fDeltaTime );
	void  RemoveAnimation( TFLOAT a_fVal );

	void   SetState( State a_eState ) { m_eState = a_eState; }
	TFLOAT SetSpeed( TFLOAT a_fSpeed ) { return std::exchange( m_fSpeed, a_fSpeed ); }
	TFLOAT SetDestWeight( TFLOAT a_fDestWeight, TFLOAT a_fBlendInSpeed );
	void   SetBlendInSpeed( TFLOAT a_fBlendInSpeed ) { m_fBlendInSpeed = a_fBlendInSpeed; }
	void   SetBlendOutSpeed( TFLOAT a_fBlendOutSpeed ) { m_fBlendOutSpeed = a_fBlendOutSpeed; }

	void ChangeToManaged( TFLOAT a_fBlendOutSpeed );

	Flags GetFlags() const { return m_eFlags; }
	State GetState() const { return m_eState; }
	TBOOL IsActive() const { return m_eFlags & Flags_Active; }
	TBOOL IsManaged() const { return m_eFlags & Flags_Managed; }
	TBOOL IsUpdateStateOnRemove() const { return m_eFlags & Flags_UpdateStateOnRemove; }

	TUINT16                  GetSequence() const { return m_iSeqID; }
	class TSkeletonSequence* GetSequencePtr() const;
	TSkeletonInstance*       GetSkeletonInstance() const { return m_pSkeletonInstance; }
	TFLOAT                   GetSpeed() const { return m_fSpeed; }
	TFLOAT                   GetSeqTime() const { return m_fSeqTime; }
	TFLOAT                   GetTotalTime() const { return m_fTotalTime; }
	TFLOAT                   GetWeight() const { return m_fWeight; }
	TFLOAT                   GetDestWeight() const { return m_fDestWeight; }
	TFLOAT                   GetBlendInSpeed() const { return m_fBlendInSpeed; }
	TFLOAT                   GetBlendOutSpeed() const { return m_fBlendOutSpeed; }

	TAnimationBone* GetBones() { return TREINTERPRETCAST( TAnimationBone*, this + 1 ); }
	TAnimationBone* GetBone( TINT a_iIndex ) { return &TREINTERPRETCAST( TAnimationBone*, this + 1 )[ a_iIndex ]; }

private:
	TSkeletonInstance* m_pSkeletonInstance;
	TUINT16            m_iSeqID;
	Flags              m_eFlags;
	TINT16             m_iUnk3;
	State              m_eState;
	TFLOAT             m_fSpeed;
	TFLOAT             m_fWeight;
	TFLOAT             m_fDestWeight;
	TFLOAT             m_fTotalTime;
	TFLOAT             m_fSeqTime;
	TFLOAT             m_fBlendInSpeed;
	TFLOAT             m_fBlendOutSpeed;

	friend class TSkeletonInstance;
};

class TKeyframeLibrary : public TDList<TKeyframeLibrary>::TNode
{
public:
	friend class TKeyframeLibraryInstance;

	struct TRBHeader
	{
		const TCHAR*     m_szName;
		TVector3         m_SomeVector;
		TINT             m_iNumTranslations;
		TINT             m_iNumQuaternions;
		TINT             m_iNumScales;
		TINT             m_iTranslationSize;
		TINT             m_iQuaternionSize;
		TINT             m_iScaleSize;
		TAnimVector*     m_pTranslations;
		TAnimQuaternion* m_pQuaternions;
		TAnimScale*      m_pScales;
	};

public:
	static TKeyframeLibrary* CreateFromTRB( TTRB* a_pTRB, const TCHAR* a_szSymbolName );

	void Delete();
	void SetTRB( TTRB* a_pTRB ) { m_pTRB = a_pTRB; }

	const TCHAR* GetName() const
	{
		return m_szName;
	}

private:
	TKeyframeLibrary();

	void IncreaseRefCount()
	{
		m_iReferenceCount++;
	}

	void DecreaseRefCount()
	{
		m_iReferenceCount--;
	}

private:
	TUINT8           m_iNameLength;
	TCHAR            m_szName[ 31 ];
	TVector3         m_SomeVector;
	TINT             m_iReferenceCount;
	TINT             m_iNumTranslations;
	TINT             m_iNumQuaternions;
	TINT             m_iNumScales;
	TINT             m_iTranslationSize;
	TINT             m_iQuaternionSize;
	TINT             m_iScaleSize;
	TAnimVector*     m_pTranslations;
	TAnimQuaternion* m_pQuaternions;
	TAnimScale*      m_pScales;
	TTRB*            m_pTRB;
};

class TKeyframeLibraryInstance
{
public:
	TKeyframeLibraryInstance();

	TKeyframeLibrary* Create( TKeyframeLibrary* a_pLibrary );
	TKeyframeLibrary* CreateEx( TKeyframeLibrary* a_pLibrary, TINT a_iTKeyCount, TINT a_iQKeyCount, TINT a_iSKeyCount, TINT a_iTBaseIndex, TINT a_iQBaseIndex, TINT a_iSBaseIndex );

	void Destroy();

	TAnimVector* GetT( TINT a_iIndex ) const
	{
		TASSERT( a_iIndex < m_iTKeyCount );
		return &m_pTranslations[ a_iIndex ];
	}

	TAnimQuaternion* GetQ( TINT a_iIndex ) const
	{
		TASSERT( a_iIndex < m_iQKeyCount );
		return &m_pQuaternions[ a_iIndex ];
	}

	TAnimScale GetS( TINT a_iIndex ) const
	{
		TASSERT( a_iIndex < m_iQKeyCount );
		return m_pScales[ a_iIndex ];
	}

	void SetLibrary( TKeyframeLibrary* a_pLibrary ) { m_pLibrary = a_pLibrary; }
	void SetTCount( TINT a_iTCount ) { m_iTKeyCount = a_iTCount; }
	void SetQCount( TINT a_iQCount ) { m_iQKeyCount = a_iQCount; }
	void SetSCount( TINT a_iSCount ) { m_iSKeyCount = a_iSCount; }

	TKeyframeLibrary* GetLibrary() const { return m_pLibrary; }
	TINT              GetTCount() const { return m_iTKeyCount; }
	TINT              GetQCount() const { return m_iQKeyCount; }
	TINT              GetSCount() const { return m_iSKeyCount; }

private:
	TKeyframeLibrary* m_pLibrary;
	TINT              m_iTBaseIndex;
	TINT              m_iQBaseIndex;
	TINT              m_iSBaseIndex;
	TINT              m_iTKeyCount;
	TINT              m_iQKeyCount;
	TINT              m_iSKeyCount;
	TAnimVector*      m_pTranslations;
	TAnimQuaternion*  m_pQuaternions;
	TAnimScale*       m_pScales;
};

class TKeyframeLibraryManager
{
public:
	TKeyframeLibraryManager()  = default;
	~TKeyframeLibraryManager() = default;

	TKeyframeLibrary* GetLibrary( const TCHAR* a_szName );

	TKeyframeLibrary* LoadLibrary( const TCHAR* a_szFilePath );
	TKeyframeLibrary* LoadLibrary( TTRB* a_pTRB, const TCHAR* a_szSymbolName );

	void UnloadLibrary( TKeyframeLibrary* a_pLibrary );

private:
	TDList<TKeyframeLibrary> m_List;
	TINT                     m_iNumLibraries = 0;
};

TOSHI_NAMESPACE_END
