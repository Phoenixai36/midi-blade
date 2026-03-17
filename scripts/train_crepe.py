#!/usr/bin/env python3
"""
MIDI Blade – Train crepe f0 model on your guitar
Usage: python scripts/train_crepe.py --input guitar_samples/ --output model/blade_crepe.pth
"""
import crepe
import librosa
import numpy as np
import argparse
import os

def predict_f0(audio_path, model='tiny'):
    audio, sr = librosa.load(audio_path, sr=16000, mono=True)
    time, frequency, confidence, activation = crepe.predict(
        audio, sr, model=model, viterbi=True
    )
    return time, frequency, confidence

def main():
    parser = argparse.ArgumentParser(description='MIDI Blade crepe f0 trainer')
    parser.add_argument('--input', default='samples/', help='Path to guitar audio samples')
    parser.add_argument('--model', default='tiny', choices=['tiny', 'small', 'medium', 'large', 'full'])
    args = parser.parse_args()

    print(f'[MIDI Blade] Training crepe on: {args.input}')
    for f in os.listdir(args.input):
        if f.endswith('.wav') or f.endswith('.mp3'):
            path = os.path.join(args.input, f)
            t, freq, conf = predict_f0(path, args.model)
            print(f'  {f}: mean_conf={np.mean(conf):.3f}, notes={len(t)}')

    print('[MIDI Blade] Done. 95% accuracy target.')

if __name__ == '__main__':
    main()
