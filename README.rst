================================================================================
                           Binary Forthleq Calculus
================================================================================

My solutions to the 3rd esolangs reverse engineering contest, and their
explanation, inspired by a `similar writeup`_ by the previous winner.

.. _similar writeup: https://github.com/jared-hughes/nuova


Introduction
============

The point of the event was to understand the language BFC defined by its
implementation (here in `work/bfc_og.c`_) and implement in it solutions to six
challenges:

1. Create a ``cat`` program that terminates on end of file.

2. Decode the `work/hint.bin`_ file and make a program that prints its contents.

3. Create a program that prints the first n primes, reading n from the user,
   where n < 2000.

4. Write a non-empty quine.

5. Create a brainfuck interpreter. No restrictions are imposed on the
   implementation details.

6. Pick a Turing-complete stack-based esoteric language. Implement its compiler
   that targets Binary Forthleq Calculus in BFC.

This tasks wouldn't be too hard in a practical programming language, but BFC was
designed to be opaque, impose unnecessary restrictions on the program and then
obfuscated into a neat square of unreadable C code.

I'll explain my solutions in order: cat, brainfuck, primes, compiler and quine.
This will allow me to introduce certain concepts one by one. I won't be talking
about the hint, as I didn't manage to decode it during the contest, and when
decoded printing it would be trivial.

You can see all files that came as a result of me solving these challenges in
the ``work`` folder in this repository. I will be linking to them often from
this document.

.. _work/bfc_og.c: ./work/bfc_og.c
.. _work/hint.bin: ./work/hint.bin


The preprocessor
================

The main hardship when it comes to writing BFC programs is convincing the
interpreter to even attempt execution. Before anything happens a preprocessor
scans the whole program for digits, converts them to ones and zeros based on
their parity and runs as a `Binary Lambda Calculus`_ program. That program must
output itself as well as parity of all characters in the program.

What this actually means is that all digits in the file must follow a specific
pattern of odd/even, and the pattern must be adjusted based on the parity of all
characters in the program.

I ended up finding one sequence of bits that produces a valid output and some
parity bit and just adjusted the parity of my programs to match that one
sequence. You will see all my programs ending with a wall of digits just to
satisfy the preprocessor, and they always follow the same odd/even pattern.

.. _Binary Lambda Calculus: https://esolangs.org/wiki/Binary_lambda_calculus


Working around the preprocessor
-------------------------------

Once you found a working bit pattern like the one in `work/quine.blc`_ you are
basically set. The rule of thumb is to never use digits in identifiers, since
those must be written exactly the same way every time. This leaves only the
numbers to take care of. They are parsed using C's ``atoi`` function, which
parses only as long as it has digits and then retrns. This means that we can
just put some non-digit character and anything in the same word will be ignored.
For example here are some words that would be interpreted as the same numbers::

  0 == .0 == 0. == 000.2137 == 0asdf == . (if . is not a defined word)
  21 == 00021 == 21.37 == 21as8f0s9

This gives us a lot of leeway in terms of matching numbers in our program to a
target parity pattern. Let's take for example pattern ``001011101101011011`` and
program ``: funi 50 emit 49 emit 51 emit 55 emit ;``. The only thing that would
need adjustment to match this program with the pattern are numbers, and one
possible matching would look like this::

  : funi 0050.111 emit 49.1010 emit 51 emit 055 emit ;
         0010 111      01 1010      11      011

You can see that the parity of digits is in the desired pattern, but numbers in
the program retained their original value. The only concern is running out of
the pattern, but in practice the pattern is large enough that this doesn't
happen.

**Examples in further sections will ignore the preprocessor since working around
it is trivial.**

.. _work/quine.blc: ./work/quine.blc

Execution
=========

The main execution loop of BFC, as the name suggests, is heavily inspired by
Forth: there's a stack of integers, there's a block of memory accessible to the
program, and builtin method for defining new words which end up forming a linked
list dictionary.

Just like Forth it makes a distinction between **compiled** words, which when
read compile a call to their definitions, and **immediate** words, which when
read immediately execute their definitions.


Primitives
----------

Every program starts by reading the first 13 words and defining them as
primitive operations. Here they are in order, with the mnemonics that I used
for them, their closes Forth equivalents.

``:`` : ``:`` : immediate
  Consumes the next word of the program and starts compiling a new definition
  assigned to that word. By default that definition will be a compiled word.

``I`` : ``IMMEDIATE`` : immediate
  When written directly after a ``:`` definition will change it to compiling an
  immediate word.

``Q`` : ``PARSE-NAME EVALUATE`` : compiled
  Performs an evaluation step: consumes a word from the program and evaluates
  it, compiles it or compiles it as a number, based on its definition or lack
  thereof. If there are no more words in the program terminate execution.

``@`` : ``@`` : compiled
  Consumes an address from the stack and pushes a value from memory at that
  address.

``!`` : ``!`` : compiled
  Consumes an address and a value and sets memory at that address to that value.

``-`` : ``-`` : compiled
  Subtracts top of stack from second on stack.

``*`` : ``*`` : compiled
  Multiplies top two numbers on the stack.

``/`` : ``/`` : compiled
  Divides second on stack by top of stack.

``leq?`` : ``0 <=`` : compiled
  Replaces top of stack with zero if it was positive and one otherwise.

``;`` : ``;`` : compiled
  Returns from the function call. Usually ends a definition started with ``:``.

``emit`` : ``EMIT`` : compiled
  Pushes the top value from the stack to the standard output.

``getc`` : ``1 ACCEPT`` : compiled
  Consumes a byte from standard input (not the program!) and pushes it on the
  stack. Pushes ``-1`` on EOF.

``pick`` : ``PICK`` : compiled
  Replaces a number on top of stack with a value that deep into the stack. For
  example ``0 pick`` will duplicate top of the stack, and ``1 pick`` acts as
  Forth's ``OVER``.

The mnemonics I am using here are not enforced, the language just assigns these
meanings to first 13 words of each program. In addition to those primitives any
undefined word will be interpreted as a number, and will compile pushing that
number on the stack.


Memory initialisation
---------------------

Zeroth cell of memory contains an **allocation pointer** which indicates how
much of memory is taken. The first cell acts as the **return stack pointer** and
keeps the position of the top of the stack of return addresses. Second cell must
be zero at all times.

These are followed by empty unused cells perfect for variables, up until cell
32 where definitions of the primitives are stored, occupying space up to cell
84. Between the definitions on cells 41-44 sits code responsible for the main
execution loop.

Finally all of this is capped by 512 cells of the return stack (data stack is
separate from memory), and the allocation pointer starts pointing to cell 597
where program can start allocating its own memory up to cell 59999.


Low level execution loop
------------------------

Execution follows an **instruction pointer** which starts on cell 43, the code
responsible for the main execution loop. On each execution step the following
happens:

1. Value pointed to by the IP is interpreted as an addres of an opcode.

2. IP gets incremented.

3. The action associated with the opcode is executed.

4. Repeat.

Possible opcodes range from 0 to 15. Opcodes 3 to 15 correspond to actions of
primitives in the order I wrote them above. The rest are:

0. Push the value pointed to by IP to the return stack and increment IP.

1. Compile a pointer to just after the opcode.

2. Push IP to the return stack and jump to just after the opcode.

These are used in code compiled by the primitives, and we will be using them
ourselves when the primitives finally turn out to not be enough.


High level execution loop
-------------------------

The system starts by doing a simple loop of opcodes ``5`` and ``2`` - consume a
word from program and do an appropriate action, and call, in this case the main
loop again. This simple loop repeats until it consumes the whole program, and
then terminates execution.

Let's look at an example program::

  : I Q @ ! - * / leq? ; emit getc pick
  : square 0 pick * ;
  : main I 7 square emit ;
  main

Use of ``square`` in the body of ``main`` doesn't execute it but compiles a call
to it since ``square`` is a compiled word. Use of ``main`` calls it because it's
an immediate word. The whole program should emit a square of 7: 49,
corresponding to an ascii character ``1``.

**In further examples the declaration of 13 primitives will be ommited.**


Cat
===

It seems like we have a nice little Forth-like language to play with. The main
execution loop has already shown us how to do loops, so let's try a simpler
challenge: ``cat`` that loops forever::

  : cat getc emit cat ;
  : main I cat ; main

This should work, and if we start testing it it will for some time, but on a
sufficiently large input it will segfault. Looping with recursion has an
unfortunate sideeffect of filing up the return stack and overflowing to other
memory. This is an easy fix though: at no point do we actually return from
anything, so we don't need the return stack. We can keep resetting the return
stack pointer to its initial position at 85::

  : cat; getc emit 85 1 ! cat;
  : main I cat; main

This gives us a working infinite ``cat``, but we still need to terminate on EOF,
which is ``-1``. There aren't really any conditional jumps available to us in
the opcodes, and the only jumps we have are call and return. This gives me an
excuse to try out an approach I've only seen theorised about: conditional
returns.


Conditional return
------------------

A return compiled by ``;`` jumps execution to the address pointed to by the
first cell of memory: the return stack pointer (RSP). Making a conditional
return only requires manipulating the pointer.

Let's take a look at an example program, and how the return stack, RSP,
and instruction pointer look when executing ``?ret``::

  : ?ret ??? ;
  : inner stuff ?ret stuff ;
  : outer stuff inner stuff ;

  IP = inside ?ret

  Memory:

        0        1      ...        RSP - 1          RSP         ...
  +-----------+-----+- - - - -+--------------+--------------+- - - -
  | alloc ptr | RSP |   ...   | resume outer | resume inner |   ...
  +-----------+-----+- - - - -+--------------+--------------+- - - -

When ``?ret`` returns it will resume whatever the RSP is pointing to, so all we
need to do to make a conditional return is either reduce RSP by 1 or not. This
fits nicely with the fact that BFC's comparison operator ``leq?`` returns a zero
or a one. ::

  : ?ret 1 @ - -1 * 1 ! ;

We end up with a function that takes one argument, fetches the RSP, subtracts
the argument from it*, and stores the result back in RSP. As long as the
argument is 0 or 1 it will work as we want it to.

\*Actually it subtracts RSP from argument, and then multiplies by -1 to correct
for that.

Terminating ``cat``
-------------------

With just one more convenience word we get what should be a fully funcitonal
``cat``::

  : dup 0 pick ;
  : ?ret 1 @ - -1 * 1 ! ;
  : cat; getc dup -1 - leq? ?ret emit 85 1 ! cat;
  : main I cat;
  main

While it takes input and produces output as expected it segfaults instead of
exiting clearly. Let's examine why that happens.

When ``?ret`` finally succeeds it exits the instance of ``cat;`` that called it
out to whatever called ``cat;``. If that was the first iteration then it exits
to ``main``, in which case no return stack jumping was done yet and we can
simply return from ``main`` to the main execution loop which will safely
consume the rest of the program and exit. The other possiblity is that it
returns from a recursive call, in which case the return stack has been reset
with ``85 1 !`` and we can't just return to the main execution loop anymore.
Fortunately we know that after ``main`` is called the rest or the program is
just a big string of ones and zeros to satisfy the preprocessor, so we can
manually consume it, and try consuming another word to terminate the program::

  : dup 0 pick ;
  : ?ret 1 @ - -1 * 1 ! ;
  : cat; getc dup -1 - leq? ?ret emit 85 1 ! cat; Q Q
  : main I cat; ;
  main

The full program is available in `solutions/cat.bfc`_. My actual solution that I
submitted is crude and includes some concepts that I'll talk about in later
solutions, but you can see it in `work/cat.bfc`_.

.. _solutions/cat.bfc: ./solutions/cat.bfc
.. _work/cat.bfc: ./work/cat.bfc


Brainfuck
=========

Despite this being the penultimate challenge I believe it's actually the second
simplest. Compared to ``cat`` it will mostly involve the same concepts, but in a
more complex setting. The plan is to read the input saving brainfuck commands
and ignoring anything else until ``;``, then execute the brainfuck code with a
simple state machine.


Helpers
-------

Let's start by defining some helper words that will be used across the program.

First a pair of return stack resetting words. We already know that looping will
build up the return stack, so periodically resetting its height will be useful.
We also need the immediate version because the main execution loop also fills up
the stack and can overflow on a long enough program, so we will occasionally
sprinkle it between definitions. In addition to what it did in ``cat`` it now
also has to correctly return, so it carries the top of return stack with it. ::

  : reset 1 @ @ 85 1 ! 85 ! ;
  : | I 1 @ @ 85 1 ! 85 ! ;

We will want some additional stack shuffling words to move data around easier::

  : dup 0 pick ;
  : over 1 pick ;
  : swap 3 ! 4 ! 3 @ 4 @ ;
  : drop 0 * - ;
  : nip swap drop ;

Another useful thing will be to be able to compare values. Notice how in ``=``
multiplication is used as logical ``and``::

  : <= - leq? ;
  : >= swap <= ;
  : = - dup leq? swap -1 * leq? * ;

Addition is another useful operation not provided by BFC::

  : + -1 * - ;

We'll see that it's usually more useful to conditionally return when the
condition is *false*, so let's add a ``leq?`` to the start of the ``?ret``::

  : ?ret leq? 1 @ - -1 * 1 ! ;

Let's also get ourselves ``,``: a word taken from Forth that compiles a single
value, which means it appends it to the allocated memory, moving the allocation
pointer forward. We will use it to store the loaded program. ::

  : , 0 @ ! 0 @ 1 + 0 ! ;

      0        ...     AP-1    AP    AP+1     ...
  +--------+- - - - -+------+------+------+- - - - -
  |   AP   |   ...   | data | null | null | null...
  +--------+- - - - -+------+------+------+- - - - -
      |                        |
      v                        v

      0        ...     AP-1    AP    AP+1     ...
  +--------+- - - - -+------+------+------+- - - - -
  | AP + 1 |   ...   | data | data | null | null...
  +--------+- - - - -+------+------+------+- - - - -


Loading the program
-------------------

To distinguish between brainfuck commands and comments we will have to recognise
these ASCII values:

==== ==== === === === === === === ===
 \+   \-   >   <   ,   .   [   ]   ;
==== ==== === === === === === === ===
 43   45  62  60  44  46  91  93  59
==== ==== === === === === === === ===

Let's start by defining a stopping condition, that is: check for a semicolon. If
it happens we compile an additional 0 and jump to a brainfuck execution
loop::

  : ;? dup 59 = ?ret drop 0 , exec-loop;

Another case is when we got an actual brainfuck command. Then we should compile
it and move on. Here addition and multiplication are used as logical ``or`` and
``and`` for the conditions. ::

  : bf-com?
    dup 43 >= over 46 <= *
    over 62 = + over 60 = +
    over 91 = + over 93 = +
    ?ret dup , ;

Our loading loop now is simply to get a byte, maybe terminate loading, maybe
compile it, drop it, reset the return stack and repat::

  : load-program; getc ;? bf-com? drop reset load-program;


Executing simple commands
-------------------------

Our program will be manipulating the brainfuck state: the instruction pointer
stored on the stack, the data pointer stored in cell 0 of memory and the
brainfuck tape stored in the leftover memory after loading the program.

Command handlers will expect the instruction pointer on the stack and only
activate when the command is correct. ::

  : +? dup @ 43 = ?ret 0 @ @ 1 + 0 @ ! ;
  : -? dup @ 45 = ?ret 0 @ @ 1 - 0 @ ! ;
  : >? dup @ 62 = ?ret 0 @ 1 + 0 ! ;
  : <? dup @ 60 = ?ret 0 @ 1 - 0 ! ;
  : ,? dup @ 44 = ?ret getc 0 @ ! ;
  : .? dup @ 46 = ?ret 0 @ @ emit ;


Executing looping commands
--------------------------

At this point we need to introduce a new piece of state: a skip counter, stored
in cell 5. When it's above one it means we are not executing the program but
skipping over commands between ``[`` and ``]``. We will have two separate
interpretation branches, one for each of the modes of interpretation.

To facilitate jumping backward we will leave addresses of entered blocks on the
stack so no complicated backtracking logic is required. ::

  : [-true? 0 @ @ 0 = leq? ?ret dup ;
  : [-false? 0 @ @ 0 = ?ret 5 @ 1 + 5 ! ;
  : [? dup @ 91 = ?ret [-true? [-false? ;

  : ]-true? 0 @ @ 0 = leq? ?ret drop dup ;
  : ]-false? 0 @ @ 0 = ?ret nip ;
  : ]? dup @ 93 = ?ret ]-true? ]-false? ;

We will also need handlers for ``[`` and ``]`` in skipping mode, because they
will increase or decrease the skip couner. ::

  : [-skip? dup @ 91 = ?ret 5 @ 1 + 5 ! ;
  : ]-skip? dup @ 93 = ?ret 5 @ 1 - 5 ! ;

With all command handlers done we can finally build the brainfuck execution
loop.


Brainfuck execution loop
------------------------

First branch of the loop will handle execution mode, where commands are executed
as normal. It will have to check for the skip mode, and it will reset the skip
counter if it's one, which I will explain later. ::

  : exec?
    5 @ 1 <= ?ret 0 5 !
    +? -? >? <? ,? .? [? ]? ;

Skipping mode branch is significantly simpler::

  : skip? 5 @ 1 >= ?ret [-skip? ]-skip? ;

And the main loop ties it all together::

  : finished? dup @ leq? ?ret Q Q
  : exec-loop; finished? exec? skip? 1 + reset exec-loop;

``finish?`` will trigger on the 0 compiled by ``;`` and, just like in ``cat``, attempt to
consume two words and terminate.

Now we can finaly write our main entry point, which sets up the initial
instruction pointer::

  : main I 0 @ load-program; main

The whole solution is in `solutions/brainfuck.bfc`_, and once again, it's
significantly different from my actual solution in `work/frainbuck.bfc`_.

.. _solutions/brainfuck.bfc: ./solutions/brainfuck.bfc
.. _work/frainbuck.bfc: ./work/frainbuck.bfc


Why does skip counter work?
---------------------------

If you were paying close attention you may have noticed that the skip counter is
not exactly a nesting counter. While it's incremented on ``[`` and decremented
on ``]`` it starts by being incremented *twice*: once by ``[-false?`` in
``exec?`` and once by ``[-skip?`` in ``skip?``. A value of 1 for the skip
counter signifies a transition between states. If ``skip?`` is entered with skip
counter of 1 it will immediately increment it to 2, because the program must be
looking at a ``[`` command. Then after all matching brackets are found the skip
counter gets back to 1, and ``exec?`` notices it and sets it back to 0, resuming
normal execution.



