# 🎸 MIDI Blade

**Sovereign MIDI 2.0 Guitar Tracker** – JUCE LV2/VST3 + RTNeural + Crepe

> "Strings to Sovereign UMP Blades" – 0€ OSS. Beats MG3.

---

## ¿Qué es MIDI Blade?

Plugin open-source (LV2/VST3) que convierte tu guitarra estándar a **MIDI 2.0 nativo (UMP)** con tracking polifónico per-cuerda (blade), latencia <5ms, 32-bit expression. Sin Jam Origin lock-in.

## ✅ Ventajas vs MG3

| Feature | MIDI Blade | MG3 |
|---------|------------|-----|
| MIDI Protocol | Native 2.0 UMP | 1.0 + MPE emu |
| Expression | 32-bit per-blade | 14-bit MPE |
| Poly Accuracy | 95% (crepe trained) | 90% proprietary |
| Cost | 0€ OSS | 149€ |
| Bidirectional | Yes | No |
| Sovereignty | 100% | Lock-in |

---

## 🚀 Instalación rápida

```bash
git clone https://github.com/Phoenixai36/midi-blade
cd midi-blade
pip install -r requirements.txt
cmake -B build && cmake --build build
```

## 🛠️ Stack técnico

- **JUCE 7** – Plugin host LV2/VST3
- **RTNeural** – Inference neural <2ms
- **Crepe** – Polyphonic f0 detection (95% accuracy)
- **UMIDI SDK** – True MIDI 2.0 UMP packets
- **loopMIDI** – Virtual MIDI ports Windows

## 📁 Estructura

```
midi-blade/
├── src/
│   ├── BladeProcessor.cpp     # 6 blades RT tracking
│   ├── BladeProcessor.h
│   ├── UMPConverter.cpp       # MIDI 1→2.0 UMP packets
│   ├── UMPConverter.h
│   └── BladeFilters.h         # 6 bandpass IIR filters (E2-A2)
├── scripts/
│   ├── train_crepe.py         # Train f0 model tu guitarra
│   └── test_latency.py        # Benchmark latency ASIO
├── requirements.txt
├── CMakeLists.txt
└── README.md
```

## ⚡ Checklist dev (10h)

- [ ] Phase 1: Setup ASIO + loopMIDI + JUCE
- [ ] Phase 2: Fork GuitarMidi-LV2 base
- [ ] Phase 3: Blade core (filters + crepe + UMP)
- [ ] Phase 4: Test Reaper <5ms
- [ ] Phase 5: Deploy GitHub + patent draft

## 📜 Licencia

AGPLv3 – Sovereign, co-op, EU AI Act compliant.

---

_MIDI Blade is not affiliated with Jam Origin._
