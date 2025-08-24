#! python 3
import sys
import os

script = os.path.dirname(os.path.dirname(__file__))
sys.path.append(script)
from lib import *

# new_file()
import_usda_file('prims')

# Layers that need to exist

layers = [
  'One',
  'One::OneOne',
  'One::OneOne::OneOneOne',
  'One::OneOne::OneOneOne::OneOneOneOne',
  'One::OneOne::OneOneOne::OneOneOneTwo',
  'One::OneOne::OneOneTwo',
  'One::OneTwo',
  'One::OneThree',
  'One::OneThree::OneThreeOne',
  'One::OneThree::OneThreeOne::OneThreeOneOne',
  'One::OneThree::OneThreeTwo',
  'One::OneThree::OneThreeTwo::OneThreeTwoOne',
]

doc = __rhino_doc__

for layer in layers:
    index = doc.Layers.FindByFullPath(layer, -1);
    assertmsg(index != -1, f"{layer} is missing!")
