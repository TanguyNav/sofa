/******************************************************************************
*                 SOFA, Simulation Open-Framework Architecture                *
*                    (c) 2006 INRIA, USTL, UJF, CNRS, MGH                     *
*                                                                             *
* This program is free software; you can redistribute it and/or modify it     *
* under the terms of the GNU Lesser General Public License as published by    *
* the Free Software Foundation; either version 2.1 of the License, or (at     *
* your option) any later version.                                             *
*                                                                             *
* This program is distributed in the hope that it will be useful, but WITHOUT *
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or       *
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License *
* for more details.                                                           *
*                                                                             *
* You should have received a copy of the GNU Lesser General Public License    *
* along with this program. If not, see <http://www.gnu.org/licenses/>.        *
*******************************************************************************
* Authors: The SOFA Team and external contributors (see Authors.txt)          *
*                                                                             *
* Contact information: contact@sofa-framework.org                             *
******************************************************************************/
#include <SofaUserInteraction/RayModel.h>

#include <sofa/core/visual/VisualParams.h>
#include <sofa/core/ObjectFactory.h>
#include <sofa/core/behavior/MechanicalState.h>
#include <SofaBaseCollision/CubeModel.h>

namespace sofa::component::collision
{

int RayCollisionModelClass = core::RegisterObject("Collision model representing a ray in space, e.g. a mouse click")
        .add< RayCollisionModel >()
        ;

using namespace sofa::type;
using namespace sofa::defaulttype;

RayCollisionModel::RayCollisionModel(SReal length)
    : defaultLength(initData(&defaultLength, length, "", "TODO"))
{
    this->contactResponse.setValue("RayContact"); // use RayContact response class
}

void RayCollisionModel::resize(Size size)
{
    this->core::CollisionModel::resize(size);

    if (length.size() < size)
    {
        length.reserve(size);
        while (length.size() < size)
            length.push_back(defaultLength.getValue());
        direction.reserve(size);
        while (direction.size() < size)
            direction.push_back(Vector3());

    }
    else
    {
        length.resize(size);
        direction.resize(size);
    }
}


void RayCollisionModel::init()
{
    this->CollisionModel::init();

    mstate = dynamic_cast< core::behavior::MechanicalState<Vec3Types>* > (getContext()->getMechanicalState());
    if (mstate==nullptr)
    {
        msg_error() << "RayCollisionModel requires a Vec3 Mechanical Model";
        return;
    }

    {
        const int npoints = mstate->getSize();
        resize(npoints);
    }
}


int RayCollisionModel::addRay(const Vector3& origin, const Vector3& direction, SReal length)
{
    int i = size;
    resize(i);
    Ray r = getRay(i);
    r.setOrigin(origin);
    r.setDirection(direction);
    r.setL(length);
    return i;
}

void RayCollisionModel::draw(const core::visual::VisualParams* vparams, Index index)
{
    if( !vparams->isSupported(core::visual::API_OpenGL) ) return;

    Ray r(this, index);
    const Vector3& p1 = r.origin();
    const Vector3 p2 = p1 + r.direction()*r.l();

    vparams->drawTool()->saveLastState();
    vparams->drawTool()->disableLighting();
    sofa::type::RGBAColor color(1.0, 0.0, 1.0, 1.0);
    vparams->drawTool()->drawLine(p1,p2,color);
    vparams->drawTool()->restoreLastState();
}

void RayCollisionModel::draw(const core::visual::VisualParams* vparams)
{
    if (vparams->displayFlags().getShowCollisionModels())
    {       
        for (sofa::Index i=0; i<size; i++)
        {
            draw(vparams,i);
        }
    }
    if (getPrevious()!=nullptr && vparams->displayFlags().getShowBoundingCollisionModels())
    {
        getPrevious()->draw(vparams);
    }
}

void RayCollisionModel::computeBoundingTree(int maxDepth)
{
    CubeCollisionModel* cubeModel = createPrevious<CubeCollisionModel>();

    if (!isMoving() && !cubeModel->empty()) return; // No need to recompute BBox if immobile

    Vector3 minElem, maxElem;

    cubeModel->resize(size);
    if (!empty())
    {
        for (sofa::Index i=0; i<size; i++)
        {
            Ray r(this, i);
            const Vector3& o = r.origin();
            const Vector3& d = r.direction();
            const SReal l = r.l();
            for (int c=0; c<3; c++)
            {
                if (d[c]<0)
                {
                    minElem[c] = o[c] + d[c]*l;
                    maxElem[c] = o[c];
                }
                else
                {
                    minElem[c] = o[c];
                    maxElem[c] = o[c] + d[c]*l;
                }
            }
            cubeModel->setParentOf(i, minElem, maxElem);
        }
        cubeModel->computeBoundingTree(maxDepth);
    }

}

void RayCollisionModel::applyTranslation(double dx, double dy, double dz)
{
    Vector3 d(dx,dy,dz);
    for (int i = 0; i < getNbRay(); i++)
    {
        Ray ray = getRay(i);
        ray.setOrigin(ray.origin() + d);
    }
}

const type::Vector3& Ray::origin() const
{
    return model->getMechanicalState()->read(core::ConstVecCoordId::position())->getValue()[index];
}

const type::Vector3& Ray::direction() const
{
    return model->direction[index];
}

type::Vector3::value_type Ray::l() const
{
    return model->length[index];
}

void Ray::setOrigin(const type::Vector3& newOrigin)
{
    helper::WriteAccessor<Data<type::vector<type::Vector3> > > xData =
        *model->getMechanicalState()->write(core::VecCoordId::position());
    xData.wref()[index] = newOrigin;

    helper::WriteAccessor<Data<type::vector<type::Vector3> > > xDataFree =
        *model->getMechanicalState()->write(core::VecCoordId::freePosition());
    defaulttype::Vec3Types::VecCoord& freePos = xDataFree.wref();
    freePos.resize(model->getMechanicalState()->getSize());
    freePos[index] = newOrigin;
}


} //namespace sofa::component::collision
