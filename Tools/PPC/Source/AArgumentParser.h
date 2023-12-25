#pragma once

class AArgumentParser
{
public:
	enum class Mode
	{
		Compile,
	};

public:
	AArgumentParser(char** argv, int argc) :
		m_argv(argv),
		m_argc(argc),
		m_inPath(TNULL),
		m_outPath(TNULL)
	{
		m_useCompression = TFALSE;

		if (IsDebuggerPresent())
		{
			m_mode = Mode::Compile;
			m_inPath = "test.json";
		}
		else
		{
			TOSHI_TRACE("Num arguments: {}", argc);

			for (TINT i = 1; i < argc; i++)
			{
				if (*m_argv == TNULL) return;

				TOSHI_TRACE("Parsing argument {}", *m_argv);

				if (Toshi::TStringManager::String8Compare(*m_argv, "-c") == 0)
				{
					TOSHI_TRACE("Found -c parameter");
					m_mode = Mode::Compile;
					ParseString(m_inPath);
				}
				else if (Toshi::TStringManager::String8Compare(*m_argv, "-o") == 0)
				{
					TOSHI_TRACE("Found -o parameter");
					ParseString(m_outPath);
				}
				else if (Toshi::TStringManager::String8Compare(*m_argv, "-btec") == 0)
				{
					TOSHI_TRACE("Found -btec parameter");
					m_useCompression = TTRUE;
				}

				m_argv++;
			}
		}
	}

	Mode GetMode() const { return m_mode; }

	TBOOL IsUsingBTEC() const { return m_useCompression; }

	const char* GetInPath() const { return m_inPath; }
	const char* GetOutPath() const { return m_outPath; }

private:
	TBOOL ParseString(const char*& a_rOut, const char* a_szDefault = TNULL)
	{
		if (*(m_argv + 1) == TNULL)
		{
			if (!a_szDefault)
			{
				TOSHI_ERROR("Invalid usage");
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
};
