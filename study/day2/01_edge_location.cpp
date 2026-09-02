#include <BRep_Tool.hxx>
#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <Geom2d_Curve.hxx>
#include <Geom_Curve.hxx>

#include <TopAbs_Orientation.hxx>
#include <TopExp_Explorer.hxx>
#include <TopLoc_Location.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Shape.hxx>

#include <gp_Trsf.hxx>
#include <gp_Vec.hxx>
#include <gp_XYZ.hxx>

#include <iostream>
#include <string_view>

namespace
{
std::string_view orientationName(const TopAbs_Orientation orientation)
{
    switch (orientation)
    {
    case TopAbs_FORWARD: return "FORWARD";
    case TopAbs_REVERSED: return "REVERSED";
    case TopAbs_INTERNAL: return "INTERNAL";
    case TopAbs_EXTERNAL: return "EXTERNAL";
    }
    return "UNKNOWN";
}
}

int main()
{
    std::cout << "=== Edge: 3D Curve + P-Curve ===\n";

    const TopoDS_Shape cylinder = BRepPrimAPI_MakeCylinder(10.0, 30.0).Shape();

    TopExp_Explorer faceExp(cylinder, TopAbs_FACE);
    if (!faceExp.More())
    {
        std::cerr << "No face found.\n";
        return 1;
    }

    const TopoDS_Face face = TopoDS::Face(faceExp.Current());

    int edgeIndex = 0;
    for (TopExp_Explorer edgeExp(face, TopAbs_EDGE); edgeExp.More(); edgeExp.Next())
    {
        ++edgeIndex;
        const TopoDS_Edge edge = TopoDS::Edge(edgeExp.Current());

        Standard_Real first3d = 0.0;
        Standard_Real last3d = 0.0;
        TopLoc_Location curveLocation;
        const Handle(Geom_Curve) curve3d =
            BRep_Tool::Curve(edge, curveLocation, first3d, last3d);

        Standard_Real first2d = 0.0;
        Standard_Real last2d = 0.0;
        const Handle(Geom2d_Curve) pcurve =
            BRep_Tool::CurveOnSurface(edge, face, first2d, last2d);

        std::cout << "Edge #" << edgeIndex << '\n';
        std::cout << "  3D curve: " << (curve3d.IsNull() ? "missing" : "present")
                  << " range=[" << first3d << ", " << last3d << "]\n";
        std::cout << "  P-Curve : " << (pcurve.IsNull() ? "missing" : "present")
                  << " range=[" << first2d << ", " << last2d << "]\n";
    }

    std::cout << "\n=== TShape + Location + Orientation ===\n";

    const TopoDS_Shape box = BRepPrimAPI_MakeBox(10.0, 20.0, 30.0).Shape();

    gp_Trsf translation;
    translation.SetTranslation(gp_Vec(100.0, 0.0, 0.0));

    TopoDS_Shape moved = box;
    moved.Location(TopLoc_Location(translation));

    const gp_XYZ shift = moved.Location().Transformation().TranslationPart();

    std::cout << std::boolalpha;
    std::cout << "box.IsPartner(moved) = " << box.IsPartner(moved) << '\n';
    std::cout << "box.IsSame(moved)    = " << box.IsSame(moved) << '\n';
    std::cout << "box.IsEqual(moved)   = " << box.IsEqual(moved) << '\n';
    std::cout << "moved translation    = ("
              << shift.X() << ", " << shift.Y() << ", " << shift.Z() << ")\n";
    std::cout << "box orientation      = " << orientationName(box.Orientation()) << '\n';

    const TopoDS_Shape reversed = box.Reversed();
    std::cout << "reversed orientation = " << orientationName(reversed.Orientation()) << '\n';
    std::cout << "box.IsSame(reversed) = " << box.IsSame(reversed) << '\n';
    std::cout << "box.IsEqual(reversed)= " << box.IsEqual(reversed) << '\n';

    return 0;
}
