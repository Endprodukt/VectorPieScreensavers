"""make_test_track.py - a test track for trying the screensaver on a PC.

Writes music/test_120bpm.wav (30 s, stereo, 44.1 kHz: a kick on every beat,
a hi-hat between, a chord whose two sides drift apart so the XY scope draws
figures) and its beat map music/test_120bpm.beats (<seconds>\t<beat in bar>,
as vendor/VectorPie/sdk/docs/09-screensavers.md describes).

    python tools/make_test_track.py
    build/pc/saver_custom.exe music
"""
import math
import os
import struct
import wave

RATE, BPM, SECONDS = 44100, 120, 30
root = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "music")
os.makedirs(root, exist_ok=True)
beat = 60.0 / BPM

n = RATE * SECONDS
left, right = [0.0] * n, [0.0] * n
for i in range(n):
    t = i / RATE
    pos = t % beat
    kick = math.sin(2 * math.pi * (50 + 120 * math.exp(-pos * 30)) * pos) * math.exp(-pos * 9)
    hpos = (t + beat / 2) % beat
    hat = (((i * 1103515245 + 12345) >> 16) % 2000 / 1000.0 - 1) * math.exp(-hpos * 60) * 0.25
    bar = int(t / (beat * 4)) % 4
    root_hz = (110.0, 87.31, 130.81, 98.0)[bar]
    chord_l = sum(math.sin(2 * math.pi * root_hz * m * t) for m in (1, 1.5, 2)) / 3
    chord_r = sum(math.sin(2 * math.pi * root_hz * m * 1.002 * t + 0.6) for m in (1, 1.5, 2)) / 3
    left[i] = 0.55 * kick + hat + 0.3 * chord_l
    right[i] = 0.55 * kick + hat + 0.3 * chord_r

with wave.open(os.path.join(root, "test_120bpm.wav"), "wb") as w:
    w.setnchannels(2)
    w.setsampwidth(2)
    w.setframerate(RATE)
    frames = bytearray()
    for a, b in zip(left, right):
        frames += struct.pack("<hh", int(max(-1, min(1, a)) * 30000), int(max(-1, min(1, b)) * 30000))
    w.writeframes(bytes(frames))

with open(os.path.join(root, "test_120bpm.beats"), "w", newline="\n") as f:
    k = 0
    while k * beat < SECONDS:
        f.write("%.3f\t%d\n" % (k * beat, k % 4 + 1))
        k += 1
print("music/test_120bpm.wav and music/test_120bpm.beats written")
