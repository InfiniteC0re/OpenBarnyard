#include "ToshiPCH.h"
#include "TXUIResource.h"
#include "TXUIElement.h"
#include "TXUICanvas.h"
#include "Toshi/Xui/TXUIScene.h"
#include "Toshi/Xui/TXUI.h"
#include "TXUIGroup.h"
#include "TXUIBackButton.h"
#include "TXUIText.h"
#include "TXUIImage.h"
#include "TXUIFigure.h"
#include "TXUINineGrid.h"
#include "XURReader.h"
#include "TXUILabel.h"
#include "TXUITabScene.h"
#include "TXUISliderBar.h"
#include "TXUICheckBox.h"
#include "TXUIList.h"

namespace Toshi {

	TXUIResource::~TXUIResource()
	{
		if (m_pMemoryHeap)
		{
			m_oHeader.m_apSections = TNULL;
			TMemory::DestroyHeap(m_pMemoryHeap);
		}
		else
		{
			for (size_t i = 0; i < m_uiStringCount; i++)
				delete[] m_asStringTable[i];

			delete[] m_asStringTable;
			delete[] m_avVectors;
			delete[] m_aqZRotTable;

			if (m_pRoot)
				delete m_pRoot;

			if (m_pStringTable)
				delete m_pStringTable;

			delete[] m_pCustData;
			delete[] m_oHeader.m_apSections;
		}
	}

	TBOOL TXUIResource::ReadHeader(uint8_t* buffer)
	{
		XURReader reader(buffer);

		m_oHeader.m_uiFileID = reader.ReadUInt32LE();
		TASSERT(m_oHeader.m_uiFileID == IDXUR, "Not a .xur file!");

		m_oHeader.m_uiVersion = reader.ReadUInt32();
		m_oHeader.m_uiFlags = reader.ReadUInt16();
		m_oHeader.m_uiXuiVersion = reader.ReadUInt32();
		m_oHeader.m_usBinSize = reader.ReadUInt32();
		m_oHeader.m_usNumSections = reader.ReadUInt16();

		TASSERT(m_oHeader.m_usNumSections > 0, "There must be one or more Sections");
		m_oHeader.m_apSections = new (TXUI::MemoryBlock()) Section[m_oHeader.m_usNumSections];

		uint32_t sectionID = PARSEDWORD(buffer);

		if (sectionID != IDXURSTRING && sectionID != IDXURVEC && sectionID != IDXURQUAT && sectionID != IDXURCUST)
			buffer += 0x28;

		for (size_t i = 0; i < m_oHeader.m_usNumSections; i++)
		{
			m_oHeader.m_apSections[i].m_uiSectionID = reader.ReadUInt32LE();

			TASSERT(
				m_oHeader.m_apSections[i].m_uiSectionID == IDXURSTRING ||
				m_oHeader.m_apSections[i].m_uiSectionID == IDXURVEC ||
				m_oHeader.m_apSections[i].m_uiSectionID == IDXURQUAT ||
				m_oHeader.m_apSections[i].m_uiSectionID == IDXURCUST ||
				m_oHeader.m_apSections[i].m_uiSectionID == IDXURDATA,
				"Invalid Section ID"
			);

			m_oHeader.m_apSections[i].m_uiOffset = reader.ReadUInt32();
			m_oHeader.m_apSections[i].m_uiSize = reader.ReadUInt32();
		}

		return TTRUE;
	}


	void TXUIResource::Load(TBOOL loadStringTables, const char* filenameXUIB, const char* fileNameStringTable, TBOOL loadTrb, void* unk3)
	{
		Destroy();

		if (loadStringTables && fileNameStringTable != TNULL)
		{
			TTODO("Load Stringtables");
		}

		const char* extension = loadTrb ? ".trb" : ".xur";
		const char* fullFileName = strstr(filenameXUIB, extension);

		if (!loadTrb)
		{
			TFile* file = TFile::Create(fullFileName);

			if (file != TNULL)
			{
				int size = file->GetSize();
				uint8_t* buffer = new uint8_t[size];
				file->Read(buffer, size);
				file->Destroy();
				TBOOL bRes = ReadHeader(buffer);

				if (bRes)
				{
					for (size_t i = 0; i < m_oHeader.m_usNumSections; i++)
					{
						TTODO("Finish LoadXUIB2");
					}
				}
			}
		}
	}

	TBOOL TXUIResource::Load(uint8_t* buffer)
	{
		Destroy();
		ReadHeader(buffer);

		int totalSize = m_oHeader.m_usNumSections * sizeof(Section) + 4;

		for (size_t i = 0; i < m_oHeader.m_usNumSections; i++)
		{
			auto& section = m_oHeader.m_apSections[i];
			uint8_t* currentSectionBuffer = buffer + section.m_uiOffset;

			if (section.m_uiSectionID == IDXURQUAT)
			{
				ReadQuatSection(currentSectionBuffer, section.m_uiSize);
				totalSize += (section.m_uiSize / sizeof(TQuaternion)) * 4 + 4;
			}
			else if (section.m_uiSectionID == IDXURCUST)
			{
				ReadCustSection(currentSectionBuffer, section.m_uiSize);
			}
			else if (section.m_uiSectionID == IDXURDATA)
			{
				ReadDataSection(currentSectionBuffer, section.m_uiSize);
			}
			else if (section.m_uiSectionID == IDXURSTRING)
			{
				ReadStringSection(currentSectionBuffer, section.m_uiSize);
				totalSize += GetStringTableSize(currentSectionBuffer, section.m_uiSize);
			}
			else if (section.m_uiSectionID == IDXURVEC)
			{
				totalSize += (section.m_uiSize / sizeof(TVector3)) * 8 + 0xC;
			}
		}

		return TTRUE;
	}

	TBOOL TXUIResource::ReadDataSection(uint8_t* buffer, uint32_t size)
	{
		XURReader reader(buffer);

		uint16_t rootNameId = reader.ReadUInt16();
		uint8_t opcode = reader.ReadUInt8();

		TASSERT(0 == TStringManager::String16Compare(GetString(rootNameId), _TS16("XuiCanvas")), "First Element is not XuiCanvas!");
		m_pRoot = CreateObjectData(*this, rootNameId);
		m_pRoot->Load(*this, buffer);

		if ((opcode & 2) != 0)
		{
			m_pRoot->LoadChildren(*this, buffer);
		}

		if ((opcode & 4) != 0 && m_pRoot->LoadNamedFrames(*this, buffer) && (opcode & 2) != 0)
		{
			m_pRoot->LoadTimelines(*this, buffer);
		}

		return TTRUE;
	}

	TBOOL TXUIResource::ReadStringSection(uint8_t* buffer, uint32_t size)
	{
		TASSERT(TNULL == m_asStringTable, "StringTable must not be initialized");
		XURReader reader(buffer);

		uint8_t* pStart = buffer;
		uint8_t* pEnd = buffer + size;
		m_uiStringCount = 1;

		while (reader.GetPos() < pEnd)
		{
			uint16_t stringLength = reader.ReadUInt16();
			reader.SeekFromCur(stringLength * sizeof(wchar_t));
			m_uiStringCount++;
		}

		reader.SetPos(pStart);

		m_asStringTable       = new (TXUI::MemoryBlock()) wchar_t*[m_uiStringCount];
		m_asStringTable[0]    = new (TXUI::MemoryBlock()) wchar_t[1];
		m_asStringTable[0][0] = L'\0';

		for (size_t i = 1; i < m_uiStringCount; i++)
		{
			TASSERT(buffer < pEnd, "Pointer overflow");

			uint16_t stringLength = reader.ReadUInt16();
			m_asStringTable[i] = new (TXUI::MemoryBlock()) wchar_t[stringLength + 1];
			m_asStringTable[i][stringLength] = L'\0';

			for (size_t j = 0; j < stringLength; j++)
			{
				m_asStringTable[i][j] = TSTATICCAST(wchar_t, reader.ReadUInt16());
			}
		}

		return TTRUE;
	}

	TBOOL TXUIResource::ReadCustSection(uint8_t* buffer, uint32_t size)
	{
		m_pCustData = new (TXUI::MemoryBlock()) uint8_t[size];
		TUtil::MemCopy(m_pCustData, buffer, size);
		m_uiCustDataSize = size;
		return TTRUE;
	}

	TBOOL TXUIResource::ReadQuatSection(uint8_t* buffer, uint32_t size)
	{
		XURReader reader(buffer);
		m_uiQuatCount = size / sizeof(TQuaternion);

		m_aqZRotTable = new (TXUI::MemoryBlock()) float[m_uiQuatCount];

		for (size_t i = 0; i < m_uiQuatCount; i++)
		{
			TQuaternion quaternion = {
				reader.ReadFloat(),
				reader.ReadFloat(),
				reader.ReadFloat(),
				reader.ReadFloat()
			};

			TVector3 euler;
			quaternion.GetEulerXYZ(euler);

			m_aqZRotTable[i] = -euler.z;
			TASSERT(m_aqZRotTable[i] >= -TMath::PI && m_aqZRotTable[i] <= TMath::PI);
		}

		return TTRUE;
	}

	int TXUIResource::GetStringTableSize(uint8_t* pPtr, uint32_t size)
	{
		XURReader reader(pPtr);

		uint8_t* pStart = pPtr;
		uint8_t* pEnd = pPtr + size;
		int iStringCount = 1;

		while (reader.GetPos() < pEnd)
		{
			uint16_t stringLength = reader.ReadUInt16();
			reader.SeekFromCur(stringLength * sizeof(wchar_t));
			iStringCount++;
		}

		reader.SetPos(pStart);

		int result = iStringCount * 4 + 8;
		int iStringsLeft = iStringCount - 1;

		while (iStringsLeft != 0)
		{
			TASSERT(reader.GetPos() < pEnd);

			uint16_t stringSize = reader.ReadUInt16() * sizeof(wchar_t);
			reader.SeekFromCur(stringSize);

			result += (stringSize + 5U & 0xfffffffc);
			iStringsLeft--;
		}

		return result;
	}

	void TXUIResource::PushID(const wchar_t* a_wsID)
	{
		m_oIDComparisonStack.Push(a_wsID);

		if (m_ID.Length() != 0)
			m_ID.Concat(_TS16("."), -1);
		
		m_ID.Concat(a_wsID, -1);
	}

	void TXUIResource::PopID()
	{
		const wchar_t* top = m_oIDComparisonStack.Top();
		m_oIDComparisonStack.Pop();

		m_ID = m_ID.Left(m_ID.Length() - TStringManager::String16Length(top));
	}

	TXUIStringTable& TXUIResource::LookupStringTable()
	{
		if (m_uiStringCount != 0)
		{
			//TXUIStringTable::Lookup()
		}
		auto lal = TXUIStringTable();

		return lal;
		// TODO: insert return statement here
	}

	XURXUIObjectData* TXUIResource::CreateObjectData(TXUIResource& a_rResource, uint16_t index)
	{
		if (index == 0) return TNULL;
		return CreateObjectData(a_rResource, a_rResource.m_asStringTable[index]);
	}

	XURXUIObjectData* TXUIResource::CreateObjectData(TXUIResource& a_rResource, const wchar_t* objectName)
	{
		if (TStringManager::String16Compare(objectName, _TS16("XuiCanvas"), -1) == 0)
		{
			return new (TXUI::MemoryBlock()) XURXUICanvasData();
		}
		else if (TStringManager::String16Compare(objectName, _TS16("XuiScene"), -1) == 0)
		{
			return new (TXUI::MemoryBlock()) XURXUISceneData();
		}
		else if (TStringManager::String16Compare(objectName, _TS16("XuiTabScene"), -1) == 0)
		{
			return new (TXUI::MemoryBlock()) XURXUITabSceneData();
		}
		else if (TStringManager::String16Compare(objectName, _TS16("XuiFigure"), -1) == 0)
		{
			return new (TXUI::MemoryBlock()) XURXUIFigureData();
		}
		else if (TStringManager::String16Compare(objectName, _TS16("XuiVisual"), -1) == 0)
		{
			TASSERT(TFALSE, "Some XUI class is not implemented, can't continue reading data");
			return TNULL;
		}
		else if (TStringManager::String16Compare(objectName, _TS16("XuiImage"), -1) == 0)
		{
			return new (TXUI::MemoryBlock()) XURXUIImageData();
		}
		else if (TStringManager::String16Compare(objectName, _TS16("XuiText"), -1) == 0)
		{
			return new (TXUI::MemoryBlock()) XURXUITextData();
		}
		else if (TStringManager::String16Compare(objectName, _TS16("XuiButton"), -1) == 0)
		{
			return new (TXUI::MemoryBlock()) XURXUIButtonData();
		}
		else if (TStringManager::String16Compare(objectName, _TS16("XuiGroup"), -1) == 0)
		{
			return new (TXUI::MemoryBlock()) XURXUIGroupData();
		}
		else if (TStringManager::String16Compare(objectName, _TS16("XuiImagePresenter"), -1) == 0)
		{
			TASSERT(TFALSE, "Some XUI class is not implemented, can't continue reading data");
			return TNULL;
		}
		else if (TStringManager::String16Compare(objectName, _TS16("XuiTextPresenter"), -1) == 0)
		{
			TASSERT(TFALSE, "Some XUI class is not implemented, can't continue reading data");
			return TNULL;
		}
		else if (TStringManager::String16Compare(objectName, _TS16("XuiNineGrid"), -1) == 0)
		{
			return new (TXUI::MemoryBlock()) XURXUINineGridData();
		}
		else if (TStringManager::String16Compare(objectName, _TS16("XuiSoundXAudio"), -1) == 0)
		{
			TASSERT(TFALSE, "Some XUI class is not implemented, can't continue reading data");
			return TNULL;
		}
		else if (TStringManager::String16Compare(objectName, _TS16("XuiSoundXAct"), -1) == 0)
		{
			TASSERT(TFALSE, "Some XUI class is not implemented, can't continue reading data");
			return TNULL;
		}
		else if (TStringManager::String16Compare(objectName, _TS16("XuiLabel"), -1) == 0)
		{
			return new (TXUI::MemoryBlock()) XURXUILabelData();
		}
		else if (TStringManager::String16Compare(objectName, _TS16("XuiEdit"), -1) == 0)
		{
			TASSERT(TFALSE, "Some XUI class is not implemented, can't continue reading data");
			return TNULL;
		}
		else if (TStringManager::String16Compare(objectName, _TS16("XuiMessageBoxButton"), -1) == 0)
		{
			TASSERT(TFALSE, "Some XUI class is not implemented, can't continue reading data");
			return TNULL;
		}
		else if (TStringManager::String16Compare(objectName, _TS16("XuiNavButton"), -1) == 0)
		{
			TASSERT(TFALSE, "Some XUI class is not implemented, can't continue reading data");
			return TNULL;
		}
		else if (TStringManager::String16Compare(objectName, _TS16("XuiBackButton"), -1) == 0)
		{
			return new (TXUI::MemoryBlock()) XURXUIBackButtonData();
		}
		else if (TStringManager::String16Compare(objectName, _TS16("XuiRadioButton"), -1) == 0)
		{
			TASSERT(TFALSE, "Some XUI class is not implemented, can't continue reading data");
			return TNULL;
		}
		else if (TStringManager::String16Compare(objectName, _TS16("XuiCheckbox"), -1) == 0)
		{
			return new (TXUI::MemoryBlock()) XURXUICheckBoxData();
		}
		else if (TStringManager::String16Compare(objectName, _TS16("XuiProgressbar"), -1) == 0)
		{
			TASSERT(TFALSE, "Some XUI class is not implemented, can't continue reading data");
			return TNULL;
		}
		else if (TStringManager::String16Compare(objectName, _TS16("XuiSlider"), -1) == 0)
		{
			return new (TXUI::MemoryBlock()) XURXUISliderBarData();
		}
		else if (TStringManager::String16Compare(objectName, _TS16("XuiList"), -1) == 0)
		{
			return new (TXUI::MemoryBlock()) XURXUIListData();
		}
		else if (TStringManager::String16Compare(objectName, _TS16("XuiCommonList"), -1) == 0)
		{
			TASSERT(TFALSE, "Some XUI class is not implemented, can't continue reading data");
			return TNULL;
		}
		else if (TStringManager::String16Compare(objectName, _TS16("XuiRadioGroup"), -1) == 0)
		{
			TASSERT(TFALSE, "Some XUI class is not implemented, can't continue reading data");
			return TNULL;
		}
		else if (TStringManager::String16Compare(objectName, _TS16("XuiScrollEnd"), -1) == 0)
		{
			TASSERT(TFALSE, "Some XUI class is not implemented, can't continue reading data");
			return TNULL;
		}
		else if (TStringManager::String16Compare(objectName, _TS16("XuiCaret"), -1) == 0)
		{
			TASSERT(TFALSE, "Some XUI class is not implemented, can't continue reading data");
			return TNULL;
		}
		else if (TStringManager::String16Compare(objectName, _TS16("XuiListItem"), -1) == 0)
		{
			TASSERT(TFALSE, "Some XUI class is not implemented, can't continue reading data");
			return TNULL;
		}
		else if (TStringManager::String16Compare(objectName, _TS16("XuiHtmlPresenter"), -1) == 0)
		{
			TASSERT(TFALSE, "Some XUI class is not implemented, can't continue reading data");
			return TNULL;
		}
		else if (TStringManager::String16Compare(objectName, _TS16("XuiHtmlElement"), -1) == 0)
		{
			TASSERT(TFALSE, "Some XUI class is not implemented, can't continue reading data");
			return TNULL;
		}
		else if (TStringManager::String16Compare(objectName, _TS16("XuiHtmlControl"), -1) == 0)
		{
			TASSERT(TFALSE, "Some XUI class is not implemented, can't continue reading data");
			return TNULL;
		}
		else if (TStringManager::String16Compare(objectName, _TS16("XuiControl"), -1) == 0)
		{
			return new (TXUI::MemoryBlock()) XURXUIControlData();
		}
		else if (TStringManager::String16Compare(objectName, _TS16("XuiVideo"), -1) == 0)
		{
			TASSERT(TFALSE, "Some XUI class is not implemented, can't continue reading data");
			return TNULL;
		}

		return TNULL;
	}

	void TXUIResource::Destroy()
	{
		if (m_asStringTable)
		{
			for (size_t i = 0; i < m_uiStringCount; i++)
				delete m_asStringTable[i];

			delete[] m_asStringTable;
		}

		if (m_avVectors)
			delete[] m_avVectors;

		if (m_aqZRotTable)
			delete[] m_aqZRotTable;

		if (m_pRoot)
			delete m_pRoot;
	}

}