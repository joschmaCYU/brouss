# Brouss

A small educational compiler for a simple C-like language that emits x86-64 NASM assembly.

## Overview

Inspired by: [Compiler Tutorial Series](https://www.youtube.com/watch?v=pupdmHjzrTI&list=PLUDlas_Zy_qC7c5tCgTMYq2idyyT241qs)

The compiler reads a `.bro` source file and produces `out.asm`. You can assemble and link it on Linux to get an executable.

## Requirements

- CMake 3.10+
- C++17 compiler
- NASM
- GNU (works only on linux)

## Build

```bash
mkdir build
cd build
make
```

## Run

By default, the binary looks for `test.bro` in the build directory and writes `out.asm` and `out.o`.

```bash
cd build
./brouss
nasm -felf64 out.asm -o out.o
./out
echo $? (to see exit value)
```

## Example

```
// This is a comment
int x = 1 * 3 - 6 / 2 + 2

string str = "Hello World!"

while(i) {
  print(str)
  i = i - 1
}

if (x) {
  exit(20)
} elif (1) {
  exit(1)
} else {
  exit(0)
}
```

## Grammar

Formal grammar (BNF-like) with simple operator precedence:

$$
\begin{aligned}
    [\text{Prog}] &\to [\text{Stmt}]^* \\
    [\text{Statement}] &\to 
    \begin{cases}
        	\text{exit}([\text{Expr}]) \\
        	\text{TYPE}\ \text{ident} = [\text{Expr}] \\
        	\text{ident} = [\text{Expr}] \\
        	\text{print}([\text{Expr}]) \\
        	\text{if } ([\text{Expr}]) [\text{Scope}] \\
        	\text{while } ([\text{Expr}]) [\text{Scope}] \\
        [\text{Scope}]
    \end{cases} \\
    [\text{Scope}] &\to \{ [\text{Statement}]^* \} \\
    [\text{Expr}] &\to [\text{Term}] \mid [\text{BinExpr}] \\
    [\text{BinExpr}] &\to
    \begin{cases} 
        [\text{Expr}] * [\text{Expr}] & \text{priority} = 1 \\
        [\text{Expr}] / [\text{Expr}] & \text{priority} = 1 \\
        [\text{Expr}] + [\text{Expr}] & \text{priority} = 0 \\
        [\text{Expr}] - [\text{Expr}] & \text{priority} = 0 
    \end{cases} \\
    [\text{Term}] &\to
    \begin{cases}
        	\text{int\\_lit} \\
        	\text{string\\_lit} \\
        	\text{ident} \\
        ([\text{Expr}])
    \end{cases}
\end{aligned}
$$
