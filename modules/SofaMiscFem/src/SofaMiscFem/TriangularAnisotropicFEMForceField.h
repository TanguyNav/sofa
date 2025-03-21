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

#include <SofaMiscFem/config.h>



#include <sofa/core/behavior/ForceField.h>
#include <sofa/core/topology/BaseMeshTopology.h>
#include <sofa/type/Vec.h>
#include <sofa/type/Mat.h>
#include "TriangularFEMForceField.h"
#include <sofa/core/topology/TopologyData.h>


namespace sofa::component::forcefield
{


template<class DataTypes>
class TriangularAnisotropicFEMForceField : public sofa::component::forcefield::TriangularFEMForceField<DataTypes>
{

public:
    SOFA_CLASS(SOFA_TEMPLATE(TriangularAnisotropicFEMForceField, DataTypes), SOFA_TEMPLATE(TriangularFEMForceField, DataTypes));

    typedef sofa::component::forcefield::TriangularFEMForceField<DataTypes> Inherited;
    typedef typename DataTypes::VecCoord VecCoord;
    typedef typename DataTypes::VecDeriv VecDeriv;
    typedef typename DataTypes::VecReal VecReal;
    typedef VecCoord Vector;
    typedef typename DataTypes::Coord    Coord   ;
    typedef typename DataTypes::Deriv    Deriv   ;
    typedef typename Coord::value_type   Real    ;
    typedef typename Inherited::TriangleInformation   TriangleInformation  ;

    typedef core::objectmodel::Data<VecCoord> DataVecCoord;
    typedef core::objectmodel::Data<VecDeriv> DataVecDeriv;

    typedef sofa::core::topology::BaseMeshTopology::Index Index;
    typedef sofa::core::topology::BaseMeshTopology::Triangle Element;
    typedef sofa::core::topology::BaseMeshTopology::SeqTriangles VecElement;

    void init() override;
    void reinit() override;
    void draw(const core::visual::VisualParams* vparams) override;
protected:
    TriangularAnisotropicFEMForceField();
    ~TriangularAnisotropicFEMForceField();
public:
    void computeMaterialStiffness(int i, Index& a, Index& b, Index& c) override;
    void getFiberDir(int element, Deriv& dir);

    //Data<Real> f_poisson2;
    //Data<Real> f_young2; ///< Young modulus along transverse direction
    Data<type::vector<Real> > f_poisson2;
    Data<type::vector<Real> > f_young2; ///< Young modulus along transverse direction
    Data<Real> f_theta; ///< Fiber angle in global reference frame (in degrees)
    Data<VecCoord> f_fiberCenter; ///< Concentric fiber center in global reference frame
    Data<bool> showFiber; ///< Flag activating rendering of fiber directions within each triangle
    typedef typename TriangularAnisotropicFEMForceField::Deriv TriangleFiberDirection;
    core::topology::TriangleData < sofa::type::vector< TriangleFiberDirection > > localFiberDirection; ///< Computed fibers direction within each triangle

    /// Link to be set to the topology container in the component graph.
    using Inherit1::l_topology;

    /** Method to initialize @sa TriangleFiberDirection when a new Triangle is created.
    * Will be set as creation callback in the TriangleData @sa localFiberDirection
    */
    void createTriangleInfo(Index triangleIndex,
        TriangleFiberDirection&,
        const core::topology::BaseMeshTopology::Triangle& t,
        const sofa::type::vector< unsigned int >&,
        const sofa::type::vector< double >&);

    /// Inherited member
    /// Bring inherited member in the current lookup context.
    /// otherwise any access to the Inherit1::member would require "this->".
    /// @see https://gcc.gnu.org/onlinedocs/gcc/Name-lookup.html
    using Inherit1::m_topology;
};

#if  !defined(SOFA_COMPONENT_FORCEFIELD_TRIANGULARANISOTROPICFEMFORCEFIELD_CPP)
extern template class SOFA_SOFAMISCFEM_API TriangularAnisotropicFEMForceField<defaulttype::Vec3Types>;

#endif

} // namespace sofa::component::forcefield
