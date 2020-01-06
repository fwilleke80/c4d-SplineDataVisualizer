#include "main.h"
#include "commons.h"

Bool PluginStart()
{
	if (!RegisterSplineDataVisCommand())
		return false;
	if (!RegisterSplineDataSpline())
		return false;
	if (!RegisterSplineDataAxisSpline())
		return false;
	if (!RegisterFaceCameraTag())
		return false;

	return true;
}

void PluginEnd()
{
}

Bool PluginMessage(Int32 id, void* data)
{
	switch (id)
	{
		case C4DPL_INIT_SYS:
			if (!g_resource.Init())
				return false;		// don't start plugin without resource
			return true;
	}

	return false;
}
