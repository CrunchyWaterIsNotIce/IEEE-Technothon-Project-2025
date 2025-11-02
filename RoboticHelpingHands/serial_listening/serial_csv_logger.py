import serial
import time

SERIAL_PORT = 'COM3'
BAUD_RATE = 115200
 
try:
    ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
    
except serial.SerialException as e:
    print(f'Error opening serial port: {e}.')


while True:
    try:
        line = ser.readline().strip().decode('utf-8')

        if line:
            print(f"Received: {line}") 

            if (line == "Press ~ to run record custom gesture:") or (line == "Press ~ to rerun recording:"):
                input_from_python = input()
                ser.write(input_from_python.encode())

    except serial.SerialException as e:
        print(f"Serial error: {e}")
        break
    except KeyboardInterrupt:
        print("Exiting.")
        break
    except UnicodeDecodeError:
        print("Could not decode, skipping line.")
        continue
    
    time.sleep(0.1) # delay to prevent busy-waiting
    
    
if 'ser' in locals() and ser.is_open:
    ser.close()