#include "SwellingDeformationGradient.h"

registerADMooseObject("StingrayApp", SwellingDeformationGradient);

InputParameters
SwellingDeformationGradient::validParams()
{
  InputParameters params = Material::validParams();
  params += BaseNameInterface::validParams();
  params.addClassDescription("This class computes the eigen deformation gradient due to swelling.");

  params.addRequiredCoupledVar(
      "concentrations",
      "Vector of concentrations of chemical species, each contributing to a portion of the "
      "swelling eigen deformation gradient");
  params.addRequiredCoupledVar("reference_concentrations",
                               "Vector of reference concentrations of chemical species, at which "
                               "no swelling occurs");
  params.addRequiredParam<std::vector<MaterialPropertyName>>(
      "molar_volumes", "Vector of molar volumes for the species.");
  params.addRequiredParam<MaterialPropertyName>("swelling_coefficient", "The swelling coefficient");

  return params;
}

SwellingDeformationGradient::SwellingDeformationGradient(const InputParameters & parameters)
  : Material(parameters),
    BaseNameInterface(parameters),
    _Fs(declareADProperty<RankTwoTensor>(prependBaseName("swelling_deformation_gradient"))),
    _c_names(coupledNames("concentrations")),
    _c(adCoupledValues("concentrations")),
    _c_ref(adCoupledValues("reference_concentrations")),
    _Omega_names(getParam<std::vector<MaterialPropertyName>>("molar_volumes")),
    _Omega(_Omega_names.size()),
    _beta(getADMaterialPropertyByName<Real>(prependBaseName("swelling_coefficient", true))),
    _d_Fs_d_c(coupledComponents("concentrations"))
{
  if (_c.size() != _c_ref.size() || _c.size() != _Omega.size())
    mooseError("Number of chemical species concentrations, reference concentrations, and molar "
               "volumes must be the same");

  // Get molar volums
  for (auto i : make_range(_Omega_names.size()))
    _Omega[i] = &getADMaterialPropertyByName<Real>(_Omega_names[i]);

  // Declare d_Fg_d_c
  for (auto i : make_range(_c.size()))
    _d_Fs_d_c[i] = &declareADProperty<RankTwoTensor>(
        derivativePropertyName(prependBaseName("swelling_deformation_gradient"), {_c_names[i]}));
}

void
SwellingDeformationGradient::initQpStatefulProperties()
{
  computeQpProperties();
}

void
SwellingDeformationGradient::computeQpProperties()
{
  ADReal Js = 1;

  for (auto i : make_range(_c.size()))
    Js += _beta[_qp] * (*_Omega[i])[_qp] * ((*_c[i])[_qp] - (*_c_ref[i])[_qp]);

  _Fs[_qp] = std::cbrt(Js) * ADRankTwoTensor::Identity();

  for (auto i : make_range(_c.size()))
    (*_d_Fs_d_c[i])[_qp] =
        std::pow(Js, -2. / 3.) / 3 * _beta[_qp] * (*_Omega[i])[_qp] * ADRankTwoTensor::Identity();
}