import serial

with serial.Serial('/dev/cu.usbserial-1140', 115200, timeout=1) as ser:
	while True:
		print(ser.readline().decode("utf-8"), end='')

