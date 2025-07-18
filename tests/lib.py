#! python 3
import os
from Rhino import RhinoApp
import scriptcontext as sc

def get_test_file(file_name:str):
    root = os.path.dirname(__file__)
    files = os.path.join(root, 'files')
    test_file = os.path.join(files, file_name)
    
    return test_file;

def get_export_dir():
    root = os.path.dirname(__file__)
    files = os.path.join(root, 'files')
    test_file = os.path.join(files, 'material-mapping.3dm')
    export_dir = os.path.join(root, 'exports')
    
    return export_dir;

def export_usd_files():
    
    export_dir = get_export_dir();

    export_file_usd = os.path.join(export_dir, 'material-mapping.usd')
    export_file_usda = os.path.join(export_dir, 'material-mapping.usda')
    export_file_usdc = os.path.join(export_dir, 'material-mapping.usdc')
    export_file_usdz = os.path.join(export_dir, 'material-mapping.usdz')

    RhinoApp.RunScript("selAll", True);

    RhinoApp.RunScript(f"-export {export_file_usd} _Enter", True)
    RhinoApp.RunScript(f"-export {export_file_usda} _Enter", True)
    RhinoApp.RunScript(f"-export {export_file_usdc} _Enter", True)
    RhinoApp.RunScript(f"-export {export_file_usdz} _Enter", True)

def open_file(path:str):
    RhinoApp.RunScript(f"-open {path}", True);
    RhinoApp.RunScript(f"ZE", True);

def close_file(path:str):
    RhinoApp.RunScript(f"_-Close \"{path}\" _Enter", True);