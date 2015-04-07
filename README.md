# classio

![screenshot](classio-battery-connection-screenshot.png)

Example watchface showing the time, including seconds.

This version also displays the battery level and status of the Bluetooth connection.

## Economy window modified version by Ron

This version demonstrate the concept of reducing power consumption of watch by adding a window for the part that update frequently. Thus reducing power consumption because:
* reducing redrawing of non-updated part of the screen.
* Reducing recalculating/rebuilding all other layers that are not updated frequently.

### used steps
1. Define `update_all()` as the method to be called by any function that updated part of the screen, such as update minutes, update battery etc. This method ensure the main window is shown.
2. Define `update_only_sec()` to be called when only the frequent part is updated, such as update seconds. This method ensure the dedicated smaller window is shown, disabling draw of all other layer in main window. 
3. Add calls to above functions.
4. Add a window to be used only for the frequently updated part. This window root layer have update procedure that clear only the area used by the frequently updated part, and leaving the rest of main window visible.
5. Separated the frequently updated data (seconds) to be in new window. It should overlap same part in main window. Alternatively, the layer can move between main window and small economy window according to which is active. 

### Licence
All modification I made are under MIT licence. (Original watchface is under whatever licence Pebble uses)