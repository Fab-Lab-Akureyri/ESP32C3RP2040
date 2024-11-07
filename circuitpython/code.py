import time
import board
import busio

# Define UART pins
TX_PIN = board.D6  # GPIO6, desired TX pin
RX_PIN = board.D7  # GPIO7, desired RX pin

# Initialize UART1
uart = busio.UART(TX_PIN, RX_PIN, baudrate=115200)

print("RP2040 UART Transmitter Initialized on UART1 (TX=D6, RX=D7)")

counter = 0

while True:
    message = f"Hello {counter}\n"
    uart.write(message.encode('utf-8'))
    print(f"Sent: {message.strip()}")
    counter += 1
    time.sleep(1)
