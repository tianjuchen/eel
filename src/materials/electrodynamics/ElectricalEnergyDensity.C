#include "ElectricalEnergyDensity.h"

InputParameters
ElectricalEnergyDensity::validParams()
{
  InputParameters params = DerivativeMaterialInterface<Material>::validParams();
  params += BaseNameInterface::validParams();
  params.addClassDescription(
      "This class computes the electrical energy density and its corresponding "
      "thermodynamic forces. We assume the electrical energy density depends "
      "on at least the deformation gradient and the gradient of electrical potential");
  params.addRequiredCoupledVar("electric_potential", "The electrical potential");
  params.addRequiredParam<MaterialPropertyName>("electrical_energy_density",
                                                "Name of the electrical energy density");
  return params;
}

ElectricalEnergyDensity::ElectricalEnergyDensity(const InputParameters & parameters)
  : DerivativeMaterialInterface<Material>(parameters),
    BaseNameInterface(parameters),
    _Phi(adCoupledValue("electric_potential")),
    _grad_Phi(adCoupledGradient("electric_potential")),
    _F(hasADMaterialProperty<RankTwoTensor>(prependBaseName("deformation_gradient"))
           ? &getADMaterialPropertyByName<RankTwoTensor>(prependBaseName("deformation_gradient"))
           : nullptr),
    _Phi_name(getVar("electric_potential", 0)->name()),
    _psi_name(getParam<MaterialPropertyName>("electrical_energy_density")),
    _psi(declareADProperty<Real>(prependBaseName(_psi_name))),
    _d_psi_d_Phi(
        declareADProperty<Real>(derivativePropertyName(prependBaseName(_psi_name), {_Phi_name}))),
    _d_psi_d_grad_Phi(declareADProperty<RealVectorValue>(
        derivativePropertyName(prependBaseName(_psi_name), {"grad_" + _Phi_name}))),
    _d_psi_d_F(_F ? &declareADProperty<RankTwoTensor>(derivativePropertyName(
                        prependBaseName(_psi_name), {prependBaseName("deformation_gradient")}))
                  : nullptr)
{
}

void
ElectricalEnergyDensity::computeQpProperties()
{
  precomputeQpProperties();
  _d_psi_d_Phi[_qp] = computeQpDElectricalEnergyDensityDElectricalPotential();
  _d_psi_d_grad_Phi[_qp] = computeQpDElectricalEnergyDensityDElectricalPotentialGradient();
  if (_F)
    (*_d_psi_d_F)[_qp] = computeQpDElectricalEnergyDensityDDeformationGradient();
  _psi[_qp] = computeQpElectricalEnergyDensity();
}