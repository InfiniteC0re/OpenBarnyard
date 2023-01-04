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
};
