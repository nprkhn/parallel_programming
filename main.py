import subprocess
import sys

def main():
    size = 1000
    if len(sys.argv) > 1:
        size = int(sys.argv[1])
    
    block_size = 16
    if len(sys.argv) > 2:
        block_size = int(sys.argv[2])
    
    gen = subprocess.run([sys.executable, "generate_matr.py", str(size)], 
                         capture_output=True, text=True)
    
    cuda_proc = subprocess.run(["./cuda_multiply.exe", str(block_size)], 
                               capture_output=True, text=True)
    print(cuda_proc.stdout)
    
    verify = subprocess.run([sys.executable, "verify.py"], 
                            capture_output=True, text=True)
    print(verify.stdout)

if __name__ == "__main__":
    main()