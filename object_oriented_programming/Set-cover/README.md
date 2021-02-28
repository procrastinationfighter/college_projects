# Set cover

## Task

Write program in Java that solves the set cover problem (https://en.wikipedia.org/wiki/Set_cover_problem)
Implement accurate algorithm, naive heuristic and greedy heuristic.

## Input

A sequence of decimal integers and spaces, separated by spaces and endlines, is located on standard input.
Syntax of input is described by context-free grammar defined below in Backus-Naur form. The starting symbol is Data.

        Data ::= { Set | Query }
        Set ::= { Element } 0
        Element ::= Single | Infinite | Finite
        Single ::= d
        Infinite ::= d u
        Finite ::= d u u
        Query ::= u t

Terminal symbols:

        -d represents a positive (non-zero) integer
        -u represents a negative integer
        -t represents number 1, 2 or 3

Sequence of numbers derived from Set symbol represents members of set that is a member of set family R. 
Members of R are given indices in order they appear on input.
Every element in a given set can have one of the forms below:

    a

    the only element is the number a

    a b

    element contains all numbers of an infinite arithmetic sequence, with first term equal to a and common difference equal to -b

    a b c

    element contains numbers less or equal to -c, with first term equal to a and common difference equal to -b

Numbers x and y derived from Query symbol, represent the set we want to cover and used algorithm. 
Set that is to be covered contains all integers from 1 to -x. To solve the problem, all previously introduced sets can be used.

Number y represents used algorithm:

    3

    naive heuristic

    2

    greedy heuristic

    1

    accurate algorithm
The program assumes that input is correct, according to this grammar. Example input is located in the file example.in.

## Output
For every query, program writes answer for it on standard output.
Answer consists of spaces and decimal integers (there is a space between every number in a line, there are no spaces at the beginning or the end of a line). Answer is always a single line. 
If given set can be covered, the answer consists of indices of used sets in ascending order. Otherwise, the answer is a single 0 number.
Example output for input located in example.in, is located in the file example.out.

Program assumes that numbers will not exceed the int type.
