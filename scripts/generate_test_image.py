# import numpy as np

# w = 100
# h = 100

# img = np.zeros((h, w), dtype=np.uint8)

# img[50:, :] = 255

# img.tofile("../images/input/horizontal.raw")

# import numpy as np

# w = 100
# h = 100

# img = np.zeros((h, w), dtype=np.uint8)

# img[:, 50:] = 255

# img.tofile("../images/input/vertical.raw")

# import numpy as np

# w = 100
# h = 100

# img = np.zeros((h, w), dtype=np.uint8)

# for y in range(h):
#     for x in range(w):
#         if x > y:
#             img[y, x] = 255

# img.tofile("../images/input/diagonal.raw")

# import numpy as np

# img = np.full((100,100), 128, dtype=np.uint8)

# img.tofile("../images/input/uniform.raw")

# import numpy as np

# img = np.zeros((100,100), dtype=np.uint8)

# img[50,50] = 255

# img.tofile("../images/input/impulse.raw")

import numpy as np
import matplotlib.pyplot as plt

WIDTH = 100
HEIGHT = 100

img = np.fromfile(
    "../images/output/blur.raw",
    dtype=np.uint8
).reshape(HEIGHT, WIDTH)

plt.imshow(img, cmap="gray")
plt.show()