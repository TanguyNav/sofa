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
#include "CudaTypes.h"
#include "CudaSpringForceField.inl"
#include "CudaMechanicalObject.inl"
#include "CudaIdentityMapping.inl"
#include "CudaContactMapper.h"
#include "CudaPenalityContactForceField.h"
#include "CudaSpringForceField.h"
#include "CudaSphereModel.h"
#include "CudaPointModel.h"

#include <SofaUserInteraction/MouseInteractor.inl>
#include <SofaBaseCollision/NewProximityIntersection.inl>
#include <SofaMeshCollision/MeshNewProximityIntersection.inl>
#include <SofaUserInteraction/RayDiscreteIntersection.inl>
#include <SofaBaseCollision/DiscreteIntersection.h>
#include <SofaUserInteraction/ComponentMouseInteraction.inl>
#include <SofaUserInteraction/AttachBodyPerformer.inl>
#include <SofaUserInteraction/FixParticlePerformer.inl>
#include <SofaUserInteraction/RayContact.h>
#include <SofaMeshCollision/BarycentricPenalityContact.inl>
#include <SofaMeshCollision/BarycentricContactMapper.inl>
#include <SofaObjectInteraction/PenalityContactForceField.h>
#include <SofaGeneralDeformable/VectorSpringForceField.h>
#include <sofa/gl/gl.h>
#include <sofa/helper/Factory.inl>
#include <sofa/core/Mapping.inl>
#include <fstream>

namespace sofa
{

namespace component
{

namespace collision
{

using namespace sofa::gpu::cuda;


template class SOFA_GPU_CUDA_API MouseInteractor<CudaVec3fTypes>;
template class SOFA_GPU_CUDA_API TComponentMouseInteraction< CudaVec3fTypes >;
template class SOFA_GPU_CUDA_API AttachBodyPerformer< CudaVec3fTypes >;
template class SOFA_GPU_CUDA_API FixParticlePerformer< CudaVec3fTypes >;

#ifdef SOFA_GPU_CUDA_DOUBLE
template class SOFA_GPU_CUDA_API MouseInteractor<CudaVec3dTypes>;
template class SOFA_GPU_CUDA_API TComponentMouseInteraction< CudaVec3dTypes >;
template class SOFA_GPU_CUDA_API AttachBodyPerformer< CudaVec3dTypes >;
template class SOFA_GPU_CUDA_API FixParticlePerformer< CudaVec3dTypes >;
#endif

ContactMapperCreator< ContactMapper<sofa::component::collision::SphereCollisionModel<gpu::cuda::CudaVec3Types>> > CudaSphereContactMapperClass("PenalityContactForceField",true);

#ifdef WIN32
template<> SOFA_GPU_CUDA_API
std::unordered_map<std::type_index, typename FixParticlePerformer<CudaVec3fTypes>::GetFixationPointsOnModelFunction >
FixParticlePerformer<CudaVec3fTypes>::s_mapSupportedModels;

#ifdef SOFA_GPU_CUDA_DOUBLE
std::unordered_map<std::type_index, typename FixParticlePerformer<CudaVec3dTypes>::GetFixationPointsOnModelFunction >
FixParticlePerformer<CudaVec3dTypes>::s_mapSupportedModels;
#endif

#endif // WIN32

helper::Creator<ComponentMouseInteraction::ComponentMouseInteractionFactory, TComponentMouseInteraction<CudaVec3fTypes> > ComponentMouseInteractionCudaVec3fClass ("MouseSpringCudaVec3f",true);
helper::Creator<InteractionPerformer::InteractionPerformerFactory, AttachBodyPerformer <CudaVec3fTypes> >  AttachBodyPerformerCudaVec3fClass("AttachBody",true);
helper::Creator<InteractionPerformer::InteractionPerformerFactory, FixParticlePerformer<CudaVec3fTypes> >  FixParticlePerformerCudaVec3fClass("FixParticle",true);

#ifdef SOFA_GPU_CUDA_DOUBLE
helper::Creator<ComponentMouseInteraction::ComponentMouseInteractionFactory, TComponentMouseInteraction<CudaVec3dTypes> > ComponentMouseInteractionCudaVec3dClass ("MouseSpringCudaVec3d",true);
helper::Creator<InteractionPerformer::InteractionPerformerFactory, AttachBodyPerformer <CudaVec3dTypes> >  AttachBodyPerformerCudaVec3dClass("AttachBody",true);
helper::Creator<InteractionPerformer::InteractionPerformerFactory, FixParticlePerformer<CudaVec3dTypes> >  FixParticlePerformerCudaVec3dClass("FixParticle",true);
#endif

using FixParticlePerformerCuda3d = FixParticlePerformer<gpu::cuda::CudaVec3Types>;

int triangleFixParticle = FixParticlePerformerCuda3d::RegisterSupportedModel<TriangleCollisionModel<gpu::cuda::Vec3Types>>(&FixParticlePerformerCuda3d::getFixationPointsTriangle<TriangleCollisionModel<gpu::cuda::Vec3Types>>);


} //namespace collision


} //namespace component


namespace gpu
{

namespace cuda
{


int MouseInteractorCudaClass = core::RegisterObject("Supports Mouse Interaction using CUDA")
        .add< component::collision::MouseInteractor<CudaVec3fTypes> >()
#ifdef SOFA_GPU_CUDA_DOUBLE
        .add< component::collision::MouseInteractor<CudaVec3dTypes> >()
#endif
        ;


using namespace sofa::component::collision;

class CudaProximityIntersection : public sofa::component::collision::NewProximityIntersection
{
public:
    SOFA_CLASS(CudaProximityIntersection,sofa::component::collision::NewProximityIntersection);

    virtual void init() override
    {
        using CudaSphereCollisionModel = sofa::component::collision::SphereCollisionModel<gpu::cuda::CudaVec3Types>;

        sofa::component::collision::NewProximityIntersection::init();
        intersectors.add<CudaSphereCollisionModel, CudaSphereCollisionModel, NewProximityIntersection>(this);
        RayDiscreteIntersection* rayIntersector = new RayDiscreteIntersection(this, false);
        intersectors.add<RayCollisionModel,        CudaSphereCollisionModel,   RayDiscreteIntersection>(rayIntersector);
        MeshNewProximityIntersection* meshIntersector = new MeshNewProximityIntersection(this, false);
        intersectors.add<TriangleCollisionModel<sofa::defaulttype::Vec3Types>,   CudaSphereCollisionModel,   MeshNewProximityIntersection>(meshIntersector);
    }

};


int CudaProximityIntersectionClass = core::RegisterObject("GPGPU Proximity Intersection based on CUDA")
        .add< CudaProximityIntersection >()
        ;

sofa::helper::Creator<core::collision::Contact::Factory, component::collision::RayContact<sofa::component::collision::SphereCollisionModel<gpu::cuda::CudaVec3Types>> > RayCudaSphereContactClass("RayContact",true);

} // namespace cuda

} // namespace gpu

} // namespace sofa
