# ENIGMA 2000

ENIGMA 2000 is a symmetric cryptographic algorithm developed in C++ for Windows/DOS, to encrypt any file.

It is not optimized for speed processing but it is extremly reliable. Tests state that a huge input file just with zeros, will return an output file whose frequency analysis is white noise.

It uses six phases to encrypt any file and the phases sequence depends on the password. On phase 5, it uses the Rubik's cube paradigm to encrypt the data. The password varies between 8 and 16 characters.

The execution syntax is:<br>
`coder.exe [file to encode] [password (min 8 char, max 16)] [mode ('c'oder 'd'ecoder)]`
