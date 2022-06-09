#include "Redox.h"

registerMooseObject("StingrayApp", Redox);

InputParameters
Redox::validParams()
{
  InputParameters params = ADInterfaceKernel::validParams();
  params.addClassDescription("The Lagrange multiplier for the Butler-Volmer condition across the "
                             "electrode/electrolyte interface");
  params.addRequiredCoupledVar("electric_potential", "The electric potential");
  params.addRequiredParam<Real>("exchange_current_density",
                                "The exchange current density (normal to the interface) for the "
                                "electrode/electrolyte interface");
  params.addRequiredParam<Real>("anodic_charge_transfer_coefficient",
                                "The dimensionless anodic charge transfer coefficient");
  params.addRequiredParam<Real>("cathodic_charge_transfer_coefficient",
                                "The dimensionless cathodic charge transfer coefficient");
  params.addRequiredParam<Real>("faraday_constant", "The Faraday's constant");
  params.addRequiredParam<Real>("ideal_gas_constant", "The ideal gas constant");
  params.addRequiredParam<Real>("electric_conductivity", "The electric conductivity");
  params.addRequiredCoupledVar("temperature", "The temperature");
  params.addRequiredParam<SubdomainName>("electrode_subdomain",
                                         "The subdomain name of the electrode");
  params.addRequiredParam<SubdomainName>("electrolyte_subdomain",
                                         "The subdomain name of the electrolyte");
  return params;
}

Redox::Redox(const InputParameters & parameters)
  : ADInterfaceKernel(parameters),
    _Phi(adCoupledValue("lagrange_multiplier")),
    _Phi_neighbor(adCoupledNeighborValue("lagrange_multiplier")),
    _i0(getParam<Real>("exchange_current_density")),
    _alpha_a(getParam<Real>("anodic_charge_transfer_coefficient")),
    _alpha_c(getParam<Real>("cathodic_charge_transfer_coefficient")),
    _F(getParam<Real>("faraday_constant")),
    _R(getParam<Real>("ideal_gas_constant")),
    _sigma(getParam<Real>("electric_conductivity")),
    _T(adCoupledValue("temperature")),
    _T_neighbor(adCoupledNeighborValue("temperature")),
    _electrode_subdomain_id(_mesh.getSubdomainID(getParam<SubdomainName>("electrode_subdomain"))),
    _electrolyte_subdomain_id(
        _mesh.getSubdomainID(getParam<SubdomainName>("electrolyte_subdomain")))
{
}

ADReal
Redox::computeQpResidual(Moose::DGResidualType type)
{
  // Surface overpotential
  ADReal eta = 0;
  ADReal d_eta_d_u = 0;
  if (_current_elem->subdomain_id() == _electrode_subdomain_id &&
      _neighbor_elem->subdomain_id() == _electrolyte_subdomain_id)
  {
    eta = _u[_qp] - _neighbor_value[_qp];
    d_eta_d_u = 1;
  }
  else if (_current_elem->subdomain_id() == _electrolyte_subdomain_id &&
           _neighbor_elem->subdomain_id() == _electrode_subdomain_id)
  {
    eta = _neighbor_value[_qp] - _u[_qp];
    d_eta_d_u = -1;
  }
  else
    mooseError("Internal error");

  const auto [i, d_i_d_eta] = computeQpCurrentDensity(eta);

  switch (type)
  {
    case Moose::Element:
      return _lm[_qp] *
             (_sigma * _grad_test[_qp] * _normals[_qp] - _test[_qp] * d_i_d_eta * d_eta_d_u);

    case Moose::Neighbor:
      return -_lm_neighbor[_qp] * (_sigma * _grad_test_neighbor[_qp] * _normals[_qp] -
                                   _test_neighbor[_qp] * d_i_d_eta * d_eta_d_u);
  }

  return 0;
}

std::tuple<ADReal, ADReal>
Redox::computeQpCurrentDensity(const ADReal & eta) const
{
  ADReal T = (_T[_qp] + _T_neighbor[_qp]) / 2;
  ADReal coef = _F / _R / T * eta;
  ADReal i = _i0 * (std::exp(_alpha_a * coef) - std::exp(-_alpha_c * coef));

  ADReal d_coef_d_eta = _F / _R / T;
  ADReal d_i_d_coef =
      _i0 * (_alpha_a * std::exp(_alpha_a * coef) + _alpha_c * std::exp(-_alpha_c * coef));
  ADReal d_i_d_eta = d_i_d_coef * d_coef_d_eta;

  return {i, d_i_d_eta};
}
