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
#ifndef SOFA_OGLGRID_H
#define SOFA_OGLGRID_H
#include "config.h"

#include <sofa/core/visual/VisualModel.h>
#include <sofa/type/RGBAColor.h>

namespace sofa
{

namespace component
{

namespace visualmodel
{

class OglGrid : public core::visual::VisualModel
{
public:
    SOFA_CLASS(OglGrid, VisualModel);

    typedef sofa::type::Vector3 Vector3;

    enum PLANE {PLANE_X, PLANE_Y, PLANE_Z};

    Data<std::string> plane; ///< Plane of the grid
    PLANE internalPlane;

    Data<float> size; ///< Size of the squared grid
    Data<int> nbSubdiv; ///< Number of subdivisions

    Data<sofa::type::RGBAColor> color; ///< Color of the lines in the grid. default=(0.34,0.34,0.34,1.0)
    Data<float> thickness; ///< Thickness of the lines in the grid
    Data<bool> draw; ///< Display the grid or not

    OglGrid();

    void init() override;
    void reinit() override;
    void drawVisual(const core::visual::VisualParams*) override;
    void updateVisual() override;

protected:

};

} // namespace visualmodel

} // namespace component

} // namespace sofa

#endif //SOFA_OGLGRID_H
