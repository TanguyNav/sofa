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
#define SOFA_COMPONENT_CONSTRAINTSET_UNCOUPLEDCONSTRAINTCORRECTION_CPP

#include <SofaConstraint/UncoupledConstraintCorrection.inl>
#include <sofa/defaulttype/VecTypes.h>
#include <sofa/core/ObjectFactory.h>
#include <sofa/defaulttype/RigidTypes.h>
#include <sofa/simulation/Node.h>
#include <SofaBaseMechanics/UniformMass.h>

namespace sofa::component::constraintset
{

using namespace sofa::defaulttype;

template<>
SOFA_SOFACONSTRAINT_API void UncoupledConstraintCorrection< defaulttype::Rigid3Types >::init()
{
    Inherit::init();


    const VecReal& comp = compliance.getValue();

    VecReal usedComp;
    double  odeFactor = 1.0;

    this->getContext()->get(m_pOdeSolver);
    if (!m_pOdeSolver)
    {
        if (d_useOdeSolverIntegrationFactors.getValue() == true)
        {
            msg_error() << "Can't find any odeSolver";
            d_useOdeSolverIntegrationFactors.setValue(false);
        }
        d_useOdeSolverIntegrationFactors.setReadOnly(true);
    }
    else
    {
        if( !d_useOdeSolverIntegrationFactors.getValue() )
        {
            const double dt = this->getContext()->getDt();
            odeFactor = dt*dt; // W = h^2 * JMinvJt : only correct when solving in constraint equation in position. Must be deprecated.
        }
    }


    if (comp.size() != 7)
    {
        using sofa::component::mass::UniformMass;
        using sofa::defaulttype::Rigid3Types;
        using sofa::defaulttype::Rigid3Mass;
        using sofa::simulation::Node;

        Node *node = dynamic_cast< Node * >(getContext());
        Rigid3Mass massValue;

        //Should use the BaseMatrix API to get the Mass
        if (node != nullptr)
        {
            core::behavior::BaseMass *m = node->mass;
            UniformMass< Rigid3Types, Rigid3Mass > *um = dynamic_cast< UniformMass< Rigid3Types, Rigid3Mass >* > (m);

            if (um)
                massValue = um->getVertexMass();
            else
                msg_warning() << "No mass found.";
        }
        else
        {
            msg_warning() << "Node is not found => massValue could be incorrect in addComplianceInConstraintSpace function.";
        }
        

        if (defaultCompliance.isSet())
        {
            usedComp.push_back(defaultCompliance.getValue());
        }
        else
        {
            usedComp.push_back(odeFactor / massValue.mass);
        }

        usedComp.push_back( odeFactor * massValue.invInertiaMassMatrix[0][0]);
        usedComp.push_back( odeFactor * massValue.invInertiaMassMatrix[0][1]);
        usedComp.push_back( odeFactor * massValue.invInertiaMassMatrix[0][2]);
        usedComp.push_back( odeFactor * massValue.invInertiaMassMatrix[1][1]);
        usedComp.push_back( odeFactor * massValue.invInertiaMassMatrix[1][2]);
        usedComp.push_back( odeFactor * massValue.invInertiaMassMatrix[2][2]);
        compliance.setValue(usedComp);
    }
    else
    {
        msg_info() << "COMPLIANCE VALUE FOUND";
    }
    

}


template<>
SOFA_SOFACONSTRAINT_API void UncoupledConstraintCorrection< defaulttype::Rigid3Types >::getComplianceMatrix(linearalgebra::BaseMatrix *m) const
{
    const VecReal& comp = compliance.getValue();
    const unsigned int dimension = defaulttype::DataTypeInfo<Deriv>::size();
    const unsigned int numDofs = comp.size() / 7;

    m->resize(dimension * numDofs, dimension * numDofs);

    /// @todo Optimization
    for (unsigned int d = 0; d < numDofs; ++d)
    {
        const unsigned int d6 = 6 * d;
        const unsigned int d7 = 7 * d;
        const SReal invM = comp[d7];

        m->set(d6, d6, invM);
        m->set(d6 + 1, d6 + 1, invM);
        m->set(d6 + 2, d6 + 2, invM);

        m->set(d6 + 3, d6 + 3, comp[d7 + 1]);

        m->set(d6 + 3, d6 + 4, comp[d7 + 2]);
        m->set(d6 + 4, d6 + 3, comp[d7 + 2]);

        m->set(d6 + 3, d6 + 5, comp[d7 + 3]);
        m->set(d6 + 5, d6 + 3, comp[d7 + 3]);

        m->set(d6 + 4, d6 + 4, comp[d7 + 4]);

        m->set(d6 + 4, d6 + 5, comp[d7 + 5]);
        m->set(d6 + 5, d6 + 4, comp[d7 + 5]);

        m->set(d6 + 5, d6 + 5, comp[d7 + 6]);
    }
}


int UncoupledConstraintCorrectionClass = core::RegisterObject("Component computing constraint forces within a simulated body using the compliance method.")
        .add< UncoupledConstraintCorrection< Vec1Types > >()
        .add< UncoupledConstraintCorrection< Vec2Types > >()
        .add< UncoupledConstraintCorrection< Vec3Types > >()
        .add< UncoupledConstraintCorrection< Rigid3Types > >()
    ;

template class SOFA_SOFACONSTRAINT_API UncoupledConstraintCorrection< Vec1Types >;
template class SOFA_SOFACONSTRAINT_API UncoupledConstraintCorrection< Vec2Types >;
template class SOFA_SOFACONSTRAINT_API UncoupledConstraintCorrection< Vec3Types >;
template class SOFA_SOFACONSTRAINT_API UncoupledConstraintCorrection< Rigid3Types >;


} //namespace sofa::component::constraintset
