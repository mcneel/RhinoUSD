#! python 3
import sys
import os

script = os.path.dirname(os.path.dirname(__file__))
sys.path.append(script)
from lib import *

import Rhino.DocObjects as do

# new_file()
import_usda_file('box')

# One _valid_ mesh should exist

doc = __rhino_doc__

objects = doc.Objects.GetObjectsByType(do.ObjectType.Mesh)  

assertmsg(len(objects) == 1, 'Too many objects!')

mesh = objects[0]
assertmsg(mesh.IsValid, 'Object is not a Valid Mesh')
