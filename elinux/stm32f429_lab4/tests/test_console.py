"""Exercise terminal behavior with a fake USART byte transport."""
from pathlib import Path
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]


class ConsoleTests(unittest.TestCase):
    def test_console(self):
        with tempfile.TemporaryDirectory() as directory:
            executable = Path(directory) / "console_test"
            subprocess.run([
                "cc", "-std=c11", "-Wall", "-Wextra", "-Werror", "-I.",
                "tests/console_test.c", "board/stm32f429i-disc1/console.c",
                "-o", str(executable),
            ], cwd=ROOT, check=True)
            subprocess.run([str(executable)], check=True)
