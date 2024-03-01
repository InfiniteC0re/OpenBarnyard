#include "pch.h"
#include "AArgumentParser.h"

#include <Core/TMemoryInitialiser.h>

#include <Toshi/Toshi.h>
#include <Toshi/TString8.h>
#include <Toshi/TString16.h>
#include <Toshi/T2FixedWString.h>
#include <Toshi/TArray.h>

#include <T2Locale/T2Locale.h>

#include <Plugins/PTRB.h>

TOSHI_NAMESPACE_USING

static TMemoryInitialiser s_MemoryInitialiser;

int main(int argc, char** argv)
{
	TUtil::TOSHIParams toshiParams;
	toshiParams.szCommandLine = GetCommandLineA();

	TUtil::ToshiCreate(toshiParams);

	AArgumentParser args(argv, argc);

	if (!args.GetOutPath())
	{
		TERROR("No output path specified!\n");
		return 1;
	}

	if (args.GetMode() == AArgumentParser::Mode::Compile)
	{
		auto pTextFile = TFile::Create(args.GetInPath());

		if (pTextFile)
		{
			TINFO("Opened file for input!\n");
			TINFO("Reading input file...\n");

			PTRB::TRBF outTrbFile;
			auto pOutStack = outTrbFile.GetSECT()->CreateStack();

			// Read file data into a buffer
			auto uiFileSize = pTextFile->GetSize();
			char* pFileBuffer = new char[uiFileSize + 2];

			pTextFile->Read(pFileBuffer, uiFileSize);
			pFileBuffer[uiFileSize] = '\0';
			pFileBuffer[uiFileSize + 1] = '\0';
			pTextFile->Destroy();

			INT iIsUnicodeResult;
			IsTextUnicode(pFileBuffer, uiFileSize, &iIsUnicodeResult);

			if (iIsUnicodeResult & IS_TEXT_UNICODE_UNICODE_MASK)
			{
				TArray<TString16> foundStrings;

				T2FixedWString2048 localeString;
				wchar_t* pFileStart = TREINTERPRETCAST(wchar_t*, pFileBuffer);
				wchar_t* pFileEnd = TREINTERPRETCAST(wchar_t*, pFileBuffer + uiFileSize);
				wchar_t* pFileCursor = TREINTERPRETCAST(wchar_t*, pFileBuffer);

				while (pFileCursor < pFileEnd)
				{
					TINT iLocaleStringLength;
					pFileCursor += localeString.ParseLine(pFileCursor, -1, &iLocaleStringLength, TFALSE, TTRUE);

					foundStrings.Push(localeString.Get());
				}

				TINFO("Parsed %d locale strings.\n", foundStrings.Size());

				auto pLocaleStrings = pOutStack->Alloc<T2Locale::LocaleStrings>();
				
				pLocaleStrings->m_numstrings = foundStrings.Size();
				pOutStack->Alloc<T2Locale::LocaleString>(&pLocaleStrings->Strings, foundStrings.Size());

				TARRAY_FOREACH(foundStrings, str)
				{
					auto iIndex = str.GetCurrentIndex();
					pOutStack->Alloc<TWCHAR>(&pLocaleStrings->Strings[iIndex], str->Length() + 1);

					T2String16::Copy(pLocaleStrings->Strings[iIndex], str.Get());
				}

				outTrbFile.GetSYMB()->Add(pOutStack, "LocaleStrings", pLocaleStrings.get());
				outTrbFile.WriteToFile(args.GetOutPath());
			}
			else
			{
				TERROR("The input file's encoding is not UTF-16!\n");
			}
		}
		else
		{
			TERROR("Couldn't open file for input!\n");
		}
	}
	else if (args.GetMode() == AArgumentParser::Mode::Export)
	{
		TTRB inTrb;

		if (TTRB::ERROR_OK == inTrb.Load(args.GetInPath()))
		{
			auto pLocaleStrings = inTrb.CastSymbol<T2Locale::LocaleStrings>("LocaleStrings");
			
			if (pLocaleStrings)
			{
				auto pOutFile = TFile::Create(args.GetOutPath(), TFILEMODE_CREATENEW | TFILEMODE_WRITE);

				if (pOutFile)
				{
					for (TINT i = 0; i < pLocaleStrings->m_numstrings; i++)
					{
						auto pString = pLocaleStrings->Strings[i];
						T2FixedWString2048 finalString;
						TUINT iPos = 0;

						while (*pString != L'\0')
						{
							if (*pString == L'\n')
							{
								finalString[iPos++] = '\\';
								finalString[iPos++] = 'n';
							}
							else
							{
								finalString[iPos++] = *pString;
							}

							pString++;
						}

						finalString[iPos] = '\0';
						pOutFile->WPrintf(L"%s\n", finalString.Get());
					}

					pOutFile->Destroy();
				}
				else
				{
					TERROR("Couldn't open file for output!\n");
				}
			}
			else
			{
				TERROR("The specified TRB file doesn't contain any locale strings!\n");
			}
		}
		else
		{
			TERROR("Couldn't open TRB file to export from!\n");
		}
	}

	return 0;
}