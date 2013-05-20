/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Csaba Pinter, PerkLab, Queen's University
  and was supported through the Applied Cancer Research Unit program of Cancer Care
  Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care

==============================================================================*/

// Contours includes
#include "vtkSlicerContoursModuleLogic.h"
#include "vtkMRMLContourNode.h"
#include "vtkSlicerContoursPatientHierarchyPlugin.h"

// SlicerRT includes
#include "SlicerRtCommon.h"
#include "vtkSlicerPatientHierarchyModuleLogic.h"
#include "vtkSlicerPatientHierarchyPluginHandler.h"

// VTK includes
#include <vtkNew.h>
#include <vtkPolyData.h>
#include <vtkLookupTable.h>

// MRML includes
#include <vtkMRMLModelNode.h>
#include <vtkMRMLModelDisplayNode.h>
#include <vtkMRMLDisplayableHierarchyNode.h>
#include <vtkMRMLColorTableNode.h>

// STD includes
#include <cassert>

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerContoursModuleLogic);

//----------------------------------------------------------------------------
vtkSlicerContoursModuleLogic::vtkSlicerContoursModuleLogic()
{
}

//----------------------------------------------------------------------------
vtkSlicerContoursModuleLogic::~vtkSlicerContoursModuleLogic()
{
}

//----------------------------------------------------------------------------
void vtkSlicerContoursModuleLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
void vtkSlicerContoursModuleLogic::SetMRMLSceneInternal(vtkMRMLScene* newScene)
{
  vtkNew<vtkIntArray> events;
  events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
  events->InsertNextValue(vtkMRMLScene::EndCloseEvent);
  this->SetAndObserveMRMLSceneEvents(newScene, events.GetPointer());
}

//-----------------------------------------------------------------------------
void vtkSlicerContoursModuleLogic::RegisterNodes()
{
  assert(this->GetMRMLScene() != 0);

  this->GetMRMLScene()->RegisterNodeClass(vtkSmartPointer<vtkMRMLContourNode>::New());

  // Register Patient Hierarchy plugin
  vtkSlicerPatientHierarchyPluginHandler::GetInstance()->RegisterPlugin(vtkSmartPointer<vtkSlicerContoursPatientHierarchyPlugin>::New());
}

//---------------------------------------------------------------------------
void vtkSlicerContoursModuleLogic::UpdateFromMRMLScene()
{
  assert(this->GetMRMLScene() != 0);

  this->CreateDefaultStructureSetNode();
  this->Modified();
}

//---------------------------------------------------------------------------
void vtkSlicerContoursModuleLogic::OnMRMLSceneNodeAdded(vtkMRMLNode* node)
{
  if (!node || !this->GetMRMLScene())
  {
    return;
  }

  if (node->IsA("vtkMRMLContourNode"))
  {
    // Create empty ribbon model
    this->CreateEmptyRibbonModelForContour(node);

    this->Modified();
  }
}

//---------------------------------------------------------------------------
void vtkSlicerContoursModuleLogic::OnMRMLSceneNodeRemoved(vtkMRMLNode* node)
{
  if (!node || !this->GetMRMLScene())
  {
    return;
  }

  vtkMRMLContourNode* contourNode = vtkMRMLContourNode::SafeDownCast(node);
  if (contourNode)
  {
    // Delete ribbon model representation if it is the default empty model
    if (contourNode->RibbonModelContainsEmptyPolydata())
    {
      this->GetMRMLScene()->RemoveNode(contourNode->GetRibbonModelNode());
    }

    this->Modified();
  }
}

//---------------------------------------------------------------------------
void vtkSlicerContoursModuleLogic::OnMRMLSceneEndClose()
{
  assert(this->GetMRMLScene() != 0);

  this->CreateDefaultStructureSetNode();
  this->Modified();
}

//---------------------------------------------------------------------------
void vtkSlicerContoursModuleLogic::CreateDefaultStructureSetNode()
{
  assert(this->GetMRMLScene() != 0);

  vtkSmartPointer<vtkCollection> defaultStructureSetNodes = vtkSmartPointer<vtkCollection>::Take(
    this->GetMRMLScene()->GetNodesByClassByName("vtkMRMLHierarchyNode", SlicerRtCommon::PATIENTHIERARCHY_DEFAULT_STRUCTURE_SET_NODE_NAME) );
  if (defaultStructureSetNodes->GetNumberOfItems() > 0)
  {
    vtkWarningMacro("CreateDefaultStructureSetNode: Default structure set node already exists");
    return;
  }

  vtkSmartPointer<vtkMRMLDisplayableHierarchyNode> structureSetPatientHierarchyNode = vtkSmartPointer<vtkMRMLDisplayableHierarchyNode>::New();
  structureSetPatientHierarchyNode->SetName(SlicerRtCommon::PATIENTHIERARCHY_DEFAULT_STRUCTURE_SET_NODE_NAME);
  structureSetPatientHierarchyNode->AllowMultipleChildrenOn();
  structureSetPatientHierarchyNode->HideFromEditorsOff();
  structureSetPatientHierarchyNode->SetAttribute(SlicerRtCommon::PATIENTHIERARCHY_NODE_TYPE_ATTRIBUTE_NAME, SlicerRtCommon::PATIENTHIERARCHY_NODE_TYPE_ATTRIBUTE_VALUE);
  structureSetPatientHierarchyNode->SetAttribute(SlicerRtCommon::PATIENTHIERARCHY_DICOMLEVEL_ATTRIBUTE_NAME, vtkSlicerPatientHierarchyModuleLogic::PATIENTHIERARCHY_LEVEL_SERIES);
  structureSetPatientHierarchyNode->SetAttribute(SlicerRtCommon::DICOMRTIMPORT_CONTOUR_HIERARCHY_ATTRIBUTE_NAME.c_str(), "1");
  this->GetMRMLScene()->AddNode(structureSetPatientHierarchyNode);

  // A hierarchy node needs a display node
  vtkSmartPointer<vtkMRMLModelDisplayNode> structureSetPatientHierarchyDisplayNode = vtkSmartPointer<vtkMRMLModelDisplayNode>::New();
  std::string contourHierarchyDisplayNodeName = std::string(SlicerRtCommon::PATIENTHIERARCHY_DEFAULT_STRUCTURE_SET_NODE_NAME) + "Display";
  structureSetPatientHierarchyDisplayNode->SetName(contourHierarchyDisplayNodeName.c_str());
  structureSetPatientHierarchyDisplayNode->SetVisibility(1);

  this->GetMRMLScene()->AddNode(structureSetPatientHierarchyDisplayNode);
  structureSetPatientHierarchyNode->SetAndObserveDisplayNodeID(structureSetPatientHierarchyDisplayNode->GetID());

  // Add color table node and default colors
  vtkSmartPointer<vtkMRMLColorTableNode> structureSetColorTableNode = vtkSmartPointer<vtkMRMLColorTableNode>::New();
  std::string structureSetColorTableNodeName;
  structureSetColorTableNodeName = SlicerRtCommon::PATIENTHIERARCHY_DEFAULT_STRUCTURE_SET_NODE_NAME + SlicerRtCommon::DICOMRTIMPORT_COLOR_TABLE_NODE_NAME_POSTFIX;
  structureSetColorTableNodeName = this->GetMRMLScene()->GenerateUniqueName(structureSetColorTableNodeName);
  structureSetColorTableNode->SetName(structureSetColorTableNodeName.c_str());
  structureSetColorTableNode->HideFromEditorsOff();
  structureSetColorTableNode->SetTypeToUser();
  this->GetMRMLScene()->AddNode(structureSetColorTableNode);

  structureSetColorTableNode->SetNumberOfColors(2);
  structureSetColorTableNode->GetLookupTable()->SetTableRange(0,1);
  structureSetColorTableNode->AddColor(SlicerRtCommon::COLOR_NAME_BACKGROUND, 0.0, 0.0, 0.0, 0.0); // Black background
  structureSetColorTableNode->AddColor(SlicerRtCommon::COLOR_NAME_INVALID,
    SlicerRtCommon::COLOR_VALUE_INVALID[0], SlicerRtCommon::COLOR_VALUE_INVALID[1],
    SlicerRtCommon::COLOR_VALUE_INVALID[2], SlicerRtCommon::COLOR_VALUE_INVALID[3] ); // Color indicating invalid index

  // Add color table in patient hierarchy
  vtkSmartPointer<vtkMRMLHierarchyNode> patientHierarchyColorTableNode = vtkSmartPointer<vtkMRMLHierarchyNode>::New();
  std::string phColorTableNodeName;
  phColorTableNodeName = structureSetColorTableNodeName + SlicerRtCommon::DICOMRTIMPORT_PATIENT_HIERARCHY_NODE_NAME_POSTFIX;
  phColorTableNodeName = this->GetMRMLScene()->GenerateUniqueName(phColorTableNodeName);
  patientHierarchyColorTableNode->SetName(phColorTableNodeName.c_str());
  patientHierarchyColorTableNode->HideFromEditorsOff();
  patientHierarchyColorTableNode->SetAssociatedNodeID(structureSetColorTableNode->GetID());
  patientHierarchyColorTableNode->SetAttribute(SlicerRtCommon::PATIENTHIERARCHY_NODE_TYPE_ATTRIBUTE_NAME, SlicerRtCommon::PATIENTHIERARCHY_NODE_TYPE_ATTRIBUTE_VALUE);
  patientHierarchyColorTableNode->SetAttribute(SlicerRtCommon::PATIENTHIERARCHY_DICOMLEVEL_ATTRIBUTE_NAME, vtkSlicerPatientHierarchyModuleLogic::PATIENTHIERARCHY_LEVEL_SUBSERIES);
  patientHierarchyColorTableNode->SetParentNodeID(structureSetPatientHierarchyNode->GetID());
  this->GetMRMLScene()->AddNode(patientHierarchyColorTableNode);
}

//---------------------------------------------------------------------------
void vtkSlicerContoursModuleLogic::CreateEmptyRibbonModelForContour(vtkMRMLNode* node)
{
  vtkMRMLScene* mrmlScene = this->GetMRMLScene();
  if (!mrmlScene)
  {
    vtkErrorMacro("CreateEmptyRibbonModelForContour: Invalid MRML scene!");
    return;
  }

  vtkMRMLContourNode* contourNode = vtkMRMLContourNode::SafeDownCast(node);
  if (!contourNode)
  {
    vtkErrorMacro("CreateEmptyRibbonModelForContour: Argument node is not a contour node!");
    return;
  }

  vtkSmartPointer<vtkPolyData> emptyPolyData = vtkSmartPointer<vtkPolyData>::New();

  vtkSmartPointer<vtkMRMLModelDisplayNode> emptyRibbonModelDisplayNode = vtkSmartPointer<vtkMRMLModelDisplayNode>::New();
  mrmlScene->AddNode(emptyRibbonModelDisplayNode);

  vtkSmartPointer<vtkMRMLModelNode> emptyRibbonModelNode = vtkSmartPointer<vtkMRMLModelNode>::New();
  mrmlScene->AddNode(emptyRibbonModelNode);
  emptyRibbonModelNode->SetAndObserveDisplayNodeID(emptyRibbonModelDisplayNode->GetID());
  emptyRibbonModelNode->SetAndObservePolyData(emptyPolyData);

  std::string emptyRibbonModelName(contourNode->GetName());
  emptyRibbonModelName.append(SlicerRtCommon::CONTOUR_RIBBON_MODEL_NODE_NAME_POSTFIX);
  emptyRibbonModelNode->SetName(emptyRibbonModelName.c_str());

  contourNode->SetAndObserveRibbonModelNodeId(emptyRibbonModelNode->GetID());
}