# cs-ini
A yet another header only library for ini in C89. **INCOMPLETE** **PLANNING STAGE**

## Planned Features
* Custom Library Function Aliasing.
* Optional UTF-8 Support. Goal: Optionally enable ASCII only for sections, keys and values. For example, sections and keys only accepts 1-127 character codes otherwise it results in an error. However, values can have valid UTF-8 values except for control codes.
* INI Lexer. Data Structure: Linked Lists containing Arrays which holds the tokens.
* INI Parser. Convert the tokens and place them in a hash table. This should not be too hard to write.
* Create Hash Table for key value pairs.
* Int value decode support.
* Float value decode support.
* Section Support.
* Create Unit Tests for this header-only library with CMake.
