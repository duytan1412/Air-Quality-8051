#!/usr/bin/env python3
"""
parse_uart.py - UART Packet Validator for Air Quality System
============================================================
Connects to 8051 via Serial, captures UART packets, and validates format.
Usage:
    pip install pyserial
    python parse_uart.py COM3        # Windows
    python parse_uart.py /dev/ttyUSB0  # Linux
"""

import serial
import time
import re
import sys

# Configuration
BAUDRATE = 9600
TIMEOUT  = 2       # seconds per readline
DURATION = 10      # total capture time (seconds)
MIN_OK   = 3       # minimum valid packets to PASS

def parse_packet(line):
    """Validate UART packet format: [START] PM2.5: XXug/m3 | GAS: XXXppm | ..."""
    pattern = r'\[START\]\s*PM2\.5:\s*(\d+)ug/m3\s*\|\s*GAS:\s*(\d+)ppm\s*\|\s*TEMP:\s*([\d.]+)C\s*\|\s*HUMID:\s*(\d+)%'
    match = re.search(pattern, line)
    if match:
        return {
            'pm25':  int(match.group(1)),
            'gas':   int(match.group(2)),
            'temp':  float(match.group(3)),
            'humid': int(match.group(4)),
        }
    return None

def main():
    if len(sys.argv) < 2:
        print("Usage: python parse_uart.py <COM_PORT>")
        print("Example: python parse_uart.py COM3")
        sys.exit(1)

    port = sys.argv[1]
    print(f"[*] Connecting to {port} @ {BAUDRATE} baud...")

    try:
        ser = serial.Serial(port, BAUDRATE, timeout=TIMEOUT)
    except serial.SerialException as e:
        print(f"[ERROR] Cannot open {port}: {e}")
        sys.exit(1)

    print(f"[*] Capturing packets for {DURATION}s...\n")

    ok = 0
    total = 0
    alerts = 0
    start = time.time()

    while time.time() - start < DURATION:
        line = ser.readline().decode(errors='ignore').strip()
        if not line:
            continue

        total += 1
        print(f"  RX: {line}")

        data = parse_packet(line)
        if data:
            ok += 1
            # Boundary checks (sensor sanity)
            if data['pm25'] > 500:
                print(f"  ⚠ PM2.5 out of range: {data['pm25']}")
            if data['gas'] > 1000:
                print(f"  ⚠ GAS out of range: {data['gas']}")
            if data['temp'] < -10 or data['temp'] > 60:
                print(f"  ⚠ TEMP out of range: {data['temp']}")

        if '[ALERT]' in line:
            alerts += 1
            print(f"  🚨 ALERT detected!")

    ser.close()

    # Summary
    print(f"\n{'='*50}")
    print(f"  UART Capture Summary")
    print(f"{'='*50}")
    print(f"  Total lines received : {total}")
    print(f"  Valid packets        : {ok}")
    print(f"  Alerts               : {alerts}")
    print(f"  Duration             : {time.time()-start:.1f}s")
    print(f"{'='*50}")

    if ok >= MIN_OK:
        print(f"  ✅ PASS ({ok}/{MIN_OK} minimum valid packets)")
        sys.exit(0)
    else:
        print(f"  ❌ FAIL (only {ok}/{MIN_OK} valid packets)")
        sys.exit(2)

if __name__ == '__main__':
    main()
