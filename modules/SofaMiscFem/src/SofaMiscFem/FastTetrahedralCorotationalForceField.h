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
#include <sofa/type/fixed_array.h>
#include <sofa/type/vector.h>
#include <sofa/type/Vec.h>
#include <sofa/type/Mat.h>
#include <sofa/core/topology/TopologyData.h>


namespace sofa::component::forcefield
{


template<class DataTypes>
class FastTetrahedralCorotationalForceField : public core::behavior::ForceField<DataTypes>
{
public:
    SOFA_CLASS(SOFA_TEMPLATE(FastTetrahedralCorotationalForceField,DataTypes), SOFA_TEMPLATE(core::behavior::ForceField,DataTypes));

    typedef core::behavior::ForceField<DataTypes> Inherited;
    typedef typename DataTypes::Real        Real        ;
    typedef typename DataTypes::Coord       Coord       ;
    typedef typename DataTypes::Deriv       Deriv       ;
    typedef typename DataTypes::VecCoord    VecCoord    ;
    typedef typename DataTypes::VecDeriv    VecDeriv    ;
    typedef typename DataTypes::VecReal     VecReal     ;
    typedef Data<VecCoord>                  DataVecCoord;
    typedef Data<VecDeriv>                  DataVecDeriv;

    typedef type::Mat<3,3,Real>       Mat3x3  ;

    typedef enum
    {
        POLAR_DECOMPOSITION,
        QR_DECOMPOSITION,
	    POLAR_DECOMPOSITION_MODIFIED,
		LINEAR_ELASTIC
    } RotationDecompositionMethod;

    typedef core::topology::BaseMeshTopology::Tetra Tetra;
    typedef core::topology::BaseMeshTopology::EdgesInTetrahedron EdgesInTetrahedron;
    typedef core::topology::BaseMeshTopology::Tetra Tetrahedron;
    typedef sofa::Index Index;
    

protected:    
    /// data structure stored for each tetrahedron
    class TetrahedronRestInformation
    {
    public:
        /// shape vector at the rest configuration
        Coord shapeVector[4];
        /// rest volume
        Real restVolume;
        Coord restEdgeVector[6];
        Mat3x3 linearDfDxDiag[4];  // the diagonal 3x3 block matrices that makes the 12x12 linear elastic matrix
        Mat3x3 linearDfDx[6];  // the off-diagonal 3x3 block matrices that makes the 12x12 linear elastic matrix
        Mat3x3 rotation; // rotation from deformed to rest configuration
        Mat3x3 restRotation; // used for QR decomposition
        //unsigned int v[4]; // the indices of the 4 vertices

        Real edgeOrientation[6];


        /// Output stream
        inline friend std::ostream& operator<< ( std::ostream& os, const TetrahedronRestInformation& /*eri*/ )
        {
            return os;
        }

        /// Input stream
        inline friend std::istream& operator>> ( std::istream& in, TetrahedronRestInformation& /*eri*/ )
        {
            return in;
        }

        TetrahedronRestInformation()
        {
        }
    };

    core::topology::PointData<sofa::type::vector<Mat3x3> > pointInfo; ///< Internal point data
    core::topology::EdgeData<sofa::type::vector<Mat3x3> > edgeInfo; ///< Internal edge data
    core::topology::TetrahedronData<sofa::type::vector<TetrahedronRestInformation> > tetrahedronInfo; ///< Internal tetrahedron data

    /** Method to initialize @sa TetrahedronRestInformation when a new Tetrahedron is created.
    * Will be set as creation callback in the TetrahedronData @sa tetrahedronInfo
    */
    void createTetrahedronRestInformation(Index, TetrahedronRestInformation& t,
        const core::topology::BaseMeshTopology::Tetrahedron&,
        const sofa::type::vector<Index>&,
        const sofa::type::vector<double>&);

    sofa::core::topology::BaseMeshTopology* m_topology;
    VecCoord  _initialPoints;///< the intial positions of the points

    bool updateMatrix;
    bool updateTopologyInfo;

    Data<std::string> f_method; ///< the computation method of the displacements
    RotationDecompositionMethod decompositionMethod;

    Data<Real> f_poissonRatio; ///< Poisson ratio in Hooke's law
    Data<Real> f_youngModulus; ///< Young modulus in Hooke's law

    Real lambda;  /// first Lame coefficient
    Real mu;    /// second Lame coefficient

    Data<bool> f_drawing; ///<  draw the forcefield if true
    Data<sofa::type::RGBAColor> drawColor1; ///<  draw color for faces 1
    Data<sofa::type::RGBAColor> drawColor2; ///<  draw color for faces 2
    Data<sofa::type::RGBAColor> drawColor3; ///<  draw color for faces 3
    Data<sofa::type::RGBAColor> drawColor4; ///<  draw color for faces 4

    /// Link to be set to the topology container in the component graph.
    SingleLink<FastTetrahedralCorotationalForceField<DataTypes>, sofa::core::topology::BaseMeshTopology, BaseLink::FLAG_STOREPATH | BaseLink::FLAG_STRONGLINK> l_topology;

    FastTetrahedralCorotationalForceField();

    virtual ~FastTetrahedralCorotationalForceField();

public:

    void init() override;


    void addForce(const sofa::core::MechanicalParams* /*mparams*/, DataVecDeriv &  dataF, const DataVecCoord &  dataX , const DataVecDeriv & dataV ) override;
    void addDForce(const sofa::core::MechanicalParams* /*mparams*/, DataVecDeriv&   datadF , const DataVecDeriv&   datadX ) override;
    SReal getPotentialEnergy(const core::MechanicalParams* /*mparams*/, const DataVecCoord&  /* x */) const override
    {
        msg_warning() << "Method getPotentialEnergy not implemented yet.";
        return 0.0;
    }

    void addKToMatrix(sofa::linearalgebra::BaseMatrix *m, SReal kFactor, unsigned int &offset) override;
    void addKToMatrix(const core::MechanicalParams* /*mparams*/, const sofa::core::behavior::MultiMatrixAccessor* /*matrix*/ ) override;

    void updateTopologyInformation();

    virtual Real getLambda() const { return lambda;}
    virtual Real getMu() const { return mu;}

    void setYoungModulus(const double modulus)
    {
        f_youngModulus.setValue((Real)modulus);
    }
    void setPoissonRatio(const double ratio)
    {
        f_poissonRatio.setValue((Real)ratio);
    }
    void setRotationDecompositionMethod( const RotationDecompositionMethod m)
    {
        decompositionMethod=m;
    }
    void draw(const core::visual::VisualParams* vparams) override;
    /// compute lambda and mu based on the Young modulus and Poisson ratio
    void updateLameCoefficients();



protected :
    static void computeQRRotation( Mat3x3 &r, const Coord *dp);

    core::topology::EdgeData<sofa::type::vector<Mat3x3> > &getEdgeInfo() {return edgeInfo;}
};

#if  !defined(SOFA_COMPONENT_INTERACTIONFORCEFIELD_FASTTETRAHEDRALCOROTATIONALFORCEFIELD_CPP)
extern template class SOFA_SOFAMISCFEM_API FastTetrahedralCorotationalForceField<sofa::defaulttype::Vec3Types>;

#endif

} // namespace sofa::component::forcefield
