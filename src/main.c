#include "htrack.h"
#include "paths.h"
#include <XPLMPlugin.h>
#include <XPLMUtilities.h>
#include <XPLMDisplay.h>
#include <XPLMProcessing.h>
#include <XPLMDataAccess.h>
#include <string.h>
#include <stdio.h>
#include <acfutils/log.h>
#include <acfutils/crc64.h>
#include <acfutils/glew.h>
#include <acfutils/time.h>

#if IBM==1
#include <windows.h>
BOOL APIENTRY DllMain( HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    UNUSED(hModule);
    UNUSED(ul_reason_for_call);
    UNUSED(lpReserved);
    return TRUE;
}
#endif

PLUGIN_API float flight_loop(float since_last, float since_last_fl, int count, void *refcon) {
    UNUSED(since_last);
    UNUSED(since_last_fl);
    UNUSED(count);
    UNUSED(refcon);
    htk_frame();
    return -1;
}

PLUGIN_API int XPluginStart(char * outName, char * outSig, char *outDesc) {
    // Plugin details
    strcpy(outName, "HeadTrack");
    strcpy(outSig, "com.amyinorbit.htrack");
    strcpy(outDesc, "Lightweight head tracking plugin");
    XPLMEnableFeature("XPLM_USE_NATIVE_PATHS", 1);

    log_init(XPLMDebugString, "headtrack");
    logMsg("HeadTrack - version %s - Amy Alex Parent", HTK_VERSION);
    crc64_init();
    crc64_srand(microclock());
    glewInit();

    xpath_reload();
    htk_setup();
    return 1;
}

PLUGIN_API void	XPluginStop(void) {
    htk_cleanup();
}

PLUGIN_API int XPluginEnable(void) {
    logMsg("starting...");
    XPLMRegisterFlightLoopCallback(flight_loop, 0.5f, NULL);
    return htk_start();
}

PLUGIN_API void XPluginDisable(void) {
    logMsg("stopping");
    XPLMUnregisterFlightLoopCallback(flight_loop, NULL);
    htk_stop();
}

PLUGIN_API void XPluginReceiveMessage(XPLMPluginID id, intptr_t inMessage, void * inParam) {
    UNUSED(id);
    UNUSED(inParam);

    if(inMessage != XPLM_MSG_PLANE_LOADED) return;
    xpath_reload();
    htk_plane_did_load();
}

