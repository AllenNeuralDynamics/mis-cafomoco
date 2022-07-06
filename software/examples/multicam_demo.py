#!/usr/bin/env python3
"""Motor Api Demo."""

from time import sleep
from multicam_focus import MulticamFocusRig


if __name__ == "__main__":

    print("Connecting to Multicam Focus Rig... ", end=" ", flush=True)
    focus_rig = MulticamFocusRig("/dev/ttyACM0")
    print("done.")

    on_time = 3.0
    focus_rig.set_speed(0, 30)
    print(f"Moving motor 0 forward for {on_time} [sec].")
    focus_rig.time_move(0, True, round(on_time * 1000), wait=True)
    sleep(0.1)
    print(f"Moving motor 0 backward for {on_time} [sec].")
    focus_rig.time_move(0, False, round(on_time * 1000), wait=True)
    print("End of demo.")
