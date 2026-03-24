# Build Guide

Complete instructions for building Drum Library from source on macOS.

## Prerequisites

### Required Tools

1. **Xcode** (14.0 or later)
   ```bash
   # Install from Mac App Store, then:
   xcode-select --install
   ```

2. **CMake** (3.22 or later)
   ```bash
   # Download CMake 3.28.1
   curl -L "https://github.com/Kitware/CMake/releases/download/v3.28.1/cmake-3.28.1-macos-universal.tar.gz" -o cmake.tar.gz
   tar -xzf cmake.tar.gz
   sudo mv cmake-3.28.1-macos-universal /usr/local/
   
   # Add to PATH (add to ~/.zshrc for persistence)
   export PATH="/usr/local/cmake-3.28.1-macos-universal/CMake.app/Contents/bin:$PATH"
   ```

3. **JUCE 8.0.4**
   ```bash
   # Download JUCE
   curl -L "https://github.com/juce-framework/JUCE/archive/refs/tags/8.0.4.tar.gz" -o juce.tar.gz
   tar -xzf juce.tar.gz
   sudo mv JUCE-8.0.4 /usr/local/JUCE-8.0.4
   ```

### System Requirements

- macOS 10.15 (Catalina) or later
- 4 GB RAM minimum (8 GB recommended)
- 2 GB free disk space
- Apple Silicon (arm64) or Intel (x86_64)

## Quick Start

```bash
# Clone repository
git clone https://github.com/yourusername/drum-library.git
cd drum-library

# Configure
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release

# Build all targets
cmake --build build --config Release -j4

# Install
./scripts/install.sh
```

## Detailed Build Steps

### 1. Clone Repository

```bash
git clone https://github.com/yourusername/drum-library.git
cd drum-library
```

### 2. Update CMakeLists.txt JUCE Path

Edit `CMakeLists.txt` line 8 to point to your JUCE installation:

```cmake
# Change this line:
add_subdirectory(/tmp/JUCE-8.0.4 jucelib)

# To your actual JUCE path:
add_subdirectory(/usr/local/JUCE-8.0.4 jucelib)
```

### 3. Configure CMake

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
```

**Expected output:**
```
-- The C compiler identification is AppleClang 16.0.0
-- The CXX compiler identification is AppleClang 16.0.0
-- Configuring JUCE...
-- Building juceaide
-- Configuring done (25.1s)
-- Generating done (0.0s)
-- Build files have been written to: .../build
```

**Common errors:**

- **"JUCE not found"**: Check JUCE path in CMakeLists.txt
- **"CMake version too old"**: Upgrade to CMake 3.22+
- **"Xcode not found"**: Run `xcode-select --install`

### 4. Build Targets

Build all targets (AU, VST3, Standalone):

```bash
cmake --build build --config Release -j4
```

Or build specific targets:

```bash
# AU only
cmake --build build --config Release --target DrumLibrary_AU

# VST3 only
cmake --build build --config Release --target DrumLibrary_VST3

# Standalone only
cmake --build build --config Release --target DrumLibrary_Standalone
```

**Build time:** ~2-5 minutes depending on CPU

**Expected output:**
```
[  3%] Building CXX object CMakeFiles/DrumLibrary.dir/Source/SamplerEngine.cpp.o
[ 20%] Linking CXX static library libDrum Library_SharedCode.a
[ 37%] Built target DrumLibrary
[ 98%] Linking CXX CFBundle shared module Drum Library.component
[100%] Built target DrumLibrary_AU
[100%] Built target DrumLibrary_VST3
[100%] Built target DrumLibrary_Standalone
```

### 5. Locate Build Artifacts

```bash
ls -R build/DrumLibrary_artefacts/Release/
```

**Output:**
```
AU/
  Drum Library.component/
VST3/
  Drum Library.vst3/
Standalone/
  Drum Library.app/
```

### 6. Install Plugins

#### Manual Installation

```bash
# AU
cp -R "build/DrumLibrary_artefacts/Release/AU/Drum Library.component" \
  ~/Library/Audio/Plug-Ins/Components/

# VST3
cp -R "build/DrumLibrary_artefacts/Release/VST3/Drum Library.vst3" \
  ~/Library/Audio/Plug-Ins/VST3/
```

#### Code Signing (Required for macOS)

```bash
# Sign AU
codesign --force --deep --sign - \
  ~/Library/Audio/Plug-Ins/Components/Drum\ Library.component

# Sign VST3
codesign --force --deep --sign - \
  ~/Library/Audio/Plug-Ins/VST3/Drum\ Library.vst3
```

**Verify signature:**
```bash
codesign -dv ~/Library/Audio/Plug-Ins/Components/Drum\ Library.component
```

Expected: `Signature=adhoc`

#### Clear DAW Cache

```bash
# Logic Pro
rm -f ~/Library/Caches/AudioUnitCache/com.apple.audiounits.cache

# Ableton Live
rm -rf ~/Library/Preferences/Ableton/Live*/Cache/

# Reaper
rm -rf ~/Library/Application\ Support/REAPER/reaper-vstplugins64.ini
```

### 7. Verify Installation

```bash
# Check files exist
ls ~/Library/Audio/Plug-Ins/Components/Drum\ Library.component
ls ~/Library/Audio/Plug-Ins/VST3/Drum\ Library.vst3

# Check code signature
codesign -dv ~/Library/Audio/Plug-Ins/Components/Drum\ Library.component 2>&1 | grep "Signature"
```

## Build Configurations

### Debug Build

```bash
cmake -S . -B build-debug -DCMAKE_BUILD_TYPE=Debug
cmake --build build-debug --config Debug
```

Debug builds include:
- Assertions enabled
- No optimization
- Debug symbols
- Larger binary size

### Release Build (Recommended)

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

Release builds include:
- Optimizations enabled (`-O3`)
- No debug symbols
- Smaller binary size
- Better performance

### Universal Binary (Intel + Apple Silicon)

Edit `CMakeLists.txt` to add:

```cmake
set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64")
```

Then rebuild:

```bash
cmake -S . -B build-universal -DCMAKE_BUILD_TYPE=Release
cmake --build build-universal --config Release
```

Verify:
```bash
lipo -info build-universal/DrumLibrary_artefacts/Release/AU/Drum\ Library.component/Contents/MacOS/Drum\ Library
```

Expected: `Architectures in the fat file: arm64 x86_64`

## Troubleshooting

### Build Fails with "JuceHeader.h not found"

**Cause:** Using old JUCE-style includes  
**Fix:** Source files should use module-specific includes:

```cpp
// Wrong:
#include <JuceHeader.h>

// Correct:
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
```

### "CGWindowListCreateImage is unavailable"

**Cause:** JUCE 7 on macOS 15  
**Fix:** Use JUCE 8.0.4 or later

### Plugin Doesn't Show in DAW

1. **Check installation:**
   ```bash
   ls ~/Library/Audio/Plug-Ins/Components/Drum\ Library.component
   ```

2. **Check code signature:**
   ```bash
   codesign -dv ~/Library/Audio/Plug-Ins/Components/Drum\ Library.component
   ```

3. **Clear DAW cache** (see step 6 above)

4. **Rescan plugins in DAW:**
   - Logic Pro: Settings → Plug-in Manager → Reset & Rescan All
   - Ableton Live: Preferences → Plug-ins → Rescan
   - Reaper: Preferences → Plug-ins → VST → Re-scan

### Logic Pro Shows Blank UI

**Known Issue:** JUCE 8.0.4 AU wrapper + Logic Pro on macOS 15  
**Workaround:** Use Standalone app or VST3 in different DAW  
**Status:** Awaiting JUCE 8.0.5+ fix

### Build is Slow

**Tips:**
- Use `-j` flag: `cmake --build build -j8` (adjust number to CPU cores)
- Use Ninja generator: `cmake -G Ninja -S . -B build`
- Build specific target only: `--target DrumLibrary_Standalone`

## Clean Build

```bash
# Remove build directory
rm -rf build/

# Reconfigure and rebuild
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

## Advanced Options

### Custom JUCE Path

```bash
cmake -S . -B build \
  -DCMAKE_BUILD_TYPE=Release \
  -DJUCE_DIR=/path/to/JUCE
```

### Custom Install Prefix

```bash
cmake -S . -B build \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX=/custom/path
```

### Enable LTO (Link-Time Optimization)

Already enabled via `juce_recommended_lto_flags` in CMakeLists.txt.

### Disable Specific Formats

Edit `CMakeLists.txt` line 14:

```cmake
# Build only Standalone
FORMATS Standalone

# Build only VST3
FORMATS VST3

# Build AU and VST3
FORMATS AU VST3
```

## Continuous Integration

### GitHub Actions Example

```yaml
name: Build

on: [push, pull_request]

jobs:
  build:
    runs-on: macos-latest
    steps:
      - uses: actions/checkout@v3
      
      - name: Install JUCE
        run: |
          curl -L "https://github.com/juce-framework/JUCE/archive/refs/tags/8.0.4.tar.gz" -o juce.tar.gz
          tar -xzf juce.tar.gz
          sudo mv JUCE-8.0.4 /usr/local/
      
      - name: Configure
        run: cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
      
      - name: Build
        run: cmake --build build --config Release -j4
      
      - name: Upload Artifacts
        uses: actions/upload-artifact@v3
        with:
          name: drum-library-macos
          path: build/DrumLibrary_artefacts/Release/
```

## Next Steps

After successful build:

1. **Test the Standalone app:**
   ```bash
   open build/DrumLibrary_artefacts/Release/Standalone/Drum\ Library.app
   ```

2. **Load sample folder** via LOAD SAMPLES button

3. **Test in DAW** (Logic Pro, Ableton, Reaper)

4. **Read [ARCHITECTURE.md](ARCHITECTURE.md)** for code structure

5. **Contribute** - see [CONTRIBUTING.md](../CONTRIBUTING.md)

## Support

- **Build issues**: [GitHub Issues](https://github.com/yourusername/drum-library/issues)
- **JUCE help**: [JUCE Forum](https://forum.juce.com/)
- **CMake help**: [CMake Documentation](https://cmake.org/documentation/)
