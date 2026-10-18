#pragma once
#include <Toshi/TNodeList.h>

TOSHI_NAMESPACE_START

//-----------------------------------------------------------------------------
// Purpose: a key and a value in a TNodeList (the fixed widget properties of a
// style).
//-----------------------------------------------------------------------------
template <class K, class V>
struct TGUIPairNode : public TNodeList<TGUIPairNode<K, V>>::TNode
{
	TGUIPairNode( const K& a_rcKey, const V& a_rcValue )
	    : oKey( a_rcKey ), oValue( a_rcValue )
	{}

	K oKey;   // 0x0C
	V oValue; // 0x10
};

TOSHI_NAMESPACE_END
