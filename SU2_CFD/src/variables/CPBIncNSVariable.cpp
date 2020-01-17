/*!
 * \file CIncNSVariable.cpp
 * \brief Definition of the variable classes for incompressible flow.
 * \author F. Palacios, T. Economon
 * \version 6.2.0 "Falcon"
 *
 * The current SU2 release has been coordinated by the
 * SU2 International Developers Society <www.su2devsociety.org>
 * with selected contributions from the open-source community.
 *
 * The main research teams contributing to the current release are:
 *  - Prof. Juan J. Alonso's group at Stanford University.
 *  - Prof. Piero Colonna's group at Delft University of Technology.
 *  - Prof. Nicolas R. Gauger's group at Kaiserslautern University of Technology.
 *  - Prof. Alberto Guardone's group at Polytechnic University of Milan.
 *  - Prof. Rafael Palacios' group at Imperial College London.
 *  - Prof. Vincent Terrapon's group at the University of Liege.
 *  - Prof. Edwin van der Weide's group at the University of Twente.
 *  - Lab. of New Concepts in Aeronautics at Tech. Institute of Aeronautics.
 *
 * Copyright 2012-2019, Francisco D. Palacios, Thomas D. Economon,
 *                      Tim Albring, and the SU2 contributors.
 *
 * SU2 is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * SU2 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with SU2. If not, see <http://www.gnu.org/licenses/>.
 */

#include "../../include/variables/CPBIncNSVariable.hpp"

CPBIncNSVariable::CPBIncNSVariable(su2double val_pressure, su2double *val_velocity, unsigned long nPoint,
                         unsigned short nDim, unsigned short nvar, CConfig *config) :
                         CPBIncEulerVariable(val_pressure, val_velocity, nPoint, nDim, nvar, config) {
  
  Vorticity.resize(nPoint,3);
  StrainMag.resize(nPoint);
  DES_LengthScale.resize(nPoint) = su2double(0.0);
  Max_Lambda_Visc.resize(nPoint);
}

bool CPBIncNSVariable::SetPrimVar(unsigned long iPoint, su2double Density_Inf, su2double Viscosity_Inf, su2double eddy_visc, su2double turb_ke, CConfig *config) {
      
  unsigned short iVar;
  bool check_dens = false, physical = true;
  
  /*--- Set eddy viscosity locally and in the fluid model. ---*/
  SetEddyViscosity(iPoint, eddy_visc);
  
 /*--- Set the value of the density ---*/
  
  check_dens = SetDensity(iPoint, Density_Inf);

  /*--- Set the value of the velocity and velocity^2 (requires density) ---*/

  SetVelocity(iPoint);
  
  /*--- Set laminar viscosity ---*/
  
  SetLaminarViscosity(iPoint, Viscosity_Inf);

  return physical;
}


bool CPBIncNSVariable::SetVorticity_StrainMag(void) {
  
 for (unsigned long iPoint = 0; iPoint < nPoint; ++iPoint) {

    /*--- Vorticity ---*/

    Vorticity(iPoint,0) = 0.0; Vorticity(iPoint,1) = 0.0;

    Vorticity(iPoint,2) = Gradient_Primitive(iPoint,2,0)-Gradient_Primitive(iPoint,1,1);

    if (nDim == 3) {
      Vorticity(iPoint,0) = Gradient_Primitive(iPoint,3,1)-Gradient_Primitive(iPoint,2,2);
      Vorticity(iPoint,1) = -(Gradient_Primitive(iPoint,3,0)-Gradient_Primitive(iPoint,1,2));
    }

    /*--- Strain Magnitude ---*/
    su2double Div = 0.0;
    for (unsigned long iDim = 0; iDim < nDim; iDim++)
      Div += Gradient_Primitive(iPoint,iDim+1,iDim);

    StrainMag(iPoint) = 0.0;

    /*--- Add diagonal part ---*/

    for (unsigned long iDim = 0; iDim < nDim; iDim++) {
      StrainMag(iPoint) += pow(Gradient_Primitive(iPoint,iDim+1,iDim) - 1.0/3.0*Div, 2.0);
    }
    if (nDim == 2) {
      StrainMag(iPoint) += pow(1.0/3.0*Div, 2.0);
    }

    /*--- Add off diagonals ---*/

    StrainMag(iPoint) += 2.0*pow(0.5*(Gradient_Primitive(iPoint,1,1) + Gradient_Primitive(iPoint,2,0)), 2);

    if (nDim == 3) {
      StrainMag(iPoint) += 2.0*pow(0.5*(Gradient_Primitive(iPoint,1,2) + Gradient_Primitive(iPoint,3,0)), 2);
      StrainMag(iPoint) += 2.0*pow(0.5*(Gradient_Primitive(iPoint,2,2) + Gradient_Primitive(iPoint,3,1)), 2);
    }

    StrainMag(iPoint) = sqrt(2.0*StrainMag(iPoint));

  }
  return false;
}