# C++ Compiler

A compiler for a subset of C++ implementing a full compilation pipeline from source code to assembly generation.

---

## Inspiration / Related Work

This project is inspired by and based on concepts from:

- CS6120: [Advanced Compilers (Cornell PhD-level course)](https://www.cs.cornell.edu/courses/cs6120/)
- L.EIC026: [Compilers (University of Porto)](https://sigarra.up.pt/feup/en/ucurr_geral.ficha_uc_view?pv_ocorrencia_id=541891)
- Why SSA?: https://mcyoung.xyz/2025/10/21/ssa-1/


---
## Overview

The compiler implements a classical multi-stage architecture:

- Grammar (Lexer & Parser)
- Abstract Syntax Tree (AST)
- Symbol Table
- Semantic Analysis
- SSA (Intermediate Representation) - Dominator Tree
- Optimization Passes
- Assembly Generation

---

## Technologies

- ANTLR (parser generator)
- Abseil (C++ utility library, *'A Better STL Library'* )
- C++23
- Qt

Developed in accordance with the  
[Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html)

---

## Grammar

Grammar is defined using ANTLR `.g4` files and follows an [Extended Backus–Naur Form (EBNF)](https://en.wikipedia.org/wiki/Extended_Backus%E2%80%93Naur_form) structure.

It encompasses and concerns itself with Lexer, Parser, and generates AST, defining:
- language syntax rules
- statement structure
- expression rules

Example artifacts:
- AST construction rules
- parse tree generation

---
## Lexer

A **lexer (lexical analyzer)** converts raw source code into a sequence of tokens.

It validates whether the input consists of valid language elements (keywords, identifiers, literals, operators).

Example:

```cpp
int x = 5;    // Valid C++ code ✔️
``` 

```cpp
var x = 5;    // Not C++ syntax ❌
``` 
#### Lexer Rules:

![Lexer Rules](/img/lexer.png)

view [Cpp.g4](/src/C++00/grammar/Cpp.g4) file for more.



---

## Parser

The **parser** takes tokens and builds a structured representation (AST).

It validates syntactic correctness, ensuring that tokens follow the grammar rules of the language.

Example:

```cpp
int 5 = x;    // Invalid syntax ❌
``` 

The parser ensures:
- correct statement structure
- valid expressions
- proper operator usage

#### Parser rules:

![Parser](/img/parser.png)

view [Cpp.g4](/src/C++00/grammar/Cpp.g4) file for more.

---


### Challenges that C++ introduces, but should me mitigated
```cpp
int *x, &y = *x;    // allowed, but causes UB
```

```cpp
const int *&z = 1;  // not allowed in C++
```

these are to be handled at either lexer level or semantic analysis level

---
## Abstract Syntax Tree (AST)

The AST is a hierarchical representation of program structure.

It removes syntactic noise and represents only meaningful constructs such as:
- expressions
- statements
- declarations

*The AST is called `abstract` because it removes surface-level, human-readable formatting. Instead of representing every punctuation mark or space, it abstracts away non-essential details to expose the pure logical and structural meaning of the code.*

***Example:***

`return 2 + 3;`

![AST tree](/img/ast.png)

## Symbol Table

The symbol table tracks identifiers declared in the program.

It stores:

- variable names
- function declarations
- scopes
- types

Current implementation uses the so-called ***"swiss tables"*** from `Abseil` Library for instantaneous look-up.

```cpp
absl::flat_hash_map<std::string, Symbol> symbol_table_;
```


## Semantic Analysis

Semantic analysis verifies that the program is logically correct.

Examples:

`Using an undeclared variable`
```cpp
int main() {
    x = 5;
}
```

`Variable declared twice`
```cpp
int main() {
    int x;
    int x;
}
```

`Wrong assignment type`
```cpp
int main() {
    int x;
    x = "hello";
}
```

`Invalid binary operation`
```cpp
int main() {
    int x = 5;
    bool y = true;

    x + y;
}
```

`Scope checking`
```cpp
int main() {

    if (true)
    {
        int x = 10;
    }

    x = 5;
}
```

`Wrong number of arguments`
```cpp
void add(int a, int b)
{

}

int main()
{
    add(5);
}
```

`Return type mismatch`
```cpp
int foo()
{
    return "hello";
}
```

### SSA (Static Single Assignment)

SSA is an intermediate representation where each variable is assigned exactly once.

This simplifies optimization and analysis.

Example:

```cpp
x = 1
x = x + 2
```

becomes:

```cpp
x1 = 1
x2 = x1 + 2
``` 

In control-flow graphs, SSA uses **φ-functions** to merge values from different branches.

---

## Optimizations

The compiler implements several optimization passes on SSA IR:

- Constant folding
- Constant propagation
- Dead code elimination
- Expression simplification
- Redundant assignment removal

Example:

```cpp
x = 2 + 3
y = x * 0
```

becomes:

```cpp
x = 5
y = 0
```

---

## Assembly Generation

The final stage converts optimized SSA IR into assembly code.

Responsibilities:
- instruction selection
- IR-to-machine mapping
- low-level code generation

The backend is intentionally simplified and focuses on correctness and clarity.

---

## Design Goals

- Understand full compiler pipeline design
- Implement SSA-based IR representation
- Explore compiler optimization techniques
- Improve systems programming skills in modern C++
- Build modular and extensible architecture

---


---

## References

- CS6120: Advanced Compilers (Cornell University)
  https://www.cs.cornell.edu/courses/cs6120/

- L.EIC026: Compilers (University of Porto)
  https://sigarra.up.pt/feup/en/ucurr_geral.ficha_uc_view?pv_ocorrencia_id=541891

- Why SSA?
  https://mcyoung.xyz/2025/10/21/ssa-1/

- LLVM Compiler Infrastructure
  https://llvm.org/