import os
import vtk, qt, ctk, slicer, logging

#
# Abstract class of python scripted segment editor effects
#

class AbstractScriptedSegmentEditorEffect():
  """ Abstract scripted segment editor effects for effects implemented in python

      USAGE:
      1. Instantiation and registration
        Instantiate segment editor effect adaptor class from
        module (e.g. from setup function), and set python source:
        > import qSlicerSegmentationsEditorEffectsPythonQt
        > scriptedEffect = qSlicerSegmentationsEditorEffectsPythonQt.qSlicerSegmentEditorScriptedEffect(None)
        > scriptedEffect.setPythonSource(MyEffect.filePath)
        Registration is automatic

      2. Call host C++ implementation using
        > self.scriptedEffect.functionName()
        
      2.a. Most frequently used such methods are:
        Parameter get/set: parameter, integerParameter, doubleParameter, setParameter
        Add options widget: addOptionsWidget 
        Coordinate transforms: rasToXy, xyzToRas, xyToRas, xyzToIjk, xyToIjk
        Convenience getters: renderWindow, renderer, viewNode

      2.b. Always call API functions (the ones that are defined in the adaptor
        class qSlicerSegmentEditorScriptedEffect) using the adaptor accessor:
        > self.scriptedEffect.updateGUIFromMRML()

      An example for a generic effect is the ThresholdEffect

  """

  def __init__(self, scriptedEffect):
    self.scriptedEffect = scriptedEffect

    # Register plugin on initialization
    self.register()

  def register(self):
    import qSlicerSegmentationsEditorEffectsPythonQt
    #TODO: For some reason the instance() function cannot be called as a class function although it's static
    factory = qSlicerSegmentationsEditorEffectsPythonQt.qSlicerSegmentEditorEffectFactory()
    effectFactorySingleton = factory.instance()
    effectFactorySingleton.registerEffect(self.scriptedEffect)

  #
  # Utility functions for convenient coordinate transformations
  #
  def rasToXy(self, ras, viewWidget):
    rasVector = qt.QVector3D(ras[0], ras[1], ras[2])
    xyPoint = self.scriptedEffect.rasToXy(rasVector, viewWidget)
    return [xyPoint.x(), xyPoint.y()]
   
  def xyzToRas(self, xyz, viewWidget):
    xyzVector = qt.QVector3D(xyz[0], xyz[1], xyz[2])
    rasVector = self.scriptedEffect.xyzToRas(xyzVector, viewWidget)
    return [rasVector.x(), rasVector.y(), rasVector.z()]

  def xyToRas(self, xy, viewWidget):
    xyPoint = qt.QPoint(xy[0], xy[1])
    rasVector = self.scriptedEffect.xyToRas(xyPoint, viewWidget)
    return [rasVector.x(), rasVector.y(), rasVector.z()]

  def xyzToIjk(self, xyz, viewWidget, image):
    import vtkSegmentationCore
    xyzVector = qt.QVector3D(xyz[0], xyz[1], xyz[2])
    ijkVector = self.scriptedEffect.xyzToIjk(xyzVector, viewWidget, image)
    return [ijkVector.x(), ijkVector.y(), ijkVector.z()]

  def xyzToIjk(self, xy, viewWidget, image):
    import vtkSegmentationCore
    xyPoint = qt.QPoint(xy[0], xy[1])
    ijkVector = self.scriptedEffect.xyzToIjk(xyPoint, viewWidget, image)
    return [ijkVector.x(), ijkVector.y(), ijkVector.z()]
