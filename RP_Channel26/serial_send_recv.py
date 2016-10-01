#!/usr/bin/python -e

'''
Script for serial listener and receiver
'''

import serial
import sys
import time

ser = None
uInput = None

def serialOpen():
    global ser
    try:
        ser = serial.Serial(port="/dev/ttyUSB0",baudrate=19200,parity='N',bytesize=8,stopbits=1)
        print 'Serial connection established'
    except:
        print 'Looks like something went wrong.'
        sys.exit(0)

def userInput():
    global uInput
    uInput = raw_input("Enter serial string :")

def serialPrint():
    global ser
    global uInput
    try:
        sendString = uInput.decode('hex')
        print 'Send string is : ' + sendString.encode('hex')
        ser.write(sendString)
        time.sleep(1)
        print ser.read(ser.inWaiting()).encode('hex')
    except:
        pass

def main():
    global ser
    serialOpen()
    while True:
        try:
            userInput()
        except BaseException as ex:
            print 'Keyboard interrupt'
            try:
                ser.close()
                break
            except:
                pass
        serialPrint()

if __name__ == "__main__":
    main()
