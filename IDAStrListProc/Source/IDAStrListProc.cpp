///////////////////////////////////////////////////////////////////////////////
//
//  File     : IDAStrListProc.cpp
//  Author   : darknesswind
//  Date     : 04/04/2015
//  Homepage : http://.
//  
//  License  : Copyright ?2015 
//
//  This software is provided 'as-is', without any express or
//  implied warranty. In no event will the authors be held liable
//  for any damages arising from the use of this software.
//
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
#include "stdafx.h"
#include "IDAStrListProc.h"
#include "Gnenbu.h"

// Global Variables:
int gSdkVersion;
char gszVersion[]      = "1.0.0.1";
// Plugin name listed in (Edit | Plugins)
char gszWantedName[]   = "Game Text Dumper";
// plug-in hotkey
char gszWantedHotKey[] = "";

char *gszPluginHelp;
char *gszPluginComment;



bool GetKernelVersion(char *szBuf, int bufSize)
{
	int major, minor, len;
	get_kernel_version(szBuf, bufSize);
	if ( qsscanf(szBuf, "%d.%n%d", &major, &len, &minor) != 2 )
		return false;
	if ( isdigit(szBuf[len + 1]) )
		gSdkVersion = 100*major + minor;
	else
		gSdkVersion = 10 * (10*major + minor);
	return true;
}

//-----------------------------------------------------------------------------
// Function: init
//
// init is a plugin_t function. It is executed when the plugin is
// initially loaded by IDA.
// Three return codes are possible:
//    PLUGIN_SKIP - Plugin is unloaded and not made available
//    PLUGIN_KEEP - Plugin is kept in memory
//    PLUGIN_OK   - Plugin will be loaded upon 1st use
//
// Check are added here to ensure the plug-in is compatible with
// the current disassembly.
//-----------------------------------------------------------------------------
int initPlugin(void)
{
	char szBuffer[MAXSTR];
	char sdkVersion[32];
	int nRetCode = PLUGIN_OK;
	HINSTANCE hInstance = ::GetModuleHandle(NULL);

	// Initialize global strings
	LoadString(hInstance, IDS_PLUGIN_HELP, szBuffer, sizeof(szBuffer));
	gszPluginHelp = qstrdup(szBuffer);
	LoadString(hInstance, IDS_PLUGIN_COMMENT, szBuffer, sizeof(szBuffer));
	gszPluginComment = qstrdup(szBuffer);
	if ( !GetKernelVersion(sdkVersion, sizeof(sdkVersion)) )
	{
		msg("%s: could not determine IDA version\n", gszWantedName);
		nRetCode = PLUGIN_SKIP;
	}
	else if ( gSdkVersion < 650 )
	{
		warning("Sorry, the %s plugin required IDA v%s or higher\n", gszWantedName, sdkVersion);
		nRetCode = PLUGIN_SKIP;
	}
	else if ( ph.id != PLFM_386 || ( !inf.is_32bit() && !inf.is_64bit() ) || inf.like_binary() )
	{
		msg("%s: could not load plugin\n", gszWantedName);
		nRetCode = PLUGIN_SKIP;
	}
	else
	{
		msg( "%s (v%s) plugin has been loaded\n"
			"  The hotkeys to invoke the plugin is %s.\n"
			"  Please check the Edit/Plugins menu for more informaton.\n",
			gszWantedName, gszVersion, gszWantedHotKey);
	}
	return nRetCode;
}

//-----------------------------------------------------------------------------
// Function: term
//
// term is a plugin_t function. It is executed when the plugin is
// unloading. Typically cleanup code is executed here.
//-----------------------------------------------------------------------------
void termPlugin(void)
{
}

//-----------------------------------------------------------------------------
// Function: run
//
// run is a plugin_t function. It is executed when the plugin is run.
//
// The argument 'arg' can be passed by adding an entry in
// plugins.cfg or passed manually via IDC:
//
//   success RunPlugin(string name, long arg);
//-----------------------------------------------------------------------------
void runPlugin(int arg)
{
//  Uncomment the following code to allow plugin unloading.
//  This allows the editing/building of the plugin without
//  restarting IDA.
//
//  1. to unload the plugin execute the following IDC statement:
//        RunPlugin("IDAStrListProc", 415);
//  2. Make changes to source code and rebuild within Visual Studio
//  3. Copy plugin to IDA plugin dir
//     (may be automatic if option was selected within wizard)
//  4. Run plugin via the menu, hotkey, or IDC statement
//
	if (arg == 415)
	{
		PLUGIN.flags |= PLUGIN_UNL;
		msg("Unloading IDAStrListProc plugin...\n");
		return;
	}

	char form[] = "Game Text Dumper:\n"
		"Game:\n"
		"<General:R>\n"
		"<THLabyrinth2:R>>\n";

	ushort sel = 0;
	int res = AskUsingForm_c(form, &sel);
	if (res)
	{
		switch (sel)
		{
		case 0:
		default:
			Gnenbu plugin;
			plugin.run(arg);
			break;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
//
//                         PLUGIN DESCRIPTION BLOCK
//
///////////////////////////////////////////////////////////////////////////////
plugin_t PLUGIN =
{
  IDP_INTERFACE_VERSION,
  0,              // plugin flags
  initPlugin,           // initialize
  termPlugin,           // terminate. this pointer may be NULL.
  runPlugin,            // invoke plugin
  gszPluginComment,     // comment about the plugin
  gszPluginHelp,        // multiline help about the plugin
  gszWantedName,        // the preferred short name of the plugin
  gszWantedHotKey       // the preferred hotkey to run the plugin
};

