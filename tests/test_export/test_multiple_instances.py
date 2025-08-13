#! python 3
import sys
import os

script = os.path.dirname(os.path.dirname(__file__))
sys.path.append(script)
from lib import *

test_file = get_test_file('multiple_instances.3dm')
open_file(test_file)

export_dir = get_export_dir()
block_file = os.path.join(export_dir, 'sphere_block.usda')
main_file = os.path.join(export_dir, 'main.usda')

export_usda_file('main')
assertmsg(os.path.exists(main_file), 'Main USDC file is missing')
assertmsg(os.path.exists(block_file), 'External Block file is missing')

close_file(test_file)
