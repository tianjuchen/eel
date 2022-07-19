#pragma once

#include "ChemicalEnergyDensity.h"

/**
 * Mass transport of a charged species driven by electric potential
 */
class Charging : public ChemicalEnergyDensity
{
public:
  static InputParameters validParams();

  Charging(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;
  virtual ADReal computeQpChemicalEnergyDensity() const override;
  virtual ADReal computeQpDChemicalEnergyDensityDConcentration() override;
  virtual ADRealVectorValue computeQpDChemicalEnergyDensityDConcentrationGradient() override;
  virtual ADRankTwoTensor computeQpDChemicalEnergyDensityDDeformationGradient() override;

  /// The electric field
  const ADVariableGradient & _grad_Phi;

  /// The electric conductivity
  const ADMaterialProperty<Real> & _sigma;

  /// Faraday's constant
  const Real _F;

  /// Ideal gas constant
  const Real _R;

  /// Temperature
  const ADVariableValue & _T;

  /// The charge number of this charged species
  const Real _z;

  /// Derivative of the chemical energy density w.r.t. the electric potential gradient
  ADMaterialProperty<RealVectorValue> & _d_psi_d_grad_Phi;

private:
  ADReal _Xi;
};
