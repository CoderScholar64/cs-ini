# cs-ini
A yet another header only library for ini in C89. **INCOMPLETE**

## Planned Features
* *Partially Done* Optional UTF-8 Support. Goal: Optionally enable ASCII only for sections, keys and values. For example, sections and keys only accepts 1-127 character codes otherwise it results in an error. However, values can have valid UTF-8 values except for control codes.
* INI Lexer.
* Create Hash Table for key value pairs.
* INI Parser. Convert the tokens and place them in a hash table. This should not be too hard to write.
* Section Support.

## Features
* ASCII Encoding/Decoding Support.
* Custom Library Function Aliasing.
* UTF-8 Encoding/Decoding Support.
* Unit Test for ASCII and UTF-8.
* Unit Test for Lexer Storage.
* Linked Lists each containing an array with a fixed amount which holds the tokens.
