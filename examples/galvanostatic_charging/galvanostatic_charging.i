[GlobalParams]
  # energy_densities = 'psi_c psi_e psi_m psi_charging'
  energy_densities = 'psi_c psi_e psi_charging'
  dissipation_densities = 'delta_c_vis'
[]

[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    xmax = 5
    ymax = 2
    nx = 50
    ny = 20
  []
  [anode]
    type = SubdomainBoundingBoxGenerator
    input = gmg
    bottom_left = '0 0 0'
    top_right = '2 2 0'
    block_id = 1
  []
  [electrolyte]
    type = SubdomainBoundingBoxGenerator
    input = anode
    bottom_left = '2 0 0'
    top_right = '3 2 0'
    block_id = 2
  []
  [cathode]
    type = SubdomainBoundingBoxGenerator
    input = electrolyte
    bottom_left = '3 0 0'
    top_right = '5 2 0'
    block_id = 3
  []
  [anode-electrolyte]
    type = BreakMeshByBlockGenerator
    input = cathode
    split_interface = true
  []
[]

[Variables]
  [c]
  []
  [Phi]
  []
  # [T]
  #   initial_condition = 300
  # []
  # [disp_x]
  # []
  # [disp_y]
  # []
[]

[AuxVariables]
  [T]
    initial_condition = 300
  []
  [bounds]
  []
[]

# [Bounds]
#   [c]
#     type = ConstantBoundsAux
#     variable = bounds
#     bounded_variable = c
#     bound_type = lower
#     bound_value = 0
#   []
# []

[InterfaceKernels]
  [redox_anode-electrolyte]
    type = ButlerVolmerCondition
    variable = Phi
    neighbor_var = Phi
    boundary = Block1_Block2
    electrode_subdomain = 2
    electrolyte_subdomain = 1
    anodic_charge_transfer_coefficient = 0.8
    cathodic_charge_transfer_coefficient = 0.2
    electric_conductivity = 3.8
    exchange_current_density = 1e-1
    faraday_constant = ${F}
    ideal_gas_constant = 8.3145
    temperature = T
    penalty = 1e-4
  []
  [redox_electrolyte-cathode]
    type = ButlerVolmerCondition
    variable = Phi
    neighbor_var = Phi
    boundary = Block2_Block3
    electrode_subdomain = 3
    electrolyte_subdomain = 2
    anodic_charge_transfer_coefficient = 0.8
    cathodic_charge_transfer_coefficient = 0.2
    electric_conductivity = 3.8
    exchange_current_density = 1e-1
    faraday_constant = ${F}
    ideal_gas_constant = 8.3145
    temperature = T
    penalty = 1e-4
  []
  [mass_electrolyte-anode]
    type = HenrysLaw
    variable = c
    neighbor_var = c
    boundary = Block1_Block2
    from_subdomain = 1
    to_subdomain = 2
    ratio = 1
    penalty = 10
  []
  [mass_electrolyte-cathode]
    type = HenrysLaw
    variable = c
    neighbor_var = c
    boundary = Block2_Block3
    from_subdomain = 2
    to_subdomain = 3
    ratio = 1
    penalty = 10
  []
[]

[BCs]
  [Phi_left]
    type = DirichletBC
    variable = Phi
    boundary = 'left'
    value = 0
  []
  [Phi_right]
    type = DirichletBC
    variable = Phi
    boundary = 'right'
    value = 0.04
  []
  [c_right]
    type = DirichletBC
    variable = c
    boundary = 'right'
    value = 0
  []
[]

[Postprocessors]
  [c_surface]
    type = PointValue
    variable = c
    point = '0 0 0'
    outputs = none
  []
  [T]
    type = ElementAverageValue
    variable = T
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]

[UserObjects]
  [terminator]
    type = Terminator
    expression = 'c_surface >= ${c_m}'
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  petsc_options_iname = '-pc_type -snes_type'
  petsc_options_value = 'lu       vinewtonrsls'
  automatic_scaling = true
  nl_rel_tol = 1e-08
  nl_abs_tol = 1e-10
  dt = 1
  end_time = 1000
[]

[Outputs]
  file_base = charge_galvanostatic
  csv = true
  exodus = true
[]
