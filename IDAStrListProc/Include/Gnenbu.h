#pragma once
#include "DictTree.h"
typedef std::vector<BYTE> ByteArray;
class RawStrReader;
class Gnenbu : public IPlugin
{
public:
	Gnenbu();
	~Gnenbu();

public:
	void init();
	void terminate();
	void run(int arg);

protected:
	void Export();
	void ExportStrList(FILE* hFile);
	bool CheckShiftJIS(ByteArray& bytes, bool& bAllLatin);
	bool CheckShiftJIS(BYTE* pBegin, BYTE* pEnd, bool& bAllLatin);
	ea_t FindXRefString(RawStrReader& reader, bool& bAllLatin);
	FILE* OpenSaveFile(std::string &filePath);
	void EncodeCtrlChar(qwstring& wstr);

	void Import();
	void ImportStrList(FILE* hFile);
	void DecodeCtrlChar(qwstring& dat);
	bool CanPatchString(ea_t addr, const qstring& str);


private:
	StrRefMap m_refMap;
	wstr_dict m_filter;
};

