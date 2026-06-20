# C++ Examples and Snippets

This repository contains small C++ examples, snippets, and focused implementation ideas.

The purpose of this repository is not to provide complete applications or production-ready frameworks. 
Instead, it collects compact examples that demonstrate specific C++ techniques, language features, library usage, 
or implementation patterns.

Most examples are intentionally small and self-contained enough to show the relevant idea without adding unnecessary 
infrastructure around it.

## What this repository contains

This repository may include examples for:

 - modern C++ language features
 - standard library algorithms
 - file handling
 - error handling
 - formatting
 - ranges and views
 - small utility functions
 - implementation patterns
 - focused solutions to common programming tasks

The examples are meant to be read, copied, adapted, and discussed.

## What this repository is not

This repository does not aim to provide full programs, complete libraries, or ready-made applications.

Some snippets may omit surrounding project structure, build system files, command-line parsing, tests, or 
platform-specific integration code when those parts are not relevant to the demonstrated idea.

## Style and assumptions

The examples generally prefer modern C++ and the C++ standard library where appropriate. The exact C++ standard required 
may differ between examples and should be visible from the used language and library features.

Unless stated otherwise, the snippets should be understood as educational or demonstrational code. Review and adapt them 
before using them in production code.

License

This repository is licensed under the MIT License.

You are free to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the code, subject to the 
terms of the MIT License.


## Example: Replacing decimal separators in a file

This example shows how to replace all occurrences of . with , inside a file without interpreting the numeric values.

The file format is assumed to contain lines beginning with a month in the form YYYY-MM, followed by numeric values using '.' 
as the decimal separator. Since the month part does not contain a dot, the replacement can be performed directly and mechanically.

The implementation opens the file for binary input and output, reads it block by block, replaces every '.' byte in the 
buffer with ',', and writes the modified buffer back to the same position in the file.

This works in-place because both characters have the same size in the file encoding: one byte for '.' and one byte for ','. 
Therefore, the file size does not change and no reformatting or parsing is required.

The example intentionally does not parse numbers. It only performs a byte-level replacement.
