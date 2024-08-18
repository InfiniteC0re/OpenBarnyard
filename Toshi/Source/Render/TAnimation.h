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
	using Flags = uint8_t;
	enum Flags_ : Flags
	{
		Flags_None                = 0,
		Flags_Active              = BITFLAG( 0 ),
		Flags_Managed             = BITFLAG( 1 ),
		Flags_Unknown2            = BITFLAG( 2 ),
		Flags_UpdateStateOnRemove = BITFLAG( 3 ),
	};

	using Mode = TUINT32;
	enum Mode_ : Mode
	{
		MODE_UNK1,
		MODE_UNK2,
		MODE_UNK3,
	};

public:
	TAnimation() = default;

	TBOOL UpdateTime( float a_fDeltaTime );
	void  RemoveAnimation( float a_fVal );

	void  SetMode( Mode a_eMode ) { m_eMode = a_eMode; }
	float SetSpeed( float a_fSpeed ) { return std::exchange( m_fSpeed, a_fSpeed ); }

	float SetDestWeight( float a_fDestWeight, float a_fBlendInSpeed );

	Flags GetFlags() const { return m_eFlags; }
	Mode  GetMode() const { return m_eMode; }
	TBOOL IsActive() const { return m_eFlags & Flags_Active; }
	TBOOL IsManaged() const { return m_eFlags & Flags_Managed; }
	TBOOL IsUpdateStateOnRemove() const { return m_eFlags & Flags_UpdateStateOnRemove; }

	unsigned short           GetSequence() const { return m_iSeqID; }
	class TSkeletonSequence* GetSequencePtr() const;
	TSkeletonInstance*       GetSkeletonInstance() const { return m_pSkeletonInstance; }
	float                    GetSpeed() const { return m_fSpeed; }
	float                    GetSeqTime() const { return m_fSeqTime; }
	float                    GetTotalTime() const { return m_fTotalTime; }
	float                    GetWeight() const { return m_fWeight; }
	float                    GetDestWeight() const { return m_fDestWeight; }
	float                    GetBlendInSpeed() const { return m_fBlendInSpeed; }
	float                    GetBlendOutSpeed() const { return m_fBlendOutSpeed; }

	TAnimationBone* GetBones() { return TREINTERPRETCAST( TAnimationBone*, this + 1 ); }
	TAnimationBone* GetBone( TINT a_iIndex ) { return &TREINTERPRETCAST( TAnimationBone*, this + 1 )[ a_iIndex ]; }

private:
	TSkeletonInstance* m_pSkeletonInstance;
	unsigned short     m_iSeqID;
	Flags              m_eFlags;
	TINT               m_iUnk3;
	Mode               m_eMode;
	float              m_fSpeed;
	float              m_fWeight;
	float              m_fDestWeight;
	float              m_fTotalTime;
	float              m_fSeqTime;
	float              m_fBlendInSpeed;
	float              m_fBlendOutSpeed;
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
