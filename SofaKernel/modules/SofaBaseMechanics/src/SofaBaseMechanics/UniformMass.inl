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
#include <SofaBaseMechanics/UniformMass.h>
#include <sofa/core/fwd.h>
#include <sofa/core/visual/VisualParams.h>
#include <sofa/core/objectmodel/Context.h>
#include <sofa/helper/accessor.h>
#include <sofa/defaulttype/DataTypeInfo.h>
#include <SofaBaseMechanics/AddMToMatrixFunctor.h>
#include <sofa/core/behavior/MultiMatrixAccessor.h>
#include <sofa/core/topology/BaseTopology.h>
#include <sofa/core/topology/TopologyData.inl>
#include <sofa/core/MechanicalParams.h>

namespace sofa::component::mass
{

using helper::WriteAccessor;
using helper::ReadAccessor;
using helper::WriteOnlyAccessor;
using type::vector;

using std::list;

using core::behavior::Mass;
using core::topology::BaseMeshTopology;
using core::topology::TopologyChange;
using core::MechanicalParams;
using core::behavior::MultiMatrixAccessor;
using core::visual::VisualParams;
using core::ConstVecCoordId;

using linearalgebra::BaseVector;
using type::Vec;
using type::Vec3d;
using defaulttype::DataTypeInfo;
using linearalgebra::BaseMatrix;

template <class DataTypes, class MassType>
UniformMass<DataTypes, MassType>::UniformMass()
    : d_vertexMass ( initData ( &d_vertexMass, MassType ( 1.0f ), "vertexMass", "Specify one single, positive, real value for the mass of each particle. \n"
                                                                                "If unspecified or wrongly set, the totalMass information is used." ) )
    , d_totalMass ( initData ( &d_totalMass, SReal(1.0), "totalMass", "Specify the total mass resulting from all particles. \n"
                                                                      "If unspecified or wrongly set, the default value is used: totalMass = 1.0") )
    , d_filenameMass ( initData ( &d_filenameMass, "filename", "rigid file to load the mass parameters" ) )
    , d_showCenterOfGravity ( initData ( &d_showCenterOfGravity, false, "showGravityCenter", "display the center of gravity of the system" ) )
    , d_showAxisSize ( initData ( &d_showAxisSize, 1.0f, "showAxisSizeFactor", "factor length of the axis displayed (only used for rigids)" ) )
    , d_computeMappingInertia ( initData ( &d_computeMappingInertia, false, "compute_mapping_inertia", "to be used if the mass is placed under a mapping" ) )
    , d_showInitialCenterOfGravity ( initData ( &d_showInitialCenterOfGravity, false, "showInitialCenterOfGravity", "display the initial center of gravity of the system" ) )
    , d_showX0 ( initData ( &d_showX0, false, "showX0", "display the rest positions" ) )
    , d_localRange ( initData ( &d_localRange, Vec<2,int> ( -1,-1 ), "localRange", "optional range of local DOF indices. \n"
                                                                                   "Any computation involving only indices outside of this range \n"
                                                                                   "are discarded (useful for parallelization using mesh partitionning)" ) )
    , d_indices ( initData ( &d_indices, "indices", "optional local DOF indices. Any computation involving only indices outside of this list are discarded" ) )
    , d_preserveTotalMass( initData ( &d_preserveTotalMass, false, "preserveTotalMass", "Prevent totalMass from decreasing when removing particles."))
    , l_topology(initLink("topology", "link to the topology container"))
{
    constructor_message();
}

template <class DataTypes, class MassType>
UniformMass<DataTypes, MassType>::~UniformMass()
{}

template <class DataTypes, class MassType>
void UniformMass<DataTypes, MassType>::constructor_message()
{
    d_filenameMass.setDisplayed(true) ;
    d_filenameMass.setReadOnly(true) ;
    d_filenameMass.setValue("unused") ;
    d_filenameMass.setHelp("File storing the mass parameters [rigid objects only].");
}

template <class DataTypes, class MassType>
void UniformMass<DataTypes, MassType>::setMass ( const MassType& m )
{
    const MassType& currentVertexMass = d_vertexMass.getValue();
    d_vertexMass.setValue( m );
    if(!checkVertexMass())
    {
        msg_warning() << "Given value to setVertexMass() is not a strictly positive value\n"
                      << "Previous value is used: vertexMass = " << currentVertexMass;
        d_vertexMass.setValue(currentVertexMass);
    }
}

template <class DataTypes, class MassType>
void UniformMass<DataTypes, MassType>::setTotalMass ( SReal m )
{
    Real currentTotalMass = d_totalMass.getValue();
    d_totalMass.setValue( m );
    if(!checkTotalMass())
    {
        msg_warning() << "Given value to setTotalMass() is not a strictly positive value\n"
                      << "Previous value is used: totalMass = " << currentTotalMass;
        d_totalMass.setValue(currentTotalMass);
    }
}


template <class DataTypes, class MassType>
void UniformMass<DataTypes, MassType>::init()
{
    initDefaultImpl();
}


template <class DataTypes, class MassType>
void UniformMass<DataTypes, MassType>::initDefaultImpl()
{
    this->d_componentState.setValue(sofa::core::objectmodel::ComponentState::Valid);

    Mass<DataTypes>::init();

    WriteAccessor<Data<SetIndexArray > > indices = d_indices;
    
    if(mstate==nullptr)
    {
        msg_warning(this) << "Missing mechanical state. \n"
                             "UniformMass need to be used with an object also having a MechanicalState. \n"
                             "To remove this warning: add a <MechanicalObject/> to the parent node of the one \n"
                             " containing this <UniformMass/>";
        return;
    }

    if ( d_filenameMass.isSet() && d_filenameMass.getValue() != "unused" )
    {
        loadRigidMass(d_filenameMass.getFullPath()) ;
    }

    //If localRange is set, update indices
    if (d_localRange.getValue()[0] >= 0
        && d_localRange.getValue()[1] > 0
        && d_localRange.getValue()[1] + 1 < int(mstate->getSize()))
    {
        indices.clear();
        for(int i=d_localRange.getValue()[0]; i<=d_localRange.getValue()[1]; i++)
            indices.push_back(i);
    }

    //If no given indices
    if(indices.size()==0)
    {
        indices.clear();
        for(int i=0; i<int(mstate->getSize()); i++)
            indices.push_back(i);
    }

    // check if mass should use topology
    if (l_topology.empty())
    {
        l_topology.set(this->getContext()->getMeshTopologyLink());
    }

    BaseMeshTopology* meshTopology = l_topology.get();
    if (meshTopology != nullptr && dynamic_cast<sofa::core::topology::TopologyContainer*>(meshTopology) != nullptr)
    {
        msg_info() << "Topology path used: '" << l_topology.getLinkedPath() << "'";

        d_indices.createTopologyHandler(meshTopology);
        d_indices.addTopologyEventCallBack(sofa::core::topology::TopologyChangeType::ENDING_EVENT, [this](const core::topology::TopologyChange* eventTopo) {
            updateMassOnResize(d_indices.getValue().size());
        });
    }

    //If user defines the vertexMass, use this as the mass
    if (d_vertexMass.isSet())
    {
        //Check double definition : both totalMass and vertexMass are user-defined
        if (d_totalMass.isSet())
        {
            msg_warning(this) << "totalMass value overriding the value of the attribute vertexMass. \n"
                                 "vertexMass = totalMass / nb_dofs. \n"
                                 "To remove this warning you need to set either totalMass or vertexMass data field, but not both.";
            checkTotalMassInit();
            initFromTotalMass();
        }
        else
        {
            if(checkVertexMass())
            {
                initFromVertexMass();
            }
            else
            {
                checkTotalMassInit();
                initFromTotalMass();
            }
        }
    }
    //else totalMass is used
    else
    {
        if(!d_totalMass.isSet())
        {
            msg_info() << "No information about the mass is given. Default totatMass is used as reference.";
        }

        checkTotalMassInit();
        initFromTotalMass();
    }

    this->trackInternalData(d_vertexMass);
    this->trackInternalData(d_totalMass);

    //Info post-init
    msg_info() << "totalMass  = " << d_totalMass.getValue() << " \n"
                  "vertexMass = " << d_vertexMass.getValue();
}


template <class DataTypes, class MassType>
void UniformMass<DataTypes, MassType>::reinit()
{
    // Now update is handled through the doUpdateInternal mechanism
    // called at each begin of step through the UpdateInternalDataVisitor
}


template <class DataTypes, class MassType>
void UniformMass<DataTypes, MassType>::doUpdateInternal()
{
    if (this->hasDataChanged(d_totalMass))
    {
        if(checkTotalMass())
        {
            initFromTotalMass();
            this->d_componentState.setValue(sofa::core::objectmodel::ComponentState::Valid);
        }
        else
        {
            msg_error() << "doUpdateInternal: incorrect update from totalMass";
            this->d_componentState.setValue(sofa::core::objectmodel::ComponentState::Invalid);
        }
    }
    else if(this->hasDataChanged(d_vertexMass))
    {
        if(checkVertexMass())
        {
            initFromVertexMass();
            this->d_componentState.setValue(sofa::core::objectmodel::ComponentState::Valid);
        }
        else
        {
            msg_error() << "doUpdateInternal: incorrect update from vertexMass";
            this->d_componentState.setValue(sofa::core::objectmodel::ComponentState::Invalid);
        }
    }

    //Info post-reinit
    msg_info() << "totalMass  = " << d_totalMass.getValue() << " \n"
                  "vertexMass = " << d_vertexMass.getValue();
}


template <class DataTypes, class MassType>
bool UniformMass<DataTypes, MassType>::checkVertexMass()
{
    if(d_vertexMass.getValue() < 0.0 )
    {
        msg_warning(this) << "vertexMass data can not have a negative value. \n"
                             "To remove this warning, you need to set one single, non-zero and positive value to the vertexMass data";
        return false;
    }
    else
    {
        return true;
    }
}


template <class DataTypes, class MassType>
void UniformMass<DataTypes, MassType>::initFromVertexMass()
{
    //If the vertexMass attribute is set then the totalMass is computed from it
    //using the following formula: totalMass = vertexMass * number of particules
    auto size = d_indices.getValue().size();
    SReal vertexMass = SReal(d_vertexMass.getValue());
    SReal totalMass = vertexMass * SReal(size);
    d_totalMass.setValue(totalMass);
    msg_info() << "vertexMass information is used";
}


template <class DataTypes, class MassType>
bool UniformMass<DataTypes, MassType>::checkTotalMass()
{
    if(d_totalMass.getValue() < 0.0)
    {
        msg_warning(this) << "totalMass data can not have a negative value. \n"
                             "To remove this warning, you need to set a non-zero positive value to the totalMass data";
        return false;
    }
    else
    {
        return true;
    }
}


template <class DataTypes, class MassType>
void UniformMass<DataTypes, MassType>::checkTotalMassInit()
{
    //Check for negative or null value, if wrongly set use the default value totalMass = 1.0
    if(!checkTotalMass())
    {
        msg_warning(this) << "Switching back to default values: totalMass = 1.0\n";
        d_totalMass.setValue(1.0) ;
        this->d_componentState.setValue(sofa::core::objectmodel::ComponentState::Invalid);
    }
}


template <class DataTypes, class MassType>
void UniformMass<DataTypes, MassType>::initFromTotalMass()
{
    //If the totalMass attribute is set then the vertexMass is computed from it
    //using the following formula: vertexMass = totalMass / number of particules

    if(d_indices.getValue().size() > 0)
    {
        MassType *m = d_vertexMass.beginEdit();
        *m = d_totalMass.getValue() / Real(d_indices.getValue().size());
        d_vertexMass.endEdit();

        msg_info() << "totalMass information is used";
    }
    else
    {
        msg_warning() << "indices vector size is <= 0";
    }
}


template <class DataTypes, class MassType>
void UniformMass<DataTypes, MassType>::updateMassOnResize(sofa::Size newSize)
{
    if (newSize == 0)
    {
        d_vertexMass.setValue(static_cast<MassType>(0.0));
        d_totalMass.setValue(Real(0));
        return;
    }

    if (d_preserveTotalMass.getValue())
    {
        Real newVertexMass = d_totalMass.getValue() / Real(newSize);
        d_vertexMass.setValue(static_cast<MassType>(newVertexMass));
    }
    else
    {
        d_totalMass.setValue(Real(newSize) * Real(d_vertexMass.getValue()));
    }
}


// -- Mass interface
template <class DataTypes, class MassType>
void UniformMass<DataTypes, MassType>::addMDx ( const core::MechanicalParams*,
                                                DataVecDeriv& vres,
                                                const DataVecDeriv& vdx,
                                                SReal factor)
{
    helper::WriteAccessor<DataVecDeriv> res = vres;
    helper::ReadAccessor<DataVecDeriv> dx = vdx;

    WriteAccessor<Data<SetIndexArray > > indices = d_indices;

    MassType m = d_vertexMass.getValue();
    if ( factor != 1.0 )
        m *= typename DataTypes::Real(factor);

    for ( unsigned int i=0; i<indices.size(); i++ )
        res[indices[i]] += dx[indices[i]] * m;
}


template <class DataTypes, class MassType>
void UniformMass<DataTypes, MassType>::accFromF ( const core::MechanicalParams*,
                                                  DataVecDeriv& va,
                                                  const DataVecDeriv& vf )
{
    WriteOnlyAccessor<DataVecDeriv> a = va;
    ReadAccessor<DataVecDeriv> f = vf;

    ReadAccessor<Data<SetIndexArray > > indices = d_indices;

    MassType m = d_vertexMass.getValue();
    for ( unsigned int i=0; i<indices.size(); i++ )
        a[indices[i]] = f[indices[i]] / m;
}


template <class DataTypes, class MassType>
void UniformMass<DataTypes, MassType>::addMDxToVector ( BaseVector * resVect,
                                                        const VecDeriv* dx,
                                                        SReal mFact,
                                                        unsigned int& offset )
{
    SOFA_UNUSED(resVect);
    SOFA_UNUSED(dx);
    SOFA_UNUSED(mFact);
    SOFA_UNUSED(offset);
}


template <class DataTypes, class MassType>
void UniformMass<DataTypes, MassType>::addGravityToV(const MechanicalParams* mparams,
                                                     DataVecDeriv& d_v)
{
    if (mparams)
    {
        VecDeriv& v = *d_v.beginEdit();

        const SReal* g = getContext()->getGravity().ptr();
        Deriv theGravity;
        DataTypes::set ( theGravity, g[0], g[1], g[2] );
        Deriv hg = theGravity * Real(sofa::core::mechanicalparams::dt(mparams));

        dmsg_info()<< " addGravityToV hg = "<<theGravity<<"*"<<sofa::core::mechanicalparams::dt(mparams)<<"="<<hg ;

        for ( unsigned int i=0; i<v.size(); i++ )
        {
            v[i] += hg;
        }

        d_v.endEdit();
    }
}

template <class DataTypes, class MassType>
void UniformMass<DataTypes, MassType>::addForce ( const core::MechanicalParams*, DataVecDeriv& vf, const DataVecCoord& /*x*/, const DataVecDeriv& /*v*/ )
{
    //if gravity was added separately (in solver's "solve" method), then nothing to do here
    if ( this->m_separateGravity.getValue() )
        return;

    helper::WriteAccessor<DataVecDeriv> f = vf;

    // weight
    const SReal* g = getContext()->getGravity().ptr();
    Deriv theGravity;
    DataTypes::set
    ( theGravity, g[0], g[1], g[2] );
    const MassType& m = d_vertexMass.getValue();
    Deriv mg = theGravity * m;

    dmsg_info() <<" addForce, mg = "<<d_vertexMass<<" * "<<theGravity<<" = "<<mg;



    ReadAccessor<Data<SetIndexArray > > indices = d_indices;

    // add weight and inertia force
    if (this->m_separateGravity.getValue()) for ( unsigned int i=0; i<indices.size(); i++ )
    {
    }
    else for ( unsigned int i=0; i<indices.size(); i++ )
    {
        f[indices[i]] += mg;
    }
}

template <class DataTypes, class MassType>
SReal UniformMass<DataTypes, MassType>::getKineticEnergy ( const MechanicalParams* params,
                                                           const DataVecDeriv& d_v  ) const
{
    SOFA_UNUSED(params);

    ReadAccessor<DataVecDeriv> v = d_v;
    ReadAccessor<Data<SetIndexArray > > indices = d_indices;

    SReal e = 0;
    const MassType& m = d_vertexMass.getValue();

    for ( unsigned int i=0; i<indices.size(); i++ )
        e+= v[indices[i]]*m*v[indices[i]];

    return e/2;
}

template <class DataTypes, class MassType>
SReal UniformMass<DataTypes, MassType>::getPotentialEnergy ( const MechanicalParams* params,
                                                             const DataVecCoord& d_x  ) const
{
    SOFA_UNUSED(params);
    ReadAccessor<DataVecCoord> x = d_x;
    ReadAccessor<Data<SetIndexArray > > indices = d_indices;

    SReal e = 0;
    const MassType& m = d_vertexMass.getValue();

    Vec3d g( getContext()->getGravity());
    Deriv gravity;
    DataTypes::set(gravity, g[0], g[1], g[2]);

    Deriv mg = gravity * m;

    for ( unsigned int i=0; i<indices.size(); i++ )
        e -= mg*x[indices[i]];

    return e;
}


// does nothing by default, need to be specialized in .cpp
template <class DataTypes, class MassType>
type::Vector6
UniformMass<DataTypes, MassType>::getMomentum ( const core::MechanicalParams* params,
                                                const DataVecCoord& d_x,
                                                const DataVecDeriv& d_v  ) const
{
    SOFA_UNUSED(params);
    SOFA_UNUSED(d_x);
    SOFA_UNUSED(d_v);

    msg_warning(this) << "You are using the getMomentum function that has not been implemented"
                         "for the template '"<< this->getTemplateName() << "'.\n" ;

    return type::Vector6();
}


/// Add Mass contribution to global Matrix assembling
template <class DataTypes, class MassType>
void UniformMass<DataTypes, MassType>::addMToMatrix (const MechanicalParams *mparams,
                                                     const MultiMatrixAccessor* matrix)
{
    const MassType& m = d_vertexMass.getValue();

    const size_t N = DataTypeInfo<Deriv>::size();

    AddMToMatrixFunctor<Deriv,MassType> calc;
    MultiMatrixAccessor::MatrixRef r = matrix->getMatrix(mstate);

    Real mFactor = Real(sofa::core::mechanicalparams::mFactorIncludingRayleighDamping(mparams, this->rayleighMass.getValue()));

    ReadAccessor<Data<SetIndexArray > > indices = d_indices;
    for ( unsigned int i=0; i<indices.size(); i++ )
        calc ( r.matrix, m, int(r.offset + N*indices[i]), mFactor);
}


template <class DataTypes, class MassType>
SReal UniformMass<DataTypes, MassType>::getElementMass (sofa::Index ) const
{
    return (SReal ( d_vertexMass.getValue() ));
}


template <class DataTypes, class MassType>
void UniformMass<DataTypes, MassType>::getElementMass (sofa::Index  index ,
                                                        BaseMatrix *m ) const
{
    SOFA_UNUSED(index);

    static const BaseMatrix::Index dimension = BaseMatrix::Index(DataTypeInfo<Deriv>::size());
    if ( m->rowSize() != dimension || m->colSize() != dimension )
        m->resize ( dimension, dimension );

    m->clear();
    AddMToMatrixFunctor<Deriv,MassType>() ( m, d_vertexMass.getValue(), 0, 1 );
}


template <class DataTypes, class MassType>
void UniformMass<DataTypes, MassType>::draw(const VisualParams* vparams)
{
    if ( !vparams->displayFlags().getShowBehaviorModels() )
        return;

    if (!mstate.get())
        return;

    if (!d_showCenterOfGravity.getValue())
        return;

    ReadAccessor<VecCoord> x = mstate->read(ConstVecCoordId::position())->getValue();
    ReadAccessor<Data<SetIndexArray > > indices = d_indices;

    Coord gravityCenter;
    std::vector<  sofa::type::Vector3 > points;
    for ( unsigned int i=0; i<indices.size(); i++ )
    {
        sofa::type::Vector3 p;
        p = DataTypes::getCPos(x[indices[i]]);

        points.push_back ( p );        
        gravityCenter += x[indices[i]];
    }
    vparams->drawTool()->drawSpheres(points, 0.01, sofa::type::RGBAColor::yellow());
    
    {
        gravityCenter /= indices.size();
        const sofa::type::RGBAColor color = sofa::type::RGBAColor::yellow();

        Real axisSize = d_showAxisSize.getValue();
        sofa::type::Vector3 temp;

        for ( unsigned int i=0 ; i<3 ; i++ )
            if(i < Coord::spatial_dimensions )
                temp[i] = gravityCenter[i];

        vparams->drawTool()->drawCross(temp, float(axisSize), color);
    }
}


template<class DataTypes, class MassType>
void UniformMass<DataTypes, MassType>::loadRigidMass( const std::string&  filename)
{
    msg_warning(this) << "The attribute filename is set to ["<< filename << "] while " << msgendl <<
                         " the current object is not based on a Rigid template. It is thus ignored. " << msgendl <<
                         "To remove this warning you can: " << msgendl <<
                         "  - remove the filename attribute from <UniformMass filename='"<< filename << "'/>." << msgendl <<
                         "  - use a Rigid mechanical object instead of a VecXX one." ;
}

} // namespace sofa::component::mass
