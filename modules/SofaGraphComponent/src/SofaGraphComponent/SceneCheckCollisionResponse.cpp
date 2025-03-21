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
#include "SceneCheckCollisionResponse.h"

#include <sofa/simulation/Node.h>
#include <SofaBaseCollision/DefaultContactManager.h>
#include <sofa/core/behavior/BaseAnimationLoop.h>
#include <sofa/core/behavior/ConstraintSolver.h>

namespace sofa::simulation::_scenechecking_
{

using sofa::simulation::Node;

const std::string SceneCheckCollisionResponse::getName()
{
    return "SceneCheckCollisionResponse";
}

const std::string SceneCheckCollisionResponse::getDesc()
{
    return "Check that the appropriate components are in the scene to compute the desired collision response";
}

void SceneCheckCollisionResponse::doInit(Node* node)
{
    SOFA_UNUSED(node);
    m_message.str("");
}

void SceneCheckCollisionResponse::doCheckOn(Node* node)
{
    if(m_checkDone)
        return;

    const sofa::core::objectmodel::BaseContext* root = node->getContext()->getRootContext();
    std::vector<sofa::component::collision::DefaultContactManager*> contactManager;
    root->get<sofa::component::collision::DefaultContactManager>(&contactManager, core::objectmodel::BaseContext::SearchDown);
    m_checkDone=true;
    const Size nbContactManager = contactManager.size();
    if( nbContactManager  > 0 )
    {
        if( nbContactManager!= 1 )
        {
            m_message << "Only one DefaultContactManager is needed."<< msgendl;
        }
        else
        {
            const std::string response = contactManager[0]->response.getValue().getSelectedItem();

            if ( response == "StickContactConstraint" )
            {
                sofa::core::behavior::BaseAnimationLoop* animationLoop;
                root->get(animationLoop, sofa::core::objectmodel::BaseContext::SearchRoot);
                if (!animationLoop || ( animationLoop && ( animationLoop->getClassName() != "FreeMotionAnimationLoop" )) )
                {
                    m_message <<"A FreeMotionAnimationLoop must be in the scene to solve StickContactConstraint" << msgendl;
                }

                sofa::core::behavior::ConstraintSolver* constraintSolver;
                root->get(constraintSolver, sofa::core::objectmodel::BaseContext::SearchRoot);
                if (!constraintSolver || ( constraintSolver && ( constraintSolver->getClassName() != "GenericConstraintSolver" )) )
                {
                    m_message <<"A GenericConstraintSolver must be in the scene to solve StickContactConstraint" << msgendl;
                }
            }
            /// If StickContactConstraint is chosen, make sure the scene includes a FreeMotionAnimationLoop and a GenericConstraintSolver (specifically)
            else if ( response == "FrictionContactConstraint")
            {
                sofa::core::behavior::BaseAnimationLoop* animationLoop;
                root->get(animationLoop, sofa::core::objectmodel::BaseContext::SearchRoot);
                if (!animationLoop || ( animationLoop && ( animationLoop->getClassName() != "FreeMotionAnimationLoop" )) )
                {
                    m_message <<"A FreeMotionAnimationLoop must be in the scene to solve FrictionContactConstraint" << msgendl;
                }
            }
        }
    }
}

void SceneCheckCollisionResponse::doPrintSummary()
{
    if(m_checkDone && m_message.str()!= "")
    {
        msg_warning(this->getName()) << m_message.str();
    }
}


} // namespace sofa::simulation::_scenechecking_
