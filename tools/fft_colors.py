#!/usr/bin/python3

import numpy as np

# from https://rgbcolorpicker.com/565
def rgb888_to_rgb565(red8, green8, blue8):
    # Convert 8-bit red to 5-bit red.
    red5 = round(red8 / 255 * 31)
    # Convert 8-bit green to 6-bit green.
    green6 = round(green8 / 255 * 63)
    # Convert 8-bit blue to 5-bit blue.
    blue5 = round(blue8 / 255 * 31)

    # Shift the red value to the left by 11 bits.
    red5_shifted = red5 << 11
    # Shift the green value to the left by 5 bits.
    green6_shifted = green6 << 5

    # Combine the red, green, and blue values.
    rgb565 = red5_shifted | green6_shifted | blue5

    return rgb565

def pack_color(rgb_arr):
    return rgb888_to_rgb565(rgb_arr[0], rgb_arr[1], rgb_arr[2])

def lerp(a, b, t):
    # how many times have I written this function? ...asked this question in a comment?
    return (1 - t) * a + t * b


high_amp_left = np.array((255, 0, 251))
low_amp_left = np.array((138, 0, 135))

high_amp_right = np.array((13, 0, 255))
low_amp_right = np.array((7, 0, 143))

def interp(a, b, steps):
    for s in steps:
        yield lerp(a, b, s)

def interp2d(aa, bb, steps):
    table = []
    for i, a in enumerate(aa):
        col = list(interp(a, bb[i], steps))
        table.append(col)
    return table

columns = 16
rows = 16

bright_row = list(interp(high_amp_left, high_amp_right, np.linspace(0, 1, columns)))
dim_row = list(interp(low_amp_left, low_amp_right, np.linspace(0, 1, columns)))

print(bright_row)
print(dim_row)

full_table = interp2d(bright_row, dim_row, np.linspace(0, 1, rows))

def remap(t):
    table = []
    for c in t:
        table.append(list(map(pack_color, c)))
    return table

int_table = remap(full_table)

print(int_table)
print(np.transpose(int_table))