#pragma once

#include <TopoDS_Shape.hxx>

#include <array>
#include <optional>
#include <ostream>
#include <string>
#include <vector>

namespace occexplorer
{

struct TopologyCounts
{
    int solids = 0;
    int shells = 0;
    int faces = 0;
    int wires = 0;
    int edges = 0;
    int vertices = 0;
};

struct CylinderInfo
{
    double radius = 0.0;
    std::array<double, 3> axisOrigin{};
    std::array<double, 3> axisDirection{};
};

struct FaceInfo
{
    int id = 0;
    std::string surfaceType;
    std::string orientation;
    double area = 0.0;
    std::optional<CylinderInfo> cylinder;
};

struct EdgeAdjacency
{
    int edgeId = 0;
    std::vector<int> faceIds;
};

struct ShapeReport
{
    TopologyCounts topology;
    std::vector<FaceInfo> faces;
    std::vector<EdgeAdjacency> adjacency;
};

class ShapeInspector
{
public:
    static ShapeReport Analyze(const TopoDS_Shape& shape);
    static void Print(const ShapeReport& report, std::ostream& os);
};

} // namespace occexplorer
