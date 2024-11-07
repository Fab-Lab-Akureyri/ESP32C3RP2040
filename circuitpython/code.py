import time
import board
import busio
import neopixel

# Define UART pins
TX_PIN = board.D6  # GPIO6, desired TX pin
RX_PIN = board.D7  # GPIO7, desired RX pin

# Initialize UART1
uart = busio.UART(TX_PIN, RX_PIN, baudrate=115200)

print("RP2040 UART Receiver Initialized on UART1 (TX=D6, RX=D7)")

# Initialize built-in NeoPixel
pixel = neopixel.NeoPixel(board.NEOPIXEL, 1)
pixel.brightness = 0.2  # Adjust brightness as needed

while True:
    data = uart.readline()
    if data:
        command = data.decode('utf-8').strip()
        print(f"Received command: {command}")
        if command == "OFF":
            pixel.fill((0, 0, 0))
        elif command == "RED":
            pixel.fill((255, 0, 0))
        elif command == "GREEN":
            pixel.fill((0, 255, 0))
        elif command == "BLUE":
            pixel.fill((0, 0, 255))
        else:
            print(f"Unknown command: {command}")
    time.sleep(0.1)
