#include "Sofa_test.h"
#include "InitPlugin_test.h"

#include <SofaBaseTopology/MeshTopology.h>
#include <SofaMiscCollision/MeshIntTool.h>

#include <sofa/simulation/Node.h>

namespace sofa{

namespace PrimitiveCreationTest{

using sofa::type::Vec3;

/**
  *\brief Makes up an OBBCollisionModel<sofa::defaulttype::Rigid3Types> containing just one OBB. angles and order are the rotations used to make up this OBB.
  *
  *\param p the center of the OBB
  *\param angles it is of size 3 and contains the rotations around axes, i.e., angles[0] contains rotation around x axis etc...
  *\param order it is the order we rotate, i.e, if we want to rotate first around z axis, then x axis and then y axis order will be {2,0,1}
  *\param v it is the velocity of the OBB
  *\param extents it contains half-extents of the OBB
  *\param father it is a node that will contain the returned OBBModel
  */
SOFA_SOFATEST_API sofa::component::collision::OBBCollisionModel<sofa::defaulttype::Rigid3Types>::SPtr makeOBB(const Vec3 & p,const double *angles,const int *order,const Vec3 &v,const Vec3 &extents, sofa::simulation::Node::SPtr &father);

SOFA_SOFATEST_API sofa::component::collision::TriangleCollisionModel<sofa::defaulttype::Vec3Types>::SPtr makeTri(const Vec3 & p0,const Vec3 & p1,const Vec3 & p2,const Vec3 & v, sofa::simulation::Node::SPtr &father);

SOFA_SOFATEST_API sofa::component::collision::CapsuleCollisionModel<sofa::defaulttype::Vec3Types>::SPtr makeCap(const Vec3 & p0,const Vec3 & p1,double radius,const Vec3 & v,
                                                                   sofa::simulation::Node::SPtr & father);

SOFA_SOFATEST_API sofa::component::collision::SphereCollisionModel<sofa::defaulttype::Rigid3Types>::SPtr makeRigidSphere(const Vec3 & p,SReal radius,const Vec3 &v,const double *angles,const int *order,
                                                                            sofa::simulation::Node::SPtr & father);

SOFA_SOFATEST_API sofa::component::collision::SphereCollisionModel<sofa::defaulttype::Vec3Types>::SPtr makeSphere(const Vec3 & p,SReal radius,const Vec3 & v,sofa::simulation::Node::SPtr & father);

void rotx(double ax,Vec3 & x,Vec3 & y,Vec3 & z);

void roty(double ax,Vec3 & x,Vec3 & y,Vec3 & z);

void rotz(double ax,Vec3 & x,Vec3 & y,Vec3 & z);

}
}
