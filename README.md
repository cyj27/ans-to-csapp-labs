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
* **Proxy Lab**

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
**Final Score:** 98/100  

### Implementation Details & Optimizations

This allocator was optimized incrementally rather than by one-shot parameter tuning. The main improvements were:

1. **Explicit Free List + Segregated Classes**:
   Free blocks are managed in 12 segregated lists to reduce search time. Lists are ordered by size to provide a fast Best-Fit match.

2. **Heuristic Split Direction in place**:
   Request-size aware splitting (Threshold=96). Large blocks are placed at the end of free areas, small blocks at the front, significantly reducing external fragmentation.

3. **realloc In-place Growth First**:
   Optimized `realloc` to prioritize merging next free blocks and "Touch-the-Sky" expansion at the heap end to avoid unnecessary data copies.

4. **Footerless Design (Footless)**:
   Implemented Footer Elimination for allocated blocks using the `PREV_ALLOC` bit in headers. This reduces metadata overhead by 50% for 16-byte blocks.

### Performance Results & Data Comparison

The following table compares the performance of the Segregated List version (with footers) against the final Footless version:

| Version | Utilization | Throughput (Kops) | Perf Index |
| :--- | :--- | :--- | :--- |
| Footer-based (v1) | 97% | 30.5 | 98/100 |
| Footless Optimized (v2) | **97%** | **30.7** | **98/100** |

Conclusion: The Footless design combined with a staged heap expansion policy (`INITCHUNKSIZE=64`, `CHUNKSIZE=4096`) achieves the optimal balance of memory efficiency and speed.

---

## Lab 7: Proxy Lab

**Status:** Completed

### Implementation Details

This is a concurrent HTTP/1.0 proxy server with an LRU cache, implemented in three parts:

1. **Part I — Sequential Proxy**:
   - Accepts client HTTP GET requests, parses the URI to extract hostname, port, and path
   - Forwards the request to the target server as HTTP/1.0
   - Relays the server response back to the client
   - Handles `SIGPIPE` gracefully to avoid crashes on broken connections

2. **Part II — Thread-based Concurrency**:
   - Uses `Pthread_create` to spawn a detached thread for each incoming connection
   - Thread-safe design ensures the proxy can handle multiple clients concurrently

3. **Part III — LRU Cache**:
   - Thread-safe LRU (Least Recently Used) cache with a semaphore lock
   - Doubly linked list for O(1) insertion/deletion/reordering
   - Configurable limits: `MAX_CACHE_SIZE` = 1 MiB, `MAX_OBJECT_SIZE` = 100 KiB
   - Automatic eviction of least-recently-used entries when space is insufficient

### Key Design Decisions

- All forwarded requests use HTTP/1.0 with `Connection: close` to simplify proxy logic
- Fixed `User-Agent` header sent per lab specification
- Client's `Host`, `Connection`, and `Proxy-Connection` headers are filtered and replaced
- RIO (Robust I/O) functions used for all socket I/O to handle partial reads/writes
