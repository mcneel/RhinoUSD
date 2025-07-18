#! python 3
import sys
import os

sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))
from lib import *

test_file = get_test_file('material-mapping.3dm')
open_file(test_file)

export_dir = get_export_dir()
texture_file = os.path.join(export_dir, 'mattano blue.png')

export_usd_file()

assertmsg(os.path.exists(texture_file), 'USD texture file is missing')
os.remove(texture_file)

export_usda_file()
assertmsg(os.path.exists(texture_file), 'USDa texture file is missing')
os.remove(texture_file)

export_usdc_file()
assertmsg(os.path.exists(texture_file), 'USDc texture file is missing')
os.remove(texture_file)

export_usdz_file()
# Texture File is inside usdz

close_file(test_file);
