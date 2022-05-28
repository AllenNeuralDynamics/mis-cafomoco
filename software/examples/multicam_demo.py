#!/usr/bin/env python3
"""Connects to the vibratome and sets the desired power level."""

from multicam_focus import MulticamFocusRig


if __name__ == "__main__":

    print("Connecting to Multicam Focus Rig... ", end=" ", flush=True)
    focus_rig = MulticamFocusRig() # take the default port: /dev/ttyACM0
    print("done.")

    # Set the speed.
    focus_rig.set_speed(0, 50)
    focus_rig.time_move(0, True, 3000) # move motor 0 forward for 3[sec]; wait.
