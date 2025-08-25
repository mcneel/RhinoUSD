#! python 3
import sys
import os

script = os.path.dirname(os.path.dirname(__file__))
sys.path.append(script)
from lib import *

import Rhino.DocObjects as do

# new_file()
import_usda_file('data')

# One _valid_ mesh should exist

doc = __rhino_doc__

objects = doc.Objects.GetObjectsByType(do.ObjectType.Mesh)  

assertmsg(len(objects) == 1, 'Too many objects!')

mesh = objects[0]

assertmsh(mesh.Attributes.GetUserString("Key") == "Value", "Key/Value is missing")
assertmsh(mesh.Attributes.GetUserString("Angel") == "😇", "Angel/😇 is missing")
assertmsh(mesh.Attributes.GetUserString("Japanese") == "ポンーポン", "Japanese/ポンーポン is missing")
