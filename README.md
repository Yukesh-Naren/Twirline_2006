# Ferrox Compiler

Ferrox is a simple compiler for a custom `.frx` language. It performs:

- Lexical Analysis
- Parsing (AST generation)
- Semantic Analysis
- Three Address Code (TAC) generation
- Code Generation to RISC-V assembly

The generated RISC-V code is executed using RARS.

---

## 🛠 Requirements

Install required tools:

```bash
sudo apt update
sudo apt install build-essential cmake default-jdk

## Installation

git clone https://github.com/YOUR_USERNAME/Ferrox.git
cd Ferrox

mkdir build
cd build
cmake ..
make
sudo make install
cd ..

## Running the Compiler

ferrox tests/test.frx (current working directory + the file name)
./a.out
