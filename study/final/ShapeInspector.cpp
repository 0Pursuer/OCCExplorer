#include "ShapeInspector.hpp"

#include <BRepAdaptor_Surface.hxx>
#include <BRepGProp.hxx>
#include <GProp_GProps.hxx>
#include <GeomAbs_SurfaceType.hxx>

#include <TopAbs_Orientation.hxx>
#include <TopAbs_ShapeEnum.hxx>
#include <TopExp.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Face.hxx>
#include <TopTools_IndexedDataMapOfShapeListOfShape.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <TopTools_ListIteratorOfListOfShape.hxx>
#include <TopTools_ListOfShape.hxx>

#include <gp_Ax1.hxx>
#include <gp_Cylinder.hxx>
#include <gp_Dir.hxx>
#include <gp_Pnt.hxx>

#include <iomanip>
#include <stdexcept>
#include <utility>

namespace occexplorer
{
namespace
{

int countSubShapes(const TopoDS_Shape& shape, const TopAbs_ShapeEnum type)
{
    TopTools_IndexedMapOfShape map;
    TopExp::MapShapes(shape, type, map);
    return map.Extent();
}

std::string surfaceTypeName(const GeomAbs_SurfaceType type)
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

std::string orientationName(const TopAbs_Orientation orientation)
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

FaceInfo analyzeFace(const TopoDS_Face& face, const int id)
{
    FaceInfo info;
    info.id = id;
    info.orientation = orientationName(face.Orientation());

    GProp_GProps props;
    BRepGProp::SurfaceProperties(face, props);
    info.area = props.Mass();

    const BRepAdaptor_Surface surface(face, true);
    const GeomAbs_SurfaceType type = surface.GetType();
    info.surfaceType = surfaceTypeName(type);

    if (type == GeomAbs_Cylinder)
    {
        const gp_Cylinder cylinder = surface.Cylinder();
        const gp_Ax1 axis = cylinder.Axis();
        const gp_Pnt origin = axis.Location();
        const gp_Dir direction = axis.Direction();

        CylinderInfo cylinderInfo;
        cylinderInfo.radius = cylinder.Radius();
        cylinderInfo.axisOrigin = {
            origin.X(), origin.Y(), origin.Z()
        };
        cylinderInfo.axisDirection = {
            direction.X(), direction.Y(), direction.Z()
        };
        info.cylinder = cylinderInfo;
    }

    return info;
}

} // namespace

ShapeReport ShapeInspector::Analyze(const TopoDS_Shape& shape)
{
    if (shape.IsNull())
    {
        throw std::invalid_argument(
            "ShapeInspector::Analyze received a null shape");
    }

    ShapeReport report;

    report.topology.solids = countSubShapes(shape, TopAbs_SOLID);
    report.topology.shells = countSubShapes(shape, TopAbs_SHELL);
    report.topology.faces = countSubShapes(shape, TopAbs_FACE);
    report.topology.wires = countSubShapes(shape, TopAbs_WIRE);
    report.topology.edges = countSubShapes(shape, TopAbs_EDGE);
    report.topology.vertices = countSubShapes(shape, TopAbs_VERTEX);

    TopTools_IndexedMapOfShape faces;
    TopTools_IndexedMapOfShape edges;
    TopExp::MapShapes(shape, TopAbs_FACE, faces);
    TopExp::MapShapes(shape, TopAbs_EDGE, edges);

    report.faces.reserve(static_cast<std::size_t>(faces.Extent()));
    for (int i = 1; i <= faces.Extent(); ++i)
    {
        report.faces.push_back(
            analyzeFace(TopoDS::Face(faces(i)), i));
    }

    TopTools_IndexedDataMapOfShapeListOfShape edgeToFaces;
    TopExp::MapShapesAndAncestors(
        shape,
        TopAbs_EDGE,
        TopAbs_FACE,
        edgeToFaces);

    report.adjacency.reserve(
        static_cast<std::size_t>(edgeToFaces.Extent()));

    for (int mapIndex = 1;
         mapIndex <= edgeToFaces.Extent();
         ++mapIndex)
    {
        EdgeAdjacency adjacency;
        adjacency.edgeId =
            edges.FindIndex(edgeToFaces.FindKey(mapIndex));

        const TopTools_ListOfShape& ancestorFaces =
            edgeToFaces.FindFromIndex(mapIndex);

        for (TopTools_ListIteratorOfListOfShape it(ancestorFaces);
             it.More();
             it.Next())
        {
            const int faceId = faces.FindIndex(it.Value());
            if (faceId > 0)
            {
                adjacency.faceIds.push_back(faceId);
            }
        }

        report.adjacency.push_back(std::move(adjacency));
    }

    return report;
}

void ShapeInspector::Print(
    const ShapeReport& report,
    std::ostream& os)
{
    os << "=== OCCExplorer Shape Report ===\n";

    os << "Topology\n";
    os << "  Solids   : " << report.topology.solids << '\n';
    os << "  Shells   : " << report.topology.shells << '\n';
    os << "  Faces    : " << report.topology.faces << '\n';
    os << "  Wires    : " << report.topology.wires << '\n';
    os << "  Edges    : " << report.topology.edges << '\n';
    os << "  Vertices : " << report.topology.vertices << "\n\n";

    os << std::fixed << std::setprecision(6);
    os << "Faces\n";

    for (const FaceInfo& face : report.faces)
    {
        os << "  Face #" << face.id
           << " type=" << face.surfaceType
           << " orientation=" << face.orientation
           << " area=" << face.area << '\n';

        if (face.cylinder)
        {
            const CylinderInfo& cylinder = *face.cylinder;

            os << "    cylinder.radius="
               << cylinder.radius << '\n';

            os << "    cylinder.axis.origin=("
               << cylinder.axisOrigin[0] << ", "
               << cylinder.axisOrigin[1] << ", "
               << cylinder.axisOrigin[2] << ")\n";

            os << "    cylinder.axis.direction=("
               << cylinder.axisDirection[0] << ", "
               << cylinder.axisDirection[1] << ", "
               << cylinder.axisDirection[2] << ")\n";
        }
    }

    os << "\nEdge -> Faces adjacency\n";

    for (const EdgeAdjacency& edge : report.adjacency)
    {
        os << "  Edge #" << edge.edgeId << " ->";

        for (const int faceId : edge.faceIds)
        {
            os << " Face #" << faceId;
        }

        os << '\n';
    }
}

} // namespace occexplorer
