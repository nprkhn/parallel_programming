import random

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

generate_matrices(1000)
