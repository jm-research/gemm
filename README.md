# gemm

ref:

[how-to-optimize-gemm](https://github.com/flame/how-to-optimize-gemm)
[OpenBLAS gemm从零入门](https://zhuanlan.zhihu.com/p/65436463)

## 1. MMult0 到 MMult1

第一个版本直接在主函数 MatrixMultiply 中实现了矩阵乘法的内循环，而第二个版本将内循环抽象成了一个单独的函数 AddDot，这样做的目的是为了模块化和潜在的优化。

在第一个版本中，矩阵乘法的计算逻辑直接嵌套在 MY_MMult 函数的三层循环中：

外层循环遍历矩阵 C 的所有行。

中层循环遍历矩阵 C 的所有列。

内层循环计算矩阵 C 的一个元素，通过累加矩阵 A 的一行与矩阵 B 的一列的点积。

在第二个版本中，矩阵乘法的计算逻辑被重构：

外层循环仍然遍历矩阵 C 的所有列。

中层循环仍然遍历矩阵 C 的所有行。


内层循环被替换为对 AddDot 函数的调用，该函数计算矩阵 A 的一行与矩阵 B 的一列的点积，并将其累加到矩阵 C 的对应元素上。

AddDot 函数接收四个参数：

- k：点积的长度，也就是 A 的行和 B 的列的元素个数。
- x：向量 x 的起始地址，x 是 A 的一行。
- incx：向量 x 的步长，对于 A 来说就是矩阵的列宽。
- y：向量 y 的起始地址，y 是 B 的一列。
- gamma：一个指针，指向矩阵 C 的一个元素，点积的结果将累加到该元素上。

## 2. MMult1 到 MMult2

在这两个版本之间，主要的变化在于循环展开(loop unrolling)技术的应用。这种技术在编译器优化和手动代码优化中经常使用，其目标是减少循环控制结构的开销，从而提高运行时性能。

**原始版本**

原始版本中的 MatrixMul 函数使用了两层循环来更新矩阵 C 的每个元素，其中外层循环遍历矩阵 C 的列，内层循环遍历矩阵 C 的行。每次迭代，它都调用 AddDot 函数来计算 A 的一行与 B 的一列的点积，并将结果累加到 C 的相应位置。

**修改后的版本**

修改后的版本中，MatrixMul 函数的外层循环被展开(unrolled)了 4 倍。这意味着现在外层循环每次迭代处理的是矩阵 C 的连续四列，而不是一列。因此，每轮外层循环都会进行四次 AddDot 调用来更新矩阵 C 的四个不同位置。

具体来说，对于每行 i，它会依次更新 C(i, j)，C(i, j+1)，C(i, j+2)，和 C(i, j+3)。这减少了循环控制结构的开销，因为原本需要四次迭代才能完成的工作，现在只需要一次循环迭代就可以完成。

**循环展开的优势**

循环展开可以带来以下好处：

减少分支预测开销：每次循环迭代都需要条件判断，这可能会导致处理器的分支预测错误，从而增加延迟。通过减少迭代次数，可以减少这些潜在的预测错误。

减少循环开销：每次迭代都需要维护循环计数器并检查退出条件，这增加了额外的指令和周期。通过减少迭代次数，可以节省这些额外的开销。

可能的向量化：循环展开后，某些处理器的向量指令（如 SSE 或 AVX）可能更容易应用，从而利用 SIMD（单指令多数据）并行性，进一步加速计算。

然而，循环展开也可能带来一些缺点，比如代码膨胀和可能的缓存不友好。但是，在许多情况下，特别是在矩阵运算这样的密集型计算场景下，这些优点通常超过了缺点。

## 3. MMult2 到 MMult_1x4_3

在这次代码变更中，主要的改动集中在将原本在 MatrixMul 函数中对 AddDot 函数的多次调用整合到了一个新的函数 AddDot1x4 中。这个新的函数用于同时计算矩阵 C 的四列与矩阵 A 的一行和矩阵 B 的四列的点积，这实质上是对之前循环展开技术的一次封装和抽象。

**原始版本**

在原始版本中，MatrixMul 函数通过四次调用 AddDot 函数来更新矩阵 C 的四列，这意味着每次外层循环迭代都会针对每一行调用四次 AddDot，分别更新 C 的 j, j+1, j+2, 和 j+3 列。

**修改后的版本**

在修改后的版本中，MatrixMul 函数调用了新定义的 AddDot1x4 函数，这个函数接受额外的参数来描述矩阵 B 和 C 的列宽度（ldb 和 ldc），并一次性计算出四列的点积结果。这样做的目的主要是为了减少函数调用的开销，同时保持循环展开带来的性能优势。

AddDot1x4 函数内部实际上调用了四次 AddDot 函数，分别计算四个点积结果，并更新到矩阵 C 的四个不同位置。尽管在内部它仍然调用 AddDot 四次，但是通过将这四次调用封装在一个函数中，可以减少函数调用的开销，特别是函数调用前后的上下文切换和参数传递的时间。

**优化考量**

减少函数调用开销：在 CPU 上，函数调用涉及到保存和恢复寄存器状态、跳转指令和参数传递，这些都是有成本的。通过将多个相似的操作封装在同一个函数中，可以减少这些开销。

潜在的向量化机会：通过将相似的计算合并在一起，有可能使向量化指令更易于应用，这在支持 SIMD 的处理器上尤其重要，可以显著提升性能。

代码可读性和维护性：将相关功能封装在同一个函数中，可以使代码更整洁，更易于理解，也便于未来的维护和优化。

## 4. MMult_1x4_3 到 MMult_1x4_4

在这次修改中，核心的变化是将原本 AddDot1x4 函数内部对 AddDot 函数的调用“内联”了。也就是说，原本通过调用 AddDot 函数来执行的计算现在直接在 AddDot1x4 函数的代码体内完成，消除了函数调用的开销。

**原始版本**

在原始版本中，AddDot1x4 函数通过四次调用 AddDot 函数来计算矩阵 C 的四列与矩阵 A 的一行和矩阵 B 的四列的点积。这种方式虽然封装了重复的计算逻辑，但每次调用 AddDot 都会有一定的函数调用开销。

**修改后的版本**

在修改后的版本中，AddDot1x4 函数不再调用 AddDot 函数，而是直接在函数体内部实现了四次点积计算。这样做的目的是为了减少函数调用所带来的额外开销，如保存和恢复 CPU 寄存器状态、函数调用前后的上下文切换等。

**内联计算**

在 AddDot1x4 函数中，原本的四次 AddDot 函数调用被替换为四个类似的循环，每个循环分别计算矩阵 C 的一个元素与矩阵 A 的一行和矩阵 B 的一列的点积。这种方法直接在函数体内部完成了计算，避免了函数调用的额外成本。

## 5. MMult_1x4_4 到 MMult_1x4_5（性能提升）

这次的代码变更进一步优化了 AddDot1x4 函数，将原本独立的四个循环合并成一个单一的循环，同时计算矩阵 C 的四列与矩阵 A 的一行和矩阵 B 的四列的点积。这种合并循环的技术称为“合并循环”或“循环融合”，旨在减少循环控制结构的数量，从而降低循环的管理开销，提高代码的执行效率。

**原始版本**

在原始版本中，AddDot1x4 函数包含四个独立的循环，每个循环分别计算矩阵 C 的一个元素与矩阵 A 的一行和矩阵 B 的一列的点积。虽然这些循环进行了内联操作，但是每个循环都有其自身的循环控制结构，这会带来额外的开销。

**修改后的版本**

在修改后的版本中，AddDot1x4 函数的四个独立循环被合并成一个单一的循环。这个循环在每次迭代中同时更新矩阵 C 的四个元素，即 C(0,0), C(0,1), C(0,2), 和 C(0,3)。通过这种方式，循环的控制结构被简化，只保留了一个循环计数器和条件检查，从而降低了控制流的开销。

**合并循环的优势**

减少循环控制开销：合并循环减少了循环控制结构的数量，意味着减少了循环初始化、循环条件检查和循环计数器更新的次数，从而提高了代码的执行效率。

可能的向量化和并行化：合并循环后，如果硬件支持向量化指令（如 SSE 或 AVX），那么同一循环迭代中的计算可能更容易向量化，进一步加速计算。此外，合并循环也有助于并行处理，例如在 GPU 或多核 CPU 上。

**计算效率提升**
在合并循环之前，原本有四个独立的循环，每个循环都执行相似的计算，但是各自维护着自己的循环控制结构。这意味着对于每一次循环迭代，都要进行一次初始化、条件检查和计数器更新。当循环被合并后，这些操作被减少到了一个循环中，因此只需要一次更新循环计数器 p 的操作，即可执行八个浮点运算（四个乘法和四个加法）。这是因为四个点积计算被组合在了一起，所以每次迭代都在同时更新矩阵 C 的四个不同元素。

**数据访问模式与缓存利用**

另一个重要的性能提升来自于数据访问模式的优化。在未合并循环的情况下，对于给定的行 A(0, p)，它会被从内存中加载四次，分别用于四个独立的点积计算。然而，当循环被合并后，A(0, p) 只需要被加载一次，然后用于所有四个点积计算。这是因为在合并后的循环中，所有的计算都在一个循环迭代中完成，而不是像原来那样分散在四个独立的循环中。

**缓存效应**

当矩阵足够大以至于无法完全装入 L2 缓存中时，上述的数据访问模式优化变得尤为重要。L2 缓存是位于 CPU 和主内存之间的高速缓存，用于减少 CPU 访问主内存的延迟。如果数据能够被有效地复用，那么就可以减少从主内存中加载数据的次数，从而减少等待时间，提高整体性能。在合并循环的情况下，由于 A(0, p) 的数据被更高效地复用，因此可以减少对主内存的访问，更多地利用缓存中的数据，从而加快计算速度。

## 6. MMult_1x4_5 到 MMult_1x4_6（性能提升）

这次的代码变更进一步优化了 `AddDot1x4` 函数，通过使用寄存器来存储中间结果，以减少内存访问并加速计算。这一技术利用了处理器的寄存器，它们是 CPU 内部的高速存储单元，访问速度远高于缓存或主内存。通过将频繁访问的变量放入寄存器中，可以显著减少数据访问的延迟，从而提高计算效率。

**原始版本**

在原始版本中，`AddDot1x4` 函数直接在循环中对矩阵 C 的元素进行更新，每次迭代都会计算 `A(0, p)` 与矩阵 B 相应列的乘积，并累加到矩阵 C 的对应位置。然而，这样做会导致频繁的内存访问，尤其是对 `A(0, p)` 的访问，每次迭代都需要从内存中读取。

**修改后的版本**

在修改后的版本中，`AddDot1x4` 函数首先将 `A(0, p)` 的值存储在寄存器 `a_0p_reg` 中，然后在循环中使用这个寄存器的值来进行计算，而不是每次都从内存中读取。同样，累积到矩阵 C 的四个元素的贡献也被存储在四个不同的寄存器 `c_00_reg`, `c_01_reg`, `c_02_reg`, 和 `c_03_reg` 中，直到循环结束才将最终结果写回到矩阵 C 中的相应位置。

**使用寄存器的优势**

- **减少内存访问**：通过将频繁使用的变量存储在寄存器中，可以减少对内存的访问次数，从而减少数据加载的延迟。
- **提高计算速度**：寄存器的访问速度远快于内存，因此使用寄存器可以显著加速计算。
- **减少缓存污染**：由于减少了对内存的访问，也减轻了对缓存的依赖，避免了缓存污染问题，即频繁访问的少量数据覆盖了缓存中其他有用数据的情况。

## 7. MMult_1x4_6 到 MMult_1x4_7（性能提升）

在这次代码变更中，`AddDot1x4` 函数被进一步优化，通过使用指针来追踪矩阵 B 中四个列的当前位置，以替代之前直接使用索引表达式 `B(p, 0)`, `B(p, 1)`, `B(p, 2)`, 和 `B(p, 3)` 的方式。这一改动旨在减少计算索引的开销，同时改善代码的可读性和维护性。

**原始版本**

在原始版本中，每次迭代中，`AddDot1x4` 函数使用索引表达式 `B(p, 0)`, `B(p, 1)`, `B(p, 2)`, 和 `B(p, 3)` 来访问矩阵 B 的相应列元素。虽然这种方式直观且容易理解，但是每次访问都需要计算索引，这可能会带来额外的计算开销。

**修改后的版本**

在修改后的版本中，`AddDot1x4` 函数使用四个指针 `bp0_pntr`, `bp1_pntr`, `bp2_pntr`, 和 `bp3_pntr` 来指向矩阵 B 的四个列的起始位置。在循环开始前，这些指针被初始化为矩阵 B 的四列的首元素地址。在循环中，使用 `*bp0_pntr++`, `*bp1_pntr++`, `*bp2_pntr++`, 和 `*bp3_pntr++` 来访问和更新矩阵 B 的元素，同时自动递增指针以指向下一位置。这样，每次迭代中就不需要重新计算索引，而是直接使用指针访问，减少了计算量。

**指针访问的优势**

- **减少索引计算**：使用指针访问可以避免每次迭代中重复计算相同的索引表达式，从而减少了计算开销。
- **改善代码可读性**：使用指针可以使得代码更加清晰，特别是当需要访问连续的内存位置时，指针的递增和访问操作直观明了。
- **提高性能**：在某些情况下，使用指针访问可以生成更高效的机器码，因为编译器可以直接处理指针的偏移，而不需要每次迭代时都进行复杂的索引计算。






