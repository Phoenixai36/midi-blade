# RebornGuitar – Build Guide

## Prerequisites
- CMake ≥ 3.22
- C++17 compiler (GCC 11+, MSVC 2022, Clang 14+)
- JUCE 7.x (git submodule)

```bash
git submodule add https://github.com/juce-framework/JUCE
git submodule update --init --recursive
```

---

## Build Mode A – IIR Blade Tracker (no ML, fast)
Simple hex-blade bandpass filters, no LibTorch needed.

```bash
cmake -B build
cmake --build build --config Release
```

Outputs:
- `build/RebornGuitar_artefacts/VST3/RebornGuitar.vst3`
- `build/RebornGuitar_artefacts/Standalone/RebornGuitar`

---

## Build Mode B – Crepe AI Model (LibTorch, 98% poly accuracy)

### 1. Download LibTorch
```bash
# Linux CPU
wget https://download.pytorch.org/libtorch/cpu/libtorch-cxx11-abi-shared-with-deps-2.2.0%2Bcpu.zip
unzip libtorch-*.zip
```

### 2. Train your model
```bash
pip install crepe librosa torch
python scripts/train_crepe.py --wav_dir ./data/guitar_samples --out models/crepe_guitar.ptl
```

### 3. CMake with Torch
```bash
cmake -B build \
  -DREBORN_USE_TORCH=ON \
  -DTorch_DIR=$(pwd)/libtorch/share/cmake/Torch

cmake --build build --config Release
```

---

## Load in Ableton Live 12
1. Copy `RebornGuitar.vst3` → `C:/Program Files/Common Files/VST3/` (Win) or `~/Library/Audio/Plug-Ins/VST3/` (Mac)
2. Ableton > Preferences > Plug-ins > Rescan
3. Insert on Audio track (guitar input)
4. Create MIDI track, set Input: `RebornGuitar MIDI Out`
5. Set buffer 128 samples ASIO → target latency <3ms

---

## MCP Server (AI text→MIDI)
```bash
pip install fastapi uvicorn python-rtmidi
uvicorn scripts.perse_mcp:app --port 9000
# POST http://localhost:9000/mcp/midi?prompt=Cmaj7+arp
```

---

## Latency Benchmark
```bash
python scripts/test_latency.py
```
Target: <5ms total with blocksize=128
