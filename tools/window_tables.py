#!/usr/bin/python3

from math import cos, pi

def blackman_term(m, M):
    if m >= 0 and m <= M:
        return 0.42 - \
            (0.5 * cos((2 * pi * m) / M)) + \
                (0.08 * cos((4 * pi * m) / M))
    return 0

n_rows = 32
blackman_size = 8
window_size = blackman_size + 1
half_window = int(blackman_size / 2) # is it 8, is it 9? it's wonder dog!

def make_blackman(m_offset):
    r = range(0, window_size)
    return [blackman_term(i + m_offset, blackman_size) for i in r]

print(f"std::array<float, {n_rows * (blackman_size + 1)}> windowTableData {{")

for i in range(0, n_rows):
    offset = (i / n_rows)
    print("\t" + ", ".join(map(str, make_blackman(offset ))) + ",")

print("};")

print(f"arm_bilinear_interp_instance_f32 windowTableInterpolator {{ {n_rows}, {blackman_size + 1}, windowTableData.data() }};")

