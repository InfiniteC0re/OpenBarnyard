#include "pch.h"
#include "AGopherShopInterface.h"
#include "FSM/AFunctionPointer.h"
#include "FSM/AMethodPointer.h"

#include <Plugins/PPropertyParser/PPropertyValue.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 00499a10, the destructor is the shared TObject one
TDEFINE_CLASS_INIT( AGopherShopInterface );

// DAT_00782d48
AFunctionTable AGopherShopInterface::ms_oFunctionTable( &TGetClass( AGopherShopInterface ), TNULL );

AGopherShopInterface::JoeyOffer AGopherShopInterface::ms_aJoeyOffers[ NUM_JOEY_OFFERS ] = {
	{ 1742, 383 },
	{ 1743, 369 },
	{ 1744, 374 },
	{ 1745, 397 },
	{ 1746, 408 },
	{ 1747, 396 },
	{ 1748, 395 },
	{ 1749, 407 },
	{ 1750, 409 },
	{ 1751, 381 },
	{ 1752, 377 },
	{ 1753, 406 },
	{ 1754, 404 },
	{ 1755, 403 },
	{ 1756, 400 },
};

// $Barnyard: FUNCTION 00499e80
void AGopherShopInterface::InitialiseClass()
{
	ms_oFunctionTable.Register( TNULL );

	ms_oFunctionTable.AddFunction( AFunctionPointer1ArgsRT<TINT, TINT>::Create( "AGopherShopInterface::GetNumBought", &AGopherShopInterface::GetNumBought ) );
	ms_oFunctionTable.AddFunction( AFunctionPointer0ArgsRT<TBOOL>::Create( "RemoveAllBoughtItems", &AGopherShopInterface::RemoveAllBoughtItems ) );
}

// $Barnyard: FUNCTION 00499a80
void AGopherShopInterface::DeinitialiseClass()
{
	ms_oFunctionTable.Unregister();
}

AHackedQuickSave::Buffer* AGopherShopInterface::GetShopBuffer()
{
	// NOTE: inlined *( DAT_007b4ce8 + 0x10 ), Joey's offer is the first byte of the buffer
	return AHackedQuickSave::GetSingleton()->GetBuffer( AHackedQuickSave::BUFFER_OFFERS );
}

// $Barnyard: FUNCTION 00499de0
TINT AGopherShopInterface::GetNumBought( TINT a_iItemId )
{
	AHackedQuickSave::Buffer* pBuffer = GetShopBuffer();

	if ( !pBuffer->IsSaved() )
		return 0;

	pBuffer->SetCursor( 0 );

	TUINT8 uiJoeyOffer;
	TINT   iNumItems;
	TBOOL  bUnk;
	TINT   iUnk;
	pBuffer->ReadUInt8( uiJoeyOffer );
	pBuffer->ReadInt( iNumItems );
	pBuffer->ReadBool( bUnk );
	pBuffer->ReadInt( iUnk );

	for ( TINT i = 0; i < iNumItems; i++ )
	{
		TINT iItemId;
		TINT iCount;
		pBuffer->ReadInt( iItemId );
		pBuffer->ReadInt( iCount );

		if ( a_iItemId == iItemId )
			return iCount;
	}

	return 0;
}

// $Barnyard: FUNCTION 00499ed0
TBOOL AGopherShopInterface::HasBought( TINT a_iItemId )
{
	return GetNumBought( a_iItemId ) > 0;
}

// $Barnyard: FUNCTION 00499ef0
TINT AGopherShopInterface::GetPercentBought()
{
	AHackedQuickSave::Buffer* pBuffer = GetShopBuffer();

	if ( !pBuffer->IsSaved() )
		return 0;

	pBuffer->SetCursor( 0 );

	TUINT8 uiJoeyOffer;
	TINT   iNumItems;
	TBOOL  bUnk;
	TINT   iUnk;
	pBuffer->ReadUInt8( uiJoeyOffer );
	pBuffer->ReadInt( iNumItems );
	pBuffer->ReadBool( bUnk );
	pBuffer->ReadInt( iUnk );

	TINT iNumBought = 0;

	for ( TINT i = 0; i < iNumItems; i++ )
	{
		// NOTE: retail reads them into an item of the shop (36 bytes with 5 TPString8)
		TINT iItemId;
		TINT iCount;
		pBuffer->ReadInt( iItemId );
		pBuffer->ReadInt( iCount );

		// 310-313 and 896 don't count
		if ( iItemId != 310 && iItemId != 311 && iItemId != 312 && iItemId != 313 && iItemId != 896 )
			iNumBought++;
	}

	return 100 * iNumBought / NUM_ITEMS;
}

// $Barnyard: FUNCTION 00499b60
TBOOL AGopherShopInterface::RemoveAllBoughtItems()
{
	AHackedQuickSave::Buffer* pBuffer = GetShopBuffer();

	pBuffer->Reset();
	TUtil::MemClear( pBuffer->GetData(), pBuffer->GetSize() );

	return TTRUE;
}

// $Barnyard: FUNCTION 00499a90
AGopherShopInterface::JoeyOffer AGopherShopInterface::GetJoeyOffer( TUINT8 a_uiIndex )
{
	return ms_aJoeyOffers[ a_uiIndex ];
}
