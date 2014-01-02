Parameter Set Documentation
===========================

The ParameterSet is the format in which parameters for programs and scripts are specified.
A so-called parset file is a text file holding the ParameterSet definitions. Its syntax
rules are explained in the next sections.

Syntax
------

Normally the parameters are stored in a text file as key-value pairs. The rules for it are as follows:

1. A key/value pair is given as::

    key = value

2. Whitespace (blanks and horizontal tabs) can be given at will and are ignored.

3. Comments can be given at the end of a line after the #-sign (also on a continuation line).

4. Empty lines (or lines only containing comment) are ignored.

5. The name of the key can consist of several parts separated by a dot. This can be used to group
   the keywords as the software has functionality to create subset of keys with a common prefix.

6. String values can optionally be enclosed in quotes (single or double quotes). If containing
   special characters (e.g. whitespace), they have to be enclosed in quotes.

7. Numeric values should not be enclosed in quotes.

8. There is no limit to the length of a line.

9. A value can be continued on the next line without having to give a continuation character.
   A line is seen as the continuation of a value if it does not start with a string like ‘key=’.

  a. For backward compatibility a trailing backslash is allowed as continuation character.

  b. For unquoted values, a blank is inserted before the first word of the continuation.
     In this way it catches the line wrapping done by editors. E.g::

        Key = this is not too       # comment 1
              long a string         # comment 2

    results in::

        Key = this is not too long a string

  c. Continuation of quoted values must also be enclosed in quotes. It is possible to use
     single quotes on one line and double quotes on the continuation. That makes it possible
     to make quotes part of a value. E.g::

        Key = "this is not too"     # comment 1
              "long a string"       # comment 2
     results in::

        Key = "this is not toolong a string"

10. A vector of values can be given by enclosing it in square brackets and separating it
    by commas. An empty vector is possible. E.g::

        keyvec = []
        keyvec = [1,2,3]
        keyvec = [‘aa’, ’bb’, ’cc’]

11. Nested vectors can be given by a nested pair of brackets. It can be nested as deeply as
    one likes. E.g::

        nested = [[1,2,3], [4,5,6]]

12. Square brackets, comma, #-sign, and =-sign are special characters and have to be enclosed
    in quotes if they are part of a string value.


Expandable vector values
------------------------

Special syntax exists for easy specification of particular vector values, thus for values enclosed
in square brackets. They are expanded if required.

1. Sequential values can be given by means of the range operator ‘..’. E.g::

        [8..11]

   is expanded to::

        [8,9,10,11]

   Not only numeric values can be given this way, but also strings of which the last part is a
   numeric value. The minimal width of each numeric part is the width of the leading numeric value.
   E.g::

        [/aa000..2]

   is expanded to::

        [/aa000,/aa001,/aa002]

   The string prefix can be given in the end value as well, thus::

        [/aa000../aa2]

   gives the same result. Note that expansion if only done if the range operator is preceeded by
   a numeric value. Thus a path name like::

        [a/b/../c/d]

   will not be changed.

2. If end < start, the values will be counted backwards. E.g::

        [ab013..010]

   is expanded to::

        [ab013,ab012,ab011,ab010]

3. The repeat operator ‘*’ can be used to repeat the same value n times. It must be preceeded
   by an integer value to take effect. E.g::

        [5*0] is expanded to [0,0,0,0,0]

   Expansion is recursive, because evaluation is done from left to right. Thus::

        [2*3*0]

   results first in::

        [3*0,3*0]

   and finally in::

        [0,0,0,0,0,0]

   In this example 2*3 looks like a multiplication (and has the same result), but it is a repeat.

4. Subvectors can also be repeated, e.g. (note the nested repeat)::

        [2*[1,2*2,3]]

  is expanded to::

        [[1,2,2,3],[1,2,2,3]]

5. Multiple values can be repeated by enclosing them in parentheses. E.g::

        [2*(1,2,3)]

   is expanded to::

        [1,2,3,1,2,3]

   For backward compatibility the elements in such a set can be separated by semicolons
   as well. Thus::

        [2*(1;2;3)]

   gives the same result.  A value can be any value, thus also a vector or another repeated
   value set. E.g::

        [2*(0,2*(1,2),[3,4])]

   is expanded to::

        [0,1,2,1,2,[3,4],0,1,2,1,2,[3,4]]

   This example is not really meaningful, but shows that expansion is fully recursive.

6. From above it should be clear that a string value containing commas, semicolons,
   brackets, or parentheses should be enclosed in quotes.

7. The range operator has a higher precedence than the repeat operator::

        [2*0..3]

   is expanded to::

        [0,1,2,3,0,1,2,3]

Here are some examples of expansion results to get a better feel what it does::

[3*3*2]         ==> [2,2,2,2,2,2,2,2,2]
[3*'2*3']       ==> ['2*3','2*3','2*3']
[3*ab]          ==> [ab,ab,ab]
[2*3*ab]        ==> [ab,ab,ab,ab,ab,ab]
[3*10,5*2]      ==> [10,10,10,2,2,2,2,2]
[3*(1,2,3,4)]   ==> [1,2,3,4,1,2,3,4,1,2,3,4]
[3 * 1 .. 4]    ==> [1,2,3,4,1,2,3,4,1,2,3,4]
[2*[[1,2,3],[4,5,6]]]  ==> [[[1,2,3],[4,5,6]],[[1,2,3],[4,5,6]]]
[3*'10.5*ab']   ==> ['10.5*ab','10.5*ab','10.5*ab']
[10.5*'ab']     ==> [10.5*'ab']
[3*10.5*'ab']   ==> [10.5*'ab',10.5*'ab',10.5*'ab']
[3*'ab'*2]      ==> ['ab'*2,'ab'*2,'ab'*2]
[3*ab*2]        ==> [ab*2,ab*2,ab*2]
[1*(1,2,3)]     ==> [1,2,3]
[(1,2,3)]       ==> [(1,2,3)]

The last example shows that parentheses are not removed if no repeat operator is
given before it.

**Acknowledgement:** This documentation is heavily based on the original Parameter Set documentation written by Ger van Diepen of ASTRON.
