#!/usr/bin/python3

import sys
import serial
from time import sleep

def main():
    if len(sys.argv) < 2:
        print("Error: expected more command line arguments")
        print("Syntax: %s </dev/serial_port>" %sys.argv[0])
        exit(1)

    # 'Local Read' Frame acording to the datasheet
    msg = b'\x00\x00\xE2\x00\x00\x00\x15\xB8'

    # Open the serial port
    lora = serial.Serial(sys.argv[1], 9600, timeout=1)

    # Send the request through the serial port and wait
    # the read the serial port and print it out
    lora.write(msg)
    sleep(1)
    recv = lora.read_all()
    if recv != b'':
        for byte in recv:
            print("0x%02X" % byte, end='')
            print(' ', end='')
        print()

    # close the serial port
    lora.close()

    exit(0)

if __name__ == '__main__':
    main()
