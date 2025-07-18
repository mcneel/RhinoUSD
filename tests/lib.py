#! python 3
import os
from Rhino import RhinoApp
import scriptcontext as sc

def get_test_file(file_name:str):
    root = os.path.dirname(__file__)
    files = os.path.join(root, 'files')
    test_file = os.path.join(files, file_name)
    
    return test_file

def get_export_dir():
    root = os.path.dirname(__file__)
    files = os.path.join(root, 'files')
    test_file = os.path.join(files, 'material-mapping.3dm')
    export_dir = os.path.join(root, 'exports')
    
    return export_dir

def export_usd_files():
    
    export_usd_file()
    export_usda_file()
    export_usdc_file()
    export_usdz_file()

def export_usd_file() -> str:
    
    export_dir = get_export_dir()
    export_file_usd = os.path.join(export_dir, 'material-mapping.usd')

    RhinoApp.RunScript("selAll", True)
    RhinoApp.RunScript(f"-export {export_file_usd} _Enter", True)

    # Ensure files exists
    assertmsg(os.path.exists(export_file_usd), f'File {export_file_usd} was not exported successfully')

    return export_file_usd

def export_usda_file() -> str:
    
    export_dir = get_export_dir()
    export_file_usda = os.path.join(export_dir, 'material-mapping.usda')

    RhinoApp.RunScript("selAll", True)
    RhinoApp.RunScript(f"-export {export_file_usda} _Enter", True)

    # Ensure files exists
    assertmsg(os.path.exists(export_file_usda), f'File {export_file_usda} was not exported successfully')

    return export_file_usda

def export_usdc_file() -> str:
    
    export_dir = get_export_dir()
    export_file_usdc = os.path.join(export_dir, 'material-mapping.usdc')

    RhinoApp.RunScript("selAll", True)
    RhinoApp.RunScript(f"-export {export_file_usdc} _Enter", True)

    # Ensure files exists
    assertmsg(os.path.exists(export_file_usdc), f'File {export_file_usdc} was not exported successfully')

    return export_file_usdc

def export_usdz_file() -> str:
    
    export_dir = get_export_dir()
    export_file_usdz = os.path.join(export_dir, 'material-mapping.usdz')

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
    return os.path.open(filepath)

def assert_element_in_usda_file(usda_data:str, type_name:str element_name:str):
    assertmsg(f'def {type_name} {element_name}' in usd_data, f'{element_name_name} not found in USDA file!')

def assert_property_in_usda_file(usda_data:str, property_name:str):
    assertmsg(f'{property_name}' in usd_data, f'{property_name} not found in USDA file!')
