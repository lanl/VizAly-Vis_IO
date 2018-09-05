The GenericIO Reader plugin is avaliable in ParaView 5.5.1 and above

## Activating the plugin
- In ParaView, go to Tools -> Manage Plugins
- In the "Local Plgins" window, locate "GenericIOReader". Select the plugin and check "Auto Load" and click "Load Selected"
- In client-server mode, do the same for Remote Plugins


## Loading GenericIO data
 - Open a GenericIO file by clicking on File -> Open... and navigating to the file you wish to visualize
 - A new window called "Open Data With ..." should popup and choose "GenericIO Files".
   - this plugin is **NOT** in "GenericIO Files to MultiBlockDataSet" or "GenericIO Files to UnstructuredGrid"
 - In the properties window, check the fields that you want to load and click on Apply
   - This should load the data sampled at 0.1^3. Move the slider to vary how much data to load
   
## UI Elements
  - Power Cube Smapling: when checked, the data is sampled at x^3 where x is the value on the slider. Uncheck it to sample at x^1
  - Sampling Type:
    - All data (sampled): No filters are applied
    - Selection (AND) : Applies an "AND operation to chain selection criteria"
  - Selection (Sampling Type must be "Selection(AND)"):
    - Salar: which scalar to filter on
    - Selection Criteria: which operator to select (is(==), greater or equal to, less or equal to, is between)
    - Value: operand to compare against
    - Value 2 (range): for "is betweeen" operator
  - Reser Selection: clears the chain on operations in Selection mode
  
  
## Source code
This should also be available at https://gitlab.kitware.com/paraview/paraview/tree/master/Plugins/GenericIOReader

### Building with ParaView
To enable in ParaView's superbuild, make sure that ENABLE_genericio=TRUE in cmake
``
cmake ../../paraview-superbuild \
-DENABLE_genericio=TRUE
``
## Sample Datasets
A small halo dataset is available at: https://github.com/lanl/VizAly-Vis_IO/blob/master/GenericIOReader/sodproperties.zip
Sample large datasets are available at: https://oceans11.lanl.gov/~qwofford/woodring_data/
