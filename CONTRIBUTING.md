# Contributing to Drum Library

Thank you for your interest in contributing! This document provides guidelines for contributing to the project.

## Code of Conduct

- Be respectful and inclusive
- Focus on constructive feedback
- Help others learn and grow

## Getting Started

1. **Fork the repository**
2. **Clone your fork:**
   ```bash
   git clone https://github.com/yourusername/drum-library.git
   cd drum-library
   ```
3. **Create a branch:**
   ```bash
   git checkout -b feature/your-feature-name
   ```
4. **Build and test** (see [BUILD.md](docs/BUILD.md))

## Development Workflow

### Making Changes

1. **Write code** following the style guide below
2. **Test thoroughly** - manual testing checklist in [ARCHITECTURE.md](docs/ARCHITECTURE.md)
3. **Update documentation** if adding features
4. **Commit with clear messages:**
   ```bash
   git commit -m "Add velocity curve editor to kit view"
   ```

### Code Style

**C++ Style:**
- C++17 standard
- JUCE coding conventions
- Use `juce::` namespace prefix explicitly
- Prefer `auto` for complex iterator types
- Keep functions under 50 lines
- Use descriptive variable names

**Example:**
```cpp
void SamplerEngine::noteOn(int midiNote, int velocity)
{
    std::vector<int> candidates;
    for (int i = 0; i < static_cast<int>(samples.size()); ++i)
    {
        auto& s = samples[i];
        if (s.midiNote == midiNote &&
            velocity >= s.velLow &&
            velocity <= s.velHigh)
        {
            candidates.push_back(i);
        }
    }
    // ...
}
```

**Formatting:**
- Indent: 4 spaces (no tabs)
- Braces: Allman style (opening brace on new line)
- Line length: 100 characters max
- Use clang-format with JUCE style

**Comments:**
- Use `//` for single-line comments
- Use `/** */` for documentation comments
- Explain *why*, not *what*

### Commit Messages

Format: `<type>: <description>`

**Types:**
- `feat`: New feature
- `fix`: Bug fix
- `docs`: Documentation only
- `style`: Code style/formatting
- `refactor`: Code restructuring
- `perf`: Performance improvement
- `test`: Adding tests
- `chore`: Build/tooling changes

**Examples:**
```
feat: Add MIDI flash animation to kit view drum pads
fix: Resolve Logic Pro AU blank UI on macOS 15
docs: Update BUILD.md with universal binary instructions
refactor: Extract voice allocation to separate method
```

## Pull Request Process

1. **Update documentation** for any user-facing changes
2. **Update CHANGELOG.md** under `[Unreleased]` section
3. **Ensure builds pass:**
   ```bash
   cmake --build build --config Release
   ```
4. **Create pull request** with:
   - Clear title and description
   - Link to related issues
   - Screenshots for UI changes
5. **Address review feedback**

## Areas for Contribution

### High Priority

- [ ] Fix Logic Pro AU compatibility on macOS 15
- [ ] Implement MIDI flash animation on kit view
- [ ] Add sample preview in file browser
- [ ] Implement Mixer page (volume/pan/mute)
- [ ] Implement FX page (reverb/delay)

### Medium Priority

- [ ] Add preset system
- [ ] Implement sample streaming
- [ ] Add drag-and-drop sample loading
- [ ] Create unit tests
- [ ] Add Windows/Linux support

### Low Priority

- [ ] Multi-output routing
- [ ] Built-in groove player
- [ ] Cloud sample library
- [ ] Advanced round-robin modes

## Testing

### Manual Testing Checklist

Before submitting PR, verify:

- [ ] Standalone app launches and shows UI
- [ ] LOAD SAMPLES button opens file browser
- [ ] Samples load from folder
- [ ] MIDI triggers correct samples
- [ ] Velocity affects volume
- [ ] Round-robin cycles through samples
- [ ] Tab switching works (KIT/MIXER/FX)
- [ ] Window resizes correctly
- [ ] State saves/restores in DAW
- [ ] AU validates in Logic Pro
- [ ] VST3 loads in Reaper/Ableton

### Unit Tests (Future)

We plan to add unit tests using JUCE's UnitTest framework:

```cpp
class SamplerEngineTests : public juce::UnitTest
{
public:
    SamplerEngineTests() : UnitTest("SamplerEngine") {}
    
    void runTest() override
    {
        beginTest("Sample loading");
        SamplerEngine engine;
        engine.loadSamplesFromFolder(testFolder);
        expect(engine.getNumSamples() > 0);
    }
};
```

## Documentation

### What to Document

- New features (README.md)
- API changes (ARCHITECTURE.md)
- Build changes (BUILD.md)
- Version history (CHANGELOG.md)

### Documentation Style

- Use Markdown
- Include code examples
- Add screenshots for UI features
- Keep language clear and concise

## Bug Reports

Use GitHub Issues with this template:

```markdown
**Describe the bug**
A clear description of what the bug is.

**To Reproduce**
Steps to reproduce:
1. Load plugin in Logic Pro
2. Click LOAD SAMPLES
3. Select folder with 100+ samples
4. Observe crash

**Expected behavior**
Plugin should load samples without crashing.

**Environment:**
- OS: macOS 14.2
- DAW: Logic Pro 11.0.1
- Plugin version: 1.0.0
- Build: AU

**Screenshots**
If applicable, add screenshots.

**Additional context**
Console logs, crash reports, etc.
```

## Feature Requests

Use GitHub Issues with this template:

```markdown
**Feature description**
A clear description of the feature.

**Use case**
Why is this feature needed? What problem does it solve?

**Proposed solution**
How should this feature work?

**Alternatives considered**
Other approaches you've thought about.

**Additional context**
Mockups, examples from other plugins, etc.
```

## Architecture Guidelines

### Adding New UI Components

1. Create component class in `PluginEditor.h`
2. Initialize in `PluginEditor` constructor
3. Position in `resized()` method
4. Add paint routine if custom drawing needed

### Adding New Audio Features

1. Add to `SamplerEngine` or `PluginProcessor`
2. Update `prepareToPlay()` if state needed
3. Process in `processBlock()`
4. Add UI controls in `PluginEditor`
5. Update documentation

### Adding New File Formats

1. Implement `juce::AudioFormat` subclass
2. Register with `AudioFormatManager`
3. Update filename parsing in `SamplerEngine`
4. Document in README.md

## Release Process

1. **Update version** in `CMakeLists.txt`:
   ```cmake
   project(DrumLibrary VERSION 1.1.0)
   ```

2. **Update CHANGELOG.md:**
   - Move `[Unreleased]` items to new version section
   - Add release date

3. **Tag release:**
   ```bash
   git tag -a v1.1.0 -m "Release version 1.1.0"
   git push origin v1.1.0
   ```

4. **Build release binaries:**
   ```bash
   cmake --build build --config Release
   ```

5. **Create GitHub release** with:
   - Release notes from CHANGELOG
   - Compiled binaries (AU, VST3, Standalone)
   - Installation instructions

## Questions?

- **General questions**: [GitHub Discussions](https://github.com/yourusername/drum-library/discussions)
- **Bug reports**: [GitHub Issues](https://github.com/yourusername/drum-library/issues)
- **JUCE help**: [JUCE Forum](https://forum.juce.com/)

## License

By contributing, you agree that your contributions will be licensed under the MIT License (see [LICENSE](LICENSE)).
