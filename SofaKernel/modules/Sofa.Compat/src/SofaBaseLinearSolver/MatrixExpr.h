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

#include <sofa/linearalgebra/MatrixExpr.h>

SOFA_DEPRECATED_HEADER("v21.12", "v22.06", "sofa/linearalgebra/MatrixExpr.h")

namespace sofa::component::linearsolver
{

    template<class M1, class M2>
    using MatrixProduct = sofa::linearalgebra::MatrixProduct<M1, M2>;

    template<class M1, class M2>
    using MatrixAddition = sofa::linearalgebra::MatrixAddition<M1, M2>;

    template<class M1, class M2>
    using MatrixSubstraction = sofa::linearalgebra::MatrixSubstraction<M1, M2>;

    template<class M1>
    using MatrixTranspose = sofa::linearalgebra::MatrixTranspose<M1>;

    template<class M1>
    using MatrixNegative = sofa::linearalgebra::MatrixNegative<M1>;

    template<class M1, class M2>
    using MatrixScale = sofa::linearalgebra::MatrixScale<M1, M2>;

    template<class T>
    using MatrixExpr = sofa::linearalgebra::MatrixExpr<T>;

} // namespace sofa::component::linearsolver
