/*
 * TeamSpeak 3 Troll Hunter Plugin
 *
 *	Thanks for the help to:
 *
 *  - CoolerDing (Twitch-Chat)
 *  - Tango2K (Idea)
 *  - StackOverflow
 *  - TeamSpeak-Doku
 *   - Carsten612 (Twitch-Chat)
 *  - Github <3 
 * 
 * Project on Github: https://github.com/DrOpossum/TeamSpeak-Addon_Troll-Hunter
 */

#ifdef _WIN32
#pragma warning (disable : 4100)  /* Disable Unreferenced parameter warning */
#include <Windows.h>
#endif

#include <stdlib.h>
#include <string>
#include <regex>
#include <iterator>
#include <assert.h>
#include <iostream>
#include "teamspeak/public_errors.h"
#include "teamspeak/public_errors_rare.h"
#include "teamspeak/public_definitions.h"
#include "teamspeak/public_rare_definitions.h"
#include "teamspeak/clientlib_publicdefinitions.h"
#include "ts3_functions.h"
#include "plugin.h"

static struct TS3Functions ts3Functions;

#ifdef _WIN32
#define _strcpy(dest, destSize, src) strcpy_s(dest, destSize, src)
#define snprintf sprintf_s
#else
#define _strcpy(dest, destSize, src) { strncpy(dest, src, destSize-1); (dest)[destSize-1] = '\0'; }
#endif

#define PLUGIN_API_VERSION 22

#define PATH_BUFSIZE 512
#define COMMAND_BUFSIZE 128
#define INFODATA_BUFSIZE 128
#define SERVERINFO_BUFSIZE 256
#define CHANNELINFO_BUFSIZE 512
#define RETURNCODE_BUFSIZE 128

static char* pluginID = NULL;

#ifdef _WIN32
/* Helper function to convert wchar_T to Utf-8 encoded strings on Windows */
static int wcharToUtf8(const wchar_t* str, char** result) {
	int outlen = WideCharToMultiByte(CP_UTF8, 0, str, -1, 0, 0, 0, 0);
	*result = (char*)malloc(outlen);
	if(WideCharToMultiByte(CP_UTF8, 0, str, -1, *result, outlen, 0, 0) == 0) {
		*result = NULL;
		return -1;
	}
	return 0;
}
#endif

/*********************************** Required functions ************************************/
/*
 * If any of these required functions is not implemented, TS3 will refuse to load the plugin
 */

/* Unique name identifying this plugin */
const char* ts3plugin_name() {
#ifdef _WIN32
	/* TeamSpeak expects UTF-8 encoded characters. Following demonstrates a possibility how to convert UTF-16 wchar_t into UTF-8. */
	static char* result = NULL;  /* Static variable so it's allocated only once */
	if(!result) {
		const wchar_t* name = L"Troll Hunter - Link Security ";
		if(wcharToUtf8(name, &result) == -1) {  /* Convert name into UTF-8 encoded result */
			result = "Troll Hunter Link Security ";  /* Conversion failed, fallback here */
		}
	}
	return result;
#else
	return "Troll Hunter - Link Security";
#endif
}

/* Plugin version */
const char* ts3plugin_version() {
    return "1.0";
}

/* Plugin API version. Must be the same as the clients API major version, else the plugin fails to load. */
int ts3plugin_apiVersion() {
	return PLUGIN_API_VERSION;
}

/* Plugin author */
const char* ts3plugin_author() {
	/* If you want to use wchar_t, see ts3plugin_name() on how to use */
    return "DrOpossum and Twitch-Chat";
}

/* Plugin description */
const char* ts3plugin_description() {
	/* If you want to use wchar_t, see ts3plugin_name() on how to use */
    return "This Plugin tells you where BBCode-Links are really going.";
}

/* Set TeamSpeak 3 callback functions */
void ts3plugin_setFunctionPointers(const struct TS3Functions funcs) {
    ts3Functions = funcs;
}

/*
 * Custom code called right after loading the plugin. Returns 0 on success, 1 on failure.
 * If the function returns 1 on failure, the plugin will be unloaded again.
 */
int ts3plugin_init() {
    char appPath[PATH_BUFSIZE];
    char resourcesPath[PATH_BUFSIZE];
    char configPath[PATH_BUFSIZE];
	char pluginPath[PATH_BUFSIZE];

    /* Your plugin init code here */
    printf("Troll Hunter: init\n");

    /* Example on how to query application, resources and configuration paths from client */
    /* Note: Console client returns empty string for app and resources path */
    ts3Functions.getAppPath(appPath, PATH_BUFSIZE);
    ts3Functions.getResourcesPath(resourcesPath, PATH_BUFSIZE);
    ts3Functions.getConfigPath(configPath, PATH_BUFSIZE);
	ts3Functions.getPluginPath(pluginPath, PATH_BUFSIZE, pluginID);

    return 0;  /* 0 = success, 1 = failure, -2 = failure but client will not show a "failed to load" warning */
	/* -2 is a very special case and should only be used if a plugin displays a dialog (e.g. overlay) asking the user to disable
	 * the plugin again, avoiding the show another dialog by the client telling the user the plugin failed to load.
	 * For normal case, if a plugin really failed to load because of an error, the correct return value is 1. */
}

/* Custom code called right before the plugin is unloaded */
void ts3plugin_shutdown() {
    /* Your plugin cleanup code here */
    printf("Troll Hunter: shutdown\n");

	/*
	 * Note:
	 * If your plugin implements a settings dialog, it must be closed and deleted here, else the
	 * TeamSpeak client will most likely crash (DLL removed but dialog from DLL code still open).
	 */

	/* Free pluginID if we registered it */
	if(pluginID) {
		free(pluginID);
		pluginID = NULL;
	}
}

int ts3plugin_onTextMessageEvent(uint64 serverConnectionHandlerID, anyID targetMode, anyID toID, anyID fromID, const char* fromName, const char* fromUniqueIdentifier, const char* message, int ffIgnored)
{

	if (strlen(message) > 0) {

		std::string msg(message);
		std::regex msgregex("\\[url=http.*\\]http.*\\[\\/url\\]");

		if (std::regex_match(msg, msgregex)) {

			std::vector<std::string> url;
			std::vector<std::string> text;

			std::regex urlregex("\\[url\\]([^\\[\\s]*)\\[\\/url\\]");
			std::sregex_token_iterator passt(msg.begin(), msg.end(), urlregex, 1);
			std::sregex_token_iterator passt_nicht;

			if (passt != passt_nicht) {

				url.push_back(passt->str());
				text.push_back("");

			}

			std::regex linkregex("\\[url\\=([^\\]\\s]*)\\]([^\\[]*)\\[\\/url\\]");
			std::sregex_token_iterator passt_link1(msg.begin(), msg.end(), linkregex, 1);
			std::sregex_token_iterator passt_link2(msg.begin(), msg.end(), linkregex, 2);


			if (passt_link1 != passt_nicht) {

				url.push_back(passt_link1->str());

			}

			if (passt_link2 != passt_nicht) {

				text.push_back(passt_link2->str());

			}

			for (int i = url.size() - 1; i >= 0; i--){

				if (text[i] != url[i]) {

					std::string nachricht = "[b]Troll-Hunter[/b]: Careful, URL [b]" + text[i] + "[/b] is fake! It leads to [url]" + url[i] + "[/url]!";
					ts3Functions.printMessageToCurrentTab(nachricht.c_str());

				}
			
			}
		}

	}

	return 0;

}
