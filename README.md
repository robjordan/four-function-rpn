# four-function-rpn
A simple Reverse Polish Notation calculator. 

It uses the TM1638 module, which has 16 keys and an 8-digit 7-segment LED. The microcontroller is ATtiny1614.

Currently it implements just plus, minus, multiply and divide.

The 16 unmodified keys are used for:
- Digits 0-9,
- decimal point,
- four operators,
- enter. 

A second set of 16 keys is available via long-press. So far I have only implemented (+/-) ie change sign, via long-press of the minus key.

Key layout is based on the numeric and basic operator part of classic HP calculators such as [HP 42S](https://en.wikipedia.org/wiki/HP-42S):

|<div style="width:290px">7</div>|8|9|&divide;|
|:---:|:---:|:---:|:---:|
|**4**|**5**|**6**|**&times;**|
|**1**|**2**|**3**|**&minus;**|
|**0**|**.**|**enter**|**&plus;**|
