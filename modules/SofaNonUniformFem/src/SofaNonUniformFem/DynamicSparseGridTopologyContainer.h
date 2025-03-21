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

#include <SofaNonUniformFem/config.h>

#include <SofaBaseTopology/HexahedronSetTopologyContainer.h>
#include <sofa/core/topology/TopologyData.h>
#include <sofa/helper/map.h>
#include <sofa/type/Vec.h>

namespace sofa::component::topology
{

/** a class that stores a sparse regular grid of hexahedra and provides a better loading and access to neighbors than HexahedronSetTopologyContainer */
class SOFA_SOFANONUNIFORMFEM_API DynamicSparseGridTopologyContainer : public HexahedronSetTopologyContainer
{
    friend class DynamicSparseGridTopologyModifier;

public:
    SOFA_CLASS(DynamicSparseGridTopologyContainer,HexahedronSetTopologyContainer);

    typedef Hexa Hexahedron;
    typedef EdgesInHexahedron EdgesInHexahedron;
    typedef QuadsInHexahedron QuadsInHexahedron;

    Data<sofa::type::Vec3i> resolution; ///< voxel grid resolution

    Data< sofa::type::vector<unsigned char> > valuesIndexedInRegularGrid; ///< values indexed in the Regular Grid
    core::topology::HexahedronData< sofa::type::vector<unsigned char> > valuesIndexedInTopology; ///< values indexed in the topology

    Data< sofa::type::vector<BaseMeshTopology::HexaID> > idxInRegularGrid; ///< indices in the Regular Grid
    Data< std::map< unsigned int, BaseMeshTopology::HexaID> > idInRegularGrid2IndexInTopo; ///< map between id in the Regular Grid and index in the topology
    Data< type::Vector3 > voxelSize; ///< Size of the Voxels
protected:
    DynamicSparseGridTopologyContainer();
    ~DynamicSparseGridTopologyContainer() override {}
public:
    void init() override;

};

} // namespace sofa::component::topology
