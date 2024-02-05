#!/usr/bin/python3

import numpy as np
from matplotlib import pyplot as plot

sinc_scale = 1
n_entries = 1024

def sinc(xs):
    for x in xs:
        if x == 0: yield 1
        yield np.sin(x * np.pi) / (x * np.pi)


steps = np.linspace(-9, 9, num=n_entries)

sinc_table = list(sinc(steps * sinc_scale))

plot.plot(steps, sinc_table)
plot.show()

print("#include <array>")
print("namespace resample {")
print(f"std::array<float, {len(sinc_table)}> _sinc_table {{", end='')
print(", ".join(map(str, sinc_table)), end='')
print("};\n}\n")
