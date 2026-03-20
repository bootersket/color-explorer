Sandbox project to tinker with color theory stuff to better understand the practical side of color calibration in pursuit of GI275-12872

Ideas:
### Color Space Explorer
Build a small “color space explorer”

Goal: understand RGB ↔ XYZ ↔ chromaticity.

What to implement:

* Input RGB (0–255)
* Convert to **linear** RGB (remove gamma)
* Convert to XYZ
* Compute chromaticity (x,y)

Then visualize it on a chromaticity diagram from CIE 1931 XYZ.

What you'll learn:

* gamma removal
* RGB→XYZ matrices
* chromaticity
* gamut triangles

A simple Python program with matplotlib works great.

<hr>