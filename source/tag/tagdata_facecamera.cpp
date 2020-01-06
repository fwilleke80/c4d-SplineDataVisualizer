// Includes from API
#include "c4d_basetag.h"
#include "c4d_includes.h"
#include "c4d_tagdata.h"
#include "customgui_priority.h"

// Includes from plugin project
#include "c4d_symbols.h"
#include "main.h"

// Local resources
#include "tfacecamera.h"
#include "c4d_resource.h"

// Common values
#include "commons.h"


namespace SplineDataVisualizationHelpers
{
	inline maxon::Result<Matrix> InheritCameraMatrix(BaseDocument *doc, const Matrix &objectMatrix, Bool reverseZ)
	{
		//  Get active camera (use Editor camera if no user camera available)
		BaseDraw* bd = doc->GetRenderBaseDraw();
		if (!bd)
			return maxon::UnexpectedError(MAXON_SOURCE_LOCATION, "Could not get render BaseDraw!"_s);
		BaseObject* cam = bd->GetSceneCamera(doc);
		if (!cam)
			cam = bd->GetEditorCamera();
		if (!cam)
			return maxon::UnexpectedError(MAXON_SOURCE_LOCATION, "Could not get active camera!"_s);

		Matrix camMg = cam->GetMg();
		const Vector opScale = objectMatrix.sqmat.GetScale();
		return Matrix(objectMatrix.off, !camMg.sqmat.v1 * opScale.x, !camMg.sqmat.v2 * opScale.y, !(reverseZ ? (camMg.sqmat.v3 * -1.0) : camMg.sqmat.v3) * opScale.z);
	}
}


//------------------------------------------------------------------------------------------------
/// TagData implementation for a Tag that makes its object inherit the camera alignment
//------------------------------------------------------------------------------------------------
class FaceCameraTag : public TagData
{
public:
	virtual Bool Init(GeListNode* node);
	virtual EXECUTIONRESULT Execute(BaseTag* tag, BaseDocument* doc, BaseObject* op, BaseThread* bt, Int32 priority, EXECUTIONFLAGS flags);

	static NodeData* Alloc()
	{
		return NewObjClear(FaceCameraTag);
	}
};

Bool FaceCameraTag::Init(GeListNode* node)
{
	if (!node)
		return false;

	// Set attributes
	BaseContainer* tagDataPtr = static_cast<BaseTag*>(node)->GetDataInstance();
	tagDataPtr->SetBool(FACECAMERATAG_REVERSE, false);

	// Set expression priority
	GeData d;
	if (node->GetParameter(DescLevel(EXPRESSION_PRIORITY), d, DESCFLAGS_GET::NONE))
	{
		PriorityData* pd = (PriorityData*)d.GetCustomDataType(CUSTOMGUI_PRIORITY_DATA);
		if (pd)
			pd->SetPriorityValue(PRIORITYVALUE_CAMERADEPENDENT, GeData(true));
		node->SetParameter(DescLevel(EXPRESSION_PRIORITY), d, DESCFLAGS_SET::NONE);
	}

	return true;
}


EXECUTIONRESULT FaceCameraTag::Execute(BaseTag* tag, BaseDocument* doc, BaseObject* op, BaseThread* bt, Int32 priority, EXECUTIONFLAGS flags)
{
	iferr_scope_handler
	{
		DiagnosticOutput("Error on InheritCameraMatrix: @", err);
		return EXECUTIONRESULT::OUTOFMEMORY;
	};

	if (!tag || !doc || !op)
		return EXECUTIONRESULT::OUTOFMEMORY;

	// Get attributes
	BaseContainer *tagDataPtr = tag->GetDataInstance();
	if (!tagDataPtr)
		return EXECUTIONRESULT::OUTOFMEMORY;
	Bool reverseZ = tagDataPtr->GetBool(FACECAMERATAG_REVERSE, false);

	Matrix resultMatrix = SplineDataVisualizationHelpers::InheritCameraMatrix(doc, op->GetMg(), reverseZ) iferr_return;
	op->SetMg(resultMatrix);

	return EXECUTIONRESULT::OK;
}


Bool RegisterFaceCameraTag()
{
	String registeredName = GeLoadString(IDS_TAGDATA_FACECAMERA);
	if (!registeredName.IsPopulated())
		return false;
	return RegisterTagPlugin(SplineDataVisualizationHelpers::ID_FACECAMERA, registeredName, TAG_EXPRESSION | TAG_VISIBLE, FaceCameraTag::Alloc, "tfacecamera"_s, AutoBitmap("tfacecamera.tif"_s), 0);
}
