#include <Standard_Version.hxx>

#include <gp_Dir.hxx>
#include <gp_Pnt.hxx>
#include <gp_Vec.hxx>

#include <iostream>

int main()
{
    const gp_Pnt p(10.0, 20.0, 30.0);
    const gp_Vec v(3.0, 4.0, 0.0);
    const gp_Dir zDir(0.0, 0.0, 1.0);

    std::cout << "OCCT version: " << OCC_VERSION_COMPLETE << '\n';
    std::cout << "Point P = (" << p.X() << ", " << p.Y() << ", " << p.Z() << ")\n";
    std::cout << "Vector length = " << v.Magnitude() << '\n';
    std::cout << "Direction Z = (" << zDir.X() << ", " << zDir.Y() << ", " << zDir.Z() << ")\n";
    std::cout << "OCCExplorer environment is ready.\n";
    return 0;
}
