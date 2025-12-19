# A simple compiler !
This simple compiler is very inspired by [this](https://www.youtube.com/watch?v=pupdmHjzrTI&list=PLUDlas_Zy_qC7c5tCgTMYq2idyyT241qs) tutorial 

Here is my parse tree:  
  
$$
\begin{align}
    [\text{prog}] &\to [\text {statment}]^* \\
    [\text{statment}] &\to 
    \begin{cases}
        \text{exit}([\text{expr}]) \\
        \text{TYPE}\space\text{indent} = [\text{expr}]
    \end{cases} \\
    \begin{cases}
        [\text{Term}] \\
        [\text{NodeBinExp}] \\
    \end{cases} \\
    \begin{cases} \\
        [\text{expr}] * [\text{expr}] & \text{priority} = 1 \\
        [\text{expr}] + [\text{expr}] & \text{priority} = 0 \\
    \end{cases} \\
    \begin{cases}
        \text{int\_lit} \\
        \text{ident} \\
    \end{cases} \\
\end{align}
$$
