# four-function-rpn
A simple Reverse Polish Notation calculator. 

It uses the TM1638 module, which has 16 keys and an 8-digit 7-segment LED. The microcontroller is ATtiny1614.

Currently it implements just plus, minus, multiply and divide.

Digits 0-9, decimal point, four operators, and enter use the 16 keys. A second set of 16 keys is available via long-press. So far I have only implemented (+/-) ie change sign, via long-press of the minus key.

Key layout is based on the numeric and basic operator part of classic HP calculators such as [HP 42S](https://en.wikipedia.org/wiki/HP-42S):

|7|8|9|&divide;|

