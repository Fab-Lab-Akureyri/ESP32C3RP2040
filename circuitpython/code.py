import time
import board
import busio
import neopixel
import gc
import microcontroller
import os

# Define UART pins
TX_PIN = board.D6  # GPIO6, desired TX pin
RX_PIN = board.D7  # GPIO7, desired RX pin

# Initialize UART
uart = busio.UART(TX_PIN, RX_PIN, baudrate=115200)

print("RP2040 UART Transmitter Initialized on UART1 (TX=D6, RX=D7)")

# Initialize built-in NeoPixel
pixel = neopixel.NeoPixel(board.NEOPIXEL, 1)
pixel.brightness = 0.2  # Adjust brightness as needed

def collect_system_info():
    # Run garbage collection and get free memory
    gc.collect()
    free_memory = gc.mem_free()
    
    # Get CPU temperature
    cpu_temperature = microcontroller.cpu.temperature
    
    # Get filesystem information
    filesystem_info = os.statvfs("/")
    total_storage = filesystem_info[0] * filesystem_info[2]  # Total storage in bytes
    free_storage = filesystem_info[0] * filesystem_info[3]   # Free storage in bytes
    
    # Get CPU frequency
    cpu_frequency = microcontroller.cpu.frequency
    
    # Get reset reason
    reset_reason = microcontroller.cpu.reset_reason
    
    # Get device runtime
    runtime = time.monotonic()
    
    # Format data as a JSON-like string without f-strings
    system_info = '{{"free_memory": {}, "cpu_temperature": {:.2f}, "total_storage": {}, "free_storage": {}, "cpu_frequency": {}, "reset_reason": "{}", "runtime": {:.2f}}}'.format(
        free_memory,
        cpu_temperature,
        total_storage,
        free_storage,
        cpu_frequency,
        reset_reason,
        runtime
    )

    
    return system_info

while True:
    # Check for commands from ESP32-C3
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
    
    # Collect and send system information every second
    system_info = collect_system_info()
    print("Sending system info:", system_info)
    uart.write((system_info + "\n").encode('utf-8'))
    time.sleep(1)
