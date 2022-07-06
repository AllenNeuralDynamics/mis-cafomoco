#!/usr/bin/env python3
"""Prompt."""

from time import sleep
from inpromptu import Inpromptu
from multicam_focus import MulticamFocusRig


if __name__ == "__main__":

    focus_rig = MulticamFocusRig("/dev/ttyACM0")

    focus_rig_prompt = Inpromptu(focus_rig)
    focus_rig_prompt.cmdloop()
