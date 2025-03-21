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
#include <SofaDeformable/SpringForceField.h>
#include <sofa/core/visual/VisualParams.h>
#include <sofa/core/topology/BaseMeshTopology.h>
#include <sofa/core/topology/TopologyChange.h>
#include <sofa/helper/io/XspLoader.h>
#include <cassert>
#include <iostream>
#include <fstream>

namespace sofa::component::interactionforcefield
{

template<class DataTypes>
SpringForceField<DataTypes>::SpringForceField(SReal _ks, SReal _kd)
    : SpringForceField(nullptr, nullptr, _ks, _kd)
{
}

template<class DataTypes>
SpringForceField<DataTypes>::SpringForceField(MechanicalState* mstate1, MechanicalState* mstate2, SReal _ks, SReal _kd)
    : Inherit(mstate1, mstate2)
    , ks(initData(&ks,_ks,"stiffness","uniform stiffness for the all springs"))
    , kd(initData(&kd,_kd,"damping","uniform damping for the all springs"))
    , showArrowSize(initData(&showArrowSize,0.01f,"showArrowSize","size of the axis"))
    , drawMode(initData(&drawMode,0,"drawMode","The way springs will be drawn:\n- 0: Line\n- 1:Cylinder\n- 2: Arrow"))
    , springs(initData(&springs,"spring","pairs of indices, stiffness, damping, rest length"))
    , maskInUse(false)
{
    this->addAlias(&fileSprings, "fileSprings");
}

template <class DataTypes>
class SpringForceField<DataTypes>::Loader : public helper::io::XspLoaderDataHook
{
public:
    SpringForceField<DataTypes>* dest;
    Loader(SpringForceField<DataTypes>* dest) : dest(dest) {}
    void addSpring(size_t m1, size_t m2, SReal ks, SReal kd, SReal initpos) override
    {
        type::vector<Spring>& springs = *dest->springs.beginEdit();
        springs.push_back(Spring(sofa::Index(m1), sofa::Index(m2),ks,kd,initpos));
        dest->springs.endEdit();
    }
};

template <class DataTypes>
bool SpringForceField<DataTypes>::load(const char *filename)
{
    bool ret = true;
    if (filename && filename[0])
    {
        Loader loader(this);
        ret &= helper::io::XspLoader::Load(filename, loader);
    }
    else ret = false;
    return ret;
}


template <class DataTypes>
void SpringForceField<DataTypes>::reinit()
{
    for (sofa::Index i=0; i<springs.getValue().size(); ++i)
    {
        (*springs.beginEdit())[i].ks = (Real) ks.getValue();
        (*springs.beginEdit())[i].kd = (Real) kd.getValue();
    }
}

template <class DataTypes>
void SpringForceField<DataTypes>::init()
{
    // Load
    if (!fileSprings.getValue().empty())
        load(fileSprings.getFullPath().c_str());
    this->Inherit::init();
}

template<class DataTypes>
void SpringForceField<DataTypes>::addSpringForce(Real& ener, VecDeriv& f1, const VecCoord& p1, const VecDeriv& v1, VecDeriv& f2, const VecCoord& p2, const VecDeriv& v2, sofa::Index /*i*/, const Spring& spring)
{
    sofa::Index a = spring.m1;
    sofa::Index b = spring.m2;
    typename DataTypes::CPos u = DataTypes::getCPos(p2[b])-DataTypes::getCPos(p1[a]);
    Real d = u.norm();
    if( spring.enabled && d<1.0e-4 ) // null length => no force
        return;
    Real inverseLength = 1.0f/d;
    u *= inverseLength;
    Real elongation = d - spring.initpos;
    ener += elongation * elongation * spring.ks /2;
    typename DataTypes::DPos relativeVelocity = DataTypes::getDPos(v2[b])-DataTypes::getDPos(v1[a]);
    Real elongationVelocity = dot(u,relativeVelocity);
    Real forceIntensity = spring.ks*elongation+spring.kd*elongationVelocity;
    typename DataTypes::DPos force = u*forceIntensity;

    DataTypes::setDPos( f1[a], DataTypes::getDPos(f1[a]) + force ) ;
    DataTypes::setDPos( f2[b], DataTypes::getDPos(f2[b]) - force ) ;
}

template<class DataTypes>
void SpringForceField<DataTypes>::addForce(
    const core::MechanicalParams* /* mparams */, DataVecDeriv& data_f1, DataVecDeriv& data_f2,
    const DataVecCoord& data_x1, const DataVecCoord& data_x2,
    const DataVecDeriv& data_v1, const DataVecDeriv& data_v2)
{

    VecDeriv&       f1 = *data_f1.beginEdit();
    const VecCoord& x1 = data_x1.getValue();
    const VecDeriv& v1 = data_v1.getValue();
    VecDeriv&       f2 = *data_f2.beginEdit();
    const VecCoord& x2 = data_x2.getValue();
    const VecDeriv& v2 = data_v2.getValue();


    const type::vector<Spring>& springs= this->springs.getValue();

    f1.resize(x1.size());
    f2.resize(x2.size());
    this->m_potentialEnergy = 0;
    for (unsigned int i=0; i<this->springs.getValue().size(); i++)
    {
        this->addSpringForce(this->m_potentialEnergy,f1,x1,v1,f2,x2,v2, i, springs[i]);
    }
    data_f1.endEdit();
    data_f2.endEdit();
}

template<class DataTypes>
void SpringForceField<DataTypes>::addDForce(const core::MechanicalParams*, DataVecDeriv&, DataVecDeriv&, const DataVecDeriv&, const DataVecDeriv& )
{
    msg_error() << "SpringForceField does not support implicit integration. Use StiffSpringForceField instead.";
}


template<class DataTypes>
SReal SpringForceField<DataTypes>::getPotentialEnergy(const core::MechanicalParams* /* PARAMS FIRST */, const DataVecCoord& data_x1, const DataVecCoord& data_x2) const
{
    const type::vector<Spring>& springs= this->springs.getValue();
    const VecCoord& p1 =  data_x1.getValue();
    const VecCoord& p2 =  data_x2.getValue();

    SReal ener = 0;

    for (sofa::Index i=0; i<springs.size(); i++)
    {
        sofa::Index a = springs[i].m1;
        sofa::Index b = springs[i].m2;
        Coord u = p2[b]-p1[a];
        Real d = u.norm();
        Real elongation = (Real)(d - springs[i].initpos);
        ener += elongation * elongation * springs[i].ks /2;
    }

    return ener;
}


template<class DataTypes>
void SpringForceField<DataTypes>::addKToMatrix(sofa::linearalgebra::BaseMatrix *, SReal, unsigned int &)
{
    msg_error() << "SpringForceField does not support implicit integration. Use StiffSpringForceField instead.";
}



template<class DataTypes>
void SpringForceField<DataTypes>::draw(const core::visual::VisualParams* vparams)
{
    using namespace sofa::defaulttype;
    using namespace sofa::type;

    if (!((this->mstate1 == this->mstate2) ? vparams->displayFlags().getShowForceFields() : vparams->displayFlags().getShowInteractionForceFields())) return;
    const VecCoord& p1 = this->mstate1->read(core::ConstVecCoordId::position())->getValue();
    const VecCoord& p2 = this->mstate2->read(core::ConstVecCoordId::position())->getValue();

    std::vector< Vector3 > points[4];
    bool external = (this->mstate1 != this->mstate2);
    const type::vector<Spring>& springs = this->springs.getValue();
    for (sofa::Index i = 0; i < springs.size(); i++)
    {
        if (!springs[i].enabled) continue;
        Real d = (p2[springs[i].m2] - p1[springs[i].m1]).norm();
        Vector3 point2, point1;
        point1 = DataTypes::getCPos(p1[springs[i].m1]);
        point2 = DataTypes::getCPos(p2[springs[i].m2]);

        if (external)
        {
            if (d < springs[i].initpos * 0.9999)
            {
                points[0].push_back(point1);
                points[0].push_back(point2);
            }
            else
            {
                points[1].push_back(point1);
                points[1].push_back(point2);
            }
        }
        else
        {
            if (d < springs[i].initpos * 0.9999)
            {
                points[2].push_back(point1);
                points[2].push_back(point2);
            }
            else
            {
                points[3].push_back(point1);
                points[3].push_back(point2);
            }
        }
    }
    const RGBAColor c0 = RGBAColor::red();
    const RGBAColor c1 = RGBAColor::green();
    const RGBAColor c2 {1.0f, 0.5f, 0.0f, 1.0f };
    const RGBAColor c3{ 0.0f, 1.0f, 0.5f, 1.0f };

    if (showArrowSize.getValue()==0 || drawMode.getValue() == 0)
    {
        vparams->drawTool()->drawLines(points[0], 1, c0);
        vparams->drawTool()->drawLines(points[1], 1, c1);
        vparams->drawTool()->drawLines(points[2], 1, c2);
        vparams->drawTool()->drawLines(points[3], 1, c3);
    }
    else if (drawMode.getValue() == 1)
    {
        const auto numLines0=points[0].size()/2;
        const auto numLines1=points[1].size()/2;
        const auto numLines2=points[2].size()/2;
        const auto numLines3=points[3].size()/2;

        for (unsigned int i=0; i<numLines0; ++i) vparams->drawTool()->drawCylinder(points[0][2*i+1], points[0][2*i], showArrowSize.getValue(), c0);
        for (unsigned int i=0; i<numLines1; ++i) vparams->drawTool()->drawCylinder(points[1][2*i+1], points[1][2*i], showArrowSize.getValue(), c1);
        for (unsigned int i=0; i<numLines2; ++i) vparams->drawTool()->drawCylinder(points[2][2*i+1], points[2][2*i], showArrowSize.getValue(), c2);
        for (unsigned int i=0; i<numLines3; ++i) vparams->drawTool()->drawCylinder(points[3][2*i+1], points[3][2*i], showArrowSize.getValue(), c3);

    }
    else if (drawMode.getValue() == 2)
    {
        const auto numLines0=points[0].size()/2;
        const auto numLines1=points[1].size()/2;
        const auto numLines2=points[2].size()/2;
        const auto numLines3=points[3].size()/2;

        for (unsigned int i=0; i<numLines0; ++i) vparams->drawTool()->drawArrow(points[0][2*i+1], points[0][2*i], showArrowSize.getValue(), c0);
        for (unsigned int i=0; i<numLines1; ++i) vparams->drawTool()->drawArrow(points[1][2*i+1], points[1][2*i], showArrowSize.getValue(), c1);
        for (unsigned int i=0; i<numLines2; ++i) vparams->drawTool()->drawArrow(points[2][2*i+1], points[2][2*i], showArrowSize.getValue(), c2);
        for (unsigned int i=0; i<numLines3; ++i) vparams->drawTool()->drawArrow(points[3][2*i+1], points[3][2*i], showArrowSize.getValue(), c3);
    }
    else
    {
        msg_error()<< "No proper drawing mode found!";
    }
}

template<class DataTypes>
void SpringForceField<DataTypes>::handleTopologyChange(core::topology::Topology *topo)
{
    if(this->mstate1->getContext()->getTopology() == topo)
    {
        core::topology::BaseMeshTopology*	_topology = topo->toBaseMeshTopology();

        if(_topology != nullptr)
        {
            std::list<const core::topology::TopologyChange *>::const_iterator itBegin=_topology->beginChange();
            std::list<const core::topology::TopologyChange *>::const_iterator itEnd=_topology->endChange();

            while( itBegin != itEnd )
            {
                core::topology::TopologyChangeType changeType = (*itBegin)->getChangeType();

                switch( changeType )
                {
                case core::topology::POINTSREMOVED:
                {

                    break;
                }

                default:
                    break;
                } // switch( changeType )

                ++itBegin;
            } // while( changeIt != last; )
        }
    }

    if(this->mstate2->getContext()->getTopology() == topo)
    {
        core::topology::BaseMeshTopology*	_topology = topo->toBaseMeshTopology();

        if(_topology != nullptr)
        {
            std::list<const core::topology::TopologyChange *>::const_iterator changeIt=_topology->beginChange();
            std::list<const core::topology::TopologyChange *>::const_iterator itEnd=_topology->endChange();

            while( changeIt != itEnd )
            {
                core::topology::TopologyChangeType changeType = (*changeIt)->getChangeType();

                switch( changeType )
                {
                case core::topology::POINTSREMOVED:
                {
                    auto nbPoints = _topology->getNbPoints();
                    const auto& tab = (static_cast<const sofa::core::topology::PointsRemoved *>(*changeIt))->getArray();

                    type::vector<Spring>& springs = *this->springs.beginEdit();
                    // springs.push_back(Spring(m1,m2,ks,kd,initpos));

                    for(sofa::Index i=0; i<tab.size(); ++i)
                    {
                        sofa::Index pntId = tab[i];
                        nbPoints -= 1;

                        for(sofa::Index j=0; j<springs.size(); ++j)
                        {
                            Spring& spring = springs[j];
                            if(spring.m2 == pntId)
                            {
                                spring = springs[springs.size() - 1];
                                springs.resize(springs.size() - 1);
                            }

                            if(spring.m2 == nbPoints)
                            {
                                spring.m2 = pntId;
                            }
                        }
                    }

                    this->springs.endEdit();

                    break;
                }

                default:
                    break;
                } // switch( changeType )

                ++changeIt;
            } // while( changeIt != last; )
        }
    }
}

template<class DataTypes>
void SpringForceField<DataTypes>::initGnuplot(const std::string path)
{
    if (!this->getName().empty())
    {
        if (m_gnuplotFileEnergy != nullptr)
        {
            m_gnuplotFileEnergy->close();
            delete m_gnuplotFileEnergy;
        }
        m_gnuplotFileEnergy = new std::ofstream( (path+this->getName()+"_PotentialEnergy.txt").c_str() );
    }
}

template<class DataTypes>
void SpringForceField<DataTypes>::exportGnuplot(SReal time)
{
    if (m_gnuplotFileEnergy!=nullptr)
    {
        (*m_gnuplotFileEnergy) << time <<"\t"<< this->m_potentialEnergy << std::endl;
    }
}

} // namespace sofa::component::interactionforcefield
