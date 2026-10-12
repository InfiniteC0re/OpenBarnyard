#pragma once
#ifdef BARNYARD_LATEST

#  include "XURData.h"
#  include "AGUI2XUIStringTable.h"

#  include <Math/TVector4.h>
#  include <Math/TQuaternion.h>
#  include <Toshi/TString16.h>
#  include <ToshiTools/T2DynamicVector.h>

class AGUI2XUIScene;
class AGUI2XUIVisual;

//-----------------------------------------------------------------------------
// A loaded XUR file (Data/XUI/<lang>/<name>.trb) and its optional string table.
// NOTE: there's no RTTI for this class, the name follows TXUIResource of newer TOSHI.
//-----------------------------------------------------------------------------
class AGUI2XUIResource
{
public:
	// Stack of the ids of the elements being created, used to build the string table path
	struct IDStack
	{
		static constexpr TINT MAX_SIZE = 32;

		// $BarnyardWiiJP: FUNCTION 80415048
		void Push( const TWCHAR* const& a_rwszId )
		{
			if ( ++m_iTop < MAX_SIZE )
				m_aIds[ m_iTop ] = a_rwszId;
		}

		// $BarnyardWiiJP: FUNCTION 80415070
		const TWCHAR*& Pop()
		{
			if ( --m_iTop < 0 )
				m_iTop = 0;

			return m_aIds[ m_iTop ];
		}

		// $BarnyardWiiJP: FUNCTION 8041509c
		const TWCHAR*& Top() { return m_aIds[ m_iTop ]; }

		TINT          m_iTop;             // 0x00
		const TWCHAR* m_aIds[ MAX_SIZE ]; // 0x04
	};

public:
	AGUI2XUIResource();
	~AGUI2XUIResource();

	TBOOL Load( const TCHAR* a_szFileName, const TCHAR* a_szStringTableFileName, TBOOL a_bIsTRB, Toshi::TTRB* a_pTRB );

	XURXUIElementData* FindScene( const TWCHAR* a_wszName );
	XURXUIElementData* FindScene( TUINT32 a_uiIndex );
	XURXUIElementData* FindVisual( const TWCHAR* a_wszName );

	AGUI2XUIScene*  CreateScene( TUINT32 a_uiIndex );
	AGUI2XUIScene*  CreateScene( const TWCHAR* a_wszName );
	AGUI2XUIVisual* CreateVisual( const TWCHAR* a_wszName );

	const Toshi::TVector4&    GetVector( TINT a_iIndex );
	const Toshi::TQuaternion& GetQuat( TINT a_iIndex );

	void          PushID( const TWCHAR* a_wszId );
	void          PopID();
	const TWCHAR* LookupStringTable();

	const TWCHAR* GetString( TUINT16 a_usIndex ) const { return m_apStrings[ a_usIndex ]; }
	TUINT8*       GetCustData() const { return m_pCustData; }

	static XURXUIObjectData* CreateObjectData( AGUI2XUIResource& a_rResource, TUINT16 a_usClassIndex );

private:
	TBOOL ReadHeader( TUINT8* a_pData );
	TBOOL ReadStringSection( TUINT8* a_pData, TUINT32 a_uiSize );
	TBOOL ReadVectorSection( TUINT8* a_pData, TUINT32 a_uiSize );
	TBOOL ReadQuatSection( TUINT8* a_pData, TUINT32 a_uiSize );
	TBOOL ReadCustSection( TUINT8* a_pData, TUINT32 a_uiSize );
	TBOOL ReadDataSection( TUINT8* a_pData );

private:
	// NOTE: these three vectors are created and freed but not used by any recovered code
	Toshi::T2DynamicVector<TWCHAR*>            m_vecUnk1;      // 0x00
	Toshi::T2DynamicVector<Toshi::TVector4>    m_vecUnk2;      // 0x14
	Toshi::T2DynamicVector<Toshi::TQuaternion> m_vecUnk3;      // 0x28
	XURHeader                                  m_oHeader;      // 0x3C
	TWCHAR**                                   m_apStrings;    // 0x58
	Toshi::TVector4*                           m_pVectors;     // 0x5C
	Toshi::TQuaternion*                        m_pQuats;       // 0x60
	TUINT8*                                    m_pCustData;    // 0x64
	XURXUIObjectData*                          m_pRoot;        // 0x68
	TINT                                       m_iNumStrings;  // 0x6C
	TUINT32                                    m_uiNumVectors; // 0x70
	TUINT32                                    m_uiNumQuats;   // 0x74
	TUINT32                                    m_uiCustSize;   // 0x78
	AGUI2XUIStringTable*                       m_pStringTable; // 0x7C
	Toshi::TString16                           m_strIDPath;    // 0x80
	IDStack                                    m_oIDStack;     // 0x8C
};

#endif // BARNYARD_LATEST
