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
#define SOFA_SIMULATION_MECHANICALVISITOR_MECHANICALVALLOCVISITOR_CPP
#include <sofa/simulation/mechanicalvisitor/MechanicalVAllocVisitor.h>

#include <sofa/core/behavior/BaseMechanicalState.h>

namespace sofa::simulation::mechanicalvisitor
{

template< sofa::core::VecType vtype>
Visitor::Result MechanicalVAllocVisitor<vtype>::fwdMechanicalState(simulation::Node* /*node*/, core::behavior::BaseMechanicalState* mm)
{
    mm->vAlloc(this->params, v.getId(mm) );
    return RESULT_CONTINUE;
}


template< sofa::core::VecType vtype>
std::string  MechanicalVAllocVisitor<vtype>::getInfos() const
{
    std::string name="[" + v.getName() + "]";
    return name;
}

template class SOFA_SIMULATION_CORE_API MechanicalVAllocVisitor<sofa::core::V_COORD>;
template class SOFA_SIMULATION_CORE_API MechanicalVAllocVisitor<sofa::core::V_DERIV>;
}