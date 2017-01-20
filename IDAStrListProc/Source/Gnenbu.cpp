#include "stdafx.h"
#include "Gnenbu.h"
#include <array>
#include "tools.h"

Gnenbu::Gnenbu()
	: m_opt(optDefault)
{
	m_filter.insert(L"ｰ・");
	m_filter.insert(L"ﾜ踰");
	m_filter.insert(L"犲V");
}

Gnenbu::~Gnenbu()
{
}

void Gnenbu::init()
{

}

void Gnenbu::terminate()
{

}

void Gnenbu::run(int arg)
{
	char form[] = "Game Text Dumper:\n"
		"Action:\n"
		"<Export:R>\n"
		"<Import:R>>\n"
		"<Filter out ascii string:C>>\n";

	ushort sel = 0;
	int res = AskUsingForm_c(form, &sel, &m_opt);
	if (res)
	{
		if (0 == sel)
			Export();
		else
			Import();
	}
}

void Gnenbu::Export()
{
	std::string filePath;
	FILE* hFile = OpenSaveFile(filePath);
	if (!hFile)
	{
		msg("Open File \"%s\" Failed!\n", filePath.c_str());
		return;
	}
	const WORD bom = 0xFEFF;
	ewrite(hFile, &bom, sizeof(bom));

	const int cp_shift_jis = 932;
	CodePageKeeper codepage(cp_shift_jis);


	try
	{
		ExportStrList(hFile);
	}
	catch (...)
	{
		msg("Error!\n");
	}

	eclose(hFile);
	msg("Saved To %s\n", filePath.c_str());
}

FILE* Gnenbu::OpenSaveFile(std::string &filePath)
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
	return fopenWB(filePath.c_str());
}

void Gnenbu::EncodeCtrlChar(qwstring& wstr)
{
	for (size_t i = 0; i < wstr.size(); ++i)
	{
		switch (wstr[i])
		{
		case L'\\':
			wstr.insert(i, L'\\');
			++i;
			break;
		case L'\n':
			wstr.insert(i, L'\\');
			wstr[++i] = L'n';
			break;
		default:
			break;
		}
	}
}

void Gnenbu::ReadFilterConfig(ea_t& defBegin, ea_t& defEnd)
{
	std::string filePath;
	ssize_t filePathLen = get_input_file_path(NULL, 0);
	filePath.resize(filePathLen, '\0');
	get_input_file_path(&filePath[0], filePathLen);
	if (filePath.back() == '\0')
		filePath.pop_back();
	filePath.append(".ini");

	ea_t offBegin = 0;
	ea_t offEnd = defEnd - defBegin;

	char buff[0x100] = { 0 };
	if (GetPrivateProfileStringA("filter", "begin", "", buff, 0x100, filePath.c_str()))
		atob(buff, &offBegin);

	if (GetPrivateProfileStringA("filter", "end", "", buff, 0x100, filePath.c_str()))
		atob(buff, &offEnd);

	defEnd = defBegin + offEnd;
	defBegin = defBegin + offBegin;
}

void Gnenbu::ExportStrList(FILE* hFile)
{
	ea_t eaBegin = 0;
	ea_t eaEnd = 0x7FFFFFFF;
	segment_t* pRData = get_segm_by_name(".rdata");
	if (!pRData)
		return;
	
	eaBegin = pRData->startEA;
	eaEnd = pRData->endEA;
	refresh_strlist(eaBegin, eaEnd);

	ReadFilterConfig(eaBegin, eaEnd);

	RawStrReader reader(eaBegin, eaEnd);
	while (reader.readNext())
	{
		ea_t offset = reader.curAddr() - pRData->startEA;

		ByteArray& dat = reader.getBytes();
		bool bLatin = true;
		if (!CheckShiftJIS(dat, bLatin))
		{
			ea_t addr = FindXRefString(reader, bLatin);
			if (!addr)
				continue;

			offset = addr - pRData->startEA;
		}

		if (bLatin && (m_opt & optFilterOutAscii))
		{
			if (memcmp("jpn", dat.data(), dat.size()) &&
				memcmp("japanese", dat.data(), dat.size()))
				continue;
		}

		qwstring wstr;
		dat.push_back(0);
		if (!c2ustr((const char*)dat.data(), &wstr))
			continue;

		if (m_filter.contains(wstr.c_str()))
			continue;

		EncodeCtrlChar(wstr);
		{
			qstring suffix;
			suffix.sprnt("%08a,", offset);

			qwstring wsuffix;
			c2ustr(suffix.c_str(), &wsuffix);
			ewrite(hFile, wsuffix.c_str(), (wsuffix.size() - 1) * sizeof(wchar16_t));
		}
		ewrite(hFile, wstr.c_str(), (wstr.size() - 1) * sizeof(wchar16_t));
		ewrite(hFile, L"\r\n", 4);
	}
}

bool Gnenbu::CheckShiftJIS(ByteArray& bytes, bool& bAllLatin)
{
	BYTE* pBegin = bytes.data();
	BYTE* pEnd = pBegin + bytes.size();
	return CheckShiftJIS(pBegin, pEnd, bAllLatin);
}

bool Gnenbu::CheckShiftJIS(BYTE* pBegin, BYTE* pEnd, bool& bAllLatin)
{
	/*
	以下字元在Shift_JIS使用一个字节来表示。

	ASCII字符（0x20-0x7E），但“\”被“¥”取代
	ASCII控制字符（0x00-0x1F、0x7F）
	JIS X 0201标准内的半角标点及片假名（0xA1-0xDF）
	在部分操作系统中，0xA0用来放置“不换行空格”。

	以下字元在Shift_JIS使用两个字节来表示。

	JIS X 0208字集的所有字符
	“第一位字节”使用0x81-0x9F、0xE0-0xEF（共47个）
	“第二位字节”使用0x40-0x7E、0x80-0xFC（共188个）
	使用者定义区
	“第一位字节”使用0xF0-0xFC（共13个）
	“第二位字节”使用0x40-0x7E、0x80-0xFC（共188个）

	在Shift_JIS编码表中，并未使用0xFD、0xFE及0xFF。

	在微软及IBM的日语电脑系统中，在0xFA、0xFB及0xFC的两字节区域，加入了388个JIS X 0208没有收录的符号和汉字。
	*/

	BYTE* pByte = pBegin;
	bAllLatin = true;
	bool hasMultiByte = false;
	uint length = 0;
	while (pByte < pEnd)
	{
		bool bMultiByte = false;
		BYTE first = *pByte;
		if (0 == first)
		{
			break;
		}
		else if (first < 0x20)	// ASCII控制字符
		{
			if (first != '\n')
				return false;
		}
		else if (first < 0x80)	// 0x20 - 0x7F ASCII字符
		{
			if (0x7F == first)	// DEL
				return false;
		}
		else if (first < 0xA0)	// 0x80 - 0x9F
		{
			if (0x80 == first)	// Unused
				return false;
			bMultiByte = true;
		}
		else if (first < 0xE0)	// 0xA0 - 0xDF 半角标点及片假名
		{
			if (0xA0 == first)	// Unused
				return false;
			bAllLatin = false;
		}
		else if (first < 0xF0)	// 0xE0 - 0xEF
		{
			bMultiByte = true;
		}
		else
		{
			return false;
		}

		// 处理第二字节
		if (bMultiByte)
		{
			bAllLatin = false;
			hasMultiByte = true;
			if (++pByte >= pEnd)
				return false;

			BYTE second = *pByte;
			if (second < 0x40)
			{
				return false;
			}
			else if (second < 0x9F)
			{
				if (0x7F == second)
					return false;

				// 				if (0 == (first & 1))
				// 					return false;
			}
			else if (second < 0xFD)
			{
				// 				if (first & 1)
				// 					return false;
			}
			else
			{
				return false;
			}
		}
		++length;
		++pByte;
	}

	if (!hasMultiByte && !bAllLatin && length <= 3)
		return false;

	return true;
}

ea_t Gnenbu::FindXRefString(RawStrReader& reader, bool& bAllLatin)
{
	ByteArray& dat = reader.getBytes();
	ea_t eaBegin = reader.curAddr();
	ea_t eaEnd = eaBegin + dat.size();
	BYTE* pData = dat.data();

	std::vector<ea_t> xrefAddrs;
	for (ea_t addr = eaBegin + 1; addr < eaEnd; ++addr)
	{
		xrefblk_t xb;
		if (xb.first_to(addr, XREF_ALL))
			xrefAddrs.push_back(addr);
	}
	if (xrefAddrs.empty())
		return 0;

	xrefAddrs.push_back(eaEnd);
	for (size_t i = 0; i < xrefAddrs.size() - 1; ++i)
	{
		BYTE* pBegin = pData + (xrefAddrs[i] - eaBegin);
		BYTE* pEnd = pData + (xrefAddrs[i + 1] - eaBegin);
		bool bLatin = true;
		if (!CheckShiftJIS(pBegin, pEnd, bLatin))
			continue;

		if (bLatin)
			continue;

		dat.swap(ByteArray(pBegin, pEnd));
		bAllLatin = bLatin;
		return xrefAddrs[i];
	}

	return 0;
}

void Gnenbu::Import()
{
	char* fileName = askfile2_c(false, NULL, "*.utf16", "select input");
	if (!fileName)
		return;

	FILE* hFile = fopenRB(fileName);
	if (!hFile)
		return;

	const int cp_gb2312 = 936;
	CodePageKeeper codepage(cp_gb2312);

	try
	{
		ImportStrList(hFile);
	}
	catch (...)
	{
		msg("Error!\n");
	}

	eclose(hFile);
}

void Gnenbu::ImportStrList(FILE* hFile)
{
	segment_t* pRData = get_segm_by_name(".rdata");
	if (!pRData)
		return;
	ea_t eaBegin = pRData->startEA;

	Utf16Reader reader(hFile);
	while (reader.readNext())
	{
		qwstring& wstr = reader.getLine();
		DecodeCtrlChar(wstr);

		qstring cstr;
		if (!u2cstr(wstr.c_str(), &cstr))
			continue;

		qstring sAddr = cstr.substr(0, 8);
		sAddr.append(L'h');
		ea_t addr = 0;
		atob(sAddr.c_str(), &addr);
		addr += eaBegin;

		cstr.remove(0, 9);
		if (!CanPatchString(addr, cstr))
			continue;

		patch_many_bytes(addr, cstr.c_str(), cstr.size());
	}
}

void Gnenbu::DecodeCtrlChar(qwstring& wstr)
{
	for (uint i = 0; i < wstr.size(); ++i)
	{
		if (L'\\' == wstr[i])
		{
			wstr.remove(i, 1);
			if (i >= wstr.size())
				return;

			switch (wstr[i])
			{
			case L'r':
				wstr[i] = L'\r';
				break;
			case L'n':
				wstr[i] = L'\n';
				break;
			default:
				break;
			}
		}
	}
}

bool Gnenbu::CanPatchString(ea_t addr, const qstring& str)
{
	std::vector<char> buff(str.size());
	if (!get_many_bytes(addr, buff.data(), buff.size()))
		return false;

	uint strEnd = 0;
	for (strEnd = 0; strEnd < buff.size(); ++strEnd)
	{
		if (!buff[strEnd])
			break;
	}
	if (strEnd >= str.size() - 1)
		return true;
	
	for (; strEnd < buff.size(); ++strEnd)
	{
		if (buff[strEnd])
			return false;
	}

	return true;
	int res = askyn_c(ASKBTN_YES, "QUESTION\nAUTOHIDE SESSION\nHIDECANCEL\nString too long at %a: %s", addr, str.c_str());
	return (ASKBTN_YES == res);
}
