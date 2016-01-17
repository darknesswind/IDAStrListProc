#pragma once

#ifdef __NT__
#include <windows.h>
#endif

#include <ida.hpp>
#include <idp.hpp>
#include <loader.hpp>
#include <kernwin.hpp>
#include "resource.h"

#include "strList.hpp"
#include "xref.hpp"
#include "bytes.hpp"
#include "diskio.hpp"
#include "segment.hpp"
#include "ua.hpp"
#include "allins.hpp"
#include "funcs.hpp"

#include <map>
#include <unordered_set>
#include <vector>

__interface IPlugin
{
	void init();
	void terminate();
	void run(int arg);
};

typedef std::vector<string_info_t> StrInfoList;
typedef std::map<ea_t, StrInfoList> StrRefMap;

class CodePageKeeper
{
	int oldCP = CP_OEMCP;
public:
	CodePageKeeper(int cp)
	{
		get_codepages(&oldCP);
		set_codepages(cp, CP_OEMCP);
	}
	~CodePageKeeper()
	{
		set_codepages(oldCP, CP_OEMCP);
	}
};
