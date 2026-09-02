# OCCExplorer

一个面向 **Open CASCADE Technology (OCCT)** 入门与 CAD 算法学习的实战仓库。

当前阶段覆盖 **Day 1 / Day 2**：从 Windows / Linux 安装 OCCT、CMake 跑通第一个 C++ 程序开始，逐步学习 `gp` / `Geom` / `TopoDS` / B-Rep / `TopExp` / `BRepAdaptor`，再进入 P-Curve、Location、Orientation、Primitive 建模、Boolean，以及最终的 Face 几何分析和 Edge→Face 邻接分析。

教程入口：[`study/README.md`](study/README.md)

Day 2 完成后，你应该可以对一个 `TopoDS_Shape` 输出拓扑数量、Face 曲面类型、圆柱半径/轴线及 Edge→Face 邻接关系，并能解释这些信息为什么是后续孔/槽/Pocket 等加工特征识别的基础。
