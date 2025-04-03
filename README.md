<p align="center">
  <a href="" rel="noopener">
 <img src="https://i.imgur.com/HFjhr8F.png" alt="Project logo"></a>
</p>

## 📝 Table of Contents

- [About](#about)
- [How It Works](#how_it_works)
- [Work Progress](#work_progress)
- [Operating time study](#time)
- [Conclusion](#conclusion)
- [Built Using](#built_using)
- [Authors](#authors)

---

## 🧐 About <a name = "about"></a>

This project studies SIMD instructions, the effect of different optimization techniques on the speed of computational algorithms. The rendering of [Mandelbrot set](https://en.wikipedia.org/wiki/Mandelbrot_set) is considered as a time-consuming algorithm.

---

## 🔄 How It Works <a name = "how_it_works"></a>

The Mandelbrot set is constructed as follows:
1. The radius of some circle on the complex plane is set, inside which we will build a set (`MAX_RADIUS = 4.0f` is taken).
2. Some point with coordinates $(x_{0}, y_{0})$ inside this circle is selected.
3. Then we consider points with coordinates $(x_{n}, y_{n})$, where 
  - $$x_{n} = x_{n-1}^2 - y_{n-1}^2 + x_{0}$$
  - $$y_{n} = 2 \cdot x_{n-1} \cdot y_{n-1} + y_{0}$$
4. We will build a sequence of such points until the k-th point is not removed by a distance greater than `MAX_RADIUS`.
5. Let's color the very first point with coordinates $(x_{0}, y_{0})$ with some color (depends on the color themes).

<p align="center">
  <a href="" rel="noopener">
 <img src="https://i.imgur.com/m9lE6s3.png" alt="theme_0"></a>
</p>

<p align="center">
  <a href="" rel="noopener">
 <img src="https://i.imgur.com/aoAR43e.png" alt="theme_1"></a>
</p>

<p align="center">
  <a href="" rel="noopener">
 <img src="https://i.imgur.com/EQ3q7Xm.png" alt="theme_2"></a>
</p>


## 🏗 Work Progress <a name = "work_progress"></a>

We will compare two implementations of this algorithm.
1. Simple version **without any optimizations**. Each pixel is enumerated and points are calculated, then it is painted.
2. A version using **arrays and AVX instructions**. We will sequentially take several points (in my case, 8 points) and calculate the color of each point within this array. This way we will hint to the compiler that the algorithm can be simplified.

For some inference, we will compare the running time of each implementation of the Mandelbrot set rendering.
To measure the time, we will use the assembly instruction `__rdtsc()`, which returns the CPU timestamp (the number of clock cycles since the last reset). We will count the clock cycle difference between the beginning of the set calculation and its end. Thus, we will get the running time of the algorithm in clock cycles. 

For more accurate measurements, we will measure the running time for `TEST` = 300 frames, then get the average number of clock cycles per frame. We will run each program `3 times` to get the most accurate result (then we will use the least squares method).

Let's try to run the programs with different compiler optimization levels (`-O0` and `-O3`), besides, let's specify the **compilation flag** `-mavx2` to allow the compiler to apply `AVX instructions` for acceleration.

---

## ⏱️ Operating time study <a name = "time"></a>

First, we analyze the performance of the standard implementation depending on compiler optimizations. We will use the method of least squares.

During program execution, the processor frequency was maintained at `4.2GHz`.

<p align="center">
  <a href="" rel="noopener">
 <img src="https://i.imgur.com/gA5BIXI.jpeg" alt="theme_2"></a>
</p>

Let's convert ticks to seconds.

* -O0:
  + $t_1 = 9,032,576,960$ ticks -> $\frac{9,032,576,960}{4.2 \cdot 10^9} \approx 2.1506 s$
  + $t_2 = 8,962,383,657$ ticks -> $\frac{8,962,383,657}{4.2 \cdot 10^9} \approx 2.1339 s$
  + $t_3 = 8,976,928,612$ ticks -> $\frac{8,976,928,612}{4.2 \cdot 10^9} \approx 2.1374 s$

* -O3:
  + $t_1 = 4,320,616,733$ ticks -> $\frac{4,320,616,733}{4.2 \cdot 10^9} \approx 1.0287 s$
  + $t_2 = 4,315,166,582$ ticks -> $\frac{4,315,166,582}{4.2 \cdot 10^9} \approx 1.0274 s$
  + $t_3 = 4,373,371,197$ ticks -> $\frac{4,373,371,197}{4.2 \cdot 10^9} \approx 1.0413 s$

Let's calculate the arithmetic mean of the obtained values.

* -O0: $t_{avg} = \frac{2.1506 + 2.1339 + 2.1374}{3} \approx 2.1406s$
* -O3: $t_{avg} = \frac{1.0287 + 1.0274 + 1.0413}{3} \approx 1.0325s$

Standard deviation ($\sigma$):

* -O0: $\sigma = \sqrt{\frac{(2.1506 - 2.1406)^2 + (2.1339 - 2.1406)^2 + (2.1374 - 2.1406)^2}{2}} \approx 0.0088 s$
* -O3: $\sigma = \sqrt{\frac{(1.0287 - 1.0325)^2 + (1.0274 - 1.0325)^2 + (1.0413 - 1.0325)^2}{2}} \approx 0.0077 s$

Z-score ($Z$):

* -O0:
  + $Z_{1} = \frac{2.1506 - 2.1406}{0.0088} \approx 1.14$
  + $Z_{2} = \frac{2.1339 - 2.1406}{0.0088} \approx -0.76$
  + $Z_{3} = \frac{2.1374 - 2.1406}{0.0088} \approx -0.36$

* -O3:
  + $Z_{1} = \frac{1.0287 - 1.0325}{0.0077} \approx -0.49$
  + $Z_{2} = \frac{1.0274 - 1.0325}{0.0077} \approx -0.66$
  + $Z_{3} = \frac{1.0413 - 1.0325}{0.0077} \approx 1.14$

$|Z|< 3 \Rightarrow $ no emissions. 

Let's take into account the margin of error.

* -O0: $\sigma = 0.0088 s (\approx 0.4\% \text{ of avg.})$ - stable data
* -O3: $\sigma = 0.0077 s (\approx 0.7\% \text{ of avg.})$ - stable data

| Compiler Optimization   | $t_{1}, s$ | $t_{2}, s$ | $t_{3}, s$ | $t_{avg}, s$ | $t_{control}, s$ |
|:-----------------------:|:----------:|:----------:|:----------:|:------------:|:----------------:|
|  -O0                    | 2.1506     | 2.1339     | 2.1374     | 2.1406       |  2.1392          |
|  -O3                    | 1.0287     | 1.0274     | 1.0413     | 1.0325       |  1.0387          |

> [!NOTE]
> `-O3` optimization reduces the time by a factor of about **2.07** > (from 2.141 s to 1.032 s).
> The data is consistent, the arithmetic mean is suitable as there are no outliers.

Now let's analyze the optimized ``SIMD technology``.
We will realize calculations with the help of intrinsic functions, and the calculation will be performed in parallel for several points (in my case, we will use `__m256` vectors of size 256 bits, store type `float`).   

| Compiler Optimization   | $t_{1}, s$ | $t_{2}, s$ | $t_{3}, s$ | $t_{avg}, s$ | $t_{control}, s$ |
|:-----------------------:|:----------:|:----------:|:----------:|:------------:|:----------------:|
|  -O0                    | 0.7236     | 0.7453     | 0.7063     | 0.7251       |  0.7302          |
|  -O3                    | 0.1488     | 0.1486     | 0.1500     | 0.1491       |  0.1468          |


> [!NOTE]
> `-O3` optimization reduces the time by a factor of about **4.87** > (from 0.7251 s to 0.1491 s).

Now let's compare the performance gain at `-O0` and `-O3` of the standard and optimized versions.

* -O0: $\frac{2.1406}{0.7251} \approx 2.95 $
* -O3: $\frac{1.0325}{0.1491} \approx 6.92 $

> [!WARNING]
> So, compiler optimizations `-O3` and `SIMD technologies and AVX instructions` by rendering optimization give almost 7 times increase!

---

## 🛠️ Conclusion <a name = "conclusion"></a>

`SIMD instructions` really speed up the program a lot. The ability of **parallel computing** has a strong impact on performance. In the case of **Mandelbrot set** rendering, these optimizations (together with compiler optimizations `-O3`) can give a gain of almost `7 times`!

---

## ⛏ Built Using <a name = "built_using"></a>

- [customWarning](https://github.com/lvbealr/customWarning) - Warning Module
- [colorPrint](https://github.com/lvbealr/colorPrint) - Module For Colourful Print

---

## ✍ Authors <a name = "authors"></a>

- [@lvbealr](https://github.com/lvbealr) - Idea & Initial work
