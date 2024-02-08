#pragma once

class AArgumentParser
{
public:
	enum class Mode
	{
		Pack,
		Unpack,
	};

public:
	AArgumentParser(char** argv, int argc) :
		m_argv(argv),
		m_argc(argc),
		m_outPath(TNULL)
	{
		m_useCompression = TFALSE;

		if (IsDebuggerPresent())
		{
			m_mode = Mode::Unpack;
			m_useCompression = TFALSE;
			ParseString(m_inPath, "C:\\Stuff\\Barnyard\\Game\\Data\\Assets\\lib_sharpsquirter.trb");
		}
		else
		{
			TDEBUG("Num arguments: %i\n", argc);

			for (TINT i = 1; i < argc; i++)
			{
				if (*m_argv == TNULL) return;

				TDEBUG("Parsing argument %s\n", *m_argv);

				if (Toshi::TStringManager::String8Compare(*m_argv, "-p") == 0)
				{
					TDEBUG("Found -p parameter\n");

					m_mode = Mode::Pack;
					ParseString(m_inPath);
					ParseString(m_assetName);
				}
				else if (Toshi::TStringManager::String8Compare(*m_argv, "-u") == 0)
				{
					TDEBUG("Found -u parameter\n");
					m_mode = Mode::Unpack;
					ParseString(m_inPath);
				}
				else if (Toshi::TStringManager::String8Compare(*m_argv, "-btec") == 0)
				{
					TDEBUG("Found -btec parameter\n");
					m_useCompression = TTRUE;
				}
				else if (Toshi::TStringManager::String8Compare(*m_argv, "-o") == 0)
				{
					TDEBUG("Found -o parameter\n");
					ParseString(m_outPath);
				}

				m_argv++;
			}
		}
	}

	Mode GetMode() const { return m_mode; }

	TBOOL IsUsingBTEC() const { return m_useCompression; }

	const char* GetInPath() const { return m_inPath; }
	const char* GetOutPath() const { return m_outPath; }
	const char* GetAssetName() const { return m_assetName; }

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
	const char* m_inPath;
	const char* m_outPath;
	const char* m_assetName;
};
