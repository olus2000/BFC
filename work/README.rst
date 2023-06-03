================================================================================
                            Historical work folder
================================================================================

Stored in this folder are files I created while working on the contest. I have
rearranged them a bit and renamed some, but otherwise they remain the way I left
them after finishing ``quine.bfc``.


Top level
=========


BFC source code
---------------

The first thing that appeared in this folder.

- bfc_og.c_ is the original, unchanged, obfuscated source code as provided by
  Palaiologos.

- bfc_deobf_1.c_ is the source after basic deobfuscation: code formatting and
  replacement of ``#define``\s.

- bfc_deobf_2.c_ is the final version of the source with all my comments and
  debug ``printf``\s.

- hint.bin_ is the hint file which I never ended up decoding. It's bzip3 xored
  with 48.

All three versions should be equivalent up to debug printing in 2nd deobfuscated
one.

.. _bfc_og.c: ./bfc_og.c
.. _bfc_deobf_1.c: ./bfc_deobf_1.c
.. _bfc_deobf_2.c: ./bfc_deobf_2.c
.. _hint.bin: ./hint.bin


BLC quine
---------

The binary lambda calculus quine required for the preprocessor.

- quine_design.blc_ is the pseudocode version designed with my non-standard
  approach to BLC.

- quine_deobf.blc_ has generated code responsible for printing stuff, should be
  fully functional.

- quine.blc_ is the minified version with just ones and zeros.

- code_gen.py_ contains code that I used to fill up spaces in quine_design.blc_,
  as well as a function that I used later for BECC compiler and a BFC quine that
  encodes a list of numbers such that they match the given bit pattern.

.. _quine_design.blc: ./quine_design.blc
.. _quine_deobf.blc: ./quine_deobf.blc
.. _quine.blc: ./quine.blc
.. _code_gen.py: ./code_gen.py


Simple solutions
----------------

Solutions to the three problems I managed to do in just one file.

- cat.bfc_ is the BFC cat program terminating on EOF.

- primes.bfc_ is the BFC primes generator.

- frainbuck.bfc_ is the brainfuck interpreter. It takes a brainfuck program
  terminated by a semicolon followed by the program input.

.. _cat.bfc: ./cat.bfc
.. _primes.bfc: ./primes.bfc
.. _frainbuck.bfc: ./frainbuck.bfc


BECC compiler
=============

This folder contains a bunch of resources I used to implement the BECC compiler
in BFC.

- becc.bfc_ is the final solution to the challenge.

- becc.py_ is a BECC interpreter in python designed to emulate the way the
  compiled BFC version would run. It calculates some stats regarding memory
  usage and garbage collection.

- becc_test.becc_ is a BECC program that takes a decimal number terminated by a
  newline and prints it doubled. I used is as a stress test for the compiler.

- becc_prelude_deobf.bfc_ is the design of a set of definitions sufficient to
  define words corresponding to BECC's eight characters.

- becc_prelude.bfc_ is a naive minification of the prelude to make
  the code generating it smaller.

- becc_prelude_obf.bfc_ is the final minification with a slight redesign to fix
  a garbage collection bug.

.. _becc.bfc: ./becc/becc.bfc
.. _becc.py: ./becc/becc.py
.. _becc_test.becc: ./becc/becc_test.becc
.. _becc_prelude_deobf.bfc: ./becc/becc_prelude_deobf.bfc
.. _becc_prelude.bfc: ./becc/becc_prelude.bfc
.. _becc_prelude_deobf.bfc: ./becc/becc_prelude_deobf.bfc


BFC quine
=========

This folder contains stages of writing the BFC quine. They may not agree with
each other because I was mostly fixing bugs only in the most recent versions, so
earlier designs may be lagging behind.

- quine_deobf.bfc_ is the first design with the smallest prelude I could think
  of that would be capable of self-printing.

- quine_minified.bfc_ is the same design but minified.

- quine_numbers.bfc_ is the minified design but with numbers inserted.

- quine_no_payload.bfc_ is a ready quine program lacking only the code
  responsible for self-representation. It's split into two parts for easier
  algorithmic processing. The payload goes between them.

- quine.bfc_ is a ready BFC quine.

.. _quine_deobf.bfc: ./quine/quine_deobf.bfc
.. _quine_minified.bfc: ./quine/quine_minified.bfc
.. _quine_numbers.bfc: ./quine/quine_numbers.bfc
.. _quine_no_payload.bfc: ./quine/quine_no_payload.bfc
.. _quine.bfc: ./quine/quine.bfc
