mGBA
====

mGBA is a new emulator for running Game Boy Advance games. It aims to be faster and more accurate than many existing Game Boy Advance emulators, as well as adding features that other emulators lack.

Up-to-date news and downloads can be found at [mgba.io](http://mgba.io/).

![Build status](https://travis-ci.org/mgba-emu/mgba.svg?branch=master)

Features
--------

- Near full Game Boy Advance hardware support[<sup>[1]</sup>](#missing).
- Fast emulation. Known to run at full speed even on low end hardware, such as netbooks.
- Qt and SDL ports for a heavy-weight and a light-weight frontend.
- Local (same computer) link cable support.
- Save type detection, even for flash memory size[<sup>[2]</sup>](#flashdetect).
- Real-time clock support, even without configuration.
- A built-in BIOS implementation, and ability to load external BIOS files.
- Turbo/fast-forward support by holding Tab.
- Frameskip, configurable up to 10.
- Screenshot support.
- Cheat code support.
- 9 savestate slots. Savestates are also viewable as screenshots.
- Video and GIF recording.
- Remappable controls for both keyboards and gamepads.
- Loading from ZIP and 7z files.
- IPS, UPS and BPS patch support.
- Game debugging via a command-line interface (not available with Qt port) and GDB remote support.
- Configurable emulation rewinding.

### Planned features

- Networked multiplayer link cable support ([Bug #1](http://mgba.io/b/1)).
- Dolphin/JOY bus link cable support ([Bug #73](http://mgba.io/b/73)).
- Re-recording support for tool-assist runs. ([Bugzilla keyword "TASBlocker"](https://endrift.com/mgba/bugs/buglist.cgi?quicksearch=TASBlocker))
- Lua support for scripting ([Bug #62](http://mgba.io/b/62)).
- A comprehensive debug suite ([Bug #132](http://mgba.io/b/132)).
- OpenEmu core.
- e-Reader support. ([Bug #171](http://mgba.io/b/171))


Supported Platforms
-------------------

- Windows Vista or newer
- OS X 10.7 (Lion)[<sup>[3]</sup>](#osxver) or newer
- Linux
- FreeBSD

Other Unix-like platforms, such as OpenBSD, are known to work as well, but are untested and not fully supported.

### System requirements

Requirements are minimal. Any computer that can run Windows Vista or newer should be able to handle emulation. Support for OpenGL 1.1 or newer is also required.

Downloads
---------

Downloads can be found on the official website, in the [Downloads][downloads] section. The source code can be found on [GitHub][source].

Controls
--------

Controls are configurable in the menu. The default gamepad controls are mapped so as to work with a DualShock 3. The default keyboard controls are as follows:

- **A**: X
- **B**: Z
- **L**: A
- **R**: S
- **Start**: Enter
- **Select**: Backspace

Compiling
---------

Compiling requires using CMake 2.8.11 or newer. GCC and Clang are both known to work to compile mGBA, but Visual Studio 2013 and older are known not to work. To use CMake to build on a Unix-based system, the recommended commands are as follows:

	mkdir build
	cd build
	cmake -DCMAKE_INSTALL_PREFIX:PATH=/usr ..
	make
	sudo make install

This will build and install mGBA into `/usr/bin` and `/usr/lib`. Dependencies that are installed will be automatically detected, and features that are disabled if the dependencies are not found will be shown after running the `cmake` command after warnings about being unable to find them.

### Dependencies

mGBA has no hard dependencies, however, the following optional dependencies are required for specific features. The features will be disabled if the dependencies can't be found.

- Qt 5: for the GUI frontend. Qt Multimedia or SDL are required for audio.
- SDL: for a more basic frontend and gamepad support in the Qt frontend. SDL 2 is recommended, but 1.2 is supported.
- zlib and libpng: for screenshot support and savestate-in-PNG support.
- libedit: for command-line debugger support.
- ffmpeg or libav: for video recording.
- libzip: for loading ROMs stored in zip files.
- ImageMagick: for GIF recording.

Footnotes
---------

<a name="missing">[1]</a> Currently missing features are

- OBJ window for modes 3, 4 and 5 ([Bug #5](http://mgba.io/b/5))
- Mosaic for transformed OBJs ([Bug #9](http://mgba.io/b/9))
- BIOS call RegisterRamReset is partially stubbed out ([Bug #141](http://mgba.io/b/141))
- Audio channel reset flags ([Bug #142](http://mgba.io/b/142))
- Game Pak prefetch ([Bug #195](http://mgba.io/b/195))
- BIOS call Stop, for entering sleep mode ([Bug #199](http://mgba.io/b/199))

<a name="flashdetect">[2]</a> Flash memory size detection does not work in some cases. These can be configured at runtime, but filing a bug is recommended if such a case is encountered.

<a name="osxver">[3]</a> 10.7 is only needed for the Qt port. The SDL port is known to work on 10.6, and may work on older.

[downloads]: http://mgba.io/downloads.html
[source]: https://github.com/mgba-emu/mgba/

Copyright
---------

mGBA is Copyright © 2013 – 2015 Jeffrey Pfau. It is distributed under the [Mozilla Public License version 2.0](https://www.mozilla.org/MPL/2.0/). A copy of the license is available in the distributed LICENSE file.

mGBA contains the following third-party libraries:

- [inih](https://code.google.com/p/inih/), which is copyright © 2009 Brush Technology and used under a BSD 3-clause license.
- [blip-buf](https://code.google.com/p/blip-buf/), which is copyright © 2003 – 2009 Shay Green and used under a Lesser GNU Public License.
- [LZMA SDK](http://www.7-zip.org/sdk.html), which is public doman.
