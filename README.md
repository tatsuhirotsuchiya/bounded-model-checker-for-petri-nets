# bounded-model-checker-for-petri-nets
bounded model checker for petri nets

## Overview

This program implements a deadlock detection algorithm for **safe** petri nets based on bounded model checking. 
The algorithm is proposed by:
* [Shougo Ogata, Tatsuhiro Tsuchiya, and Tohru Kikuno, ``SAT-Based Verification of Safe Petri Nets,'' In Proc. Second International Conference on Automated Technology for Verification and Analysis (ATVA 2004), LNCS 3299, pp. 79-92, Taipei, Taiwan, ROC, October 31-November 3, 2004.](https://doi.org/10.1007/978-3-540-30476-0_11)

## Installing

Just compile the program using a C compiler. 

## Usage 

The program takes a PEP file as input and outputs a Boolean formula. 
For example, `a.out 2 test.pep` outputs the following formula.

    (s1_0 & !s2_0 & !s3_0 & !s4_0 & !s5_0 & !s6_0 & !s7_0)
    & ((s1_0 & !s1_1 & s2_1 & s3_1) | ((s1_0 = s1_1) & (s2_0 = s2_1) & (s3_0 = s3_1)))
    & ((s2_1 & !s2_2 & s6_2) | ((s2_1 = s2_2) & (s6_0 = s6_2)))
    & ((s3_1 & !s3_3 & s7_3) | ((s3_1 = s3_3) & (s7_0 = s7_3)))
    & ((s6_2 & s7_3 & !s6_4 & !s7_4 & s1_4) | ((s1_1 = s1_4) & (s6_2 = s6_4) & (s7_3 = s7_4)))
    & ((s1_4 & !s1_5 & s4_5 & s5_5) | ((s1_4 = s1_5) & (s4_0 = s4_5) & (s5_0 = s5_5)))
    & ((s4_5 & !s4_6 & s6_6) | ((s4_5 = s4_6) & (s6_4 = s6_6)))
    & ((s5_5 & !s5_7 & s7_7) | ((s5_5 = s5_7) & (s7_4 = s7_7)))
    & ((s1_5 & !s1_8 & s2_8 & s3_8) | ((s1_5 = s1_8) & (s2_2 = s2_8) & (s3_3 = s3_8)))
    & ((s2_8 & !s2_9 & s6_9) | ((s2_8 = s2_9) & (s6_6 = s6_9)))
    & ((s3_8 & !s3_10 & s7_10) | ((s3_8 = s3_10) & (s7_7 = s7_10)))
    & ((s6_9 & s7_10 & !s6_11 & !s7_11 & s1_11) | ((s1_8 = s1_11) & (s6_9 = s6_11) & (s7_10 = s7_11)))
    & ((s1_11 & !s1_12 & s4_12 & s5_12) | ((s1_11 = s1_12) & (s4_6 = s4_12) & (s5_7 = s5_12)))
    & ((s4_12 & !s4_13 & s6_13) | ((s4_12 = s4_13) & (s6_11 = s6_13)))
    & ((s5_12 & !s5_14 & s7_14) | ((s5_12 = s5_14) & (s7_11 = s7_14)))
    & (!(
    (s1_12)
    | (s2_9)
    | (s3_10)
    | (s6_13 & s7_14)
    | (s1_12)
    | (s4_13)
    | (s5_14)
    ))

A deadlock is detected if the formula is satisfiable. 
(On the other hand, no conclusion can be drawn if the formula is not satisfiable.) 
The first command argument (`2` in the above example) is the number of *steps* to explore; 
but the notion of steps here is quite different from the ordinary one. 
For technical details, see the above paper. 

It should be noted that sastisfiablity solvers usually require the input formula to be in CNF. 
For CNF transtation, [`bool2cnf`](https://github.com/tkren/bool2cnf) can be used.
Hence a typical usage is like: `./a.out 2 test.pep > tmp.bool; ./bool2cnf < tmp.bool > tmp.cnf; minisat tmp.cnf`

# License 

This program is free software under the zlib license. 

# Author
[Tatsuhiro Tsuchiya](http://www-ise4.ist.osaka-u.ac.jp/~t-tutiya/en/)
