import subprocess
import sys

generate_matr = subprocess.run(
            [sys.executable, "generate_matr.py"],
            capture_output=True,
            text=True
)

multiple_matr = subprocess.run(
            ["mul_matrix.exe"],
            capture_output=True,
            text=True
)

verify = subprocess.run(
            [sys.executable, "verify.py"],
            capture_output=True,
            text=True
)

print(multiple_matr.stdout)
print(verify.stdout)
