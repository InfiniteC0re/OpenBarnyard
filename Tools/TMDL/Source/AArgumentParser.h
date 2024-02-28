#pragma once

class AArgumentParser
{
public:
	enum class Mode
	{
		Compile,
		Decompile,
		Info
	};

public:
	AArgumentParser(char** argv, int argc) :
		m_argv(argv),
		m_argc(argc),
		m_inPath(TNULL),
		m_texturesPath(TNULL),
		m_outPath(TNULL),
		m_keylib("MiscObj"),
		m_isTerrain(TFALSE),
		m_bNoSkeleton(TTRUE)
	{
		m_useCompression = TFALSE;

		if (IsDebuggerPresent())
		{
//#if 0
//			m_mode = Mode::Compile;
//			m_useCompression = TFALSE;
//			m_isTerrain = TTRUE;
//			m_inPath = "C:\\Users\\InfiniteC0re\\Desktop\\TMDLWS\\Barn_L0Mod0.fbx";
//			m_texturesPath = "C:\\Users\\InfiniteC0re\\Desktop\\TMDLWS\\Textures\\";
//			m_outPath = "C:\\Stuff\\Barnyard\\Game (ND)\\Data\\Terrain\\EnvMain\\Barn_L0Mod0.trb";
//#else
//			m_mode = Mode::Decompile;
//			m_useCompression = TFALSE;
//			m_isTerrain = TTRUE;
//			m_inPath = "C:\\Stuff\\Barnyard\\Game (ND)\\Mods\\BYWinterMod\\DevTools\\Assets\\bn_crocodile.trb";
//			m_texturesPath = "C:\\Users\\InfiniteC0re\\Desktop\\TMDLWS\\Textures\\";
//			m_outPath = "C:\\Users\\InfiniteC0re\\Desktop\\TMDLWS\\bn_crocodile.fbx";
//#endif
		}
		else
		{
			TTRACE("Num arguments: %i\n", argc);

			for (TINT i = 1; i < argc; i++)
			{
				if (*m_argv == TNULL) return;

				TTRACE("Parsing argument &s\n", *m_argv);

				if (Toshi::TStringManager::String8Compare(*m_argv, "-i") == 0)
				{
					TTRACE("Found -i parameter\n");
					m_mode = Mode::Info;
					ParseString(m_inPath);
				}
				else if (Toshi::TStringManager::String8Compare(*m_argv, "-c") == 0)
				{
					TTRACE("Found -c parameter\n");
					m_mode = Mode::Compile;
					ParseString(m_inPath);
					ParseString(m_texturesPath);
				}
				else if (Toshi::TStringManager::String8Compare(*m_argv, "-d") == 0)
				{
					TTRACE("Found -d parameter\n");
					m_mode = Mode::Decompile;
					ParseString(m_inPath);
					ParseString(m_texturesPath);
				}
				else if (Toshi::TStringManager::String8Compare(*m_argv, "-o") == 0)
				{
					TTRACE("Found -o parameter\n");
					ParseString(m_outPath);
				}
				else if (Toshi::TStringManager::String8Compare(*m_argv, "-btec") == 0)
				{
					TTRACE("Found -btec parameter\n");
					m_useCompression = TTRUE;
				}
				else if (Toshi::TStringManager::String8Compare(*m_argv, "-keylib") == 0)
				{
					TTRACE("Found -keylib parameter\n");
					ParseString(m_keylib);
					m_bNoSkeleton = TFALSE;
				}
				else if (Toshi::TStringManager::String8Compare(*m_argv, "-terrain") == 0)
				{
					TTRACE("Found -terrain parameter\n");
					m_isTerrain = TTRUE;
				}

				m_argv++;
			}
		}
	}

	Mode GetMode() const { return m_mode; }

	TBOOL IsUsingBTEC() const { return m_useCompression; }
	TBOOL IsTerrain() const { return m_isTerrain; }
	TBOOL NoSkeleton() const { return m_bNoSkeleton; }

	const char* GetInPath() const { return m_inPath; }
	const char* GetTexturesPath() const { return m_texturesPath; }
	const char* GetOutPath() const { return m_outPath; }
	const char* GetKeyLibName() const { return m_keylib; }

private:
	TBOOL ParseString(const char*& a_rOut, const char* a_szDefault = TNULL)
	{
		if (*(m_argv + 1) == TNULL)
		{
			if (!a_szDefault)
			{
				TERROR("Invalid usage\n");
				a_rOut = TNULL;
				exit(1);
			}

			a_rOut = a_szDefault;
		}
		else
		{
			a_rOut = *(++m_argv);
		}

		return a_rOut != TNULL;
	}

private:
	char** m_argv;
	int m_argc;
	Mode m_mode;
	TBOOL m_useCompression;
	TBOOL m_isTerrain;
	TBOOL m_bNoSkeleton;
	const char* m_inPath;
	const char* m_texturesPath;
	const char* m_outPath;
	const char* m_keylib;
};
