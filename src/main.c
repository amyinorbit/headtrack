#include "htrack.h"
#include <XPLMPlugin.h>
#include <XPLMUtilities.h>
#include <XPLMDisplay.h>
#include <XPLMProcessing.h>
#include <XPLMDataAccess.h>
#include <string.h>
#include <stdio.h>
#include <ccore/log.h>


#if IBM==1
#include <windows.h>
BOOL APIENTRY DllMain( HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    CCUNUSED(hModule);
    CCUNUSED(ul_reason_for_call);
    CCUNUSED(lpReserved);
    return TRUE;
}
#endif

PLUGIN_API float flight_loop(float since_last, float since_last_fl, int count, void *refcon) {
    CCUNUSED(since_last);
    CCUNUSED(since_last_fl);
    CCUNUSED(count);
    CCUNUSED(refcon);

    htk_frame();
    return -1;
}


static FILE* file_out = NULL;
void log_printer(const char *msg) {
    if(!file_out) {
        file_out = fopen("htrack.log", "w");
    }
    fprintf(file_out, "%s", msg);
    fflush(file_out);
    XPLMDebugString(msg);
}

PLUGIN_API int XPluginStart(char * outName, char * outSig, char *outDesc) {
    // Plugin details
    strcpy(outName, "HeadTrack");
    strcpy(outSig, "com.amyinorbit.htrack");
    strcpy(outDesc, "Lightweight head tracking plugin");
    XPLMEnableFeature("XPLM_USE_NATIVE_PATHS", 1);
    
    cc_set_log_name("headtrack");
    cc_set_printer(log_printer);
    CCINFO("HeadTrack - version 2011.1");

    htk_setup();
    return 1;
}

PLUGIN_API void	XPluginStop(void) {
    htk_cleanup();
}

PLUGIN_API int XPluginEnable(void) {
    CCINFO("starting...");
    XPLMRegisterFlightLoopCallback(flight_loop, 0.5f, NULL);
    htk_start();
	return 1;
}

PLUGIN_API void XPluginDisable(void) {
    CCINFO("stopping");
    XPLMUnregisterFlightLoopCallback(flight_loop, NULL);
    htk_stop();
}

PLUGIN_API void XPluginReceiveMessage(XPLMPluginID id, intptr_t inMessage, void * inParam) {
    CCUNUSED(id);
    CCUNUSED(inParam);

    if(inMessage != XPLM_MSG_PLANE_LOADED) return;
    htk_reset_default_head();
}
