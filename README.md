<p align="center">
  <a href="" rel="noopener">
 <img src="https://i.imgur.com/HFjhr8F.png" alt="Project logo"></a>
</p>

## 📝 Table of Contents

- [About](#about)
- [How It Works](#how_it_works)
- [Work Progress](#work_progress)
- [Operating time study](#time)
- [GLSL Implementation](#gpu)
- [Multithreading](#multithreading)
- [Installation](#installation)
- [Example Usage](#example_usage)
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

<div style="display: flex; justify-content: space-between;">
  <img src="https://i.imgur.com/PxwyMLQ.png" alt="Black & White" width="33%">
  <img src="https://i.imgur.com/vibS6Ox.png" alt="Blue & Purple" width="33%">
  <img src="https://i.imgur.com/KxzxFk5.png" alt="Yellow & Brown" width="33%">
</div>


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

> [!NOTE]
> The project was run on an 12th Gen Intel(R) Core(TM) i5-1235U (10 Cores, 12 Threads, Alder Lake, AVX2 support).

First, we analyze the performance of the standard implementation depending on compiler optimizations. Tests were conducted at a resolution of `800x600` pixels, `MAX_ITER = 256`. **100 frames** were rendered **100 times**. The [graphics/default.py](https://github.com/lvbealr/MandelbrotSet/blob/main/graphics/default.py) and [graphics/comparison.py](https://github.com/lvbealr/MandelbrotSet/blob/main/graphics/comparison.py) files calculate data for comparing the performance of **compiler-only optimizations on the naive version** and the **array-optimized** and **AVX256-optimized** versions, respectively. For each iteration number, the arithmetic mean of execution time over all frames is calculated, and the standard deviation is a measure of data dispersion. Ideally, we should get at least an **8-fold** increase. Will it work?

First, let's see how well the compiler works: how much can it optimize the code?

| Compiler Optimization   | Default Version, ticks per frame | 
|:-----------------------:|:--------------------------------:|
|  -O0                    | 165452507.43                     |
|  -O3                    | 86191819.96                      |

`-O3` optimization reduces the time by a factor of about **1.91** > (from 16.55e7 to 8.61e7 ticks).

<p align="center">
  <a href="" rel="noopener">
 <img src="https://github.com/lvbealr/MandelbrotSet/blob/main/graphics/img/defaultCmp.png" alt="Default Comparison"></a>
</p>

---

Now let's look at the result of my own optimizations. In the array-optimized version, 8 points are processed at each loop step - so let's `try to parallelize the calculations`. In addition, `Intel Intrinsics similar functions` are implemented. I wonder if the compiler will see my hints about using intrinsics?

Here we will also consider the result of the program with explicit use of `AVX256 instructions` (in my case, i use `__m256` vectors of size 256 bits, store type `float`).

| Compiler Optimization   | Default Version, ticks per frame | Array-optimized, ticks per frame | AVX256-optimized, ticks per frame |
|:-----------------------:|:--------------------------------:|:--------------------------------:|:---------------------------------:|
|  -O3                    | 86191819.96                      | 43982076.92                      | 18982180.16                       |

The array-optimized version works `1.96 times faster` than the primitive version. 

<p align="center">
  <a href="" rel="noopener">
 <img src="https://github.com/lvbealr/MandelbrotSet/blob/main/graphics/img/totalCmp.png" alt="Total Comparison"></a>
</p>

In general, my hints to the compiler did not help much. It used AVX2 instructions only when specifying `R2max` and `POINTS` vectors. 

```asm
               00103053 c5 fc 29 05 45        VMOVAPS         ymmword ptr [_ZZ26calculateMandelbrotSectionPhfffmmE5R2max],YMM0
                        52 00 00
               0010305b 48 39 d6              CMP             param_2,param_3
               0010305e c5 fc 28 05 da        VMOVAPS         YMM0,ymmword ptr [DAT_00106240]
                        31 00 00
               00103066 c5 fc 29 05 12        VMOVAPS         ymmword ptr [_ZZ26calculateMandelbrotSectionPhfffmmE6POINTS],YMM0
                        52 00 00
```
Finally, explicit use of **intrinsic-functions** gives a performance boost of more than `4.54 times!`

---

## 📽️ GLSL Implementation <a name = "gpu"></a>

In this work, rendering of the Mandelbrot set on the GPU was implemented using GLSL shaders. This approach speeds up rendering by several thousand times. However, it was implemented only for entertainment and comparison, so it was not subjected to thorough analysis.

<div style="display: flex; justify-content: space-between;">
  <img src="https://github.com/lvbealr/MandelbrotSet/blob/main/gif/rainbow.gif" alt="Rainbow" width="48%">
  <img src="https://github.com/lvbealr/MandelbrotSet/blob/main/gif/darkBlue.gif" alt="Dark Blue" width="48%">
</div>

<div style="display: flex; justify-content: space-between;">
  <img src="https://github.com/lvbealr/MandelbrotSet/blob/main/gif/purple.gif" alt="Purple" width="48%">
  <img src="https://github.com/lvbealr/MandelbrotSet/blob/main/gif/redTurquoise.gif" alt="Red Turquoise" width="48%">
</div>

<div style="display: flex; justify-content: space-between;">
  <img src="https://github.com/lvbealr/MandelbrotSet/blob/main/gif/yellowGold.gif" alt="Yellow & Gold" width="48%">
  <img src="https://github.com/lvbealr/MandelbrotSet/blob/main/gif/blackPulse.gif" alt="Black Pulse" width="48%">
</div>

---

## 🕶️ Multithreading <a name = "multithreading"></a>

To further optimize performance, the rendering process was parallelized using **multithreading**. The image is divided into horizontal sections (stripes), each processed by a separate CPU thread. This approach leverages modern multi-core processors to significantly reduce computation time.

### Implementation Details:
  - Thread Management: The C++ Standard Thread Library (`std::thread`) is used to create and manage worker threads.
  - Work Distribution: The image height is divided into N equal segments (where N is the number of available CPU threads). Each thread processes its assigned rows independently.

`Threads = 12`

| Implementation          | Ticks per frame, ticks per frame | FPS (avg, on start) |
|:-----------------------:|:--------------------------------:|:-------------------:|
|  Default (-O0)          | 76544332                         | 31                  |
|  Default (-O3)          | 34346892                         | 69                  |
|  Array-optimized (-O3)  | 22714218                         | 118                 |
|  AVX-optimized (-O3)    | 7610524                          | 284                 |

Combined with AVX256 optimizations, the multithreaded version achieves over **21x speedup** compared to the original single-threaded implementation without vectorization.

---

## ⚙️ Installation <a name = "installation"></a>

First of all, clone the repository to your computer and update the submodules in the project.

```bash
git clone git@github.com:lvbealr/MandelbrotSet.git
git submodule update --init --remote --recursive
```

Next, use Make to compile the files.

```bash
make <target> [OPTIMIZATION_LEVEL=<lvl>=-O0] [MULTITHREADING=<status>=] [TEST=<testNumber>=300]
```

Now run the program, e.g.:

```bash
./build/gpu/gpu
```

---

## 🕹️ Example Usage <a name = "example_usage"></a>

The program has the ability to dynamically change the coloring and display FPS. Below is a list of hot keys:

| Hotkey |       Action        | 
|:------:|:-------------------:|
|  W     | Move up             |
|  S     | Move down           |
|  A     | Move left           |
|  D     | Move right          |
|  ↑     | Zoom in             |
|  ↓     | Zoom out            |
|  T     | Change color scheme |
|  F11   | Show FPS / Hide FPS |

---

## 🛠️ Conclusion <a name = "conclusion"></a>

`SIMD instructions` really speed up the program a lot. The ability of **parallel computing** has a strong impact on performance. In the case of **Mandelbrot set** rendering, these optimizations (together with compiler optimizations `-O3`) can give a gain of almost `8.7 times` (compared to default `-O0` version)!



---

## ⛏ Built Using <a name = "built_using"></a>

- [customWarning](https://github.com/lvbealr/customWarning) - Warning Module
- [colorPrint](https://github.com/lvbealr/colorPrint) - Module For Colourful Print

---

## ✍ Authors <a name = "authors"></a>

- [@lvbealr](https://github.com/lvbealr) - Idea & Initial work
