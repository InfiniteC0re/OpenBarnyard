#include "pch.h"
#include "AArgumentParser.h"

#include <Toshi/Toshi.h>
#include <Toshi/TPString8.h>

#include <Core/TMemoryInitialiser.h>

#include <Render/TTMDBase.h>
#include <Render/TTMDWin.h>

#include <Plugins/PTRB.h>
#include <Plugins/PPropertyParser/PPropertiesWriter.h>

#include "json.hpp"

TOSHI_NAMESPACE_USING

using json = nlohmann::json;

static TMemoryInitialiser s_MemoryInitialiser;

void JsonToProperties(json& a_rJSON, PProperties& a_rProperties)
{
	for (auto it = a_rJSON.begin(); it != a_rJSON.end(); it++)
	{
		if (it->is_string())
		{
			a_rProperties.AddProperty(it.key().c_str(), it.value().get<std::string>().c_str());
		}
		else if (it->is_number_integer())
		{
			a_rProperties.AddProperty(it.key().c_str(), it.value().get<TINT>());
		}
		else if (it->is_number_unsigned())
		{
			a_rProperties.AddProperty(it.key().c_str(), it.value().get<TUINT>());
		}
		else if (it->is_number_float())
		{
			a_rProperties.AddProperty(it.key().c_str(), it.value().get<TFLOAT>());
		}
		else if (it->is_array())
		{
			auto propertyArray = a_rProperties.AddPropertyArray(it.key().c_str())->GetValue()->GetArray();

			for (auto arrIt = it->begin(); arrIt != it->end(); arrIt++)
			{
				if (arrIt->is_string())
				{
					propertyArray->Add(arrIt.value().get<std::string>().c_str());
				}
				else if (arrIt->is_number_integer())
				{
					propertyArray->Add(arrIt.value().get<TINT>());
				}
				else if (arrIt->is_number_unsigned())
				{
					propertyArray->Add(arrIt.value().get<TUINT>());
				}
				else if (arrIt->is_number_float())
				{
					propertyArray->Add(arrIt.value().get<TFLOAT>());
				}
			}
		}
		else if (it->is_object())
		{
			auto pProperties = a_rProperties.AddProperties(it.key().c_str())->GetValue()->GetProperties();
			JsonToProperties(*it, *pProperties);
		}
	}
}

int main(int argc, char** argv)
{
	TUtil::ToshiCreate(
		TUtil::TOSHIParams{
			.szCommandLine = GetCommandLineA()
		}
	);

	TUtil::SetTPStringPool(new TPString8Pool(1024, 0, &T2Allocator::s_GlobalAllocator, TNULL));

	AArgumentParser args(argv, argc);
	if (args.GetMode() == AArgumentParser::Mode::Compile)
	{
		auto pJSONFile = TFile::Create(args.GetInPath());

		if (pJSONFile)
		{
			auto uiFileSize = pJSONFile->GetSize();
			char* pFileData = new char[uiFileSize + 1];
			pJSONFile->Read(pFileData, uiFileSize);
			pJSONFile->Destroy();
			pFileData[uiFileSize] = '\0';

			json document = json::parse(pFileData);
			delete[] pFileData;

			PProperties properties;
			JsonToProperties(document, properties);

			TString8 inFilepath = args.GetInPath();
			TString8 outFilepath;

			for (TUINT i = 0; i < inFilepath.Length(); i++)
			{
				if (inFilepath[i] == '/') inFilepath[i] = '\\';
			}

			if (!args.GetOutPath())
			{
				TString8 inputFileDirName;
				inputFileDirName.Copy(inFilepath, inFilepath.FindReverse('\\', -1) + 1);

				TString8 inputFileName = inFilepath.GetString(inFilepath.FindReverse('\\', -1) + 1);
				inputFileName.Truncate(inputFileName.FindReverse('.', -1));

				if (inputFileDirName.Length() != 0)
				{
					outFilepath = TString8::Format("%s%s.trb", inputFileDirName.GetString(), inputFileName.GetString());
				}
				else
				{
					outFilepath = TString8::Format("%s.trb", inputFileName.GetString());
				}
			}
			else
			{
				outFilepath = args.GetOutPath();
			}

			PPropertiesWriter::WriteTRB(outFilepath, properties, args.IsUsingBTEC());
		}
	}

	return 0;
}