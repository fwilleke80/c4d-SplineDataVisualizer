
// Includes from API
#include "c4d_commanddata.h"
#include "c4d_includes.h"

// Includes from plugin project
#include "c4d_symbols.h"
#include "main.h"

// Local resources
#include "c4d_resource.h"

// Common values
#include "commons.h"


namespace SplineDataVisualizationHelpers
{
	/// \brief Returns a material with only luminance in the specified color
	static maxon::Result<BaseMaterial*> BuildLuminanceMaterial(const Vector &color)
	{
		// Allocate material
		BaseMaterial *mat = BaseMaterial::Alloc(Mmaterial);
		if (!mat)
			return maxon::OutOfMemoryError(MAXON_SOURCE_LOCATION, "Could not allocate material!"_s);

		// Enable only luminance channel
		mat->SetParameter(MATERIAL_USE_COLOR, false, DESCFLAGS_SET::NONE);
		mat->SetParameter(MATERIAL_USE_REFLECTION, false, DESCFLAGS_SET::NONE);
		mat->SetParameter(MATERIAL_USE_LUMINANCE, true, DESCFLAGS_SET::NONE);

		// Set luminance color
		BaseContainer *matDataPtr = mat->GetDataInstance();
		if (!matDataPtr)
			return maxon::UnexpectedError(MAXON_SOURCE_LOCATION, "Could not access BaseContainer of material!"_s);

		matDataPtr->SetVector(MATERIAL_LUMINANCE_COLOR, color);

		mat->Message(MSG_UPDATE);
		mat->Update(true, true);

		return mat;
	}
}


//------------------------------------------------------------------------------------------------
/// CommandData implementation for a command that creates a nice SplineData Vis setup
//------------------------------------------------------------------------------------------------
class SplineDataVisCommand : public CommandData
{
	INSTANCEOF(SplineDataVisCommand, CommandData)

public:
	virtual Bool Execute(BaseDocument* doc, GeDialog* parentManager)
	{
		// Allocate objects
		BaseObject *groupObject = BaseObject::Alloc(Onull);
		if (!groupObject)
			return false;
		BaseObject *axisSplineObject = BaseObject::Alloc(SplineDataVisualizationHelpers::ID_OBJECTDATA_SPLINEDATAAXISSPLINE);
		if (!axisSplineObject)
			return false;
		BaseObject *splineDataSplineObject = BaseObject::Alloc(SplineDataVisualizationHelpers::ID_OBJECTDATA_SPLINEDATASPLINE);
		if (!splineDataSplineObject)
			return false;
		BaseObject *splineDataSweepObject = BaseObject::Alloc(Osweep);
		BaseObject *axisSweepObject = BaseObject::Alloc(Osweep);
		if (!splineDataSweepObject || !axisSweepObject)
			return false;
		BaseObject *splineDataProfile = BaseObject::Alloc(Osplinecircle);
		BaseObject *axisProfile = BaseObject::Alloc(Osplinecircle);
		if (!splineDataProfile || !axisProfile)
			return false;
		BaseObject *textObject = BaseObject::Alloc(Osplinetext);
		if (!textObject)
			return false;
		BaseObject *textExtrudeObject = BaseObject::Alloc(Oextrude);
		if (!textExtrudeObject)
			return false;

		// Set profile splines' attributes
		BaseContainer *splineDataProfileDataPtr = splineDataProfile->GetDataInstance();
		BaseContainer *axisProfileDataPtr = axisProfile->GetDataInstance();
		if (!splineDataProfileDataPtr || !axisProfileDataPtr)
			return false;
		splineDataProfileDataPtr->SetFloat(PRIM_CIRCLE_RADIUS, SplineDataVisualizationHelpers::DEFAULT_VIS_SPLINEDATA_PROFILE_RADIUS);
		axisProfileDataPtr->SetFloat(PRIM_CIRCLE_RADIUS, SplineDataVisualizationHelpers::DEFAULT_VIS_AXIS_PROFILE_RADIUS);

		// Set text properties
		BaseContainer *textObjectDataPtr = textObject->GetDataInstance();
		BaseContainer *textExtrudeDataPtr = textExtrudeObject->GetDataInstance();
		if (!textObjectDataPtr || !textExtrudeDataPtr)
			return false;
		textObjectDataPtr->SetInt32(PRIM_PLANE, PRIM_PLANE_XY);
		textObjectDataPtr->SetString(PRIM_TEXT_TEXT, GeLoadString(IDS_SPLINEDATAVIS_TEXT_DEFAULT));
		textObjectDataPtr->SetFloat(PRIM_TEXT_HEIGHT, SplineDataVisualizationHelpers::DEFAULT_VIS_TEXT_HEIGHT);
		textExtrudeDataPtr->SetVector(EXTRUDEOBJECT_MOVE, Vector(0.0, 0.0, SplineDataVisualizationHelpers::DEFAULT_VIS_TEXT_DEPTH));
		textExtrudeObject->SetAbsPos(Vector(0.0, -SplineDataVisualizationHelpers::DEFAULT_VIS_TEXT_HEIGHT - SplineDataVisualizationHelpers::DEFAULT_VIS_TEXT_OFFSET, 0.0));

		// Set objects' names
		groupObject->SetName(GeLoadString(IDS_SPLINEDATAVIS_GROUPOBJ));
		splineDataSweepObject->SetName(GeLoadString(IDS_SPLINEDATAVIS_SPLINEDATA_SWEEP));
		splineDataSplineObject->SetName(GeLoadString(IDS_SPLINEDATAVIS_SPLINEDATA_OBJECT));
		splineDataProfile->SetName(GeLoadString(IDS_SPLINEDATAVIS_SPLINEDATA_PROFILE));
		axisSweepObject->SetName(GeLoadString(IDS_SPLINEDATAVIS_AXIS_SWEEP));
		axisSplineObject->SetName(GeLoadString(IDS_SPLINEDATAVIS_AXIS_OBJECT));
		axisProfile->SetName(GeLoadString(IDS_SPLINEDATAVIS_AXIS_PROFILE));
		textObject->SetName(GeLoadString(IDS_SPLINEDATAVIS_TEXT_TEXTOBJECT));
		textExtrudeObject->SetName(GeLoadString(IDS_SPLINEDATAVIS_TEXT_EXTRUDEOBJECT));

		// Create object hierarchy
		textExtrudeObject->InsertUnder(groupObject);
		textObject->InsertUnder(textExtrudeObject);
		axisSweepObject->InsertUnder(groupObject);
		axisSplineObject->InsertUnder(axisSweepObject);
		axisProfile->InsertUnder(axisSweepObject);
		splineDataSweepObject->InsertUnder(groupObject);
		splineDataSplineObject->InsertUnder(splineDataSweepObject);
		splineDataProfile->InsertUnder(splineDataSweepObject);

		// Insert group into document
		doc->InsertObject(groupObject, nullptr, nullptr);

		// Create materials
		iferr (BaseMaterial *splineDataMat = SplineDataVisualizationHelpers::BuildLuminanceMaterial(SplineDataVisualizationHelpers::DEFAULT_VIS_SPLINEDATA_COLOR))
		{
			DiagnosticOutput("@", err);
			return false;
		}
		iferr (BaseMaterial *axisMat = SplineDataVisualizationHelpers::BuildLuminanceMaterial(SplineDataVisualizationHelpers::DEFAULT_VIS_AXIS_COLOR))
		{
			DiagnosticOutput("@", err);
			return false;
		}
		iferr (BaseMaterial *textMat = SplineDataVisualizationHelpers::BuildLuminanceMaterial(SplineDataVisualizationHelpers::DEFAULT_VIS_TEXT_COLOR))
		{
			DiagnosticOutput("@", err);
			return false;
		}

		// Set materials' names
		splineDataMat->SetName(GeLoadString(IDS_SPLINEDATAVIS_MAT_SPLINEDATA));
		axisMat->SetName(GeLoadString(IDS_SPLINEDATAVIS_MAT_AXIS));
		textMat->SetName(GeLoadString(IDS_SPLINEDATAVIS_MAT_TEXT));

		// Insert materials into document
		doc->InsertMaterial(splineDataMat);
		doc->InsertMaterial(axisMat);
		doc->InsertMaterial(textMat);

		// Apply material to objects
		TextureTag *textureTagSweep = static_cast<TextureTag*>(splineDataSweepObject->MakeTag(Ttexture));
		if (!textureTagSweep)
			return false;
		textureTagSweep->SetMaterial(splineDataMat);
		TextureTag *textureTagSweep2 = static_cast<TextureTag*>(axisSweepObject->MakeTag(Ttexture));
		if (!textureTagSweep2)
			return false;
		textureTagSweep2->SetMaterial(axisMat);
		TextureTag *textureTagExtrude = static_cast<TextureTag*>(textExtrudeObject->MakeTag(Ttexture));
		if (!textureTagExtrude)
			return false;
		textureTagExtrude->SetMaterial(textMat);

		// Add Face Camera expression tag
		BaseTag *faceCameraTag = groupObject->MakeTag(SplineDataVisualizationHelpers::ID_FACECAMERA);
		if (!faceCameraTag)
			return false;

		// Update all created nodes
		axisSplineObject->Message(MSG_UPDATE);
		axisProfile->Message(MSG_UPDATE);
		axisSweepObject->Message(MSG_UPDATE);
		splineDataSplineObject->Message(MSG_UPDATE);
		splineDataProfile->Message(MSG_UPDATE);
		splineDataSweepObject->Message(MSG_UPDATE);
		textObject->Message(MSG_UPDATE);
		textExtrudeObject->Message(MSG_UPDATE);
		groupObject->Message(MSG_UPDATE);
		textureTagSweep->Message(MSG_UPDATE);
		textureTagSweep2->Message(MSG_UPDATE);
		textureTagExtrude->Message(MSG_UPDATE);
		splineDataMat->Message(MSG_UPDATE);
		axisMat->Message(MSG_UPDATE);
		textMat->Message(MSG_UPDATE);
		faceCameraTag->Message(MSG_UPDATE);

		EventAdd();
		return true;
	}

	static CommandData *Alloc()
	{
		return NewObjClear(SplineDataVisCommand);
	}
};


Bool RegisterSplineDataVisCommand()
{
	String registeredName = GeLoadString(IDS_COMMANDDATA_SPLINEDATAVIS);
	if (!registeredName.IsPopulated())
		return false;
	String helpString = GeLoadString(IDH_COMMANDDATA_SPLINEDATAVIS);

	return RegisterCommandPlugin(SplineDataVisualizationHelpers::ID_COMMANDDATA_SPLINEDATAVIS, registeredName, 0, AutoBitmap("splinedataviscommand.tif"_s), helpString, SplineDataVisCommand::Alloc());
}
