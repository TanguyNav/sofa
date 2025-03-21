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

#include <SofaGeneralDeformable/config.h>

#include <sofa/core/behavior/ForceField.h>
#include <sofa/core/topology/BaseMeshTopology.h>
#include <sofa/type/Vec.h>
#include <sofa/type/Mat.h>
#include <sofa/core/topology/TopologyData.h>

#include <sofa/type/Mat.h>
#include <sofa/type/fixed_array.h>

#include <map>
#include <set>


namespace sofa::component::forcefield
{

/**
Bending springs added between vertices of quads sharing a common edge.
The springs connect the vertices not belonging to the common edge. It compresses when the surface bends along the common edge.
*/
template<class DataTypes>
class QuadularBendingSprings : public core::behavior::ForceField<DataTypes>
{
public:
    SOFA_CLASS(SOFA_TEMPLATE(QuadularBendingSprings,DataTypes), SOFA_TEMPLATE(core::behavior::ForceField,DataTypes));

    typedef core::behavior::ForceField<DataTypes> Inherited;
    //typedef typename DataTypes::Real Real;
    typedef typename DataTypes::VecCoord VecCoord;

    typedef typename DataTypes::VecDeriv VecDeriv;
    typedef typename DataTypes::Coord Coord;
    typedef typename DataTypes::Deriv Deriv;
    typedef typename Coord::value_type Real;

    typedef core::objectmodel::Data<VecDeriv>    DataVecDeriv;
    typedef core::objectmodel::Data<VecCoord>    DataVecCoord;

    enum { N=DataTypes::spatial_dimensions };
    typedef type::Mat<N,N,Real> Mat;

    using Index = sofa::Index;

    QuadularBendingSprings();

    ~QuadularBendingSprings();

protected:

    class EdgeInformation
    {
    public:
        Mat DfDx; /// the edge stiffness matrix

        int     m1, m2;  /// the two extremities of the first spring: masses m1 and m2
        int     m3, m4;  /// the two extremities of the second spring: masses m3 and m4

        double  ks;      /// spring stiffness (initialized to the default value)
        double  kd;      /// damping factor (initialized to the default value)

        double  restlength1; /// rest length of the first spring
        double  restlength2; /// rest length of the second spring

        bool is_activated;

        bool is_initialized;

        EdgeInformation(int m1=0, int m2=0, int m3=0, int m4=0, double restlength1=0.0, double restlength2=0.0, bool is_activated=false, bool is_initialized=false)
            : m1(m1), m2(m2), m3(m3), m4(m4), restlength1(restlength1), restlength2(restlength2), is_activated(is_activated), is_initialized(is_initialized)
        {
        }

        /// Output stream
        inline friend std::ostream& operator<< ( std::ostream& os, const EdgeInformation& /*ei*/ )
        {
            return os;
        }

        /// Input stream
        inline friend std::istream& operator>> ( std::istream& in, EdgeInformation& /*ei*/ )
        {
            return in;
        }
    };



public:
    /// Searches quad topology and creates the bending springs
    void init() override;

    void addForce(const core::MechanicalParams* mparams, DataVecDeriv& d_f, const DataVecCoord& d_x, const DataVecDeriv& d_v) override;
    void addDForce(const core::MechanicalParams* mparams, DataVecDeriv& d_df, const DataVecDeriv& d_dx) override;

    SReal getPotentialEnergy(const core::MechanicalParams* /* mparams */, const DataVecCoord& /* d_x */) const override;

    virtual double getKs() const { return f_ks.getValue();}
    virtual double getKd() const { return f_kd.getValue();}

    void setKs(const double ks)
    {
        f_ks.setValue((double)ks);
    }
    void setKd(const double kd)
    {
        f_kd.setValue((double)kd);
    }

    // -- VisualModel interface
    void draw(const core::visual::VisualParams* vparams) override;
    void initTextures() { }
    void update() { }

    sofa::core::topology::EdgeData<sofa::type::vector<EdgeInformation> > &getEdgeInfo() {return edgeInfo;}

    /** Method to initialize @sa EdgeInformation when a new edge is created.
    * Will be set as creation callback in the EdgeData @sa edgeInfo
    */
    void applyEdgeCreation(Index edgeIndex, EdgeInformation& ei,
        const core::topology::BaseMeshTopology::Edge&,
        const sofa::type::vector< Index >&,
        const sofa::type::vector< double >&);

    /** Method to update @sa edgeInfo when a new quad is created.
    * Will be set as callback in the EdgeData @sa edgeInfo when QUADSADDED event is fired
    * to create a new spring between new created triangles.
    */
    void applyQuadCreation(const sofa::type::vector<Index>& quadAdded,
        const sofa::type::vector<core::topology::BaseMeshTopology::Quad>&,
        const sofa::type::vector<sofa::type::vector<Index> >&,
        const sofa::type::vector<sofa::type::vector<double> >&);

    /** Method to update @sa edgeInfo when a quad is removed.
    * Will be set as callback in the EdgeData @sa edgeInfo when QUADSREMOVED event is fired
    * to remove spring if needed or update pair of quad.
    */
    void applyQuadDestruction(const sofa::type::vector<Index>& quadRemoved);

    /// Method to update @sa edgeInfo when a point is removed. Will be set as callback when POINTSREMOVED event is fired
    void applyPointDestruction(const sofa::type::vector<Index>& pointIndices);

    /// Method to update @sa edgeInfo when points are renumbered. Will be set as callback when POINTSRENUMBERING event is fired
    void applyPointRenumbering(const sofa::type::vector<Index>& pointToRenumber);


    Data<double> f_ks; ///< uniform stiffness for the all springs
    Data<double> f_kd; ///< uniform damping for the all springs

    /// Link to be set to the topology container in the component graph.
    SingleLink<QuadularBendingSprings<DataTypes>, sofa::core::topology::BaseMeshTopology, BaseLink::FLAG_STOREPATH | BaseLink::FLAG_STRONGLINK> l_topology;

protected:
    sofa::core::topology::EdgeData<sofa::type::vector<EdgeInformation> > edgeInfo; ///< Internal edge data

    /// Pointer to the current topology
    sofa::core::topology::BaseMeshTopology* m_topology;

    bool updateMatrix;
    SReal m_potentialEnergy;
};


#if  !defined(SOFA_COMPONENT_FORCEFIELD_QUADULARBENDINGSPRINGS_CPP)

extern template class SOFA_SOFAGENERALDEFORMABLE_API QuadularBendingSprings<sofa::defaulttype::Vec3Types>;



#endif //  !defined(SOFA_COMPONENT_FORCEFIELD_QUADULARBENDINGSPRINGS_CPP)

} // namespace sofa::component::forcefield
