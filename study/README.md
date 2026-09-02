# OCCT Day 1 / Day 2 实战教程

这份教程的目标不是两天背完 Open CASCADE Technology 的类名，而是建立一条以后可以继续扩展到 CAD/CAM 算法开发的主线。

截至 2026-09-02，OCCT 官方在线开发文档显示 8.0.1；官方 GitHub 已进入 8.0 系列。与此同时，vcpkg 的 opencascade port 和 Linux 发行版仓库可能提供更早的 7.x 版本。例如 Ubuntu 24.04 的官方仓库是 7.6 系列。Day 1 / Day 2 使用的 API 都是长期稳定的基础接口，因此本教程以 **C++17 + OCCT 7.6 及以上** 为兼容目标。

官方资料：

- OCCT Overview: https://occt3d.com/dev/doc/overview/html/
- Build OCCT: https://occt3d.com/dev/doc/overview/html/build_upgrade__building_occt.html
- Modeling Data: https://occt3d.com/dev/doc/overview/html/occt_user_guides__modeling_data.html
- Modeling Algorithms: https://occt3d.com/dev/doc/overview/html/occt_user_guides__modeling_algos.html
- Boolean Operations: https://occt3d.com/dev/doc/overview/html/specification__boolean_operations.html
- OCCT GitHub: https://github.com/Open-Cascade-SAS/OCCT
- vcpkg opencascade: https://vcpkg.io/en/package/opencascade.html

---

# 0. 两天后你应该达到什么程度？

完成本教程后，你应该能闭卷解释下面这条链：

~~~text
数学值对象
  gp_Pnt / gp_Vec / gp_Dir / gp_Ax2
        ↓
参数几何
  Geom_Curve / Geom_Surface
        ↓
B-Rep 拓扑
  Vertex → Edge → Wire → Face → Shell → Solid
        ↓
几何与拓扑关联
  Edge ↔ 3D Curve
  Edge ↔ P-Curve on Face
  Face ↔ Surface
        ↓
拓扑遍历与几何分析
  TopExp / BRep_Tool / BRepAdaptor
        ↓
建模算法
  Primitive / Boolean
        ↓
最终 B-Rep 反向分析
  Face descriptors
  Edge → Face adjacency
        ↓
后续加工特征识别
  Hole / Pocket / Slot / Step ...
~~~

最重要的是始终问自己两个问题：

1. 我现在操作的是 **数学几何 Geometry**，还是 **拓扑实体 Topology**？
2. 这个 API 是在 **创建模型**，还是在 **分析一个已经存在的模型**？

如果这两个问题始终清楚，OCCT 那些看似杂乱的类名会很快形成结构。

---

# 1. 仓库结构

当前 Day 1 / Day 2 只保留必要内容：

~~~text
study/
├─ README.md
├─ CMakeLists.txt
├─ vcpkg.json
├─ day1/
│  ├─ 01_hello_occ.cpp
│  └─ 02_brep_basics.cpp
├─ day2/
│  ├─ 01_edge_location.cpp
│  └─ 02_boolean_adjacency.cpp
└─ final/
   ├─ ShapeInspector.hpp
   ├─ ShapeInspector.cpp
   └─ main.cpp
~~~

构建完成后有五个可执行目标：

~~~text
day1_hello_occ
day1_brep_basics
day2_edge_location
day2_boolean_adjacency
occ_explorer
~~~

学习时不要一口气运行完。推荐顺序是：

~~~text
读对应章节
→ 预测代码输出
→ 自己敲核心 API
→ 编译运行
→ 修改参数
→ 解释变化
→ 闭卷复述
~~~

---

# 2. 环境搭建

## 2.1 Windows 推荐方案：Visual Studio 2022 + vcpkg

这是第一次搭 OCCT 环境时最省事的一条路线。

准备：

- Windows x64；
- Visual Studio 2022；
- 安装 “使用 C++ 的桌面开发”；
- Git；
- CMake；
- PowerShell。

安装 vcpkg：

~~~powershell
cd C:\dev
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat
~~~

当前 PowerShell 会话设置：

~~~powershell
$env:VCPKG_ROOT = "C:\dev\vcpkg"
~~~

本仓库已经有 study/vcpkg.json：

~~~json
{
  "name": "occ-explorer-study",
  "version-string": "0.1.0",
  "dependencies": [
    "opencascade"
  ]
}
~~~

因此使用 manifest mode 时，不需要额外执行 vcpkg install opencascade。

在 OCCExplorer 根目录配置：

~~~powershell
cmake -S study -B study\build-vcpkg -G "Visual Studio 17 2022" -A x64 -DCMAKE_TOOLCHAIN_FILE="$env:VCPKG_ROOT\scripts\buildsystems\vcpkg.cmake"
~~~

构建：

~~~powershell
cmake --build study\build-vcpkg --config Release --parallel
~~~

本工程把 exe 统一放到 build-vcpkg/bin，运行：

~~~powershell
.\study\build-vcpkg\bin\day1_hello_occ.exe
~~~

如果你使用的 CMake / generator 行为有所不同，可以搜索：

~~~powershell
Get-ChildItem study\build-vcpkg -Recurse day1_hello_occ.exe
~~~

### 为什么推荐 vcpkg？

重点不是因为 vcpkg “更高级”，而是它帮你减少三类环境问题：

1. include 路径；
2. lib 路径；
3. OCCT 第三方依赖。

你在学习 OCCT 内核概念时，不应该把第一天的大量时间耗在手工拼几十个库路径上。

---

## 2.2 Windows 方案二：官方预编译 OCCT SDK

如果你已经下载了官方 Windows binary package，也完全可以使用。

一般安装目录中会有：

~~~text
OCCT_INSTALL/
├─ cmake/
│  └─ OpenCASCADEConfig.cmake
├─ inc/ 或 include/
├─ win64/.../bin
└─ win64/.../lib
~~~

实际目录随版本和安装方式变化，因此不要照抄固定绝对路径。

正确思路是让 CMake 找：

~~~text
OpenCASCADEConfig.cmake
~~~

配置示例：

~~~powershell
cmake -S study -B study\build-sdk -G "Visual Studio 17 2022" -A x64 -DOpenCASCADE_DIR="C:\path\to\folder\containing\OpenCASCADEConfig.cmake"
~~~

然后：

~~~powershell
cmake --build study\build-sdk --config Release --parallel
~~~

### 常见问题：编译成功但 exe 找不到 DLL

如果出现类似：

~~~text
TKernel.dll was not found
TKBRep.dll was not found
~~~

这不是编译错误，而是 Windows runtime loader 找不到 DLL。

可以临时：

~~~powershell
$env:PATH = "C:\path\to\occt\bin;$env:PATH"
~~~

如果 OCCT package 自带 env.bat，优先运行它。

### ABI 一定要匹配

合理：

~~~text
OCCT = Visual Studio 2022 / x64
应用 = Visual Studio 2022 / x64
~~~

不合理：

~~~text
OCCT = MinGW
应用 = MSVC
~~~

以及：

~~~text
OCCT = x86
应用 = x64
~~~

这些问题和 OCCT API 本身没有关系，但会导致大量奇怪链接问题。

---

## 2.3 Linux 推荐方案：Ubuntu 系统包

以 Ubuntu 24.04 为例：

~~~bash
sudo apt update
sudo apt install -y build-essential cmake ninja-build libtbb-dev libocct-foundation-dev libocct-modeling-data-dev libocct-modeling-algorithms-dev
~~~

配置：

~~~bash
cmake -S study -B study/build -G Ninja -DCMAKE_BUILD_TYPE=Release
~~~

构建：

~~~bash
cmake --build study/build -j
~~~

运行：

~~~bash
./study/build/bin/day1_hello_occ
~~~

Ubuntu / Debian 会把 OpenCASCADEConfig.cmake 放到系统 CMake package 路径中，通常 find_package 可以直接找到。这里显式安装 `libtbb-dev` 是因为 Ubuntu 24.04 的 OCCT 7.6 CMake 导出目标会引用开发版 `libtbb.so` 链接名；只安装运行时 `libtbb12` 可能出现 `libtbb.so ... missing` 的构建错误。

如果你的发行版路径不同，可以查：

~~~bash
find /usr -name OpenCASCADEConfig.cmake 2>/dev/null
~~~

然后：

~~~bash
cmake -S study -B study/build -DOpenCASCADE_DIR=/path/to/cmake/package
~~~

### 为什么 Linux 仓库版本可能不是最新？

发行版追求稳定性，因此 OCCT 版本通常落后于 upstream。

对 Day 1 / Day 2 来说，这没有必要焦虑。你现在学习的是：

- gp；
- Geom；
- TopoDS；
- BRep；
- TopExp；
- BRepAdaptor；
- BRepPrimAPI；
- BRepAlgoAPI。

这些核心思想远比“当前装的是 7.6 还是 7.9”重要。

---

## 2.4 Linux 使用 vcpkg

如果希望 Windows/Linux 安装方式统一：

~~~bash
git clone https://github.com/microsoft/vcpkg.git ~/vcpkg
~/vcpkg/bootstrap-vcpkg.sh
export VCPKG_ROOT=$HOME/vcpkg
~~~

然后：

~~~bash
cmake -S study -B study/build-vcpkg -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake
cmake --build study/build-vcpkg -j
~~~

---

## 2.5 是否应该从源码编译 OCCT？

Day 1 不建议把它作为第一选择。

源码构建更适合：

- 后续要读 OCCT 内核源码；
- 想进 Debug 模式跟 Boolean；
- 需要特定 patch/version；
- 需要定制模块；
- 需要符号和源码级调试。

官方当前使用 CMake，OCCT 8.0 系列要求 C++17。

请始终把三个目录分开：

~~~text
source  = OCCT 源码
build   = CMake 中间产物
install = 你的应用真正消费的 SDK
~~~

示意：

~~~bash
git clone https://github.com/Open-Cascade-SAS/OCCT.git
cmake -S OCCT -B occt-build -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/opt/occt
cmake --build occt-build -j
cmake --install occt-build
~~~

实际是否启用 DRAW、Visualization、TBB、FreeType、VTK 等，以对应版本官方 Build 文档为准。

Day 1 / Day 2 只需要：

~~~text
Foundation Classes
Modeling Data
Modeling Algorithms
~~~

---

# 3. 先看懂本项目的 CMake

study/CMakeLists.txt 里面最重要的是：

~~~cmake
find_package(OpenCASCADE REQUIRED
    COMPONENTS
        FoundationClasses
        ModelingData
        ModelingAlgorithms
)
~~~

OCCT 的组织层次大致是：

~~~text
Module
  ↓
Toolkit
  ↓
Package
  ↓
Class
~~~

例如：

~~~text
Modeling Algorithms
  ↓
TKBO
  ↓
BRepAlgoAPI
  ↓
BRepAlgoAPI_Cut
~~~

所以你看到：

~~~cpp
#include <BRepAlgoAPI_Cut.hxx>
~~~

应该逐渐想到：

~~~text
这是 BRepAlgoAPI package
→ 它属于 TKBO toolkit
→ 它属于 Modeling Algorithms
~~~

本项目链接：

~~~text
TKernel
TKMath
TKG2d
TKG3d
TKGeomBase
TKBRep
TKTopAlgo
TKGeomAlgo
TKPrim
TKBO
~~~

现在不要背。

当你以后遇到 unresolved external / undefined reference 时，再去查一个类属于哪个 toolkit。这比“把 OpenCASCADE_LIBRARIES 全部链接进去”更有学习价值。

---

# Day 1：Foundation → Geometry → B-Rep → Traversal

---

# 4. 第一个程序：gp_Pnt / gp_Vec / gp_Dir

运行：

~~~bash
./study/build/bin/day1_hello_occ
~~~

Windows：

~~~powershell
.\study\build-vcpkg\bin\day1_hello_occ.exe
~~~

源码：

~~~text
study/day1/01_hello_occ.cpp
~~~

核心：

~~~cpp
gp_Pnt p(10.0, 20.0, 30.0);
gp_Vec v(3.0, 4.0, 0.0);
gp_Dir zDir(0.0, 0.0, 1.0);
~~~

## gp_Pnt

表示位置：

~~~text
P = (x, y, z)
~~~

## gp_Vec

表示带长度的向量：

~~~text
V = (x, y, z)
~~~

可以：

- Magnitude；
- Dot；
- Cross；
- Normalize；
- 加减。

例如：

~~~text
(3,4,0)
~~~

模长是 5。

## gp_Dir

表示方向。

它关心方向，而不是长度，可以把它理解成归一化方向对象。

最简记法：

~~~text
gp_Pnt = where
gp_Vec = how far + direction
gp_Dir = direction only
~~~

### 第一个动手修改

把：

~~~cpp
gp_Vec v(3.0, 4.0, 0.0);
~~~

改成：

~~~cpp
gp_Vec v(6.0, 8.0, 0.0);
~~~

先不要运行。

预测 Magnitude。

再运行验证。

---

# 5. gp_Ax1 / gp_Ax2：CAD 为什么到处都是坐标轴？

一个圆柱不能只由：

~~~text
radius = 10
~~~

定义。

还需要：

~~~text
圆柱在哪里？
轴朝哪里？
局部 X/Y 怎么定义？
~~~

所以大量解析几何 API 都会使用 gp_Ax1 / gp_Ax2。

可以先这样理解：

~~~text
gp_Ax1
= point + direction
~~~

而：

~~~text
gp_Ax2
= origin
+ Z direction
+ X direction
+ Y direction
~~~

后面 BRepPrimAPI_MakeCylinder 就会用到。

---

# 6. gp_* 和 Geom_* 为什么同时存在？

运行：

~~~bash
./study/build/bin/day1_brep_basics
~~~

这个程序第一部分创建：

~~~cpp
Handle(Geom_Circle) circle =
    new Geom_Circle(frame, 10.0);
~~~

你可能会问：

> 已经有 gp_Circ，为什么还有 Geom_Circle？

先记：

~~~text
gp_*
= 轻量数学值对象
= primitive geometry values

Geom_*
= 参数化几何对象
= 有继承体系
= 常通过 Handle 管理
= 可以 Value / D1 / D2
~~~

例如：

~~~cpp
circle->D1(u, point, tangent);
~~~

同时得到：

~~~text
C(u)
C'(u)
~~~

---

# 7. 什么是参数曲线？

三维参数曲线：

~~~text
C(u) = (x(u), y(u), z(u))
~~~

例如半径 R 的圆：

~~~text
x = R cos(u)
y = R sin(u)
z = 0
~~~

非常重要：

~~~text
u 不是 x/y/z 坐标
u 是曲线参数
~~~

程序取：

~~~text
u = pi / 4
~~~

然后算：

~~~text
C(u)
C'(u)
~~~

C'(u) 是切向信息。

以后：

- tangent；
- curvature；
- closest point；
- intersection；
- projection；

都会围绕这种统一参数几何接口展开。

---

# 8. 参数曲面

参数曲面写成：

~~~text
S(u,v) = (x(u,v), y(u,v), z(u,v))
~~~

例如圆柱：

~~~text
u = 绕轴旋转
v = 沿轴移动
~~~

一阶偏导：

~~~text
Su = dS/du
Sv = dS/dv
~~~

数学法向：

~~~text
N = Su × Sv
~~~

这里先埋一个 Day 2 的问题：

> Surface 的数学法向是否一定等于实体 Face 的外法向？

答案：不一定。

因为 TopoDS_Face 还有 Orientation。

---

# 9. Geometry 和 Topology 是 OCCT 的第一条分界线

Geometry 回答：

~~~text
它数学上是什么？
~~~

例如：

~~~text
Line
Circle
Ellipse
BSplineCurve
Plane
Cylinder
Cone
Sphere
BSplineSurface
~~~

Topology 回答：

~~~text
这些几何对象如何被裁剪、连接、组成 CAD 模型？
~~~

这两个概念一定要分开。

---

# 10. B-Rep 拓扑层级

OCCT 的基本层级：

~~~text
Vertex
  ↓
Edge
  ↓
Wire
  ↓
Face
  ↓
Shell
  ↓
Solid
  ↓
CompSolid
  ↓
Compound
~~~

## Vertex

零维拓扑实体，通常关联一个点。

## Edge

有限拓扑边。

关键：

~~~text
Edge != Curve
~~~

一个 Geom_Line 数学上可以无限延伸。

TopoDS_Edge 通常只使用它的一段参数范围。

## Wire

一组按拓扑连接关系组织的 Edge。

## Face

被边界裁剪的一块曲面。

关键：

~~~text
Face != Surface
~~~

## Shell

一组连接起来的 Face。

## Solid

由闭合 Shell 定义的三维区域。

---

# 11. Face 和 Surface 到底是什么关系？

这是最值得面试时讲清楚的问题之一。

假设有无限平面：

~~~text
Geom_Plane
~~~

Box 顶面只使用其中一个矩形区域。

所以：

~~~text
Geom_Surface
     │
     │ mathematical support
     ▼
TopoDS_Face
     │
     └─ Wire / Edge trim the surface
~~~

换句话说：

> Surface 是数学几何；Face 是在这块 Surface 上被拓扑边界裁剪出来的区域。

同样：

~~~text
Geom_Curve
    ↑
TopoDS_Edge uses a parameter interval of it
~~~

---

# 12. 为什么叫 Boundary Representation？

一个 Solid 并不是“内部装满点”。

而是通过边界描述：

~~~text
Solid
  ↓
Shell
  ↓
Face
  ↓
Wire
  ↓
Edge
  ↓
Vertex
~~~

Face 背后有 Surface。

Edge 背后有 Curve。

于是：

~~~text
拓扑结构
+
精确几何
=
B-Rep CAD model
~~~

---

# 13. TopExp：怎样遍历模型？

常见：

~~~cpp
for (TopExp_Explorer exp(shape, TopAbs_FACE);
     exp.More();
     exp.Next())
{
    TopoDS_Face face =
        TopoDS::Face(exp.Current());
}
~~~

意思：

~~~text
从 shape 开始
→ 找所有 TopAbs_FACE
→ Current 得到 TopoDS_Shape
→ TopoDS::Face 做类型化访问
~~~

同理可以遍历：

~~~text
TopAbs_EDGE
TopAbs_VERTEX
TopAbs_SOLID
~~~

---

# 14. TopExp::MapShapes 与 IndexedMap

如果只是逐个处理：

~~~text
TopExp_Explorer
~~~

很方便。

如果你需要稳定编号：

~~~text
Face #1
Face #2
Face #3
~~~

更适合：

~~~cpp
TopTools_IndexedMapOfShape faces;
TopExp::MapShapes(shape, TopAbs_FACE, faces);
~~~

然后：

~~~cpp
for (int i = 1; i <= faces.Extent(); ++i)
{
    TopoDS_Face face =
        TopoDS::Face(faces(i));
}
~~~

注意：

> OCCT 很多 collection 是 1-based。

长期写 STL 的人非常容易下意识从 0 开始。

---

# 15. BRepAdaptor_Surface：分析 Face 的几何类型

day1_brep_basics 的最后一部分：

~~~cpp
BRepAdaptor_Surface surface(face, true);

GeomAbs_SurfaceType type =
    surface.GetType();
~~~

可能得到：

~~~text
GeomAbs_Plane
GeomAbs_Cylinder
GeomAbs_Cone
GeomAbs_Sphere
GeomAbs_Torus
GeomAbs_BSplineSurface
...
~~~

如果是 Cylinder：

~~~cpp
gp_Cylinder cylinder =
    surface.Cylinder();
~~~

可以拿：

~~~text
radius
axis location
axis direction
~~~

这一步对加工特征识别极其重要。

因为以后我们看到一个 Face，不再只知道：

~~~text
Face #17
~~~

而是：

~~~text
Face #17
surface = Cylinder
radius = 5
axis = ...
~~~

这就开始具有可分析的几何语义。

---

# 16. Day 1 必做实验

## 实验 A：改 Box 尺寸

把：

~~~cpp
BRepPrimAPI_MakeBox(100, 80, 20)
~~~

改成：

~~~cpp
BRepPrimAPI_MakeBox(10, 10, 10)
~~~

预测：

- Face 数会变吗？
- Edge 数会变吗？
- Vertex 数会变吗？

你应该逐渐发现：

> 几何尺寸变化不一定改变拓扑结构。

## 实验 B：改变 Cylinder 半径

把 10 改 5。

问：

~~~text
Face 数是否变化？
圆柱 Face 的 radius 是否变化？
~~~

## 实验 C：把 Cylinder 换 Cone

查 BRepPrimAPI_MakeCone。

然后观察：

~~~text
GeomAbs_Cone
~~~

---

# 17. Day 1 闭卷面试题

### Q1 gp_Pnt、gp_Vec、gp_Dir 区别？

位置、带模长向量、单位方向。

### Q2 gp_Circ 与 Geom_Circle 区别？

轻量数学值对象 vs 参数化几何对象 / 继承体系 / Handle。

### Q3 Geometry 与 Topology 区别？

数学描述 vs 裁剪、连接与组成关系。

### Q4 Face 与 Surface 区别？

Face 是拓扑实体，是被 Wire/Edge 边界裁剪的一块 Surface。

### Q5 Edge 与 Curve 区别？

Edge 是有限拓扑边；Curve 是底层数学曲线。

### Q6 什么是 B-Rep？

不要只答“边界表示法”。

应该答：

> 用 Vertex/Edge/Wire/Face/Shell 等拓扑实体组织底层曲线和曲面，通过物体边界描述三维实体。

### Q7 怎样遍历所有 Face？

TopExp_Explorer 或 TopExp::MapShapes。

### Q8 怎样判断一个 Face 是不是圆柱面？

BRepAdaptor_Surface + GetType，判断 GeomAbs_Cylinder，再调用 Cylinder 取得参数。

---

# Day 2：P-Curve → TShape/Location/Orientation → Boolean → Adjacency

---

# 18. Edge 为什么不仅有 3D Curve？

运行：

~~~bash
./study/build/bin/day2_edge_location
~~~

程序第一部分：

~~~cpp
BRep_Tool::Curve(...)
~~~

取得 Edge 的三维 Curve。

但又调用：

~~~cpp
BRep_Tool::CurveOnSurface(
    edge,
    face,
    first,
    last);
~~~

它取得的是：

~~~text
P-Curve
Curve on Surface
~~~

为什么？

---

# 19. P-Curve 的数学意义

Face 底层曲面：

~~~text
S(u,v)
~~~

Edge 在三维中：

~~~text
C(t)
~~~

同一条 Edge 在 Surface 参数空间里可以表示为：

~~~text
P(t) = (u(t), v(t))
~~~

满足：

~~~text
C(t) ≈ S(u(t), v(t))
~~~

因此一个 Edge 可以同时具有：

~~~text
3D Curve
+
2D P-Curve on Face A
+
2D P-Curve on Face B
~~~

这件事很重要。

因为 Face 的边界裁剪，本质上必须在 Surface 的 UV 参数域里正确表达。

---

# 20. 为什么“3D 看起来对”还不够？

你在 CAD kernel 里会逐渐遇到这种情况：

~~~text
三维视觉上边界好像闭合
~~~

但是：

~~~text
UV parameter domain 中的 Wire 有 gap
P-Curve 不一致
参数范围不同
seam 表示有问题
~~~

于是某些后续算法仍会失败。

所以：

> CAD 内核不能只看最终三维图形，也要看几何与参数域中的拓扑一致性。

这就是后面 SameParameter、Shape Healing、Seam 等知识的入口。

---

# 21. Seam Edge：先建立概念

以圆柱参数面为例：

~~~text
u = 0
~~~

和：

~~~text
u = 2π
~~~

在三维空间实际上对应同一条母线。

但在 UV 参数域中，它们是周期参数域的两侧边界。

于是周期曲面会出现 seam 的概念。

Day 2 不要求你修 seam。

只要求记住：

> 周期曲面在三维空间和 UV 参数空间里的“闭合”不是同一个层面的问题。

---

# 22. TopoDS_Shape 的核心：TShape + Location + Orientation

day2_edge_location 的第二部分专门演示这个设计。

概念上：

~~~text
TopoDS_Shape
├─ TShape
├─ Location
└─ Orientation
~~~

这也是 OCCT 面试非常经典的理解题。

---

# 23. TShape 是什么？

可以先把它理解为：

~~~text
底层拓扑实体本体
~~~

TopoDS_Shape 本身更像：

~~~text
一个引用/包装
+
位置
+
方向
~~~

因此复制一个 TopoDS_Shape 并不是深拷贝完整 B-Rep。

---

# 24. IsPartner / IsSame / IsEqual

示例程序故意打印：

~~~cpp
box.IsPartner(moved)
box.IsSame(moved)
box.IsEqual(moved)
~~~

三者区别非常值得记。

## IsPartner

只要求：

~~~text
same TShape
~~~

Location 和 Orientation 可以不同。

## IsSame

要求：

~~~text
same TShape
+
same Location
~~~

Orientation 可以不同。

## IsEqual

要求：

~~~text
same TShape
+
same Location
+
same Orientation
~~~

所以你可以把它记成逐级增加条件：

~~~text
Partner
  + Location
= Same

Same
  + Orientation
= Equal
~~~

---

# 25. Location

示例：

~~~cpp
gp_Trsf translation;
translation.SetTranslation(
    gp_Vec(100, 0, 0));

TopoDS_Shape moved = box;
moved.Location(
    TopLoc_Location(translation));
~~~

box 与 moved 可以共享同一 TShape，但是 moved 有不同 Location。

这种结构非常适合：

~~~text
实例复用
装配
重复零件
~~~

因为不需要为每个实例深拷贝底层拓扑和几何。

---

# 26. Orientation

常见：

~~~text
TopAbs_FORWARD
TopAbs_REVERSED
TopAbs_INTERNAL
TopAbs_EXTERNAL
~~~

最常见：

~~~text
FORWARD
REVERSED
~~~

假设 Surface 数学法向：

~~~text
N_surface = Su × Sv
~~~

如果 Face 在拓扑中是 REVERSED，则实体语义下的 Face 方向通常需要反过来处理。

因此：

~~~text
Surface Normal
!= always
Face outward normal
~~~

为什么加工特征识别需要在意？

因为以后要判断：

~~~text
材料在哪一侧
这是凹还是凸
孔朝哪里
刀具从哪里接近
~~~

仅仅拿 Surface 自己的数学法向是不够的。

---

# 27. Primitive：为什么学习算法时要自己制造测试模型？

Day 2 使用：

~~~text
BRepPrimAPI_MakeBox
BRepPrimAPI_MakeCylinder
~~~

目的不只是“学会造 Box”。

真正价值是：

> 你可以创建几何和拓扑都已知的测试输入。

例如：

~~~cpp
TopoDS_Shape box =
    BRepPrimAPI_MakeBox(
        100,
        80,
        20).Shape();
~~~

再创建 Cylinder。

我们故意让圆柱：

~~~text
start z = -1
height = 22
~~~

而 Box：

~~~text
z = 0 ... 20
~~~

这样 Cylinder 完整穿过 Box。

为什么不直接：

~~~text
start z = 0
height = 20
~~~

因为后者会让 tool 的顶底 Face 与 Box 顶底面完全重合，给第一个 Boolean 实验额外引入 coincident geometry。

算法开发中一个好习惯是：

> 先用普通位置关系验证主流程，再主动制造 tangent / coincident / tiny feature 等边界情况。

---

# 28. Boolean Cut

运行：

~~~bash
./study/build/bin/day2_boolean_adjacency
~~~

核心：

~~~cpp
BRepAlgoAPI_Cut cut(
    box,
    cylinder);

if (!cut.IsDone())
{
    ...
}

TopoDS_Shape result =
    cut.Shape();
~~~

数学上：

~~~text
result = box - cylinder
~~~

但是如果面试官问：

> Boolean 大概怎么做？

不能只答 “A 减 B”。

---

# 29. Boolean 内部可以怎么理解？

建立这个高层模型：

~~~text
输入 Shape A / B
      ↓
interference / intersection
      ↓
计算交点、交线
      ↓
split Edge / Face
      ↓
classify fragments
      ↓
根据 Cut/Fuse/Common 选择片段
      ↓
rebuild topology
      ↓
输出新 Shape
~~~

这解释了为什么 Boolean 后：

~~~text
原来的 Face 可能被切开
原来的 Edge 可能变成多段
会产生新 Face
会产生新 Edge
拓扑编号完全可能变化
~~~

所以 CAD 算法中不能轻易假设：

> Boolean 前 Face #3 到 Boolean 后还一定是 Face #3。

---

# 30. 为什么 Box - Cylinder 是极好的学习例子？

因为建模历史告诉我们：

~~~text
这里有一个 Through Hole
~~~

但做完 Boolean 后，我们故意“忘掉”建模历史，只保留：

~~~text
TopoDS_Shape result
~~~

然后问：

> 能不能只看最终 B-Rep，重新发现圆柱侧壁、半径、轴线及其邻接关系？

这就是从：

~~~text
CAD modeling
~~~

走向：

~~~text
CAD feature analysis
~~~

的关键一步。

---

# 31. Face descriptor

Day 2 最终项目对每个 Face 提取：

~~~text
id
surface type
orientation
area
~~~

如果是 Cylinder：

~~~text
radius
axis origin
axis direction
~~~

这已经构成一个最小 Face descriptor。

以后可以继续扩展：

~~~text
centroid
normal
UV bounds
outer wire
inner wires
curvature
surface periodicity
tolerance
~~~

但 Day 2 不需要一次学完。

---

# 32. 为什么一个 Cylinder 不能直接认定是 Hole？

这是加工特征识别非常重要的一点。

下面都可能包含圆柱面：

~~~text
贯穿孔
盲孔
圆柱凸台
轴
外圆柱壁
圆柱槽
~~~

所以：

~~~text
if surface == Cylinder
    return Hole
~~~

一定是不够的。

更合理的是：

~~~text
surface type
+
geometry parameters
+
face adjacency
+
shared edge type
+
convex / concave
+
material side
+
feature pattern
~~~

共同决定高层制造特征。

Day 2 先做到 adjacency。

---

# 33. Edge → Face 邻接

核心：

~~~cpp
TopTools_IndexedDataMapOfShapeListOfShape edgeToFaces;

TopExp::MapShapesAndAncestors(
    shape,
    TopAbs_EDGE,
    TopAbs_FACE,
    edgeToFaces);
~~~

得到：

~~~text
Edge #1 → Face #2, Face #5
Edge #2 → Face #1, Face #4
...
~~~

为什么这很重要？

因为在典型 manifold B-Rep 中：

> 两个相邻 Face 往往通过共享 Edge 建立边界关系。

于是可以进一步转换为：

~~~text
Face Adjacency Graph
~~~

节点：

~~~text
Face
~~~

图上的连接：

~~~text
two Faces share an Edge
~~~

节点可以挂属性：

~~~text
Plane / Cylinder
area
radius
axis
normal
...
~~~

边可以挂属性：

~~~text
shared edge
angle
convex / concave
continuity
...
~~~

这就是经典 Attributed Adjacency Graph 思路的雏形。

---

# 34. 最终项目 occ_explorer

现在运行：

~~~bash
./study/build/bin/occ_explorer
~~~

Windows：

~~~powershell
.\study\build-vcpkg\bin\occ_explorer.exe
~~~

流程：

~~~text
1. 创建 100 × 80 × 20 Box
2. 创建 radius=10 的贯穿 Cylinder
3. Boolean Cut
4. 得到最终 TopoDS_Shape
5. 统计 Solid/Shell/Face/Wire/Edge/Vertex
6. 遍历 Face
7. 提取 surface type
8. 计算 Face area
9. 对 Cylinder 提取 radius / axis
10. 构建 Edge → Faces adjacency
11. 输出 ShapeReport
~~~

你会得到类似：

~~~text
=== OCCExplorer Shape Report ===
Topology
  Solids   : ...
  Shells   : ...
  Faces    : ...
  Wires    : ...
  Edges    : ...
  Vertices : ...

Faces
  Face #1 type=Plane ...
  ...
  Face #? type=Cylinder ...
    cylinder.radius=10.000000
    cylinder.axis.origin=(...)
    cylinder.axis.direction=(0, 0, 1)

Edge -> Faces adjacency
  Edge #1 -> Face #...
  ...
~~~

不要死记具体 Face/Edge 编号。

编号取决于 B-Rep 构造结果和遍历顺序。

你真正应该关注的是：

~~~text
结构
+
几何类型
+
参数
+
邻接
~~~

---

# 35. 为什么最终项目拆出 ShapeInspector？

现在结构：

~~~text
Geometry creation
      ↓
TopoDS_Shape
      ↓
ShapeInspector
      ↓
ShapeReport
      ↓
console output
~~~

ShapeInspector 根本不关心 Shape 从哪里来。

所以以后可以替换输入：

~~~text
STEP
IGES
其他 CAD SDK
数据库
单元测试
~~~

而分析层不用推倒重写。

这是比“把所有逻辑塞在 main.cpp”更接近真实 CAD 工程的结构。

---

# 36. Day 2 必做实验

## 实验 A：孔半径 10 → 5

修改 final/main.cpp。

预测：

~~~text
Cylinder radius 改变
Cylinder area 改变
拓扑结构大体类似
~~~

运行验证。

## 实验 B：改变孔中心

从：

~~~text
(50,40)
~~~

改成：

~~~text
(20,20)
~~~

问：

~~~text
Face 类型改变了吗？
Cylinder axis origin 改了吗？
Adjacency 是否仍然类似？
~~~

## 实验 C：做盲孔

不要再让 tool 完整穿透。

例如从顶面向下只切一部分。

观察：

~~~text
Face 数
Plane 数
Cylinder Face
孔底 Face
邻接关系
~~~

你会直观看见：

> Through Hole 和 Blind Hole 的 B-Rep pattern 不同。

## 实验 D：让孔碰到 Box 侧边

把圆柱中心移动到边缘。

观察拓扑怎样显著改变。

这说明：

> 同样叫 “Cylinder Cut”，空间关系不同，最终 B-Rep 可以完全不同。

这正是 feature recognition 难的原因之一。

---

# 37. Day 2 闭卷面试题

### Q1 为什么 Edge 需要 P-Curve？

因为 Face 是参数曲面上的裁剪区域，Edge 在 Face 的 UV 参数空间中需要二维 Curve-on-Surface 表示。

### Q2 TopoDS_Shape 可以怎样理解？

共享 TShape + Location + Orientation。

### Q3 IsPartner / IsSame / IsEqual 区别？

~~~text
Partner = same TShape
Same = same TShape + Location
Equal = same TShape + Location + Orientation
~~~

### Q4 Location 有什么价值？

可以在共享底层拓扑/几何的基础上表达不同实例位置，避免无意义深拷贝。

### Q5 Orientation 为什么重要？

Surface 参数方向不等于实体 Face 方向；法向、材料侧、凹凸和 CAM 语义都需要考虑 Face Orientation。

### Q6 Boolean Cut 大致怎么做？

回答到：

~~~text
intersection
→ split
→ classify
→ rebuild
~~~

即可形成不错的高层认识。

### Q7 Boolean 为什么可能改变 Face/Edge？

因为交线会切分原有拓扑，再根据布尔操作重新构造结果。

### Q8 怎么建立 Face adjacency？

通过共享 Edge。

使用：

~~~cpp
TopExp::MapShapesAndAncestors(
    shape,
    TopAbs_EDGE,
    TopAbs_FACE,
    map);
~~~

### Q9 为什么 Cylinder 不等于 Hole？

因为 Cylinder 只是局部曲面几何类型。孔是多个 Face/Edge 及材料侧关系形成的高层制造语义。

---

# 38. 两天完成后的知识图

现在你应该可以解释：

~~~text
                    TopoDS_Shape
                    /    |     \
               TShape Location Orientation
                  │
                  ▼
                 B-Rep
                  │
        ┌─────────┴─────────┐
        ▼                   ▼
      Face                 Edge
        │                 /    \
        ▼                ▼      ▼
 Geom_Surface        3D Curve   P-Curve
                                 │
                                 ▼
                           Face UV domain

        B-Rep Modeling Algorithms
                  │
           Primitive / Boolean
                  │
                  ▼
            Result Shape
                  │
                  ▼
           ShapeInspector
            /          \
     Face geometry   Edge→Face graph
~~~

如果这张图你能够逐项讲清楚，Day 1 / Day 2 就完成了。

---

# 39. 这两天暂时不要陷进去的内容

先不深入：

~~~text
完整 NURBS 数值算法实现
OCCT Boolean 全部源码
STEP/XDE
Shape Healing
BRepMesh
AIS/V3d Viewer
OCAF
Fillet 内部算法
完整 Hole/Pocket/Slot rule engine
~~~

不是因为它们不重要，而是学习顺序问题。

当前先把：

~~~text
Geometry
Topology
B-Rep
Traversal
Adaptor
P-Curve
Location
Orientation
Boolean
Adjacency
~~~

彻底串起来。

---

# 40. 下一阶段应该是什么？

Day 3 很自然地会接：

~~~text
STEPControl_Reader
→ 导入真实工业模型
→ BRepCheck_Analyzer
→ tolerance
→ Shape Healing
→ 真实模型 Face/Edge 分析
~~~

再之后：

~~~text
Face normal
→ shared Edge geometry
→ dihedral angle
→ convex / concave
→ coaxial / coplanar
→ Hole / Pocket / Slot recognition
~~~

但在开始这些内容以前，请先把本仓库五个可执行程序全部亲手改过至少一次。

---

# 41. 最后一个学习要求：不要只“看懂”

对每个知识点执行：

~~~text
看代码
→ 预测
→ 运行
→ 修改
→ 制造反例
→ 闭卷解释
~~~

例如学习 BRepAdaptor_Surface：

不要只做到：

> 我知道 GetType 可以判断 Cylinder。

而要做到：

1. 自己创建 Box；
2. 自己创建 Cylinder；
3. 遍历所有 Face；
4. 预测 Plane/Cylinder 数量；
5. 输出 radius/axis；
6. 换 Cone；
7. Boolean 后再观察；
8. 解释为什么 Cylinder Face 不能直接叫 Hole。

到这一步，它才真正成为你面试时能够使用的知识。
