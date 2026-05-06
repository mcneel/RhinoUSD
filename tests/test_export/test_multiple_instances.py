#! python 3
import sys
import os
import re

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

# Regression guard for RH-88698: every instance reference in the source 3dm
# must produce a distinct BlockInstance prim with its own transform.
usda = get_usd_file_text(main_file)

instance_names = re.findall(r'def\s+Xform\s+"(BlockInstance\d+)"', usda)
assertmsg(len(instance_names) >= 2,
          f'Expected at least 2 BlockInstance prims, found {len(instance_names)}: {instance_names}')
assertmsg(len(instance_names) == len(set(instance_names)),
          f'BlockInstance prim names are not unique: {instance_names}')

# Each BlockInstance must declare an xformOp:transform. The translation lives
# in the last *row* (row-vector convention); if the Convert helper regresses
# and skips the transpose, every matrix collapses to identity-with-zero-row3
# and all instances stack at the origin.
transform_blocks = re.findall(
    r'def\s+Xform\s+"BlockInstance\d+"[^{]*\{[^}]*?matrix4d\s+xformOp:transform\s*=\s*\(([^)]+(?:\)[^)]*\)[^)]*)+)\)',
    usda, re.DOTALL)
assertmsg(len(transform_blocks) == len(instance_names),
          f'Each BlockInstance must have an xformOp:transform; '
          f'instances={len(instance_names)}, transforms={len(transform_blocks)}')

close_file(test_file)
