#pragma once

class THLabyrinth2 : public IPlugin
{
public:
	THLabyrinth2();
	~THLabyrinth2();

	void init();
	void terminate();
	void run(int arg);

private:
	void MeasureFilter();
	void InitNameMap();
	FILE* OpenSaveFile(std::string &filePath);
	void LoadStrRefMap(StrRefMap& refMap);
	void ExportStrList(const StrRefMap& refMap, FILE* hFile);
	bool CheckSkipString(const qwstring& str);

protected:
	bool GetTalkInfo(ea_t eaRef, int& nFunc, BYTE& nChara);
	void AddChara(uint id, LPCWSTR name);
	bool IsValidChar(WCHAR ch);
	bool CanSkipChar(WCHAR ch);

private:
	static std::unordered_set<WCHAR> m_filter;
	static std::map<uint, qstring> m_nameMap;

};

