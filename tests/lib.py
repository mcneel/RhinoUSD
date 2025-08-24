#! python 3
import os
from Rhino import RhinoApp
import scriptcontext as sc

def get_test_file(file_name:str):
    root = os.path.dirname(__file__)
    print(root)
    files = os.path.join(root, 'files')
    test_file = os.path.join(files, file_name)
    
    return test_file

def get_export_dir():
    root = os.path.dirname(__file__)
    export_dir = os.path.join(root, 'exports')
    if not os.path.exists(export_dir):
        os.mkdir(export_dir)
    
    return export_dir

def export_usd_files(filename:str):
    
    export_usd_file(filename)
    export_usda_file(filename)
    export_usdc_file(filename)
    export_usdz_file(filename)

def export_usd_file(filename:str) -> str:
    
    export_dir = get_export_dir()
    export_file_usd = os.path.join(export_dir, f"{filename}.usd")

    RhinoApp.RunScript("selAll", True)
    RhinoApp.RunScript(f"-export {export_file_usd} _Enter", True)

    # Ensure files exists
    assertmsg(os.path.exists(export_file_usd), f'File {export_file_usd} was not exported successfully')

    return export_file_usd

def export_usda_file(filename:str) -> str:
    
    export_dir = get_export_dir()
    export_file_usda = os.path.join(export_dir, f"{filename}.usda")

    RhinoApp.RunScript("selAll", True)
    RhinoApp.RunScript(f"-export {export_file_usda} _Enter", True)

    # Ensure files exists
    assertmsg(os.path.exists(export_file_usda), f'File {export_file_usda} was not exported successfully')

    return export_file_usda

def export_usdc_file(filename:str) -> str:
    
    export_dir = get_export_dir()
    export_file_usdc = os.path.join(export_dir, f"{filename}.usdc")

    RhinoApp.RunScript("selAll", True)
    RhinoApp.RunScript(f"-export {export_file_usdc} _Enter", True)

    # Ensure files exists
    assertmsg(os.path.exists(export_file_usdc), f'File {export_file_usdc} was not exported successfully')

    return export_file_usdc

def export_usdz_file(filename:str) -> str:
    
    export_dir = get_export_dir()
    export_file_usdz = os.path.join(export_dir, f"{filename}.usdz")

    RhinoApp.RunScript("selAll", True)
    RhinoApp.RunScript(f"-export {export_file_usdz} _Enter", True)

    # Ensure files exists
    assertmsg(os.path.exists(export_file_usdz), f'File {export_file_usdz} was not exported successfully')

    return export_file_usdz

def open_file(path:str):
    RhinoApp.RunScript(f"-open {path}", True)
    RhinoApp.RunScript(f"ZE", True)

def close_file(path:str):
    RhinoApp.RunScript(f"_-Close \"{path}\" _Enter", True)

def assertmsg(result:bool, msg:str):
    if not result:
        print(msg)
        assert(False)

def get_usd_file_text(filepath:str) -> str:
    assertmsg('.usda' in filepath, 'File is not USDA!')
    
    with open(filepath, 'r') as file:
        data = file.read()
        return data;
    
    return ''

def assert_element_in_usda_file(usda_data:str, type_name:str, element_name:str):
    assertmsg(f"def {type_name} {element_name}" in usda_data, f"{element_name} not found in USDA file!")

def assert_property_in_usda_file(usda_data:str, property_name:str):
    assertmsg(f"{property_name}" in usda_data, f"{property_name} not found in USDA file!")
