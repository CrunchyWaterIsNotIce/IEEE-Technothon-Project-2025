import serial
import time
import sys
SERIAL_PORT = 'COM3'
BAUD_RATE = 115200
 
try:
    ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=0.01)
    print(f'--- Python connected to {SERIAL_PORT} at {BAUD_RATE} baud ---')
    time.sleep(2)
except serial.SerialException as e:
    print(f'--- Error opening serial port: {e} ---')
    sys.exit(1)

try:
    while True:
        # reads all available data immediately without blocking (prior attempt)
        if ser.in_waiting > 0:
            try:
                line = ser.readline().decode().strip()
                
                if line:
                    print(line)
                    
                    if "Press ~ to " in line:
                        user_input = input()
                        ser.write(user_input.encode())
                        ser.flush() # data is sent immediately, needed
            except UnicodeDecodeError:
                # skips invalid characters
                pass
    
        time.sleep(0.001) # prevents CPU spinning?

except KeyboardInterrupt:
    print('--- Exited ---')
except serial.SerialException as e:
    print(f"Serial error: {e}")
finally:
    if ser.is_open:
        ser.close()
        print("Serial port closed.")