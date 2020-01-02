// Includes from API
#include "c4d_baseobject.h"
#include "c4d_includes.h"
#include "c4d_objectdata.h"

// Includes from plugin project
#include "c4d_symbols.h"
#include "main.h"

// Local resources
#include "osplinedataspline.h"
#include "c4d_resource.h"

// Common values
#include "commons.h"


namespace SplineDataVisualizationHelpers
{
	//----------------------------------------------------------------------------------------
	/// Struct to hold spline generation parameters
	//----------------------------------------------------------------------------------------
	struct SplineDataSplineParameters
	{
		Float amplitude;
		Float width;
		UInt32 samples;
		SplineData *splineData;

		Bool closed;
		SPLINETYPE splineType;
		Int32 splineInterpolation;
		Int32 splineSubdivision;
		Float splineSubAngle;
		Float splineSubMaxLength;

		/// \brief Default constructor
		SplineDataSplineParameters() : amplitude(0.0), width(0.0), samples(0), splineData(nullptr),
			closed(false), splineType(SPLINETYPE::LINEAR),
			splineInterpolation(SPLINEOBJECT_INTERPOLATION_NONE),
			splineSubdivision(0), splineSubAngle(0.0),
			splineSubMaxLength(0.0)
		{
		}

		/// \brief Contruct from values
		SplineDataSplineParameters(Float _amplitude,
															 Float _width,
															 UInt32 _samples,
															 SplineData *_splineData,
															 Bool _closed,
															 SPLINETYPE _splineType,
															 Int32 _splineInterpolation,
															 Int32 _splineSubdivision,
															 Float _splineSubAngle,
															 Float _splineSubMaxLength) :
			amplitude(_amplitude), width(_width),
			samples(_samples), splineData(_splineData),
			closed(_closed), splineType(_splineType),
			splineInterpolation(_splineInterpolation),
			splineSubdivision(_splineSubdivision),
			splineSubAngle(_splineSubAngle),
			splineSubMaxLength(_splineSubMaxLength)
		{
		}
	};

	//----------------------------------------------------------------------------------------
	/// Global function responsible to create the spline points.
	/// @brief Global function responsible to create the spline points.
	/// @param[out] splineObj					SplineObject that will receive the resulting points
	/// @param[in] params							Parameter set for spline generation
	/// @return												True if creation process succeeds
	//----------------------------------------------------------------------------------------
	static maxon::Result<void> CreateSplineDataSpline(SplineObject& splineObj, SplineDataSplineParameters &params)
	{
		if (!params.splineData || params.samples == 0)
			return maxon::IllegalArgumentError(MAXON_SOURCE_LOCATION);
		if (splineObj.GetPointCount() != (Int32)params.samples)
			return maxon::UnexpectedError(MAXON_SOURCE_LOCATION);

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
		const Int32 segmentCount = 1;
		if (!splineObj.MakeVariableTag(Tsegment, segmentCount))
			return maxon::UnexpectedError(MAXON_SOURCE_LOCATION);

		// Access the writable array of the points representing the curve passing points
		Vector* splinePntsPtr = splineObj.GetPointW();
		if (nullptr == splinePntsPtr)
			return maxon::NullptrError(MAXON_SOURCE_LOCATION);

		// Sample splineData, set point positions
		for (Int32 pointIndex = 0; pointIndex < (Int32)params.samples; ++pointIndex)
		{
			Float samplePos = (Float)pointIndex / ((Float)params.samples - 1);
			Float splineValue = params.splineData->GetPoint(samplePos).y;
			splinePntsPtr[pointIndex] = Vector(samplePos * params.width, splineValue * params.amplitude, 0.0);
		}

		// Access the curve's segments array.
		Segment* splineSegsPtr = splineObj.GetSegmentW();
		if (nullptr == splineSegsPtr)
			return maxon::NullptrError(MAXON_SOURCE_LOCATION);

		// Set the closure status and the number of CVs for the only one segment existing.
		splineSegsPtr[0].closed = params.closed;
		splineSegsPtr[0].cnt = params.samples;

		return maxon::OK;
	}
}


//------------------------------------------------------------------------------------------------
/// ObjectData implementation generating a spline based on a SplineData
//------------------------------------------------------------------------------------------------
class SplineDataSplineObject : public ObjectData
{
	INSTANCEOF(SplineDataSplineObject, ObjectData)

public:
	virtual Bool Init(GeListNode* node);
	virtual void GetDimension(BaseObject* op, Vector* mp, Vector* rad);
	virtual SplineObject* GetContour(BaseObject* op, BaseDocument* doc, Float lod, BaseThread* bt);
	Bool GetDEnabling(GeListNode *node, const DescID &id, const GeData &t_data, DESCFLAGS_ENABLE flags, const BaseContainer *itemdesc);

	static NodeData* Alloc()
	{
		return NewObj(SplineDataSplineObject) iferr_ignore("SplineDataSplineObject plugin not instanced");
	}
};

Bool SplineDataSplineObject::Init(GeListNode* node)
{
	if (!node)
		return false;

	// Retrieve the BaseContainer object belonging to the generator.
	BaseObject*		 baseObjectPtr = static_cast<BaseObject*>(node);
	BaseContainer* objectDataPtr = baseObjectPtr->GetDataInstance();

	// Fill object container with initial values
	objectDataPtr->SetFloat(OSPLINEDATA_WIDTH, SplineDataVisualizationHelpers::DEFAULT_WIDTH);
	objectDataPtr->SetFloat(OSPLINEDATA_HEIGHT, SplineDataVisualizationHelpers::DEFAULT_HEIGHT);
	objectDataPtr->SetUInt32(OSPLINEDATA_SUBDIVISION, SplineDataVisualizationHelpers::DEFAULT_SPLINEDATA_SUBDIVISION);

	GeData geSplineData(CUSTOMDATATYPE_SPLINE, DEFAULTVALUE);
	SplineData *splineData = (SplineData*)geSplineData.GetCustomDataType(CUSTOMDATATYPE_SPLINE);
	splineData->MakeLinearSplineBezier(2);
	objectDataPtr->SetData(OSPLINEDATA_SPLINECURVE, geSplineData);

	// SplineObject standard parameters
	objectDataPtr->SetInt32(SPLINEOBJECT_TYPE, SPLINEOBJECT_TYPE_CUBIC);
	objectDataPtr->SetBool(SPLINEOBJECT_CLOSED, false);
	objectDataPtr->SetInt32(SPLINEOBJECT_INTERPOLATION, SPLINEOBJECT_INTERPOLATION_NATURAL);
	objectDataPtr->SetInt32(SPLINEOBJECT_SUB, 8);
	objectDataPtr->SetFloat(SPLINEOBJECT_ANGLE, DegToRad(5.0));
	objectDataPtr->SetFloat(SPLINEOBJECT_MAXIMUMLENGTH, 5.0);

	return true;
}

void SplineDataSplineObject::GetDimension(BaseObject *op, Vector *mp, Vector *rad)
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
	rad->x = objectDataPtr->GetFloat(OSPLINEDATA_WIDTH);
	rad->y = objectDataPtr->GetFloat(OSPLINEDATA_HEIGHT);
}

SplineObject* SplineDataSplineObject::GetContour(BaseObject *op, BaseDocument *doc, Float lod, BaseThread *bt)
{
	// Check the passed pointer.
	if (!op)
		return nullptr;

	// Retrieve the BaseContainer object belonging to the generator.
	BaseContainer* objectDataPtr = op->GetDataInstance();

	// Fill the retrieve BaseContainer object with initial values.
	const Float width = objectDataPtr->GetFloat(OSPLINEDATA_WIDTH, SplineDataVisualizationHelpers::DEFAULT_WIDTH);
	const Float amplitude = objectDataPtr->GetFloat(OSPLINEDATA_HEIGHT, SplineDataVisualizationHelpers::DEFAULT_HEIGHT);
	const UInt32 samples = objectDataPtr->GetUInt32(OSPLINEDATA_SUBDIVISION, SplineDataVisualizationHelpers::DEFAULT_SPLINEDATA_SUBDIVISION) + 1;
	GeData geSplineData = objectDataPtr->GetData(OSPLINEDATA_SPLINECURVE);
	SplineData *splineData = (SplineData*)geSplineData.GetCustomDataType(CUSTOMDATATYPE_SPLINE);
	if (!splineData)
		return nullptr;

	const Bool closed = objectDataPtr->GetBool(SPLINEOBJECT_CLOSED, false);
	const SPLINETYPE splineType = (SPLINETYPE)objectDataPtr->GetInt32(SPLINEOBJECT_TYPE, SPLINEOBJECT_TYPE_LINEAR);
	const Int32 splineInterpolation = objectDataPtr->GetInt32(SPLINEOBJECT_INTERPOLATION, SPLINEOBJECT_INTERPOLATION_NONE);
	const Int32 splineSubdivision = objectDataPtr->GetInt32(SPLINEOBJECT_SUB, 0);
	const Float splineAngle = objectDataPtr->GetFloat(SPLINEOBJECT_ANGLE, 0.0);
	const Float splineMaxLength = objectDataPtr->GetFloat(SPLINEOBJECT_MAXIMUMLENGTH, 0.0);

	// Set parameters to params set
	SplineDataVisualizationHelpers::SplineDataSplineParameters params(amplitude, width, samples, splineData, closed, splineType, splineInterpolation, splineSubdivision, splineAngle, splineMaxLength);

	// Alloc a SplineObject and check it.
	SplineObject* splineObjPtr = SplineObject::Alloc(params.samples, params.splineType);
	if (!splineObjPtr)
		return nullptr;

	// Invoke the helper function to set the SplineObject object member accordingly
	iferr (SplineDataVisualizationHelpers::CreateSplineDataSpline(*splineObjPtr, params))
	{
		DiagnosticOutput("Error on CreateSplineDataSpline: @", err);
		SplineObject::Free(splineObjPtr);
		return nullptr;
	}

	splineObjPtr->Message(MSG_UPDATE);

	return splineObjPtr;
}

Bool SplineDataSplineObject::GetDEnabling(GeListNode *node, const DescID &id, const GeData &t_data, DESCFLAGS_ENABLE flags, const BaseContainer *itemdesc)
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
		case SPLINEOBJECT_ANGLE:
			return interpolation == SPLINEOBJECT_INTERPOLATION_ADAPTIVE || interpolation == SPLINEOBJECT_INTERPOLATION_SUBDIV;
		case SPLINEOBJECT_MAXIMUMLENGTH:
			return interpolation == SPLINEOBJECT_INTERPOLATION_SUBDIV;
		case SPLINEOBJECT_SUB:
			return interpolation == SPLINEOBJECT_INTERPOLATION_NATURAL || interpolation == SPLINEOBJECT_INTERPOLATION_UNIFORM;
	}

	return SUPER::GetDEnabling(node, id, t_data, flags, itemdesc);
}


Bool RegisterSplineDataSpline()
{
	String registeredName = GeLoadString(IDS_OBJECTDATA_SPLINEDATASPLINE);
	if (!registeredName.IsPopulated())
		return false;

	return RegisterObjectPlugin(SplineDataVisualizationHelpers::ID_OBJECTDATA_SPLINEDATASPLINE, registeredName, OBJECT_GENERATOR|OBJECT_ISSPLINE, SplineDataSplineObject::Alloc, "osplinedataspline"_s, AutoBitmap("osplinedataspline.tif"_s), 0);
}
