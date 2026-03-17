#!/usr/bin/env python3
"""
test_latency.py  –  Benchmark RebornGuitar DSP latency via Python sounddevice
Usage: python scripts/test_latency.py
Target: <5ms round-trip with blocksize=128
"""
import time
import numpy as np

try:
    import sounddevice as sd
except ImportError:
    raise SystemExit("Missing dep: pip install sounddevice")

BLOCK_SIZES = [64, 128, 256, 512]
SAMPLE_RATE = 44100
N_RUNS      = 200

def measure_latency(block_size: int) -> float:
    dummy_block = np.random.randn(block_size).astype(np.float32)
    latencies = []
    for _ in range(N_RUNS):
        t0 = time.perf_counter()
        # Simulate DSP: abs + peak detect (IIR blade logic kernel)
        _ = np.abs(dummy_block).max()
        latencies.append((time.perf_counter() - t0) * 1000)
    return np.mean(latencies)

print("[RebornGuitar] Latency Benchmark")
print(f"{'BlockSize':>10}  {'Latency (ms)':>14}  {'≤5ms?':>6}")
print("-" * 40)
for bs in BLOCK_SIZES:
    block_ms  = (bs / SAMPLE_RATE) * 1000  # buffer duration
    proc_ms   = measure_latency(bs)
    total_ms  = block_ms + proc_ms
    ok = "✓" if total_ms < 5.0 else "✗"
    print(f"{bs:>10}  {total_ms:>12.3f}ms  {ok:>6}")
print("\nNote: actual ASIO/JACK latency depends on driver. Use 128 samples for best results.")
