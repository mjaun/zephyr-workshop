#!/usr/bin/env python3
"""Send a signed firmware image to the demo board over its USB serial port.

The device stores the image in slot1, requests a test upgrade and reboots.
MCUboot then verifies the signature and swaps the images; the new
application confirms itself on startup.

Protocol (see app/src/serial_dfu.c), stop-and-wait, little endian:
  request:  0xA5 | command (u8) | length (u16) | payload | crc32 (u32)
            crc32 = zlib.crc32 over command, length and payload
  response: 0x5A | status (u8)

  START  (0x01): payload = image size (u32)
  DATA   (0x02): payload = offset (u32) + 1..512 data bytes
  FINISH (0x03): no payload

Usage:
  python scripts/dfu.py /dev/ttyACM1
  python scripts/dfu.py /dev/ttyACM1 build/demo/app/zephyr/zephyr.signed.bin
"""

import argparse
import struct
import sys
import zlib
from pathlib import Path

import serial

FRAME_START = 0xA5
RESPONSE_START = 0x5A

CMD_START = 0x01
CMD_DATA = 0x02
CMD_FINISH = 0x03

MAX_DATA_SIZE = 512
MCUBOOT_IMAGE_MAGIC = 0x96F3B83D

STATUS_OK = 0x00
STATUS_FRAME_ERROR = 0x01
STATUS_NAMES = {
    0x00: "ok",
    0x01: "frame error",
    0x02: "invalid command",
    0x03: "invalid state",
    0x04: "invalid offset",
    0x05: "invalid size",
    0x06: "flash error",
}

DEFAULT_IMAGE = Path(__file__).resolve().parent.parent / "build/demo/app/zephyr/zephyr.signed.bin"

# erasing the whole slot on START takes a while
START_TIMEOUT = 10.0
RESPONSE_TIMEOUT = 1.0
RETRIES = 3


class DfuError(Exception):
    pass


def build_frame(command: int, payload: bytes = b"") -> bytes:
    header = struct.pack("<BH", command, len(payload))
    crc = zlib.crc32(header + payload)
    return bytes([FRAME_START]) + header + payload + struct.pack("<I", crc)


def read_response(port: serial.Serial, timeout: float) -> int | None:
    """Returns the status byte, or None on timeout."""
    port.timeout = timeout
    while True:
        byte = port.read(1)
        if not byte:
            return None
        if byte[0] != RESPONSE_START:
            continue  # skip garbage
        status = port.read(1)
        return status[0] if status else None


def transfer(port: serial.Serial, command: int, payload: bytes = b"",
             timeout: float = RESPONSE_TIMEOUT, retries: int = RETRIES) -> None:
    """Sends a frame and waits for an OK response, retrying on timeouts and frame errors."""
    frame = build_frame(command, payload)
    for _ in range(retries + 1):
        port.write(frame)
        status = read_response(port, timeout)
        if status == STATUS_OK:
            return
        if status is not None and status != STATUS_FRAME_ERROR:
            raise DfuError(f"device reported: {STATUS_NAMES.get(status, hex(status))}")
    reason = "no response" if status is None else STATUS_NAMES[status]
    raise DfuError(f"command 0x{command:02x} failed after {retries + 1} attempts ({reason})")


def send_image(port_name: str, image: bytes) -> None:
    with serial.Serial(port_name, baudrate=115200) as port:
        port.reset_input_buffer()

        print(f"Erasing slot1 for {len(image)} bytes...")
        transfer(port, CMD_START, struct.pack("<I", len(image)), timeout=START_TIMEOUT)

        for offset in range(0, len(image), MAX_DATA_SIZE):
            chunk = image[offset:offset + MAX_DATA_SIZE]
            transfer(port, CMD_DATA, struct.pack("<I", offset) + chunk)
            done = offset + len(chunk)
            print(f"\rSending {done * 100 // len(image):3d}% ({done}/{len(image)} bytes)", end="", flush=True)
        print()

        # no retries: the device reboots right after acknowledging
        transfer(port, CMD_FINISH, retries=0)
        print("Done, the device reboots and MCUboot installs the image.")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("port", help="serial port of the device, e.g. /dev/ttyACM1")
    parser.add_argument("image", nargs="?", type=Path, default=DEFAULT_IMAGE,
                        help=f"signed image (default: {DEFAULT_IMAGE})")
    args = parser.parse_args()

    image = args.image.read_bytes()
    if len(image) < 4 or struct.unpack_from("<I", image)[0] != MCUBOOT_IMAGE_MAGIC:
        print(f"error: {args.image} is not a signed MCUboot image (use zephyr.signed.bin)", file=sys.stderr)
        return 1

    try:
        send_image(args.port, image)
    except (DfuError, serial.SerialException) as e:
        print(f"\nerror: {e}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
