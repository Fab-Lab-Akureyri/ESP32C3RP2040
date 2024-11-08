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
uart = busio.UART(TX_PIN, RX_PIN, baudrate=115200, timeout=0.1)  # Set a small timeout

print("RP2040 UART Transmitter Initialized on UART1 (TX=D6, RX=D7)")

# Initialize built-in NeoPixel
pixel = neopixel.NeoPixel(board.NEOPIXEL, 1)
pixel.brightness = 0.2  # Adjust brightness as needed

# Track the last time we sent system info
last_system_info_time = time.monotonic()
system_info_interval = 1.0  # Send system info every 1 second

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
    
    # Format data as a JSON-like string
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

# Main loop
while True:
    # Non-blocking UART read
    data = uart.read(32)  # Read up to 32 bytes, or None if nothing available
    if data:
        try:
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
        except UnicodeDecodeError:
            print("Received data could not be decoded")

    # Send system info at defined intervals
    current_time = time.monotonic()
    if current_time - last_system_info_time >= system_info_interval:
        system_info = collect_system_info()
        print("Sending system info:", system_info)
        uart.write((system_info + "\n").encode('utf-8'))
        last_system_info_time = current_time
