"""
@author Jesse Campbell
"""

import numpy as np
import sys
import matplotlib.pyplot as plt
circle = 2 * np.pi
epsilon = 1e-2  # to avoid zero division


def makeAngles(theta_min: float, theta_max: float, num_rings: int, hit_density: int) -> list:
    """
    :param theta_min: minimum polar angle of dRICH in radians
    :param theta_max: maximum polar angle of dRICH in radians
    :param num_rings: number of concentric rings
    :param hit_density: the amount of photon hits for the minimum polar angle
    :return: returns a list of tuples containing polar and azimuthal angles, in radians, in the form
    [(polar, azimuthal), (...), ...] for generating photons that will have an even distribution on the dRICH sensor
    """

    angles = []
    thetas = np.linspace(theta_min, theta_max, num=num_rings)
    phis = np.linspace(hit_density, hit_density * (theta_max / (theta_min + epsilon)), num=num_rings, dtype=int)

    for r, t in convert(thetas, phis):
        angles.append(tuple((r, t)))
    """
        plt.plot(r * np.cos(t), r * np.sin(t), 'bo')
    plt.show()
    """
    return angles


def convert(thetas, phis):
    # Helper function for makePositions()
    for i in range(len(thetas)):
        for j in range(phis[i]):
            yield thetas[i], j * (circle / phis[i])


if __name__ == '__main__':  # Code for running makePositions() in command line
    args = sys.argv
    theta_min = float(args[2])
    theta_max = float(args[3])
    num_rings = int(args[4])
    hit_density = int(args[5])
    globals()[args[1]](theta_min, theta_max, num_rings, hit_density)
