#!/usr/bin/python3

from math import cos, pi

def blackman_term(m, M):
    return 0.42 - \
        (0.5 * cos((2 * pi * m) / M)) + \
            (0.08 * cos((4 * pi * m) / M))

blackman_size = 8

def make_blackman(m_offset):
    return [blackman_term(i + m_offset, blackman_size) for i in range(0, blackman_size + 1)]

print("std::array windowTableData {")

n_rows = 20
offset = 0
for i in range(0, 20):
    print(", ".join(map(str, make_blackman(offset))) + ",")
    offset += 1 / n_rows

print("}")

#values = ", ".join(map(str, blackman_table))

#print(f"constexpr std::array blackman_window = {{{values}}};")