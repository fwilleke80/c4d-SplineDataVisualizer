#include "main.h"

Bool PluginStart()
{
	if (!RegisterSplineDataSpline())
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
