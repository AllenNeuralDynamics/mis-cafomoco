#!/usr/bin/env python3
"""Prompt."""

from time import sleep
from inpromptu import Inpromptu
from mis_focus_controller import FocusController


if __name__ == "__main__":

    focus_rig = FocusController("/dev/ttyACM0")

    focus_rig_prompt = Inpromptu(focus_rig)
    focus_rig_prompt.cmdloop()
