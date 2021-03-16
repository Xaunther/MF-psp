/******************************************************************************

	homehook.prx

******************************************************************************/

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif
	void initHomeButton(int devkit_version);
	unsigned int readHomeButton(void);
#ifdef __cplusplus
}
#endif