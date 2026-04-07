# CSAPP Lab Solutions

This repository documents my learning progress through the classic textbook *Computer Systems: A Programmer's Perspective (CSAPP)*. It serves as a personal archive for my lab solutions.

## Repository Structure

Currently, this repository contains my solutions for the following labs:

* **Data Lab**
* **Bomb Lab**
* **Attack Lab**

---

## Lab 1: Data Lab

**Status:** Completed  
**Final Score:** 62/62  
**Total Operators Used:** 139  

### Performance and Correctness Results

Below is the final output from the `driver.pl` evaluation script, verifying the correctness and operator count for each puzzle:

```text
Correctness Results     Perf Results
Points  Rating  Errors  Points  Ops     Puzzle
1       1       0       2       8       bitXor
1       1       0       2       1       tmin
1       1       0       2       6       isTmax
2       2       0       2       7       allOddBits
2       2       0       2       2       negate
3       3       0       2       11      isAsciiDigit
3       3       0       2       8       conditional
3       3       0       2       15      isLessOrEqual
4       4       0       2       5       logicalNeg
4       4       0       2       36      howManyBits
4       4       0       2       12      floatScale2
4       4       0       2       19      floatFloat2Int
4       4       0       2       9       floatPower2

Score = 62/62 [36/36 Corr + 26/26 Perf] (139 total operators)
```

---

## Lab 2: Bomb Lab

**Status:** Completed  
**Phases Defused:** 6/6 (+ Secret Phase)

### Final Defuse Inputs

Below are the exact inputs that successfully defuse all phases in my bomb instance:

```text
Border relations with Canada have never been better.
1 2 4 8 16 32
1 311
0 0 DrEvil
IONEFG
4 3 2 1 6 5
22
```

The last line corresponds to the secret phase input, and it may differ on different machines.

---

## Lab 3: Attack Lab

**Status:** Completed  