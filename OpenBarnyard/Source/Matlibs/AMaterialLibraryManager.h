#pragma once
#include <Toshi2/T2RedBlackTree.h>

class AMaterialLibraryManager : 
	public Toshi::TGenericClassDerived<AMaterialLibraryManager, Toshi::TObject, "AMaterialLibraryManager", TMAKEVERSION(1, 0), TTRUE>
{
public:
	class Slot
	{
	public:
		Slot()
		{
			Next = this;
		}

	public:
		Slot* Next;
		void* Unknown1;
		Toshi::TString8 Name;
		void* Unknown2;
	};

	static constexpr TUINT NUM_SLOTS = 600;

public:
	AMaterialLibraryManager();
	
private:
	Toshi::T2RedBlackTree<void*> m_Tree1;
	Toshi::T2RedBlackTree<void*> m_LoadedLibraries;
	Slot m_aSlots[600];
};