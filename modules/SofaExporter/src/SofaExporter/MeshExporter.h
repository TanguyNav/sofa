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
#ifndef SOFA_COMPONENT_MISC_MESHEXPORTER_H
#define SOFA_COMPONENT_MISC_MESHEXPORTER_H
#include <SofaExporter/config.h>

#include <sofa/core/objectmodel/BaseObject.h>
#include <sofa/core/objectmodel/DataFileName.h>
#include <sofa/defaulttype/VecTypes.h>
#include <sofa/helper/OptionsGroup.h>
#include <sofa/simulation/BaseSimulationExporter.h>

///////////////////////////// FORWARD DECLARATION //////////////////////////////////////////////////
namespace sofa {
    namespace core {
        namespace objectmodel {
            class Event ;
        }
        namespace behavior {
            class BaseMechanicalState;
        }
        namespace topology {
            class BaseMeshTopology ;
        }
    }
}



////////////////////////////////// DECLARATION /////////////////////////////////////////////////////
namespace sofa::component
{
namespace _meshexporter_
{

using sofa::core::behavior::BaseMechanicalState ;
using sofa::core::objectmodel::Event ;
using sofa::core::topology::BaseMeshTopology ;
using sofa::simulation::BaseSimulationExporter ;
using sofa::core::topology::Topology ;

class SOFA_SOFAEXPORTER_API MeshExporter : public BaseSimulationExporter
{
public:
    SOFA_CLASS(MeshExporter, BaseSimulationExporter);

public:
    Data<sofa::helper::OptionsGroup> d_fileFormat; ///< File format to use
    Data<defaulttype::Vec3Types::VecCoord> d_position; ///< points position (will use points from topology or mechanical state if this is empty)
    Data<bool> d_writeEdges; ///< write edge topology
    Data<bool> d_writeTriangles; ///< write triangle topology
    Data<bool> d_writeQuads; ///< write quad topology
    Data<bool> d_writeTetras; ///< write tetra topology
    Data<bool> d_writeHexas; ///< write hexa topology

    type::vector<std::string> pointsDataObject;
    type::vector<std::string> pointsDataField;
    type::vector<std::string> pointsDataName;

    type::vector<std::string> cellsDataObject;
    type::vector<std::string> cellsDataField;
    type::vector<std::string> cellsDataName;

    void doInit() override ;
    void doReInit() override ;
    void handleEvent(Event *) override ;

    bool write() override ;

    bool writeMesh();
    bool writeMeshVTKXML();
    bool writeMeshVTK();
    bool writeMeshGmsh();
    bool writeMeshNetgen();
    bool writeMeshTetgen();
    bool writeMeshObj();


protected:
    MeshExporter();
    ~MeshExporter() override;

    BaseMeshTopology*     m_inputtopology {nullptr};
    BaseMechanicalState*  m_inputmstate {nullptr};

    std::string getMeshFilename(const char* ext);
};

} // namespace _meshexporter_

// Import the object in the exporter namespace to avoid having all the object straight in component.
namespace exporter {
    using MeshExporter = _meshexporter_::MeshExporter;
} // namespace exporter

// Import the object in the "old" namespace to allow smooth update of code base.
namespace misc {
    using MeshExporter
        SOFA_ATTRIBUTE_DISABLED__SOFAEXPORTER_NAMESPACE_1712()
        = DeprecatedAndRemoved;
} // namespace misc

} // namespace sofa::component

#endif // SOFA_COMPONENT_MISC_MESHEXPORTER_H
