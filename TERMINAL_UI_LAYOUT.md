# Terminal UI Layout

```
┌─────────────────────────────────────────────────────────────────────────────────────────┐
│                                                                                         │
│                              Homecorrupter VST3 Plugin                                  │
│                                      903 x 675px                                        │
│                                                                                         │
│    ┌─────────────────────────────────────────────────────────────────────────────┐    │
│    │                                                                              │    │
│    │  ╔════════════════════════════════════════════════════════════════════╗   │    │
│    │  ║                     TERMINAL VIEW                                   ║   │    │
│    │  ║  Origin: (90, 75)                                                   ║   │    │
│    │  ║  Size: 760 x 380 pixels                                             ║   │    │
│    │  ║                                                                      ║   │    │
│    │  ║  Homecorrupter Terminal v1.0                                        ║   │    │
│    │  ║  Type 'help' for available commands                                 ║   │    │
│    │  ║                                                                      ║   │    │
│    │  ║  > info                                                              ║   │    │
│    │  ║  Homecorrupter VST/AU Plugin                                        ║   │    │
│    │  ║  Version 1.1.3                                                       ║   │    │
│    │  ║  Terminal powered by libghostty integration                         ║   │    │
│    │  ║  (c) igorski.nl 2020-2024                                           ║   │    │
│    │  ║                                                                      ║   │    │
│    │  ║  > time                                                              ║   │    │
│    │  ║  Current time: 2026-02-13 19:00:00                                  ║   │    │
│    │  ║                                                                      ║   │    │
│    │  ║  > █                                                                 ║   │    │
│    │  ║                                                                      ║   │    │
│    │  ║                                                                      ║   │    │
│    │  ║                                                                      ║   │    │
│    │  ╚════════════════════════════════════════════════════════════════════╝   │    │
│    │                                                                              │    │
│    └─────────────────────────────────────────────────────────────────────────────┘    │
│                                                                                         │
│              [Resample]    [Resample LFO]                                              │
│              ─────●────     ─────●────                                                  │
│                                                                                         │
│              [Resolution]  [Bit Crush LFO]                                             │
│              ─────●────     ─────●────                                                  │
│                                                                                         │
│              [Playback]    [Playback LFO]                                              │
│              ─────●────     ─────●────                                                  │
│                                                                                         │
│              [Wet Mix]     [Dry Mix]                                                   │
│              ─────●────     ─────●────                                                  │
│                                                                                         │
│                                                                      v1.1.3             │
└─────────────────────────────────────────────────────────────────────────────────────────┘
```

## Features Visible in the UI

### Monitor Screen (Terminal Area)
- **Location**: Center-left of the retro TV interface
- **Appearance**: Classic green-on-black terminal aesthetic
- **Interactive**: Click to focus, type commands, see real-time output
- **Cursor**: Blinking green cursor indicates active input

### Plugin Controls
The original plugin controls remain on the right side:
- Resample rate slider with LFO control
- Bit depth/resolution slider with LFO control  
- Playback rate slider with LFO control
- Wet/Dry mix controls

### Integration
The terminal seamlessly integrates into the existing retro TV interface, replacing the static monitor image with a functional command-line interface while preserving all original audio processing controls.
