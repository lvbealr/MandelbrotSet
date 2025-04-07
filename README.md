<p align="center">
  <a href="" rel="noopener">
 <img src="https://i.imgur.com/HFjhr8F.png" alt="Project logo"></a>
</p>

## 📖 Версия / Version
- [🇷🇺 RU](#RU)
- [🇺🇸 ENG](#ENG)

---

## 🇷🇺 RU <a name="RU"></a>

## 📝 Содержимое

- [О проекте](#aboutRU)
- [Принцип работы](#how_it_worksRU)
- [Ход работы](#work_progressRU)
- [Вычисление времени работы](#timeRU)
- [GLSL реализация](#gpuRU)
- [Многопоточность](#multithreadingRU)
- [Установка](#installationRU)
- [Пример использования](#example_usageRU)
- [Заключение](#conclusionRU)
- [Инструменты](#built_usingRU)
- [Авторы](#authorsRU)

---

## 🧐 О проекте <a name = "aboutRU"></a>

В данном проекте изучаются SIMD-инструкции, влияние различных методов оптимизации на скорость работы вычислительных алгоритмов. В качестве трудоемкого алгоритма рассматривается рендеринг [множества Мандельброта](https://en.wikipedia.org/wiki/Mandelbrot_set).

---

## 🔄 Принцип работы <a name = "how_it_worksRU"></a>

Множество Мандельброта строится следующим образом:
1. Задается радиус некоторой окружности на комплексной плоскости, внутри которой мы будем строить множество (берется `MAX_RADIUS = 4.0f`).
2. Выбирается некоторая точка с координатами $(x_{0}, y_{0})$ внутри этой окружности.
3. Далее рассматриваются точки с координатами $(x_{n}, y_{n})$, где
  - $$x_{n} = x_{n-1}^2 - y_{n-1}^2 + x_{0}$$
  - $$y_{n} = 2 \cdot x_{n-1} \cdot y_{n-1} + y_{0}$$
4. Будем строить последовательность таких точек до тех пор, пока k-ая точка не будет удалена на расстояние большее, чем `MAX_RADIUS`.
5. Раскрасим самую первую точку с координатами $(x_{0}, y_{0})$ каким-либо цветом (зависит от цветовой темы)

<div style="display: flex; justify-content: space-between;">
  <img src="https://i.imgur.com/PxwyMLQ.png" alt="Black & White" width="33%">
  <img src="https://i.imgur.com/vibS6Ox.png" alt="Blue & Purple" width="33%">
  <img src="https://i.imgur.com/KxzxFk5.png" alt="Yellow & Brown" width="33%">
</div>


## 🏗 Ход работы <a name = "work_progressRU"></a>

Сравним три реализации этого алгоритма.

1. Наивная версия **без каких-либо оптимизаций**. Каждая точка обрабатывается и закрашивается последовательно.
2. Версия с использованием **массива точек**. Мы будем последовательно брать несколько точек (в моем случае 8 точек) и вычислять цвет каждой точки в этом массиве. Таким образом мы намекнем компилятору, что алгоритм может быть упрощен.
3. Версия и использованием **AVX256 инструкций**. Массивы точек обрабатываются параллельно благодаря конвейеризации вычислений.

Чтобы сделать некоторые выводы, мы сравним время работы каждой реализации рендеринга множества Мандельброта.
Для измерения времени мы воспользуемся ассемблерной инструкцией `__rdtsc()`, которая возвращает временную метку процессора (количество тактов с момента последнего сброса). Мы подсчитаем разницу в тактах между началом вычисления набора и его окончанием. Таким образом, мы получим время работы алгоритма в тактах.

Для более точных измерений мы измерим время работы для `TEST` = 100 кадров, а затем получим среднее количество тактов на кадр. Мы запустим каждую программу `100 раз`, чтобы получить наиболее точный результат.

Попробуем запустить программы с разными уровнями оптимизации компилятора (`-O0` и `-O3`), кроме того, укажем **флаг компиляции** `-mavx2`, чтобы компилятор мог применить `AVX-инструкции` для ускорения.

---

## ⏱️ Вычисление времени работы <a name = "timeRU"></a>

### 🖥️ Технические характеристики тестовой среды:
  + Процессор: 12th Gen Intel® Core™ i5-1235U (4+8) @ 4.40GHz (10 Cores, 12 Threads, Alder Lake, AVX2 support)
  + Оперативная память: 16GB DDR4 (3200 MT/s)
  + Операционная система: Arch Linux 6.13.5-arch1-1 x86-64
  + Компилятор: g++ (GCC) 14.2.1
  + Разрешение: 800x600 pixels
  + Максимальное количество итераций: 256
  + Тестовые конфигурации: 100 запусков программы с отрисовкой 100 кадров

Сначала мы проанализируем производительность стандартной реализации в зависимости от оптимизаций компилятора. В файлах [graphics/default.py](https://github.com/lvbealr/MandelbrotSet/blob/main/graphics/default.py) и [graphics/comparison.py](https://github.com/lvbealr/MandelbrotSet/blob/main/graphics/comparison.py) вычисляются данные для сравнения производительности **наивной версии только лишь с компиляторными оптимизациями** и версиях **отпимизированной массивами** и **оптимизированной AVX256 инструкциями**, соответственно. Для каждого номера итерации вычисляется среднее арифметическое время выполнения по всем кадрам, а стандартное отклонение является мерой разброса данных. В идеале мы должны получить как минимум **8-кратный** прирост. Будет ли это работать?

Сначала посмотрим, насколько хорошо работает компилятор: как сильно он может оптимизировать код?

| Уровень оптимизаций компилятора   | Наивная версия, тиков на кадр | 
|:---------------------------------:|:-----------------------------:|
|  -O0                              | 165452507.43                  |
|  -O3                              | 86191819.96                   |

Оптимизация `-O3` сокращает время примерно в **1,91** > раз (с 16,55e7 до 8,61e7 тиков).

```md
[Статистика для наивной версии (-O0)]
Среднее время выполнения: 165452507.44 ± 546207.74 тиков (стандартная ошибка среднего)
Среднее стандартное отклонение: 5462077.40 тиков
Погрешность измерений: 0.33%

[Статистика для наивной версии (-O3)]
Среднее время выполнения: 86191819.96 ± 253457.40 тиков (стандартная ошибка среднего)
Среднее стандартное отклонение: 2534573.96 тиков
Погрешность измерений: 0.29%
```

<p align="center">
  <a href="" rel="noopener">
 <img src="https://github.com/lvbealr/MandelbrotSet/blob/main/graphics/img/defaultCmp.png" alt="Сравнение наивных версий"></a>
</p>

---

Теперь давайте посмотрим на результат моих собственных оптимизаций. В оптимизированной массивами версии на каждом шаге цикла обрабатывается 8 точек - попробуем распараллелить вычисления.
В этом режиме программа обрабатывает массив из 8 пикселей за итерацию цикла, что позволяет компилятору в режиме `-O3` выполнить частичную векторизацию этого процесса.
Кроме того, реализованы `Intel Intrinsics - подобные функции`. Интересно, увидит ли компилятор мои намеки на использование intrinsic-функций?

Здесь мы рассмотрим результат работы программы с явным использованием инструкций `AVX256` (в моем случае я использую векторы `__m256` размером 256 бит, хранимый тип - `float`).

| Уровень оптимизаций компилятора   | Наивная версия, тиков на кадр | Оптимизация массивами, тиков на кадр | AVX256 оптимизация, тиков на кадр |
|:---------------------------------:|:-----------------------------:|:------------------------------------:|:---------------------------------:|
|  -O3                              | 86191819.96                   | 43982076.92                          | 18982180.16                       |

Версия, оптимизированная массивами, работает в `1,96 раза быстрее`, чем наивная версия.

```md
[Статистика для наивной версии (-O3)]
Среднее время выполнения: 86191819.96 ± 253457.40 тиков (стандартная ошибка среднего)
Среднее стандартное отклонение: 2534573.96 тиков
Погрешность измерений: 0.29%

[Статистика для оптимизированной массивами версии (-O3)]
Среднее время выполнения: 43982076.93 ± 114784.04 тиков (стандартная ошибка среднего)
Среднее стандартное отклонение: 1147840.45 тиков
Погрешность измерений: 0.26%

[Статистика для оптимизированной AVX256 инструкциями версии (-O3)]
Среднее время выполнения: 18982180.16 ± 62965.22 тиков (стандартная ошибка среднего)
Среднее стандартное отклонение: 629652.23 тиков
Погрешность измерений: 0.33%
```

<p align="center">
  <a href="" rel="noopener">
 <img src="https://github.com/lvbealr/MandelbrotSet/blob/main/graphics/img/totalCmp.png" alt="Общее сравнение"></a>
</p>


В общем, мои подсказки компилятору мало чем помогли. Он использовал инструкции AVX2 только при инициализации векторов `R2max` и `POINTS`.

```asm
               00103053 c5 fc 29 05 45        VMOVAPS         ymmword ptr [_ZZ26calculateMandelbrotSectionPhfffmmE5R2max],YMM0
                        52 00 00
               0010305b 48 39 d6              CMP             param_2,param_3
               0010305e c5 fc 28 05 da        VMOVAPS         YMM0,ymmword ptr [DAT_00106240]
                        31 00 00
               00103066 c5 fc 29 05 12        VMOVAPS         ymmword ptr [_ZZ26calculateMandelbrotSectionPhfffmmE6POINTS],YMM0
                        52 00 00
```
Наконец, явное использование **intrinsic-функций** дает прирост производительности более чем в `4,54 раза`.

Теперь давайте сравним различные реализации с оптимизациями компилятора `-O0`. Без оптимизаций компилятора версия, оптимизированная массивами, очевидно, будет **медленнее**, даже чем наивная. Доступ к элементам массива требует **вычисления адресов**, **чтения** и **записи в память**, что гораздо медленнее, чем работа с регистрами процессора. При этом каждая операция над элементом массива выполняется отдельно, что значительно медленнее, чем в последовательном коде, из-за накладных расходов на управление массивами.

| Уровень оптимизаций компилятора   | Наивная версия, тиков на кадр | Оптимизация массивами, тиков на кадр | AVX256 оптимизация, тиков на кадр |
|:---------------------------------:|:-----------------------------:|:------------------------------------:|:---------------------------------:|
|  -O0                              | 158205005.31                  | 704089035.51                         | 73275478.12                       |

---

## 📽️ GLSL реализация <a name = "gpuRU"></a>

В данной работе рендеринг множества Мандельброта на GPU был реализован с помощью шейдеров GLSL. Такой подход ускоряет рендеринг в несколько тысяч раз. Однако он был реализован только для развлечения и сравнения, поэтому не подвергался тщательному анализу.

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

## 🕶️ Многопоточность <a name = "multithreadingRU"></a>

Для дальнейшей оптимизации производительности процесс рендеринга был распараллелен с использованием **многопоточности**. Изображение делится на горизонтальные участки (полосы), каждый из которых обрабатывается отдельным потоком процессора. Такой подход позволяет использовать современные многоядерные процессоры для значительного сокращения времени вычислений.

### Детали реализации:
  - Управление потоками: для создания и управления рабочими потоками используется стандартная библиотека потоков C++ (`std::thread`)
  - Распределение работы: изображение делится на N равных сегментов (где N - количество доступных потоков процессора). Каждый поток обрабатывает назначенные ему строки независимо от других потоков.

`Количество потоков = 12`

| Реализация              | Тиков на кадр | FPS (среднее, на запуске) |
|:-----------------------:|:-------------:|:-------------------------:|
|  Default (-O0)          | 76544332      | 31                        |
|  Default (-O3)          | 34346892      | 69                        |
|  Array-optimized (-O3)  | 22714218      | 118                       |
|  AVX-optimized (-O3)    | 7610524       | 284                       |

В сочетании с оптимизациями AVX256 многопоточная версия достигает более чем **21-кратного ускорения** по сравнению с оригинальной однопоточной реализацией без векторизации.

---

## ⚙️ Установка <a name = "installationRU"></a>

Прежде всего, склонируйте репозиторий на свой компьютер и обновите подмодули в проекте.

```bash
git clone git@github.com:lvbealr/MandelbrotSet.git
git submodule update --init --remote --recursive
```

Затем используйте Make для компиляции файлов.

```bash
make <target> [OPTIMIZATION_LEVEL=<lvl>=-O0] [MULTITHREADING=<status>=] [TEST=<testNumber>=300]
```

Теперь запустите программу, например:

```bash
./build/gpu/gpu
```

---

## 🕹️ Пример использования <a name = "example_usageRU"></a>

Программа имеет возможность динамически изменять раскраску и отображать FPS. Ниже приведен список горячих клавиш:

| Клавиша |       Действие            | 
|:-------:|:-------------------------:|
|  W      | Вверх                     |
|  S      | Вверх                     |
|  A      | Влево                     |
|  D      | Вправо                    |
|  ↑      | Увеличить изображение     |
|  ↓      | Уменьшить изображение     |
|  T      | Сменить цветовую палитру  |
|  F11    | Показать FPS / скрыть FPS |

---

## 🛠️ Заключение <a name = "conclusionRU"></a>

Технология `SIMD` действительно значительно ускоряет работу программы. Возможность **параллельных вычислений** сильно влияет на производительность. В случае рендеринга **множества Мандельброта** эти оптимизации (вместе с оптимизациями компилятора `-O3`) могут дать выигрыш почти в `8,7 раз` (по сравнению со стандартной версией `-O0`)!

---

## ⛏ Инструменты <a name = "built_usingRU"></a>

- [customWarning](https://github.com/lvbealr/customWarning) - модуль для исключений
- [colorPrint](https://github.com/lvbealr/colorPrint) - модуль для вывода цветных сообщений

---

## ✍ Авторы <a name = "authorsRU"></a>

- [@lvbealr](https://github.com/lvbealr) - реализация

---

## 🇺🇸 ENG <a name="ENG"></a>

---

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

We will compare three implementations of this algorithm.
1. Simple version **without any optimizations**. Each pixel is enumerated and points are calculated, then it is painted.
2. A version using **arrays of pixels**. We will sequentially take several points (in my case, 8 points) and calculate the color of each point within this array. This way we will hint to the compiler that the algorithm can be simplified.
3. A version using **AVX256 instructions**. Point arrays are processed in parallel due to computation pipelining.

For some inference, we will compare the running time of each implementation of the Mandelbrot set rendering.
To measure the time, we will use the assembly instruction `__rdtsc()`, which returns the CPU timestamp (the number of clock cycles since the last reset). We will count the clock cycle difference between the beginning of the set calculation and its end. Thus, we will get the running time of the algorithm in clock cycles. 

For more accurate measurements, we will measure the running time for `TEST` = 100 frames, and then get the average number of clock cycles per frame. We will run each program `100 times` to get the most accurate result.

Let's try to run the programs with different compiler optimization levels (`-O0` and `-O3`), besides, let's specify the **compilation flag** `-mavx2` to allow the compiler to apply `AVX instructions` for acceleration.

---

## ⏱️ Operating time study <a name = "time"></a>

### 🖥️ Test Environment Specifications:
  + CPU: 12th Gen Intel® Core™ i5-1235U (4+8) @ 4.40GHz (10 Cores, 12 Threads, Alder Lake, AVX2 support)
  + RAM: 16GB DDR4 (3200 MT/s)
  + OS: Arch Linux 6.13.5-arch1-1 x86-64
  + Compiler: g++ (GCC) 14.2.1
  + Resolution: 800x600 pixels
  + Max Iterations: 256
  + Test Configurations: 100 program runs with rendering of 100 frames

First, we analyze the performance of the standard implementation depending on compiler optimizations. The [graphics/default.py](https://github.com/lvbealr/MandelbrotSet/blob/main/graphics/default.py) and [graphics/comparison.py](https://github.com/lvbealr/MandelbrotSet/blob/main/graphics/comparison.py) files calculate data for comparing the performance of **compiler-only optimizations on the naive version** and the **array-optimized** and **AVX256-optimized** versions, respectively. For each iteration number, the arithmetic mean of execution time over all frames is calculated, and the standard deviation is a measure of data dispersion. Ideally, we should get at least an **8-fold** increase. Will it work?

First, let's see how well the compiler works: how much can it optimize the code?

| Compiler Optimization   | Default Version, ticks per frame | 
|:-----------------------:|:--------------------------------:|
|  -O0                    | 165452507.43                     |
|  -O3                    | 86191819.96                      |

`-O3` optimization reduces the time by a factor of about **1.91** > (from 16.55e7 to 8.61e7 ticks).

```md
[Statistics for Default Version (-O0)]
Average Execution Time: 165452507.44 ± 546207.74 ticks (SEM)
Mean Standard Deviation: 5462077.40 ticks
Relative Error: 0.33%

[Statistics for Default Version (-O3)]
Average Execution Time: 86191819.96 ± 253457.40 ticks (SEM)
Mean Standard Deviation: 2534573.96 ticks
Relative Error: 0.29%
```

<p align="center">
  <a href="" rel="noopener">
 <img src="https://github.com/lvbealr/MandelbrotSet/blob/main/graphics/img/defaultCmp.png" alt="Default Comparison"></a>
</p>

---

Now let's look at the result of my own optimizations. In the array-optimized version, 8 points are processed at each loop step - so let's `try to parallelize the calculations`.
In this mode, the program processes an array of 8 pixels per loop iteration, which allows the compiler in `-O3 mode` to perform partial vectorization of this process.
In addition, `Intel Intrinsics similar functions` are implemented. I wonder if the compiler will see my hints about using intrinsics?

Here we will also consider the result of the program with explicit use of `AVX256 instructions` (in my case, i use `__m256` vectors of size 256 bits, store type `float`).

| Compiler Optimization   | Default Version, ticks per frame | Array-optimized, ticks per frame | AVX256-optimized, ticks per frame |
|:-----------------------:|:--------------------------------:|:--------------------------------:|:---------------------------------:|
|  -O3                    | 86191819.96                      | 43982076.92                      | 18982180.16                       |

The array-optimized version works `1.96 times faster` than the primitive version. 

```md
[Statistics for Default Version (-O3)]
Average Execution Time: 86191819.96 ± 253457.40 ticks (SEM)
Mean Standard Deviation: 2534573.96 ticks
Relative Error: 0.29%

[Statistics for Array Optimized Version (-O3)]
Average Execution Time: 43982076.93 ± 114784.04 ticks (SEM)
Mean Standard Deviation: 1147840.45 ticks
Relative Error: 0.26%

[Statistics for AVX256 Optimized Version (-O3)]
Average Execution Time: 18982180.16 ± 62965.22 ticks (SEM)
Mean Standard Deviation: 629652.23 ticks
Relative Error: 0.33%
```

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

Now let's compare different implementations with compiler optimizations `-O0`. Without compiler optimizations, the array-optimized version will obviously be **slower** even than the naive one. Accessing the array elements requires **calculating addresses**, **reading** and **writing to memory**, which is much slower than working with processor registers. In this case, each operation on an array element is performed separately, which is much slower than sequential code due to the overhead of managing arrays.

| Compiler Optimization   | Default Version, ticks per frame | Array-optimized, ticks per frame | AVX256-optimized, ticks per frame |
|:-----------------------:|:--------------------------------:|:--------------------------------:|:---------------------------------:|
|  -O0                    | 158205005.31                     | 704089035.51                     | 73275478.12                       |

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

| Implementation          | Ticks per frame | FPS (avg, on start) |
|:-----------------------:|:---------------:|:-------------------:|
|  Default (-O0)          | 76544332        | 31                  |
|  Default (-O3)          | 34346892        | 69                  |
|  Array-optimized (-O3)  | 22714218        | 118                 |
|  AVX-optimized (-O3)    | 7610524         | 284                 |

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

- [@lvbealr](https://github.com/lvbealr) - Initial work
