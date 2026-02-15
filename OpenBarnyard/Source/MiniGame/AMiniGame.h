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

	static constexpr TINT MAX_NUM_VARIANTS = 12;
	friend class AMiniGameManager;
	friend class AFrontEndMiniGameState2;

public:
	AMiniGame();
	~AMiniGame();

	const Variant& GetVariant( TINT a_iIndex );
	AMiniGame&     AddVariant( TINT a_eLocaleTitle, TINT a_eLocaleHint, TINT a_eLocaleDescription );

	TBOOL IsPlaceholder() const { return m_iDefaultVariant == -1; }

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
	TINT                    field13_0x28;
	TINT                    m_iDefaultVariant;
	TINT                    m_iNumSelectableVariants;
	TINT                    m_iNumVariants;
	Variant                 m_aVariants[ MAX_NUM_VARIANTS ];
	TINT                    m_iNumUnknown1;
	TINT                    m_aUnknown[ MAX_NUM_VARIANTS ];
	TINT                    field20_0xfc;
	TINT                    field21_0x100;
};
