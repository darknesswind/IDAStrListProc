#pragma once
#include <unordered_map>

template <typename _Ele>
class DictTree
{
protected:
	struct Node
	{
		std::unordered_map<_Ele, Node> children;
		bool bExist = false;
	};
public:
	DictTree() = default;
	~DictTree() = default;

	void insert(const _Ele* pBegin, const _Ele* pEnd)
	{
		if (!pBegin || pEnd < pBegin)
			return;

		Node* pNode = &root;
		for (_Ele* iter = pBegin; iter != pEnd; ++iter)
		{
			pNode = &pNode->children[*iter];
		}
		pNode->bExist = true;
	}

protected:
	Node root;
};


template <typename _CHAR>
class _StrDictTree : public DictTree<_CHAR>
{
public:
	_StrDictTree() = default;
	~_StrDictTree() = default;

	void insert(const _CHAR* lpcstr)
	{
		if (!lpcstr)
			return;

		Node* pNode = &root;
		for (const _CHAR* pChar = lpcstr; *pChar != 0; ++pChar)
		{
			pNode = &pNode->children[*pChar];
		}
		pNode->bExist = true;
	}

	bool contains(const _CHAR* lpcstr)
	{
		if (!lpcstr)
			return false;

		Node* pNode = &root;
		_CHAR ch = 0;
		for (const _CHAR* pChar = lpcstr; (ch = *pChar); ++pChar)
		{
			auto& children = pNode->children;
			auto iter = children.find(ch);
			if (iter != children.end())
				pNode = &iter->second;
			else
				return false;
		}
		return pNode->bExist;
	}
private:

};

typedef _StrDictTree<char> str_dict;
typedef _StrDictTree<wchar_t> wstr_dict;