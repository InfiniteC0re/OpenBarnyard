#pragma once
#include <File/TTRB.h>
#include <Toshi/TRefCounted.h>
#include <Render/TModelManager.h>
#include <Toshi/TPString8.h>

#include <Toshi/T2DList.h>
#include <Toshi/T2Vector.h>
#include <Toshi/TSceneObject.h>

class AModel;

// Rotates a_rcVector by a_rcQuaternion (w of the result is 1)
void RotateVector( Toshi::TVector4& a_rOutVector, const Toshi::TQuaternion& a_rcQuaternion, const Toshi::TVector4& a_rcVector );

class AModelInstance : public Toshi::TObject
    , public Toshi::TRefCounted
{
public:
	TDECLARE_CLASS( AModelInstance, Toshi::TObject );

public:
	using Flags = TUINT8;
	enum Flags_ : Flags
	{
		Flags_None                     = 0,
		Flags_UpdatingSkeleton         = BITFLAG( 0 ),
		Flags_Visible                  = BITFLAG( 1 ),
		Flags_DrawWithDefaultMaterials = BITFLAG( 2 ),
		Flags_ReceivesLight            = BITFLAG( 3 ),
		Flags_Glow                     = BITFLAG( 4 ), // set with the glow material flag (SetGlow), rendered by the glow pass
	};

	// How the instance is lit by AInstanceLightingHelper, stored in the upper bits of m_eFlags
	using SHADOWMODE = TUINT8;
	enum SHADOWMODE_ : SHADOWMODE
	{
		SHADOWMODE_DEFAULT = 0,            // UpdateLighting samples the shade and light values at the position
		SHADOWMODE_UNLIT   = BITFLAG( 5 ), // SetUnlit, uses the stored shade and light values
		SHADOWMODE_DYNAMIC = BITFLAG( 6 ), // sampled at the position every frame
		SHADOWMODE_SECTION = BITFLAG( 7 ), // sampled from the spheres of its lighting section
		SHADOWMODE_MASK    = SHADOWMODE_UNLIT | SHADOWMODE_DYNAMIC | SHADOWMODE_SECTION,
	};

	enum ChangeEvent : TUINT32
	{
		ChangeEvent_Visibility,
		ChangeEvent_UNK2,
		ChangeEvent_EnabledSkeletonUpdate,
		ChangeEvent_DisabledSkeletonUpdate,
	};

	Toshi::TEmitter<AModelInstance, ChangeEvent>& GetChangeEmitter() { return m_ChangeEmitter; }

	friend class AModel;

public:
	AModelInstance();
	AModelInstance( AModel* a_pModel, Toshi::TSceneObject* a_pSceneObject, TBOOL a_bEnableSkeletonUpdate );
	~AModelInstance();

	void SetTransform( const Toshi::TMatrix44& a_rcTransform );

	// Places the instance at a_rcPosition facing a_rcDirection (Y axis up)
	void SetTransform( const Toshi::TVector4& a_rcPosition, const Toshi::TVector4& a_rcDirection );

	// Same as SetTransform( pos, dir ), but uses a_rcFallback as the right vector when a_rcDirection is parallel to the up axis
	void SetTransform( const Toshi::TVector4& a_rcPosition, const Toshi::TVector4& a_rcDirection, const Toshi::TVector4& a_rcFallback );

	// Same as SetTransform( pos, dir ); the third vector is ignored
	void SetTransformUnusedUp( const Toshi::TVector4& a_rcPosition, const Toshi::TVector4& a_rcDirection, const Toshi::TVector4& a_rcUnused );

	// Same as SetTransform( pos, dir ), but keeps the current right vector when a_rcDirection is parallel to the up axis
	void SetTransformKeepRight( const Toshi::TVector4& a_rcPosition, const Toshi::TVector4& a_rcDirection );

	// Places the instance at a_rcPosition facing a_rcDirection, pitched by a_fPitch and rolled by a_fRoll (radians)
	void SetTransformTilted( const Toshi::TVector4& a_rcPosition, const Toshi::TVector4& a_rcDirection, TFLOAT a_fPitch, TFLOAT a_fRoll );

	void SetTranslation( const Toshi::TVector3& a_rcTranslation );

	// Makes the instance face a_rcDirection without moving it
	void SetDirection( const Toshi::TVector4& a_rcDirection );

	// Same as SetDirection, but uses a_rcFallback as the right vector when a_rcDirection is parallel to the up axis
	void SetDirection( const Toshi::TVector4& a_rcDirection, const Toshi::TVector4& a_rcFallback );

	// Returns TTRUE if the instance isn't facing a_rcDirection exactly
	TBOOL IsFacingDifferent( const Toshi::TVector4& a_rcDirection ) const;

	// Returns translation of the instance (w = 1)
	void GetPosition( Toshi::TVector4& a_rOutPosition ) const;

	// Builds a rotation matrix whose second basis vector is -a_rcDirection
	static void CreateRotationFromDirection( Toshi::TMatrix44& a_rOutMatrix, const Toshi::TVector4& a_rcDirection, const Toshi::TVector4& a_rcUp );

	// Same as above, but uses a_rcFallback as the up vector (and swaps the first two basis vectors) when a_rcDirection is parallel to a_rcUp
	static void CreateRotationFromDirection( Toshi::TMatrix44& a_rOutMatrix, const Toshi::TVector4& a_rcDirection, const Toshi::TVector4& a_rcUp, const Toshi::TVector4& a_rcFallback );
	void        SetSkeletonUpdating( TBOOL a_bUpdating );
	void        SetReceivesLight( TBOOL a_bReceives );
	void        SetVisible( TBOOL a_bVisible );

	// Sets the top three bits of the flags (the lighting isn't updated by UpdateLighting while any of them is set)
	void SetShadowMode( SHADOWMODE a_uiMode );

	// Sets shadow mode 0x20 and makes the skin materials of the model use only their texture (no lighting textures).
	// Used for models shown on the HUD and in the inventory.
	void SetUnlit();

	// Sets the a_szTextureName texture as the environment map of the ASkinMaterials of the model,
	// except the ones whose name contains one of the a_iNumExcluded names of a_ppExcluded.
	void SetEnvironmentMap( const TCHAR* a_szTextureName, TINT a_iNumExcluded, const TCHAR* const* a_ppExcluded );

	// Places the instance at a_rcPosition rotated by a_rcRotation
	void SetTransform( const Toshi::TVector4& a_rcPosition, const Toshi::TQuaternion& a_rcRotation );

	// Second basis vector of the local transform
	void GetUpVector( Toshi::TVector4& a_rOutUp ) const;

	// Updates the cached lighting of the instance at its position (unless the shadow mode is set)
	void UpdateLighting();

	// Transform of a_iBone in world space.
	void GetBoneTransform( TINT a_iBone, Toshi::TMatrix44& a_rOutTransform );

	// Transform of a_iBone in model space, translation scaled by m_Scale.
	void GetScaledBoneTransform( TINT a_iBone, Toshi::TMatrix44& a_rOutTransform );

	// NOTE: retail has two identical copies of this function (00610da0 and 00610de0)
	TINT GetBoneID( const Toshi::TPString8& a_rcBoneName );
	TINT GetBoneIndex( const Toshi::TPString8& a_rcBoneName );

	// NOTE: the original calls a DOES_NOTHING function (00628f40) when the bone isn't found
	// $Barnyard: FUNCTION 00610e40
	TINT GetBoneID( const TCHAR* a_szBoneName ) { return GetSkeleton()->GetBoneID( a_szBoneName, 0 ); }

	// $Barnyard: FUNCTION 006103c0
	TINT GetNumBones() const { return GetSkeleton()->GetBoneCount(); }

	const TCHAR* GetBoneName( TINT a_iBone );

	// Replaces the texture of every material named a_szMaterialName (and its skin light, if specified)
	void ApplyMaterial( const TCHAR* a_szTextureName, const TCHAR* a_szMaterialName, const TCHAR* a_szSkinLight );

	void SetOpacity( TFLOAT a_fOpacity );

	// Stops using the opacity set with SetOpacity
	void ResetOpacity();

	// $Barnyard: FUNCTION 00610730
	TFLOAT GetOpacity() const { return m_pSceneObject->GetFadeParams().y; }

	AModel*                  GetModel() const { return m_pModel; }
	const Toshi::TPString8&  GetModelName() const;
	Toshi::TSceneObject*     GetSceneObject() const { return m_pSceneObject; }
	Toshi::TTransformObject& GetTransform() const { return m_pSceneObject->GetTransform(); }
	// $Barnyard: FUNCTION 00610320
	void GetLocalTransform( Toshi::TMatrix44& a_rOutTransform ) const { GetTransform().GetLocalMatrixImp( a_rOutTransform ); }
	// $Barnyard: FUNCTION 006100a0
	Toshi::TSkeleton* GetSkeleton() const { return GetSceneObject()->GetInstance()->GetSkeletonInstance()->GetSkeleton(); }
	// $Barnyard: FUNCTION 006100b0
	Toshi::TSkeletonInstance* GetSkeletonInstance() const { return m_pSceneObject->GetInstance()->GetSkeletonInstance(); }
	TUINT                     GetClipFlags() const { return m_uiClipFlags; }
	void                      SetClipFlags( TUINT a_uiClipFlags ) { m_uiClipFlags = a_uiClipFlags; }

	const Toshi::TVector4& GetScale() const { return m_Scale; }
	void                   SetScale( const Toshi::TVector4& a_rcScale ) { m_Scale = a_rcScale; }

	TBOOL IsUpdatingSkeleton() const { return m_eFlags & Flags_UpdatingSkeleton; }
	TBOOL IsVisible() const { return m_eFlags & Flags_Visible; }
	TBOOL DrawWithDefaultMaterials() const { return m_eFlags & Flags_DrawWithDefaultMaterials; }
	TBOOL ReceivesLight() const { return m_eFlags & Flags_ReceivesLight; }

	TFLOAT     GetShadeValue() const { return m_fShadeValue; }
	TFLOAT     GetLightValue() const { return m_fLightValue; }
	TINT       GetLightingSection() const { return m_iLightingSection; }
	SHADOWMODE GetShadowMode() const { return m_eFlags & SHADOWMODE_MASK; }

	// $Barnyard: FUNCTION 00610780
	void DisableDrawWithDefaultMaterials() { m_eFlags &= ~Flags_DrawWithDefaultMaterials; }

	// Makes the instance render in the glow pass (see AAnimatableObject::SetGlow)
	// $Barnyard: FUNCTION 006103a0
	void SetGlow( TBOOL a_bGlow )
	{
		if ( a_bGlow )
			m_eFlags |= Flags_Glow;
		else
			m_eFlags &= ~Flags_Glow;
	}

public:
	static void RenderInstanceCallback( Toshi::TModelInstance* a_pInstance, void* a_pUserData );

private:
	Toshi::TVector4                              m_Scale;
	AModel*                                      m_pModel;
	TUINT                                        m_uiClipFlags;
	TFLOAT                                       m_fShadeValue;      // 0x20, shade coefficient of the sphereshade locators at the instance (day)
	TFLOAT                                       m_fLightValue;      // 0x24, shade coefficient of the spherelight locators at the instance (night)
	TINT                                         m_iLightingSection; // 0x28, terrain section whose spheres light the instance in shadow mode 0x80
	Toshi::TSceneObject*                         m_pSceneObject;
	Toshi::TEmitter<AModelInstance, ChangeEvent> m_ChangeEmitter;
	Flags                                        m_eFlags;
};

using AModelInstanceRef = Toshi::TRef<AModelInstance>;

class AModel : public Toshi::T2DList<AModel>::Node
{
public:
	static constexpr TUINT32 MAX_NUM_INSTANCES = 250;

public:
	AModel( const Toshi::TPString8& a_rName, Toshi::TTRB* a_pTRB );
	virtual ~AModel();

	void Update( TFLOAT a_fDeltaTime );
	void Render( TBOOL a_bGlow ); // renders visible instances with skeleton update enabled whose glow flag matches

	AModelInstanceRef CreateInstance();
	void              DestroyInstance( AModelInstance* a_pInstance );

	TSIZE                                                  GetNumInstances() const;
	Toshi::T2Vector<AModelInstanceRef, MAX_NUM_INSTANCES>& GetInstances() { return m_vecInstanceRefs; }

	const Toshi::TPString8& GetName() const { return m_Name; }
	Toshi::TSkeleton*       GetSkeleton() const { return m_pManagedModel->GetModel()->GetSkeleton(); }
	Toshi::TManagedModel*   GetManagedModel() const { return m_pManagedModel; }

public:
	static Toshi::TManagedModel* Create( const Toshi::TPString8& a_rFilePath, Toshi::TTRB* a_pTRB );
	static void                  GetNameFromPath( const Toshi::TPString8& a_FilePath, Toshi::TString8& a_rName );
	static Toshi::TString8       GenerateInstanceName( const Toshi::TPString8& a_FilePath );

private:
	inline static TUINT ms_uiNumCreated;

private:
	TUINT                                                 m_uiID;
	Toshi::TManagedModel*                                 m_pManagedModel;
	Toshi::TPString8                                      m_Name;
	Toshi::T2Vector<AModelInstanceRef, MAX_NUM_INSTANCES> m_vecInstanceRefs;
	Toshi::TVector3                                       m_Vec1;
	Toshi::TVector3                                       m_Vec2;
};
