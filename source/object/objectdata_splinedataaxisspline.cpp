// Includes from API
#include "c4d_baseobject.h"
#include "c4d_includes.h"
#include "c4d_objectdata.h"

// Includes from plugin project
#include "c4d_symbols.h"
#include "main.h"

// Local resources
#include "osplinedataaxisspline.h"
#include "c4d_resource.h"

// Common values
#include "commons.h"


namespace SplineDataVisualizationHelpers
{
	//----------------------------------------------------------------------------------------
	/// Struct to hold spline generation parameters
	//----------------------------------------------------------------------------------------
	struct SplineDataAxisSplineParameters
	{
		Float height;
		Float width;
		Float overshoot;
		Bool arrows;
		Float arrowHeight;
		Float arrowWidth;
		Float arrowOvershoot;

		Bool closed;
		SPLINETYPE splineType;
		Int32 splineInterpolation;
		Int32 splineSubdivision;
		Float splineSubAngle;
		Float splineSubMaxLength;

		/// \brief Default constructor
		SplineDataAxisSplineParameters() : height(0.0), width(0.0), overshoot(0.0), arrows(false),
			arrowHeight(0.0), arrowWidth(0.0), arrowOvershoot(0.0),
			closed(false), splineType(SPLINETYPE::LINEAR),
			splineInterpolation(SPLINEOBJECT_INTERPOLATION_NONE),
			splineSubdivision(0), splineSubAngle(0.0),
			splineSubMaxLength(0.0)
		{
		}

		/// \brief Contruct from values
		SplineDataAxisSplineParameters(Float _height,
																	 Float _width,
																	 Float _overshoot,
																	 Bool _arrows,
																	 Float _arrowHeight,
																	 Float _arrowWidth,
																	 Float _arrowOvershoot,
																	 Bool _closed,
																	 SPLINETYPE _splineType,
																	 Int32 _splineInterpolation,
																	 Int32 _splineSubdivision,
																	 Float _splineSubAngle,
																	 Float _splineSubMaxLength) :
			height(_height), width(_width),
			overshoot(_overshoot), arrows(_arrows),
			arrowHeight(_arrowHeight), arrowWidth(_arrowWidth),
			arrowOvershoot(_arrowOvershoot),
			closed(_closed), splineType(_splineType),
			splineInterpolation(_splineInterpolation),
			splineSubdivision(_splineSubdivision),
			splineSubAngle(_splineSubAngle),
			splineSubMaxLength(_splineSubMaxLength)
		{
		}
	};

	/// \brief Return number of required spline segments for a spline with or without arrows
	inline UInt32 GetNumberOfRequiredSplineSegments(Bool arrows)
	{
		return arrows ? 4 : 2;
	}

	/// \brief Return number of required spline points for a spline with or without arrows
	inline UInt32 GetNumberOfRequiredSplinePoints(Bool arrows)
	{
		return arrows ? 10 : 4;
	}

	//----------------------------------------------------------------------------------------
	/// Global function responsible to create the spline points.
	/// @brief Global function responsible to create the spline points.
	/// @param[out] splineObj					SplineObject that will receive the resulting points
	/// @param[in] params							Parameter set for spline generation
	/// @return												True if creation process succeeds
	//----------------------------------------------------------------------------------------
	static maxon::Result<void> CreateSplineDataAxisSpline(SplineObject& splineObj, SplineDataAxisSplineParameters &params)
	{
		// Set the closed/open status of the SplineObject instance accessing its BaseContainer instance
		BaseContainer* splineObjBCPtr = splineObj.GetDataInstance();
		if (splineObjBCPtr)
		{
			splineObjBCPtr->SetBool(SPLINEOBJECT_CLOSED, params.closed);
			splineObjBCPtr->SetInt32(SPLINEOBJECT_INTERPOLATION, params.splineInterpolation);
			splineObjBCPtr->SetInt32(SPLINEOBJECT_SUB, params.splineSubdivision);
			splineObjBCPtr->SetFloat(SPLINEOBJECT_ANGLE, params.splineSubAngle);
			splineObjBCPtr->SetFloat(SPLINEOBJECT_MAXIMUMLENGTH, params.splineSubMaxLength);
		}

		// Set the number of segments
		const Int32 segmentCount = GetNumberOfRequiredSplineSegments(params.arrows);
		if (!splineObj.MakeVariableTag(Tsegment, segmentCount))
			return maxon::UnexpectedError(MAXON_SOURCE_LOCATION);

		// Access the writable array of the points representing the curve passing points
		Vector* splinePntsPtr = splineObj.GetPointW();
		if (nullptr == splinePntsPtr)
			return maxon::NullptrError(MAXON_SOURCE_LOCATION);

		// Set spline points

		// Some value caching
		const Float widthPlusArrowOvershoot = params.width + params.arrowOvershoot + params.arrowHeight * 0.25;
		const Float heightPlusArrowOvershoot = params.height + params.arrowOvershoot + params.arrowHeight * 0.25;
		const Float halfArrowHeight = params.arrowHeight * 0.5;
		const Float halfArrowWidth = params.arrowWidth * 0.5;

		// X axis
		splinePntsPtr[0] = Vector(-params.overshoot, 0.0, 0.0);
		splinePntsPtr[1] = Vector(widthPlusArrowOvershoot, 0.0, 0.0);

		// Y axis
		splinePntsPtr[2] = Vector(0.0, -params.overshoot, 0.0);
		splinePntsPtr[3] = Vector(0.0, heightPlusArrowOvershoot, 0.0);

		if (params.arrows)
		{
			// X arrow
			splinePntsPtr[4] = Vector(widthPlusArrowOvershoot - halfArrowHeight, halfArrowWidth, 0.0);
			splinePntsPtr[5] = Vector(widthPlusArrowOvershoot + halfArrowHeight, 0.0, 0.0);
			splinePntsPtr[6] = Vector(widthPlusArrowOvershoot - halfArrowHeight, -halfArrowWidth, 0.0);

			// Y arrow
			splinePntsPtr[7] = Vector(-halfArrowWidth, heightPlusArrowOvershoot - halfArrowHeight, 0.0);
			splinePntsPtr[8] = Vector(0.0, heightPlusArrowOvershoot + halfArrowHeight, 0.0);
			splinePntsPtr[9] = Vector(halfArrowWidth, heightPlusArrowOvershoot - halfArrowHeight, 0.0);
		}

		// Access the curve's segments array.
		Segment* splineSegsPtr = splineObj.GetSegmentW();
		if (nullptr == splineSegsPtr)
			return maxon::NullptrError(MAXON_SOURCE_LOCATION);

		// Set the closure status and the number of CVs for the only one segment existing.
		splineSegsPtr[0].closed = false;
		splineSegsPtr[0].cnt = 2;

		splineSegsPtr[1].closed = false;
		splineSegsPtr[1].cnt = 2;

		// If arrows are desired, set their segment data, too
		if (params.arrows)
		{
			splineSegsPtr[2].closed = false;
			splineSegsPtr[2].cnt = 3;

			splineSegsPtr[3].closed = false;
			splineSegsPtr[3].cnt = 3;
		}

		return maxon::OK;
	}
}


//------------------------------------------------------------------------------------------------
/// ObjectData implementation generating a pair of coordinate system axes
//------------------------------------------------------------------------------------------------
class SplineDataAxisSplineObject : public ObjectData
{
	INSTANCEOF(SplineDataAxisSplineObject, ObjectData)

public:
	virtual Bool Init(GeListNode* node);
	virtual void GetDimension(BaseObject* op, Vector* mp, Vector* rad);
	virtual SplineObject* GetContour(BaseObject* op, BaseDocument* doc, Float lod, BaseThread* bt);
	Bool GetDEnabling(GeListNode *node, const DescID &id, const GeData &t_data, DESCFLAGS_ENABLE flags, const BaseContainer *itemdesc);

	static NodeData* Alloc()
	{
		return NewObj(SplineDataAxisSplineObject) iferr_ignore("SplineDataAxisSplineObject plugin not instanced");
	}
};

Bool SplineDataAxisSplineObject::Init(GeListNode* node)
{
	if (!node)
		return false;

	// Retrieve the BaseContainer object belonging to the generator.
	BaseObject*		 baseObjectPtr = static_cast<BaseObject*>(node);
	BaseContainer* objectDataPtr = baseObjectPtr->GetDataInstance();

	// Fill object container with initial values
	objectDataPtr->SetFloat(OSPLINEDATAAXIS_WIDTH, SplineDataVisualizationHelpers::DEFAULT_WIDTH);
	objectDataPtr->SetFloat(OSPLINEDATAAXIS_HEIGHT, SplineDataVisualizationHelpers::DEFAULT_HEIGHT);
	objectDataPtr->SetFloat(OSPLINEDATAAXIS_OVERSHOOT, SplineDataVisualizationHelpers::DEFAULT_AXIS_OVERSHOOT);
	objectDataPtr->SetBool(OSPLINEDATAAXIS_ARROWS, SplineDataVisualizationHelpers::DEFAULT_AXIS_ARROWS);
	objectDataPtr->SetFloat(OSPLINEDATAAXIS_ARROW_HEIGHT, SplineDataVisualizationHelpers::DEFAULT_AXIS_ARROW_HEIGHT);
	objectDataPtr->SetFloat(OSPLINEDATAAXIS_ARROW_WIDTH, SplineDataVisualizationHelpers::DEFAULT_AXIS_ARROW_WIDTH);
	objectDataPtr->SetFloat(OSPLINEDATAAXIS_ARROW_OVERSHOOT, SplineDataVisualizationHelpers::DEFAULT_AXIS_ARROW_OVERSHOOT);

	// SplineObject standard parameters
	objectDataPtr->SetInt32(SPLINEOBJECT_TYPE, SPLINEOBJECT_TYPE_LINEAR);
	objectDataPtr->SetBool(SPLINEOBJECT_CLOSED, false);
	objectDataPtr->SetInt32(SPLINEOBJECT_INTERPOLATION, SPLINEOBJECT_INTERPOLATION_NATURAL);
	objectDataPtr->SetInt32(SPLINEOBJECT_SUB, 8);
	objectDataPtr->SetFloat(SPLINEOBJECT_ANGLE, DegToRad(5.0));
	objectDataPtr->SetFloat(SPLINEOBJECT_MAXIMUMLENGTH, 5.0);

	return true;
}

void SplineDataAxisSplineObject::GetDimension(BaseObject *op, Vector *mp, Vector *rad)
{
	// Check the passed pointers.
	if (!op || ! mp || !rad)
		return;

	// Reset the barycenter position and the bbox radius vector.
	mp->SetZero();
	rad->SetZero();

	// Set the barycenter position to match the generator center.
	const Vector objGlobalOffset = op->GetMg().off;
	mp->x = objGlobalOffset.x;
	mp->y = objGlobalOffset.y;
	mp->z = objGlobalOffset.z;

	// Retrieve the BaseContainer object belonging to the generator.
	BaseContainer* objectDataPtr = op->GetDataInstance();
	if (!objectDataPtr)
		return;

	// Set radius values accordingly to the bbox values stored during the init.
	const Float a = Max(objectDataPtr->GetFloat(OSPLINEDATAAXIS_OVERSHOOT), objectDataPtr->GetBool(OSPLINEDATAAXIS_ARROWS) ? (objectDataPtr->GetFloat(OSPLINEDATAAXIS_ARROW_WIDTH) * 0.5) : 0.0) + objectDataPtr->GetFloat(OSPLINEDATAAXIS_ARROW_OVERSHOOT) + objectDataPtr->GetFloat(OSPLINEDATAAXIS_ARROW_HEIGHT) * 0.5;
	rad->x = objectDataPtr->GetFloat(OSPLINEDATAAXIS_WIDTH) + a;
	rad->y = objectDataPtr->GetFloat(OSPLINEDATAAXIS_HEIGHT) + a;
}

SplineObject* SplineDataAxisSplineObject::GetContour(BaseObject *op, BaseDocument *doc, Float lod, BaseThread *bt)
{
	// Check the passed pointer.
	if (!op)
		return nullptr;

	// Retrieve the BaseContainer object belonging to the generator.
	BaseContainer* objectDataPtr = op->GetDataInstance();

	// Fill the retrieve BaseContainer object with initial values.
	const Float width = objectDataPtr->GetFloat(OSPLINEDATAAXIS_WIDTH, SplineDataVisualizationHelpers::DEFAULT_WIDTH);
	const Float height = objectDataPtr->GetFloat(OSPLINEDATAAXIS_HEIGHT, SplineDataVisualizationHelpers::DEFAULT_HEIGHT);
	const Float overshoot = objectDataPtr->GetFloat(OSPLINEDATAAXIS_OVERSHOOT, SplineDataVisualizationHelpers::DEFAULT_AXIS_OVERSHOOT);
	const Bool arrows = objectDataPtr->GetBool(OSPLINEDATAAXIS_ARROWS, SplineDataVisualizationHelpers::DEFAULT_AXIS_ARROWS);
	const Float arrowHeight = objectDataPtr->GetFloat(OSPLINEDATAAXIS_ARROW_HEIGHT, SplineDataVisualizationHelpers::DEFAULT_AXIS_ARROW_HEIGHT);
	const Float arrowWidth = objectDataPtr->GetFloat(OSPLINEDATAAXIS_ARROW_WIDTH, SplineDataVisualizationHelpers::DEFAULT_AXIS_ARROW_WIDTH);
	const Float arrowOvershoot = objectDataPtr->GetFloat(OSPLINEDATAAXIS_ARROW_OVERSHOOT, SplineDataVisualizationHelpers::DEFAULT_AXIS_ARROW_OVERSHOOT);

	const Bool closed = objectDataPtr->GetBool(SPLINEOBJECT_CLOSED, false);
	const SPLINETYPE splineType = (SPLINETYPE)objectDataPtr->GetInt32(SPLINEOBJECT_TYPE, SPLINEOBJECT_TYPE_LINEAR);
	const Int32 splineInterpolation = objectDataPtr->GetInt32(SPLINEOBJECT_INTERPOLATION, SPLINEOBJECT_INTERPOLATION_NONE);
	const Int32 splineSubdivision = objectDataPtr->GetInt32(SPLINEOBJECT_SUB, 0);
	const Float splineAngle = objectDataPtr->GetFloat(SPLINEOBJECT_ANGLE, 0.0);
	const Float splineMaxLength = objectDataPtr->GetFloat(SPLINEOBJECT_MAXIMUMLENGTH, 0.0);

	// Set parameters to params set
	SplineDataVisualizationHelpers::SplineDataAxisSplineParameters params(height, width, overshoot, arrows, arrowHeight, arrowWidth, arrowOvershoot, closed, splineType, splineInterpolation, splineSubdivision, splineAngle, splineMaxLength);

	// Alloc a SplineObject and check it.
	SplineObject* splineObjPtr = SplineObject::Alloc(SplineDataVisualizationHelpers::GetNumberOfRequiredSplinePoints(params.arrows), params.splineType);
	if (!splineObjPtr)
		return nullptr;

	// Invoke the helper function to set the SplineObject object member accordingly
	iferr (SplineDataVisualizationHelpers::CreateSplineDataAxisSpline(*splineObjPtr, params))
	{
		DiagnosticOutput("Error on CreateSplineDataAxisSpline: @", err);
		SplineObject::Free(splineObjPtr);
		return nullptr;
	}

	splineObjPtr->Message(MSG_UPDATE);

	return splineObjPtr;
}

Bool SplineDataAxisSplineObject::GetDEnabling(GeListNode *node, const DescID &id, const GeData &t_data, DESCFLAGS_ENABLE flags, const BaseContainer *itemdesc)
{
	// Check the passed pointer.
	if (!node)
		return false;

	// Retrieve the BaseContainer object belonging to the generator.
	BaseContainer* objectDataPtr = static_cast<BaseObject*>(node)->GetDataInstance();
	if (!objectDataPtr)
		return false;

	Int32 interpolation = objectDataPtr->GetInt32(SPLINEOBJECT_INTERPOLATION);
	switch (id[0].id)
	{
		// Axis Spline attributes
		case OSPLINEDATAAXIS_ARROW_HEIGHT:
		case OSPLINEDATAAXIS_ARROW_WIDTH:
		case OSPLINEDATAAXIS_ARROW_OVERSHOOT:
			return objectDataPtr->GetBool(OSPLINEDATAAXIS_ARROWS);

		// SplineObject standard attributes
		case SPLINEOBJECT_ANGLE:
			return interpolation == SPLINEOBJECT_INTERPOLATION_ADAPTIVE || interpolation == SPLINEOBJECT_INTERPOLATION_SUBDIV;
		case SPLINEOBJECT_MAXIMUMLENGTH:
			return interpolation == SPLINEOBJECT_INTERPOLATION_SUBDIV;
		case SPLINEOBJECT_SUB:
			return interpolation == SPLINEOBJECT_INTERPOLATION_NATURAL || interpolation == SPLINEOBJECT_INTERPOLATION_UNIFORM;
	}

	return SUPER::GetDEnabling(node, id, t_data, flags, itemdesc);
}


Bool RegisterSplineDataAxisSpline()
{
	String registeredName = GeLoadString(IDS_OBJECTDATA_SPLINEDATAAXISSPLINE);
	if (!registeredName.IsPopulated())
		return false;

	return RegisterObjectPlugin(SplineDataVisualizationHelpers::ID_OBJECTDATA_SPLINEDATAAXISSPLINE, registeredName, OBJECT_GENERATOR|OBJECT_ISSPLINE, SplineDataAxisSplineObject::Alloc, "osplinedataaxisspline"_s, AutoBitmap("osplinedataaxisspline.tif"_s), 0);
}
