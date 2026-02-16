# cfib
A C program that uses GMP to calculate Fibonacci and Lucas numbers. Unlike the Zig version at [zfib](https://github.com/munastronaut/zfib), the final binaries here are only ~17 kilobytes large*. The fast doubling algorithm is used for this purpose.

---
*\* on x64 Linux with `gcc -O3`, at least. Size may vary on other systems.
