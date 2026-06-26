grammar Cpp;

/* ============================================================
 * LEXER RULES (UPPERCASE)
 * ============================================================
 */

/* 84+ C++ Keywords */
ALIGNAS        : 'alignas';
ALIGNOF        : 'alignof';
ASM            : 'asm';
AUTO           : 'auto';
BREAK          : 'break';
CASE           : 'case';
CATCH          : 'catch';
CLASS          : 'class';
CONST_CAST     : 'const_cast';
CONTINUE       : 'continue';
DECLTYPE       : 'decltype';
DEFAULT        : 'default';
DELETE         : 'delete';
DO             : 'do';
DYNAMIC_CAST   : 'dynamic_cast';
ELSE           : 'else';
ENUM           : 'enum';
EXPORT         : 'export';
FOR            : 'for';
FRIEND         : 'friend';
GOTO           : 'goto';
IF             : 'if';
NAMESPACE      : 'namespace';
NEW            : 'new';
NOEXCEPT       : 'noexcept';
NULLPTR        : 'nullptr';
OPERATOR       : 'operator';
PRIVATE        : 'private';
PROTECTED      : 'protected';
PUBLIC         : 'public';
REGISTER       : 'register';
REINTERPRET_CAST: 'reinterpret_cast';
RETURN         : 'return';
SIZEOF         : 'sizeof';
STATIC_ASSERT  : 'static_assert';
STATIC_CAST    : 'static_cast';
STRUCT         : 'struct';
SWITCH         : 'switch';
TEMPLATE       : 'template';
THIS           : 'this';
THROW          : 'throw';
TRY            : 'try';
TYPEDEF        : 'typedef';
TYPEID         : 'typeid';
TYPENAME       : 'typename';
UNION          : 'union';
USING          : 'using';
WHILE          : 'while';

/* Type Modifiers */
CONST          : 'const';
CONSTEXPR      : 'constexpr';
CONSTINIT      : 'constinit';
CONSTEVAL      : 'consteval';
VOLATILE       : 'volatile';
SIGNED         : 'signed';
UNSIGNED       : 'unsigned';
MUTABLE        : 'mutable';
STATIC         : 'static';
EXTERN         : 'extern';
THREAD_LOCAL   : 'thread_local';

/* function behavior */
VIRTUAL        : 'virtual';
EXPLICIT       : 'explicit';
INLINE         : 'inline';

/* Primitive Types & Literals */
TRUE           : 'true' | 'True' | 'yes';
FALSE          : 'false' | 'False' | 'no';
BOOL           : 'bool' | 'boolean';
VOID           : 'void';
DOUBLE         : 'double';
FLOAT          : 'float';
CHAR16_T       : 'char16_t';
CHAR32_T       : 'char32_t';
WCHAR_T        : 'wchar_t';

INT128         : 'int128_t' | 'longlonglonglong' | 'llll' | 'OCTOWORD';
INT64          : 'int64_t' | 'longlong' | 'll' | 'QWORD';
INT32          : 'int32_t' | 'int' | 'long' | 'DWORD';
INT16          : 'int16_t' | 'short' |'WORD';
INT8           : 'int8_t' | 'char' | 'BYTE';

/* Text-Based Operators & Keyword Alternatives */
AND            : '&&' | 'and';
ANDEQ          : '&=' | 'and_eq';
BITAND         : '&' | 'bitand';
BITOR          : '|' | 'bitor';
COMPL          : '~' | 'compl';
NOT            : '!' | 'not';
NOTEQUALS      : '!=' | 'not_eq';
OR             : '||' | 'or';
OREQ           : '|=' | 'or_eq';
XOR            : '^' | 'xor';
XOREQ          : '^=' | 'xor_eq';

/* Strings and Character Literals */
STRING_LITERAL : '"' (~["\\\r\n] | '\\' .)* '"' ;
CHAR_LITERAL   : '\'' (~['\\\r\n] | '\\' .) '\'' ;

/* Numeric Literals */
DOUBLE_LITERAL  : [0-9]* '.' [0-9]+ | [0-9]+ '.' [0-9]* ;
INT_LITERAL    : [1-9][0-9]* | '0' ;

/* Catch-All for Custom Identifiers (Variables, Functions, Classes) */
IDENTIFIER     : [a-zA-Z_][a-zA-Z0-9_]* ;

/* Pure Symbols & Operators */
LBRACE         : '{';
RBRACE         : '}';
LPAREN         : '(';
RPAREN         : ')';
LBRACK         : '[';
RBRACK         : ']';
SEMICOLON      : ';';
COLON          : ':';
COMMA          : ',';
SCOPE          : '::';
ARROW          : '->';
DOT            : '.';
ASTERISK       : '*';
SLASH          : '/';
EQUALS         : '=';
LT             : '<';
GT             : '>';
LE             : '<=';
GE             : '>=';
INCREMENT      : '++';
DECREMENT      : '--';
PLUS           : '+';
MINUS          : '-';
LSHIFT         : '<<';
RSHIFT         : '>>';
MOD            : '%' | 'mod';

/* Compound Assignment Operators */
MULTEQ         : '*=';
DIVIDEEQ       : '/=';
PLUSEQ         : '+=';
MINUSEQ        : '-=';

/* 8. Comments & Whitespace (Skipped from AST layout) */
LINE_COMMENT   : '//' ~[\r\n]* -> skip ;
BLOCK_COMMENT  : '/*' .*? '*/' -> skip ;
WHITESPACE     : [ \t\r\n]+ -> skip ;


/* ============================================================
 * parser rules (lowercase)
 * ============================================================
 */


// 1. Entry Point: A program is a sequence of statements
translationUnit
    : (statement)* EOF
    | RETURN expression
    ;

// 2. A statement can be a declaration OR a standard expression
statement
    : variableDeclaration     #VariableDeclarationStatement
    | expression SEMICOLON    #ExpressionStatement
    ;

variableDeclaration
    : declarationModifiers primitiveType declaratorList SEMICOLON #VariableDeclarationClause
    ;

declaratorList
    : initializedDeclarator (COMMA initializedDeclarator)*
    ;

initializedDeclarator
    : declarator (EQUALS expression)?
    ;

// The Declarator captures the operators (*, &, &&, []) wrapping the variable name
declarator
    : ASTERISK declarator        #PointerModifier   // Pointer modification nest
    | BITAND declarator          #LvalueRefModifier // lvalue reference (&)
    | AND declarator             #RvalueRefModifier // rvalue reference (&&)
    | declarator LBRACK RBRACK   #ArrayModifier     // Matches multi-dimensional arrays []
    | IDENTIFIER                 #BaseIdentifier    // Core variable name anchor boundary
    ;

// 4. Groups all your type lexer tokens into one parser concept
primitiveType
    : BOOL | VOID | DOUBLE | FLOAT | INT128 | INT64 | INT32 | INT16 | INT8
    | CHAR16_T | CHAR32_T | WCHAR_T
    ;

// Matches any sequence of modifiers preceding a type definition
declarationModifiers
    : (typeModifier | storageSpecifier | functionSpecifier)*
    ;

// Modifiers that fundamentally alter the data storage layout itself
typeModifier
    : CONST | VOLATILE | SIGNED | UNSIGNED
    ;

// Modifiers controlling lifetime scope and thread boundaries
storageSpecifier
    : STATIC | EXTERN | THREAD_LOCAL | MUTABLE
    ;

// Modifiers checking execution rules and optimization limits
functionSpecifier
    : INLINE | VIRTUAL | EXPLICIT | CONSTEXPR | CONSTEVAL | CONSTINIT
    ;

expression
    /* 1. Primary Expressions & Literals (Highest Precedence) */
    :   LPAREN expression RPAREN                     #ParentExpression
    |   INT_LITERAL                                  #IntLiteral
    |   DOUBLE_LITERAL                               #DoubleLiteral
    |   STRING_LITERAL                               #StringLiteral
    |   CHAR_LITERAL                                 #CharLiteral
    |   TRUE                                         #LiteralTrue
    |   FALSE                                        #LiteralFalse
    |   NULLPTR                                      #LiteralNullptr
    |   THIS                                         #LiteralThis
    |   IDENTIFIER                                   #VariableIdentifier

    /* 2. Postfix Expressions */
    |   expression LBRACK expression RBRACK          #ArraySubscript
    |   expression LPAREN expressionList? RPAREN     #FunctionCall
    |   expression DOT IDENTIFIER                    #MemberAccessDot
    |   expression ARROW IDENTIFIER                  #MemberAccessArrow
    |   expression INCREMENT                         #PostIncrement
    |   expression DECREMENT                         #PostDecrement

    /* 3. Unary Operators */
    |   PLUS expression                              #UnaryPlus
    |   MINUS expression                             #UnaryMinus
    |   NOT expression                               #LogicalNot
    |   COMPL expression                             #BitwiseNot
    |   BITAND expression                            #AddressOf
    |   ASTERISK expression                          #PointerDereference
    |   INCREMENT expression                         #PreIncrement
    |   DECREMENT expression                         #PreDecrement
    |   SIZEOF expression                            #SizeOfExpr
    |   NEW expression                               #NewExpression
    |   DELETE expression                            #DeleteExpression

    /* 4. Multiplicative Operators */
    |   expression ASTERISK expression               #Multiplication
    |   expression SLASH expression                  #Division
    |   expression MOD expression                    #Modulo

    /* 5. Additive Operators */
    |   expression PLUS expression                   #Addition
    |   expression MINUS expression                  #Subtraction

    /* 6. Bitwise Shift Operators */
    |   expression LSHIFT expression                 #BitwiseLeftShift
    |   expression RSHIFT expression                 #BitwiseRightShift

    /* 7. Relational Operators */
    |   expression LT expression                     #LessThan
    |   expression GT expression                     #GreaterThan
    |   expression LE expression                     #LessThanOrEqual
    |   expression GE expression                     #GreaterThanOrEqual

    /* 8. Equality Operators */
    |   expression EQUALS EQUALS expression          #EqualityAttempt
    |   expression NOTEQUALS expression              #InequalityAttempt

    /* 9. Bitwise AND */
    |   expression BITAND expression                 #BitwiseAnd

    /* 10. Bitwise XOR */
    |   expression XOR expression                    #BitwiseXor

    /* 11. Bitwise OR */
    |   expression BITOR expression                  #BitwiseOr

    /* 12. Logical AND */
    |   expression AND expression                    #LogicalAnd

    /* 13. Logical OR */
    |   expression OR expression                     #LogicalOr

    /* 14. Assignment Operators (Right-to-Left Associativity) */
    |   <assoc=right> expression EQUALS expression   #Assignment
    |   <assoc=right> expression MULTEQ expression   #AssignmentMult
    |   <assoc=right> expression DIVIDEEQ expression #AssignmentDivision
    |   <assoc=right> expression PLUSEQ expression   #AssignmentPlus
    |   <assoc=right> expression MINUSEQ expression  #AssignmentMinus
    |   <assoc=right> expression ANDEQ expression    #AssignmentAnd
    |   <assoc=right> expression OREQ expression     #AssignmentOr
    |   <assoc=right> expression XOREQ expression    #AssignmentXor
    ;

/* Helper rule for function arguments */
expressionList
    :   expression (COMMA expression)*
    ;
