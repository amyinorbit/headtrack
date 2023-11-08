#pragma once

#include "XPLMDataAccess.h"
#include "XPLMUtilities.h"


/*
* 

Author: Mark Ellis
		Stick and Rudder Studios, LLC
		mark@StickandRudderStudios.com

Current Version 1.0 Final

This class implememts the interface to X-Camera's headtracking API.

Call isEnabled() to determine if X-Camera is installed and currently enabled.

If X-Camera is enabled then call setOffsets with the X,Y,Z, and camera rotation offsets when in a 3DCockpit or Free camera view mode.

*/

class HeadData
{
public:
    
    float X,Y,Z;
    float Pitch,Yaw,Roll;
    
};

class X_Camera
{

public:
	X_Camera()
	{

		gXCameraStatus = nullptr;
		gHeadTrackingPresent = nullptr;
		gHeadTrackingX = nullptr;
		gHeadTrackingY = nullptr;
		gHeadTrackingZ = nullptr;
		gHeadTrackingPitch = nullptr;
		gHeadTrackingYaw = nullptr;
		gHeadTrackingRoll = nullptr;

		isInitialized = false;

	}

	bool isEnabled()
	{
		if (!isInitialized)
		{
			init();		// Get all the dataref references
		}

		if (gXCameraStatus != nullptr)	// If the X-Camera status dataref exitis then X-Camera is installed
		{
			int status = XPLMGetDatai(gXCameraStatus);	// Get the current status of X-Camera
			return status & 1;	// Enabled if bit 0 is set
		}

		return false;	// No X-Camera Installed
	}

	void setOffsets(HeadData& data)
	{
		XPLMSetDatai(gHeadTrackingPresent, 1);		// Turn on Head Tracking and set all the offsets
		XPLMSetDataf(gHeadTrackingX, data.X);
		XPLMSetDataf(gHeadTrackingY, data.Y);
		XPLMSetDataf(gHeadTrackingZ, data.Z);
		XPLMSetDataf(gHeadTrackingPitch, data.Pitch);
		XPLMSetDataf(gHeadTrackingYaw, data.Yaw);
		XPLMSetDataf(gHeadTrackingRoll, data.Roll * -1.0f);	// Roll angle is backwards so multiply by -1.0
	}

private:

	XPLMDataRef      gHeadTrackingPresent;
	XPLMDataRef      gHeadTrackingX;
	XPLMDataRef      gHeadTrackingY;
	XPLMDataRef      gHeadTrackingZ;
	XPLMDataRef      gHeadTrackingPitch;
	XPLMDataRef      gHeadTrackingYaw;
	XPLMDataRef      gHeadTrackingRoll;
	XPLMDataRef      gXCameraStatus;
	bool			 isInitialized;

	void init()
	{
		gXCameraStatus = XPLMFindDataRef("SRS/X-Camera/integration/overall_status");
		gHeadTrackingPresent = XPLMFindDataRef("SRS/X-Camera/integration/headtracking_present");
		gHeadTrackingX = XPLMFindDataRef("SRS/X-Camera/integration/headtracking_x_offset");
		gHeadTrackingY = XPLMFindDataRef("SRS/X-Camera/integration/headtracking_y_offset");
		gHeadTrackingZ = XPLMFindDataRef("SRS/X-Camera/integration/headtracking_z_offset");
		gHeadTrackingPitch = XPLMFindDataRef("SRS/X-Camera/integration/headtracking_pitch_offset");
		gHeadTrackingYaw = XPLMFindDataRef("SRS/X-Camera/integration/headtracking_heading_offset");
		gHeadTrackingRoll = XPLMFindDataRef("SRS/X-Camera/integration/headtracking_roll_offset");

		XPLMDebugString("Head Track X-Camera integration is available\n");
		isInitialized = true;
	}
};
