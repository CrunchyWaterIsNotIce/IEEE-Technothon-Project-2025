import serial

SERIAL_PORT = 'COM3'
BAUD_RATE = 115200
 
try:
    ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)

    while True:
        line = ser.readline().strip().decode()
        if line:
            print(f"Listened message: {line}")
    
except serial.SerialException as e:
    print(f'Error opening serial port: {e}.')
except KeyboardInterrupt:
    print(f'-- Stopped serial port listening --')