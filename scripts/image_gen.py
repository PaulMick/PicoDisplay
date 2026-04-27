import numpy as np
import cv2

# set these manually
width = 23
height = 32
fname = ""
cname = ""

black_is_transparent = True

cv_im = cv2.imread(f"input/{fname}.png")
np_im = np.asarray(cv_im)

print(f"#define {cname.upper()}_WIDTH {width}")
print(f"#define {cname.upper()}_HEIGHT {height}")
print(f"static uint32_t {cname}_img[{height}][{width}] = {'{'}")
for y in range(height):
    print("    {", end = "")
    for x in range(width):
        b = int(np_im[y, x, 0])
        g = int(np_im[y, x, 1])
        r = int(np_im[y, x, 2])
        if r == 1: r = 0
        if g == 1: g = 0
        if b == 1: b = 0
        xbgr = (b << 16) | (g << 8) | r
        if black_is_transparent and r == 0 and g == 0 and b == 0:
            xbgr = xbgr | (1 << 24)
        print(f"{xbgr:#0{8}x}, ", end = "")
    print("},")
print("};")