#pragma once

using TFloat = float;
using TBOOL = bool;
using TINT = int;

constexpr TBOOL TFALSE = false;
constexpr TBOOL TTRUE = true;

constexpr nullptr_t TNULL = nullptr;

union TVersion {
	uint32_t Value;

	struct {
		uint16_t Minor;
		uint16_t Major;
	} Parts;

	void operator=(uint32_t version)
	{
		Value = version;
	}

	operator uint32_t&()
	{
		return Value;
	}
};

#define _BYTE  uint8_t
#define _WORD  uint16_t
#define _DWORD uint32_t
#define _QWORD uint64_t

#define LOWBYTE(x)   (*((_BYTE*)&(x)))   // low byte
#define LOWWORD(x)   (*((_WORD*)&(x)))   // low word
#define LOWDWORD(x)  (*((_DWORD*)&(x)))  // low dword
#define HIGHBYTE(x)   (*((_BYTE*)&(x)+1))
#define HIGHWORD(x)   (*((_WORD*)&(x)+1))
#define HIGHDWORD(x)  (*((_DWORD*)&(x)+1))
#define BYTEn(x, n)   (*((_BYTE*)&(x)+n))
#define WORDn(x, n)   (*((_WORD*)&(x)+n))

#define BYTE1(x)   BYTEn(x,  1)
#define BYTE2(x)   BYTEn(x,  2)
#define BYTE3(x)   BYTEn(x,  3)
#define BYTE4(x)   BYTEn(x,  4)
#define BYTE5(x)   BYTEn(x,  5)
#define BYTE6(x)   BYTEn(x,  6)
#define BYTE7(x)   BYTEn(x,  7)
#define BYTE8(x)   BYTEn(x,  8)
#define BYTE9(x)   BYTEn(x,  9)
#define BYTE10(x)  BYTEn(x, 10)
#define BYTE11(x)  BYTEn(x, 11)
#define BYTE12(x)  BYTEn(x, 12)
#define BYTE13(x)  BYTEn(x, 13)
#define BYTE14(x)  BYTEn(x, 14)
#define BYTE15(x)  BYTEn(x, 15)
