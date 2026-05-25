import random
import sys

def generate_matrices(size: int):
    with open("matrix_a.txt", "w", encoding='utf-8') as file:
        file.write(str(size) + '\n')
        for i in range(size):
            row = [str(random.randint(0, 9)) for j in range(size)]
            file.write(" ".join(row) + "\n")
    
    with open("matrix_b.txt", "w", encoding='utf-8') as file:
        file.write(str(size) + '\n')
        for i in range(size):
            row = [str(random.randint(0, 9)) for j in range(size)]
            file.write(" ".join(row) + "\n")

if __name__ == "__main__":
    size = int(sys.argv[1]) if len(sys.argv) > 1 else 1000
    generate_matrices(size)