# Changelog

## [0.4 (10-13-2026)]

### Added
 - Ship bobbing toggle (Options > Toggles)
 - Gamepad X deletes the highlighted pilot in the pilot list (or press and hold a pilot)
 - Gamepad X on the New Game mission list removes the highlighted imported mission pack
 - Analog stick look option (Options): stick turns/pitches in proportion to how far it is pushed and follows the sensitivity slider; pitch and turn speeds now match

### Changed
 - Removed the brightness slider (it had no effect; use Android's brightness)

### Fixed
 - Touch controls permanently hidden on some phones with built-in or virtual gamepad devices
 - Deleting a pilot failed with "couldn't delete pilot"

## [0.3 (10-13-2026)]

### Added
 - Typing paths for devices without a system file picker (Android TV)

## [0.2 (10-9-2026)]

### Added
 - Can now bind M1/M2, extra buttons  

### Changed
 - Add Mission Packs only in main menu  

### Fixed
 - FPS counter is off by default  
 - Custom mission loading  
 - Garbled text in message boxes  
 - Option menu sliders  
 - Stuck pixels during level exit cutscene  
 - Other random crashes  

## [0.1 (10-7-2026)]

### Added
Mostly Vanilla Descent II.
 - Gamepad support
 - Gamepad rebinding
 - Render scaling+
 - FPS counter
 - Content file selector
 - Add-on mission pack support
 - Menu option to change size and opacity of on-screen touch controls
 - Cheats menu
 - Force 4:3 mode, use to force a 4:3 scale adding black bars
 - If you are using a 4:3 device already like a Nova you don't need to enable this
 - Various menu navigation

### Changed
 - Touch controls auto-show/hide based on gamepad availability
 - Removed Reverse Stereo from the Options menu (still available as `StereoReverse` in the config file)
 - No more idle movie playback from main menu

### Fixed
 - Misc. renderer and content pipeline fixes
 - Gyroscope now works
 - Added approximation of acid mode cheat
 - Various quality of life and bug fixes
