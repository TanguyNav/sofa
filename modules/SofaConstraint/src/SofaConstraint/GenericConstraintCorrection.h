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
#include <SofaConstraint/config.h>

#include <sofa/core/behavior/BaseConstraintCorrection.h>

namespace sofa::component::constraintset
{

class SOFA_SOFACONSTRAINT_API GenericConstraintCorrection : public core::behavior::BaseConstraintCorrection
{
public:
    SOFA_CLASS(GenericConstraintCorrection, core::behavior::BaseConstraintCorrection);

    void bwdInit() override;
    void cleanup() override;
    void addConstraintSolver(core::behavior::ConstraintSolver *s) override;
    void removeConstraintSolver(core::behavior::ConstraintSolver *s) override;

    void computeMotionCorrectionFromLambda(const core::ConstraintParams* cparams, core::MultiVecDerivId dx, const linearalgebra::BaseVector * lambda) override;

    void addComplianceInConstraintSpace(const core::ConstraintParams *cparams, linearalgebra::BaseMatrix* W) override;

    void getComplianceMatrix(linearalgebra::BaseMatrix* ) const override;

    void applyMotionCorrection(const core::ConstraintParams *cparams, core::MultiVecCoordId x, core::MultiVecDerivId v, core::MultiVecDerivId dx, core::ConstMultiVecDerivId correction) override;

    void applyPositionCorrection(const core::ConstraintParams *cparams, core::MultiVecCoordId x, core::MultiVecDerivId dx, core::ConstMultiVecDerivId correction) override;

    void applyVelocityCorrection(const core::ConstraintParams *cparams, core::MultiVecDerivId v, core::MultiVecDerivId dv, core::ConstMultiVecDerivId correction) override;

    void applyPredictiveConstraintForce(const core::ConstraintParams *cparams, core::MultiVecDerivId f, const linearalgebra::BaseVector *lambda) override;

    void rebuildSystem(double massFactor, double forceFactor) override;

    void applyContactForce(const linearalgebra::BaseVector *f) override;

    void resetContactForce() override;

    void computeResidual(const core::ExecParams* params, linearalgebra::BaseVector *lambda) override;

    Data< type::vector< std::string > >  d_linearSolversName; ///< name of the constraint solver
    Data< std::string >                    d_ODESolverName; ///< name of the ode solver
    Data< double > d_complianceFactor; ///< Factor applied to the position factor and velocity factor used to calculate compliance matrix.

protected:
    GenericConstraintCorrection();
    ~GenericConstraintCorrection() override;

    std::list<core::behavior::ConstraintSolver*> constraintsolvers;

    void applyMotionCorrection(const core::ConstraintParams* cparams, core::MultiVecCoordId xId, core::MultiVecDerivId vId, core::MultiVecDerivId dxId,
                               core::ConstMultiVecDerivId correction, double positionFactor, double velocityFactor);

    core::behavior::OdeSolver* m_ODESolver;
    std::vector< core::behavior::LinearSolver* > m_linearSolvers;
};

} //namespace sofa::component::constraintset
