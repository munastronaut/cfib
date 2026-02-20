# cfib
A C program that uses GMP to calculate Fibonacci and Lucas numbers. Unlike the Zig version at [zfib](https://github.com/munastronaut/zfib), the final binaries here are only \~15 kilobytes large\*. The fast doubling algorithm is used for this purpose. If you are on Windows, please use MSYS2, Cygwin, Git for Windows, MinGW, or WSL2 to compile this.

---
*\* on x64 Linux with `gcc -O3 -s`, at least. Size may vary on other systems.*
