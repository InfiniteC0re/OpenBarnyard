#pragma once
#include "Render/ASysShader/ASysMesh.h"

#include <File/TTRB.h>
#include <Toshi/TArray.h>
#include <Toshi/TNodeList.h>
#include <Math/TMatrix44.h>
#include <Math/TQuaternion.h>
#include <Math/TPlane.h>
#include <Math/TRandom.h>

class ASysMaterial;
class AMaterialLibrary;
class AParticleSystem;
class AParticleEmitter;
class AParticleGenericEmitter;
class AParticleEffectPool;
class AParticleEffectInstance;

namespace Toshi
{
class TManagedModel;
class TSceneObject;
class TKeyframeLibrary;
class TTexture;
} // namespace Toshi

//-----------------------------------------------------------------------------
// Paged attribute storage
//-----------------------------------------------------------------------------
// Particle attributes (positions, ages, colours...) are stored in pages of 32
// elements. Every attribute type has a global pool of free pages, the emitters
// take pages from the pool when they grow and give them back when they shrink.
//-----------------------------------------------------------------------------

using AParticleAttributePoolType = TINT;
enum AParticleAttributePoolType_ : AParticleAttributePoolType
{
	AParticleAttributePoolType_Vector,
	AParticleAttributePoolType_Float,
	AParticleAttributePoolType_Dword,
	AParticleAttributePoolType_Byte,
	AParticleAttributePoolType_Vector2,
};

template <typename T, AParticleAttributePoolType POOL>
class AParticleAttributePool
{
public:
	inline static constexpr TINT PAGE_SHIFT = 5;
	inline static constexpr TINT PAGE_SIZE  = 1 << PAGE_SHIFT;

public:
	// Allocates a_iNumPages pages for the pool
	// $Barnyard: FUNCTION 00678d00
	// $Barnyard: FUNCTION 00678da0
	// $Barnyard: FUNCTION 00678e40
	// $Barnyard: FUNCTION 00678ee0
	// $Barnyard: FUNCTION 00678f80
	static void Create( TINT a_iNumPages )
	{
		ms_iNumPages = a_iNumPages;

		if ( a_iNumPages > ms_oFreePages.Capacity() )
			ms_oFreePages.Resize( a_iNumPages );

		ms_oFreePages.SetSize( a_iNumPages );
		ms_pMemory = TSTATICCAST( T, TMalloc( ms_iNumPages * PAGE_SIZE * sizeof( T ) ) );

		for ( TINT i = 0; i < ms_iNumPages; i++ )
			ms_oFreePages[ i ] = ms_pMemory + i * PAGE_SIZE;

		ms_iShift = PAGE_SHIFT;
		ms_iMask  = PAGE_SIZE - 1;
	}

	static void Destroy()
	{
		ms_oFreePages.Resize( 0 );

		TFree( ms_pMemory );
		ms_iNumPages = 0;
		ms_pMemory   = TNULL;
	}

public:
	inline static Toshi::TArray<T*> ms_oFreePages;
	inline static TINT              ms_iNumPages;
	inline static T*                ms_pMemory;
	inline static TINT              ms_iShift;
	inline static TINT              ms_iMask;
};

template <typename T, AParticleAttributePoolType POOL>
class AParticleAttribute
{
public:
	using Pool = AParticleAttributePool<T, POOL>;

	inline static constexpr TINT MAX_NUM_PAGES = 10;

public:
	AParticleAttribute()
	    : m_aPages( 4, 10 )
	{}

	// $Barnyard: FUNCTION 006724d0
	// $Barnyard: FUNCTION 00672530
	// $Barnyard: FUNCTION 00672590
	// $Barnyard: FUNCTION 006725f0
	~AParticleAttribute()
	{
		Resize( 0 );
	}

	// Takes or gives back pages so the attribute can store a_iNumElements, returns TFALSE if there are not enough pages
	// $Barnyard: FUNCTION 00671d00
	// $Barnyard: FUNCTION 00671e10
	// $Barnyard: FUNCTION 00671f20
	// $Barnyard: FUNCTION 00672030
	TBOOL Resize( TINT a_iNumElements )
	{
		TINT iNumPages = a_iNumElements >> Pool::ms_iShift;

		if ( ( a_iNumElements & Pool::ms_iMask ) > 0 )
			iNumPages++;

		if ( iNumPages == m_aPages.Size() )
			return TTRUE;

		if ( iNumPages > MAX_NUM_PAGES )
			return TFALSE;

		if ( iNumPages < m_aPages.Size() )
		{
			// Give the pages back to the pool
			do
			{
				Pool::ms_oFreePages.Push( m_aPages[ m_aPages.Size() - 1 ] );

				if ( m_aPages.Size() > 0 )
					m_aPages.SetSize( m_aPages.Size() - 1 );

			} while ( iNumPages < m_aPages.Size() );

			return TTRUE;
		}

		TINT iNumFree = Pool::ms_oFreePages.Size();

		if ( iNumFree > 0 )
		{
			TINT iPage = m_aPages.Size();

			if ( iPage >= iNumPages )
				return TTRUE;

			while ( iNumFree > 0 )
			{
				m_aPages.Push( Pool::ms_oFreePages[ iNumFree - 1 ] );

				iNumFree = Pool::ms_oFreePages.Size();
				if ( iNumFree > 0 )
				{
					iNumFree--;
					Pool::ms_oFreePages.SetSize( iNumFree );
				}

				if ( ++iPage >= iNumPages )
					return TTRUE;
			}
		}

		return TFALSE;
	}

	// Number of pages this attribute holds
	TINT GetNumPages() const { return m_aPages.Size(); }

	// Number of elements the pages can hold
	TINT GetCapacity() const { return m_aPages.Size() << Pool::ms_iShift; }

	T&       operator[]( TINT a_iIndex ) { return m_aPages[ a_iIndex >> Pool::ms_iShift ][ a_iIndex & Pool::ms_iMask ]; }
	const T& operator[]( TINT a_iIndex ) const { return m_aPages[ a_iIndex >> Pool::ms_iShift ][ a_iIndex & Pool::ms_iMask ]; }

private:
	Toshi::TArray<T*> m_aPages;
};

using AParticleVectorAttribute = AParticleAttribute<Toshi::TVector4, AParticleAttributePoolType_Vector>;
using AParticleFloatAttribute  = AParticleAttribute<TFLOAT, AParticleAttributePoolType_Float>;
using AParticleDwordAttribute  = AParticleAttribute<TUINT32, AParticleAttributePoolType_Dword>;
using AParticleByteAttribute   = AParticleAttribute<TUINT8, AParticleAttributePoolType_Byte>;

//-----------------------------------------------------------------------------
// Data of the particle library (Data\Particles\*.trb)
//-----------------------------------------------------------------------------
// The function pointers of the data are stored in the TRB as placeholders and
// are resolved by the particle system when the library is loaded.
//-----------------------------------------------------------------------------

// Header of the parameters of particle functions
struct AParticleFunctionData
{
	TUINT32 uiUnk;  // 0x0
	TINT    iType;  // 0x4
	TUINT32 uiSize; // 0x8, size of the whole data in bytes
};

struct AParticleInitData
{
	// NOTE: does nothing on PC (it probably swaps the bytes on the consoles)
	// $Barnyard: FUNCTION 00672960
	static TUINT32 Convert( TUINT32 a_uiValue ) { return a_uiValue; }
};

// Initialises an attribute of the particles a_iStart to a_iEnd
using AParticleInitFunc = void( __stdcall* )( void* a_pAttribute, TINT a_iStart, TINT a_iEnd, AParticleFunctionData* a_pData );

// Changes the particles a_iStart to a_iEnd every frame
using AParticleAffectorFunc = void( __stdcall* )( AParticleGenericEmitter* a_pEmitter, TINT a_iStart, TINT a_iEnd, TFLOAT a_fDeltaTime, AParticleFunctionData* a_pData );

struct AParticleInit
{
	// Type of AParticleFunctionData::iType, picks fnInit
	using TYPE = TINT;
	enum TYPE_ : TYPE
	{
		TYPE_VECTOR_FIXED        = 0,
		TYPE_VECTOR_IN_BOX       = 1,
		TYPE_VECTOR_ON_SPHERE    = 2,
		TYPE_VECTOR_FROM_EMITTER = 3,
		TYPE_VECTOR_TO_EMITTER   = 4,
		TYPE_VECTOR_IN_CONE      = 5,
		TYPE_VECTOR_IN_ANGLE     = 6,
		TYPE_VECTOR_IN_SPHERE    = 7,
		TYPE_VECTOR_ON_SPHERE_2  = 8, // same as TYPE_VECTOR_ON_SPHERE
		TYPE_VECTOR_ON_BOX       = 9,
		TYPE_VECTOR_IN_CIRCLE    = 10,
		TYPE_VECTOR_ON_CIRCLE    = 11,
		TYPE_VECTOR_IN_DOME      = 12,
		TYPE_FLOAT_FIXED         = 13,
		TYPE_FLOAT_RANGE         = 14,
		TYPE_FLOAT_FROM_SIZE     = 15,
		TYPE_DWORD_FIXED         = 16,
		TYPE_DWORD_RANGE         = 17,
		TYPE_COLOUR_FIXED        = 18,
		TYPE_COLOUR_RANGE        = 19,
	};

	AParticleInitFunc      fnInit; // 0x0
	AParticleFunctionData* pData;  // 0x4

	AParticleInit( const AParticleInit& a_rcInit );

	// $Barnyard: FUNCTION 00675180
	~AParticleInit() { TFree( pData ); }

	// Resolves fnInit from the type of the data
	void Resolve();
};

struct AParticleAffector
{
	// Type of AParticleFunctionData::iType, picks fnAffector
	using TYPE = TINT;
	enum TYPE_ : TYPE
	{
		TYPE_MOVE            = 0,
		TYPE_DRAG            = 1,
		TYPE_DAMPING         = 2,
		TYPE_FORCE           = 3,
		TYPE_RANDOM_FORCE    = 4,
		TYPE_ROTATE_VELOCITY = 5,
		TYPE_ATTRACT         = 6,
		TYPE_ROTATE_X        = 7,
		TYPE_ROTATE_Y        = 8,
		TYPE_ROTATE_Z        = 9,
		TYPE_NONE            = 10,
	};

	AParticleFunctionData* pData;      // 0x0
	AParticleAffector*     pNext;      // 0x4
	AParticleAffectorFunc  fnAffector; // 0x8

	AParticleAffector( const AParticleAffector& a_rcAffector );

	// NOTE: the original shares the destructor with other holders of a single allocation (00421c40)
	~AParticleAffector() { TFree( pData ); }

	// Resolves fnAffector from the type of the data
	void Resolve();
};

// A key of the particle timelines
struct AParticleKeyframeData
{
	// How the progress between two keys is eased, see AParticleKeyframe::GetProgress
	using CURVE = TINT16;
	enum CURVE_ : CURVE
	{
		CURVE_STEP           = 0,
		CURVE_LINEAR         = 1,
		CURVE_EASE_IN        = 2, // quadratic
		CURVE_EASE_OUT       = 3, // quadratic
		CURVE_EASE_IN_CUBIC  = 4,
		CURVE_EASE_OUT_CUBIC = 5,
		CURVE_EASE_IN_OUT    = 6,
		CURVE_GAIN           = 7,
	};

	TUINT32 uiUnk;     // 0x0
	CURVE   iCurve;    // 0x4
	TINT16  iLoopType; // 0x6
	TINT    iType;     // 0x8
	TFLOAT  fTime;     // 0xc
	TUINT32 uiSize;    // 0x10, size of the whole data in bytes
	union
	{
		TFLOAT  fValue;  // 0x14
		TUINT32 uiValue; // 0x14
	};
};

struct AParticleTimelineState;
struct AParticleEffectEmitter;
struct AParticleEmitterSettings;
struct AParticleKeyframe;

// Data of the colour keys (TYPE_COLOUR)
struct AParticleColourKeyframeData
{
	TUINT32 uiUnk;     // 0x0
	TINT16  iCurve;    // 0x4
	TINT16  iLoopType; // 0x6
	TINT    iType;     // 0x8
	TFLOAT  fTime;     // 0xc
	TUINT32 uiSize;    // 0x10
	TUINT32 uiUnk2;    // 0x14
	TUINT32 uiColour;  // 0x18, ARGB
};

// Data of the keys that spawn emitters (TYPE_EMITTER)
struct AParticleEmitterKeyframeData
{
	TUINT32                   uiUnk;         // 0x0
	TINT16                    iCurve;        // 0x4
	TINT16                    iLoopType;     // 0x6
	TINT                      iType;         // 0x8
	TFLOAT                    fTime;         // 0xc
	TUINT32                   uiSize;        // 0x10
	AParticleEmitterSettings* pSettings;     // 0x14
	AParticleKeyframe*        pPrevious;     // 0x18
	TFLOAT                    fNumParticles; // 0x1c, part of the max particles of the settings
};

struct AParticleKeyframe
{
	using t_OnStart = void( __stdcall* )( AParticleKeyframe* a_pKeyframe, AParticleKeyframe* a_pPrevious );
	using t_Update  = void( __stdcall* )( AParticleKeyframe* a_pKeyframe, TFLOAT a_fDeltaTime, TFLOAT a_fProgress, AParticleTimelineState* a_pState );
	using t_Apply   = void( __stdcall* )( AParticleKeyframe* a_pKeyframe, AParticleGenericEmitter* a_pEmitter, TFLOAT a_fProgress, TINT a_iParticle );

	using TYPE = TINT;
	enum TYPE_ : TYPE
	{
		TYPE_COLOUR             = 1,
		TYPE_MASS               = 2,
		TYPE_SIZE               = 4,
		TYPE_SIZE_SCALE         = 5,
		TYPE_UV                 = 6,
		TYPE_EMITTER            = 7,
		TYPE_REVERSE_VELOCITY   = 8,
		TYPE_RANDOMISE_VELOCITY = 9,
	};

	using LOOPTYPE = TINT16;
	enum LOOPTYPE_ : LOOPTYPE
	{
		LOOPTYPE_NONE,
		LOOPTYPE_START,
		LOOPTYPE_END,
	};

	AParticleKeyframeData* pData;     // 0x0
	AParticleKeyframe*     pPrevious; // 0x4
	t_OnStart              fnOnStart; // 0x8
	t_Update               fnUpdate;  // 0xc
	t_Apply                fnApply;   // 0x10

	AParticleKeyframe( const AParticleKeyframe& a_rcKeyframe );

	// NOTE: the original shares the destructor with other holders of a single allocation (00421c40)
	~AParticleKeyframe() { TFree( pData ); }

	// Resolves the callbacks from the type of the data
	void Resolve();

	// Remaps the linear progress a_fProgress with the curve of the keyframe
	TFLOAT GetProgress( TFLOAT a_fProgress );

	// Blends the colour of a_iParticle to the colour of the key according to its age
	TUINT32 BlendColour( AParticleGenericEmitter* a_pEmitter, TINT a_iParticle );

	// Blends the size of a_iParticle to the size of the key according to its age
	TFLOAT BlendSize( AParticleGenericEmitter* a_pEmitter, TINT a_iParticle );

	// $Barnyard: FUNCTION 00676800
	TBOOL IsType( TYPE a_eType ) const { return pData->iType == a_eType; }
};

struct AParticleTimeline
{
	AParticleKeyframe** ppKeyframes;   // 0x0
	TINT                iNumKeyframes; // 0x4

	AParticleTimeline( const AParticleTimeline& a_rcTimeline );
	~AParticleTimeline();

	// Resolves the keyframes and links them to the previous ones
	void Resolve();

	// Goes to the next keyframe of a_pState, returns TNULL if the timeline is over
	AParticleKeyframe* GetNextKeyframe( AParticleTimelineState* a_pState );

	// Updates the current keyframe of a_pState, returns TFALSE if the timeline is over
	TBOOL Update( TFLOAT a_fDeltaTime, AParticleTimelineState* a_pState );

	// Applies the keyframes to the particles a_iStart to a_iEnd according to their age
	void ApplyByAge( AParticleGenericEmitter* a_pEmitter, TINT a_iStart, TINT a_iEnd );

	// Applies the next keyframe of the particles a_iStart to a_iEnd once they reach it
	void ApplyEvents( AParticleGenericEmitter* a_pEmitter, TINT a_iStart, TINT a_iEnd );
};

// State of a timeline that is being played
struct AParticleTimelineState
{
	TINT16             iKeyframe; // 0x0
	TINT16             iLoop;     // 0x2, goes back to the start of the loop at the end of it
	AParticleKeyframe* pKeyframe; // 0x4
	AParticleKeyframe* pPrevious; // 0x8
	TFLOAT             fTime;     // 0xc

	void Reset();
};

// Settings of the emitters (0x94 bytes)
struct AParticleEmitterSettings
{
	using FLAGS = TUINT32;
	enum FLAGS_ : FLAGS
	{
		FLAGS_NONE     = 0,
		FLAGS_ROTATE   = BITFLAG( 2 ),
		FLAGS_LIGHTING = BITFLAG( 3 ),
	};

	using RENDERTYPE = TINT;
	enum RENDERTYPE_ : RENDERTYPE
	{
		RENDERTYPE_SPRITE,
		RENDERTYPE_SPARK,
		RENDERTYPE_STRETCHED,
		RENDERTYPE_RIBBON,
		RENDERTYPE_MESH,
	};

	using INIT = TINT;
	enum INIT_ : INIT
	{
		INIT_POSITION,
		INIT_VELOCITY,
		INIT_LIFE,
		INIT_MASS,
		INIT_SIZE,
		INIT_UV,
		INIT_COLOUR,
		INIT_NUMOF
	};

	using TIMELINE = TINT;
	enum TIMELINE_ : TIMELINE
	{
		TIMELINE_UNK,
		TIMELINE_COLOUR,
		TIMELINE_MASS,
		TIMELINE_UNK2,
		TIMELINE_SIZE,
		TIMELINE_UV,
		TIMELINE_EVENTS,
		TIMELINE_NUMOF
	};

	// Parameters of the UV animation (0x24 bytes)
	struct UVData
	{
		TUINT32 uiUnk;        // 0x0
		TINT    iUnk;         // 0x4
		TUINT32 auiUnk[ 3 ];  // 0x8
		TINT    iUnk2;        // 0x14
		TUINT32 auiUnk2[ 3 ]; // 0x18

		UVData( const UVData& a_rcData );
	};

	// Parameters of the rotating sprites (0x50 bytes)
	struct RotationData
	{
		TINT   iNumRotations;  // 0x0
		TFLOAT fMinSpeed;      // 0x4
		TFLOAT fMaxSpeed;      // 0x8
		TFLOAT afSpeeds[ 16 ]; // 0xc
		TBOOL  bFixedCorners;  // 0x4c

		void RandomiseSpeeds();
	};

	// Parameters of the spark sprites (0xf4 bytes)
	struct SparkData
	{
		using FLAGS = TUINT8;
		enum FLAGS_ : FLAGS
		{
			FLAGS_NONE       = 0,
			FLAGS_LOCAL_AXES = BITFLAG( 0 ), // vecForward and vecUp are in the space of the emitter
		};

		Toshi::TVector3 vecForward;      // 0x0
		Toshi::TVector3 vecUp;           // 0xc
		TINT16          iNumRotations;   // 0x18
		FLAGS           uiFlags;         // 0x1a
		TUINT8          uiPad;           // 0x1b
		TFLOAT          fMinSpeedX;      // 0x1c
		TFLOAT          fMaxSpeedX;      // 0x20
		TFLOAT          fMinSpeedY;      // 0x24
		TFLOAT          fMaxSpeedY;      // 0x28
		TFLOAT          fMinSpeedZ;      // 0x2c
		TFLOAT          fMaxSpeedZ;      // 0x30
		TFLOAT          afSpeedsX[ 16 ]; // 0x34
		TFLOAT          afSpeedsY[ 16 ]; // 0x74
		TFLOAT          afSpeedsZ[ 16 ]; // 0xb4

		void RandomiseSpeeds();
	};

	// Parameters of the stretched sprites (0x14 bytes)
	struct StretchData
	{
		TUINT32 uiUnk;       // 0x0
		TFLOAT  fFrontScale; // 0x4
		TFLOAT  fBackScale;  // 0x8
		TUINT32 uiUnk2;      // 0xc
		TFLOAT  fWidthScale; // 0x10
	};

	// Parameters of the ribbons and the attachment of the emitter (0x14 bytes)
	struct AttachData
	{
		TUINT32         bAttachFirst; // 0x0, the first particle stays at the emitter
		Toshi::TVector3 vecUp;        // 0x4
		TUINT8          uiUpAxis;     // 0x10, 1-3: uses the basis vector of the emitter's previous rotation as up
		TUINT8          uiStartAxis;  // 0x11, 1-3: stores the basis vector of the emitter in the new particles
		TUINT8          auiPad[ 2 ];  // 0x12
	};

	AParticleEmitterSettings( const AParticleEmitterSettings& a_rcSettings );
	~AParticleEmitterSettings();

	// Resolves the functions, the material and the model
	void Resolve();

	// Initialises the attributes of the particles a_iStart to a_iEnd of a_pEmitter
	void InitParticles( AParticleGenericEmitter* a_pEmitter, TINT a_iStart, TINT a_iEnd, TBOOL a_bInitLife );

	// The emitter being initialised (0x007cd978)
	inline static AParticleGenericEmitter* ms_pInitEmitter;

	TUINT32            uiUnk;                         // 0x0
	const TCHAR*       szMaterialName;                // 0x4
	const TCHAR*       szModelName;                   // 0x8
	TINT               iMaxParticles;                 // 0xc
	TUINT32            uiUnk3;                        // 0x10
	TFLOAT             fLODDistance;                  // 0x14
	TINT16             iMinParticles;                 // 0x18
	TBOOL              bComputeCentre;                // 0x1a
	TBOOL              bComputeRadius;                // 0x1b
	TFLOAT             fRadius;                       // 0x1c
	TINT               iMaterial;                     // 0x20
	TINT               iModel;                        // 0x24
	TFLOAT             fLightScale;                   // 0x28
	TUINT32            uiUnk4;                        // 0x2c
	TUINT32            uiUnk5;                        // 0x30
	FLAGS              uiFlags;                       // 0x34
	TUINT32            uiAttributes;                  // 0x38
	RENDERTYPE         eRenderType;                   // 0x3c
	AParticleAffector* pAffectors;                    // 0x40
	AParticleInit*     apInits[ INIT_NUMOF ];         // 0x44
	UVData*            apUVs[ 2 ];                    // 0x60
	RotationData*      pRotationData;                 // 0x68
	SparkData*         pSparkData;                    // 0x6c
	StretchData*       pStretchData;                  // 0x70
	AttachData*        pAttachData;                   // 0x74
	AParticleTimeline* apTimelines[ TIMELINE_NUMOF ]; // 0x78
};

TSTATICASSERT( sizeof( AParticleEmitterSettings::RotationData ) == 0x50 );
TSTATICASSERT( sizeof( AParticleEmitterSettings::SparkData ) == 0xf4 );
TSTATICASSERT( sizeof( AParticleEmitterSettings ) == 0x94 );

// An emitter of an effect (0x10 bytes)
struct AParticleEffectEmitter
{
	using FLAGS = TUINT32;
	enum FLAGS_ : FLAGS
	{
		FLAGS_NONE         = 0,
		FLAGS_FOLLOW_POS   = BITFLAG( 0 ), // The particles move with the effect
		FLAGS_FOLLOW_TRANS = BITFLAG( 1 ), // The particles move and rotate with the effect
	};

	Toshi::TMatrix44*         pOffset;   // 0x0, transform relative to the effect
	AParticleEmitterSettings* pSettings; // 0x4
	AParticleTimeline*        pTimeline; // 0x8
	FLAGS                     uiFlags;   // 0xc

	AParticleEffectEmitter( const AParticleEffectEmitter& a_rcEmitter );
	~AParticleEffectEmitter();

	// Resolves the emitter timeline and makes it spawn the emitters of a_pSettings
	void Resolve();
};

// An effect of the library (0x10 bytes)
struct AParticleEffect
{
	const TCHAR*            szName;       // 0x0
	TINT16                  iNumEmitters; // 0x4
	TINT16                  iStealable;   // 0x6, the instance can be stopped to play other effects
	AParticleEffectEmitter* pEmitters;    // 0x8
	TUINT32                 uiUnk;        // 0xc

	// Copies the effect (it can be changed without affecting the library)
	AParticleEffect* Clone() const;

	// Resolves the emitters
	void Resolve();

	// Deletes the clone of an effect
	void DeleteClone();
};

// The "EffectManager" symbol of the particle library
struct AParticleEffectLibrary
{
	TINT                       iNumEffects;  // 0x0
	AParticleEffect**          ppEffects;    // 0x4
	TINT                       iNumSettings; // 0x8
	AParticleEmitterSettings** ppSettings;   // 0xc
};

// A material of the particle library (0x1c bytes)
struct AParticleMaterial
{
	using BLENDMODE = TINT;
	enum BLENDMODE_ : BLENDMODE
	{
		BLENDMODE_NORMAL,
		BLENDMODE_ADDITIVE,
		BLENDMODE_SUBTRACTIVE,
		BLENDMODE_MULTIPLY,
	};

	const TCHAR*     szName;        // 0x0
	const TCHAR*     szTextureName; // 0x4
	Toshi::TTexture* pTexture;      // 0x8
	ASysMaterial*    pMaterial;     // 0xc
	TUINT32          uiUnk;         // 0x10
	Toshi::TVector4* pUVs;          // 0x14, ( u1, v1, u2, v2 ) of every frame
	BLENDMODE        eBlendMode;    // 0x18

	void Create();
	void Destroy();
};

// The "MaterialManager" symbol of the particle library
struct AParticleMaterialLibrary
{
	TINT               iNumMaterials; // 0x0
	AParticleMaterial* pMaterials;    // 0x4

	void CreateMaterials();
	void DestroyMaterials();

	// Returns index of the material or 0 if there's no such material
	TINT FindMaterial( const TCHAR* a_szName ) const;
};

// A model of the particle library (0xc bytes)
struct AParticleModel
{
	const TCHAR*          szName;        // 0x0
	Toshi::TManagedModel* pManagedModel; // 0x4
	Toshi::TSceneObject*  pSceneObject;  // 0x8

	void Load( const TCHAR* a_szDirectory );
};

// The "ModelManager" symbol of the particle library
struct AParticleModelLibrary
{
	TINT                     iNumModels; // 0x0
	AParticleModel*          pModels;    // 0x4
	const TCHAR*             szKeylib;   // 0x8
	Toshi::TKeyframeLibrary* pKeylib;    // 0xc

	void Load( const TCHAR* a_szDirectory );
	void Unload();

	// Returns index of the model or -1 if there's no such model
	TINT FindModel( const TCHAR* a_szName ) const;
};

//-----------------------------------------------------------------------------
// Rendering
//-----------------------------------------------------------------------------

// Vertices and indices of the mesh a material renders into (0x40 bytes)
struct AParticleMeshLock
{
	ASysMesh*            pMesh;         // 0x0
	TUINT16              uiNumVertices; // 0x4
	TUINT16              uiNumIndices;  // 0x6
	ASysMesh::LockBuffer oLockBuffer;   // 0x8
	TUINT16*             pIndices;      // 0x38
	ASysMesh::Vertex*    pVertices;     // 0x3c

	AParticleMeshLock();

	// Locks a_pMesh to fill it with the particles of a_pMaterial
	void Begin( ASysMesh* a_pMesh, ASysMaterial* a_pMaterial );

	// Moves the write pointers after a_uiNumVertices vertices and a_uiNumIndices indices, renders the mesh when it's full
	void Advance( TUINT a_uiNumVertices, TUINT a_uiNumIndices );

	// Renders what's in the mesh
	void Flush();
};

TSTATICASSERT( sizeof( AParticleMeshLock ) == 0x40 );

// Fills the vertices of the particle a_iParticle (positions, UVs or colours)
using AParticleVertexFunc = void( __stdcall* )( AParticleMeshLock* a_pLock, TINT a_iVertex, TINT a_iParticle, AParticleEmitter* a_pEmitter );

// Fills the indices of a_iNumParticles particles, returns the number of indices
using AParticleIndexFunc = TINT( __stdcall* )( AParticleMeshLock* a_pLock, TINT a_iNumParticles, AParticleEmitter* a_pEmitter );

// Returns size of the particle a_iParticle
using AParticleSizeFunc = TFLOAT( __stdcall* )( AParticleEmitter* a_pEmitter, TINT a_iParticle );

//-----------------------------------------------------------------------------
// Emitters
//-----------------------------------------------------------------------------

// vtable 0x0073b97c
class AParticleEmitter
{
public:
	AParticleEmitter();
	virtual ~AParticleEmitter();                                // +0x0
	virtual void  Start() = 0;                                  // +0x4
	virtual void  Release();                                    // +0x8
	virtual TINT  GetNumVertices( TINT a_iNumParticles );       // +0xc
	virtual TINT  GetMaxParticles() { return m_iMaxParticles; } // +0x10, shared getter (006d6760)
	virtual void  SetNumParticles( TINT a_iNumParticles );      // +0x14
	virtual void  SetMaxParticles( TUINT16 a_uiMaxParticles );  // +0x18
	virtual TBOOL IsVisible();                                  // +0x1c
	virtual TBOOL Update( TFLOAT a_fDeltaTime ) = 0;            // +0x20
	virtual void  Render()                      = 0;            // +0x24

	TINT GetNumParticles() const { return m_iNumParticles; }
	TINT GetMaterial() const { return m_iMaterial; }

protected:
	// Renders the particles as quads
	void RenderQuads( AParticleVertexFunc a_fnPositions, AParticleVertexFunc a_fnUVs, AParticleVertexFunc a_fnColours, AParticleIndexFunc a_fnIndices, AParticleEmitter* a_pEmitter );

	// Writes the indices of a_iNumParticles quads
	static TINT __stdcall WriteQuadIndices( AParticleMeshLock* a_pLock, TINT a_iNumParticles, AParticleEmitter* a_pEmitter );

public:
	Toshi::TVector4 m_vecBoundingSphere; // 0x4
	TINT            m_iMaxParticles;     // 0x14
	TINT            m_iNumParticles;     // 0x18
	TINT            m_iMaterial;         // 0x1c
	TFLOAT          m_fLODScale;         // 0x20, 1 / LOD distance ^ 2
	TINT            m_iMinParticles;     // 0x24
};

// An emitter that shows a single quad for 3 seconds (0x2c bytes)
// NOTE: nothing in the game uses it
// vtable 0x0073b9dc
class AParticleTestEmitter
    : public AParticleEmitter
{
public:
	AParticleTestEmitter();
	~AParticleTestEmitter();

	//-----------------------------------------------------------------------------
	// AParticleEmitter
	//-----------------------------------------------------------------------------
	virtual void  Start() OVERRIDE;
	virtual void  Release() OVERRIDE;
	virtual TINT  GetNumVertices( TINT a_iNumParticles ) OVERRIDE;
	virtual TBOOL Update( TFLOAT a_fDeltaTime ) OVERRIDE;
	virtual void  Render() OVERRIDE;

private:
	static void __stdcall SetPositions( AParticleMeshLock* a_pLock, TINT a_iVertex, TINT a_iParticle, AParticleEmitter* a_pEmitter );
	static void __stdcall SetUVs( AParticleMeshLock* a_pLock, TINT a_iVertex, TINT a_iParticle, AParticleEmitter* a_pEmitter );
	static void __stdcall SetColours( AParticleMeshLock* a_pLock, TINT a_iVertex, TINT a_iParticle, AParticleEmitter* a_pEmitter );

private:
	TFLOAT m_fTime; // 0x28
};

TSTATICASSERT( sizeof( AParticleTestEmitter ) == 0x2c );

// The emitter of the particle effects (0x210 bytes)
// vtable 0x0073b8bc
class AParticleGenericEmitter
    : public AParticleEmitter
{
public:
	friend AParticleEffectInstance;

	using FLAGS = TUINT32;
	enum FLAGS_ : FLAGS
	{
		FLAGS_NONE         = 0,
		FLAGS_POSITION     = BITFLAG( 0 ),
		FLAGS_VELOCITY     = BITFLAG( 1 ),
		FLAGS_LIFE         = BITFLAG( 4 ),
		FLAGS_UNK5         = BITFLAG( 5 ), // set by AParticleEmitterSettings::Resolve, no attribute array
		FLAGS_MASS         = BITFLAG( 6 ),
		FLAGS_SIZE         = BITFLAG( 7 ),
		FLAGS_UV           = BITFLAG( 8 ),
		FLAGS_COLOUR       = BITFLAG( 9 ),
		FLAGS_START_AXIS   = BITFLAG( 10 ),
		FLAGS_KEEP_ALIVE   = BITFLAG( 28 ), // Owned by a playing effect
		FLAGS_FOLLOW_POS   = BITFLAG( 29 ),
		FLAGS_FOLLOW_TRANS = BITFLAG( 30 ),
		FLAGS_SPAWN_NEW    = BITFLAG( 31 ), // Respawns the particles that die
		FLAGS_ATTRIBUTES   = 0x0fffffff,
	};

	using t_OnFinished = void( __stdcall* )( AParticleGenericEmitter* a_pEmitter, void* a_pUserData );

public:
	AParticleGenericEmitter();
	~AParticleGenericEmitter();

	//-----------------------------------------------------------------------------
	// AParticleEmitter
	//-----------------------------------------------------------------------------
	virtual void  Start() OVERRIDE;
	virtual void  Release() OVERRIDE;
	virtual TINT  GetNumVertices( TINT a_iNumParticles ) OVERRIDE;
	virtual void  SetNumParticles( TINT a_iNumParticles ) OVERRIDE;
	virtual void  SetMaxParticles( TUINT16 a_uiMaxParticles ) OVERRIDE;
	virtual TBOOL Update( TFLOAT a_fDeltaTime ) OVERRIDE;
	virtual void  Render() OVERRIDE;

	// Uses settings of the emitter
	void Setup( AParticleEmitterSettings* a_pSettings );

	// Sets the transform the particles are emitted from
	void SetTransform( const Toshi::TMatrix44& a_rcTransform );

	// Spawns the particles a_iStart to a_iEnd, a_fDeltaTime is the time they could already live
	void SpawnParticles( TINT a_iStart, TINT a_iEnd, TFLOAT a_fDeltaTime, TBOOL a_bInitLife );

	AParticleEmitterSettings* GetSettings() const { return m_pSettings; }

	FLAGS GetFlags() const { return m_eFlags; }
	void  SetFlags( FLAGS a_eFlags ) { m_eFlags = a_eFlags; }

private:
	// Initialises the attributes of the particles a_iStart to a_iEnd
	void InitParticles( TINT a_iStart, TINT a_iEnd, TBOOL a_bInitLife );

	// Picks random rotation speeds for the particles
	void RandomiseRotations();

	// Updates the corners of the rotated sprites
	void UpdateRotations( TFLOAT a_fDeltaTime );

	// Updates the colour of the light (multiplies the colours of the particles)
	void UpdateLighting();

	// Renders the particles as a strip of quads, ordered by age
	void RenderRibbon( AParticleVertexFunc a_fnPositions, AParticleVertexFunc a_fnUVs, AParticleVertexFunc a_fnColours, AParticleIndexFunc a_fnIndices, AParticleEmitter* a_pEmitter );

	// Renders the model of the settings at every particle
	void RenderMeshes();

	static void __stdcall SetRibbonPositions( AParticleMeshLock* a_pLock, TINT a_iVertex, TINT a_iParticle, AParticleEmitter* a_pEmitter, const Toshi::TVector4& a_rcSide );
	static TINT __stdcall WriteRibbonIndices( AParticleMeshLock* a_pLock, TINT a_iNumParticles, AParticleEmitter* a_pEmitter );

	// Vertex functions
	static void __stdcall SetSpritePositions( AParticleMeshLock* a_pLock, TINT a_iVertex, TINT a_iParticle, AParticleEmitter* a_pEmitter );
	static void __stdcall SetRotatedSpritePositions( AParticleMeshLock* a_pLock, TINT a_iVertex, TINT a_iParticle, AParticleEmitter* a_pEmitter );
	static void __stdcall SetStretchedPositions( AParticleMeshLock* a_pLock, TINT a_iVertex, TINT a_iParticle, AParticleEmitter* a_pEmitter );
	static void __stdcall SetRibbonUVs( AParticleMeshLock* a_pLock, TINT a_iVertex, TINT a_iParticle, AParticleEmitter* a_pEmitter );
	static void __stdcall SetFrameUVs( AParticleMeshLock* a_pLock, TINT a_iVertex, TINT a_iParticle, AParticleEmitter* a_pEmitter );
	static void __stdcall SetStretchedFrameUVs( AParticleMeshLock* a_pLock, TINT a_iVertex, TINT a_iParticle, AParticleEmitter* a_pEmitter );
	static void __stdcall SetRibbonFrameUVs( AParticleMeshLock* a_pLock, TINT a_iVertex, TINT a_iParticle, AParticleEmitter* a_pEmitter );
	static void __stdcall SetColours( AParticleMeshLock* a_pLock, TINT a_iVertex, TINT a_iParticle, AParticleEmitter* a_pEmitter );
	static void __stdcall SetKeyframeColours( AParticleMeshLock* a_pLock, TINT a_iVertex, TINT a_iParticle, AParticleEmitter* a_pEmitter );
	static void __stdcall SetLitColours( AParticleMeshLock* a_pLock, TINT a_iVertex, TINT a_iParticle, AParticleEmitter* a_pEmitter );
	static void __stdcall SetLitKeyframeColours( AParticleMeshLock* a_pLock, TINT a_iVertex, TINT a_iParticle, AParticleEmitter* a_pEmitter );
	static TFLOAT __stdcall GetSize( AParticleEmitter* a_pEmitter, TINT a_iParticle );
	static TFLOAT __stdcall GetKeyframeSize( AParticleEmitter* a_pEmitter, TINT a_iParticle );

public:
	AParticleVectorAttribute  m_aPositions;           // 0x28
	AParticleVectorAttribute  m_aVelocities;          // 0x38
	AParticleVectorAttribute  m_aStartAxes;           // 0x48
	AParticleFloatAttribute   m_aAges;                // 0x58, 0-1
	AParticleFloatAttribute   m_aAgeRates;            // 0x68, 1 / life
	AParticleFloatAttribute   m_aMasses;              // 0x78
	AParticleFloatAttribute   m_aSizes;               // 0x88
	AParticleDwordAttribute   m_aUVFrames;            // 0x98
	AParticleDwordAttribute   m_aColours;             // 0xa8
	AParticleDwordAttribute   m_aColourKeys;          // 0xb8, AParticleKeyframe* blending to the colour
	AParticleDwordAttribute   m_aSizeKeys;            // 0xc8, AParticleKeyframe* blending to the size
	AParticleByteAttribute    m_aEventKeys;           // 0xd8, index of the next key of the events timeline
	Toshi::TVector4           m_vecPrevPosition;      // 0xe8
	Toshi::TQuaternion        m_quatPrevRotation;     // 0xf8
	Toshi::TMatrix44          m_mTransform;           // 0x108
	t_OnFinished              m_fnOnFinished;         // 0x148
	void*                     m_pOnFinishedData;      // 0x14c
	TFLOAT                    m_afRotations[ 16 ];    // 0x150
	AParticleVertexFunc       m_fnSetPositions;       // 0x190
	AParticleVertexFunc       m_fnSetUVs;             // 0x194
	AParticleVertexFunc       m_fnSetColours;         // 0x198
	AParticleIndexFunc        m_fnWriteIndices;       // 0x19c
	AParticleSizeFunc         m_fnGetSize;            // 0x1a0
	FLAGS                     m_eFlags;               // 0x1a4
	TFLOAT                    m_fTime;                // 0x1a8
	AParticleEmitterSettings* m_pSettings;            // 0x1ac
	AParticleTimelineState    m_aTimelineStates[ 4 ]; // 0x1b0
	TFLOAT                    m_afUnk[ 8 ];           // 0x1f0, set by Render and never used
};

TSTATICASSERT( sizeof( AParticleGenericEmitter ) == 0x210 );

//-----------------------------------------------------------------------------
// Effects
//-----------------------------------------------------------------------------

// A playing effect (0x144 bytes)
class AParticleEffectInstance
{
public:
	using t_OnFinished = void( __stdcall* )( AParticleEffectInstance* a_pInstance, void* a_pUserData );

	using EMITTERFLAGS = TUINT32;
	enum EMITTERFLAGS_ : EMITTERFLAGS
	{
		EMITTERFLAGS_NONE     = 0,
		EMITTERFLAGS_FINISHED = BITFLAG( 0 ),
		EMITTERFLAGS_STOPPING = BITFLAG( 1 ),
	};

	struct Emitter
	{
		AParticleTimelineState   oTimeline; // 0x0
		AParticleGenericEmitter* pEmitter;  // 0x10
		EMITTERFLAGS             eFlags;    // 0x14
	};

	inline static constexpr TINT MAX_NUM_EMITTERS = 10;

public:
	AParticleEffectInstance();

	// Starts playing a_pEffect
	void Start( AParticleEffect* a_pEffect );

	// Updates the emitters, returns TFALSE if the effect has finished
	TBOOL Update( TFLOAT a_fDeltaTime );

	// Stops spawning new particles, the effect finishes when the particles die
	void Stop();

	// Kills the particles and finishes the effect
	void Kill();

	// Stops looping the timelines of the emitters
	void StopLooping();

	Toshi::TMatrix44& GetTransform() { return m_mTransform; }
	AParticleEffect*  GetEffect() const { return m_pEffect; }

	void SetOnFinishedCallback( t_OnFinished a_fnCallback, void* a_pUserData )
	{
		m_fnOnFinished    = a_fnCallback;
		m_pOnFinishedData = a_pUserData;
	}

	// Called when an emitter of the instance has finished
	static void __stdcall OnEmitterFinished( AParticleGenericEmitter* a_pEmitter, void* a_pUserData );

public:
	inline static AParticleEffectInstance* ms_pCurrentInstance;      // 0x007cd934, the instance being updated
	inline static AParticleEffectEmitter*  ms_pCurrentEffectEmitter; // 0x007cd930, the emitter being updated
	inline static Toshi::TMatrix44         ms_mEmitterTransform;     // 0x007cd938, transform of the emitter being updated

public:
	Toshi::TMatrix44         m_mTransform;                    // 0x0
	AParticleEffect*         m_pEffect;                       // 0x40
	t_OnFinished             m_fnOnFinished;                  // 0x44
	void*                    m_pOnFinishedData;               // 0x48
	Emitter                  m_aEmitters[ MAX_NUM_EMITTERS ]; // 0x4c
	AParticleEffectInstance* m_pNext;                         // 0x13c
	AParticleEffectInstance* m_pPrev;                         // 0x140
};

TSTATICASSERT( sizeof( AParticleEffectInstance ) == 0x144 );

// The effects of the library and the instances playing them (0x1c bytes)
class AParticleEffectPool
{
public:
	AParticleEffectPool( TINT a_iMaxInstances );
	~AParticleEffectPool();

	// Uses the effects of the library loaded by a_pSystem
	void Load( AParticleSystem* a_pSystem );
	void Unload();

	// Returns a new instance playing a_pEffect or TNULL if there are no free instances
	AParticleEffectInstance* CreateInstance( AParticleEffect* a_pEffect );

	// Puts a_pInstance back to the free list
	void ReleaseInstance( AParticleEffectInstance* a_pInstance );

	// Updates the instances and releases the finished ones
	void Update( TFLOAT a_fDeltaTime );

	// Kills every instance
	void KillAll();

	// Kills the instances that play a_pEffect
	void KillEffect( AParticleEffect* a_pEffect );

	// Kills the first instance that plays a stealable effect to free its emitters
	void StealInstance();

	// Returns index of the effect (case insensitive) or -1 if there's no such effect
	TINT FindEffect( const TCHAR* a_szName ) const;

	// Returns the effect a_iEffect or TNULL if there's no such effect
	AParticleEffect* GetEffect( TINT a_iEffect ) const;

	TINT GetMaxInstances() const { return m_iMaxInstances; }

private:
	TINT                     m_iMaxInstances;  // 0x0
	AParticleEffectLibrary*  m_pLibrary;       // 0x4
	AParticleEffectInstance* m_pInstances;     // 0x8
	AParticleEffectInstance* m_pFreeInstances; // 0xc
	AParticleEffectInstance* m_pUsedHead;      // 0x10
	AParticleEffectInstance* m_pUsedTail;      // 0x14
	TBOOL                    m_bHasStealable;  // 0x18
};

TSTATICASSERT( sizeof( AParticleEffectPool ) == 0x1c );

//-----------------------------------------------------------------------------
// The system
//-----------------------------------------------------------------------------

// Sizes of the attribute pools (in elements) and the max number of instances
struct AParticleSystemConfig
{
	TINT iNumFloats;    // 0x0
	TINT iNumVectors;   // 0x4
	TINT iNumVectors2;  // 0x8
	TINT iNumBytes;     // 0xc
	TINT iNumDwords;    // 0x10
	TINT iNumInstances; // 0x14

	AParticleSystemConfig();
};

// The particle library and every emitter (0x2b8 bytes)
class AParticleSystem
    : public Toshi::TTRB
{
public:
	using EMITTERTYPE = TINT;
	enum EMITTERTYPE_ : EMITTERTYPE
	{
		EMITTERTYPE_TEST,
		EMITTERTYPE_GENERIC,
		EMITTERTYPE_NUMOF
	};

	using DETAIL = TINT;
	enum DETAIL_ : DETAIL
	{
		DETAIL_NONE = -1, // not set yet
		DETAIL_HIGH = 2,
	};

	struct MeshNode
	    : Toshi::TNodeList<MeshNode>::TNode
	{
		ASysMesh* pMesh = TNULL; // 0xc

		~MeshNode();
	};

	struct EmitterNode
	    : Toshi::TNodeList<EmitterNode>::TNode
	{
		AParticleEmitter* pEmitter = TNULL;             // 0xc
		EMITTERTYPE       eType    = EMITTERTYPE_NUMOF; // 0x10

		// $Barnyard: FUNCTION 00678150
		EmitterNode() {}
		~EmitterNode();
	};

	using t_MaterialLibraryCallback = void( __stdcall* )( AMaterialLibrary* a_pLibrary );

public:
	AParticleSystem( const AParticleSystemConfig& a_rcConfig );
	~AParticleSystem();

	// Loads a_szName.ttl (textures) and a_szName.trb (effects)
	void LoadLibrary( const TCHAR* a_szName );
	void UnloadLibrary();

	// Updates the emitters and the effects
	void Update( TFLOAT a_fDeltaTime );

	// Renders the visible emitters
	void Render();

	// Takes a free emitter of the type a_eType, returns TNULL if there are no free emitters
	AParticleEmitter* CreateEmitter( EMITTERTYPE a_eType );

	// Puts a_pEmitter back to the free emitters
	void ReleaseEmitter( AParticleEmitter* a_pEmitter );

	// Releases the emitters that use a_pSettings
	void ReleaseEmittersOfSettings( AParticleEmitterSettings* a_pSettings );

	// Kills every effect and releases every emitter
	void KillAll();

	// Returns the mesh lock of a_iMaterial that has room for a_uiNumVertices or TNULL if there's no free mesh
	AParticleMeshLock* GetMeshLock( TUINT a_uiMaxVertices, TUINT a_uiNumVertices, TINT a_iMaterial );

	// Returns TFALSE if the sphere is outside of the frustum of the last render
	static TBOOL IsSphereVisible( const Toshi::TVector4& a_rcCentre, TFLOAT a_fRadius );

	// Returns the current model view matrix
	const Toshi::TMatrix44& GetModelView();

	// Flushes the meshes and sets the model view matrix
	void SetModelView( const Toshi::TMatrix44& a_rcModelView );

	AParticleEffectPool*      GetEffectPool() const { return m_pEffectPool; }
	AParticleMaterialLibrary* GetMaterialLibrary() const { return m_pMaterials; }
	AParticleModelLibrary*    GetModelLibrary() const { return m_pModels; }
	TBOOL                     IsUpdated() const { return m_bUpdated; }
	const Toshi::TPlane*      GetFrustumPlanes() const { return m_aFrustumPlanes; }
	TINT                      GetNumFrustumPlanes() const { return m_iNumFrustumPlanes; }

	void SetMaterialLibraryCallbacks( t_MaterialLibraryCallback a_fnOnLoaded, t_MaterialLibraryCallback a_fnOnUnloaded )
	{
		m_fnOnMaterialLibraryLoaded   = a_fnOnLoaded;
		m_fnOnMaterialLibraryUnloaded = a_fnOnUnloaded;
	}

	static AParticleSystem* GetSingleton() { return ms_pSingleton; }
	static AParticleSystem* GetRenderSystem() { return ms_pRenderSystem; }

	// The billboard corners of the current model view matrix
	static const Toshi::TVector4* GetQuadCorners() { return ms_pRenderSystem->m_aQuadCorners; }

private:
	void CreatePools( const AParticleSystemConfig& a_rcConfig );
	void CreateEmitters();
	void DestroyEmitters();

	// Updates the billboard axes and corners from the model view matrix
	void UpdateQuadCorners();

private:
	inline static AParticleSystem* ms_pSingleton;    // 0x007cdedc
	inline static AParticleSystem* ms_pRenderSystem; // 0x007cde40

private:
	AParticleEffectPool*          m_pEffectPool;                        // 0x140
	AParticleMaterialLibrary*     m_pMaterials;                         // 0x144
	AParticleModelLibrary*        m_pModels;                            // 0x148
	AMaterialLibrary*             m_pMaterialLibrary;                   // 0x14c
	Toshi::TNodeList<MeshNode>    m_llUsedMeshes;                       // 0x150, the meshes rendered this frame
	Toshi::TNodeList<MeshNode>    m_llFreeMeshes;                       // 0x160
	Toshi::TNodeList<EmitterNode> m_aFreeEmitters[ EMITTERTYPE_NUMOF ]; // 0x170
	Toshi::TNodeList<EmitterNode> m_llActiveEmitters;                   // 0x190
	AParticleTestEmitter*         m_pTestEmitters;                      // 0x1a0
	AParticleGenericEmitter*      m_pGenericEmitters;                   // 0x1a4
	EmitterNode*                  m_pEmitterNodes;                      // 0x1a8
	Toshi::TPlane                 m_aFrustumPlanes[ 6 ];                // 0x1ac
	TINT                          m_iNumFrustumPlanes;                  // 0x20c
	Toshi::TVector4               m_aQuadCorners[ 4 ];                  // 0x210
	Toshi::TVector4               m_vecRight;                           // 0x250
	Toshi::TVector4               m_vecUp;                              // 0x260
	Toshi::TVector4               m_vecForward;                         // 0x270
	AParticleMeshLock*            m_pMeshLocks;                         // 0x280
	TINT*                         m_piNumParticles;                     // 0x284
	TINT*                         m_piNumVertices;                      // 0x288
	void*                         m_pModelData;                         // 0x28c
	void*                         m_pUnk;                               // 0x290
	DETAIL                        m_eDetail;                            // 0x294
	TFLOAT                        m_fLODStart;                          // 0x298
	TUINT                         m_uiMaxVertices;                      // 0x29c
	TINT                          m_iNumMeshes;                         // 0x2a0
	TINT                          m_aNumEmitters[ EMITTERTYPE_NUMOF ];  // 0x2a4
	TBOOL                         m_bUpdated;                           // 0x2ac
	t_MaterialLibraryCallback     m_fnOnMaterialLibraryLoaded;          // 0x2b0
	t_MaterialLibraryCallback     m_fnOnMaterialLibraryUnloaded;        // 0x2b4
};
