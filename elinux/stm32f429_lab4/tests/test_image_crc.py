"""Host checks for the packager and actual bootloader loader (Linux, cc required)."""

import importlib.util
from pathlib import Path
import struct
import subprocess
import tempfile
import unittest
import zlib


ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location("package_image", ROOT / "scripts/package_image.py")
packager = importlib.util.module_from_spec(spec)
spec.loader.exec_module(packager)


class ImageCrcTests(unittest.TestCase):
    def test_packaging_and_loader(self):
        with tempfile.TemporaryDirectory() as directory:
            directory = Path(directory)
            executable = directory / "image_loader_test"
            subprocess.run([
                "cc", "-std=c11", "-Wall", "-Wextra", "-Werror",
                "-I.", "-Icommon/include", "-Ibootloader/include",
                "tests/image_loader_test.c", "bootloader/src/crc32.c",
                "bootloader/src/image_loader.c", "-o", str(executable),
            ], cwd=ROOT, check=True)
            for payload in (b"123456789", bytes(range(256)), bytes(range(256)) * 17 + b"\xff"):
                with self.subTest(size=len(payload)):
                    source = directory / "application.bin"
                    target = directory / "application.img"
                    source.write_bytes(payload)
                    packager.package_image(source, target, 0x00070013, 0xD0000000)
                    image = target.read_bytes()
                    self.assertEqual(struct.unpack("<IIII", image[:16]),
                                     (0x00070013, len(payload), 0xD0000000, zlib.crc32(payload)))
                    self.assertEqual(image[16:], payload)
                    subprocess.run([str(executable), str(target)], check=True)

    def test_empty_image_rejected(self):
        with tempfile.TemporaryDirectory() as directory:
            source = Path(directory) / "empty.bin"
            target = Path(directory) / "empty.img"
            source.write_bytes(b"")
            with self.assertRaises(ValueError):
                packager.package_image(source, target, 0x00070013, 0xD0000000)
            self.assertFalse(target.exists())


if __name__ == "__main__":
    unittest.main()
