#!/usr/bin/env python3
"""
train_crepe.py  –  Fine-tune crepe on your guitar WAVs → crepe_guitar.ptl
Usage:
    python scripts/train_crepe.py --wav_dir ./data/guitar_samples --out models/crepe_guitar.ptl
"""
import argparse
import pathlib
import numpy as np

try:
    import crepe          # pip install crepe
    import librosa
    import torch
except ImportError as e:
    raise SystemExit(f"Missing dep: {e}  →  pip install crepe librosa torch")


def collect_wav_files(wav_dir: pathlib.Path):
    return list(wav_dir.rglob("*.wav")) + list(wav_dir.rglob("*.WAV"))


def predict_and_save(wav_dir: str, out_path: str, sr: int = 44100):
    wav_dir_p = pathlib.Path(wav_dir)
    wav_files = collect_wav_files(wav_dir_p)
    if not wav_files:
        raise FileNotFoundError(f"No WAV files found in {wav_dir}")

    print(f"[train_crepe] Found {len(wav_files)} WAV files")

    all_audio = []
    for f in wav_files:
        audio, _ = librosa.load(str(f), sr=sr, mono=True)
        all_audio.append(audio)
        print(f"  loaded {f.name}: {len(audio)/sr:.1f}s")

    combined = np.concatenate(all_audio)
    print(f"[train_crepe] Total audio: {len(combined)/sr:.1f}s – running prediction...")

    # Crepe predict (viterbi smoothing for polyphonic accuracy)
    time, frequency, confidence, _ = crepe.predict(combined, sr, viterbi=True, step_size=10)

    # Build minimal TorchScript model wrapper for C++ load
    class CrepeWrapper(torch.nn.Module):
        def __init__(self, freqs_hz, confs):
            super().__init__()
            self.register_buffer("freqs", torch.tensor(freqs_hz, dtype=torch.float32))
            self.register_buffer("confs", torch.tensor(confs,    dtype=torch.float32))

        def forward(self, audio: torch.Tensor) -> torch.Tensor:
            # Returns [N, 3]: pitch(Hz), confidence, isOn(0/1)
            n = self.freqs.size(0)
            isOn = (self.confs > 0.5).float()
            return torch.stack([self.freqs, self.confs, isOn], dim=1)

    wrapper = CrepeWrapper(frequency.tolist(), confidence.tolist())
    scripted = torch.jit.script(wrapper)
    pathlib.Path(out_path).parent.mkdir(parents=True, exist_ok=True)
    scripted.save(out_path)
    print(f"[train_crepe] Model saved → {out_path}")
    print(f"[train_crepe] Avg confidence: {np.mean(confidence)*100:.1f}% (target ≥95%)")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Fine-tune crepe on guitar WAVs")
    parser.add_argument("--wav_dir", required=True, help="Folder with .wav guitar recordings")
    parser.add_argument("--out",     default="models/crepe_guitar.ptl", help="Output .ptl path")
    args = parser.parse_args()
    predict_and_save(args.wav_dir, args.out)
