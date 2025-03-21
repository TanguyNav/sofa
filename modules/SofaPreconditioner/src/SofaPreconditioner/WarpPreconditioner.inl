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
#ifndef SOFA_COMPONENT_LINEARSOLVER_WARPPRECONDITIONER_INL
#define SOFA_COMPONENT_LINEARSOLVER_WARPPRECONDITIONER_INL

#include <SofaPreconditioner/WarpPreconditioner.h>
#include <sofa/core/visual/VisualParams.h>
#include <sofa/linearalgebra/SparseMatrix.h>
#include <sofa/core/objectmodel/BaseContext.h>
#include <sofa/core/behavior/LinearSolver.h>
#include <sofa/helper/accessor.h>
#include <sofa/helper/system/thread/CTime.h>
#include <sofa/core/ObjectFactory.h>
#include <sofa/defaulttype/VecTypes.h>

#include <iostream>
#include <cmath>

#include <sofa/type/Quat.h>

namespace sofa
{

namespace component
{

namespace linearsolver
{

template<class TMatrix, class TVector,class ThreadManager>
WarpPreconditioner<TMatrix,TVector,ThreadManager >::WarpPreconditioner()
    : solverName(initData(&solverName, std::string(""), "solverName", "Name of the solver/preconditioner to warp"))
    , f_useRotationFinder(initData(&f_useRotationFinder, (unsigned)0, "useRotationFinder", "Which rotation Finder to use" ) )
{

    realSolver = nullptr;

    rotationWork[0] = nullptr;
    rotationWork[1] = nullptr;

    first = true;
    indexwork = 0;
}

template<class TMatrix, class TVector,class ThreadManager>
WarpPreconditioner<TMatrix,TVector,ThreadManager >::~WarpPreconditioner()
{
    if (rotationWork[0]) delete rotationWork[0];
    if (rotationWork[1]) delete rotationWork[1];

    rotationWork[0] = nullptr;
    rotationWork[1] = nullptr;
}

template<class TMatrix, class TVector,class ThreadManager>
void WarpPreconditioner<TMatrix,TVector,ThreadManager >::bwdInit() {
    this->getContext()->get(realSolver, solverName.getValue());

    if (realSolver == nullptr) {
        msg_error() << "The cannot find the solver " << solverName.getValue();
    }

    sofa::core::objectmodel::BaseContext * c = this->getContext();
    c->get<sofa::core::behavior::BaseRotationFinder >(&rotationFinders, sofa::core::objectmodel::BaseContext::Local);

    std::stringstream tmpStr;
    tmpStr << "Found " << rotationFinders.size() << " Rotation finders" << msgendl;
    for (unsigned i=0; i<rotationFinders.size(); i++) {
        tmpStr << i << " : " << rotationFinders[i]->getName() << msgendl;
    }
    msg_info() << tmpStr.str();

    first = true;
    indexwork = 0;
}

template<class TMatrix, class TVector,class ThreadManager>
typename  WarpPreconditioner<TMatrix, TVector, ThreadManager >::Index 
WarpPreconditioner<TMatrix,TVector,ThreadManager >::getSystemDimention(const sofa::core::MechanicalParams* mparams) {
    simulation::common::MechanicalOperations mops(mparams, this->getContext());

    this->linearSystem.matrixAccessor.setGlobalMatrix(this->linearSystem.systemMatrix);
    this->linearSystem.matrixAccessor.clear();
    mops.getMatrixDimension(&(this->linearSystem.matrixAccessor));
    this->linearSystem.matrixAccessor.setupMatrices();
    return this->linearSystem.matrixAccessor.getGlobalDimension();
}

template<class TMatrix, class TVector,class ThreadManager>
void WarpPreconditioner<TMatrix,TVector,ThreadManager >::setSystemMBKMatrix(const sofa::core::MechanicalParams* mparams)
{
    this->currentMFactor = mparams->mFactor();
    this->currentBFactor = sofa::core::mechanicalparams::bFactor(mparams);
    this->currentKFactor = mparams->kFactor();
    if (!this->frozen) {
        simulation::common::MechanicalOperations mops(mparams, this->getContext());
        if (!this->linearSystem.systemMatrix) this->linearSystem.systemMatrix = this->createMatrix();
    }

    realSolver->setSystemMBKMatrix(mparams);

    if (first) {
        updateSystemSize = getSystemDimention(mparams);
        this->resizeSystem(updateSystemSize);

        first = false;

        if (!rotationWork[indexwork]) rotationWork[indexwork] = new TRotationMatrix();

        rotationWork[indexwork]->resize(updateSystemSize,updateSystemSize);
        rotationFinders[f_useRotationFinder.getValue()]->getRotations(rotationWork[indexwork]);

        if (realSolver->isAsyncSolver()) indexwork = (indexwork==0) ? 1 : 0;

        if (!rotationWork[indexwork]) rotationWork[indexwork] = new TRotationMatrix();

        rotationWork[indexwork]->resize(updateSystemSize,updateSystemSize);
        rotationFinders[f_useRotationFinder.getValue()]->getRotations(rotationWork[indexwork]);

        this->linearSystem.systemMatrix->resize(updateSystemSize,updateSystemSize);
        this->linearSystem.systemMatrix->setIdentity(); // identity rotationa after update

    } else if (realSolver->hasUpdatedMatrix()) {
        updateSystemSize = getSystemDimention(mparams);
        this->resizeSystem(updateSystemSize);

        if (!rotationWork[indexwork]) rotationWork[indexwork] = new TRotationMatrix();

        rotationWork[indexwork]->resize(updateSystemSize,updateSystemSize);
        rotationFinders[f_useRotationFinder.getValue()]->getRotations(rotationWork[indexwork]);

        if (realSolver->isAsyncSolver()) indexwork = (indexwork==0) ? 1 : 0;

        this->linearSystem.systemMatrix->resize(updateSystemSize,updateSystemSize);
        this->linearSystem.systemMatrix->setIdentity(); // identity rotationa after update
    } else {
        currentSystemSize = getSystemDimention(sofa::core::mechanicalparams::defaultInstance());


        this->linearSystem.systemMatrix->clear();
        this->linearSystem.systemMatrix->resize(currentSystemSize,currentSystemSize);
        rotationFinders[f_useRotationFinder.getValue()]->getRotations(this->linearSystem.systemMatrix);

        this->linearSystem.systemMatrix->opMulTM(this->linearSystem.systemMatrix,rotationWork[indexwork]);
    }
}

template<class TMatrix, class TVector,class ThreadManager>
void WarpPreconditioner<TMatrix,TVector,ThreadManager >::invert(Matrix& /*Rcur*/) {}

template<class TMatrix, class TVector,class ThreadManager>
void WarpPreconditioner<TMatrix,TVector,ThreadManager >::updateSystemMatrix() {
    realSolver->updateSystemMatrix();
}


/// Solve the system as constructed using the previous methods
template<class TMatrix, class TVector,class ThreadManager>
void WarpPreconditioner<TMatrix,TVector,ThreadManager >::solve(Matrix& Rcur, Vector& solution, Vector& rh) {
    Rcur.opMulTV(realSolver->getSystemRHBaseVector(),&rh);

    realSolver->solveSystem();

    Rcur.opMulV(&solution,realSolver->getSystemLHBaseVector());
}

/// Solve the system as constructed using the previous methods
template<class TMatrix, class TVector,class ThreadManager>
bool WarpPreconditioner<TMatrix,TVector,ThreadManager >::addJMInvJt(linearalgebra::BaseMatrix* result, linearalgebra::BaseMatrix* J, double fact) {
    if (J->rowSize()==0 || !realSolver) return true;

    this->linearSystem.systemMatrix->rotateMatrix(&j_local,J);

    return realSolver->addJMInvJt(result,&j_local,fact);
}

template<class TMatrix, class TVector,class ThreadManager>
bool WarpPreconditioner<TMatrix,TVector,ThreadManager >::addMInvJt(linearalgebra::BaseMatrix* result, linearalgebra::BaseMatrix* J, double fact) {
    this->linearSystem.systemMatrix->rotateMatrix(&j_local,J);
    return realSolver->addMInvJt(result,&j_local,fact);
}

template<class TMatrix, class TVector,class ThreadManager>
void WarpPreconditioner<TMatrix,TVector,ThreadManager >::computeResidual(const core::ExecParams* params, linearalgebra::BaseVector* f) {
    realSolver->computeResidual(params,f);
}

} // namespace linearsolver

} // namespace component

} // namespace sofa

#endif
