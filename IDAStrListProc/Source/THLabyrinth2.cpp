#include "stdafx.h"
#include "THLabyrinth2.h"

std::unordered_set<WCHAR> THLabyrinth2::m_filter;
std::map<uint, qstring> THLabyrinth2::m_nameMap;

THLabyrinth2::THLabyrinth2()
{
}

THLabyrinth2::~THLabyrinth2()
{
}

void THLabyrinth2::init()
{

}

void THLabyrinth2::terminate()
{

}

void THLabyrinth2::run(int arg)
{
	StrRefMap refMap;
	LoadStrRefMap(refMap);

	std::string filePath;
	FILE* hFile = OpenSaveFile(filePath);
	if (!hFile)
	{
		msg("Open File \"%s\" Failed!\n", filePath.c_str());
		return;
	}

	const int cp_shift_jis = 932;
	const int cp_gb2312 = 936;
	int oemcp = CP_OEMCP;
	get_codepages(&oemcp);
	set_codepages(cp_shift_jis, CP_OEMCP);

	InitNameMap();
	MeasureFilter();
	try
	{
		ExportStrList(refMap, hFile);
	}
	catch (...)
	{
		msg("Error!\n");
	}
	set_codepages(oemcp, CP_OEMCP);

	eclose(hFile);
	msg("Saved To %s\n", filePath.c_str());
}


FILE* THLabyrinth2::OpenSaveFile(std::string &filePath)
{
	if (filePath.empty())
	{
		ssize_t filePathLen = get_input_file_path(NULL, 0);
		filePath.resize(filePathLen, '\0');
		get_input_file_path(&filePath[0], filePathLen);
		if (filePath.back() == '\0')
			filePath.pop_back();
		filePath.append(".txt");
	}
	return fopenWT(filePath.c_str());
}

void THLabyrinth2::LoadStrRefMap(StrRefMap& refMap)
{
	ea_t eaBegin = 0;
	ea_t eaEnd = 0x7FFFFFFF;
	segment_t* pRData = get_segm_by_name(".rdata");
	if (pRData)
	{
		eaBegin = pRData->startEA;
		eaEnd = pRData->endEA;
	}

	refresh_strlist(eaBegin, eaEnd);
	size_t count = get_strlist_qty();
	string_info_t info;
	for (size_t i = 0; i < count; ++i)
	{
		get_strlist_item(i, &info);
		if (info.ea < eaBegin || info.ea > eaEnd)
			continue;

		xrefblk_t xb;
		bool bHasRef = false;
		for (bool ok = xb.first_to(info.ea, XREF_ALL); ok; ok = xb.next_to())
		{
			bHasRef = true;
			refMap[xb.from].push_back(info);
			break;
		}
		if (!bHasRef)
		{
			refMap[0xFFFFFFFF].push_back(info);
		}
	}
}

bool THLabyrinth2::GetTalkInfo(ea_t eaRef, int& nFunc, BYTE& nChara)
{
	bool bIsTalk = false;
	ea_t pushPos = eaRef - 2;

	int instLen = decode_insn(eaRef);
	eaRef += cmd.size;
	instLen = decode_insn(eaRef);
	eaRef += cmd.size;
	instLen = decode_insn(eaRef);
	if (NN_call == cmd.itype)
	{
		segment_t* pSeg = getseg(eaRef);
		ea_t addrOffset = cmd.Op1.addr - pSeg->startEA;
		if (addrOffset == 0x0176716D - 0x0175D000)
		{
			instLen = decode_insn(pushPos);
			if (NN_push == cmd.itype && o_imm == cmd.Op1.type)
			{
				nChara = cmd.Op1.value;
				nFunc = get_func_num(eaRef);
				bIsTalk = true;
			}
		}
	}
	return bIsTalk;
}

void THLabyrinth2::AddChara(uint id, LPCWSTR name)
{
	unicode_utf8(&m_nameMap[id], name, -1);
}

void THLabyrinth2::InitNameMap()
{
	if (!m_nameMap.empty())
		return;

	AddChara(1, L"ë‘‰ô");
	AddChara(2, L"Ä§ÀíÉ³");
	AddChara(3, L"ÁØÖ®Öú");
	AddChara(4, L"»ÛÒô");
	AddChara(5, L"—É");
	AddChara(6, L"Ñý‰ô");
	AddChara(7, L"Ð¡‚ã");
	AddChara(8, L"Â¶Ã×ÑÇ");
	AddChara(9, L"ç÷Â¶Åµ");
	AddChara(10, L"·y×Ó");
	AddChara(11, L"Ð¡î®");
	AddChara(12, L"³È");
	AddChara(13, L"ºÉÈ¡");
	AddChara(14, L"ÅÁÂ¶Î÷");
	AddChara(15, L"Àò¸ñÂ¶");
	AddChara(16, L"ÝxÒ¹");
	AddChara(17, L"ÃÃ¼t");
	AddChara(18, L"ÎÄÎÄ");
	AddChara(19, L"Ò¹È¸");
	AddChara(20, L"ÈAÉÈ");
	AddChara(21, L"ÄÈ×ÈÁÕ");
	AddChara(22, L"ër");
	AddChara(23, L"Ÿû");
	AddChara(24, L"¿Õ");
	AddChara(25, L"¾õ");
	AddChara(26, L"ÓÂƒx");
	AddChara(27, L"ÃÀâ");
	AddChara(28, L"°®ÀöË¿");
	AddChara(29, L"ÅÁç÷");
	AddChara(30, L"ÓÀÁÕ");
	AddChara(31, L"âÏÉ");
	AddChara(32, L"ÔçÃç");
	AddChara(33, L"ÒÂ¾Á");
	AddChara(34, L"ÝÍÏã");
	AddChara(35, L"Ë{");
	AddChara(36, L"ÀÙÃ×");
	AddChara(37, L"†DÒ¹");
	AddChara(38, L"ÉñÄÎ×Ó");
	AddChara(39, L"ÕŒÔL×Ó");
	AddChara(40, L"Ìì×Ó");
	AddChara(41, L"Ü½À¼");
	AddChara(42, L"ÓÄ¡©×Ó");
	AddChara(43, L"ÓÄÏã");
	AddChara(44, L"×Ï");
	AddChara(45, L"°×É");
	AddChara(46, L"Ó³Šª");
	AddChara(47, L"Á«×Ó");
	AddChara(48, L"Ã·Àò");
	AddChara(101, L"°¢Çó");
	AddChara(102, L"Ð¡‚ã");
	AddChara(103, L"Ìì×Ó");
	AddChara(105, L"éL½­ÒÂ¾Á");
	AddChara(106, L"Ìì×Ó");
	AddChara(108, L"Ìì…²ë…„‡");
}

void THLabyrinth2::ExportStrList(const StrRefMap& refMap, FILE* hFile)
{
	for (auto iter = refMap.cbegin(); iter != refMap.cend(); ++iter)
	{
		BYTE nChara = 0xFF;
		int nFunc = -1;
		bool bIsTalk = GetTalkInfo(iter->first, nFunc, nChara);

		const StrInfoList& infoList = iter->second;
		for (auto iterInfo = infoList.cbegin(); iterInfo != infoList.cend(); ++iterInfo)
		{
			const string_info_t& strInfo = *iterInfo;
			std::string str(strInfo.length, '\0');
			get_many_bytes(strInfo.ea, &str[0], strInfo.length);

			qwstring wstr;
			c2ustr(str.c_str(), &wstr);
			if (CheckSkipString(wstr))
				continue;

			qstring utf8str;
			unicode_utf8(&utf8str, wstr.c_str(), -1);
			if (!utf8str.empty())
			{
				if (bIsTalk)
				{
					if (m_nameMap.find(nChara) != m_nameMap.end())
					{
						qstring suffix;
						suffix.sprnt("<%4X, %s>", nFunc, m_nameMap[nChara].c_str());
						ewrite(hFile, suffix.c_str(), suffix.size() - 1);
					}
					else
					{
						qstring suffix;
						suffix.sprnt("<%4X, %2d>", nFunc, nChara);
						ewrite(hFile, suffix.c_str(), suffix.size() - 1);
					}
				}
				ewrite(hFile, utf8str.c_str(), utf8str.size() - 1);
				ewrite(hFile, "\n", 1);
			}
		}
	}
}

void THLabyrinth2::MeasureFilter()
{
	if (!m_filter.empty())
		return;

	for (WCHAR i = 1; i < 0x20; ++i)
	{
		m_filter.insert(i);
	}

	m_filter.erase(L'\r');
	m_filter.erase(L'\n');
	m_filter.erase(L'\t');

	m_filter.insert(0xF8F3);
}

bool THLabyrinth2::IsValidChar(WCHAR ch)
{
	if (m_filter.find(ch) != m_filter.end())
		return false;
	else
		return true;
}

bool THLabyrinth2::CanSkipChar(WCHAR ch)
{
	if (ch < 0x7F)	// ascii char
		return true;

	return false;
}

bool THLabyrinth2::CheckSkipString(const qwstring& str)
{
	bool bSkipStr = true;
	for (uint i = 0; i < str.size(); ++i)
	{
		if (!IsValidChar(str[i]))
			return true;

		if (!CanSkipChar(str[i]))
			bSkipStr = false;
	}
	return bSkipStr;
}
