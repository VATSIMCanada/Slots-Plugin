# VATCANSitu

Latest Version 0.2.4.5a

- RVSM aircraft will show with the diamond PPS symbology. 
- Primary targets will show in magenta.
- Squawk 7600 and 7700 will show a red triangle.
- Aircrafts identing will have their PPS flash instead of the unusual ES target.
- CJS will flash if aircraft are nearing your airspace border to remind you to hand-off (I believe an option on the real thing)
- FP predicted tracks show with the appropriate orange airplane symbol.
- Altitude filter will be on by default.

Disable the default ES symbology for correlated targets in the symbology settings by replacing with the line "MOVETO 0 0"

version 0.2.3
- Added altitude filter settings, and altitude filter toggle. Only works for tags draw for by the plugin (i.e. the VFR tags) since there is no way to hook directly into the ES default filters. Saves settings to sector files. 

version 0.2.1: AUG3
- hotfix for 0.2 retargetted character encoding for better compatibility

- Added the range display
- Aaded Controller jurisdiction symbol that updates depending on the position you log in as.

![Screenshot](https://i.imgur.com/CKYPSyb.png)

Compatible with Euroscope beta r24 or higher (plugin was compiled using VS toolset 2019)
https://www.euroscope.hu/wp/2020/04/12/3-2-1-24/

EuroScope Plugin to simulate tools from NAVCANsuite.

The author of this plugin has NO affiliation with Nav Canada; This tool is for simulation use only on the VATSIM network, not for real world aviation. 

https://vimeo.com/443838489

# Features
1. Correlated radar targets with a VFR flight plan will be shown using an orange present position symbol.
2. Halo tool allows rings to be drawn around specific aircraft. The built in function in ES draws around all planes and there's no way to only apply rings to specific planes.
3. Mouse halo tool to aid with separation. (please see known issues)
4. Range displayed in menu per the real scope
5. CJS button shows your logged in position
6. RVSM aircraft will show with the diamond PPS symbology. 
7. Primary targets will show in magenta.
8. Squawk 7600 and 7700 will show a red triangle.
9. Aircrafts identing will have their PPS flash instead of the unusual ES target.
10. CJS will flash if aircraft are nearing your airspace border to remind you to hand-off (I believe an option on the real thing)
11. FP predicted tracks show with the appropriate orange airplane symbol.

Not implemented for now: There are some sham buttons just to replicate the UI (also I don't know what some of them do in the real system). The PTL and RBL default ES tools work well, unlikely will be a priority.

# Installation
The dll was compiled using Visual Studio 2019 (v142) using MFC libraries. The source code is provided to allow you to review and compile yourself.

If you opt not to compile yourself, binaries are under releases. Load the .dll using the Plug-ins folder in EuroScope. Allow the plugin to draw on the "Standard ES radar screen"

# Known Issues
EuroScope runs at a very low framerate unless a function asks for more screen draws. Essentially runs at 1FPS most of the time! The RBL is an example of this; when it is called, the screen refreshes much quicker to make it follow your mouse and give you a smooth experience. This is quite taxing on CPU usage; try drawing a RBL line and spinning it around it a circle (CPU use will rise dramatically). To get smooth mouse tracking of the mouse halo, the plugin does the same thing. CPU use will be quite high when it is on, so I recommend turning it on only when needed. When the mouse halo is on, it may interfere with your ability to open up other ES windows (RWY selection, ATIS), so turn it off before opening these other windows.

(partially resolved with altitude filters v0.2.3)
If you use NARDS tags for ground operations, the VFR PPS symbol will show on top of the plane logo. Can potentially be resolved by adding an additional variable to the .asr fil (not yet implemented) VFR PPS symbols ignore altitude filters (I've emailed the developer of ES, unfortunately the PlugIn enviroment does not allow access to these built-in data). Fixing this would involve making another altitude filter function within the plugin, but this seems redudant...
