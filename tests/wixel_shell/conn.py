import time
import serial
import os
if os.path.exists('/dev/ttyACM0'):
    print("Using ttyACM0")
    ser = serial.Serial(
        port = '/dev/ttyACM0',
        baudrate=9600,
        parity=serial.PARITY_NONE,
        stopbits=serial.STOPBITS_ONE,
        bytesize=serial.EIGHTBITS
    )
else:
    print("Using ttyACM1")
    ser = serial.Serial(
        port = '/dev/ttyACM1',
        baudrate=9600,
        parity=serial.PARITY_NONE,
        stopbits=serial.STOPBITS_ONE,
        bytesize=serial.EIGHTBITS
    )

ser.isOpen()
print("Serial driver for wixel shell\n")
input = 1
while 1:
    time.sleep(1)
    out = ""
    try:
        while ser.inWaiting() > 0:
            out += ser.read_all()
    except IOError as err:
        print("Error reading from serial with error: {0}".format(err))
    
    print(">> " + out)
    input = raw_input("> ")
    if input == 'exit':
        ser.close()
        exit()
    ser.write(input)
    ser.flush()
    input = 1
