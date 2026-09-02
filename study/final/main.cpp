#include "ShapeInspector.hpp"

#include <BRepAlgoAPI_Cut.hxx>
#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>

#include <TopoDS_Shape.hxx>

#include <gp_Ax2.hxx>
#include <gp_Dir.hxx>
#include <gp_Pnt.hxx>

#include <exception>
#include <iostream>
#include <stdexcept>

namespace
{
TopoDS_Shape makeThroughHoleDemo()
{
    const TopoDS_Shape box =
        BRepPrimAPI_MakeBox(100.0, 80.0, 20.0).Shape();

    const gp_Ax2 holeAxis(
        gp_Pnt(50.0, 40.0, -1.0),
        gp_Dir(0.0, 0.0, 1.0));

    const TopoDS_Shape cylinder =
        BRepPrimAPI_MakeCylinder(
            holeAxis,
            10.0,
            22.0).Shape();

    BRepAlgoAPI_Cut cut(box, cylinder);

    if (!cut.IsDone() || cut.Shape().IsNull())
    {
        throw std::runtime_error(
            "Failed to build box-with-through-hole demo");
    }

    return cut.Shape();
}
}

int main()
{
    try
    {
        const TopoDS_Shape shape = makeThroughHoleDemo();

        const occexplorer::ShapeReport report =
            occexplorer::ShapeInspector::Analyze(shape);

        occexplorer::ShapeInspector::Print(
            report,
            std::cout);

        return 0;
    }
    catch (const std::exception& ex)
    {
        std::cerr << "Error: " << ex.what() << '\n';
        return 1;
    }
}
