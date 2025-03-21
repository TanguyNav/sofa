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
#pragma once

#include <sofa/core/topology/TopologySubsetData.h>

SOFA_DEPRECATED_HEADER("v21.12", "v22.06", "sofa/core/topology/TopologySubsetData.h")

namespace sofa::component::topology
{
    template< class TopologyElementType, class VecT>
    using TopologySubsetData = sofa::core::topology::TopologySubsetData< TopologyElementType, VecT>;

    template< class VecT > using PointSubsetData = sofa::core::topology::PointSubsetData<VecT>;
    template< class VecT > using EdgeSubsetData = sofa::core::topology::EdgeSubsetData<VecT>;
    template< class VecT > using TriangleSubsetData = sofa::core::topology::TriangleSubsetData<VecT>;
    template< class VecT > using QuadSubsetData = sofa::core::topology::QuadSubsetData<VecT>;
    template< class VecT > using TetrahedronSubsetData = sofa::core::topology::TetrahedronSubsetData<VecT>;
    template< class VecT > using HexahedronSubsetData = sofa::core::topology::HexahedronSubsetData<VecT>;

} // namespace sofa::component::topology
