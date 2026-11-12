#!/usr/bin/env python3
"""Compare rendered held notes (tests/busy_match.c) with the reference's harmonic profiles: per note the levels of
harmonics 1..20 relative to the loudest, and the RMS difference in dB."""
import sys, wave, numpy as np
REF = {62: [-15, -17, -1, 0, -16, -29, -21, -21, -32, -14, -22, -32, -35, -40, -30, -33, -26, -37, -45, -40],
       64: [-17, -16, -3, 0, -16, -28, -21, -21, -33, -16, -23, -34, -36, -39, -32, -35, -28, -39, -47, -42],
       71: [-25, -13, 0, -3, -18, -30, -25, -22, -34, -17, -21, -36, -38, -41, -36, -39],
       76: [-19, -12, 0, -1, -19, -33, -25, -26, -35, -20, -27, -40, -41, -50, -45, -44]}
w = wave.open(sys.argv[1]); sr = w.getframerate()
x = np.frombuffer(w.readframes(w.getnframes()), dtype=np.int16).reshape(-1, 2)[:, 0].astype(float) / 32768
notes = [int(a) for a in sys.argv[2:]]
tot = []
for i, n in enumerate(notes):
    st = int(i * 1.5 * sr)
    seg = x[st + int(0.3 * sr): st + int(1.15 * sr)]
    f0 = 440 * 2 ** ((n - 69) / 12)
    N = 1 << 18
    S = np.abs(np.fft.rfft(seg * np.hanning(len(seg)), N)); f = np.fft.rfftfreq(N, 1 / sr)
    a = np.array([S[(f > h * f0 * 0.97) & (f < h * f0 * 1.03)].max() for h in range(1, 21)])
    d = 20 * np.log10(a / a.max())
    r = REF.get(n)
    line = ' '.join(f"{v:4.0f}" for v in d[:16])
    if r:
        k = len(r); e = np.sqrt(np.mean((d[:k] - np.array(r)) ** 2)); tot.append(e)
        print(f"{n}: {line}   rms diff {e:.1f} dB")
        print(f"ref {' '.join(f'{v:4.0f}' for v in r[:16])}")
    else:
        print(f"{n}: {line}")
if tot:
    print(f"mean rms diff {np.mean(tot):.2f} dB")
