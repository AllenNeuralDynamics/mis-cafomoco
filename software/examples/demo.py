#!/usr/bin/env python3
"""Motor Api Demo."""

from time import sleep
from mis_focus_controller import FocusController


if __name__ == "__main__":

    MOTOR_INDEX = 2

    print("Connecting to mis-focus-controller... ", end=" ", flush=True)
    focus_rig = FocusController("/dev/ttyACM0")
    print("done.")

    on_time = 3.0
    #focus_rig.set_speed(MOTOR_INDEX, 30)
    #print(f"Moving motor {MOTOR_INDEX} forward for {on_time} [sec].")
    #focus_rig.time_move(MOTOR_INDEX, True, round(on_time * 1000), wait=True)
    #sleep(0.1)
    #print(f"Moving motor {MOTOR_INDEX} backward for {on_time} [sec].")
    #focus_rig.time_move(MOTOR_INDEX, False, round(on_time * 1000), wait=True)
    #print("End of demo.")


    for i in range(3):
        focus_rig.set_speed(i, 30)
        focus_rig.set_speed(i, 30)
        print(f"Moving motor {i} forward for {on_time} [sec].")
        focus_rig.time_move(i, True, round(on_time * 1000), wait=True)
        sleep(0.1)
        print(f"Moving motor {i} backward for {on_time} [sec].")
        focus_rig.time_move(i, False, round(on_time * 1000), wait=True)
    print("End of demo.")
