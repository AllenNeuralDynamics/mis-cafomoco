#!/usr/bin/env python3
import serial
import re
from serial.serialutil import SerialException


# TODO: consider case where we send commands to the device faster than it can process them.


class MulticamFocusRings:
    """Class for interacting with the focus rings of the MIS instrument."""

    # On Linux, the symlink to the first detected vibratome.
    # Name set in udev rules and will increment with subsequent devices.
    DEFAULT_PORT_NAME = "/dev/ttyACM0"

    ENABLE_CMD = b"start\r\n"
    DISABLE_CMD = b"stop\r\n"


    def __init__(self, port_name=None):
        """ Connect to the hardware."""

        # Try to connect to the predefined port if no port is entered.
        # Use input port_name otherwise.
        # Controller implements a virtual serial port, so port settings are irrelevant.
        self.serial_port = None
        try:
            if port_name is None:
                self.serial_port = serial.Serial(self.__class__.DEFAULT_PORT_NAME)
            else:
               self.serial_port = serial.Serial(port_name)
        except (FileNotFoundError, SerialException):
            print("Error: Failed to connect to the Multicam Focus Rings. Is it plugged in?")
            raise


    def set_speed(self, motor_index : int, speed_percentage : int, wait=True):
        """set the corresponding motor's speed."""

        cmd = f"SET_SPEED {motor_index} {speed_percentage}\r\n".encode("ascii")
        self._blocking_write(cmd)
        if wait:
            while self.is_busy:
                pass


    def time_move(self, motor_index: int, forward: bool, move_time_ms, wait=True)
        """Rotate the specified motor forward (or backwards) for a specified number of milliseconds.

        :param forward: moves the device forward if true; otherwise backwards.
        """
        direction = 1 if forward else 0
        cmd = f"TIME_MOVE {motor_index} {direction} {move_time_ms}\r\n".encode("ascii")
        self._blocking_write(cmd)
        if wait:
            while self.is_busy:
                pass



    @property
    def is_busy(self):
        """True if the device is busy. False otherwise."""

        response = self._blocking_read()
        if response == "True":
            return True
        return False


    def _blocking_write(self, message):
        """Write a string; wait until it has exited the PC."""
        self.serial_port.write(message)
        while self.serial_port.out_waiting:
            pass


    def _blocking_read(self):
        """ Read an entire message.
        Blocks until an entire reply is received or timeout.
        """
        # make sure there is nothin the buffer to begin with. (Check that message is complete too.)

        cmd = "IS_BUSY\r\n".encode("ascii")
        self._blocking_write(cmd)

        # Read until the end of the line. Collect a fully-formed line before
        # processing input.
        # Tips from:
        # https://stackoverflow.com/questions/1093598/pyserial-how-to-read-the-last-line-sent-from-a-serial-device
        recv_buffer = []
        while True:
            if '\n' == recv_buffer[-1]:
                break
            recv_buffer += ser.read(1) # blocks until we get at least one char or timeout.
        # strip off \r\n
        return re.split("\r\n", "".join(recv_buffer))[0]

