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

#ifndef __qSlicerDoseComparisonModuleWidget_h
#define __qSlicerDoseComparisonModuleWidget_h

// SlicerQt includes
#include "qSlicerAbstractModuleWidget.h"

#include "qSlicerDoseComparisonModuleExport.h"

class qSlicerDoseComparisonModuleWidgetPrivate;
class vtkMRMLNode;

/// \ingroup Slicer_QtModules_DoseComparison
class Q_SLICER_QTMODULES_DOSECOMPARISON_EXPORT qSlicerDoseComparisonModuleWidget :
  public qSlicerAbstractModuleWidget
{
  Q_OBJECT

public:

  typedef qSlicerAbstractModuleWidget Superclass;
  qSlicerDoseComparisonModuleWidget(QWidget *parent=0);
  virtual ~qSlicerDoseComparisonModuleWidget();

public slots:


protected:
  QScopedPointer<qSlicerDoseComparisonModuleWidgetPrivate> d_ptr;
  
  virtual void setup();

private:
  Q_DECLARE_PRIVATE(qSlicerDoseComparisonModuleWidget);
  Q_DISABLE_COPY(qSlicerDoseComparisonModuleWidget);
};

#endif