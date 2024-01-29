#!/usr/bin/python3

from math import cos, pi

def blackman_term(m, M):
    return 0.42 - \
        (0.5 * cos((2 * pi * m) / M)) + \
            (0.08 * cos((4 * pi * m) / M))

blackman_size = 8

blackman_table = [blackman_term(i, blackman_size) for i in range(0, blackman_size + 1)]

values = ", ".join(map(str, blackman_table))

print(f"constexpr std::array blackman_window = {{{values}}};")