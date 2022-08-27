#include "HeatFlux.h"

registerADMooseObject("StingrayApp", HeatFlux);

InputParameters
HeatFlux::validParams()
{
  InputParameters params = ThermodynamicForce<RealVectorValue>::validParams();
  params.addClassDescription("This class computes the heat flux associated with "
                             "given energy densities for a given species.");
  params.addRequiredParam<MaterialPropertyName>("heat_flux", "Name of the heat flux");
  params.addRequiredParam<VariableName>("temperature", "The temperature variable");
  return params;
}

HeatFlux::HeatFlux(const InputParameters & parameters)
  : ThermodynamicForce<RealVectorValue>(parameters)
{
  const VariableName grad_lnT_name = "∇ln(" + getParam<VariableName>("temperature") + ")";

  // Get forces
  getThermodynamicForces(_d_psi_d_s, _psi_names, grad_lnT_name);

  // Declare the (total) thermodynamic force
  _force = &declareADProperty<RealVectorValue>(getParam<MaterialPropertyName>("heat_flux"));
}