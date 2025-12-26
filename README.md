# A simple compiler !
This simple compiler is very inspired by [this](https://www.youtube.com/watch?v=pupdmHjzrTI&list=PLUDlas_Zy_qC7c5tCgTMYq2idyyT241qs) tutorial 

The compiler generates assembly, which gets assembled in an executable by nasm

## Example of brouss :
```
// This is a comment
int x = 1 * 3 - 6 / 2

string str = "Hello World!"
print(str)

if (x) {
  exit(20)
} elif (1) {
  exit(1)
} else {
  exit(0)
}
``` 

 ## Parse tree:  

$$
\begin{aligned}
    [\text{Prog}] &\to [\text{Stmt}]^* \\
    [\text{Statement}] &\to 
    \begin{cases}
        \text{exit}([\text{Expr}]) \\
        \text{TYPE}\ \text{ident} = [\text{Expr}] \\
        \text{if } ([\text{Expr}]) [\text{Scope}] \\
        [\text{Scope}]
    \end{cases} \\
    [\text{Scope}] &\to
    \begin{cases}
        \{ [\text{Statement}]^* \}
    \end{cases} \\
    [\text{Expr}] &\to
    \begin{cases}
        [\text{Term}] \\
        [\text{BinExpr}]
    \end{cases} \\
    [\text{BinExpr}] &\to
    \begin{cases} 
        [\text{Expr}] * [\text{expr}] & \text{priority} = 1 \\
        [\text{Expr}] / [\text{expr}] & \text{priority} = 1 \\
        [\text{Expr}] + [\text{expr}] & \text{priority} = 0 \\
        [\text{Expr}] - [\text{expr}] & \text{priority} = 0 
    \end{cases} \\
    [\text{Term}] &\to
    \begin{cases}
        \texttt{int\\_lit} \\
        \text{ident} \\
        ([\text{Expr}])
    \end{cases}
\end{aligned}
$$

