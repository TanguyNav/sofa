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
#include <SofaSimulationCommon/xml/Element.h>
#include <sofa/core/objectmodel/BaseObject.h>

namespace sofa::simulation::xml
{

class SOFA_SOFASIMULATIONCOMMON_API ObjectElement : public Element<sofa::core::objectmodel::BaseObject>
{
public:
    ObjectElement(const std::string& name, const std::string& type, BaseElement* parent=nullptr);

    ~ObjectElement() override;

    bool initNode() override;

    bool init() override;

    virtual const char* getClass() const override;
};

} // namespace sofa::simulation::xml
