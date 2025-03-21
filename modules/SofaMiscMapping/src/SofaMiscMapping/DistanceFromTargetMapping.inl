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

#include "DistanceFromTargetMapping.h"
#include <sofa/core/behavior/MechanicalState.h>
#include <sofa/core/visual/VisualParams.h>
#include <sofa/core/MechanicalParams.h>
#include <sofa/type/RGBAColor.h>
#include <iostream>

namespace sofa::component::mapping
{

template <class TIn, class TOut>
DistanceFromTargetMapping<TIn, TOut>::DistanceFromTargetMapping()
    : Inherit()
    , f_indices(initData(&f_indices, "indices", "Indices of the parent points"))
    , f_targetPositions(initData(&f_targetPositions, "targetPositions", "Positions to compute the distances from"))
    , f_restDistances(initData(&f_restDistances, "restLengths", "Rest lengths of the connections."))
    //TODO(dmarchal): use a list of options instead of numeric values.
    , d_geometricStiffness(initData(&d_geometricStiffness, (unsigned)2, "geometricStiffness", "0 -> no GS, 1 -> exact GS, 2 -> stabilized GS (default)"))
    , d_showObjectScale(initData(&d_showObjectScale, 0.f, "showObjectScale", "Scale for object display"))
    , d_color(initData(&d_color, sofa::type::RGBAColor(1,1,0,1), "showColor", "Color for object display. (default=[1.0,1.0,0.0,1.0])"))
{
}

template <class TIn, class TOut>
DistanceFromTargetMapping<TIn, TOut>::~DistanceFromTargetMapping()
{
}

template <class TIn, class TOut>
void DistanceFromTargetMapping<TIn, TOut>::createTarget(unsigned index, const InCoord &position, Real distance)
{
    helper::WriteAccessor< Data< type::vector<Real> > > distances(f_restDistances);
    helper::WriteAccessor< Data<type::vector<unsigned> > > indices(f_indices);
    helper::WriteAccessor< Data<InVecCoord > > targetPositions(f_targetPositions);

    indices.push_back(index);
    targetPositions.push_back(position);
    distances.push_back(distance);
}

template <class TIn, class TOut>
void DistanceFromTargetMapping<TIn, TOut>::updateTarget(unsigned index, const InCoord &position)
{
    helper::WriteAccessor< Data<InVecCoord > > targetPositions(f_targetPositions);
    helper::WriteAccessor< Data<type::vector<unsigned> > > indices(f_indices);

    // find the target with given index
    unsigned i=0; while(i<indices.size() && indices[i]!=index) i++;

    targetPositions[i] = position;
}

template <class TIn, class TOut>
void DistanceFromTargetMapping<TIn, TOut>::updateTarget(unsigned index, SReal x, SReal y, SReal z)
{
    InCoord pos;
    TIn::set( pos, x, y, z );
    updateTarget( index, pos );
}

template <class TIn, class TOut>
void DistanceFromTargetMapping<TIn, TOut>::clear()
{
    helper::WriteAccessor< Data< type::vector<Real> > > distances(f_restDistances);
    helper::WriteAccessor< Data<InVecCoord > > positions(f_targetPositions);
    helper::WriteAccessor< Data<type::vector<unsigned> > > indices(f_indices);

    distances.clear();
    positions.clear();
    indices.clear();

    this->getToModel()->resize( 0 );
}



template <class TIn, class TOut>
void DistanceFromTargetMapping<TIn, TOut>::init()
{
    assert( f_indices.getValue().size()==f_targetPositions.getValue().size()) ;

    // unset distances are set to 0
    if(f_restDistances.getValue().size() != f_indices.getValue().size())
    {
        helper::WriteAccessor< Data< type::vector<Real> > > distances(f_restDistances);
        unsigned prevsize = distances.size();
        distances.resize( f_indices.getValue().size() );
        for(unsigned i=prevsize; i<distances.size(); i++ )
            distances[i] = 0;
    }

    this->getToModel()->resize( f_indices.getValue().size() );


    baseMatrices.resize( 1 );
    baseMatrices[0] = &jacobian;

    this->Inherit::init();  // applies the mapping, so after the Data init
}

template <class TIn, class TOut>
void DistanceFromTargetMapping<TIn, TOut>::computeCoordPositionDifference( Direction& r, const InCoord& a, const InCoord& b )
{
    r = TIn::getCPos(b)-TIn::getCPos(a);
}

template <class TIn, class TOut>
void DistanceFromTargetMapping<TIn, TOut>::apply(const core::MechanicalParams * /*mparams*/ , Data<OutVecCoord>& dOut, const Data<InVecCoord>& dIn)
{
    helper::WriteAccessor< Data<OutVecCoord> >  out = dOut;
    helper::ReadAccessor< Data<InVecCoord> >  in = dIn;
    helper::WriteAccessor<Data<type::vector<Real> > > restDistances(f_restDistances);
    helper::ReadAccessor< Data<type::vector<unsigned> > > indices(f_indices);
    helper::ReadAccessor< Data<InVecCoord > > targetPositions(f_targetPositions);

    jacobian.resizeBlocks(out.size(),in.size());
    directions.resize(out.size());
    invlengths.resize(out.size());


    for(unsigned i=0; i<indices.size() ; i++ )
    {
        Direction& gap = directions[i];

        // gap = in[indices[i]] - targetPositions[i] (only for position)
        computeCoordPositionDifference( gap, targetPositions[i], in[indices[i]] );

        Real gapNorm = gap.norm();
        out[i] = gapNorm - restDistances[i];  // output

        if( gapNorm>1.e-10 )
        {
            invlengths[i] = 1/gapNorm;
            gap *= invlengths[i];
        }
        else
        {
            invlengths[i] = 0;
            gap = Direction();
            gap[0]=1.0;  // arbitrary unit vector
        }

        for(unsigned j=0; j<Nout; j++)
        {
            jacobian.beginRow(i*Nout+j);
            for(unsigned k=0; k<Nin; k++ )
            {
                jacobian.insertBack( i*Nout+j, indices[i]*Nin+k, gap[k] );
            }
        }
    }

    jacobian.compress();
}



template <class TIn, class TOut>
void DistanceFromTargetMapping<TIn, TOut>::applyJ(const core::MechanicalParams * /*mparams*/ , Data<OutVecDeriv>& dOut, const Data<InVecDeriv>& dIn)
{
    if( jacobian.rowSize() > 0 )
    {
        auto dOutWa = sofa::helper::getWriteOnlyAccessor(dOut);
        auto dInRa = sofa::helper::getReadAccessor(dIn);
        jacobian.mult(dOutWa.wref(),dInRa.ref());
    }

}

template <class TIn, class TOut>
void DistanceFromTargetMapping<TIn, TOut>::applyJT(const core::MechanicalParams * /*mparams*/ , Data<InVecDeriv>& dIn, const Data<OutVecDeriv>& dOut)
{
    if( jacobian.rowSize() > 0 )
    {
        auto dOutRa = sofa::helper::getReadAccessor(dOut);
        auto dInWa = sofa::helper::getWriteOnlyAccessor(dIn);
        jacobian.addMultTranspose(dInWa.wref(),dOutRa.ref());
    }
}

template <class TIn, class TOut>
void DistanceFromTargetMapping<TIn, TOut>::applyJT(const core::ConstraintParams*, Data<InMatrixDeriv>& , const Data<OutMatrixDeriv>& )
{
}


template <class TIn, class TOut>
void DistanceFromTargetMapping<TIn, TOut>::applyDJT(const core::MechanicalParams* mparams, core::MultiVecDerivId parentDfId, core::ConstMultiVecDerivId )
{
    const unsigned& geometricStiffness = d_geometricStiffness.getValue();
     if( !geometricStiffness ) return;

    helper::WriteAccessor<Data<InVecDeriv> > parentForce (*parentDfId[this->fromModel.get()].write());
    helper::ReadAccessor<Data<InVecDeriv> > parentDisplacement (*mparams->readDx(this->fromModel));  // parent displacement
    const SReal kfactor = mparams->kFactor();
    helper::ReadAccessor<Data<OutVecDeriv> > childForce (*mparams->readF(this->toModel));
    helper::ReadAccessor< Data<type::vector<unsigned> > > indices(f_indices);

    for(unsigned i=0; i<indices.size(); i++ )
    {
        // force in compression (>0) can lead to negative eigen values in geometric stiffness
        // this results in a undefinite implicit matrix that causes instabilies
        // if stabilized GS (geometricStiffness==2) -> keep only force in extension
        if( childForce[i][0] < 0 || geometricStiffness==1 )
        {
            sofa::type::Mat<Nin,Nin,Real> b;  // = (I - uu^T)
            for(unsigned j=0; j<Nin; j++)
            {
                for(unsigned k=0; k<Nin; k++)
                {
                    if( j==k )
                        b[j][k] = 1.f - directions[i][j]*directions[i][k];
                    else
                        b[j][k] =     - directions[i][j]*directions[i][k];
                }
            }
            // (I - uu^T)*f/l*kfactor  --  do not forget kfactor !
            b *= (Real)(childForce[i][0] * invlengths[i] * kfactor);
            // note that computing a block is not efficient here, but it would
            // makes sense for storing a stiffness matrix

            InDeriv dx = parentDisplacement[indices[i]];
            InDeriv df;
            for(unsigned j=0; j<Nin; j++)
            {
                for(unsigned k=0; k<Nin; k++)
                {
                    df[j]+=b[j][k]*dx[k];
                }
            }
           // InDeriv df = b*dx;
            parentForce[indices[i]] += df;
        }
    }
}




template <class TIn, class TOut>
const sofa::linearalgebra::BaseMatrix* DistanceFromTargetMapping<TIn, TOut>::getJ()
{
    return &jacobian;
}

template <class TIn, class TOut>
const type::vector<sofa::linearalgebra::BaseMatrix*>* DistanceFromTargetMapping<TIn, TOut>::getJs()
{
    return &baseMatrices;
}

template <class TIn, class TOut>
const linearalgebra::BaseMatrix* DistanceFromTargetMapping<TIn, TOut>::getK()
{
    return &K;
}

template <class TIn, class TOut>
void DistanceFromTargetMapping<TIn, TOut>::updateK( const core::MechanicalParams* mparams, core::ConstMultiVecDerivId childForceId )
{
    SOFA_UNUSED(mparams);
    const unsigned& geometricStiffness = d_geometricStiffness.getValue();
    if( !geometricStiffness ) { K.resize(0,0); return; }

    helper::ReadAccessor<Data<OutVecDeriv> > childForce( *childForceId[this->toModel.get()].read() );
    helper::ReadAccessor< Data<type::vector<unsigned> > > indices(f_indices);
    helper::ReadAccessor<Data<InVecCoord> > in (*this->fromModel->read(core::ConstVecCoordId::position()));

    K.resizeBlocks(in.size(),in.size());
    for(size_t i=0; i<indices.size(); i++)
    {
        // force in compression (>0) can lead to negative eigen values in geometric stiffness
        // this results in a undefinite implicit matrix that causes instabilies
        // if stabilized GS (geometricStiffness==2) -> keep only force in extension
        if( childForce[i][0] < 0 || geometricStiffness==1 )
        {
            size_t idx = indices[i];

            sofa::type::Mat<Nin,Nin,Real> b;  // = (I - uu^T)
            for(unsigned j=0; j<Nin; j++)
            {
                for(unsigned k=0; k<Nin; k++)
                {
                    if( j==k )
                        b[j][k] = 1.f - directions[i][j]*directions[i][k];
                    else
                        b[j][k] =     - directions[i][j]*directions[i][k];
                }
            }
            b *= childForce[i][0] * invlengths[i];  // (I - uu^T)*f/l

            K.addBlock(idx,idx,b);
        }
    }
    K.compress();
}



template <class TIn, class TOut>
void DistanceFromTargetMapping<TIn, TOut>::draw(const core::visual::VisualParams* vparams)
{
    float arrowsize = d_showObjectScale.getValue();
    if( arrowsize<0 ) return;

    typename core::behavior::MechanicalState<In>::ReadVecCoord pos = this->getFromModel()->readPositions();
    helper::ReadAccessor< Data<InVecCoord > > targetPositions(f_targetPositions);
    helper::ReadAccessor< Data<type::vector<unsigned> > > indices(f_indices);

    type::vector< sofa::type::Vector3 > points;

    for(unsigned i=0; i<indices.size(); i++ )
    {
        points.push_back( sofa::type::Vector3(TIn::getCPos(targetPositions[i]) ) );
        points.push_back( sofa::type::Vector3(TIn::getCPos(pos[indices[i]]) ) );
    }

    if( !arrowsize )
        vparams->drawTool()->drawLines ( points, 1, d_color.getValue() );
    else
        for (unsigned int i=0; i<points.size()/2; ++i)
            vparams->drawTool()->drawArrow( points[2*i+1], points[2*i], arrowsize, d_color.getValue() );

}

} // namespace sofa::component::mapping
