/******************************************************************************

	dvemgr.h

******************************************************************************/

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

	int pspDveMgrCheckVideoOut();
	int pspDveMgrSetVideoOut(int u, int displaymode, int width, int height, int x, int y, int z);

#ifdef __cplusplus
}
#endif