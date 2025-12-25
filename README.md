# A simple compiler !
This simple compiler is very inspired by [this](https://www.youtube.com/watch?v=pupdmHjzrTI&list=PLUDlas_Zy_qC7c5tCgTMYq2idyyT241qs) tutorial 

The compiler generates assembly, which gets assembled in an executable by nasm

Here is my parse tree:  
  
$$
\begin{align}
    [\text{prog}] &\to [\text {Statment}]^* \\
    [\text{Statment}] &\to 
    \begin{cases}
        \text{exit}([\text{Expr}]) \\
        \text{TYPE}\space\text{indent} = [\text{Expr}]
        \text{if} ([\text{Expr}])[\text{Scope}]\\
        [\text{Scope}]
    \end{cases} \\
    [\text{Scope}] &\to
    \begin{cases}
        \{[\text{Statment}]^*} \\
    \end{cases}
    [\text{Expr}] &\to
    \begin{cases}
        [\text{Term}] \\
        [\text{NodeBinExp}] \\
    \end{cases} \\
    [\text{BinExpr}] &\to
    \begin{cases} \\
        [\text{Expr}] * [\text{expr}] & \text{priority} = 1 \\
        [\text{Expr}] / [\text{expr}] & \text{priority} = 1 \\
        [\text{Expr}] + [\text{expr}] & \text{priority} = 0 \\
        [\text{Expr}] - [\text{expr}] & \text{priority} = 0 \\
    \end{cases} \\
    [\text{Term}] &\to
    \begin{cases}
        \text{int\_lit} \\
        \text{ident} \\
        ([\text{Expr}]) \\
    \end{cases} \\
\end{align}
$$
