"""Check post-transfer draining with simulated UART input and timer ticks."""
from pathlib import Path
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]


class DrainTests(unittest.TestCase):
    def test_drain(self):
        with tempfile.TemporaryDirectory() as directory:
            executable = Path(directory) / "drain"
            subprocess.run([
                "cc", "-std=c11", "-Wall", "-Wextra", "-Werror", "-I.",
                "-Ibootloader/include", "tests/test_xmodem_drain.c",
                "bootloader/src/boot_xmodem_io.c", "-o", str(executable),
            ], cwd=ROOT, check=True)
            subprocess.run([str(executable)], check=True, timeout=5)
