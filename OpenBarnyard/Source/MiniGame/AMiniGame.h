#pragma once
#include <Toshi/TPString8.h>

class AMiniGame
{
public:
	struct Variant
	{
		TINT eLocaleTitle;
		TINT eLocaleHint;
		TINT eLocaleDescription;
	};

	// Assets a minigame needs, loaded by AGameLoader's LoadMiniGameImpl
	using LOADFLAGS = TUINT32;
	enum LOADFLAGS_ : LOADFLAGS
	{
		LOADFLAGS_NONE     = 0,
		LOADFLAGS_GATES    = BITFLAG( 0 ), // creates AGateManager
		LOADFLAGS_REGROWTH = BITFLAG( 1 ), // activates ARegrowthManager and loads its models
	};

	static constexpr TINT MAX_NUM_VARIANTS = 12;
	friend class AMiniGameManager;
	friend class AFrontEndMiniGameState2;
	friend class AFrontEndVariantSelectState;

public:
	AMiniGame();
	AMiniGame( const AMiniGame& a_rcOther );
	~AMiniGame();

	const Variant& GetVariant( TINT a_iIndex );
	AMiniGame&     AddVariant( TINT a_eLocaleTitle, TINT a_eLocaleHint, TINT a_eLocaleDescription, const TCHAR* a_szUnlockStateName = TNULL );

	TBOOL IsPlaceholder() const { return m_iDefaultVariant == -1; }

	const Toshi::TPString8& GetName() const { return m_strMiniGameName; }
	const Toshi::TClass*    GetGameStateClass() const { return m_pGameStateClass; }
	TINT                    GetInstancesType() const { return m_eInstancesType; }
	TINT                    GetMinNumPlayers() const { return m_iMinNumPlayers; }
	TINT                    GetMaxNumPlayers() const { return m_iMaxNumPlayers; }
	TINT                    GetDefaultVariant() const { return m_iDefaultVariant; }
	TINT                    GetNumSelectableVariants() const { return m_iNumSelectableVariants; }
	TINT                    GetLocaleTitle() const { return m_eLocaleTitle; }
	const TCHAR*            GetIconName() const { return m_szIconName; }

	LOADFLAGS GetLoadFlags() const { return m_eLoadFlags; }

	// Picks the AStartMiniGameHudElement type in AMiniGameState::OnInsertion
	TINT GetStartHudKind() const { return m_eStartHudKind; }

	// Sharp Squirter: 2 and 3 move the variant to the Woods ones (APrecisionSquirt::OnInsertion)
	TINT GetUnk18() const { return field6_0x18; }

private:
	Toshi::TPString8        m_strMiniGameName;
	TINT                    m_eLocaleTitle;
	TINT                    m_eLocaleDescription;
	const Toshi::TClass*    m_pGameStateClass;
	TINT                    m_iMinNumPlayers;
	TINT                    m_iMaxNumPlayers;
	TINT                    field6_0x18;
	TBOOL                   field7_0x1c;
	const TCHAR*            m_szIconName;
	const Toshi::TPString8* m_pLoadingScreenName;
	TINT                    m_eInstancesType;
	TINT                    m_iDefaultVariant;
	TINT                    m_iNumSelectableVariants;
	TINT                    m_iNumVariants;
	Variant                 m_aVariants[ MAX_NUM_VARIANTS ];
	TINT                    m_iNumStateNames;
	const TCHAR*            m_aszAlternativeStateNames[ MAX_NUM_VARIANTS ];
	TINT                    m_eStartHudKind;
	LOADFLAGS               m_eLoadFlags;
};
