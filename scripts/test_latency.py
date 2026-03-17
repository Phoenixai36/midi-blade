#!/usr/bin/env python3
"""
MIDI Blade – Latency benchmark
Target: <5ms with ASIO 128buf
"""
import time
import numpy as np
import sounddevice as sd

BUFFER_SIZE = 128
SAMPLE_RATE = 48000
TARGET_MS = 5.0

def latency_test(duration_sec=5):
    print('[MIDI Blade] Latency Test')
    print(f'Buffer: {BUFFER_SIZE} samples @ {SAMPLE_RATE}Hz = {BUFFER_SIZE/SAMPLE_RATE*1000:.2f}ms base')

    latencies = []

    def callback(indata, outdata, frames, time_info, status):
        t_start = time.perf_counter()
        # Simulate blade filter processing
        for blade in range(6):
            _ = np.abs(np.fft.rfft(indata[:, 0])).max()
        t_end = time.perf_counter()
        latencies.append((t_end - t_start) * 1000)
        outdata[:] = indata

    with sd.Stream(samplerate=SAMPLE_RATE, blocksize=BUFFER_SIZE,
                   channels=1, callback=callback):
        sd.sleep(int(duration_sec * 1000))

    mean_ms = np.mean(latencies)
    max_ms = np.max(latencies)
    print(f'Processing latency: mean={mean_ms:.3f}ms, max={max_ms:.3f}ms')
    print(f'Status: {"✅ PASS" if max_ms < TARGET_MS else "❌ FAIL – tune ASIO buffer"}')

if __name__ == '__main__':
    latency_test()
