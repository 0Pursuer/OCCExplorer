#include <BRepAdaptor_Surface.hxx>
#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <GeomAbs_SurfaceType.hxx>
#include <Geom_Circle.hxx>

#include <TopAbs_ShapeEnum.hxx>
#include <TopExp.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Shape.hxx>
#include <TopTools_IndexedMapOfShape.hxx>

#include <gp_Ax1.hxx>
#include <gp_Ax2.hxx>
#include <gp_Cylinder.hxx>
#include <gp_Dir.hxx>
#include <gp_Pnt.hxx>
#include <gp_Vec.hxx>

#include <cmath>
#include <iomanip>
#include <iostream>
#include <string_view>

namespace
{
int countSubShapes(const TopoDS_Shape& shape, const TopAbs_ShapeEnum type)
{
    TopTools_IndexedMapOfShape map;
    TopExp::MapShapes(shape, type, map);
    return map.Extent();
}

std::string_view surfaceTypeName(const GeomAbs_SurfaceType type)
{
    switch (type)
    {
    case GeomAbs_Plane: return "Plane";
    case GeomAbs_Cylinder: return "Cylinder";
    case GeomAbs_Cone: return "Cone";
    case GeomAbs_Sphere: return "Sphere";
    case GeomAbs_Torus: return "Torus";
    case GeomAbs_BezierSurface: return "BezierSurface";
    case GeomAbs_BSplineSurface: return "BSplineSurface";
    case GeomAbs_SurfaceOfRevolution: return "SurfaceOfRevolution";
    case GeomAbs_SurfaceOfExtrusion: return "SurfaceOfExtrusion";
    case GeomAbs_OffsetSurface: return "OffsetSurface";
    default: return "OtherSurface";
    }
}
}

int main()
{
    // Part A: gp_* and Geom_*
    const gp_Ax2 frame(gp_Pnt(0.0, 0.0, 0.0), gp_Dir(0.0, 0.0, 1.0));
    Handle(Geom_Circle) circle = new Geom_Circle(frame, 10.0);

    constexpr double pi = 3.14159265358979323846;
    gp_Pnt point;
    gp_Vec tangent;
    circle->D1(pi / 4.0, point, tangent);

    std::cout << std::fixed << std::setprecision(6);
    std::cout << "=== Parameter geometry ===\n";
    std::cout << "Circle radius = " << circle->Radius() << '\n';
    std::cout << "C(pi/4) = (" << point.X() << ", " << point.Y() << ", " << point.Z() << ")\n";
    std::cout << "C'(pi/4) = (" << tangent.X() << ", " << tangent.Y() << ", " << tangent.Z() << ")\n\n";

    // Part B: topology counts
    const TopoDS_Shape box = BRepPrimAPI_MakeBox(100.0, 80.0, 20.0).Shape();

    std::cout << "=== Box B-Rep topology ===\n";
    std::cout << "Solids   : " << countSubShapes(box, TopAbs_SOLID) << '\n';
    std::cout << "Shells   : " << countSubShapes(box, TopAbs_SHELL) << '\n';
    std::cout << "Faces    : " << countSubShapes(box, TopAbs_FACE) << '\n';
    std::cout << "Wires    : " << countSubShapes(box, TopAbs_WIRE) << '\n';
    std::cout << "Edges    : " << countSubShapes(box, TopAbs_EDGE) << '\n';
    std::cout << "Vertices : " << countSubShapes(box, TopAbs_VERTEX) << "\n\n";

    // Part C: inspect the supporting surface of every Face.
    const TopoDS_Shape cylinder = BRepPrimAPI_MakeCylinder(10.0, 30.0).Shape();

    TopTools_IndexedMapOfShape faces;
    TopExp::MapShapes(cylinder, TopAbs_FACE, faces);

    std::cout << "=== Cylinder Face surfaces ===\n";
    for (int i = 1; i <= faces.Extent(); ++i)
    {
        const TopoDS_Face face = TopoDS::Face(faces(i));
        const BRepAdaptor_Surface surface(face, true);
        const GeomAbs_SurfaceType type = surface.GetType();

        std::cout << "Face #" << i << " = " << surfaceTypeName(type);

        if (type == GeomAbs_Cylinder)
        {
            const gp_Cylinder c = surface.Cylinder();
            const gp_Ax1 axis = c.Axis();
            std::cout << " radius=" << c.Radius()
                      << " axisDir=("
                      << axis.Direction().X() << ", "
                      << axis.Direction().Y() << ", "
                      << axis.Direction().Z() << ")";
        }

        std::cout << '\n';
    }

    return 0;
}
