#! python 3
import sys
import os

sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))
from lib import *

test_file = get_test_file('material-mapping.3dm')
open_file(test_file)
export_usd_files();
close_file(test_file);
