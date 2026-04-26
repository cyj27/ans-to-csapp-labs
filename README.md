# CSAPP Lab Solutions

This repository documents my learning progress through the classic textbook *Computer Systems: A Programmer's Perspective (CSAPP)*. It serves as a personal archive for my lab solutions.

## Repository Structure

Currently, this repository contains my solutions for the following labs:

* **Data Lab**
* **Bomb Lab**
* **Attack Lab**
* **Cache Lab**
* **Shell Lab**
* **Malloc Lab**

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

---

## Lab 4: Cache Lab

**Status:** Completed  
**Final Score:** 53.0/53  

### Performance and Correctness Results

Below is the summary of the evaluation results:

```text
Cache Lab summary:
                        Points   Max pts      Misses
Csim correctness          27.0        27
Trans perf 32x32           8.0         8         288
Trans perf 64x64           8.0         8        1228
Trans perf 61x67          10.0        10        1993
          Total points    53.0        53
```

---

## Lab 5: Shell Lab

**Status:** Completed  

---

## Lab 6: Malloc Lab

**Status:** Completed  
**Final Score:** 97/100  

### Implementation Details & Simple Optimizations

I have attempted several simple optimizations to improve the efficiency of this memory allocator:

1. **Segregated Free Lists**: 
   I used an array of 10 segregated lists to manage free blocks by size. To slightly refine the search, I maintained these lists in **size-ascending order**. This allows the First-Fit search to behave somewhat like a Best-Fit strategy, which helped in reducing memory fragmentation while keeping search times relatively low.

2. **Heuristic Block Splitting**:
   In the `place` function, I implemented a simple heuristic where the placement of the allocated block (at the beginning or end of a free block) depends on the requested size. This was a modest attempt to better align with specific allocation patterns and minimize external fragmentation.

3. **Basic `realloc` Enhancements**:
   I tried to make `mm_realloc` a bit more efficient by avoiding unnecessary memory copies. If a block can be expanded in-place by merging with a following free block or by extending the heap at the very end, I prioritized those simple actions over a fresh allocation.

4. **Footer Removal (v2 Implementation)**:
   In version `v2`, I explored the idea of removing footers from allocated blocks to save a bit of space. By encoding the previous block's status in the current block's header, I could save 4 bytes per allocation. It was a simple experiment that yielded a satisfying improvement in throughput.

### Performance Results

While there is certainly still much room for improvement, I am satisfied with the current results:
- **v1 Throughput**: 19,170 Kops
- **v2 Throughput**: 27,386 Kops

The footer removal optimization in `v2` provided a decent boost to the operations per second. The final performance index reached 97/100, which I consider a fortunate outcome for these relatively straightforward techniques.
- **Total Performance Index**: 57 (utilization) + 40 (throughput) = 97/100.