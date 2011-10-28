import sys
import os

"""
    For this script to work you need to copy Jack.mesh from bin/media/models
    to bin/data/assets

    1. Run this script on your tundra 1.x txml
    2. This will create a new file with "_" appended before the .txml
    extension.
    3. Open this file in your tundra 2.x server
    4. Find the "Jack" entity, he will be lurking at pos 0,0,0.
    5. Shift+E for entity editor, select jack, if you dont see the
    manip visual aids, hit "tilde" (next to "1" key)
    6. Open Placeable component and rotate how you like, the whole
    scene will rotate with jack. Seems that -90 to x will do the
    trick.
    7. Shitf+S for scene editor, right click -> save scene as...

    Problems with this technique
    * You cannot remove or edit the grandparent jacks placeable
      component scale needs to be 1,1,1 or it will scale the whole
      scene (but might be handy if you want to do this)
    * Jack grandparent needs to be there always or the scene will flip back.

    There is also some experimental placeable Transform manipulations
    that you can do to the txml, but it wont work propably on any
    scene as everything needs a common pivot point for the rotation.

    So this is a temp hack. Proper way is to export your modelfs from
    eg. blender with the correct axis flip built in and import again.
    
"""

class Transform:

    def __init__(self, value):
        splitValue = value.split(",")
        
        self.pos = {}
        self.pos["x"] = splitValue[0]
        self.pos["y"] = splitValue[1]
        self.pos["z"] = splitValue[2]

        self.rot = {}
        self.rot["x"] = splitValue[3]
        self.rot["y"] = splitValue[4]
        self.rot["z"] = splitValue[5]

        self.scale = {}
        self.scale["x"] = splitValue[6]
        self.scale["y"] = splitValue[7]
        self.scale["z"] = splitValue[8]

    def flip(self, vec, first, second):
        temp = vec[first]
        vec[first] = vec[second]
        vec[second] = temp

    def rotate(self, vec, axis, deg):
        curDeg = float(vec[axis])
        curDeg += deg
        vec[axis] = str(curDeg)
        
    def getNewValue(self):
        line = self.pos["x"] + "," + self.pos["y"] + "," + self.pos["z"]
        line += "," + self.rot["x"] + "," + self.rot["y"] + "," + self.rot["z"]
        line += "," + self.scale["x"] + "," + self.scale["y"] + "," + self.scale["z"]
        return line

def usage_fail(errmsg):
    sys.stderr.write(errmsg + "\n")
    sys.stderr.write("Usage: tundra2-txml-converter.py -i|--input=inputTXML [-o|--output=outputTXML]\n")
    sys.exit(1)
        
def main():
    import getopt
    try:
        opts, args = getopt.getopt(sys.argv[1:], "i:o:", ["input=", "output="])
    except getopt.GetoptError, err:
        usage_fail(str(err))

    newFileName = fileName = None
    for o, a in opts:
        if o in ("-i", "--input"):
            fileName = a
        elif o in ("-o", "--output"):
            newFileName = a

    if not fileName:
        usage_fail("input filename missing")

    if not newFileName:
        newFileName = fileName[:fileName.index(".txml")] + "_.txml"

    try:
        c = open(fileName).read()
    except IOError, what:
        sys.exit("error reading input file %s: %s" % (fileName, what))

    lines = c.splitlines(True)
    parentName = "GeneratedGrandParentEntity"

    parentEntXml =  """ <entity id="1">
  <component type="EC_Mesh" sync="1">
   <attribute value="0,0,0,0,0,0,1,1,1" name="Transform"/>
   <attribute value="local://Jack.mesh" name="Mesh ref"/>
   <attribute value="" name="Skeleton ref"/>
   <attribute value="" name="Mesh materials"/>
   <attribute value="0" name="Draw distance"/>
   <attribute value="false" name="Cast shadows"/>
  </component>
  <component type="EC_Placeable" sync="1">
   <attribute value="0,0,-20,0,0,0,1,1,1" name="Transform"/>
   <attribute value="false" name="Show bounding box"/>
   <attribute value="true" name="Visible"/>
   <attribute value="1" name="Selection layer"/>
   <attribute value="" name="Parent entity ref"/>
   <attribute value="" name="Parent bone name"/>
  </component>
  <component type="EC_Name" sync="1">
   <attribute value="GeneratedGrandParentEntity" name="name"/>
   <attribute value="" name="description"/>
  </component>
 </entity>
"""

    out = ""
    totalIndex = 0
    expectParentAttr = False
    for line in lines:
        totalIndex += len(line)

        if line.count("<scene>") > 0:
            out += line
            out += parentEntXml
            continue
            
        if line.count("component type=\"EC_Placeable\"") > 0:
            out += line
            compEnd = c.find("</component>", totalIndex)
            iPlaceableEnd = c.find("name=\"Parent entity ref\"", totalIndex, compEnd)

            # Found existing, update
            if iPlaceableEnd > 0:
                expectParentAttr = True
            # did not find, generate
            else:  
                out += "   <attribute value=\"" + parentName + "\" name=\"Parent entity ref\"/>\n"                    
        elif expectParentAttr:
            if line.count("name=\"Parent entity ref\"") > 0:
                expectParentAttr = False
                start = line.find("\"")
                end = line.find("\"", start+1)
                value = line[start+1:end]
                if value == "":
                    out += "   <attribute value=\"" + parentName + "\" name=\"Parent entity ref\"/>\n" 
                else:
                    newLine = line[:start+1] + parentName + line[end:]
                    out += newLine
            else:
                out += line
        else:
            out += line
            
    try:
        f = open(newFileName, "w")
        f.write(out)
        f.close()
    except IOError, what:
        sys.exit("error writing to file %s: %s" % (newFileName, what))

if __name__ == "__main__":
    main()
