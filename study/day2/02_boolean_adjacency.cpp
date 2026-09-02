#include <BRepAlgoAPI_Cut.hxx>
#include <BRepAdaptor_Surface.hxx>
#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <GeomAbs_SurfaceType.hxx>

#include <TopExp.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Shape.hxx>
#include <TopTools_IndexedDataMapOfShapeListOfShape.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <TopTools_ListIteratorOfListOfShape.hxx>
#include <TopTools_ListOfShape.hxx>

#include <gp_Ax2.hxx>
#include <gp_Cylinder.hxx>
#include <gp_Dir.hxx>
#include <gp_Pnt.hxx>

#include <iostream>
#include <string_view>

namespace
{
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
    const TopoDS_Shape box = BRepPrimAPI_MakeBox(100.0, 80.0, 20.0).Shape();

    // Tool is made slightly longer than the box so that the test does not
    // accidentally introduce coincident top/bottom faces.
    const gp_Ax2 holeAxis(
        gp_Pnt(50.0, 40.0, -1.0),
        gp_Dir(0.0, 0.0, 1.0));
    const TopoDS_Shape tool =
        BRepPrimAPI_MakeCylinder(holeAxis, 10.0, 22.0).Shape();

    BRepAlgoAPI_Cut cut(box, tool);
    if (!cut.IsDone() || cut.Shape().IsNull())
    {
        std::cerr << "Boolean cut failed.\n";
        return 1;
    }

    const TopoDS_Shape shape = cut.Shape();

    TopTools_IndexedMapOfShape faces;
    TopExp::MapShapes(shape, TopAbs_FACE, faces);

    std::cout << "=== Faces after Box - Cylinder ===\n";
    for (int i = 1; i <= faces.Extent(); ++i)
    {
        const TopoDS_Face face = TopoDS::Face(faces(i));
        const BRepAdaptor_Surface surface(face, true);

        std::cout << "Face #" << i << " = "
                  << surfaceTypeName(surface.GetType());

        if (surface.GetType() == GeomAbs_Cylinder)
        {
            const gp_Cylinder cylinder = surface.Cylinder();
            std::cout << " radius=" << cylinder.Radius()
                      << " axisDir=("
                      << cylinder.Axis().Direction().X() << ", "
                      << cylinder.Axis().Direction().Y() << ", "
                      << cylinder.Axis().Direction().Z() << ")";
        }

        std::cout << '\n';
    }

    TopTools_IndexedDataMapOfShapeListOfShape edgeToFaces;
    TopExp::MapShapesAndAncestors(
        shape,
        TopAbs_EDGE,
        TopAbs_FACE,
        edgeToFaces);

    std::cout << "\n=== Edge -> Faces adjacency ===\n";
    for (int edgeId = 1; edgeId <= edgeToFaces.Extent(); ++edgeId)
    {
        std::cout << "Edge #" << edgeId << " ->";

        const TopTools_ListOfShape& ancestorFaces =
            edgeToFaces.FindFromIndex(edgeId);

        for (TopTools_ListIteratorOfListOfShape it(ancestorFaces);
             it.More();
             it.Next())
        {
            const int faceId = faces.FindIndex(it.Value());
            if (faceId > 0)
            {
                std::cout << " Face #" << faceId;
            }
        }

        std::cout << '\n';
    }

    return 0;
}
