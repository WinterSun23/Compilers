$$
1.Datatypes\\
2._Variables\\
3.if\\
4.loops\\
5.Funtions\\

Datatypes(include\ "int(a)" \ cases\ later)\Rightarrow 
\begin{cases}
int\\
float\\
double\\
bool\\
char\\
string\\
array \ (singleType)\\
array \ (anyType)
\end{cases}
\\
Grammar:\\
Start \ token:\\
[Datatype]\Rightarrow 
\begin{cases}
[varName]\\
[funtionName]
\end{cases}\\
[varName]\Rightarrow 
\begin{cases}
[Expr]\\
\end{cases}
\\
[Expr]\Rightarrow 
\begin{cases}
[value]\\
[Expr]\\
[varName]\\
[funtionName] \ (funtionCall)
\end{cases}\\
[return]\Rightarrow 
\begin{cases}
[Expr]\\
\end{cases}
