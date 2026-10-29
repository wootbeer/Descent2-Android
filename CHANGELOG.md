# Changelog

## [0.11 (10-29-2026)]

### Fixed
 - Big slowdown on Snapdragon 8 Gen 3 (Adreno 750) whenever an explosion or laser hit effect was on screen

## [0.10 (10-28-2026)]

### Added
 - D-pad directions can be bound to actions in Remap Gamepad (menus still use them to navigate)

### Fixed
 - White menu backgrounds on some Mali TVs (wrong screen format picked)

## [0.9 (10-20-2026)]

### Fixed
 - Game closed when starting a new game (or finishing a level) without the movie files (.MVL); missing movies are now skipped

## [0.8 (10-20-2026)]

### Added
 - If the game closes on an error or crash, the reason is shown the next time it starts, for bug reports

## [0.7 (10-19-2026)]

### Fixed
 - Older DESCENT2.HOG versions: a missing briefing or title picture is skipped (or the low-res one used) instead of closing the game

## [0.6 (10-15-2026)]

### Added
 - Disable thief bot toggle

### Changed
 - Redbook music plays through now

### Fixed
 - White/gray menu backgrounds and flashing on Mali GPUs
 - Briefing background disappearing when the robot animation started on Mali
 - Android TV: folder path screen. added a Type Folder Path button

## [0.5 (10-13-2026)]

### Added
 - Weapon-cycling binds
 - Individual weapon binds

## [0.4 (10-13-2026)]

### Added
 - Ship bob toggle
 - Delete pilot feature
 - Delete custom mission feature
 - Analog option for left stick so sensitivity works

### Changed
 - Removed the brightness option, Android has this

### Fixed
 - Touch controls perma hidden

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
