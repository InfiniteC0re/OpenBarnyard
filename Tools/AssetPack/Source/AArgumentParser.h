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
		m_argc(argc)
	{
		m_useCompression = TFALSE;

		if (IsDebuggerPresent())
		{
			m_mode = Mode::Unpack;
			m_useCompression = TFALSE;
			ParseString(m_path1, "C:\\Stuff\\Barnyard\\Game\\Data\\Assets\\lib_sharpsquirter.trb");
		}
		else
		{
			TOSHI_TRACE("Num arguments: {}", argc);

			for (TINT i = 1; i < argc; i++)
			{
				if (*m_argv == TNULL) return;

				TOSHI_TRACE("Parsing argument {}", *m_argv);

				if (Toshi::TStringManager::String8Compare(*m_argv, "-p") == 0)
				{
					TOSHI_TRACE("Found -p parameter");

					m_mode = Mode::Pack;
					ParseString(m_path1);
				}
				else if (Toshi::TStringManager::String8Compare(*m_argv, "-u") == 0)
				{
					TOSHI_TRACE("Found -u parameter");
					m_mode = Mode::Unpack;
					ParseString(m_path1);
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

	const char* GetPath1() const { return m_path1; }

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
	const char* m_path1;
};