import serial
import time
import sys
import csv

SERIAL_PORT = 'COM3'
BAUD_RATE = 115200

raw_data_filename = ""
raw_data = []
reading_raw_data = False
count = 1
 
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
                    
                    if not reading_raw_data:
                        if "Now recording gesture" in line: # initializes gesture .csv file name
                            raw_data_filename = line[line.index('[') + 1: line.index(']')] + f'_{count}.csv'
                        if "proximity" in line: # prepares to record data from serial
                            raw_data.append(line.split(','))
                            reading_raw_data = True
                        if "Press ~ to " in line: # toggles recording from python input
                            user_input = input()
                            ser.write(user_input.encode())
                            ser.flush()                           
                    else:
                        if "Finished current recording." in line: # creates and write on gesture .csv
                            with open(raw_data_filename, mode='w', newline='') as file:
                                writer = csv.writer(file)
                                writer.writerows(raw_data)
                            
                            raw_data_filename = ""
                            raw_data.clear()
                            reading_raw_data = False
                            count += 1
                        else: # adds list data to list raw_data
                            raw_data.append(line.split(','))
                            
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