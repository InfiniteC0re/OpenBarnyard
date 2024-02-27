#include "pch.h"
#include "AArgumentParser.h"

#include <Toshi/Toshi.h>
#include <Toshi/TPString8.h>
#include <T2Locale/T2Locale.h>

#include <Core/TMemoryInitialiser.h>

#include <Render/TTMDBase.h>
#include <Render/TTMDWin.h>

#include <Plugins/PTRB.h>
#include <Plugins/PPropertyParser/PPropertiesWriter.h>

#include "json.hpp"
#include "tinyxml2.h"

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

void PropertiesToXml(tinyxml2::XMLElement* a_pRootElement, const PProperties* a_pProperties);

void PropertyValueToXml(tinyxml2::XMLElement* a_pRootElement, const PPropertyValue* a_pValue)
{
	switch (a_pValue->GetType())
	{
	case PPropertyValue::Type::String:
		a_pRootElement->SetAttribute("type", "string");
		a_pRootElement->SetAttribute("value", a_pValue->GetString());
		break;
	case PPropertyValue::Type::Float:
		a_pRootElement->SetAttribute("type", "float");
		a_pRootElement->SetAttribute("value", a_pValue->GetFloat());
		break;
	case PPropertyValue::Type::Bool:
		a_pRootElement->SetAttribute("type", "bool");
		a_pRootElement->SetAttribute("value", a_pValue->GetBoolean());
		break;
	case PPropertyValue::Type::Int:
		a_pRootElement->SetAttribute("type", "int");
		a_pRootElement->SetAttribute("value", a_pValue->GetInteger());
		break;
	case PPropertyValue::Type::UInt32:
		a_pRootElement->SetAttribute("type", "uint");
		a_pRootElement->SetAttribute("value", a_pValue->GetUINT32());
		break;
	case PPropertyValue::Type::Props:
		a_pRootElement->SetAttribute("type", "property");
		PropertiesToXml(a_pRootElement, a_pValue->GetProperties());
		break;
	case PPropertyValue::Type::Array:
		a_pRootElement->SetAttribute("type", "array");
		auto pArray = a_pValue->GetArray();

		for (TUINT i = 0; i < pArray->GetSize(); i++)
		{
			auto pArrayElement = a_pRootElement->InsertNewChildElement("item");
			auto pArrayValue = pArray->GetValue(i);

			PropertyValueToXml(pArrayElement, pArrayValue);
		}
		break;
	}
}


void PropertiesToXml(tinyxml2::XMLElement* a_pRootElement, const PProperties* a_pProperties)
{
	T2_FOREACH((*a_pProperties), it)
	{
		auto pName = &it->GetName();
		auto pValue = it->GetValue();

		auto pElement = a_pRootElement->InsertNewChildElement(pName->GetString());
		PropertyValueToXml(pElement, pValue);
	}
}

void TranslateDocument(tinyxml2::XMLElement* a_pCurrentElement)
{
	static TTRB* s_pLocaleTrb = TNULL;
	static T2Locale::LocaleStrings* s_pLocaleStrings = TNULL;

	if (s_pLocaleTrb == TNULL)
	{
		s_pLocaleTrb = new TTRB;

		s_pLocaleTrb->Load("C:\\Stuff\\Barnyard\\Game\\Data\\Locale\\eng-uk.trb");
		s_pLocaleStrings = s_pLocaleTrb->CastSymbol<T2Locale::LocaleStrings>("LocaleStrings");
	}

	if (T2String8::CompareNoCase(a_pCurrentElement->Name(), "hudtext") == 0 ||
		T2String8::CompareNoCase(a_pCurrentElement->Name(), "message") == 0 ||
		T2String8::CompareNoCase(a_pCurrentElement->Name(), "title") == 0)
	{
		TINT iLocaleId = T2String8::StringToInt(a_pCurrentElement->Attribute("value"));

		static TCHAR s_Buffer[2048];
		TStringManager::StringUnicodeToChar(s_Buffer, s_pLocaleStrings->Strings[iLocaleId], -1);

		a_pCurrentElement->SetAttribute("value", s_Buffer);
	}
	else
	{
		auto pChildElement = a_pCurrentElement->FirstChildElement();

		while (pChildElement)
		{
			TranslateDocument(pChildElement);
			pChildElement = pChildElement->NextSiblingElement();
		}
	}
}

void JoinInteractionsRecurse(tinyxml2::XMLDocument* a_pInteractions, tinyxml2::XMLElement* a_pActivitySet, tinyxml2::XMLElement* a_pCurrentElement)
{
	if (T2String8::CompareNoCase(a_pCurrentElement->Name(), "activities") == 0)
	{
		auto pElem = a_pCurrentElement->FirstChildElement("_elem");
		auto pItemElement = pElem->FirstChildElement();

		while (pItemElement)
		{
			TINT iActivityId = T2String8::StringToInt(pItemElement->Attribute("value"));
			TASSERT(iActivityId < a_pActivitySet->ChildElementCount());

			auto pActivityElement = a_pActivitySet->FirstChildElement();
			TINT iActivityCounter = 0;

			while (pActivityElement)
			{
				if (iActivityCounter == iActivityId) break;

				pActivityElement = pActivityElement->NextSiblingElement();
				iActivityCounter++;
			}

			a_pCurrentElement->InsertEndChild(pActivityElement->DeepClone(a_pInteractions));
			pItemElement = pItemElement->NextSiblingElement();
		}

		a_pCurrentElement->DeleteChild(pElem);
	}
	else
	{
		auto pChildElement = a_pCurrentElement->FirstChildElement();

		while (pChildElement)
		{
			JoinInteractionsRecurse(a_pInteractions, a_pActivitySet, pChildElement);
			pChildElement = pChildElement->NextSiblingElement();
		}
	}
}

int main(int argc, char** argv)
{
	TUtil::TOSHIParams toshiParams;
	toshiParams.szCommandLine = GetCommandLineA();

	TUtil::ToshiCreate(toshiParams);
	TUtil::SetTPStringPool(new TPString8Pool(1024, 0, &T2Allocator::s_GlobalAllocator, TNULL));

	tinyxml2::XMLDocument activitysets;
	tinyxml2::XMLDocument interactions;

	interactions.LoadFile("editor_quests__1.xml");
	TranslateDocument(interactions.FirstChildElement("PProperties"));

	interactions.SaveFile("editor_quests__1_Merged.xml");

	//activitysets.LoadFile("ActivitySets1.xml");
	//interactions.LoadFile("AllInteractions.xml");

	//TranslateDocument(activitysets.FirstChildElement("PProperties"));
	//TranslateDocument(interactions.FirstChildElement("PProperties"));

	//JoinInteractionsRecurse(
	//	&interactions,
	//	activitysets.FirstChildElement("PProperties")->FirstChildElement("activityset"),
	//	interactions.FirstChildElement("PProperties")
	//);

	//interactions.SaveFile("AllInteractions_Joined.xml");

	return 0;

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
	else if (args.GetMode() == AArgumentParser::Mode::Decompile)
	{
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
				outFilepath = TString8::Format("%s%s.xml", inputFileDirName.GetString(), inputFileName.GetString());
			}
			else
			{
				outFilepath = TString8::Format("%s.xml", inputFileName.GetString());
			}
		}
		else
		{
			outFilepath = args.GetOutPath();
		}

		TTRB inTrb;
		if (inTrb.Load(inFilepath) == TTRB::ERROR_OK)
		{
			auto pProperties = PProperties::LoadFromTRB(inTrb);

			tinyxml2::XMLDocument doc;
			doc.InsertEndChild(doc.NewDeclaration());
			doc.InsertEndChild(doc.NewComment("Decompiled by OpenToshi's XML2PProperties"));

			auto pRootElement = doc.NewElement("PProperties");
			pRootElement->SetAttribute("Compressed", TFALSE);

			PropertiesToXml(pRootElement, pProperties);

			doc.InsertEndChild(pRootElement);
			doc.SaveFile(outFilepath);
		}
		
	}

	return 0;
}