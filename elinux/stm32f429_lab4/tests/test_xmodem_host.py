"""Build and run the standalone CRC16/XMODEM C tests with a native compiler."""
import os
from pathlib import Path
import shlex
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]


class XmodemHostTests(unittest.TestCase):
    def test_c_suites(self):
        with tempfile.TemporaryDirectory() as directory:
            for name in ("test_crc16", "test_xmodem", "test_xmodem_receive"):
                with self.subTest(suite=name):
                    executable = Path(directory) / name
                    sources = [f"tests/{name}.c", "lib/crc/crc16.c"]
                    if name != "test_crc16":
                        sources.append("lib/protocol/xmodem/xmodem.c")
                    subprocess.run([
                        *shlex.split(os.environ.get("HOST_CC", "cc")),
                        "-std=c11", "-Wall", "-Wextra", "-Werror", "-g", "-I.",
                        *shlex.split(os.environ.get("HOST_TEST_CFLAGS", "")),
                        "-UNDEBUG", *sources, "-o", str(executable),
                    ], cwd=ROOT, check=True)
                    subprocess.run([str(executable)], check=True, timeout=10)
