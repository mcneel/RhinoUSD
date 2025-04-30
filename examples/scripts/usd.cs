using System;
using System.IO;

using Rhino;
using Rhino.FileIO;
using Rhino.Collections;

var desktop = Environment.GetFolderPath(Environment.SpecialFolder.Desktop);

var options = new FileWriteOptions()
                {
                    WriteSelectedObjectsOnly = true,
                    SuppressAllInput = true,
                    SuppressDialogBoxes = true,
                };

var dictionary = new ArchivableDictionary();
dictionary.Set("blocks", 2);
dictionary.Set("root-layer", "RootPrim");
dictionary.Set("model-name", "MyModel");
dictionary.Set("force-meshes", true);
dictionary.Set("include-user-strings", false);

var doc = RhinoDoc.ActiveDoc;
doc.Export(Path.Combine(desktop, "usd.usda"), dictionary);