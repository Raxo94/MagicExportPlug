import pymel.core as pm
import time
import pymel.core.datatypes as dt
import maya.mel as mel

class Keyframe:

    scale = [1,1,1]
    translate = [0,0,0]
    rotation = [0,0,0]  
    time = 0   

def ChangeLayer(LayerName,boolOn):
   if boolOn == True:
       command = 'animLayerEditorOnSelect ' + LayerName +' 1;'
   else:
       command = 'animLayerEditorOnSelect ' + LayerName +' 0;'
   mel.eval(command)


def returnThis(this):
    return this

def GetJointKeyframes(jointName,keyframeList):
    
    pm.select(jointName)  
    curr = first = pm.findKeyframe(jointName, which='first')
    pm.setCurrentTime(first)

    
    last = pm.findKeyframe(jointName, which='last')
    kc = pm.keyframe( jointName, sl=False, q=True, keyframeCount=True )/10 #THIS IS LOGICAL AND MAKES SENSE!
   
    print("\n")    
    if (kc>0):
        
        while curr <= last:
            
            kf = Keyframe()
            kf.time = curr
            
            node = pm.PyNode(jointName)
            kf.scale = node.getScale()
            kf.rotation = node.getRotation()
            kf.translation = node.getTranslation()

            print ("JointName: " + jointName)
            print ("Time: " + str(kf.time))
            print("SCALE: " + str(kf.scale))
            print("ROTATION: " + str(kf.rotation))
            print("TRANSLATION: " + str(kf.translation))
            print("\n")
    
            keyframeList.append(kf)
            if curr==last:
                break
          
            
            curr = pm.findKeyframe(jointName, time=curr, which='next')
            pm.setCurrentTime(curr)
            
            
            
            
    else:
        print("No Keyframes in this animation layer")     
            
keyframes = []  
