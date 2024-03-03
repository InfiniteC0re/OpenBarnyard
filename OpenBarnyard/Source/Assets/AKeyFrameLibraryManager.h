#pragma once
#include "Memory/AMemory.h"

#include <Plugins/PPropertyParser/PBProperties.h>
#include <Render/TAnimation.h>
#include <Toshi/TPString8.h>
#include <Toshi/T2Map.h>

class AKeyFrameLibraryManager :
	public Toshi::TObject,
	public Toshi::TSingleton<AKeyFrameLibraryManager>
{
public:
	TDECLARE_CLASS(Toshi::TObject);

	using LibraryMap = Toshi::T2Map<Toshi::TPString8, Toshi::TKeyframeLibrary*, Toshi::TPString8::Comparator>;

public:
	AKeyFrameLibraryManager();
	~AKeyFrameLibraryManager();

	// Loads all libraries listed in the a_pArray
	void LoadLibrariesFromProperties(const PBPropertyValue* a_pArray, Toshi::TTRB& a_rTRB);

	// Creates a reference to library if it exists or loads if from the file
	TBOOL LoadLibrary(const Toshi::TPString8& a_rLibraryName, Toshi::TTRB* a_pTRB = TNULL);

	// Removes reference to the library and unloads it if no references left
	TBOOL UnrefLibrary(const Toshi::TPString8& a_rLibraryName);

private:
	void UnloadLibrary(LibraryMap::Node*& a_rLibrary);
	void UnloadAllLibraries();

private:
	LibraryMap m_Libraries;
	Toshi::T2Map<Toshi::TPString8, TINT, Toshi::TPString8::Comparator> m_LibrariesRefs;
};