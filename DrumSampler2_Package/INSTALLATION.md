# Drum Sampler 2.0 - Installation Guide

**Version**: 2.0.0  
**Date**: March 21, 2026  
**Platform**: macOS

---

## Package Contents

This package contains:

```
DrumSampler2_Package/
├── AU/
│   └── Drum Sampler 2.component
├── VST3/
│   └── Drum Sampler 2.vst3
├── Standalone/
│   └── Drum Sampler 2.app
└── Documentation/
    ├── INSTALLATION.md (this file)
    ├── USER_MANUAL.md
    ├── RELEASE_CHECKLIST.md
    └── PHASE6_COMPLETE.md
```

---

## System Requirements

- **macOS**: 10.13 (High Sierra) or later
- **CPU**: Intel Core i5 or better (Apple Silicon M1/M2/M3 supported)
- **RAM**: 4 GB minimum, 8 GB recommended
- **Disk Space**: 500 MB for plugin
- **DAW**: Any AU or VST3 compatible host (Logic Pro, Ableton Live, Pro Tools, etc.)

---

## Installation Instructions

### Option 1: Manual Installation (Recommended)

#### Install AU Plugin (for Logic Pro, GarageBand, etc.)

1. Open Finder and navigate to the package folder
2. Copy `AU/Drum Sampler 2.component`
3. Paste into: `/Library/Audio/Plug-Ins/Components/`
   - Press `Cmd+Shift+G` in Finder to open "Go to Folder"
   - Type: `/Library/Audio/Plug-Ins/Components/`
   - Paste the component file

#### Install VST3 Plugin (for Ableton Live, FL Studio, etc.)

1. Open Finder and navigate to the package folder
2. Copy `VST3/Drum Sampler 2.vst3`
3. Paste into: `/Library/Audio/Plug-Ins/VST3/`
   - Press `Cmd+Shift+G` in Finder
   - Type: `/Library/Audio/Plug-Ins/VST3/`
   - Paste the VST3 file

#### Install Standalone App

1. Copy `Standalone/Drum Sampler 2.app`
2. Paste into: `/Applications/`
3. Double-click to launch

### Option 2: Terminal Installation

Open Terminal and run these commands:

```bash
# Navigate to package directory
cd /path/to/DrumSampler2_Package

# Install AU
sudo cp -R "AU/Drum Sampler 2.component" "/Library/Audio/Plug-Ins/Components/"

# Install VST3
sudo cp -R "VST3/Drum Sampler 2.vst3" "/Library/Audio/Plug-Ins/VST3/"

# Install Standalone
cp -R "Standalone/Drum Sampler 2.app" "/Applications/"

# Set permissions
sudo chmod -R 755 "/Library/Audio/Plug-Ins/Components/Drum Sampler 2.component"
sudo chmod -R 755 "/Library/Audio/Plug-Ins/VST3/Drum Sampler 2.vst3"
```

---

## Post-Installation

### 1. Restart Your DAW

After installation, restart your DAW to scan for new plugins.

### 2. Verify Installation

**In Logic Pro**:
1. Open Logic Pro
2. Create a new Software Instrument track
3. Click the instrument slot
4. Navigate to: AU Instruments → Manufacturer → Drum Sampler 2

**In Ableton Live**:
1. Open Ableton Live
2. Go to Preferences → Plug-ins
3. Click "Rescan"
4. Look for "Drum Sampler 2" in the VST3 section

**Standalone**:
1. Open Finder
2. Navigate to Applications
3. Double-click "Drum Sampler 2.app"

### 3. First Launch

1. The plugin will open with default settings
2. Click "LOAD SAMPLES" to select your drum sample folder
3. The plugin will scan and load compatible audio files (WAV, AIFF, MP3)
4. Start playing!

---

## macOS Security (Important!)

### Gatekeeper Warning

If you see a message saying "Drum Sampler 2 cannot be opened because it is from an unidentified developer":

**Solution**:
1. Open **System Preferences** → **Security & Privacy**
2. Click the **General** tab
3. You'll see a message about "Drum Sampler 2" being blocked
4. Click **"Open Anyway"**
5. Confirm by clicking **"Open"**

**Alternative Method**:
1. Right-click (or Control-click) the plugin/app
2. Select **"Open"**
3. Click **"Open"** in the dialog

### For Advanced Users

To bypass Gatekeeper entirely for this package:

```bash
# Remove quarantine attribute
sudo xattr -r -d com.apple.quarantine "/Library/Audio/Plug-Ins/Components/Drum Sampler 2.component"
sudo xattr -r -d com.apple.quarantine "/Library/Audio/Plug-Ins/VST3/Drum Sampler 2.vst3"
xattr -r -d com.apple.quarantine "/Applications/Drum Sampler 2.app"
```

---

## Uninstallation

To remove Drum Sampler 2:

```bash
# Remove AU
sudo rm -rf "/Library/Audio/Plug-Ins/Components/Drum Sampler 2.component"

# Remove VST3
sudo rm -rf "/Library/Audio/Plug-Ins/VST3/Drum Sampler 2.vst3"

# Remove Standalone
rm -rf "/Applications/Drum Sampler 2.app"

# Remove user data (optional)
rm -rf ~/Library/Application\ Support/DrumSampler2
```

---

## Troubleshooting

### Plugin Not Showing in DAW

1. **Verify installation location**:
   - AU: `/Library/Audio/Plug-Ins/Components/`
   - VST3: `/Library/Audio/Plug-Ins/VST3/`

2. **Rescan plugins** in your DAW:
   - Logic Pro: Preferences → Plug-in Manager → Reset & Rescan
   - Ableton Live: Preferences → Plug-ins → Rescan
   - Pro Tools: Setup → Plug-ins → Rescan

3. **Check permissions**:
   ```bash
   ls -la "/Library/Audio/Plug-Ins/Components/Drum Sampler 2.component"
   ```
   Should show: `drwxr-xr-x` (755 permissions)

4. **Restart your Mac** (sometimes required)

### Standalone App Won't Open

1. Check Security & Privacy settings (see above)
2. Try removing quarantine attribute (see above)
3. Check Console.app for error messages

### Plugin Crashes DAW

1. Check DAW compatibility (see System Requirements)
2. Update to latest macOS version
3. Increase audio buffer size in DAW preferences
4. Check error logs in the plugin's debug console (BUGS button)

### No Sound

1. Verify samples are loaded (click LOAD SAMPLES)
2. Check channel is not muted in mixer
3. Verify DAW track is armed/monitoring
4. Check audio interface is connected and selected

---

## Getting Help

### Documentation

- **User Manual**: See `Documentation/USER_MANUAL.md`
- **Technical Docs**: See `Documentation/PHASE6_COMPLETE.md`

### Support

- **Email**: support@drumsampler2.com
- **Forum**: forum.drumsampler2.com
- **Discord**: discord.gg/drumsampler2

### Reporting Issues

1. Click the **"BUGS"** button in the plugin
2. Review the error log
3. Click **"Export Log"**
4. Email the log file to support with a description of the issue

---

## Quick Start

Once installed:

1. **Load the plugin** in your DAW or open standalone
2. **Click "LOAD SAMPLES"** and select a folder with drum samples
3. **Play** using MIDI keyboard or click drum pads
4. **Adjust** volume, pan, and effects in the Mixer tab
5. **Save** your kit with `Cmd+S`

For detailed instructions, see the User Manual in the Documentation folder.

---

## License

This is a beta/development build. For production use, please obtain a proper license.

**© 2026 Drum Sampler. All rights reserved.**

---

## Version Information

- **Version**: 2.0.0
- **Build Date**: March 21, 2026
- **Build Type**: Release
- **Formats**: AU, VST3, Standalone
- **Architecture**: Universal (Intel + Apple Silicon)

Enjoy making music with Drum Sampler 2.0! 🥁
