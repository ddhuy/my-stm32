#!/usr/bin/env python3

import argparse
import struct

from pathlib import Path


APP_IMAGE_MAGIC      = 0x0007_0013
APP_IMAGE_LOAD_ADDR  = 0xD000_0000

# Header format for the packaged image (little-endian)
HEADER_FORMAT = "<IIII"


def package_image(input_path: Path, output_path: Path) -> None:
    application = input_path.read_bytes()

    if not application:
        raise ValueError("Application file is empty")

    header = struct.pack(
        HEADER_FORMAT,
        APP_IMAGE_MAGIC,
        len(application),
        APP_IMAGE_LOAD_ADDR,
        0, # Reserved for future use
    )

    output_path.parent.mkdir(parents=True, exist_ok=True)
    output_path.write_bytes(header + application)

    print(f"Application size : {len(application)} bytes")
    print(f"Header size      : {len(header)} bytes")
    print(f"Package size     : {len(header) + len(application)} bytes")


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Package an SDRAM application image for the bootloader"
    )

    parser.add_argument("input", type=Path, help="Path to the input application binary")
    parser.add_argument("output", type=Path, help="Path to the output packaged image")

    args = parser.parse_args()
    package_image(args.input, args.output)


if __name__ == "__main__":
    main()