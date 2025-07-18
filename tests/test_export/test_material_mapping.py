#! python 3
import sys
import os

script = os.path.dirname(os.path.dirname(__file__))
sys.path.append(script)
from lib import *

test_file = get_test_file('material-mapping.3dm')
open_file(test_file)

export_dir = get_export_dir()
texture_file = os.path.join(export_dir, 'mattano blue.png')

export_usd_file()

assertmsg(os.path.exists(texture_file), 'USD texture file is missing')
os.remove(texture_file)

export_file_usda = export_usda_file()
assertmsg(os.path.exists(texture_file), 'USDa texture file is missing')
os.remove(texture_file)

export_usdc_file()
assertmsg(os.path.exists(texture_file), 'USDc texture file is missing')

export_usdz_file()
# Texture File is inside usdz

usda_data = get_usd_file_text(export_file_usda)
assert_element_in_usda_file(usda_data, 'Mesh' 'mesh0')
assert_property_in_usda_file(usda_data, 'material:binding')
assert_property_in_usda_file(usda_data, 'primvars:st1')

close_file(test_file)
