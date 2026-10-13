#!/usr/bin/env python3
"""Package an application binary for the bootloader.

The magic number and load address are not defined here: the Makefile reads
them from common/include/image_format.h and memory_map.h and passes them in,
so the bootloader and this script cannot disagree.
"""

import argparse
import struct
import zlib

from pathlib import Path


# Header format for the packaged image (little-endian), see app_image_header_t
HEADER_FORMAT = "<IIII"


def package_image(input_path: Path, output_path: Path, magic: int, load_address: int) -> None:
    application = input_path.read_bytes()

    if not application:
        raise ValueError("Application file is empty")

    crc = zlib.crc32(application)
    header = struct.pack(
        HEADER_FORMAT,
        magic,
        len(application),
        load_address,
        crc,
    )

    output_path.parent.mkdir(parents=True, exist_ok=True)
    output_path.write_bytes(header + application)

    print(f"Application size : {len(application)} bytes")
    print(f"Header size      : {len(header)} bytes")
    print(f"Package size     : {len(header) + len(application)} bytes")
    print(f"Load address     : 0x{load_address:08X}")
    print(f"Payload CRC32    : 0x{crc:08X}")


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Package an SDRAM application image for the bootloader"
    )

    parser.add_argument("input", type=Path, help="Path to the input application binary")
    parser.add_argument("output", type=Path, help="Path to the output packaged image")
    parser.add_argument("--magic", type=lambda s: int(s, 0), required=True,
                        help="APP_IMAGE_MAGIC from image_format.h")
    parser.add_argument("--load-address", type=lambda s: int(s, 0), required=True,
                        help="APP_SDRAM_BASE from memory_map.h")

    args = parser.parse_args()
    package_image(args.input, args.output, args.magic, args.load_address)


if __name__ == "__main__":
    main()
