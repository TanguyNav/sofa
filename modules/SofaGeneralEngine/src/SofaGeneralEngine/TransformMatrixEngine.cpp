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

#include <SofaGeneralEngine/TransformMatrixEngine.h>

#include <sofa/core/ObjectFactory.h>
#include <sofa/helper/accessor.h>

#include <sofa/type/Quat.h>

namespace sofa::component::engine
{

using namespace sofa::type;
using namespace sofa::defaulttype;

int TranslateTransformMatrixEngineClass = core::RegisterObject("Compose the input transform (if any) with the given translation")
        .add< TranslateTransformMatrixEngine >();

int InvertTransformMatrixEngineClass = core::RegisterObject("Inverts the input transform")
        .add< InvertTransformMatrixEngine >();

int ScaleTransformMatrixEngineClass = core::RegisterObject("Compose the input transform (if any) with the given scale transformation")
        .add< ScaleTransformMatrixEngine >();

int RotateTransformMatrixEngineClass = core::RegisterObject("Compose the input transform (if any) with the given rotation")
        .add< RotateTransformMatrixEngine >();

/*
 * AbstractTransformMatrixEngine
 */

AbstractTransformMatrixEngine::AbstractTransformMatrixEngine()
    : d_inT ( initData (&d_inT, Matrix4::s_identity, "inT", "input transformation if any") )
    , d_outT( initData (&d_outT, "outT", "output transformation") )
{}

void AbstractTransformMatrixEngine::init()
{
    addInput(&d_inT);
    addOutput(&d_outT);
    setDirtyValue();
}

void AbstractTransformMatrixEngine::reinit()
{
    update();
}

/*
 * InvertTransformMatrixEngine
 */

void InvertTransformMatrixEngine::doUpdate()
{
    helper::ReadAccessor< Data<Matrix4> > inT = d_inT;
    helper::WriteAccessor< Data<Matrix4> > outT = d_outT;

    const bool canInvert = type::transformInvertMatrix((*outT), (*inT));
    if(!canInvert)
    {
        msg_warning() << "Could not invert matrix";
    }
}

/*
 * TranslateTransformMatrixEngine
 */

TranslateTransformMatrixEngine::TranslateTransformMatrixEngine()
    : d_translation ( initData (&d_translation, "translation", "translation vector") )
{
}

void TranslateTransformMatrixEngine::init()
{
    AbstractTransformMatrixEngine::init();
    addInput(&d_translation);
    setDirtyValue();
}

void TranslateTransformMatrixEngine::doUpdate()
{
    helper::ReadAccessor< Data<Matrix4> > inT = d_inT;
    helper::ReadAccessor< Data<Vector3> > translation = d_translation;
    helper::WriteAccessor< Data<Matrix4> > outT = d_outT;

    Matrix4 myT;
    myT.identity();
    myT.setsub(0,3,(*translation));

    (*outT) = (*inT) * myT;
}

/*
 * RotateTransformMatrixEngine
 */

RotateTransformMatrixEngine::RotateTransformMatrixEngine()
    : d_rotation ( initData (&d_rotation, "rotation", "euler angles") )
{
}

void RotateTransformMatrixEngine::init()
{
    AbstractTransformMatrixEngine::init();
    addInput(&d_rotation);
    setDirtyValue();
}

void RotateTransformMatrixEngine::doUpdate()
{
    helper::ReadAccessor< Data<Matrix4> > inT = d_inT;
    helper::ReadAccessor< Data<Vector3> > rotation = d_rotation;
    helper::WriteAccessor< Data<Matrix4> > outT = d_outT;

    Matrix4 myT;
    myT.identity();
    Matrix3 R;
    auto q = Quat<SReal>::createQuaterFromEuler((*rotation) * M_PI / 180.0);
    q.toMatrix(R);
    myT.setsub(0,0,R);

    (*outT) = (*inT) * myT;
}


/*
 * ScaleTransformMatrixEngine
 */
ScaleTransformMatrixEngine::ScaleTransformMatrixEngine()
    : d_scale ( initData (&d_scale, "scale", "scaling values") )
{
}

void ScaleTransformMatrixEngine::init()
{
    AbstractTransformMatrixEngine::init();
    addInput(&d_scale);
    setDirtyValue();
}

void ScaleTransformMatrixEngine::doUpdate()
{
    helper::ReadAccessor< Data<Matrix4> > inT = d_inT;
    helper::ReadAccessor< Data<Vector3> > scale = d_scale;
    helper::WriteAccessor< Data<Matrix4> > outT = d_outT;

    Matrix4 myT;
    myT.identity();
    myT(0,0) = (*scale)(0);
    myT(1,1) = (*scale)(1);
    myT(2,2) = (*scale)(2);

    (*outT) = (*inT) * myT;
}

} //namespace sofa::component::engine
