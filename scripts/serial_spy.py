#!/usr/bin/python3

import sys
import serial
from time import sleep

def main():
    if len(sys.argv) < 2:
        print("Error: expected more command line arguments")
        print("Syntax: %s </dev/serial_port>" %sys.argv[0])
        exit(1)

    try:
        # Open the serial port
        lora = serial.Serial(sys.argv[1], 9600, timeout=None)

        while True:
            sleep(1)
            if lora.readable():
                recv = lora.read_all()
                if recv != b'':
                    for byte in recv:
                        print("0x%02X" % byte, end='')
                        print(' ', end='')
                    print()
                lora.flush()

    except KeyboardInterrupt:
        # close the serial port
        lora.close()
        exit(0)

if __name__ == '__main__':
    main()
