#!/usr/bin/env python3
import serial
from serial.serialutil import SerialException


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



    def set_speed(self, motor_index : int, speed_percentage : int):
        """set the corresponding motor's speed."""

        cmd = f"SET_SPEED {motor_index} {speed_percentage}\r\n".encode("ascii")
        self.serial_port.write(cmd)


    def time_move(self, motor_index: int, forward: bool, move_time_ms)
        """Rotate the specified motor forward (or backwards) for a specified number of milliseconds.

        :param forward: moves the device forward if true; otherwise backwards.
        """
        direction = 1 if forward else 0
        cmd = f"TIME_MOVE {motor_index} {direction} {move_time_ms}\r\n".encode("ascii")
        self.serial_port.write(cmd)

    @property
    def is_busy(self):
        """True if the device is busy; false otherwise.

        Blocks for a serial port reply or timeout.
        """
        # make sure there is nothin the buffer to begin with. (Check that message is complete too.)

        cmd = "IS_BUSY\r\n".encode("ascii")
        self.serial_port.write(cmd)

        # wait for a carriage return.
        raise NotImplementedError
