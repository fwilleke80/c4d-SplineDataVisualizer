#ifndef COMMONS_H__
#define COMMONS_H__

namespace SplineDataVisualizationHelpers
{
	// Plugin IDs
	static const Int32 ID_OBJECTDATA_SPLINEDATASPLINE = 1054229;
	static const Int32 ID_OBJECTDATA_SPLINEDATAAXISSPLINE = 1054271;
	static const Int32 ID_COMMANDDATA_SPLINEDATAVIS = 1054272;
	static const Int32 ID_SPLINEDATAVIS_SEPARATOR = 1054273;

	// General defaults
	const Float DEFAULT_WIDTH = 200.0;
	const Float DEFAULT_HEIGHT = 50.0;

	// SplineDataSpline defaults
	const UInt32 DEFAULT_SPLINEDATA_SUBDIVISION = 100;

	// SplineDataAxis defaults
	const Float DEFAULT_AXIS_OVERSHOOT = 2.0;
	const Bool DEFAULT_AXIS_ARROWS = true;
	const Float DEFAULT_AXIS_ARROW_HEIGHT = 5.0;
	const Float DEFAULT_AXIS_ARROW_WIDTH = 5.0;
	const Float DEFAULT_AXIS_ARROW_OVERSHOOT = 10.0;

	// Visualization components defaults
	const Float DEFAULT_VIS_SPLINEDATA_PROFILE_RADIUS = 0.5;
	const Float DEFAULT_VIS_AXIS_PROFILE_RADIUS = 0.6;
	const Vector DEFAULT_VIS_SPLINEDATA_COLOR = Vector(1.0);
	const Vector DEFAULT_VIS_AXIS_COLOR = Vector(0.0);
	const Vector DEFAULT_VIS_TEXT_COLOR = Vector(1.0);
	const Float DEFAULT_VIS_TEXT_HEIGHT = 15.0;
	const Float DEFAULT_VIS_TEXT_OFFSET = 2.0;
	const Float DEFAULT_VIS_TEXT_DEPTH = 0.0;
}

#endif // COMMONS_H__
