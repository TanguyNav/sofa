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
#ifndef SOFA_COMPONENT_VISUALMODEL_DATADISPLAY_H
#define SOFA_COMPONENT_VISUALMODEL_DATADISPLAY_H
#include "config.h"

#include <sofa/core/visual/VisualModel.h>
#include <sofa/core/topology/BaseMeshTopology.h>
#include <SofaOpenglVisual/OglColorMap.h>
#include <SofaBaseVisual/VisualModelImpl.h>

#include <sofa/type/RGBAColor.h>

namespace sofa
{

namespace component
{

namespace visualmodel
{

class SOFA_OPENGL_VISUAL_API DataDisplay : public core::visual::VisualModel, public Vec3State
{
public:
    SOFA_CLASS2(DataDisplay, core::visual::VisualModel, Vec3State);

    typedef core::topology::BaseMeshTopology::Triangle Triangle;
    typedef core::topology::BaseMeshTopology::Quad     Quad;

    typedef type::vector<Real> VecPointData;
    typedef type::vector<Real> VecCellData;

public:
    Data<bool> f_maximalRange; ///< Keep the maximal range through all timesteps
    Data<VecPointData> f_pointData; ///< Data associated with nodes
    Data<VecCellData> f_triangleData; ///< Data associated with triangles
    Data<VecCellData> f_quadData; ///< Data associated with quads
    Data<VecPointData> f_pointTriangleData; ///< Data associated with nodes per triangle
    Data<VecPointData> f_pointQuadData; ///< Data associated with nodes per quad
    Data<sofa::type::RGBAColor> f_colorNaN; ///< Color for NaNs
    Data<type::Vec2f> d_userRange; ///< Clamp to this values (if max>min)
    Data<Real> d_currentMin; ///< Current min range
    Data<Real> d_currentMax; ///< Current max range
    Data<float> d_shininess; ///< Shininess for rendering point-based data [0,128].  <0 means no specularity
    Data<Real> d_transparency; ///< Add transparency when we draw triangles (this allows to see inside the volume).

    visualmodel::OglColorMap *colorMap;
    core::State<DataTypes> *state;
    core::topology::BaseMeshTopology* m_topology;

    /// Link to be set to the topology container in the component graph.
    SingleLink <DataDisplay, sofa::core::topology::BaseMeshTopology, BaseLink::FLAG_STOREPATH | BaseLink::FLAG_STRONGLINK> l_topology;

    Real oldMin, oldMax;

    void init() override;
    void drawVisual(const core::visual::VisualParams* vparams) override;
    void updateVisual() override;

    bool insertInNode( core::objectmodel::BaseNode* node ) override { Inherit1::insertInNode(node); Inherit2::insertInNode(node); return true; }
    bool removeInNode( core::objectmodel::BaseNode* node ) override { Inherit1::removeInNode(node); Inherit2::removeInNode(node); return true; }

protected:
    void computeNormals();
    type::vector<type::Vec3f> m_normals;

    DataDisplay();
};

} // namespace visualmodel

} // namespace component

} // namespace sofa

#endif // #ifndef SOFA_COMPONENT_VISUALMODEL_DATADISPLAY_H
