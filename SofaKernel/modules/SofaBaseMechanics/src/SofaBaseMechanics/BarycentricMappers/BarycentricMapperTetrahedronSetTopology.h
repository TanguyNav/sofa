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
#include <SofaBaseMechanics/BarycentricMappers/BarycentricMapperTopologyContainer.h>

namespace sofa::component::mapping
{

using sofa::defaulttype::Vec3dTypes;
using sofa::defaulttype::Vec3fTypes;
using sofa::type::Mat3x3d;
using sofa::type::Vector3;
typedef typename sofa::core::topology::BaseMeshTopology::Tetrahedron Tetrahedron;

/// Class allowing barycentric mapping computation on a TetrahedronSetTopology
template<class In, class Out>
class BarycentricMapperTetrahedronSetTopology : public BarycentricMapperTopologyContainer<In,Out,typename BarycentricMapper<In,Out>::MappingData3D,Tetrahedron>
{
    typedef typename BarycentricMapper<In,Out>::MappingData3D MappingData;
    using Index = sofa::Index;

public:
    SOFA_CLASS(SOFA_TEMPLATE2(BarycentricMapperTetrahedronSetTopology,In,Out),SOFA_TEMPLATE4(BarycentricMapperTopologyContainer,In,Out,MappingData,Tetrahedron));
    typedef typename Inherit1::Real Real;
    typedef typename In::VecCoord VecCoord;
    using Inherit1::m_toTopology;

    Index addPointInTetra(const Index index, const SReal* baryCoords) override ;

protected:
    BarycentricMapperTetrahedronSetTopology(sofa::core::topology::TopologyContainer* fromTopology,
        core::topology::BaseMeshTopology* toTopology);
    ~BarycentricMapperTetrahedronSetTopology() override = default;

    virtual type::vector<Tetrahedron> getElements() override;
    virtual type::vector<SReal> getBaryCoef(const Real* f) override;
    type::vector<SReal> getBaryCoef(const Real fx, const Real fy, const Real fz);
    void computeBase(Mat3x3d& base, const typename In::VecCoord& in, const Tetrahedron& element) override;
    void computeCenter(Vector3& center, const typename In::VecCoord& in, const Tetrahedron& element) override;
    void computeDistance(double& d, const Vector3& v) override;
    void addPointInElement(const Index elementIndex, const SReal* baryCoords) override;

    //handle topology changes depending on the topology
    void processTopologicalChanges(const typename Out::VecCoord& out, const typename In::VecCoord& in, core::topology::Topology* t);

    void processAddPoint(const sofa::type::Vec3d & pos, const typename In::VecCoord& in, MappingData & vectorData);

    using Inherit1::d_map;
    using Inherit1::m_fromTopology;
    using Inherit1::m_matrixJ;
    using Inherit1::m_updateJ;
};

#if !defined(SOFA_COMPONENT_MAPPING_BARYCENTRICMAPPERTETRAHEDRONSETTOPOLOGY_CPP)
extern template class SOFA_SOFABASEMECHANICS_API BarycentricMapperTetrahedronSetTopology< Vec3dTypes, Vec3dTypes >;
#endif

} // namespace sofa::component::mapping
