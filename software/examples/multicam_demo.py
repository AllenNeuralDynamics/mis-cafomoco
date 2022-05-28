#!/usr/bin/env python3
"""Connects to the vibratome and sets the desired power level."""

from multicam_focus import MulticamFocusRig


if __name__ == "__main__":

    print("Connecting to Multicam Focus Rig... ", end=" ", flush=True)
    focus_rig = MulticamFocusRig() # take the default port: /dev/ttyACM0
    print("done.")

    # Set the speed.
    focus_rig.set_speeds([0, 1, 2, 3], [30, 30, 30, 30])
    focus_rig.time_moves([0, 1, 2, 3], [True, True, True, True], [1000, 1000, 1000, 1000])
