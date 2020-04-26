"""
File name:  MVVisualiser.py
Author:     Gerbrand De Laender
Date:       23/04/2020
Email:      gerbrand.delaender@ugent.be
Brief:      E017920A, Assignment, motion vector visualisation
About:      Script that visualises a series of motion vectors, stored as a CSV
            file on top of their respective frames. A slider allows to change
            the current frame and their motion vectors.
"""

################################################################################
################################################################################

import matplotlib.pyplot as plt
import numpy as np
from matplotlib.widgets import Slider

################################################################################
################################################################################

def update_frame(n):
   ax_1.clear()
   U = 10 / 16 * data[dim * int(n / i_interval) : dim * (int(n / i_interval) + 1), 0]
   V = 10 / 16 * data[dim * int(n / i_interval) : dim * (int(n / i_interval) + 1), 1]
   frame = plt.imread(f"{directory}frame{int(n + 1):03}.png")
   ax_1.imshow(frame, extent = [0, x_dim, 0, y_dim])
   ax_1.quiver(X, Y, U, V, units = "xy", scale = 10, color = "red")

################################################################################

directory = "C:\\Users\\gerbr\\Code\\VideoCodec\\data\\foreman_50\\"
n_frames, i_interval = 50, 2
x_dim, y_dim, dim = 22, 18, 22 * 18

X, Y = np.meshgrid(np.arange(0.5, x_dim + 0.5, 1), np.arange(y_dim - 0.5, -0.5, -1))
data = np.genfromtxt(f"{directory}vectors.csv", delimiter = ',')

ax_1 = plt.figure().add_subplot(111)
ax_2 = plt.axes([0.1, 0.01, 0.8, 0.03])
slider = Slider(ax_2, "Frame", 0, n_frames - 1, 0, valstep = 1)
slider.on_changed(update_frame)
update_frame(0)

plt.show()

################################################################################
################################################################################