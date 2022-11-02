#!/usr/bin/env python3
import serial
import re
from serial.serialutil import SerialException

from typing import List


# TODO: consider case where we send commands to the device faster than it can process them.


class FocusController:
    """Class for interacting with the mis-focus-controller device."""

    DEFAULT_PORT_NAME = "/dev/ttyACM0"
    MOTOR_COUNT = 6
    DEFAULT_SPEED_PERCENT = 25

    def __init__(self, port_name=DEFAULT_PORT_NAME):
        """ Connect to the hardware."""
        # Try to connect to the predefined port if no port is entered.
        # Use input port_name otherwise.
        # RP2040 implements a virtual serial port, so baud rate is irrelevant.
        self.ser = None
        try:
            self.ser = serial.Serial(port_name)
            #for motor_index in range(self.__class__.MOTOR_COUNT):
            #    self.set_speed(motor_index, self.__class__.DEFAULT_PORT_NAME)
        except (FileNotFoundError, SerialException):
            print("Error: Failed to connect to the mis-focus-controller device. "
                  "Is it plugged in?")
            raise

    def set_speed(self, motor_index : int, speed_percentage : int,
                  wait: bool = True):
        """set the corresponding motor's speed."""

        cmd = f"SET_SPEED {motor_index} {speed_percentage}\r\n".encode("ascii")
        self._blocking_write(cmd)
        if wait:
            while self.is_busy:
                pass

    def time_move(self, motor_index: int, forward: bool, move_time_ms: int,
                  wait: bool =True):
        """Rotate specified motor forward (or backwards) for desired time [ms].

        :param forward: moves the device forward if true; otherwise backwards.
        """
        direction = 1 if forward else 0
        cmd = f"TIME_MOVE {motor_index} {direction} {move_time_ms}\r\n".encode("ascii")
        self._blocking_write(cmd)
        if wait:
            while self.is_busy:
                pass

    def set_speeds(self, motor_indices : List, speed_percentages: List,
                   wait: bool = True):

        # Convert to comma-delimited string of args.
        motors = "".join(str(motor_indices).strip("[]").split())
        speeds = "".join(str(speed_percentages).strip("[]").split())
        cmd = f"SET_SPEED {motors} {speeds}\r\n".encode("ascii")
        self._blocking_write(cmd)
        if wait:
            while self.is_busy:
                pass

    def time_moves(self, motor_indices : List[int],
                   speed_percentages: List[int],
                   direction_indices : List[bool],
                   wait: bool = True):

        # Convert to comma-delimited string of args.
        motors = "".join(str(motor_indices).strip("[]").split())
        dirs_as_ints = [int(direction) for direction in direction_indices]
        dirs = "".join(str(dirs_as_ints).strip("[]").split())
        speeds = "".join(str(speed_percentages).strip("[]").split())
        cmd = f"TIME_MOVE {motors} {directions} {speeds}\r\n".encode("ascii")
        self._blocking_write(cmd)
        if wait:
            while self.is_busy:
                pass

    @property
    def is_busy(self):
        """True if the device is busy. False otherwise."""

        cmd = "IS_BUSY\r\n".encode("ascii")
        self._blocking_write(cmd)
        response = self._blocking_read()
        assert response in ["True", "False"], \
            "Error: got unknown response while polling busy status. " \
            f"Device response is: '{repr(response)}'"
        if response == "True":
            return True
        return False

    def _blocking_write(self, message):
        """Write a string; wait until it has exited the PC."""
        #print(f"Sending: '{repr(message)}'")
        self.ser.write(message)
        while self.ser.out_waiting:
            pass

    def _blocking_read(self):
        """ Read an entire message.
        Blocks until an entire reply is received or timeout.
        """
        # Read until the end of the line. Collect a fully-formed line before
        # processing input.
        # read line. strip \r\n.
        return self.ser.read_until(b'\r\n').decode("utf8").rstrip("\r\n")

