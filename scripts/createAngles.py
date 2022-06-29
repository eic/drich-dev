"""
@author Jesse Campbell
"""

import numpy as np
import sys
# import matplotlib.pyplot as plt
circle = 2 * np.pi


def makeAngles(theta_min: float, theta_max: float, theta_increment: float, hit_density: int) -> list:
    """
    :param theta_min: minimum polar angle of photon, in radian4
    :param theta_max: maximum polar angle of photon, in radians
    :param theta_increment: increment of polar angle between theta_min and theta_max
    :param hit_density: the amount of photon hits for the smallest polar angle
    :return: returns a list of tuples containing polar and azimuthal angles, in radians, in the form
    [(polar, azimuthal), (...), ...] for generating photons that will have an even distribution on the dRICH sensor
    """

    angles = []
    rings = int(np.round((theta_max - theta_min)/theta_increment))
    thetas = np.linspace(theta_min, theta_max, num=rings)
    phis = np.linspace(hit_density, hit_density * (theta_increment + 2), num=rings, dtype=int)

    for r, t in convert(thetas, phis):
        angles.append(tuple((r, t)))
    """
        plt.plot(r * np.cos(t), r * np.sin(t), 'bo')
    plt.show()
    """
    return angles


def convert(thetas, phis):
    # Helper function for makeAngles()
    for i in range(len(thetas)):
        for j in range(phis[i]):
            yield thetas[i], j * (circle/phis[i])


if __name__ == '__main__':  # Code for running makeAngles() in command line
    args = sys.argv
    theta_min = float(args[2])
    theta_max = float(args[3])
    theta_increment = float(args[4])
    hit_density = int(args[5])
    globals()[args[1]](theta_min, theta_max, theta_increment, hit_density)
