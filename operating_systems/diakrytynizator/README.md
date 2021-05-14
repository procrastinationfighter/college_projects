# Diakrytynizator
A program that with given arguments:
``` ./diakrytynizator a0 a1 a2 ... an```
tranforms user input changing every character ```x``` greater than ```127``` 
through calculating the value of  ```w(x - 0x80) + 0x80``` where ```w(x) = an * x^n + ... + a2 * x^2 + a1 * x + a0``` and printing it on the standard output.

# Example
Calling ```echo "ŁOŚ" | ./diakrytynizator 1075041 623420 1; echo $?``` writes on standard input: 

„O”

0
