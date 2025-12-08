# A simple compiler !
This simple compiler is very inspired by [this](https://www.youtube.com/watch?v=pupdmHjzrTI&list=PLUDlas_Zy_qC7c5tCgTMYq2idyyT241qs) tutorial 

Here is my parse tree
$$
\begin{align}
    [\text{prog}] &\to [\text {statment}]^* \\
    [\text{statment}] &\to 
    \begin{cases}
      exit([\text{expr}]) \\
      let\space\text{indent} = [\text{expr}]
    \end{cases} \\
    \begin{cases}
      \text{expr} \\
      \text{int\_lit}
    \end{cases}
\end{align}
$$
