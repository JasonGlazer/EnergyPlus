// EnergyPlus, Copyright (c) 1996-2020, The Board of Trustees of the University of Illinois,
// The Regents of the University of California, through Lawrence Berkeley National Laboratory
// (subject to receipt of any required approvals from the U.S. Dept. of Energy), Oak Ridge
// National Laboratory, managed by UT-Battelle, Alliance for Sustainable Energy, LLC, and other
// contributors. All rights reserved.
//
// NOTICE: This Software was developed under funding from the U.S. Department of Energy and the
// U.S. Government consequently retains certain rights. As such, the U.S. Government has been
// granted for itself and others acting on its behalf a paid-up, nonexclusive, irrevocable,
// worldwide license in the Software to reproduce, distribute copies to the public, prepare
// derivative works, and perform publicly and display publicly, and to permit others to do so.
//
// Redistribution and use in source and binary forms, with or without modification, are permitted
// provided that the following conditions are met:
//
// (1) Redistributions of source code must retain the above copyright notice, this list of
//     conditions and the following disclaimer.
//
// (2) Redistributions in binary form must reproduce the above copyright notice, this list of
//     conditions and the following disclaimer in the documentation and/or other materials
//     provided with the distribution.
//
// (3) Neither the name of the University of California, Lawrence Berkeley National Laboratory,
//     the University of Illinois, U.S. Dept. of Energy nor the names of its contributors may be
//     used to endorse or promote products derived from this software without specific prior
//     written permission.
//
// (4) Use of EnergyPlus(TM) Name. If Licensee (i) distributes the software in stand-alone form
//     without changes from the version obtained under this License, or (ii) Licensee makes a
//     reference solely to the software portion of its product, Licensee must refer to the
//     software as "EnergyPlus version X" software, where "X" is the version number Licensee
//     obtained under this License and may not use a different name for the software. Except as
//     specifically required in this Section (4), Licensee shall not use in a company name, a
//     product name, in advertising, publicity, or other promotional activities any name, trade
//     name, trademark, logo, or other designation of "EnergyPlus", "E+", "e+" or confusingly
//     similar designation, without the U.S. Department of Energy's prior written consent.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
// AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

// C++ Headers
#include <cassert>
#include <cmath>
#include <string>

// ObjexxFCL Headers
#include <ObjexxFCL/Array.functions.hh>
#include <ObjexxFCL/Fmath.hh>

// EnergyPlus Headers
#include <EnergyPlus/Autosizing/All_Simple_Sizing.hh>
#include <EnergyPlus/Autosizing/CoolingAirFlowSizing.hh>
#include <EnergyPlus/Autosizing/CoolingCapacitySizing.hh>
#include <EnergyPlus/Autosizing/HeatingAirFlowSizing.hh>
#include <EnergyPlus/Autosizing/HeatingCapacitySizing.hh>
#include <EnergyPlus/Autosizing/SystemAirFlowSizing.hh>
#include <EnergyPlus/Autosizing/WaterHeatingCapacitySizing.hh>
#include <EnergyPlus/BranchNodeConnections.hh>
#include <EnergyPlus/CurveManager.hh>
#include <EnergyPlus/DXCoils.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataAirLoop.hh>
#include <EnergyPlus/DataAirSystems.hh>
#include <EnergyPlus/DataDefineEquip.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataGlobalConstants.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataHeatBalFanSys.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/DataZoneControls.hh>
#include <EnergyPlus/DataZoneEnergyDemands.hh>
#include <EnergyPlus/DataZoneEquipment.hh>
#include <EnergyPlus/EMSManager.hh>
#include <EnergyPlus/Fans.hh>
#include <EnergyPlus/FluidProperties.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/GeneralRoutines.hh>
#include <EnergyPlus/GlobalNames.hh>
#include <EnergyPlus/HVACFan.hh>
#include <EnergyPlus/HVACVariableRefrigerantFlow.hh>
#include <EnergyPlus/HeatingCoils.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/MixedAir.hh>
#include <EnergyPlus/NodeInputManager.hh>
#include <EnergyPlus/OutAirNodeManager.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/Plant/DataPlant.hh>
#include <EnergyPlus/Plant/PlantLocation.hh>
#include <EnergyPlus/PlantUtilities.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/SteamCoils.hh>
#include <EnergyPlus/TempSolveRoot.hh>
#include <EnergyPlus/UtilityRoutines.hh>
#include <EnergyPlus/WaterCoils.hh>
#include <EnergyPlus/WaterManager.hh>

namespace EnergyPlus {

namespace HVACVariableRefrigerantFlow {
    // Module containing the Variable Refrigerant Flow (VRF or VRV) simulation routines

    // MODULE INFORMATION:
    //       AUTHOR         Richard Raustad, FSEC
    //       DATE WRITTEN   August 2010
    //       MODIFIED       Apr 2012, R. Raustad, FSEC, Added Heat Recovery Operating Mode
    //                      Jul 2015, RP Zhang, XF Pang, LBNL, Added a new physics based VRF model applicable for Fluid Temperature Control
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS MODULE:
    // To encapsulate the data and algorithms required to
    // manage the VRF System Component

    using namespace DataZoneEnergyDemands;
    using namespace Psychrometrics;
    using namespace DataPlant;

    // Compressor operation
    int const On(1);  // normal compressor operation
    int const Off(0); // signal DXCoil that compressor shouldn't run

    // Heat Recovery System used
    int const No(1);  // Heat Pump mode only
    int const Yes(2); // Heat Pump or Heat Recovery Mode (not available at this time)

    // Defrost strategy
    int const ReverseCycle(1); // uses reverse cycle defrost strategy
    int const Resistive(2);    // uses electric resistance heater for defrost

    // Defrost control
    int const Timed(1);    // defrost cycle is timed
    int const OnDemand(2); // defrost cycle occurs only when required

    // Thermostat Priority Control Type
    int const LoadPriority(1);             // total of zone loads dictate operation in cooling or heating
    int const ZonePriority(2);             // # of zones requiring cooling or heating dictate operation in cooling or heating
    int const ThermostatOffsetPriority(3); // zone with largest deviation from setpoint dictates operation
    int const ScheduledPriority(4);        // cooling and heating modes are scheduled
    int const MasterThermostatPriority(5); // Master zone thermostat dictates operation
    int const FirstOnPriority(6);          // first unit to respond dictates operation (not used at this time)

    // Water Systems
    int const CondensateDiscarded(1001); // default mode where water is "lost"
    int const CondensateToTank(1002);    // collect coil condensate from air and store in water storage tank

    int const WaterSupplyFromMains(101); // mains water line used as water source
    int const WaterSupplyFromTank(102);  // storage tank used as water source

    Real64 const MaxCap(1.0e+20); // limit of zone terminal unit capacity

    // VRF System Types (strings used in integer conversions)
    int const NumVRFSystemTypes(1);
    int const VRF_HeatPump(1);
    Array1D_string const cVRFTypes(NumVRFSystemTypes, std::string("AirConditioner:VariableRefrigerantFlow"));

    static std::string const fluidNameSteam("STEAM");

    // VRF Algorithm Type
    int const AlgorithmTypeSysCurve(1);   // VRF model based on system curve
    int const AlgorithmTypeFluidTCtrl(2); // VRF model based on physics, appreciable for Fluid Temperature Control

    // Flag for hex operation
    int const FlagCondMode(0); // Flag for the hex running as condenser [-]
    int const FlagEvapMode(1); // Flag for the hex running as evaporator [-]

    // Flag for VRF operational mode
    int const ModeCoolingOnly(1);       // Flag for Cooling Only Mode [-]
    int const ModeHeatingOnly(2);       // Flag for Heating Only Mode [-]
    int const ModeCoolingAndHeating(3); // Flag for Simultaneous Cooling and Heating Only Mode [-]

    static std::string const BlankString;

    bool GetVRFInputFlag(true);             // Flag set to make sure you get input once
    bool MyOneTimeFlag(true);               // One time flag used to allocate MyEnvrnFlag and MySizeFlag
    bool MyOneTimeSizeFlag(true);           // One time flag used to allocate MyEnvrnFlag and MySizeFlag
    bool ZoneEquipmentListNotChecked(true); // False after the Zone Equipment List has been checked for items
    int NumVRFCond(0);                      // total number of VRF condensers (All VRF Algorithm Types)
    int NumVRFCond_SysCurve(0);             // total number of VRF condensers with VRF Algorithm Type 1
    int NumVRFCond_FluidTCtrl_HP(0);        // total number of VRF condensers with VRF Algorithm Type 2 (HP)
    int NumVRFCond_FluidTCtrl_HR(0);        // total number of VRF condensers with VRF Algorithm Type 2 (HR)
    int NumVRFTU(0);                        // total number of VRF terminal units
    int NumVRFTULists(0);                   // The number of VRF TU lists
    Real64 CompOnMassFlow(0.0);             // Supply air mass flow rate w/ compressor ON
    Real64 OACompOnMassFlow(0.0);           // OA mass flow rate w/ compressor ON
    Real64 CompOffMassFlow(0.0);            // Supply air mass flow rate w/ compressor OFF
    Real64 OACompOffMassFlow(0.0);          // OA mass flow rate w/ compressor OFF
    Real64 CompOnFlowRatio(0.0);            // fan flow ratio when coil on
    Real64 CompOffFlowRatio(0.0);           // fan flow ratio when coil off
    Real64 FanSpeedRatio(0.0);              // ratio of air flow ratio passed to fan object
    Real64 LoopDXCoolCoilRTF(0.0);          // holds value of DX cooling coil RTF
    Real64 LoopDXHeatCoilRTF(0.0);          // holds value of DX heating coil RTF
    Real64 CondenserWaterMassFlowRate(0.0); // VRF water-cooled condenser mass flow rate (kg/s)
    Array1D_bool HeatingLoad;               // defines a heating load on VRFTerminalUnits
    Array1D_bool CoolingLoad;               // defines a cooling load on VRFTerminalUnits
    Array1D_bool LastModeHeating;           // defines last mode was heating mode
    Array1D_bool LastModeCooling;           // defines last mode was cooling mode
    Array1D_bool CheckEquipName;            // Flag set to check equipment connections once
    Array1D_bool MyEnvrnFlag;               // Flag for initializing at beginning of each new environment
    Array1D_bool MySizeFlag;                // False after TU has been sized
    Array1D_bool MyBeginTimeStepFlag;       // Flag to sense beginning of time step
    Array1D_bool MyVRFFlag;                 // used for sizing VRF inputs one time
    Array1D_bool MyVRFCondFlag;             // used to reset timer counter
    Array1D_bool MyZoneEqFlag;              // used to set up zone equipment availability managers
    Array1D_int NumCoolingLoads;            // number of TU's requesting cooling
    Array1D_int NumHeatingLoads;            // number of TU's requesting heating
    Array1D<Real64> MaxCoolingCapacity;     // maximum capacity of any terminal unit
    Array1D<Real64> MaxHeatingCapacity;     // maximum capacity of any terminal unit
    Array1D<Real64> CoolCombinationRatio;   // ratio of terminal unit capacity to VRF condenser capacity
    Array1D<Real64> HeatCombinationRatio;   // ratio of terminal unit capacity to VRF condenser capacity
    Array1D<Real64> MaxDeltaT;              // maximum zone temperature difference from setpoint
    Array1D<Real64> MinDeltaT;              // minimum zone temperature difference from setpoint
    Array1D<Real64> SumCoolingLoads;        // sum of cooling loads
    Array1D<Real64> SumHeatingLoads;        // sum of heating loads

    // Object Data
    Array1D<VRFCondenserEquipment> VRF; // AirConditioner:VariableRefrigerantFlow object
    std::unordered_map<std::string, std::string> VrfUniqueNames;
    Array1D<VRFTerminalUnitEquipment> VRFTU;           // ZoneHVAC:TerminalUnit:VariableRefrigerantFlow object
    Array1D<TerminalUnitListData> TerminalUnitList;    // zoneTerminalUnitList object
    Array1D<VRFTUNumericFieldData> VRFTUNumericFields; // holds VRF TU numeric input fields character field name

    void SimulateVRF(EnergyPlusData &state,
                     std::string const &CompName,
                     bool const FirstHVACIteration,
                     int const ZoneNum,
                     int &CompIndex,
                     bool &HeatingActive,
                     bool &CoolingActive,
                     int const OAUnitNum,         // If the system is an equipment of OutdoorAirUnit
                     Real64 const OAUCoilOutTemp, // the coil inlet temperature of OutdoorAirUnit
                     bool const ZoneEquipment,    // TRUE if called as zone equipment
                     Real64 &SysOutputProvided,
                     Real64 &LatOutputProvided)
    {

        // SUBROUTINE INFORMATION:
        // AUTHOR         Richard Raustad, FSEC
        // DATE WRITTEN   August 2010
        // MODIFIED       Jul 2015, RP Zhang (LBNL), XF Pang (LBNL), Y Yura (Daikin Inc). Add a physics-based VRF model applicable for Fluid
        // Temperature Control RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine manages VRF terminal unit simulation.

        // METHODOLOGY EMPLOYED:
        // Simulate all terminal units
        // Once all terminal units have been simulated, simulate VRF condenser

        using DXCoils::DXCoilTotalCooling;
        using DXCoils::DXCoilTotalHeating;
        using General::TrimSigDigits;

        int VRFTUNum;             // current VRF system terminal unit index
        int VRFCondenser;         // index to VRF AC system object - AirConditioner:VariableRefrigerantFlow
        int TUListNum;            // index to VRF AC system terminal unit list
        int IndexToTUInTUList;    // index to pointer in VRF AC system terminal unit list
        Real64 OnOffAirFlowRatio; // ratio of compressor ON airflow to average airflow over timestep
        int DXCoolingCoilIndex;   // index to this terminal units DX cooling coil
        int DXHeatingCoilIndex;   // index to this terminal units DX heating coil
        Real64 QZnReq;

        // Obtains and Allocates VRF system related parameters from input file
        if (GetVRFInputFlag) { // First time subroutine has been entered
            GetVRFInput(state);
            GetVRFInputFlag = false;
        }

        // CompIndex accounting
        if (CompIndex == 0) {
            VRFTUNum = UtilityRoutines::FindItemInList(CompName, VRFTU);
            if (VRFTUNum == 0) {
                ShowFatalError("SimulateVRF: VRF Terminal Unit not found=" + CompName);
            }
            CompIndex = VRFTUNum;

            // suppress unused warnings temporarily until VRF inherits HVACSystemData
            if (OAUnitNum > 0) {
                bool tmpFlag = false;
                if (OAUCoilOutTemp > 0.0) tmpFlag = true;
                if (ZoneEquipment) tmpFlag = true;
            }

        } else {
            VRFTUNum = CompIndex;
            if (VRFTUNum > NumVRFTU || VRFTUNum < 1) {
                ShowFatalError("SimulateVRF: Invalid CompIndex passed=" + TrimSigDigits(VRFTUNum) +
                               ", Number of VRF Terminal Units = " + TrimSigDigits(NumVRFTU) + ", VRF Terminal Unit name = " + CompName);
            }
            if (CheckEquipName(VRFTUNum)) {
                if (!CompName.empty() && CompName != VRFTU(VRFTUNum).Name) {
                    ShowFatalError("SimulateVRF: Invalid CompIndex passed=" + TrimSigDigits(VRFTUNum) + ", VRF Terminal Unit name=" + CompName +
                                   ", stored VRF TU Name for that index=" + VRFTU(VRFTUNum).Name);
                }
                CheckEquipName(VRFTUNum) = false;
            }
        }

        // the VRF condenser index
        VRFCondenser = VRFTU(VRFTUNum).VRFSysNum;

        if ((VRF(VRFCondenser).CondenserType == DataHVACGlobals::WaterCooled) && (VRF(VRFCondenser).checkPlantCondTypeOneTime)) {
            // scan for loop connection data
            bool errFlag = false;
            PlantUtilities::ScanPlantLoopsForObject(state,
                                                    VRF(VRFCondenser).Name,
                                                    VRF(VRFCondenser).VRFPlantTypeOfNum,
                                                    VRF(VRFCondenser).SourceLoopNum,
                                                    VRF(VRFCondenser).SourceLoopSideNum,
                                                    VRF(VRFCondenser).SourceBranchNum,
                                                    VRF(VRFCondenser).SourceCompNum,
                                                    errFlag,
                                                    _,
                                                    _,
                                                    _,
                                                    VRF(VRFCondenser).CondenserNodeNum,
                                                    _);

            if (errFlag) {
                ShowSevereError("GetVRFInput: Error scanning for plant loop data");
            }

            VRF(VRFCondenser).checkPlantCondTypeOneTime = false;
        }

        // the terminal unit list object index
        TUListNum = VRFTU(VRFTUNum).TUListIndex;
        // the entry number in the terminal unit list (which item in the terminal unit list, e.g. second in list)
        IndexToTUInTUList = VRFTU(VRFTUNum).IndexToTUInTUList;
        // index to cooling coil (coil is optional but at least one must be present)
        DXCoolingCoilIndex = VRFTU(VRFTUNum).CoolCoilIndex;
        // index to heating coil (coil is optional but at least one must be present)
        DXHeatingCoilIndex = VRFTU(VRFTUNum).HeatCoilIndex;
        QZnReq = 0.0;

        // Initialize terminal unit
        InitVRF(state, VRFTUNum, ZoneNum, FirstHVACIteration, OnOffAirFlowRatio, QZnReq); // Initialize all VRFTU related parameters

        // Simulate terminal unit
        SimVRF(state, VRFTUNum, FirstHVACIteration, OnOffAirFlowRatio, SysOutputProvided, LatOutputProvided, QZnReq);

        // mark this terminal unit as simulated
        TerminalUnitList(TUListNum).IsSimulated(IndexToTUInTUList) = true;

        // keep track of individual coil loads
        if (DXCoolingCoilIndex > 0) {
            TerminalUnitList(TUListNum).TotalCoolLoad(IndexToTUInTUList) = DXCoilTotalCooling(DXCoolingCoilIndex);
        } else {
            TerminalUnitList(TUListNum).TotalCoolLoad(IndexToTUInTUList) = 0.0;
        }
        if (DXHeatingCoilIndex > 0) {
            TerminalUnitList(TUListNum).TotalHeatLoad(IndexToTUInTUList) = DXCoilTotalHeating(DXHeatingCoilIndex);
        } else {
            TerminalUnitList(TUListNum).TotalHeatLoad(IndexToTUInTUList) = 0.0;
        }

        // Report the current VRF terminal unit
        ReportVRFTerminalUnit(VRFTUNum);

        if (VRFTU(VRFTUNum).TotalCoolingRate > 0.0) CoolingActive = true;
        if (VRFTU(VRFTUNum).TotalHeatingRate > 0.0) HeatingActive = true;

        // make sure all TU in a list are able to get simulated, otherwise condenser is never simulated **
        // either fatal on GetInput, or keep track of unused TU's and set their respective flag to TRUE **
        // after all VRF terminal units have been simulated, call the VRF condenser model
        if (all(TerminalUnitList(TUListNum).IsSimulated)) {

            if (VRF(VRFCondenser).VRFAlgorithmTypeNum == AlgorithmTypeFluidTCtrl) {
                // Algorithm Type: VRF model based on physics, applicable for Fluid Temperature Control
                VRF(VRFCondenser).CalcVRFCondenser_FluidTCtrl(state);
            } else {
                // Algorithm Type: VRF model based on system curve
                CalcVRFCondenser(state, VRFCondenser);
            }

            ReportVRFCondenser(VRFCondenser);

            if (VRF(VRFCondenser).CondenserType == DataHVACGlobals::WaterCooled) UpdateVRFCondenser(VRFCondenser);
        }
    }

    PlantComponent *VRFCondenserEquipment::factory(EnergyPlusData &state, std::string const &objectName)
    {
        // Process the input data if it hasn't been done already
        if (GetVRFInputFlag) {
            GetVRFInput(state);
            GetVRFInputFlag = false;
        }
        // Now look for this object in the list
        for (auto &obj : VRF) {
            if (obj.Name == objectName) {
                return &obj;
            }
        }
        // If we didn't find it, fatal
        ShowFatalError("LocalVRFCondenserFactory: Error getting inputs for object named: " + objectName); // LCOV_EXCL_LINE
        // Shut up the compiler
        return nullptr; // LCOV_EXCL_LINE
    }

    void VRFCondenserEquipment::onInitLoopEquip(EnergyPlusData &state, const PlantLocation &EP_UNUSED(calledFromLocation))
    {
        this->SizeVRFCondenser(state);
    }

    void
    VRFCondenserEquipment::getDesignCapacities(EnergyPlusData &EP_UNUSED(state), const PlantLocation &EP_UNUSED(calledFromLocation), Real64 &MaxLoad, Real64 &MinLoad, Real64 &OptLoad)
    {
        MinLoad = 0.0;
        MaxLoad = max(this->CoolingCapacity, this->HeatingCapacity); // greater of cooling and heating capacity
        OptLoad = max(this->CoolingCapacity,
                      this->HeatingCapacity); // connects to single loop, need to switch between cooling/heating capacity?
    }

    void VRFCondenserEquipment::simulate(EnergyPlusData &state,
                                         const PlantLocation &calledFromLocation,
                                         bool FirstHVACIteration,
                                         Real64 &EP_UNUSED(CurLoad),
                                         bool EP_UNUSED(RunFlag))
    {
        if (calledFromLocation.loopNum == this->SourceLoopNum) { // condenser loop
            PlantUtilities::UpdateChillerComponentCondenserSide(state, this->SourceLoopNum,
                                                                this->SourceLoopSideNum,
                                                                TypeOf_HeatPumpVRF,
                                                                this->CondenserNodeNum,
                                                                this->CondenserOutletNodeNum,
                                                                this->QCondenser,
                                                                this->CondenserInletTemp,
                                                                this->CondenserSideOutletTemp,
                                                                this->WaterCondenserMassFlow,
                                                                FirstHVACIteration);
        } else {
            ShowFatalError("SimVRFCondenserPlant:: Invalid loop connection " + cVRFTypes(VRF_HeatPump));
        }
    }

    void CalcVRFCondenser(EnergyPlusData &state, int const VRFCond)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         R. Raustad, FSEC
        //       DATE WRITTEN   September 2010

        // PURPOSE OF THIS SUBROUTINE:
        // Model the interactions of VRF terminal units with a single variable-speed condenser.
        // The terminal units are simulated first, and then the condenser is simulated.
        // If terminal units require more capacity than can be delivered by condenser, a limit is set.

        using CurveManager::CurveValue;
        using DataEnvironment::CurMnDy;
        using DataEnvironment::EnvironmentName;
        using DataEnvironment::OutBaroPress;
        using DataEnvironment::OutDryBulbTemp;
        using DataEnvironment::OutHumRat;
        using DataEnvironment::OutWetBulbTemp;
        using DXCoils::DXCoilCoolInletAirWBTemp;
        using DXCoils::DXCoilHeatInletAirDBTemp;
        using DXCoils::DXCoilHeatInletAirWBTemp;
        using FluidProperties::GetSpecificHeatGlycol;
        using General::TrimSigDigits;
        using PlantUtilities::SetComponentFlowRate;
        using Psychrometrics::RhoH2O;

        static std::string const RoutineName("VRFCondenser");

        int NumTU;         // loop counter
        int TUIndex;       // Index to terminal unit
        int CoolCoilIndex; // index to cooling coil in terminal unit
        int HeatCoilIndex; // index to heating coil in terminal unit

        Real64 TotCoolCapTempModFac;      // cooling CAPFT curve output
        Real64 TotHeatCapTempModFac;      // heating CAPFT curve output
        Real64 TotCoolEIRTempModFac;      // cooling EIRFT curve output
        Real64 TotHeatEIRTempModFac;      // heating EIRFT curve output
        Real64 InletAirWetBulbC;          // coil inlet air wet-bulb temperature (C)
        Real64 InletAirDryBulbC;          // coil inlet air dry-bulb temperature (C)
        Real64 CondInletTemp(0.0);        // condenser inlet air temperature (C)
        Real64 CondInletHumRat;           // condenser inlet air humidity ratio (kg/kg)
        Real64 OutdoorDryBulb;            // outdoor dry-bulb temperature (C)
        Real64 OutdoorHumRat;             // outdoor humidity ratio (kg/kg)
        Real64 OutdoorPressure;           // outdoor pressure (Pa)
        Real64 OutdoorWetBulb;            // outdoor wet-bulb temperature (C)
        Real64 CoolOABoundary;            // output of cooling boundary curve (outdoor temperature, C)
        Real64 HeatOABoundary;            // output of heating boundary curve (outdoor temperature, C)
        Real64 EIRFPLRModFac;             // EIRFPLR curve output
        int Stage;                        // used for crankcase heater power calculation
        Real64 UpperStageCompressorRatio; // used for crankcase heater power calculation
        Real64 RhoAir;                    // Density of air [kg/m3]
        Real64 RhoWater;                  // Density of water [kg/m3]
        Real64 CpCond;                    // Specific Heat of water [J/kg-k]
        Real64 CondAirMassFlow;           // Condenser air mass flow rate [kg/s]
        Real64 CondWaterMassFlow;         // Condenser water mass flow rate [kg/s]
        Real64 PartLoadFraction;          // Part load fraction from PLFFPLR curve
        Real64 VRFRTF;                    // VRF runtime fraction when cycling below MINPLR
        Real64 OutdoorCoilT;              // Outdoor coil temperature (C)
        Real64 OutdoorCoildw;             // Outdoor coil delta w assuming coil temp of OutdoorCoilT (kg/kg)
        Real64 FractionalDefrostTime;     // Fraction of time step system is in defrost
        Real64 HeatingCapacityMultiplier; // Multiplier for heating capacity when system is in defrost
        Real64 InputPowerMultiplier;      // Multiplier for power when system is in defrost
        Real64 LoadDueToDefrost;          // Additional load due to defrost
        Real64 DefrostEIRTempModFac;      // EIR modifier for defrost (function of entering drybulb, outside wetbulb)
        int HRCAPFT;                      // index to heat recovery CAPFTCool curve
        Real64 HRCAPFTConst;              // stead-state capacity fraction
        Real64 HRInitialCapFrac;          // Fractional cooling degradation at the start of heat recovery from cooling mode
        Real64 HRCapTC;                   // Time constant used to recover from initial degradation in cooling heat recovery
        int HREIRFT;                      // Index to cool EIR as a function of temperature curve for heat recovery
        Real64 HREIRFTConst;              // stead-state EIR fraction
        Real64 HRInitialEIRFrac;          // Fractional cooling degradation at the start of heat recovery from cooling mode
        Real64 HREIRTC;                   // Time constant used to recover from initial degradation in cooling heat recovery
        static Real64 CurrentEndTime;     // end time of current time step
        static Real64 CurrentEndTimeLast; // end time of last time step
        static Real64 TimeStepSysLast;    // system time step on last time step
        Real64 SUMultiplier;              // multiplier for simulating mode changes
        Real64 CondPower;                 // condenser power [W]
        Real64 CondCapacity;              // condenser heat rejection [W]
        Real64 CondOutletTemp;            // Outlet temperature from VRF condenser [C]
        Real64 TotPower;                  // total condenser power use [W]
        bool HRHeatRequestFlag;           // flag indicating VRF TU could operate in heating mode
        bool HRCoolRequestFlag;           // flag indicating VRF TU could operate in cooling mode
        // FLOW

        // variable initializations
        int TUListNum = VRF(VRFCond).ZoneTUListPtr;
        int NumTUInList = TerminalUnitList(TUListNum).NumTUInList;
        int NumTUInCoolingMode = 0;            // number of terminal units actually cooling
        int NumTUInHeatingMode = 0;            // number of terminal units actually heating
        Real64 TUCoolingLoad = 0.0;            // sum of TU's cooling coil load {W}
        Real64 TUHeatingLoad = 0.0;            // sum of TU's heating coil load (W)
        Real64 TUParasiticPower = 0.0;         // total terminal unit parasitic power (W)
        Real64 TUFanPower = 0.0;               // total terminal unit fan power (W)
        Real64 CoolingPLR = 0.0;               // condenser cooling PLR
        Real64 HeatingPLR = 0.0;               // condenser heating PLR
        Real64 CyclingRatio = 1.0;             // cycling ratio of condenser's compressors
        Real64 SumCoolInletWB = 0.0;           // sum of active TU's DX cooling coil inlet air wet-bulb temperature
        Real64 SumHeatInletDB = 0.0;           // sum of active TU's DX heating coil inlet air dry-bulb temperature
        Real64 SumHeatInletWB = 0.0;           // sum of active TU's DX heating coil inlet air wet-bulb temperature
        Real64 TotalCondCoolingCapacity = 0.0; // total available condenser cooling capacity (W)
        Real64 TotalCondHeatingCapacity = 0.0; // total available condenser heating capacity (W)
        Real64 TotalTUCoolingCapacity = 0.0;   // sum of TU's cooling capacity including piping losses (W)
        Real64 TotalTUHeatingCapacity = 0.0;   // sum of TU's heating capacity including piping losses (W)

        VRF(VRFCond).ElecCoolingPower = 0.0;
        VRF(VRFCond).ElecHeatingPower = 0.0;
        VRF(VRFCond).CrankCaseHeaterPower = 0.0;
        VRF(VRFCond).EvapCondPumpElecPower = 0.0;
        VRF(VRFCond).EvapWaterConsumpRate = 0.0;
        VRF(VRFCond).DefrostPower = 0.0;
        VRF(VRFCond).OperatingCoolingCOP = 0.0;
        VRF(VRFCond).OperatingHeatingCOP = 0.0;
        VRF(VRFCond).OperatingCOP = 0.0;
        VRF(VRFCond).SCHE = 0.0;
        VRF(VRFCond).BasinHeaterPower = 0.0;
        VRF(VRFCond).VRFHeatRec = 0.0;

        // set condenser entering air conditions
        if (VRF(VRFCond).CondenserNodeNum != 0) {
            OutdoorDryBulb = DataLoopNode::Node(VRF(VRFCond).CondenserNodeNum).Temp;
            if (VRF(VRFCond).CondenserType != DataHVACGlobals::WaterCooled) {
                OutdoorHumRat = DataLoopNode::Node(VRF(VRFCond).CondenserNodeNum).HumRat;
                OutdoorPressure = DataLoopNode::Node(VRF(VRFCond).CondenserNodeNum).Press;
                OutdoorWetBulb = DataLoopNode::Node(VRF(VRFCond).CondenserNodeNum).OutAirWetBulb;
            } else {
                OutdoorHumRat = OutHumRat;
                OutdoorPressure = OutBaroPress;
                OutdoorWetBulb = OutWetBulbTemp;
            }
        } else {
            OutdoorDryBulb = OutDryBulbTemp;
            OutdoorHumRat = OutHumRat;
            OutdoorPressure = OutBaroPress;
            OutdoorWetBulb = OutWetBulbTemp;
        }

        if (VRF(VRFCond).CondenserType == DataHVACGlobals::AirCooled) {
            CondInletTemp = OutdoorDryBulb; // Outdoor dry-bulb temp
        } else if (VRF(VRFCond).CondenserType == DataHVACGlobals::EvapCooled) {
            RhoAir = PsyRhoAirFnPbTdbW(OutdoorPressure, OutdoorDryBulb, OutdoorHumRat);
            CondAirMassFlow = RhoAir * VRF(VRFCond).EvapCondAirVolFlowRate;
            // (Outdoor wet-bulb temp from DataEnvironment) + (1.0-EvapCondEffectiveness) * (drybulb - wetbulb)
            CondInletTemp = OutdoorWetBulb + (OutdoorDryBulb - OutdoorWetBulb) * (1.0 - VRF(VRFCond).EvapCondEffectiveness);
            CondInletHumRat = PsyWFnTdbTwbPb(CondInletTemp, OutdoorWetBulb, OutdoorPressure);
        } else if (VRF(VRFCond).CondenserType == DataHVACGlobals::WaterCooled) {
            CondInletTemp = OutdoorDryBulb; // node inlet temp from above
            OutdoorWetBulb = CondInletTemp; // for watercooled
            CondWaterMassFlow = VRF(VRFCond).WaterCondenserDesignMassFlow;
        } else {
            assert(false);
        }
        VRF(VRFCond).CondenserInletTemp = CondInletTemp;

        // sum loads on TU coils
        for (NumTU = 1; NumTU <= NumTUInList; ++NumTU) {
            TUCoolingLoad += TerminalUnitList(TUListNum).TotalCoolLoad(NumTU);
            TUHeatingLoad += TerminalUnitList(TUListNum).TotalHeatLoad(NumTU);
        }

        VRF(VRFCond).TUCoolingLoad = TUCoolingLoad;
        VRF(VRFCond).TUHeatingLoad = TUHeatingLoad;

        // no need to do anything else if the terminal units are off
        if (TUCoolingLoad == 0.0 && TUHeatingLoad == 0.0) {
            VRF(VRFCond).SUMultiplier = 0.0;
            VRF(VRFCond).VRFCondPLR = 0.0;
            VRF(VRFCond).VRFCondRTF = 0.0;
            VRF(VRFCond).VRFCondCyclingRatio = 0.0;
            VRF(VRFCond).QCondenser = 0.0;
            VRF(VRFCond).TotalCoolingCapacity = 0.0;
            VRF(VRFCond).TotalHeatingCapacity = 0.0;
            VRF(VRFCond).OperatingMode = 0.0;
            VRF(VRFCond).HRHeatingActive = false;
            VRF(VRFCond).HRCoolingActive = false;
            CurrentEndTimeLast = double((DayOfSim - 1) * 24) + CurrentTime - TimeStepZone + DataHVACGlobals::SysTimeElapsed;
            if (VRF(VRFCond).CondenserType == DataHVACGlobals::WaterCooled) {
                CondenserWaterMassFlowRate = 0.0;
                SetComponentFlowRate(CondenserWaterMassFlowRate,
                                     VRF(VRFCond).CondenserNodeNum,
                                     VRF(VRFCond).CondenserOutletNodeNum,
                                     VRF(VRFCond).SourceLoopNum,
                                     VRF(VRFCond).SourceLoopSideNum,
                                     VRF(VRFCond).SourceBranchNum,
                                     VRF(VRFCond).SourceCompNum);
                VRF(VRFCond).WaterCondenserMassFlow = CondenserWaterMassFlowRate;
                VRF(VRFCond).CondenserSideOutletTemp = CondInletTemp;
            }
            return;
        }

        // switch modes if summed coil capacity shows opposite operating mode
        // if total TU heating exceeds total TU cooling * ( 1 + 1/COP) then system is in heating mode
        if (CoolingLoad(VRFCond) && TUHeatingLoad > (TUCoolingLoad * (1.0 + 1.0 / VRF(VRFCond).CoolingCOP))) {
            HeatingLoad(VRFCond) = true;
            CoolingLoad(VRFCond) = false;
            VRF(VRFCond).ModeChange = true;
            if (!LastModeHeating(VRFCond)) {
                LastModeHeating(VRFCond) = true;
                // reset heat recovery startup timer
                VRF(VRFCond).HRTimer = 0.0;
                VRF(VRFCond).HRHeatingActive = false;
                VRF(VRFCond).HRCoolingActive = false;
            }
        } else if (HeatingLoad(VRFCond) && (TUCoolingLoad * (1.0 + 1.0 / VRF(VRFCond).CoolingCOP)) > TUHeatingLoad) {
            CoolingLoad(VRFCond) = true;
            HeatingLoad(VRFCond) = false;
            VRF(VRFCond).ModeChange = true;
            if (!LastModeCooling(VRFCond)) {
                LastModeCooling(VRFCond) = true;
                // reset heat recovery startup timer
                VRF(VRFCond).HRTimer = 0.0;
                VRF(VRFCond).HRHeatingActive = false;
                VRF(VRFCond).HRCoolingActive = false;
            }
        } else if (TUCoolingLoad > 0.0 && TUHeatingLoad > 0.0 &&
                   ((CoolingLoad(VRFCond) && LastModeHeating(VRFCond)) || (HeatingLoad(VRFCond) && LastModeCooling(VRFCond)))) {
            VRF(VRFCond).ModeChange = true;
            // reset heat recovery startup timer
            VRF(VRFCond).HRTimer = 0.0;
            VRF(VRFCond).HRHeatingActive = false;
            VRF(VRFCond).HRCoolingActive = false;
        }

        // loop through TU's and calculate average inlet conditions for active coils
        for (NumTU = 1; NumTU <= NumTUInList; ++NumTU) {
            TUIndex = TerminalUnitList(TUListNum).ZoneTUPtr(NumTU);
            CoolCoilIndex = VRFTU(TUIndex).CoolCoilIndex;
            HeatCoilIndex = VRFTU(TUIndex).HeatCoilIndex;
            TUParasiticPower += VRFTU(TUIndex).ParasiticCoolElecPower + VRFTU(TUIndex).ParasiticHeatElecPower;
            TUFanPower += VRFTU(TUIndex).FanPower;

            if (TerminalUnitList(TUListNum).TotalCoolLoad(NumTU) > 0.0) {
                SumCoolInletWB += DXCoilCoolInletAirWBTemp(CoolCoilIndex) * TerminalUnitList(TUListNum).TotalCoolLoad(NumTU) / TUCoolingLoad;
                ++NumTUInCoolingMode;
            }
            if (TerminalUnitList(TUListNum).TotalHeatLoad(NumTU) > 0.0) {
                SumHeatInletDB += DXCoilHeatInletAirDBTemp(HeatCoilIndex) * TerminalUnitList(TUListNum).TotalHeatLoad(NumTU) / TUHeatingLoad;
                SumHeatInletWB += DXCoilHeatInletAirWBTemp(HeatCoilIndex) * TerminalUnitList(TUListNum).TotalHeatLoad(NumTU) / TUHeatingLoad;
                ++NumTUInHeatingMode;
            }
        }

        bool CoolingCoilAvailableFlag = any(TerminalUnitList(TUListNum).CoolingCoilAvailable);
        bool HeatingCoilAvailableFlag = any(TerminalUnitList(TUListNum).HeatingCoilAvailable);

        // calculate capacities and energy use
        if (CoolingLoad(VRFCond) && CoolingCoilAvailableFlag) {
            InletAirWetBulbC = SumCoolInletWB;
            TotCoolCapTempModFac = CurveValue(state, VRF(VRFCond).CoolCapFT, InletAirWetBulbC, CondInletTemp);
            TotCoolEIRTempModFac = CurveValue(state, VRF(VRFCond).CoolEIRFT, InletAirWetBulbC, CondInletTemp);

            // recalculate cooling Cap and EIR curve output if using boundary curve along with dual Cap and EIR curves.
            if (VRF(VRFCond).CoolBoundaryCurvePtr > 0) {
                CoolOABoundary = CurveValue(state, VRF(VRFCond).CoolBoundaryCurvePtr, InletAirWetBulbC);
                if (OutdoorDryBulb > CoolOABoundary) {
                    if (VRF(VRFCond).CoolCapFTHi > 0) TotCoolCapTempModFac = CurveValue(state, VRF(VRFCond).CoolCapFTHi, InletAirWetBulbC, CondInletTemp);
                }
            }
            if (VRF(VRFCond).EIRCoolBoundaryCurvePtr > 0) {
                CoolOABoundary = CurveValue(state, VRF(VRFCond).EIRCoolBoundaryCurvePtr, InletAirWetBulbC);
                if (OutdoorDryBulb > CoolOABoundary) {
                    if (VRF(VRFCond).CoolEIRFTHi > 0) TotCoolEIRTempModFac = CurveValue(state, VRF(VRFCond).CoolEIRFTHi, InletAirWetBulbC, CondInletTemp);
                }
            }

            //   Warn user if curve output goes negative
            if (TotCoolCapTempModFac < 0.0) {
                if (!WarmupFlag && NumTUInCoolingMode > 0) {
                    if (VRF(VRFCond).CoolCapFTErrorIndex == 0) {
                        ShowSevereMessage(cVRFTypes(VRF_HeatPump) + " \"" + VRF(VRFCond).Name + "\":");
                        ShowContinueError(" Cooling Capacity Modifier curve (function of temperature) output is negative (" +
                                          TrimSigDigits(TotCoolCapTempModFac, 3) + ").");
                        ShowContinueError(" Negative value occurs using an outdoor air temperature of " + TrimSigDigits(CondInletTemp, 1) +
                                          " C and an average indoor air wet-bulb temperature of " + TrimSigDigits(InletAirWetBulbC, 1) + " C.");
                        ShowContinueErrorTimeStamp(" Resetting curve output to zero and continuing simulation.");
                    }
                    ShowRecurringWarningErrorAtEnd(
                        ccSimPlantEquipTypes(TypeOf_HeatPumpVRF) + " \"" + VRF(VRFCond).Name +
                            "\": Cooling Capacity Modifier curve (function of temperature) output is negative warning continues...",
                        VRF(VRFCond).CoolCapFTErrorIndex,
                        TotCoolCapTempModFac,
                        TotCoolCapTempModFac);
                    TotCoolCapTempModFac = 0.0;
                }
            }

            //   Warn user if curve output goes negative
            if (TotCoolEIRTempModFac < 0.0) {
                if (!WarmupFlag && NumTUInCoolingMode > 0) {
                    if (VRF(VRFCond).EIRFTempCoolErrorIndex == 0) {
                        ShowSevereMessage(cVRFTypes(VRF_HeatPump) + " \"" + VRF(VRFCond).Name + "\":");
                        ShowContinueError(" Cooling Energy Input Ratio Modifier curve (function of temperature) output is negative (" +
                                          TrimSigDigits(TotCoolEIRTempModFac, 3) + ").");
                        ShowContinueError(" Negative value occurs using an outdoor air temperature of " + TrimSigDigits(CondInletTemp, 1) +
                                          " C and an average indoor air wet-bulb temperature of " + TrimSigDigits(InletAirWetBulbC, 1) + " C.");
                        ShowContinueErrorTimeStamp(" Resetting curve output to zero and continuing simulation.");
                    }
                    ShowRecurringWarningErrorAtEnd(
                        ccSimPlantEquipTypes(TypeOf_HeatPumpVRF) + " \"" + VRF(VRFCond).Name +
                            "\": Cooling Energy Input Ratio Modifier curve (function of temperature) output is negative warning continues...",
                        VRF(VRFCond).EIRFTempCoolErrorIndex,
                        TotCoolEIRTempModFac,
                        TotCoolEIRTempModFac);
                    TotCoolEIRTempModFac = 0.0;
                }
            }

            TotalCondCoolingCapacity = VRF(VRFCond).CoolingCapacity * CoolCombinationRatio(VRFCond) * TotCoolCapTempModFac;
            TotalTUCoolingCapacity = TotalCondCoolingCapacity * VRF(VRFCond).PipingCorrectionCooling;

            if (TotalCondCoolingCapacity > 0.0) {
                CoolingPLR = (TUCoolingLoad / VRF(VRFCond).PipingCorrectionCooling) / TotalCondCoolingCapacity;
            } else {
                CoolingPLR = 0.0;
            }

            //   Warn user if curve output goes negative
            if (TotCoolCapTempModFac < 0.0) {
                if (!WarmupFlag && NumTUInCoolingMode > 0) {
                    if (VRF(VRFCond).CoolCapFTErrorIndex == 0) {
                        ShowSevereMessage(cVRFTypes(VRF_HeatPump) + " \"" + VRF(VRFCond).Name + "\":");
                        ShowContinueError(" Cooling Capacity Modifier curve (function of temperature) output is negative (" +
                                          TrimSigDigits(TotCoolCapTempModFac, 3) + ").");
                        ShowContinueError(" Negative value occurs using an outdoor air temperature of " + TrimSigDigits(CondInletTemp, 1) +
                                          " C and an average indoor air wet-bulb temperature of " + TrimSigDigits(InletAirWetBulbC, 1) + " C.");
                        ShowContinueErrorTimeStamp(" Resetting curve output to zero and continuing simulation.");
                    }
                    ShowRecurringWarningErrorAtEnd(
                        ccSimPlantEquipTypes(TypeOf_HeatPumpVRF) + " \"" + VRF(VRFCond).Name +
                            "\": Cooling Capacity Modifier curve (function of temperature) output is negative warning continues...",
                        VRF(VRFCond).CoolCapFTErrorIndex,
                        TotCoolCapTempModFac,
                        TotCoolCapTempModFac);
                    TotCoolCapTempModFac = 0.0;
                }
            }
            //   Warn user if curve output goes negative
            if (TotCoolEIRTempModFac < 0.0) {
                if (!WarmupFlag && NumTUInCoolingMode > 0) {
                    if (VRF(VRFCond).EIRFTempCoolErrorIndex == 0) {
                        ShowSevereMessage(cVRFTypes(VRF_HeatPump) + " \"" + VRF(VRFCond).Name + "\":");
                        ShowContinueError(" Cooling Energy Input Ratio Modifier curve (function of temperature) output is negative (" +
                                          TrimSigDigits(TotCoolEIRTempModFac, 3) + ").");
                        ShowContinueError(" Negative value occurs using an outdoor air temperature of " + TrimSigDigits(CondInletTemp, 1) +
                                          " C and an average indoor air wet-bulb temperature of " + TrimSigDigits(InletAirWetBulbC, 1) + " C.");
                        ShowContinueErrorTimeStamp(" Resetting curve output to zero and continuing simulation.");
                    }
                    ShowRecurringWarningErrorAtEnd(
                        ccSimPlantEquipTypes(TypeOf_HeatPumpVRF) + " \"" + VRF(VRFCond).Name +
                            "\": Cooling Energy Input Ratio Modifier curve (function of temperature) output is negative warning continues...",
                        VRF(VRFCond).EIRFTempCoolErrorIndex,
                        TotCoolEIRTempModFac,
                        TotCoolEIRTempModFac);
                    TotCoolEIRTempModFac = 0.0;
                }
            }

        } else if (HeatingLoad(VRFCond) && HeatingCoilAvailableFlag) {
            InletAirDryBulbC = SumHeatInletDB;
            InletAirWetBulbC = SumHeatInletWB;
            {
                auto const SELECT_CASE_var(VRF(VRFCond).HeatingPerformanceOATType);
                if (SELECT_CASE_var == DataHVACGlobals::DryBulbIndicator) {
                    TotHeatCapTempModFac = CurveValue(state, VRF(VRFCond).HeatCapFT, InletAirDryBulbC, CondInletTemp);
                    TotHeatEIRTempModFac = CurveValue(state, VRF(VRFCond).HeatEIRFT, InletAirDryBulbC, CondInletTemp);
                } else if (SELECT_CASE_var == DataHVACGlobals::WetBulbIndicator) {
                    TotHeatCapTempModFac = CurveValue(state, VRF(VRFCond).HeatCapFT, InletAirDryBulbC, OutdoorWetBulb);
                    TotHeatEIRTempModFac = CurveValue(state, VRF(VRFCond).HeatEIRFT, InletAirDryBulbC, OutdoorWetBulb);
                } else {
                    TotHeatCapTempModFac = 1.0;
                    TotHeatEIRTempModFac = 1.0;
                }
            }
            // recalculate heating Cap and EIR curve output if using boundary curve along with dual Cap and EIR curves.
            if (VRF(VRFCond).HeatBoundaryCurvePtr > 0) {
                HeatOABoundary = CurveValue(state, VRF(VRFCond).HeatBoundaryCurvePtr, InletAirDryBulbC);
                {
                    auto const SELECT_CASE_var(VRF(VRFCond).HeatingPerformanceOATType);
                    if (SELECT_CASE_var == DataHVACGlobals::DryBulbIndicator) {
                        if (OutdoorDryBulb > HeatOABoundary) {
                            if (VRF(VRFCond).HeatCapFTHi > 0)
                                TotHeatCapTempModFac = CurveValue(state, VRF(VRFCond).HeatCapFTHi, InletAirDryBulbC, CondInletTemp);
                        }
                    } else if (SELECT_CASE_var == DataHVACGlobals::WetBulbIndicator) {
                        if (OutdoorWetBulb > HeatOABoundary) {
                            if (VRF(VRFCond).HeatCapFTHi > 0)
                                TotHeatCapTempModFac = CurveValue(state, VRF(VRFCond).HeatCapFTHi, InletAirDryBulbC, OutdoorWetBulb);
                        }
                    } else {
                        TotHeatCapTempModFac = 1.0;
                    }
                }
            }
            if (VRF(VRFCond).EIRHeatBoundaryCurvePtr > 0) {
                HeatOABoundary = CurveValue(state, VRF(VRFCond).EIRHeatBoundaryCurvePtr, InletAirDryBulbC);
                {
                    auto const SELECT_CASE_var(VRF(VRFCond).HeatingPerformanceOATType);
                    if (SELECT_CASE_var == DataHVACGlobals::DryBulbIndicator) {
                        if (OutdoorDryBulb > HeatOABoundary) {
                            if (VRF(VRFCond).HeatEIRFTHi > 0)
                                TotHeatEIRTempModFac = CurveValue(state, VRF(VRFCond).HeatEIRFTHi, InletAirDryBulbC, CondInletTemp);
                        }
                    } else if (SELECT_CASE_var == DataHVACGlobals::WetBulbIndicator) {
                        if (OutdoorWetBulb > HeatOABoundary) {
                            if (VRF(VRFCond).HeatEIRFTHi > 0)
                                TotHeatEIRTempModFac = CurveValue(state, VRF(VRFCond).HeatEIRFTHi, InletAirDryBulbC, OutdoorWetBulb);
                        }
                    } else {
                        TotHeatEIRTempModFac = 1.0;
                    }
                }
            }

            //   Warn user if curve output goes negative
            if (TotHeatCapTempModFac < 0.0) {
                if (!WarmupFlag && NumTUInHeatingMode > 0) {
                    if (VRF(VRFCond).HeatCapFTErrorIndex == 0) {
                        ShowSevereMessage(cVRFTypes(VRF_HeatPump) + " \"" + VRF(VRFCond).Name + "\":");
                        ShowContinueError(" Heating Capacity Modifier curve (function of temperature) output is negative (" +
                                          TrimSigDigits(TotHeatCapTempModFac, 3) + ").");
                        auto const SELECT_CASE_var(VRF(VRFCond).HeatingPerformanceOATType);
                        if (SELECT_CASE_var == DataHVACGlobals::DryBulbIndicator) {
                            ShowContinueError(" Negative value occurs using an outdoor air temperature of " + TrimSigDigits(CondInletTemp, 1) +
                                              " C and an average indoor air dry-bulb temperature of " + TrimSigDigits(InletAirDryBulbC, 1) + " C.");
                        } else if (SELECT_CASE_var == DataHVACGlobals::WetBulbIndicator) {
                            ShowContinueError(" Negative value occurs using an outdoor air wet-bulb temperature of " +
                                              TrimSigDigits(OutdoorWetBulb, 1) + " C and an average indoor air wet-bulb temperature of " +
                                              TrimSigDigits(InletAirWetBulbC, 1) + " C.");
                        } else {
                            // should never get here
                        }
                        ShowContinueErrorTimeStamp(" Resetting curve output to zero and continuing simulation.");
                    }
                    ShowRecurringWarningErrorAtEnd(
                        ccSimPlantEquipTypes(TypeOf_HeatPumpVRF) + " \"" + VRF(VRFCond).Name +
                            "\": Heating Capacity Ratio Modifier curve (function of temperature) output is negative warning continues...",
                        VRF(VRFCond).HeatCapFTErrorIndex,
                        TotHeatCapTempModFac,
                        TotHeatCapTempModFac);
                    TotHeatCapTempModFac = 0.0;
                }
            }
            //   Warn user if curve output goes negative
            if (TotHeatEIRTempModFac < 0.0) {
                if (!WarmupFlag && NumTUInHeatingMode > 0) {
                    if (VRF(VRFCond).EIRFTempHeatErrorIndex == 0) {
                        ShowSevereMessage(cVRFTypes(VRF_HeatPump) + " \"" + VRF(VRFCond).Name + "\":");
                        ShowContinueError(" Heating Energy Input Ratio Modifier curve (function of temperature) output is negative (" +
                                          TrimSigDigits(TotHeatEIRTempModFac, 3) + ").");
                        {
                            auto const SELECT_CASE_var(VRF(VRFCond).HeatingPerformanceOATType);
                            if (SELECT_CASE_var == DataHVACGlobals::DryBulbIndicator) {
                                ShowContinueError(" Negative value occurs using an outdoor air dry-bulb temperature of " +
                                                  TrimSigDigits(CondInletTemp, 1) + " C and an average indoor air dry-bulb temperature of " +
                                                  TrimSigDigits(InletAirDryBulbC, 1) + " C.");
                            } else if (SELECT_CASE_var == DataHVACGlobals::WetBulbIndicator) {
                                ShowContinueError(" Negative value occurs using an outdoor air wet-bulb temperature of " +
                                                  TrimSigDigits(OutdoorWetBulb, 1) + " C and an average indoor air wet-bulb temperature of " +
                                                  TrimSigDigits(InletAirWetBulbC, 1) + " C.");
                            } else {
                            }
                        }
                        ShowContinueErrorTimeStamp(" Resetting curve output to zero and continuing simulation.");
                    }
                    ShowRecurringWarningErrorAtEnd(
                        ccSimPlantEquipTypes(TypeOf_HeatPumpVRF) + " \"" + VRF(VRFCond).Name +
                            "\": Heating Energy Input Ratio Modifier curve (function of temperature) output is negative warning continues...",
                        VRF(VRFCond).EIRFTempHeatErrorIndex,
                        TotHeatEIRTempModFac,
                        TotHeatEIRTempModFac);
                    TotHeatEIRTempModFac = 0.0;
                }
            }

            // Initializing defrost adjustment factors
            LoadDueToDefrost = 0.0;
            HeatingCapacityMultiplier = 1.0;
            FractionalDefrostTime = 0.0;
            InputPowerMultiplier = 1.0;

            // Check outdoor temperature to determine of defrost is active
            if (OutdoorDryBulb <= VRF(VRFCond).MaxOATDefrost && VRF(VRFCond).CondenserType != DataHVACGlobals::WaterCooled) {

                // Calculating adjustment factors for defrost
                // Calculate delta w through outdoor coil by assuming a coil temp of 0.82*DBT-9.7(F) per DOE2.1E
                OutdoorCoilT = 0.82 * OutdoorDryBulb - 8.589;
                OutdoorCoildw = max(1.0e-6, (OutdoorHumRat - PsyWFnTdpPb(OutdoorCoilT, OutdoorPressure)));

                // Calculate defrost adjustment factors depending on defrost control type
                if (VRF(VRFCond).DefrostControl == Timed) {
                    FractionalDefrostTime = VRF(VRFCond).DefrostFraction;
                    if (FractionalDefrostTime > 0.0) {
                        HeatingCapacityMultiplier = 0.909 - 107.33 * OutdoorCoildw;
                        InputPowerMultiplier = 0.90 - 36.45 * OutdoorCoildw;
                    }
                } else { // else defrost control is on-demand
                    FractionalDefrostTime = 1.0 / (1.0 + 0.01446 / OutdoorCoildw);
                    HeatingCapacityMultiplier = 0.875 * (1.0 - FractionalDefrostTime);
                    InputPowerMultiplier = 0.954 * (1.0 - FractionalDefrostTime);
                }

                if (FractionalDefrostTime > 0.0) {
                    // Calculate defrost adjustment factors depending on defrost control strategy
                    if (VRF(VRFCond).DefrostStrategy == ReverseCycle) {
                        LoadDueToDefrost = (0.01 * FractionalDefrostTime) * (7.222 - OutdoorDryBulb) * (VRF(VRFCond).HeatingCapacity / 1.01667);
                        DefrostEIRTempModFac = CurveValue(state, VRF(VRFCond).DefrostEIRPtr, max(15.555, InletAirWetBulbC), max(15.555, OutdoorDryBulb));

                        //         Warn user if curve output goes negative
                        if (DefrostEIRTempModFac < 0.0) {
                            if (!WarmupFlag) {
                                if (VRF(VRFCond).DefrostHeatErrorIndex == 0) {
                                    ShowSevereMessage(cVRFTypes(VRF_HeatPump) + " \"" + VRF(VRFCond).Name + "\":");
                                    ShowContinueError(" Defrost Energy Input Ratio Modifier curve (function of temperature) output is negative (" +
                                                      TrimSigDigits(DefrostEIRTempModFac, 3) + ").");
                                    ShowContinueError(" Negative value occurs using an outdoor air dry-bulb temperature of " +
                                                      TrimSigDigits(OutdoorDryBulb, 1) + " C and an average indoor air wet-bulb temperature of " +
                                                      TrimSigDigits(InletAirWetBulbC, 1) + " C.");
                                    ShowContinueErrorTimeStamp(" Resetting curve output to zero and continuing simulation.");
                                }
                                ShowRecurringWarningErrorAtEnd(ccSimPlantEquipTypes(TypeOf_HeatPumpVRF) + " \"" + VRF(VRFCond).Name +
                                                                   "\": Defrost Energy Input Ratio Modifier curve (function of temperature) output "
                                                                   "is negative warning continues...",
                                                               VRF(VRFCond).DefrostHeatErrorIndex,
                                                               DefrostEIRTempModFac,
                                                               DefrostEIRTempModFac);
                                DefrostEIRTempModFac = 0.0;
                            }
                        }

                        VRF(VRFCond).DefrostPower = DefrostEIRTempModFac * (VRF(VRFCond).HeatingCapacity / 1.01667) * FractionalDefrostTime;

                    } else { // Defrost strategy is resistive
                        VRF(VRFCond).DefrostPower = VRF(VRFCond).DefrostCapacity * FractionalDefrostTime;
                    }
                }
            }

            TotalCondHeatingCapacity =
                VRF(VRFCond).HeatingCapacity * HeatCombinationRatio(VRFCond) * TotHeatCapTempModFac * HeatingCapacityMultiplier;
            TotalTUHeatingCapacity = TotalCondHeatingCapacity * VRF(VRFCond).PipingCorrectionHeating;
            if (TotalCondHeatingCapacity > 0.0) {
                HeatingPLR = (TUHeatingLoad / VRF(VRFCond).PipingCorrectionHeating) / TotalCondHeatingCapacity;
                HeatingPLR += (LoadDueToDefrost * HeatingPLR) / TotalCondHeatingCapacity;
            } else {
                HeatingPLR = 0.0;
            }
        }

        VRF(VRFCond).VRFCondPLR = max(CoolingPLR, HeatingPLR);
        Real64 tmpVRFCondPLR = 0.0;
        if (CoolingPLR > 0.0 || HeatingPLR > 0.0) tmpVRFCondPLR = max(VRF(VRFCond).MinPLR, VRF(VRFCond).VRFCondPLR);

        HRHeatRequestFlag = any(TerminalUnitList(TUListNum).HRHeatRequest);
        HRCoolRequestFlag = any(TerminalUnitList(TUListNum).HRCoolRequest);
        HREIRFTConst = 1.0;
        Real64 HREIRAdjustment = 1.0;

        if (!DoingSizing && !WarmupFlag) {
            if (HRHeatRequestFlag && HRCoolRequestFlag) {
                // determine operating mode change
                if (!VRF(VRFCond).HRCoolingActive && !VRF(VRFCond).HRHeatingActive) {
                    VRF(VRFCond).ModeChange = true;
                    VRF(VRFCond).HRTimer = 0.0;
                }
                if (CoolingLoad(VRFCond)) {
                    if (VRF(VRFCond).HRHeatingActive && !VRF(VRFCond).HRCoolingActive) {
                        VRF(VRFCond).HRModeChange = true;
                    }
                    VRF(VRFCond).HRCoolingActive = true;
                    VRF(VRFCond).HRHeatingActive = false;
                    HRCAPFT = VRF(VRFCond).HRCAPFTCool; // Index to cool capacity as a function of temperature\PLR curve for heat recovery
                    if (HRCAPFT > 0) {
                        //         VRF(VRFCond)%HRCAPFTCoolConst = 0.9d0 ! initialized to 0.9
                        if (state.dataCurveManager->PerfCurve(VRF(VRFCond).HRCAPFTCool).NumDims == 2) { // Curve type for HRCAPFTCool
                            VRF(VRFCond).HRCAPFTCoolConst = CurveValue(state, HRCAPFT, InletAirWetBulbC, CondInletTemp);
                        } else {
                            VRF(VRFCond).HRCAPFTCoolConst = CurveValue(state, HRCAPFT, tmpVRFCondPLR);
                        }
                    }
                    HRCAPFTConst = VRF(VRFCond).HRCAPFTCoolConst;
                    HRInitialCapFrac =
                        VRF(VRFCond).HRInitialCoolCapFrac; // Fractional cooling degradation at the start of heat recovery from cooling mode
                    HRCapTC = VRF(VRFCond).HRCoolCapTC;    // Time constant used to recover from initial degradation in cooling heat recovery

                    HREIRFT = VRF(VRFCond).HREIRFTCool; // Index to cool EIR as a function of temperature curve for heat recovery
                    if (HREIRFT > 0) {
                        //         VRF(VRFCond)%HREIRFTCoolConst = 1.1d0 ! initialized to 1.1
                        if (state.dataCurveManager->PerfCurve(VRF(VRFCond).HREIRFTCool).NumDims == 2) { // Curve type for HREIRFTCool
                            VRF(VRFCond).HREIRFTCoolConst = CurveValue(state, HREIRFT, InletAirWetBulbC, CondInletTemp);
                        } else {
                            VRF(VRFCond).HREIRFTCoolConst = CurveValue(state, HREIRFT, tmpVRFCondPLR);
                        }
                    }
                    HREIRFTConst = VRF(VRFCond).HREIRFTCoolConst;
                    HRInitialEIRFrac =
                        VRF(VRFCond).HRInitialCoolEIRFrac; // Fractional cooling degradation at the start of heat recovery from cooling mode
                    HREIRTC = VRF(VRFCond).HRCoolEIRTC;    // Time constant used to recover from initial degradation in cooling heat recovery
                } else if (HeatingLoad(VRFCond)) {
                    if (!VRF(VRFCond).HRHeatingActive && VRF(VRFCond).HRCoolingActive) {
                        VRF(VRFCond).HRModeChange = true;
                    }
                    VRF(VRFCond).HRCoolingActive = false;
                    VRF(VRFCond).HRHeatingActive = true;
                    HRCAPFT = VRF(VRFCond).HRCAPFTHeat; // Index to heat capacity as a function of temperature\PLR curve for heat recovery
                    if (HRCAPFT > 0) {
                        //         VRF(VRFCond)%HRCAPFTHeatConst = 1.1d0 ! initialized to 1.1
                        if (state.dataCurveManager->PerfCurve(VRF(VRFCond).HRCAPFTHeat).NumDims == 2) { // Curve type for HRCAPFTCool
                            {
                                auto const SELECT_CASE_var(VRF(VRFCond).HeatingPerformanceOATType);
                                if (SELECT_CASE_var == DataHVACGlobals::DryBulbIndicator) {
                                    VRF(VRFCond).HRCAPFTHeatConst = CurveValue(state, HRCAPFT, InletAirDryBulbC, CondInletTemp);
                                } else if (SELECT_CASE_var == DataHVACGlobals::WetBulbIndicator) {
                                    VRF(VRFCond).HRCAPFTHeatConst = CurveValue(state, HRCAPFT, InletAirDryBulbC, OutdoorWetBulb);
                                } else {
                                    VRF(VRFCond).HRCAPFTHeatConst = 1.0;
                                }
                            }
                        } else {
                            VRF(VRFCond).HRCAPFTHeatConst = CurveValue(state, HRCAPFT, tmpVRFCondPLR);
                        }
                    }
                    HRCAPFTConst = VRF(VRFCond).HRCAPFTHeatConst;
                    HRInitialCapFrac =
                        VRF(VRFCond).HRInitialHeatCapFrac; // Fractional heating degradation at the start of heat recovery from cooling mode
                    HRCapTC = VRF(VRFCond).HRHeatCapTC;    // Time constant used to recover from initial degradation in heating heat recovery

                    HREIRFT = VRF(VRFCond).HREIRFTHeat; // Index to cool EIR as a function of temperature curve for heat recovery
                    if (HREIRFT > 0) {
                        //         VRF(VRFCond)%HREIRFTCoolConst = 1.1d0 ! initialized to 1.1
                        if (state.dataCurveManager->PerfCurve(VRF(VRFCond).HREIRFTHeat).NumDims == 2) { // Curve type for HREIRFTHeat
                            {
                                auto const SELECT_CASE_var(VRF(VRFCond).HeatingPerformanceOATType);
                                if (SELECT_CASE_var == DataHVACGlobals::DryBulbIndicator) {
                                    VRF(VRFCond).HREIRFTHeatConst = CurveValue(state, HREIRFT, InletAirDryBulbC, CondInletTemp);
                                } else if (SELECT_CASE_var == DataHVACGlobals::WetBulbIndicator) {
                                    VRF(VRFCond).HREIRFTHeatConst = CurveValue(state, HREIRFT, InletAirDryBulbC, OutdoorWetBulb);
                                } else {
                                    VRF(VRFCond).HREIRFTHeatConst = 1.0;
                                }
                            }
                        } else {
                            VRF(VRFCond).HREIRFTHeatConst = CurveValue(state, HREIRFT, tmpVRFCondPLR);
                        }
                    }
                    HREIRFTConst = VRF(VRFCond).HREIRFTHeatConst;
                    HRInitialEIRFrac =
                        VRF(VRFCond).HRInitialHeatEIRFrac; // Fractional heating degradation at the start of heat recovery from heating mode
                    HREIRTC = VRF(VRFCond).HRHeatEIRTC;    // Time constant used to recover from initial degradation in heating heat recovery
                } else {
                    //   zone thermostats satisfied, condenser is off. Set values anyway
                    HRCAPFTConst = 1.0;
                    HRInitialCapFrac = 1.0;
                    HRCapTC = 1.0;
                    HREIRFTConst = 1.0;
                    HRInitialEIRFrac = 1.0;
                    HREIRTC = 1.0;
                    if (VRF(VRFCond).HRHeatingActive || VRF(VRFCond).HRCoolingActive) {
                        VRF(VRFCond).HRModeChange = true;
                    }
                    VRF(VRFCond).HRCoolingActive = false;
                    VRF(VRFCond).HRHeatingActive = false;
                }

            } else { // IF(HRHeatRequestFlag .AND. HRCoolRequestFlag)THEN -- Heat recovery turned off
                HRCAPFTConst = 1.0;
                HRInitialCapFrac = 1.0;
                HRCapTC = 0.0;
                HREIRFTConst = 1.0;
                HRInitialEIRFrac = 1.0;
                HREIRTC = 0.0;
                VRF(VRFCond).HRModeChange = false;
                VRF(VRFCond).HRCoolingActive = false;
                VRF(VRFCond).HRHeatingActive = false;
                VRF(VRFCond).HRTimer = 0.0;
            }

            // calculate end time of current time step to determine if max capacity reset is required
            CurrentEndTime = double((DayOfSim - 1) * 24) + CurrentTime - TimeStepZone + DataHVACGlobals::SysTimeElapsed;

            if (VRF(VRFCond).ModeChange || VRF(VRFCond).HRModeChange) {
                if (VRF(VRFCond).HRCoolingActive && VRF(VRFCond).HRTimer == 0.0) {
                    VRF(VRFCond).HRTimer = CurrentEndTimeLast;
                } else if (VRF(VRFCond).HRHeatingActive && VRF(VRFCond).HRTimer == 0.0) {
                    VRF(VRFCond).HRTimer = CurrentEndTimeLast;
                } else if (!VRF(VRFCond).HRCoolingActive && !VRF(VRFCond).HRHeatingActive) {
                    VRF(VRFCond).HRTimer = 0.0;
                }
            }

            VRF(VRFCond).HRTime = max(0.0, CurrentEndTime - VRF(VRFCond).HRTimer);
            if (VRF(VRFCond).HRTime < (HRCapTC * 5.0)) {
                if (HRCapTC > 0.0) {
                    SUMultiplier = min(1.0, 1.0 - std::exp(-VRF(VRFCond).HRTime / HRCapTC));
                } else {
                    SUMultiplier = 1.0;
                }
            } else {
                SUMultiplier = 1.0;
                VRF(VRFCond).ModeChange = false;
                VRF(VRFCond).HRModeChange = false;
            }
            VRF(VRFCond).SUMultiplier = SUMultiplier;

            TimeStepSysLast = DataHVACGlobals::TimeStepSys;
            CurrentEndTimeLast = CurrentEndTime;

            if (VRF(VRFCond).HeatRecoveryUsed && VRF(VRFCond).HRCoolingActive) {
                TotalCondCoolingCapacity *= HRCAPFTConst;
                TotalCondCoolingCapacity =
                    HRInitialCapFrac * TotalCondCoolingCapacity + (1.0 - HRInitialCapFrac) * TotalCondCoolingCapacity * SUMultiplier;
                TotalTUCoolingCapacity = TotalCondCoolingCapacity * VRF(VRFCond).PipingCorrectionCooling;
                if (TotalCondCoolingCapacity > 0.0) {
                    CoolingPLR = min(1.0, (TUCoolingLoad / VRF(VRFCond).PipingCorrectionCooling) / TotalCondCoolingCapacity);
                } else {
                    CoolingPLR = 0.0;
                }
                HREIRAdjustment = HRInitialEIRFrac + (HREIRFTConst - HRInitialEIRFrac) * SUMultiplier;
                VRF(VRFCond).VRFHeatRec = TUHeatingLoad;
            } else if (VRF(VRFCond).HeatRecoveryUsed && VRF(VRFCond).HRHeatingActive) {
                TotalCondHeatingCapacity *= HRCAPFTConst;
                TotalCondHeatingCapacity =
                    HRInitialCapFrac * TotalCondHeatingCapacity + (1.0 - HRInitialCapFrac) * TotalCondHeatingCapacity * SUMultiplier;
                TotalTUHeatingCapacity = TotalCondHeatingCapacity * VRF(VRFCond).PipingCorrectionHeating;
                if (TotalCondHeatingCapacity > 0.0) {
                    HeatingPLR = min(1.0, (TUHeatingLoad / VRF(VRFCond).PipingCorrectionHeating) / TotalCondHeatingCapacity);
                } else {
                    HeatingPLR = 0.0;
                }
                HREIRAdjustment = HRInitialEIRFrac + (HREIRFTConst - HRInitialEIRFrac) * SUMultiplier;
                VRF(VRFCond).VRFHeatRec = TUCoolingLoad;
            }
            VRF(VRFCond).VRFCondPLR = max(CoolingPLR, HeatingPLR);
        }

        if (VRF(VRFCond).MinPLR > 0.0) {
            CyclingRatio = min(1.0, VRF(VRFCond).VRFCondPLR / VRF(VRFCond).MinPLR);
            if (VRF(VRFCond).VRFCondPLR < VRF(VRFCond).MinPLR && VRF(VRFCond).VRFCondPLR > 0.0) {
                VRF(VRFCond).VRFCondPLR = VRF(VRFCond).MinPLR;
                if (CoolingPLR > 0.0) CoolingPLR = VRF(VRFCond).MinPLR; // also adjust local PLR variables
                if (HeatingPLR > 0.0) HeatingPLR = VRF(VRFCond).MinPLR; // also adjust local PLR variables
            }
        }
        VRF(VRFCond).VRFCondCyclingRatio = CyclingRatio; // report variable for cycling rate
        VRF(VRFCond).TotalCoolingCapacity = TotalCondCoolingCapacity * CoolingPLR * CyclingRatio;
        VRF(VRFCond).TotalHeatingCapacity = TotalCondHeatingCapacity * HeatingPLR * CyclingRatio;

        VRF(VRFCond).OperatingMode = 0; // report variable for heating or cooling mode
        EIRFPLRModFac = 1.0;
        VRFRTF = 0.0;
        // cooling and heating is optional (only one may exist), if so then performance curve for missing coil are not required
        if (CoolingLoad(VRFCond) && CoolingPLR > 0.0) {
            VRF(VRFCond).OperatingMode = ModeCoolingOnly;
            if (CoolingPLR > 1.0) {
                if (VRF(VRFCond).CoolEIRFPLR2 > 0) EIRFPLRModFac = CurveValue(state, VRF(VRFCond).CoolEIRFPLR2, max(VRF(VRFCond).MinPLR, CoolingPLR));
            } else {
                if (VRF(VRFCond).CoolEIRFPLR1 > 0) EIRFPLRModFac = CurveValue(state, VRF(VRFCond).CoolEIRFPLR1, max(VRF(VRFCond).MinPLR, CoolingPLR));
            }
            // find part load fraction to calculate RTF
            if (VRF(VRFCond).CoolPLFFPLR > 0) {
                PartLoadFraction = max(0.7, CurveValue(state, VRF(VRFCond).CoolPLFFPLR, CyclingRatio));
            } else {
                PartLoadFraction = 1.0;
            }
            VRFRTF = min(1.0, (CyclingRatio / PartLoadFraction));

            VRF(VRFCond).ElecCoolingPower =
                (VRF(VRFCond).RatedCoolingPower * TotCoolCapTempModFac) * TotCoolEIRTempModFac * EIRFPLRModFac * HREIRAdjustment * VRFRTF;
        }
        if (HeatingLoad(VRFCond) && HeatingPLR > 0.0) {
            VRF(VRFCond).OperatingMode = ModeHeatingOnly;
            if (HeatingPLR > 1.0) {
                if (VRF(VRFCond).HeatEIRFPLR2 > 0) EIRFPLRModFac = CurveValue(state, VRF(VRFCond).HeatEIRFPLR2, max(VRF(VRFCond).MinPLR, HeatingPLR));
            } else {
                if (VRF(VRFCond).HeatEIRFPLR1 > 0) EIRFPLRModFac = CurveValue(state, VRF(VRFCond).HeatEIRFPLR1, max(VRF(VRFCond).MinPLR, HeatingPLR));
            }
            // find part load fraction to calculate RTF
            if (VRF(VRFCond).HeatPLFFPLR > 0) {
                PartLoadFraction = max(0.7, CurveValue(state, VRF(VRFCond).HeatPLFFPLR, CyclingRatio));
            } else {
                PartLoadFraction = 1.0;
            }
            VRFRTF = min(1.0, (CyclingRatio / PartLoadFraction));

            VRF(VRFCond).ElecHeatingPower = (VRF(VRFCond).RatedHeatingPower * TotHeatCapTempModFac) * TotHeatEIRTempModFac * EIRFPLRModFac *
                                            HREIRAdjustment * VRFRTF * InputPowerMultiplier;

            // adjust defrost power based on heating RTF
            VRF(VRFCond).DefrostPower *= VRFRTF;
        }
        VRF(VRFCond).VRFCondRTF = VRFRTF;

        // calculate crankcase heater power
        if (VRF(VRFCond).MaxOATCCHeater > OutdoorDryBulb) {
            // calculate crankcase heater power
            VRF(VRFCond).CrankCaseHeaterPower = VRF(VRFCond).CCHeaterPower * (1.0 - VRFRTF);
            if (VRF(VRFCond).NumCompressors > 1) {
                UpperStageCompressorRatio = (1.0 - VRF(VRFCond).CompressorSizeRatio) / (VRF(VRFCond).NumCompressors - 1);
                for (Stage = 1; Stage <= VRF(VRFCond).NumCompressors - 2; ++Stage) {
                    if (VRF(VRFCond).VRFCondPLR < (VRF(VRFCond).CompressorSizeRatio + Stage * UpperStageCompressorRatio)) {
                        VRF(VRFCond).CrankCaseHeaterPower += VRF(VRFCond).CCHeaterPower;
                    }
                }
            }
        } else {
            VRF(VRFCond).CrankCaseHeaterPower = 0.0;
        }

        CondCapacity = max(VRF(VRFCond).TotalCoolingCapacity, VRF(VRFCond).TotalHeatingCapacity);
        CondPower = max(VRF(VRFCond).ElecCoolingPower, VRF(VRFCond).ElecHeatingPower);
        if (VRF(VRFCond).ElecCoolingPower > 0.0) {
            VRF(VRFCond).QCondenser = CondCapacity + CondPower - VRF(VRFCond).TUHeatingLoad / VRF(VRFCond).PipingCorrectionHeating;
        } else if (VRF(VRFCond).ElecHeatingPower > 0.0) {
            VRF(VRFCond).QCondenser = -CondCapacity + CondPower + VRF(VRFCond).TUCoolingLoad / VRF(VRFCond).PipingCorrectionCooling;
        } else {
            VRF(VRFCond).QCondenser = 0.0;
        }

        if (VRF(VRFCond).CondenserType == DataHVACGlobals::EvapCooled) {
            // Calculate basin heater power
            CalcBasinHeaterPower(VRF(VRFCond).BasinHeaterPowerFTempDiff,
                                 VRF(VRFCond).BasinHeaterSchedulePtr,
                                 VRF(VRFCond).BasinHeaterSetPointTemp,
                                 VRF(VRFCond).BasinHeaterPower);
            VRF(VRFCond).BasinHeaterPower *= (1.0 - VRFRTF);

            // calculate evaporative condenser pump power and water consumption
            if (CoolingLoad(VRFCond) && CoolingPLR > 0.0) {
                //******************
                // WATER CONSUMPTION IN m3 OF WATER FOR DIRECT
                // H2O [m3/s] = Delta W[kgWater/kgDryAir]*Mass Flow Air[kgDryAir/s]
                //                    /RhoWater [kgWater/m3]
                //******************
                RhoWater = RhoH2O(OutdoorDryBulb);
                VRF(VRFCond).EvapWaterConsumpRate = (CondInletHumRat - OutdoorHumRat) * CondAirMassFlow / RhoWater * VRF(VRFCond).VRFCondPLR;
                VRF(VRFCond).EvapCondPumpElecPower = VRF(VRFCond).EvapCondPumpPower * VRFRTF;
            }
        } else if (VRF(VRFCond).CondenserType == DataHVACGlobals::WaterCooled) {

            if (CondCapacity > 0.0) {
                CondenserWaterMassFlowRate = CondWaterMassFlow;
            } else {
                CondenserWaterMassFlowRate = 0.0;
            }
            SetComponentFlowRate(CondenserWaterMassFlowRate,
                                 VRF(VRFCond).CondenserNodeNum,
                                 VRF(VRFCond).CondenserOutletNodeNum,
                                 VRF(VRFCond).SourceLoopNum,
                                 VRF(VRFCond).SourceLoopSideNum,
                                 VRF(VRFCond).SourceBranchNum,
                                 VRF(VRFCond).SourceCompNum);

            // should be the same as above just entering this function
            //			VRF( VRFCond ).CondenserInletTemp = DataLoopNode::Node(VRF(VRFCond).CondenserNodeNum).Temp;
            VRF(VRFCond).WaterCondenserMassFlow = DataLoopNode::Node(VRF(VRFCond).CondenserNodeNum).MassFlowRate;

            CpCond = GetSpecificHeatGlycol(state,
                                           PlantLoop(VRF(VRFCond).SourceLoopNum).FluidName,
                                           VRF(VRFCond).CondenserInletTemp,
                                           PlantLoop(VRF(VRFCond).SourceLoopNum).FluidIndex,
                                           RoutineName);
            if (CondWaterMassFlow > 0.0) {
                CondOutletTemp = VRF(VRFCond).QCondenser / (CondWaterMassFlow * CpCond) + CondInletTemp;
            } else {
                CondOutletTemp = CondInletTemp;
            }
            VRF(VRFCond).CondenserSideOutletTemp = CondOutletTemp;
        }

        // calculate operating COP
        if (CoolingLoad(VRFCond) && CoolingPLR > 0.0) {
            if (VRF(VRFCond).ElecCoolingPower != 0.0) {
                // this calc should use delivered capacity, not condenser capacity, use VRF(VRFCond)%TUCoolingLoad
                VRF(VRFCond).OperatingCoolingCOP =
                    (VRF(VRFCond).TotalCoolingCapacity) / (VRF(VRFCond).ElecCoolingPower + VRF(VRFCond).CrankCaseHeaterPower +
                                                           VRF(VRFCond).EvapCondPumpElecPower + VRF(VRFCond).DefrostPower);
            } else {
                VRF(VRFCond).OperatingCoolingCOP = 0.0;
            }
        }
        if (HeatingLoad(VRFCond) && HeatingPLR > 0.0) {
            if (VRF(VRFCond).ElecHeatingPower != 0.0) {
                // this calc should use delivered capacity, not condenser capacity, use VRF(VRFCond)%TUHeatingLoad
                VRF(VRFCond).OperatingHeatingCOP =
                    (VRF(VRFCond).TotalHeatingCapacity) / (VRF(VRFCond).ElecHeatingPower + VRF(VRFCond).CrankCaseHeaterPower +
                                                           VRF(VRFCond).EvapCondPumpElecPower + VRF(VRFCond).DefrostPower);
            } else {
                VRF(VRFCond).OperatingHeatingCOP = 0.0;
            }
        }

        TotPower = TUParasiticPower + TUFanPower + VRF(VRFCond).ElecHeatingPower + VRF(VRFCond).ElecCoolingPower + VRF(VRFCond).CrankCaseHeaterPower +
                   VRF(VRFCond).EvapCondPumpElecPower + VRF(VRFCond).DefrostPower;
        if (TotPower > 0.0) {
            VRF(VRFCond).OperatingCOP = (VRF(VRFCond).TUCoolingLoad + VRF(VRFCond).TUHeatingLoad) / TotPower;
            VRF(VRFCond).SCHE = VRF(VRFCond).OperatingCOP * 3.412141633; // see StandardRatings::ConvFromSIToIP
        }

        // limit the TU capacity when the condenser is maxed out on capacity
        // I think this next line will make the max cap report variable match the coil objects, will probably change the answer though
        //  IF(CoolingLoad(VRFCond) .AND. NumTUInCoolingMode .GT. 0 .AND. MaxCoolingCapacity(VRFCond) == MaxCap)THEN
        if (CoolingLoad(VRFCond) && NumTUInCoolingMode > 0) {

            //   IF TU capacity is greater than condenser capacity find maximum allowed TU capacity (i.e., conserve energy)
            if (TUCoolingLoad > TotalTUCoolingCapacity) {
                LimitTUCapacity(VRFCond,
                                NumTUInList,
                                TotalTUCoolingCapacity,
                                TerminalUnitList(TUListNum).TotalCoolLoad,
                                MaxCoolingCapacity(VRFCond),
                                TotalTUHeatingCapacity,
                                TerminalUnitList(TUListNum).TotalHeatLoad,
                                MaxHeatingCapacity(VRFCond));
            }
        } else if (HeatingLoad(VRFCond) && NumTUInHeatingMode > 0) {
            //   IF TU capacity is greater than condenser capacity
            if (TUHeatingLoad > TotalTUHeatingCapacity) {
                LimitTUCapacity(VRFCond,
                                NumTUInList,
                                TotalTUHeatingCapacity,
                                TerminalUnitList(TUListNum).TotalHeatLoad,
                                MaxHeatingCapacity(VRFCond),
                                TotalTUCoolingCapacity,
                                TerminalUnitList(TUListNum).TotalCoolLoad,
                                MaxCoolingCapacity(VRFCond));
            }
        } else {
        }
    }

    void GetVRFInput(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Richard Raustad, FSEC
        //       DATE WRITTEN   August 2015
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Manages GetInput processing and program termination

        // METHODOLOGY EMPLOYED:
        // Calls "Get" routines to read in data.

        // SUBROUTINE PARAMETER DEFINITIONS:
        static std::string const RoutineName("GetVRFInput: "); // include trailing blank space

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        bool ErrorsFound(false); // If errors detected in input

        // Flow
        GetVRFInputData(state, ErrorsFound);

        if (ErrorsFound) {
            ShowFatalError(RoutineName +
                           "Errors found in getting AirConditioner:VariableRefrigerantFlow system input. Preceding condition(s) causes termination.");
        }
    }

    void GetVRFInputData(EnergyPlusData &state, bool &ErrorsFound)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Richard Raustad, FSEC
        //       DATE WRITTEN   August 2010
        //       MODIFIED       July 2012, Chandan Sharma - FSEC: Added zone sys avail managers
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Obtains input data for VRF systems and stores it in data structures

        using namespace DataLoopNode;
        using BranchNodeConnections::SetUpCompSets;
        using BranchNodeConnections::TestCompSet;
        using CurveManager::checkCurveIsNormalizedToOne;
        using CurveManager::CurveValue;
        using CurveManager::GetCurveIndex;
        using CurveManager::SetCurveOutputMinMaxValues;
        using DXCoils::GetDXCoilIndex;
        using Fans::GetFanAvailSchPtr;
        using Fans::GetFanDesignVolumeFlowRate;
        using Fans::GetFanIndex;
        using Fans::GetFanInletNode;
        using Fans::GetFanOutletNode;
        using Fans::GetFanType;
        using General::TrimSigDigits;
        using MixedAir::GetOAMixerNodeNumbers;
        using NodeInputManager::GetOnlySingleNode;
        using ScheduleManager::CheckScheduleValueMinMax;
        using ScheduleManager::GetScheduleIndex;
        auto &GetDXCoilInletNode(DXCoils::GetCoilInletNode);
        auto &GetDXCoilOutletNode(DXCoils::GetCoilOutletNode);
        using DataHeatBalance::Zone;
        using DataSizing::AutoSize;
        using DataSizing::ZoneHVACSizing;
        using DataZoneEquipment::ZoneEquipConfig;
        using DXCoils::GetCoilCondenserInletNode;
        using DXCoils::GetCoilTypeNum;
        using DXCoils::GetDXCoilAvailSchPtr;
        using DXCoils::GetDXCoilCapFTCurveIndex;
        using DXCoils::GetDXCoilName;
        using DXCoils::RatedInletAirTempHeat;
        using DXCoils::RatedInletWetBulbTemp;
        using DXCoils::RatedOutdoorAirTemp;
        using DXCoils::RatedOutdoorAirTempHeat;
        using DXCoils::RatedOutdoorWetBulbTempHeat;
        using DXCoils::SetDXCoolingCoilData;
        using OutAirNodeManager::CheckOutAirNodeNumber;
        using SingleDuct::GetATMixer;
        using WaterManager::SetupTankDemandComponent;
        using WaterManager::SetupTankSupplyComponent;

        static std::string const RoutineName("GetVRFInput: "); // include trailing blank space

        int NumVRFCTU; // The number of VRF constant volume TUs (anticipating different types of TU's)
        int VRFTUNum;  // Loop index to the total number of VRF terminal units
        int VRFNum;    // Loop index to the total number of VRF terminal units
        int TUListNum; // Loop index to the total number of VRF terminal unit lists
        int NumAlphas; // Number of alpha arguments
        int NumNums;   // Number of real arguments
        //    INTEGER :: checkNum
        int IOStat;   // Status
        bool errFlag; // error flag for mining functions
        bool IsNotOK; // Flag to verify name
        Array1D_string cAlphaFieldNames;
        Array1D_string cNumericFieldNames;
        Array1D_bool lNumericFieldBlanks;
        Array1D_bool lAlphaFieldBlanks;
        Array1D_string cAlphaArgs;
        Array1D<Real64> rNumericArgs;
        std::string cCurrentModuleObject;
        int NumParams;
        int MaxAlphas;
        int MaxNumbers;
        std::string FanType;             // Type of supply air fan
        std::string FanName;             // Supply air fan name
        std::string OAMixerType;         // Type of OA mixer
        std::string DXCoolingCoilType;   // Type of VRF DX cooling coil
        std::string DXHeatingCoilType;   // Type of VRF DX heating coil
        std::string SuppHeatingCoilType; // Type of VRF supplemental heating coil
        std::string SuppHeatingCoilName; // Name of VRF supplemental heating coil
        Real64 FanVolFlowRate;           // Fan Max Flow Rate from Fan object (for comparisons to validity)
        int FanInletNodeNum;             // Used in TU configuration setup
        int FanOutletNodeNum;            // Used in TU configuration setup
        Array1D_int OANodeNums(4);       // Node numbers of OA mixer (OA, EA, RA, MA)
        int CCoilInletNodeNum;           // Used in TU configuration setup
        int CCoilOutletNodeNum;          // Used in TU configuration setup
        int HCoilInletNodeNum;           // Used in TU configuration setup
        int HCoilOutletNodeNum;          // Used in TU configuration setup
        int SuppHeatCoilAirInletNode;    // supplemental heating coil air inlet node
        int SuppHeatCoilAirOutletNode;   // supplemental heating coil air outlet node
        int ZoneTerminalUnitListNum;     // Used to find connection between VRFTU, TUList and VRF condenser
        int NumCond;                     // loop counter
        int NumList;                     // loop counter
        int CtrlZone;                    // index to loop counter
        int NodeNum;                     // index to loop counter
        // Followings for VRF FluidTCtrl Only
        int NumCompSpd;     // XP_loop counter
        int NumOfCompSpd;   // XP_ number of compressor speed inputs
        Real64 CurveVal;    // Used to verify modifier curves equal 1 at rated conditions
        Real64 MinCurveVal; // used for testing PLF curve output
        Real64 MinCurvePLR; // used for testing PLF curve output
        Real64 MaxCurveVal; // used for testing PLF curve output
        Real64 MaxCurvePLR; // used for testing PLF curve output
        Real64 CurveInput;  // index used for testing PLF curve output

        // Flow
        MaxAlphas = 0;
        MaxNumbers = 0;

        NumVRFCTU = inputProcessor->getNumObjectsFound("ZoneHVAC:TerminalUnit:VariableRefrigerantFlow");
        if (NumVRFCTU > 0) {
            inputProcessor->getObjectDefMaxArgs("ZoneHVAC:TerminalUnit:VariableRefrigerantFlow", NumParams, NumAlphas, NumNums);
            MaxAlphas = max(MaxAlphas, NumAlphas);
            MaxNumbers = max(MaxNumbers, NumNums);
        }

        NumVRFCond_SysCurve = inputProcessor->getNumObjectsFound("AirConditioner:VariableRefrigerantFlow");
        if (NumVRFCond_SysCurve > 0) {
            inputProcessor->getObjectDefMaxArgs("AirConditioner:VariableRefrigerantFlow", NumParams, NumAlphas, NumNums);
            MaxAlphas = max(MaxAlphas, NumAlphas);
            MaxNumbers = max(MaxNumbers, NumNums);
        }

        NumVRFCond_FluidTCtrl_HP = inputProcessor->getNumObjectsFound("AirConditioner:VariableRefrigerantFlow:FluidTemperatureControl");
        if (NumVRFCond_FluidTCtrl_HP > 0) {
            inputProcessor->getObjectDefMaxArgs("AirConditioner:VariableRefrigerantFlow:FluidTemperatureControl", NumParams, NumAlphas, NumNums);
            MaxAlphas = max(MaxAlphas, NumAlphas);
            MaxNumbers = max(MaxNumbers, NumNums);
        }

        NumVRFCond_FluidTCtrl_HR = inputProcessor->getNumObjectsFound("AirConditioner:VariableRefrigerantFlow:FluidTemperatureControl:HR");
        if (NumVRFCond_FluidTCtrl_HR > 0) {
            inputProcessor->getObjectDefMaxArgs("AirConditioner:VariableRefrigerantFlow:FluidTemperatureControl:HR", NumParams, NumAlphas, NumNums);
            MaxAlphas = max(MaxAlphas, NumAlphas);
            MaxNumbers = max(MaxNumbers, NumNums);
        }

        NumVRFCond = NumVRFCond_SysCurve + NumVRFCond_FluidTCtrl_HP + NumVRFCond_FluidTCtrl_HR;

        NumVRFTULists = inputProcessor->getNumObjectsFound("ZoneTerminalUnitList");
        if (NumVRFTULists > 0) {
            inputProcessor->getObjectDefMaxArgs("ZoneTerminalUnitList", NumParams, NumAlphas, NumNums);
            MaxAlphas = max(MaxAlphas, NumAlphas);
            MaxNumbers = max(MaxNumbers, NumNums);
        }

        cAlphaArgs.allocate(MaxAlphas);
        cAlphaFieldNames.allocate(MaxAlphas);
        lAlphaFieldBlanks.dimension(MaxAlphas, false);
        cNumericFieldNames.allocate(MaxNumbers);
        lNumericFieldBlanks.dimension(MaxNumbers, false);
        rNumericArgs.dimension(MaxNumbers, 0.0);

        NumVRFTU = NumVRFCTU;
        if (NumVRFTU > 0) {
            VRFTU.allocate(NumVRFTU);
            CheckEquipName.dimension(NumVRFTU, true);
            VRFTUNumericFields.allocate(NumVRFTU);
        }

        if (NumVRFCond > 0) {
            VRF.allocate(NumVRFCond);
            VrfUniqueNames.reserve(static_cast<unsigned>(NumVRFCond));
            MaxCoolingCapacity.allocate(NumVRFCond);
            MaxHeatingCapacity.allocate(NumVRFCond);
            CoolCombinationRatio.allocate(NumVRFCond);
            HeatCombinationRatio.allocate(NumVRFCond);
            MaxCoolingCapacity = MaxCap;
            MaxHeatingCapacity = MaxCap;
            CoolCombinationRatio = 1.0;
            HeatCombinationRatio = 1.0;
        }

        if (NumVRFTULists > 0) {
            TerminalUnitList.allocate(NumVRFTULists);
        }

        // read all terminal unit list objects
        cCurrentModuleObject = "ZoneTerminalUnitList";
        for (VRFNum = 1; VRFNum <= NumVRFTULists; ++VRFNum) {
            inputProcessor->getObjectItem(state,
                                          cCurrentModuleObject,
                                          VRFNum,
                                          cAlphaArgs,
                                          NumAlphas,
                                          rNumericArgs,
                                          NumNums,
                                          IOStat,
                                          lNumericFieldBlanks,
                                          lAlphaFieldBlanks,
                                          cAlphaFieldNames,
                                          cNumericFieldNames);
            UtilityRoutines::IsNameEmpty(cAlphaArgs(1), cCurrentModuleObject, ErrorsFound);

            TerminalUnitList(VRFNum).Name = cAlphaArgs(1);
            TerminalUnitList(VRFNum).NumTUInList = NumAlphas - 1;
            TerminalUnitList(VRFNum).ZoneTUPtr.allocate(TerminalUnitList(VRFNum).NumTUInList);
            TerminalUnitList(VRFNum).ZoneTUName.allocate(TerminalUnitList(VRFNum).NumTUInList);
            TerminalUnitList(VRFNum).IsSimulated.allocate(TerminalUnitList(VRFNum).NumTUInList);
            TerminalUnitList(VRFNum).TotalCoolLoad.allocate(TerminalUnitList(VRFNum).NumTUInList);
            TerminalUnitList(VRFNum).TotalHeatLoad.allocate(TerminalUnitList(VRFNum).NumTUInList);
            TerminalUnitList(VRFNum).CoolingCoilPresent.allocate(TerminalUnitList(VRFNum).NumTUInList);
            TerminalUnitList(VRFNum).HeatingCoilPresent.allocate(TerminalUnitList(VRFNum).NumTUInList);
            TerminalUnitList(VRFNum).TerminalUnitNotSizedYet.allocate(TerminalUnitList(VRFNum).NumTUInList);
            TerminalUnitList(VRFNum).HRHeatRequest.allocate(TerminalUnitList(VRFNum).NumTUInList);
            TerminalUnitList(VRFNum).HRCoolRequest.allocate(TerminalUnitList(VRFNum).NumTUInList);
            TerminalUnitList(VRFNum).CoolingCoilAvailable.allocate(TerminalUnitList(VRFNum).NumTUInList);
            TerminalUnitList(VRFNum).HeatingCoilAvailable.allocate(TerminalUnitList(VRFNum).NumTUInList);
            TerminalUnitList(VRFNum).CoolingCoilAvailSchPtr.allocate(TerminalUnitList(VRFNum).NumTUInList);
            TerminalUnitList(VRFNum).HeatingCoilAvailSchPtr.allocate(TerminalUnitList(VRFNum).NumTUInList);
            TerminalUnitList(VRFNum).ZoneTUPtr = 0;
            TerminalUnitList(VRFNum).IsSimulated = false;
            TerminalUnitList(VRFNum).TotalCoolLoad = 0.0;
            TerminalUnitList(VRFNum).TotalHeatLoad = 0.0;
            TerminalUnitList(VRFNum).CoolingCoilPresent = true;
            TerminalUnitList(VRFNum).HeatingCoilPresent = true;
            TerminalUnitList(VRFNum).TerminalUnitNotSizedYet = true;
            TerminalUnitList(VRFNum).HRHeatRequest = false;
            TerminalUnitList(VRFNum).HRCoolRequest = false;
            TerminalUnitList(VRFNum).CoolingCoilAvailable = false;
            TerminalUnitList(VRFNum).HeatingCoilAvailable = false;
            TerminalUnitList(VRFNum).CoolingCoilAvailSchPtr = -1;
            TerminalUnitList(VRFNum).HeatingCoilAvailSchPtr = -1;

            for (TUListNum = 1; TUListNum <= TerminalUnitList(VRFNum).NumTUInList; ++TUListNum) {
                TerminalUnitList(VRFNum).ZoneTUName(TUListNum) = cAlphaArgs(TUListNum + 1);
            }
        }

        // read all VRF condenser objects: Algorithm Type 1_system curve based model
        cCurrentModuleObject = "AirConditioner:VariableRefrigerantFlow";
        for (VRFNum = 1; VRFNum <= NumVRFCond_SysCurve; ++VRFNum) {
            inputProcessor->getObjectItem(state,
                                          cCurrentModuleObject,
                                          VRFNum,
                                          cAlphaArgs,
                                          NumAlphas,
                                          rNumericArgs,
                                          NumNums,
                                          IOStat,
                                          lNumericFieldBlanks,
                                          lAlphaFieldBlanks,
                                          cAlphaFieldNames,
                                          cNumericFieldNames);
            GlobalNames::VerifyUniqueInterObjectName(VrfUniqueNames, cAlphaArgs(1), cCurrentModuleObject, cAlphaFieldNames(1), ErrorsFound);

            VRF(VRFNum).Name = cAlphaArgs(1);
            VRF(VRFNum).VRFSystemTypeNum = VRF_HeatPump;
            VRF(VRFNum).VRFAlgorithmTypeNum = AlgorithmTypeSysCurve;
            if (lAlphaFieldBlanks(2)) {
                VRF(VRFNum).SchedPtr = ScheduleAlwaysOn;
            } else {
                VRF(VRFNum).SchedPtr = GetScheduleIndex(state, cAlphaArgs(2));
                if (VRF(VRFNum).SchedPtr == 0) {
                    ShowSevereError(cCurrentModuleObject + "=\"" + VRF(VRFNum).Name + "\" invalid data");
                    ShowContinueError("Invalid-not found " + cAlphaFieldNames(2) + "=\"" + cAlphaArgs(2) + "\".");
                    ErrorsFound = true;
                }
            }
            //     CALL TestCompSet(TRIM(cCurrentModuleObject),VRF(VRFTUNum)%Name,cAlphaArgs(3),cAlphaArgs(4),'Air Nodes')

            VRF(VRFNum).CoolingCapacity = rNumericArgs(1);
            VRF(VRFNum).CoolingCOP = rNumericArgs(2);
            VRF(VRFNum).MinOATCooling = rNumericArgs(3);
            VRF(VRFNum).MaxOATCooling = rNumericArgs(4);

            VRF(VRFNum).CoolCapFT = GetCurveIndex(state, cAlphaArgs(3));
            if (VRF(VRFNum).CoolCapFT > 0) {
                // Verify Curve Object, only legal type is biquadratic
                ErrorsFound |= CurveManager::CheckCurveDims(state, VRF(VRFNum).CoolCapFT, // Curve index
                                                            {2},                   // Valid dimensions
                                                            RoutineName,           // Routine name
                                                            cCurrentModuleObject,  // Object Type
                                                            VRF(VRFNum).Name,      // Object Name
                                                            cAlphaFieldNames(3));  // Field Name

                if (!ErrorsFound) {
                    checkCurveIsNormalizedToOne( state,
                                                RoutineName + cCurrentModuleObject,
                                                VRF(VRFNum).Name,
                                                VRF(VRFNum).CoolCapFT,
                                                cAlphaFieldNames(3),
                                                cAlphaArgs(3),
                                                RatedInletWetBulbTemp,
                                                RatedOutdoorAirTemp);
                }
            }

            VRF(VRFNum).CoolBoundaryCurvePtr = GetCurveIndex(state, cAlphaArgs(4));
            if (VRF(VRFNum).CoolBoundaryCurvePtr > 0) {
                // Verify Curve Object, only legal type is linear, quadratic, or cubic
                ErrorsFound |= CurveManager::CheckCurveDims(state, VRF(VRFNum).CoolBoundaryCurvePtr, // Curve index
                                                            {1},                              // Valid dimensions
                                                            RoutineName,                      // Routine name
                                                            cCurrentModuleObject,             // Object Type
                                                            VRF(VRFNum).Name,                 // Object Name
                                                            cAlphaFieldNames(4));             // Field Name
            }

            VRF(VRFNum).CoolCapFTHi = GetCurveIndex(state, cAlphaArgs(5));
            if (VRF(VRFNum).CoolCapFTHi > 0) {
                // Verify Curve Object, only legal type is biquadratic
                ErrorsFound |= CurveManager::CheckCurveDims(state, VRF(VRFNum).CoolCapFTHi, // Curve index
                                                            {2},                     // Valid dimensions
                                                            RoutineName,             // Routine name
                                                            cCurrentModuleObject,    // Object Type
                                                            VRF(VRFNum).Name,        // Object Name
                                                            cAlphaFieldNames(5));    // Field Name
            }

            VRF(VRFNum).CoolEIRFT = GetCurveIndex(state, cAlphaArgs(6));
            if (VRF(VRFNum).CoolEIRFT > 0) {
                // Verify Curve Object, only legal type is biquadratic
                ErrorsFound |= CurveManager::CheckCurveDims(state, VRF(VRFNum).CoolEIRFT, // Curve index
                                                            {2},                   // Valid dimensions
                                                            RoutineName,           // Routine name
                                                            cCurrentModuleObject,  // Object Type
                                                            VRF(VRFNum).Name,      // Object Name
                                                            cAlphaFieldNames(6));  // Field Name
            }

            VRF(VRFNum).EIRCoolBoundaryCurvePtr = GetCurveIndex(state, cAlphaArgs(7));
            if (VRF(VRFNum).EIRCoolBoundaryCurvePtr > 0) {
                // Verify Curve Object, only legal type is linear, quadratic, or cubic
                ErrorsFound |= CurveManager::CheckCurveDims(state, VRF(VRFNum).EIRCoolBoundaryCurvePtr, // Curve index
                                                            {1},                                 // Valid dimensions
                                                            RoutineName,                         // Routine name
                                                            cCurrentModuleObject,                // Object Type
                                                            VRF(VRFNum).Name,                    // Object Name
                                                            cAlphaFieldNames(7));                // Field Name
            }

            VRF(VRFNum).CoolEIRFTHi = GetCurveIndex(state, cAlphaArgs(8));
            if (VRF(VRFNum).CoolEIRFTHi > 0) {
                // Verify Curve Object, only legal type is biquadratic
                ErrorsFound |= CurveManager::CheckCurveDims(state, VRF(VRFNum).CoolEIRFTHi, // Curve index
                                                            {2},                     // Valid dimensions
                                                            RoutineName,             // Routine name
                                                            cCurrentModuleObject,    // Object Type
                                                            VRF(VRFNum).Name,        // Object Name
                                                            cAlphaFieldNames(8));    // Field Name
            }

            VRF(VRFNum).CoolEIRFPLR1 = GetCurveIndex(state, cAlphaArgs(9));
            if (VRF(VRFNum).CoolEIRFPLR1 > 0) {
                // Verify Curve Object, only legal type is linear, quadratic, or cubic
                ErrorsFound |= CurveManager::CheckCurveDims(state, VRF(VRFNum).CoolEIRFPLR1, // Curve index
                                                            {1},                      // Valid dimensions
                                                            RoutineName,              // Routine name
                                                            cCurrentModuleObject,     // Object Type
                                                            VRF(VRFNum).Name,         // Object Name
                                                            cAlphaFieldNames(9));     // Field Name
            }

            VRF(VRFNum).CoolEIRFPLR2 = GetCurveIndex(state, cAlphaArgs(10));
            if (VRF(VRFNum).CoolEIRFPLR2 > 0) {
                // Verify Curve Object, only legal type is linear, quadratic, or cubic
                ErrorsFound |= CurveManager::CheckCurveDims(state, VRF(VRFNum).CoolEIRFPLR2, // Curve index
                                                            {1},                      // Valid dimensions
                                                            RoutineName,              // Routine name
                                                            cCurrentModuleObject,     // Object Type
                                                            VRF(VRFNum).Name,         // Object Name
                                                            cAlphaFieldNames(10));    // Field Name
            }

            VRF(VRFNum).CoolCombRatioPTR = GetCurveIndex(state, cAlphaArgs(11));
            if (VRF(VRFNum).CoolCombRatioPTR > 0) {
                // Verify Curve Object, only legal type is linear, quadratic, or cubic
                ErrorsFound |= CurveManager::CheckCurveDims(state, VRF(VRFNum).CoolCombRatioPTR, // Curve index
                                                            {1},                          // Valid dimensions
                                                            RoutineName,                  // Routine name
                                                            cCurrentModuleObject,         // Object Type
                                                            VRF(VRFNum).Name,             // Object Name
                                                            cAlphaFieldNames(11));        // Field Name
            }

            VRF(VRFNum).CoolPLFFPLR = GetCurveIndex(state, cAlphaArgs(12));
            if (VRF(VRFNum).CoolPLFFPLR > 0) {
                // Verify Curve Object, only legal type is linear, quadratic, or cubic
                ErrorsFound |= CurveManager::CheckCurveDims(state, VRF(VRFNum).CoolPLFFPLR, // Curve index
                                                            {1},                     // Valid dimensions
                                                            RoutineName,             // Routine name
                                                            cCurrentModuleObject,    // Object Type
                                                            VRF(VRFNum).Name,        // Object Name
                                                            cAlphaFieldNames(12));   // Field Name
                if (!ErrorsFound) {
                    //     Test PLF curve minimum and maximum. Cap if less than 0.7 or greater than 1.0.
                    MinCurveVal = 999.0;
                    MaxCurveVal = -999.0;
                    CurveInput = 0.0;
                    while (CurveInput <= 1.0) {
                        CurveVal = CurveValue(state, VRF(VRFNum).CoolPLFFPLR, CurveInput);
                        if (CurveVal < MinCurveVal) {
                            MinCurveVal = CurveVal;
                            MinCurvePLR = CurveInput;
                        }
                        if (CurveVal > MaxCurveVal) {
                            MaxCurveVal = CurveVal;
                            MaxCurvePLR = CurveInput;
                        }
                        CurveInput += 0.01;
                    }
                    if (MinCurveVal < 0.7) {
                        ShowWarningError(RoutineName + cCurrentModuleObject + "=\"" + VRF(VRFNum).Name + "\", invalid");
                        ShowContinueError("..." + cAlphaFieldNames(12) + "=\"" + cAlphaArgs(12) + "\" has out of range values.");
                        ShowContinueError("...Curve minimum must be >= 0.7, curve min at PLR = " + TrimSigDigits(MinCurvePLR, 2) + " is " +
                                          TrimSigDigits(MinCurveVal, 3));
                        ShowContinueError("...Setting curve minimum to 0.7 and simulation continues.");
                        SetCurveOutputMinMaxValues(state, VRF(VRFNum).CoolPLFFPLR, ErrorsFound, 0.7, _);
                    }

                    if (MaxCurveVal > 1.0) {
                        ShowWarningError(RoutineName + cCurrentModuleObject + "=\"" + VRF(VRFNum).Name + "\", invalid");
                        ShowContinueError("..." + cAlphaFieldNames(12) + " = " + cAlphaArgs(12) + " has out of range value.");
                        ShowContinueError("...Curve maximum must be <= 1.0, curve max at PLR = " + TrimSigDigits(MaxCurvePLR, 2) + " is " +
                                          TrimSigDigits(MaxCurveVal, 3));
                        ShowContinueError("...Setting curve maximum to 1.0 and simulation continues.");
                        SetCurveOutputMinMaxValues(state, VRF(VRFNum).CoolPLFFPLR, ErrorsFound, _, 1.0);
                    }
                }
            }

            VRF(VRFNum).HeatingCapacity = rNumericArgs(5);
            VRF(VRFNum).HeatingCapacitySizeRatio = rNumericArgs(6);
            if (!lNumericFieldBlanks(6) && VRF(VRFNum).HeatingCapacity == AutoSize) {
                VRF(VRFNum).LockHeatingCapacity = true;
            }
            VRF(VRFNum).HeatingCOP = rNumericArgs(7);
            VRF(VRFNum).MinOATHeating = rNumericArgs(8);
            VRF(VRFNum).MaxOATHeating = rNumericArgs(9);
            if (VRF(VRFNum).MinOATHeating >= VRF(VRFNum).MaxOATHeating) {
                ShowSevereError(cCurrentModuleObject + ", \"" + VRF(VRFNum).Name + "\"");
                ShowContinueError("... " + cNumericFieldNames(8) + " (" + TrimSigDigits(VRF(VRFNum).MinOATHeating, 3) +
                                  ") must be less than maximum (" + TrimSigDigits(VRF(VRFNum).MaxOATHeating, 3) + ").");
                ErrorsFound = true;
            }

            VRF(VRFNum).HeatCapFT = GetCurveIndex(state, cAlphaArgs(13));
            if (VRF(VRFNum).HeatCapFT > 0) {
                // Verify Curve Object, only legal type is biquadratic
                ErrorsFound |= CurveManager::CheckCurveDims(state, VRF(VRFNum).HeatCapFT, // Curve index
                                                            {2},                   // Valid dimensions
                                                            RoutineName,           // Routine name
                                                            cCurrentModuleObject,  // Object Type
                                                            VRF(VRFNum).Name,      // Object Name
                                                            cAlphaFieldNames(13)); // Field Name

                if (!ErrorsFound) {
                    if (UtilityRoutines::SameString(cAlphaArgs(19), "WETBULBTEMPERATURE")) {
                        checkCurveIsNormalizedToOne( state,
                                                    RoutineName + cCurrentModuleObject,
                                                    VRF(VRFNum).Name,
                                                    VRF(VRFNum).HeatCapFT,
                                                    cAlphaFieldNames(13),
                                                    cAlphaArgs(13),
                                                    RatedInletAirTempHeat,
                                                    RatedOutdoorWetBulbTempHeat);
                    } else if (UtilityRoutines::SameString(cAlphaArgs(19), "DRYBULBTEMPERATURE")) {
                        checkCurveIsNormalizedToOne( state,
                                                    RoutineName + cCurrentModuleObject,
                                                    VRF(VRFNum).Name,
                                                    VRF(VRFNum).HeatCapFT,
                                                    cAlphaFieldNames(13),
                                                    cAlphaArgs(13),
                                                    RatedInletAirTempHeat,
                                                    RatedOutdoorAirTempHeat);
                    }
                }
            }

            VRF(VRFNum).HeatBoundaryCurvePtr = GetCurveIndex(state, cAlphaArgs(14));
            if (VRF(VRFNum).HeatBoundaryCurvePtr > 0) {
                // Verify Curve Object, only legal type is linear, quadratic, or cubic
                ErrorsFound |= CurveManager::CheckCurveDims(state, VRF(VRFNum).HeatBoundaryCurvePtr, // Curve index
                                                            {1},                              // Valid dimensions
                                                            RoutineName,                      // Routine name
                                                            cCurrentModuleObject,             // Object Type
                                                            VRF(VRFNum).Name,                 // Object Name
                                                            cAlphaFieldNames(14));            // Field Name
            }

            VRF(VRFNum).HeatCapFTHi = GetCurveIndex(state, cAlphaArgs(15));
            if (VRF(VRFNum).HeatCapFTHi > 0) {
                // Verify Curve Object, only legal type is biquadratic
                ErrorsFound |= CurveManager::CheckCurveDims(state, VRF(VRFNum).HeatCapFTHi, // Curve index
                                                            {2},                     // Valid dimensions
                                                            RoutineName,             // Routine name
                                                            cCurrentModuleObject,    // Object Type
                                                            VRF(VRFNum).Name,        // Object Name
                                                            cAlphaFieldNames(15));   // Field Name
            }

            VRF(VRFNum).HeatEIRFT = GetCurveIndex(state, cAlphaArgs(16));
            if (VRF(VRFNum).HeatEIRFT > 0) {
                // Verify Curve Object, only legal type is biquadratic
                ErrorsFound |= CurveManager::CheckCurveDims(state, VRF(VRFNum).HeatEIRFT, // Curve index
                                                            {2},                   // Valid dimensions
                                                            RoutineName,           // Routine name
                                                            cCurrentModuleObject,  // Object Type
                                                            VRF(VRFNum).Name,      // Object Name
                                                            cAlphaFieldNames(16)); // Field Name
            }

            VRF(VRFNum).EIRHeatBoundaryCurvePtr = GetCurveIndex(state, cAlphaArgs(17));
            if (VRF(VRFNum).EIRHeatBoundaryCurvePtr > 0) {
                // Verify Curve Object, only legal type is linear, quadratic, or cubic
                ErrorsFound |= CurveManager::CheckCurveDims(state, VRF(VRFNum).EIRHeatBoundaryCurvePtr, // Curve index
                                                            {1},                                 // Valid dimensions
                                                            RoutineName,                         // Routine name
                                                            cCurrentModuleObject,                // Object Type
                                                            VRF(VRFNum).Name,                    // Object Name
                                                            cAlphaFieldNames(17));               // Field Name
            }

            VRF(VRFNum).HeatEIRFTHi = GetCurveIndex(state, cAlphaArgs(18));
            if (VRF(VRFNum).HeatEIRFTHi > 0) {
                // Verify Curve Object, only legal type is biquadratic
                ErrorsFound |= CurveManager::CheckCurveDims(state, VRF(VRFNum).HeatEIRFTHi, // Curve index
                                                            {2},                     // Valid dimensions
                                                            RoutineName,             // Routine name
                                                            cCurrentModuleObject,    // Object Type
                                                            VRF(VRFNum).Name,        // Object Name
                                                            cAlphaFieldNames(18));   // Field Name
            }

            if (UtilityRoutines::SameString(cAlphaArgs(19), "WETBULBTEMPERATURE")) {
                VRF(VRFNum).HeatingPerformanceOATType = DataHVACGlobals::WetBulbIndicator;
            } else if (UtilityRoutines::SameString(cAlphaArgs(19), "DRYBULBTEMPERATURE")) {
                VRF(VRFNum).HeatingPerformanceOATType = DataHVACGlobals::DryBulbIndicator;
            } else {
                ShowSevereError(cCurrentModuleObject + ", \"" + VRF(VRFNum).Name + "\" illegal " + cAlphaFieldNames(19) +
                                " input for this object = " + cAlphaArgs(19));
                ShowContinueError("... input must be WETBULBTEMPERATURE or DRYBULBTEMPERATURE.");
                ErrorsFound = true;
            }

            VRF(VRFNum).HeatEIRFPLR1 = GetCurveIndex(state, cAlphaArgs(20));
            if (VRF(VRFNum).HeatEIRFPLR1 > 0) {
                // Verify Curve Object, only legal type is linear, quadratic, or cubic
                ErrorsFound |= CurveManager::CheckCurveDims(state, VRF(VRFNum).HeatEIRFPLR1, // Curve index
                                                            {1},                      // Valid dimensions
                                                            RoutineName,              // Routine name
                                                            cCurrentModuleObject,     // Object Type
                                                            VRF(VRFNum).Name,         // Object Name
                                                            cAlphaFieldNames(20));    // Field Name
            }

            VRF(VRFNum).HeatEIRFPLR2 = GetCurveIndex(state, cAlphaArgs(21));
            if (VRF(VRFNum).HeatEIRFPLR2 > 0) {
                // Verify Curve Object, only legal type is linear, quadratic, or cubic
                ErrorsFound |= CurveManager::CheckCurveDims(state, VRF(VRFNum).HeatEIRFPLR2, // Curve index
                                                            {1},                      // Valid dimensions
                                                            RoutineName,              // Routine name
                                                            cCurrentModuleObject,     // Object Type
                                                            VRF(VRFNum).Name,         // Object Name
                                                            cAlphaFieldNames(21));    // Field Name
            }

            VRF(VRFNum).HeatCombRatioPTR = GetCurveIndex(state, cAlphaArgs(22));
            if (VRF(VRFNum).HeatCombRatioPTR > 0) {
                // Verify Curve Object, only legal type is linear, quadratic, or cubic
                ErrorsFound |= CurveManager::CheckCurveDims(state, VRF(VRFNum).HeatCombRatioPTR, // Curve index
                                                            {1},                          // Valid dimensions
                                                            RoutineName,                  // Routine name
                                                            cCurrentModuleObject,         // Object Type
                                                            VRF(VRFNum).Name,             // Object Name
                                                            cAlphaFieldNames(22));        // Field Name
            }
            VRF(VRFNum).HeatPLFFPLR = GetCurveIndex(state, cAlphaArgs(23));
            if (VRF(VRFNum).HeatPLFFPLR > 0) {
                // Verify Curve Object, only legal type is linear, quadratic, or cubic
                ErrorsFound |= CurveManager::CheckCurveDims(state, VRF(VRFNum).HeatPLFFPLR, // Curve index
                                                            {1},                     // Valid dimensions
                                                            RoutineName,             // Routine name
                                                            cCurrentModuleObject,    // Object Type
                                                            VRF(VRFNum).Name,        // Object Name
                                                            cAlphaFieldNames(23));   // Field Name

                if (!ErrorsFound) {
                    MinCurveVal = 999.0;
                    MaxCurveVal = -999.0;
                    CurveInput = 0.0;
                    while (CurveInput <= 1.0) {
                        CurveVal = CurveValue(state, VRF(VRFNum).HeatPLFFPLR, CurveInput);
                        if (CurveVal < MinCurveVal) {
                            MinCurveVal = CurveVal;
                            MinCurvePLR = CurveInput;
                        }
                        if (CurveVal > MaxCurveVal) {
                            MaxCurveVal = CurveVal;
                            MaxCurvePLR = CurveInput;
                        }
                        CurveInput += 0.01;
                    }
                    if (MinCurveVal < 0.7) {
                        ShowWarningError(RoutineName + cCurrentModuleObject + "=\"" + VRF(VRFNum).Name + "\", invalid");
                        ShowContinueError("..." + cAlphaFieldNames(23) + "=\"" + cAlphaArgs(23) + "\" has out of range values.");
                        ShowContinueError("...Curve minimum must be >= 0.7, curve min at PLR = " + TrimSigDigits(MinCurvePLR, 2) + " is " +
                                          TrimSigDigits(MinCurveVal, 3));
                        ShowContinueError("...Setting curve minimum to 0.7 and simulation continues.");
                        SetCurveOutputMinMaxValues(state, VRF(VRFNum).HeatPLFFPLR, ErrorsFound, 0.7, _);
                    }

                    if (MaxCurveVal > 1.0) {
                        ShowWarningError(RoutineName + cCurrentModuleObject + "=\"" + VRF(VRFNum).Name + "\", invalid");
                        ShowContinueError("..." + cAlphaFieldNames(23) + " = " + cAlphaArgs(23) + " has out of range value.");
                        ShowContinueError("...Curve maximum must be <= 1.0, curve max at PLR = " + TrimSigDigits(MaxCurvePLR, 2) + " is " +
                                          TrimSigDigits(MaxCurveVal, 3));
                        ShowContinueError("...Setting curve maximum to 1.0 and simulation continues.");
                        SetCurveOutputMinMaxValues(state, VRF(VRFNum).HeatPLFFPLR, ErrorsFound, _, 1.0);
                    }
                }
            }

            VRF(VRFNum).MinPLR = rNumericArgs(10);
            Real64 minEIRfLowPLRXInput = 0.0;
            Real64 maxEIRfLowPLRXInput = 0.0;

            if (VRF(VRFNum).CoolEIRFPLR1 > 0) {
                CurveManager::GetCurveMinMaxValues(state,VRF(VRFNum).CoolEIRFPLR1, minEIRfLowPLRXInput, maxEIRfLowPLRXInput);
                if (minEIRfLowPLRXInput > VRF(VRFNum).MinPLR) {
                    ShowWarningError(RoutineName + cCurrentModuleObject + "=\"" + VRF(VRFNum).Name + "\", invalid");
                    ShowContinueError("..." + cAlphaFieldNames(9) + " = " + cAlphaArgs(9) + " has out of range value.");
                    ShowContinueError("...Curve minimum value of X = " + TrimSigDigits(minEIRfLowPLRXInput, 3) +
                                      " must be <= Minimum Heat Pump Part-Load Ratio = " + TrimSigDigits(VRF(VRFNum).MinPLR, 3) + ".");
                    ErrorsFound = true;
                }
                if (maxEIRfLowPLRXInput < 1.0) {
                    ShowWarningError(RoutineName + cCurrentModuleObject + "=\"" + VRF(VRFNum).Name + "\", suspicious");
                    ShowContinueError("..." + cAlphaFieldNames(9) + " = " + cAlphaArgs(9) + " has unexpected value.");
                    ShowContinueError("...Curve maximum value of X = " + TrimSigDigits(maxEIRfLowPLRXInput, 3) +
                                      " should be 1 and will result in lower energy use than expected.");
                }
                minEIRfLowPLRXInput = 0.0;
                maxEIRfLowPLRXInput = 0.0;
            }
            if (VRF(VRFNum).HeatEIRFPLR1 > 0) {
                CurveManager::GetCurveMinMaxValues(state,VRF(VRFNum).HeatEIRFPLR1, minEIRfLowPLRXInput, maxEIRfLowPLRXInput);
                if (minEIRfLowPLRXInput > VRF(VRFNum).MinPLR) {
                    ShowWarningError(RoutineName + cCurrentModuleObject + "=\"" + VRF(VRFNum).Name + "\", invalid");
                    ShowContinueError("..." + cAlphaFieldNames(20) + " = " + cAlphaArgs(20) + " has out of range value.");
                    ShowContinueError("...Curve minimum value of X = " + TrimSigDigits(minEIRfLowPLRXInput, 3) +
                                      " must be <= Minimum Heat Pump Part-Load Ratio = " + TrimSigDigits(VRF(VRFNum).MinPLR, 3) + ".");
                    ErrorsFound = true;
                }
                if (maxEIRfLowPLRXInput < 1.0) {
                    ShowWarningError(RoutineName + cCurrentModuleObject + "=\"" + VRF(VRFNum).Name + "\", suspicious");
                    ShowContinueError("..." + cAlphaFieldNames(20) + " = " + cAlphaArgs(20) + " has unexpected value.");
                    ShowContinueError("...Curve maximum value of X = " + TrimSigDigits(maxEIRfLowPLRXInput, 3) +
                                      " should be 1 and will result in lower energy use than expected.");
                }
            }

            VRF(VRFNum).MasterZonePtr = UtilityRoutines::FindItemInList(cAlphaArgs(24), Zone);

            if (UtilityRoutines::SameString(cAlphaArgs(25), "LoadPriority")) {
                VRF(VRFNum).ThermostatPriority = LoadPriority;
            } else if (UtilityRoutines::SameString(cAlphaArgs(25), "ZonePriority")) {
                VRF(VRFNum).ThermostatPriority = ZonePriority;
            } else if (UtilityRoutines::SameString(cAlphaArgs(25), "ThermostatOffsetPriority")) {
                VRF(VRFNum).ThermostatPriority = ThermostatOffsetPriority;
            } else if (UtilityRoutines::SameString(cAlphaArgs(25), "Scheduled")) {
                VRF(VRFNum).ThermostatPriority = ScheduledPriority;
            } else if (UtilityRoutines::SameString(cAlphaArgs(25), "MasterThermostatPriority")) {
                VRF(VRFNum).ThermostatPriority = MasterThermostatPriority;
                if (VRF(VRFNum).MasterZonePtr == 0) {
                    ShowSevereError(cCurrentModuleObject + " = \"" + VRF(VRFNum).Name + "\"");
                    ShowContinueError(cAlphaFieldNames(24) + " must be entered when " + cAlphaFieldNames(25) + " = " + cAlphaArgs(25));
                    ErrorsFound = true;
                }
                //      ELSE IF (UtilityRoutines::SameString(cAlphaArgs(25),'FirstOnPriority') )THEN ! strategy not used
                //        VRF(VRFNum)%ThermostatPriority = FirstOnPriority
            } else {
                ShowSevereError(cCurrentModuleObject + " = " + VRF(VRFNum).Name);
                ShowContinueError("Illegal " + cAlphaFieldNames(25) + " = " + cAlphaArgs(25));
                ErrorsFound = true;
            }

            if (VRF(VRFNum).ThermostatPriority == ScheduledPriority) {
                VRF(VRFNum).SchedPriorityPtr = GetScheduleIndex(state, cAlphaArgs(26));
                if (VRF(VRFNum).SchedPriorityPtr == 0) {
                    ShowSevereError(cCurrentModuleObject + " = " + VRF(VRFNum).Name);
                    ShowContinueError("..." + cAlphaFieldNames(26) + " = " + cAlphaArgs(26) + " not found.");
                    ShowContinueError("A schedule name is required when " + cAlphaFieldNames(25) + " = " + cAlphaArgs(25));
                    ErrorsFound = true;
                }
            }

            VRF(VRFNum).ZoneTUListPtr = UtilityRoutines::FindItemInList(cAlphaArgs(27), TerminalUnitList);
            if (VRF(VRFNum).ZoneTUListPtr == 0) {
                ShowSevereError(cCurrentModuleObject + " = \"" + VRF(VRFNum).Name + "\"");
                ShowContinueError(cAlphaFieldNames(27) + " = " + cAlphaArgs(27) + " not found.");
                ErrorsFound = true;
            }

            VRF(VRFNum).HeatRecoveryUsed = false;
            if (!lAlphaFieldBlanks(28)) {
                if (UtilityRoutines::SameString(cAlphaArgs(28), "No")) {
                    VRF(VRFNum).HeatRecoveryUsed = false;
                } else if (UtilityRoutines::SameString(cAlphaArgs(28), "Yes")) {
                    VRF(VRFNum).HeatRecoveryUsed = true;
                } else {
                    ShowSevereError(cCurrentModuleObject + " = " + VRF(VRFNum).Name);
                    ShowContinueError("Illegal " + cAlphaFieldNames(28) + " = " + cAlphaArgs(28));
                    ErrorsFound = true;
                }
            }

            VRF(VRFNum).EquivPipeLngthCool = rNumericArgs(11);
            VRF(VRFNum).VertPipeLngth = rNumericArgs(12);
            VRF(VRFNum).PCFLengthCoolPtr = GetCurveIndex(state, cAlphaArgs(29));
            if (VRF(VRFNum).PCFLengthCoolPtr > 0) {
                // Verify Curve Object, only legal type is linear, quadratic, cubic, or biquadratic
                ErrorsFound |= CurveManager::CheckCurveDims(state, VRF(VRFNum).PCFLengthCoolPtr, // Curve index
                                                            {1, 2},                       // Valid dimensions
                                                            RoutineName,                  // Routine name
                                                            cCurrentModuleObject,         // Object Type
                                                            VRF(VRFNum).Name,             // Object Name
                                                            cAlphaFieldNames(29));        // Field Name
            }
            VRF(VRFNum).PCFHeightCool = rNumericArgs(13);

            VRF(VRFNum).EquivPipeLngthHeat = rNumericArgs(14);
            VRF(VRFNum).PCFLengthHeatPtr = GetCurveIndex(state, cAlphaArgs(30));
            if (VRF(VRFNum).PCFLengthHeatPtr > 0) {
                // Verify Curve Object, only legal type is linear, quadratic, cubic, or biquadratic
                ErrorsFound |= CurveManager::CheckCurveDims(state, VRF(VRFNum).PCFLengthHeatPtr, // Curve index
                                                            {1, 2},                       // Valid dimensions
                                                            RoutineName,                  // Routine name
                                                            cCurrentModuleObject,         // Object Type
                                                            VRF(VRFNum).Name,             // Object Name
                                                            cAlphaFieldNames(30));        // Field Name
            }

            VRF(VRFNum).PCFHeightHeat = rNumericArgs(15);

            VRF(VRFNum).CCHeaterPower = rNumericArgs(16);
            VRF(VRFNum).NumCompressors = rNumericArgs(17);
            VRF(VRFNum).CompressorSizeRatio = rNumericArgs(18);
            VRF(VRFNum).MaxOATCCHeater = rNumericArgs(19);

            if (!lAlphaFieldBlanks(31)) {
                if (UtilityRoutines::SameString(cAlphaArgs(31), "ReverseCycle")) VRF(VRFNum).DefrostStrategy = ReverseCycle;
                if (UtilityRoutines::SameString(cAlphaArgs(31), "Resistive")) VRF(VRFNum).DefrostStrategy = Resistive;
                if (VRF(VRFNum).DefrostStrategy == 0) {
                    ShowSevereError(cCurrentModuleObject + ", \"" + VRF(VRFNum).Name + "\" " + cAlphaFieldNames(31) +
                                    " not found: " + cAlphaArgs(31));
                    ErrorsFound = true;
                }
            } else {
                VRF(VRFNum).DefrostStrategy = ReverseCycle;
            }

            if (!lAlphaFieldBlanks(32)) {
                if (UtilityRoutines::SameString(cAlphaArgs(32), "Timed")) VRF(VRFNum).DefrostControl = Timed;
                if (UtilityRoutines::SameString(cAlphaArgs(32), "OnDemand")) VRF(VRFNum).DefrostControl = OnDemand;
                if (VRF(VRFNum).DefrostControl == 0) {
                    ShowSevereError(cCurrentModuleObject + ", \"" + VRF(VRFNum).Name + "\" " + cAlphaFieldNames(32) +
                                    " not found: " + cAlphaArgs(32));
                    ErrorsFound = true;
                }
            } else {
                VRF(VRFNum).DefrostControl = Timed;
            }

            if (!lAlphaFieldBlanks(33)) {
                VRF(VRFNum).DefrostEIRPtr = GetCurveIndex(state, cAlphaArgs(33));
                if (VRF(VRFNum).DefrostEIRPtr > 0) {
                    // Verify Curve Object, expected type is BiQuadratic
                    ErrorsFound |= CurveManager::CheckCurveDims(state, VRF(VRFNum).DefrostEIRPtr, // Curve index
                                                                {2},                       // Valid dimensions
                                                                RoutineName,               // Routine name
                                                                cCurrentModuleObject,      // Object Type
                                                                VRF(VRFNum).Name,          // Object Name
                                                                cAlphaFieldNames(33));     // Field Name
                } else {
                    if (VRF(VRFNum).DefrostStrategy == ReverseCycle) {
                        ShowSevereError(cCurrentModuleObject + ", \"" + VRF(VRFNum).Name + "\" " + cAlphaFieldNames(33) +
                                        " not found:" + cAlphaArgs(33));
                        ErrorsFound = true;
                    }
                }
            } else {
                if (VRF(VRFNum).DefrostStrategy == ReverseCycle) {
                    ShowSevereError(cCurrentModuleObject + ", \"" + VRF(VRFNum).Name + "\" " + cAlphaFieldNames(33) + " not found:" + cAlphaArgs(33));
                    ErrorsFound = true;
                }
            }

            VRF(VRFNum).DefrostFraction = rNumericArgs(20);
            VRF(VRFNum).DefrostCapacity = rNumericArgs(21);
            if (VRF(VRFNum).DefrostCapacity == 0.0 && VRF(VRFNum).DefrostStrategy == Resistive) {
                ShowWarningError(cCurrentModuleObject + ", \"" + VRF(VRFNum).Name + "\" " + cNumericFieldNames(21) +
                                 " = 0.0 for defrost strategy = RESISTIVE.");
            }

            VRF(VRFNum).MaxOATDefrost = rNumericArgs(22);

            if (!lAlphaFieldBlanks(35)) {
                if (UtilityRoutines::SameString(cAlphaArgs(34), "AirCooled")) VRF(VRFNum).CondenserType = DataHVACGlobals::AirCooled;
                if (UtilityRoutines::SameString(cAlphaArgs(34), "EvaporativelyCooled")) VRF(VRFNum).CondenserType = DataHVACGlobals::EvapCooled;
                if (UtilityRoutines::SameString(cAlphaArgs(34), "WaterCooled")) {
                    VRF(VRFNum).CondenserType = DataHVACGlobals::WaterCooled;
                    VRF(VRFNum).VRFPlantTypeOfNum = TypeOf_HeatPumpVRF;
                    if (VRF(VRFNum).HeatingPerformanceOATType == DataHVACGlobals::WetBulbIndicator) {
                        ShowSevereError(cCurrentModuleObject + " = " + VRF(VRFNum).Name);
                        ShowContinueError(cAlphaFieldNames(34) + " = " + cAlphaArgs(34));
                        ShowContinueError("Illegal " + cAlphaFieldNames(19) + " input for this object = " + cAlphaArgs(19));
                        ShowContinueError("... input must be DRYBULBTEMPERATURE when Condenser Type is WaterCooled.");
                        ShowContinueError("... " + cAlphaFieldNames(19) + " will be reset to DRYBULBTEMPERATURE and simulation continues.");
                    }
                }
                if (VRF(VRFNum).CondenserType == 0) {
                    ShowSevereError(cCurrentModuleObject + " = " + VRF(VRFNum).Name);
                    ShowContinueError("Illegal " + cAlphaFieldNames(34) + " = " + cAlphaArgs(34));
                    ErrorsFound = true;
                }
            } else {
                VRF(VRFNum).CondenserType = DataHVACGlobals::AirCooled;
            }

            // outdoor condenser node
            if (lAlphaFieldBlanks(35)) {
                VRF(VRFNum).CondenserNodeNum = 0;
            } else {
                {
                    auto const SELECT_CASE_var(VRF(VRFNum).CondenserType);
                    if ((SELECT_CASE_var == DataHVACGlobals::AirCooled) || (SELECT_CASE_var == DataHVACGlobals::EvapCooled)) {
                        VRF(VRFNum).CondenserNodeNum = GetOnlySingleNode(state, cAlphaArgs(35),
                                                                         ErrorsFound,
                                                                         cCurrentModuleObject,
                                                                         VRF(VRFNum).Name,
                                                                         DataLoopNode::NodeType_Air,
                                                                         DataLoopNode::NodeConnectionType_OutsideAirReference,
                                                                         1,
                                                                         ObjectIsNotParent);
                        if (!CheckOutAirNodeNumber(state, VRF(VRFNum).CondenserNodeNum)) {
                            ShowSevereError(cCurrentModuleObject + ", \"" + VRF(VRFNum).Name + "\" " + cAlphaFieldNames(35) +
                                            " not a valid Outdoor Air Node = " + cAlphaArgs(35));
                            ShowContinueError("...node name does not appear in an OutdoorAir:NodeList or as an OutdoorAir:Node.");
                            ErrorsFound = true;
                        }
                    } else if (SELECT_CASE_var == DataHVACGlobals::WaterCooled) {
                        VRF(VRFNum).CondenserNodeNum = GetOnlySingleNode(state, cAlphaArgs(35),
                                                                         ErrorsFound,
                                                                         cCurrentModuleObject,
                                                                         VRF(VRFNum).Name,
                                                                         DataLoopNode::NodeType_Water,
                                                                         DataLoopNode::NodeConnectionType_Inlet,
                                                                         2,
                                                                         ObjectIsNotParent);
                    } else {
                    }
                }
            }

            if (!lAlphaFieldBlanks(36) && VRF(VRFNum).CondenserType == DataHVACGlobals::WaterCooled) {
                VRF(VRFNum).CondenserOutletNodeNum = GetOnlySingleNode(state, cAlphaArgs(36),
                                                                       ErrorsFound,
                                                                       cCurrentModuleObject,
                                                                       VRF(VRFNum).Name,
                                                                       DataLoopNode::NodeType_Water,
                                                                       DataLoopNode::NodeConnectionType_Outlet,
                                                                       2,
                                                                       ObjectIsNotParent);
                TestCompSet(cCurrentModuleObject, VRF(VRFNum).Name, cAlphaArgs(35), cAlphaArgs(36), "Condenser Water Nodes");
            } else if (lAlphaFieldBlanks(36) && VRF(VRFNum).CondenserType == DataHVACGlobals::WaterCooled) {
                ShowSevereError(cCurrentModuleObject + ", \"" + VRF(VRFNum).Name + "\" " + cAlphaFieldNames(36) + " is blank.");
                ShowContinueError("...node name must be entered when Condenser Type = WaterCooled.");
                ErrorsFound = true;
            }

            if (lNumericFieldBlanks(23)) {
                if (VRF(VRFNum).CondenserType == DataHVACGlobals::WaterCooled) {
                    ShowSevereError(cCurrentModuleObject + ", \"" + VRF(VRFNum).Name + "\" " + cNumericFieldNames(23) + " is blank.");
                    ShowContinueError("...input is required when " + cAlphaFieldNames(34) + " = " + cAlphaArgs(34));
                    ErrorsFound = true;
                }
            } else {
                VRF(VRFNum).WaterCondVolFlowRate = rNumericArgs(23);
            }
            VRF(VRFNum).EvapCondEffectiveness = rNumericArgs(24);
            VRF(VRFNum).EvapCondAirVolFlowRate = rNumericArgs(25);
            VRF(VRFNum).EvapCondPumpPower = rNumericArgs(26);

            // Get Water System tank connections
            // A37, \field Supply Water Storage Tank Name
            VRF(VRFNum).EvapWaterSupplyName = cAlphaArgs(37);
            if (lAlphaFieldBlanks(37)) {
                VRF(VRFNum).EvapWaterSupplyMode = WaterSupplyFromMains;
            } else {
                VRF(VRFNum).EvapWaterSupplyMode = WaterSupplyFromTank;
                SetupTankDemandComponent(state, VRF(VRFNum).Name,
                                         cCurrentModuleObject,
                                         VRF(VRFNum).EvapWaterSupplyName,
                                         ErrorsFound,
                                         VRF(VRFNum).EvapWaterSupTankID,
                                         VRF(VRFNum).EvapWaterTankDemandARRID);
            }

            //   Basin heater power as a function of temperature must be greater than or equal to 0
            VRF(VRFNum).BasinHeaterPowerFTempDiff = rNumericArgs(27);
            if (rNumericArgs(27) < 0.0) {
                ShowSevereError(cCurrentModuleObject + " = \"" + VRF(VRFNum).Name + "\", " + cNumericFieldNames(27) + " must be >= 0");
                ErrorsFound = true;
            }

            VRF(VRFNum).BasinHeaterSetPointTemp = rNumericArgs(28);
            if (VRF(VRFNum).BasinHeaterPowerFTempDiff > 0.0) {
                if (NumNums < 27) {
                    VRF(VRFNum).BasinHeaterSetPointTemp = 2.0;
                }
                if (VRF(VRFNum).BasinHeaterSetPointTemp < 2.0) {
                    ShowWarningError(cCurrentModuleObject + " = \"" + VRF(VRFNum).Name + "\", " + cNumericFieldNames(28) +
                                     " is less than 2 deg C. Freezing could occur.");
                }
            }

            if (!lAlphaFieldBlanks(38)) {
                VRF(VRFNum).BasinHeaterSchedulePtr = GetScheduleIndex(state, cAlphaArgs(38));
                if (VRF(VRFNum).BasinHeaterSchedulePtr == 0) {
                    ShowWarningError(cCurrentModuleObject + " = \"" + VRF(VRFNum).Name + "\", " + cAlphaFieldNames(38) + " = \"" + cAlphaArgs(38) +
                                     "\" was not found.");
                    ShowContinueError("Basin heater will be available to operate throughout the simulation.");
                }
            }

            VRF(VRFNum).FuelType = "Electricity";
            if (!lAlphaFieldBlanks(39)) {
                // A39; \field Fuel type, Validate fuel type input
                bool FuelTypeError(false);
                UtilityRoutines::ValidateFuelTypeWithAssignResourceTypeNum(cAlphaArgs(39), VRF(VRFNum).FuelType, VRF(VRFNum).FuelTypeNum, FuelTypeError);
                if (FuelTypeError) {
                    ShowSevereError(cCurrentModuleObject + ", \"" + VRF(VRFNum).Name + "\", " + cAlphaFieldNames(39) +
                                    " not found = " + cAlphaArgs(39));
                    ShowContinueError(
                        "Valid choices are Electricity, NaturalGas, Propane, Diesel, Gasoline, FuelOilNo1, FuelOilNo2, OtherFuel1 or OtherFuel2");
                    ErrorsFound = true;
                    FuelTypeError = false;
                }
            }

            if (VRF(VRFNum).HeatRecoveryUsed) {
                if (lNumericFieldBlanks(29)) {
                    VRF(VRFNum).MinOATHeatRecovery = max(VRF(VRFNum).MinOATCooling, VRF(VRFNum).MinOATHeating);
                } else {
                    VRF(VRFNum).MinOATHeatRecovery = rNumericArgs(29);
                    if (VRF(VRFNum).MinOATHeatRecovery < VRF(VRFNum).MinOATCooling || VRF(VRFNum).MinOATHeatRecovery < VRF(VRFNum).MinOATHeating) {
                        ShowWarningError(cCurrentModuleObject + " = \"" + VRF(VRFNum).Name + "\", " + cNumericFieldNames(29) +
                                         " is less than the minimum temperature in heat pump mode.");
                        ShowContinueError("..." + cNumericFieldNames(29) + " = " + TrimSigDigits(VRF(VRFNum).MinOATHeatRecovery, 2) + " C");
                        ShowContinueError("...Minimum Outdoor Temperature in Cooling Mode = " + TrimSigDigits(VRF(VRFNum).MinOATCooling, 2) + " C");
                        ShowContinueError("...Minimum Outdoor Temperature in Heating Mode = " + TrimSigDigits(VRF(VRFNum).MinOATHeating, 2) + " C");
                        ShowContinueError("...Minimum Outdoor Temperature in Heat Recovery Mode reset to greater of cooling or heating minimum "
                                          "temperature and simulation continues.");
                        VRF(VRFNum).MinOATHeatRecovery = max(VRF(VRFNum).MinOATCooling, VRF(VRFNum).MinOATHeating);
                        ShowContinueError("... adjusted " + cNumericFieldNames(29) + " = " + TrimSigDigits(VRF(VRFNum).MinOATHeatRecovery, 2) + " C");
                    }
                }
                if (lNumericFieldBlanks(30)) {
                    VRF(VRFNum).MaxOATHeatRecovery = min(VRF(VRFNum).MaxOATCooling, VRF(VRFNum).MaxOATHeating);
                } else {
                    VRF(VRFNum).MaxOATHeatRecovery = rNumericArgs(30);
                    if (VRF(VRFNum).MaxOATHeatRecovery > VRF(VRFNum).MaxOATCooling || VRF(VRFNum).MaxOATHeatRecovery > VRF(VRFNum).MaxOATHeating) {
                        ShowWarningError(cCurrentModuleObject + " = \"" + VRF(VRFNum).Name + "\", " + cNumericFieldNames(30) +
                                         " is greater than the maximum temperature in heat pump mode.");
                        ShowContinueError("..." + cNumericFieldNames(30) + " = " + TrimSigDigits(VRF(VRFNum).MaxOATHeatRecovery, 2) + " C");
                        ShowContinueError("...Maximum Outdoor Temperature in Cooling Mode = " + TrimSigDigits(VRF(VRFNum).MaxOATCooling, 2) + " C");
                        ShowContinueError("...Maximum Outdoor Temperature in Heating Mode = " + TrimSigDigits(VRF(VRFNum).MaxOATHeating, 2) + " C");
                        ShowContinueError("...Maximum Outdoor Temperature in Heat Recovery Mode reset to lesser of cooling or heating minimum "
                                          "temperature and simulation continues.");
                        VRF(VRFNum).MaxOATHeatRecovery = min(VRF(VRFNum).MaxOATCooling, VRF(VRFNum).MaxOATHeating);
                        ShowContinueError("... adjusted " + cNumericFieldNames(30) + " = " + TrimSigDigits(VRF(VRFNum).MaxOATHeatRecovery, 2) + " C");
                    }
                }

                VRF(VRFNum).HRCAPFTCool = GetCurveIndex(state, cAlphaArgs(40));
                if (VRF(VRFNum).HRCAPFTCool > 0) {
                    // Verify Curve Object, only legal type is bi-quadratic or linear, quadratic, or cubic
                    ErrorsFound |= CurveManager::CheckCurveDims(state, VRF(VRFNum).HRCAPFTCool, // Curve index
                                                                {1, 2},                  // Valid dimensions
                                                                RoutineName,             // Routine name
                                                                cCurrentModuleObject,    // Object Type
                                                                VRF(VRFNum).Name,        // Object Name
                                                                cAlphaFieldNames(40));   // Field Name
                }
                if (!lNumericFieldBlanks(31)) {
                    VRF(VRFNum).HRInitialCoolCapFrac = rNumericArgs(31);
                }
                VRF(VRFNum).HRCoolCapTC = rNumericArgs(32);
                VRF(VRFNum).HREIRFTCool = GetCurveIndex(state, cAlphaArgs(41));
                if (VRF(VRFNum).HREIRFTCool > 0) {
                    // Verify Curve Object, only legal type is bi-quadratic or linear, quadratic, or cubic
                    ErrorsFound |= CurveManager::CheckCurveDims(state, VRF(VRFNum).HREIRFTCool, // Curve index
                                                                {1, 2},                  // Valid dimensions
                                                                RoutineName,             // Routine name
                                                                cCurrentModuleObject,    // Object Type
                                                                VRF(VRFNum).Name,        // Object Name
                                                                cAlphaFieldNames(41));   // Field Name
                }
                VRF(VRFNum).HRInitialCoolEIRFrac = rNumericArgs(33);
                VRF(VRFNum).HRCoolEIRTC = rNumericArgs(34);

                //  INTEGER      :: HRCAPFTHeat                =0   ! Index to heat capacity as a function of temperature curve for heat recovery
                //  REAL(r64)    :: HRInitialHeatCapFrac       =0.0d0 ! Fractional heating degradation at the start of heat recovery from heating mode
                //  REAL(r64)    :: HRHeatCapTC                =0.0d0 ! Time constant used to recover from initial degradation in heating heat
                //  recovery
                VRF(VRFNum).HRCAPFTHeat = GetCurveIndex(state, cAlphaArgs(42));
                if (VRF(VRFNum).HRCAPFTHeat > 0) {
                    // Verify Curve Object, only legal type is bi-quadratic or linear, quadratic, or cubic
                    ErrorsFound |= CurveManager::CheckCurveDims(state, VRF(VRFNum).HRCAPFTHeat, // Curve index
                                                                {1, 2},                  // Valid dimensions
                                                                RoutineName,             // Routine name
                                                                cCurrentModuleObject,    // Object Type
                                                                VRF(VRFNum).Name,        // Object Name
                                                                cAlphaFieldNames(42));   // Field Name
                }
                VRF(VRFNum).HRInitialHeatCapFrac = rNumericArgs(35);
                VRF(VRFNum).HRHeatCapTC = rNumericArgs(36);

                //  INTEGER      :: HREIRFTHeat                =0   ! Index to heat EIR as a function of temperature curve for heat recovery
                //  REAL(r64)    :: HRInitialHeatEIRFrac       =0.0d0 ! Fractional EIR degradation at the start of heat recovery from heating mode
                //  REAL(r64)    :: HRHeatEIRTC                =0.0d0 ! Time constant used to recover from initial degradation in heating heat
                //  recovery
                VRF(VRFNum).HREIRFTHeat = GetCurveIndex(state, cAlphaArgs(43));
                if (VRF(VRFNum).HREIRFTHeat > 0) {
                    // Verify Curve Object, only legal type is bi-quadratic or linear, quadratic, or cubic
                    ErrorsFound |= CurveManager::CheckCurveDims(state, VRF(VRFNum).HREIRFTHeat, // Curve index
                                                                {1, 2},                  // Valid dimensions
                                                                RoutineName,             // Routine name
                                                                cCurrentModuleObject,    // Object Type
                                                                VRF(VRFNum).Name,        // Object Name
                                                                cAlphaFieldNames(43));   // Field Name
                }
                VRF(VRFNum).HRInitialHeatEIRFrac = rNumericArgs(37);
                VRF(VRFNum).HRHeatEIRTC = rNumericArgs(38);
            }
        }

        // Read all VRF condenser objects: Algorithm Type 2_physics based model (VRF-FluidTCtrl-HP)_Aug. 2015, zrp
        cCurrentModuleObject = "AirConditioner:VariableRefrigerantFlow:FluidTemperatureControl";
        for (VRFNum = NumVRFCond_SysCurve + 1; VRFNum <= NumVRFCond_SysCurve + NumVRFCond_FluidTCtrl_HP; ++VRFNum) {
            inputProcessor->getObjectItem(state,
                                          cCurrentModuleObject,
                                          VRFNum,
                                          cAlphaArgs,
                                          NumAlphas,
                                          rNumericArgs,
                                          NumNums,
                                          IOStat,
                                          lNumericFieldBlanks,
                                          lAlphaFieldBlanks,
                                          cAlphaFieldNames,
                                          cNumericFieldNames);
            GlobalNames::VerifyUniqueInterObjectName(VrfUniqueNames, cAlphaArgs(1), cCurrentModuleObject, cAlphaFieldNames(1), ErrorsFound);

            VRF(VRFNum).Name = cAlphaArgs(1);
            VRF(VRFNum).VRFSystemTypeNum = VRF_HeatPump;
            VRF(VRFNum).VRFAlgorithmTypeNum = AlgorithmTypeFluidTCtrl;
            VRF(VRFNum).FuelType = "Electricity";
            VRF(VRFNum).FuelTypeNum = DataGlobalConstants::iRT_Electricity;

            if (lAlphaFieldBlanks(2)) {
                VRF(VRFNum).SchedPtr = ScheduleAlwaysOn;
            } else {
                VRF(VRFNum).SchedPtr = GetScheduleIndex(state, cAlphaArgs(2));
                if (VRF(VRFNum).SchedPtr == 0) {
                    ShowSevereError(cCurrentModuleObject + "=\"" + VRF(VRFNum).Name + "\" invalid data");
                    ShowContinueError("Invalid-not found " + cAlphaFieldNames(2) + "=\"" + cAlphaArgs(2) + "\".");
                    ErrorsFound = true;
                }
            }

            VRF(VRFNum).ZoneTUListPtr = UtilityRoutines::FindItemInList(cAlphaArgs(3), TerminalUnitList, NumVRFTULists);
            if (VRF(VRFNum).ZoneTUListPtr == 0) {
                ShowSevereError(cCurrentModuleObject + " = \"" + VRF(VRFNum).Name + "\"");
                ShowContinueError(cAlphaFieldNames(3) + " = " + cAlphaArgs(3) + " not found.");
                ErrorsFound = true;
            }

            // Refrigerant type
            VRF(VRFNum).RefrigerantName = cAlphaArgs(4);
            if (EnergyPlus::FluidProperties::GetInput) {
                EnergyPlus::FluidProperties::GetFluidPropertiesData(state);
                EnergyPlus::FluidProperties::GetInput = false;
            }
            if (UtilityRoutines::FindItemInList(
                    VRF(VRFNum).RefrigerantName, EnergyPlus::FluidProperties::RefrigData, EnergyPlus::FluidProperties::NumOfRefrigerants) == 0) {
                ShowSevereError(cCurrentModuleObject + " = " + VRF(VRFNum).Name);
                ShowContinueError("Illegal " + cAlphaFieldNames(4) + " = " + cAlphaArgs(4));
                ErrorsFound = true;
            }

            VRF(VRFNum).RatedEvapCapacity = rNumericArgs(1);
            VRF(VRFNum).RatedCompPowerPerCapcity = rNumericArgs(2);
            VRF(VRFNum).RatedCompPower = VRF(VRFNum).RatedCompPowerPerCapcity * VRF(VRFNum).RatedEvapCapacity;
            VRF(VRFNum).CoolingCapacity = VRF(VRFNum).RatedEvapCapacity;
            VRF(VRFNum).RatedHeatCapacity = VRF(VRFNum).RatedEvapCapacity * (1 + VRF(VRFNum).RatedCompPowerPerCapcity);
            VRF(VRFNum).HeatingCapacity = VRF(VRFNum).RatedHeatCapacity;

            // Reference system COP
            VRF(VRFNum).CoolingCOP = 1 / VRF(VRFNum).RatedCompPowerPerCapcity;
            VRF(VRFNum).HeatingCOP = 1 / VRF(VRFNum).RatedCompPowerPerCapcity + 1;

            // OA temperature range for VRF-HP operations
            VRF(VRFNum).MinOATCooling = rNumericArgs(3);
            VRF(VRFNum).MaxOATCooling = rNumericArgs(4);
            VRF(VRFNum).MinOATHeating = rNumericArgs(5);
            VRF(VRFNum).MaxOATHeating = rNumericArgs(6);
            if (VRF(VRFNum).MinOATCooling >= VRF(VRFNum).MaxOATCooling) {
                ShowSevereError(cCurrentModuleObject + ", \"" + VRF(VRFNum).Name + "\"");
                ShowContinueError("... " + cNumericFieldNames(3) + " (" + TrimSigDigits(VRF(VRFNum).MinOATCooling, 3) +
                                  ") must be less than maximum (" + TrimSigDigits(VRF(VRFNum).MaxOATCooling, 3) + ").");
                ErrorsFound = true;
            }
            if (VRF(VRFNum).MinOATHeating >= VRF(VRFNum).MaxOATHeating) {
                ShowSevereError(cCurrentModuleObject + ", \"" + VRF(VRFNum).Name + "\"");
                ShowContinueError("... " + cNumericFieldNames(5) + " (" + TrimSigDigits(VRF(VRFNum).MinOATHeating, 3) +
                                  ") must be less than maximum (" + TrimSigDigits(VRF(VRFNum).MaxOATHeating, 3) + ").");
                ErrorsFound = true;
            }

            // Reference OU SH/SC
            VRF(VRFNum).SH = rNumericArgs(7);
            VRF(VRFNum).SC = rNumericArgs(8);

            if (UtilityRoutines::SameString(cAlphaArgs(5), "VariableTemp")) {
                VRF(VRFNum).AlgorithmIUCtrl = 1;
            } else if (UtilityRoutines::SameString(cAlphaArgs(5), "ConstantTemp")) {
                VRF(VRFNum).AlgorithmIUCtrl = 2;
            } else {
                VRF(VRFNum).AlgorithmIUCtrl = 1;
            }

            // Reference IU Te/Tc for IU Control Algorithm: ConstantTemp
            VRF(VRFNum).EvapTempFixed = rNumericArgs(9);
            VRF(VRFNum).CondTempFixed = rNumericArgs(10);

            // Bounds of Te/Tc for IU Control Algorithm: VariableTemp
            VRF(VRFNum).IUEvapTempLow = rNumericArgs(11);
            VRF(VRFNum).IUEvapTempHigh = rNumericArgs(12);
            VRF(VRFNum).IUCondTempLow = rNumericArgs(13);
            VRF(VRFNum).IUCondTempHigh = rNumericArgs(14);
            if (VRF(VRFNum).IUEvapTempLow >= VRF(VRFNum).IUEvapTempHigh) {
                ShowSevereError(cCurrentModuleObject + ", \"" + VRF(VRFNum).Name + "\"");
                ShowContinueError("... " + cNumericFieldNames(11) + " (" + TrimSigDigits(VRF(VRFNum).IUEvapTempLow, 3) +
                                  ") must be less than maximum (" + TrimSigDigits(VRF(VRFNum).IUEvapTempHigh, 3) + ").");
                ErrorsFound = true;
            }
            if (VRF(VRFNum).IUCondTempLow >= VRF(VRFNum).IUCondTempHigh) {
                ShowSevereError(cCurrentModuleObject + ", \"" + VRF(VRFNum).Name + "\"");
                ShowContinueError("... " + cNumericFieldNames(13) + " (" + TrimSigDigits(VRF(VRFNum).IUCondTempLow, 3) +
                                  ") must be less than maximum (" + TrimSigDigits(VRF(VRFNum).IUCondTempHigh, 3) + ").");
                ErrorsFound = true;
            }

            // Get OU fan data
            VRF(VRFNum).RatedOUFanPowerPerCapcity = rNumericArgs(15);
            VRF(VRFNum).OUAirFlowRatePerCapcity = rNumericArgs(16);
            VRF(VRFNum).RatedOUFanPower = VRF(VRFNum).RatedOUFanPowerPerCapcity * VRF(VRFNum).RatedEvapCapacity;
            VRF(VRFNum).OUAirFlowRate = VRF(VRFNum).OUAirFlowRatePerCapcity * VRF(VRFNum).RatedEvapCapacity;

            // OUEvapTempCurve
            int indexOUEvapTempCurve = GetCurveIndex(state, cAlphaArgs(6)); // convert curve name to index number
            // Verify curve name and type
            if (indexOUEvapTempCurve == 0) {
                if (lAlphaFieldBlanks(6)) {
                    ShowSevereError(RoutineName + cCurrentModuleObject + "=\"" + VRF(VRFNum).Name + "\", missing");
                    ShowContinueError("...required " + cAlphaFieldNames(6) + " is blank.");
                } else {
                    ShowSevereError(RoutineName + cCurrentModuleObject + "=\"" + VRF(VRFNum).Name + "\", invalid");
                    ShowContinueError("...not found " + cAlphaFieldNames(6) + "=\"" + cAlphaArgs(6) + "\".");
                }
                ErrorsFound = true;
            } else {
                {
                    if (state.dataCurveManager->PerfCurve(indexOUEvapTempCurve).ObjectType == "Curve:Quadratic") {
                        VRF(VRFNum).C1Te = state.dataCurveManager->PerfCurve(indexOUEvapTempCurve).Coeff1;
                        VRF(VRFNum).C2Te = state.dataCurveManager->PerfCurve(indexOUEvapTempCurve).Coeff2;
                        VRF(VRFNum).C3Te = state.dataCurveManager->PerfCurve(indexOUEvapTempCurve).Coeff3;

                    } else {
                        ShowSevereError(RoutineName + cCurrentModuleObject + "=\"" + VRF(VRFNum).Name + "\", invalid");
                        ShowContinueError("...illegal " + cAlphaFieldNames(6) +
                                          " type for this object = " + state.dataCurveManager->PerfCurve(indexOUEvapTempCurve).ObjectType);
                        ShowContinueError("... Curve type must be Quadratic.");
                        ErrorsFound = true;
                    }
                }
            }

            // OUCondTempCurve
            int indexOUCondTempCurve = GetCurveIndex(state, cAlphaArgs(7)); // convert curve name to index number
            // Verify curve name and type
            if (indexOUCondTempCurve == 0) {
                if (lAlphaFieldBlanks(7)) {
                    ShowSevereError(RoutineName + cCurrentModuleObject + "=\"" + VRF(VRFNum).Name + "\", missing");
                    ShowContinueError("...required " + cAlphaFieldNames(7) + " is blank.");
                } else {
                    ShowSevereError(RoutineName + cCurrentModuleObject + "=\"" + VRF(VRFNum).Name + "\", invalid");
                    ShowContinueError("...not found " + cAlphaFieldNames(7) + "=\"" + cAlphaArgs(7) + "\".");
                }
                ErrorsFound = true;
            } else {
                {
                    if (state.dataCurveManager->PerfCurve(indexOUCondTempCurve).ObjectType == "Curve:Quadratic") {
                        VRF(VRFNum).C1Tc = state.dataCurveManager->PerfCurve(indexOUCondTempCurve).Coeff1;
                        VRF(VRFNum).C2Tc = state.dataCurveManager->PerfCurve(indexOUCondTempCurve).Coeff2;
                        VRF(VRFNum).C3Tc = state.dataCurveManager->PerfCurve(indexOUCondTempCurve).Coeff3;

                    } else {
                        ShowSevereError(RoutineName + cCurrentModuleObject + "=\"" + VRF(VRFNum).Name + "\", invalid");
                        ShowContinueError("...illegal " + cAlphaFieldNames(7) +
                                          " type for this object = " + state.dataCurveManager->PerfCurve(indexOUCondTempCurve).ObjectType);
                        ShowContinueError("... Curve type must be Quadratic.");
                        ErrorsFound = true;
                    }
                }
            }

            // Pipe parameters
            VRF(VRFNum).RefPipDiaSuc = rNumericArgs(17);
            VRF(VRFNum).RefPipDiaDis = rNumericArgs(17);
            VRF(VRFNum).RefPipLen = rNumericArgs(18);
            VRF(VRFNum).RefPipEquLen = rNumericArgs(19);
            VRF(VRFNum).RefPipHei = rNumericArgs(20);
            VRF(VRFNum).RefPipInsThi = rNumericArgs(21);
            VRF(VRFNum).RefPipInsCon = rNumericArgs(22);

            // Check the RefPipEquLen
            if (lNumericFieldBlanks(19) && !lNumericFieldBlanks(18)) {
                VRF(VRFNum).RefPipEquLen = 1.2 * VRF(VRFNum).RefPipLen;
                ShowWarningError(cCurrentModuleObject + ", \"" + VRF(VRFNum).Name + "\", \" " + cNumericFieldNames(19) + "\" is calculated based on");
                ShowContinueError("...the provided \"" + cNumericFieldNames(18) + "\" value.");
            }
            if (VRF(VRFNum).RefPipEquLen < VRF(VRFNum).RefPipLen) {
                VRF(VRFNum).RefPipEquLen = 1.2 * VRF(VRFNum).RefPipLen;
                ShowWarningError(cCurrentModuleObject + ", \"" + VRF(VRFNum).Name + "\", invalid \" " + cNumericFieldNames(19) + "\" value.");
                ShowContinueError("...Equivalent length of main pipe should be greater than or equal to the actual length.");
                ShowContinueError("...The value is recalculated based on the provided \"" + cNumericFieldNames(18) + "\" value.");
            }

            // Crank case
            VRF(VRFNum).CCHeaterPower = rNumericArgs(23);
            VRF(VRFNum).NumCompressors = rNumericArgs(24);
            VRF(VRFNum).CompressorSizeRatio = rNumericArgs(25);
            VRF(VRFNum).MaxOATCCHeater = rNumericArgs(26);

            // Defrost
            if (!lAlphaFieldBlanks(8)) {
                if (UtilityRoutines::SameString(cAlphaArgs(8), "ReverseCycle")) VRF(VRFNum).DefrostStrategy = ReverseCycle;
                if (UtilityRoutines::SameString(cAlphaArgs(8), "Resistive")) VRF(VRFNum).DefrostStrategy = Resistive;
                if (VRF(VRFNum).DefrostStrategy == 0) {
                    ShowSevereError(cCurrentModuleObject + ", \"" + VRF(VRFNum).Name + "\" " + cAlphaFieldNames(8) + " not found: " + cAlphaArgs(8));
                    ErrorsFound = true;
                }
            } else {
                VRF(VRFNum).DefrostStrategy = ReverseCycle;
            }

            if (!lAlphaFieldBlanks(9)) {
                if (UtilityRoutines::SameString(cAlphaArgs(9), "Timed")) VRF(VRFNum).DefrostControl = Timed;
                if (UtilityRoutines::SameString(cAlphaArgs(9), "OnDemand")) VRF(VRFNum).DefrostControl = OnDemand;
                if (VRF(VRFNum).DefrostControl == 0) {
                    ShowSevereError(cCurrentModuleObject + ", \"" + VRF(VRFNum).Name + "\" " + cAlphaFieldNames(9) + " not found: " + cAlphaArgs(9));
                    ErrorsFound = true;
                }
            } else {
                VRF(VRFNum).DefrostControl = Timed;
            }

            if (!lAlphaFieldBlanks(10)) {
                VRF(VRFNum).DefrostEIRPtr = GetCurveIndex(state, cAlphaArgs(10));
                if (VRF(VRFNum).DefrostEIRPtr > 0) {
                    // Verify Curve Object, expected type is BiQuadratic
                    ErrorsFound |= CurveManager::CheckCurveDims(state, VRF(VRFNum).DefrostEIRPtr, // Curve index
                                                                {2},                       // Valid dimensions
                                                                RoutineName,               // Routine name
                                                                cCurrentModuleObject,      // Object Type
                                                                VRF(VRFNum).Name,          // Object Name
                                                                cAlphaFieldNames(10));     // Field Name
                } else {
                    if (VRF(VRFNum).DefrostStrategy == ReverseCycle && VRF(VRFNum).DefrostControl == OnDemand) {
                        ShowSevereError(cCurrentModuleObject + ", \"" + VRF(VRFNum).Name + "\" " + cAlphaFieldNames(10) +
                                        " not found:" + cAlphaArgs(10));
                        ErrorsFound = true;
                    }
                }
            } else {
                if (VRF(VRFNum).DefrostStrategy == ReverseCycle && VRF(VRFNum).DefrostControl == OnDemand) {
                    ShowSevereError(cCurrentModuleObject + ", \"" + VRF(VRFNum).Name + "\" " + cAlphaFieldNames(10) + " not found:" + cAlphaArgs(10));
                    ErrorsFound = true;
                }
            }

            VRF(VRFNum).DefrostFraction = rNumericArgs(27);
            VRF(VRFNum).DefrostCapacity = rNumericArgs(28);
            VRF(VRFNum).MaxOATDefrost = rNumericArgs(29);
            if (VRF(VRFNum).DefrostCapacity == 0.0 && VRF(VRFNum).DefrostStrategy == Resistive) {
                ShowWarningError(cCurrentModuleObject + ", \"" + VRF(VRFNum).Name + "\" " + cNumericFieldNames(28) +
                                 " = 0.0 for defrost strategy = RESISTIVE.");
            }

            VRF(VRFNum).CompMaxDeltaP = rNumericArgs(30);

            //@@ The control type
            std::string ThermostatPriorityType = "LoadPriority"; // cAlphaArgs( 25 )
            if (UtilityRoutines::SameString(ThermostatPriorityType, "LoadPriority")) {
                VRF(VRFNum).ThermostatPriority = LoadPriority;
            } else if (UtilityRoutines::SameString(ThermostatPriorityType, "ZonePriority")) {
                VRF(VRFNum).ThermostatPriority = ZonePriority;
            } else if (UtilityRoutines::SameString(ThermostatPriorityType, "ThermostatOffsetPriority")) {
                VRF(VRFNum).ThermostatPriority = ThermostatOffsetPriority;
            } else if (UtilityRoutines::SameString(ThermostatPriorityType, "Scheduled")) {
                VRF(VRFNum).ThermostatPriority = ScheduledPriority;
            } else if (UtilityRoutines::SameString(ThermostatPriorityType, "MasterThermostatPriority")) {
                VRF(VRFNum).ThermostatPriority = MasterThermostatPriority;
                if (VRF(VRFNum).MasterZonePtr == 0) {
                    ShowSevereError(cCurrentModuleObject + " = \"" + VRF(VRFNum).Name + "\"");
                    //** ShowContinueError( cAlphaFieldNames( 24 ) + " must be entered when " + cAlphaFieldNames( 25 ) + " = " + cAlphaArgs( 25 ) );
                    ErrorsFound = true;
                }
            } else {
                ShowSevereError(cCurrentModuleObject + " = " + VRF(VRFNum).Name);
                // ShowContinueError( "Illegal " + cAlphaFieldNames( 25 ) + " = " + cAlphaArgs( 25 ) );
                ErrorsFound = true;
            }

            // The new VRF model is Air cooled
            VRF(VRFNum).CondenserType = DataHVACGlobals::AirCooled;
            VRF(VRFNum).CondenserNodeNum = 0;

            // Evaporative Capacity & Compressor Power Curves corresponding to each Loading Index / compressor speed
            NumOfCompSpd = rNumericArgs(31);
            VRF(VRFNum).CompressorSpeed.dimension(NumOfCompSpd);
            VRF(VRFNum).OUCoolingCAPFT.dimension(NumOfCompSpd);
            VRF(VRFNum).OUCoolingPWRFT.dimension(NumOfCompSpd);
            int Count1Index = 31; // the index of the last numeric field before compressor speed entries
            int Count2Index = 9;  // the index of the last alpha field before capacity/power curves
            for (NumCompSpd = 1; NumCompSpd <= NumOfCompSpd; NumCompSpd++) {
                VRF(VRFNum).CompressorSpeed(NumCompSpd) = rNumericArgs(Count1Index + NumCompSpd);

                // Evaporating Capacity Curve
                if (!lAlphaFieldBlanks(Count2Index + 2 * NumCompSpd)) {
                    int indexOUEvapCapCurve = GetCurveIndex(state, cAlphaArgs(Count2Index + 2 * NumCompSpd)); // convert curve name to index number
                    if (indexOUEvapCapCurve == 0) {                                                    // Verify curve name and type
                        if (lAlphaFieldBlanks(Count2Index + 2 * NumCompSpd)) {
                            ShowSevereError(RoutineName + cCurrentModuleObject + "=\"" + VRF(VRFNum).Name + "\", missing");
                            ShowContinueError("...required " + cAlphaFieldNames(Count2Index + 2 * NumCompSpd) + " is blank.");
                        } else {
                            ShowSevereError(RoutineName + cCurrentModuleObject + "=\"" + VRF(VRFNum).Name + "\", invalid");
                            ShowContinueError("...not found " + cAlphaFieldNames(Count2Index + 2 * NumCompSpd) + "=\"" +
                                              cAlphaArgs(Count2Index + 2 * NumCompSpd) + "\".");
                        }
                        ErrorsFound = true;
                    } else {
                        ErrorsFound |= CurveManager::CheckCurveDims(state, indexOUEvapCapCurve,                             // Curve index
                                                                    {2},                                             // Valid dimensions
                                                                    RoutineName,                                     // Routine name
                                                                    cCurrentModuleObject,                            // Object Type
                                                                    VRF(VRFNum).Name,                                // Object Name
                                                                    cAlphaFieldNames(Count2Index + 2 * NumCompSpd)); // Field Name

                        if (!ErrorsFound) {
                            VRF(VRFNum).OUCoolingCAPFT(NumCompSpd) = indexOUEvapCapCurve;
                        }
                    }
                }

                // Compressor Power Curve
                if (!lAlphaFieldBlanks(Count2Index + 2 * NumCompSpd + 1)) {
                    int indexOUCompPwrCurve = GetCurveIndex(state, cAlphaArgs(Count2Index + 2 * NumCompSpd + 1)); // convert curve name to index number
                    if (indexOUCompPwrCurve == 0) {                                                        // Verify curve name and type
                        if (lAlphaFieldBlanks(Count2Index + 2 * NumCompSpd + 1)) {
                            ShowSevereError(RoutineName + cCurrentModuleObject + "=\"" + VRF(VRFNum).Name + "\", missing");
                            ShowContinueError("...required " + cAlphaFieldNames(Count2Index + 2 * NumCompSpd + 1) + " is blank.");
                        } else {
                            ShowSevereError(RoutineName + cCurrentModuleObject + "=\"" + VRF(VRFNum).Name + "\", invalid");
                            ShowContinueError("...not found " + cAlphaFieldNames(Count2Index + 2 * NumCompSpd + 1) + "=\"" +
                                              cAlphaArgs(Count2Index + 2 * NumCompSpd + 1) + "\".");
                        }
                        ErrorsFound = true;
                    } else {
                        ErrorsFound |= CurveManager::CheckCurveDims(state, indexOUCompPwrCurve,                                 // Curve index
                                                                    {2},                                                 // Valid dimensions
                                                                    RoutineName,                                         // Routine name
                                                                    cCurrentModuleObject,                                // Object Type
                                                                    VRF(VRFNum).Name,                                    // Object Name
                                                                    cAlphaFieldNames(Count2Index + 2 * NumCompSpd + 1)); // Field Name

                        if (!ErrorsFound) {
                            VRF(VRFNum).OUCoolingPWRFT(NumCompSpd) = indexOUCompPwrCurve;
                        }
                    }
                }
            }
        }

        // Read all VRF condenser objects: Algorithm Type 2_physics based model (VRF-FluidTCtrl-HR)_Mar. 2016, zrp
        cCurrentModuleObject = "AirConditioner:VariableRefrigerantFlow:FluidTemperatureControl:HR";
        for (VRFNum = NumVRFCond_SysCurve + NumVRFCond_FluidTCtrl_HP + 1;
             VRFNum <= NumVRFCond_SysCurve + NumVRFCond_FluidTCtrl_HP + NumVRFCond_FluidTCtrl_HR;
             ++VRFNum) {
            inputProcessor->getObjectItem(state,
                                          cCurrentModuleObject,
                                          VRFNum,
                                          cAlphaArgs,
                                          NumAlphas,
                                          rNumericArgs,
                                          NumNums,
                                          IOStat,
                                          lNumericFieldBlanks,
                                          lAlphaFieldBlanks,
                                          cAlphaFieldNames,
                                          cNumericFieldNames);
            GlobalNames::VerifyUniqueInterObjectName(VrfUniqueNames, cAlphaArgs(1), cCurrentModuleObject, cAlphaFieldNames(1), ErrorsFound);

            VRF(VRFNum).Name = cAlphaArgs(1);

            VRF(VRFNum).ThermostatPriority = LoadPriority;
            VRF(VRFNum).HeatRecoveryUsed = true;
            VRF(VRFNum).VRFSystemTypeNum = VRF_HeatPump;
            VRF(VRFNum).VRFAlgorithmTypeNum = AlgorithmTypeFluidTCtrl;
            VRF(VRFNum).FuelType = "Electricity";
            VRF(VRFNum).FuelTypeNum = DataGlobalConstants::iRT_Electricity;

            if (lAlphaFieldBlanks(2)) {
                VRF(VRFNum).SchedPtr = ScheduleAlwaysOn;
            } else {
                VRF(VRFNum).SchedPtr = GetScheduleIndex(state, cAlphaArgs(2));
                if (VRF(VRFNum).SchedPtr == 0) {
                    ShowSevereError(cCurrentModuleObject + "=\"" + VRF(VRFNum).Name + "\" invalid data");
                    ShowContinueError("Invalid-not found " + cAlphaFieldNames(2) + "=\"" + cAlphaArgs(2) + "\".");
                    ErrorsFound = true;
                }
            }

            VRF(VRFNum).ZoneTUListPtr = UtilityRoutines::FindItemInList(cAlphaArgs(3), TerminalUnitList, NumVRFTULists);
            if (VRF(VRFNum).ZoneTUListPtr == 0) {
                ShowSevereError(cCurrentModuleObject + " = \"" + VRF(VRFNum).Name + "\"");
                ShowContinueError(cAlphaFieldNames(3) + " = " + cAlphaArgs(3) + " not found.");
                ErrorsFound = true;
            }

            // Refrigerant type
            VRF(VRFNum).RefrigerantName = cAlphaArgs(4);
            if (EnergyPlus::FluidProperties::GetInput) {
                EnergyPlus::FluidProperties::GetFluidPropertiesData(state);
                EnergyPlus::FluidProperties::GetInput = false;
            }
            if (UtilityRoutines::FindItemInList(
                    VRF(VRFNum).RefrigerantName, EnergyPlus::FluidProperties::RefrigData, EnergyPlus::FluidProperties::NumOfRefrigerants) == 0) {
                ShowSevereError(cCurrentModuleObject + " = " + VRF(VRFNum).Name);
                ShowContinueError("Illegal " + cAlphaFieldNames(4) + " = " + cAlphaArgs(4));
                ErrorsFound = true;
            }

            VRF(VRFNum).RatedEvapCapacity = rNumericArgs(1);
            VRF(VRFNum).RatedCompPowerPerCapcity = rNumericArgs(2);
            VRF(VRFNum).RatedCompPower = VRF(VRFNum).RatedCompPowerPerCapcity * VRF(VRFNum).RatedEvapCapacity;
            VRF(VRFNum).CoolingCapacity = VRF(VRFNum).RatedEvapCapacity;
            VRF(VRFNum).HeatingCapacity = VRF(VRFNum).RatedEvapCapacity * (1 + VRF(VRFNum).RatedCompPowerPerCapcity);

            // Reference system COP
            VRF(VRFNum).CoolingCOP = 1 / VRF(VRFNum).RatedCompPowerPerCapcity;
            VRF(VRFNum).HeatingCOP = 1 / VRF(VRFNum).RatedCompPowerPerCapcity + 1;

            // OA temperature range for VRF-HP operations
            VRF(VRFNum).MinOATCooling = rNumericArgs(3);
            VRF(VRFNum).MaxOATCooling = rNumericArgs(4);
            VRF(VRFNum).MinOATHeating = rNumericArgs(5);
            VRF(VRFNum).MaxOATHeating = rNumericArgs(6);
            VRF(VRFNum).MinOATHeatRecovery = rNumericArgs(7);
            VRF(VRFNum).MaxOATHeatRecovery = rNumericArgs(8);
            if (VRF(VRFNum).MinOATCooling >= VRF(VRFNum).MaxOATCooling) {
                ShowSevereError(cCurrentModuleObject + ", \"" + VRF(VRFNum).Name + "\"");
                ShowContinueError("... " + cNumericFieldNames(3) + " (" + TrimSigDigits(VRF(VRFNum).MinOATCooling, 3) +
                                  ") must be less than maximum (" + TrimSigDigits(VRF(VRFNum).MaxOATCooling, 3) + ").");
                ErrorsFound = true;
            }
            if (VRF(VRFNum).MinOATHeating >= VRF(VRFNum).MaxOATHeating) {
                ShowSevereError(cCurrentModuleObject + ", \"" + VRF(VRFNum).Name + "\"");
                ShowContinueError("... " + cNumericFieldNames(5) + " (" + TrimSigDigits(VRF(VRFNum).MinOATHeating, 3) +
                                  ") must be less than maximum (" + TrimSigDigits(VRF(VRFNum).MaxOATHeating, 3) + ").");
                ErrorsFound = true;
            }
            if (VRF(VRFNum).MinOATHeatRecovery >= VRF(VRFNum).MaxOATHeatRecovery) {
                ShowSevereError(cCurrentModuleObject + ", \"" + VRF(VRFNum).Name + "\"");
                ShowContinueError("... " + cNumericFieldNames(7) + " (" + TrimSigDigits(VRF(VRFNum).MinOATHeating, 3) +
                                  ") must be less than maximum (" + TrimSigDigits(VRF(VRFNum).MaxOATHeating, 3) + ").");
                ErrorsFound = true;
            }
            if (VRF(VRFNum).MinOATHeatRecovery < VRF(VRFNum).MinOATCooling && VRF(VRFNum).MinOATHeatRecovery < VRF(VRFNum).MinOATHeating) {
                ShowWarningError(cCurrentModuleObject + " = \"" + VRF(VRFNum).Name + "\", " + cNumericFieldNames(7) +
                                 " is less than the minimum temperature in heat pump mode.");
                ShowContinueError("..." + cNumericFieldNames(7) + " = " + TrimSigDigits(VRF(VRFNum).MinOATHeatRecovery, 2) + " C");
                ShowContinueError("...Minimum Outdoor Temperature in Cooling Mode = " + TrimSigDigits(VRF(VRFNum).MinOATCooling, 2) + " C");
                ShowContinueError("...Minimum Outdoor Temperature in Heating Mode = " + TrimSigDigits(VRF(VRFNum).MinOATHeating, 2) + " C");
                ShowContinueError("...Minimum Outdoor Temperature in Heat Recovery Mode reset to lesser of cooling or heating minimum temperature "
                                  "and simulation continues.");
                VRF(VRFNum).MinOATHeatRecovery = min(VRF(VRFNum).MinOATCooling, VRF(VRFNum).MinOATHeating);
                ShowContinueError("... adjusted " + cNumericFieldNames(7) + " = " + TrimSigDigits(VRF(VRFNum).MinOATHeatRecovery, 2) + " C");
            }
            if (VRF(VRFNum).MaxOATHeatRecovery > VRF(VRFNum).MaxOATCooling && VRF(VRFNum).MaxOATHeatRecovery > VRF(VRFNum).MaxOATHeating) {
                ShowWarningError(cCurrentModuleObject + " = \"" + VRF(VRFNum).Name + "\", " + cNumericFieldNames(8) +
                                 " is greater than the maximum temperature in heat pump mode.");
                ShowContinueError("..." + cNumericFieldNames(8) + " = " + TrimSigDigits(VRF(VRFNum).MaxOATHeatRecovery, 2) + " C");
                ShowContinueError("...Maximum Outdoor Temperature in Cooling Mode = " + TrimSigDigits(VRF(VRFNum).MaxOATCooling, 2) + " C");
                ShowContinueError("...Maximum Outdoor Temperature in Heating Mode = " + TrimSigDigits(VRF(VRFNum).MaxOATHeating, 2) + " C");
                ShowContinueError("...Maximum Outdoor Temperature in Heat Recovery Mode reset to greater of cooling or heating maximum temperature "
                                  "and simulation continues.");
                VRF(VRFNum).MaxOATHeatRecovery = max(VRF(VRFNum).MaxOATCooling, VRF(VRFNum).MaxOATHeating);
                ShowContinueError("... adjusted " + cNumericFieldNames(8) + " = " + TrimSigDigits(VRF(VRFNum).MaxOATHeatRecovery, 2) + " C");
            }

            // IU Control Type
            if (UtilityRoutines::SameString(cAlphaArgs(5), "VariableTemp")) {
                VRF(VRFNum).AlgorithmIUCtrl = 1;
            } else if (UtilityRoutines::SameString(cAlphaArgs(5), "ConstantTemp")) {
                VRF(VRFNum).AlgorithmIUCtrl = 2;
            } else {
                VRF(VRFNum).AlgorithmIUCtrl = 1;
            }

            // Reference IU Te/Tc for IU Control Algorithm: ConstantTemp
            VRF(VRFNum).EvapTempFixed = rNumericArgs(9);
            VRF(VRFNum).CondTempFixed = rNumericArgs(10);

            // Bounds of Te/Tc for IU Control Algorithm: VariableTemp
            VRF(VRFNum).IUEvapTempLow = rNumericArgs(11);
            VRF(VRFNum).IUEvapTempHigh = rNumericArgs(12);
            VRF(VRFNum).IUCondTempLow = rNumericArgs(13);
            VRF(VRFNum).IUCondTempHigh = rNumericArgs(14);
            if (VRF(VRFNum).IUEvapTempLow >= VRF(VRFNum).IUEvapTempHigh) {
                ShowSevereError(cCurrentModuleObject + ", \"" + VRF(VRFNum).Name + "\"");
                ShowContinueError("... " + cNumericFieldNames(11) + " (" + TrimSigDigits(VRF(VRFNum).IUEvapTempLow, 3) +
                                  ") must be less than maximum (" + TrimSigDigits(VRF(VRFNum).IUEvapTempHigh, 3) + ").");
                ErrorsFound = true;
            }
            if (VRF(VRFNum).IUCondTempLow >= VRF(VRFNum).IUCondTempHigh) {
                ShowSevereError(cCurrentModuleObject + ", \"" + VRF(VRFNum).Name + "\"");
                ShowContinueError("... " + cNumericFieldNames(13) + " (" + TrimSigDigits(VRF(VRFNum).IUCondTempLow, 3) +
                                  ") must be less than maximum (" + TrimSigDigits(VRF(VRFNum).IUCondTempHigh, 3) + ").");
                ErrorsFound = true;
            }

            // Reference OU SH/SC
            VRF(VRFNum).SH = rNumericArgs(15);
            VRF(VRFNum).SC = rNumericArgs(16);
            if (VRF(VRFNum).SH > 20) {
                ShowWarningError(cCurrentModuleObject + ", \"" + VRF(VRFNum).Name + "\", \" " + cNumericFieldNames(15));
                ShowContinueError("...is higher than 20C, which is usually the maximum of normal range.");
            }
            if (VRF(VRFNum).SC > 20) {
                ShowWarningError(cCurrentModuleObject + ", \"" + VRF(VRFNum).Name + "\", \" " + cNumericFieldNames(15));
                ShowContinueError("...is higher than 20C, which is usually the maximum of normal range.");
            }

            // OU Heat Exchanger Rated Bypass Factor
            VRF(VRFNum).RateBFOUEvap = rNumericArgs(17);
            VRF(VRFNum).RateBFOUCond = rNumericArgs(18);

            // Difference between Outdoor Unit Te and OAT during Simultaneous Heating and Cooling operations
            VRF(VRFNum).DiffOUTeTo = rNumericArgs(19);

            // HR OU Heat Exchanger Capacity Ratio
            VRF(VRFNum).HROUHexRatio = rNumericArgs(20);

            // Get OU fan data
            VRF(VRFNum).RatedOUFanPowerPerCapcity = rNumericArgs(21);
            VRF(VRFNum).OUAirFlowRatePerCapcity = rNumericArgs(22);
            VRF(VRFNum).RatedOUFanPower = VRF(VRFNum).RatedOUFanPowerPerCapcity * VRF(VRFNum).RatedEvapCapacity;
            VRF(VRFNum).OUAirFlowRate = VRF(VRFNum).OUAirFlowRatePerCapcity * VRF(VRFNum).RatedEvapCapacity;

            // OUEvapTempCurve
            int indexOUEvapTempCurve = GetCurveIndex(state, cAlphaArgs(6)); // convert curve name to index number
            // Verify curve name and type
            if (indexOUEvapTempCurve == 0) {
                if (lAlphaFieldBlanks(6)) {
                    ShowSevereError(RoutineName + cCurrentModuleObject + "=\"" + VRF(VRFNum).Name + "\", missing");
                    ShowContinueError("...required " + cAlphaFieldNames(6) + " is blank.");
                } else {
                    ShowSevereError(RoutineName + cCurrentModuleObject + "=\"" + VRF(VRFNum).Name + "\", invalid");
                    ShowContinueError("...not found " + cAlphaFieldNames(6) + "=\"" + cAlphaArgs(6) + "\".");
                }
                ErrorsFound = true;
            } else {
                if (state.dataCurveManager->PerfCurve(indexOUEvapTempCurve).ObjectType == "Curve:Quadratic") {
                    VRF(VRFNum).C1Te = state.dataCurveManager->PerfCurve(indexOUEvapTempCurve).Coeff1;
                    VRF(VRFNum).C2Te = state.dataCurveManager->PerfCurve(indexOUEvapTempCurve).Coeff2;
                    VRF(VRFNum).C3Te = state.dataCurveManager->PerfCurve(indexOUEvapTempCurve).Coeff3;
                } else {
                    ShowSevereError(RoutineName + cCurrentModuleObject + "=\"" + VRF(VRFNum).Name + "\", invalid");
                    ShowContinueError("...illegal " + cAlphaFieldNames(6) +
                                      " type for this object = " + state.dataCurveManager->PerfCurve(indexOUEvapTempCurve).ObjectType);
                    ShowContinueError("... Curve type must be Quadratic.");
                    ErrorsFound = true;
                }
            }

            // OUCondTempCurve
            int indexOUCondTempCurve = GetCurveIndex(state, cAlphaArgs(7)); // convert curve name to index number
            // Verify curve name and type
            if (indexOUCondTempCurve == 0) {
                if (lAlphaFieldBlanks(7)) {
                    ShowSevereError(RoutineName + cCurrentModuleObject + "=\"" + VRF(VRFNum).Name + "\", missing");
                    ShowContinueError("...required " + cAlphaFieldNames(7) + " is blank.");
                } else {
                    ShowSevereError(RoutineName + cCurrentModuleObject + "=\"" + VRF(VRFNum).Name + "\", invalid");
                    ShowContinueError("...not found " + cAlphaFieldNames(7) + "=\"" + cAlphaArgs(7) + "\".");
                }
                ErrorsFound = true;
            } else {
                if (state.dataCurveManager->PerfCurve(indexOUCondTempCurve).ObjectType == "Curve:Quadratic") {
                    VRF(VRFNum).C1Tc = state.dataCurveManager->PerfCurve(indexOUCondTempCurve).Coeff1;
                    VRF(VRFNum).C2Tc = state.dataCurveManager->PerfCurve(indexOUCondTempCurve).Coeff2;
                    VRF(VRFNum).C3Tc = state.dataCurveManager->PerfCurve(indexOUCondTempCurve).Coeff3;
                } else {
                    ShowSevereError(RoutineName + cCurrentModuleObject + "=\"" + VRF(VRFNum).Name + "\", invalid");
                    ShowContinueError("...illegal " + cAlphaFieldNames(7) +
                                      " type for this object = " + state.dataCurveManager->PerfCurve(indexOUCondTempCurve).ObjectType);
                    ShowContinueError("... Curve type must be Quadratic.");
                    ErrorsFound = true;
                }
            }

            // Pipe parameters
            VRF(VRFNum).RefPipDiaSuc = rNumericArgs(23);
            VRF(VRFNum).RefPipDiaDis = rNumericArgs(24);
            VRF(VRFNum).RefPipLen = rNumericArgs(25);
            VRF(VRFNum).RefPipEquLen = rNumericArgs(26);
            VRF(VRFNum).RefPipHei = rNumericArgs(27);
            VRF(VRFNum).RefPipInsThi = rNumericArgs(28);
            VRF(VRFNum).RefPipInsCon = rNumericArgs(29);

            // Check the RefPipEquLen
            if (lNumericFieldBlanks(26) && !lNumericFieldBlanks(25)) {
                VRF(VRFNum).RefPipEquLen = 1.2 * VRF(VRFNum).RefPipLen;
                ShowWarningError(cCurrentModuleObject + ", \"" + VRF(VRFNum).Name + "\", \" " + cNumericFieldNames(26) + "\" is calculated based on");
                ShowContinueError("...the provided \"" + cNumericFieldNames(25) + "\" value.");
            }
            if (VRF(VRFNum).RefPipEquLen < VRF(VRFNum).RefPipLen) {
                VRF(VRFNum).RefPipEquLen = 1.2 * VRF(VRFNum).RefPipLen;
                ShowWarningError(cCurrentModuleObject + ", \"" + VRF(VRFNum).Name + "\", invalid \" " + cNumericFieldNames(26) + "\" value.");
                ShowContinueError("...Equivalent length of main pipe should be greater than or equal to the actual length.");
                ShowContinueError("...The value is recalculated based on the provided \"" + cNumericFieldNames(25) + "\" value.");
            }

            // Crank case
            VRF(VRFNum).CCHeaterPower = rNumericArgs(30);
            VRF(VRFNum).NumCompressors = rNumericArgs(31);
            VRF(VRFNum).CompressorSizeRatio = rNumericArgs(32);
            VRF(VRFNum).MaxOATCCHeater = rNumericArgs(33);

            // Defrost
            if (!lAlphaFieldBlanks(8)) {
                if (UtilityRoutines::SameString(cAlphaArgs(8), "ReverseCycle")) VRF(VRFNum).DefrostStrategy = ReverseCycle;
                if (UtilityRoutines::SameString(cAlphaArgs(8), "Resistive")) VRF(VRFNum).DefrostStrategy = Resistive;
                if (VRF(VRFNum).DefrostStrategy == 0) {
                    ShowSevereError(cCurrentModuleObject + ", \"" + VRF(VRFNum).Name + "\" " + cAlphaFieldNames(8) + " not found: " + cAlphaArgs(8));
                    ErrorsFound = true;
                }
            } else {
                VRF(VRFNum).DefrostStrategy = ReverseCycle;
            }

            if (!lAlphaFieldBlanks(9)) {
                if (UtilityRoutines::SameString(cAlphaArgs(9), "Timed")) VRF(VRFNum).DefrostControl = Timed;
                if (UtilityRoutines::SameString(cAlphaArgs(9), "OnDemand")) VRF(VRFNum).DefrostControl = OnDemand;
                if (VRF(VRFNum).DefrostControl == 0) {
                    ShowSevereError(cCurrentModuleObject + ", \"" + VRF(VRFNum).Name + "\" " + cAlphaFieldNames(9) + " not found: " + cAlphaArgs(9));
                    ErrorsFound = true;
                }
            } else {
                VRF(VRFNum).DefrostControl = Timed;
            }

            if (!lAlphaFieldBlanks(10)) {
                VRF(VRFNum).DefrostEIRPtr = GetCurveIndex(state, cAlphaArgs(10));
                if (VRF(VRFNum).DefrostEIRPtr > 0) {
                    // Verify Curve Object, expected type is BiQuadratic
                    ErrorsFound |= CurveManager::CheckCurveDims(state, VRF(VRFNum).DefrostEIRPtr, // Curve index
                                                                {2},                       // Valid dimensions
                                                                RoutineName,               // Routine name
                                                                cCurrentModuleObject,      // Object Type
                                                                VRF(VRFNum).Name,          // Object Name
                                                                cAlphaFieldNames(10));     // Field Name
                } else {
                    if (VRF(VRFNum).DefrostStrategy == ReverseCycle && VRF(VRFNum).DefrostControl == OnDemand) {
                        ShowSevereError(cCurrentModuleObject + ", \"" + VRF(VRFNum).Name + "\" " + cAlphaFieldNames(10) +
                                        " not found:" + cAlphaArgs(10));
                        ErrorsFound = true;
                    }
                }
            } else {
                if (VRF(VRFNum).DefrostStrategy == ReverseCycle && VRF(VRFNum).DefrostControl == OnDemand) {
                    ShowSevereError(cCurrentModuleObject + ", \"" + VRF(VRFNum).Name + "\" " + cAlphaFieldNames(10) + " not found:" + cAlphaArgs(10));
                    ErrorsFound = true;
                }
            }

            VRF(VRFNum).DefrostFraction = rNumericArgs(34);
            VRF(VRFNum).DefrostCapacity = rNumericArgs(35);
            VRF(VRFNum).MaxOATDefrost = rNumericArgs(36);
            if (VRF(VRFNum).DefrostCapacity == 0.0 && VRF(VRFNum).DefrostStrategy == Resistive) {
                ShowWarningError(cCurrentModuleObject + ", \"" + VRF(VRFNum).Name + "\" " + cNumericFieldNames(35) +
                                 " = 0.0 for defrost strategy = RESISTIVE.");
            }

            // HR mode transition
            VRF(VRFNum).HRInitialCoolCapFrac = rNumericArgs(37);
            VRF(VRFNum).HRCoolCapTC = rNumericArgs(38);
            VRF(VRFNum).HRInitialCoolEIRFrac = rNumericArgs(39);
            VRF(VRFNum).HRCoolEIRTC = rNumericArgs(40);
            VRF(VRFNum).HRInitialHeatCapFrac = rNumericArgs(41);
            VRF(VRFNum).HRHeatCapTC = rNumericArgs(42);
            VRF(VRFNum).HRInitialHeatEIRFrac = rNumericArgs(43);
            VRF(VRFNum).HRHeatEIRTC = rNumericArgs(44);

            // Compressor configuration
            VRF(VRFNum).CompMaxDeltaP = rNumericArgs(45);
            VRF(VRFNum).EffCompInverter = rNumericArgs(46);
            VRF(VRFNum).CoffEvapCap = rNumericArgs(47);

            // The new VRF model is Air cooled
            VRF(VRFNum).CondenserType = DataHVACGlobals::AirCooled;
            VRF(VRFNum).CondenserNodeNum = 0;

            // Evaporative Capacity & Compressor Power Curves corresponding to each Loading Index / compressor speed
            NumOfCompSpd = rNumericArgs(48);
            VRF(VRFNum).CompressorSpeed.dimension(NumOfCompSpd);
            VRF(VRFNum).OUCoolingCAPFT.dimension(NumOfCompSpd);
            VRF(VRFNum).OUCoolingPWRFT.dimension(NumOfCompSpd);
            int Count1Index = 48; // the index of the last numeric field before compressor speed entries
            int Count2Index = 9;  // the index of the last alpha field before capacity/power curves
            for (NumCompSpd = 1; NumCompSpd <= NumOfCompSpd; NumCompSpd++) {
                VRF(VRFNum).CompressorSpeed(NumCompSpd) = rNumericArgs(Count1Index + NumCompSpd);

                // Evaporating Capacity Curve
                if (!lAlphaFieldBlanks(Count2Index + 2 * NumCompSpd)) {
                    int indexOUEvapCapCurve = GetCurveIndex(state, cAlphaArgs(Count2Index + 2 * NumCompSpd)); // convert curve name to index number
                    if (indexOUEvapCapCurve == 0) {                                                    // Verify curve name and type
                        if (lAlphaFieldBlanks(Count2Index + 2 * NumCompSpd)) {
                            ShowSevereError(RoutineName + cCurrentModuleObject + "=\"" + VRF(VRFNum).Name + "\", missing");
                            ShowContinueError("...required " + cAlphaFieldNames(Count2Index + 2 * NumCompSpd) + " is blank.");
                        } else {
                            ShowSevereError(RoutineName + cCurrentModuleObject + "=\"" + VRF(VRFNum).Name + "\", invalid");
                            ShowContinueError("...not found " + cAlphaFieldNames(Count2Index + 2 * NumCompSpd) + "=\"" +
                                              cAlphaArgs(Count2Index + 2 * NumCompSpd) + "\".");
                        }
                        ErrorsFound = true;
                    } else {
                        ErrorsFound |= CurveManager::CheckCurveDims(state, indexOUEvapCapCurve,                             // Curve index
                                                                    {2},                                             // Valid dimensions
                                                                    RoutineName,                                     // Routine name
                                                                    cCurrentModuleObject,                            // Object Type
                                                                    VRF(VRFNum).Name,                                // Object Name
                                                                    cAlphaFieldNames(Count2Index + 2 * NumCompSpd)); // Field Name

                        if (!ErrorsFound) {
                            VRF(VRFNum).OUCoolingCAPFT(NumCompSpd) = indexOUEvapCapCurve;
                        }
                    }
                }

                // Compressor Power Curve
                if (!lAlphaFieldBlanks(Count2Index + 2 * NumCompSpd + 1)) {
                    int indexOUCompPwrCurve = GetCurveIndex(state, cAlphaArgs(Count2Index + 2 * NumCompSpd + 1)); // convert curve name to index number
                    if (indexOUCompPwrCurve == 0) {                                                        // Verify curve name and type
                        if (lAlphaFieldBlanks(Count2Index + 2 * NumCompSpd + 1)) {
                            ShowSevereError(RoutineName + cCurrentModuleObject + "=\"" + VRF(VRFNum).Name + "\", missing");
                            ShowContinueError("...required " + cAlphaFieldNames(Count2Index + 2 * NumCompSpd + 1) + " is blank.");
                        } else {
                            ShowSevereError(RoutineName + cCurrentModuleObject + "=\"" + VRF(VRFNum).Name + "\", invalid");
                            ShowContinueError("...not found " + cAlphaFieldNames(Count2Index + 2 * NumCompSpd + 1) + "=\"" +
                                              cAlphaArgs(Count2Index + 2 * NumCompSpd + 1) + "\".");
                        }
                        ErrorsFound = true;
                    } else {
                        ErrorsFound |= CurveManager::CheckCurveDims(state, indexOUCompPwrCurve,                                 // Curve index
                                                                    {2},                                                 // Valid dimensions
                                                                    RoutineName,                                         // Routine name
                                                                    cCurrentModuleObject,                                // Object Type
                                                                    VRF(VRFNum).Name,                                    // Object Name
                                                                    cAlphaFieldNames(Count2Index + 2 * NumCompSpd + 1)); // Field Name

                        if (!ErrorsFound) {
                            VRF(VRFNum).OUCoolingPWRFT(NumCompSpd) = indexOUCompPwrCurve;
                        }
                    }
                }
            }
        }

        cCurrentModuleObject = "ZoneHVAC:TerminalUnit:VariableRefrigerantFlow";
        for (VRFNum = 1; VRFNum <= NumVRFTU; ++VRFNum) {
            VRFTUNum = VRFNum;

            //     initialize local node number variables
            FanInletNodeNum = 0;
            FanOutletNodeNum = 0;
            CCoilInletNodeNum = 0;
            CCoilOutletNodeNum = 0;
            HCoilInletNodeNum = 0;
            HCoilOutletNodeNum = 0;
            OANodeNums = 0;

            inputProcessor->getObjectItem(state,
                                          cCurrentModuleObject,
                                          VRFTUNum,
                                          cAlphaArgs,
                                          NumAlphas,
                                          rNumericArgs,
                                          NumNums,
                                          IOStat,
                                          lNumericFieldBlanks,
                                          lAlphaFieldBlanks,
                                          cAlphaFieldNames,
                                          cNumericFieldNames);

            VRFTUNumericFields(VRFTUNum).FieldNames.allocate(NumNums);
            VRFTUNumericFields(VRFTUNum).FieldNames = cNumericFieldNames;
            UtilityRoutines::IsNameEmpty(cAlphaArgs(1), cCurrentModuleObject, ErrorsFound);

            VRFTU(VRFTUNum).Name = cAlphaArgs(1);
            ZoneTerminalUnitListNum = 0;
            for (NumList = 1; NumList <= NumVRFTULists; ++NumList) {
                ZoneTerminalUnitListNum = UtilityRoutines::FindItemInList(
                    VRFTU(VRFTUNum).Name, TerminalUnitList(NumList).ZoneTUName, TerminalUnitList(NumList).NumTUInList);
                if (ZoneTerminalUnitListNum > 0) {
                    VRFTU(VRFTUNum).IndexToTUInTUList = ZoneTerminalUnitListNum;
                    TerminalUnitList(NumList).ZoneTUPtr(ZoneTerminalUnitListNum) = VRFTUNum;
                    VRFTU(VRFTUNum).TUListIndex = NumList;
                    break;
                }
            }
            if (VRFTU(VRFTUNum).TUListIndex == 0) {
                ShowSevereError(cCurrentModuleObject + " = " + VRFTU(VRFTUNum).Name);
                ShowContinueError("Terminal unit not found on any ZoneTerminalUnitList.");
                ErrorsFound = true;
            }

            for (NumCond = 1; NumCond <= NumVRFCond; ++NumCond) {
                if (VRF(NumCond).ZoneTUListPtr != VRFTU(VRFTUNum).TUListIndex) continue;
                VRFTU(VRFTUNum).VRFSysNum = NumCond;
                break;
            }
            VRFTU(VRFTUNum).VRFTUType_Num = DataHVACGlobals::VRFTUType_ConstVolume;
            if (lAlphaFieldBlanks(2)) {
                VRFTU(VRFTUNum).SchedPtr = ScheduleAlwaysOn;
            } else {
                VRFTU(VRFTUNum).SchedPtr = GetScheduleIndex(state, cAlphaArgs(2));
                if (VRFTU(VRFTUNum).SchedPtr == 0) {
                    ShowSevereError(cCurrentModuleObject + "=\"" + VRFTU(VRFTUNum).Name + "\" invalid data");
                    ShowContinueError("Invalid-not found " + cAlphaFieldNames(2) + "=\"" + cAlphaArgs(2) + "\".");
                    ErrorsFound = true;
                }
            }

            VRFTU(VRFTUNum).VRFTUInletNodeNum = GetOnlySingleNode(state, cAlphaArgs(3),
                                                                  ErrorsFound,
                                                                  cCurrentModuleObject,
                                                                  VRFTU(VRFTUNum).Name,
                                                                  DataLoopNode::NodeType_Air,
                                                                  DataLoopNode::NodeConnectionType_Inlet,
                                                                  1,
                                                                  ObjectIsParent);

            VRFTU(VRFTUNum).VRFTUOutletNodeNum = GetOnlySingleNode(state, cAlphaArgs(4),
                                                                   ErrorsFound,
                                                                   cCurrentModuleObject,
                                                                   VRFTU(VRFTUNum).Name,
                                                                   DataLoopNode::NodeType_Air,
                                                                   DataLoopNode::NodeConnectionType_Outlet,
                                                                   1,
                                                                   ObjectIsParent);

            VRFTU(VRFTUNum).MaxCoolAirVolFlow = rNumericArgs(1);
            VRFTU(VRFTUNum).MaxNoCoolAirVolFlow = rNumericArgs(2);
            VRFTU(VRFTUNum).MaxHeatAirVolFlow = rNumericArgs(3);
            VRFTU(VRFTUNum).MaxNoHeatAirVolFlow = rNumericArgs(4);
            VRFTU(VRFTUNum).CoolOutAirVolFlow = rNumericArgs(5);
            VRFTU(VRFTUNum).HeatOutAirVolFlow = rNumericArgs(6);
            VRFTU(VRFTUNum).NoCoolHeatOutAirVolFlow = rNumericArgs(7);

            VRFTU(VRFTUNum).FanOpModeSchedPtr = GetScheduleIndex(state, cAlphaArgs(5));
            // default to constant fan operating mode
            if (VRFTU(VRFTUNum).FanOpModeSchedPtr == 0) {
                if (!lAlphaFieldBlanks(5)) {
                    ShowSevereError(cCurrentModuleObject + " = " + VRFTU(VRFTUNum).Name);
                    ShowContinueError("..." + cAlphaFieldNames(5) + " = " + cAlphaArgs(5) + " not found.");
                    ShowContinueError("...Defaulting to constant fan operating mode and simulation continues.");
                }
                VRFTU(VRFTUNum).OpMode = DataHVACGlobals::ContFanCycCoil;
            }

            if (UtilityRoutines::SameString(cAlphaArgs(6), "BlowThrough")) VRFTU(VRFTUNum).FanPlace = DataHVACGlobals::BlowThru;
            if (UtilityRoutines::SameString(cAlphaArgs(6), "DrawThrough")) VRFTU(VRFTUNum).FanPlace = DataHVACGlobals::DrawThru;

            if (!lAlphaFieldBlanks(7) && !lAlphaFieldBlanks(8)) {
                // Get fan data
                FanType = cAlphaArgs(7);
                FanName = cAlphaArgs(8);
                if (UtilityRoutines::SameString(FanType, "Fan:SystemModel")) {
                    if (!HVACFan::checkIfFanNameIsAFanSystem(state, FanName)) {
                        ErrorsFound = true;
                    } else {
                        VRFTU(VRFTUNum).fanType_Num = DataHVACGlobals::FanType_SystemModelObject;
                    }
                } else {
                    errFlag = false;
                    GetFanType(state, FanName, VRFTU(VRFTUNum).fanType_Num, errFlag, cCurrentModuleObject);
                    if (errFlag) {
                        ShowContinueError("...occurs in " + cCurrentModuleObject + " = " + VRFTU(VRFTUNum).Name);
                        ErrorsFound = true;
                    }
                }

                // Check the type of the fan is correct
                if (!UtilityRoutines::SameString(DataHVACGlobals::cFanTypes(VRFTU(VRFTUNum).fanType_Num), FanType)) {
                    ShowSevereError(cCurrentModuleObject + " = " + VRFTU(VRFTUNum).Name);
                    ShowContinueError("Fan type specified = " + cAlphaArgs(7));
                    ShowContinueError("Based on the fan name the type of fan actually used = " +
                                      DataHVACGlobals::cFanTypes(VRFTU(VRFTUNum).fanType_Num));
                    ErrorsFound = true;
                }

                if (VRFTU(VRFTUNum).VRFSysNum > 0) {
                    // VRFTU Supply Air Fan Object Type must be Fan:VariableVolume if VRF Algorithm Type is AlgorithmTypeFluidTCtrl
                    if (VRF(VRFTU(VRFTUNum).VRFSysNum).VRFAlgorithmTypeNum == AlgorithmTypeFluidTCtrl &&
                        !(VRFTU(VRFTUNum).fanType_Num == DataHVACGlobals::FanType_SimpleVAV ||
                          VRFTU(VRFTUNum).fanType_Num == DataHVACGlobals::FanType_SystemModelObject)) {
                        ShowSevereError(cCurrentModuleObject + " = " + VRFTU(VRFTUNum).Name);
                        ShowContinueError("Fan type specified = " + cAlphaArgs(7));
                        ShowContinueError(
                            "Fan Object Type must be Fan:VariableVolume if VRF AirConditioner:VariableRefrigerantFlow:FluidTemperatureControl");
                        ShowContinueError("is used to model VRF outdoor unit.");
                        ErrorsFound = true;
                    }
                    // VRFTU Supply Air Fan Object Type must be Fan:OnOff or Fan:ConstantVolume if VRF Algorithm Type is AlgorithmTypeSysCurve
                    if (VRF(VRFTU(VRFTUNum).VRFSysNum).VRFAlgorithmTypeNum == AlgorithmTypeSysCurve &&
                        !(VRFTU(VRFTUNum).fanType_Num == DataHVACGlobals::FanType_SimpleOnOff ||
                          VRFTU(VRFTUNum).fanType_Num == DataHVACGlobals::FanType_SimpleConstVolume ||
                          VRFTU(VRFTUNum).fanType_Num == DataHVACGlobals::FanType_SystemModelObject)) {
                        ShowSevereError(cCurrentModuleObject + " = " + VRFTU(VRFTUNum).Name);
                        ShowContinueError("Fan type specified = " + cAlphaArgs(7));
                        ShowContinueError("Fan Object Type must be Fan:SystemModel, Fan:OnOff, or Fan:ConstantVolume if VRF "
                                          "AirConditioner:VariableRefrigerantFlow");
                        ShowContinueError("is used to model VRF outdoor unit.");
                        ErrorsFound = true;
                    }
                }

                if (VRFTU(VRFTUNum).fanType_Num == DataHVACGlobals::FanType_SimpleOnOff ||
                    VRFTU(VRFTUNum).fanType_Num == DataHVACGlobals::FanType_SimpleConstVolume ||
                    VRFTU(VRFTUNum).fanType_Num == DataHVACGlobals::FanType_SimpleVAV) {

                    ValidateComponent(state, DataHVACGlobals::cFanTypes(VRFTU(VRFTUNum).fanType_Num), FanName, IsNotOK, cCurrentModuleObject);
                    if (IsNotOK) {
                        ShowContinueError("...occurs in " + cCurrentModuleObject + " = " + VRFTU(VRFTUNum).Name);
                        ErrorsFound = true;

                    } else { // mine data from fan object

                        // Get the fan index
                        errFlag = false;
                        GetFanIndex(state, FanName, VRFTU(VRFTUNum).FanIndex, errFlag, ObjexxFCL::Optional_string_const());
                        if (errFlag) {
                            ShowContinueError("...occurs in " + cCurrentModuleObject + " = " + VRFTU(VRFTUNum).Name);
                            ErrorsFound = true;
                        } else {
                            VRFTU(VRFTUNum).fanInletNode = Fans::Fan(VRFTU(VRFTUNum).FanIndex).InletNodeNum;
                            VRFTU(VRFTUNum).fanOutletNode = Fans::Fan(VRFTU(VRFTUNum).FanIndex).OutletNodeNum;
                        }

                        // Set the Design Fan Volume Flow Rate
                        errFlag = false;
                        FanVolFlowRate = GetFanDesignVolumeFlowRate(state, FanType, FanName, errFlag);
                        VRFTU(VRFTUNum).ActualFanVolFlowRate = FanVolFlowRate;

                        if (errFlag) {
                            ShowContinueError("...occurs in " + cCurrentModuleObject + " =" + VRFTU(VRFTUNum).Name);
                            ErrorsFound = true;
                        }

                        // Get the Fan Inlet node
                        errFlag = false;
                        FanInletNodeNum = GetFanInletNode(state, FanType, FanName, errFlag);
                        if (errFlag) {
                            ShowContinueError("...occurs in " + cCurrentModuleObject + " = " + VRFTU(VRFTUNum).Name);
                            ErrorsFound = true;
                        }

                        // Get the Fan Outlet node
                        errFlag = false;
                        FanOutletNodeNum = GetFanOutletNode(state, FanType, FanName, errFlag);
                        if (errFlag) {
                            ShowContinueError("...occurs in " + cCurrentModuleObject + " = " + VRFTU(VRFTUNum).Name);
                            ErrorsFound = true;
                        }

                        // Get the fan's availability schedule
                        errFlag = false;
                        VRFTU(VRFTUNum).FanAvailSchedPtr = GetFanAvailSchPtr(state, FanType, FanName, errFlag);
                        if (errFlag) {
                            ShowContinueError("...occurs in " + cCurrentModuleObject + " = " + VRFTU(VRFTUNum).Name);
                            ErrorsFound = true;
                        }

                        // Check fan's schedule for cycling fan operation if constant volume fan is used
                        if (VRFTU(VRFTUNum).FanOpModeSchedPtr > 0 && VRFTU(VRFTUNum).fanType_Num == DataHVACGlobals::FanType_SimpleConstVolume) {
                            if (!CheckScheduleValueMinMax(VRFTU(VRFTUNum).FanOpModeSchedPtr, ">", 0.0, "<=", 1.0)) {
                                ShowSevereError(cCurrentModuleObject + " = " + VRFTU(VRFTUNum).Name);
                                ShowContinueError("For fan type = " + DataHVACGlobals::cFanTypes(DataHVACGlobals::FanType_SimpleConstVolume));
                                ShowContinueError("Fan operating mode must be continuous (fan operating mode schedule values > 0).");
                                ShowContinueError("Error found in " + cAlphaFieldNames(5) + " = " + cAlphaArgs(5));
                                ShowContinueError("...schedule values must be (>0., <=1.)");
                                ErrorsFound = true;
                            }
                        }
                    } // IF (IsNotOK) THEN

                } else if (VRFTU(VRFTUNum).fanType_Num == DataHVACGlobals::FanType_SystemModelObject) {

                    ValidateComponent(state, DataHVACGlobals::cFanTypes(VRFTU(VRFTUNum).fanType_Num), FanName, IsNotOK, cCurrentModuleObject);
                    if (IsNotOK) {
                        ShowContinueError("...occurs in " + cCurrentModuleObject + " = " + VRFTU(VRFTUNum).Name);
                        ErrorsFound = true;

                    } else {                                                                   // mine data from fan object
                        HVACFan::fanObjs.emplace_back(new HVACFan::FanSystem(state, FanName)); // call constructor
                        VRFTU(VRFTUNum).FanIndex = HVACFan::getFanObjectVectorIndex(FanName);
                        VRFTU(VRFTUNum).ActualFanVolFlowRate = HVACFan::fanObjs[VRFTU(VRFTUNum).FanIndex]->designAirVolFlowRate;
                        FanInletNodeNum = HVACFan::fanObjs[VRFTU(VRFTUNum).FanIndex]->inletNodeNum;
                        FanOutletNodeNum = HVACFan::fanObjs[VRFTU(VRFTUNum).FanIndex]->outletNodeNum;
                        VRFTU(VRFTUNum).FanAvailSchedPtr = HVACFan::fanObjs[VRFTU(VRFTUNum).FanIndex]->availSchedIndex;
                        VRFTU(VRFTUNum).fanInletNode = FanInletNodeNum;
                        VRFTU(VRFTUNum).fanOutletNode = FanOutletNodeNum;
                    }
                } else { // IF (FanType_Num == DataHVACGlobals::FanType_SimpleOnOff .OR. FanType_Num ==
                         // DataHVACGlobals::FanType_SimpleConstVolume)THEN
                    ShowSevereError(cCurrentModuleObject + " = " + VRFTU(VRFTUNum).Name);
                    ShowContinueError("Illegal " + cAlphaFieldNames(7) + " = " + cAlphaArgs(7));
                    ErrorsFound = true;
                } // IF (FanType_Num == DataHVACGlobals::FanType_SimpleOnOff .OR. FanType_Num == DataHVACGlobals::FanType_SimpleConstVolume)THEN

                // Add TU to component sets array
                SetUpCompSets(cCurrentModuleObject,
                              VRFTU(VRFTUNum).Name,
                              DataHVACGlobals::cFanTypes(VRFTU(VRFTUNum).fanType_Num),
                              FanName,
                              DataLoopNode::NodeID(FanInletNodeNum),
                              DataLoopNode::NodeID(FanOutletNodeNum));

            } else if (lAlphaFieldBlanks(7) || lAlphaFieldBlanks(8)) {
                VRFTU(VRFTUNum).FanPlace = 0; // reset fan placement when fan is not used so as not to call the fan
            } else {
                VRFTU(VRFTUNum).FanPlace = 0;
            }

            // Get OA mixer data
            OAMixerType = cAlphaArgs(9);

            if (!lAlphaFieldBlanks(10)) {
                VRFTU(VRFTUNum).OAMixerName = cAlphaArgs(10);
                errFlag = false;
                OANodeNums = GetOAMixerNodeNumbers(state, VRFTU(VRFTUNum).OAMixerName, errFlag);

                //       OANodeNums(1) = OAMixer(OAMixerNum)%InletNode
                //       OANodeNums(2) = OAMixer(OAMixerNum)%RelNode
                //       OANodeNums(3) = OAMixer(OAMixerNum)%RetNode
                //       OANodeNums(4) = OAMixer(OAMixerNum)%MixNode

                if (errFlag) {
                    ShowContinueError("Occurs in " + cCurrentModuleObject + " = " + VRFTU(VRFTUNum).Name);
                    ErrorsFound = true;
                } else {
                    VRFTU(VRFTUNum).OAMixerUsed = true;
                }
                VRFTU(VRFTUNum).VRFTUOAMixerOANodeNum = OANodeNums(1);
                VRFTU(VRFTUNum).VRFTUOAMixerRelNodeNum = OANodeNums(2);
                VRFTU(VRFTUNum).VRFTUOAMixerRetNodeNum = OANodeNums(3);
            }

            // Get DX cooling coil data
            DXCoolingCoilType = cAlphaArgs(11);

            errFlag = false;
            VRFTU(VRFTUNum).DXCoolCoilType_Num = GetCoilTypeNum(state, DXCoolingCoilType, cAlphaArgs(12), errFlag, false);
            if (VRFTU(VRFTUNum).DXCoolCoilType_Num == 0) {
                VRFTU(VRFTUNum).CoolingCoilPresent = false;
                if (VRFTU(VRFTUNum).TUListIndex > 0 && VRFTU(VRFTUNum).IndexToTUInTUList > 0) {
                    TerminalUnitList(VRFTU(VRFTUNum).TUListIndex).CoolingCoilPresent(VRFTU(VRFTUNum).IndexToTUInTUList) = false;
                }
            } else {
                if (VRFTU(VRFTUNum).VRFSysNum > 0) {
                    if (VRF(VRFTU(VRFTUNum).VRFSysNum).VRFAlgorithmTypeNum == AlgorithmTypeFluidTCtrl) {
                        // Algorithm Type: VRF model based on physics, applicable for Fluid Temperature Control

                        if (UtilityRoutines::SameString(DataHVACGlobals::cAllCoilTypes(VRFTU(VRFTUNum).DXCoolCoilType_Num),
                                                        DataHVACGlobals::cAllCoilTypes(DataHVACGlobals::CoilVRF_FluidTCtrl_Cooling))) {
                            errFlag = false;
                            if (VRFTU(VRFTUNum).TUListIndex > 0 && VRFTU(VRFTUNum).IndexToTUInTUList > 0) {
                                TerminalUnitList(VRFTU(VRFTUNum).TUListIndex).CoolingCoilAvailSchPtr(VRFTU(VRFTUNum).IndexToTUInTUList) =
                                    GetDXCoilAvailSchPtr(state, DXCoolingCoilType, cAlphaArgs(12), errFlag);
                            }
                            GetDXCoilIndex(state,
                                           cAlphaArgs(12),
                                           VRFTU(VRFTUNum).CoolCoilIndex,
                                           errFlag,
                                           DataHVACGlobals::cAllCoilTypes(DataHVACGlobals::CoilVRF_FluidTCtrl_Cooling),
                                           ObjexxFCL::Optional_bool_const());
                            CCoilInletNodeNum = GetDXCoilInletNode(
                                state, DataHVACGlobals::cAllCoilTypes(DataHVACGlobals::CoilVRF_FluidTCtrl_Cooling), cAlphaArgs(12), errFlag);
                            CCoilOutletNodeNum = GetDXCoilOutletNode(
                                state, DataHVACGlobals::cAllCoilTypes(DataHVACGlobals::CoilVRF_FluidTCtrl_Cooling), cAlphaArgs(12), errFlag);
                            VRFTU(VRFTUNum).coolCoilAirInNode = CCoilInletNodeNum;
                            VRFTU(VRFTUNum).coolCoilAirOutNode = CCoilOutletNodeNum;

                            if (errFlag) ShowContinueError("...occurs in " + cCurrentModuleObject + " \"" + VRFTU(VRFTUNum).Name + "\"");

                            if (VRFTU(VRFTUNum).VRFSysNum > 0) {
                                SetDXCoolingCoilData(
                                    state, VRFTU(VRFTUNum).CoolCoilIndex, ErrorsFound, _, VRF(VRFTU(VRFTUNum).VRFSysNum).CondenserType);
                                SetDXCoolingCoilData(
                                    state, VRFTU(VRFTUNum).CoolCoilIndex, ErrorsFound, _, _, VRF(VRFTU(VRFTUNum).VRFSysNum).CondenserNodeNum);
                                SetDXCoolingCoilData(
                                    state, VRFTU(VRFTUNum).CoolCoilIndex, ErrorsFound, _, _, _, VRF(VRFTU(VRFTUNum).VRFSysNum).MaxOATCCHeater);
                                SetDXCoolingCoilData(
                                    state, VRFTU(VRFTUNum).CoolCoilIndex, ErrorsFound, _, _, _, _, VRF(VRFTU(VRFTUNum).VRFSysNum).MinOATCooling);
                                SetDXCoolingCoilData(
                                    state, VRFTU(VRFTUNum).CoolCoilIndex, ErrorsFound, _, _, _, _, _, VRF(VRFTU(VRFTUNum).VRFSysNum).MaxOATCooling);

                                DXCoils::DXCoil(VRFTU(VRFTUNum).CoolCoilIndex).VRFIUPtr = VRFTUNum;
                                DXCoils::DXCoil(VRFTU(VRFTUNum).CoolCoilIndex).VRFOUPtr = VRFTU(VRFTUNum).VRFSysNum;
                                DXCoils::DXCoil(VRFTU(VRFTUNum).CoolCoilIndex).SupplyFanIndex = VRFTU(VRFTUNum).FanIndex;
                                if (VRFTU(VRFTUNum).fanType_Num == DataHVACGlobals::FanType_SystemModelObject) {
                                    if (VRFTU(VRFTUNum).FanIndex > -1) {
                                        DXCoils::DXCoil(VRFTU(VRFTUNum).CoolCoilIndex).RatedAirVolFlowRate(1) =
                                            HVACFan::fanObjs[VRFTU(VRFTUNum).FanIndex]->designAirVolFlowRate;
                                    } else {
                                        DXCoils::DXCoil(VRFTU(VRFTUNum).CoolCoilIndex).RatedAirVolFlowRate(1) = AutoSize;
                                    }
                                } else {
                                    if (VRFTU(VRFTUNum).FanIndex > 0) {
                                        DXCoils::DXCoil(VRFTU(VRFTUNum).CoolCoilIndex).RatedAirVolFlowRate(1) =
                                            EnergyPlus::Fans::Fan(VRFTU(VRFTUNum).FanIndex).MaxAirFlowRate;
                                    } else {
                                        DXCoils::DXCoil(VRFTU(VRFTUNum).CoolCoilIndex).RatedAirVolFlowRate(1) = AutoSize;
                                    }
                                }

                            } else {
                                ShowSevereError(cCurrentModuleObject + " \"" + VRFTU(VRFTUNum).Name + "\"");
                                ShowContinueError("... when checking " + DataHVACGlobals::cAllCoilTypes(VRFTU(VRFTUNum).DXCoolCoilType_Num) + " \"" +
                                                  cAlphaArgs(12) + "\"");
                                ShowContinueError("... terminal unit not connected to condenser.");
                                ShowContinueError("... check that terminal unit is specified in a terminal unit list object.");
                                ShowContinueError("... also check that the terminal unit list name is specified in an "
                                                  "AirConditioner:VariableRefrigerantFlow object.");
                                ErrorsFound = true;
                            }
                        } else {
                            ShowSevereError(cCurrentModuleObject + " \"" + VRFTU(VRFTUNum).Name + "\"");
                            ShowContinueError("... illegal " + cAlphaFieldNames(12) + " = " + cAlphaArgs(12));
                            ErrorsFound = true;
                        }

                    } else {
                        // Algorithm Type: VRF model based on system curve

                        if (UtilityRoutines::SameString(DataHVACGlobals::cAllCoilTypes(VRFTU(VRFTUNum).DXCoolCoilType_Num),
                                                        DataHVACGlobals::cAllCoilTypes(DataHVACGlobals::CoilVRF_Cooling))) {
                            if (VRFTU(VRFTUNum).TUListIndex > 0 && VRFTU(VRFTUNum).IndexToTUInTUList > 0) {
                                TerminalUnitList(VRFTU(VRFTUNum).TUListIndex).CoolingCoilAvailSchPtr(VRFTU(VRFTUNum).IndexToTUInTUList) =
                                    GetDXCoilAvailSchPtr(state, DXCoolingCoilType, cAlphaArgs(12), errFlag);
                            } else {
                                VRFTU(VRFTUNum).CoolingCoilPresent = false;
                            }
                            errFlag = false;
                            GetDXCoilIndex(state,
                                           cAlphaArgs(12),
                                           VRFTU(VRFTUNum).CoolCoilIndex,
                                           errFlag,
                                           DataHVACGlobals::cAllCoilTypes(DataHVACGlobals::CoilVRF_Cooling),
                                           ObjexxFCL::Optional_bool_const());
                            CCoilInletNodeNum =
                                GetDXCoilInletNode(state, DataHVACGlobals::cAllCoilTypes(DataHVACGlobals::CoilVRF_Cooling), cAlphaArgs(12), errFlag);
                            CCoilOutletNodeNum =
                                GetDXCoilOutletNode(state, DataHVACGlobals::cAllCoilTypes(DataHVACGlobals::CoilVRF_Cooling), cAlphaArgs(12), errFlag);
                            VRFTU(VRFTUNum).coolCoilAirInNode = CCoilInletNodeNum;
                            VRFTU(VRFTUNum).coolCoilAirOutNode = CCoilOutletNodeNum;

                            if (errFlag) ShowContinueError("...occurs in " + cCurrentModuleObject + " \"" + VRFTU(VRFTUNum).Name + "\"");

                            SetDXCoolingCoilData(state, VRFTU(VRFTUNum).CoolCoilIndex, ErrorsFound, _, VRF(VRFTU(VRFTUNum).VRFSysNum).CondenserType);
                            SetDXCoolingCoilData(
                                state, VRFTU(VRFTUNum).CoolCoilIndex, ErrorsFound, _, _, VRF(VRFTU(VRFTUNum).VRFSysNum).CondenserNodeNum);
                            SetDXCoolingCoilData(
                                state, VRFTU(VRFTUNum).CoolCoilIndex, ErrorsFound, _, _, _, VRF(VRFTU(VRFTUNum).VRFSysNum).MaxOATCCHeater);
                            SetDXCoolingCoilData(
                                state, VRFTU(VRFTUNum).CoolCoilIndex, ErrorsFound, _, _, _, _, VRF(VRFTU(VRFTUNum).VRFSysNum).MinOATCooling);
                            SetDXCoolingCoilData(
                                state, VRFTU(VRFTUNum).CoolCoilIndex, ErrorsFound, _, _, _, _, _, VRF(VRFTU(VRFTUNum).VRFSysNum).MaxOATCooling);

                        } else {
                            ShowSevereError(cCurrentModuleObject + " \"" + VRFTU(VRFTUNum).Name + "\"");
                            ShowContinueError("... illegal " + cAlphaFieldNames(12) + " = " + cAlphaArgs(12));
                            ErrorsFound = true;
                        }
                    }
                } else {
                    ShowSevereError(cCurrentModuleObject + " \"" + VRFTU(VRFTUNum).Name + "\"");
                    ShowContinueError("... when checking " + DataHVACGlobals::cAllCoilTypes(VRFTU(VRFTUNum).DXCoolCoilType_Num) + " \"" +
                                      cAlphaArgs(12) + "\"");
                    ShowContinueError("... terminal unit not connected to condenser.");
                    ShowContinueError("... check that terminal unit is specified in a terminal unit list object.");
                    ShowContinueError(
                        "... also check that the terminal unit list name is specified in an AirConditioner:VariableRefrigerantFlow object.");
                    ErrorsFound = true;
                }
            }

            // Get DX heating coil data
            DXHeatingCoilType = cAlphaArgs(13);

            // Get the heating to cooling sizing ratio input before writing to DX heating coil data
            if (!lNumericFieldBlanks(10)) {
                VRFTU(VRFTUNum).HeatingCapacitySizeRatio = rNumericArgs(10);
            }

            errFlag = false;
            VRFTU(VRFTUNum).DXHeatCoilType_Num = GetCoilTypeNum(state, DXHeatingCoilType, cAlphaArgs(14), errFlag, false);
            if (VRFTU(VRFTUNum).DXHeatCoilType_Num == 0) {
                VRFTU(VRFTUNum).HeatingCoilPresent = false;
                if (VRFTU(VRFTUNum).TUListIndex > 0 && VRFTU(VRFTUNum).IndexToTUInTUList > 0) {
                    TerminalUnitList(VRFTU(VRFTUNum).TUListIndex).HeatingCoilPresent(VRFTU(VRFTUNum).IndexToTUInTUList) = false;
                }
            } else {
                if (VRFTU(VRFTUNum).VRFSysNum > 0) {
                    if (VRF(VRFTU(VRFTUNum).VRFSysNum).VRFAlgorithmTypeNum == AlgorithmTypeFluidTCtrl) {
                        // Algorithm Type: VRF model based on physics, applicable for Fluid Temperature Control

                        if (UtilityRoutines::SameString(DataHVACGlobals::cAllCoilTypes(VRFTU(VRFTUNum).DXHeatCoilType_Num),
                                                        DataHVACGlobals::cAllCoilTypes(DataHVACGlobals::CoilVRF_FluidTCtrl_Heating))) {
                            errFlag = false;
                            if (VRFTU(VRFTUNum).TUListIndex > 0 && VRFTU(VRFTUNum).IndexToTUInTUList > 0) {
                                TerminalUnitList(VRFTU(VRFTUNum).TUListIndex).HeatingCoilAvailSchPtr(VRFTU(VRFTUNum).IndexToTUInTUList) =
                                    GetDXCoilAvailSchPtr(state, DXHeatingCoilType, cAlphaArgs(14), errFlag);
                            }
                            GetDXCoilIndex(state,
                                           cAlphaArgs(14),
                                           VRFTU(VRFTUNum).HeatCoilIndex,
                                           errFlag,
                                           DataHVACGlobals::cAllCoilTypes(DataHVACGlobals::CoilVRF_FluidTCtrl_Heating),
                                           ObjexxFCL::Optional_bool_const());
                            HCoilInletNodeNum = GetDXCoilInletNode(
                                state, DataHVACGlobals::cAllCoilTypes(DataHVACGlobals::CoilVRF_FluidTCtrl_Heating), cAlphaArgs(14), errFlag);
                            HCoilOutletNodeNum = GetDXCoilOutletNode(
                                state, DataHVACGlobals::cAllCoilTypes(DataHVACGlobals::CoilVRF_FluidTCtrl_Heating), cAlphaArgs(14), errFlag);
                            VRFTU(VRFTUNum).heatCoilAirInNode = HCoilInletNodeNum;
                            VRFTU(VRFTUNum).heatCoilAirOutNode = HCoilOutletNodeNum;

                            if (errFlag) ShowContinueError("...occurs in " + cCurrentModuleObject + " \"" + VRFTU(VRFTUNum).Name + "\"");

                            if (VRFTU(VRFTUNum).VRFSysNum > 0) {
                                SetDXCoolingCoilData(
                                    state, VRFTU(VRFTUNum).HeatCoilIndex, ErrorsFound, _, VRF(VRFTU(VRFTUNum).VRFSysNum).CondenserType);
                                SetDXCoolingCoilData(
                                    state, VRFTU(VRFTUNum).HeatCoilIndex, ErrorsFound, _, _, VRF(VRFTU(VRFTUNum).VRFSysNum).CondenserNodeNum);
                                SetDXCoolingCoilData(
                                    state, VRFTU(VRFTUNum).HeatCoilIndex, ErrorsFound, _, _, _, VRF(VRFTU(VRFTUNum).VRFSysNum).MaxOATCCHeater);
                                SetDXCoolingCoilData(state,
                                                     VRFTU(VRFTUNum).HeatCoilIndex,
                                                     ErrorsFound,
                                                     _,
                                                     _,
                                                     _,
                                                     _,
                                                     _,
                                                     _,
                                                     VRF(VRFTU(VRFTUNum).VRFSysNum).MinOATHeating);
                                SetDXCoolingCoilData(state,
                                                     VRFTU(VRFTUNum).HeatCoilIndex,
                                                     ErrorsFound,
                                                     _,
                                                     _,
                                                     _,
                                                     _,
                                                     _,
                                                     _,
                                                     _,
                                                     VRF(VRFTU(VRFTUNum).VRFSysNum).MaxOATHeating);
                                SetDXCoolingCoilData(state,
                                                     VRFTU(VRFTUNum).HeatCoilIndex,
                                                     ErrorsFound,
                                                     _,
                                                     _,
                                                     _,
                                                     _,
                                                     _,
                                                     _,
                                                     _,
                                                     _,
                                                     VRF(VRFTU(VRFTUNum).VRFSysNum).HeatingPerformanceOATType);
                                // Set defrost controls in child object to trip child object defrost calculations
                                SetDXCoolingCoilData(state,
                                                     VRFTU(VRFTUNum).HeatCoilIndex,
                                                     ErrorsFound,
                                                     _,
                                                     _,
                                                     _,
                                                     _,
                                                     _,
                                                     _,
                                                     _,
                                                     _,
                                                     _,
                                                     VRF(VRFTU(VRFTUNum).VRFSysNum).DefrostStrategy);
                                SetDXCoolingCoilData(state,
                                                     VRFTU(VRFTUNum).HeatCoilIndex,
                                                     ErrorsFound,
                                                     _,
                                                     _,
                                                     _,
                                                     _,
                                                     _,
                                                     _,
                                                     _,
                                                     _,
                                                     _,
                                                     _,
                                                     VRF(VRFTU(VRFTUNum).VRFSysNum).DefrostControl);
                                SetDXCoolingCoilData(state,
                                                     VRFTU(VRFTUNum).HeatCoilIndex,
                                                     ErrorsFound,
                                                     _,
                                                     _,
                                                     _,
                                                     _,
                                                     _,
                                                     _,
                                                     _,
                                                     _,
                                                     _,
                                                     _,
                                                     _,
                                                     VRF(VRFTU(VRFTUNum).VRFSysNum).DefrostEIRPtr);
                                SetDXCoolingCoilData(state,
                                                     VRFTU(VRFTUNum).HeatCoilIndex,
                                                     ErrorsFound,
                                                     _,
                                                     _,
                                                     _,
                                                     _,
                                                     _,
                                                     _,
                                                     _,
                                                     _,
                                                     _,
                                                     _,
                                                     _,
                                                     _,
                                                     VRF(VRFTU(VRFTUNum).VRFSysNum).DefrostFraction);
                                SetDXCoolingCoilData(state,
                                                     VRFTU(VRFTUNum).HeatCoilIndex,
                                                     ErrorsFound,
                                                     _,
                                                     _,
                                                     _,
                                                     _,
                                                     _,
                                                     _,
                                                     _,
                                                     _,
                                                     _,
                                                     _,
                                                     _,
                                                     _,
                                                     _,
                                                     _,
                                                     VRF(VRFTU(VRFTUNum).VRFSysNum).MaxOATDefrost);
                                // If defrost is disabled in the VRF condenser, it must be disabled in the DX coil
                                // Defrost primarily handled in parent object, set defrost capacity to 1 to avoid autosizing.
                                // Defrost capacity is used for nothing more than setting defrost power/consumption report
                                // variables which are not reported. The coil's defrost algorythm IS used to derate the coil
                                SetDXCoolingCoilData(state,
                                                     VRFTU(VRFTUNum).HeatCoilIndex,
                                                     ErrorsFound,
                                                     _,
                                                     _,
                                                     _,
                                                     _,
                                                     _,
                                                     _,
                                                     _,
                                                     _,
                                                     _,
                                                     _,
                                                     _,
                                                     _,
                                                     _,
                                                     1.0); // DefrostCapacity=1.0

                                DXCoils::DXCoil(VRFTU(VRFTUNum).HeatCoilIndex).VRFIUPtr = VRFTUNum;
                                DXCoils::DXCoil(VRFTU(VRFTUNum).HeatCoilIndex).VRFOUPtr = VRFTU(VRFTUNum).VRFSysNum;
                                DXCoils::DXCoil(VRFTU(VRFTUNum).HeatCoilIndex).SupplyFanIndex = VRFTU(VRFTUNum).FanIndex;
                                if (VRFTU(VRFTUNum).fanType_Num == DataHVACGlobals::FanType_SystemModelObject) {
                                    if (VRFTU(VRFTUNum).FanIndex > -1) {
                                        DXCoils::DXCoil(VRFTU(VRFTUNum).HeatCoilIndex).RatedAirVolFlowRate(1) =
                                            HVACFan::fanObjs[VRFTU(VRFTUNum).FanIndex]->designAirVolFlowRate;
                                    } else {
                                        DXCoils::DXCoil(VRFTU(VRFTUNum).HeatCoilIndex).RatedAirVolFlowRate(1) = AutoSize;
                                    }
                                } else {
                                    if (VRFTU(VRFTUNum).FanIndex > 0) {
                                        DXCoils::DXCoil(VRFTU(VRFTUNum).HeatCoilIndex).RatedAirVolFlowRate(1) =
                                            EnergyPlus::Fans::Fan(VRFTU(VRFTUNum).FanIndex).MaxAirFlowRate;
                                    } else {
                                        DXCoils::DXCoil(VRFTU(VRFTUNum).HeatCoilIndex).RatedAirVolFlowRate(1) = AutoSize;
                                    }
                                }

                                // Terminal unit heating to cooling sizing ratio has precedence over VRF system sizing ratio
                                if (VRFTU(VRFTUNum).HeatingCapacitySizeRatio > 1.0) {
                                    SetDXCoolingCoilData(state,
                                                         VRFTU(VRFTUNum).HeatCoilIndex,
                                                         ErrorsFound,
                                                         _,
                                                         _,
                                                         _,
                                                         _,
                                                         _,
                                                         _,
                                                         _,
                                                         _,
                                                         _,
                                                         _,
                                                         _,
                                                         _,
                                                         _,
                                                         _,
                                                         _,
                                                         _,
                                                         _,
                                                         VRFTU(VRFTUNum).HeatingCapacitySizeRatio);
                                } else if (VRF(VRFTU(VRFTUNum).VRFSysNum).HeatingCapacitySizeRatio > 1.0) {
                                    SetDXCoolingCoilData(state,
                                                         VRFTU(VRFTUNum).HeatCoilIndex,
                                                         ErrorsFound,
                                                         _,
                                                         _,
                                                         _,
                                                         _,
                                                         _,
                                                         _,
                                                         _,
                                                         _,
                                                         _,
                                                         _,
                                                         _,
                                                         _,
                                                         _,
                                                         _,
                                                         _,
                                                         _,
                                                         _,
                                                         VRF(VRFTU(VRFTUNum).VRFSysNum).HeatingCapacitySizeRatio);
                                }
                            } else {
                                ShowSevereError(cCurrentModuleObject + " \"" + VRFTU(VRFTUNum).Name + "\"");
                                ShowContinueError("... when checking " + DataHVACGlobals::cAllCoilTypes(VRFTU(VRFTUNum).DXHeatCoilType_Num) + " \"" +
                                                  cAlphaArgs(14) + "\"");
                                ShowContinueError("... terminal unit not connected to condenser.");
                                ShowContinueError("... check that terminal unit is specified in a terminal unit list object.");
                                ShowContinueError("... also check that the terminal unit list name is specified in an "
                                                  "AirConditioner:VariableRefrigerantFlow object.");
                                ErrorsFound = true;
                            }
                        } else {
                            ShowSevereError(cCurrentModuleObject + " \"" + VRFTU(VRFTUNum).Name + "\"");
                            ShowContinueError("... illegal " + cAlphaFieldNames(14) + " = " + cAlphaArgs(14));
                            ErrorsFound = true;
                        }

                    } else {
                        // Algorithm Type: VRF model based on system curve
                        if (UtilityRoutines::SameString(DataHVACGlobals::cAllCoilTypes(VRFTU(VRFTUNum).DXHeatCoilType_Num),
                                                        DataHVACGlobals::cAllCoilTypes(DataHVACGlobals::CoilVRF_Heating))) {
                            if (VRFTU(VRFTUNum).TUListIndex > 0 && VRFTU(VRFTUNum).IndexToTUInTUList > 0) {
                                TerminalUnitList(VRFTU(VRFTUNum).TUListIndex).HeatingCoilAvailSchPtr(VRFTU(VRFTUNum).IndexToTUInTUList) =
                                    GetDXCoilAvailSchPtr(state, DXHeatingCoilType, cAlphaArgs(14), errFlag);
                            } else {
                                VRFTU(VRFTUNum).HeatingCoilPresent = false;
                            }
                            errFlag = false;
                            GetDXCoilIndex(state,
                                           cAlphaArgs(14),
                                           VRFTU(VRFTUNum).HeatCoilIndex,
                                           errFlag,
                                           DataHVACGlobals::cAllCoilTypes(DataHVACGlobals::CoilVRF_Heating),
                                           ObjexxFCL::Optional_bool_const());
                            HCoilInletNodeNum =
                                GetDXCoilInletNode(state, DataHVACGlobals::cAllCoilTypes(DataHVACGlobals::CoilVRF_Heating), cAlphaArgs(14), errFlag);
                            HCoilOutletNodeNum =
                                GetDXCoilOutletNode(state, DataHVACGlobals::cAllCoilTypes(DataHVACGlobals::CoilVRF_Heating), cAlphaArgs(14), errFlag);
                            VRFTU(VRFTUNum).heatCoilAirInNode = HCoilInletNodeNum;
                            VRFTU(VRFTUNum).heatCoilAirOutNode = HCoilOutletNodeNum;

                            if (errFlag) ShowContinueError("...occurs in " + cCurrentModuleObject + " \"" + VRFTU(VRFTUNum).Name + "\"");

                            SetDXCoolingCoilData(state, VRFTU(VRFTUNum).HeatCoilIndex, ErrorsFound, _, VRF(VRFTU(VRFTUNum).VRFSysNum).CondenserType);
                            SetDXCoolingCoilData(
                                state, VRFTU(VRFTUNum).HeatCoilIndex, ErrorsFound, _, _, VRF(VRFTU(VRFTUNum).VRFSysNum).CondenserNodeNum);
                            SetDXCoolingCoilData(
                                state, VRFTU(VRFTUNum).HeatCoilIndex, ErrorsFound, _, _, _, VRF(VRFTU(VRFTUNum).VRFSysNum).MaxOATCCHeater);
                            SetDXCoolingCoilData(
                                state, VRFTU(VRFTUNum).HeatCoilIndex, ErrorsFound, _, _, _, _, _, _, VRF(VRFTU(VRFTUNum).VRFSysNum).MinOATHeating);
                            SetDXCoolingCoilData(state,
                                                 VRFTU(VRFTUNum).HeatCoilIndex,
                                                 ErrorsFound,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 VRF(VRFTU(VRFTUNum).VRFSysNum).HeatingPerformanceOATType);
                            // Set defrost controls in child object to trip child object defrost calculations
                            SetDXCoolingCoilData(state,
                                                 VRFTU(VRFTUNum).HeatCoilIndex,
                                                 ErrorsFound,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 VRF(VRFTU(VRFTUNum).VRFSysNum).DefrostStrategy);
                            SetDXCoolingCoilData(state,
                                                 VRFTU(VRFTUNum).HeatCoilIndex,
                                                 ErrorsFound,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 VRF(VRFTU(VRFTUNum).VRFSysNum).DefrostControl);
                            SetDXCoolingCoilData(state,
                                                 VRFTU(VRFTUNum).HeatCoilIndex,
                                                 ErrorsFound,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 VRF(VRFTU(VRFTUNum).VRFSysNum).DefrostEIRPtr);
                            SetDXCoolingCoilData(state,
                                                 VRFTU(VRFTUNum).HeatCoilIndex,
                                                 ErrorsFound,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 VRF(VRFTU(VRFTUNum).VRFSysNum).DefrostFraction);
                            SetDXCoolingCoilData(state,
                                                 VRFTU(VRFTUNum).HeatCoilIndex,
                                                 ErrorsFound,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 VRF(VRFTU(VRFTUNum).VRFSysNum).MaxOATDefrost);
                            // If defrost is disabled in the VRF condenser, it must be disabled in the DX coil
                            // Defrost primarily handled in parent object, set defrost capacity to 1 to avoid autosizing.
                            // Defrost capacity is used for nothing more than setting defrost power/consumption report
                            // variables which are not reported. The coil's defrost algorythm IS used to derate the coil
                            SetDXCoolingCoilData(state,
                                                 VRFTU(VRFTUNum).HeatCoilIndex,
                                                 ErrorsFound,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 1.0); // DefrostCapacity=1.0
                            // Terminal unit heating to cooling sizing ratio has precedence over VRF system sizing ratio
                            if (VRFTU(VRFTUNum).HeatingCapacitySizeRatio > 1.0) {
                                SetDXCoolingCoilData(state,
                                                     VRFTU(VRFTUNum).HeatCoilIndex,
                                                     ErrorsFound,
                                                     _,
                                                     _,
                                                     _,
                                                     _,
                                                     _,
                                                     _,
                                                     _,
                                                     _,
                                                     _,
                                                     _,
                                                     _,
                                                     _,
                                                     _,
                                                     _,
                                                     _,
                                                     _,
                                                     _,
                                                     VRFTU(VRFTUNum).HeatingCapacitySizeRatio);
                            } else if (VRF(VRFTU(VRFTUNum).VRFSysNum).HeatingCapacitySizeRatio > 1.0) {
                                SetDXCoolingCoilData(state,
                                                     VRFTU(VRFTUNum).HeatCoilIndex,
                                                     ErrorsFound,
                                                     _,
                                                     _,
                                                     _,
                                                     _,
                                                     _,
                                                     _,
                                                     _,
                                                     _,
                                                     _,
                                                     _,
                                                     _,
                                                     _,
                                                     _,
                                                     _,
                                                     _,
                                                     _,
                                                     _,
                                                     VRF(VRFTU(VRFTUNum).VRFSysNum).HeatingCapacitySizeRatio);
                            }
                            // Check VRF DX heating coil heating capacity as a fuction of temperature performance curve. Only report here for
                            // biquadratic curve type.
                            if (VRFTU(VRFTUNum).VRFSysNum > 0 && VRFTU(VRFTUNum).HeatCoilIndex > 0 &&
                                state.dataCurveManager->PerfCurve(GetDXCoilCapFTCurveIndex(state, VRFTU(VRFTUNum).HeatCoilIndex, ErrorsFound)).NumDims == 2) {
                                if (VRF(VRFTU(VRFTUNum).VRFSysNum).HeatingPerformanceOATType == DataHVACGlobals::WetBulbIndicator) {
                                    checkCurveIsNormalizedToOne(state,
                                        "GetDXCoils: " + DataHVACGlobals::cAllCoilTypes(VRFTU(VRFTUNum).DXHeatCoilType_Num),
                                        DXCoils::GetDXCoilName(state,
                                                               VRFTU(VRFTUNum).HeatCoilIndex,
                                                               ErrorsFound,
                                                               DataHVACGlobals::cAllCoilTypes(VRFTU(VRFTUNum).DXHeatCoilType_Num),
                                                               ObjexxFCL::Optional_bool_const()),
                                        GetDXCoilCapFTCurveIndex(state, VRFTU(VRFTUNum).HeatCoilIndex, ErrorsFound),
                                        "Heating Capacity Ratio Modifier Function of Temperature Curve Name",
                                        CurveManager::GetCurveName(state, GetDXCoilCapFTCurveIndex(state, VRFTU(VRFTUNum).HeatCoilIndex, ErrorsFound)),
                                        RatedInletAirTempHeat,
                                        RatedOutdoorWetBulbTempHeat);
                                } else if (VRF(VRFTU(VRFTUNum).VRFSysNum).HeatingPerformanceOATType == DataHVACGlobals::DryBulbIndicator) {
                                    checkCurveIsNormalizedToOne(state,
                                        "GetDXCoils: " + DataHVACGlobals::cAllCoilTypes(VRFTU(VRFTUNum).DXHeatCoilType_Num),
                                        DXCoils::GetDXCoilName(state,
                                                               VRFTU(VRFTUNum).HeatCoilIndex,
                                                               ErrorsFound,
                                                               DataHVACGlobals::cAllCoilTypes(VRFTU(VRFTUNum).DXHeatCoilType_Num),
                                                               ObjexxFCL::Optional_bool_const()),
                                        GetDXCoilCapFTCurveIndex(state, VRFTU(VRFTUNum).HeatCoilIndex, ErrorsFound),
                                        "Heating Capacity Ratio Modifier Function of Temperature Curve Name",
                                        CurveManager::GetCurveName(state, GetDXCoilCapFTCurveIndex(state, VRFTU(VRFTUNum).HeatCoilIndex, ErrorsFound)),
                                        RatedInletAirTempHeat,
                                        RatedOutdoorAirTempHeat);
                                }
                            }

                        } else {
                            ShowSevereError(cCurrentModuleObject + " \"" + VRFTU(VRFTUNum).Name + "\"");
                            ShowContinueError("... illegal " + cAlphaFieldNames(14) + " = " + cAlphaArgs(14));
                            ErrorsFound = true;
                        }
                    }
                } else {
                    ShowSevereError(cCurrentModuleObject + " \"" + VRFTU(VRFTUNum).Name + "\"");
                    ShowContinueError("... when checking " + DataHVACGlobals::cAllCoilTypes(VRFTU(VRFTUNum).DXHeatCoilType_Num) + " \"" +
                                      cAlphaArgs(14) + "\"");
                    ShowContinueError("... terminal unit not connected to condenser.");
                    ShowContinueError("... check that terminal unit is specified in a terminal unit list object.");
                    ShowContinueError(
                        "... also check that the terminal unit list name is specified in an AirConditioner:VariableRefrigerantFlow object.");
                    ErrorsFound = true;
                }
            }

            if (!VRFTU(VRFTUNum).CoolingCoilPresent && VRFTU(VRFTUNum).DXCoolCoilType_Num == 0 && !VRFTU(VRFTUNum).HeatingCoilPresent &&
                VRFTU(VRFTUNum).DXHeatCoilType_Num == 0) {
                ShowSevereError(cCurrentModuleObject + " \"" + VRFTU(VRFTUNum).Name + "\"");
                ShowContinueError("... no valid coils entered for this terminal unit. Simulation will not proceed.");
                ErrorsFound = true;
            }

            if (!lAlphaFieldBlanks(15)) {
                VRFTU(VRFTUNum).AvailManagerListName = cAlphaArgs(15);
            }
            VRFTU(VRFTUNum).ParasiticElec = rNumericArgs(8);
            VRFTU(VRFTUNum).ParasiticOffElec = rNumericArgs(9);

            VRFTU(VRFTUNum).HVACSizingIndex = 0;
            if (!lAlphaFieldBlanks(16)) {
                VRFTU(VRFTUNum).HVACSizingIndex = UtilityRoutines::FindItemInList(cAlphaArgs(16), ZoneHVACSizing);
                if (VRFTU(VRFTUNum).HVACSizingIndex == 0) {
                    ShowSevereError(cAlphaFieldNames(16) + " = " + cAlphaArgs(16) + " not found.");
                    ShowContinueError("Occurs in " + cCurrentModuleObject + " = " + VRFTU(VRFTUNum).Name);
                    ErrorsFound = true;
                }
            }

            // supplemental heating coil
            if (!lAlphaFieldBlanks(17) && !lAlphaFieldBlanks(18)) {

                SuppHeatingCoilType = cAlphaArgs(17);
                SuppHeatingCoilName = cAlphaArgs(18);

                VRFTU(VRFTUNum).SuppHeatCoilName = SuppHeatingCoilName;

                errFlag = false;
                if (UtilityRoutines::SameString(SuppHeatingCoilType, "Coil:Heating:Water")) {
                    VRFTU(VRFTUNum).SuppHeatCoilType_Num = DataHVACGlobals::Coil_HeatingWater;
                } else if (UtilityRoutines::SameString(SuppHeatingCoilType, "Coil:Heating:Steam")) {
                    VRFTU(VRFTUNum).SuppHeatCoilType_Num = DataHVACGlobals::Coil_HeatingSteam;
                } else if (UtilityRoutines::SameString(SuppHeatingCoilType, "Coil:Heating:Fuel") ||
                           UtilityRoutines::SameString(SuppHeatingCoilType, "Coil:Heating:Electric")) {
                    VRFTU(VRFTUNum).SuppHeatCoilType_Num =
                        HeatingCoils::GetHeatingCoilTypeNum(state, SuppHeatingCoilType, SuppHeatingCoilName, errFlag);
                }

                VRFTU(VRFTUNum).SuppHeatCoilType = SuppHeatingCoilType;
                VRFTU(VRFTUNum).SuppHeatingCoilPresent = true;

                if (VRFTU(VRFTUNum).SuppHeatCoilType_Num == DataHVACGlobals::Coil_HeatingGasOrOtherFuel ||
                    VRFTU(VRFTUNum).SuppHeatCoilType_Num == DataHVACGlobals::Coil_HeatingElectric) {
                    errFlag = false;
                    VRFTU(VRFTUNum).SuppHeatCoilType_Num =
                        HeatingCoils::GetHeatingCoilTypeNum(state, SuppHeatingCoilType, SuppHeatingCoilName, errFlag);
                    if (errFlag) {
                        ShowContinueError("Occurs in " + cCurrentModuleObject + " = " + VRFTU(VRFTUNum).Name);
                        ErrorsFound = true;
                    } else {
                        ValidateComponent(state, SuppHeatingCoilType, SuppHeatingCoilName, IsNotOK, cCurrentModuleObject);
                        if (IsNotOK) {
                            ShowContinueError("Occurs in " + cCurrentModuleObject + " = " + VRFTU(VRFTUNum).Name);
                            ErrorsFound = true;
                        } else { // mine data from supplemental heating coil
                            // Get the supplemental heating coil index
                            VRFTU(VRFTUNum).SuppHeatCoilIndex =
                                HeatingCoils::GetHeatingCoilIndex(state, SuppHeatingCoilType, SuppHeatingCoilName, IsNotOK);
                            if (IsNotOK) {
                                ShowContinueError("Occurs in " + cCurrentModuleObject + " = " + VRFTU(VRFTUNum).Name);
                                ErrorsFound = true;
                            }
                            // Get the design supplemental heating capacity
                            errFlag = false;
                            VRFTU(VRFTUNum).DesignSuppHeatingCapacity =
                                HeatingCoils::GetCoilCapacity(state, SuppHeatingCoilType, SuppHeatingCoilName, errFlag);
                            if (errFlag) {
                                ShowContinueError("Occurs in " + cCurrentModuleObject + " = " + VRFTU(VRFTUNum).Name);
                                ErrorsFound = true;
                            }
                            // Get the supplemental heating Coil air inlet node
                            errFlag = false;
                            SuppHeatCoilAirInletNode = HeatingCoils::GetCoilInletNode(state, SuppHeatingCoilType, SuppHeatingCoilName, errFlag);
                            if (errFlag) {
                                ShowContinueError("Occurs in " + cCurrentModuleObject + " = " + VRFTU(VRFTUNum).Name);
                                ErrorsFound = true;
                            }
                            // Get the supplemental heating Coil air outlet node
                            errFlag = false;
                            SuppHeatCoilAirOutletNode = HeatingCoils::GetCoilOutletNode(state, SuppHeatingCoilType, SuppHeatingCoilName, errFlag);
                            if (errFlag) {
                                ShowContinueError("Occurs in " + cCurrentModuleObject + " = " + VRFTU(VRFTUNum).Name);
                                ErrorsFound = true;
                            }
                        } // IF (IsNotOK) THEN
                    }

                    VRFTU(VRFTUNum).SuppHeatCoilAirInletNode = SuppHeatCoilAirInletNode;
                    VRFTU(VRFTUNum).SuppHeatCoilAirOutletNode = SuppHeatCoilAirOutletNode;

                } else if (VRFTU(VRFTUNum).SuppHeatCoilType_Num == DataHVACGlobals::Coil_HeatingWater) {

                    ValidateComponent(state, SuppHeatingCoilType, SuppHeatingCoilName, IsNotOK, cCurrentModuleObject);
                    if (IsNotOK) {
                        ShowContinueError("Occurs in " + cCurrentModuleObject + " = " + VRFTU(VRFTUNum).Name);
                        ErrorsFound = true;
                    } else { // mine data from heating coil object

                        // Get the supplemental heating coil water Inlet or control node number
                        errFlag = false;
                        VRFTU(VRFTUNum).SuppHeatCoilFluidInletNode =
                            WaterCoils::GetCoilWaterInletNode(state, "Coil:Heating:Water", SuppHeatingCoilName, errFlag);
                        if (errFlag) {
                            ShowContinueError("Occurs in " + cCurrentModuleObject + " = " + VRFTU(VRFTUNum).Name);
                            ErrorsFound = true;
                        }
                        // Get the supplemental heating coil hot water max volume flow rate
                        errFlag = false;
                        VRFTU(VRFTUNum).SuppHeatCoilFluidMaxFlow =
                            WaterCoils::GetCoilMaxWaterFlowRate(state, "Coil:Heating:Water", SuppHeatingCoilName, errFlag);
                        if (errFlag) {
                            ShowContinueError("Occurs in " + cCurrentModuleObject + " = " + VRFTU(VRFTUNum).Name);
                            ErrorsFound = true;
                        }
                        // Get the supplemental heating Coil air inlet node
                        errFlag = false;
                        SuppHeatCoilAirInletNode = WaterCoils::GetCoilInletNode(state, "Coil:Heating:Water", SuppHeatingCoilName, errFlag);
                        VRFTU(VRFTUNum).SuppHeatCoilAirInletNode = SuppHeatCoilAirInletNode;
                        if (errFlag) {
                            ShowContinueError("Occurs in " + cCurrentModuleObject + " = " + VRFTU(VRFTUNum).Name);
                            ErrorsFound = true;
                        }
                        // Get the supplemental heating coil air outlet node
                        errFlag = false;
                        SuppHeatCoilAirOutletNode = WaterCoils::GetCoilOutletNode(state, "Coil:Heating:Water", SuppHeatingCoilName, errFlag);
                        VRFTU(VRFTUNum).SuppHeatCoilAirOutletNode = SuppHeatCoilAirOutletNode;
                        if (errFlag) {
                            ShowContinueError("Occurs in " + cCurrentModuleObject + " = " + VRFTU(VRFTUNum).Name);
                            ErrorsFound = true;
                        }
                    }

                } else if (VRFTU(VRFTUNum).SuppHeatCoilType_Num == DataHVACGlobals::Coil_HeatingSteam) {

                    ValidateComponent(state, SuppHeatingCoilType, SuppHeatingCoilName, IsNotOK, cCurrentModuleObject);
                    if (IsNotOK) {
                        ShowContinueError("Occurs in " + cCurrentModuleObject + " = " + VRFTU(VRFTUNum).Name);
                        ErrorsFound = true;
                    } else { // mine data from supplemental heating coil object
                        errFlag = false;
                        VRFTU(VRFTUNum).SuppHeatCoilIndex = SteamCoils::GetSteamCoilIndex(state, "COIL:HEATING:STEAM", SuppHeatingCoilName, errFlag);
                        if (VRFTU(VRFTUNum).SuppHeatCoilIndex == 0) {
                            ShowSevereError(cCurrentModuleObject + " = " + VRFTU(VRFTUNum).Name);
                            ErrorsFound = true;
                        }
                        // Get the supplemental heating Coil steam inlet node number
                        errFlag = false;
                        VRFTU(VRFTUNum).SuppHeatCoilFluidInletNode =
                            SteamCoils::GetCoilSteamInletNode(state, "Coil:Heating:Steam", SuppHeatingCoilName, errFlag);
                        if (errFlag) {
                            ShowContinueError("Occurs in " + cCurrentModuleObject + " = " + VRFTU(VRFTUNum).Name);
                            ErrorsFound = true;
                        }
                        // Get the supplemental heating coil steam max volume flow rate
                        VRFTU(VRFTUNum).SuppHeatCoilFluidMaxFlow = SteamCoils::GetCoilMaxSteamFlowRate(state, VRFTU(VRFTUNum).SuppHeatCoilIndex, errFlag);
                        if (VRFTU(VRFTUNum).SuppHeatCoilFluidMaxFlow > 0.0) {
                            int SteamIndex = 0; // fluid type index of 0 is passed if steam
                            Real64 TempSteamIn = 100.0;
                            Real64 SteamDensity = FluidProperties::GetSatDensityRefrig(state, fluidNameSteam, TempSteamIn, 1.0, SteamIndex, RoutineName);
                            VRFTU(VRFTUNum).SuppHeatCoilFluidMaxFlow =
                                SteamCoils::GetCoilMaxSteamFlowRate(state, VRFTU(VRFTUNum).SuppHeatCoilIndex, errFlag) * SteamDensity;
                        }
                        // Get the supplemental heating coil air inlet node
                        errFlag = false;
                        SuppHeatCoilAirInletNode = SteamCoils::GetCoilAirInletNode(state, VRFTU(VRFTUNum).SuppHeatCoilIndex, SuppHeatingCoilName, errFlag);
                        VRFTU(VRFTUNum).SuppHeatCoilAirInletNode = SuppHeatCoilAirInletNode;
                        if (errFlag) {
                            ShowContinueError("Occurs in " + cCurrentModuleObject + " = " + VRFTU(VRFTUNum).Name);
                            ErrorsFound = true;
                        }
                        // Get the supplemental heating coil air outlet node
                        errFlag = false;
                        SuppHeatCoilAirOutletNode = SteamCoils::GetCoilAirOutletNode(state, VRFTU(VRFTUNum).SuppHeatCoilIndex, SuppHeatingCoilName, errFlag);
                        VRFTU(VRFTUNum).SuppHeatCoilAirOutletNode = SuppHeatCoilAirOutletNode;
                        if (errFlag) {
                            ShowContinueError("Occurs in " + cCurrentModuleObject + " = " + VRFTU(VRFTUNum).Name);
                            ErrorsFound = true;
                        }
                    }
                }
            } else { // if (!lAlphaFieldBlanks(17) && !lAlphaFieldBlanks(18)) {
                if (!lAlphaFieldBlanks(17) && lAlphaFieldBlanks(18)) {
                    ShowWarningError(cCurrentModuleObject + " = " + VRFTU(VRFTUNum).Name + "\"");
                    ShowContinueError("...Supplemental heating coil type = " + cAlphaArgs(17));
                    ShowContinueError("...But missing the associated supplemental heating coil name. ");
                    ShowContinueError("...The supplemental heating coil will not be simulated. ");
                }
                if (lAlphaFieldBlanks(17) && !lAlphaFieldBlanks(18)) {
                    ShowWarningError(cCurrentModuleObject + " = " + VRFTU(VRFTUNum).Name + "\"");
                    ShowContinueError("...Supplemental heating coil name = " + cAlphaArgs(18));
                    ShowContinueError("...But missing the associated supplemental heating coil type. ");
                    ShowContinueError("...The supplemental heating coil will not be simulated. ");
                }
            }

            if (!lAlphaFieldBlanks(19)) {
                VRFTU(VRFTUNum).ZoneNum = UtilityRoutines::FindItemInList(cAlphaArgs(19), Zone);
                if (VRFTU(VRFTUNum).ZoneNum == 0) {
                    ShowSevereError(cCurrentModuleObject + " = " + cAlphaArgs(1));
                    ShowContinueError("Illegal " + cAlphaFieldNames(19) + " = " + cAlphaArgs(19));
                    ErrorsFound = true;
                }
            }

            // set supplemental heating coil operation temperature limits
            if (VRFTU(VRFTUNum).SuppHeatingCoilPresent) {
                // Set maximum supply air temperature for supplemental heating coil
                VRFTU(VRFTUNum).MaxSATFromSuppHeatCoil = rNumericArgs(11);
                // set maximum outdoor dry-bulb temperature for supplemental heating coil operation
                VRFTU(VRFTUNum).MaxOATSuppHeatingCoil = rNumericArgs(12);
            }

            // Add cooling coil to component sets array
            if (VRFTU(VRFTUNum).CoolingCoilPresent) {

                SetUpCompSets(cCurrentModuleObject,
                              VRFTU(VRFTUNum).Name,
                              DataHVACGlobals::cAllCoilTypes(VRFTU(VRFTUNum).DXCoolCoilType_Num),
                              cAlphaArgs(12),
                              DataLoopNode::NodeID(CCoilInletNodeNum),
                              DataLoopNode::NodeID(CCoilOutletNodeNum));
                //     set heating coil present flag
                SetDXCoolingCoilData(state,
                                     VRFTU(VRFTUNum).CoolCoilIndex,
                                     ErrorsFound,
                                     _,
                                     _,
                                     _,
                                     _,
                                     _,
                                     _,
                                     _,
                                     _,
                                     _,
                                     _,
                                     _,
                                     _,
                                     _,
                                     _,
                                     _,
                                     _,
                                     VRFTU(VRFTUNum).HeatingCoilPresent);

                //   check that curve types are present in VRF Condenser if cooling coil is present in terminal unit (can be blank)
                //   all curves are checked for correct type if a curve name is entered in the VRF condenser object. Check that the
                //   curve is present if the corresponding coil is entered in the terminal unit.
                if (VRFTU(VRFTUNum).VRFSysNum > 0) {

                    if (VRF(VRFTU(VRFTUNum).VRFSysNum).VRFAlgorithmTypeNum != AlgorithmTypeFluidTCtrl) {

                        if (VRF(VRFTU(VRFTUNum).VRFSysNum).CoolingCapacity <= 0 && VRF(VRFTU(VRFTUNum).VRFSysNum).CoolingCapacity != AutoSize) {
                            ShowSevereError(cCurrentModuleObject + " \"" + VRFTU(VRFTUNum).Name + "\"");
                            ShowContinueError("...This terminal unit contains a cooling coil and rated cooling capacity is also required in the "
                                              "associated condenser object.");
                            ShowContinueError("...Rated Cooling Capacity must also be specified for condenser = " +
                                              cVRFTypes(VRF(VRFTU(VRFTUNum).VRFSysNum).VRFSystemTypeNum) + " \"" +
                                              VRF(VRFTU(VRFTUNum).VRFSysNum).Name + "\".");
                            ErrorsFound = true;
                        }
                    }
                }
            }

            // Add heating coil to component sets array
            if (VRFTU(VRFTUNum).HeatingCoilPresent) {

                SetUpCompSets(cCurrentModuleObject,
                              VRFTU(VRFTUNum).Name,
                              DataHVACGlobals::cAllCoilTypes(VRFTU(VRFTUNum).DXHeatCoilType_Num),
                              cAlphaArgs(14),
                              DataLoopNode::NodeID(HCoilInletNodeNum),
                              DataLoopNode::NodeID(HCoilOutletNodeNum));
                //     set cooling coil present flag
                SetDXCoolingCoilData(state,
                                     VRFTU(VRFTUNum).HeatCoilIndex,
                                     ErrorsFound,
                                     _,
                                     _,
                                     _,
                                     _,
                                     _,
                                     _,
                                     _,
                                     _,
                                     _,
                                     _,
                                     _,
                                     _,
                                     _,
                                     _,
                                     _,
                                     VRFTU(VRFTUNum).CoolingCoilPresent);

                if (VRFTU(VRFTUNum).VRFSysNum > 0) {

                    if (VRF(VRFTU(VRFTUNum).VRFSysNum).VRFAlgorithmTypeNum != AlgorithmTypeFluidTCtrl) {

                        if (VRF(VRFTU(VRFTUNum).VRFSysNum).HeatingCapacity <= 0 && VRF(VRFTU(VRFTUNum).VRFSysNum).HeatingCapacity != AutoSize) {
                            ShowSevereError(cCurrentModuleObject + " \"" + VRFTU(VRFTUNum).Name + "\"");
                            ShowContinueError("...This terminal unit contains a heating coil and rated heating capacity is also required in the "
                                              "associated condenser object.");
                            ShowContinueError("...Rated Heating Capacity must also be specified for condenser = " +
                                              cVRFTypes(VRF(VRFTU(VRFTUNum).VRFSysNum).VRFSystemTypeNum) + " \"" +
                                              VRF(VRFTU(VRFTUNum).VRFSysNum).Name + "\".");
                            ErrorsFound = true;
                        }

                        if (VRF(VRFTU(VRFTUNum).VRFSysNum).HeatCapFT == 0) {
                            ShowSevereError(cCurrentModuleObject + " \"" + VRFTU(VRFTUNum).Name + "\"");
                            ShowContinueError("...This terminal unit contains a heating coil and heating performance curves are also required in the "
                                              "associated condenser object.");
                            ShowContinueError(
                                "...Heating Capacity Ratio Modifier Function of Low Temperature Curve must also be specified for condenser = " +
                                cVRFTypes(VRF(VRFTU(VRFTUNum).VRFSysNum).VRFSystemTypeNum) + " \"" + VRF(VRFTU(VRFTUNum).VRFSysNum).Name + "\".");
                            ErrorsFound = true;
                        }

                        if (VRF(VRFTU(VRFTUNum).VRFSysNum).HeatEIRFT == 0) {
                            ShowSevereError(cCurrentModuleObject + " \"" + VRFTU(VRFTUNum).Name + "\"");
                            ShowContinueError("...This terminal unit contains a heating coil and heating performance curves are also required in the "
                                              "associated condenser object.");
                            ShowContinueError(
                                "...Heating Energy Input Ratio Modifier Function of Low Temperature Curve must also be specified for condenser = " +
                                cVRFTypes(VRF(VRFTU(VRFTUNum).VRFSysNum).VRFSystemTypeNum) + " \"" + VRF(VRFTU(VRFTUNum).VRFSysNum).Name + "\".");
                            ErrorsFound = true;
                        }

                        if (VRF(VRFTU(VRFTUNum).VRFSysNum).HeatEIRFPLR1 == 0) {
                            ShowSevereError(cCurrentModuleObject + " \"" + VRFTU(VRFTUNum).Name + "\"");
                            ShowContinueError("...This terminal unit contains a heating coil and heating performance curves are also required in the "
                                              "associated condenser object.");
                            ShowContinueError("...Heating Energy Input Ratio Modifier Function of Low Part-Load Ratio Curve must also be specified "
                                              "for condenser = " +
                                              cVRFTypes(VRF(VRFTU(VRFTUNum).VRFSysNum).VRFSystemTypeNum) + " \"" +
                                              VRF(VRFTU(VRFTUNum).VRFSysNum).Name + "\".");
                        }
                    }
                }
            }

            // Add supplemental heating coil to component sets array
            if (VRFTU(VRFTUNum).SuppHeatingCoilPresent) {
                SetUpCompSets(cCurrentModuleObject,
                              VRFTU(VRFTUNum).Name,
                              DataHVACGlobals::cAllCoilTypes(VRFTU(VRFTUNum).SuppHeatCoilType_Num),
                              SuppHeatingCoilName,
                              DataLoopNode::NodeID(SuppHeatCoilAirInletNode),
                              DataLoopNode::NodeID(SuppHeatCoilAirOutletNode));
            }
            // Set up component set for OA mixer - use OA node and Mixed air node
            if (VRFTU(VRFTUNum).OAMixerUsed)
                SetUpCompSets(cCurrentModuleObject,
                              VRFTU(VRFTUNum).Name,
                              "UNDEFINED",
                              VRFTU(VRFTUNum).OAMixerName,
                              DataLoopNode::NodeID(OANodeNums(1)),
                              DataLoopNode::NodeID(OANodeNums(4)));

            // Get AirTerminal mixer data
            GetATMixer(state,
                       VRFTU(VRFTUNum).Name,
                       VRFTU(VRFTUNum).ATMixerName,
                       VRFTU(VRFTUNum).ATMixerIndex,
                       VRFTU(VRFTUNum).ATMixerType,
                       VRFTU(VRFTUNum).ATMixerPriNode,
                       VRFTU(VRFTUNum).ATMixerSecNode,
                       VRFTU(VRFTUNum).ATMixerOutNode,
                       VRFTU(VRFTUNum).VRFTUOutletNodeNum);
            if (VRFTU(VRFTUNum).ATMixerType == DataHVACGlobals::ATMixer_InletSide ||
                VRFTU(VRFTUNum).ATMixerType == DataHVACGlobals::ATMixer_SupplySide) {
                VRFTU(VRFTUNum).ATMixerExists = true;
            }
            // check that the VRF TU have local outside air and DOA
            if (VRFTU(VRFTUNum).ATMixerExists && OANodeNums(4) > 0) {
                ShowSevereError(cCurrentModuleObject + " = \"" + VRFTU(VRFTUNum).Name +
                                "\". VRF terminal unit has local as well as central outdoor air specified");
                ErrorsFound = true;
            }

            // for ZoneHVAC check that TU inlet node is a zone exhaust node otherwise ZoneAirNode and ZoneNum = 0
            if (!VRFTU(VRFTUNum).ATMixerExists || VRFTU(VRFTUNum).ATMixerType == DataHVACGlobals::ATMixer_SupplySide) {
                for (CtrlZone = 1; CtrlZone <= NumOfZones; ++CtrlZone) {
                    if (!ZoneEquipConfig(CtrlZone).IsControlled) continue;
                    for (NodeNum = 1; NodeNum <= ZoneEquipConfig(CtrlZone).NumExhaustNodes; ++NodeNum) {
                        if (VRFTU(VRFTUNum).VRFTUInletNodeNum == ZoneEquipConfig(CtrlZone).ExhaustNode(NodeNum)) {
                            VRFTU(VRFTUNum).ZoneAirNode = ZoneEquipConfig(CtrlZone).ZoneNode;
                            VRFTU(VRFTUNum).ZoneNum = CtrlZone;
                            break;
                        }
                    }
                }
            } else if (VRFTU(VRFTUNum).ATMixerType == DataHVACGlobals::ATMixer_InletSide) {
                for (CtrlZone = 1; CtrlZone <= NumOfZones; ++CtrlZone) {
                    if (!ZoneEquipConfig(CtrlZone).IsControlled) continue;
                    for (NodeNum = 1; NodeNum <= ZoneEquipConfig(CtrlZone).NumInletNodes; ++NodeNum) {
                        if (VRFTU(VRFTUNum).VRFTUOutletNodeNum == ZoneEquipConfig(CtrlZone).InletNode(NodeNum)) {
                            VRFTU(VRFTUNum).ZoneAirNode = ZoneEquipConfig(CtrlZone).ZoneNode;
                            VRFTU(VRFTUNum).ZoneNum = CtrlZone;
                            break;
                        }
                    }
                }
            }
        } // end Number of VRF Terminal Unit Loop

        //   perform additional error checking
        for (NumList = 1; NumList <= NumVRFTULists; ++NumList) {
            for (VRFNum = 1; VRFNum <= TerminalUnitList(NumList).NumTUInList; ++VRFNum) {
                if (TerminalUnitList(NumList).ZoneTUPtr(VRFNum) == 0) {
                    // TU name in zone terminal unit list not found
                    ShowSevereError("ZoneTerminalUnitList \"" + TerminalUnitList(NumList).Name + "\"");
                    ShowContinueError("...Zone Terminal Unit = " + TerminalUnitList(NumList).ZoneTUName(VRFNum) + " improperly connected to system.");
                    ShowContinueError("...either the ZoneHVAC:TerminalUnit:VariableRefrigerantFlow object does not exist,");
                    ShowContinueError("...the ZoneHVAC:TerminalUnit:VariableRefrigerantFlow object name is misspelled,");
                    ShowContinueError("...or the ZoneTerminalUnitList object is not named in an AirConditioner:VariableRefrigerantFlow object.");
                    ErrorsFound = true;
                }
                if (VRFTU(VRFNum).VRFSysNum > 0) {
                    if (TerminalUnitList(NumList).NumTUInList == 1 && VRF(VRFTU(VRFNum).VRFSysNum).VRFAlgorithmTypeNum == AlgorithmTypeSysCurve) {
                        if (VRF(VRFTU(VRFNum).VRFSysNum).HeatRecoveryUsed) {
                            ShowWarningError("ZoneTerminalUnitList \"" + TerminalUnitList(NumList).Name + "\"");
                            ShowWarningError("...Only 1 Terminal Unit connected to system and heat recovery is selected.");
                            ShowContinueError("...Heat recovery will be disabled.");
                            VRF(VRFNum).HeatRecoveryUsed = false;
                        }
                    }
                }
            }
        }

        //   warn when number of ZoneTerminalUnitList different from number of AirConditioner:VariableRefrigerantFlow
        if (NumVRFTULists != NumVRFCond) {
            ShowSevereError("The number of AirConditioner:VariableRefrigerantFlow objects (" + TrimSigDigits(NumVRFCond) +
                            ") does not match the number of ZoneTerminalUnitList objects (" + TrimSigDigits(NumVRFTULists) + ").");
            for (NumCond = 1; NumCond <= NumVRFCond; ++NumCond) {
                if (VRF(NumCond).ZoneTUListPtr > 0) {
                    ShowContinueError("...AirConditioner:VariableRefrigerantFlow = " + VRF(NumCond).Name +
                                      " specifies Zone Terminal Unit List Name = " + TerminalUnitList(VRF(NumCond).ZoneTUListPtr).Name);
                } else {
                    ShowContinueError("...AirConditioner:VariableRefrigerantFlow = " + VRF(NumCond).Name +
                                      " Zone Terminal Unit List Name not found.");
                }
            }
            ShowContinueError("...listing ZoneTerminalUnitList objects.");
            for (NumList = 1; NumList <= NumVRFTULists; ++NumList) {
                ShowContinueError("...ZoneTerminalUnitList = " + TerminalUnitList(NumList).Name);
            }
            ErrorsFound = true;
        }

        // Set up output variables
        for (VRFNum = 1; VRFNum <= NumVRFTU; ++VRFNum) {
            if (VRFTU(VRFNum).CoolingCoilPresent) {
                SetupOutputVariable(state, "Zone VRF Air Terminal Cooling Electricity Rate",
                                    OutputProcessor::Unit::W,
                                    VRFTU(VRFNum).ParasiticCoolElecPower,
                                    "System",
                                    "Average",
                                    VRFTU(VRFNum).Name);
                SetupOutputVariable(state, "Zone VRF Air Terminal Cooling Electricity Energy",
                                    OutputProcessor::Unit::J,
                                    VRFTU(VRFNum).ParasiticElecCoolConsumption,
                                    "System",
                                    "Sum",
                                    VRFTU(VRFNum).Name,
                                    _,
                                    "Electricity",
                                    "COOLING",
                                    _,
                                    "System");
                SetupOutputVariable(state, "Zone VRF Air Terminal Total Cooling Rate",
                                    OutputProcessor::Unit::W,
                                    VRFTU(VRFNum).TotalCoolingRate,
                                    "System",
                                    "Average",
                                    VRFTU(VRFNum).Name);
                SetupOutputVariable(state, "Zone VRF Air Terminal Sensible Cooling Rate",
                                    OutputProcessor::Unit::W,
                                    VRFTU(VRFNum).SensibleCoolingRate,
                                    "System",
                                    "Average",
                                    VRFTU(VRFNum).Name);
                SetupOutputVariable(state, "Zone VRF Air Terminal Latent Cooling Rate",
                                    OutputProcessor::Unit::W,
                                    VRFTU(VRFNum).LatentCoolingRate,
                                    "System",
                                    "Average",
                                    VRFTU(VRFNum).Name);
                SetupOutputVariable(state, "Zone VRF Air Terminal Total Cooling Energy",
                                    OutputProcessor::Unit::J,
                                    VRFTU(VRFNum).TotalCoolingEnergy,
                                    "System",
                                    "Sum",
                                    VRFTU(VRFNum).Name);
                SetupOutputVariable(state, "Zone VRF Air Terminal Sensible Cooling Energy",
                                    OutputProcessor::Unit::J,
                                    VRFTU(VRFNum).SensibleCoolingEnergy,
                                    "System",
                                    "Sum",
                                    VRFTU(VRFNum).Name);
                SetupOutputVariable(state, "Zone VRF Air Terminal Latent Cooling Energy",
                                    OutputProcessor::Unit::J,
                                    VRFTU(VRFNum).LatentCoolingEnergy,
                                    "System",
                                    "Sum",
                                    VRFTU(VRFNum).Name);
            }
            if (VRFTU(VRFNum).HeatingCoilPresent) {
                SetupOutputVariable(state, "Zone VRF Air Terminal Heating Electricity Rate",
                                    OutputProcessor::Unit::W,
                                    VRFTU(VRFNum).ParasiticHeatElecPower,
                                    "System",
                                    "Average",
                                    VRFTU(VRFNum).Name);
                SetupOutputVariable(state, "Zone VRF Air Terminal Heating Electricity Energy",
                                    OutputProcessor::Unit::J,
                                    VRFTU(VRFNum).ParasiticElecHeatConsumption,
                                    "System",
                                    "Sum",
                                    VRFTU(VRFNum).Name,
                                    _,
                                    "Electricity",
                                    "HEATING",
                                    _,
                                    "System");
                SetupOutputVariable(state, "Zone VRF Air Terminal Total Heating Rate",
                                    OutputProcessor::Unit::W,
                                    VRFTU(VRFNum).TotalHeatingRate,
                                    "System",
                                    "Average",
                                    VRFTU(VRFNum).Name);
                SetupOutputVariable(state, "Zone VRF Air Terminal Sensible Heating Rate",
                                    OutputProcessor::Unit::W,
                                    VRFTU(VRFNum).SensibleHeatingRate,
                                    "System",
                                    "Average",
                                    VRFTU(VRFNum).Name);
                SetupOutputVariable(state, "Zone VRF Air Terminal Latent Heating Rate",
                                    OutputProcessor::Unit::W,
                                    VRFTU(VRFNum).LatentHeatingRate,
                                    "System",
                                    "Average",
                                    VRFTU(VRFNum).Name);
                SetupOutputVariable(state, "Zone VRF Air Terminal Total Heating Energy",
                                    OutputProcessor::Unit::J,
                                    VRFTU(VRFNum).TotalHeatingEnergy,
                                    "System",
                                    "Sum",
                                    VRFTU(VRFNum).Name);
                SetupOutputVariable(state, "Zone VRF Air Terminal Sensible Heating Energy",
                                    OutputProcessor::Unit::J,
                                    VRFTU(VRFNum).SensibleHeatingEnergy,
                                    "System",
                                    "Sum",
                                    VRFTU(VRFNum).Name);
                SetupOutputVariable(state, "Zone VRF Air Terminal Latent Heating Energy",
                                    OutputProcessor::Unit::J,
                                    VRFTU(VRFNum).LatentHeatingEnergy,
                                    "System",
                                    "Sum",
                                    VRFTU(VRFNum).Name);
            }
            SetupOutputVariable(state, "Zone VRF Air Terminal Fan Availability Status",
                                OutputProcessor::Unit::None,
                                VRFTU(VRFNum).AvailStatus,
                                "System",
                                "Average",
                                VRFTU(VRFNum).Name);
            if (AnyEnergyManagementSystemInModel) {
                SetupEMSActuator("Variable Refrigerant Flow Terminal Unit",
                                 VRFTU(VRFNum).Name,
                                 "Part Load Ratio",
                                 "[fraction]",
                                 VRFTU(VRFNum).EMSOverridePartLoadFrac,
                                 VRFTU(VRFNum).EMSValueForPartLoadFrac);
            }
        }

        for (NumCond = 1; NumCond <= NumVRFCond; ++NumCond) {
            SetupOutputVariable(state, "VRF Heat Pump Total Cooling Rate",
                                OutputProcessor::Unit::W,
                                VRF(NumCond).TotalCoolingCapacity,
                                "System",
                                "Average",
                                VRF(NumCond).Name);
            SetupOutputVariable(state, "VRF Heat Pump Total Heating Rate",
                                OutputProcessor::Unit::W,
                                VRF(NumCond).TotalHeatingCapacity,
                                "System",
                                "Average",
                                VRF(NumCond).Name);
            SetupOutputVariable(state, "VRF Heat Pump Cooling " + VRF(NumCond).FuelType + " Rate",
                                OutputProcessor::Unit::W,
                                VRF(NumCond).ElecCoolingPower,
                                "System",
                                "Average",
                                VRF(NumCond).Name);
            SetupOutputVariable(state, "VRF Heat Pump Cooling " + VRF(NumCond).FuelType + " Energy",
                                OutputProcessor::Unit::J,
                                VRF(NumCond).CoolElecConsumption,
                                "System",
                                "Sum",
                                VRF(NumCond).Name,
                                _,
                                VRF(NumCond).FuelType,
                                "COOLING",
                                _,
                                "System");
            SetupOutputVariable(state, "VRF Heat Pump Heating " + VRF(NumCond).FuelType + " Rate",
                                OutputProcessor::Unit::W,
                                VRF(NumCond).ElecHeatingPower,
                                "System",
                                "Average",
                                VRF(NumCond).Name);
            SetupOutputVariable(state, "VRF Heat Pump Heating " + VRF(NumCond).FuelType + " Energy",
                                OutputProcessor::Unit::J,
                                VRF(NumCond).HeatElecConsumption,
                                "System",
                                "Sum",
                                VRF(NumCond).Name,
                                _,
                                VRF(NumCond).FuelType,
                                "HEATING",
                                _,
                                "System");

            SetupOutputVariable(state,
                "VRF Heat Pump Cooling COP", OutputProcessor::Unit::None, VRF(NumCond).OperatingCoolingCOP, "System", "Average", VRF(NumCond).Name);
            SetupOutputVariable(state,
                "VRF Heat Pump Heating COP", OutputProcessor::Unit::None, VRF(NumCond).OperatingHeatingCOP, "System", "Average", VRF(NumCond).Name);
            SetupOutputVariable(state, "VRF Heat Pump COP", OutputProcessor::Unit::None, VRF(NumCond).OperatingCOP, "System", "Average", VRF(NumCond).Name);

            if (VRF(NumCond).VRFAlgorithmTypeNum == AlgorithmTypeFluidTCtrl) {
                // For VRF_FluidTCtrl Model
                SetupOutputVariable(state, "VRF Heat Pump Compressor Electricity Rate",
                                    OutputProcessor::Unit::W,
                                    VRF(NumCond).Ncomp,
                                    "System",
                                    "Average",
                                    VRF(NumCond).Name);
                SetupOutputVariable(state, "VRF Heat Pump Outdoor Unit Fan Power",
                                    OutputProcessor::Unit::W,
                                    VRF(NumCond).OUFanPower,
                                    "System",
                                    "Average",
                                    VRF(NumCond).Name);
                SetupOutputVariable(state, "VRF Heat Pump Compressor Rotating Speed",
                                    OutputProcessor::Unit::rev_min,
                                    VRF(NumCond).CompActSpeed,
                                    "System",
                                    "Average",
                                    VRF(NumCond).Name);
                SetupOutputVariable(state, "VRF Heat Pump Indoor Unit Evaporating Temperature",
                                    OutputProcessor::Unit::C,
                                    VRF(NumCond).IUEvaporatingTemp,
                                    "System",
                                    "Average",
                                    VRF(NumCond).Name);
                SetupOutputVariable(state, "VRF Heat Pump Outdoor Unit Condensing Temperature",
                                    OutputProcessor::Unit::C,
                                    VRF(NumCond).CondensingTemp,
                                    "System",
                                    "Average",
                                    VRF(NumCond).Name);
                SetupOutputVariable(state, "VRF Heat Pump Indoor Unit Condensing Temperature",
                                    OutputProcessor::Unit::C,
                                    VRF(NumCond).IUCondensingTemp,
                                    "System",
                                    "Average",
                                    VRF(NumCond).Name);
                SetupOutputVariable(state, "VRF Heat Pump Outdoor Unit Evaporating Temperature",
                                    OutputProcessor::Unit::C,
                                    VRF(NumCond).EvaporatingTemp,
                                    "System",
                                    "Average",
                                    VRF(NumCond).Name);
                SetupOutputVariable(state, "VRF Heat Pump Cooling Capacity at Max Compressor Speed",
                                    OutputProcessor::Unit::W,
                                    VRF(NumCond).CoolingCapacity,
                                    "System",
                                    "Average",
                                    VRF(NumCond).Name);
                SetupOutputVariable(state, "VRF Heat Pump Heating Capacity at Max Compressor Speed",
                                    OutputProcessor::Unit::W,
                                    VRF(NumCond).HeatingCapacity,
                                    "System",
                                    "Average",
                                    VRF(NumCond).Name);
                SetupOutputVariable(state, "VRF Heat Pump Indoor Unit Piping Correction for Cooling",
                                    OutputProcessor::Unit::None,
                                    VRF(NumCond).PipingCorrectionCooling,
                                    "System",
                                    "Average",
                                    VRF(NumCond).Name);
                SetupOutputVariable(state, "VRF Heat Pump Indoor Unit Piping Correction for Heating",
                                    OutputProcessor::Unit::None,
                                    VRF(NumCond).PipingCorrectionHeating,
                                    "System",
                                    "Average",
                                    VRF(NumCond).Name);
                SetupOutputVariable(state, "VRF Heat Pump Outdoor Unit Evaporator Heat Extract Rate",
                                    OutputProcessor::Unit::W,
                                    VRF(NumCond).OUEvapHeatRate,
                                    "System",
                                    "Average",
                                    VRF(NumCond).Name);
                SetupOutputVariable(state, "VRF Heat Pump Outdoor Unit Condenser Heat Release Rate",
                                    OutputProcessor::Unit::W,
                                    VRF(NumCond).OUCondHeatRate,
                                    "System",
                                    "Average",
                                    VRF(NumCond).Name);

            } else {
                // For VRF_SysCurve Model
                SetupOutputVariable(state, "VRF Heat Pump Maximum Capacity Cooling Rate",
                                    OutputProcessor::Unit::W,
                                    MaxCoolingCapacity(NumCond),
                                    "System",
                                    "Average",
                                    VRF(NumCond).Name);
                SetupOutputVariable(state, "VRF Heat Pump Maximum Capacity Heating Rate",
                                    OutputProcessor::Unit::W,
                                    MaxHeatingCapacity(NumCond),
                                    "System",
                                    "Average",
                                    VRF(NumCond).Name);
            }

            if (VRF(NumCond).DefrostStrategy == Resistive ||
                (VRF(NumCond).DefrostStrategy == ReverseCycle && VRF(NumCond).FuelTypeNum == DataGlobalConstants::iRT_Electricity)) {
                SetupOutputVariable(state, "VRF Heat Pump Defrost Electricity Rate",
                                    OutputProcessor::Unit::W,
                                    VRF(NumCond).DefrostPower,
                                    "System",
                                    "Average",
                                    VRF(NumCond).Name);
                SetupOutputVariable(state, "VRF Heat Pump Defrost Electricity Energy",
                                    OutputProcessor::Unit::J,
                                    VRF(NumCond).DefrostConsumption,
                                    "System",
                                    "Sum",
                                    VRF(NumCond).Name,
                                    _,
                                    "Electricity",
                                    "HEATING",
                                    _,
                                    "System");
            } else { // defrost energy applied to fuel type
                SetupOutputVariable(state, "VRF Heat Pump Defrost " + VRF(NumCond).FuelType + " Rate",
                                    OutputProcessor::Unit::W,
                                    VRF(NumCond).DefrostPower,
                                    "System",
                                    "Average",
                                    VRF(NumCond).Name);
                SetupOutputVariable(state, "VRF Heat Pump Defrost " + VRF(NumCond).FuelType + " Energy",
                                    OutputProcessor::Unit::J,
                                    VRF(NumCond).DefrostConsumption,
                                    "System",
                                    "Sum",
                                    VRF(NumCond).Name,
                                    _,
                                    VRF(NumCond).FuelType,
                                    "HEATING",
                                    _,
                                    "System");
            }

            SetupOutputVariable(state,
                "VRF Heat Pump Part Load Ratio", OutputProcessor::Unit::None, VRF(NumCond).VRFCondPLR, "System", "Average", VRF(NumCond).Name);
            SetupOutputVariable(state,
                "VRF Heat Pump Runtime Fraction", OutputProcessor::Unit::None, VRF(NumCond).VRFCondRTF, "System", "Average", VRF(NumCond).Name);
            SetupOutputVariable(state,
                "VRF Heat Pump Cycling Ratio", OutputProcessor::Unit::None, VRF(NumCond).VRFCondCyclingRatio, "System", "Average", VRF(NumCond).Name);

            SetupOutputVariable(state,
                "VRF Heat Pump Operating Mode", OutputProcessor::Unit::None, VRF(NumCond).OperatingMode, "System", "Average", VRF(NumCond).Name);
            SetupOutputVariable(state, "VRF Heat Pump Condenser Inlet Temperature",
                                OutputProcessor::Unit::C,
                                VRF(NumCond).CondenserInletTemp,
                                "System",
                                "Average",
                                VRF(NumCond).Name);

            SetupOutputVariable(state, "VRF Heat Pump Crankcase Heater Electricity Rate",
                                OutputProcessor::Unit::W,
                                VRF(NumCond).CrankCaseHeaterPower,
                                "System",
                                "Average",
                                VRF(NumCond).Name);
            SetupOutputVariable(state, "VRF Heat Pump Crankcase Heater Electricity Energy",
                                OutputProcessor::Unit::J,
                                VRF(NumCond).CrankCaseHeaterElecConsumption,
                                "System",
                                "Sum",
                                VRF(NumCond).Name,
                                _,
                                "Electricity",
                                "COOLING",
                                _,
                                "System");
            SetupOutputVariable(state, "VRF Heat Pump Terminal Unit Cooling Load Rate",
                                OutputProcessor::Unit::W,
                                VRF(NumCond).TUCoolingLoad,
                                "System",
                                "Average",
                                VRF(NumCond).Name);
            SetupOutputVariable(state, "VRF Heat Pump Terminal Unit Heating Load Rate",
                                OutputProcessor::Unit::W,
                                VRF(NumCond).TUHeatingLoad,
                                "System",
                                "Average",
                                VRF(NumCond).Name);
            if (VRF(NumCond).HeatRecoveryUsed) {
                SetupOutputVariable(state, "VRF Heat Pump Heat Recovery Status Change Multiplier",
                                    OutputProcessor::Unit::None,
                                    VRF(NumCond).SUMultiplier,
                                    "System",
                                    "Average",
                                    VRF(NumCond).Name);
                SetupOutputVariable(state, "VRF Heat Pump Simultaneous Cooling and Heating Efficiency",
                                    OutputProcessor::Unit::Btu_h_W,
                                    VRF(NumCond).SCHE,
                                    "System",
                                    "Average",
                                    VRF(NumCond).Name);
                SetupOutputVariable(state,
                    "VRF Heat Pump Heat Recovery Rate", OutputProcessor::Unit::W, VRF(NumCond).VRFHeatRec, "System", "Average", VRF(NumCond).Name);
                SetupOutputVariable(state, "VRF Heat Pump Heat Recovery Energy",
                                    OutputProcessor::Unit::J,
                                    VRF(NumCond).VRFHeatEnergyRec,
                                    "System",
                                    "Sum",
                                    VRF(NumCond).Name,
                                    _,
                                    "ENERGYTRANSFER",
                                    "HEATRECOVERY",
                                    _,
                                    "Plant");
            }

            if (VRF(NumCond).CondenserType == DataHVACGlobals::EvapCooled) {
                SetupOutputVariable(state, "VRF Heat Pump Evaporative Condenser Water Use Volume",
                                    OutputProcessor::Unit::m3,
                                    VRF(NumCond).EvapWaterConsumpRate,
                                    "System",
                                    "Sum",
                                    VRF(NumCond).Name,
                                    _,
                                    "Water",
                                    "Cooling",
                                    _,
                                    "System");
                SetupOutputVariable(state, "VRF Heat Pump Evaporative Condenser Pump Electricity Rate",
                                    OutputProcessor::Unit::W,
                                    VRF(NumCond).EvapCondPumpElecPower,
                                    "System",
                                    "Average",
                                    VRF(NumCond).Name);
                SetupOutputVariable(state, "VRF Heat Pump Evaporative Condenser Pump Electricity Energy",
                                    OutputProcessor::Unit::J,
                                    VRF(NumCond).EvapCondPumpElecConsumption,
                                    "System",
                                    "Sum",
                                    VRF(NumCond).Name,
                                    _,
                                    "Electricity",
                                    "COOLING",
                                    _,
                                    "System");

                if (VRF(NumCond).BasinHeaterPowerFTempDiff > 0.0) {
                    SetupOutputVariable(state, "VRF Heat Pump Basin Heater Electricity Rate",
                                        OutputProcessor::Unit::W,
                                        VRF(NumCond).BasinHeaterPower,
                                        "System",
                                        "Average",
                                        VRF(NumCond).Name);
                    SetupOutputVariable(state, "VRF Heat Pump Basin Heater Electricity Energy",
                                        OutputProcessor::Unit::J,
                                        VRF(NumCond).BasinHeaterConsumption,
                                        "System",
                                        "Sum",
                                        VRF(NumCond).Name,
                                        _,
                                        "Electricity",
                                        "COOLING",
                                        _,
                                        "System");
                }

            } else if (VRF(NumCond).CondenserType == DataHVACGlobals::WaterCooled) {
                SetupOutputVariable(state, "VRF Heat Pump Condenser Outlet Temperature",
                                    OutputProcessor::Unit::C,
                                    VRF(NumCond).CondenserSideOutletTemp,
                                    "System",
                                    "Average",
                                    VRF(NumCond).Name);
                SetupOutputVariable(state, "VRF Heat Pump Condenser Mass Flow Rate",
                                    OutputProcessor::Unit::kg_s,
                                    VRF(NumCond).WaterCondenserMassFlow,
                                    "System",
                                    "Average",
                                    VRF(NumCond).Name);
                SetupOutputVariable(state, "VRF Heat Pump Condenser Heat Transfer Rate",
                                    OutputProcessor::Unit::W,
                                    VRF(NumCond).QCondenser,
                                    "System",
                                    "Average",
                                    VRF(NumCond).Name);
                SetupOutputVariable(state, "VRF Heat Pump Condenser Heat Transfer Energy",
                                    OutputProcessor::Unit::J,
                                    VRF(NumCond).QCondEnergy,
                                    "System",
                                    "Sum",
                                    VRF(NumCond).Name);
            }

            if (AnyEnergyManagementSystemInModel) {
                SetupEMSActuator("Variable Refrigerant Flow Heat Pump",
                                 VRF(NumCond).Name,
                                 "Operating Mode",
                                 "[integer]",
                                 VRF(NumCond).EMSOverrideHPOperatingMode,
                                 VRF(NumCond).EMSValueForHPOperatingMode);
            }
        }
    }

    void
    InitVRF(EnergyPlusData &state, int const VRFTUNum, int const ZoneNum, bool const FirstHVACIteration, Real64 &OnOffAirFlowRatio, Real64 &QZnReq)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Richard Raustad, FSEC
        //       DATE WRITTEN   August 2010
        //       MODIFIED       July 2012, Chandan Sharma - FSEC: Added zone sys avail managers
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine is for initializations of the VRF Components.

        // METHODOLOGY EMPLOYED:
        // Uses the status flags to trigger initializations.

        using DataEnvironment::OutDryBulbTemp;
        using DataEnvironment::StdRhoAir;
        using DataHeatBalFanSys::TempControlType;
        using DataHeatBalFanSys::ZoneThermostatSetPointHi;
        using DataHeatBalFanSys::ZoneThermostatSetPointLo;
        using DataHeatBalFanSys::ZT;
        using DataSizing::AutoSize;
        using DataZoneEquipment::CheckZoneEquipmentList;
        using DataZoneEquipment::VRFTerminalUnit_Num;
        using DataZoneEquipment::ZoneEquipList;
        using Fans::GetFanVolFlow;
        using FluidProperties::GetDensityGlycol;
        using General::RoundSigDigits;
        using General::TrimSigDigits;
        using MixedAir::SimOAMixer;
        using PlantUtilities::InitComponentNodes;
        using ScheduleManager::GetCurrentScheduleValue;
        using SingleDuct::SimATMixer;

        static std::string const RoutineName("InitVRF");

        int InNode;                       // TU inlet node
        int OutNode;                      // TU outlet node
        int OutsideAirNode;               // TU mixer outside air inlet node
        int NumTULoop;                    // loop counter, number of TU's in list
        int ELLoop;                       // loop counter, number of zone equipment lists
        int ListLoop;                     // loop counter, number of equipment is each list
        int VRFCond;                      // index to VRF condenser
        int TUIndex;                      // index to TU
        int TUListNum;                    // index to VRF AC system terminal unit list
        int TUListIndex;                  // pointer to TU list for this VRF system
        int IndexToTUInTUList;            // index to TU in TerminalUnilList
        Real64 RhoAir;                    // air density at InNode
        static Real64 CurrentEndTime;     // end time of current time step
        static Real64 CurrentEndTimeLast; // end time of last time step
        static Real64 TimeStepSysLast;    // system time step on last time step
        Real64 TempOutput;                // Sensible output of TU
        Real64 LoadToCoolingSP;           // thermostat load to cooling setpoint (W)
        Real64 LoadToHeatingSP;           // thermostat load to heating setpoint (W)
        bool EnableSystem;                // use to turn on secondary operating mode if OA temp limits exceeded
        bool ErrorsFound;                 // flag returned from mining call
        Real64 rho;                       // density of water (kg/m3)
        Real64 OutsideDryBulbTemp;        // Outdoor air temperature at external node height
        bool errFlag;                     // local error flag
        Real64 SuppHeatCoilLoad;          // additional heating required by supplemental heater (W)
        Real64 SuppHeatCoilCapacity;      // supplemental heating coil size (W)

        // ALLOCATE and Initialize subroutine variables
        if (MyOneTimeFlag) {

            MyEnvrnFlag.allocate(NumVRFTU);
            MySizeFlag.allocate(NumVRFTU);
            MyVRFFlag.allocate(NumVRFTU);
            MyZoneEqFlag.allocate(NumVRFTU);
            MyBeginTimeStepFlag.allocate(NumVRFCond);
            MaxDeltaT.allocate(NumVRFCond);
            MinDeltaT.allocate(NumVRFCond);
            LastModeCooling.allocate(NumVRFCond);
            LastModeHeating.allocate(NumVRFCond);
            HeatingLoad.allocate(NumVRFCond);
            CoolingLoad.allocate(NumVRFCond);
            NumCoolingLoads.allocate(NumVRFCond);
            SumCoolingLoads.allocate(NumVRFCond);
            NumHeatingLoads.allocate(NumVRFCond);
            SumHeatingLoads.allocate(NumVRFCond);
            MyVRFCondFlag.allocate(NumVRFCond);
            MyEnvrnFlag = true;
            MySizeFlag = true;
            MyVRFFlag = true;
            MyZoneEqFlag = true;
            MyBeginTimeStepFlag = true;
            MaxDeltaT = 0.0;
            MinDeltaT = 0.0;
            LastModeCooling = false;
            LastModeHeating = true;
            NumCoolingLoads = 0;
            SumCoolingLoads = 0.0;
            NumHeatingLoads = 0;
            SumHeatingLoads = 0.0;

            MyOneTimeFlag = false;
            MyVRFCondFlag = true;

        } // IF (MyOneTimeFlag) THEN

        // identify VRF condenser connected to this TU
        VRFCond = VRFTU(VRFTUNum).VRFSysNum;
        TUListIndex = VRF(VRFCond).ZoneTUListPtr;
        InNode = VRFTU(VRFTUNum).VRFTUInletNodeNum;
        OutNode = VRFTU(VRFTUNum).VRFTUOutletNodeNum;
        OutsideAirNode = VRFTU(VRFTUNum).VRFTUOAMixerOANodeNum;
        IndexToTUInTUList = VRFTU(VRFTUNum).IndexToTUInTUList;

        SuppHeatCoilCapacity = 0.0;
        SuppHeatCoilLoad = 0.0;
        LoadToCoolingSP = 0.0;
        LoadToHeatingSP = 0.0;
        ErrorsFound = false;
        bool SetPointErrorFlag = false;

        // set condenser inlet temp, used as surrogate for OAT (used to check limits of operation)
        if (VRF(VRFCond).CondenserType == DataHVACGlobals::WaterCooled) {
            OutsideDryBulbTemp = DataLoopNode::Node(VRF(VRFCond).CondenserNodeNum).Temp;
        } else {
            if (OutsideAirNode == 0) {
                OutsideDryBulbTemp = OutDryBulbTemp;
            } else {
                OutsideDryBulbTemp = DataLoopNode::Node(OutsideAirNode).Temp;
            }
        }

        if (allocated(DataHVACGlobals::ZoneComp)) {
            if (MyZoneEqFlag(VRFTUNum)) { // initialize the name of each availability manager list and zone number
                DataHVACGlobals::ZoneComp(VRFTerminalUnit_Num).ZoneCompAvailMgrs(VRFTUNum).AvailManagerListName =
                    VRFTU(VRFTUNum).AvailManagerListName;
                DataHVACGlobals::ZoneComp(VRFTerminalUnit_Num).ZoneCompAvailMgrs(VRFTUNum).ZoneNum = ZoneNum;
                MyZoneEqFlag(VRFTUNum) = false;
            }
            VRFTU(VRFTUNum).AvailStatus = DataHVACGlobals::ZoneComp(VRFTerminalUnit_Num).ZoneCompAvailMgrs(VRFTUNum).AvailStatus;
        }

        if (VRFTU(VRFTUNum).MySuppCoilPlantScanFlag && allocated(PlantLoop)) {
            if (VRFTU(VRFTUNum).SuppHeatCoilType_Num == DataHVACGlobals::Coil_HeatingWater) {
                // hot water supplemental heating coil
                errFlag = false;
                PlantUtilities::ScanPlantLoopsForObject(state,
                                                        VRFTU(VRFTUNum).SuppHeatCoilName,
                                                        TypeOf_CoilWaterSimpleHeating,
                                                        VRFTU(VRFTUNum).SuppHeatCoilLoopNum,
                                                        VRFTU(VRFTUNum).SuppHeatCoilLoopSide,
                                                        VRFTU(VRFTUNum).SuppHeatCoilBranchNum,
                                                        VRFTU(VRFTUNum).SuppHeatCoilCompNum,
                                                        errFlag,
                                                        _,
                                                        _,
                                                        _,
                                                        _,
                                                        _);

                WaterCoils::SetCoilDesFlow(state, DataHVACGlobals::cAllCoilTypes(VRFTU(VRFTUNum).SuppHeatCoilType_Num),
                                           VRFTU(VRFTUNum).SuppHeatCoilName,
                                           VRFTU(VRFTUNum).MaxHeatAirVolFlow,
                                           errFlag);

                if (errFlag) {
                    ShowFatalError(RoutineName + ": Program terminated for previous conditions.");
                }
                VRFTU(VRFTUNum).SuppHeatCoilFluidMaxFlow =
                    WaterCoils::GetCoilMaxWaterFlowRate(state, "Coil:Heating:Water", VRFTU(VRFTUNum).SuppHeatCoilName, ErrorsFound);

                if (VRFTU(VRFTUNum).SuppHeatCoilFluidMaxFlow > 0.0) {
                    rho = GetDensityGlycol(state,
                                           PlantLoop(VRFTU(VRFTUNum).SuppHeatCoilLoopNum).FluidName,
                                           DataGlobals::HWInitConvTemp,
                                           PlantLoop(VRFTU(VRFTUNum).SuppHeatCoilLoopNum).FluidIndex,
                                           RoutineName);
                    VRFTU(VRFTUNum).SuppHeatCoilFluidMaxFlow = VRFTU(VRFTUNum).SuppHeatCoilFluidMaxFlow * rho;
                }

                // fill fluid outlet node for hot water coil SuppHeatCoilFluidOutletNode
                VRFTU(VRFTUNum).SuppHeatCoilFluidOutletNode = PlantLoop(VRFTU(VRFTUNum).SuppHeatCoilLoopNum)
                                                                  .LoopSide(VRFTU(VRFTUNum).SuppHeatCoilLoopSide)
                                                                  .Branch(VRFTU(VRFTUNum).SuppHeatCoilBranchNum)
                                                                  .Comp(VRFTU(VRFTUNum).SuppHeatCoilCompNum)
                                                                  .NodeNumOut;
                VRFTU(VRFTUNum).MySuppCoilPlantScanFlag = false;

            } else if (VRFTU(VRFTUNum).SuppHeatCoilType_Num == DataHVACGlobals::Coil_HeatingSteam) {
                // steam supplemental heating coil
                errFlag = false;
                PlantUtilities::ScanPlantLoopsForObject(state,
                                                        VRFTU(VRFTUNum).SuppHeatCoilName,
                                                        TypeOf_CoilSteamAirHeating,
                                                        VRFTU(VRFTUNum).SuppHeatCoilLoopNum,
                                                        VRFTU(VRFTUNum).SuppHeatCoilLoopSide,
                                                        VRFTU(VRFTUNum).SuppHeatCoilBranchNum,
                                                        VRFTU(VRFTUNum).SuppHeatCoilCompNum,
                                                        errFlag,
                                                        _,
                                                        _,
                                                        _,
                                                        _,
                                                        _);
                if (errFlag) {
                    ShowFatalError(RoutineName + ": Program terminated for previous conditions.");
                }
                VRFTU(VRFTUNum).SuppHeatCoilFluidMaxFlow = SteamCoils::GetCoilMaxSteamFlowRate(state, VRFTU(VRFTUNum).SuppHeatCoilIndex, ErrorsFound);
                if (VRFTU(VRFTUNum).SuppHeatCoilFluidMaxFlow > 0.0) {
                    int SteamIndex = 0; // fluid type index of 0 is passed if steam
                    Real64 TempSteamIn = 100.0;
                    Real64 SteamDensity = FluidProperties::GetSatDensityRefrig(state, fluidNameSteam, TempSteamIn, 1.0, SteamIndex, RoutineName);
                    VRFTU(VRFTUNum).SuppHeatCoilFluidMaxFlow = VRFTU(VRFTUNum).SuppHeatCoilFluidMaxFlow * SteamDensity;
                }

                // fill fluid outlet node for steam coil SuppHeatCoilFluidOutletNode
                VRFTU(VRFTUNum).SuppHeatCoilFluidOutletNode = PlantLoop(VRFTU(VRFTUNum).SuppHeatCoilLoopNum)
                                                                  .LoopSide(VRFTU(VRFTUNum).SuppHeatCoilLoopSide)
                                                                  .Branch(VRFTU(VRFTUNum).SuppHeatCoilBranchNum)
                                                                  .Comp(VRFTU(VRFTUNum).SuppHeatCoilCompNum)
                                                                  .NodeNumOut;
                VRFTU(VRFTUNum).MySuppCoilPlantScanFlag = false;

            } else { // VRF terminal unit not connected to plant
                VRFTU(VRFTUNum).MySuppCoilPlantScanFlag = false;
            }
        } else if (VRFTU(VRFTUNum).MySuppCoilPlantScanFlag && !AnyPlantInModel) {
            VRFTU(VRFTUNum).MySuppCoilPlantScanFlag = false;
        }

        // one-time check to see if VRF TU's are on ZoneHVAC:EquipmentList or AirloopHVAC or issue warning
        if (ZoneEquipmentListNotChecked) {
            if (state.dataAirLoop->AirLoopInputsFilled) ZoneEquipmentListNotChecked = false;
            bool AirLoopFound = false;
            bool errorsFound = false;
            bool AirNodeFound = false;
            int ctrlZoneNum = 0;
            std::string const cCurrentModuleObject = "ZoneHVAC:TerminalUnit:VariableRefrigerantFlow";
            for (TUListNum = 1; TUListNum <= NumVRFTULists; ++TUListNum) {
                for (NumTULoop = 1; NumTULoop <= TerminalUnitList(TUListNum).NumTUInList; ++NumTULoop) {
                    AirLoopFound = false; // reset for next TU
                    ctrlZoneNum = 0;      // reset for next TU
                    TUIndex = TerminalUnitList(TUListNum).ZoneTUPtr(NumTULoop);
                    std::string const thisObjectName = VRFTU(TUIndex).Name;
                    if (VRFTU(TUIndex).isInZone) goto EquipList_exit;   // already found previously
                    for (ELLoop = 1; ELLoop <= NumOfZones; ++ELLoop) {  // NumOfZoneEquipLists
                        if (ZoneEquipList(ELLoop).Name == "") continue; // dimensioned by NumOfZones.  Only valid ones have names.
                        for (ListLoop = 1; ListLoop <= ZoneEquipList(ELLoop).NumOfEquipTypes; ++ListLoop) {
                            if (!UtilityRoutines::SameString(ZoneEquipList(ELLoop).EquipType(ListLoop),
                                                             DataHVACGlobals::cVRFTUTypes(VRFTU(TUIndex).VRFTUType_Num)))
                                continue;
                            if (!UtilityRoutines::SameString(ZoneEquipList(ELLoop).EquipName(ListLoop), VRFTU(TUIndex).Name)) continue;
                            VRFTU(TUIndex).ZoneNum = ELLoop;
                            VRFTU(TUIndex).isInZone = true;
                            if (VRF(VRFTU(TUIndex).VRFSysNum).MasterZonePtr == ELLoop) {
                                VRF(VRFTU(TUIndex).VRFSysNum).MasterZoneTUIndex = TUIndex;
                            }
                            if (VRFTU(TUIndex).ZoneAirNode == 0) {
                                bool ZoneNodeNotFound = true;
                                for (int CtrlZone = 1; CtrlZone <= NumOfZones; ++CtrlZone) {
                                    if (!DataZoneEquipment::ZoneEquipConfig(CtrlZone).IsControlled) continue;
                                    for (int NodeNum = 1; NodeNum <= DataZoneEquipment::ZoneEquipConfig(CtrlZone).NumExhaustNodes; ++NodeNum) {
                                        if (VRFTU(TUIndex).VRFTUInletNodeNum == DataZoneEquipment::ZoneEquipConfig(CtrlZone).ExhaustNode(NodeNum)) {
                                            VRFTU(TUIndex).ZoneAirNode = DataZoneEquipment::ZoneEquipConfig(CtrlZone).ZoneNode;
                                            ZoneNodeNotFound = false;
                                            break;
                                        }
                                    }
                                    for (int NodeNum = 1; NodeNum <= DataZoneEquipment::ZoneEquipConfig(CtrlZone).NumInletNodes; ++NodeNum) {
                                        if (VRFTU(TUIndex).VRFTUOutletNodeNum == DataZoneEquipment::ZoneEquipConfig(CtrlZone).InletNode(NodeNum)) {
                                            VRFTU(TUIndex).ZoneAirNode = DataZoneEquipment::ZoneEquipConfig(CtrlZone).ZoneNode;
                                            ZoneNodeNotFound = false;
                                            break;
                                        }
                                    }
                                    if (!ZoneNodeNotFound) break;
                                }
                                if (ZoneNodeNotFound) {
                                    ShowSevereError(
                                        "ZoneHVAC:TerminalUnit:VariableRefrigerantFlow \"" + VRFTU(TUIndex).Name +
                                        "\" Zone terminal unit air inlet node name must be the same as a zone inlet or exhaust node name.");
                                    ShowContinueError("... Zone inlet and exhaust node name is specified in ZoneHVAC:EquipmentConnections object.");
                                    ShowContinueError("... Zone terminal unit inlet node name  = " +
                                                      DataLoopNode::NodeID(VRFTU(TUIndex).VRFTUInletNodeNum));
                                    ShowContinueError("... Zone terminal unit outlet node name = " +
                                                      DataLoopNode::NodeID(VRFTU(TUIndex).VRFTUOutletNodeNum));
                                }
                            }
                            goto EquipList_exit;
                        }
                    }
                    // check if the TU is connected to an air loop
                    if (!VRFTU(TUIndex).isInAirLoop) {
                        for (int AirLoopNum = 1; AirLoopNum <= DataHVACGlobals::NumPrimaryAirSys; ++AirLoopNum) {
                            for (int BranchNum = 1; BranchNum <= DataAirSystems::PrimaryAirSystem(AirLoopNum).NumBranches; ++BranchNum) {
                                for (int CompNum = 1; CompNum <= DataAirSystems::PrimaryAirSystem(AirLoopNum).Branch(BranchNum).TotalComponents;
                                     ++CompNum) {
                                    if (UtilityRoutines::SameString(DataAirSystems::PrimaryAirSystem(AirLoopNum).Branch(BranchNum).Comp(CompNum).Name,
                                                                    thisObjectName) &&
                                        UtilityRoutines::SameString(
                                            DataAirSystems::PrimaryAirSystem(AirLoopNum).Branch(BranchNum).Comp(CompNum).TypeOf,
                                            cCurrentModuleObject)) {
                                        VRFTU(TUIndex).airLoopNum = AirLoopNum;
                                        AirLoopFound = true;
                                        VRFTU(TUIndex).isInAirLoop = true;
                                        BranchNodeConnections::TestCompSet(cCurrentModuleObject,
                                                                           thisObjectName,
                                                                           DataLoopNode::NodeID(VRFTU(TUIndex).VRFTUInletNodeNum),
                                                                           DataLoopNode::NodeID(VRFTU(TUIndex).VRFTUOutletNodeNum),
                                                                           "Air Nodes");
                                        if (VRFTU(TUIndex).ZoneNum > 0) {
                                            VRFTU(TUIndex).ZoneAirNode = DataZoneEquipment::ZoneEquipConfig(VRFTU(TUIndex).ZoneNum).ZoneNode;
                                            for (int ControlledZoneNum = 1; ControlledZoneNum <= DataGlobals::NumOfZones; ++ControlledZoneNum) {
                                                if (DataZoneEquipment::ZoneEquipConfig(ControlledZoneNum).ActualZoneNum != VRFTU(TUIndex).ZoneNum)
                                                    continue;
                                                for (int TstatZoneNum = 1; TstatZoneNum <= DataZoneControls::NumTempControlledZones; ++TstatZoneNum) {
                                                    if (DataZoneControls::TempControlledZone(TstatZoneNum).ActualZoneNum != VRFTU(TUIndex).ZoneNum)
                                                        continue;
                                                    VRF(VRFTU(TUIndex).VRFSysNum).MasterZoneTUIndex = TUIndex;
                                                    AirNodeFound = true;
                                                    ctrlZoneNum = ControlledZoneNum;
                                                    goto EquipList_exit;
                                                }
                                                for (int TstatZoneNum = 1; TstatZoneNum <= DataZoneControls::NumComfortControlledZones;
                                                     ++TstatZoneNum) {
                                                    if (DataZoneControls::ComfortControlledZone(TstatZoneNum).ActualZoneNum != VRFTU(TUIndex).ZoneNum)
                                                        continue;
                                                    VRF(VRFTU(TUIndex).VRFSysNum).MasterZoneTUIndex = TUIndex;
                                                    AirNodeFound = true;
                                                    ctrlZoneNum = ControlledZoneNum;
                                                    goto EquipList_exit;
                                                }
                                                if (!AirNodeFound && VRFTU(TUIndex).ZoneNum > 0) {
                                                    ShowSevereError("Input errors for " + cCurrentModuleObject + ":" + thisObjectName);
                                                    ShowContinueError("Did not find Air node (Zone with Thermostat or Thermal Comfort Thermostat).");
                                                    // ShowContinueError("specified Controlling Zone or Thermostat Location name = " +
                                                    // loc_controlZoneName);
                                                    errorsFound = true;
                                                }
                                            }
                                        } else if (AirLoopFound) { // control zone name not entered in TU object input
                                            VRFTU(TUIndex).isSetPointControlled = true;
                                        }
                                    }
                                    if (AirLoopFound) break;
                                }
                                if (AirLoopFound) break;
                            }
                            if (AirLoopFound) break;
                        }
                    }

                    // check if the TU is connected to an outside air system
                    if (!AirLoopFound && !VRFTU(TUIndex).isInOASys) {
                        for (int OASysNum = 1; OASysNum <= state.dataAirLoop->NumOASystems; ++OASysNum) {
                            for (int OACompNum = 1; OACompNum <= state.dataAirLoop->OutsideAirSys(OASysNum).NumComponents; ++OACompNum) {
                                if (!UtilityRoutines::SameString(state.dataAirLoop->OutsideAirSys(OASysNum).ComponentName(OACompNum),
                                                                 VRFTU(TUIndex).Name) ||
                                    !UtilityRoutines::SameString(state.dataAirLoop->OutsideAirSys(OASysNum).ComponentType(OACompNum), cCurrentModuleObject))
                                    continue;
                                VRFTU(TUIndex).airLoopNum = 0; // need air loop number here?
                                VRFTU(TUIndex).isInOASys = true;
                                AirLoopFound = true;
                                VRFTU(TUIndex).isSetPointControlled = true;
                                // user may have inadvertently entered a zone name in the OA system TU object
                                VRFTU(TUIndex).ZoneNum = 0;
                                VRFTU(TUIndex).ZoneAirNode = 0;
                                BranchNodeConnections::TestCompSet(cCurrentModuleObject,
                                                                   thisObjectName,
                                                                   DataLoopNode::NodeID(VRFTU(TUIndex).VRFTUInletNodeNum),
                                                                   DataLoopNode::NodeID(VRFTU(TUIndex).VRFTUOutletNodeNum),
                                                                   "Air Nodes");
                                goto EquipList_exit;
                            }
                        }
                    }
                EquipList_exit:;
                    if (ctrlZoneNum > 0) {
                        int inletNodeADUNum = 0;
                        int sysType_Num = 0;
                        std::string sysName = "";
                        for (int inletNode = 1; inletNode <= DataZoneEquipment::ZoneEquipConfig(ctrlZoneNum).NumInletNodes; inletNode++) {
                            if (DataZoneEquipment::ZoneEquipConfig(ctrlZoneNum).InletNodeAirLoopNum(inletNode) != VRFTU(TUIndex).airLoopNum) continue;
                            inletNodeADUNum = DataZoneEquipment::ZoneEquipConfig(ctrlZoneNum).InletNodeADUNum(inletNode);
                            if (inletNodeADUNum > 0 && inletNodeADUNum <= DataDefineEquip::NumAirDistUnits) {
                                sysType_Num = DataZoneEquipment::AirDistUnit_Num;
                                sysName = DataDefineEquip::AirDistUnit(inletNodeADUNum).Name;
                                break;
                            }
                        }
                        if (inletNodeADUNum > 0) {
                            if (DataZoneEquipment::ZoneEquipConfig(ctrlZoneNum).EquipListIndex > 0) {
                                for (int EquipNum = 1;
                                     EquipNum <=
                                     DataZoneEquipment::ZoneEquipList(DataZoneEquipment::ZoneEquipConfig(ctrlZoneNum).EquipListIndex).NumOfEquipTypes;
                                     ++EquipNum) {
                                    if ((DataZoneEquipment::ZoneEquipList(DataZoneEquipment::ZoneEquipConfig(ctrlZoneNum).EquipListIndex)
                                             .EquipType_Num(EquipNum) != sysType_Num) ||
                                        DataZoneEquipment::ZoneEquipList(DataZoneEquipment::ZoneEquipConfig(ctrlZoneNum).EquipListIndex)
                                                .EquipName(EquipNum) != sysName)
                                        continue;
                                    VRFTU(TUIndex).zoneSequenceCoolingNum =
                                        DataZoneEquipment::ZoneEquipList(DataZoneEquipment::ZoneEquipConfig(ctrlZoneNum).EquipListIndex)
                                            .CoolingPriority(EquipNum);
                                    VRFTU(TUIndex).zoneSequenceHeatingNum =
                                        DataZoneEquipment::ZoneEquipList(DataZoneEquipment::ZoneEquipConfig(ctrlZoneNum).EquipListIndex)
                                            .HeatingPriority(EquipNum);
                                    break;
                                }
                            }
                        } else {
                            ShowSevereError("Input errors for " + cCurrentModuleObject + ":" + thisObjectName);
                            ShowContinueError("Did not find ZoneHVAC:EquipmentList connected to this VRF terminal unit.");
                            errorsFound = true;
                        }
                    }

                    // Find the number of zones (zone Inlet nodes) attached to an air loop from the air loop number
                    if (AirLoopFound || VRFTU(TUIndex).isInAirLoop) {
                        int NumAirLoopZones = 0;
                        bool initLoadBasedControlFlowFracFlagReady = false;
                        Real64 initLoadBasedControlCntrlZoneTerminalUnitMassFlowRateMax = 0.0;
                        if (allocated(state.dataAirLoop->AirToZoneNodeInfo) && VRFTU(TUIndex).airLoopNum > 0)
                            NumAirLoopZones = state.dataAirLoop->AirToZoneNodeInfo(VRFTU(TUIndex).airLoopNum).NumZonesCooled +
                                              state.dataAirLoop->AirToZoneNodeInfo(VRFTU(TUIndex).airLoopNum).NumZonesHeated;
                        if (allocated(state.dataAirLoop->AirToZoneNodeInfo)) {
                            initLoadBasedControlFlowFracFlagReady = true;
                            for (int ZoneInSysIndex = 1; ZoneInSysIndex <= NumAirLoopZones; ++ZoneInSysIndex) {
                                // zone inlet nodes for cooling
                                if (state.dataAirLoop->AirToZoneNodeInfo(VRFTU(TUIndex).airLoopNum).NumZonesCooled > 0) {
                                    if (state.dataAirLoop->AirToZoneNodeInfo(VRFTU(TUIndex).airLoopNum).TermUnitCoolInletNodes(ZoneInSysIndex) == -999) {
                                        // the data structure for the zones inlet nodes has not been filled
                                        initLoadBasedControlFlowFracFlagReady = false;
                                    } else {
                                        int ZoneInletNodeNum =
                                            state.dataAirLoop->AirToZoneNodeInfo(VRFTU(TUIndex).airLoopNum).TermUnitCoolInletNodes(ZoneInSysIndex);
                                        if (DataLoopNode::Node(ZoneInletNodeNum).MassFlowRateMax == -999.0) {
                                            // the node mass flow rate has not been set
                                            initLoadBasedControlFlowFracFlagReady = false;
                                        }
                                    }
                                }
                                // zone inlet nodes for heating
                                if (state.dataAirLoop->AirToZoneNodeInfo(VRFTU(TUIndex).airLoopNum).NumZonesHeated > 0) {
                                    if (state.dataAirLoop->AirToZoneNodeInfo(VRFTU(TUIndex).airLoopNum).TermUnitHeatInletNodes(ZoneInSysIndex) == -999) {
                                        // the data structure for the zones inlet nodes has not been filled
                                        initLoadBasedControlFlowFracFlagReady = false;
                                    } else {
                                        int ZoneInletNodeNum =
                                            state.dataAirLoop->AirToZoneNodeInfo(VRFTU(TUIndex).airLoopNum).TermUnitHeatInletNodes(ZoneInSysIndex);
                                        if (DataLoopNode::Node(ZoneInletNodeNum).MassFlowRateMax == -999.0) {
                                            // the node mass flow rate has not been set
                                            initLoadBasedControlFlowFracFlagReady = false;
                                        }
                                    }
                                }
                            }
                        }
                        if (allocated(state.dataAirLoop->AirToZoneNodeInfo) && initLoadBasedControlFlowFracFlagReady) {
                            Real64 SumOfMassFlowRateMax = 0.0; // initialize the sum of the maximum flows
                            for (int ZoneInSysIndex = 1; ZoneInSysIndex <= NumAirLoopZones; ++ZoneInSysIndex) {
                                int ZoneInletNodeNum =
                                    state.dataAirLoop->AirToZoneNodeInfo(VRFTU(TUIndex).airLoopNum).TermUnitCoolInletNodes(ZoneInSysIndex);
                                SumOfMassFlowRateMax += DataLoopNode::Node(ZoneInletNodeNum).MassFlowRateMax;
                                if (state.dataAirLoop->AirToZoneNodeInfo(VRFTU(TUIndex).airLoopNum).CoolCtrlZoneNums(ZoneInSysIndex) ==
                                    VRFTU(TUIndex).ZoneNum) {
                                    initLoadBasedControlCntrlZoneTerminalUnitMassFlowRateMax = DataLoopNode::Node(ZoneInletNodeNum).MassFlowRateMax;
                                }
                            }
                            if (SumOfMassFlowRateMax != 0.0 && state.dataAirLoop->AirLoopInputsFilled) {
                                if (initLoadBasedControlCntrlZoneTerminalUnitMassFlowRateMax >= DataHVACGlobals::SmallAirVolFlow) {
                                    VRFTU(TUIndex).controlZoneMassFlowFrac =
                                        initLoadBasedControlCntrlZoneTerminalUnitMassFlowRateMax / SumOfMassFlowRateMax;
                                    BaseSizer::reportSizerOutput(DataHVACGlobals::cVRFTUTypes(VRFTU(TUIndex).VRFTUType_Num),
                                                                 VRFTU(TUIndex).Name,
                                                                 "Fraction of Supply Air Flow That Goes Through the Controlling Zone",
                                                                 VRFTU(TUIndex).controlZoneMassFlowFrac);
                                    VRFTU(TUIndex).isSetPointControlled = false; // redundant
                                } else {
                                    if (VRFTU(TUIndex).isInAirLoop && VRFTU(TUIndex).ZoneNum == 0 && VRFTU(TUIndex).ZoneAirNode == 0) {
                                        // TU must be set point controlled and use constant fan mode (or coil out T won't change with PLR/air flow)
                                        VRFTU(TUIndex).isSetPointControlled = true;
                                        if (VRFTU(TUIndex).FanOpModeSchedPtr > 0) {
                                            if (ScheduleManager::GetScheduleMinValue(VRFTU(TUIndex).FanOpModeSchedPtr) == 0.0) {
                                                ShowSevereError(DataHVACGlobals::cVRFTUTypes(VRFTU(TUIndex).VRFTUType_Num) + " = " +
                                                                VRFTU(TUIndex).Name);
                                                ShowContinueError("When using set point control, fan operating mode must be continuous (fan "
                                                                  "operating mode schedule values > 0).");
                                                ShowContinueError("Error found in Supply Air Fan Operating Mode Schedule Name = " +
                                                                  ScheduleManager::Schedule(VRFTU(TUIndex).FanOpModeSchedPtr).Name);
                                                ShowContinueError("...schedule values must be (>0., <=1.)");
                                                ErrorsFound = true;
                                            }
                                        }
                                    } else {
                                        ShowSevereError(DataHVACGlobals::cVRFTUTypes(VRFTU(TUIndex).VRFTUType_Num) + " = " + VRFTU(TUIndex).Name);
                                        ShowContinueError(" The Fraction of Supply Air Flow That Goes Through the Controlling Zone is set to 1.");
                                        VRFTU(TUIndex).controlZoneMassFlowFrac = 1.0;
                                        BaseSizer::reportSizerOutput(DataHVACGlobals::cVRFTUTypes(VRFTU(TUIndex).VRFTUType_Num),
                                                                     VRFTU(TUIndex).Name,
                                                                     "Fraction of Supply Air Flow That Goes Through the Controlling Zone",
                                                                     VRFTU(TUIndex).controlZoneMassFlowFrac);
                                    }
                                }
                            } else if (VRFTU(TUIndex).ZoneNum == 0) {
                                // TU must be set point controlled and use constant fan mode (or coil outlet T won't change with PLR/air flow rate)
                                // TU inlet air flow rate is also determined by OA system, not TU
                                VRFTU(TUIndex).isSetPointControlled = true;
                            }
                        }
                    }

                    if (VRFTU(TUIndex).isInZone && state.dataAirLoop->AirLoopInputsFilled) {
                        if (VRFTU(TUIndex).FanPlace == 0) {
                            ShowSevereError("ZoneHVAC:TerminalUnit:VariableRefrigerantFlow = " + VRFTU(TUIndex).Name);
                            ShowContinueError("Illegal Supply Air Fan Placement.");
                            ErrorsFound = true;
                        }
                        if (VRFTU(TUIndex).fanType_Num == DataHVACGlobals::FanType_SystemModelObject) {
                            if (VRFTU(TUIndex).FanIndex == -1) {
                                ShowSevereError("ZoneHVAC:TerminalUnit:VariableRefrigerantFlow = " + VRFTU(TUIndex).Name);
                                ShowContinueError("VRF Terminal Unit fan is required when used as zone equipment.");
                                ErrorsFound = true;
                            }
                        } else {
                            if (VRFTU(TUIndex).FanIndex == 0) {
                                ShowSevereError("ZoneHVAC:TerminalUnit:VariableRefrigerantFlow = " + VRFTU(TUIndex).Name);
                                ShowContinueError("VRF Terminal Unit fan is required when used as zone equipment.");
                                ErrorsFound = true;
                            }
                        }
                    }

                    if (VRFTU(TUIndex).isSetPointControlled && state.dataAirLoop->AirLoopInputsFilled) {
                        bool missingSetPoint = false;
                        Real64 TUOutNodeSP = DataLoopNode::Node(VRFTU(TUIndex).VRFTUOutletNodeNum).TempSetPoint;
                        Real64 coolCoilOutNodeSP = DataLoopNode::Node(VRFTU(TUIndex).coolCoilAirOutNode).TempSetPoint;
                        Real64 heatCoilOutNodeSP = DataLoopNode::Node(VRFTU(TUIndex).heatCoilAirOutNode).TempSetPoint;
                        // SP can be at outlet of TU or at outlet of coils
                        // if supp heat coil is present, a SP must be at the outlet of the TU
                        if (VRFTU(TUIndex).SuppHeatingCoilPresent) {
                            if (TUOutNodeSP == DataLoopNode::SensedNodeFlagValue) missingSetPoint = true;
                        } else {
                            if (VRFTU(TUIndex).FanPlace == DataHVACGlobals::DrawThru) {
                                // then SP must be at TU outlet
                                if (TUOutNodeSP == DataLoopNode::SensedNodeFlagValue) missingSetPoint = true;
                                // or at coil outlet nodes
                                if (missingSetPoint) {
                                    if (coolCoilOutNodeSP != DataLoopNode::SensedNodeFlagValue &&
                                        heatCoilOutNodeSP != DataLoopNode::SensedNodeFlagValue)
                                        missingSetPoint = false;
                                }
                            } else {
                                // else fan is blow thru or missing
                                if (TUOutNodeSP == DataLoopNode::SensedNodeFlagValue) missingSetPoint = true;
                            }
                        }
                        if (missingSetPoint) {
                            if (!DataGlobals::AnyEnergyManagementSystemInModel) {
                                ShowSevereError("ZoneHVAC:TerminalUnit:VariableRefrigerantFlow: Missing temperature setpoint for " +
                                                VRFTU(TUIndex).Name);
                                ShowContinueError("...use a Setpoint Manager to establish a setpoint at the TU or coil(s) outlet node.");
                                ErrorsFound = true;
                            } else if (DataGlobals::AnyEnergyManagementSystemInModel) {
                                bool SPNotFound = false;
                                EMSManager::CheckIfNodeSetPointManagedByEMS(
                                    VRFTU(TUIndex).VRFTUOutletNodeNum, EMSManager::iTemperatureSetPoint, SetPointErrorFlag);
                                SPNotFound = SPNotFound || SetPointErrorFlag;
                                EMSManager::CheckIfNodeSetPointManagedByEMS(
                                    VRFTU(TUIndex).coolCoilAirOutNode, EMSManager::iTemperatureSetPoint, SetPointErrorFlag);
                                SPNotFound = SPNotFound || SetPointErrorFlag;
                                EMSManager::CheckIfNodeSetPointManagedByEMS(
                                    VRFTU(TUIndex).heatCoilAirOutNode, EMSManager::iTemperatureSetPoint, SetPointErrorFlag);
                                SPNotFound = SPNotFound || SetPointErrorFlag;

                                // We disable the check at end (if API), because one of the nodes is enough, so there's an almost certainty
                                // that it will throw as you're unlikely going to actuate all three nodes
                                // It's not ideal, but it's better to let slide a bad condition rather than throw false positives...
                                DataLoopNode::NodeSetpointCheck(VRFTU(TUIndex).VRFTUOutletNodeNum).needsSetpointChecking = false;
                                DataLoopNode::NodeSetpointCheck(VRFTU(TUIndex).coolCoilAirOutNode).needsSetpointChecking = false;
                                DataLoopNode::NodeSetpointCheck(VRFTU(TUIndex).heatCoilAirOutNode).needsSetpointChecking = false;

                                if (SPNotFound && state.dataAirLoop->AirLoopInputsFilled) {
                                    ShowSevereError(
                                        "ZoneHVAC:TerminalUnit:VariableRefrigerantFlow: Missing temperature setpoint for unitary system = " +
                                        VRFTU(TUIndex).Name);
                                    ShowContinueError("...use a Setpoint Manager to establish a setpoint at the TU or coil(s) outlet node.");
                                    ShowContinueError("...or use an EMS actuator to establish a temperature setpoint at the coil control node.");
                                    ErrorsFound = true;
                                }
                            }
                        }
                    }

                    if (VRFTU(TUIndex).isInAirLoop || VRFTU(TUIndex).isInOASys || VRFTU(TUIndex).isInZone) continue;
                    if (!state.dataAirLoop->AirLoopInputsFilled) continue;
                    ShowSevereError("InitVRF: VRF Terminal Unit = [" + DataHVACGlobals::cVRFTUTypes(VRFTU(TUIndex).VRFTUType_Num) + ',' +
                                    VRFTU(TUIndex).Name +
                                    "] is not on any ZoneHVAC:EquipmentList, AirloopHVAC or AirLoopHVAC:OutdoorAirSystem:EquipmentList.  It will not "
                                    "be simulated.");
                    ShowContinueError("...The VRF AC System associated with this terminal unit may also not be simulated.");
                }
            }

            // TU inlet node must be the same as a zone exhaust node and the OA Mixer return node
            // check that TU inlet node is a zone exhaust node.
            if (VRFTU(VRFTUNum).isInZone && (!VRFTU(VRFTUNum).ATMixerExists || VRFTU(VRFTUNum).ATMixerType == DataHVACGlobals::ATMixer_SupplySide)) {
                bool ZoneNodeNotFound = true;
                for (int CtrlZone = 1; CtrlZone <= NumOfZones; ++CtrlZone) {
                    if (!DataZoneEquipment::ZoneEquipConfig(CtrlZone).IsControlled) continue;
                    for (int NodeNum = 1; NodeNum <= DataZoneEquipment::ZoneEquipConfig(CtrlZone).NumExhaustNodes; ++NodeNum) {
                        if (VRFTU(VRFTUNum).VRFTUInletNodeNum == DataZoneEquipment::ZoneEquipConfig(CtrlZone).ExhaustNode(NodeNum)) {
                            VRFTU(VRFTUNum).ZoneAirNode = DataZoneEquipment::ZoneEquipConfig(CtrlZone).ZoneNode;
                            ZoneNodeNotFound = false;
                            break;
                        }
                    }
                    if (!ZoneNodeNotFound) break;
                }
                if (ZoneNodeNotFound && !VRFTU(VRFTUNum).isInAirLoop) {
                    ShowSevereError(cCurrentModuleObject + " \"" + VRFTU(VRFTUNum).Name +
                                    "\" Zone terminal unit air inlet node name must be the same as a zone exhaust node name.");
                    ShowContinueError("... Zone exhaust node name is specified in ZoneHVAC:EquipmentConnections object.");
                    ShowContinueError("... Zone terminal unit inlet node name = " + DataLoopNode::NodeID(VRFTU(VRFTUNum).VRFTUInletNodeNum));
                    ErrorsFound = true;
                }
            }
            // check OA Mixer return node
            if (VRFTU(VRFTUNum).isInZone && !VRFTU(VRFTUNum).ATMixerExists && VRFTU(VRFTUNum).OAMixerUsed) {
                Array1D_int OANodeNums = MixedAir::GetOAMixerNodeNumbers(state, VRFTU(VRFTUNum).OAMixerName, errFlag);
                if (VRFTU(VRFTUNum).VRFTUInletNodeNum != OANodeNums(3)) {
                    ShowSevereError(cCurrentModuleObject + " \"" + VRFTU(VRFTUNum).Name +
                                    "\" Zone terminal unit air inlet node name must be the same as the OutdoorAir:Mixer return air node name.");
                    ShowContinueError("... Zone terminal unit air inlet node name = " + DataLoopNode::NodeID(VRFTU(VRFTUNum).VRFTUInletNodeNum));
                    ShowContinueError("... OutdoorAir:Mixer return air node name = " + DataLoopNode::NodeID(OANodeNums(3)));
                    ErrorsFound = true;
                }
            }
            // check that TU outlet node is a zone inlet node.
            if ((VRFTU(VRFTUNum).isInZone && (!VRFTU(VRFTUNum).ATMixerExists || VRFTU(VRFTUNum).ATMixerType == DataHVACGlobals::ATMixer_InletSide))) {
                bool ZoneNodeNotFound = true;
                for (int CtrlZone = 1; CtrlZone <= NumOfZones; ++CtrlZone) {
                    if (!DataZoneEquipment::ZoneEquipConfig(CtrlZone).IsControlled) continue;
                    for (int NodeNum = 1; NodeNum <= DataZoneEquipment::ZoneEquipConfig(CtrlZone).NumInletNodes; ++NodeNum) {
                        if (VRFTU(VRFTUNum).VRFTUOutletNodeNum == DataZoneEquipment::ZoneEquipConfig(CtrlZone).InletNode(NodeNum)) {
                            VRFTU(VRFTUNum).ZoneAirNode = DataZoneEquipment::ZoneEquipConfig(CtrlZone).ZoneNode;
                            ZoneNodeNotFound = false;
                            break;
                        }
                    }
                    if (!ZoneNodeNotFound) break;
                }
                if (ZoneNodeNotFound) {
                    ShowSevereError(cCurrentModuleObject + " \"" + VRFTU(VRFTUNum).Name +
                                    "\" Zone terminal unit air outlet node name must be the same as a zone inlet node name.");
                    ShowContinueError("... Zone inlet node name is specified in ZoneHVAC:EquipmentConnections object.");
                    ShowContinueError("... Zone terminal unit outlet node name = " + DataLoopNode::NodeID(VRFTU(VRFTUNum).VRFTUOutletNodeNum));
                    ErrorsFound = true;
                }
            }
            // check fan inlet and outlet nodes
            int FanInletNodeNum = 0;
            if (VRFTU(VRFTUNum).fanType_Num == DataHVACGlobals::FanType_SystemModelObject) {
                if (VRFTU(VRFTUNum).FanIndex > -1) FanInletNodeNum = HVACFan::fanObjs[VRFTU(VRFTUNum).FanIndex]->inletNodeNum;
            } else {
                if (VRFTU(VRFTUNum).FanIndex > 0) FanInletNodeNum = Fans::getFanInNodeIndex(state, VRFTU(VRFTUNum).FanIndex, errFlag);
            }
            int CCoilInletNodeNum = DXCoils::getCoilInNodeIndex(state, VRFTU(VRFTUNum).CoolCoilIndex, errFlag);
            int CCoilOutletNodeNum = DXCoils::getCoilOutNodeIndex(state, VRFTU(VRFTUNum).CoolCoilIndex, errFlag);
            int HCoilInletNodeNum = DXCoils::getCoilInNodeIndex(state, VRFTU(VRFTUNum).HeatCoilIndex, errFlag);
            int HCoilOutletNodeNum = DXCoils::getCoilOutNodeIndex(state, VRFTU(VRFTUNum).HeatCoilIndex, errFlag);
            if (VRFTU(VRFTUNum).isInZone && VRFTU(VRFTUNum).FanPlace == DataHVACGlobals::BlowThru) {
                if (!VRFTU(VRFTUNum).ATMixerExists && VRFTU(VRFTUNum).OAMixerUsed) {
                    Array1D_int OANodeNums = MixedAir::GetOAMixerNodeNumbers(state, VRFTU(VRFTUNum).OAMixerName, errFlag);
                    if (FanInletNodeNum != OANodeNums(4)) {
                        ShowSevereError(cCurrentModuleObject + " \"" + VRFTU(VRFTUNum).Name + "\" Fan inlet node name must be the same");
                        ShowContinueError("as the outside air mixers mixed air node name when blow through fan is specified and an outside air "
                                          "mixer is present.");
                        ShowContinueError("... Fan inlet node = " + DataLoopNode::NodeID(FanInletNodeNum));
                        ShowContinueError("... OA mixers mixed air node = " + DataLoopNode::NodeID(OANodeNums(4)));
                        ErrorsFound = true;
                    }
                } else if (!VRFTU(VRFTUNum).OAMixerUsed &&
                           (VRFTU(VRFTUNum).ATMixerExists && VRFTU(VRFTUNum).ATMixerType == DataHVACGlobals::ATMixer_SupplySide)) {
                    if (FanInletNodeNum != VRFTU(VRFTUNum).VRFTUInletNodeNum) {
                        ShowSevereError(cCurrentModuleObject + " \"" + VRFTU(VRFTUNum).Name + "\" Fan inlet node name must be the same");
                        ShowContinueError("as the terminal unit air inlet node name when blow through fan is specified and an outside air mixer "
                                          "is not present.");
                        ShowContinueError("... Fan inlet node = " + DataLoopNode::NodeID(FanInletNodeNum));
                        ShowContinueError("... Terminal unit air inlet node = " + DataLoopNode::NodeID(VRFTU(VRFTUNum).VRFTUInletNodeNum));
                        ErrorsFound = true;
                    }
                }
                if (VRFTU(VRFTUNum).CoolingCoilPresent) {
                    if (VRFTU(VRFTUNum).fanOutletNode != CCoilInletNodeNum) {
                        ShowSevereError(cCurrentModuleObject + " \"" + VRFTU(VRFTUNum).Name + "\" Fan outlet node name must be the same");
                        ShowContinueError("as the DX cooling coil air inlet node name when blow through fan is specified.");
                        ShowContinueError("... Fan outlet node = " + DataLoopNode::NodeID(VRFTU(VRFTUNum).fanOutletNode));
                        ShowContinueError("... DX cooling coil air inlet node = " + DataLoopNode::NodeID(CCoilInletNodeNum));
                        ErrorsFound = true;
                    }
                    if (VRFTU(VRFTUNum).HeatingCoilPresent && !VRFTU(VRFTUNum).SuppHeatingCoilPresent) {
                        if (HCoilOutletNodeNum != VRFTU(VRFTUNum).VRFTUOutletNodeNum) {
                            ShowSevereError(cCurrentModuleObject + " \"" + VRFTU(VRFTUNum).Name +
                                            "\" Heating coil outlet node name must be the same");
                            ShowContinueError("as the terminal unit air outlet node name when blow through fan is specified.");
                            ShowContinueError("... Heating coil outlet node      = " + DataLoopNode::NodeID(HCoilOutletNodeNum));
                            ShowContinueError("... Terminal Unit air outlet node = " + DataLoopNode::NodeID(VRFTU(VRFTUNum).VRFTUOutletNodeNum));
                            ErrorsFound = true;
                        }
                    } else if (VRFTU(VRFTUNum).SuppHeatingCoilPresent) {
                        if (VRFTU(VRFTUNum).SuppHeatCoilAirOutletNode != VRFTU(VRFTUNum).VRFTUOutletNodeNum) {
                            ShowSevereError(cCurrentModuleObject + " \"" + VRFTU(VRFTUNum).Name +
                                            "\" Supplemental Heating coil outlet node name must be the same");
                            ShowContinueError("as the terminal unit air outlet node name when blow through fan is specified.");
                            ShowContinueError("... Supplemental Heating coil outlet node = " +
                                              DataLoopNode::NodeID(VRFTU(VRFTUNum).SuppHeatCoilAirOutletNode));
                            ShowContinueError("... Terminal Unit air outlet node = " + DataLoopNode::NodeID(VRFTU(VRFTUNum).VRFTUOutletNodeNum));
                            ErrorsFound = true;
                        }
                    } else {
                        if (CCoilOutletNodeNum != VRFTU(VRFTUNum).VRFTUOutletNodeNum) {
                            ShowSevereError(cCurrentModuleObject + " \"" + VRFTU(VRFTUNum).Name +
                                            "\" Cooling coil outlet node name must be the same");
                            ShowContinueError(
                                "as the terminal unit air outlet node name when blow through fan is specified and no DX heating coil is present.");
                            ShowContinueError("... Cooling coil outlet node      = " + DataLoopNode::NodeID(CCoilOutletNodeNum));
                            ShowContinueError("... Terminal Unit air outlet node = " + DataLoopNode::NodeID(VRFTU(VRFTUNum).VRFTUOutletNodeNum));
                            ErrorsFound = true;
                        }
                    }
                } else if (VRFTU(VRFTUNum).HeatingCoilPresent) {
                    if (VRFTU(VRFTUNum).fanOutletNode != HCoilInletNodeNum) {
                        ShowSevereError(cCurrentModuleObject + " \"" + VRFTU(VRFTUNum).Name + "\" Fan outlet node name must be the same");
                        ShowContinueError(
                            "as the DX heating coil air inlet node name when blow through fan is specified and a DX cooling coil is not present.");
                        ShowContinueError("... Fan outlet node = " + DataLoopNode::NodeID(VRFTU(VRFTUNum).fanOutletNode));
                        ShowContinueError("... DX heating coil air inlet node = " + DataLoopNode::NodeID(HCoilInletNodeNum));
                        ErrorsFound = true;
                    }
                    if (VRFTU(VRFTUNum).SuppHeatingCoilPresent) {
                        if (VRFTU(VRFTUNum).SuppHeatCoilAirOutletNode != VRFTU(VRFTUNum).VRFTUOutletNodeNum) {
                            ShowSevereError(cCurrentModuleObject + " \"" + VRFTU(VRFTUNum).Name +
                                            "\" Supplemental Heating coil outlet node name must be the same");
                            ShowContinueError("as the terminal unit air outlet node name when blow through fan is specified.");
                            ShowContinueError("... Supplemental Heating coil outlet node = " +
                                              DataLoopNode::NodeID(VRFTU(VRFTUNum).SuppHeatCoilAirOutletNode));
                            ShowContinueError("... Terminal Unit air outlet node = " + DataLoopNode::NodeID(VRFTU(VRFTUNum).VRFTUOutletNodeNum));
                            ErrorsFound = true;
                        }
                    } else {
                        if (HCoilOutletNodeNum != VRFTU(VRFTUNum).VRFTUOutletNodeNum) {
                            ShowSevereError(cCurrentModuleObject + " \"" + VRFTU(VRFTUNum).Name +
                                            "\" Heating coil outlet node name must be the same");
                            ShowContinueError("as the terminal unit air outlet node name when blow through fan is specified.");
                            ShowContinueError("... Heating coil outlet node      = " + DataLoopNode::NodeID(HCoilOutletNodeNum));
                            ShowContinueError("... Terminal Unit air outlet node = " + DataLoopNode::NodeID(VRFTU(VRFTUNum).VRFTUOutletNodeNum));
                            ErrorsFound = true;
                        }
                    }
                }
            } else if (VRFTU(VRFTUNum).isInZone && VRFTU(VRFTUNum).FanPlace == DataHVACGlobals::DrawThru) {
                if (VRFTU(VRFTUNum).CoolingCoilPresent) {
                    if (!VRFTU(VRFTUNum).OAMixerUsed) {
                        if (VRFTU(VRFTUNum).VRFTUInletNodeNum != CCoilInletNodeNum) {
                            ShowSevereError(cCurrentModuleObject + " \"" + VRFTU(VRFTUNum).Name + "\" Cooling coil inlet node name must be the same");
                            ShowContinueError("as the terminal unit air inlet node name when draw through fan is specified.");
                            ShowContinueError("... Terminal unit air inlet node = " + DataLoopNode::NodeID(VRFTU(VRFTUNum).VRFTUInletNodeNum));
                            ShowContinueError("... DX cooling coil air inlet node = " + DataLoopNode::NodeID(CCoilInletNodeNum));
                            ErrorsFound = true;
                        }
                    }
                    if (VRFTU(VRFTUNum).HeatingCoilPresent) {
                        if (FanInletNodeNum != HCoilOutletNodeNum) {
                            ShowSevereError(cCurrentModuleObject + " \"" + VRFTU(VRFTUNum).Name + "\" Fan inlet node name must be the same");
                            ShowContinueError("as the DX heating coil air outlet node name when draw through fan is specified.");
                            ShowContinueError("... Fan inlet node = " + DataLoopNode::NodeID(FanInletNodeNum));
                            ShowContinueError("... DX heating coil air outlet node = " + DataLoopNode::NodeID(HCoilOutletNodeNum));
                            ErrorsFound = true;
                        }
                    } else {
                        if (FanInletNodeNum != CCoilOutletNodeNum) {
                            ShowSevereError(cCurrentModuleObject + " \"" + VRFTU(VRFTUNum).Name + "\" Fan inlet node name must be the same");
                            ShowContinueError("as the DX cooling coil air outlet node name when draw through fan is specified and a DX heating coil "
                                              "is not present.");
                            ShowContinueError("... Fan inlet node = " + DataLoopNode::NodeID(FanInletNodeNum));
                            ShowContinueError("... DX cooling coil air outlet node = " + DataLoopNode::NodeID(CCoilOutletNodeNum));
                            ErrorsFound = true;
                        }
                    }
                } else if (VRFTU(VRFTUNum).HeatingCoilPresent) {
                    if (FanInletNodeNum != HCoilOutletNodeNum) {
                        ShowSevereError(cCurrentModuleObject + " \"" + VRFTU(VRFTUNum).Name + "\" Fan inlet node name must be the same");
                        ShowContinueError("as the DX heating coil air outlet node name when draw through fan is specified.");
                        ShowContinueError("... Fan inlet node = " + DataLoopNode::NodeID(FanInletNodeNum));
                        ShowContinueError("... DX heating coil air outlet node = " + DataLoopNode::NodeID(HCoilOutletNodeNum));
                        ErrorsFound = true;
                    }
                }
                if (VRFTU(VRFTUNum).SuppHeatingCoilPresent) {
                    if (VRFTU(VRFTUNum).SuppHeatCoilAirOutletNode != VRFTU(VRFTUNum).VRFTUOutletNodeNum) {
                        ShowSevereError(cCurrentModuleObject + " \"" + VRFTU(VRFTUNum).Name +
                                        "\" Supplemental heating coil air outlet node name must be the same");
                        ShowContinueError("as the terminal unit air outlet node name when draw through fan is specified.");
                        ShowContinueError("... Supplemental heating coil air outlet node = " +
                                          DataLoopNode::NodeID(VRFTU(VRFTUNum).SuppHeatCoilAirOutletNode));
                        ShowContinueError("... Terminal unit air outlet node = " + DataLoopNode::NodeID(VRFTU(VRFTUNum).VRFTUOutletNodeNum));
                        ErrorsFound = true;
                    }
                } else {
                    if (VRFTU(VRFTUNum).fanOutletNode != VRFTU(VRFTUNum).VRFTUOutletNodeNum) {
                        ShowSevereError(cCurrentModuleObject + " \"" + VRFTU(VRFTUNum).Name + "\" Fan outlet node name must be the same");
                        ShowContinueError("as the terminal unit air outlet node name when draw through fan is specified.");
                        ShowContinueError("... Fan outlet node = " + DataLoopNode::NodeID(VRFTU(VRFTUNum).fanOutletNode));
                        ShowContinueError("... Terminal unit air outlet node = " + DataLoopNode::NodeID(VRFTU(VRFTUNum).VRFTUOutletNodeNum));
                        ErrorsFound = true;
                    }
                }
            }
            if (VRFTU(VRFTUNum).CoolingCoilPresent && VRFTU(VRFTUNum).HeatingCoilPresent) {
                if (CCoilOutletNodeNum != HCoilInletNodeNum) {
                    ShowSevereError(cCurrentModuleObject + " \"" + VRFTU(VRFTUNum).Name + "\" DX cooling coil air outlet node name must be the same");
                    ShowContinueError(" as the DX heating coil air inlet node name.");
                    ShowContinueError("... DX cooling coil air outlet node = " + DataLoopNode::NodeID(CCoilOutletNodeNum));
                    ShowContinueError("... DX heating coil air inlet node  = " + DataLoopNode::NodeID(HCoilInletNodeNum));
                    ErrorsFound = true;
                }
            }

            if (VRFTU(VRFTUNum).isInZone && VRFTU(VRFTUNum).ATMixerExists) {
                //   check that OA flow in cooling must be set to zero when connected to DOAS
                if (VRFTU(VRFTUNum).CoolOutAirVolFlow != 0) {
                    ShowWarningError(cCurrentModuleObject + " = " + VRFTU(VRFTUNum).Name);
                    ShowContinueError(".. Cooling Outdoor Air Flow Rate must be zero when " + cCurrentModuleObject);
                    ShowContinueError("..object is connected to central dedicated outdoor air system via AirTerminal:SingleDuct:Mixer");
                    ShowContinueError(".. Cooling Outdoor Air Flow Rate is set to 0 and simulation continues.");
                    VRFTU(VRFTUNum).CoolOutAirVolFlow = 0;
                }
                //   check that OA flow in heating must be set to zero when connected to DOAS
                if (VRFTU(VRFTUNum).HeatOutAirVolFlow != 0) {
                    ShowWarningError(cCurrentModuleObject + " = " + VRFTU(VRFTUNum).Name);
                    ShowContinueError(".. Heating Outdoor Air Flow Rate must be zero when " + cCurrentModuleObject);
                    ShowContinueError("..object is connected to central dedicated outdoor air system via AirTerminal:SingleDuct:Mixer");
                    ShowContinueError(".. Heating Outdoor Air Flow Rate is set to 0 and simulation continues.");
                    VRFTU(VRFTUNum).HeatOutAirVolFlow = 0;
                }
                //   check that OA flow in no cooling and no heating must be set to zero when connected to DOAS
                if (VRFTU(VRFTUNum).NoCoolHeatOutAirVolFlow != 0) {
                    ShowWarningError(cCurrentModuleObject + " = " + VRFTU(VRFTUNum).Name);
                    ShowContinueError(".. No Load Outdoor Air Flow Rate must be zero when " + cCurrentModuleObject);
                    ShowContinueError("..object is connected to central dedicated outdoor air system via AirTerminal:SingleDuct:Mixer");
                    ShowContinueError(".. No Load Outdoor Air Flow Rate is set to 0 and simulation continues.");
                    VRFTU(VRFTUNum).NoCoolHeatOutAirVolFlow = 0;
                }
            }
        } // IF(ZoneEquipmentListNotChecked)THEN

        // Size TU
        if (MySizeFlag(VRFTUNum)) {
            if (!ZoneSizingCalc && !SysSizingCalc) {
                SizeVRF(state, VRFTUNum);
                TerminalUnitList(TUListIndex).TerminalUnitNotSizedYet(IndexToTUInTUList) = false;
                MySizeFlag(VRFTUNum) = false;
            } // IF ( .NOT. ZoneSizingCalc) THEN
        }     // IF (MySizeFlag(VRFTUNum)) THEN

        // Do the Begin Environment initializations
        if (BeginEnvrnFlag && MyEnvrnFlag(VRFTUNum)) {

            // Change the Volume Flow Rates to Mass Flow Rates

            RhoAir = StdRhoAir;
            // set the mass flow rates from the input volume flow rates
            VRFTU(VRFTUNum).MaxCoolAirMassFlow = RhoAir * VRFTU(VRFTUNum).MaxCoolAirVolFlow;
            VRFTU(VRFTUNum).CoolOutAirMassFlow = RhoAir * VRFTU(VRFTUNum).CoolOutAirVolFlow;
            VRFTU(VRFTUNum).MaxHeatAirMassFlow = RhoAir * VRFTU(VRFTUNum).MaxHeatAirVolFlow;
            VRFTU(VRFTUNum).HeatOutAirMassFlow = RhoAir * VRFTU(VRFTUNum).HeatOutAirVolFlow;
            VRFTU(VRFTUNum).MaxNoCoolAirMassFlow = RhoAir * VRFTU(VRFTUNum).MaxNoCoolAirVolFlow;
            VRFTU(VRFTUNum).MaxNoHeatAirMassFlow = RhoAir * VRFTU(VRFTUNum).MaxNoHeatAirVolFlow;
            VRFTU(VRFTUNum).NoCoolHeatOutAirMassFlow = RhoAir * VRFTU(VRFTUNum).NoCoolHeatOutAirVolFlow;
            // set the node max and min mass flow rates
            // outside air mixer is optional, check that node num > 0
            if (OutsideAirNode > 0) {
                DataLoopNode::Node(OutsideAirNode).MassFlowRateMax = max(VRFTU(VRFTUNum).CoolOutAirMassFlow, VRFTU(VRFTUNum).HeatOutAirMassFlow);
                DataLoopNode::Node(OutsideAirNode).MassFlowRateMin = 0.0;
                DataLoopNode::Node(OutsideAirNode).MassFlowRateMinAvail = 0.0;
            }
            DataLoopNode::Node(OutNode).MassFlowRateMax = max(VRFTU(VRFTUNum).MaxCoolAirMassFlow, VRFTU(VRFTUNum).MaxHeatAirMassFlow);
            DataLoopNode::Node(OutNode).MassFlowRateMin = 0.0;
            DataLoopNode::Node(OutNode).MassFlowRateMinAvail = 0.0;
            DataLoopNode::Node(InNode).MassFlowRateMax = max(VRFTU(VRFTUNum).MaxCoolAirMassFlow, VRFTU(VRFTUNum).MaxHeatAirMassFlow);
            DataLoopNode::Node(InNode).MassFlowRateMin = 0.0;
            DataLoopNode::Node(InNode).MassFlowRateMinAvail = 0.0;
            if (VRFTU(VRFTUNum).VRFTUOAMixerRelNodeNum > 0) {
                DataLoopNode::Node(VRFTU(VRFTUNum).VRFTUOAMixerRelNodeNum).MassFlowRateMinAvail = 0.0;
            }

            MyEnvrnFlag(VRFTUNum) = false;

            if (VRF(VRFCond).CondenserType == DataHVACGlobals::WaterCooled) {
                rho = GetDensityGlycol(
                    state, PlantLoop(VRF(VRFCond).SourceLoopNum).FluidName, CWInitConvTemp, PlantLoop(VRF(VRFCond).SourceLoopNum).FluidIndex, RoutineName);
                VRF(VRFCond).WaterCondenserDesignMassFlow = VRF(VRFCond).WaterCondVolFlowRate * rho;

                InitComponentNodes(0.0,
                                   VRF(VRFCond).WaterCondenserDesignMassFlow,
                                   VRF(VRFCond).CondenserNodeNum,
                                   VRF(VRFCond).CondenserOutletNodeNum,
                                   VRF(VRFCond).SourceLoopNum,
                                   VRF(VRFCond).SourceLoopSideNum,
                                   VRF(VRFCond).SourceBranchNum,
                                   VRF(VRFCond).SourceCompNum);
            }
            //    IF(MyVRFCondFlag(VRFCond))THEN
            VRF(VRFCond).HRTimer = 0.0;
            VRF(VRFCond).ModeChange = false;
            VRF(VRFCond).HRModeChange = false;
            MyVRFCondFlag(VRFCond) = false;
            //    END IF

            if (VRFTU(VRFTUNum).SuppHeatCoilFluidInletNode > 0) {
                if (VRFTU(VRFTUNum).SuppHeatCoilType_Num == DataHVACGlobals::Coil_HeatingWater) {
                    if (VRFTU(VRFTUNum).SuppHeatCoilFluidMaxFlow == DataSizing::AutoSize) {
                        WaterCoils::SimulateWaterCoilComponents(
                            state, VRFTU(VRFTUNum).SuppHeatCoilName, FirstHVACIteration, VRFTU(VRFTUNum).SuppHeatCoilIndex);
                        // design hot water volume flow rate
                        Real64 CoilMaxVolFlowRate =
                            WaterCoils::GetCoilMaxWaterFlowRate(state, "Coil:Heating:Water", VRFTU(VRFTUNum).SuppHeatCoilName, ErrorsFound);
                        if (CoilMaxVolFlowRate != DataSizing::AutoSize) {
                            rho = GetDensityGlycol(state,
                                                   PlantLoop(VRFTU(VRFTUNum).SuppHeatCoilLoopNum).FluidName,
                                                   DataGlobals::HWInitConvTemp,
                                                   PlantLoop(VRFTU(VRFTUNum).SuppHeatCoilLoopNum).FluidIndex,
                                                   RoutineName);
                            VRFTU(VRFTUNum).SuppHeatCoilFluidMaxFlow = CoilMaxVolFlowRate * rho;
                        }
                    }
                }

                if (VRFTU(VRFTUNum).SuppHeatCoilType_Num == DataHVACGlobals::Coil_HeatingSteam) {
                    if (VRFTU(VRFTUNum).SuppHeatCoilFluidMaxFlow == DataSizing::AutoSize) {
                        SteamCoils::SimulateSteamCoilComponents(
                            state, VRFTU(VRFTUNum).SuppHeatCoilName, FirstHVACIteration, VRFTU(VRFTUNum).SuppHeatCoilIndex, 1.0);
                        // design steam volume flow rate
                        Real64 CoilMaxVolFlowRate = SteamCoils::GetCoilMaxSteamFlowRate(state, VRFTU(VRFTUNum).SuppHeatCoilIndex, ErrorsFound);
                        if (CoilMaxVolFlowRate != DataSizing::AutoSize) {
                            int SteamIndex = 0; // fluid type index of 0 is passed if steam
                            Real64 TempSteamIn = 100.0;
                            Real64 SteamDensity = FluidProperties::GetSatDensityRefrig(state, fluidNameSteam, TempSteamIn, 1.0, SteamIndex, RoutineName);
                            VRFTU(VRFTUNum).SuppHeatCoilFluidMaxFlow = CoilMaxVolFlowRate * SteamDensity;
                        }
                    }
                }
                // init water/steam coils min and max flow rates
                InitComponentNodes(0.0,
                                   VRFTU(VRFTUNum).SuppHeatCoilFluidMaxFlow,
                                   VRFTU(VRFTUNum).SuppHeatCoilFluidInletNode,
                                   VRFTU(VRFTUNum).SuppHeatCoilFluidOutletNode,
                                   VRFTU(VRFTUNum).SuppHeatCoilLoopNum,
                                   VRFTU(VRFTUNum).SuppHeatCoilLoopSide,
                                   VRFTU(VRFTUNum).SuppHeatCoilBranchNum,
                                   VRFTU(VRFTUNum).SuppHeatCoilCompNum);
            }

            // the first time an air loop VRF TU is simulated set isSimulated = true so that the TU initialization
            // will occur with the first TU simulated this time step. Zone VRF TUs are called during sizing which, if air
            // loop TUs are included, alters when all TUs appear to have been simulated. Also, BeginEnvrnFlag is true multiple
            // times during the simulation, reset each time to avoid a different order during sizing and simulation
            if (TerminalUnitList(VRFTU(VRFTUNum).TUListIndex).reset_isSimulatedFlags) {
                // if no TUs are in the air loop or outdoor air system they will all be simulated during ManageZoneEquipment
                // and there is no need to adjust the order of simulation (i.e., when isSimulated are all true for a given system)
                if (VRFTU(VRFTUNum).isInAirLoop || VRFTU(VRFTUNum).isInOASys) {
                    TerminalUnitList(VRFTU(VRFTUNum).TUListIndex).IsSimulated = true;
                    TerminalUnitList(VRFTU(VRFTUNum).TUListIndex).reset_isSimulatedFlags = false;
                }
            }

        } // IF (BeginEnvrnFlag .and. MyEnvrnFlag(VRFTUNum)) THEN

        // reset environment flag for next environment
        if (!BeginEnvrnFlag) {
            MyEnvrnFlag(VRFTUNum) = true;
            MyVRFCondFlag(VRFCond) = true;
            TerminalUnitList(VRFTU(VRFTUNum).TUListIndex).reset_isSimulatedFlags = true;
        }

        // If all VRF Terminal Units on this VRF AC System have been simulated, reset the IsSimulated flag
        // The condenser will be simulated after all terminal units have been simulated (see Sub SimulateVRF)
        if (all(TerminalUnitList(TUListIndex).IsSimulated)) {
            //   this should be the first time through on the next iteration. All TU's and condenser have been simulated.
            //   reset simulation flag for each terminal unit
            TerminalUnitList(TUListIndex).IsSimulated = false;
            //     after all TU's have been simulated, reset operating mode flag if necessary
            if (LastModeHeating(VRFCond) && CoolingLoad(VRFCond)) {
                LastModeCooling(VRFCond) = true;
                LastModeHeating(VRFCond) = false;
                //        SwitchedMode(VRFCond)    = .TRUE.
            }
            if (LastModeCooling(VRFCond) && HeatingLoad(VRFCond)) {
                LastModeHeating(VRFCond) = true;
                LastModeCooling(VRFCond) = false;
                //        SwitchedMode(VRFCond)    = .TRUE.
            }
        } // IF(ALL(TerminalUnitList(VRFTU(VRFTUNum)%TUListIndex)%IsSimulated))THEN

        // get operating capacity of water and steam coil
        if (FirstHVACIteration) {
            if (VRFTU(VRFTUNum).SuppHeatCoilFluidInletNode > 0) {
                if (VRFTU(VRFTUNum).SuppHeatCoilType_Num == DataHVACGlobals::Coil_HeatingWater) {
                    //     set hot water full flow rate for sizing
                    Real64 mdot = VRFTU(VRFTUNum).SuppHeatCoilFluidMaxFlow;
                    PlantUtilities::SetComponentFlowRate(mdot,
                                                         VRFTU(VRFTUNum).SuppHeatCoilFluidInletNode,
                                                         VRFTU(VRFTUNum).SuppHeatCoilFluidOutletNode,
                                                         VRFTU(VRFTUNum).SuppHeatCoilLoopNum,
                                                         VRFTU(VRFTUNum).SuppHeatCoilLoopSide,
                                                         VRFTU(VRFTUNum).SuppHeatCoilBranchNum,
                                                         VRFTU(VRFTUNum).SuppHeatCoilCompNum);

                    //     simulate water coil to find operating capacity
                    WaterCoils::SimulateWaterCoilComponents(
                        state, VRFTU(VRFTUNum).SuppHeatCoilName, FirstHVACIteration, VRFTU(VRFTUNum).SuppHeatCoilIndex, SuppHeatCoilCapacity);
                    VRFTU(VRFTUNum).DesignSuppHeatingCapacity = SuppHeatCoilCapacity;
                } // from iF VRFTU(VRFTUNum).SuppHeatCoilType_Num == DataHVACGlobals::Coil_HeatingWater
                if (VRFTU(VRFTUNum).SuppHeatCoilType_Num == DataHVACGlobals::Coil_HeatingSteam) {
                    //     set hot water full flow rate for sizing
                    Real64 mdot = VRFTU(VRFTUNum).SuppHeatCoilFluidMaxFlow;
                    PlantUtilities::SetComponentFlowRate(mdot,
                                                         VRFTU(VRFTUNum).SuppHeatCoilFluidInletNode,
                                                         VRFTU(VRFTUNum).SuppHeatCoilFluidOutletNode,
                                                         VRFTU(VRFTUNum).SuppHeatCoilLoopNum,
                                                         VRFTU(VRFTUNum).SuppHeatCoilLoopSide,
                                                         VRFTU(VRFTUNum).SuppHeatCoilBranchNum,
                                                         VRFTU(VRFTUNum).SuppHeatCoilCompNum);

                    //     simulate steam coil to find operating capacity
                    SteamCoils::SimulateSteamCoilComponents(state,
                                                            VRFTU(VRFTUNum).SuppHeatCoilName,
                                                            FirstHVACIteration,
                                                            VRFTU(VRFTUNum).SuppHeatCoilIndex,
                                                            1.0,
                                                            ErrorsFound); // QCoilReq, simulate any load > 0 to get max capacity of steam coil
                    SuppHeatCoilCapacity = SteamCoils::GetCoilCapacity(state, "Coil:Heating:Steam", VRFTU(VRFTUNum).SuppHeatCoilName, ErrorsFound);

                    VRFTU(VRFTUNum).DesignSuppHeatingCapacity = SuppHeatCoilCapacity;
                } // from if VRFTU( VRFTUNum ).SuppHeatCoilType_Num == DataHVACGlobals::Coil_HeatingSteam
            }
        }
        // initialize water/steam coil inlet flow rate to zero
        if (VRFTU(VRFTUNum).SuppHeatCoilFluidInletNode > 0) {
            Real64 mdot = 0.0;
            PlantUtilities::SetComponentFlowRate(mdot,
                                                 VRFTU(VRFTUNum).SuppHeatCoilFluidInletNode,
                                                 VRFTU(VRFTUNum).SuppHeatCoilFluidOutletNode,
                                                 VRFTU(VRFTUNum).SuppHeatCoilLoopNum,
                                                 VRFTU(VRFTUNum).SuppHeatCoilLoopSide,
                                                 VRFTU(VRFTUNum).SuppHeatCoilBranchNum,
                                                 VRFTU(VRFTUNum).SuppHeatCoilCompNum);
        }

        // one-time checks of flow rate vs fan flow rate
        if (MyVRFFlag(VRFTUNum)) {
            if (!ZoneSizingCalc && !SysSizingCalc) {
                if (VRFTU(VRFTUNum).FanPlace > 0) {
                    if (VRFTU(VRFTUNum).ActualFanVolFlowRate != AutoSize) {

                        if (VRFTU(VRFTUNum).MaxCoolAirVolFlow > VRFTU(VRFTUNum).ActualFanVolFlowRate) {
                            ShowWarningError("InitVRF: VRF Terminal Unit = [" + DataHVACGlobals::cVRFTUTypes(VRFTU(VRFTUNum).VRFTUType_Num) + ", \"" +
                                             VRFTU(VRFTUNum).Name + "\"]");
                            ShowContinueError("... has Supply Air Flow Rate During Cooling Operation > Max Fan Volume Flow Rate, should be <=");
                            ShowContinueError("... Supply Air Flow Rate During Cooling Operation = " +
                                              RoundSigDigits(VRFTU(VRFTUNum).MaxCoolAirVolFlow, 4) + " m3/s");
                            ShowContinueError("... Max Fan Volume Flow Rate                      = " +
                                              RoundSigDigits(VRFTU(VRFTUNum).ActualFanVolFlowRate, 4) + " m3/s");
                            ShowContinueError(
                                "...the supply air flow rate during cooling operation will be reduced to match and the simulation continues.");
                            VRFTU(VRFTUNum).MaxCoolAirVolFlow = VRFTU(VRFTUNum).ActualFanVolFlowRate;
                        }

                        if (VRFTU(VRFTUNum).MaxNoCoolAirVolFlow > VRFTU(VRFTUNum).ActualFanVolFlowRate) {
                            ShowWarningError("InitVRF: VRF Terminal Unit = [" + DataHVACGlobals::cVRFTUTypes(VRFTU(VRFTUNum).VRFTUType_Num) + ", \"" +
                                             VRFTU(VRFTUNum).Name + "\"]");
                            ShowContinueError("... has Supply Air Flow Rate When No Cooling is Needed > Max Fan Volume Flow Rate, should be <=");
                            ShowContinueError("... Supply Air Flow Rate When No Cooling is Needed = " +
                                              RoundSigDigits(VRFTU(VRFTUNum).MaxNoCoolAirVolFlow, 4) + " m3/s");
                            ShowContinueError("... Max Fan Volume Flow Rate                       = " +
                                              RoundSigDigits(VRFTU(VRFTUNum).ActualFanVolFlowRate, 4) + " m3/s");
                            ShowContinueError(
                                "...the supply air flow rate when no cooling is needed will be reduced to match and the simulation continues.");
                            VRFTU(VRFTUNum).MaxNoCoolAirVolFlow = VRFTU(VRFTUNum).ActualFanVolFlowRate;
                        }

                        if (VRFTU(VRFTUNum).CoolOutAirVolFlow > VRFTU(VRFTUNum).MaxCoolAirVolFlow) {
                            ShowWarningError("InitVRF: VRF Terminal Unit = [" + DataHVACGlobals::cVRFTUTypes(VRFTU(VRFTUNum).VRFTUType_Num) + ", \"" +
                                             VRFTU(VRFTUNum).Name + "\"]");
                            ShowContinueError(
                                "...The Outdoor Air Flow Rate During Cooling Operation exceeds the Supply Air Flow Rate During Cooling Operation.");
                            ShowContinueError("...Outdoor Air Flow Rate During Cooling Operation = " +
                                              RoundSigDigits(VRFTU(VRFTUNum).CoolOutAirVolFlow, 4) + " m3/s");
                            ShowContinueError("... Supply Air Flow Rate During Cooling Operation = " +
                                              RoundSigDigits(VRFTU(VRFTUNum).MaxCoolAirVolFlow, 4) + " m3/s");
                            ShowContinueError("...the outdoor air flow rate will be reduced to match and the simulation continues.");
                            VRFTU(VRFTUNum).CoolOutAirVolFlow = VRFTU(VRFTUNum).MaxCoolAirVolFlow;
                        }

                        if (VRFTU(VRFTUNum).MaxHeatAirVolFlow > VRFTU(VRFTUNum).ActualFanVolFlowRate) {
                            ShowWarningError("InitVRF: VRF Terminal Unit = [" + DataHVACGlobals::cVRFTUTypes(VRFTU(VRFTUNum).VRFTUType_Num) + ", \"" +
                                             VRFTU(VRFTUNum).Name + "\"]");
                            ShowContinueError("... has Supply Air Flow Rate During Heating Operation > Max Fan Volume Flow Rate, should be <=");
                            ShowContinueError("... Supply Air Flow Rate During Heating Operation = " +
                                              RoundSigDigits(VRFTU(VRFTUNum).MaxHeatAirVolFlow, 4) + " m3/s");
                            ShowContinueError("... Max Fan Volume Flow Rate                      = " +
                                              RoundSigDigits(VRFTU(VRFTUNum).ActualFanVolFlowRate, 4) + " m3/s");
                            ShowContinueError(
                                "...the supply air flow rate during cooling operation will be reduced to match and the simulation continues.");
                            VRFTU(VRFTUNum).MaxHeatAirVolFlow = VRFTU(VRFTUNum).ActualFanVolFlowRate;
                        }

                        if (VRFTU(VRFTUNum).MaxNoHeatAirVolFlow > VRFTU(VRFTUNum).ActualFanVolFlowRate) {
                            ShowWarningError("InitVRF: VRF Terminal Unit = [" + DataHVACGlobals::cVRFTUTypes(VRFTU(VRFTUNum).VRFTUType_Num) + ", \"" +
                                             VRFTU(VRFTUNum).Name + "\"]");
                            ShowContinueError("... has Supply Air Flow Rate When No Heating is Needed > Max Fan Volume Flow Rate, should be <=");
                            ShowContinueError("... Supply Air Flow Rate When No Heating is Needed = " +
                                              RoundSigDigits(VRFTU(VRFTUNum).MaxNoHeatAirVolFlow, 4) + " m3/s");
                            ShowContinueError("... Max Fan Volume Flow Rate                       = " +
                                              RoundSigDigits(VRFTU(VRFTUNum).ActualFanVolFlowRate, 4) + " m3/s");
                            ShowContinueError(
                                "...the supply air flow rate when no cooling is needed will be reduced to match and the simulation continues.");
                            VRFTU(VRFTUNum).MaxNoHeatAirVolFlow = VRFTU(VRFTUNum).ActualFanVolFlowRate;
                        }

                        if (VRFTU(VRFTUNum).HeatOutAirVolFlow > VRFTU(VRFTUNum).MaxHeatAirVolFlow) {
                            ShowWarningError("InitVRF: VRF Terminal Unit = [" + DataHVACGlobals::cVRFTUTypes(VRFTU(VRFTUNum).VRFTUType_Num) + ", \"" +
                                             VRFTU(VRFTUNum).Name + "\"]");
                            ShowContinueError(
                                "...The Outdoor Air Flow Rate During Heating Operation exceeds the Supply Air Flow Rate During Heating Operation.");
                            ShowContinueError("...Outdoor Air Flow Rate During Heating Operation = " +
                                              RoundSigDigits(VRFTU(VRFTUNum).HeatOutAirVolFlow, 4) + " m3/s");
                            ShowContinueError("... Supply Air Flow Rate During Heating Operation = " +
                                              RoundSigDigits(VRFTU(VRFTUNum).MaxHeatAirVolFlow, 4) + " m3/s");
                            ShowContinueError("...the outdoor air flow rate will be reduced to match and the simulation continues.");
                            VRFTU(VRFTUNum).HeatOutAirVolFlow = VRFTU(VRFTUNum).MaxHeatAirVolFlow;
                        }

                        if (VRFTU(VRFTUNum).NoCoolHeatOutAirVolFlow > VRFTU(VRFTUNum).ActualFanVolFlowRate) {
                            ShowWarningError("InitVRF: VRF Terminal Unit = [" + DataHVACGlobals::cVRFTUTypes(VRFTU(VRFTUNum).VRFTUType_Num) + ", \"" +
                                             VRFTU(VRFTUNum).Name + "\"]");
                            ShowContinueError(
                                "... has a Outdoor Air Flow Rate When No Cooling or Heating is Needed > Max Fan Volume Flow Rate, should be <=");
                            ShowContinueError("... Outdoor Air Flow Rate When No Cooling or Heating is Needed = " +
                                              RoundSigDigits(VRFTU(VRFTUNum).NoCoolHeatOutAirVolFlow, 4) + " m3/s");
                            ShowContinueError("... Max Fan Volume Flow Rate                                   = " +
                                              RoundSigDigits(VRFTU(VRFTUNum).ActualFanVolFlowRate, 4) + " m3/s");
                            ShowContinueError("...the outdoor air flow rate when no cooling or heating is needed will be reduced to match and the "
                                              "simulation continues.");
                            VRFTU(VRFTUNum).NoCoolHeatOutAirVolFlow = VRFTU(VRFTUNum).ActualFanVolFlowRate;
                        }

                        if (VRFTU(VRFTUNum).ActualFanVolFlowRate > 0.0) {
                            VRFTU(VRFTUNum).HeatingSpeedRatio = VRFTU(VRFTUNum).MaxHeatAirVolFlow / VRFTU(VRFTUNum).ActualFanVolFlowRate;
                            VRFTU(VRFTUNum).CoolingSpeedRatio = VRFTU(VRFTUNum).MaxCoolAirVolFlow / VRFTU(VRFTUNum).ActualFanVolFlowRate;
                        }

                        MyVRFFlag(VRFTUNum) = false;
                    } else {
                        if (VRFTU(VRFTUNum).fanType_Num == DataHVACGlobals::FanType_SystemModelObject) {
                            VRFTU(VRFTUNum).ActualFanVolFlowRate = HVACFan::fanObjs[VRFTU(VRFTUNum).FanIndex]->designAirVolFlowRate;
                        } else {
                            GetFanVolFlow(VRFTU(VRFTUNum).FanIndex, VRFTU(VRFTUNum).ActualFanVolFlowRate);
                        }
                    }
                } else {
                    MyVRFFlag(VRFTUNum) = false;
                }
            }
        } // IF(MyVRFFlag(VRFTUNum))THEN

        // calculate end time of current time step to determine if max capacity reset is required
        CurrentEndTime = CurrentTime + DataHVACGlobals::SysTimeElapsed;

        // Initialize the maximum allowed terminal unit capacity. Total terminal unit capacity must not
        // exceed the available condenser capacity. This variable is used to limit the terminal units
        // providing more capacity than allowed. Example: TU loads are 1-ton, 2-ton, 3-ton, and 4-ton connected
        // to a condenser having only 9-tons available. This variable will be set to 3-tons and the 4-ton
        // terminal unit will be limited to 3-tons (see SimVRFCondenser where this variable is calculated).
        if (CurrentEndTime > CurrentEndTimeLast || TimeStepSysLast > DataHVACGlobals::TimeStepSys ||
            (FirstHVACIteration && MyBeginTimeStepFlag(VRFCond))) {
            MaxCoolingCapacity(VRFCond) = MaxCap;
            MaxHeatingCapacity(VRFCond) = MaxCap;
            MyBeginTimeStepFlag(VRFCond) = false;
        }

        if (!FirstHVACIteration) MyBeginTimeStepFlag(VRFCond) = true;

        // Do the following initializations (every time step).

        TimeStepSysLast = DataHVACGlobals::TimeStepSys;
        CurrentEndTimeLast = CurrentEndTime;

        if (VRFTU(VRFTUNum).FanOpModeSchedPtr > 0) {
            if (GetCurrentScheduleValue(VRFTU(VRFTUNum).FanOpModeSchedPtr) == 0.0) {
                VRFTU(VRFTUNum).OpMode = DataHVACGlobals::CycFanCycCoil;
            } else {
                VRFTU(VRFTUNum).OpMode = DataHVACGlobals::ContFanCycCoil;
            }
        }

        // if condenser is off, all terminal unit coils are off
        if (GetCurrentScheduleValue(VRF(VRFCond).SchedPtr) == 0.0) {
            HeatingLoad(VRFCond) = false;
            CoolingLoad(VRFCond) = false;
        } else {

            //*** Operating Mode Initialization done at beginning of each iteration ***!
            //*** assumes all TU's and Condenser were simulated last iteration ***!
            //*** this code is done ONCE each iteration when all TU's IsSimulated flag is FALSE ***!
            // Determine operating mode prior to simulating any terminal units connected to a VRF condenser
            // this should happen at the beginning of a time step where all TU's are polled to see what
            // mode the heat pump condenser will operate in
            if (!any(TerminalUnitList(TUListIndex).IsSimulated)) {
                InitializeOperatingMode(state, FirstHVACIteration, VRFCond, TUListIndex, OnOffAirFlowRatio);
            }
            //*** End of Operating Mode Initialization done at beginning of each iteration ***!

            // disable VRF system when outside limits of operation based on OAT
            EnableSystem = false; // flag used to switch operating modes when OAT is outside operating limits
            if (CoolingLoad(VRFCond)) {
                if ((OutsideDryBulbTemp < VRF(VRFCond).MinOATCooling || OutsideDryBulbTemp > VRF(VRFCond).MaxOATCooling) &&
                    any(TerminalUnitList(TUListIndex).CoolingCoilPresent)) {
                    CoolingLoad(VRFCond) = false;
                    // test if heating load exists, account for thermostat control type
                    {
                        auto const SELECT_CASE_var(VRF(VRFCond).ThermostatPriority);
                        if ((SELECT_CASE_var == LoadPriority) || (SELECT_CASE_var == ZonePriority)) {
                            if (SumHeatingLoads(VRFCond) > 0.0) EnableSystem = true;
                        } else if (SELECT_CASE_var == ThermostatOffsetPriority) {
                            if (MinDeltaT(VRFCond) < 0.0) EnableSystem = true;
                        } else if ((SELECT_CASE_var == ScheduledPriority) || (SELECT_CASE_var == MasterThermostatPriority)) {
                            // can't switch modes if scheduled (i.e., would be switching to unscheduled mode)
                            // or master TSTAT used (i.e., master zone only has a specific load - can't switch)
                        } else {
                        }
                    }
                    if (EnableSystem) {
                        if ((OutsideDryBulbTemp >= VRF(VRFCond).MinOATHeating && OutsideDryBulbTemp <= VRF(VRFCond).MaxOATHeating) &&
                            any(TerminalUnitList(TUListIndex).HeatingCoilPresent)) {
                            HeatingLoad(VRFCond) = true;
                        } else {
                            if (any(TerminalUnitList(TUListIndex).CoolingCoilAvailable)) {
                                if (VRF(VRFCond).CoolingMaxTempLimitIndex == 0) {
                                    ShowWarningMessage(cVRFTypes(VRF(VRFCond).VRFSystemTypeNum) + " \"" + VRF(VRFCond).Name + "\".");
                                    ShowContinueError("...InitVRF: VRF Heat Pump Min/Max Operating Temperature in Cooling Mode Limits have been "
                                                      "exceeded and VRF system is disabled.");
                                    if (VRF(VRFCond).CondenserType == DataHVACGlobals::WaterCooled) {
                                        ShowContinueError("... Outdoor Unit Inlet Water Temperature           = " +
                                                          TrimSigDigits(OutsideDryBulbTemp, 3));
                                    } else {
                                        ShowContinueError("... Outdoor Unit Inlet Air Temperature                 = " +
                                                          TrimSigDigits(OutsideDryBulbTemp, 3));
                                    }
                                    ShowContinueError("... Cooling Minimum Outdoor Unit Inlet Temperature = " +
                                                      TrimSigDigits(VRF(VRFCond).MinOATCooling, 3));
                                    ShowContinueError("... Cooling Maximum Outdoor Unit Inlet Temperature = " +
                                                      TrimSigDigits(VRF(VRFCond).MaxOATCooling, 3));
                                    ShowContinueErrorTimeStamp("... Check VRF Heat Pump Min/Max Outdoor Temperature in Cooling Mode limits.");
                                }
                                ShowRecurringWarningErrorAtEnd(
                                    cVRFTypes(VRF(VRFCond).VRFSystemTypeNum) + " \"" + VRF(VRFCond).Name +
                                        "\" -- Exceeded VRF Heat Pump min/max cooling temperature limit error continues...",
                                    VRF(VRFCond).CoolingMaxTempLimitIndex,
                                    OutsideDryBulbTemp,
                                    OutsideDryBulbTemp);
                            }
                        }
                    } else {
                        if (any(TerminalUnitList(TUListIndex).CoolingCoilAvailable)) {
                            if (VRF(VRFCond).CoolingMaxTempLimitIndex == 0) {
                                ShowWarningMessage(cVRFTypes(VRF(VRFCond).VRFSystemTypeNum) + " \"" + VRF(VRFCond).Name + "\".");
                                ShowContinueError("...InitVRF: VRF Heat Pump Min/Max Operating Temperature in Cooling Mode Limits have been exceeded "
                                                  "and VRF system is disabled.");
                                if (VRF(VRFCond).CondenserType == DataHVACGlobals::WaterCooled) {
                                    ShowContinueError("... Outdoor Unit Inlet Water Temperature           = " + TrimSigDigits(OutsideDryBulbTemp, 3));
                                } else {
                                    ShowContinueError("... Outdoor Unit Inlet Air Temperature                 = " +
                                                      TrimSigDigits(OutsideDryBulbTemp, 3));
                                }
                                ShowContinueError("... Cooling Minimum Outdoor Unit Inlet Temperature = " +
                                                  TrimSigDigits(VRF(VRFCond).MinOATCooling, 3));
                                ShowContinueError("... Cooling Maximum Outdoor Unit Inlet Temperature = " +
                                                  TrimSigDigits(VRF(VRFCond).MaxOATCooling, 3));
                                ShowContinueErrorTimeStamp("... Check VRF Heat Pump Min/Max Outdoor Temperature in Cooling Mode limits.");
                            }
                            ShowRecurringWarningErrorAtEnd(cVRFTypes(VRF(VRFCond).VRFSystemTypeNum) + " \"" + VRF(VRFCond).Name +
                                                               "\" -- Exceeded VRF Heat Pump min/max cooling temperature limit error continues...",
                                                           VRF(VRFCond).CoolingMaxTempLimitIndex,
                                                           OutsideDryBulbTemp,
                                                           OutsideDryBulbTemp);
                        }
                    }
                }
            } else if (HeatingLoad(VRFCond)) {
                if ((OutsideDryBulbTemp < VRF(VRFCond).MinOATHeating || OutsideDryBulbTemp > VRF(VRFCond).MaxOATHeating) &&
                    any(TerminalUnitList(TUListIndex).HeatingCoilPresent)) {
                    HeatingLoad(VRFCond) = false;
                    // test if cooling load exists, account for thermostat control type
                    {
                        auto const SELECT_CASE_var(VRF(VRFCond).ThermostatPriority);
                        if ((SELECT_CASE_var == LoadPriority) || (SELECT_CASE_var == ZonePriority)) {
                            if (SumCoolingLoads(VRFCond) < 0.0) EnableSystem = true;
                        } else if (SELECT_CASE_var == ThermostatOffsetPriority) {
                            if (MaxDeltaT(VRFCond) > 0.0) EnableSystem = true;
                        } else if ((SELECT_CASE_var == ScheduledPriority) || (SELECT_CASE_var == MasterThermostatPriority)) {
                        } else {
                        }
                    }
                    if (EnableSystem) {
                        if ((OutsideDryBulbTemp >= VRF(VRFCond).MinOATCooling && OutsideDryBulbTemp <= VRF(VRFCond).MaxOATCooling) &&
                            any(TerminalUnitList(TUListIndex).CoolingCoilPresent)) {
                            CoolingLoad(VRFCond) = true;
                        } else {
                            if (any(TerminalUnitList(TUListIndex).HeatingCoilAvailable)) {
                                if (VRF(VRFCond).HeatingMaxTempLimitIndex == 0) {
                                    ShowWarningMessage(cVRFTypes(VRF(VRFCond).VRFSystemTypeNum) + " \"" + VRF(VRFCond).Name + "\".");
                                    ShowContinueError("...InitVRF: VRF Heat Pump Min/Max Operating Temperature in Heating Mode Limits have been "
                                                      "exceeded and VRF system is disabled.");
                                    if (VRF(VRFCond).CondenserType == DataHVACGlobals::WaterCooled) {
                                        ShowContinueError("... Outdoor Unit Inlet Water Temperature           = " +
                                                          TrimSigDigits(OutsideDryBulbTemp, 3));
                                    } else {
                                        ShowContinueError("... Outdoor Unit Inlet Air Temperature             = " +
                                                          TrimSigDigits(OutsideDryBulbTemp, 3));
                                    }
                                    ShowContinueError("... Heating Minimum Outdoor Unit Inlet Temperature = " +
                                                      TrimSigDigits(VRF(VRFCond).MinOATHeating, 3));
                                    ShowContinueError("... Heating Maximum Outdoor Unit Inlet Temperature = " +
                                                      TrimSigDigits(VRF(VRFCond).MaxOATHeating, 3));
                                    ShowContinueErrorTimeStamp("... Check VRF Heat Pump Min/Max Outdoor Temperature in Heating Mode limits.");
                                }
                                ShowRecurringWarningErrorAtEnd(
                                    cVRFTypes(VRF(VRFCond).VRFSystemTypeNum) + " \"" + VRF(VRFCond).Name +
                                        "\" -- Exceeded VRF Heat Pump min/max heating temperature limit error continues...",
                                    VRF(VRFCond).HeatingMaxTempLimitIndex,
                                    OutsideDryBulbTemp,
                                    OutsideDryBulbTemp);
                            }
                        }
                    } else {
                        if (any(TerminalUnitList(TUListIndex).HeatingCoilAvailable)) {
                            if (VRF(VRFCond).HeatingMaxTempLimitIndex == 0) {
                                ShowWarningMessage(cVRFTypes(VRF(VRFCond).VRFSystemTypeNum) + " \"" + VRF(VRFCond).Name + "\".");
                                ShowContinueError("...InitVRF: VRF Heat Pump Min/Max Operating Temperature in Heating Mode Limits have been exceeded "
                                                  "and VRF system is disabled.");
                                if (VRF(VRFCond).CondenserType == DataHVACGlobals::WaterCooled) {
                                    ShowContinueError("... Outdoor Unit Inlet Water Temperature           = " + TrimSigDigits(OutsideDryBulbTemp, 3));
                                } else {
                                    ShowContinueError("... Outdoor Unit Inlet Air Temperature             = " + TrimSigDigits(OutsideDryBulbTemp, 3));
                                }
                                ShowContinueError("... Heating Minimum Outdoor Unit Inlet Temperature = " +
                                                  TrimSigDigits(VRF(VRFCond).MinOATHeating, 3));
                                ShowContinueError("... Heating Maximum Outdoor Unit Inlet Temperature = " +
                                                  TrimSigDigits(VRF(VRFCond).MaxOATHeating, 3));
                                ShowContinueErrorTimeStamp("... Check VRF Heat Pump Min/Max Outdoor Temperature in Heating Mode limits.");
                            }
                            ShowRecurringWarningErrorAtEnd(cVRFTypes(VRF(VRFCond).VRFSystemTypeNum) + " \"" + VRF(VRFCond).Name +
                                                               "\" -- Exceeded VRF Heat Pump min/max heating temperature limit error continues...",
                                                           VRF(VRFCond).HeatingMaxTempLimitIndex,
                                                           OutsideDryBulbTemp,
                                                           OutsideDryBulbTemp);
                        }
                    }
                }
            }

        } // IF (GetCurrentScheduleValue(VRF(VRFCond)%SchedPtr) .EQ. 0.0) THEN

        // initialize terminal unit flow rate
        if (HeatingLoad(VRFCond) || (VRF(VRFCond).HeatRecoveryUsed && TerminalUnitList(TUListIndex).HRHeatRequest(IndexToTUInTUList))) {
            if (VRFTU(VRFTUNum).OAMixerUsed) {
                DataLoopNode::Node(VRFTU(VRFTUNum).VRFTUOAMixerRetNodeNum).MassFlowRate = VRFTU(VRFTUNum).MaxHeatAirMassFlow;
                DataLoopNode::Node(OutsideAirNode).MassFlowRate = VRFTU(VRFTUNum).HeatOutAirMassFlow;
            } else {
                if (!VRFTU(VRFTUNum).isInOASys) DataLoopNode::Node(InNode).MassFlowRate = VRFTU(VRFTUNum).MaxHeatAirMassFlow;
            }
        } else if (CoolingLoad(VRFCond) || (VRF(VRFCond).HeatRecoveryUsed && TerminalUnitList(TUListIndex).HRCoolRequest(IndexToTUInTUList))) {
            if (VRFTU(VRFTUNum).OAMixerUsed) {
                DataLoopNode::Node(VRFTU(VRFTUNum).VRFTUOAMixerRetNodeNum).MassFlowRate = VRFTU(VRFTUNum).MaxCoolAirMassFlow;
                DataLoopNode::Node(OutsideAirNode).MassFlowRate = VRFTU(VRFTUNum).CoolOutAirMassFlow;
            } else {
                if (!VRFTU(VRFTUNum).isInOASys) DataLoopNode::Node(InNode).MassFlowRate = VRFTU(VRFTUNum).MaxCoolAirMassFlow;
            }
        } else {
            if (LastModeCooling(VRFCond)) {
                if (VRFTU(VRFTUNum).OAMixerUsed) {
                    DataLoopNode::Node(VRFTU(VRFTUNum).VRFTUOAMixerRetNodeNum).MassFlowRate = VRFTU(VRFTUNum).MaxNoCoolAirMassFlow;
                    DataLoopNode::Node(OutsideAirNode).MassFlowRate = VRFTU(VRFTUNum).NoCoolHeatOutAirMassFlow;
                } else {
                    if (!VRFTU(VRFTUNum).isInOASys) DataLoopNode::Node(InNode).MassFlowRate = VRFTU(VRFTUNum).MaxNoCoolAirMassFlow;
                }
            } else if (LastModeHeating(VRFCond)) {
                if (VRFTU(VRFTUNum).OAMixerUsed) {
                    DataLoopNode::Node(VRFTU(VRFTUNum).VRFTUOAMixerRetNodeNum).MassFlowRate = VRFTU(VRFTUNum).MaxNoHeatAirMassFlow;
                    DataLoopNode::Node(OutsideAirNode).MassFlowRate = VRFTU(VRFTUNum).NoCoolHeatOutAirMassFlow;
                } else {
                    if (!VRFTU(VRFTUNum).isInOASys) DataLoopNode::Node(InNode).MassFlowRate = VRFTU(VRFTUNum).MaxNoHeatAirMassFlow;
                }
            }
        }

        if (VRFTU(VRFTUNum).ATMixerExists) {
            // There is an air terminal mixer
            if (VRFTU(VRFTUNum).ATMixerType == DataHVACGlobals::ATMixer_InletSide) { // if there is an inlet side air terminal mixer
                // set the primary air inlet mass flow rate
                DataLoopNode::Node(VRFTU(VRFTUNum).ATMixerPriNode).MassFlowRate =
                    min(DataLoopNode::Node(VRFTU(VRFTUNum).ATMixerPriNode).MassFlowRateMaxAvail,
                        DataLoopNode::Node(VRFTU(VRFTUNum).VRFTUInletNodeNum).MassFlowRate);
                // now calculate the the mixer outlet air conditions (and the secondary air inlet flow rate). The mixer outlet flow rate has already
                // been set above (it is the "inlet" node flow rate)
                SimATMixer(state, VRFTU(VRFTUNum).ATMixerName, FirstHVACIteration, VRFTU(VRFTUNum).ATMixerIndex);
            }
        } else {
            if (VRFTU(VRFTUNum).OAMixerUsed) SimOAMixer(state, VRFTU(VRFTUNum).OAMixerName, FirstHVACIteration, VRFTU(VRFTUNum).OAMixerIndex);
        }
        OnOffAirFlowRatio = 1.0;

        // these flags are used in Subroutine CalcVRF to turn on the correct coil (heating or cooling)
        // valid operating modes
        // Heat Pump (heat recovery flags are set to FALSE):
        // CoolingLoad(VRFCond) - TU can only operate in this mode if heat recovery is not used and there is a cooling load
        // HeatingLoad(VRFCond) - TU can only operate in this mode if heat recovery is not used and there is a heating load
        // Heat Recovery (heat pump flags are set same as for Heat Pump operation):
        // TerminalUnitList(TUListIndex)%HRCoolRequest(IndexToTUInTUList) - TU will operate in this mode if heat recovery is used
        // TerminalUnitList(TUListIndex)%HRHeatRequest(IndexToTUInTUList) - TU will operate in this mode if heat recovery is used

        getVRFTUZoneLoad(VRFTUNum, QZnReq, LoadToHeatingSP, LoadToCoolingSP, false);

        if (std::abs(QZnReq) < DataHVACGlobals::SmallLoad) QZnReq = 0.0;
        // set initial terminal unit operating mode for heat recovery
        // operating mode for non-heat recovery set above using CoolingLoad(VRFCond) or HeatingLoad(VRFCond) variables
        // first turn off terminal unit
        TerminalUnitList(TUListIndex).HRCoolRequest(IndexToTUInTUList) = false;
        TerminalUnitList(TUListIndex).HRHeatRequest(IndexToTUInTUList) = false;
        // then set according to LoadToXXXXingSP variables
        if (LoadToCoolingSP < -1.0 * DataHVACGlobals::SmallLoad) {
            if (CoolingLoad(VRFCond) || HeatingLoad(VRFCond)) { // don't allow heat recovery if control logic dictates unit is off
                TerminalUnitList(TUListIndex).HRCoolRequest(IndexToTUInTUList) = true;
            }
        }
        if (LoadToHeatingSP > DataHVACGlobals::SmallLoad) {
            if (CoolingLoad(VRFCond) || HeatingLoad(VRFCond)) { // don't allow heat recovery if control logic dictates unit is off
                TerminalUnitList(TUListIndex).HRHeatRequest(IndexToTUInTUList) = true;
            }
        }
        if (LoadToCoolingSP > 0.0 && LoadToHeatingSP < 0.0) QZnReq = 0.0;

        // next check for overshoot when constant fan mode is used
        // check operating load to see if OA will overshoot setpoint temperature when constant fan mode is used
        if ((VRFTU(VRFTUNum).OpMode == DataHVACGlobals::ContFanCycCoil || VRFTU(VRFTUNum).ATMixerExists) && !VRFTU(VRFTUNum).isSetPointControlled) {
            SetCompFlowRate(VRFTUNum, VRFCond, true);

            if (VRF(VRFCond).VRFAlgorithmTypeNum == AlgorithmTypeFluidTCtrl) {
                // Algorithm Type: VRF model based on physics, applicable for Fluid Temperature Control
                VRFTU(VRFTUNum).CalcVRF_FluidTCtrl(state, VRFTUNum, FirstHVACIteration, 0.0, TempOutput, OnOffAirFlowRatio, SuppHeatCoilLoad);
            } else {
                // Algorithm Type: VRF model based on system curve
                VRFTU(VRFTUNum).CalcVRF(state, VRFTUNum, FirstHVACIteration, 0.0, TempOutput, OnOffAirFlowRatio, SuppHeatCoilLoad);
            }

            // If the Terminal Unit has a net cooling capacity (TempOutput < 0) and
            // the zone temp is above the Tstat heating setpoint (QToHeatSetPt < 0)
            // see if the terminal unit operation will exceed the setpoint
            // 4 tests here to cover all possibilities:
            // IF(TempOutput < 0.0d0 .AND. LoadToHeatingSP .LT. 0.0d0)THEN
            // ELSE IF(TempOutput .GT. 0.0d0 .AND. LoadToCoolingSP .GT. 0.0d0)THEN
            // ELSE IF(TempOutput .GT. 0.0d0 .AND. LoadToCoolingSP .LT. 0.0d0)THEN
            // ELSE IF(TempOutput < 0.0d0 .AND. LoadToHeatingSP .GT. 0.0d0)THEN
            // END IF
            // could compress these to 2 complex IF's but logic inside each would get more complex
            if (TempOutput < 0.0 && LoadToHeatingSP < 0.0) {
                // If the net cooling capacity overshoots the heating setpoint count as heating load
                if (TempOutput < LoadToHeatingSP) {
                    // Don't count as heating load unless mode is allowed. Also check for floating zone.
                    if (TempControlType(VRFTU(VRFTUNum).ZoneNum) != DataHVACGlobals::SingleCoolingSetPoint &&
                        TempControlType(VRFTU(VRFTUNum).ZoneNum) != 0) {
                        if (!LastModeHeating(VRFCond)) {
                            // system last operated in cooling mode, change air flows and repeat coil off capacity test
                            if (VRFTU(VRFTUNum).OAMixerUsed) {
                                DataLoopNode::Node(VRFTU(VRFTUNum).VRFTUOAMixerRetNodeNum).MassFlowRate = VRFTU(VRFTUNum).MaxHeatAirMassFlow;
                                DataLoopNode::Node(VRFTU(VRFTUNum).VRFTUOAMixerOANodeNum).MassFlowRate = VRFTU(VRFTUNum).HeatOutAirMassFlow;
                                SimOAMixer(state, VRFTU(VRFTUNum).OAMixerName, FirstHVACIteration, VRFTU(VRFTUNum).OAMixerIndex);
                            } else {
                                DataLoopNode::Node(InNode).MassFlowRate = VRFTU(VRFTUNum).MaxHeatAirMassFlow;
                            }

                            if (VRF(VRFCond).VRFAlgorithmTypeNum == AlgorithmTypeFluidTCtrl) {
                                // Algorithm Type: VRF model based on physics, applicable for Fluid Temperature Control
                                VRFTU(VRFTUNum).CalcVRF_FluidTCtrl(
                                    state, VRFTUNum, FirstHVACIteration, 0.0, TempOutput, OnOffAirFlowRatio, SuppHeatCoilLoad);
                            } else {
                                // Algorithm Type: VRF model based on system curve
                                VRFTU(VRFTUNum).CalcVRF(state, VRFTUNum, FirstHVACIteration, 0.0, TempOutput, OnOffAirFlowRatio, SuppHeatCoilLoad);
                            }

                            // if zone temp will overshoot, pass the LoadToHeatingSP as the load to meet
                            if (TempOutput < LoadToHeatingSP) {
                                QZnReq = LoadToHeatingSP;
                                TerminalUnitList(TUListIndex).HRHeatRequest(IndexToTUInTUList) = true;
                                TerminalUnitList(TUListIndex).HRCoolRequest(IndexToTUInTUList) = false;
                            }
                        } else {
                            // last mode was heating, zone temp will overshoot heating setpoint, reset QznReq to LoadtoHeatingSP
                            QZnReq = LoadToHeatingSP;
                            TerminalUnitList(TUListIndex).HRHeatRequest(IndexToTUInTUList) = true;
                            TerminalUnitList(TUListIndex).HRCoolRequest(IndexToTUInTUList) = false;
                        }
                    }
                } else if (TempOutput > LoadToCoolingSP && LoadToCoolingSP < 0.0) {
                    //       If the net cooling capacity does not meet the zone cooling load enable cooling
                    QZnReq = LoadToCoolingSP;
                    TerminalUnitList(TUListIndex).HRHeatRequest(IndexToTUInTUList) = false;
                    TerminalUnitList(TUListIndex).HRCoolRequest(IndexToTUInTUList) = true;
                } else if (TempOutput < LoadToCoolingSP && LoadToCoolingSP < 0.0) {
                    //       If the net cooling capacity meets the zone cooling load but does not overshoot heating setpoint
                    QZnReq = 0.0;
                    TerminalUnitList(TUListIndex).HRHeatRequest(IndexToTUInTUList) = false;
                    TerminalUnitList(TUListIndex).HRCoolRequest(IndexToTUInTUList) = false;
                }
                //     If the terminal unit has a net heating capacity and the zone temp is below the Tstat cooling setpoint
                //     see if the terminal unit operation will exceed the setpoint
            } else if (TempOutput > 0.0 && LoadToCoolingSP > 0.0) {
                //       If the net heating capacity overshoots the cooling setpoint count as cooling load
                if (TempOutput > LoadToCoolingSP) {
                    //         Don't count as cooling load unless mode is allowed. Also check for floating zone.
                    if (TempControlType(VRFTU(VRFTUNum).ZoneNum) != DataHVACGlobals::SingleHeatingSetPoint &&
                        TempControlType(VRFTU(VRFTUNum).ZoneNum) != 0) {
                        if (!LastModeCooling(VRFCond)) {
                            if (VRFTU(VRFTUNum).OAMixerUsed) {
                                DataLoopNode::Node(VRFTU(VRFTUNum).VRFTUOAMixerRetNodeNum).MassFlowRate = VRFTU(VRFTUNum).MaxCoolAirMassFlow;
                                DataLoopNode::Node(VRFTU(VRFTUNum).VRFTUOAMixerOANodeNum).MassFlowRate = VRFTU(VRFTUNum).CoolOutAirMassFlow;
                                SimOAMixer(state, VRFTU(VRFTUNum).OAMixerName, FirstHVACIteration, VRFTU(VRFTUNum).OAMixerIndex);
                            } else {
                                DataLoopNode::Node(VRFTU(VRFTUNum).VRFTUInletNodeNum).MassFlowRate = VRFTU(VRFTUNum).MaxCoolAirMassFlow;
                            }

                            if (VRF(VRFCond).VRFAlgorithmTypeNum == AlgorithmTypeFluidTCtrl) {
                                // Algorithm Type: VRF model based on physics, applicable for Fluid Temperature Control
                                VRFTU(VRFTUNum).CalcVRF_FluidTCtrl(
                                    state, VRFTUNum, FirstHVACIteration, 0.0, TempOutput, OnOffAirFlowRatio, SuppHeatCoilLoad);
                            } else {
                                // Algorithm Type: VRF model based on system curve
                                VRFTU(VRFTUNum).CalcVRF(state, VRFTUNum, FirstHVACIteration, 0.0, TempOutput, OnOffAirFlowRatio, SuppHeatCoilLoad);
                            }

                            // if zone temp will overshoot, pass the LoadToCoolingSP as the load to meet
                            if (TempOutput > LoadToCoolingSP) {
                                QZnReq = LoadToCoolingSP;
                                TerminalUnitList(TUListIndex).HRCoolRequest(IndexToTUInTUList) = true;
                                TerminalUnitList(TUListIndex).HRHeatRequest(IndexToTUInTUList) = false;
                            }
                        } else {
                            QZnReq = LoadToCoolingSP;
                            TerminalUnitList(TUListIndex).HRCoolRequest(IndexToTUInTUList) = true;
                            TerminalUnitList(TUListIndex).HRHeatRequest(IndexToTUInTUList) = false;
                        }
                    }
                } else if (TempOutput < LoadToHeatingSP) {
                    //         Don't count as heating load unless mode is allowed. Also check for floating zone.
                    if (TempControlType(VRFTU(VRFTUNum).ZoneNum) != DataHVACGlobals::SingleCoolingSetPoint &&
                        TempControlType(VRFTU(VRFTUNum).ZoneNum) != 0) {
                        if (!LastModeHeating(VRFCond)) {
                            if (VRFTU(VRFTUNum).OAMixerUsed) {
                                DataLoopNode::Node(VRFTU(VRFTUNum).VRFTUOAMixerRetNodeNum).MassFlowRate = VRFTU(VRFTUNum).MaxHeatAirMassFlow;
                                DataLoopNode::Node(VRFTU(VRFTUNum).VRFTUOAMixerOANodeNum).MassFlowRate = VRFTU(VRFTUNum).HeatOutAirMassFlow;
                                SimOAMixer(state, VRFTU(VRFTUNum).OAMixerName, FirstHVACIteration, VRFTU(VRFTUNum).OAMixerIndex);
                            } else {
                                DataLoopNode::Node(VRFTU(VRFTUNum).VRFTUInletNodeNum).MassFlowRate = VRFTU(VRFTUNum).MaxHeatAirMassFlow;
                            }

                            if (VRF(VRFCond).VRFAlgorithmTypeNum == AlgorithmTypeFluidTCtrl) {
                                // Algorithm Type: VRF model based on physics, applicable for Fluid Temperature Control
                                VRFTU(VRFTUNum).CalcVRF_FluidTCtrl(
                                    state, VRFTUNum, FirstHVACIteration, 0.0, TempOutput, OnOffAirFlowRatio, SuppHeatCoilLoad);
                            } else {
                                // Algorithm Type: VRF model based on system curve
                                VRFTU(VRFTUNum).CalcVRF(state, VRFTUNum, FirstHVACIteration, 0.0, TempOutput, OnOffAirFlowRatio, SuppHeatCoilLoad);
                            }

                            // if zone temp will overshoot, pass the LoadToHeatingSP as the load to meet
                            if (TempOutput < LoadToHeatingSP) {
                                QZnReq = LoadToHeatingSP;
                                TerminalUnitList(TUListIndex).HRHeatRequest(IndexToTUInTUList) = true;
                                TerminalUnitList(TUListIndex).HRCoolRequest(IndexToTUInTUList) = false;
                            }
                        } else {
                            QZnReq = LoadToHeatingSP;
                            TerminalUnitList(TUListIndex).HRHeatRequest(IndexToTUInTUList) = true;
                            TerminalUnitList(TUListIndex).HRCoolRequest(IndexToTUInTUList) = false;
                        }
                    }
                } else if (TempOutput > LoadToHeatingSP && TempOutput < LoadToCoolingSP) {
                    //         If the net capacity does not overshoot either setpoint
                    QZnReq = 0.0;
                    TerminalUnitList(TUListIndex).HRHeatRequest(IndexToTUInTUList) = false;
                    TerminalUnitList(TUListIndex).HRCoolRequest(IndexToTUInTUList) = false;
                } else {
                    //         If the net heating capacity meets the zone heating load but does not overshoot cooling setpoint
                    QZnReq = 0.0;
                    TerminalUnitList(TUListIndex).HRHeatRequest(IndexToTUInTUList) = false;
                    TerminalUnitList(TUListIndex).HRCoolRequest(IndexToTUInTUList) = false;
                }
                //     If the terminal unit has a net heating capacity and the zone temp is above the Tstat cooling setpoint
                //     see if the terminal unit operation will exceed the setpoint
            } else if (TempOutput > 0.0 && LoadToCoolingSP < 0.0) {
                //       If the net heating capacity overshoots the cooling setpoint count as cooling load
                //       Don't count as cooling load unless mode is allowed. Also check for floating zone.
                if (TempControlType(VRFTU(VRFTUNum).ZoneNum) != DataHVACGlobals::SingleHeatingSetPoint &&
                    TempControlType(VRFTU(VRFTUNum).ZoneNum) != 0) {
                    if (!LastModeCooling(VRFCond)) {
                        if (VRFTU(VRFTUNum).OAMixerUsed) {
                            DataLoopNode::Node(VRFTU(VRFTUNum).VRFTUOAMixerRetNodeNum).MassFlowRate = VRFTU(VRFTUNum).MaxCoolAirMassFlow;
                            DataLoopNode::Node(VRFTU(VRFTUNum).VRFTUOAMixerOANodeNum).MassFlowRate = VRFTU(VRFTUNum).CoolOutAirMassFlow;
                            SimOAMixer(state, VRFTU(VRFTUNum).OAMixerName, FirstHVACIteration, VRFTU(VRFTUNum).OAMixerIndex);
                        } else {
                            DataLoopNode::Node(VRFTU(VRFTUNum).VRFTUInletNodeNum).MassFlowRate = VRFTU(VRFTUNum).MaxCoolAirMassFlow;
                        }

                        if (VRF(VRFCond).VRFAlgorithmTypeNum == AlgorithmTypeFluidTCtrl) {
                            // Algorithm Type: VRF model based on physics, applicable for Fluid Temperature Control
                            VRFTU(VRFTUNum).CalcVRF_FluidTCtrl(
                                state, VRFTUNum, FirstHVACIteration, 0.0, TempOutput, OnOffAirFlowRatio, SuppHeatCoilLoad);
                        } else {
                            // Algorithm Type: VRF model based on system curve
                            VRFTU(VRFTUNum).CalcVRF(state, VRFTUNum, FirstHVACIteration, 0.0, TempOutput, OnOffAirFlowRatio, SuppHeatCoilLoad);
                        }

                        // if zone temp will overshoot, pass the LoadToCoolingSP as the load to meet
                        if (TempOutput > LoadToCoolingSP) {
                            QZnReq = LoadToCoolingSP;
                            TerminalUnitList(TUListIndex).HRCoolRequest(IndexToTUInTUList) = true;
                            TerminalUnitList(TUListIndex).HRHeatRequest(IndexToTUInTUList) = false;
                        }
                        // last mode was cooling, zone temp will overshoot cooling setpoint, reset QznReq to LoadtoCoolingSP
                    } else {
                        QZnReq = LoadToCoolingSP;
                        TerminalUnitList(TUListIndex).HRCoolRequest(IndexToTUInTUList) = true;
                        TerminalUnitList(TUListIndex).HRHeatRequest(IndexToTUInTUList) = false;
                    }
                }
                // If the Terminal Unit has a net cooling capacity (TempOutput < 0) and
                // the zone temp is below the Tstat heating setpoint (QToHeatSetPt > 0)
                // see if the terminal unit operation will exceed the setpoint
            } else if (TempOutput < 0.0 && LoadToHeatingSP > 0.0) {
                // Don't count as heating load unless mode is allowed. Also check for floating zone.
                if (TempControlType(VRFTU(VRFTUNum).ZoneNum) != DataHVACGlobals::SingleCoolingSetPoint &&
                    TempControlType(VRFTU(VRFTUNum).ZoneNum) != 0) {
                    if (!LastModeHeating(VRFCond)) {
                        // system last operated in cooling mode, change air flows and repeat coil off capacity test
                        if (VRFTU(VRFTUNum).OAMixerUsed) {
                            DataLoopNode::Node(VRFTU(VRFTUNum).VRFTUOAMixerRetNodeNum).MassFlowRate = VRFTU(VRFTUNum).MaxHeatAirMassFlow;
                            DataLoopNode::Node(VRFTU(VRFTUNum).VRFTUOAMixerOANodeNum).MassFlowRate = VRFTU(VRFTUNum).HeatOutAirMassFlow;
                            SimOAMixer(state, VRFTU(VRFTUNum).OAMixerName, FirstHVACIteration, VRFTU(VRFTUNum).OAMixerIndex);
                        } else {
                            DataLoopNode::Node(InNode).MassFlowRate = VRFTU(VRFTUNum).MaxHeatAirMassFlow;
                        }

                        if (VRF(VRFCond).VRFAlgorithmTypeNum == AlgorithmTypeFluidTCtrl) {
                            // Algorithm Type: VRF model based on physics, applicable for Fluid Temperature Control
                            VRFTU(VRFTUNum).CalcVRF_FluidTCtrl(
                                state, VRFTUNum, FirstHVACIteration, 0.0, TempOutput, OnOffAirFlowRatio, SuppHeatCoilLoad);
                        } else {
                            // Algorithm Type: VRF model based on system curve
                            VRFTU(VRFTUNum).CalcVRF(state, VRFTUNum, FirstHVACIteration, 0.0, TempOutput, OnOffAirFlowRatio, SuppHeatCoilLoad);
                        }

                        // if zone temp will overshoot, pass the LoadToHeatingSP as the load to meet
                        if (TempOutput < LoadToHeatingSP) {
                            QZnReq = LoadToHeatingSP;
                            TerminalUnitList(TUListIndex).HRHeatRequest(IndexToTUInTUList) = true;
                            TerminalUnitList(TUListIndex).HRCoolRequest(IndexToTUInTUList) = false;
                        }
                    } else {
                        // last mode was heating, zone temp will overshoot heating setpoint, reset QznReq to LoadtoHeatingSP
                        QZnReq = LoadToHeatingSP;
                        TerminalUnitList(TUListIndex).HRHeatRequest(IndexToTUInTUList) = true;
                        TerminalUnitList(TUListIndex).HRCoolRequest(IndexToTUInTUList) = false;
                    }
                }
            }
            // test that the system is active if constant fan logic enables system when thermostat control logic did not
            if (!CoolingLoad(VRFCond) && !HeatingLoad(VRFCond)) {
                if (TerminalUnitList(TUListIndex).HRCoolRequest(IndexToTUInTUList) && OutsideDryBulbTemp >= VRF(VRFCond).MinOATCooling &&
                    OutsideDryBulbTemp <= VRF(VRFCond).MaxOATCooling) {
                    CoolingLoad(VRFCond) = true;
                } else if (TerminalUnitList(TUListIndex).HRHeatRequest(IndexToTUInTUList) && OutsideDryBulbTemp >= VRF(VRFCond).MinOATHeating &&
                           OutsideDryBulbTemp <= VRF(VRFCond).MaxOATHeating) {
                    HeatingLoad(VRFCond) = true;
                }
            }
        } // IF(VRFTU(VRFTUNum)%OpMode == DataHVACGlobals::ContFanCycCoil)THEN

        if (VRF(VRFCond).HeatRecoveryUsed) {
            if (OutsideDryBulbTemp < VRF(VRFCond).MinOATHeatRecovery || OutsideDryBulbTemp > VRF(VRFCond).MaxOATHeatRecovery) {
                if ((any(TerminalUnitList(TUListIndex).HRCoolRequest) && HeatingLoad(VRFCond)) ||
                    (any(TerminalUnitList(TUListIndex).HRHeatRequest) && CoolingLoad(VRFCond))) {
                    if (VRF(VRFCond).HRMaxTempLimitIndex == 0) {
                        ShowWarningMessage(cVRFTypes(VRF(VRFCond).VRFSystemTypeNum) + " \"" + VRF(VRFCond).Name + "\".");
                        ShowContinueError("...InitVRF: VRF Heat Pump Min/Max Outdoor Temperature in Heat Recovery Mode Limits have been exceeded and "
                                          "VRF heat recovery is disabled.");
                        ShowContinueError("... Outdoor Dry-Bulb Temperature                       = " + TrimSigDigits(OutsideDryBulbTemp, 3));
                        ShowContinueError("... Heat Recovery Minimum Outdoor Dry-Bulb Temperature = " +
                                          TrimSigDigits(VRF(VRFCond).MinOATHeatRecovery, 3));
                        ShowContinueError("... Heat Recovery Maximum Outdoor Dry-Bulb Temperature = " +
                                          TrimSigDigits(VRF(VRFCond).MaxOATHeatRecovery, 3));
                        ShowContinueErrorTimeStamp("... Check VRF Heat Pump Min/Max Outdoor Temperature in Heat Recovery Mode limits.");
                        ShowContinueError("...the system will operate in heat pump mode when applicable.");
                    }
                    ShowRecurringWarningErrorAtEnd(cVRFTypes(VRF(VRFCond).VRFSystemTypeNum) + " \"" + VRF(VRFCond).Name +
                                                       "\" -- Exceeded VRF Heat Recovery min/max outdoor temperature limit error continues...",
                                                   VRF(VRFCond).HRMaxTempLimitIndex,
                                                   OutsideDryBulbTemp,
                                                   OutsideDryBulbTemp);
                }
                // Allow heat pump mode to operate if within limits
                if (OutsideDryBulbTemp < VRF(VRFCond).MinOATCooling || OutsideDryBulbTemp > VRF(VRFCond).MaxOATCooling) {
                    // Disable cooling mode only, heating model will still be allowed
                    TerminalUnitList(TUListIndex).HRCoolRequest(IndexToTUInTUList) = false;
                }
                if (OutsideDryBulbTemp < VRF(VRFCond).MinOATHeating || OutsideDryBulbTemp > VRF(VRFCond).MaxOATHeating) {
                    // Disable heating mode only, cooling model will still be allowed
                    TerminalUnitList(TUListIndex).HRHeatRequest(IndexToTUInTUList) = false;
                }
            }
        } else {
            TerminalUnitList(TUListIndex).HRHeatRequest = false;
            TerminalUnitList(TUListIndex).HRCoolRequest = false;
        }

        // Override operating mode when using EMS
        // this logic seems suspect, uses a "just run it on" mentality. Nee to test using EMS.
        if (VRF(VRFCond).EMSOverrideHPOperatingMode) {
            if (VRF(VRFCond).EMSValueForHPOperatingMode == 0.0) { // Off
                HeatingLoad(VRFCond) = false;
                CoolingLoad(VRFCond) = false;
                TerminalUnitList(TUListIndex).HRHeatRequest(IndexToTUInTUList) = false;
                TerminalUnitList(TUListIndex).HRCoolRequest(IndexToTUInTUList) = false;
            } else if (VRF(VRFCond).EMSValueForHPOperatingMode == 1.0) { // Cooling
                HeatingLoad(VRFCond) = false;
                CoolingLoad(VRFCond) = true;
                QZnReq = LoadToCoolingSP;
                if (VRF(VRFCond).HeatRecoveryUsed) {
                    TerminalUnitList(TUListIndex).HRHeatRequest(IndexToTUInTUList) = false;
                    TerminalUnitList(TUListIndex).HRCoolRequest(IndexToTUInTUList) = true;
                }
            } else if (VRF(VRFCond).EMSValueForHPOperatingMode == 2.0) { // Heating
                HeatingLoad(VRFCond) = true;
                CoolingLoad(VRFCond) = false;
                QZnReq = LoadToHeatingSP;
                if (VRF(VRFCond).HeatRecoveryUsed) {
                    TerminalUnitList(TUListIndex).HRHeatRequest(IndexToTUInTUList) = true;
                    TerminalUnitList(TUListIndex).HRCoolRequest(IndexToTUInTUList) = false;
                }
            } else {
                if (VRF(VRFCond).HPOperatingModeErrorIndex == 0) {
                    ShowWarningMessage(cVRFTypes(VRF(VRFCond).VRFSystemTypeNum) + " \"" + VRF(VRFCond).Name + "\".");
                    ShowContinueError("...InitVRF: Illegal HP operating mode = " + TrimSigDigits(VRF(VRFCond).EMSValueForHPOperatingMode, 0));
                    ShowContinueError("...InitVRF: VRF HP operating mode will not be controlled by EMS.");
                }
                ShowRecurringWarningErrorAtEnd(cVRFTypes(VRF(VRFCond).VRFSystemTypeNum) + " \"" + VRF(VRFCond).Name +
                                                   "\" -- Illegal HP operating mode error continues...",
                                               VRF(VRFCond).HPOperatingModeErrorIndex,
                                               VRF(VRFCond).EMSValueForHPOperatingMode,
                                               VRF(VRFCond).EMSValueForHPOperatingMode);
            }
        }

        // set the TU flow rate. Check for heat recovery operation first, these will be FALSE if HR is not used.
        if (TerminalUnitList(TUListIndex).HRCoolRequest(IndexToTUInTUList)) {
            CompOnMassFlow = VRFTU(VRFTUNum).MaxCoolAirMassFlow;
            CompOffMassFlow = VRFTU(VRFTUNum).MaxNoCoolAirMassFlow;
            OACompOnMassFlow = VRFTU(VRFTUNum).CoolOutAirMassFlow;
            OACompOffMassFlow = VRFTU(VRFTUNum).NoCoolHeatOutAirMassFlow;
        } else if (TerminalUnitList(TUListIndex).HRHeatRequest(IndexToTUInTUList)) {
            CompOnMassFlow = VRFTU(VRFTUNum).MaxHeatAirMassFlow;
            CompOffMassFlow = VRFTU(VRFTUNum).MaxNoHeatAirMassFlow;
            OACompOnMassFlow = VRFTU(VRFTUNum).HeatOutAirMassFlow;
            OACompOffMassFlow = VRFTU(VRFTUNum).NoCoolHeatOutAirMassFlow;
        } else if (CoolingLoad(VRFCond) && QZnReq != 0.0) {
            CompOnMassFlow = VRFTU(VRFTUNum).MaxCoolAirMassFlow;
            CompOffMassFlow = VRFTU(VRFTUNum).MaxNoCoolAirMassFlow;
            OACompOnMassFlow = VRFTU(VRFTUNum).CoolOutAirMassFlow;
            OACompOffMassFlow = VRFTU(VRFTUNum).NoCoolHeatOutAirMassFlow;
        } else if (HeatingLoad(VRFCond) && QZnReq != 0.0) {
            CompOnMassFlow = VRFTU(VRFTUNum).MaxHeatAirMassFlow;
            CompOffMassFlow = VRFTU(VRFTUNum).MaxNoHeatAirMassFlow;
            OACompOnMassFlow = VRFTU(VRFTUNum).HeatOutAirMassFlow;
            OACompOffMassFlow = VRFTU(VRFTUNum).NoCoolHeatOutAirMassFlow;
        } else {
            if (LastModeCooling(VRFCond)) {
                CompOnMassFlow = VRFTU(VRFTUNum).MaxNoCoolAirMassFlow;
                CompOffMassFlow = VRFTU(VRFTUNum).MaxNoCoolAirMassFlow;
                OACompOnMassFlow = VRFTU(VRFTUNum).CoolOutAirMassFlow;
            }
            if (LastModeHeating(VRFCond)) {
                CompOnMassFlow = VRFTU(VRFTUNum).MaxNoHeatAirMassFlow;
                CompOffMassFlow = VRFTU(VRFTUNum).MaxNoHeatAirMassFlow;
                OACompOnMassFlow = VRFTU(VRFTUNum).HeatOutAirMassFlow;
            }
            OACompOffMassFlow = VRFTU(VRFTUNum).NoCoolHeatOutAirMassFlow;
        }

        if (VRFTU(VRFTUNum).OpMode == DataHVACGlobals::CycFanCycCoil) {
            CompOffMassFlow = 0.0;
            OACompOffMassFlow = 0.0;
        }

        SetAverageAirFlow(VRFTUNum, 0.0, OnOffAirFlowRatio);

        if (ErrorsFound) {
            ShowFatalError(
                RoutineName +
                ": Errors found in getting ZoneHVAC:TerminalUnit:VariableRefrigerantFlow system input. Preceding condition(s) causes termination.");
        }
    }

    void SetCompFlowRate(int const VRFTUNum, int const VRFCond, Optional_bool_const UseCurrentMode)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Richard Raustad, FSEC
        //       DATE WRITTEN   June 2011
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine is for calling VRF terminal units during Init to initialize flow rate
        // while looping through all terminal units connected to a specific condenser.
        // This allows polling of capacities for all terminal units.
        // Since the heat pump can only heat or cool, a single operating mode is chosen for each condenser.

        // METHODOLOGY EMPLOYED:
        // Initializes flow rates for a specific terminal unit.

        bool CurrentMode;      // - specifies whether current or previous operating mode is used
        int IndexToTUInTUList; // - index to TU in specific list for this VRF system
        int TUListIndex;       // index to TU list for this VRF system

        IndexToTUInTUList = VRFTU(VRFTUNum).IndexToTUInTUList;
        TUListIndex = VRFTU(VRFTUNum).TUListIndex;
        if (present(UseCurrentMode)) {
            CurrentMode = UseCurrentMode;
        } else {
            CurrentMode = false;
        }

        // uses current operating mode to set flow rate (after mode is set)
        if (TerminalUnitList(TUListIndex).HRCoolRequest(IndexToTUInTUList)) {
            CompOnMassFlow = VRFTU(VRFTUNum).MaxCoolAirMassFlow;
            CompOffMassFlow = VRFTU(VRFTUNum).MaxNoCoolAirMassFlow;
            OACompOnMassFlow = VRFTU(VRFTUNum).CoolOutAirMassFlow;
            OACompOffMassFlow = VRFTU(VRFTUNum).NoCoolHeatOutAirMassFlow;
        } else if (TerminalUnitList(TUListIndex).HRHeatRequest(IndexToTUInTUList)) {
            CompOnMassFlow = VRFTU(VRFTUNum).MaxHeatAirMassFlow;
            CompOffMassFlow = VRFTU(VRFTUNum).MaxNoHeatAirMassFlow;
            OACompOnMassFlow = VRFTU(VRFTUNum).HeatOutAirMassFlow;
            OACompOffMassFlow = VRFTU(VRFTUNum).NoCoolHeatOutAirMassFlow;
        } else if (CurrentMode) { // uses current operating mode to set flow rate (after mode is set)
            if (CoolingLoad(VRFCond)) {
                CompOnMassFlow = VRFTU(VRFTUNum).MaxCoolAirMassFlow;
                CompOffMassFlow = VRFTU(VRFTUNum).MaxNoCoolAirMassFlow;
                OACompOnMassFlow = VRFTU(VRFTUNum).CoolOutAirMassFlow;
                OACompOffMassFlow = VRFTU(VRFTUNum).NoCoolHeatOutAirMassFlow;
            } else if (HeatingLoad(VRFCond)) {
                CompOnMassFlow = VRFTU(VRFTUNum).MaxHeatAirMassFlow;
                CompOffMassFlow = VRFTU(VRFTUNum).MaxNoHeatAirMassFlow;
                OACompOnMassFlow = VRFTU(VRFTUNum).HeatOutAirMassFlow;
                OACompOffMassFlow = VRFTU(VRFTUNum).NoCoolHeatOutAirMassFlow;
            } else if (LastModeCooling(VRFCond)) { // if NOT cooling or heating then use last mode
                CompOnMassFlow = VRFTU(VRFTUNum).MaxCoolAirMassFlow;
                CompOffMassFlow = VRFTU(VRFTUNum).MaxNoCoolAirMassFlow;
                OACompOnMassFlow = VRFTU(VRFTUNum).CoolOutAirMassFlow;
                OACompOffMassFlow = VRFTU(VRFTUNum).NoCoolHeatOutAirMassFlow;
            } else if (LastModeHeating(VRFCond)) { // if NOT cooling or heating then use last mode
                CompOnMassFlow = VRFTU(VRFTUNum).MaxHeatAirMassFlow;
                CompOffMassFlow = VRFTU(VRFTUNum).MaxNoHeatAirMassFlow;
                OACompOnMassFlow = VRFTU(VRFTUNum).HeatOutAirMassFlow;
                OACompOffMassFlow = VRFTU(VRFTUNum).NoCoolHeatOutAirMassFlow;
            } else { // should not happen so just set to cooling flow rate
                CompOnMassFlow = VRFTU(VRFTUNum).MaxCoolAirMassFlow;
                CompOffMassFlow = VRFTU(VRFTUNum).MaxNoCoolAirMassFlow;
                OACompOnMassFlow = VRFTU(VRFTUNum).CoolOutAirMassFlow;
                OACompOffMassFlow = VRFTU(VRFTUNum).NoCoolHeatOutAirMassFlow;
            }
        } else { // uses previous operating mode to set flow rate (used for looping through each TU in Init before mode is set)
            if (LastModeCooling(VRFCond)) {
                CompOnMassFlow = VRFTU(VRFTUNum).MaxCoolAirMassFlow;
                CompOffMassFlow = VRFTU(VRFTUNum).MaxNoCoolAirMassFlow;
                OACompOnMassFlow = VRFTU(VRFTUNum).CoolOutAirMassFlow;
                OACompOffMassFlow = VRFTU(VRFTUNum).NoCoolHeatOutAirMassFlow;
            } else if (LastModeHeating(VRFCond)) {
                CompOnMassFlow = VRFTU(VRFTUNum).MaxHeatAirMassFlow;
                CompOffMassFlow = VRFTU(VRFTUNum).MaxNoHeatAirMassFlow;
                OACompOnMassFlow = VRFTU(VRFTUNum).HeatOutAirMassFlow;
                OACompOffMassFlow = VRFTU(VRFTUNum).NoCoolHeatOutAirMassFlow;
            } else { // should not happen so just set to cooling flow rate
                CompOnMassFlow = VRFTU(VRFTUNum).MaxCoolAirMassFlow;
                CompOffMassFlow = VRFTU(VRFTUNum).MaxNoCoolAirMassFlow;
                OACompOnMassFlow = VRFTU(VRFTUNum).CoolOutAirMassFlow;
                OACompOffMassFlow = VRFTU(VRFTUNum).NoCoolHeatOutAirMassFlow;
            }
        }

        if (VRFTU(VRFTUNum).OpMode == DataHVACGlobals::CycFanCycCoil) {
            CompOffMassFlow = 0.0;
            OACompOffMassFlow = 0.0;
        }
    }

    void SizeVRF(EnergyPlusData &state, int const VRFTUNum)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Richard Raustad, FSEC
        //       DATE WRITTEN   August 2010
        //       MODIFIED       August 2013 Daeho Kang, add component sizing table entries
        //                      B Nigusse, FSEC, added scalable sizing
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine is for sizing VRF Components for which inputs have not been
        // specified in the input.

        // METHODOLOGY EMPLOYED:
        // Obtains flow rates from the zone or system sizing arrays.

        using namespace DataSizing;
        using CurveManager::CurveValue;
        auto &GetDXCoilCap(DXCoils::GetCoilCapacityByIndexType);
        using DataHeatBalance::Zone;
        using DataHVACGlobals::CoolingAirflowSizing;
        using DataHVACGlobals::CoolingCapacitySizing;
        using DataHVACGlobals::HeatingAirflowSizing;
        using DataHVACGlobals::HeatingCapacitySizing;
        using General::RoundSigDigits;
        using PlantUtilities::RegisterPlantCompDesignFlow;

        static std::string const RoutineName("SizeVRF: "); // include trailing blank space

        static Array1D_bool CheckVRFCombinationRatio;
        bool FoundAll;                      // temporary variable used to check all terminal units
        bool errFlag;                       // temporary variable used for error checking
        Real64 TUCoolingCapacity;           // total terminal unit cooling capacity
        Real64 TUHeatingCapacity;           // total terminal unit heating capacity
        int VRFCond;                        // index to VRF condenser
        int TUListNum;                      // index to terminal unit list
        int TUIndex;                        // index to terminal unit
        int NumTU;                          // DO Loop index counter
        static bool MyOneTimeEIOFlag(true); // eio header flag reporting
        Real64 OnOffAirFlowRat;             // temporary variable used when sizing coils
        Real64 DXCoilCap;                   // capacity of DX cooling coil (W)
        bool IsAutoSize;                    // Indicator to autosize
        Real64 MaxCoolAirVolFlowDes;        // Autosized supply air during cooling for reporting
        Real64 MaxCoolAirVolFlowUser;       // Hardsized supply air during cooling for reporting
        Real64 MaxHeatAirVolFlowDes;        // Autosized supply air during heating for reporting
        Real64 MaxHeatAirVolFlowUser;       // Hardsized supply air during heating for reporting
        Real64 MaxNoCoolAirVolFlowDes;      // Autosized supply air flow when no cooling is needed for reporting
        Real64 MaxNoCoolAirVolFlowUser;     // Hardsized supply air flow when no cooling is needed for reporting
        Real64 MaxNoHeatAirVolFlowDes;      // Autosized supply air flow when no heating is needed for reporting
        Real64 MaxNoHeatAirVolFlowUser;     // Hardsized supply air flow when no heating is needed for reporting
        Real64 CoolOutAirVolFlowDes;        // Autosized outdoor air flow during cooling for reporting
        Real64 CoolOutAirVolFlowUser;       // Hardsized outdoor air flow during cooling for reporting
        Real64 HeatOutAirVolFlowDes;        // Autosized outdoor air flow during heating for reporting
        Real64 HeatOutAirVolFlowUser;       // Hardsized outdoor air flow during heating for reporting
        Real64 NoCoolHeatOutAirVolFlowDes;  // Autosized outdoor air when unconditioned for reporting
        Real64 NoCoolHeatOutAirVolFlowUser; // Hardsized outdoor air when unconditioned for reporting
        Real64 CoolingCapacityDes;          // Autosized cooling capacity for reporting
        Real64 CoolingCapacityUser;         // Hardsized cooling capacity for reporting
        Real64 HeatingCapacityDes;          // Autosized heating capacity for reporting
        Real64 HeatingCapacityUser;         // Hardsized heating capacity for reporting
        Real64 DefrostCapacityDes;          // Autosized defrost heater capacity for reporting
        Real64 DefrostCapacityUser;         // Hardsized defrost heater capacity for reporting
        Real64 EvapCondAirVolFlowRateDes;   // Autosized evaporative condenser flow for reporting
        Real64 EvapCondAirVolFlowRateUser;  // Hardsized evaporative condenser flow for reporting
        Real64 EvapCondPumpPowerDes;        // Autosized evaporative condenser pump power for reporting
        Real64 EvapCondPumpPowerUser;       // Hardsized evaporative condenser pump power for reporting

        std::string CompName;     // component name
        std::string CompType;     // component type
        std::string SizingString; // input field sizing description (e.g., Nominal Capacity)
        Real64 TempSize;          // autosized value of coil input field
        int FieldNum = 2;         // IDD numeric field number where input field description is found
        int SizingMethod; // Integer representation of sizing method name (e.g., CoolingAirflowSizing, HeatingAirflowSizing, CoolingCapacitySizing,
                          // HeatingCapacitySizing, etc.)
        bool PrintFlag = true;  // TRUE when sizing information is reported in the eio file
        int zoneHVACIndex;      // index of zoneHVAC equipment sizing specification
        int SAFMethod(0);       // supply air flow rate sizing method (SupplyAirFlowRate, FlowPerFloorArea, FractionOfAutosizedCoolingAirflow,
                                // FractionOfAutosizedHeatingAirflow ...)
        int CapSizingMethod(0); // capacity sizing methods (HeatingDesignCapacity, CapacityPerFloorArea, FractionOfAutosizedCoolingCapacity, and
                                // FractionOfAutosizedHeatingCapacity )

        // Formats
        static ObjexxFCL::gio::Fmt Format_990(
            "('! <VRF System Information>, VRF System Type, VRF System Name, ','VRF System Cooling Combination Ratio, VRF "
            "System Heating Combination Ratio, ','VRF System Cooling Piping Correction Factor, VRF System Heating Piping "
            "Correction Factor')");
        static ObjexxFCL::gio::Fmt Format_991("(' VRF System Information',6(', ',A))");

        DataSizing::ZoneEqSizingData *select_EqSizing(nullptr);

        // sweep specific data into one pointer to avoid if statements throughout this subroutine
        if (DataSizing::CurOASysNum > 0) {
            select_EqSizing = &DataSizing::OASysEqSizing(DataSizing::CurOASysNum);
        } else if (DataSizing::CurSysNum > 0) {
            select_EqSizing = &DataSizing::UnitarySysEqSizing(DataSizing::CurSysNum);
        } else if (DataSizing::CurZoneEqNum > 0) {
            select_EqSizing = &DataSizing::ZoneEqSizing(DataSizing::CurZoneEqNum);
            DataSizing::ZoneEqUnitarySys = true;
        } else {
            assert(false);
        }
        // Object Data, points to specific array
        DataSizing::ZoneEqSizingData &EqSizing(*select_EqSizing);

        // can't hurt to initialize these going in, problably redundant
        EqSizing.AirFlow = false;
        EqSizing.CoolingAirFlow = false;
        EqSizing.HeatingAirFlow = false;
        EqSizing.AirVolFlow = 0.0;
        EqSizing.CoolingAirVolFlow = 0.0;
        EqSizing.HeatingAirVolFlow = 0.0;
        EqSizing.Capacity = false;
        EqSizing.CoolingCapacity = false;
        EqSizing.HeatingCapacity = false;
        EqSizing.DesCoolingLoad = 0.0;
        EqSizing.DesHeatingLoad = 0.0;
        EqSizing.OAVolFlow = 0.0;

        VRFCond = VRFTU(VRFTUNum).VRFSysNum;
        IsAutoSize = false;
        MaxCoolAirVolFlowDes = 0.0;
        MaxCoolAirVolFlowUser = 0.0;
        MaxHeatAirVolFlowDes = 0.0;
        MaxHeatAirVolFlowUser = 0.0;
        MaxNoCoolAirVolFlowDes = 0.0;
        MaxNoCoolAirVolFlowUser = 0.0;
        MaxNoHeatAirVolFlowDes = 0.0;
        MaxNoHeatAirVolFlowUser = 0.0;
        CoolOutAirVolFlowDes = 0.0;
        CoolOutAirVolFlowUser = 0.0;
        HeatOutAirVolFlowDes = 0.0;
        HeatOutAirVolFlowUser = 0.0;
        NoCoolHeatOutAirVolFlowDes = 0.0;
        NoCoolHeatOutAirVolFlowUser = 0.0;
        CoolingCapacityDes = 0.0;
        CoolingCapacityUser = 0.0;
        HeatingCapacityDes = 0.0;
        HeatingCapacityUser = 0.0;
        DefrostCapacityDes = 0.0;
        DefrostCapacityUser = 0.0;
        EvapCondAirVolFlowRateDes = 0.0;
        EvapCondAirVolFlowRateUser = 0.0;
        EvapCondPumpPowerDes = 0.0;
        EvapCondPumpPowerUser = 0.0;

        DataScalableSizingON = false;
        DataScalableCapSizingON = false;
        DataFracOfAutosizedCoolingAirflow = 1.0;
        DataFracOfAutosizedHeatingAirflow = 1.0;
        DataFracOfAutosizedCoolingCapacity = 1.0;
        DataFracOfAutosizedHeatingCapacity = 1.0;
        SuppHeatCap = 0.0;

        if (MyOneTimeSizeFlag) {
            // initialize the environment and sizing flags
            CheckVRFCombinationRatio.dimension(NumVRFCond, true);
            MyOneTimeSizeFlag = false;
        }

        CompType = DataHVACGlobals::cVRFTUTypes(VRFTU(VRFTUNum).VRFTUType_Num);
        CompName = VRFTU(VRFTUNum).Name;
        DataZoneNumber = VRFTU(VRFTUNum).ZoneNum;

        if (VRFTU(VRFTUNum).fanType_Num == DataHVACGlobals::FanType_SystemModelObject) {
            if (VRFTU(VRFTUNum).isInZone) {
                DataSizing::DataFanEnumType = DataAirSystems::objectVectorOOFanSystemModel;
                DataSizing::DataFanIndex = VRFTU(VRFTUNum).FanIndex;
                if (VRFTU(VRFTUNum).FanPlace == DataHVACGlobals::BlowThru) {
                    DataSizing::DataFanPlacement = DataSizing::zoneFanPlacement::zoneBlowThru;
                } else if (VRFTU(VRFTUNum).FanPlace == DataHVACGlobals::DrawThru) {
                    DataSizing::DataFanPlacement = DataSizing::zoneFanPlacement::zoneDrawThru;
                }
            } else if (VRFTU(VRFTUNum).isInAirLoop) {
                DataAirSystems::PrimaryAirSystem(VRFTU(VRFTUNum).airLoopNum).supFanModelTypeEnum = DataAirSystems::objectVectorOOFanSystemModel;
                DataAirSystems::PrimaryAirSystem(VRFTU(VRFTUNum).airLoopNum).supFanVecIndex = VRFTU(VRFTUNum).FanIndex;
                if (VRFTU(VRFTUNum).FanPlace == DataHVACGlobals::BlowThru) {
                    DataAirSystems::PrimaryAirSystem(VRFTU(VRFTUNum).airLoopNum).supFanLocation = DataAirSystems::fanPlacement::BlowThru;
                } else if (VRFTU(VRFTUNum).FanPlace == DataHVACGlobals::DrawThru) {
                    DataAirSystems::PrimaryAirSystem(VRFTU(VRFTUNum).airLoopNum).supFanLocation = DataAirSystems::fanPlacement::DrawThru;
                }
            }
        } else if (VRFTU(VRFTUNum).FanIndex > 0) {
            if (VRFTU(VRFTUNum).isInZone) {
                DataSizing::DataFanEnumType = DataAirSystems::structArrayLegacyFanModels;
                DataSizing::DataFanIndex = VRFTU(VRFTUNum).FanIndex;
                if (VRFTU(VRFTUNum).FanPlace == DataHVACGlobals::BlowThru) {
                    DataSizing::DataFanPlacement = DataSizing::zoneFanPlacement::zoneBlowThru;
                } else if (VRFTU(VRFTUNum).FanPlace == DataHVACGlobals::DrawThru) {
                    DataSizing::DataFanPlacement = DataSizing::zoneFanPlacement::zoneDrawThru;
                }
            } else if (VRFTU(VRFTUNum).isInAirLoop) {
                DataAirSystems::PrimaryAirSystem(VRFTU(VRFTUNum).airLoopNum).supFanModelTypeEnum = DataAirSystems::structArrayLegacyFanModels;
                DataAirSystems::PrimaryAirSystem(VRFTU(VRFTUNum).airLoopNum).supFanVecIndex = VRFTU(VRFTUNum).FanIndex;
                if (VRFTU(VRFTUNum).FanPlace == DataHVACGlobals::BlowThru) {
                    DataAirSystems::PrimaryAirSystem(VRFTU(VRFTUNum).airLoopNum).supFanLocation = DataAirSystems::fanPlacement::BlowThru;
                } else if (VRFTU(VRFTUNum).FanPlace == DataHVACGlobals::DrawThru) {
                    DataAirSystems::PrimaryAirSystem(VRFTU(VRFTUNum).airLoopNum).supFanLocation = DataAirSystems::fanPlacement::DrawThru;
                }
            }
        }

        if (VRFTU(VRFTUNum).HVACSizingIndex > 0) {
            // initialize OA flow for sizing other inputs (e.g., capacity)
            if (VRFTU(VRFTUNum).CoolOutAirVolFlow == AutoSize) {
                EqSizing.OAVolFlow = FinalZoneSizing(CurZoneEqNum).MinOA;
            } else {
                EqSizing.OAVolFlow = VRFTU(VRFTUNum).CoolOutAirVolFlow;
            }
            if (VRFTU(VRFTUNum).HeatOutAirVolFlow != AutoSize) {
                EqSizing.OAVolFlow = max(EqSizing.OAVolFlow, VRFTU(VRFTUNum).HeatOutAirVolFlow);
            }
            if (VRFTU(VRFTUNum).ATMixerExists && VRFTU(VRFTUNum).isInZone) { // set up ATMixer conditions for scalable capacity sizing
                EqSizing.OAVolFlow = 0.0;                                    // Equipment OA flow should always be 0 when ATMixer is used
                SingleDuct::setATMixerSizingProperties(VRFTU(VRFTUNum).ATMixerIndex, VRFTU(VRFTUNum).ZoneNum, CurZoneEqNum);
            }

            zoneHVACIndex = VRFTU(VRFTUNum).HVACSizingIndex;

            SizingMethod = CoolingAirflowSizing;
            PrintFlag = true;
            bool errorsFound = false;
            SAFMethod = ZoneHVACSizing(zoneHVACIndex).CoolingSAFMethod;
            EqSizing.SizingMethod(SizingMethod) = SAFMethod;
            if (SAFMethod == SupplyAirFlowRate || SAFMethod == FlowPerFloorArea || SAFMethod == FractionOfAutosizedCoolingAirflow) {
                if (SAFMethod == SupplyAirFlowRate) {
                    if (ZoneHVACSizing(zoneHVACIndex).MaxCoolAirVolFlow > 0.0) {
                        EqSizing.AirVolFlow = ZoneHVACSizing(zoneHVACIndex).MaxCoolAirVolFlow;
                        EqSizing.SystemAirFlow = true;
                    }
                    TempSize = ZoneHVACSizing(zoneHVACIndex).MaxCoolAirVolFlow;
                } else if (SAFMethod == FlowPerFloorArea) {
                    EqSizing.SystemAirFlow = true;
                    EqSizing.AirVolFlow = ZoneHVACSizing(zoneHVACIndex).MaxCoolAirVolFlow * Zone(DataZoneNumber).FloorArea;
                    TempSize = ZoneEqSizing(CurZoneEqNum).AirVolFlow;
                    DataScalableSizingON = true;
                } else if (SAFMethod == FractionOfAutosizedCoolingAirflow) {
                    DataFracOfAutosizedCoolingAirflow = ZoneHVACSizing(zoneHVACIndex).MaxCoolAirVolFlow;
                    TempSize = AutoSize;
                    DataScalableSizingON = true;
                } else {
                    TempSize = ZoneHVACSizing(zoneHVACIndex).MaxCoolAirVolFlow;
                }

                CoolingAirFlowSizer sizingCoolingAirFlow;
                std::string stringOverride = "Cooling Supply Air Flow Rate [m3/s]";
                if (state.dataGlobal->isEpJSON) stringOverride = "cooling_supply_air_flow_rate [m3/s]";
                sizingCoolingAirFlow.overrideSizingString(stringOverride);
                // sizingCoolingAirFlow.setHVACSizingIndexData(FanCoil(FanCoilNum).HVACSizingIndex);
                sizingCoolingAirFlow.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
                VRFTU(VRFTUNum).MaxCoolAirVolFlow = sizingCoolingAirFlow.size(state, TempSize, errorsFound);

            } else if (SAFMethod == FlowPerCoolingCapacity) {
                SizingMethod = CoolingCapacitySizing;
                TempSize = AutoSize;
                PrintFlag = false;
                DataScalableSizingON = true;
                DataFlowUsedForSizing = FinalZoneSizing(CurZoneEqNum).DesCoolVolFlow;
                if (ZoneHVACSizing(zoneHVACIndex).CoolingCapMethod == FractionOfAutosizedCoolingCapacity) {
                    DataFracOfAutosizedCoolingCapacity = ZoneHVACSizing(zoneHVACIndex).ScaledCoolingCapacity;
                }
                CoolingCapacitySizer sizerCoolingCapacity;
                sizerCoolingCapacity.overrideSizingString(SizingString);
                sizerCoolingCapacity.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
                DataAutosizedCoolingCapacity = sizerCoolingCapacity.size(state, TempSize, errorsFound);
                DataFlowPerCoolingCapacity = ZoneHVACSizing(zoneHVACIndex).MaxCoolAirVolFlow;
                PrintFlag = true;
                TempSize = AutoSize;
                CoolingAirFlowSizer sizingCoolingAirFlow;
                std::string stringOverride = "Cooling Supply Air Flow Rate [m3/s]";
                if (state.dataGlobal->isEpJSON) stringOverride = "cooling_supply_air_flow_rate [m3/s]";
                sizingCoolingAirFlow.overrideSizingString(stringOverride);
                // sizingCoolingAirFlow.setHVACSizingIndexData(FanCoil(FanCoilNum).HVACSizingIndex);
                sizingCoolingAirFlow.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
                VRFTU(VRFTUNum).MaxCoolAirVolFlow = sizingCoolingAirFlow.size(state, TempSize, errorsFound);
            }

            SizingMethod = HeatingAirflowSizing;
            FieldNum = 3; // N3, \field Supply Air Flow Rate During Heating Operation
            PrintFlag = true;
            SizingString = VRFTUNumericFields(VRFTUNum).FieldNames(FieldNum) + " [m3/s]";
            SAFMethod = ZoneHVACSizing(zoneHVACIndex).HeatingSAFMethod;
            EqSizing.SizingMethod(SizingMethod) = SAFMethod;
            if (SAFMethod == SupplyAirFlowRate || SAFMethod == FlowPerFloorArea || SAFMethod == FractionOfAutosizedHeatingAirflow) {
                if (SAFMethod == SupplyAirFlowRate) {
                    if (ZoneHVACSizing(zoneHVACIndex).MaxHeatAirVolFlow > 0.0) {
                        EqSizing.AirVolFlow = ZoneHVACSizing(zoneHVACIndex).MaxHeatAirVolFlow;
                        EqSizing.SystemAirFlow = true;
                    }
                    TempSize = ZoneHVACSizing(zoneHVACIndex).MaxHeatAirVolFlow;
                } else if (SAFMethod == FlowPerFloorArea) {
                    EqSizing.SystemAirFlow = true;
                    EqSizing.AirVolFlow = ZoneHVACSizing(zoneHVACIndex).MaxHeatAirVolFlow * Zone(DataZoneNumber).FloorArea;
                    TempSize = ZoneEqSizing(CurZoneEqNum).AirVolFlow;
                    DataScalableSizingON = true;
                } else if (SAFMethod == FractionOfAutosizedHeatingAirflow) {
                    DataFracOfAutosizedHeatingAirflow = ZoneHVACSizing(zoneHVACIndex).MaxHeatAirVolFlow;
                    TempSize = AutoSize;
                    DataScalableSizingON = true;
                } else {
                    TempSize = ZoneHVACSizing(zoneHVACIndex).MaxHeatAirVolFlow;
                }
                bool errorsFound = false;
                HeatingAirFlowSizer sizingHeatingAirFlow;
                sizingHeatingAirFlow.overrideSizingString(SizingString);
                // sizingHeatingAirFlow.setHVACSizingIndexData(FanCoil(FanCoilNum).HVACSizingIndex);
                sizingHeatingAirFlow.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
                VRFTU(VRFTUNum).MaxHeatAirVolFlow = sizingHeatingAirFlow.size(state, TempSize, errorsFound);
            } else if (SAFMethod == FlowPerHeatingCapacity) {
                SizingMethod = HeatingCapacitySizing;
                TempSize = AutoSize;
                PrintFlag = false;
                DataScalableSizingON = true;
                DataFlowUsedForSizing = FinalZoneSizing(CurZoneEqNum).DesHeatVolFlow;
                if (ZoneHVACSizing(zoneHVACIndex).HeatingCapMethod == FractionOfAutosizedHeatingCapacity) {
                    DataFracOfAutosizedHeatingCapacity = ZoneHVACSizing(zoneHVACIndex).ScaledHeatingCapacity;
                }
                bool errorsFound = false;
                HeatingCapacitySizer sizerHeatingCapacity;
                sizerHeatingCapacity.overrideSizingString(SizingString);
                sizerHeatingCapacity.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
                DataAutosizedHeatingCapacity = sizerHeatingCapacity.size(state, TempSize, errorsFound);
                DataFlowPerHeatingCapacity = ZoneHVACSizing(zoneHVACIndex).MaxHeatAirVolFlow;
                SizingMethod = HeatingAirflowSizing;
                PrintFlag = true;
                TempSize = AutoSize;
                errorsFound = false;
                HeatingAirFlowSizer sizingHeatingAirFlow;
                sizingHeatingAirFlow.overrideSizingString(SizingString);
                // sizingHeatingAirFlow.setHVACSizingIndexData(FanCoil(FanCoilNum).HVACSizingIndex);
                sizingHeatingAirFlow.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
                VRFTU(VRFTUNum).MaxHeatAirVolFlow = sizingHeatingAirFlow.size(state, TempSize, errorsFound);
            }

            PrintFlag = true;
            SAFMethod = ZoneHVACSizing(zoneHVACIndex).NoCoolHeatSAFMethod;
            EqSizing.SizingMethod(SizingMethod) = SAFMethod;
            if ((SAFMethod == SupplyAirFlowRate) || (SAFMethod == FlowPerFloorArea) || (SAFMethod == FractionOfAutosizedHeatingAirflow) ||
                (SAFMethod == FractionOfAutosizedCoolingAirflow)) {
                if (SAFMethod == SupplyAirFlowRate) {
                    if (ZoneHVACSizing(zoneHVACIndex).MaxNoCoolHeatAirVolFlow > 0.0) {
                        EqSizing.AirVolFlow = ZoneHVACSizing(zoneHVACIndex).MaxNoCoolHeatAirVolFlow;
                        EqSizing.SystemAirFlow = true;
                    }
                    TempSize = ZoneHVACSizing(zoneHVACIndex).MaxNoCoolHeatAirVolFlow;
                } else if (SAFMethod == FlowPerFloorArea) {
                    EqSizing.SystemAirFlow = true;
                    EqSizing.AirVolFlow = ZoneHVACSizing(zoneHVACIndex).MaxNoCoolHeatAirVolFlow * Zone(DataZoneNumber).FloorArea;
                    TempSize = ZoneEqSizing(CurZoneEqNum).AirVolFlow;
                    DataScalableSizingON = true;
                } else if (SAFMethod == FractionOfAutosizedCoolingAirflow) {
                    DataFracOfAutosizedCoolingAirflow = ZoneHVACSizing(zoneHVACIndex).MaxNoCoolHeatAirVolFlow;
                    DataFracOfAutosizedHeatingAirflow = ZoneHVACSizing(zoneHVACIndex).MaxNoCoolHeatAirVolFlow;
                    TempSize = AutoSize;
                    DataScalableSizingON = true;
                } else if (SAFMethod == FractionOfAutosizedHeatingAirflow) {
                    DataFracOfAutosizedCoolingAirflow = ZoneHVACSizing(zoneHVACIndex).MaxNoCoolHeatAirVolFlow;
                    DataFracOfAutosizedHeatingAirflow = ZoneHVACSizing(zoneHVACIndex).MaxNoCoolHeatAirVolFlow;
                    TempSize = AutoSize;
                    DataScalableSizingON = true;
                } else {
                    TempSize = ZoneHVACSizing(zoneHVACIndex).MaxNoCoolHeatAirVolFlow;
                }
                CoolingAirFlowSizer sizingCoolingAirFlow;
                std::string stringOverride = "No Cooling Supply Air Flow Rate [m3/s]";
                if (state.dataGlobal->isEpJSON) stringOverride = "no_cooling_supply_air_flow_rate [m3/s]";
                sizingCoolingAirFlow.overrideSizingString(stringOverride);
                // sizingCoolingAirFlow.setHVACSizingIndexData(FanCoil(FanCoilNum).HVACSizingIndex);
                sizingCoolingAirFlow.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
                VRFTU(VRFTUNum).MaxNoCoolAirVolFlow = sizingCoolingAirFlow.size(state, TempSize, errorsFound);
            }

            SizingMethod = HeatingAirflowSizing;
            FieldNum = 4; // N4, \field Supply Air Flow Rate When No Heating is Needed
            PrintFlag = true;
            SizingString = VRFTUNumericFields(VRFTUNum).FieldNames(FieldNum) + " [m3/s]";
            SAFMethod = ZoneHVACSizing(zoneHVACIndex).NoCoolHeatSAFMethod;
            EqSizing.SizingMethod(SizingMethod) = SAFMethod;
            if ((SAFMethod == SupplyAirFlowRate) || (SAFMethod == FlowPerFloorArea) || (SAFMethod == FractionOfAutosizedHeatingAirflow) ||
                (SAFMethod == FractionOfAutosizedCoolingAirflow)) {
                if (SAFMethod == SupplyAirFlowRate) {
                    if (ZoneHVACSizing(zoneHVACIndex).MaxNoCoolHeatAirVolFlow > 0.0) {
                        EqSizing.AirVolFlow = ZoneHVACSizing(zoneHVACIndex).MaxNoCoolHeatAirVolFlow;
                        EqSizing.SystemAirFlow = true;
                    }
                    TempSize = ZoneHVACSizing(zoneHVACIndex).MaxNoCoolHeatAirVolFlow;
                } else if (SAFMethod == FlowPerFloorArea) {
                    EqSizing.SystemAirFlow = true;
                    EqSizing.AirVolFlow = ZoneHVACSizing(zoneHVACIndex).MaxNoCoolHeatAirVolFlow * Zone(DataZoneNumber).FloorArea;
                    TempSize = ZoneEqSizing(CurZoneEqNum).AirVolFlow;
                    DataScalableSizingON = true;
                } else if (SAFMethod == FractionOfAutosizedHeatingAirflow) {
                    DataFracOfAutosizedCoolingAirflow = ZoneHVACSizing(zoneHVACIndex).MaxNoCoolHeatAirVolFlow;
                    DataFracOfAutosizedHeatingAirflow = ZoneHVACSizing(zoneHVACIndex).MaxNoCoolHeatAirVolFlow;
                    TempSize = AutoSize;
                    DataScalableSizingON = true;
                } else if (SAFMethod == FractionOfAutosizedCoolingAirflow) {
                    DataFracOfAutosizedCoolingAirflow = ZoneHVACSizing(zoneHVACIndex).MaxNoCoolHeatAirVolFlow;
                    DataFracOfAutosizedHeatingAirflow = ZoneHVACSizing(zoneHVACIndex).MaxNoCoolHeatAirVolFlow;
                    TempSize = AutoSize;
                    DataScalableSizingON = true;
                } else {
                    TempSize = ZoneHVACSizing(zoneHVACIndex).MaxNoCoolHeatAirVolFlow;
                }
                bool errorsFound = false;
                HeatingAirFlowSizer sizingNoHeatingAirFlow;
                sizingNoHeatingAirFlow.overrideSizingString(SizingString);
                // sizingNoHeatingAirFlow.setHVACSizingIndexData(FanCoil(FanCoilNum).HVACSizingIndex);
                sizingNoHeatingAirFlow.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
                VRFTU(VRFTUNum).MaxNoHeatAirVolFlow = sizingNoHeatingAirFlow.size(state, TempSize, errorsFound);
            }

            // initialize capacity sizing variables: cooling
            SizingMethod = CoolingCapacitySizing;
            CapSizingMethod = ZoneHVACSizing(zoneHVACIndex).CoolingCapMethod;
            EqSizing.SizingMethod(SizingMethod) = CapSizingMethod;
            if (CapSizingMethod == CoolingDesignCapacity || CapSizingMethod == CapacityPerFloorArea ||
                CapSizingMethod == FractionOfAutosizedCoolingCapacity) {
                if (CapSizingMethod == HeatingDesignCapacity) {
                    if (ZoneHVACSizing(zoneHVACIndex).ScaledCoolingCapacity > 0.0) {
                        EqSizing.CoolingCapacity = true;
                        EqSizing.DesCoolingLoad = ZoneHVACSizing(zoneHVACIndex).ScaledCoolingCapacity;
                    }
                } else if (CapSizingMethod == CapacityPerFloorArea) {
                    EqSizing.CoolingCapacity = true;
                    EqSizing.DesCoolingLoad = ZoneHVACSizing(zoneHVACIndex).ScaledCoolingCapacity * Zone(DataZoneNumber).FloorArea;
                    DataScalableCapSizingON = true;
                } else if (CapSizingMethod == FractionOfAutosizedCoolingCapacity) {
                    DataFracOfAutosizedCoolingCapacity = ZoneHVACSizing(zoneHVACIndex).ScaledCoolingCapacity;
                    DataScalableCapSizingON = true;
                }
            }

            // initialize capacity sizing variables: heating
            SizingMethod = HeatingCapacitySizing;
            CapSizingMethod = ZoneHVACSizing(zoneHVACIndex).HeatingCapMethod;
            EqSizing.SizingMethod(SizingMethod) = CapSizingMethod;
            if (CapSizingMethod == HeatingDesignCapacity || CapSizingMethod == CapacityPerFloorArea ||
                CapSizingMethod == FractionOfAutosizedHeatingCapacity) {
                if (CapSizingMethod == HeatingDesignCapacity) {
                    if (ZoneHVACSizing(zoneHVACIndex).ScaledHeatingCapacity > 0.0) {
                        EqSizing.HeatingCapacity = true;
                        EqSizing.DesHeatingLoad = ZoneHVACSizing(zoneHVACIndex).ScaledHeatingCapacity;
                    }
                } else if (CapSizingMethod == CapacityPerFloorArea) {
                    EqSizing.HeatingCapacity = true;
                    EqSizing.DesHeatingLoad = ZoneHVACSizing(zoneHVACIndex).ScaledHeatingCapacity * Zone(DataZoneNumber).FloorArea;
                    DataScalableCapSizingON = true;
                } else if (CapSizingMethod == FractionOfAutosizedHeatingCapacity) {
                    DataFracOfAutosizedHeatingCapacity = ZoneHVACSizing(zoneHVACIndex).ScaledHeatingCapacity;
                    DataScalableCapSizingON = true;
                }
            }
        } else {
            // no scalable sizing method has been specified. Sizing proceeds using the method
            // specified in the zoneHVAC object

            PrintFlag = true;

            TempSize = VRFTU(VRFTUNum).MaxCoolAirVolFlow;
            bool errorsFound = false;
            CoolingAirFlowSizer sizingCoolingAirFlow;
            std::string stringOverride = "Cooling Supply Air Flow Rate [m3/s]";
            if (state.dataGlobal->isEpJSON) stringOverride = "cooling_supply_air_flow_rate [m3/s]";
            sizingCoolingAirFlow.overrideSizingString(stringOverride);
            // sizingCoolingAirFlow.setHVACSizingIndexData(FanCoil(FanCoilNum).HVACSizingIndex);
            sizingCoolingAirFlow.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
            VRFTU(VRFTUNum).MaxCoolAirVolFlow = sizingCoolingAirFlow.size(state, TempSize, errorsFound);

            FieldNum = 3; // N3, \field Supply Air Flow Rate During Heating Operation
            SizingString = VRFTUNumericFields(VRFTUNum).FieldNames(FieldNum) + " [m3/s]";
            SizingMethod = HeatingAirflowSizing;
            TempSize = VRFTU(VRFTUNum).MaxHeatAirVolFlow;
            errorsFound = false;
            HeatingAirFlowSizer sizingHeatingAirFlow;
            sizingHeatingAirFlow.overrideSizingString(SizingString);
            // sizingHeatingAirFlow.setHVACSizingIndexData(FanCoil(FanCoilNum).HVACSizingIndex);
            sizingHeatingAirFlow.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
            VRFTU(VRFTUNum).MaxHeatAirVolFlow = sizingHeatingAirFlow.size(state, TempSize, errorsFound);

            errorsFound = false;
            SystemAirFlowSizer sizerSystemAirFlow;
            std::string sizingString = "No Cooling Supply Air Flow Rate [m3/s]";
            sizerSystemAirFlow.overrideSizingString(sizingString);
            sizerSystemAirFlow.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
            VRFTU(VRFTUNum).MaxNoCoolAirVolFlow = sizerSystemAirFlow.size(state, VRFTU(VRFTUNum).MaxNoCoolAirVolFlow, errorsFound);

            SystemAirFlowSizer sizerSystemAirFlow2;
            sizingString = "No Heating Supply Air Flow Rate [m3/s]";
            sizerSystemAirFlow2.overrideSizingString(sizingString);
            sizerSystemAirFlow2.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
            VRFTU(VRFTUNum).MaxNoHeatAirVolFlow = sizerSystemAirFlow2.size(state, VRFTU(VRFTUNum).MaxNoHeatAirVolFlow, errorsFound);
        }
        IsAutoSize = false;
        if (VRFTU(VRFTUNum).CoolOutAirVolFlow == AutoSize) {
            IsAutoSize = true;
        }
        if (CurZoneEqNum > 0) {
            if (!IsAutoSize && !ZoneSizingRunDone) { // Simulation continue
                if (VRFTU(VRFTUNum).CoolOutAirVolFlow > 0.0) {
                    BaseSizer::reportSizerOutput(DataHVACGlobals::cVRFTUTypes(VRFTU(VRFTUNum).VRFTUType_Num),
                                                 VRFTU(VRFTUNum).Name,
                                                 "User-Specified Outdoor Air Flow Rate During Cooling Operation [m3/s]",
                                                 VRFTU(VRFTUNum).CoolOutAirVolFlow);
                }
            } else {
                CheckZoneSizing(DataHVACGlobals::cVRFTUTypes(VRFTU(VRFTUNum).VRFTUType_Num), VRFTU(VRFTUNum).Name);
                CoolOutAirVolFlowDes = min(FinalZoneSizing(CurZoneEqNum).MinOA, VRFTU(VRFTUNum).MaxCoolAirVolFlow);
                if (CoolOutAirVolFlowDes < DataHVACGlobals::SmallAirVolFlow) {
                    CoolOutAirVolFlowDes = 0.0;
                }

                if (IsAutoSize) {
                    VRFTU(VRFTUNum).CoolOutAirVolFlow = CoolOutAirVolFlowDes;
                    BaseSizer::reportSizerOutput(DataHVACGlobals::cVRFTUTypes(VRFTU(VRFTUNum).VRFTUType_Num),
                                                 VRFTU(VRFTUNum).Name,
                                                 "Design Size Outdoor Air Flow Rate During Cooling Operation [m3/s]",
                                                 CoolOutAirVolFlowDes);
                } else {
                    if (VRFTU(VRFTUNum).CoolOutAirVolFlow > 0.0 && CoolOutAirVolFlowDes > 0.0) {
                        CoolOutAirVolFlowUser = VRFTU(VRFTUNum).CoolOutAirVolFlow;
                        BaseSizer::reportSizerOutput(DataHVACGlobals::cVRFTUTypes(VRFTU(VRFTUNum).VRFTUType_Num),
                                                     VRFTU(VRFTUNum).Name,
                                                     "Design Size Outdoor Air Flow Rate During Cooling Operation [m3/s]",
                                                     CoolOutAirVolFlowDes,
                                                     "User-Specified Outdoor Air Flow Rate During Cooling Operation [m3/s]",
                                                     CoolOutAirVolFlowUser);
                        if (DisplayExtraWarnings) {
                            if ((std::abs(CoolOutAirVolFlowDes - CoolOutAirVolFlowUser) / CoolOutAirVolFlowUser) > AutoVsHardSizingThreshold) {
                                ShowMessage("SizeVRF: Potential issue with equipment sizing for " +
                                            DataHVACGlobals::cVRFTUTypes(VRFTU(VRFTUNum).VRFTUType_Num) + ' ' + VRFTU(VRFTUNum).Name);
                                ShowContinueError("User-Specified Outdoor Air Flow Rate During Cooling Operation of " +
                                                  RoundSigDigits(CoolOutAirVolFlowUser, 5) + " [m3/s]");
                                ShowContinueError("differs from Design Size Outdoor Air Flow Rate During Cooling Operation of " +
                                                  RoundSigDigits(CoolOutAirVolFlowDes, 5) + " [m3/s]");
                                ShowContinueError("This may, or may not, indicate mismatched component sizes.");
                                ShowContinueError("Verify that the value entered is intended and is consistent with other components.");
                            }
                        }
                    }
                }
            }
        } else {
            if (VRFTU(VRFTUNum).CoolOutAirVolFlow == DataSizing::AutoSize) {
                if (DataAirSystems::PrimaryAirSystem(CurSysNum).OASysExists) {
                    VRFTU(VRFTUNum).CoolOutAirVolFlow = 0.0;
                } else {
                    VRFTU(VRFTUNum).CoolOutAirVolFlow = min(FinalSysSizing(CurSysNum).DesOutAirVolFlow, VRFTU(VRFTUNum).MaxCoolAirVolFlow);
                }
                BaseSizer::reportSizerOutput(DataHVACGlobals::cVRFTUTypes(VRFTU(VRFTUNum).VRFTUType_Num),
                                             VRFTU(VRFTUNum).Name,
                                             "Design Size Outdoor Air Flow Rate During Cooling Operation [m3/s]",
                                             VRFTU(VRFTUNum).CoolOutAirVolFlow);
            }
        }

        IsAutoSize = false;
        if (VRFTU(VRFTUNum).HeatOutAirVolFlow == AutoSize) {
            IsAutoSize = true;
        }
        if (CurZoneEqNum > 0) {
            if (!IsAutoSize && !ZoneSizingRunDone) { // Simulation continue
                if (VRFTU(VRFTUNum).CoolOutAirVolFlow > 0.0) {
                    BaseSizer::reportSizerOutput(DataHVACGlobals::cVRFTUTypes(VRFTU(VRFTUNum).VRFTUType_Num),
                                                 VRFTU(VRFTUNum).Name,
                                                 "Outdoor Air Flow Rate During Heating Operation [m3/s]",
                                                 VRFTU(VRFTUNum).CoolOutAirVolFlow);
                }
            } else {
                CheckZoneSizing(DataHVACGlobals::cVRFTUTypes(VRFTU(VRFTUNum).VRFTUType_Num), VRFTU(VRFTUNum).Name);
                HeatOutAirVolFlowDes = min(FinalZoneSizing(CurZoneEqNum).MinOA, VRFTU(VRFTUNum).MaxHeatAirVolFlow);
                if (HeatOutAirVolFlowDes < DataHVACGlobals::SmallAirVolFlow) {
                    HeatOutAirVolFlowDes = 0.0;
                }

                if (IsAutoSize) {
                    VRFTU(VRFTUNum).HeatOutAirVolFlow = HeatOutAirVolFlowDes;
                    BaseSizer::reportSizerOutput(DataHVACGlobals::cVRFTUTypes(VRFTU(VRFTUNum).VRFTUType_Num),
                                                 VRFTU(VRFTUNum).Name,
                                                 "Design Size Outdoor Air Flow Rate During Heating Operation [m3/s]",
                                                 HeatOutAirVolFlowDes);
                } else {
                    if (VRFTU(VRFTUNum).HeatOutAirVolFlow > 0.0 && HeatOutAirVolFlowDes > 0.0) {
                        HeatOutAirVolFlowUser = VRFTU(VRFTUNum).HeatOutAirVolFlow;
                        BaseSizer::reportSizerOutput(DataHVACGlobals::cVRFTUTypes(VRFTU(VRFTUNum).VRFTUType_Num),
                                                     VRFTU(VRFTUNum).Name,
                                                     "Design Size Outdoor Air Flow Rate During Heating Operation [m3/s]",
                                                     HeatOutAirVolFlowDes,
                                                     "User-Specified Outdoor Air Flow Rate During Heating Operation [m3/s]",
                                                     HeatOutAirVolFlowUser);
                        if (DisplayExtraWarnings) {
                            if ((std::abs(HeatOutAirVolFlowDes - HeatOutAirVolFlowUser) / HeatOutAirVolFlowUser) > AutoVsHardSizingThreshold) {
                                ShowMessage("SizeVRF: Potential issue with equipment sizing for " +
                                            DataHVACGlobals::cVRFTUTypes(VRFTU(VRFTUNum).VRFTUType_Num) + ' ' + VRFTU(VRFTUNum).Name);
                                ShowContinueError("User-Specified Outdoor Air Flow Rate During Heating Operation of " +
                                                  RoundSigDigits(HeatOutAirVolFlowUser, 5) + " [m3/s]");
                                ShowContinueError("differs from Design Size Outdoor Air Flow Rate During Heating Operation of " +
                                                  RoundSigDigits(HeatOutAirVolFlowDes, 5) + " [m3/s]");
                                ShowContinueError("This may, or may not, indicate mismatched component sizes.");
                                ShowContinueError("Verify that the value entered is intended and is consistent with other components.");
                            }
                        }
                    }
                }
            }
        } else {
            if (VRFTU(VRFTUNum).HeatOutAirVolFlow == DataSizing::AutoSize) {
                if (DataAirSystems::PrimaryAirSystem(CurSysNum).OASysExists) {
                    VRFTU(VRFTUNum).HeatOutAirVolFlow = 0.0;
                } else {
                    VRFTU(VRFTUNum).HeatOutAirVolFlow = min(FinalSysSizing(CurSysNum).DesOutAirVolFlow, VRFTU(VRFTUNum).MaxHeatAirVolFlow);
                }
                BaseSizer::reportSizerOutput(DataHVACGlobals::cVRFTUTypes(VRFTU(VRFTUNum).VRFTUType_Num),
                                             VRFTU(VRFTUNum).Name,
                                             "Design Size Outdoor Air Flow Rate During Heating Operation [m3/s]",
                                             VRFTU(VRFTUNum).HeatOutAirVolFlow);
            }
        }
        EqSizing.OAVolFlow = max(VRFTU(VRFTUNum).CoolOutAirVolFlow, VRFTU(VRFTUNum).HeatOutAirVolFlow);

        if (VRFTU(VRFTUNum).ATMixerExists && VRFTU(VRFTUNum).isInZone) { // set up ATMixer conditions for use in component sizing
            EqSizing.OAVolFlow = 0.0;                                    // Equipment OA flow should always be 0 when ATMixer is used
            SingleDuct::setATMixerSizingProperties(VRFTU(VRFTUNum).ATMixerIndex, VRFTU(VRFTUNum).ZoneNum, CurZoneEqNum);
        }

        IsAutoSize = false;
        if (VRFTU(VRFTUNum).NoCoolHeatOutAirVolFlow == AutoSize) {
            IsAutoSize = true;
        }
        if (CurZoneEqNum > 0) {
            if (!IsAutoSize && !ZoneSizingRunDone) { // Simulation continue
                if (VRFTU(VRFTUNum).NoCoolHeatOutAirVolFlow > 0.0) {
                    BaseSizer::reportSizerOutput(DataHVACGlobals::cVRFTUTypes(VRFTU(VRFTUNum).VRFTUType_Num),
                                                 VRFTU(VRFTUNum).Name,
                                                 "User-Specified Outdoor Air Flow Rate When No Cooling or Heating is Needed [m3/s]",
                                                 VRFTU(VRFTUNum).NoCoolHeatOutAirVolFlow);
                }
            } else {
                CheckZoneSizing(DataHVACGlobals::cVRFTUTypes(VRFTU(VRFTUNum).VRFTUType_Num), VRFTU(VRFTUNum).Name);
                NoCoolHeatOutAirVolFlowDes =
                    min(FinalZoneSizing(CurZoneEqNum).MinOA, VRFTU(VRFTUNum).HeatOutAirVolFlow, VRFTU(VRFTUNum).CoolOutAirVolFlow);
                if (NoCoolHeatOutAirVolFlowDes < DataHVACGlobals::SmallAirVolFlow) {
                    NoCoolHeatOutAirVolFlowDes = 0.0;
                }

                if (IsAutoSize) {
                    VRFTU(VRFTUNum).NoCoolHeatOutAirVolFlow = NoCoolHeatOutAirVolFlowDes;
                    BaseSizer::reportSizerOutput(DataHVACGlobals::cVRFTUTypes(VRFTU(VRFTUNum).VRFTUType_Num),
                                                 VRFTU(VRFTUNum).Name,
                                                 "Design Size Outdoor Air Flow Rate When No Cooling or Heating is Needed [m3/s]",
                                                 NoCoolHeatOutAirVolFlowDes);
                } else {
                    if (VRFTU(VRFTUNum).NoCoolHeatOutAirVolFlow > 0.0 && NoCoolHeatOutAirVolFlowDes > 0.0) {
                        NoCoolHeatOutAirVolFlowUser = VRFTU(VRFTUNum).NoCoolHeatOutAirVolFlow;
                        BaseSizer::reportSizerOutput(DataHVACGlobals::cVRFTUTypes(VRFTU(VRFTUNum).VRFTUType_Num),
                                                     VRFTU(VRFTUNum).Name,
                                                     "Design Size Outdoor Air Flow Rate When No Cooling or Heating is Needed [m3/s]",
                                                     NoCoolHeatOutAirVolFlowDes,
                                                     "User-Specified Outdoor Air Flow Rate When No Cooling or Heating is Needed [m3/s]",
                                                     NoCoolHeatOutAirVolFlowUser);
                        if (DisplayExtraWarnings) {
                            if ((std::abs(NoCoolHeatOutAirVolFlowDes - NoCoolHeatOutAirVolFlowUser) / NoCoolHeatOutAirVolFlowUser) >
                                AutoVsHardSizingThreshold) {
                                ShowMessage("SizeVRF: Potential issue with equipment sizing for " +
                                            DataHVACGlobals::cVRFTUTypes(VRFTU(VRFTUNum).VRFTUType_Num) + ' ' + VRFTU(VRFTUNum).Name);
                                ShowContinueError("User-Specified Outdoor Air Flow Rate When No Cooling or Heating is Needed of " +
                                                  RoundSigDigits(NoCoolHeatOutAirVolFlowUser, 5) + " [m3/s]");
                                ShowContinueError("differs from Design Size Outdoor Air Flow Rate When No Cooling or Heating is Needed of " +
                                                  RoundSigDigits(NoCoolHeatOutAirVolFlowDes, 5) + " [m3/s]");
                                ShowContinueError("This may, or may not, indicate mismatched component sizes.");
                                ShowContinueError("Verify that the value entered is intended and is consistent with other components.");
                            }
                        }
                    }
                }
            }
        } else {
            if (VRFTU(VRFTUNum).NoCoolHeatOutAirVolFlow == DataSizing::AutoSize) {
                if (DataAirSystems::PrimaryAirSystem(CurSysNum).OASysExists) {
                    VRFTU(VRFTUNum).NoCoolHeatOutAirVolFlow = 0.0;
                } else {
                    VRFTU(VRFTUNum).NoCoolHeatOutAirVolFlow = min(VRFTU(VRFTUNum).MaxCoolAirVolFlow, VRFTU(VRFTUNum).MaxHeatAirVolFlow);
                }
                BaseSizer::reportSizerOutput(DataHVACGlobals::cVRFTUTypes(VRFTU(VRFTUNum).VRFTUType_Num),
                                             VRFTU(VRFTUNum).Name,
                                             "Design Size Outdoor Air Flow Rate When No Cooling or Heating Heating is Needed [m3/s]",
                                             VRFTU(VRFTUNum).NoCoolHeatOutAirVolFlow);
            }
        }

        if (VRFTU(VRFTUNum).SuppHeatingCoilPresent) {
            bool ErrorsFound = false;
            TempSize = VRFTU(VRFTUNum).MaxSATFromSuppHeatCoil;
            MaxHeaterOutletTempSizer sizerMaxHeaterOutTemp;
            std::string stringOverride = "Maximum Supply Air Temperature from Supplemental Heater [C]";
            if (state.dataGlobal->isEpJSON) stringOverride = "maximum_supply_air_temperature_from_supplemental_heater [C]";
            sizerMaxHeaterOutTemp.overrideSizingString(stringOverride);
            sizerMaxHeaterOutTemp.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
            VRFTU(VRFTUNum).MaxSATFromSuppHeatCoil = sizerMaxHeaterOutTemp.size(state, TempSize, ErrorsFound);
        }

        if (VRFTU(VRFTUNum).SuppHeatCoilType_Num == DataHVACGlobals::Coil_HeatingWater) {
            bool ErrorsFound = false;
            WaterCoils::SetCoilDesFlow(state,
                VRFTU(VRFTUNum).SuppHeatCoilType, VRFTU(VRFTUNum).SuppHeatCoilName, VRFTU(VRFTUNum).MaxHeatAirVolFlow, ErrorsFound);
        }

        if (VRFTU(VRFTUNum).SuppHeatingCoilPresent) {
            CompType = VRFTU(VRFTUNum).SuppHeatCoilType;
            CompName = VRFTU(VRFTUNum).SuppHeatCoilName;
            PrintFlag = false; // why isn't this being reported?
            TempSize = VRFTU(VRFTUNum).DesignSuppHeatingCapacity;
            if (VRFTU(VRFTUNum).SuppHeatCoilType_Num == DataHVACGlobals::Coil_HeatingWater) {
                // sizing result should always be reported
                if (TempSize == DataSizing::AutoSize) {
                    WaterHeatingCapacitySizer sizerWaterHeatingCapacity;
                    bool ErrorsFound = false;
                    std::string stringOverride = "Supplemental Heating Coil Nominal Capacity [W]";
                    if (state.dataGlobal->isEpJSON) stringOverride = "supplemental_heating_coil_nominal_capacity [W]";
                    sizerWaterHeatingCapacity.overrideSizingString(stringOverride);
                    sizerWaterHeatingCapacity.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
                    VRFTU(VRFTUNum).DesignSuppHeatingCapacity = sizerWaterHeatingCapacity.size(state, TempSize, ErrorsFound);
                }
            } else {
                SizingMethod = DataHVACGlobals::HeatingCapacitySizing;
                SizingString = "Supplemental Heating Coil Nominal Capacity [W]";
                if (TempSize == DataSizing::AutoSize) {
                    IsAutoSize = true;
                    bool errorsFound = false;
                    HeatingCapacitySizer sizerHeatingCapacity;
                    sizerHeatingCapacity.overrideSizingString(SizingString);
                    sizerHeatingCapacity.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
                    VRFTU(VRFTUNum).DesignSuppHeatingCapacity = sizerHeatingCapacity.size(state, TempSize, errorsFound);
                }
            }
        }

        EqSizing.CoolingAirFlow = true;
        EqSizing.CoolingAirVolFlow = VRFTU(VRFTUNum).MaxCoolAirVolFlow;
        EqSizing.HeatingAirFlow = true;
        EqSizing.HeatingAirVolFlow = VRFTU(VRFTUNum).MaxHeatAirVolFlow;

        if (CheckVRFCombinationRatio(VRFCond)) {
            OnOffAirFlowRat = 1.0;
            // set up the outside air data for sizing the DX coils
            if (VRFTU(VRFTUNum).isInZone) ZoneEqDXCoil = true;
            if (CurZoneEqNum > 0) {
                if (VRFTU(VRFTUNum).CoolOutAirVolFlow > 0.0 || VRFTU(VRFTUNum).HeatOutAirVolFlow > 0.0) {
                    EqSizing.OAVolFlow = max(VRFTU(VRFTUNum).CoolOutAirVolFlow, VRFTU(VRFTUNum).HeatOutAirVolFlow);
                } else {
                    EqSizing.OAVolFlow = 0.0;
                }
            } else {
                EqSizing.OAVolFlow = 0.0;
            }

            Real64 SuppHeatCoilLoad = 0.0;
            // simulate the TU to size the coils
            if (VRF(VRFCond).VRFAlgorithmTypeNum == AlgorithmTypeFluidTCtrl) {
                // Algorithm Type: VRF model based on physics, applicable for Fluid Temperature Control
                VRFTU(VRFTUNum).CalcVRF_FluidTCtrl(state, VRFTUNum, true, 0.0, TUCoolingCapacity, OnOffAirFlowRat, SuppHeatCoilLoad);
            } else {
                // Algorithm Type: VRF model based on system curve
                VRFTU(VRFTUNum).CalcVRF(state, VRFTUNum, true, 0.0, TUCoolingCapacity, OnOffAirFlowRat, SuppHeatCoilLoad);
            }

            //    ZoneEqDXCoil = .FALSE.
            TUCoolingCapacity = 0.0;
            TUHeatingCapacity = 0.0;
            FoundAll = true;
            TUListNum = VRFTU(VRFTUNum).TUListIndex;
            for (NumTU = 1; NumTU <= TerminalUnitList(TUListNum).NumTUInList; ++NumTU) {
                TUIndex = TerminalUnitList(TUListNum).ZoneTUPtr(NumTU);
                if (VRFTU(TUIndex).CoolCoilIndex > 0) {
                    DXCoilCap = GetDXCoilCap(state, VRFTU(TUIndex).CoolCoilIndex, VRFTU(TUIndex).DXCoolCoilType_Num, errFlag);
                    TUCoolingCapacity += DXCoilCap;
                    if (DXCoilCap == AutoSize) {
                        FoundAll = false;
                        break;
                    }
                }
                if (VRFTU(TUIndex).HeatCoilIndex > 0) {
                    DXCoilCap = GetDXCoilCap(state, VRFTU(TUIndex).HeatCoilIndex, VRFTU(TUIndex).DXHeatCoilType_Num, errFlag);
                    TUHeatingCapacity += DXCoilCap;
                    if (DXCoilCap == AutoSize) {
                        FoundAll = false;
                        break;
                    }
                }
            }

            if (FoundAll && (VRF(VRFCond).VRFAlgorithmTypeNum == AlgorithmTypeSysCurve)) {
                // Size VRF rated cooling/heating capacity (VRF-SysCurve Model)

                // Size VRF( VRFCond ).CoolingCapacity
                IsAutoSize = false;
                if (VRF(VRFCond).CoolingCapacity == AutoSize) {
                    IsAutoSize = true;
                }
                CoolingCapacityDes = TUCoolingCapacity;
                if (IsAutoSize) {
                    VRF(VRFCond).CoolingCapacity = CoolingCapacityDes;
                    BaseSizer::reportSizerOutput(cVRFTypes(VRF(VRFCond).VRFSystemTypeNum),
                                                 VRF(VRFCond).Name,
                                                 "Design Size Rated Total Cooling Capacity (gross) [W]",
                                                 CoolingCapacityDes);
                } else {
                    if (VRF(VRFCond).CoolingCapacity > 0.0 && CoolingCapacityDes > 0.0) {
                        CoolingCapacityUser = VRF(VRFCond).CoolingCapacity;
                        BaseSizer::reportSizerOutput(cVRFTypes(VRF(VRFCond).VRFSystemTypeNum),
                                                     VRF(VRFCond).Name,
                                                     "Design Size Rated Total Cooling Capacity (gross) [W]",
                                                     CoolingCapacityDes,
                                                     "User-Specified Rated Total Cooling Capacity (gross) [W]",
                                                     CoolingCapacityUser);
                        if (DisplayExtraWarnings) {
                            if ((std::abs(CoolingCapacityDes - CoolingCapacityUser) / CoolingCapacityUser) > AutoVsHardSizingThreshold) {
                                ShowMessage("SizeVRF: Potential issue with equipment sizing for " + cVRFTypes(VRF(VRFCond).VRFSystemTypeNum) + ' ' +
                                            VRFTU(VRFCond).Name);
                                ShowContinueError("User-Specified Rated Total Cooling Capacity (gross) of " + RoundSigDigits(CoolingCapacityUser, 2) +
                                                  " [W]");
                                ShowContinueError("differs from Design Size Rated Total Cooling Capacity (gross) of " +
                                                  RoundSigDigits(CoolingCapacityDes, 2) + " [W]");
                                ShowContinueError("This may, or may not, indicate mismatched component sizes.");
                                ShowContinueError("Verify that the value entered is intended and is consistent with other components.");
                            }
                        }
                    }
                }

                if (VRF(VRFCond).CoolingCapacity > 0.0) {
                    VRF(VRFCond).CoolingCombinationRatio = TUCoolingCapacity / VRF(VRFCond).CoolingCapacity;
                }

                // Size VRF( VRFCond ).HeatingCapacity
                IsAutoSize = false;
                if (VRF(VRFCond).HeatingCapacity == AutoSize) {
                    IsAutoSize = true;
                }
                if (VRF(VRFCond).LockHeatingCapacity) {
                    HeatingCapacityDes = VRF(VRFCond).CoolingCapacity * VRF(VRFCond).HeatingCapacitySizeRatio;
                } else {
                    HeatingCapacityDes = TUHeatingCapacity;
                }
                if (IsAutoSize) {
                    VRF(VRFCond).HeatingCapacity = HeatingCapacityDes;
                    BaseSizer::reportSizerOutput(cVRFTypes(VRF(VRFCond).VRFSystemTypeNum),
                                                 VRF(VRFCond).Name,
                                                 "Design Size Rated Total Heating Capacity [W]",
                                                 HeatingCapacityDes);
                } else {
                    if (VRF(VRFCond).HeatingCapacity > 0.0 && HeatingCapacityDes > 0.0) {
                        HeatingCapacityUser = VRF(VRFCond).HeatingCapacity;
                        BaseSizer::reportSizerOutput(cVRFTypes(VRF(VRFCond).VRFSystemTypeNum),
                                                     VRF(VRFCond).Name,
                                                     "Design Size Rated Total Heating Capacity [W]",
                                                     HeatingCapacityDes,
                                                     "User-Specified Rated Total Heating Capacity [W]",
                                                     HeatingCapacityUser);
                        if (DisplayExtraWarnings) {
                            if ((std::abs(HeatingCapacityDes - HeatingCapacityUser) / HeatingCapacityUser) > AutoVsHardSizingThreshold) {
                                ShowMessage("SizeVRF: Potential issue with equipment sizing for " + cVRFTypes(VRF(VRFCond).VRFSystemTypeNum) + ' ' +
                                            VRFTU(VRFCond).Name);
                                ShowContinueError("User-Specified Rated Total Heating Capacity of " + RoundSigDigits(HeatingCapacityUser, 2) +
                                                  " [W]");
                                ShowContinueError("differs from Design Size Rated Total Heating Capacity of " +
                                                  RoundSigDigits(HeatingCapacityDes, 2) + " [W]");
                                ShowContinueError("This may, or may not, indicate mismatched component sizes.");
                                ShowContinueError("Verify that the value entered is intended and is consistent with other components.");
                            }
                        }
                    }
                }

                if (VRF(VRFCond).HeatingCapacity > 0.0) {
                    VRF(VRFCond).HeatingCombinationRatio = TUHeatingCapacity / VRF(VRFCond).HeatingCapacity;
                }

                // calculate the piping correction factors only once
                if (VRF(VRFCond).PCFLengthCoolPtr > 0) {
                    {
                        if (state.dataCurveManager->PerfCurve(VRF(VRFCond).PCFLengthCoolPtr).NumDims == 2) {
                            VRF(VRFCond).PipingCorrectionCooling = min(
                                1.0,
                                max(0.5,
                                    CurveValue(state, VRF(VRFCond).PCFLengthCoolPtr, VRF(VRFCond).EquivPipeLngthCool, VRF(VRFCond).CoolingCombinationRatio) +
                                        VRF(VRFCond).VertPipeLngth * VRF(VRFCond).PCFHeightCool));
                        } else {
                            VRF(VRFCond).PipingCorrectionCooling =
                                min(1.0,
                                    max(0.5,
                                        CurveValue(state, VRF(VRFCond).PCFLengthCoolPtr, VRF(VRFCond).EquivPipeLngthCool) +
                                            VRF(VRFCond).VertPipeLngth * VRF(VRFCond).PCFHeightCool));
                        }
                    }
                } else {
                    VRF(VRFCond).PipingCorrectionCooling = min(1.0, max(0.5, (1.0 + VRF(VRFCond).VertPipeLngth * VRF(VRFCond).PCFHeightCool)));
                }

                if (VRF(VRFCond).PCFLengthHeatPtr > 0) {
                    {
                        if (state.dataCurveManager->PerfCurve(VRF(VRFCond).PCFLengthHeatPtr).NumDims == 2) {
                            VRF(VRFCond).PipingCorrectionHeating = min(
                                1.0,
                                max(0.5,
                                    CurveValue(state, VRF(VRFCond).PCFLengthHeatPtr, VRF(VRFCond).EquivPipeLngthHeat, VRF(VRFCond).HeatingCombinationRatio) +
                                        VRF(VRFCond).VertPipeLngth * VRF(VRFCond).PCFHeightHeat));
                        } else {
                            VRF(VRFCond).PipingCorrectionHeating =
                                min(1.0,
                                    max(0.5,
                                        CurveValue(state, VRF(VRFCond).PCFLengthHeatPtr, VRF(VRFCond).EquivPipeLngthHeat) +
                                            VRF(VRFCond).VertPipeLngth * VRF(VRFCond).PCFHeightHeat));
                        }
                    }
                } else {
                    VRF(VRFCond).PipingCorrectionHeating = min(1.0, max(0.5, (1.0 + VRF(VRFCond).VertPipeLngth * VRF(VRFCond).PCFHeightHeat)));
                }

                VRF(VRFCond).RatedCoolingPower = VRF(VRFCond).CoolingCapacity / VRF(VRFCond).CoolingCOP;
                VRF(VRFCond).RatedHeatingPower = VRF(VRFCond).HeatingCapacity / VRF(VRFCond).HeatingCOP;

                if (VRF(VRFCond).CoolCombRatioPTR > 0) {
                    CoolCombinationRatio(VRFCond) = CurveValue(state, VRF(VRFCond).CoolCombRatioPTR, VRF(VRFCond).CoolingCombinationRatio);
                } else {
                    CoolCombinationRatio(VRFCond) = 1.0;
                }

                if (VRF(VRFCond).HeatCombRatioPTR > 0) {
                    HeatCombinationRatio(VRFCond) = CurveValue(state, VRF(VRFCond).HeatCombRatioPTR, VRF(VRFCond).HeatingCombinationRatio);
                } else {
                    HeatCombinationRatio(VRFCond) = 1.0;
                }
            }

            if (FoundAll && (VRF(VRFCond).VRFAlgorithmTypeNum == AlgorithmTypeFluidTCtrl)) {
                // Size VRF rated evaporative capacity (VRF-FluidTCtrl Model)
                // Set piping correction factors to 1.0 here for reporting to eio output - recalculated every time step in
                // VRFCondenserEquipment::CalcVRFCondenser_FluidTCtrl
                VRF(VRFCond).PipingCorrectionCooling = 1.0;
                VRF(VRFCond).PipingCorrectionHeating = 1.0;

                // Size VRF( VRFCond ).RatedEvapCapacity
                IsAutoSize = false;
                if (VRF(VRFCond).RatedEvapCapacity == AutoSize) {
                    IsAutoSize = true;
                }

                CoolingCapacityDes = TUCoolingCapacity;
                HeatingCapacityDes = TUHeatingCapacity;

                if (IsAutoSize) {
                    // RatedEvapCapacity
                    VRF(VRFCond).RatedEvapCapacity = max(CoolingCapacityDes, HeatingCapacityDes / (1 + VRF(VRFCond).RatedCompPowerPerCapcity));

                    // Other parameters dependent on RatedEvapCapacity
                    VRF(VRFCond).RatedCompPower = VRF(VRFCond).RatedCompPowerPerCapcity * VRF(VRFCond).RatedEvapCapacity;
                    VRF(VRFCond).RatedOUFanPower = VRF(VRFCond).RatedOUFanPowerPerCapcity * VRF(VRFCond).RatedEvapCapacity;
                    VRF(VRFCond).OUAirFlowRate = VRF(VRFCond).OUAirFlowRatePerCapcity * VRF(VRFCond).RatedEvapCapacity;

                    VRF(VRFCond).CoolingCapacity = VRF(VRFCond).RatedEvapCapacity;
                    VRF(VRFCond).HeatingCapacity = VRF(VRFCond).RatedEvapCapacity * (1 + VRF(VRFCond).RatedCompPowerPerCapcity);

                    BaseSizer::reportSizerOutput(cVRFTypes(VRF(VRFCond).VRFSystemTypeNum),
                                                 VRF(VRFCond).Name,
                                                 "Design Size Rated Total Heating Capacity [W]",
                                                 VRF(VRFCond).HeatingCapacity);
                    BaseSizer::reportSizerOutput(cVRFTypes(VRF(VRFCond).VRFSystemTypeNum),
                                                 VRF(VRFCond).Name,
                                                 "Design Size Rated Total Cooling Capacity (gross) [W]",
                                                 VRF(VRFCond).CoolingCapacity);
                } else {
                    CoolingCapacityUser = VRF(VRFCond).RatedEvapCapacity;
                    HeatingCapacityUser = VRF(VRFCond).RatedHeatCapacity;

                    BaseSizer::reportSizerOutput(cVRFTypes(VRF(VRFCond).VRFSystemTypeNum),
                                                 VRF(VRFCond).Name,
                                                 "Design Size Rated Total Cooling Capacity (gross) [W]",
                                                 CoolingCapacityDes,
                                                 "User-Specified Rated Total Cooling Capacity (gross) [W]",
                                                 CoolingCapacityUser);
                    BaseSizer::reportSizerOutput(cVRFTypes(VRF(VRFCond).VRFSystemTypeNum),
                                                 VRF(VRFCond).Name,
                                                 "Design Size Rated Total Heating Capacity [W]",
                                                 HeatingCapacityDes,
                                                 "User-Specified Rated Total Heating Capacity [W]",
                                                 HeatingCapacityUser);

                    if (DisplayExtraWarnings) {
                        if ((std::abs(CoolingCapacityDes - CoolingCapacityUser) / CoolingCapacityUser) > AutoVsHardSizingThreshold) {
                            ShowMessage("SizeVRF: Potential issue with equipment sizing for " + cVRFTypes(VRF(VRFCond).VRFSystemTypeNum) + ' ' +
                                        VRFTU(VRFCond).Name);
                            ShowContinueError("User-Specified Rated Total Cooling Capacity (gross) of " + RoundSigDigits(CoolingCapacityUser, 2) +
                                              " [W]");
                            ShowContinueError("differs from Design Size Rated Total Cooling Capacity (gross) of " +
                                              RoundSigDigits(CoolingCapacityDes, 2) + " [W]");
                            ShowContinueError("This may, or may not, indicate mismatched component sizes.");
                            ShowContinueError("Verify that the value entered is intended and is consistent with other components.");
                        }

                        if ((std::abs(HeatingCapacityDes - HeatingCapacityUser) / HeatingCapacityUser) > AutoVsHardSizingThreshold) {
                            ShowMessage("SizeVRF: Potential issue with equipment sizing for " + cVRFTypes(VRF(VRFCond).VRFSystemTypeNum) + ' ' +
                                        VRFTU(VRFCond).Name);
                            ShowContinueError("User-Specified Rated Total Heating Capacity of " + RoundSigDigits(HeatingCapacityUser, 2) + " [W]");
                            ShowContinueError("differs from Design Size Rated Total Heating Capacity of " + RoundSigDigits(HeatingCapacityDes, 2) +
                                              " [W]");
                            ShowContinueError("This may, or may not, indicate mismatched component sizes.");
                            ShowContinueError("Verify that the value entered is intended and is consistent with other components.");
                        }
                    }
                }
            }

            if (FoundAll) {
                // autosize resistive defrost heater capacity
                IsAutoSize = false;
                if (VRF(VRFCond).DefrostCapacity == AutoSize) {
                    IsAutoSize = true;
                }
                if (VRF(VRFCond).DefrostStrategy == Resistive) {
                    DefrostCapacityDes = VRF(VRFCond).CoolingCapacity;
                } else {
                    DefrostCapacityDes = 0.0;
                }
                if (IsAutoSize) {
                    VRF(VRFCond).DefrostCapacity = DefrostCapacityDes;
                    BaseSizer::reportSizerOutput(cVRFTypes(VRF(VRFCond).VRFSystemTypeNum),
                                                 VRF(VRFCond).Name,
                                                 "Design Size Resistive Defrost Heater Capacity",
                                                 DefrostCapacityDes);
                } else {
                    if (VRF(VRFCond).DefrostCapacity > 0.0 && DefrostCapacityDes > 0.0) {
                        DefrostCapacityUser = VRF(VRFCond).DefrostCapacity;
                        BaseSizer::reportSizerOutput(cVRFTypes(VRF(VRFCond).VRFSystemTypeNum),
                                                     VRF(VRFCond).Name,
                                                     "Design Size Resistive Defrost Heater Capacity",
                                                     DefrostCapacityDes,
                                                     "User-Specified Resistive Defrost Heater Capacity",
                                                     DefrostCapacityUser);
                        if (DisplayExtraWarnings) {
                            if ((std::abs(DefrostCapacityDes - DefrostCapacityUser) / DefrostCapacityUser) > AutoVsHardSizingThreshold) {
                                ShowMessage("SizeVRF: Potential issue with equipment sizing for " + cVRFTypes(VRF(VRFCond).VRFSystemTypeNum) + ' ' +
                                            VRFTU(VRFCond).Name);
                                ShowContinueError("User-Specified Resistive Defrost Heater Capacity of " + RoundSigDigits(DefrostCapacityUser, 2) +
                                                  " [W]");
                                ShowContinueError("differs from Design Size Resistive Defrost Heater Capacity of " +
                                                  RoundSigDigits(DefrostCapacityDes, 2) + " [W]");
                                ShowContinueError("This may, or may not, indicate mismatched component sizes.");
                                ShowContinueError("Verify that the value entered is intended and is consistent with other components.");
                            }
                        }
                    }
                }

                IsAutoSize = false;
                if (VRF(VRFCond).EvapCondAirVolFlowRate == AutoSize) {
                    IsAutoSize = true;
                }
                // Auto size condenser air flow to Total Capacity * 0.000114 m3/s/w (850 cfm/ton)
                EvapCondAirVolFlowRateDes = VRF(VRFCond).CoolingCapacity * 0.000114;
                if (IsAutoSize) {
                    VRF(VRFCond).EvapCondAirVolFlowRate = EvapCondAirVolFlowRateDes;
                    BaseSizer::reportSizerOutput(cVRFTypes(VRF(VRFCond).VRFSystemTypeNum),
                                                 VRF(VRFCond).Name,
                                                 "Design Size Evaporative Condenser Air Flow Rate [m3/s]",
                                                 EvapCondAirVolFlowRateDes);
                } else {
                    if (VRF(VRFCond).EvapCondAirVolFlowRate > 0.0 && EvapCondAirVolFlowRateDes > 0.0) {
                        EvapCondAirVolFlowRateUser = VRF(VRFCond).EvapCondAirVolFlowRate;
                        BaseSizer::reportSizerOutput(cVRFTypes(VRF(VRFCond).VRFSystemTypeNum),
                                                     VRF(VRFCond).Name,
                                                     "Design Size Evaporative Condenser Air Flow Rate [m3/s]",
                                                     EvapCondAirVolFlowRateDes,
                                                     "User-Specified Evaporative Condenser Air Flow Rate [m3/s]",
                                                     EvapCondAirVolFlowRateUser);
                        if (DisplayExtraWarnings) {
                            if ((std::abs(EvapCondAirVolFlowRateDes - EvapCondAirVolFlowRateUser) / EvapCondAirVolFlowRateUser) >
                                AutoVsHardSizingThreshold) {
                                ShowMessage("SizeVRF: Potential issue with equipment sizing for " + cVRFTypes(VRF(VRFCond).VRFSystemTypeNum) + ' ' +
                                            VRFTU(VRFCond).Name);
                                ShowContinueError("User-Specified Evaporative Condenser Air Flow Rate of " +
                                                  RoundSigDigits(EvapCondAirVolFlowRateUser, 5) + " [m3/s]");
                                ShowContinueError("differs from Design Size Evaporative Condenser Air Flow Rate of " +
                                                  RoundSigDigits(EvapCondAirVolFlowRateDes, 5) + " [m3/s]");
                                ShowContinueError("This may, or may not, indicate mismatched component sizes.");
                                ShowContinueError("Verify that the value entered is intended and is consistent with other components.");
                            }
                        }
                    }
                }

                IsAutoSize = false;
                if (VRF(VRFCond).EvapCondPumpPower == AutoSize) {
                    IsAutoSize = true;
                }
                // Auto size evap condenser pump power to Total Capacity * 0.004266 w/w (15 w/ton)
                EvapCondPumpPowerDes = VRF(VRFCond).CoolingCapacity * 0.004266;
                if (IsAutoSize) {
                    VRF(VRFCond).EvapCondPumpPower = EvapCondPumpPowerDes;
                    BaseSizer::reportSizerOutput(cVRFTypes(VRF(VRFCond).VRFSystemTypeNum),
                                                 VRF(VRFCond).Name,
                                                 "Design Size Evaporative Condenser Pump Rated Power Consumption [W]",
                                                 EvapCondPumpPowerDes);

                } else {
                    if (VRF(VRFCond).EvapCondPumpPower > 0.0 && EvapCondPumpPowerDes > 0.0) {
                        EvapCondPumpPowerUser = VRF(VRFCond).EvapCondPumpPower;
                        BaseSizer::reportSizerOutput(cVRFTypes(VRF(VRFCond).VRFSystemTypeNum),
                                                     VRF(VRFCond).Name,
                                                     "Design Size Evaporative Condenser Pump Rated Power Consumption [W]",
                                                     EvapCondPumpPowerDes,
                                                     "User-Specified Evaporative Condenser Pump Rated Power Consumption [W]",
                                                     EvapCondPumpPowerUser);
                        if (DisplayExtraWarnings) {
                            if ((std::abs(EvapCondPumpPowerDes - EvapCondPumpPowerUser) / EvapCondPumpPowerUser) > AutoVsHardSizingThreshold) {
                                ShowMessage("SizeVRF: Potential issue with equipment sizing for " + cVRFTypes(VRF(VRFCond).VRFSystemTypeNum) + ' ' +
                                            VRFTU(VRFCond).Name);
                                ShowContinueError("User-Specified Evaporative Condenser Pump Rated Power Consumption of " +
                                                  RoundSigDigits(EvapCondPumpPowerUser, 2) + " [W]");
                                ShowContinueError("differs from Design Size Evaporative Condenser Pump Rated Power Consumption of " +
                                                  RoundSigDigits(EvapCondPumpPowerDes, 2) + " [W]");
                                ShowContinueError("This may, or may not, indicate mismatched component sizes.");
                                ShowContinueError("Verify that the value entered is intended and is consistent with other components.");
                            }
                        }
                    }
                }

                // Report to eio other information not related to autosizing
                if (MyOneTimeEIOFlag) {
                    static constexpr auto Format_990(
                        "! <VRF System Information>, VRF System Type, VRF System Name, VRF System Cooling Combination Ratio, VRF "
                        "System Heating Combination Ratio, VRF System Cooling Piping Correction Factor, VRF System Heating Piping "
                        "Correction Factor\n");
                    print(state.files.eio, Format_990);
                    MyOneTimeEIOFlag = false;
                }
                static constexpr auto Format_991(" VRF System Information, {}, {}, {:.5R}, {:.5R}, {:.5R}, {:.5R}\n");
                print(state.files.eio,
                      Format_991,
                      cVRFTypes(VRF(VRFCond).VRFSystemTypeNum),
                      VRF(VRFCond).Name,
                      VRF(VRFCond).CoolingCombinationRatio,
                      VRF(VRFCond).HeatingCombinationRatio,
                      VRF(VRFCond).PipingCorrectionCooling,
                      VRF(VRFCond).PipingCorrectionHeating);

                CheckVRFCombinationRatio(VRFCond) = false;
            }
        }

        DataScalableCapSizingON = false;
    }

    void VRFCondenserEquipment::SizeVRFCondenser(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Richard Raustad, FSEC
        //       DATE WRITTEN   August 2012
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine is for sizing VRF Condenser.

        // METHODOLOGY EMPLOYED:
        // Set water-cooled plant flow rates.

        static std::string const RoutineName("SizeVRFCondenser");

        int PltSizCondNum;         // Plant Sizing index for condenser loop
        Real64 rho;                // local fluid density [kg/m3]
        Real64 Cp;                 // local fluid specific heat [J/kg-k]
        Real64 tmpCondVolFlowRate; // local condenser design volume flow rate [m3/s]
        bool ErrorsFound;          // indicates problem with sizing

        // save the design water flow rate for use by the water loop sizing algorithms
        if (this->CondenserType == DataHVACGlobals::WaterCooled) {

            ErrorsFound = false;
            PltSizCondNum = 0;

            if (this->WaterCondVolFlowRate == DataSizing::AutoSize) {
                if (this->SourceLoopNum > 0) PltSizCondNum = PlantLoop(this->SourceLoopNum).PlantSizNum;
                if (PltSizCondNum > 0) {
                    rho = FluidProperties::GetDensityGlycol(state,
                                                            PlantLoop(this->SourceLoopNum).FluidName,
                                                            DataSizing::PlantSizData(PltSizCondNum).ExitTemp,
                                                            PlantLoop(this->SourceLoopNum).FluidIndex,
                                                            RoutineName);

                    Cp = FluidProperties::GetSpecificHeatGlycol(state,
                                                                PlantLoop(this->SourceLoopNum).FluidName,
                                                                DataSizing::PlantSizData(PltSizCondNum).ExitTemp,
                                                                PlantLoop(this->SourceLoopNum).FluidIndex,
                                                                RoutineName);
                    tmpCondVolFlowRate =
                        max(this->CoolingCapacity, this->HeatingCapacity) / (DataSizing::PlantSizData(PltSizCondNum).DeltaT * Cp * rho);
                    if (this->HeatingCapacity != DataSizing::AutoSize && this->CoolingCapacity != DataSizing::AutoSize) {
                        this->WaterCondVolFlowRate = tmpCondVolFlowRate;
                        BaseSizer::reportSizerOutput("AirConditioner:VariableRefrigerantFlow",
                                                     this->Name,
                                                     "Design Condenser Water Flow Rate [m3/s]",
                                                     this->WaterCondVolFlowRate);
                    }

                    rho = FluidProperties::GetDensityGlycol(
                        state, PlantLoop(this->SourceLoopNum).FluidName, CWInitConvTemp, PlantLoop(this->SourceLoopNum).FluidIndex, RoutineName);
                    this->WaterCondenserDesignMassFlow = this->WaterCondVolFlowRate * rho;
                    PlantUtilities::InitComponentNodes(0.0,
                                                       this->WaterCondenserDesignMassFlow,
                                                       this->CondenserNodeNum,
                                                       this->CondenserOutletNodeNum,
                                                       this->SourceLoopNum,
                                                       this->SourceLoopSideNum,
                                                       this->SourceBranchNum,
                                                       this->SourceCompNum);

                } else {
                    ShowSevereError("Autosizing of condenser water flow rate requires a condenser loop Sizing:Plant object");
                    ShowContinueError("... occurs in AirConditioner:VariableRefrigerantFlow object=" + this->Name);
                    ShowContinueError("... plant loop name must be referenced in Sizing:Plant object");
                    ErrorsFound = true;
                }
            }

            if (ErrorsFound) {
                ShowFatalError("Preceding sizing errors cause program termination");
            }

            PlantUtilities::RegisterPlantCompDesignFlow(this->CondenserNodeNum, this->WaterCondVolFlowRate);
        }
    }

    void SimVRF(EnergyPlusData &state,
                int const VRFTUNum,
                bool const FirstHVACIteration,
                Real64 &OnOffAirFlowRatio,
                Real64 &SysOutputProvided,
                Real64 &LatOutputProvided,
                Real64 const QZnReq)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Richard Raustad, FSEC
        //       DATE WRITTEN   August 2010
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine simulates the VRF TU's.

        // METHODOLOGY EMPLOYED:
        // Simulate terminal unit to meet zone load.

        Real64 PartLoadRatio(1.0);
        Real64 SuppHeatCoilLoad(0.0); // supplemental heating coil load (W)

        if (VRF(VRFTU(VRFTUNum).VRFSysNum).VRFAlgorithmTypeNum == AlgorithmTypeFluidTCtrl) {
            // Algorithm Type: VRF model based on physics, applicable for Fluid Temperature Control
            VRFTU(VRFTUNum).ControlVRF_FluidTCtrl(state, VRFTUNum, QZnReq, FirstHVACIteration, PartLoadRatio, OnOffAirFlowRatio, SuppHeatCoilLoad);
            VRFTU(VRFTUNum).CalcVRF_FluidTCtrl(
                state, VRFTUNum, FirstHVACIteration, PartLoadRatio, SysOutputProvided, OnOffAirFlowRatio, SuppHeatCoilLoad, LatOutputProvided);
            if (PartLoadRatio ==
                0.0) { // set coil inlet conditions when coil does not operate. Inlet conditions are set in ControlVRF_FluidTCtrl when PLR=1
                if (VRFTU(VRFTUNum).CoolingCoilPresent) {
                    VRFTU(VRFTUNum).coilInNodeT = DataLoopNode::Node(DXCoils::DXCoil(VRFTU(VRFTUNum).CoolCoilIndex).AirInNode).Temp;
                    VRFTU(VRFTUNum).coilInNodeW = DataLoopNode::Node(DXCoils::DXCoil(VRFTU(VRFTUNum).CoolCoilIndex).AirInNode).HumRat;
                } else {
                    VRFTU(VRFTUNum).coilInNodeT = DataLoopNode::Node(DXCoils::DXCoil(VRFTU(VRFTUNum).HeatCoilIndex).AirInNode).Temp;
                    VRFTU(VRFTUNum).coilInNodeW = DataLoopNode::Node(DXCoils::DXCoil(VRFTU(VRFTUNum).HeatCoilIndex).AirInNode).HumRat;
                }
            }
            // CalcVRF( VRFTUNum, FirstHVACIteration, PartLoadRatio, SysOutputProvided, OnOffAirFlowRatio, LatOutputProvided );
        } else {
            // Algorithm Type: VRF model based on system curve
            VRFTU(VRFTUNum).ControlVRF(state, VRFTUNum, QZnReq, FirstHVACIteration, PartLoadRatio, OnOffAirFlowRatio, SuppHeatCoilLoad);
            VRFTU(VRFTUNum).CalcVRF(
                state, VRFTUNum, FirstHVACIteration, PartLoadRatio, SysOutputProvided, OnOffAirFlowRatio, SuppHeatCoilLoad, LatOutputProvided);
        }

        VRFTU(VRFTUNum).TerminalUnitSensibleRate = SysOutputProvided;
        VRFTU(VRFTUNum).TerminalUnitLatentRate = LatOutputProvided;
    }

    void VRFTerminalUnitEquipment::ControlVRF(EnergyPlusData &state,
                                              int const VRFTUNum,            // Index to VRF terminal unit
                                              Real64 const QZnReq,           // Index to zone number
                                              bool const FirstHVACIteration, // flag for 1st HVAC iteration in the time step
                                              Real64 &PartLoadRatio,         // unit part load ratio
                                              Real64 &OnOffAirFlowRatio,     // ratio of compressor ON airflow to AVERAGE airflow over timestep
                                              Real64 &SuppHeatCoilLoad       // supplemental heating coil load (W)
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Richard Raustad
        //       DATE WRITTEN   July 2005

        // PURPOSE OF THIS SUBROUTINE:
        // Determine the part load fraction of the heat pump for this time step.

        // METHODOLOGY EMPLOYED:
        // Use RegulaFalsi technique to iterate on part-load ratio until convergence is achieved.

        PartLoadRatio = 0.0;
        LoopDXCoolCoilRTF = 0.0;
        LoopDXHeatCoilRTF = 0.0;

        // The RETURNS here will jump back to SimVRF where the CalcVRF routine will simulate with latest PLR

        // do nothing else if TU is scheduled off
        if (ScheduleManager::GetCurrentScheduleValue(this->SchedPtr) == 0.0) return;

        // do nothing if TU has no load (TU will be modeled using PLR=0)
        if (QZnReq == 0.0) return;

        // Set EMS value for PLR and return
        if (this->EMSOverridePartLoadFrac) {
            PartLoadRatio = this->EMSValueForPartLoadFrac;
            return;
        }

        // Get result when DX coil is operating at the minimum PLR (1E-20) if not otherwise specified
        PartLoadRatio = this->MinOperatingPLR;

        this->ControlVRFToLoad(state, VRFTUNum, QZnReq, FirstHVACIteration, PartLoadRatio, OnOffAirFlowRatio, SuppHeatCoilLoad);
    }

    void VRFTerminalUnitEquipment::ControlVRFToLoad(EnergyPlusData &state,
                                                    int const VRFTUNum,
                                                    Real64 const QZnReq,
                                                    bool const FirstHVACIteration,
                                                    Real64 &PartLoadRatio,
                                                    Real64 &OnOffAirFlowRatio,
                                                    Real64 &SuppHeatCoilLoad)
    {

        int const MaxIte(500);        // maximum number of iterations
        Real64 const MinPLF(0.0);     // minimum part load factor allowed
        Real64 const ErrorTol(0.001); // tolerance for RegulaFalsi iterations
        static ObjexxFCL::gio::Fmt fmtLD("*");

        int VRFCond = this->VRFSysNum;
        Real64 FullOutput = 0.0;   // unit full output when compressor is operating [W]
        Real64 TempOutput = 0.0;   // unit output when iteration limit exceeded [W]
        int SolFla = 0;            // Flag of RegulaFalsi solver
        Array1D<Real64> Par(6);    // Parameters passed to RegulaFalsi
        Real64 TempMinPLR = 0.0;   // min PLR used in Regula Falsi call
        Real64 TempMaxPLR = 0.0;   // max PLR used in Regula Falsi call
        bool ContinueIter;         // used when convergence is an issue
        Real64 NoCompOutput = 0.0; // output when no active compressor [W]
        bool VRFCoolingMode = CoolingLoad(VRFCond);
        bool VRFHeatingMode = HeatingLoad(VRFCond);
        int IndexToTUInTUList = this->IndexToTUInTUList;
        int TUListIndex = VRF(VRFCond).ZoneTUListPtr;
        bool HRCoolingMode = TerminalUnitList(TUListIndex).HRCoolRequest(IndexToTUInTUList);
        bool HRHeatingMode = TerminalUnitList(TUListIndex).HRHeatRequest(IndexToTUInTUList);

        if (VRF(VRFCond).VRFAlgorithmTypeNum == AlgorithmTypeFluidTCtrl) {
            // Algorithm Type: VRF model based on physics, applicable for Fluid Temperature Control
            this->CalcVRF_FluidTCtrl(state, VRFTUNum, FirstHVACIteration, PartLoadRatio, NoCompOutput, OnOffAirFlowRatio, SuppHeatCoilLoad);
        } else {
            // Algorithm Type: VRF model based on system curve
            this->CalcVRF(state, VRFTUNum, FirstHVACIteration, PartLoadRatio, NoCompOutput, OnOffAirFlowRatio, SuppHeatCoilLoad);
        }

        if (VRFCoolingMode && HRHeatingMode) {
            // IF the system is in cooling mode, but the terminal unit requests heating (heat recovery)
            if (NoCompOutput >= QZnReq) {
                PartLoadRatio = 0.0;
                return;
            }
        } else if (VRFHeatingMode && HRCoolingMode) {
            // IF the system is in heating mode, but the terminal unit requests cooling (heat recovery)
            if (NoCompOutput <= QZnReq) {
                PartLoadRatio = 0.0;
                return;
            }
        } else if (VRFCoolingMode || HRCoolingMode) {
            // IF the system is in cooling mode and/or the terminal unit requests cooling
            if (NoCompOutput <= QZnReq) {
                PartLoadRatio = 0.0;
                return;
            }
        } else if (VRFHeatingMode || HRHeatingMode) {
            // IF the system is in heating mode and/or the terminal unit requests heating
            if (NoCompOutput >= QZnReq) {
                PartLoadRatio = 0.0;
                return;
            }
        }

        // Otherwise the coil needs to turn on. Get full load result
        PartLoadRatio = 1.0;
        if (VRF(VRFCond).VRFAlgorithmTypeNum == AlgorithmTypeFluidTCtrl) {
            // Algorithm Type: VRF model based on physics, applicable for Fluid Temperature Control
            this->CalcVRF_FluidTCtrl(state, VRFTUNum, FirstHVACIteration, PartLoadRatio, FullOutput, OnOffAirFlowRatio, SuppHeatCoilLoad);
        } else {
            // Algorithm Type: VRF model based on system curve
            this->CalcVRF(state, VRFTUNum, FirstHVACIteration, PartLoadRatio, FullOutput, OnOffAirFlowRatio, SuppHeatCoilLoad);
        }

        // set supplemental heating coil calculation if the condition requires
        if (this->SuppHeatingCoilPresent) {
            if ((QZnReq > DataHVACGlobals::SmallLoad && QZnReq > FullOutput) ||
                (this->isSetPointControlled && this->suppTempSetPoint > DataLoopNode::Node(this->SuppHeatCoilAirInletNode).Temp)) {
                Real64 ZoneLoad = 0.0;
                Real64 LoadToHeatingSP = 0.0;
                Real64 LoadToCoolingSP = 0.0;
                if (this->isSetPointControlled) {
                    Real64 mDot = DataLoopNode::Node(this->SuppHeatCoilAirInletNode).MassFlowRate;
                    Real64 Tin = DataLoopNode::Node(this->SuppHeatCoilAirInletNode).Temp;
                    Real64 Win = DataLoopNode::Node(this->SuppHeatCoilAirInletNode).HumRat;
                    Real64 CpAirIn = Psychrometrics::PsyCpAirFnW(Win);
                    SuppHeatCoilLoad = mDot * CpAirIn * (this->suppTempSetPoint - Tin);
                    this->SuppHeatingCoilLoad = SuppHeatCoilLoad;
                    if (this->DesignSuppHeatingCapacity > 0.0) {
                        this->SuppHeatPartLoadRatio = min(1.0, SuppHeatCoilLoad / this->DesignSuppHeatingCapacity);
                    }
                } else {
                    getVRFTUZoneLoad(VRFTUNum, ZoneLoad, LoadToHeatingSP, LoadToCoolingSP, false);
                    if ((FullOutput < (LoadToHeatingSP - DataHVACGlobals::SmallLoad)) && !FirstHVACIteration) {
                        SuppHeatCoilLoad = max(0.0, LoadToHeatingSP - FullOutput);
                        this->SuppHeatingCoilLoad = SuppHeatCoilLoad;
                        if (this->DesignSuppHeatingCapacity > 0.0) {
                            this->SuppHeatPartLoadRatio = min(1.0, SuppHeatCoilLoad / this->DesignSuppHeatingCapacity);
                        }
                    } else {
                        SuppHeatCoilLoad = 0.0;
                        this->SuppHeatPartLoadRatio = 0.0;
                    }
                }
            } else {
                SuppHeatCoilLoad = 0.0;
                this->SuppHeatPartLoadRatio = 0.0;
            }
        } else { // does it matter what these are if there is no supp heater?
            SuppHeatCoilLoad = 0.0;
            this->SuppHeatPartLoadRatio = 0.0;
        }

        if ((VRFCoolingMode && !VRF(VRFCond).HeatRecoveryUsed) || (VRF(VRFCond).HeatRecoveryUsed && HRCoolingMode)) {
            // Since we are cooling, we expect FullOutput < NoCompOutput
            // If the QZnReq <= FullOutput the unit needs to run full out
            if (QZnReq <= FullOutput) {
                // if no coil present in terminal unit, no need to reset PLR?
                if (VRFTU(VRFTUNum).CoolingCoilPresent) {
                    PartLoadRatio = 1.0;
                    // the zone set point could be exceeded if set point control is used so protect against that
                    if (this->isSetPointControlled) {
                        if (DataLoopNode::Node(this->coolCoilAirOutNode).Temp > this->coilTempSetPoint) return;
                    } else {
                        return;
                    }
                } else {
                    PartLoadRatio = 0.0;
                    return;
                }
            }
        } else if ((VRFHeatingMode && !VRF(VRFCond).HeatRecoveryUsed) || (VRF(VRFCond).HeatRecoveryUsed && HRHeatingMode)) {
            // Since we are heating, we expect FullOutput > NoCompOutput
            // If the QZnReq >= FullOutput the unit needs to run full out
            if (QZnReq >= FullOutput) {
                // if no coil present in terminal unit, no need reset PLR?
                if (this->HeatingCoilPresent) {
                    PartLoadRatio = 1.0;
                    // the zone set point could be exceeded if set point control is used so protect against that
                    if (this->isSetPointControlled) {
                        if (DataLoopNode::Node(this->heatCoilAirOutNode).Temp < this->coilTempSetPoint) return;
                    } else {
                        return;
                    }
                } else {
                    PartLoadRatio = 0.0;
                    return;
                }
            }
        } else {
            // VRF terminal unit is off
            // shouldn't actually get here
            PartLoadRatio = 0.0;
            return;
        }

        // The coil will not operate at PLR=0 or PLR=1, calculate the operating part-load ratio

        if ((VRFHeatingMode || HRHeatingMode) || (VRFCoolingMode || HRCoolingMode)) {

            Par(1) = VRFTUNum;
            Par(2) = 0.0;
            if (VRFTU(VRFTUNum).isSetPointControlled) Par(2) = 1.0;
            Par(4) = 0.0; // fan OpMode
            if (FirstHVACIteration) {
                Par(3) = 1.0;
            } else {
                Par(3) = 0.0;
            }
            //    Par(4) = OpMode
            Par(5) = QZnReq;
            Par(6) = OnOffAirFlowRatio;
            TempSolveRoot::SolveRoot(state, ErrorTol, MaxIte, SolFla, PartLoadRatio, PLRResidual, 0.0, 1.0, Par);
            if (SolFla == -1) {
                //     Very low loads may not converge quickly. Tighten PLR boundary and try again.
                TempMaxPLR = -0.1;
                ContinueIter = true;
                while (ContinueIter && TempMaxPLR < 1.0) {
                    TempMaxPLR += 0.1;

                    if (VRF(VRFCond).VRFAlgorithmTypeNum == AlgorithmTypeFluidTCtrl) {
                        // Algorithm Type: VRF model based on physics, applicable for Fluid Temperature Control
                        this->CalcVRF_FluidTCtrl(state, VRFTUNum, FirstHVACIteration, TempMaxPLR, TempOutput, OnOffAirFlowRatio, SuppHeatCoilLoad);
                    } else {
                        // Algorithm Type: VRF model based on system curve
                        this->CalcVRF(state, VRFTUNum, FirstHVACIteration, TempMaxPLR, TempOutput, OnOffAirFlowRatio, SuppHeatCoilLoad);
                    }

                    if (VRFHeatingMode && TempOutput > QZnReq) ContinueIter = false;
                    if (VRFCoolingMode && TempOutput < QZnReq) ContinueIter = false;
                }
                TempMinPLR = TempMaxPLR;
                ContinueIter = true;
                while (ContinueIter && TempMinPLR > 0.0) {
                    TempMaxPLR = TempMinPLR;
                    TempMinPLR -= 0.01;

                    if (VRF(VRFCond).VRFAlgorithmTypeNum == AlgorithmTypeFluidTCtrl) {
                        // Algorithm Type: VRF model based on physics, applicable for Fluid Temperature Control
                        this->CalcVRF_FluidTCtrl(state, VRFTUNum, FirstHVACIteration, TempMinPLR, TempOutput, OnOffAirFlowRatio, SuppHeatCoilLoad);
                    } else {
                        // Algorithm Type: VRF model based on system curve
                        this->CalcVRF(state, VRFTUNum, FirstHVACIteration, TempMinPLR, TempOutput, OnOffAirFlowRatio, SuppHeatCoilLoad);
                    }

                    if (VRFHeatingMode && TempOutput < QZnReq) ContinueIter = false;
                    if (VRFCoolingMode && TempOutput > QZnReq) ContinueIter = false;
                }
                TempSolveRoot::SolveRoot(state, ErrorTol, MaxIte, SolFla, PartLoadRatio, PLRResidual, TempMinPLR, TempMaxPLR, Par);
                if (SolFla == -1) {
                    if (!FirstHVACIteration && !WarmupFlag) {
                        if (this->IterLimitExceeded == 0) {
                            ShowWarningMessage(DataHVACGlobals::cVRFTUTypes(this->VRFTUType_Num) + " \"" + this->Name + "\"");
                            ShowContinueError(
                                format(" Iteration limit exceeded calculating terminal unit part-load ratio, maximum iterations = {}", MaxIte));
                            ShowContinueErrorTimeStamp(" Part-load ratio returned = " + General::RoundSigDigits(PartLoadRatio, 3));

                            if (VRF(VRFCond).VRFAlgorithmTypeNum == AlgorithmTypeFluidTCtrl) {
                                // Algorithm Type: VRF model based on physics, applicable for Fluid Temperature Control
                                this->CalcVRF_FluidTCtrl(
                                    state, VRFTUNum, FirstHVACIteration, PartLoadRatio, TempOutput, OnOffAirFlowRatio, SuppHeatCoilLoad);
                            } else {
                                // Algorithm Type: VRF model based on system curve
                                this->CalcVRF(state, VRFTUNum, FirstHVACIteration, PartLoadRatio, TempOutput, OnOffAirFlowRatio, SuppHeatCoilLoad);
                            }

                            ShowContinueError(" Load requested = " + General::TrimSigDigits(QZnReq, 5) +
                                              ", Load delivered = " + General::TrimSigDigits(TempOutput, 5));
                            ShowRecurringWarningErrorAtEnd(DataHVACGlobals::cVRFTUTypes(this->VRFTUType_Num) + " \"" + this->Name +
                                                               "\" -- Terminal unit Iteration limit exceeded error continues...",
                                                           this->IterLimitExceeded);
                        } else {
                            ShowRecurringWarningErrorAtEnd(DataHVACGlobals::cVRFTUTypes(this->VRFTUType_Num) + " \"" + this->Name +
                                                               "\" -- Terminal unit Iteration limit exceeded error continues...",
                                                           this->IterLimitExceeded);
                        }
                    }
                } else if (SolFla == -2) {
                    if (!FirstHVACIteration && !WarmupFlag) {
                        if (VRFTU(VRFTUNum).FirstIterfailed == 0) {
                            ShowWarningMessage(DataHVACGlobals::cVRFTUTypes(this->VRFTUType_Num) + " \"" + this->Name + "\"");
                            ShowContinueError("Terminal unit part-load ratio calculation failed: PLR limits of 0 to 1 exceeded");
                            ShowContinueError("Please fill out a bug report and forward to the EnergyPlus support group.");
                            ShowContinueErrorTimeStamp("");
                            if (WarmupFlag) ShowContinueError("Error occurred during warmup days.");
                            ShowRecurringWarningErrorAtEnd(DataHVACGlobals::cVRFTUTypes(this->VRFTUType_Num) + " \"" + this->Name +
                                                               "\" -- Terminal unit part-load ratio limits of 0 to 1 exceeded error continues...",
                                                           this->FirstIterfailed);
                        } else {
                            ShowRecurringWarningErrorAtEnd(DataHVACGlobals::cVRFTUTypes(this->VRFTUType_Num) + " \"" + this->Name +
                                                               "\" -- Terminal unit part-load ratio limits of 0 to 1 exceeded error continues...",
                                                           VRFTU(VRFTUNum).FirstIterfailed);
                        }
                    }
                    PartLoadRatio = max(MinPLF, std::abs(QZnReq - NoCompOutput) / std::abs(FullOutput - NoCompOutput));
                }
            } else if (SolFla == -2) {
                if (!FirstHVACIteration && !WarmupFlag) {
                    if (VRFTU(VRFTUNum).FirstIterfailed == 0) {
                        ShowWarningMessage(DataHVACGlobals::cVRFTUTypes(this->VRFTUType_Num) + " \"" + this->Name + "\"");
                        ShowContinueError("Terminal unit part-load ratio calculation failed: PLR limits of 0 to 1 exceeded");
                        ShowContinueError("Please fill out a bug report and forward to the EnergyPlus support group.");
                        ShowContinueErrorTimeStamp("");
                        if (WarmupFlag) ShowContinueError("Error occurred during warmup days.");
                        ShowRecurringWarningErrorAtEnd(DataHVACGlobals::cVRFTUTypes(this->VRFTUType_Num) + " \"" + this->Name +
                                                           "\" -- Terminal unit part-load ratio limits of 0 to 1 exceeded error continues...",
                                                       this->FirstIterfailed);
                    } else {
                        ShowRecurringWarningErrorAtEnd(DataHVACGlobals::cVRFTUTypes(this->VRFTUType_Num) + " \"" + this->Name +
                                                           "\" -- Terminal unit part-load ratio limits of 0 to 1 exceeded error continues...",
                                                       this->FirstIterfailed);
                    }
                }
                if (FullOutput - NoCompOutput == 0.0) {
                    PartLoadRatio = 0.0;
                } else {
                    PartLoadRatio = min(1.0, max(MinPLF, std::abs(QZnReq - NoCompOutput) / std::abs(FullOutput - NoCompOutput)));
                }
            }
        }
    }

    void VRFTerminalUnitEquipment::CalcVRF(EnergyPlusData &state,
                                           int const VRFTUNum,                // Unit index in VRF terminal unit array
                                           bool const FirstHVACIteration,     // flag for 1st HVAC iteration in the time step
                                           Real64 const PartLoadRatio,        // compressor part load fraction
                                           Real64 &LoadMet,                   // load met by unit (W)
                                           Real64 &OnOffAirFlowRatio,         // ratio of ON air flow to average air flow
                                           Real64 &SuppHeatCoilLoad,          // supplemental heating coil load (W)
                                           Optional<Real64> LatOutputProvided // delivered latent capacity (kgWater/s)
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Richard Raustad
        //       DATE WRITTEN   July 2005
        //       MODIFIED       July 2012, Chandan Sharma - FSEC: Added zone sys avail managers
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Simulate the components making up the VRF terminal unit.

        // METHODOLOGY EMPLOYED:
        // Simulates the unit components sequentially in the air flow direction.

        using DataZoneEquipment::ZoneEquipConfig;
        using DXCoils::SimDXCoil;
        using HeatingCoils::SimulateHeatingCoilComponents;
        using MixedAir::SimOAMixer;
        using SingleDuct::SimATMixer;
        using SteamCoils::SimulateSteamCoilComponents;
        using WaterCoils::SimulateWaterCoilComponents;

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int VRFTUOutletNodeNum;     // TU air outlet node
        int VRFTUInletNodeNum;      // TU air inlet node
        Real64 AirMassFlow;         // total supply air mass flow [m3/s]
        int OpMode;                 // fan operating mode, DataHVACGlobals::CycFanCycCoil or DataHVACGlobals::ContFanCycCoil
        int VRFCond;                // index to VRF condenser
        Real64 SpecHumOut(0.0);     // specific humidity ratio at outlet node
        Real64 SpecHumIn(0.0);      // specific humidity ratio at inlet node
        int TUListIndex;            // index to TU list for this VRF system
        int IndexToTUInTUList;      // index to TU in specific list for the VRF system
        static int ATMixOutNode(0); // terminal unit mixer outlet node
        int ZoneNode;               // Zone node of VRFTU is serving

        VRFCond = this->VRFSysNum;
        TUListIndex = VRF(VRFCond).ZoneTUListPtr;
        IndexToTUInTUList = this->IndexToTUInTUList;
        VRFTUOutletNodeNum = this->VRFTUOutletNodeNum;
        VRFTUInletNodeNum = this->VRFTUInletNodeNum;
        OpMode = this->OpMode;
        ZoneNode = this->ZoneAirNode;

        // Set inlet air mass flow rate based on PLR and compressor on/off air flow rates
        SetAverageAirFlow(VRFTUNum, PartLoadRatio, OnOffAirFlowRatio);

        AirMassFlow = DataLoopNode::Node(VRFTUInletNodeNum).MassFlowRate;
        if (this->ATMixerExists) {
            // There is an air terminal mixer
            ATMixOutNode = this->ATMixerOutNode;
            if (this->ATMixerType == DataHVACGlobals::ATMixer_InletSide) { // if there is an inlet side air terminal mixer
                // set the primary air inlet mass flow rate
                DataLoopNode::Node(this->ATMixerPriNode).MassFlowRate =
                    min(DataLoopNode::Node(this->ATMixerPriNode).MassFlowRateMaxAvail, DataLoopNode::Node(VRFTUInletNodeNum).MassFlowRate);
                // now calculate the the mixer outlet air conditions (and the secondary air inlet flow rate). The mixer outlet flow rate has already
                // been set above (it is the "inlet" node flow rate)
                SimATMixer(state, this->ATMixerName, FirstHVACIteration, this->ATMixerIndex);
            }
        } else {
            // ATMixOutNode = 0;
            if (this->OAMixerUsed) SimOAMixer(state, this->OAMixerName, FirstHVACIteration, this->OAMixerIndex);
        }
        // if blow through, simulate fan then coils
        if (this->FanPlace == DataHVACGlobals::BlowThru) {
            if (this->fanType_Num == DataHVACGlobals::FanType_SystemModelObject) {
                if (OnOffAirFlowRatio > 0.0) {
                    //                    HVACFan::fanObjs[this->FanIndex]->simulate(1.0 / OnOffAirFlowRatio, DataHVACGlobals::ZoneCompTurnFansOff,
                    //                    DataHVACGlobals::ZoneCompTurnFansOff, _);
                    HVACFan::fanObjs[this->FanIndex]->simulate(
                        state, _, DataHVACGlobals::ZoneCompTurnFansOff, DataHVACGlobals::ZoneCompTurnFansOff, _);
                } else {
                    HVACFan::fanObjs[this->FanIndex]->simulate(
                        state, PartLoadRatio, DataHVACGlobals::ZoneCompTurnFansOff, DataHVACGlobals::ZoneCompTurnFansOff, _);
                }
            } else {
                Fans::SimulateFanComponents(state,
                                            "",
                                            FirstHVACIteration,
                                            this->FanIndex,
                                            FanSpeedRatio,
                                            DataHVACGlobals::ZoneCompTurnFansOff,
                                            DataHVACGlobals::ZoneCompTurnFansOff);
            }
        }

        if (this->CoolingCoilPresent) {
            // above condition for heat pump mode, below condition for heat recovery mode
            if ((!VRF(VRFCond).HeatRecoveryUsed && CoolingLoad(VRFCond)) ||
                (VRF(VRFCond).HeatRecoveryUsed && TerminalUnitList(TUListIndex).HRCoolRequest(IndexToTUInTUList))) {
                SimDXCoil(state,
                          "",
                          On,
                          FirstHVACIteration,
                          this->CoolCoilIndex,
                          OpMode,
                          PartLoadRatio,
                          OnOffAirFlowRatio,
                          _,
                          MaxCoolingCapacity(VRFCond),
                          VRF(this->VRFSysNum).VRFCondCyclingRatio);
            } else { // cooling coil is off
                SimDXCoil(state, "", Off, FirstHVACIteration, this->CoolCoilIndex, OpMode, 0.0, OnOffAirFlowRatio);
            }
            LoopDXCoolCoilRTF = state.dataAirLoop->LoopDXCoilRTF;
        } else {
            LoopDXCoolCoilRTF = 0.0;
        }

        if (this->HeatingCoilPresent) {
            // above condition for heat pump mode, below condition for heat recovery mode
            if ((!VRF(VRFCond).HeatRecoveryUsed && HeatingLoad(VRFCond)) ||
                (VRF(VRFCond).HeatRecoveryUsed && TerminalUnitList(TUListIndex).HRHeatRequest(IndexToTUInTUList))) {
                SimDXCoil(state,
                          "",
                          Off,
                          FirstHVACIteration,
                          this->HeatCoilIndex,
                          OpMode,
                          PartLoadRatio,
                          OnOffAirFlowRatio,
                          _,
                          MaxHeatingCapacity(VRFCond));
            } else {
                SimDXCoil(state, "", Off, FirstHVACIteration, this->HeatCoilIndex, OpMode, 0.0, OnOffAirFlowRatio, _);
            }
            LoopDXHeatCoilRTF = state.dataAirLoop->LoopDXCoilRTF;
        } else {
            LoopDXHeatCoilRTF = 0.0;
        }

        // if draw through, simulate coils then fan
        if (this->FanPlace == DataHVACGlobals::DrawThru) {
            if (this->fanType_Num == DataHVACGlobals::FanType_SystemModelObject) {
                if (OnOffAirFlowRatio > 0.0) {
                    //                    HVACFan::fanObjs[this->FanIndex]->simulate(1.0 / OnOffAirFlowRatio, DataHVACGlobals::ZoneCompTurnFansOff,
                    //                    DataHVACGlobals::ZoneCompTurnFansOff, _);
                    HVACFan::fanObjs[this->FanIndex]->simulate(
                        state, _, DataHVACGlobals::ZoneCompTurnFansOff, DataHVACGlobals::ZoneCompTurnFansOff, _);
                } else {
                    HVACFan::fanObjs[this->FanIndex]->simulate(
                        state, PartLoadRatio, DataHVACGlobals::ZoneCompTurnFansOff, DataHVACGlobals::ZoneCompTurnFansOff, _);
                }

            } else {
                Fans::SimulateFanComponents(state,
                                            "",
                                            FirstHVACIteration,
                                            this->FanIndex,
                                            FanSpeedRatio,
                                            DataHVACGlobals::ZoneCompTurnFansOff,
                                            DataHVACGlobals::ZoneCompTurnFansOff);
            }
        }

        // track fan power per terminal unit for calculating COP
        if (this->fanType_Num == DataHVACGlobals::FanType_SystemModelObject) {
            this->FanPower = HVACFan::fanObjs[this->FanIndex]->fanPower();
        } else {
            this->FanPower = Fans::GetFanPower(this->FanIndex);
        }

        // run supplemental heating coil
        if (this->SuppHeatingCoilPresent) {
            Real64 SuppPLR = this->SuppHeatPartLoadRatio;
            this->CalcVRFSuppHeatingCoil(state, VRFTUNum, FirstHVACIteration, SuppPLR, SuppHeatCoilLoad);
            if ((DataLoopNode::Node(this->SuppHeatCoilAirOutletNode).Temp > this->MaxSATFromSuppHeatCoil) && SuppPLR > 0.0) {
                // adjust the heating load to maximum allowed
                Real64 MaxHeatCoilLoad = this->HeatingCoilCapacityLimit(this->SuppHeatCoilAirInletNode, this->MaxSATFromSuppHeatCoil);
                this->CalcVRFSuppHeatingCoil(state, VRFTUNum, FirstHVACIteration, SuppPLR, MaxHeatCoilLoad);
                SuppHeatCoilLoad = MaxHeatCoilLoad;
            }
        }

        Real64 LatentLoadMet = 0.0; // latent load deleivered [kgWater/s]
        Real64 TempOut = 0.0;
        Real64 TempIn = 0.0;
        if (this->ATMixerExists) {
            if (this->ATMixerType == DataHVACGlobals::ATMixer_SupplySide) {
                // Air terminal supply side mixer, calculate supply side mixer output
                SimATMixer(state, this->ATMixerName, FirstHVACIteration, this->ATMixerIndex);
                TempOut = DataLoopNode::Node(ATMixOutNode).Temp;
                SpecHumOut = DataLoopNode::Node(ATMixOutNode).HumRat;
                AirMassFlow = DataLoopNode::Node(ATMixOutNode).MassFlowRate;
            } else {
                // Air terminal inlet side mixer
                TempOut = DataLoopNode::Node(VRFTUOutletNodeNum).Temp;
                SpecHumOut = DataLoopNode::Node(VRFTUOutletNodeNum).HumRat;
            }
            TempIn = DataLoopNode::Node(ZoneNode).Temp;
            SpecHumIn = DataLoopNode::Node(ZoneNode).HumRat;
        } else {
            TempOut = DataLoopNode::Node(VRFTUOutletNodeNum).Temp;
            SpecHumOut = DataLoopNode::Node(VRFTUOutletNodeNum).HumRat;
            if (ZoneNode > 0) {
                TempIn = DataLoopNode::Node(ZoneNode).Temp;
                SpecHumIn = DataLoopNode::Node(ZoneNode).HumRat;
            } else {
                TempIn = DataLoopNode::Node(VRFTUInletNodeNum).Temp;
                SpecHumIn = DataLoopNode::Node(VRFTUInletNodeNum).HumRat;
            }
        }
        // calculate sensible load met using delta enthalpy
        LoadMet = AirMassFlow * PsyDeltaHSenFnTdb2W2Tdb1W1(TempOut, SpecHumOut, TempIn, SpecHumIn); // sensible {W}
        LatentLoadMet = AirMassFlow * (SpecHumOut - SpecHumIn);                                     // latent {kgWater/s}
        if (present(LatOutputProvided)) {
            //   CR9155 Remove specific humidity calculations
            LatOutputProvided = LatentLoadMet;
        }
    }

    void ReportVRFTerminalUnit(int const VRFTUNum) // index to VRF terminal unit
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Richard Raustad, FSEC
        //       DATE WRITTEN   August 2010
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine updates the report variables for the VRF Terminal Units.

        using namespace DataSizing;
        using DXCoils::DXCoilTotalCooling;
        using DXCoils::DXCoilTotalHeating;

        int DXCoolingCoilIndex;      // - index to DX cooling coil
        int DXHeatingCoilIndex;      // - index to DX heating coil
        Real64 TotalConditioning;    // - sum of sensible and latent rates
        Real64 SensibleConditioning; // - sensible rate
        Real64 LatentConditioning;   // - latent rate
        Real64 ReportingConstant;    // - used to convert watts to joules
        int VRFCond;                 // - index to VRF condenser
        int TUListIndex;             // - index to terminal unit list
        int IndexToTUInTUList;       // - index to the TU in the list
        bool HRHeatRequestFlag;      // - indicates TU could be in heat mode
        bool HRCoolRequestFlag;      // - indicates TU could be in cool mode

        DXCoolingCoilIndex = VRFTU(VRFTUNum).CoolCoilIndex;
        DXHeatingCoilIndex = VRFTU(VRFTUNum).HeatCoilIndex;
        VRFCond = VRFTU(VRFTUNum).VRFSysNum;
        TUListIndex = VRF(VRFCond).ZoneTUListPtr;
        IndexToTUInTUList = VRFTU(VRFTUNum).IndexToTUInTUList;
        HRHeatRequestFlag = TerminalUnitList(TUListIndex).HRHeatRequest(IndexToTUInTUList);
        HRCoolRequestFlag = TerminalUnitList(TUListIndex).HRCoolRequest(IndexToTUInTUList);
        ReportingConstant = DataHVACGlobals::TimeStepSys * SecInHour;

        // account for terminal unit parasitic On/Off power use
        // account for heat recovery first since these flags will be FALSE otherwise, each TU may have different operating mode

        if (HRCoolRequestFlag) {
            if (VRFTU(VRFTUNum).CoolingCoilPresent) {
                VRFTU(VRFTUNum).ParasiticCoolElecPower =
                    VRFTU(VRFTUNum).ParasiticElec * LoopDXCoolCoilRTF + VRFTU(VRFTUNum).ParasiticOffElec * (1.0 - LoopDXCoolCoilRTF);
                VRFTU(VRFTUNum).ParasiticElecCoolConsumption = VRFTU(VRFTUNum).ParasiticCoolElecPower * ReportingConstant;
                VRFTU(VRFTUNum).ParasiticHeatElecPower = 0.0;
                VRFTU(VRFTUNum).ParasiticElecHeatConsumption = 0.0;
            } else {
                // cooling parasitic power report variable is not even available when there is no cooling coil, report for heating
                VRFTU(VRFTUNum).ParasiticHeatElecPower = VRFTU(VRFTUNum).ParasiticOffElec;
                VRFTU(VRFTUNum).ParasiticElecHeatConsumption = VRFTU(VRFTUNum).ParasiticHeatElecPower * ReportingConstant;
            }
        } else if (HRHeatRequestFlag) {
            if (VRFTU(VRFTUNum).HeatingCoilPresent) {
                VRFTU(VRFTUNum).ParasiticCoolElecPower = 0.0;
                VRFTU(VRFTUNum).ParasiticElecCoolConsumption = 0.0;
                VRFTU(VRFTUNum).ParasiticHeatElecPower =
                    VRFTU(VRFTUNum).ParasiticElec * LoopDXHeatCoilRTF + VRFTU(VRFTUNum).ParasiticOffElec * (1.0 - LoopDXHeatCoilRTF);
                VRFTU(VRFTUNum).ParasiticElecHeatConsumption = VRFTU(VRFTUNum).ParasiticHeatElecPower * ReportingConstant;
            } else {
                // heating parasitic power report variable is not even available when there is no heating coil, report for cooling
                VRFTU(VRFTUNum).ParasiticCoolElecPower = VRFTU(VRFTUNum).ParasiticOffElec;
                VRFTU(VRFTUNum).ParasiticElecCoolConsumption = VRFTU(VRFTUNum).ParasiticCoolElecPower * ReportingConstant;
            }
        } else if (CoolingLoad(VRFCond) || (!HeatingLoad(VRFCond) && LastModeCooling(VRFTU(VRFTUNum).VRFSysNum))) {
            if (VRFTU(VRFTUNum).CoolingCoilPresent) {
                VRFTU(VRFTUNum).ParasiticCoolElecPower =
                    VRFTU(VRFTUNum).ParasiticElec * LoopDXCoolCoilRTF + VRFTU(VRFTUNum).ParasiticOffElec * (1.0 - LoopDXCoolCoilRTF);
                VRFTU(VRFTUNum).ParasiticElecCoolConsumption = VRFTU(VRFTUNum).ParasiticCoolElecPower * ReportingConstant;
                VRFTU(VRFTUNum).ParasiticHeatElecPower = 0.0;
                VRFTU(VRFTUNum).ParasiticElecHeatConsumption = 0.0;
            } else {
                // cooling parasitic power report variable is not even available when there is no cooling coil, report for heating
                VRFTU(VRFTUNum).ParasiticHeatElecPower = VRFTU(VRFTUNum).ParasiticOffElec;
                VRFTU(VRFTUNum).ParasiticElecHeatConsumption = VRFTU(VRFTUNum).ParasiticHeatElecPower * ReportingConstant;
            }
        } else if (HeatingLoad(VRFCond) || (!CoolingLoad(VRFCond) && LastModeHeating(VRFTU(VRFTUNum).VRFSysNum))) {
            if (VRFTU(VRFTUNum).HeatingCoilPresent) {
                VRFTU(VRFTUNum).ParasiticCoolElecPower = 0.0;
                VRFTU(VRFTUNum).ParasiticElecCoolConsumption = 0.0;
                VRFTU(VRFTUNum).ParasiticHeatElecPower =
                    VRFTU(VRFTUNum).ParasiticElec * LoopDXHeatCoilRTF + VRFTU(VRFTUNum).ParasiticOffElec * (1.0 - LoopDXHeatCoilRTF);
                VRFTU(VRFTUNum).ParasiticElecHeatConsumption = VRFTU(VRFTUNum).ParasiticHeatElecPower * ReportingConstant;
            } else {
                // heating parasitic power report variable is not even available when there is no heating coil, report for cooling
                VRFTU(VRFTUNum).ParasiticCoolElecPower = VRFTU(VRFTUNum).ParasiticOffElec;
                VRFTU(VRFTUNum).ParasiticElecCoolConsumption = VRFTU(VRFTUNum).ParasiticCoolElecPower * ReportingConstant;
            }
        } else {
            // happens when there is no cooling or heating load
            if (!VRFTU(VRFTUNum).CoolingCoilPresent) {
                // report all for heating
                VRFTU(VRFTUNum).ParasiticHeatElecPower = VRFTU(VRFTUNum).ParasiticOffElec;
                VRFTU(VRFTUNum).ParasiticElecHeatConsumption = VRFTU(VRFTUNum).ParasiticHeatElecPower * ReportingConstant;
            } else if (!VRFTU(VRFTUNum).HeatingCoilPresent) {
                // report all for cooling
                VRFTU(VRFTUNum).ParasiticCoolElecPower = VRFTU(VRFTUNum).ParasiticOffElec;
                VRFTU(VRFTUNum).ParasiticElecCoolConsumption = VRFTU(VRFTUNum).ParasiticCoolElecPower * ReportingConstant;
            } else {
                // split parasitic between both reporting variables
                VRFTU(VRFTUNum).ParasiticCoolElecPower = VRFTU(VRFTUNum).ParasiticOffElec / 2.0;
                VRFTU(VRFTUNum).ParasiticElecCoolConsumption = VRFTU(VRFTUNum).ParasiticCoolElecPower * ReportingConstant;
                VRFTU(VRFTUNum).ParasiticHeatElecPower = VRFTU(VRFTUNum).ParasiticOffElec / 2.0;
                VRFTU(VRFTUNum).ParasiticElecHeatConsumption = VRFTU(VRFTUNum).ParasiticHeatElecPower * ReportingConstant;
            }
        }

        SensibleConditioning = VRFTU(VRFTUNum).TerminalUnitSensibleRate;
        LatentConditioning = VRFTU(VRFTUNum).TerminalUnitLatentRate;
        Real64 TempOut = 0.0;
        Real64 TempIn = 0.0;
        if (VRFTU(VRFTUNum).ATMixerExists) {
            if (VRFTU(VRFTUNum).ATMixerType == DataHVACGlobals::ATMixer_SupplySide) {
                // Air terminal supply side mixer
                TempOut = DataLoopNode::Node(VRFTU(VRFTUNum).ATMixerOutNode).Temp;
                TempIn = DataLoopNode::Node(VRFTU(VRFTUNum).ZoneAirNode).Temp;
            } else {
                // Air terminal inlet side mixer
                TempOut = DataLoopNode::Node(VRFTU(VRFTUNum).VRFTUOutletNodeNum).Temp;
                TempIn = DataLoopNode::Node(VRFTU(VRFTUNum).ZoneAirNode).Temp;
            }
        } else {
            TempOut = DataLoopNode::Node(VRFTU(VRFTUNum).VRFTUOutletNodeNum).Temp;
            TempIn = DataLoopNode::Node(VRFTU(VRFTUNum).VRFTUInletNodeNum).Temp;
        }
        // latent heat vaporization/condensation used in moist air psychometrics
        Real64 const H2OHtOfVap = PsyHgAirFnWTdb(0.0, TempOut);
        // convert latent in kg/s to watts
        TotalConditioning = SensibleConditioning + (LatentConditioning * H2OHtOfVap);

        if (TotalConditioning <= 0.0) {
            VRFTU(VRFTUNum).TotalCoolingRate = std::abs(TotalConditioning);
            VRFTU(VRFTUNum).TotalHeatingRate = 0.0;
        } else {
            VRFTU(VRFTUNum).TotalCoolingRate = 0.0;
            VRFTU(VRFTUNum).TotalHeatingRate = TotalConditioning;
        }
        if (SensibleConditioning <= 0.0) {
            VRFTU(VRFTUNum).SensibleCoolingRate = std::abs(SensibleConditioning);
            VRFTU(VRFTUNum).SensibleHeatingRate = 0.0;
        } else {
            VRFTU(VRFTUNum).SensibleCoolingRate = 0.0;
            VRFTU(VRFTUNum).SensibleHeatingRate = SensibleConditioning;
        }
        if (LatentConditioning <= 0.0) {
            VRFTU(VRFTUNum).LatentCoolingRate = std::abs(LatentConditioning) * H2OHtOfVap;
            VRFTU(VRFTUNum).LatentHeatingRate = 0.0;
        } else {
            VRFTU(VRFTUNum).LatentCoolingRate = 0.0;
            VRFTU(VRFTUNum).LatentHeatingRate = LatentConditioning * H2OHtOfVap;
        }
        VRFTU(VRFTUNum).TotalCoolingEnergy = VRFTU(VRFTUNum).TotalCoolingRate * ReportingConstant;
        VRFTU(VRFTUNum).SensibleCoolingEnergy = VRFTU(VRFTUNum).SensibleCoolingRate * ReportingConstant;
        VRFTU(VRFTUNum).LatentCoolingEnergy = VRFTU(VRFTUNum).LatentCoolingRate * ReportingConstant;
        VRFTU(VRFTUNum).TotalHeatingEnergy = VRFTU(VRFTUNum).TotalHeatingRate * ReportingConstant;
        VRFTU(VRFTUNum).SensibleHeatingEnergy = VRFTU(VRFTUNum).SensibleHeatingRate * ReportingConstant;
        VRFTU(VRFTUNum).LatentHeatingEnergy = VRFTU(VRFTUNum).LatentHeatingRate * ReportingConstant;

        if (VRFTU(VRFTUNum).firstPass) {
            if (!MySizeFlag(VRFTUNum)) {
                DataSizing::resetHVACSizingGlobals(DataSizing::CurZoneEqNum, 0, VRFTU(VRFTUNum).firstPass);
            }
        }

        // reset to 1 in case blow through fan configuration (fan resets to 1, but for blow thru fans coil sets back down < 1)
        DataHVACGlobals::OnOffFanPartLoadFraction = 1.0;
    }

    void ReportVRFCondenser(int const VRFCond) // index to VRF condensing unit
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Richard Raustad, FSEC
        //       DATE WRITTEN   August 2010
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine updates the report variables for the VRF Condenser.

        Real64 ReportingConstant; // - conversion constant for energy

        ReportingConstant = DataHVACGlobals::TimeStepSys * SecInHour;

        //   calculate VRF condenser power/energy use
        VRF(VRFCond).CoolElecConsumption = VRF(VRFCond).ElecCoolingPower * ReportingConstant;
        VRF(VRFCond).HeatElecConsumption = VRF(VRFCond).ElecHeatingPower * ReportingConstant;

        VRF(VRFCond).DefrostConsumption = VRF(VRFCond).DefrostPower * ReportingConstant;
        VRF(VRFCond).BasinHeaterConsumption = VRF(VRFCond).BasinHeaterPower * ReportingConstant;

        VRF(VRFCond).EvapCondPumpElecConsumption = VRF(VRFCond).EvapCondPumpElecPower * ReportingConstant;
        VRF(VRFCond).CrankCaseHeaterElecConsumption = VRF(VRFCond).CrankCaseHeaterPower * ReportingConstant;

        VRF(VRFCond).QCondEnergy = VRF(VRFCond).QCondenser * ReportingConstant;
        VRF(VRFCond).VRFHeatEnergyRec = VRF(VRFCond).VRFHeatRec * ReportingConstant;
    }

    void UpdateVRFCondenser(int const VRFCond) // index to VRF condensing unit
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Richard Raustad, FSEC
        //       DATE WRITTEN   May 2012
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine updates the node data for the VRF Condenser.

        int CondenserOutletNode; // - outlet node for VRF water-cooled condenser

        CondenserOutletNode = VRF(VRFCond).CondenserOutletNodeNum;

        DataLoopNode::Node(CondenserOutletNode).Temp = VRF(VRFCond).CondenserSideOutletTemp;

        DataLoopNode::Node(CondenserOutletNode).MassFlowRate = CondenserWaterMassFlowRate;
        DataLoopNode::Node(CondenserOutletNode).MassFlowRateMaxAvail = DataLoopNode::Node(CondenserOutletNode).MassFlowRateMaxAvail;
        DataLoopNode::Node(CondenserOutletNode).MassFlowRateMinAvail = DataLoopNode::Node(CondenserOutletNode).MassFlowRateMinAvail;
    }

    void isVRFCoilPresent(EnergyPlusData &state, std::string const VRFTUName, bool &CoolCoilPresent, bool &HeatCoilPresent)
    {

        if (GetVRFInputFlag) {
            GetVRFInput(state);
            GetVRFInputFlag = false;
        }

        int WhichVRFTU = UtilityRoutines::FindItemInList(VRFTUName, VRFTU, &VRFTerminalUnitEquipment::Name, NumVRFTU);
        if (WhichVRFTU != 0) {
            CoolCoilPresent = VRFTU(WhichVRFTU).CoolingCoilPresent;
            HeatCoilPresent = VRFTU(WhichVRFTU).HeatingCoilPresent;
        } else {
            ShowSevereError("isVRFCoilPresent: Could not find VRF TU = \"" + VRFTUName + "\"");
        }
    }

    //        End of Reporting subroutines for the Module
    // *****************************************************************************

    // Utility subroutines for the Module

    Real64 PLRResidual(EnergyPlusData &state,
                       Real64 const PartLoadRatio, // compressor cycling ratio (1.0 is continuous, 0.0 is off)
                       Array1D<Real64> const &Par  // par(1) = VRFTUNum
    )
    {
        // FUNCTION INFORMATION:
        //       AUTHOR         Richard Raustad, FSEC
        //       DATE WRITTEN   August 2010
        //       MODIFIED
        //       RE-ENGINEERED

        // PURPOSE OF THIS FUNCTION:
        //  Calculates residual function ((ActualOutput - QZnReq) /QZnReq)
        //  VRF TU output depends on the part load ratio which is being varied to zero the residual.

        // METHODOLOGY EMPLOYED:
        //  Calls CalcVRF to get ActualOutput at the given part load ratio
        //  and calculates the residual as defined above

        Real64 PLRResidual;

        // SUBROUTINE ARGUMENT DEFINITIONS:
        // par(2) = indicates load (0) or set point (1) control
        // par(3) = FirstHVACIteration
        // par(4) = OpMode
        // par(5) = QZnReq
        // par(6) = OnOffAirFlowRatio

        int VRFTUNum;             // TU index
        bool FirstHVACIteration;  // FirstHVACIteration flag
        int OpMode;               // Compressor operating mode
        Real64 QZnReq;            // zone load (W)
        Real64 QZnReqTemp;        // denominator representing zone load (W)
        Real64 OnOffAirFlowRatio; // ratio of compressor ON airflow to average airflow over timestep
        Real64 ActualOutput;      // delivered capacity of VRF terminal unit
        Real64 SuppHeatCoilLoad;  // supplemetal heating coil load (W)

        VRFTUNum = int(Par(1));
        bool setPointControlled = (Par(2) == 1.0);
        // FirstHVACIteration is a logical, Par is real, so make 1.0=TRUE and 0.0=FALSE
        FirstHVACIteration = (Par(3) == 1.0);
        OpMode = int(Par(4));
        QZnReq = Par(5);
        QZnReqTemp = QZnReq;
        OnOffAirFlowRatio = Par(6);
        SuppHeatCoilLoad = 0.0;

        if (VRF(VRFTU(VRFTUNum).VRFSysNum).VRFAlgorithmTypeNum == AlgorithmTypeFluidTCtrl) {
            // Algorithm Type: VRF model based on physics, applicable for Fluid Temperature Control
            VRFTU(VRFTUNum).CalcVRF_FluidTCtrl(state, VRFTUNum, FirstHVACIteration, PartLoadRatio, ActualOutput, OnOffAirFlowRatio, SuppHeatCoilLoad);
        } else {
            // Algorithm Type: VRF model based on system curve
            VRFTU(VRFTUNum).CalcVRF(state, VRFTUNum, FirstHVACIteration, PartLoadRatio, ActualOutput, OnOffAirFlowRatio, SuppHeatCoilLoad);
        }

        if (setPointControlled) {
            Real64 outletNodeT = DataLoopNode::Node(VRFTU(VRFTUNum).VRFTUOutletNodeNum).Temp;
            PLRResidual = (outletNodeT - VRFTU(VRFTUNum).coilTempSetPoint);
        } else {
            if (std::abs(QZnReq) < 100.0) QZnReqTemp = sign(100.0, QZnReq);
            PLRResidual = (ActualOutput - QZnReq) / QZnReqTemp;
        }

        return PLRResidual;
    }

    void SetAverageAirFlow(int const VRFTUNum,         // Unit index
                           Real64 const PartLoadRatio, // unit part load ratio
                           Real64 &OnOffAirFlowRatio   // ratio of compressor ON airflow to average airflow over timestep
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Richard Raustad
        //       DATE WRITTEN   August 2010
        //       MODIFIED       July 2012, Chandan Sharma - FSEC: Added zone sys avail managers
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Set the average air mass flow rates using the part load fraction of the heat pump for this time step
        // Set OnOffAirFlowRatio to be used by DX coils

        using DataZoneEquipment::VRFTerminalUnit_Num;
        using ScheduleManager::GetCurrentScheduleValue;

        int InletNode;                   // inlet node number
        int OutsideAirNode;              // outside air node number
        int AirRelNode;                  // relief air node number
        Real64 AverageUnitMassFlow(0.0); // average supply air mass flow rate over time step
        Real64 AverageOAMassFlow(0.0);   // average outdoor air mass flow rate over time step

        InletNode = VRFTU(VRFTUNum).VRFTUInletNodeNum;
        OutsideAirNode = VRFTU(VRFTUNum).VRFTUOAMixerOANodeNum;
        AirRelNode = VRFTU(VRFTUNum).VRFTUOAMixerRelNodeNum;

        if (VRFTU(VRFTUNum).OpMode == DataHVACGlobals::CycFanCycCoil) {
            AverageUnitMassFlow = (PartLoadRatio * CompOnMassFlow) + ((1 - PartLoadRatio) * CompOffMassFlow);
            AverageOAMassFlow = (PartLoadRatio * OACompOnMassFlow) + ((1 - PartLoadRatio) * OACompOffMassFlow);
        } else {
            if (PartLoadRatio == 0.0) {
                // set the average OA air flow to off compressor values if the compressor PartLoadRatio is zero
                AverageUnitMassFlow = CompOffMassFlow;
                AverageOAMassFlow = OACompOffMassFlow;
            } else {
                AverageUnitMassFlow = CompOnMassFlow;
                AverageOAMassFlow = OACompOnMassFlow;
            }
        }
        if (CompOffFlowRatio > 0.0) {
            FanSpeedRatio = (PartLoadRatio * CompOnFlowRatio) + ((1 - PartLoadRatio) * CompOffFlowRatio);
        } else {
            FanSpeedRatio = CompOnFlowRatio;
        }

        // if the terminal unit and fan are scheduled on then set flow rate
        if (GetCurrentScheduleValue(VRFTU(VRFTUNum).SchedPtr) > 0.0 &&
            (GetCurrentScheduleValue(VRFTU(VRFTUNum).FanAvailSchedPtr) > 0.0 || DataHVACGlobals::ZoneCompTurnFansOff) &&
            !DataHVACGlobals::ZoneCompTurnFansOff) {

            // so for sure OA system TUs should use inlet node flow rate, don't overwrite inlet node flow rate
            // could there be a reason for air loops to use inlet node flow? Possibly when VAV TUs used?
            if (!VRFTU(VRFTUNum).isInOASys) DataLoopNode::Node(InletNode).MassFlowRate = AverageUnitMassFlow;
            if (!VRFTU(VRFTUNum).isInOASys) DataLoopNode::Node(InletNode).MassFlowRateMaxAvail = AverageUnitMassFlow;
            if (OutsideAirNode > 0) {
                DataLoopNode::Node(OutsideAirNode).MassFlowRate = AverageOAMassFlow;
                DataLoopNode::Node(OutsideAirNode).MassFlowRateMaxAvail = AverageOAMassFlow;
                DataLoopNode::Node(AirRelNode).MassFlowRate = AverageOAMassFlow;
                DataLoopNode::Node(AirRelNode).MassFlowRateMaxAvail = AverageOAMassFlow;
            }
            if (AverageUnitMassFlow > 0.0) {
                OnOffAirFlowRatio = CompOnMassFlow / AverageUnitMassFlow;
            } else {
                OnOffAirFlowRatio = 0.0;
            }

        } else { // terminal unit and/or fan is off

            if (!VRFTU(VRFTUNum).isInOASys) DataLoopNode::Node(InletNode).MassFlowRate = 0.0;
            if (OutsideAirNode > 0) {
                DataLoopNode::Node(OutsideAirNode).MassFlowRate = 0.0;
                DataLoopNode::Node(AirRelNode).MassFlowRate = 0.0;
            }
            OnOffAirFlowRatio = 0.0;
        }
    }

    void InitializeOperatingMode(EnergyPlusData &state,
                                 bool const FirstHVACIteration, // flag for first time through HVAC systems
                                 int const VRFCond,             // Condenser Unit index
                                 int const TUListNum,           // Condenser Unit terminal unit list
                                 Real64 &OnOffAirFlowRatio      // ratio of on to off flow rate
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Richard Raustad
        //       DATE WRITTEN   July 2012 (Moved from InitVRF)
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Scans each zone coil and determines the load based on control
        // Moved from Init to clean up and localize code segments

        using DataHeatBalFanSys::TempControlType;
        using DataHeatBalFanSys::ZoneThermostatSetPointHi;
        using DataHeatBalFanSys::ZoneThermostatSetPointLo;
        using DataHeatBalFanSys::ZT;
        using MixedAir::SimOAMixer;
        using ScheduleManager::GetCurrentScheduleValue;

        Real64 ZoneDeltaT;       // zone temperature difference from setpoint
        Real64 SPTempHi;         // thermostat setpoint high
        Real64 SPTempLo;         // thermostat setpoint low
        int NumTU;               // loop counter, number of TU's in list
        int TUIndex;             // index to TU
        int ThisZoneNum;         // index to zone number where TU is located
        Real64 ZoneLoad;         // current zone load (W)
        Real64 LoadToCoolingSP;  // thermostat load to cooling setpoint (W)
        Real64 LoadToHeatingSP;  // thermostat load to heating setpoint (W)
        Real64 TempOutput;       // terminal unit output [W]
        Real64 SuppHeatCoilLoad; // supplemental heating coil load

        MaxDeltaT = 0.0;
        MinDeltaT = 0.0;
        NumCoolingLoads = 0;
        SumCoolingLoads = 0.0;
        NumHeatingLoads = 0;
        SumHeatingLoads = 0.0;
        SuppHeatCoilLoad = 0.0;

        NumCoolingLoads(VRFCond) = 0;
        NumHeatingLoads(VRFCond) = 0;
        SumCoolingLoads(VRFCond) = 0.0;
        SumHeatingLoads(VRFCond) = 0.0;
        MaxDeltaT(VRFCond) = 0.0;
        MinDeltaT(VRFCond) = 0.0;
        ZoneDeltaT = 0.0;
        HeatingLoad(VRFCond) = false;
        CoolingLoad(VRFCond) = false;
        TerminalUnitList(TUListNum).CoolingCoilAvailable = false;
        TerminalUnitList(TUListNum).HeatingCoilAvailable = false;
        // loop through all TU's to find operating mode. Be careful not to mix loop counters with current TU/Cond index
        for (NumTU = 1; NumTU <= TerminalUnitList(TUListNum).NumTUInList; ++NumTU) {
            // make sure TU's have been sized before looping through each one of them to determine operating mode
            if (any(TerminalUnitList(TUListNum).TerminalUnitNotSizedYet)) break;
            TUIndex = TerminalUnitList(TUListNum).ZoneTUPtr(NumTU);
            ThisZoneNum = VRFTU(TUIndex).ZoneNum;

            //       check to see if coil is present
            if (TerminalUnitList(TUListNum).CoolingCoilPresent(NumTU)) {
                //         now check to see if coil is scheduled off
                if (GetCurrentScheduleValue(TerminalUnitList(TUListNum).CoolingCoilAvailSchPtr(NumTU)) > 0.0) {
                    TerminalUnitList(TUListNum).CoolingCoilAvailable(NumTU) = true;
                }
            }

            //       check to see if coil is present
            if (TerminalUnitList(TUListNum).HeatingCoilPresent(NumTU)) {
                //         now check to see if coil is scheduled off
                if (GetCurrentScheduleValue(TerminalUnitList(TUListNum).HeatingCoilAvailSchPtr(NumTU)) > 0.0) {
                    TerminalUnitList(TUListNum).HeatingCoilAvailable(NumTU) = true;
                }
            }

            if (VRFTU(TUIndex).isSetPointControlled) {
                // set point temperature may only reside at the TU outlet node
                Real64 coolCoilTempSetPoint = DataLoopNode::Node(VRFTU(TUIndex).VRFTUOutletNodeNum).TempSetPoint;
                VRFTU(TUIndex).suppTempSetPoint = coolCoilTempSetPoint;
                Real64 heatCoilTempSetPoint = coolCoilTempSetPoint;
                // adjust coil control for fan heat when set point is at outlet node
                Real64 coolfanDeltaT = 0.0;
                Real64 heatfanDeltaT = 0.0;
                if (VRFTU(TUIndex).FanPlace == DataHVACGlobals::DrawThru) {
                    if (VRFTU(TUIndex).fanOutletNode > 0)
                        coolfanDeltaT = DataLoopNode::Node(VRFTU(TUIndex).fanOutletNode).Temp - DataLoopNode::Node(VRFTU(TUIndex).fanInletNode).Temp;
                }
                heatfanDeltaT = coolfanDeltaT;
                // or the set point could be placed at either or both coils, update both if necessary
                if (VRFTU(TUIndex).CoolingCoilPresent) {
                    if (DataLoopNode::Node(VRFTU(TUIndex).coolCoilAirOutNode).TempSetPoint != DataLoopNode::SensedNodeFlagValue) {
                        coolCoilTempSetPoint = DataLoopNode::Node(VRFTU(TUIndex).coolCoilAirOutNode).TempSetPoint;
                        //// should we adjust for fan heat or not? What if it's a mixed air SP that already adjusts for fan heat?
                        // coolfanDeltaT = 0.0;
                    }
                }
                if (VRFTU(TUIndex).HeatingCoilPresent) {
                    if (DataLoopNode::Node(VRFTU(TUIndex).heatCoilAirOutNode).TempSetPoint != DataLoopNode::SensedNodeFlagValue) {
                        heatCoilTempSetPoint = DataLoopNode::Node(VRFTU(TUIndex).heatCoilAirOutNode).TempSetPoint;
                        //// should we adjust for fan heat or not? What if it's a mixed air SP that already adjusts for fan heat?
                        // heatfanDeltaT = 0.0;
                    }
                }
                // set a flow rate and simulate ATMixer/OASystem if needed
                if (FirstHVACIteration) {
                    SetAverageAirFlow(TUIndex, 1.0, OnOffAirFlowRatio);
                    if (VRFTU(TUIndex).ATMixerExists) {
                        // There is an air terminal mixer
                        if (VRFTU(TUIndex).ATMixerType == DataHVACGlobals::ATMixer_InletSide) { // if there is an inlet side air terminal mixer
                                                                                                // set the primary air inlet mass flow rate
                            DataLoopNode::Node(VRFTU(TUIndex).ATMixerPriNode).MassFlowRate =
                                min(DataLoopNode::Node(VRFTU(TUIndex).ATMixerPriNode).MassFlowRateMaxAvail,
                                    DataLoopNode::Node(VRFTU(TUIndex).VRFTUInletNodeNum).MassFlowRate);
                            // now calculate the the mixer outlet air conditions (and the secondary air inlet flow rate). The mixer outlet flow rate
                            // has already been set above (it is the "inlet" node flow rate)
                            SingleDuct::SimATMixer(state, VRFTU(TUIndex).ATMixerName, FirstHVACIteration, VRFTU(TUIndex).ATMixerIndex);
                        }
                    } else {
                        // simulate OA Mixer
                        if (VRFTU(TUIndex).OAMixerUsed) SimOAMixer(state, VRFTU(TUIndex).OAMixerName, FirstHVACIteration, VRFTU(TUIndex).OAMixerIndex);
                    }
                }
                // identify a coil inlet temperature
                if (VRFTU(TUIndex).CoolingCoilPresent) {
                    VRFTU(TUIndex).coilInNodeT = DataLoopNode::Node(VRFTU(TUIndex).coolCoilAirInNode).Temp;
                    VRFTU(TUIndex).coilInNodeW = DataLoopNode::Node(VRFTU(TUIndex).coolCoilAirInNode).HumRat;
                } else {
                    VRFTU(TUIndex).coilInNodeT = DataLoopNode::Node(VRFTU(TUIndex).heatCoilAirInNode).Temp;
                    VRFTU(TUIndex).coilInNodeW = DataLoopNode::Node(VRFTU(TUIndex).heatCoilAirInNode).HumRat;
                }
                Real64 coilInletTemp = VRFTU(TUIndex).coilInNodeT;
                Real64 coilInletHumRat = VRFTU(TUIndex).coilInNodeW;
                Real64 coilInletMassFlow = DataLoopNode::Node(VRFTU(TUIndex).VRFTUInletNodeNum).MassFlowRate;
                VRFTU(TUIndex).coolSPActive = false;
                VRFTU(TUIndex).heatSPActive = false;

                if ((heatCoilTempSetPoint - coilInletTemp - heatfanDeltaT) > DataHVACGlobals::SmallTempDiff) { // heating
                    Real64 CpAirIn = Psychrometrics::PsyCpAirFnW(coilInletHumRat);
                    ZoneLoad = coilInletMassFlow * CpAirIn * (heatCoilTempSetPoint - coilInletTemp - heatfanDeltaT);
                    VRFTU(TUIndex).heatSPActive = true;
                    VRFTU(TUIndex).heatLoadToSP = ZoneLoad;
                    ++NumHeatingLoads(VRFCond);
                    SumHeatingLoads(VRFCond) += ZoneLoad;
                    MinDeltaT(VRFCond) = min(MinDeltaT(VRFCond), -1.0);
                    VRFTU(TUIndex).coilTempSetPoint = heatCoilTempSetPoint - heatfanDeltaT;
                } else if ((coilInletTemp - coolCoilTempSetPoint - coolfanDeltaT) > DataHVACGlobals::SmallTempDiff) { // cooling
                    Real64 CpAirIn = Psychrometrics::PsyCpAirFnW(coilInletHumRat);
                    ZoneLoad = coilInletMassFlow * CpAirIn * (coolCoilTempSetPoint - coilInletTemp - coolfanDeltaT);
                    VRFTU(TUIndex).coolSPActive = true;
                    VRFTU(TUIndex).coolLoadToSP = ZoneLoad;
                    ++NumCoolingLoads(VRFCond);
                    SumCoolingLoads(VRFCond) += ZoneLoad;
                    MaxDeltaT(VRFCond) = max(MaxDeltaT(VRFCond), 1.0);
                    VRFTU(TUIndex).coilTempSetPoint = coolCoilTempSetPoint - coolfanDeltaT;
                }
            } else { // else is not set point controlled
                //     Constant fan systems are tested for ventilation load to determine if load to be met changes.
                //     more logic may be needed here, what is the OA flow rate, was last mode heating or cooling, what control is used, etc...

                getVRFTUZoneLoad(TUIndex, ZoneLoad, LoadToHeatingSP, LoadToCoolingSP, true);

                if (VRF(VRFCond).ThermostatPriority == ThermostatOffsetPriority) {
                    //         for TSTATPriority, just check difference between zone temp and thermostat setpoint
                    if (ThisZoneNum > 0) {
                        SPTempHi = ZoneThermostatSetPointHi(ThisZoneNum);
                        SPTempLo = ZoneThermostatSetPointLo(ThisZoneNum);
                        {
                            auto const SELECT_CASE_var(TempControlType(ThisZoneNum));
                            if (SELECT_CASE_var == 0) { // Uncontrolled
                                // MaxDeltaT denotes cooling, MinDeltaT denotes heating
                            } else if (SELECT_CASE_var == DataHVACGlobals::SingleHeatingSetPoint) {
                                // if heating load, ZoneDeltaT will be negative
                                ZoneDeltaT = min(0.0, ZT(ThisZoneNum) - SPTempLo);
                                MinDeltaT(VRFCond) = min(MinDeltaT(VRFCond), ZoneDeltaT);
                            } else if (SELECT_CASE_var == DataHVACGlobals::SingleCoolingSetPoint) {
                                // if cooling load, ZoneDeltaT will be positive
                                ZoneDeltaT = max(0.0, ZT(ThisZoneNum) - SPTempHi);
                                MaxDeltaT(VRFCond) = max(MaxDeltaT(VRFCond), ZoneDeltaT);
                            } else if (SELECT_CASE_var == DataHVACGlobals::SingleHeatCoolSetPoint) {
                                ZoneDeltaT = ZT(ThisZoneNum) - SPTempHi; //- SPTempHi and SPTempLo are same value
                                if (ZoneDeltaT > 0.0) {
                                    MaxDeltaT(VRFCond) = max(MaxDeltaT(VRFCond), ZoneDeltaT);
                                } else {
                                    MinDeltaT(VRFCond) = min(MinDeltaT(VRFCond), ZoneDeltaT);
                                }
                            } else if (SELECT_CASE_var == DataHVACGlobals::DualSetPointWithDeadBand) {
                                if (ZT(ThisZoneNum) - SPTempHi > 0.0) {
                                    ZoneDeltaT = max(0.0, ZT(ThisZoneNum) - SPTempHi);
                                    MaxDeltaT(VRFCond) = max(MaxDeltaT(VRFCond), ZoneDeltaT);
                                } else if (SPTempLo - ZT(ThisZoneNum) > 0.0) {
                                    ZoneDeltaT = min(0.0, ZT(ThisZoneNum) - SPTempLo);
                                    MinDeltaT(VRFCond) = min(MinDeltaT(VRFCond), ZoneDeltaT);
                                }
                            } else {
                            }
                        }
                    }
                } else if (VRF(VRFCond).ThermostatPriority == LoadPriority || VRF(VRFCond).ThermostatPriority == ZonePriority) {
                    if (VRFTU(TUIndex).OpMode == DataHVACGlobals::ContFanCycCoil) {
                        SetCompFlowRate(TUIndex, VRFCond);

                        if (VRF(VRFCond).VRFAlgorithmTypeNum == AlgorithmTypeFluidTCtrl) {
                            // Algorithm Type: VRF model based on physics, applicable for Fluid Temperature Control
                            VRFTU(TUIndex).CalcVRF_FluidTCtrl(
                                state, TUIndex, FirstHVACIteration, 0.0, TempOutput, OnOffAirFlowRatio, SuppHeatCoilLoad);
                        } else {
                            // Algorithm Type: VRF model based on system curve
                            VRFTU(TUIndex).CalcVRF(state, TUIndex, FirstHVACIteration, 0.0, TempOutput, OnOffAirFlowRatio, SuppHeatCoilLoad);
                        }

                        //           If the Terminal Unit has a net cooling capacity (NoCompOutput < 0) and
                        //           the zone temp is above the Tstat heating setpoint (QToHeatSetPt < 0)
                        if (TempOutput < 0.0 && LoadToHeatingSP < 0.0) {
                            //             If the net cooling capacity overshoots the heating setpoint count as heating load
                            if (TempOutput < LoadToHeatingSP) {
                                //               Don't count as heating load unless mode is allowed. Also check for floating zone.
                                if (TempControlType(ThisZoneNum) != DataHVACGlobals::SingleCoolingSetPoint && TempControlType(ThisZoneNum) != 0) {
                                    if (!LastModeHeating(VRFCond)) {
                                        // if last mode was cooling, make sure heating flow rate is used
                                        if (VRFTU(TUIndex).OAMixerUsed) {
                                            DataLoopNode::Node(VRFTU(TUIndex).VRFTUOAMixerRetNodeNum).MassFlowRate =
                                                VRFTU(TUIndex).MaxHeatAirMassFlow;
                                            DataLoopNode::Node(VRFTU(TUIndex).VRFTUOAMixerOANodeNum).MassFlowRate = VRFTU(TUIndex).HeatOutAirMassFlow;
                                            SimOAMixer(state, VRFTU(TUIndex).OAMixerName, FirstHVACIteration, VRFTU(TUIndex).OAMixerIndex);
                                        } else {
                                            DataLoopNode::Node(VRFTU(TUIndex).VRFTUInletNodeNum).MassFlowRate = VRFTU(TUIndex).MaxHeatAirMassFlow;
                                        }

                                        // recalculate using correct flow rate
                                        if (VRF(VRFCond).VRFAlgorithmTypeNum == AlgorithmTypeFluidTCtrl) {
                                            // Algorithm Type: VRF model based on physics, applicable for Fluid Temperature Control
                                            VRFTU(TUIndex).CalcVRF_FluidTCtrl(
                                                state, TUIndex, FirstHVACIteration, 0.0, TempOutput, OnOffAirFlowRatio, SuppHeatCoilLoad);
                                        } else {
                                            // Algorithm Type: VRF model based on system curve
                                            VRFTU(TUIndex).CalcVRF(
                                                state, TUIndex, FirstHVACIteration, 0.0, TempOutput, OnOffAirFlowRatio, SuppHeatCoilLoad);
                                        }

                                        if (TempOutput < LoadToHeatingSP) {
                                            ++NumHeatingLoads(VRFCond);
                                            // sum heating load on condenser, not total zone heating load
                                            SumHeatingLoads(VRFCond) += (LoadToHeatingSP - TempOutput);
                                        }
                                    } else {
                                        ++NumHeatingLoads(VRFCond);
                                        // sum heating load on condenser, not total zone heating load
                                        SumHeatingLoads(VRFCond) += (LoadToHeatingSP - TempOutput);
                                    }
                                }
                            } else if (TempOutput < ZoneLoad) {
                                //             If the net cooling capacity meets the zone cooling load but does not overshoot heating setpoint, turn
                                //             off coil do nothing, the zone will float
                            } else if (ZoneLoad < 0.0) {
                                //               still a cooling load
                                ++NumCoolingLoads(VRFCond);
                                // sum cooling load on condenser, not total zone cooling load
                                SumCoolingLoads(VRFCond) += (LoadToCoolingSP - TempOutput);
                            }

                            //           If the terminal unit has a net heating capacity and the zone temp is below the Tstat cooling setpoint
                        } else if (TempOutput > 0.0 && LoadToCoolingSP > 0.0) {
                            //             If the net heating capacity overshoots the cooling setpoint count as cooling load
                            if (TempOutput > LoadToCoolingSP) {
                                //               Don't count as cooling load unless mode is allowed. Also check for floating zone.
                                if (TempControlType(ThisZoneNum) != DataHVACGlobals::SingleHeatingSetPoint && TempControlType(ThisZoneNum) != 0) {
                                    if (!LastModeCooling(VRFCond)) {
                                        if (VRFTU(TUIndex).OAMixerUsed) {
                                            DataLoopNode::Node(VRFTU(TUIndex).VRFTUOAMixerRetNodeNum).MassFlowRate =
                                                VRFTU(TUIndex).MaxCoolAirMassFlow;
                                            DataLoopNode::Node(VRFTU(TUIndex).VRFTUOAMixerOANodeNum).MassFlowRate = VRFTU(TUIndex).CoolOutAirMassFlow;
                                            SimOAMixer(state, VRFTU(TUIndex).OAMixerName, FirstHVACIteration, VRFTU(TUIndex).OAMixerIndex);
                                        } else {
                                            DataLoopNode::Node(VRFTU(TUIndex).VRFTUInletNodeNum).MassFlowRate = VRFTU(TUIndex).MaxCoolAirMassFlow;
                                        }

                                        if (VRF(VRFCond).VRFAlgorithmTypeNum == AlgorithmTypeFluidTCtrl) {
                                            // Algorithm Type: VRF model based on physics, applicable for Fluid Temperature Control
                                            VRFTU(TUIndex).CalcVRF_FluidTCtrl(
                                                state, TUIndex, FirstHVACIteration, 0.0, TempOutput, OnOffAirFlowRatio, SuppHeatCoilLoad);
                                        } else {
                                            // Algorithm Type: VRF model based on system curve
                                            VRFTU(TUIndex).CalcVRF(
                                                state, TUIndex, FirstHVACIteration, 0.0, TempOutput, OnOffAirFlowRatio, SuppHeatCoilLoad);
                                        }

                                        if (TempOutput > LoadToCoolingSP) {
                                            ++NumCoolingLoads(VRFCond);
                                            SumCoolingLoads(VRFCond) += (LoadToCoolingSP - TempOutput);
                                        }
                                    } else {
                                        ++NumCoolingLoads(VRFCond);
                                        SumCoolingLoads(VRFCond) += (LoadToCoolingSP - TempOutput);
                                    }
                                }
                            } else if (TempOutput > ZoneLoad) {
                                // do nothing, zone will float
                            } else if (ZoneLoad > 0.0) {
                                ++NumHeatingLoads(VRFCond);
                                SumHeatingLoads(VRFCond) += ZoneLoad;
                            }
                            //           ELSE there is no overshoot and the zone has a valid cooling load
                        } else if (ZoneLoad < 0.0) {
                            ++NumCoolingLoads(VRFCond);
                            SumCoolingLoads(VRFCond) += ZoneLoad;
                            // ELSE there is no overshoot and the zone has a valid heating load
                        } else if (ZoneLoad > 0.0) {
                            ++NumHeatingLoads(VRFCond);
                            SumHeatingLoads(VRFCond) += ZoneLoad;
                        }
                    } else { // is cycling fan
                        if (ZoneLoad > 0.0) {
                            ++NumHeatingLoads(VRFCond);
                            SumHeatingLoads(VRFCond) += ZoneLoad;
                        } else if (ZoneLoad < 0.0) {
                            ++NumCoolingLoads(VRFCond);
                            SumCoolingLoads(VRFCond) += ZoneLoad;
                        }
                    } // IF(VRFTU(TUIndex)%OpMode == DataHVACGlobals::ContFanCycCoil)THEN
                }
            }
        }

        // Determine operating mode based on VRF type and thermostat control selection
        {
            auto const SELECT_CASE_var(VRF(VRFCond).ThermostatPriority);
            if (SELECT_CASE_var == ThermostatOffsetPriority) {
                if (MaxDeltaT(VRFCond) > std::abs(MinDeltaT(VRFCond)) && MaxDeltaT(VRFCond) > 0.0) {
                    HeatingLoad(VRFCond) = false;
                    CoolingLoad(VRFCond) = true;
                } else if (MaxDeltaT(VRFCond) < std::abs(MinDeltaT(VRFCond)) && MinDeltaT(VRFCond) < 0.0) {
                    HeatingLoad(VRFCond) = true;
                    CoolingLoad(VRFCond) = false;
                } else {
                    HeatingLoad(VRFCond) = false;
                    CoolingLoad(VRFCond) = false;
                }
            } else if (SELECT_CASE_var == LoadPriority) {
                if (SumHeatingLoads(VRFCond) > std::abs(SumCoolingLoads(VRFCond)) && SumHeatingLoads(VRFCond) > 0.0) {
                    HeatingLoad(VRFCond) = true;
                    CoolingLoad(VRFCond) = false;
                } else if (SumHeatingLoads(VRFCond) <= std::abs(SumCoolingLoads(VRFCond)) && SumCoolingLoads(VRFCond) < 0.0) {
                    HeatingLoad(VRFCond) = false;
                    CoolingLoad(VRFCond) = true;
                } else {
                    HeatingLoad(VRFCond) = false;
                    CoolingLoad(VRFCond) = false;
                }
            } else if (SELECT_CASE_var == ZonePriority) {
                if (NumHeatingLoads(VRFCond) > NumCoolingLoads(VRFCond) && NumHeatingLoads(VRFCond) > 0) {
                    HeatingLoad(VRFCond) = true;
                    CoolingLoad(VRFCond) = false;
                } else if (NumHeatingLoads(VRFCond) <= NumCoolingLoads(VRFCond) && NumCoolingLoads(VRFCond) > 0) {
                    HeatingLoad(VRFCond) = false;
                    CoolingLoad(VRFCond) = true;
                } else {
                    HeatingLoad(VRFCond) = false;
                    CoolingLoad(VRFCond) = false;
                }
            } else if (SELECT_CASE_var == ScheduledPriority) {
                if (GetCurrentScheduleValue(VRF(VRFCond).SchedPriorityPtr) == 0) {
                    HeatingLoad(VRFCond) = true;
                    CoolingLoad(VRFCond) = false;
                } else if (GetCurrentScheduleValue(VRF(VRFCond).SchedPriorityPtr) == 1) {
                    HeatingLoad(VRFCond) = false;
                    CoolingLoad(VRFCond) = true;
                } else {
                    HeatingLoad(VRFCond) = false;
                    CoolingLoad(VRFCond) = false;
                }
            } else if (SELECT_CASE_var == MasterThermostatPriority) {
                ZoneLoad = ZoneSysEnergyDemand(VRF(VRFCond).MasterZonePtr).RemainingOutputRequired /
                           VRFTU(VRF(VRFCond).MasterZoneTUIndex).controlZoneMassFlowFrac;
                if (VRFTU(VRF(VRFCond).MasterZoneTUIndex).OpMode == DataHVACGlobals::ContFanCycCoil) {
                    SetCompFlowRate(VRF(VRFCond).MasterZoneTUIndex, VRFCond);

                    if (VRF(VRFCond).VRFAlgorithmTypeNum == AlgorithmTypeFluidTCtrl) {
                        // Algorithm Type: VRF model based on physics, applicable for Fluid Temperature Control
                        VRFTU(VRF(VRFCond).MasterZoneTUIndex)
                            .CalcVRF_FluidTCtrl(
                                state, VRF(VRFCond).MasterZoneTUIndex, FirstHVACIteration, 0.0, TempOutput, OnOffAirFlowRatio, SuppHeatCoilLoad);
                    } else {
                        // Algorithm Type: VRF model based on system curve
                        VRFTU(VRF(VRFCond).MasterZoneTUIndex)
                            .CalcVRF(state, VRF(VRFCond).MasterZoneTUIndex, FirstHVACIteration, 0.0, TempOutput, OnOffAirFlowRatio, SuppHeatCoilLoad);
                    }

                    LoadToCoolingSP = ZoneSysEnergyDemand(VRF(VRFCond).MasterZonePtr).OutputRequiredToCoolingSP /
                                      VRFTU(VRF(VRFCond).MasterZoneTUIndex).controlZoneMassFlowFrac;
                    LoadToHeatingSP = ZoneSysEnergyDemand(VRF(VRFCond).MasterZonePtr).OutputRequiredToHeatingSP /
                                      VRFTU(VRF(VRFCond).MasterZoneTUIndex).controlZoneMassFlowFrac;
                    if (TempOutput < LoadToHeatingSP) {
                        CoolingLoad(VRFCond) = false;
                        HeatingLoad(VRFCond) = true;
                    } else if (TempOutput > LoadToCoolingSP) {
                        CoolingLoad(VRFCond) = true;
                        HeatingLoad(VRFCond) = false;
                    } else {
                        CoolingLoad(VRFCond) = false;
                        HeatingLoad(VRFCond) = false;
                    }
                } else if (ZoneLoad > 0.0) {
                    HeatingLoad(VRFCond) = true;
                    CoolingLoad(VRFCond) = false;
                } else if (ZoneLoad < 0.0) {
                    HeatingLoad(VRFCond) = false;
                    CoolingLoad(VRFCond) = true;
                } else {
                    HeatingLoad(VRFCond) = false;
                    CoolingLoad(VRFCond) = false;
                }
            } else if (SELECT_CASE_var == FirstOnPriority) {
                // na
            } else {
            }
        }

        // limit to one possible mode
        if (CoolingLoad(VRFCond) && HeatingLoad(VRFCond)) HeatingLoad(VRFCond) = false;
    }

    void LimitTUCapacity(int const VRFCond,               // Condenser Unit index
                         int const NumTUInList,           // Number of terminal units in list
                         Real64 const StartingCapacity,   // temporary variable holding condenser capacity [W]
                         const Array1D<Real64> &CapArray, // Array of coil capacities in either cooling or heating mode [W]
                         Real64 &MaxLimit,                // Maximum terminal unit capacity for coils in same operating mode [W]
                         Real64 const AltCapacity,        // temporary variable holding heat recovery capacity [W]
                         const Array1D<Real64> &AltArray, // Array of coil capacities of heat recovery [W]
                         Real64 &AltLimit                 // Maximum terminal unit capacity of heat recovery coils [W]
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Richard Raustad
        //       DATE WRITTEN   July 2012 (Moved from InitVRF)
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Calculate the maximum allowed terminal unit capacity. Total terminal unit capacity must not
        // exceed the available condenser capacity. This variable, MaxCapacity (passed out to MaxCoolingCapacity
        // or MaxHeatingCapacity), is used to limit the terminal units providing more capacity than allowed.
        // Example: TU loads are 1-ton, 2-ton, 3-ton, and 4-ton connected to a condenser having only 9-tons available.
        // This variable is will be set to 3-tons and the 4-ton terminal unit will be limited to 3-tons
        // (see InitVRF where this variable is reset and CalcVRF where the call to the DX coils passes this argument).

        // METHODOLOGY EMPLOYED:
        // The coils are simulated and summed. This value is compared to the available capacity. If the summed
        // TU capacity is greater than the available capacity, limit the TU's with the highest capacity so that
        // the TU capacity equals the available capacity. The report variable Variable Refrigerant Flow Heat Pump
        // Maximum Terminal Unit Cool/Heating Capacity holds the value for maximum TU capacity. This value may not
        // match the maximum individual coil capacity exactly since the available capacity uses a load weighted
        // average WB temperature to calculate available capacity. When the TU's are limited, this weighting changes.
        // The extra iterations required for these values to converge is considered excessive.
        // If the global flag SimZoneEquipment could be set for 1 additional iteration, these variables would
        // converge more closely (setting this global flag is not yet implemented).

        Real64 RemainingCapacity; // decrement capacity counter to find limiting TU capacity [W]

        // limit TU coil capacity to be equal to the condenser capacity (piping losses already accounted for)
        LimitCoilCapacity(NumTUInList, StartingCapacity, CapArray, MaxLimit);

        // ** add in logic to limit coils operating opposite to mode when heat recovery is used
        // ** this is a hard one since we are here because the system is overloaded. That means
        // ** that we do not know at this point the actual operating capacity or compressor power.
        if (VRF(VRFCond).HeatRecoveryUsed) {
            if (CoolingLoad(VRFCond)) {
                RemainingCapacity = StartingCapacity * (1 + 1 / VRF(VRFCond).CoolingCOP);
                if (AltCapacity > RemainingCapacity) {
                    LimitCoilCapacity(NumTUInList, RemainingCapacity, AltArray, AltLimit);
                }
            }
            if (HeatingLoad(VRFCond)) {
                RemainingCapacity = StartingCapacity / (1 + 1 / VRF(VRFCond).HeatingCOP);
                if (AltCapacity > RemainingCapacity) {
                    LimitCoilCapacity(NumTUInList, RemainingCapacity, AltArray, AltLimit);
                }
            }
        }
    }

    void LimitCoilCapacity(int const NumTUInList,           // Number of terminal units in list
                           Real64 const TotalCapacity,      // temporary variable holding condenser capacity [W]
                           const Array1D<Real64> &CapArray, // Array of coil capacities in either cooling or heating mode [W]
                           Real64 &MaxLimit                 // Maximum terminal unit capacity for coils in same operating mode [W]
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Richard Raustad
        //       DATE WRITTEN   July 2012 (Moved from InitVRF)
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Calculate the maximum allowed terminal unit capacity. Total terminal unit capacity must not
        // exceed the available condenser capacity. This variable, MaxCapacity (passed out to MaxCoolingCapacity
        // or MaxHeatingCapacity), is used to limit the terminal units providing more capacity than allowed.
        // Example: TU loads are 1-ton, 2-ton, 3-ton, and 4-ton connected to a condenser having only 9-tons available.
        // This variable is will be set to 3-tons and the 4-ton terminal unit will be limited to 3-tons
        // (see InitVRF where this variable is reset and CalcVRF where the call to the DX coils passes this argument).

        // METHODOLOGY EMPLOYED:
        // The coils are simulated and summed. This value is compared to the available capacity. If the summed
        // TU capacity is greater than the available capacity, limit the TU's with the highest capacity so that
        // the TU capacity equals the available capacity. The report variable Variable Refrigerant Flow Heat Pump
        // Maximum Terminal Unit Cool/Heating Capacity holds the value for maximum TU capacity. This value may not
        // match the maximum individual coil capacity exactly since the available capacity uses a load weighted
        // average WB temperature to calculate available capacity. When the TU's are limited, this weighting changes.
        // The extra iterations required for these values to converge is considered excessive.
        // If the global flag SimZoneEquipment could be set for 1 additional iteration, these variables would
        // converge more closely (setting this global flag is not yet implemented).

        int NumTU;                                   // loop counter
        int TempTUIndex;                             // temp variable used to find max terminal unit limit
        int MinOutputIndex;                          // index to TU with lowest load
        Real64 MinOutput;                            // used when finding TU "max" capacity limit
        Real64 RemainingCapacity;                    // decrement capacity counter to find limiting TU capacity [W]
        Array1D<Real64> Temp(NumTUInList, CapArray); // temporary array for processing terminal units
        Array1D<Real64> Temp2(NumTUInList, Temp);    // temporary array for processing terminal units

        RemainingCapacity = TotalCapacity;

        // sort TU capacity from lowest to highest
        for (TempTUIndex = 1; TempTUIndex <= NumTUInList; ++TempTUIndex) {
            MinOutput = MaxCap;
            for (NumTU = 1; NumTU <= NumTUInList; ++NumTU) {
                if (Temp2(NumTU) < MinOutput) {
                    MinOutput = Temp2(NumTU);
                    Temp(TempTUIndex) = MinOutput;
                    MinOutputIndex = NumTU;
                }
            }
            Temp2(MinOutputIndex) = MaxCap;
        }

        // find limit of "terminal unit" capacity so that sum of all TU's does not exceed condenser capacity
        // if the terminal unit capacity multiplied by number of remaining TU's does not exceed remaining available, subtract and cycle
        for (TempTUIndex = 1; TempTUIndex <= NumTUInList; ++TempTUIndex) {
            if ((Temp(TempTUIndex) * (NumTUInList - TempTUIndex + 1)) < RemainingCapacity) {
                RemainingCapacity -= Temp(TempTUIndex);
                continue;
            } else {
                // if it does exceed, limit is found
                MaxLimit = RemainingCapacity / (NumTUInList - TempTUIndex + 1);
                break;
            }
        }
    }

    int GetVRFTUOutAirNode(EnergyPlusData &state, int const VRFTUNum)
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         R. Raustad (copy of B Griffith routine)
        //       DATE WRITTEN   Jan  2015
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS FUNCTION:
        // lookup function for VRF terminal unit OA inlet node

        if (GetVRFInputFlag) {
            GetVRFInput(state);
            GetVRFInputFlag = false;
        }

        if (VRFTUNum > 0 && VRFTUNum <= NumVRFTU) {
            return VRFTU(VRFTUNum).VRFTUOAMixerOANodeNum;
        } else {
            return 0;
        }
    }

    int GetVRFTUZoneInletAirNode(EnergyPlusData &state, int const VRFTUNum)
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         R. Raustad (copy of B Griffith routine)
        //       DATE WRITTEN   Jan  2015
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS FUNCTION:
        // lookup function for VRF terminal unit zone inlet node

        if (GetVRFInputFlag) {
            GetVRFInput(state);
            GetVRFInputFlag = false;
        }

        if (VRFTUNum > 0 && VRFTUNum <= NumVRFTU) {
            return VRFTU(VRFTUNum).VRFTUOutletNodeNum;
        } else {
            return 0;
        }
    }

    int GetVRFTUOutAirNodeFromName(EnergyPlusData &state, std::string const VRFTUName, bool &errorsFound)
    {
        int NodeNum; // return value of node number

        if (GetVRFInputFlag) {
            GetVRFInput(state);
            GetVRFInputFlag = false;
        }

        int WhichVRFTU = UtilityRoutines::FindItemInList(VRFTUName, VRFTU, &VRFTerminalUnitEquipment::Name, NumVRFTU);
        if (WhichVRFTU != 0) {
            NodeNum = VRFTU(WhichVRFTU).VRFTUOutletNodeNum;
        } else {
            ShowSevereError("GetVRFTUOutAirNodeFromName: Could not find VRF TU = \"" + VRFTUName + "\"");
            errorsFound = true;
            NodeNum = 0;
        }

        return NodeNum;
    }

    int GetVRFTUInAirNodeFromName(EnergyPlusData &state, std::string const VRFTUName, bool &errorsFound)
    {
        int NodeNum; // return value of node number

        if (GetVRFInputFlag) {
            GetVRFInput(state);
            GetVRFInputFlag = false;
        }

        int WhichVRFTU = UtilityRoutines::FindItemInList(VRFTUName, VRFTU, &VRFTerminalUnitEquipment::Name, NumVRFTU);
        if (WhichVRFTU != 0) {
            NodeNum = VRFTU(WhichVRFTU).VRFTUInletNodeNum;
        } else {
            ShowSevereError("GetVRFTUInAirNodeFromName: Could not find VRF TU = \"" + VRFTUName + "\"");
            errorsFound = true;
            NodeNum = 0;
        }

        return NodeNum;
    }

    int GetVRFTUMixedAirNode(EnergyPlusData &state, int const VRFTUNum)
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         R. Raustad (copy of B Griffith routine)
        //       DATE WRITTEN   Jan  2015
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS FUNCTION:
        // lookup function for VRF terminal unit mixed air node

        if (GetVRFInputFlag) {
            GetVRFInput(state);
            GetVRFInputFlag = false;
        }

        if (VRFTUNum > 0 && VRFTUNum <= NumVRFTU) {
            return VRFTU(VRFTUNum).VRFTUOAMixerOANodeNum;
        } else {
            return 0;
        }
    }

    int GetVRFTUReturnAirNode(EnergyPlusData &state, int const VRFTUNum)
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         R. Raustad (copy of B Griffith routine)
        //       DATE WRITTEN   Jan  2015
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS FUNCTION:
        // lookup function for VRF terminal unit return air node

        if (GetVRFInputFlag) {
            GetVRFInput(state);
            GetVRFInputFlag = false;
        }

        if (VRFTUNum > 0 && VRFTUNum <= NumVRFTU) {
            return VRFTU(VRFTUNum).VRFTUOAMixerRetNodeNum;
        } else {
            return 0;
        }
    }

    void getVRFTUZoneLoad(int const VRFTUNum, Real64 &zoneLoad, Real64 &LoadToHeatingSP, Real64 &LoadToCoolingSP, bool const InitFlag)
    {

        if (VRFTU(VRFTUNum).zoneSequenceCoolingNum > 0 && VRFTU(VRFTUNum).zoneSequenceHeatingNum > 0 && VRFTU(VRFTUNum).isInAirLoop) {
            // air loop equipment uses sequenced variables
            LoadToCoolingSP = DataZoneEnergyDemands::ZoneSysEnergyDemand(VRFTU(VRFTUNum).ZoneNum)
                                  .SequencedOutputRequiredToCoolingSP(VRFTU(VRFTUNum).zoneSequenceCoolingNum) /
                              VRFTU(VRFTUNum).controlZoneMassFlowFrac;
            LoadToHeatingSP = DataZoneEnergyDemands::ZoneSysEnergyDemand(VRFTU(VRFTUNum).ZoneNum)
                                  .SequencedOutputRequiredToHeatingSP(VRFTU(VRFTUNum).zoneSequenceHeatingNum) /
                              VRFTU(VRFTUNum).controlZoneMassFlowFrac;
            if (LoadToHeatingSP > 0.0 && LoadToCoolingSP > 0.0 &&
                DataHeatBalFanSys::TempControlType(VRFTU(VRFTUNum).ZoneNum) != DataHVACGlobals::SingleCoolingSetPoint) {
                zoneLoad = LoadToHeatingSP;
            } else if (LoadToHeatingSP > 0.0 && LoadToCoolingSP > 0.0 &&
                       DataHeatBalFanSys::TempControlType(VRFTU(VRFTUNum).ZoneNum) == DataHVACGlobals::SingleCoolingSetPoint) {
                zoneLoad = 0.0;
            } else if (LoadToHeatingSP < 0.0 && LoadToCoolingSP < 0.0 &&
                       DataHeatBalFanSys::TempControlType(VRFTU(VRFTUNum).ZoneNum) != DataHVACGlobals::SingleHeatingSetPoint) {
                zoneLoad = LoadToCoolingSP;
            } else if (LoadToHeatingSP < 0.0 && LoadToCoolingSP < 0.0 &&
                       DataHeatBalFanSys::TempControlType(VRFTU(VRFTUNum).ZoneNum) == DataHVACGlobals::SingleHeatingSetPoint) {
                zoneLoad = 0.0;
            } else if (LoadToHeatingSP <= 0.0 && LoadToCoolingSP >= 0.0) {
                zoneLoad = 0.0;
            }
        } else if (VRFTU(VRFTUNum).ZoneNum > 0) {
            // zone equipment uses Remaining* variables
            if (InitFlag) {
                // this will need more investigation. Using Remaining* variable during the initial load calculation seems wrong.
                // This may also have implications when VRF TUs are in the air loop or if SP control is used
                // another question is whether initialization of the opeating mode should look at TotalOutputRequired or RemainingOutputRequired
                zoneLoad = ZoneSysEnergyDemand(VRFTU(VRFTUNum).ZoneNum).RemainingOutputRequired / VRFTU(VRFTUNum).controlZoneMassFlowFrac;
                LoadToCoolingSP = DataZoneEnergyDemands::ZoneSysEnergyDemand(VRFTU(VRFTUNum).ZoneNum).OutputRequiredToCoolingSP /
                                  VRFTU(VRFTUNum).controlZoneMassFlowFrac;
                LoadToHeatingSP = DataZoneEnergyDemands::ZoneSysEnergyDemand(VRFTU(VRFTUNum).ZoneNum).OutputRequiredToHeatingSP /
                                  VRFTU(VRFTUNum).controlZoneMassFlowFrac;
            } else {
                zoneLoad = ZoneSysEnergyDemand(VRFTU(VRFTUNum).ZoneNum).RemainingOutputRequired / VRFTU(VRFTUNum).controlZoneMassFlowFrac;
                LoadToCoolingSP = DataZoneEnergyDemands::ZoneSysEnergyDemand(VRFTU(VRFTUNum).ZoneNum).RemainingOutputReqToCoolSP /
                                  VRFTU(VRFTUNum).controlZoneMassFlowFrac;
                LoadToHeatingSP = DataZoneEnergyDemands::ZoneSysEnergyDemand(VRFTU(VRFTUNum).ZoneNum).RemainingOutputReqToHeatSP /
                                  VRFTU(VRFTUNum).controlZoneMassFlowFrac;
            }
        } else if (VRFTU(VRFTUNum).isSetPointControlled) {
            if (VRFTU(VRFTUNum).coolSPActive) {
                LoadToCoolingSP = VRFTU(VRFTUNum).coolLoadToSP;
                zoneLoad = LoadToCoolingSP;
                LoadToHeatingSP = 0.0;
            }
            if (VRFTU(VRFTUNum).heatSPActive) {
                LoadToHeatingSP = VRFTU(VRFTUNum).heatLoadToSP;
                zoneLoad = LoadToHeatingSP;
                LoadToCoolingSP = 0.0;
            }
        }
    }

    void VRFCondenserEquipment::CalcVRFIUTeTc_FluidTCtrl()
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         RP Zhang (LBNL), XF Pang (LBNL), Y Yura (Daikin Inc)
        //       DATE WRITTEN   June 2015
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        //       This subroutine is part of the new VRF model based on physics, applicable for Fluid Temperature Control.
        //       This subroutine determines the VRF evaporating temperature at cooling mode and the condensing temperature
        //       at heating mode. This is the indoor unit side analysis.

        // METHODOLOGY EMPLOYED:
        //       There are two options to calculate the IU Te/Tc: (1) HighSensible method analyzes the conditions of each IU
        //       and then decide and Te/Tc that can satisfy all the zones (2) TeTcConstant method uses fixed values provided
        //       by the user.

        // Followings for FluidTCtrl Only
        Array1D<Real64> EvapTemp;
        Array1D<Real64> CondTemp;
        Real64 IUMinEvapTemp;
        Real64 IUMaxCondTemp;

        int TUListNum = this->ZoneTUListPtr;
        EvapTemp.allocate(TerminalUnitList(TUListNum).NumTUInList);
        CondTemp.allocate(TerminalUnitList(TUListNum).NumTUInList);
        IUMinEvapTemp = 100.0;
        IUMaxCondTemp = 0.0;

        if (this->AlgorithmIUCtrl == 1) {
            // 1. HighSensible: analyze the conditions of each IU

            for (int i = 1; i <= TerminalUnitList(TUListNum).NumTUInList; i++) {
                int VRFTUNum = TerminalUnitList(TUListNum).ZoneTUPtr(i);
                // analyze the conditions of each IU
                VRFTU(VRFTUNum).CalcVRFIUVariableTeTc(EvapTemp(i), CondTemp(i));

                // select the Te/Tc that can satisfy all the zones
                IUMinEvapTemp = min(IUMinEvapTemp, EvapTemp(i), this->IUEvapTempHigh);
                IUMaxCondTemp = max(IUMaxCondTemp, CondTemp(i), this->IUCondTempLow);
            }

            this->IUEvaporatingTemp = max(IUMinEvapTemp, this->IUEvapTempLow);
            this->IUCondensingTemp = min(IUMaxCondTemp, this->IUCondTempHigh);

        } else {
            // 2. TeTcConstant: use fixed values provided by the user
            this->IUEvaporatingTemp = this->EvapTempFixed;
            this->IUCondensingTemp = this->CondTempFixed;
        }
    }

    void VRFTerminalUnitEquipment::CalcVRFIUVariableTeTc(Real64 &EvapTemp, // evaporating temperature
                                                         Real64 &CondTemp  // condensing temperature
    )
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Xiufeng Pang, LBNL
        //       DATE WRITTEN   Feb 2014
        //       MODIFIED       Jul 2015, RP Zhang, LBNL, Modify the bounds of the Te/Tc
        //       MODIFIED       Nov 2015, RP Zhang, LBNL, take into account OA in Te/Tc determination
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        //       Calculate the VRF IU Te (cooling mode) and Tc (heating mode), given zonal loads.

        // METHODOLOGY EMPLOYED:
        //       A new physics based VRF model applicable for Fluid Temperature Control.

        using namespace DataZoneEnergyDemands;
        using DataEnvironment::OutBaroPress;
        using DXCoils::DXCoil;
        using HVACVariableRefrigerantFlow::VRF;
        using HVACVariableRefrigerantFlow::VRFTU;
        using MixedAir::SimOAMixer;
        using Psychrometrics::PsyHFnTdbW;
        using SingleDuct::SimATMixer;

        int CoolCoilNum;             // index to the VRF Cooling DX coil to be simulated
        int HeatCoilNum;             // index to the VRF Heating DX coil to be simulated
        int IndexToTUInTUList;       // index to TU in specific list for the VRF system
        int TUListIndex;             // index to TU list for this VRF system
        int VRFNum;                  // index to VRF that the VRF Terminal Unit serves
        int VRFInletNode;            // VRF inlet node number
        int ZoneIndex;               // index to zone where the VRF Terminal Unit resides
        Real64 BFC;                  // Bypass factor at the cooling mode (-)
        Real64 BFH;                  // Bypass factor at the heating mode (-)
        Real64 C1Tevap;              // Coefficient for indoor unit coil evaporating temperature curve (-)
        Real64 C2Tevap;              // Coefficient for indoor unit coil evaporating temperature curve (-)
        Real64 C3Tevap;              // Coefficient for indoor unit coil evaporating temperature curve (-)
        Real64 C1Tcond;              // Coefficient for indoor unit coil condensing temperature curve (-)
        Real64 C2Tcond;              // Coefficient for indoor unit coil condensing temperature curve (-)
        Real64 C3Tcond;              // Coefficient for indoor unit coil condensing temperature curve (-)
        Real64 CondTempMin;          // Min condensing temperature (C)
        Real64 CondTempMax;          // Max condensing temperature, correspond to the maximum heating capacity (C)
        Real64 DeltaT;               // Difference between evaporating/condensing temperature and coil surface temperature (C)
        Real64 EvapTempMax;          // Max evaporating temperature (C)
        Real64 EvapTempMin;          // Min evaporating temperature, correspond to the maximum cooling capacity (C)
        Real64 Garate;               // Nominal air mass flow rate
        Real64 H_coil_in;            // Air enthalpy at the coil inlet (kJ/kg)
        Real64 QZnReqSenCoolingLoad; // Zone required sensible cooling load (W)
        Real64 QZnReqSenHeatingLoad; // Zone required sensible heating load (W)
        Real64 RHsat;                // Relative humidity of the air at saturated condition(-)
        Real64 SH;                   // Super heating degrees (C)
        Real64 SC;                   // Subcooling degrees (C)
        Real64 T_coil_in;            // Temperature of the air at the coil inlet, after absorbing the heat released by fan (C)
        Real64 T_TU_in;              // Air temperature at the indoor unit inlet (C)
        Real64 Tout;                 // Air temperature at the indoor unit outlet (C)
        Real64 Th2;                  // Air temperature at the coil surface (C)
        Real64 W_coil_in;            // coil inlet air humidity ratio [kg/kg]
        Real64 W_TU_in;              // Air humidity ratio at the indoor unit inlet[kg/kg]

        // Get the equipment/zone index corresponding to the VRFTU
        CoolCoilNum = this->CoolCoilIndex;
        HeatCoilNum = this->HeatCoilIndex;
        ZoneIndex = this->ZoneNum;
        VRFNum = this->VRFSysNum;
        TUListIndex = VRF(VRFNum).ZoneTUListPtr;
        IndexToTUInTUList = this->IndexToTUInTUList;

        // Bounds of Te/Tc for VRF IU Control Algorithm: VariableTemp
        EvapTempMin = VRF(VRFNum).IUEvapTempLow;
        EvapTempMax = VRF(VRFNum).IUEvapTempHigh;
        CondTempMin = VRF(VRFNum).IUCondTempLow;
        CondTempMax = VRF(VRFNum).IUCondTempHigh;

        // Coefficients describing coil performance
        SH = DXCoil(CoolCoilNum).SH;
        SC = DXCoil(HeatCoilNum).SC;
        C1Tevap = DXCoil(CoolCoilNum).C1Te;
        C2Tevap = DXCoil(CoolCoilNum).C2Te;
        C3Tevap = DXCoil(CoolCoilNum).C3Te;
        C1Tcond = DXCoil(HeatCoilNum).C1Tc;
        C2Tcond = DXCoil(HeatCoilNum).C2Tc;
        C3Tcond = DXCoil(HeatCoilNum).C3Tc;

        VRFInletNode = this->VRFTUInletNodeNum;
        T_TU_in = DataLoopNode::Node(VRFInletNode).Temp;
        W_TU_in = DataLoopNode::Node(VRFInletNode).HumRat;
        T_coil_in = this->coilInNodeT;
        W_coil_in = this->coilInNodeW;

        Garate = CompOnMassFlow;
        H_coil_in = PsyHFnTdbW(T_coil_in, W_coil_in);
        RHsat = 0.98;
        BFC = 0.0592;
        BFH = 0.136;
        Real64 ZoneLoad = 0.0;
        Real64 LoadToHeatingSP = 0.0;
        Real64 LoadToCoolingSP = 0.0;

        // 1. COOLING Mode
        if ((Garate > 0.0) && ((!VRF(VRFNum).HeatRecoveryUsed && CoolingLoad(VRFNum)) ||
                               (VRF(VRFNum).HeatRecoveryUsed && TerminalUnitList(TUListIndex).HRCoolRequest(IndexToTUInTUList)))) {
            // 1.1) Cooling coil is running
            getVRFTUZoneLoad(IndexToTUInTUList, ZoneLoad, LoadToHeatingSP, LoadToCoolingSP, false);
            QZnReqSenCoolingLoad = max(0.0, -1.0 * LoadToCoolingSP);
            Tout = T_TU_in - QZnReqSenCoolingLoad * 1.2 / Garate / 1005;
            Th2 = T_coil_in - (T_coil_in - Tout) / (1 - BFC);
            DeltaT = C3Tevap * SH * SH + C2Tevap * SH + C1Tevap;
            EvapTemp = max(min((Th2 - DeltaT), EvapTempMax), EvapTempMin);

        } else {
            // 1.2) Cooling coil is not running
            EvapTemp = T_coil_in;
        }

        // 2. HEATING Mode
        if ((Garate > 0.0) && ((!VRF(VRFNum).HeatRecoveryUsed && HeatingLoad(VRFNum)) ||
                               (VRF(VRFNum).HeatRecoveryUsed && TerminalUnitList(TUListIndex).HRHeatRequest(IndexToTUInTUList)))) {
            // 2.1) Heating coil is running
            getVRFTUZoneLoad(IndexToTUInTUList, ZoneLoad, LoadToHeatingSP, LoadToCoolingSP, false);
            QZnReqSenHeatingLoad = max(0.0, LoadToHeatingSP);
            Tout = T_TU_in + QZnReqSenHeatingLoad / Garate / 1005;
            Th2 = T_coil_in + (Tout - T_coil_in) / (1 - BFH);
            DeltaT = C3Tcond * SC * SC + C2Tcond * SC + C1Tcond;
            CondTemp = max(min((Th2 + DeltaT), CondTempMax), CondTempMin);
        } else {
            // 2.2) Heating coil is not running
            CondTemp = T_coil_in;
        }
    }

    void VRFCondenserEquipment::CalcVRFCondenser_FluidTCtrl(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         RP Zhang (LBNL), XF Pang (LBNL), Y Yura (Daikin Inc)
        //       DATE WRITTEN   June 2015
        //       MODIFIED       Feb 2016, RP Zhang, add the control logics for VRF-HR operations
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        //       This subroutine is part of the new VRF model based on physics, applicable for Fluid Temperature Control.
        //       This is adapted from subroutine CalcVRFCondenser, which is part of the VRF model based on system curves.
        //       This subroutine models the interactions of VRF indoor units with the outdoor unit.
        //       The indoor terminal units are simulated first, and then the outdoor unit is simulated.

        // METHODOLOGY EMPLOYED:
        //       A new physics based VRF model applicable for Fluid Temperature Control.

        using CurveManager::CurveValue;
        using DataEnvironment::CurMnDy;
        using DataEnvironment::EnvironmentName;
        using DataEnvironment::OutBaroPress;
        using DataEnvironment::OutDryBulbTemp;
        using DataEnvironment::OutHumRat;
        using DataEnvironment::OutWetBulbTemp;
        using DXCoils::DXCoil;
        using DXCoils::DXCoilCoolInletAirWBTemp;
        using DXCoils::DXCoilHeatInletAirDBTemp;
        using DXCoils::DXCoilHeatInletAirWBTemp;
        using DXCoils::DXCoilTotalHeating;
        using FluidProperties::FindRefrigerant;
        using FluidProperties::GetSatEnthalpyRefrig;
        using FluidProperties::GetSatPressureRefrig;
        using FluidProperties::GetSatTemperatureRefrig;
        using FluidProperties::GetSpecificHeatGlycol;
        using FluidProperties::GetSupHeatDensityRefrig;
        using FluidProperties::GetSupHeatEnthalpyRefrig;
        using FluidProperties::GetSupHeatTempRefrig;
        using FluidProperties::RefrigData;
        using General::SolveRoot;
        using General::TrimSigDigits;
        using PlantUtilities::SetComponentFlowRate;
        using Psychrometrics::RhoH2O;
        using TempSolveRoot::SolveRoot;

        static std::string const RoutineName("CalcVRFCondenser_FluidTCtrl");

        int VRFCond;            // index to VRF condenser
        int TUListNum;          // index to TU List
        int NumTUInList;        // number of terminal units is list
        int NumTU;              // index for loop on terminal units
        int TUIndex;            // Index to terminal unit
        int CoolCoilIndex;      // index to cooling coil in terminal unit
        int HeatCoilIndex;      // index to heating coil in terminal unit
        int NumTUInCoolingMode; // number of terminal units actually cooling
        int NumTUInHeatingMode; // number of terminal units actually heating

        Real64 TUParasiticPower;          // total terminal unit parasitic power (W)
        Real64 TUFanPower;                // total terminal unit fan power (W)
        Real64 InletAirWetBulbC;          // coil inlet air wet-bulb temperature (C)
        Real64 InletAirDryBulbC;          // coil inlet air dry-bulb temperature (C)
        Real64 CondInletTemp;             // condenser inlet air temperature (C)
        Real64 OutdoorDryBulb;            // outdoor dry-bulb temperature (C)
        Real64 OutdoorHumRat;             // outdoor humidity ratio (kg/kg)
        Real64 OutdoorPressure;           // outdoor pressure (Pa)
        Real64 OutdoorWetBulb;            // outdoor wet-bulb temperature (C)
        Real64 SumCoolInletWB;            // sum of active TU's DX cooling coil inlet air wet-bulb temperature
        Real64 SumHeatInletDB;            // sum of active TU's DX heating coil inlet air dry-bulb temperature
        Real64 SumHeatInletWB;            // sum of active TU's DX heating coil inlet air wet-bulb temperature
        Real64 TotalTUCoolingCapacity;    // sum of TU's cooling capacity (W)
        Real64 TotalTUHeatingCapacity;    // sum of TU's heating capacity (W)
        Real64 TotalCondCoolingCapacity;  // total available condenser cooling capacity (W)
        Real64 TotalCondHeatingCapacity;  // total available condenser heating capacity (W)
        Real64 CoolingPLR;                // condenser cooling PLR
        Real64 HeatingPLR;                // condenser heating PLR
        Real64 CyclingRatio;              // cycling ratio of condenser's compressors
        int Stage;                        // used for crankcase heater power calculation
        Real64 UpperStageCompressorRatio; // used for crankcase heater power calculation
        Real64 RhoAir;                    // Density of air [kg/m3]
        Real64 PartLoadFraction;          // Part load fraction from PLFFPLR curve
        Real64 VRFRTF;                    // VRF runtime fraction when cycling below MINPLR
        Real64 OutdoorCoilT;              // Outdoor coil temperature (C)
        Real64 OutdoorCoildw;             // Outdoor coil delta w assuming coil temp of OutdoorCoilT (kg/kg)
        Real64 FractionalDefrostTime;     // Fraction of time step system is in defrost
        Real64 HeatingCapacityMultiplier; // Multiplier for heating capacity when system is in defrost
        Real64 InputPowerMultiplier;      // Multiplier for power when system is in defrost
        Real64 LoadDueToDefrost;          // Additional load due to defrost
        Real64 DefrostEIRTempModFac;      // EIR modifier for defrost (function of entering drybulb, outside wetbulb)
        Real64 HRInitialCapFrac;          // Fractional cooling degradation at the start of heat recovery from cooling mode
        Real64 HRCapTC;                   // Time constant used to recover from initial degradation in cooling heat recovery
        Real64 HRInitialEIRFrac;          // Fractional cooling degradation at the start of heat recovery from cooling mode
        Real64 HREIRTC;                   // Time constant used to recover from initial degradation in cooling heat recovery
        static Real64 CurrentEndTime;     // end time of current time step
        static Real64 CurrentEndTimeLast; // end time of last time step
        static Real64 TimeStepSysLast;    // system time step on last time step
        Real64 SUMultiplier;              // multiplier for simulating mode changes
        Real64 CondPower;                 // condenser power [W]
        Real64 CondCapacity;              // condenser heat rejection [W]
        Real64 TotPower;                  // total condenser power use [W]
        bool HRHeatRequestFlag;           // flag indicating VRF TU could operate in heating mode
        bool HRCoolRequestFlag;           // flag indicating VRF TU could operate in cooling mode

        // Followings for VRF FluidTCtrl Only
        int Counter;                     // index for iterations [-]
        int NumIteHIUIn;                 // index for HIU calculation iterations [-]
        int NumOfCompSpdInput;           // Number of compressor speed input by the user [-]
        int RefrigerantIndex;            // Index of the refrigerant [-]
        Real64 CompSpdActual;            // Actual compressor running speed [rps]
        Real64 C_cap_operation;          // Compressor capacity modification algorithm_modified Cap [-]
        Real64 CompEvaporatingCAPSpdMin; // evaporating capacity at the lowest compressor speed [W]
        Real64 CompEvaporatingCAPSpdMax; // evaporating capacity at the highest compressor speed [W]
        Real64 CompEvaporatingPWRSpdMin; // compressor power at the lowest compressor speed [W]
        Real64 CompEvaporatingPWRSpdMax; // compressor power at the highest compressor speed [W]
        Real64 CapMaxTe;                 // maximum Te during operation, for capacity calculations [C]
        Real64 CapMinTe;                 // minimum Te during operation, for capacity calculations [C]
        Real64 CapMinPe;                 // minimum Pe during operation, for capacity calculations [Pa]
        Real64 CapMaxTc;                 // maximum Tc during operation, for capacity calculations [C]
        Real64 CapMaxPc;                 // maximum Pc during operation, for capacity calculations [Pa]
        Real64 CapMinTc;                 // minimum Tc during operation, for capacity calculations [C]
        Real64 CapMinPc;                 // minimum Pc during operation, for capacity calculations [Pa]
        Real64 h_IU_evap_in;             // enthalpy of IU evaporator at inlet [kJ/kg]
        Real64 h_IU_evap_in_new;         // enthalpy of IU evaporator at inlet (new) [kJ/kg]
        Real64 h_IU_evap_in_low;         // enthalpy of IU evaporator at inlet (low) [kJ/kg]
        Real64 h_IU_evap_in_up;          // enthalpy of IU evaporator at inlet (up) [kJ/kg]
        Real64 h_IU_evap_out;            // enthalpy of IU evaporator at outlet [kJ/kg]
        Real64 h_IU_evap_out_i;          // enthalpy of IU evaporator at outlet (individual) [kJ/kg]
        Real64 h_IU_cond_in;             // enthalpy of IU condenser at inlet [kJ/kg]
        Real64 h_IU_cond_in_low;         // enthalpy of IU condenser at inlet (low) [kJ/kg]
        Real64 h_IU_cond_in_up;          // enthalpy of IU condenser at inlet (up) [kJ/kg]
        Real64 h_IU_cond_out;            // enthalpy of IU condenser at outlet [kJ/kg]
        Real64 h_IU_cond_out_i;          // enthalpy of IU condenser at outlet (individual) [kJ/kg]
        Real64 h_IU_cond_out_ave;        // average enthalpy of the refrigerant leaving IU condensers [kJ/kg]
        Real64 h_IU_PLc_out;             // enthalpy of refrigerant at the outlet of IU evaporator side main pipe, after piping loss (c) [kJ/kg]
        Real64 h_comp_in;                // enthalpy of refrigerant at compressor inlet, after piping loss (c) [kJ/kg]
        Real64 h_comp_in_new;            // enthalpy of refrigerant at compressor inlet (new) [kJ/kg]
        Real64 h_comp_out;               // enthalpy of refrigerant at compressor outlet [kJ/kg]
        Real64 h_comp_out_new;           // enthalpy of refrigerant at compressor outlet (new) [kJ/kg]
        Real64 m_air;                    // OU coil air mass flow rate [kg/s]
        Real64 m_ref_IU_cond;            // mass flow rate of Refrigerant through IU condensers [kg/s]
        Real64 m_ref_IU_cond_i;          // mass flow rate of Refrigerant through an individual IU condenser [kg/s]
        Real64 m_ref_IU_evap;            // mass flow rate of Refrigerant through IU evaporators [kg/s]
        Real64 m_ref_IU_evap_i;          // mass flow rate of Refrigerant through an individual IU evaporator [kg/s]
        Real64 m_ref_OU_evap;            // mass flow rate of Refrigerant through OU evaporator [kg/s]
        Real64 m_ref_OU_cond;            // mass flow rate of Refrigerant through OU condenser [kg/s]
        Real64 Ncomp;                    // compressor power [W]
        Real64 Ncomp_new;                // compressor power for temporary use in iterations [W]
        Real64 P_comp_in;                // pressure of refrigerant at IU condenser outlet [Pa]
        Real64 Pcond;                    // VRF condensing pressure [Pa]
        Real64 Pevap;                    // VRF evaporating pressure [Pa]
        Real64 Pdischarge;               // VRF compressor discharge pressure [Pa]
        Real64 Psuction;                 // VRF compressor suction pressure [Pa]
        Real64 Pipe_DeltP_c;             // Piping Loss Algorithm Parameter: Pipe pressure drop (c) [Pa]
        Real64 Pipe_DeltP_h;             // Piping Loss Algorithm Parameter: Pipe pressure drop (h) [Pa]
        Real64 Pipe_Q_c;                 // Piping Loss Algorithm Parameter: Heat loss (c) [W]
        Real64 Pipe_Q_h;                 // Piping Loss Algorithm Parameter: Heat loss (h) [W]
        Real64 Q_c_TU_PL;                // Cooling load to be met at heating mode, including the piping loss(W)
        Real64 Q_h_TU_PL;                // Heating load to be met at heating mode, including the piping loss (W)
        Real64 Q_h_OU;                   // outdoor unit condenser heat release (cooling mode) [W]
        Real64 Q_c_OU;                   // outdoor unit evaporator heat extract (heating mode) [W]
        Real64 RefMaxPc;                 // maximum refrigerant condensing pressure [Pa]
        Real64 RefMinTe;                 // Minimum refrigerant evaporating temperature [Pa]
        Real64 RefMinPe;                 // Minimum refrigerant evaporating pressure [Pa]
        Real64 RefPLow;                  // Low Pressure Value for Ps (>0.0) [Pa]
        Real64 RefPHigh;                 // High Pressure Value for Ps (max in tables) [Pa]
        Real64 RefTLow;                  // Low Temperature Value for Ps (max in tables) [C]
        Real64 RefTHigh;                 // High Temperature Value for Ps (max in tables) [C]
        Real64 RefTSat;        // Saturated temperature of the refrigerant. Used to check whether the refrigerant is in the superheat area [C]
        Real64 SC_IU_merged;   // Piping Loss Algorithm Parameter: average subcooling degrees after the indoor units [C]
        Real64 SH_IU_merged;   // Piping Loss Algorithm Parameter: average super heating degrees after the indoor units [C]
        Real64 SC_OU;          // subcooling degrees at OU condenser [C]
        Real64 SH_OU;          // super heating degrees at OU evaporator [C]
        Real64 SH_Comp;        // Temperature difference between compressor inlet node and Tsuction [C]
        Real64 T_comp_in;      // temperature of refrigerant at compressor inlet, after piping loss (c) [C]
        Real64 TU_HeatingLoad; // Heating load from terminal units, excluding heating loss [W]
        Real64 TU_CoolingLoad; // Cooling load from terminal units, excluding heating loss [W]
        Real64 Tdischarge;     // VRF Compressor discharge refrigerant temperature [C]
        Real64 Tsuction;       // VRF compressor suction refrigerant temperature [C]
        Real64 Tolerance;      // Tolerance for condensing temperature calculation [C]
        Real64 Tfs;            // Temperature of the air at the coil surface [C]
        Array1D<Real64> CompEvaporatingPWRSpd; // Array for the compressor power at certain speed [W]
        Array1D<Real64> CompEvaporatingCAPSpd; // Array for the evaporating capacity at certain speed [W]
        Array1D<Real64> Par(3);                // Array for the parameters [-]

        // FLOW

        // variable initializations
        TUListNum = this->ZoneTUListPtr;
        NumTUInList = TerminalUnitList(TUListNum).NumTUInList;
        VRFCond = VRFTU(TerminalUnitList(TUListNum).ZoneTUPtr(1)).VRFSysNum;
        TU_CoolingLoad = 0.0;
        TU_HeatingLoad = 0.0;
        TUParasiticPower = 0.0;
        TUFanPower = 0.0;
        CoolingPLR = 0.0;
        HeatingPLR = 0.0;
        CyclingRatio = 1.0;
        SumCoolInletWB = 0.0;
        SumHeatInletDB = 0.0;
        SumHeatInletWB = 0.0;
        TotalCondCoolingCapacity = 0.0;
        TotalCondHeatingCapacity = 0.0;
        TotalTUCoolingCapacity = 0.0;
        TotalTUHeatingCapacity = 0.0;
        NumTUInCoolingMode = 0;
        NumTUInHeatingMode = 0;
        Tolerance = 0.05;
        RefrigerantIndex = -1;
        Counter = 1;
        NumIteHIUIn = 1;
        this->ElecCoolingPower = 0.0;
        this->ElecHeatingPower = 0.0;
        this->CrankCaseHeaterPower = 0.0;
        this->EvapCondPumpElecPower = 0.0; // for EvaporativelyCooled condenser
        this->EvapWaterConsumpRate = 0.0;
        this->DefrostPower = 0.0;
        this->OperatingCoolingCOP = 0.0;
        this->OperatingHeatingCOP = 0.0;
        this->OperatingCOP = 0.0;
        this->SCHE = 0.0;
        this->BasinHeaterPower = 0.0;
        this->CondensingTemp = 60.0; // OutDryBulbTemp;
        this->VRFHeatRec = 0.0;

        // Refrigerant data
        RefMinTe = -15;
        RefMaxPc = 4000000.0;
        RefrigerantIndex = FindRefrigerant(state, this->RefrigerantName);
        RefMinPe = GetSatPressureRefrig(state, this->RefrigerantName, RefMinTe, RefrigerantIndex, RoutineName);
        RefMinPe = GetSatPressureRefrig(state, this->RefrigerantName, RefMinTe, RefrigerantIndex, RoutineName);
        RefTLow = RefrigData(RefrigerantIndex).PsLowTempValue;   // High Temperature Value for Ps (max in tables)
        RefTHigh = RefrigData(RefrigerantIndex).PsHighTempValue; // High Temperature Value for Ps (max in tables)
        RefPLow = RefrigData(RefrigerantIndex).PsLowPresValue;   // Low Pressure Value for Ps (>0.0)
        RefPHigh = RefrigData(RefrigerantIndex).PsHighPresValue; // High Pressure Value for Ps (max in tables)

        // sum loads on TU coils
        for (NumTU = 1; NumTU <= NumTUInList; ++NumTU) {
            TU_CoolingLoad += TerminalUnitList(TUListNum).TotalCoolLoad(NumTU);
            TU_HeatingLoad += TerminalUnitList(TUListNum).TotalHeatLoad(NumTU);
            TUParasiticPower += VRFTU(TerminalUnitList(TUListNum).ZoneTUPtr(NumTU)).ParasiticCoolElecPower +
                                VRFTU(TerminalUnitList(TUListNum).ZoneTUPtr(NumTU)).ParasiticHeatElecPower;
            TUFanPower += VRFTU(TerminalUnitList(TUListNum).ZoneTUPtr(NumTU)).FanPower;
        }
        this->TUCoolingLoad = TU_CoolingLoad; // this is cooling coil load, not terminal unit load
        this->TUHeatingLoad = TU_HeatingLoad; // this is heating coil load, not terminal unit load

        // loop through TU's and calculate average inlet conditions for active coils
        for (NumTU = 1; NumTU <= NumTUInList; ++NumTU) {
            TUIndex = TerminalUnitList(TUListNum).ZoneTUPtr(NumTU);
            CoolCoilIndex = VRFTU(TUIndex).CoolCoilIndex;
            HeatCoilIndex = VRFTU(TUIndex).HeatCoilIndex;

            if (TerminalUnitList(TUListNum).TotalCoolLoad(NumTU) > 0.0) {
                SumCoolInletWB += DXCoilCoolInletAirWBTemp(CoolCoilIndex) * TerminalUnitList(TUListNum).TotalCoolLoad(NumTU) / TU_CoolingLoad;
                ++NumTUInCoolingMode;
            }
            if (TerminalUnitList(TUListNum).TotalHeatLoad(NumTU) > 0.0) {
                SumHeatInletDB += DXCoilHeatInletAirDBTemp(HeatCoilIndex) * TerminalUnitList(TUListNum).TotalHeatLoad(NumTU) / TU_HeatingLoad;
                SumHeatInletWB += DXCoilHeatInletAirWBTemp(HeatCoilIndex) * TerminalUnitList(TUListNum).TotalHeatLoad(NumTU) / TU_HeatingLoad;
                ++NumTUInHeatingMode;
            }
        }

        // set condenser entering air conditions (Outdoor air conditions)
        if (this->CondenserNodeNum != 0) {
            OutdoorDryBulb = DataLoopNode::Node(this->CondenserNodeNum).Temp;
            if (this->CondenserType != DataHVACGlobals::WaterCooled) {
                OutdoorHumRat = DataLoopNode::Node(this->CondenserNodeNum).HumRat;
                OutdoorPressure = DataLoopNode::Node(this->CondenserNodeNum).Press;
                OutdoorWetBulb = DataLoopNode::Node(this->CondenserNodeNum).OutAirWetBulb;
            } else {
                OutdoorHumRat = OutHumRat;
                OutdoorPressure = OutBaroPress;
                OutdoorWetBulb = OutWetBulbTemp;
            }
        } else {
            OutdoorDryBulb = OutDryBulbTemp;
            OutdoorHumRat = OutHumRat;
            OutdoorPressure = OutBaroPress;
            OutdoorWetBulb = OutWetBulbTemp;
        }
        RhoAir = PsyRhoAirFnPbTdbW(OutdoorPressure, OutdoorDryBulb, OutdoorHumRat);

        CondInletTemp = OutdoorDryBulb; // this->CondenserType == AirCooled
        this->CondenserInletTemp = CondInletTemp;

        //*************
        // VRF-HP MODES:
        //	 1. Cooling
        //	 2. Heating
        //	 3. No running
        // VRF-HR MODES:
        //	 1. Cooling Only
        //	 2. Cooling Dominant w/o HR Loss
        //	 3. Cooling Dominant w/ HR Loss
        //	 4. Heating Dominant w/ HR Loss
        //	 5. Heating Dominant w/o HR Loss
        //	 6. Heating Only
        //	 7. No running

        // Flag for VRF-HR Operations
        if (TU_HeatingLoad > 0) {
            HRHeatRequestFlag = true;
        } else {
            HeatingLoad(VRFCond) = false;
            HRHeatRequestFlag = false;
        }
        if (TU_CoolingLoad > 0) {
            HRCoolRequestFlag = true;
        } else {
            CoolingLoad(VRFCond) = false;
            HRCoolRequestFlag = false;
        }

        // Initialization for Ncomp iterations
        NumOfCompSpdInput = this->CompressorSpeed.size();
        CompEvaporatingPWRSpd.dimension(NumOfCompSpdInput);
        CompEvaporatingCAPSpd.dimension(NumOfCompSpdInput);
        this->OperatingMode = 0; // report variable for heating or cooling mode

        // 1. VRF-HP Cooling Mode .OR. VRF-HR Mode_1
        if ((!this->HeatRecoveryUsed && CoolingLoad(VRFCond)) || (this->HeatRecoveryUsed && !HRHeatRequestFlag && HRCoolRequestFlag)) {

            this->OperatingMode = ModeCoolingOnly;
            this->VRFOperationSimPath = 10;

            // Initialization of VRF-FluidTCtrl Model
            Q_c_TU_PL = TU_CoolingLoad;

            // Evaporator (IU side) operational parameters
            Pevap = GetSatPressureRefrig(state, this->RefrigerantName, this->IUEvaporatingTemp, RefrigerantIndex, RoutineName);
            Psuction = Pevap;
            Tsuction = this->IUEvaporatingTemp; // GetSatTemperatureRefrig(state,  this->RefrigerantName, max( min( Psuction, RefPHigh ), RefPLow ),
                                                // RefrigerantIndex, RoutineName );
            this->EvaporatingTemp = this->IUEvaporatingTemp; // GetSatTemperatureRefrig(state,  this->RefrigerantName, max( min( Pevap, RefPHigh ), RefPLow
                                                             // ), RefrigerantIndex, RoutineName );

            // Condenser (OU side) operation ranges
            CapMaxPc = min(Psuction + this->CompMaxDeltaP, RefMaxPc);
            CapMaxTc = GetSatTemperatureRefrig(state, this->RefrigerantName, max(min(CapMaxPc, RefPHigh), RefPLow), RefrigerantIndex, RoutineName);
            CapMinTc = OutdoorDryBulb + this->SC;
            CapMinPc = GetSatPressureRefrig(state, this->RefrigerantName, CapMinTc, RefrigerantIndex, RoutineName);

            // Evaporator (IU side) operation ranges
            CapMinPe = max(CapMinPc - this->CompMaxDeltaP, RefMinPe);
            CapMinTe = GetSatTemperatureRefrig(state, this->RefrigerantName, max(min(CapMinPe, RefPHigh), RefPLow), RefrigerantIndex, RoutineName);

            // Evaporative capacity ranges
            CompEvaporatingCAPSpdMin = this->CoffEvapCap * this->RatedEvapCapacity * CurveValue(state, this->OUCoolingCAPFT(1), CapMinTc, CapMinTe);
            CompEvaporatingPWRSpdMin = this->RatedCompPower * CurveValue(state, this->OUCoolingPWRFT(1), CapMinTc, CapMinTe);
            CompEvaporatingCAPSpdMax = this->CoffEvapCap * this->RatedEvapCapacity *
                                       CurveValue(state, this->OUCoolingCAPFT(NumOfCompSpdInput), this->CondensingTemp, this->IUEvaporatingTemp);
            CompEvaporatingPWRSpdMax =
                this->RatedCompPower * CurveValue(state, this->OUCoolingPWRFT(NumOfCompSpdInput), this->CondensingTemp, this->IUEvaporatingTemp);

            // Initialization for h_IU_evap_in iterations (Label12)
            h_IU_evap_in_low =
                GetSatEnthalpyRefrig(state, this->RefrigerantName, OutdoorDryBulb - this->SC, 0.0, RefrigerantIndex, RoutineName);         // Tc = Tamb
            h_IU_evap_in_up = GetSatEnthalpyRefrig(state, this->RefrigerantName, CapMaxTc - this->SC, 0.0, RefrigerantIndex, RoutineName); // Tc = CapMaxTc
            h_IU_evap_in =
                GetSatEnthalpyRefrig(state, this->RefrigerantName, OutdoorDryBulb + 10 - this->SC, 0.0, RefrigerantIndex, RoutineName); // Tc = Tamb+10

            NumIteHIUIn = 1;
        Label12:;
            m_ref_IU_evap = 0;
            h_IU_evap_out = 0;
            h_IU_evap_out_i = 0;
            m_ref_IU_evap_i = 0;
            SH_IU_merged = 0;

            // Calculate total IU refrigerant flow rate and SH_IU_merged
            if (Q_c_TU_PL > CompEvaporatingCAPSpdMax) {
                // Required load is beyond the max system capacity

                Q_c_TU_PL = CompEvaporatingCAPSpdMax;
                TU_CoolingLoad = CompEvaporatingCAPSpdMax;
                this->TUCoolingLoad = TU_CoolingLoad;
                RefTSat = GetSatTemperatureRefrig(state, this->RefrigerantName, max(min(Pevap, RefPHigh), RefPLow), RefrigerantIndex, RoutineName);
                h_IU_evap_out = GetSupHeatEnthalpyRefrig(state,
                                                         this->RefrigerantName,
                                                         max(RefTSat, this->IUEvaporatingTemp + 3),
                                                         max(min(Pevap, RefPHigh), RefPLow),
                                                         RefrigerantIndex,
                                                         RoutineName);
                SH_IU_merged = 3;
                m_ref_IU_evap = TU_CoolingLoad / (h_IU_evap_out - h_IU_evap_in);

            } else {

                for (NumTU = 1; NumTU <= NumTUInList; NumTU++) { // Calc total refrigerant flow rate
                    if (TerminalUnitList(TUListNum).TotalCoolLoad(NumTU) > 0) {
                        TUIndex = TerminalUnitList(TUListNum).ZoneTUPtr(NumTU);
                        CoolCoilIndex = VRFTU(TUIndex).CoolCoilIndex;

                        RefTSat = GetSatTemperatureRefrig(state, this->RefrigerantName, max(min(Pevap, RefPHigh), RefPLow), RefrigerantIndex, RoutineName);
                        h_IU_evap_out_i = GetSupHeatEnthalpyRefrig(state,
                                                                   this->RefrigerantName,
                                                                   max(RefTSat, this->IUEvaporatingTemp + DXCoil(CoolCoilIndex).ActualSH),
                                                                   max(min(Pevap, RefPHigh), RefPLow),
                                                                   RefrigerantIndex,
                                                                   RoutineName);

                        if (h_IU_evap_out_i > h_IU_evap_in) {
                            m_ref_IU_evap_i = (TerminalUnitList(TUListNum).TotalCoolLoad(NumTU) <= 0.0)
                                                  ? 0.0
                                                  : (TerminalUnitList(TUListNum).TotalCoolLoad(NumTU) /
                                                     (h_IU_evap_out_i - h_IU_evap_in)); // Ref Flow Rate in the IU( kg/s )
                            m_ref_IU_evap = m_ref_IU_evap + m_ref_IU_evap_i;
                            h_IU_evap_out = h_IU_evap_out + m_ref_IU_evap_i * h_IU_evap_out_i;
                            SH_IU_merged = SH_IU_merged + m_ref_IU_evap_i * DXCoil(CoolCoilIndex).ActualSH;
                        }
                    }
                }
                if (m_ref_IU_evap > 0) {
                    h_IU_evap_out = h_IU_evap_out / m_ref_IU_evap;
                    SH_IU_merged = SH_IU_merged / m_ref_IU_evap;
                } else {
                    RefTSat = GetSatTemperatureRefrig(state, this->RefrigerantName, max(min(Pevap, RefPHigh), RefPLow), RefrigerantIndex, RoutineName);
                    h_IU_evap_out = GetSupHeatEnthalpyRefrig(state,
                                                             this->RefrigerantName,
                                                             max(RefTSat, this->IUEvaporatingTemp + 3),
                                                             max(min(Pevap, RefPHigh), RefPLow),
                                                             RefrigerantIndex,
                                                             RoutineName);
                    SH_IU_merged = 3;
                    m_ref_IU_evap = TU_CoolingLoad / (h_IU_evap_out - h_IU_evap_in);
                }
            }

            // *Calculate piping loss
            this->VRFOU_PipeLossC(
                state, m_ref_IU_evap, max(min(Pevap, RefPHigh), RefPLow), h_IU_evap_out, SH_IU_merged, OutdoorDryBulb, Pipe_Q_c, Pipe_DeltP_c, h_comp_in);
            Tsuction =
                GetSatTemperatureRefrig(state, this->RefrigerantName, max(min(Pevap - Pipe_DeltP_c, RefPHigh), RefPLow), RefrigerantIndex, RoutineName);
            Psuction = Pevap - Pipe_DeltP_c; // This Psuction is used for rps > min; will be updated for rps = min

            // Perform iteration to calculate T_comp_in
            T_comp_in = GetSupHeatTempRefrig(state,
                                             this->RefrigerantName,
                                             max(min(Pevap - Pipe_DeltP_c, RefPHigh), RefPLow),
                                             h_comp_in,
                                             Tsuction + 3,
                                             Tsuction + 30,
                                             RefrigerantIndex,
                                             RoutineName);
            SH_Comp = T_comp_in - Tsuction; // This is used for rps > min; will be updated for rps = min

            Q_c_TU_PL = TU_CoolingLoad + Pipe_Q_c;
            Q_h_OU = Q_c_TU_PL + CompEvaporatingPWRSpdMin;

            // *Calculate capacity modification factor
            C_cap_operation = this->VRFOU_CapModFactor(
                state, h_comp_in, h_IU_evap_in, max(min(Psuction, RefPHigh), RefPLow), Tsuction + SH_Comp, Tsuction + 8, CapMinTc - 5);

            if (Q_c_TU_PL * C_cap_operation < CompEvaporatingCAPSpdMin) {
                // Required cooling load is less than the min cooling capacity, on-off strategy

                this->VRFOperationSimPath = 11;

                CyclingRatio = Q_c_TU_PL * C_cap_operation / CompEvaporatingCAPSpdMin;
                double CyclingRatioFrac = 0.85 + 0.15 * CyclingRatio;
                double HPRTF = CyclingRatio / CyclingRatioFrac;
                Ncomp = CompEvaporatingPWRSpdMin * HPRTF; //
                CompSpdActual = this->CompressorSpeed(1); //
                this->CondensingTemp = CapMinTc;          //

            } else {
                // Required cooling load is greater than or equal to the min cooling capacity

                // Iteration_Ncomp: Perform iterations to calculate Ncomp (Label10)
                Counter = 1;
                Ncomp = TU_CoolingLoad / this->CoolingCOP;
                Ncomp_new = Ncomp;
            Label10:;
                Q_h_OU = Q_c_TU_PL + Ncomp_new; // Ncomp_new may be updated during Iteration_Ncomp Label10

                // *VRF OU TeTc calculations
                m_air = this->OUAirFlowRate * RhoAir;
                SC_OU = this->SC;
                this->VRFOU_TeTc(FlagCondMode, Q_h_OU, SC_OU, m_air, OutdoorDryBulb, OutdoorHumRat, OutdoorPressure, Tfs, this->CondensingTemp);
                this->CondensingTemp = min(CapMaxTc, this->CondensingTemp);
                this->SC = SC_OU;

                // *VEF OU Compressor Simulation at cooling mode: Specify the compressor speed and power consumption
                this->VRFOU_CalcCompC(state,
                                      TU_CoolingLoad,
                                      Tsuction,
                                      this->CondensingTemp,
                                      Psuction,
                                      T_comp_in,
                                      h_comp_in,
                                      h_IU_evap_in,
                                      Pipe_Q_c,
                                      CapMaxTc,
                                      Q_h_OU,
                                      CompSpdActual,
                                      Ncomp);

                if ((std::abs(Ncomp - Ncomp_new) > (Tolerance * Ncomp_new)) && (Counter < 30)) {
                    Ncomp_new = Ncomp;
                    Counter = Counter + 1;
                    goto Label10;
                }
            }

            // Update h_IU_evap_in in iterations Label12
            h_IU_evap_in_new = GetSatEnthalpyRefrig(state, this->RefrigerantName, this->CondensingTemp - this->SC, 0.0, RefrigerantIndex, RoutineName);
            if ((std::abs(h_IU_evap_in - h_IU_evap_in_new) > Tolerance * h_IU_evap_in) && (h_IU_evap_in < h_IU_evap_in_up) &&
                (h_IU_evap_in > h_IU_evap_in_low)) {
                h_IU_evap_in = h_IU_evap_in_new;
                NumIteHIUIn = NumIteHIUIn + 1;
                goto Label12;
            }
            if ((std::abs(h_IU_evap_in - h_IU_evap_in_new) > Tolerance * h_IU_evap_in)) {
                h_IU_evap_in = 0.5 * (h_IU_evap_in_low + h_IU_evap_in_up);
            } else if (h_IU_evap_in > h_IU_evap_in_up) {
                h_IU_evap_in = h_IU_evap_in_up;
            } else if (h_IU_evap_in < h_IU_evap_in_low) {
                h_IU_evap_in = h_IU_evap_in_low;
            } else {
                h_IU_evap_in = (h_IU_evap_in + h_IU_evap_in_new) / 2;
            }

            // Key outputs of this subroutine
            this->CompActSpeed = max(CompSpdActual, 0.0);
            this->Ncomp = max(Ncomp, 0.0) / this->EffCompInverter; // 0.95 is the efficiency of the compressor inverter, can come from IDF //@minor
            this->OUFanPower = this->RatedOUFanPower;              //@ * pow_3( CondFlowRatio )
            this->VRFCondCyclingRatio = CyclingRatio;              // report variable for cycling rate

            Tdischarge = this->CondensingTemp; // outdoor unit condensing temperature
            this->CoolingCapacity =
                this->CoffEvapCap * this->RatedEvapCapacity *
                CurveValue(state, this->OUCoolingCAPFT(NumOfCompSpdInput), Tdischarge, Tsuction); // Include the piping loss, at the highest compressor speed
            this->PipingCorrectionCooling = TU_CoolingLoad / (TU_CoolingLoad + Pipe_Q_c);
            MaxCoolingCapacity(VRFCond) = this->CoolingCapacity; // for report, maximum evaporating capacity of the system

            this->HeatingCapacity = 0.0;         // Include the piping loss
            this->PipingCorrectionHeating = 1.0; // 1 means no piping loss
            MaxHeatingCapacity(VRFCond) = 0.0;

            this->OUCondHeatRate = Q_h_OU;
            this->OUEvapHeatRate = 0;
            this->IUCondHeatRate = 0;
            this->IUEvapHeatRate = TU_CoolingLoad;

            // 2. VRF-HP Heating Mode .OR. VRF-HR Mode_6
        } else if ((!this->HeatRecoveryUsed && HeatingLoad(VRFCond)) || (this->HeatRecoveryUsed && !HRCoolRequestFlag && HRHeatRequestFlag)) {

            this->OperatingMode = ModeHeatingOnly;
            this->VRFOperationSimPath = 60;

            // Initialization of VRF-FluidTCtrl Model
            Q_h_TU_PL = TU_HeatingLoad;
            Ncomp = TU_HeatingLoad / this->HeatingCOP;
            this->CondensingTemp = this->IUCondensingTemp;

            // Evaporative capacity ranges_Max
            CapMaxTe = OutdoorDryBulb - this->SH;
            CompEvaporatingCAPSpdMax =
                this->CoffEvapCap * this->RatedEvapCapacity * CurveValue(state, this->OUCoolingCAPFT(NumOfCompSpdInput), this->IUCondensingTemp, CapMaxTe);
            CompEvaporatingPWRSpdMax = this->RatedCompPower * CurveValue(state, this->OUCoolingPWRFT(NumOfCompSpdInput), this->IUCondensingTemp, CapMaxTe);

            // Initialization of h_comp_out iterations (Label23)
            Pcond = GetSatPressureRefrig(state, this->RefrigerantName, 40.0, RefrigerantIndex, RoutineName);
            RefTSat = GetSatTemperatureRefrig(state, this->RefrigerantName, Pcond, RefrigerantIndex, RoutineName);
            h_IU_cond_in_up = GetSupHeatEnthalpyRefrig(
                state, this->RefrigerantName, max(RefTSat, min(this->IUCondensingTemp + 50, RefTHigh)), Pcond, RefrigerantIndex, RoutineName);
            h_IU_cond_in_low = GetSatEnthalpyRefrig(state, this->RefrigerantName, this->IUCondensingTemp, 1.0, RefrigerantIndex, RoutineName); // Quality=1
            h_IU_cond_in = h_IU_cond_in_low;

        Label23:;
            m_ref_IU_cond = 0;
            h_IU_cond_out_ave = 0;
            SC_IU_merged = 0;

            // Calculate total refrigerant flow rate
            if (Q_h_TU_PL > CompEvaporatingCAPSpdMax + CompEvaporatingPWRSpdMax) {
                // Required load is beyond the max system capacity

                Q_h_TU_PL = CompEvaporatingCAPSpdMax;
                TU_HeatingLoad = CompEvaporatingCAPSpdMax;
                this->TUHeatingLoad = TU_HeatingLoad;
                h_IU_cond_out = GetSatEnthalpyRefrig(state,
                                                     this->RefrigerantName,
                                                     GetSatTemperatureRefrig(state, this->RefrigerantName, max(min(Pcond, RefPHigh), RefPLow), RefrigerantIndex, RoutineName) - 5.0,
                                                     0.0,
                                                     RefrigerantIndex,
                                                     RoutineName); // Quality=0
                h_IU_cond_out_ave = h_IU_cond_out;
                SC_IU_merged = 5;
                m_ref_IU_cond = TU_HeatingLoad / (h_IU_cond_in - h_IU_cond_out);

            } else {
                for (NumTU = 1; NumTU <= NumTUInList; NumTU++) {
                    if (TerminalUnitList(TUListNum).TotalHeatLoad(NumTU) > 0) {
                        TUIndex = TerminalUnitList(TUListNum).ZoneTUPtr(NumTU);
                        HeatCoilIndex = VRFTU(TUIndex).HeatCoilIndex;
                        h_IU_cond_out_i = GetSatEnthalpyRefrig(state,
                                                               this->RefrigerantName,
                                                               GetSatTemperatureRefrig(state, this->RefrigerantName, max(min(Pcond, RefPHigh), RefPLow), RefrigerantIndex, RoutineName) -
                                                                   DXCoil(HeatCoilIndex).ActualSC,
                                                               0.0,
                                                               RefrigerantIndex,
                                                               RoutineName); // Quality=0
                        m_ref_IU_cond_i = (TerminalUnitList(TUListNum).TotalHeatLoad(NumTU) <= 0.0)
                                              ? 0.0
                                              : (TerminalUnitList(TUListNum).TotalHeatLoad(NumTU) / (h_IU_cond_in - h_IU_cond_out_i));
                        m_ref_IU_cond = m_ref_IU_cond + m_ref_IU_cond_i;
                        h_IU_cond_out_ave = h_IU_cond_out_ave + m_ref_IU_cond_i * h_IU_cond_out_i;
                        SC_IU_merged = SC_IU_merged + m_ref_IU_cond_i * DXCoil(HeatCoilIndex).ActualSC;
                    }
                }
                if (m_ref_IU_cond > 0) {
                    h_IU_cond_out_ave = h_IU_cond_out_ave / m_ref_IU_cond; // h_merge
                    SC_IU_merged = SC_IU_merged / m_ref_IU_cond;
                } else {
                    h_IU_cond_out_ave = GetSatEnthalpyRefrig(state,
                                                             this->RefrigerantName,
                                                             GetSatTemperatureRefrig(state, this->RefrigerantName, max(min(Pcond, RefPHigh), RefPLow), RefrigerantIndex, RoutineName) - 5.0,
                                                             0.0,
                                                             RefrigerantIndex,
                                                             RoutineName); // Quality=0
                    SC_IU_merged = 5;
                    m_ref_IU_cond = TU_HeatingLoad / (h_IU_cond_in - h_IU_cond_out_ave);
                }
            }

            // *Calculate piping loss
            this->VRFOU_PipeLossH(
                state, m_ref_IU_cond, max(min(Pcond, RefPHigh), RefPLow), h_IU_cond_in, OutdoorDryBulb, Pipe_Q_h, Pipe_DeltP_h, h_comp_out);

            Pdischarge = max(Pcond + Pipe_DeltP_h, Pcond); // affected by piping loss
            Tdischarge = GetSatTemperatureRefrig(state, this->RefrigerantName, max(min(Pdischarge, RefPHigh), RefPLow), RefrigerantIndex, RoutineName);

            // Evaporative capacity ranges_Min
            CapMinPe = min(Pdischarge - this->CompMaxDeltaP, RefMinPe);
            CapMinTe = GetSatTemperatureRefrig(state, this->RefrigerantName, max(min(CapMinPe, RefPHigh), RefPLow), RefrigerantIndex, RoutineName);
            CompEvaporatingCAPSpdMin = this->CoffEvapCap * this->RatedEvapCapacity * CurveValue(state, this->OUCoolingCAPFT(1), Tdischarge, CapMinTe);
            CompEvaporatingPWRSpdMin = this->RatedCompPower * CurveValue(state, this->OUCoolingPWRFT(1), Tdischarge, CapMinTe);

            Q_h_TU_PL = TU_HeatingLoad + Pipe_Q_h;
            Q_c_OU = max(0.0, Q_h_TU_PL - CompEvaporatingPWRSpdMin);

            // *Calculate capacity modification factor
            RefTSat = GetSatTemperatureRefrig(state, this->RefrigerantName, max(min(CapMinPe, RefPHigh), RefPLow), RefrigerantIndex, RoutineName);
            h_comp_in = GetSupHeatEnthalpyRefrig(
                state, this->RefrigerantName, max(RefTSat, CapMinTe + this->SH), max(min(CapMinPe, RefPHigh), RefPLow), RefrigerantIndex, RoutineName);
            C_cap_operation = this->VRFOU_CapModFactor(
                state, h_comp_in, h_IU_cond_out_ave, max(min(CapMinPe, RefPHigh), RefPLow), CapMinTe + this->SH, CapMinTe + 8, this->IUCondensingTemp - 5);

            if ((Q_c_OU * C_cap_operation) <= CompEvaporatingCAPSpdMin) {
                // Required heating load is smaller than the min heating capacity

                if (Q_c_OU == 0) {
                    // Q_h_TU_PL is less than or equal to CompEvaporatingPWRSpdMin
                    CyclingRatio = Q_h_TU_PL / CompEvaporatingPWRSpdMin;
                    this->EvaporatingTemp = OutdoorDryBulb;
                } else {
                    // Q_h_TU_PL is greater than CompEvaporatingPWRSpdMin
                    CyclingRatio = Q_c_OU * C_cap_operation / CompEvaporatingCAPSpdMin;
                    this->EvaporatingTemp = max(CapMinTe, RefTLow);
                }

                double CyclingRatioFrac = 0.85 + 0.15 * CyclingRatio;
                double HPRTF = CyclingRatio / CyclingRatioFrac;
                Ncomp = CompEvaporatingPWRSpdMin * HPRTF;
                CompSpdActual = this->CompressorSpeed(1);

            } else {
                // Required heating load is greater than or equal to the min heating capacity

                // Iteration_Ncomp: Perform iterations to calculate Ncomp (Label20)
                Counter = 1;
            Label20:;
                Ncomp_new = Ncomp;
                Q_c_OU = max(0.0, Q_h_TU_PL - Ncomp);

                // *VRF OU Te calculations
                m_air = this->OUAirFlowRate * RhoAir;
                SH_OU = this->SH;
                this->VRFOU_TeTc(FlagEvapMode, Q_c_OU, SH_OU, m_air, OutdoorDryBulb, OutdoorHumRat, OutdoorPressure, Tfs, this->EvaporatingTemp);
                this->SH = SH_OU;

                // *VRF OU Compressor Simulation at heating mode: Specify the compressor speed and power consumption
                this->VRFOU_CalcCompH(state,
                                      TU_HeatingLoad,
                                      this->EvaporatingTemp,
                                      Tdischarge,
                                      h_IU_cond_out_ave,
                                      this->IUCondensingTemp,
                                      CapMinTe,
                                      Tfs,
                                      Pipe_Q_h,
                                      Q_c_OU,
                                      CompSpdActual,
                                      Ncomp_new);

                if ((std::abs(Ncomp_new - Ncomp) > (Tolerance * Ncomp)) && (Counter < 30)) {
                    Ncomp = Ncomp_new;
                    Counter = Counter + 1;
                    goto Label20;
                }

                // Update h_comp_out in iteration Label23
                P_comp_in = GetSatPressureRefrig(state, this->RefrigerantName, this->EvaporatingTemp, RefrigerantIndex, RoutineName);
                RefTSat = GetSatTemperatureRefrig(state, this->RefrigerantName, max(min(P_comp_in, RefPHigh), RefPLow), RefrigerantIndex, RoutineName);
                h_comp_in_new = GetSupHeatEnthalpyRefrig(state,
                                                         this->RefrigerantName,
                                                         max(RefTSat, this->SH + this->EvaporatingTemp),
                                                         max(min(P_comp_in, RefPHigh), RefPLow),
                                                         RefrigerantIndex,
                                                         RoutineName);
                h_comp_out_new = Ncomp_new / m_ref_IU_cond + h_comp_in_new;

                if ((std::abs(h_comp_out - h_comp_out_new) > Tolerance * h_comp_out) && (h_IU_cond_in < h_IU_cond_in_up)) {
                    h_IU_cond_in = h_IU_cond_in + 0.1 * (h_IU_cond_in_up - h_IU_cond_in_low);
                    goto Label23;
                }
                if (h_IU_cond_in > h_IU_cond_in_up) {
                    h_IU_cond_in = 0.5 * (h_IU_cond_in_up + h_IU_cond_in_low);
                }
                Ncomp = Ncomp_new;
            }

            // Key outputs of this subroutine
            this->CompActSpeed = max(CompSpdActual, 0.0);
            this->Ncomp = max(Ncomp, 0.0) / this->EffCompInverter;
            this->OUFanPower = this->RatedOUFanPower;
            this->VRFCondCyclingRatio = CyclingRatio;

            Tsuction = this->EvaporatingTemp; // Outdoor unit evaporating temperature
            this->HeatingCapacity =
                this->CoffEvapCap * this->RatedEvapCapacity * CurveValue(state, this->OUCoolingCAPFT(NumOfCompSpdInput), Tdischarge, Tsuction) +
                this->RatedCompPower * CurveValue(state, this->OUCoolingPWRFT(NumOfCompSpdInput),
                                                  Tdischarge,
                                                  Tsuction); // Include the piping loss, at the highest compressor speed
            this->PipingCorrectionHeating = TU_HeatingLoad / (TU_HeatingLoad + Pipe_Q_h);
            MaxHeatingCapacity(VRFCond) = this->HeatingCapacity; // for report, maximum condensing capacity the system can provide

            this->CoolingCapacity = 0.0; // Include the piping loss
            this->PipingCorrectionCooling = 0.0;
            MaxCoolingCapacity(VRFCond) = 0.0; // for report

            this->OUCondHeatRate = 0;
            this->OUEvapHeatRate = Q_c_OU;
            this->IUCondHeatRate = TU_HeatingLoad;
            this->IUEvapHeatRate = 0;

            // 3. VRF-HR Mode_2-5, Simultaneous Heating and Cooling
        } else if (this->HeatRecoveryUsed && HRCoolRequestFlag && HRHeatRequestFlag) {

            this->OperatingMode = ModeCoolingAndHeating;

            // Initialization of VRF-FluidTCtrl Model
            Q_c_TU_PL = TU_CoolingLoad;
            Q_h_TU_PL = TU_HeatingLoad;

            // Evaporator (IU side) operational parameters
            Pevap = GetSatPressureRefrig(state, this->RefrigerantName, this->IUEvaporatingTemp, RefrigerantIndex, RoutineName);
            Psuction = Pevap;
            Tsuction = this->IUEvaporatingTemp;
            this->EvaporatingTemp = this->IUEvaporatingTemp;

            // Condenser (OU side) operation ranges
            CapMaxPc = min(Psuction + this->CompMaxDeltaP, RefMaxPc);
            CapMaxTc = GetSatTemperatureRefrig(state, this->RefrigerantName, max(min(CapMaxPc, RefPHigh), RefPLow), RefrigerantIndex, RoutineName);
            CapMinTc = OutdoorDryBulb + this->SC;
            CapMinPc = GetSatPressureRefrig(state, this->RefrigerantName, CapMinTc, RefrigerantIndex, RoutineName);

            // Evaporator (IU side) operation ranges
            CapMinPe = max(CapMinPc - this->CompMaxDeltaP, RefMinPe);
            CapMinTe = GetSatTemperatureRefrig(state, this->RefrigerantName, max(min(CapMinPe, RefPHigh), RefPLow), RefrigerantIndex, RoutineName);

            //===**h_comp_out Iteration Starts

            // Initialization of h_comp_out iterations (Label230)
            {
                Pcond = GetSatPressureRefrig(state, this->RefrigerantName, this->IUCondensingTemp, RefrigerantIndex, RoutineName);
                Real64 Pcond_temp = GetSatPressureRefrig(state, this->RefrigerantName, 40.0, RefrigerantIndex, RoutineName);
                RefTSat = GetSatTemperatureRefrig(state, this->RefrigerantName, Pcond_temp, RefrigerantIndex, RoutineName);
                h_IU_cond_in_up = GetSupHeatEnthalpyRefrig(
                    state, this->RefrigerantName, max(RefTSat, min(this->IUCondensingTemp + 50, RefTHigh)), Pcond_temp, RefrigerantIndex, RoutineName);
                h_IU_cond_in_low =
                    GetSatEnthalpyRefrig(state, this->RefrigerantName, this->IUCondensingTemp, 1.0, RefrigerantIndex, RoutineName); // Quality=1
                h_IU_cond_in = h_IU_cond_in_low;
            }

        Label230:;

            // *PL-h: Calculate total refrigerant flow rate
            m_ref_IU_cond = 0;
            h_IU_cond_out_ave = 0;
            SC_IU_merged = 0;
            for (NumTU = 1; NumTU <= NumTUInList; NumTU++) {
                if (TerminalUnitList(TUListNum).TotalHeatLoad(NumTU) > 0) {
                    TUIndex = TerminalUnitList(TUListNum).ZoneTUPtr(NumTU);
                    HeatCoilIndex = VRFTU(TUIndex).HeatCoilIndex;
                    h_IU_cond_out_i = GetSatEnthalpyRefrig(state,
                        this->RefrigerantName,
                        GetSatTemperatureRefrig(state, this->RefrigerantName, max(min(Pcond, RefPHigh), RefPLow), RefrigerantIndex, RoutineName) -
                            DXCoil(HeatCoilIndex).ActualSC,
                        0.0,
                        RefrigerantIndex,
                        RoutineName); // Quality=0
                    m_ref_IU_cond_i = (TerminalUnitList(TUListNum).TotalHeatLoad(NumTU) <= 0.0)
                                          ? 0.0
                                          : (TerminalUnitList(TUListNum).TotalHeatLoad(NumTU) / (h_IU_cond_in - h_IU_cond_out_i));
                    m_ref_IU_cond = m_ref_IU_cond + m_ref_IU_cond_i;
                    h_IU_cond_out_ave = h_IU_cond_out_ave + m_ref_IU_cond_i * h_IU_cond_out_i;
                    SC_IU_merged = SC_IU_merged + m_ref_IU_cond_i * DXCoil(HeatCoilIndex).ActualSC;
                }
            }
            if (m_ref_IU_cond > 0) {
                h_IU_cond_out_ave = h_IU_cond_out_ave / m_ref_IU_cond;
                SC_IU_merged = SC_IU_merged / m_ref_IU_cond;
            } else {
                h_IU_cond_out_ave = GetSatEnthalpyRefrig(state,
                    this->RefrigerantName,
                    GetSatTemperatureRefrig(state, this->RefrigerantName, max(min(Pcond, RefPHigh), RefPLow), RefrigerantIndex, RoutineName) - 5.0,
                    0.0,
                    RefrigerantIndex,
                    RoutineName); // Quality=0
                SC_IU_merged = 5;
                m_ref_IU_cond = TU_HeatingLoad / (h_IU_cond_in - h_IU_cond_out_ave);
            }

            // *PL-h: Calculate piping loss
            this->VRFOU_PipeLossH(
                state, m_ref_IU_cond, max(min(Pcond, RefPHigh), RefPLow), h_IU_cond_in, OutdoorDryBulb, Pipe_Q_h, Pipe_DeltP_h, h_comp_out);
            Pdischarge = max(Pcond + Pipe_DeltP_h, Pcond); // affected by piping loss
            Tdischarge = GetSatTemperatureRefrig(state, this->RefrigerantName, max(min(Pdischarge, RefPHigh), RefPLow), RefrigerantIndex, RoutineName);
            Q_h_TU_PL = TU_HeatingLoad + Pipe_Q_h;

            // *PL-c: Calculate total IU refrigerant flow rate and SH_IU_merged
            h_IU_evap_in = h_IU_cond_out_ave;
            m_ref_IU_evap = 0;
            h_IU_evap_out = 0;
            SH_IU_merged = 0;
            for (NumTU = 1; NumTU <= NumTUInList; NumTU++) { // Calc total refrigerant flow rate
                if (TerminalUnitList(TUListNum).TotalCoolLoad(NumTU) > 0) {
                    TUIndex = TerminalUnitList(TUListNum).ZoneTUPtr(NumTU);
                    CoolCoilIndex = VRFTU(TUIndex).CoolCoilIndex;

                    RefTSat = GetSatTemperatureRefrig(state, this->RefrigerantName, max(min(Pevap, RefPHigh), RefPLow), RefrigerantIndex, RoutineName);
                    h_IU_evap_out_i = GetSupHeatEnthalpyRefrig(state,
                                                               this->RefrigerantName,
                                                               max(RefTSat, this->IUEvaporatingTemp + DXCoil(CoolCoilIndex).ActualSH),
                                                               max(min(Pevap, RefPHigh), RefPLow),
                                                               RefrigerantIndex,
                                                               RoutineName);

                    if (h_IU_evap_out_i > h_IU_evap_in) {
                        m_ref_IU_evap_i = (TerminalUnitList(TUListNum).TotalCoolLoad(NumTU) <= 0.0)
                                              ? 0.0
                                              : (TerminalUnitList(TUListNum).TotalCoolLoad(NumTU) /
                                                 (h_IU_evap_out_i - h_IU_evap_in)); // Ref Flow Rate in the IU( kg/s )
                        m_ref_IU_evap = m_ref_IU_evap + m_ref_IU_evap_i;
                        h_IU_evap_out = h_IU_evap_out + m_ref_IU_evap_i * h_IU_evap_out_i;
                        SH_IU_merged = SH_IU_merged + m_ref_IU_evap_i * DXCoil(CoolCoilIndex).ActualSH;
                    }
                }
            }
            if (m_ref_IU_evap > 0) {
                h_IU_evap_out = h_IU_evap_out / m_ref_IU_evap;
                SH_IU_merged = SH_IU_merged / m_ref_IU_evap;
            } else {
                RefTSat = GetSatTemperatureRefrig(state, this->RefrigerantName, max(min(Pevap, RefPHigh), RefPLow), RefrigerantIndex, RoutineName);
                h_IU_evap_out = GetSupHeatEnthalpyRefrig(state,
                                                         this->RefrigerantName,
                                                         max(RefTSat, this->IUEvaporatingTemp + 3),
                                                         max(min(Pevap, RefPHigh), RefPLow),
                                                         RefrigerantIndex,
                                                         RoutineName);
                SH_IU_merged = 3;
                m_ref_IU_evap = TU_CoolingLoad / (h_IU_evap_out - h_IU_evap_in);
            }

            // *PL-c: Calculate piping loss
            this->VRFOU_PipeLossC(
                state, m_ref_IU_evap, max(min(Pevap, RefPHigh), RefPLow), h_IU_evap_out, SH_IU_merged, OutdoorDryBulb, Pipe_Q_c, Pipe_DeltP_c, h_IU_PLc_out);
            Psuction = min(Pevap - Pipe_DeltP_c, Pevap); // This Psuction is used for rps > min; will be updated for rps = min
            Tsuction = GetSatTemperatureRefrig(state, this->RefrigerantName, max(min(Psuction, RefPHigh), RefPLow), RefrigerantIndex, RoutineName);
            h_comp_in = h_IU_PLc_out;
            Q_c_TU_PL = TU_CoolingLoad + Pipe_Q_c;

            //**OU operations: Determine VRF-HR OU system operational mode
            //  Determine the operational mode of the VRF-HR system, given the terminal unit side load conditions.
            //  A number of OU side operational parameters are also calculated here, including:
            //  (1) OU evaporator load Q_c_OU, (2) OU condenser load Q_h_OU,
            //  (3) m_ref_OU_evap, (4) m_ref_OU_cond
            //  Note that Te and Te' may be updated here, and thus IU evaporator side piping loss recalculations.
            //  Then a number of operational parameters need to be updated, including:
            //  (1) IU evaporating temperature Te (2) OU evaporating temperature Te' etc (3) m_ref_IU_evap
            //  (4) Pipe_Q_c (5) h_IU_PLc_out (6) h_comp_in
            //*VRF OU Compressor Simulation at HR mode: Specify the compressor speed and power consumption
            {
                Real64 Pipe_Q_c_new = Pipe_Q_c;
                Real64 Tsuction_new = Tsuction;
                Real64 Te_new = this->IUEvaporatingTemp;
                Real64 N_fan_OU;

                this->VRFHR_OU_HR_Mode(state,
                                       h_IU_evap_in,
                                       h_comp_out,
                                       Q_c_TU_PL,
                                       Q_h_TU_PL,
                                       Tdischarge,
                                       Tsuction_new,
                                       Te_new,
                                       h_comp_in,
                                       h_IU_PLc_out,
                                       Pipe_Q_c_new,
                                       Q_c_OU,
                                       Q_h_OU,
                                       m_ref_IU_evap,
                                       m_ref_OU_evap,
                                       m_ref_OU_cond,
                                       N_fan_OU,
                                       CompSpdActual,
                                       Ncomp);

                // parameter update
                Tsuction = Tsuction_new;
                Pipe_Q_c = Pipe_Q_c_new;
                this->OUFanPower = N_fan_OU;
                this->IUEvaporatingTemp = Te_new;
            }

            //* Update h_comp_out in iteration (Label230)
            h_comp_out_new = Ncomp / (m_ref_IU_evap + m_ref_OU_evap) + h_comp_in;

            if ((std::abs(h_comp_out - h_comp_out_new) > Tolerance * h_comp_out) && (h_IU_cond_in < h_IU_cond_in_up)) {
                h_IU_cond_in = h_IU_cond_in + 0.1 * (h_IU_cond_in_up - h_IU_cond_in_low);
                goto Label230;
            }
            if (h_IU_cond_in > h_IU_cond_in_up) {
                h_IU_cond_in = 0.5 * (h_IU_cond_in_up + h_IU_cond_in_low);
            }

            //===**h_comp_out Iteration Ends (Label230)

            // Key outputs of this subroutine
            this->CompActSpeed = max(CompSpdActual, 0.0);
            this->Ncomp = max(Ncomp, 0.0) / this->EffCompInverter;
            this->VRFCondCyclingRatio = 1.0;

            this->HeatingCapacity =
                this->CoffEvapCap * this->RatedEvapCapacity * CurveValue(state, this->OUCoolingCAPFT(NumOfCompSpdInput), Tdischarge, Tsuction) +
                this->RatedCompPower * CurveValue(state, this->OUCoolingPWRFT(NumOfCompSpdInput), Tdischarge, Tsuction); // Include the piping loss
            MaxHeatingCapacity(VRFCond) =
                this->HeatingCapacity; // for report, maximum heating capacity of the system, at the highest compressor speed
            this->PipingCorrectionHeating = TU_HeatingLoad / Q_h_TU_PL;

            this->CoolingCapacity =
                this->CoffEvapCap * this->RatedEvapCapacity * CurveValue(state, this->OUCoolingCAPFT(NumOfCompSpdInput), Tdischarge, Tsuction);
            MaxCoolingCapacity(VRFCond) =
                this->CoolingCapacity; // for report, maximum evaporating capacity of the system, at the highest compressor speed
            this->PipingCorrectionCooling = TU_CoolingLoad / Q_c_TU_PL;

            this->CondensingTemp = Tdischarge; // OU condensing temperature
            this->EvaporatingTemp = Tsuction;  // OU evaporating temperature

            this->OUCondHeatRate = Q_h_OU;
            this->OUEvapHeatRate = Q_c_OU;
            this->IUCondHeatRate = TU_HeatingLoad;
            this->IUEvapHeatRate = TU_CoolingLoad;

            // 4. Stop running
        } else {

            this->OperatingMode = 0;
            this->VRFOperationSimPath = 0;

            this->Ncomp = 0.0;
            this->CompActSpeed = 0.0;
            this->OUFanPower = 0.0;
            this->VRFCondCyclingRatio = 0.0;

            this->HeatingCapacity = 0.0;         // Include the piping loss
            this->PipingCorrectionHeating = 1.0; // 1 means no piping loss
            MaxHeatingCapacity(VRFCond) = 0.0;

            this->CoolingCapacity = 0.0; // Include the piping loss
            this->PipingCorrectionCooling = 0.0;
            MaxCoolingCapacity(VRFCond) = 0.0; // for report

            this->CondensingTemp = OutDryBulbTemp;
            this->EvaporatingTemp = OutDryBulbTemp;

            this->OUCondHeatRate = 0.0;
            this->OUEvapHeatRate = 0.0;
            this->IUCondHeatRate = 0.0;
            this->IUEvapHeatRate = 0.0;
        }

        // calculate capacities and energy use
        if (((!this->HeatRecoveryUsed && CoolingLoad(VRFCond)) || (this->HeatRecoveryUsed && HRCoolRequestFlag)) &&
            TerminalUnitList(TUListNum).CoolingCoilPresent(NumTUInList)) {
            InletAirWetBulbC = SumCoolInletWB;

            // From the VRF_FluidTCtrl model
            TotalCondCoolingCapacity = this->CoolingCapacity;
            TotalTUCoolingCapacity = TotalCondCoolingCapacity * this->PipingCorrectionCooling;

            if (TotalCondCoolingCapacity > 0.0) {
                CoolingPLR = min(1.0, (this->TUCoolingLoad / this->PipingCorrectionCooling) / TotalCondCoolingCapacity);
            } else {
                CoolingPLR = 0.0;
            }
        }
        if (((!this->HeatRecoveryUsed && HeatingLoad(VRFCond)) || (this->HeatRecoveryUsed && HRHeatRequestFlag)) &&
            TerminalUnitList(TUListNum).HeatingCoilPresent(NumTUInList)) {
            InletAirDryBulbC = SumHeatInletDB;
            InletAirWetBulbC = SumHeatInletWB;

            // Initializing defrost adjustment factors
            LoadDueToDefrost = 0.0;
            HeatingCapacityMultiplier = 1.0;
            FractionalDefrostTime = 0.0;
            InputPowerMultiplier = 1.0;

            // Check outdoor temperature to determine of defrost is active
            if (OutdoorDryBulb <= this->MaxOATDefrost && this->CondenserType != DataHVACGlobals::WaterCooled) {

                // Calculating adjustment factors for defrost
                // Calculate delta w through outdoor coil by assuming a coil temp of 0.82*DBT-9.7(F) per DOE2.1E
                OutdoorCoilT = 0.82 * OutdoorDryBulb - 8.589;
                OutdoorCoildw = max(1.0e-6, (OutdoorHumRat - PsyWFnTdpPb(OutdoorCoilT, OutdoorPressure)));

                // Calculate defrost adjustment factors depending on defrost control type
                if (this->DefrostControl == Timed) {
                    FractionalDefrostTime = this->DefrostFraction;
                    if (FractionalDefrostTime > 0.0) {
                        HeatingCapacityMultiplier = 0.909 - 107.33 * OutdoorCoildw;
                        InputPowerMultiplier = 0.90 - 36.45 * OutdoorCoildw;
                    }
                } else { // else defrost control is on-demand
                    FractionalDefrostTime = 1.0 / (1.0 + 0.01446 / OutdoorCoildw);
                    HeatingCapacityMultiplier = 0.875 * (1.0 - FractionalDefrostTime);
                    InputPowerMultiplier = 0.954 * (1.0 - FractionalDefrostTime);
                }

                if (FractionalDefrostTime > 0.0) {
                    // Calculate defrost adjustment factors depending on defrost control strategy
                    if (this->DefrostStrategy == ReverseCycle && this->DefrostControl == OnDemand) {
                        LoadDueToDefrost = (0.01 * FractionalDefrostTime) * (7.222 - OutdoorDryBulb) * (this->HeatingCapacity / 1.01667);
                        DefrostEIRTempModFac = CurveValue(state, this->DefrostEIRPtr, max(15.555, InletAirWetBulbC), max(15.555, OutdoorDryBulb));

                        //         Warn user if curve output goes negative
                        if (DefrostEIRTempModFac < 0.0) {
                            if (!WarmupFlag) {
                                if (this->DefrostHeatErrorIndex == 0) {
                                    ShowSevereMessage(cVRFTypes(VRF_HeatPump) + " \"" + this->Name + "\":");
                                    ShowContinueError(" Defrost Energy Input Ratio Modifier curve (function of temperature) output is negative (" +
                                                      TrimSigDigits(DefrostEIRTempModFac, 3) + ").");
                                    ShowContinueError(" Negative value occurs using an outdoor air dry-bulb temperature of " +
                                                      TrimSigDigits(OutdoorDryBulb, 1) + " C and an average indoor air wet-bulb temperature of " +
                                                      TrimSigDigits(InletAirWetBulbC, 1) + " C.");
                                    ShowContinueErrorTimeStamp(" Resetting curve output to zero and continuing simulation.");
                                }
                                ShowRecurringWarningErrorAtEnd(ccSimPlantEquipTypes(TypeOf_HeatPumpVRF) + " \"" + this->Name +
                                                                   "\": Defrost Energy Input Ratio Modifier curve (function of temperature) output "
                                                                   "is negative warning continues...",
                                                               this->DefrostHeatErrorIndex,
                                                               DefrostEIRTempModFac,
                                                               DefrostEIRTempModFac);
                                DefrostEIRTempModFac = 0.0;
                            }
                        }

                        this->DefrostPower = DefrostEIRTempModFac * (this->HeatingCapacity / 1.01667) * FractionalDefrostTime;

                    } else { // Defrost strategy is resistive
                        this->DefrostPower = this->DefrostCapacity * FractionalDefrostTime;
                    }
                } else { // Defrost is not active because FractionalDefrostTime = 0.0
                    this->DefrostPower = 0.0;
                }
            }

            // From the VRF_FluidTCtrl model
            TotalCondHeatingCapacity = this->HeatingCapacity;
            TotalTUHeatingCapacity = TotalCondHeatingCapacity * this->PipingCorrectionHeating;

            if (TotalCondHeatingCapacity > 0.0) {
                HeatingPLR = min(1.0, (this->TUHeatingLoad / this->PipingCorrectionHeating) / TotalCondHeatingCapacity);
                HeatingPLR += (LoadDueToDefrost * HeatingPLR) / TotalCondHeatingCapacity;
            } else {
                HeatingPLR = 0.0;
            }
        }

        this->VRFCondPLR = max(CoolingPLR, HeatingPLR);

        // For VRF-HR Operations
        HRInitialCapFrac = 1.0;
        HRInitialEIRFrac = 1.0;
        HRCapTC = 0.0;
        HREIRTC = 0.0;
        if (!DoingSizing && !WarmupFlag) {
            if (HRHeatRequestFlag && HRCoolRequestFlag) { // Simultaneous Heating and Cooling operations for HR system
                // determine operating mode change: (1) ModeChange (2) HRCoolingActive (3) HRHeatingActive
                if (!this->HRCoolingActive && !this->HRHeatingActive) {
                    this->ModeChange = true;
                }
                if (CoolingLoad(VRFCond)) {
                    if (this->HRHeatingActive && !this->HRCoolingActive) {
                        this->HRModeChange = true;
                    }
                    this->HRCoolingActive = true;
                    this->HRHeatingActive = false;

                    HRInitialCapFrac = this->HRInitialCoolCapFrac; // Fractional cooling degradation at the start of heat recovery from cooling mode
                    HRCapTC = this->HRCoolCapTC;                   // Time constant used to recover from initial degradation in cooling heat recovery

                    HRInitialEIRFrac = this->HRInitialCoolEIRFrac; // Fractional cooling degradation at the start of heat recovery from cooling mode
                    HREIRTC = this->HRCoolEIRTC;                   // Time constant used to recover from initial degradation in cooling heat recovery

                } else if (HeatingLoad(VRFCond)) {
                    if (!this->HRHeatingActive && this->HRCoolingActive) {
                        this->HRModeChange = true;
                    }
                    this->HRCoolingActive = false;
                    this->HRHeatingActive = true;

                    HRInitialCapFrac = this->HRInitialHeatCapFrac; // Fractional heating degradation at the start of heat recovery from cooling mode
                    HRCapTC = this->HRHeatCapTC;                   // Time constant used to recover from initial degradation in heating heat recovery

                    HRInitialEIRFrac = this->HRInitialHeatEIRFrac; // Fractional heating degradation at the start of heat recovery from heating mode
                    HREIRTC = this->HRHeatEIRTC;                   // Time constant used to recover from initial degradation in heating heat recovery

                } else {
                    // zone thermostats satisfied, condenser is off. Set values anyway
                    // HRCAPFTConst = 1.0;
                    HRInitialCapFrac = 1.0;
                    HRCapTC = 1.0;
                    // HREIRFTConst = 1.0;
                    HRInitialEIRFrac = 1.0;
                    HREIRTC = 1.0;
                    if (this->HRHeatingActive || this->HRCoolingActive) {
                        this->HRModeChange = true;
                    }
                    this->HRCoolingActive = false;
                    this->HRHeatingActive = false;
                }

            } else { // IF(HRHeatRequestFlag .AND. HRCoolRequestFlag)THEN -- Heat recovery turned off
                HRInitialCapFrac = 1.0;
                HRCapTC = 0.0;
                HRInitialEIRFrac = 1.0;
                HREIRTC = 0.0;
                this->HRModeChange = false;
                this->HRCoolingActive = false;
                this->HRHeatingActive = false;
            }

            // Calculate the capacity modification factor (SUMultiplier) for the HR mode transition period
            {
                // calculate end time of current time step to determine if max capacity reset is required
                CurrentEndTime = double((DayOfSim - 1) * 24) + CurrentTime - TimeStepZone + DataHVACGlobals::SysTimeElapsed;

                if (this->ModeChange || this->HRModeChange) {
                    if (this->HRCoolingActive && this->HRTimer == 0.0) {
                        this->HRTimer = CurrentEndTimeLast;
                    } else if (this->HRHeatingActive && this->HRTimer == 0.0) {
                        this->HRTimer = CurrentEndTimeLast;
                    } else if (!this->HRCoolingActive && !this->HRHeatingActive) {
                        this->HRTimer = 0.0;
                    }
                }

                this->HRTime = max(0.0, CurrentEndTime - this->HRTimer);
                if (this->HRTime < (HRCapTC * 5.0)) {
                    if (HRCapTC > 0.0) {
                        SUMultiplier = min(1.0, 1.0 - std::exp(-this->HRTime / HRCapTC));
                    } else {
                        SUMultiplier = 1.0;
                    }
                } else {
                    SUMultiplier = 1.0;
                    this->ModeChange = false;
                    this->HRModeChange = false;
                }
                this->SUMultiplier = SUMultiplier;

                TimeStepSysLast = DataHVACGlobals::TimeStepSys;
                CurrentEndTimeLast = CurrentEndTime;
            }

            // Modify HR capacity for the transition period
            {
                if (this->HeatRecoveryUsed && this->HRCoolingActive) {
                    TotalCondCoolingCapacity =
                        HRInitialCapFrac * TotalCondCoolingCapacity + (1.0 - HRInitialCapFrac) * TotalCondCoolingCapacity * SUMultiplier;
                    TotalTUCoolingCapacity = TotalCondCoolingCapacity * this->PipingCorrectionCooling;
                    if (TotalCondCoolingCapacity > 0.0) {
                        CoolingPLR = min(1.0, (this->TUCoolingLoad / this->PipingCorrectionCooling) / TotalCondCoolingCapacity);
                    } else {
                        CoolingPLR = 0.0;
                    }
                    this->VRFHeatRec = this->TUHeatingLoad;
                } else if (this->HeatRecoveryUsed && this->HRHeatingActive) {
                    TotalCondHeatingCapacity =
                        HRInitialCapFrac * TotalCondHeatingCapacity + (1.0 - HRInitialCapFrac) * TotalCondHeatingCapacity * SUMultiplier;
                    TotalTUHeatingCapacity = TotalCondHeatingCapacity * this->PipingCorrectionHeating;
                    if (TotalCondHeatingCapacity > 0.0) {
                        HeatingPLR = min(1.0, (this->TUHeatingLoad / this->PipingCorrectionHeating) / TotalCondHeatingCapacity);
                    } else {
                        HeatingPLR = 0.0;
                    }
                    this->VRFHeatRec = this->TUCoolingLoad;
                }

                this->VRFCondPLR = max(CoolingPLR, HeatingPLR);
            }
        }

        this->TotalCoolingCapacity = TotalCondCoolingCapacity * CoolingPLR;
        this->TotalHeatingCapacity = TotalCondHeatingCapacity * HeatingPLR;

        if (this->MinPLR > 0.0) {
            if (this->VRFCondPLR < this->MinPLR && this->VRFCondPLR > 0.0) {
                this->VRFCondPLR = this->MinPLR;
            }
        }

        VRFRTF = 0.0;
        // VRF Cooling and Heating Electric Power (output variables)
        if (this->OperatingMode == ModeCoolingOnly) {
            PartLoadFraction = 1.0;
            VRFRTF = min(1.0, (CyclingRatio / PartLoadFraction));

            this->ElecCoolingPower = VRF(VRFCond).Ncomp + this->OUFanPower;
            this->ElecHeatingPower = 0;

        } else if (this->OperatingMode == ModeHeatingOnly) {
            PartLoadFraction = 1.0;
            VRFRTF = min(1.0, (CyclingRatio / PartLoadFraction));

            this->ElecCoolingPower = 0;
            this->ElecHeatingPower = this->Ncomp + this->OUFanPower;

        } else if (this->OperatingMode == ModeCoolingAndHeating) {
            PartLoadFraction = 1.0;
            VRFRTF = min(1.0, (CyclingRatio / PartLoadFraction));

            this->ElecCoolingPower = (this->Ncomp + this->OUFanPower) * this->IUEvapHeatRate / (this->IUCondHeatRate + this->IUEvapHeatRate);
            this->ElecHeatingPower = (this->Ncomp + this->OUFanPower) * this->IUCondHeatRate / (this->IUCondHeatRate + this->IUEvapHeatRate);

        } else {
            this->ElecCoolingPower = 0;
            this->ElecHeatingPower = 0;
        }
        this->VRFCondRTF = VRFRTF;

        // Calculate CrankCaseHeaterPower: VRF Heat Pump Crankcase Heater Electric Power [W]
        if (this->MaxOATCCHeater > OutdoorDryBulb) {
            // calculate crankcase heater power
            this->CrankCaseHeaterPower = this->CCHeaterPower * (1.0 - VRFRTF);
            if (this->NumCompressors > 1) {
                UpperStageCompressorRatio = (1.0 - this->CompressorSizeRatio) / (this->NumCompressors - 1);
                for (Stage = 1; Stage <= this->NumCompressors - 2; ++Stage) {
                    if (this->VRFCondPLR < (this->CompressorSizeRatio + Stage * UpperStageCompressorRatio)) {
                        this->CrankCaseHeaterPower += this->CCHeaterPower;
                    }
                }
            }
        } else {
            this->CrankCaseHeaterPower = 0.0;
        }

        // Calculate QCondenser: VRF Heat Pump Condenser Heat Transfer Rate [W]
        CondCapacity = max(this->TotalCoolingCapacity, this->TotalHeatingCapacity) * VRFRTF;
        CondPower = max(this->ElecCoolingPower, this->ElecHeatingPower);
        if (this->ElecHeatingPower > 0.0) {
            this->QCondenser = CondCapacity + CondPower - this->TUHeatingLoad / this->PipingCorrectionHeating;
        } else if (this->ElecCoolingPower > 0.0) {
            this->QCondenser = -CondCapacity + CondPower + this->TUCoolingLoad / this->PipingCorrectionCooling;
        } else {
            this->QCondenser = 0.0;
        }
        // if ( this->CondenserType == DataHVACGlobals::EvapCooled )

        // Calculate OperatingHeatingCOP & OperatingCoolingCOP: VRF Heat Pump Operating COP []
        if (CoolingLoad(VRFCond) && CoolingPLR > 0.0) {
            if (this->ElecCoolingPower != 0.0) {
                // this calc should use delivered capacity, not condenser capacity, use VRF(VRFCond).TUCoolingLoad
                this->OperatingCoolingCOP = (this->TotalCoolingCapacity) /
                                            (this->ElecCoolingPower + this->CrankCaseHeaterPower + this->EvapCondPumpElecPower + this->DefrostPower);
            } else {
                this->OperatingCoolingCOP = 0.0;
            }
        }
        if (HeatingLoad(VRFCond) && HeatingPLR > 0.0) {
            // this calc should use delivered capacity, not condenser capacity, use VRF(VRFCond).TUHeatingLoad
            if (this->ElecHeatingPower != 0.0) {
                this->OperatingHeatingCOP = (this->TotalHeatingCapacity) /
                                            (this->ElecHeatingPower + this->CrankCaseHeaterPower + this->EvapCondPumpElecPower + this->DefrostPower);
            } else {
                this->OperatingHeatingCOP = 0.0;
            }
        }

        TotPower = TUParasiticPower + TUFanPower + this->ElecHeatingPower + this->ElecCoolingPower + this->CrankCaseHeaterPower +
                   this->EvapCondPumpElecPower + this->DefrostPower;
        if (TotPower > 0.0) {
            this->OperatingCOP = (this->TUCoolingLoad + this->TUHeatingLoad) / TotPower;
            this->SCHE = this->OperatingCOP * 3.412;
        }

        // limit the TU capacity when the condenser is maxed out on capacity
        // I think this next line will make the max cap report variable match the coil objects, will probably change the answer though
        //  IF(CoolingLoad(VRFCond) .AND. NumTUInCoolingMode .GT. 0 .AND. MaxCoolingCapacity(VRFCond) == MaxCap)THEN
        if (CoolingLoad(VRFCond) && NumTUInCoolingMode > 0) {

            //   IF TU capacity is greater than condenser capacity find maximum allowed TU capacity (i.e., conserve energy)
            if (TU_CoolingLoad > TotalTUCoolingCapacity) {
                LimitTUCapacity(VRFCond,
                                NumTUInList,
                                TotalTUCoolingCapacity,
                                TerminalUnitList(TUListNum).TotalCoolLoad,
                                MaxCoolingCapacity(VRFCond),
                                TotalTUHeatingCapacity,
                                TerminalUnitList(TUListNum).TotalHeatLoad,
                                MaxHeatingCapacity(VRFCond));
            }
        } else if (HeatingLoad(VRFCond) && NumTUInHeatingMode > 0) {
            //   IF TU capacity is greater than condenser capacity
            if (TU_HeatingLoad > TotalTUHeatingCapacity) {
                LimitTUCapacity(VRFCond,
                                NumTUInList,
                                TotalTUHeatingCapacity,
                                TerminalUnitList(TUListNum).TotalHeatLoad,
                                MaxHeatingCapacity(VRFCond),
                                TotalTUCoolingCapacity,
                                TerminalUnitList(TUListNum).TotalCoolLoad,
                                MaxCoolingCapacity(VRFCond));
            }
        } else {
        }

        // Calculate the IU Te/Tc for the next time step
        this->CalcVRFIUTeTc_FluidTCtrl();
    }

    void VRFTerminalUnitEquipment::ControlVRF_FluidTCtrl(EnergyPlusData &state,
                                                         int const VRFTUNum,            // Index to VRF terminal unit
                                                         Real64 const QZnReq,           // Index to zone number
                                                         bool const FirstHVACIteration, // flag for 1st HVAC iteration in the time step
                                                         Real64 &PartLoadRatio,         // unit part load ratio
                                                         Real64 &OnOffAirFlowRatio, // ratio of compressor ON airflow to AVERAGE airflow over timestep
                                                         Real64 &SuppHeatCoilLoad   // supplemental heating coil load (W)
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Rongpeng Zhang
        //       DATE WRITTEN   Nov 2015
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Determine the coil load and part load ratio, given the zone load
        // Determine the air mass flow rate corresponding to the coil load of the heat pump for this time step

        // METHODOLOGY EMPLOYED:
        // Use RegulaFalsi technique to iterate on part-load ratio until convergence is achieved.

        using DataEnvironment::OutDryBulbTemp;
        using General::RoundSigDigits;
        using General::SolveRoot;
        using General::TrimSigDigits;
        using HeatingCoils::SimulateHeatingCoilComponents;
        using ScheduleManager::GetCurrentScheduleValue;
        using TempSolveRoot::SolveRoot;

        int const MaxIte(500);        // maximum number of iterations
        Real64 const MinPLF(0.0);     // minimum part load factor allowed
        Real64 const ErrorTol(0.001); // tolerance for RegulaFalsi iterations
        static ObjexxFCL::gio::Fmt fmtLD("*");

        Real64 FullOutput;      // unit full output when compressor is operating [W]
        Real64 TempOutput;      // unit output when iteration limit exceeded [W]
        Real64 NoCompOutput;    // output when no active compressor [W]
        int SolFla;             // Flag of RegulaFalsi solver
        Array1D<Real64> Par(6); // Parameters passed to RegulaFalsi
        Real64 TempMinPLR;      // min PLR used in Regula Falsi call
        Real64 TempMaxPLR;      // max PLR used in Regula Falsi call
        bool ContinueIter;      // used when convergence is an issue
        int VRFCond;            // index to VRF condenser
        int IndexToTUInTUList;  // index to TU in specific list for the VRF system
        int TUListIndex;        // index to TU list for this VRF system
        bool VRFCoolingMode;
        bool VRFHeatingMode;
        bool HRCoolingMode;
        bool HRHeatingMode;

        PartLoadRatio = 0.0;
        LoopDXCoolCoilRTF = 0.0;
        LoopDXHeatCoilRTF = 0.0;
        VRFCond = this->VRFSysNum;
        IndexToTUInTUList = this->IndexToTUInTUList;
        TUListIndex = VRF(VRFCond).ZoneTUListPtr;
        VRFCoolingMode = CoolingLoad(VRFCond);
        VRFHeatingMode = HeatingLoad(VRFCond);
        HRCoolingMode = TerminalUnitList(TUListIndex).HRCoolRequest(IndexToTUInTUList);
        HRHeatingMode = TerminalUnitList(TUListIndex).HRHeatRequest(IndexToTUInTUList);

        // The RETURNS here will jump back to SimVRF where the CalcVRF routine will simulate with latest PLR

        // do nothing else if TU is scheduled off
        if (GetCurrentScheduleValue(this->SchedPtr) == 0.0) return;

        // Block the following statement: QZnReq==0 doesn't mean QCoilReq==0 due to possible OA mixer operation. zrp_201511
        // do nothing if TU has no load (TU will be modeled using PLR=0)
        // if ( QZnReq == 0.0 ) return;

        // Set EMS value for PLR and return
        if (this->EMSOverridePartLoadFrac) {
            PartLoadRatio = this->EMSValueForPartLoadFrac;
            return;
        }

        // Get result when DX coil is off
        PartLoadRatio = 0.0;

        // Algorithm Type: VRF model based on physics, applicable for Fluid Temperature Control
        this->CalcVRF_FluidTCtrl(state, VRFTUNum, FirstHVACIteration, 0.0, NoCompOutput, OnOffAirFlowRatio, SuppHeatCoilLoad);

        if (VRFCoolingMode && HRHeatingMode) {
            // IF the system is in cooling mode, but the terminal unit requests heating (heat recovery)
            if (NoCompOutput >= QZnReq) return;
        } else if (VRFHeatingMode && HRCoolingMode) {
            // IF the system is in heating mode, but the terminal unit requests cooling (heat recovery)
            if (NoCompOutput <= QZnReq) return;
        } else if (VRFCoolingMode || HRCoolingMode) {
            // IF the system is in cooling mode and/or the terminal unit requests cooling
            if (NoCompOutput <= QZnReq) return;
        } else if (VRFHeatingMode || HRHeatingMode) {
            // IF the system is in heating mode and/or the terminal unit requests heating
            if (NoCompOutput >= QZnReq) return;
        }

        // Otherwise the coil needs to turn on. Get full load result
        PartLoadRatio = 1.0;
        this->CalcVRF_FluidTCtrl(state, VRFTUNum, FirstHVACIteration, PartLoadRatio, FullOutput, OnOffAirFlowRatio, SuppHeatCoilLoad);
        if (this->CoolingCoilPresent) {
            this->coilInNodeT = DataLoopNode::Node(DXCoils::DXCoil(this->CoolCoilIndex).AirInNode).Temp;
            this->coilInNodeW = DataLoopNode::Node(DXCoils::DXCoil(this->CoolCoilIndex).AirInNode).HumRat;
        } else {
            this->coilInNodeT = DataLoopNode::Node(DXCoils::DXCoil(this->HeatCoilIndex).AirInNode).Temp;
            this->coilInNodeW = DataLoopNode::Node(DXCoils::DXCoil(this->HeatCoilIndex).AirInNode).HumRat;
        }

        // set supplemental heating coil calculation if the condition requires
        if (this->SuppHeatingCoilPresent) {
            if ((QZnReq > DataHVACGlobals::SmallLoad && QZnReq > FullOutput) ||
                (this->isSetPointControlled && this->suppTempSetPoint > DataLoopNode::Node(this->SuppHeatCoilAirInletNode).Temp)) {
                Real64 ZoneLoad = 0.0;
                Real64 LoadToHeatingSP = 0.0;
                Real64 LoadToCoolingSP = 0.0;
                if (this->isSetPointControlled) {
                    Real64 mDot = DataLoopNode::Node(this->SuppHeatCoilAirInletNode).MassFlowRate;
                    Real64 Tin = DataLoopNode::Node(this->SuppHeatCoilAirInletNode).Temp;
                    Real64 Win = DataLoopNode::Node(this->SuppHeatCoilAirInletNode).HumRat;
                    Real64 CpAirIn = Psychrometrics::PsyCpAirFnW(Win);
                    SuppHeatCoilLoad = mDot * CpAirIn * (this->suppTempSetPoint - Tin);
                    this->SuppHeatingCoilLoad = SuppHeatCoilLoad;
                    if (this->DesignSuppHeatingCapacity > 0.0) {
                        this->SuppHeatPartLoadRatio = min(1.0, SuppHeatCoilLoad / this->DesignSuppHeatingCapacity);
                    }
                } else {
                    getVRFTUZoneLoad(VRFTUNum, ZoneLoad, LoadToHeatingSP, LoadToCoolingSP, false);
                    if ((FullOutput < (LoadToHeatingSP - DataHVACGlobals::SmallLoad)) && !FirstHVACIteration) {
                        SuppHeatCoilLoad = max(0.0, LoadToHeatingSP - FullOutput);
                        this->SuppHeatingCoilLoad = SuppHeatCoilLoad;
                        if (this->DesignSuppHeatingCapacity > 0.0) {
                            this->SuppHeatPartLoadRatio = min(1.0, SuppHeatCoilLoad / this->DesignSuppHeatingCapacity);
                        }
                    } else {
                        SuppHeatCoilLoad = 0.0;
                        this->SuppHeatPartLoadRatio = 0.0;
                    }
                }
            } else {
                SuppHeatCoilLoad = 0.0;
                this->SuppHeatPartLoadRatio = 0.0;
            }
        } else {
            SuppHeatCoilLoad = 0.0;
            this->SuppHeatPartLoadRatio = 0.0;
        }

        if ((VRFCoolingMode && !VRF(VRFCond).HeatRecoveryUsed) || (VRF(VRFCond).HeatRecoveryUsed && HRCoolingMode)) {
            // Since we are cooling, we expect FullOutput < NoCompOutput
            // If the QZnReq <= FullOutput the unit needs to run full out
            if (QZnReq <= FullOutput) {
                // if no coil present in terminal unit, no need to reset PLR?
                if (VRFTU(VRFTUNum).CoolingCoilPresent) {
                    PartLoadRatio = 1.0;
                    // the zone set point could be exceeded if set point control is used so protect against that
                    if (this->isSetPointControlled) {
                        if (DataLoopNode::Node(this->coolCoilAirOutNode).Temp > this->coilTempSetPoint) return;
                    } else {
                        return;
                    }
                } else {
                    PartLoadRatio = 0.0;
                    return;
                }
            }
        } else if ((VRFHeatingMode && !VRF(VRFCond).HeatRecoveryUsed) || (VRF(VRFCond).HeatRecoveryUsed && HRHeatingMode)) {
            // Since we are heating, we expect FullOutput > NoCompOutput
            // If the QZnReq >= FullOutput the unit needs to run full out
            if (QZnReq >= FullOutput) {
                // if no coil present in terminal unit, no need reset PLR?
                if (this->HeatingCoilPresent) {
                    PartLoadRatio = 1.0;
                    // the zone set point could be exceeded if set point control is used so protect against that
                    if (this->isSetPointControlled) {
                        if (DataLoopNode::Node(this->heatCoilAirOutNode).Temp < this->coilTempSetPoint) return;
                    } else {
                        return;
                    }
                } else {
                    PartLoadRatio = 0.0;
                    return;
                }
            }
        } else {
            // VRF terminal unit is off
            // shouldn't actually get here
            PartLoadRatio = 0.0;
            return;
        }

        // The coil will not operate at PLR=0 or PLR=1, calculate the operating part-load ratio

        if ((VRFHeatingMode || HRHeatingMode) || (VRFCoolingMode || HRCoolingMode)) {

            Par(1) = VRFTUNum;
            Par(2) = 0.0;
            if (VRFTU(VRFTUNum).isSetPointControlled) Par(2) = 1.0;
            Par(4) = 0.0;
            if (FirstHVACIteration) {
                Par(3) = 1.0;
            } else {
                Par(3) = 0.0;
            }
            //    Par(4) = OpMode
            Par(5) = QZnReq;
            Par(6) = OnOffAirFlowRatio;
            SolveRoot(state, ErrorTol, MaxIte, SolFla, PartLoadRatio, PLRResidual, 0.0, 1.0, Par);
            if (SolFla == -1) {
                //     Very low loads may not converge quickly. Tighten PLR boundary and try again.
                TempMaxPLR = -0.1;
                ContinueIter = true;
                while (ContinueIter && TempMaxPLR < 1.0) {
                    TempMaxPLR += 0.1;

                    this->CalcVRF_FluidTCtrl(state, VRFTUNum, FirstHVACIteration, TempMaxPLR, TempOutput, OnOffAirFlowRatio, SuppHeatCoilLoad);

                    if (VRFHeatingMode && TempOutput > QZnReq) ContinueIter = false;
                    if (VRFCoolingMode && TempOutput < QZnReq) ContinueIter = false;
                }
                TempMinPLR = TempMaxPLR;
                ContinueIter = true;
                while (ContinueIter && TempMinPLR > 0.0) {
                    TempMaxPLR = TempMinPLR;
                    TempMinPLR -= 0.01;

                    this->CalcVRF_FluidTCtrl(state, VRFTUNum, FirstHVACIteration, TempMaxPLR, TempOutput, OnOffAirFlowRatio, SuppHeatCoilLoad);

                    if (VRFHeatingMode && TempOutput < QZnReq) ContinueIter = false;
                    if (VRFCoolingMode && TempOutput > QZnReq) ContinueIter = false;
                }
                SolveRoot(state, ErrorTol, MaxIte, SolFla, PartLoadRatio, PLRResidual, TempMinPLR, TempMaxPLR, Par);
                if (SolFla == -1) {
                    if (!FirstHVACIteration && !WarmupFlag) {
                        if (this->IterLimitExceeded == 0) {
                            ShowWarningMessage(DataHVACGlobals::cVRFTUTypes(this->VRFTUType_Num) + " \"" + this->Name + "\"");
                            ShowContinueError(
                                format(" Iteration limit exceeded calculating terminal unit part-load ratio, maximum iterations = {}", MaxIte));
                            ShowContinueErrorTimeStamp(" Part-load ratio returned = " + RoundSigDigits(PartLoadRatio, 3));

                            this->CalcVRF_FluidTCtrl(
                                state, VRFTUNum, FirstHVACIteration, TempMinPLR, TempOutput, OnOffAirFlowRatio, SuppHeatCoilLoad);

                            ShowContinueError(" Load requested = " + TrimSigDigits(QZnReq, 5) + ", Load delivered = " + TrimSigDigits(TempOutput, 5));
                            ShowRecurringWarningErrorAtEnd(DataHVACGlobals::cVRFTUTypes(this->VRFTUType_Num) + " \"" + this->Name +
                                                               "\" -- Terminal unit Iteration limit exceeded error continues...",
                                                           this->IterLimitExceeded);
                        } else {
                            ShowRecurringWarningErrorAtEnd(DataHVACGlobals::cVRFTUTypes(this->VRFTUType_Num) + " \"" + this->Name +
                                                               "\" -- Terminal unit Iteration limit exceeded error continues...",
                                                           this->IterLimitExceeded);
                        }
                    }
                } else if (SolFla == -2) {
                    PartLoadRatio = max(MinPLF, std::abs(QZnReq - NoCompOutput) / std::abs(FullOutput - NoCompOutput));
                }
            } else if (SolFla == -2) {
                if (FullOutput - NoCompOutput == 0.0) {
                    PartLoadRatio = 0.0;
                } else {
                    PartLoadRatio = min(1.0, max(MinPLF, std::abs(QZnReq - NoCompOutput) / std::abs(FullOutput - NoCompOutput)));
                }
            }
        }
    }

    void VRFTerminalUnitEquipment::CalcVRF_FluidTCtrl(EnergyPlusData &state,
                                                      int const VRFTUNum,                // Index to VRF terminal unit
                                                      bool const FirstHVACIteration,     // flag for 1st HVAC iteration in the time step
                                                      Real64 const PartLoadRatio,        // compressor part load fraction
                                                      Real64 &LoadMet,                   // load met by unit (W)
                                                      Real64 &OnOffAirFlowRatio,         // ratio of ON air flow to average air flow
                                                      Real64 &SuppHeatCoilLoad,          // supplemental heating coil load (W)
                                                      Optional<Real64> LatOutputProvided // delivered latent capacity (W)
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         RP Zhang (LBNL), XF Pang (LBNL), Y Yura (Daikin Inc)
        //       DATE WRITTEN   June 2015
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // 		This subroutine is part of the new VRF model based on physics, applicable for Fluid Temperature Control.
        // 		This is adapted from subroutine CalcVRF, which is part of the VRF model based on system curves.
        // 		This subroutine simulates the components making up the VRF indoor terminal unit.

        // METHODOLOGY EMPLOYED:
        //		A new physics based VRF model applicable for Fluid Temperature Control.
        using DataZoneEquipment::ZoneEquipConfig;
        using DXCoils::SimDXCoil;
        using HeatingCoils::SimulateHeatingCoilComponents;
        using MixedAir::SimOAMixer;
        using SingleDuct::SimATMixer;
        using SteamCoils::SimulateSteamCoilComponents;
        using WaterCoils::SimulateWaterCoilComponents;

        int VRFTUOutletNodeNum;     // TU air outlet node
        int VRFTUInletNodeNum;      // TU air inlet node
        Real64 AirMassFlow;         // total supply air mass flow [m3/s]
        int OpMode;                 // fan operating mode, DataHVACGlobals::CycFanCycCoil or DataHVACGlobals::ContFanCycCoil
        int VRFCond;                // index to VRF condenser
        Real64 SpecHumOut;          // specific humidity ratio at outlet node
        Real64 SpecHumIn;           // specific humidity ratio at inlet node
        int TUListIndex;            // index to TU list for this VRF system
        int IndexToTUInTUList;      // index to TU in specific list for the VRF system
        Real64 EvapTemp;            // evaporating temperature
        Real64 CondTemp;            // condensing temperature
        static int ATMixOutNode(0); // outlet node of ATM Mixer
        int ZoneNode;               // Zone node of VRFTU is serving
        // FLOW

        VRFCond = this->VRFSysNum;
        TUListIndex = VRF(VRFCond).ZoneTUListPtr;
        IndexToTUInTUList = this->IndexToTUInTUList;
        VRFTUOutletNodeNum = this->VRFTUOutletNodeNum;
        VRFTUInletNodeNum = this->VRFTUInletNodeNum;
        OpMode = this->OpMode;
        EvapTemp = VRF(VRFCond).IUEvaporatingTemp;
        CondTemp = VRF(VRFCond).IUCondensingTemp;
        ZoneNode = this->ZoneAirNode;

        // Set inlet air mass flow rate based on PLR and compressor on/off air flow rates
        if (PartLoadRatio == 0) {
            // only provide required OA when coil is off
            CompOnMassFlow = OACompOnMassFlow;
            CompOffMassFlow = OACompOffMassFlow;
        } else {
            // identify the air flow rate corresponding to the coil load
            CompOnMassFlow = CalVRFTUAirFlowRate_FluidTCtrl(state, VRFTUNum, PartLoadRatio, FirstHVACIteration);
        }
        SetAverageAirFlow(VRFTUNum, PartLoadRatio, OnOffAirFlowRatio);
        AirMassFlow = DataLoopNode::Node(VRFTUInletNodeNum).MassFlowRate;

        if (this->ATMixerExists) {
            // There is an air terminal mixer
            ATMixOutNode = this->ATMixerOutNode;
            if (this->ATMixerType == DataHVACGlobals::ATMixer_InletSide) { // if there is an inlet side air terminal mixer
                // set the primary air inlet mass flow rate
                DataLoopNode::Node(this->ATMixerPriNode).MassFlowRate =
                    min(DataLoopNode::Node(this->ATMixerPriNode).MassFlowRateMaxAvail, DataLoopNode::Node(VRFTUInletNodeNum).MassFlowRate);
                // now calculate the the mixer outlet air conditions (and the secondary air inlet flow rate). The mixer outlet flow rate has already
                // been set above (it is the "inlet" node flow rate)
                SimATMixer(state, this->ATMixerName, FirstHVACIteration, this->ATMixerIndex);
            }
        } else {
            ATMixOutNode = 0;
            // simulate OA Mixer
            if (this->OAMixerUsed) SimOAMixer(state, this->OAMixerName, FirstHVACIteration, this->OAMixerIndex);
        }
        // if blow through, simulate fan then coils
        if (this->FanPlace == DataHVACGlobals::BlowThru) {
            if (VRFTU(VRFTUNum).fanType_Num == DataHVACGlobals::FanType_SystemModelObject) {
                if (OnOffAirFlowRatio > 0.0) {
                    //                    HVACFan::fanObjs[VRFTU(VRFTUNum).FanIndex]->simulate(1.0 / OnOffAirFlowRatio,
                    //                    DataHVACGlobals::ZoneCompTurnFansOff, DataHVACGlobals::ZoneCompTurnFansOff, _);
                    HVACFan::fanObjs[VRFTU(VRFTUNum).FanIndex]->simulate(
                        state, _, DataHVACGlobals::ZoneCompTurnFansOff, DataHVACGlobals::ZoneCompTurnFansOff, _);
                } else {
                    HVACFan::fanObjs[VRFTU(VRFTUNum).FanIndex]->simulate(
                        state, PartLoadRatio, DataHVACGlobals::ZoneCompTurnFansOff, DataHVACGlobals::ZoneCompTurnFansOff, _);
                }
            } else {
                Fans::SimulateFanComponents(state,
                                            "",
                                            FirstHVACIteration,
                                            this->FanIndex,
                                            FanSpeedRatio,
                                            DataHVACGlobals::ZoneCompTurnFansOff,
                                            DataHVACGlobals::ZoneCompTurnFansOff);
            }
        }
        if (this->CoolingCoilPresent) {
            // above condition for heat pump mode, below condition for heat recovery mode
            if ((!VRF(VRFCond).HeatRecoveryUsed && CoolingLoad(VRFCond)) ||
                (VRF(VRFCond).HeatRecoveryUsed && TerminalUnitList(TUListIndex).HRCoolRequest(IndexToTUInTUList))) {
                SimDXCoil(state,
                          "",
                          On,
                          FirstHVACIteration,
                          this->CoolCoilIndex,
                          OpMode,
                          PartLoadRatio,
                          _,
                          _,
                          MaxCoolingCapacity(VRFCond),
                          VRF(this->VRFSysNum).VRFCondCyclingRatio);
            } else { // cooling coil is off
                SimDXCoil(state, "", Off, FirstHVACIteration, this->CoolCoilIndex, OpMode, 0.0, _);
            }
            LoopDXCoolCoilRTF = state.dataAirLoop->LoopDXCoilRTF;
        } else {
            LoopDXCoolCoilRTF = 0.0;
        }

        if (this->HeatingCoilPresent) {
            // above condition for heat pump mode, below condition for heat recovery mode
            if ((!VRF(VRFCond).HeatRecoveryUsed && HeatingLoad(VRFCond)) ||
                (VRF(VRFCond).HeatRecoveryUsed && TerminalUnitList(TUListIndex).HRHeatRequest(IndexToTUInTUList))) {
                SimDXCoil(state, "", On, FirstHVACIteration, this->HeatCoilIndex, OpMode, PartLoadRatio, _, _, MaxHeatingCapacity(VRFCond));
            } else {
                SimDXCoil(state, "", Off, FirstHVACIteration, this->HeatCoilIndex, OpMode, 0.0, _);
            }
            LoopDXHeatCoilRTF = state.dataAirLoop->LoopDXCoilRTF;
        } else {
            LoopDXHeatCoilRTF = 0.0;
        }

        // if draw through, simulate coils then fan
        if (this->FanPlace == DataHVACGlobals::DrawThru) {
            if (VRFTU(VRFTUNum).fanType_Num == DataHVACGlobals::FanType_SystemModelObject) {
                if (OnOffAirFlowRatio > 0.0) {
                    //                    HVACFan::fanObjs[VRFTU(VRFTUNum).FanIndex]->simulate(1.0 / OnOffAirFlowRatio,
                    //                    DataHVACGlobals::ZoneCompTurnFansOff, DataHVACGlobals::ZoneCompTurnFansOff, _);
                    HVACFan::fanObjs[VRFTU(VRFTUNum).FanIndex]->simulate(
                        state, _, DataHVACGlobals::ZoneCompTurnFansOff, DataHVACGlobals::ZoneCompTurnFansOff, _);
                } else {
                    HVACFan::fanObjs[VRFTU(VRFTUNum).FanIndex]->simulate(
                        state, PartLoadRatio, DataHVACGlobals::ZoneCompTurnFansOff, DataHVACGlobals::ZoneCompTurnFansOff, _);
                }

            } else {
                Fans::SimulateFanComponents(state,
                                            "",
                                            FirstHVACIteration,
                                            this->FanIndex,
                                            FanSpeedRatio,
                                            DataHVACGlobals::ZoneCompTurnFansOff,
                                            DataHVACGlobals::ZoneCompTurnFansOff);
            }
        }

        // track fan power per terminal unit for calculating COP
        if (this->fanType_Num == DataHVACGlobals::FanType_SystemModelObject) {
            this->FanPower = HVACFan::fanObjs[this->FanIndex]->fanPower();
        } else {
            this->FanPower = Fans::GetFanPower(this->FanIndex);
        }

        // run supplemental heating coil
        if (this->SuppHeatingCoilPresent) {
            Real64 SuppPLR = this->SuppHeatPartLoadRatio;
            this->CalcVRFSuppHeatingCoil(state, VRFTUNum, FirstHVACIteration, SuppPLR, SuppHeatCoilLoad);
            if ((DataLoopNode::Node(this->SuppHeatCoilAirOutletNode).Temp > this->MaxSATFromSuppHeatCoil) && SuppPLR > 0.0) {
                // adjust the heating load to maximum allowed
                Real64 MaxHeatCoilLoad = this->HeatingCoilCapacityLimit(this->SuppHeatCoilAirInletNode, this->MaxSATFromSuppHeatCoil);
                this->CalcVRFSuppHeatingCoil(state, VRFTUNum, FirstHVACIteration, SuppPLR, MaxHeatCoilLoad);
                SuppHeatCoilLoad = MaxHeatCoilLoad;
            }
        }

        Real64 LatentLoadMet = 0.0;
        Real64 TempOut = 0.0;
        Real64 TempIn = 0.0;
        if (this->ATMixerExists) {
            if (this->ATMixerType == DataHVACGlobals::ATMixer_SupplySide) {
                // Air terminal supply side mixer, calculate supply side mixer output
                SimATMixer(state, this->ATMixerName, FirstHVACIteration, this->ATMixerIndex);
                TempOut = DataLoopNode::Node(ATMixOutNode).Temp;
                SpecHumOut = DataLoopNode::Node(ATMixOutNode).HumRat;
                AirMassFlow = DataLoopNode::Node(ATMixOutNode).MassFlowRate;
            } else {
                // Air terminal inlet side mixer
                TempOut = DataLoopNode::Node(VRFTUOutletNodeNum).Temp;
                SpecHumOut = DataLoopNode::Node(VRFTUOutletNodeNum).HumRat;
            }
            TempIn = DataLoopNode::Node(ZoneNode).Temp;
            SpecHumIn = DataLoopNode::Node(ZoneNode).HumRat;
        } else {
            TempOut = DataLoopNode::Node(VRFTUOutletNodeNum).Temp;
            SpecHumOut = DataLoopNode::Node(VRFTUOutletNodeNum).HumRat;
            if (ZoneNode > 0) {
                TempIn = DataLoopNode::Node(ZoneNode).Temp;
                SpecHumIn = DataLoopNode::Node(ZoneNode).HumRat;
            } else {
                TempIn = DataLoopNode::Node(VRFTUInletNodeNum).Temp;
                SpecHumIn = DataLoopNode::Node(VRFTUInletNodeNum).HumRat;
            }
        }
        // calculate sensible load met using delta enthalpy
        LoadMet = AirMassFlow * PsyDeltaHSenFnTdb2W2Tdb1W1(TempOut, SpecHumOut, TempIn, SpecHumIn); // sensible {W}
        LatentLoadMet = AirMassFlow * (SpecHumOut - SpecHumIn);                                     // latent {kgWater/s}
        if (present(LatOutputProvided)) {
            LatOutputProvided = LatentLoadMet;
        }
    }

    Real64 VRFTerminalUnitEquipment::CalVRFTUAirFlowRate_FluidTCtrl(EnergyPlusData &state,
                                                                    int const VRFTUNum,     // Index to VRF terminal unit
                                                                    Real64 PartLoadRatio,   // part load ratio of the coil
                                                                    bool FirstHVACIteration // FirstHVACIteration flag
    )
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Rongpeng Zhang, LBNL
        //       DATE WRITTEN   Nov 2015
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS FUNCTION:
        //  This function determines the TU airflow rate corresponding to the coil load.
        //  This is used to address the coupling between OA mixer simulation and VRF-FluidTCtrl coil simulation.

        // METHODOLOGY EMPLOYED:
        //  VRF-FluidTCtrl TU airflow rate is determined by the control logic of VRF-FluidTCtrl coil to match the
        //  coil load. This is affected by the coil inlet conditions. However, the airflow rate will affect the
        //  OA mixer simulation, which leads to different coil inlet conditions. So, there is a coupling issue here.

        using DataEnvironment::OutDryBulbTemp;
        using DXCoils::DXCoil;
        using General::SolveRoot;
        using TempSolveRoot::SolveRoot;

        Real64 AirMassFlowRate; // air mass flow rate of the coil (kg/s)

        Array1D<Real64> Par(7);      // Parameters passed to RegulaFalsi
        int const Mode(1);           // Performance mode for MultiMode DX coil. Always 1 for other coil types
        int const MaxIte(500);       // maximum number of iterations
        int DXCoilNum;               // index to DX Coil
        int IndexToTUInTUList;       // index to TU in specific list for the VRF system
        int SolFla;                  // Flag of RegulaFalsi solver
        int TUListIndex;             // index to TU list for this VRF system
        int VRFCond;                 // index to VRF condenser
        Real64 const ErrorTol(0.01); // tolerance for RegulaFalsi iterations
        Real64 FanSpdRatio;          // ratio of required and rated air flow rate
        Real64 FanSpdRatioMin;       // min fan speed ratio
        Real64 FanSpdRatioMax;       // min fan speed ratio
        Real64 QCoilReq;             // required coil load (W)
        Real64 QCoilAct;             // actual coil load (W)
        Real64 TeTc;                 // evaporating temperature or condensing temperature for VRF indoor unit(C)

        VRFCond = this->VRFSysNum;
        TUListIndex = VRF(VRFCond).ZoneTUListPtr;
        IndexToTUInTUList = this->IndexToTUInTUList;

        if ((!VRF(VRFCond).HeatRecoveryUsed && CoolingLoad(VRFCond)) ||
            (VRF(VRFCond).HeatRecoveryUsed && TerminalUnitList(TUListIndex).HRCoolRequest(IndexToTUInTUList))) {
            // VRF terminal unit is on cooling mode
            DXCoilNum = this->CoolCoilIndex;
            QCoilReq = -PartLoadRatio * DXCoil(DXCoilNum).RatedTotCap(Mode);
            TeTc = VRF(VRFCond).IUEvaporatingTemp;

            // For HR operations, Te is lower than the outdoor air temperature because of outdoor evaporator operations
            // The difference is usually 2-3C according to the engineering experience. 2 is used here for a slightly bigger fan flow rate.
            if (VRF(VRFCond).HeatRecoveryUsed) TeTc = min(TeTc, OutDryBulbTemp - 2);

        } else if ((!VRF(VRFCond).HeatRecoveryUsed && HeatingLoad(VRFCond)) ||
                   (VRF(VRFCond).HeatRecoveryUsed && TerminalUnitList(TUListIndex).HRHeatRequest(IndexToTUInTUList))) {
            // VRF terminal unit is on heating mode
            DXCoilNum = this->HeatCoilIndex;
            QCoilReq = PartLoadRatio * DXCoil(DXCoilNum).RatedTotCap(Mode);
            TeTc = VRF(VRFCond).IUCondensingTemp;

        } else {
            // VRF terminal unit is off
            QCoilAct = 0.0;
            AirMassFlowRate = max(OACompOnMassFlow, 0.0);
            return AirMassFlowRate;
        }

        // minimum airflow rate
        if (DXCoil(DXCoilNum).RatedAirMassFlowRate(Mode) > 0.0) {
            FanSpdRatioMin = min(OACompOnMassFlow / DXCoil(DXCoilNum).RatedAirMassFlowRate(Mode), 1.0);
        } else {
            // VRF terminal unit is off
            QCoilAct = 0.0;
            AirMassFlowRate = max(OACompOnMassFlow, 0.0);
            return AirMassFlowRate;
        }

        if (FirstHVACIteration) {
            Par(1) = 1.0;
        } else {
            Par(1) = 0.0;
        }
        Par(2) = VRFTUNum;
        Par(3) = DXCoilNum;
        Par(4) = QCoilReq;
        Par(5) = TeTc;
        Par(6) = PartLoadRatio;
        Par(7) = OACompOnMassFlow;

        FanSpdRatioMax = 1.0;
        SolveRoot(state, ErrorTol, MaxIte, SolFla, FanSpdRatio, VRFTUAirFlowResidual_FluidTCtrl, FanSpdRatioMin, FanSpdRatioMax, Par);
        if (SolFla < 0) FanSpdRatio = FanSpdRatioMax; // over capacity

        AirMassFlowRate = FanSpdRatio * DXCoil(DXCoilNum).RatedAirMassFlowRate(Mode);

        return AirMassFlowRate;
    }

    Real64 VRFTUAirFlowResidual_FluidTCtrl(EnergyPlusData &state,
                                           Real64 const FanSpdRatio,  // fan speed ratio of VRF VAV TU
                                           Array1D<Real64> const &Par // par(1) = VRFTUNum
    )
    {
        // FUNCTION INFORMATION:
        //       AUTHOR         Rongpeng Zhang, LBNL
        //       DATE WRITTEN   Nov 2015
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // 		Calculates residual function ( FanSpdRatioAct - FanSpdRatio ) / FanSpdRatio
        // 		This is used to address the coupling between OA mixer simulation and VRF-FluidTCtrl coil simulation.

        // METHODOLOGY EMPLOYED:
        // 		VRF-FluidTCtrl TU airflow rate is determined by the control logic of VRF-FluidTCtrl coil to match the
        // 		coil load. This is affected by the coil inlet conditions. However, the airflow rate will affect the
        // 		OA mixer simulation, which leads to different coil inlet conditions. So, there is a coupling issue here.

        using DXCoils::ControlVRFIUCoil;
        using DXCoils::DXCoil;
        using Fans::Fan;
        using Fans::SimulateFanComponents;
        using MixedAir::OAMixer;
        using MixedAir::SimOAMixer;
        using Psychrometrics::PsyHFnTdbW;
        using SingleDuct::SimATMixer;

        Real64 AirFlowRateResidual;

        // SUBROUTINE ARGUMENT DEFINITIONS:
        // 	Par( 1 ) = FirstHVACIteration;
        // 	Par( 2 ) = VRFTUNum;
        // 	Par( 3 ) = DXCoilNum;
        // 	Par( 4 ) = QCoilReq;
        // 	Par( 5 ) = TeTc;
        // 	Par( 6 ) = PartLoadRatio;
        // 	Par( 7 ) = OACompOnMassFlow;

        int const Mode(1);       // Performance mode for MultiMode DX coil. Always 1 for other coil types
        int CoilIndex;           // index to coil
        int OAMixNode;           // index to the mix node of OA mixer
        int VRFCond;             // index to VRF condenser
        int VRFTUNum;            // Unit index in VRF terminal unit array
        int VRFInletNode;        // VRF inlet node number
        bool FirstHVACIteration; // flag for 1st HVAC iteration in the time step
        Real64 FanSpdRatioBase;  // baseline FanSpdRatio for VRFTUAirFlowResidual
        Real64 FanSpdRatioAct;   // calculated FanSpdRatio for VRFTUAirFlowResidual
        Real64 PartLoadRatio;    // Part load ratio
        Real64 QCoilReq;         // required coil load [W]
        Real64 QCoilAct;         // actual coil load [W]
        Real64 temp;             // for temporary use
        Real64 TeTc;             // evaporating/condensing temperature [C]
        Real64 Tin;              // coil inlet air temperature [C]
        Real64 Win;              // coil inlet air humidity ratio [kg/kg]
        Real64 Hin;              // coil inlet air enthalpy
        Real64 Wout;             // coil outlet air humidity ratio
        Real64 Tout;             // coil outlet air temperature
        Real64 Hout;             // coil outlet air enthalpy
        Real64 SHact;            // coil actual SH
        Real64 SCact;            // coil actual SC

        // FirstHVACIteration is a logical, Par is real, so make 1.0=TRUE and 0.0=FALSE
        FirstHVACIteration = (Par(1) == 1.0);
        VRFTUNum = int(Par(2));
        CoilIndex = int(Par(3));
        QCoilReq = Par(4);
        TeTc = Par(5);
        PartLoadRatio = Par(6);
        OACompOnMassFlow = Par(7);

        VRFCond = VRFTU(VRFTUNum).VRFSysNum;
        VRFInletNode = VRFTU(VRFTUNum).VRFTUInletNodeNum;

        if (std::abs(FanSpdRatio) < 0.01)
            FanSpdRatioBase = sign(0.01, FanSpdRatio);
        else
            FanSpdRatioBase = FanSpdRatio;

        // Set inlet air mass flow rate based on PLR and compressor on/off air flow rates
        CompOnMassFlow = FanSpdRatio * DXCoil(CoilIndex).RatedAirMassFlowRate(Mode);
        SetAverageAirFlow(VRFTUNum, PartLoadRatio, temp);
        Tin = DataLoopNode::Node(VRFInletNode).Temp;
        Win = DataLoopNode::Node(VRFInletNode).HumRat;

        // Simulation the OAMixer if there is any
        if (VRFTU(VRFTUNum).OAMixerUsed) {
            SimOAMixer(state, VRFTU(VRFTUNum).OAMixerName, FirstHVACIteration, VRFTU(VRFTUNum).OAMixerIndex);
            OAMixNode = OAMixer(VRFTU(VRFTUNum).OAMixerIndex).MixNode;
            Tin = DataLoopNode::Node(OAMixNode).Temp;
            Win = DataLoopNode::Node(OAMixNode).HumRat;
        }
        // Simulate the blow-through fan if there is any
        if (VRFTU(VRFTUNum).FanPlace == DataHVACGlobals::BlowThru) {
            if (VRFTU(VRFTUNum).fanType_Num == DataHVACGlobals::FanType_SystemModelObject) {
                if (temp > 0) {
                    //                    HVACFan::fanObjs[VRFTU(VRFTUNum).FanIndex]->simulate(1.0 / temp, DataHVACGlobals::ZoneCompTurnFansOff,
                    //                    DataHVACGlobals::ZoneCompTurnFansOff, _);
                    HVACFan::fanObjs[VRFTU(VRFTUNum).FanIndex]->simulate(
                        state, _, DataHVACGlobals::ZoneCompTurnFansOff, DataHVACGlobals::ZoneCompTurnFansOff, _);
                } else {
                    HVACFan::fanObjs[VRFTU(VRFTUNum).FanIndex]->simulate(
                        state, PartLoadRatio, DataHVACGlobals::ZoneCompTurnFansOff, DataHVACGlobals::ZoneCompTurnFansOff, _);
                }
            } else {
                Fans::SimulateFanComponents(state,
                                            "",
                                            false,
                                            VRFTU(VRFTUNum).FanIndex,
                                            FanSpeedRatio,
                                            DataHVACGlobals::ZoneCompTurnFansOff,
                                            DataHVACGlobals::ZoneCompTurnFansOff);
            }
            Tin = DataLoopNode::Node(VRFTU(VRFTUNum).fanOutletNode).Temp;
            Win = DataLoopNode::Node(VRFTU(VRFTUNum).fanOutletNode).HumRat;
        }

        // Call the coil control logic to determine the air flow rate to match the given coil load
        ControlVRFIUCoil(CoilIndex, QCoilReq, Tin, Win, TeTc, OACompOnMassFlow, FanSpdRatioAct, Wout, Tout, Hout, SHact, SCact);

        Hin = PsyHFnTdbW(Tin, Win);
        QCoilAct = FanSpdRatioAct * DXCoil(CoilIndex).RatedAirMassFlowRate(Mode) * (Hout - Hin); // positive for heating, negative for cooling

        AirFlowRateResidual = (FanSpdRatioAct - FanSpdRatio);

        return AirFlowRateResidual;
    }

    Real64 VRFOUTeResidual_FluidTCtrl(EnergyPlusData &state,
                                      Real64 const Te,           // outdoor unit evaporating temperature
                                      Array1D<Real64> const &Par // par(1) = VRFTUNum
    )
    {
        // FUNCTION INFORMATION:
        //       AUTHOR         Rongpeng Zhang, LBNL
        //       DATE WRITTEN   Mar 2016
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // 		Calculates residual function ( Tsuction - Tsuction_new )
        // 		This is used to calculate the VRF OU evaporating temperature at the given compressor speed and operational conditions.

        // METHODOLOGY EMPLOYED:
        // 		Call VRFOU_CompCap to calculate the total evaporative capacity Q_c_tot, at the given compressor speed and operational
        // 		conditions, and then call VRFOU_TeTc to obtain Tsuction_new based on OU evaporator air-side calculations

        using DataEnvironment::OutBaroPress;
        using DataEnvironment::OutDryBulbTemp;
        using DataEnvironment::OutHumRat;

        // Return value
        Real64 TeResidual;

        int VRFCond = int(Par(1));        // Index to VRF outdoor unit
        Real64 CompSpdActual = Par(2);    // Actual compressor running speed [rps]
        Real64 Tdischarge = Par(3);       // VRF Compressor discharge refrigerant temperature [C]
        Real64 h_IU_evap_in = Par(4);     // enthalpy of refrigerant at IU evaporator inlet [kJ/kg]
        Real64 h_comp_in = Par(5);        // enthalpy of refrigerant at compressor inlet [kJ/kg]
        Real64 Q_c_TU_PL = Par(6);        // IU evaporator load, including piping loss [W]
        Real64 m_air_evap_rated = Par(7); // Rated OU evaporator air mass flow rate [kg/s]

        // FUNCTION LOCAL VARIABLE DECLARATIONS:
        Real64 Ncomp_temp;   // compressor power [W]
        Real64 Q_c_tot_temp; // total evaporator load, including piping loss [W]
        Real64 Q_c_OU_temp;  // OU evaporator load, including piping loss [W]
        Real64 Te_new;       // newly calculated OU evaporating temperature
        Real64 Tfs;          // OU evaporator coil surface temperature [C]

        // FLOW

        // calculate the total evaporative capacity Q_c_tot, at the given compressor speed and operational conditions
        VRF(VRFCond).VRFOU_CompCap(state, CompSpdActual, Te, Tdischarge, h_IU_evap_in, h_comp_in, Q_c_tot_temp, Ncomp_temp);
        Q_c_OU_temp = Q_c_tot_temp - Q_c_TU_PL;

        // Tsuction_new calculated based on OU evaporator air-side calculations (Tsuction_new < To)
        VRF(VRFCond).VRFOU_TeTc(FlagEvapMode, Q_c_OU_temp, VRF(VRFCond).SH, m_air_evap_rated, OutDryBulbTemp, OutHumRat, OutBaroPress, Tfs, Te_new);

        TeResidual = Te_new - Te;

        return TeResidual;
    }

    Real64 CompResidual_FluidTCtrl(EnergyPlusData &state,
                                   Real64 const T_suc,        // Compressor suction temperature Te' [C]
                                   Array1D<Real64> const &Par // parameters
    )
    {
        // FUNCTION INFORMATION:
        //       AUTHOR         Xiufeng Pang (XP)
        //       DATE WRITTEN   Mar 2013
        //       MODIFIED       Jul 2015, RP Zhang, LBNL
        //       RE-ENGINEERED
        //
        // PURPOSE OF THIS FUNCTION:
        //  	 Calculates residual function ((VRV terminal unit cooling output - Zone sensible cooling load)
        //
        using CurveManager::CurveValue;

        Real64 T_dis;    // Compressor discharge temperature Tc' [C]
        Real64 CondHeat; // Evaporative capacity to be met [W]
        Real64 CAPSpd;   // Evaporative capacity of the compressor at a given spd[W]
        Real64 CompResidual;
        int CAPFT;

        T_dis = Par(1);
        CondHeat = Par(2);
        CAPFT = Par(3);

        CAPSpd = CurveValue(state, CAPFT, T_dis, T_suc);
        CompResidual = (CondHeat - CAPSpd) / CAPSpd;

        return CompResidual;
    }

    void VRFCondenserEquipment::VRFOU_TeTc(int const OperationMode, // Mode 0 for running as condenser, 1 for evaporator
                                           Real64 const Q_coil,     // // OU coil heat release at cooling mode or heat extract at heating mode [W]
                                           Real64 const SHSC,       // SC for OU condenser or SH for OU evaporator [C]
                                           Real64 const m_air,      // OU coil air mass flow rate [kg/s]
                                           Real64 const T_coil_in,  // Temperature of air at OU coil inlet [C]
                                           Real64 const W_coil_in,  // Humidity ratio of air at OU coil inlet [kg/kg]
                                           Real64 const OutdoorPressure, // Outdoor air pressure [Pa]
                                           Real64 &T_coil_surf,          // Air temperature at coil surface [C]
                                           Real64 &TeTc                  // VRF Tc at cooling mode, or Te at heating mode [C]
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Rongpeng Zhang, LBNL
        //       DATE WRITTEN   Jan 2016
        //       MODIFIED       na
        //
        //       RE-ENGINEERED  na
        //
        // PURPOSE OF THIS SUBROUTINE:
        //        Calculate the VRF OU refrigerant side temperature, i.e., condensing temperature
        //        at cooling mode, or evaporating temperature at heating mode, given the coil heat
        //        release/extract amount and air side parameters.
        //
        // METHODOLOGY EMPLOYED:
        //        This is part of the physics based VRF model applicable for Fluid Temperature Control.
        //
        using General::TrimSigDigits;

        Real64 BF;              // VRF OU bypass  [-]
        Real64 deltaT;          // Difference between Te/Tc and air temperature at coil surface [C]
        Real64 h_coil_in;       // Enthalpy of air at OU coil inlet [C]
        Real64 h_coil_out;      // Enthalpy of air at OU coil outlet [C]
        Real64 T_coil_out;      // Air temperature at coil outlet [C]
        Real64 T_coil_surf_sat; // Saturated air temperature at coil surface [C]
        Real64 W_coil_surf_sat; // Humidity ratio of saturated air at coil surface [kg/kg]

        if (OperationMode == FlagCondMode) {
            // IU Cooling: OperationMode 0

            if (m_air <= 0) {
                TeTc = this->CondensingTemp;
                ShowSevereMessage(" Unreasonable outdoor unit airflow rate (" + TrimSigDigits(m_air, 3) + " ) for \"" + this->Name + "\":");
                ShowContinueError(" This cannot be used to calculate outdoor unit refrigerant temperature.");
                ShowContinueError(" Default condensing temperature is used: " + TrimSigDigits(TeTc, 3));
            }

            BF = this->RateBFOUCond; // 0.219;
            T_coil_out = T_coil_in + Q_coil / 1005.0 / m_air;
            T_coil_surf = T_coil_in + (T_coil_out - T_coil_in) / (1 - BF);

            deltaT = this->C3Tc * pow_2(SHSC) + this->C2Tc * SHSC + this->C1Tc;

            TeTc = T_coil_surf + deltaT;

        } else if (OperationMode == FlagEvapMode) {
            // IU Heating: OperationMode 1

            if (m_air <= 0) {
                TeTc = this->EvaporatingTemp;
                ShowSevereMessage(" Unreasonable outdoor unit airflow rate (" + TrimSigDigits(m_air, 3) + " ) for \"" + this->Name + "\":");
                ShowContinueError(" This cannot be used to calculate outdoor unit refrigerant temperature.");
                ShowContinueError(" Default condensing temperature is used: " + TrimSigDigits(TeTc, 3));
            }

            BF = this->RateBFOUEvap; // 0.45581;
            h_coil_in = PsyHFnTdbW(T_coil_in, W_coil_in);
            h_coil_out = h_coil_in - Q_coil / m_air / (1 - BF);
            h_coil_out = max(0.01, h_coil_out);

            T_coil_surf_sat = PsyTsatFnHPb(h_coil_out, OutdoorPressure, "VRFOU_TeTc");
            W_coil_surf_sat = PsyWFnTdbH(T_coil_surf_sat, h_coil_out, "VRFOU_TeTc");

            if (W_coil_surf_sat < W_coil_in)
                // There is dehumidification
                T_coil_surf = T_coil_surf_sat;
            else
                // No dehumidification
                T_coil_surf = PsyTdbFnHW(h_coil_out, W_coil_in);

            deltaT = this->C3Te * pow_2(SHSC) + this->C2Te * SHSC + this->C1Te;

            TeTc = T_coil_surf - deltaT;
        }
    }

    Real64 VRFCondenserEquipment::VRFOU_Cap(int const OperationMode, // Mode 0 for running as condenser, 1 for evaporator
                                            Real64 const TeTc,       // VRF Tc at cooling mode, or Te at heating mode [C]
                                            Real64 const SHSC,       // SC for OU condenser or SH for OU evaporator [C]
                                            Real64 const m_air,      // OU coil air mass flow rate [kg/s]
                                            Real64 const T_coil_in,  // Temperature of air at OU coil inlet [C]
                                            Real64 const W_coil_in   // Humidity ratio of air at OU coil inlet [kg/kg]
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Rongpeng Zhang, LBNL
        //       DATE WRITTEN   Jan 2016
        //       MODIFIED       na
        //
        //       RE-ENGINEERED  na
        //
        // PURPOSE OF THIS SUBROUTINE:
        //        Calculate the VRF OU load, given refrigerant side temperature, i.e., condensing temperature
        //        and SC for condenser, or evaporating temperature and SH for evaporator.
        //
        // METHODOLOGY EMPLOYED:
        //        This is part of the physics based VRF model applicable for Fluid Temperature Control.

        using DataEnvironment::OutBaroPress;
        using General::TrimSigDigits;

        Real64 BF;              // VRF OU bypass [-]
        Real64 deltaT;          // Difference between Te/Tc and air temperature at coil surface [C]
        Real64 h_coil_in;       // Enthalpy of air at OU coil inlet [C]
        Real64 h_coil_out;      // Enthalpy of air at OU coil outlet [C]
        Real64 Q_coil;          // OU coil heat release at cooling mode or heat extract at heating mode [W]
        Real64 T_coil_out;      // Air temperature at coil outlet [C]
        Real64 T_coil_surf;     // Air temperature at coil surface [C]
        Real64 W_coil_surf_sat; // Humidity ratio of saturated air at coil surface [kg/kg]

        Q_coil = 0.0;

        if (OperationMode == FlagCondMode) {
            // IU Cooling: OperationMode 0
            if (m_air <= 0) {
                ShowSevereMessage(" Unreasonable outdoor unit airflow rate (" + TrimSigDigits(m_air, 3) + " ) for \"" + this->Name + "\":");
                ShowContinueError(" This cannot be used to calculate outdoor unit capacity.");
            }

            BF = this->RateBFOUCond; // 0.219;
            deltaT = this->C3Tc * pow_2(SHSC) + this->C2Tc * SHSC + this->C1Tc;
            T_coil_surf = TeTc - deltaT;
            T_coil_out = T_coil_in + (T_coil_surf - T_coil_in) * (1 - BF);
            Q_coil = (T_coil_out - T_coil_in) * 1005.0 * m_air;

        } else if (OperationMode == FlagEvapMode) {
            // IU Heating: OperationMode 1
            if (m_air <= 0) {
                ShowSevereMessage(" Unreasonable outdoor unit airflow rate (" + TrimSigDigits(m_air, 3) + " ) for \"" + this->Name + "\":");
                ShowContinueError(" This cannot be used to calculate outdoor unit capacity.");
            }

            BF = this->RateBFOUEvap; // 0.45581;
            deltaT = this->C3Te * pow_2(SHSC) + this->C2Te * SHSC + this->C1Te;
            T_coil_surf = TeTc + deltaT;

            // saturated humidity ratio corresponding to T_coil_surf
            W_coil_surf_sat = PsyWFnTdpPb(T_coil_surf, OutBaroPress);

            if (W_coil_surf_sat < W_coil_in) {
                // There is dehumidification, W_coil_out = W_coil_surf_sat
                h_coil_out = PsyHFnTdbW(T_coil_surf, W_coil_surf_sat);
            } else {
                // No dehumidification, W_coil_out = W_coil_in
                h_coil_out = PsyHFnTdbW(T_coil_surf, W_coil_in);
            }
            h_coil_out = max(0.01, h_coil_out);
            h_coil_in = PsyHFnTdbW(T_coil_in, W_coil_in);
            Q_coil = (h_coil_in - h_coil_out) * m_air * (1 - BF); // bypass airflow should not be included here

        } else {
            // Should not come here
            ShowSevereMessage(" Unreasonable outdoor unit operational mode for \"" + this->Name + "\":");
            ShowContinueError(" The operational mode is not correctly set in the function VRFOU_Cap.");
        }

        return Q_coil;
    }

    Real64 VRFCondenserEquipment::VRFOU_FlowRate(int const OperationMode, // Mode 0 for running as condenser, 1 for evaporator
                                                 Real64 const TeTc,       // VRF Tc at cooling mode, or Te at heating mode [C]
                                                 Real64 const SHSC,       // SC for OU condenser or SH for OU evaporator [C]
                                                 Real64 const Q_coil,     // absolute value of OU coil heat release or heat extract [W]
                                                 Real64 const T_coil_in,  // Temperature of air at OU coil inlet [C]
                                                 Real64 const W_coil_in   // Humidity ratio of air at OU coil inlet [kg/kg]
    ) const
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Rongpeng Zhang, LBNL
        //       DATE WRITTEN   Mar 2016
        //       MODIFIED       na
        //
        //       RE-ENGINEERED  na
        //
        // PURPOSE OF THIS SUBROUTINE:
        //        Calculate the outdoor unit fan flow rate, given VRF OU load and refrigerant side temperature, i.e.,
        //        condensing temperature and SC for condenser, or evaporating temperature and SH for evaporator.
        //
        // METHODOLOGY EMPLOYED:
        //        This is part of the physics based VRF model applicable for Fluid Temperature Control.

        using DataEnvironment::OutBaroPress;
        using General::TrimSigDigits;

        Real64 BF;              // VRF OU bypass [-]
        Real64 deltaT;          // Difference between Te/Tc and air temperature at coil surface [C]
        Real64 h_coil_in;       // Enthalpy of air at OU coil inlet [C]
        Real64 h_coil_out;      // Enthalpy of air at OU coil outlet [C]
        Real64 m_air;           // OU coil air mass flow rate [kg/s]
        Real64 T_coil_out;      // Air temperature at coil outlet [C]
        Real64 T_coil_surf;     // Air temperature at coil surface [C]
        Real64 W_coil_surf_sat; // Humidity ratio of saturated air at coil surface [kg/kg]

        m_air = 0.0;

        if (OperationMode == FlagCondMode) {
            // IU Cooling: OperationMode 0

            BF = this->RateBFOUCond; // 0.219;
            deltaT = this->C3Tc * pow_2(SHSC) + this->C2Tc * SHSC + this->C1Tc;
            T_coil_surf = TeTc - deltaT;
            T_coil_out = T_coil_in + (T_coil_surf - T_coil_in) * (1 - BF);
            m_air = Q_coil / (T_coil_out - T_coil_in) / 1005.0;

        } else if (OperationMode == FlagEvapMode) {
            // IU Heating: OperationMode 1

            BF = this->RateBFOUEvap; // 0.45581;
            deltaT = this->C3Te * pow_2(SHSC) + this->C2Te * SHSC + this->C1Te;
            T_coil_surf = TeTc + deltaT;

            // saturated humidity ratio corresponding to T_coil_surf
            W_coil_surf_sat = PsyWFnTdpPb(T_coil_surf, OutBaroPress);

            if (W_coil_surf_sat < W_coil_in) {
                // There is dehumidification, W_coil_out = W_coil_surf_sat
                h_coil_out = PsyHFnTdbW(T_coil_surf, W_coil_surf_sat);
            } else {
                // No dehumidification, W_coil_out = W_coil_in
                h_coil_out = PsyHFnTdbW(T_coil_surf, W_coil_in);
            }
            h_coil_out = max(0.01, h_coil_out);
            h_coil_in = PsyHFnTdbW(T_coil_in, W_coil_in);
            m_air = Q_coil / (h_coil_in - h_coil_out) / (1 - BF);

        } else {
            // Should not come here
            ShowSevereMessage(" Unreasonable outdoor unit operational mode for \"" + this->Name + "\":");
            ShowContinueError(" The operational mode is not correctly set in the function VRFOU_Cap.");
        }

        return m_air;
    }

    Real64 VRFCondenserEquipment::VRFOU_SCSH(int const OperationMode, // Mode 0 for running as condenser, 1 for evaporator
                                             Real64 const Q_coil,     // // OU coil heat release at cooling mode or heat extract at heating mode [W]
                                             Real64 const TeTc,       // VRF Tc at cooling mode, or Te at heating mode [C]
                                             Real64 const m_air,      // OU coil air mass flow rate [kg/s]
                                             Real64 const T_coil_in,  // Temperature of air at OU coil inlet [C]
                                             Real64 const W_coil_in,  // Humidity ratio of air at OU coil inlet [kg/kg]
                                             Real64 const OutdoorPressure // Outdoor air pressure [Pa]
    ) const
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Rongpeng Zhang, LBNL
        //       DATE WRITTEN   Jan 2016
        //       MODIFIED       na
        //
        //       RE-ENGINEERED  na
        //
        // PURPOSE OF THIS SUBROUTINE:
        //        Calculate the SC for OU condenser, or SH for OU evaporator, given
        //        VRF OU load and refrigerant side temperature, i.e., condensing temperature
        //        for condenser, or evaporating temperature for evaporator.
        //
        // METHODOLOGY EMPLOYED:
        //        This is part of the physics based VRF model applicable for Fluid Temperature Control.

        using DataEnvironment::OutBaroPress;
        using General::TrimSigDigits;

        Real64 BF;              // VRF OU bypass [-]
        Real64 deltaT;          // Difference between Te/Tc and air temperature at coil surface [C]
        Real64 h_coil_in;       // Enthalpy of air at OU coil inlet [C]
        Real64 h_coil_out;      // Enthalpy of air at OU coil outlet [C]
        Real64 SHSC;            // SC for OU condenser, or SH for OU evaporator
        Real64 T_coil_out;      // Air temperature at coil outlet [C]
        Real64 T_coil_surf;     // Air temperature at coil surface [C]
        Real64 T_coil_surf_sat; // Saturated air temperature at coil surface [C]
        Real64 W_coil_surf_sat; // Humidity ratio of saturated air at coil surface [kg/kg]

        SHSC = 0.0;

        if (OperationMode == FlagCondMode) {
            // Cooling: OperationMode 0
            if (m_air <= 0) {
                ShowSevereMessage(" Unreasonable outdoor unit airflow rate (" + TrimSigDigits(m_air, 3) + " ) for \"" + this->Name + "\":");
                ShowContinueError(" This cannot be used to calculate outdoor unit subcooling.");
            }

            BF = this->RateBFOUCond; // 0.219;
            T_coil_out = T_coil_in + Q_coil / 1005.0 / m_air;
            T_coil_surf = T_coil_in + (T_coil_out - T_coil_in) / (1 - BF);
            deltaT = TeTc - T_coil_surf;

            // SC_OU
            if (this->C3Tc == 0)
                SHSC = -(this->C1Tc - deltaT) / this->C2Tc;
            else
                SHSC = (-this->C2Tc + std::pow((pow_2(this->C2Tc) - 4 * (this->C1Tc - deltaT) * this->C3Tc), 0.5)) / (2 * this->C3Tc);

        } else if (OperationMode == FlagEvapMode) {
            // Heating: OperationMode 1
            if (m_air <= 0) {
                ShowSevereMessage(" Unreasonable outdoor unit airflow rate (" + TrimSigDigits(m_air, 3) + " ) for \"" + this->Name + "\":");
                ShowContinueError(" This cannot be used to calculate outdoor unit super heating.");
            }

            BF = this->RateBFOUEvap; // 0.45581;
            h_coil_in = PsyHFnTdbW(T_coil_in, W_coil_in);
            h_coil_out = h_coil_in - Q_coil / m_air / (1 - BF);
            h_coil_out = max(0.01, h_coil_out);

            T_coil_surf_sat = PsyTsatFnHPb(h_coil_out, OutdoorPressure, "VRFOU_TeTc");
            W_coil_surf_sat = PsyWFnTdbH(T_coil_surf_sat, h_coil_out, "VRFOU_TeTc");

            if (W_coil_surf_sat < W_coil_in)
                // There is dehumidification
                T_coil_surf = T_coil_surf_sat;
            else
                // No dehumidification
                T_coil_surf = PsyTdbFnHW(h_coil_out, W_coil_in);

            deltaT = T_coil_surf - TeTc;

            // SH_OU
            if (this->C3Te == 0)
                SHSC = -(this->C1Te - deltaT) / this->C2Te;
            else
                SHSC = (-this->C2Te + std::pow((pow_2(this->C2Te) - 4 * (this->C1Te - deltaT) * this->C3Te), 0.5)) / (2 * this->C3Te);

        } else {
            // Should not come here
            ShowSevereMessage(" Unreasonable outdoor unit operational mode for \"" + this->Name + "\":");
            ShowContinueError(" The operational mode is not correctly set in the function VRFOU_Cap.");
        }

        return SHSC;
    }

    Real64 VRFCondenserEquipment::VRFOU_CapModFactor(
        EnergyPlusData &state,
        Real64 const h_comp_in_real, // Enthalpy of refrigerant at the compressor inlet at real conditions [kJ/kg]
        Real64 const h_evap_in_real, // Enthalpy of refrigerant at the evaporator inlet at real conditions [kJ/kg]
        Real64 const P_evap_real,    // Evaporative pressure at real conditions [Pa]
        Real64 const T_comp_in_real, // Temperature of the refrigerant at the compressor inlet at real conditions [C]
        Real64 const T_comp_in_rate, // Temperature of the refrigerant at the compressor inlet at rated conditions [C]
        Real64 const T_cond_out_rate // Temperature of the refrigerant at the condenser outlet at rated conditions [C]
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Rongpeng Zhang
        //       DATE WRITTEN   Nov 2015
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Calculate capacity modification factor for the compressors at Outdoor Unit.
        // This factor is used to modify the system evaporative capacity, by describing
        // the difference between rated conditions and real conditions.

        // METHODOLOGY EMPLOYED:
        // This is part of the VRF-FluidTCtrl Model.

        using FluidProperties::FindRefrigerant;
        using FluidProperties::GetSatEnthalpyRefrig;
        using FluidProperties::GetSatTemperatureRefrig;
        using FluidProperties::GetSupHeatDensityRefrig;
        using FluidProperties::GetSupHeatEnthalpyRefrig;

        int RefrigerantIndex;   // Index of the refrigerant [-]
        Real64 C_cap_density;   // Compressor capacity modification algorithm_modified flow rate [-]
        Real64 C_cap_enthalpy;  // Compressor capacity modification algorithm_modified enthalpy difference [-]
        Real64 C_cap_operation; // Compressor capacity modification algorithm_modified Cap [-]
        Real64 RefTSat;         // Saturated temperature of the refrigerant. Used to check whether the refrigerant is in the superheat area [C].
        Real64 h_evap_out_rate; // enthalpy of refrigerant at the evaporator outlet at rated conditions [kJ/kg]
        Real64 h_evap_in_rate;  // enthalpy of refrigerant at the evaporator inlet at rated conditions [kJ/kg]
        Real64 density_rate;    // density of refrigerant at rated conditions [kg/m3]
        Real64 density_real;    // density of refrigerant at rated conditions [kg/m3]

        static std::string const RoutineName("VRFOU_CapModFactor");

        // variable initializations
        RefrigerantIndex = FindRefrigerant(state, this->RefrigerantName);

        // Saturated temperature at real evaporating pressure
        RefTSat = GetSatTemperatureRefrig(state, this->RefrigerantName, P_evap_real, RefrigerantIndex, RoutineName);

        // Enthalpy at rated conditions
        h_evap_out_rate = GetSupHeatEnthalpyRefrig(state, this->RefrigerantName, max(RefTSat, T_comp_in_rate), P_evap_real, RefrigerantIndex, RoutineName);
        h_evap_in_rate = GetSatEnthalpyRefrig(state, this->RefrigerantName, T_cond_out_rate, 0.0, RefrigerantIndex, RoutineName);

        // Density calculations
        density_rate = GetSupHeatDensityRefrig(state, this->RefrigerantName, T_comp_in_rate, P_evap_real, RefrigerantIndex, RoutineName);
        density_real = GetSupHeatDensityRefrig(state, this->RefrigerantName, T_comp_in_real, P_evap_real, RefrigerantIndex, RoutineName);

        // Modification factor calculations
        if (density_real > 0)
            C_cap_density = density_rate / density_real;
        else
            C_cap_density = 1.0;

        if ((h_comp_in_real - h_evap_in_real) > 0)
            C_cap_enthalpy = std::abs(h_evap_out_rate - h_evap_in_rate) / std::abs(h_comp_in_real - h_evap_in_real);
        else
            C_cap_enthalpy = 1.0;

        C_cap_operation = C_cap_density * C_cap_enthalpy;

        return C_cap_operation;
    }

    void VRFCondenserEquipment::VRFOU_TeModification(
        EnergyPlusData &state,
        Real64 const Te_up,          // Upper bound of Te during iteration, i.e., Te before reduction [C]
        Real64 const Te_low,         // Lower bound of Te during iteration, i.e., the given suction temperature Te' [C]
        Real64 const Pipe_h_IU_in,   // Piping Loss Algorithm Parameter: enthalpy of IU at inlet [kJ/kg]
        Real64 const OutdoorDryBulb, // outdoor dry-bulb temperature [C]
        Real64 &Te_update,           // Updated Te that can generate the required Tsuction [C]
        Real64 &Pe_update,           // Piping Loss Algorithm Parameter: evaporating pressure assumed for iterations [Pa]
        Real64 &Pipe_m_ref,          // Piping Loss Algorithm Parameter: Refrigerant mass flow rate [kg/s]
        Real64 &Pipe_h_IU_out,       // Piping Loss Algorithm Parameter: enthalpy of IU at outlet [kJ/kg]
        Real64 &Pipe_SH_merged       // Piping Loss Algorithm Parameter: Average SH after the indoor units [C]
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Rongpeng Zhang
        //       DATE WRITTEN   Jan 2016
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This is part of the low load modification algorithm for the VRF-FluidTCtrl model. It aims
        // to find a new Te (Te_update) that can generate a new compressor suction temperature (Tsuction) equalling
        // to the given compressor suction temperature (Te_low). This requires the re-calculate of piping loss.

        // METHODOLOGY EMPLOYED:
        // This is part of the VRF-FluidTCtrl Model.

        using DXCoils::DXCoil;
        using FluidProperties::FindRefrigerant;
        using FluidProperties::GetSatPressureRefrig;
        using FluidProperties::GetSatTemperatureRefrig;
        using FluidProperties::GetSupHeatEnthalpyRefrig;
        using FluidProperties::RefrigData;

        int CoolCoilIndex;      // index to cooling coil in terminal unit
        int NumTUInList;        // number of terminal units is list
        int NumTeIte;           // counter for Te calculation iterations [-]
        int RefrigerantIndex;   // Index of the refrigerant [-]
        int TUListNum;          // index to TU List
        int TUIndex;            // Index to terminal unit
        Real64 MaxNumTeIte;     // Piping Loss Algorithm Parameter: max number of iterations for Te [-]
        Real64 Pipe_h_comp_in;  // Piping Loss Algorithm Parameter: Enthalpy after piping loss (compressor inlet) [kJ/kg]
        Real64 Pipe_DeltP;      // Piping Loss Algorithm Parameter: Pipe pressure drop [Pa]
        Real64 Pipe_Q;          // Piping Loss Algorithm Parameter: Heat loss [W]
        Real64 Pipe_m_ref_i;    // Piping Loss Algorithm Parameter: Refrigerant mass flow rate for a individual IU[kg/s]
        Real64 Pipe_h_IU_out_i; // Piping Loss Algorithm Parameter: enthalpy of IU at outlet (individual) [kJ/kg]
        Real64 RefTSat;         // Saturated temperature of the refrigerant [C]
        Real64 RefPLow;         // Low Pressure Value for Ps (>0.0) [Pa]
        Real64 RefPHigh;        // High Pressure Value for Ps (max in tables) [Pa]
        Real64 SH_IU_update;    // Modified SH for VRF IU [C]
        Real64 Te_ItePreci;     // Precision of iterations for Te [C]he superheat area [C]
        Real64 Tfs;             // Temperature of the air at the coil surface [C]]
        Real64 Tsuction;        // VRF compressor suction refrigerant temperature [Pa]

        static std::string const RoutineName("VRFOU_TeModification");

        // variable initializations
        TUListNum = this->ZoneTUListPtr;
        RefrigerantIndex = FindRefrigerant(state, this->RefrigerantName);
        RefPLow = RefrigData(RefrigerantIndex).PsLowPresValue;
        RefPHigh = RefrigData(RefrigerantIndex).PsHighPresValue;
        NumTUInList = TerminalUnitList(TUListNum).NumTUInList;

        // Initialization of Te iterations (Label11)
        NumTeIte = 1;
        Te_ItePreci = 0.1;
        MaxNumTeIte = (Te_up - Te_low) / Te_ItePreci + 1; // upper bound and lower bound of Te iterations
        Te_update = Te_up - Te_ItePreci;

    Label11:;
        Pipe_m_ref = 0; // Total Ref Flow Rate( kg/s )
        Pipe_h_IU_out = 0;
        Pipe_h_IU_out_i = 0;
        Pipe_m_ref_i = 0;
        Pipe_SH_merged = 0;
        Pe_update = GetSatPressureRefrig(state, this->RefrigerantName, Te_update, RefrigerantIndex, RoutineName);

        // Re-calculate total refrigerant flow rate, with updated SH
        for (int NumTU = 1; NumTU <= NumTUInList; NumTU++) {
            if (TerminalUnitList(TUListNum).TotalCoolLoad(NumTU) > 0) {
                TUIndex = TerminalUnitList(TUListNum).ZoneTUPtr(NumTU);
                CoolCoilIndex = VRFTU(TUIndex).CoolCoilIndex;

                // The IU coil surface temperature should be the same.
                Tfs = Te_up + (this->C3Te * pow_2(DXCoil(CoolCoilIndex).ActualSH) + this->C2Te * DXCoil(CoolCoilIndex).ActualSH + this->C1Te);

                // SH_IU_update is the updated SH for a specific IU
                if (this->C3Te == 0)
                    SH_IU_update = -(this->C1Te - Tfs + Te_update) / this->C2Te;
                else
                    SH_IU_update =
                        (-this->C2Te + std::pow((pow_2(this->C2Te) - 4 * (this->C1Te - Tfs + Te_update) * this->C3Te), 0.5)) / (2 * this->C3Te);

                RefTSat = GetSatTemperatureRefrig(state, this->RefrigerantName, Pe_update, RefrigerantIndex, RoutineName);
                Pipe_h_IU_out_i = GetSupHeatEnthalpyRefrig(state,
                                                           this->RefrigerantName,
                                                           max(RefTSat, Te_update + SH_IU_update),
                                                           Pe_update,
                                                           RefrigerantIndex,
                                                           RoutineName); // hB_i for the IU

                if (Pipe_h_IU_out_i > Pipe_h_IU_in) {
                    Pipe_m_ref_i = (TerminalUnitList(TUListNum).TotalCoolLoad(NumTU) <= 0.0)
                                       ? 0.0
                                       : (TerminalUnitList(TUListNum).TotalCoolLoad(NumTU) / (Pipe_h_IU_out_i - Pipe_h_IU_in));
                    Pipe_m_ref = Pipe_m_ref + Pipe_m_ref_i;
                    Pipe_SH_merged = Pipe_SH_merged + Pipe_m_ref_i * SH_IU_update;
                    Pipe_h_IU_out = Pipe_h_IU_out + Pipe_m_ref_i * Pipe_h_IU_out_i;
                }
            }
        }
        if (Pipe_m_ref > 0) {
            Pipe_h_IU_out = Pipe_h_IU_out / Pipe_m_ref;
            Pipe_SH_merged = Pipe_SH_merged / Pipe_m_ref;
        } else {
            Pipe_SH_merged = this->SH;
            RefTSat = GetSatTemperatureRefrig(state, this->RefrigerantName, Pe_update, RefrigerantIndex, RoutineName);
            Pipe_h_IU_out =
                GetSupHeatEnthalpyRefrig(state, this->RefrigerantName, max(RefTSat, Te_update + Pipe_SH_merged), Pe_update, RefrigerantIndex, RoutineName);
        }

        // Re-calculate piping loss
        this->VRFOU_PipeLossC(state, Pipe_m_ref, Pe_update, Pipe_h_IU_out, Pipe_SH_merged, OutdoorDryBulb, Pipe_Q, Pipe_DeltP, Pipe_h_comp_in);

        Tsuction = GetSatTemperatureRefrig(state, this->RefrigerantName, max(min(Pe_update - Pipe_DeltP, RefPHigh), RefPLow), RefrigerantIndex, RoutineName);

        if ((std::abs(Tsuction - Te_low) > 0.5) && (Te_update < Te_up) && (Te_update > Te_low) && (NumTeIte < MaxNumTeIte)) {
            Te_update = Te_update - 0.1;
            NumTeIte = NumTeIte + 1;
            goto Label11;
        }

        if (std::abs(Tsuction - Te_low) > 0.5) {
            NumTeIte = 999;
            Tsuction = Te_low;
            Pipe_SH_merged = 3.0;
            Te_update = Te_low + 1;
        }
    }

    void VRFCondenserEquipment::VRFOU_CompSpd(
        EnergyPlusData &state,
        Real64 const Q_req,        // Required capacity [W]
        int const Q_type,          // Required capacity type:  0 for condenser, 1 for evaporator
        Real64 const T_suction,    // Compressor suction temperature Te' [C]
        Real64 const T_discharge,  // Compressor discharge temperature Tc' [C]
        Real64 const h_IU_evap_in, // Enthalpy of IU at inlet, for C_cap_operation calculation [kJ/kg]
        Real64 const h_comp_in,    // Enthalpy after piping loss (compressor inlet), for C_cap_operation calculation [kJ/kg]
        Real64 &CompSpdActual      // Actual compressor running speed [rps]
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Rongpeng Zhang, LBNL
        //       DATE WRITTEN   Feb 2016
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        //       This subroutine specifies the compressor speed at given operational conditions to meet the evaporator or condenser capacity provided.

        // METHODOLOGY EMPLOYED:
        //        This is part of the VRF-FluidTCtrl Model.

        using CurveManager::CurveValue;
        using FluidProperties::FindRefrigerant;
        using FluidProperties::GetSatPressureRefrig;
        using FluidProperties::GetSupHeatTempRefrig;
        using FluidProperties::RefrigData;

        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:

        // SUBROUTINE PARAMETER DEFINITIONS:
        int CounterCompSpdTemp;                // Index for the compressor speed level[-]
        int CompSpdLB;                         // index for Compressor speed low bound [-]
        int CompSpdUB;                         // index for Compressor speed up bound [-]
        int NumOfCompSpdInput;                 // Number of compressor speed input by the user [-]
        int NumTUInList;                       // number of terminal units is list
        int RefrigerantIndex;                  // Index of the refrigerant
        int TUListNum;                         // index to TU List
        Real64 C_cap_operation;                // Compressor capacity modification algorithm_modified Cap [-]
        Real64 P_suction;                      // Compressor suction pressure Pe' [Pa]
        Real64 Q_evap_req;                     // Required evaporative capacity [W]
        Real64 Q_cond_req;                     // Required evaporative capacity [W]
        Real64 RefPLow;                        // Low Pressure Value for Ps (>0.0) [Pa]
        Real64 RefPHigh;                       // High Pressure Value for Ps (max in tables) [Pa]
        Real64 SH_Comp;                        // Temperature between compressor inlet temperature and evaporative temperature Te' [C]
        Real64 T_comp_in;                      // Refrigerant temperature at compressor inlet (after piping loss) [C]
        Array1D<Real64> CompEvaporatingPWRSpd; // Array for the compressor power at certain speed [W]
        Array1D<Real64> CompEvaporatingCAPSpd; // Array for the evaporating capacity at certain speed [W]

        static std::string const RoutineName("VRFOU_CompSpd");

        // variable initializations: component index
        TUListNum = this->ZoneTUListPtr;
        NumTUInList = TerminalUnitList(TUListNum).NumTUInList;
        RefrigerantIndex = FindRefrigerant(state, this->RefrigerantName);
        RefPLow = RefrigData(RefrigerantIndex).PsLowPresValue;
        RefPHigh = RefrigData(RefrigerantIndex).PsHighPresValue;

        // variable initializations: compressor
        NumOfCompSpdInput = this->CompressorSpeed.size();
        CompEvaporatingPWRSpd.dimension(NumOfCompSpdInput);
        CompEvaporatingCAPSpd.dimension(NumOfCompSpdInput);

        // variable initializations: system operational parameters
        P_suction = GetSatPressureRefrig(state, this->RefrigerantName, T_suction, RefrigerantIndex, RoutineName);
        T_comp_in = GetSupHeatTempRefrig(
            state, this->RefrigerantName, max(min(P_suction, RefPHigh), RefPLow), h_comp_in, T_suction + 3, T_suction + 30, RefrigerantIndex, RoutineName);
        SH_Comp = T_comp_in - T_suction;

        // Calculate capacity modification factor
        C_cap_operation = this->VRFOU_CapModFactor(
            state, h_comp_in, h_IU_evap_in, max(min(P_suction, RefPHigh), RefPLow), T_suction + SH_Comp, T_suction + 8, T_discharge - 5);

        if (Q_type == FlagEvapMode) {
            // Capacity to meet is for evaporator

            Q_evap_req = Q_req;

            for (CounterCompSpdTemp = 1; CounterCompSpdTemp <= NumOfCompSpdInput; CounterCompSpdTemp++) {
                // Iteration to find the VRF speed that can meet the required load, Iteration DoName1

                CompEvaporatingPWRSpd(CounterCompSpdTemp) =
                    this->RatedCompPower * CurveValue(state, this->OUCoolingPWRFT(CounterCompSpdTemp), T_discharge, T_suction);
                CompEvaporatingCAPSpd(CounterCompSpdTemp) =
                    this->CoffEvapCap * this->RatedEvapCapacity * CurveValue(state, this->OUCoolingCAPFT(CounterCompSpdTemp), T_discharge, T_suction);

                if (Q_evap_req * C_cap_operation <= CompEvaporatingCAPSpd(CounterCompSpdTemp)) {
                    // Compressor speed stage CounterCompSpdTemp need not to be increased, finish Iteration DoName1

                    if (CounterCompSpdTemp > 1) {

                        CompSpdLB = CounterCompSpdTemp - 1;
                        CompSpdUB = CounterCompSpdTemp;

                        CompSpdActual = this->CompressorSpeed(CompSpdLB) + (this->CompressorSpeed(CompSpdUB) - this->CompressorSpeed(CompSpdLB)) /
                                                                               (CompEvaporatingCAPSpd(CompSpdUB) - CompEvaporatingCAPSpd(CompSpdLB)) *
                                                                               (Q_evap_req * C_cap_operation - CompEvaporatingCAPSpd(CompSpdLB));

                    } else {
                        CompSpdActual = this->CompressorSpeed(1) * (Q_evap_req * C_cap_operation) / CompEvaporatingCAPSpd(1);
                    }

                    break; // EXIT DoName1
                }
            } // End: Iteration DoName1

            if (CounterCompSpdTemp > NumOfCompSpdInput) {
                CompSpdActual = this->CompressorSpeed(NumOfCompSpdInput);
            }

        } else {
            // Capacity to meet is for condenser

            Q_cond_req = Q_req;

            for (CounterCompSpdTemp = 1; CounterCompSpdTemp <= NumOfCompSpdInput; CounterCompSpdTemp++) {
                // Iteration to find the VRF speed that can meet the required load, Iteration DoName1

                CompEvaporatingPWRSpd(CounterCompSpdTemp) =
                    this->RatedCompPower * CurveValue(state, this->OUCoolingPWRFT(CounterCompSpdTemp), T_discharge, T_suction);
                CompEvaporatingCAPSpd(CounterCompSpdTemp) =
                    this->CoffEvapCap * this->RatedEvapCapacity * CurveValue(state, this->OUCoolingCAPFT(CounterCompSpdTemp), T_discharge, T_suction);

                Q_evap_req = Q_cond_req - CompEvaporatingPWRSpd(CounterCompSpdTemp);

                if (Q_evap_req * C_cap_operation <= CompEvaporatingCAPSpd(CounterCompSpdTemp)) {
                    // Compressor speed stage CounterCompSpdTemp need not to be increased, finish Iteration DoName1

                    if (CounterCompSpdTemp > 1) {

                        CompSpdLB = CounterCompSpdTemp - 1;
                        CompSpdUB = CounterCompSpdTemp;

                        CompSpdActual = this->CompressorSpeed(CompSpdLB) + (this->CompressorSpeed(CompSpdUB) - this->CompressorSpeed(CompSpdLB)) /
                                                                               (CompEvaporatingCAPSpd(CompSpdUB) - CompEvaporatingCAPSpd(CompSpdLB)) *
                                                                               (Q_evap_req * C_cap_operation - CompEvaporatingCAPSpd(CompSpdLB));

                    } else {
                        CompSpdActual = this->CompressorSpeed(1) * (Q_evap_req * C_cap_operation) / CompEvaporatingCAPSpd(1);
                    }

                    break; // EXIT DoName1
                }
            } // End: Iteration DoName1

            if (CounterCompSpdTemp > NumOfCompSpdInput) {
                CompSpdActual = this->CompressorSpeed(NumOfCompSpdInput);
            }
        }
    }

    void VRFCondenserEquipment::VRFOU_CompCap(
        EnergyPlusData &state,
        int const CompSpdActual,   // Given compressor speed
        Real64 const T_suction,    // Compressor suction temperature Te' [C]
        Real64 const T_discharge,  // Compressor discharge temperature Tc' [C]
        Real64 const h_IU_evap_in, // Enthalpy of IU at inlet, for C_cap_operation calculation [kJ/kg]
        Real64 const h_comp_in,    // Enthalpy after piping loss (compressor inlet), for C_cap_operation calculation [kJ/kg]
        Real64 &Q_c_tot,           // Compressor evaporative capacity [W]
        Real64 &Ncomp              // Compressor power [W]
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Rongpeng Zhang, LBNL
        //       DATE WRITTEN   Feb 2016
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        //       This subroutine specifies the compressor performance (power and capacity) at given compressor speed and operational conditions.

        // METHODOLOGY EMPLOYED:
        //       This is part of the VRF-FluidTCtrl Model.

        using CurveManager::CurveValue;
        using FluidProperties::FindRefrigerant;
        using FluidProperties::GetSatPressureRefrig;
        using FluidProperties::GetSupHeatTempRefrig;
        using FluidProperties::RefrigData;

        int CounterCompSpdTemp;                // Index for the compressor speed level[-]
        int CompSpdLB;                         // index for Compressor speed low bound [-]
        int CompSpdUB;                         // index for Compressor speed up bound [-]
        int NumOfCompSpdInput;                 // Number of compressor speed input by the user [-]
        int NumTUInList;                       // number of terminal units is list
        int RefrigerantIndex;                  // Index of the refrigerant
        int TUListNum;                         // index to TU List
        Real64 C_cap_operation;                // Compressor capacity modification algorithm_modified Cap [-]
        Real64 P_suction;                      // Compressor suction pressure Pe' [Pa]
        Real64 Q_evap_sys;                     // evaporative capacity [W]
        Real64 RefPLow;                        // Low Pressure Value for Ps (>0.0) [Pa]
        Real64 RefPHigh;                       // High Pressure Value for Ps (max in tables) [Pa]
        Real64 SH_Comp;                        // Temperature between compressor inlet temperature and evaporative temperature Te' [C]
        Real64 T_comp_in;                      // Refrigerant temperature at compressor inlet (after piping loss) [C]
        Array1D<Real64> CompEvaporatingPWRSpd; // Array for the compressor power at certain speed [W]
        Array1D<Real64> CompEvaporatingCAPSpd; // Array for the evaporating capacity at certain speed [W]

        static std::string const RoutineName("VRFOU_CompCap");

        // variable initializations: component index
        TUListNum = this->ZoneTUListPtr;
        NumTUInList = TerminalUnitList(TUListNum).NumTUInList;
        RefrigerantIndex = FindRefrigerant(state, this->RefrigerantName);
        RefPLow = RefrigData(RefrigerantIndex).PsLowPresValue;
        RefPHigh = RefrigData(RefrigerantIndex).PsHighPresValue;

        // variable initializations: compressor
        NumOfCompSpdInput = this->CompressorSpeed.size();
        CompEvaporatingPWRSpd.dimension(NumOfCompSpdInput);
        CompEvaporatingCAPSpd.dimension(NumOfCompSpdInput);

        for (CounterCompSpdTemp = 1; CounterCompSpdTemp <= NumOfCompSpdInput; CounterCompSpdTemp++) {

            CompEvaporatingPWRSpd(CounterCompSpdTemp) =
                this->RatedCompPower * CurveValue(state, this->OUCoolingPWRFT(CounterCompSpdTemp), T_discharge, T_suction);
            CompEvaporatingCAPSpd(CounterCompSpdTemp) =
                this->CoffEvapCap * this->RatedEvapCapacity * CurveValue(state, this->OUCoolingCAPFT(CounterCompSpdTemp), T_discharge, T_suction);

            if (CompSpdActual <= this->CompressorSpeed(CounterCompSpdTemp)) {
                // Compressor speed stage CounterCompSpdTemp need not to be increased, finish Iteration DoName1

                if (CounterCompSpdTemp > 1) {

                    CompSpdLB = CounterCompSpdTemp - 1;
                    CompSpdUB = CounterCompSpdTemp;

                    Q_evap_sys = CompEvaporatingCAPSpd(CompSpdLB) + (CompEvaporatingCAPSpd(CompSpdUB) - CompEvaporatingCAPSpd(CompSpdLB)) *
                                                                        (CompSpdActual - this->CompressorSpeed(CompSpdLB)) /
                                                                        (this->CompressorSpeed(CompSpdUB) - this->CompressorSpeed(CompSpdLB));
                    Ncomp = CompEvaporatingPWRSpd(CompSpdLB) + (CompEvaporatingPWRSpd(CompSpdUB) - CompEvaporatingPWRSpd(CompSpdLB)) *
                                                                   (CompSpdActual - this->CompressorSpeed(CompSpdLB)) /
                                                                   (this->CompressorSpeed(CompSpdUB) - this->CompressorSpeed(CompSpdLB));

                } else {
                    Q_evap_sys = CompEvaporatingCAPSpd(1) * CompSpdActual / this->CompressorSpeed(1);
                    Ncomp = CompEvaporatingPWRSpd(1) * CompSpdActual / this->CompressorSpeed(1);
                }

                break;
            }
        }

        if (CounterCompSpdTemp > NumOfCompSpdInput) {
            Q_evap_sys = CompEvaporatingCAPSpd(NumOfCompSpdInput);
            Ncomp = CompEvaporatingPWRSpd(NumOfCompSpdInput);
        }

        // variable initializations: system operational parameters
        P_suction = GetSatPressureRefrig(state, this->RefrigerantName, T_suction, RefrigerantIndex, RoutineName);
        T_comp_in = GetSupHeatTempRefrig(
            state, this->RefrigerantName, max(min(P_suction, RefPHigh), RefPLow), h_comp_in, T_suction + 3, T_suction + 30, RefrigerantIndex, RoutineName);
        SH_Comp = T_comp_in - T_suction;

        // Calculate capacity modification factor
        C_cap_operation = this->VRFOU_CapModFactor(
            state, h_comp_in, h_IU_evap_in, max(min(P_suction, RefPHigh), RefPLow), T_suction + SH_Comp, T_suction + 8, T_discharge - 5);
        C_cap_operation = min(1.5, max(0.5, C_cap_operation));
        Q_c_tot = Q_evap_sys / C_cap_operation;
    }

    void VRFCondenserEquipment::VRFOU_CalcCompC(EnergyPlusData &state,
                                                Real64 TU_load,            // Indoor unit cooling load [W]
                                                Real64 T_suction,          // Compressor suction temperature Te' [C]
                                                Real64 T_discharge,        // Compressor discharge temperature Tc' [C]
                                                Real64 P_suction,          // Compressor suction pressure Pe' [Pa]
                                                Real64 Pipe_T_comp_in,     // Refrigerant temperature at compressor inlet (after piping loss) [C]
                                                Real64 Pipe_h_comp_in,     // Enthalpy after piping loss (compressor inlet) [kJ/kg]
                                                Real64 Pipe_h_IU_in,       // Enthalpy of IU at inlet [kJ/kg]
                                                Real64 Pipe_Q,             // Piping Loss Algorithm Parameter: Heat loss [W]
                                                Real64 MaxOutdoorUnitTc,   // The maximum temperature that Tc can be at heating mode [C]
                                                Real64 &OUCondHeatRelease, // Condenser heat release (cooling mode) [W]
                                                Real64 &CompSpdActual,     // Actual compressor running speed [rps]
                                                Real64 &Ncomp              // Compressor power [W]
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Xiufeng Pang
        //       DATE WRITTEN   Feb 2014
        //       MODIFIED       Rongpeng Zhang, Jan 2016
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine simulates the compressor performance at given oprtaional conditions (cooling mode). More specifically, it sepcifies
        // the compressor speed to provide sufficient evaporative capacity, and calculate the power of the compressor running at the specified
        // speed. Note that it may be needed to manipulate the operational conditions to further adjust system capacity at low load conditions.
        // The low load modification logics are different for cooling mode and heating mode.

        // METHODOLOGY EMPLOYED:
        // This is part of the VRF-FluidTCtrl Model.

        using CurveManager::CurveValue;
        using DataEnvironment::OutBaroPress;
        using DataEnvironment::OutDryBulbTemp;
        using DataEnvironment::OutHumRat;
        using DXCoils::DXCoil;
        using FluidProperties::FindRefrigerant;
        using FluidProperties::GetSatEnthalpyRefrig;
        using FluidProperties::GetSatPressureRefrig;
        using FluidProperties::GetSatTemperatureRefrig;
        using FluidProperties::GetSupHeatEnthalpyRefrig;
        using FluidProperties::GetSupHeatTempRefrig;
        using FluidProperties::RefrigData;
        using General::SolveRoot;
        using TempSolveRoot::SolveRoot;

        int CounterCompSpdTemp;                // Index for the compressor speed level[-]
        int CompSpdLB;                         // index for Compressor speed low bound [-]
        int CompSpdUB;                         // index for Compressor speed up bound [-]
        int CoolCoilIndex;                     // index to cooling coil in terminal unit
        int MaxIter(500);                      // max iteration number allowed [-]
        int NumOfCompSpdInput;                 // Number of compressor speed input by the user [-]
        int NumIteCcap;                        // counter for Ccap calculation iterations [-]
        int NumIteTe;                          // counter for Te calculation iterations [-]
        int NumTUInList;                       // number of terminal units is list
        int RefrigerantIndex;                  // Index of the refrigerant [-]
        int SolFla;                            // Slove flag for SolveRoot [-]
        int TUListNum;                         // index to TU List
        int TUIndex;                           // Index to terminal unit
        Real64 Cap_Eva0;                       // Evaporating capacity calculated based on physics model, used in the iterations [W]
        Real64 Cap_Eva1;                       // Evaporating capacity calculated by curves, used in the iterations [W]
        Real64 CapDiff;                        // Evaporating capacity difference used in the iterations [W]
        Real64 C_cap_operation;                // Compressor capacity modification algorithm_modified Cap [-]
        Real64 C_cap_operation0;               // Compressor capacity modification algorithm_modified Cap, for temporary use [-]
        Real64 SmallLoadTe;                    // Updated suction temperature at small load conditions (Te') [C]
        Real64 Modifi_SH;                      // Temperature between compressor inlet temperature and evaporative temperature Te' [C]
        Real64 MaxNumIteTe;                    // Piping Loss Algorithm Parameter: max number of iterations for Te [-]
        Real64 MinOutdoorUnitTe;               // The minimum temperature that Te can be at cooling mode (only used for calculating Min capacity)
        Real64 MinOutdoorUnitPe;               // The minimum pressure that Pe can be at cooling mode (only used for calculating Min capacity)
        Real64 MinRefriPe;                     // Minimum refrigerant evaporating pressure [Pa]
        Real64 Modifi_SHin;                    // Compressor power modification algorithm_modified SH for IDU [C]
        Real64 P_discharge;                    // VRF compressor discharge pressure [Pa]
        Real64 Pipe_m_ref;                     // Piping Loss Algorithm Parameter: Refrigerant mass flow rate [kg/s]
        Real64 Pipe_DeltP;                     // Piping Loss Algorithm Parameter: Pipe pressure drop [Pa]
        Real64 Pipe_Q0;                        // Compressor capacity modification algorithm_modified Pipe_Q, for temporary use [W]
        Real64 Pipe_m_ref_i;                   // Piping Loss Algorithm Parameter: Refrigerant mass flow rate for a individual IU[kg/s]
        Real64 Pipe_h_IU_out;                  // Piping Loss Algorithm Parameter: enthalpy of IU at outlet [kJ/kg]
        Real64 Pipe_h_IU_out_i;                // Piping Loss Algorithm Parameter: enthalpy of IU at outlet (individual) [kJ/kg]
        Real64 Pipe_Pe_assumed;                // Piping Loss Algorithm Parameter: evaporating pressure assumed for iterations[Pa]
        Real64 Pipe_SH_merged;                 // Piping Loss Algorithm Parameter: average super heating degrees after the indoor units [C]
        Real64 Pipe_Te_assumed;                // Piping Loss Algorithm Parameter: evaporating temperature assumed for iterations[C]
        Real64 Q_evap_req;                     // Required evaporative capacity [W]
        Real64 RefTSat;                        // Saturated temperature of the refrigerant [C]
        Real64 RefPLow;                        // Low Pressure Value for Ps (>0.0) [Pa]
        Real64 RefPHigh;                       // High Pressure Value for Ps (max in tables) [Pa]
        Real64 T_discharge_new;                // Condensing temperature, for temporary use in iterations [C]
        Real64 Tfs;                            // Temperature of the air at the coil surface [C]]
        Real64 Tolerance(0.05);                // Tolerance for condensing temperature calculation [C}
        Array1D<Real64> CompEvaporatingPWRSpd; // Array for the compressor power at certain speed [W]
        Array1D<Real64> CompEvaporatingCAPSpd; // Array for the evaporating capacity at certain speed [W]
        Array1D<Real64> Par(3);                // Array for the parameters [-]

        static std::string const RoutineName("VRFOU_CalcCompC");

        // variable initializations
        NumOfCompSpdInput = this->CompressorSpeed.size();
        CompEvaporatingPWRSpd.dimension(NumOfCompSpdInput);
        CompEvaporatingCAPSpd.dimension(NumOfCompSpdInput);
        Q_evap_req = TU_load + Pipe_Q;

        TUListNum = this->ZoneTUListPtr;
        RefrigerantIndex = FindRefrigerant(state, this->RefrigerantName);
        RefPLow = RefrigData(RefrigerantIndex).PsLowPresValue;
        RefPHigh = RefrigData(RefrigerantIndex).PsHighPresValue;
        NumTUInList = TerminalUnitList(TUListNum).NumTUInList;

        Modifi_SH = Pipe_T_comp_in - T_suction;

        // set condenser entering air conditions (Outdoor air conditions)
        Real64 OutdoorDryBulb = OutDryBulbTemp;
        Real64 OutdoorHumRat = OutHumRat;
        Real64 OutdoorPressure = OutBaroPress;
        Real64 RhoAir = PsyRhoAirFnPbTdbW(OutdoorPressure, OutdoorDryBulb, OutdoorHumRat);

        // Calculate capacity modification factor
        C_cap_operation = this->VRFOU_CapModFactor(
            state, Pipe_h_comp_in, Pipe_h_IU_in, max(min(P_suction, RefPHigh), RefPLow), T_suction + Modifi_SH, T_suction + 8, T_discharge - 5);

        for (CounterCompSpdTemp = 1; CounterCompSpdTemp <= NumOfCompSpdInput; CounterCompSpdTemp++) {
            // Iteration to find the VRF speed that can meet the required load, Iteration DoName1

            CompEvaporatingPWRSpd(CounterCompSpdTemp) =
                this->RatedCompPower * CurveValue(state, this->OUCoolingPWRFT(CounterCompSpdTemp), T_discharge, T_suction);
            CompEvaporatingCAPSpd(CounterCompSpdTemp) =
                this->CoffEvapCap * this->RatedEvapCapacity * CurveValue(state, this->OUCoolingCAPFT(CounterCompSpdTemp), T_discharge, T_suction);

            if (Q_evap_req * C_cap_operation <= CompEvaporatingCAPSpd(CounterCompSpdTemp)) {
                // Compressor speed stage CounterCompSpdTemp need not to be increased, finish Iteration DoName1

                if (CounterCompSpdTemp > 1) { // Since: if( CounterCompSpdTemp <= 1 )
                    // Compressor speed > min

                    CompSpdLB = CounterCompSpdTemp - 1;
                    CompSpdUB = CounterCompSpdTemp;

                    CompSpdActual = this->CompressorSpeed(CompSpdLB) + (this->CompressorSpeed(CompSpdUB) - this->CompressorSpeed(CompSpdLB)) /
                                                                           (CompEvaporatingCAPSpd(CompSpdUB) - CompEvaporatingCAPSpd(CompSpdLB)) *
                                                                           (Q_evap_req * C_cap_operation - CompEvaporatingCAPSpd(CompSpdLB));

                    Ncomp = CompEvaporatingPWRSpd(CompSpdLB) + (CompEvaporatingPWRSpd(CompSpdUB) - CompEvaporatingPWRSpd(CompSpdLB)) /
                                                                   (this->CompressorSpeed(CompSpdUB) - this->CompressorSpeed(CompSpdLB)) *
                                                                   (CompSpdActual - this->CompressorSpeed(CompSpdLB));
                    break; // EXIT DoName1

                } else {
                    // Compressor runs at the min speed
                    // Low Load Modification Algorithm for cooling (IU side modification)

                    // Initialization of NumIteCcap iterations (Label13)
                    Pipe_Q0 = Pipe_Q;
                    C_cap_operation0 = C_cap_operation;
                    T_discharge_new = T_discharge;
                    NumIteCcap = 1;

                // Update the C_cap_operation
                Label13:;
                    Q_evap_req = TU_load + Pipe_Q0; // Pipe_Q0 is updated during the iteration
                    Pipe_h_IU_in = GetSatEnthalpyRefrig(state, this->RefrigerantName, T_discharge_new - this->SC, 0.0, RefrigerantIndex, RoutineName);
                    CompSpdActual = this->CompressorSpeed(1);
                    Par(1) = T_discharge_new;
                    Par(2) = Q_evap_req * C_cap_operation0 / this->RatedEvapCapacity; // 150130 To be confirmed
                    Par(3) = this->OUCoolingCAPFT(CounterCompSpdTemp);

                    // Update Te' (SmallLoadTe) to meet the required evaporator capacity
                    MinOutdoorUnitTe = 6;
                    P_discharge = GetSatPressureRefrig(state, this->RefrigerantName, T_discharge, RefrigerantIndex, RoutineName);

                    MinRefriPe = GetSatPressureRefrig(state, this->RefrigerantName, -15, RefrigerantIndex, RoutineName);
                    MinOutdoorUnitPe = max(P_discharge - this->CompMaxDeltaP, MinRefriPe);
                    MinOutdoorUnitTe =
                        GetSatTemperatureRefrig(state, this->RefrigerantName, max(min(MinOutdoorUnitPe, RefPHigh), RefPLow), RefrigerantIndex, RoutineName);

                    TempSolveRoot::SolveRoot(state,
                                             1.0e-3,
                                             MaxIter,
                                             SolFla,
                                             SmallLoadTe,
                                             CompResidual_FluidTCtrl,
                                             MinOutdoorUnitTe,
                                             T_suction,
                                             Par);                  // SmallLoadTe is the updated Te'
                    if (SolFla < 0) SmallLoadTe = 6; // MinOutdoorUnitTe; //SmallLoadTe( Te'_new ) is constant during iterations

                    // Get an updated Te corresponding to the updated Te'
                    // VRFOU_TeModification( VRFCond, this->EvaporatingTemp, SmallLoadTe, Pipe_h_IU_in, OutdoorDryBulb, Pipe_Te_assumed,
                    // Pipe_Pe_assumed, Pipe_m_ref, Pipe_SH_merged );
                    {
                        // Initialization of Iteration_Te (Label11)
                        // i.e., find a new Te (Pipe_Te_assumed) that can generate a new T_suction equalling to SmallLoadTe.
                        // This requires the re-calculate of piping loss.
                        NumIteTe = 1;
                        MaxNumIteTe = (this->EvaporatingTemp - SmallLoadTe) / 0.1 + 1; // upper bound and lower bound of Te iterations
                        Pipe_Te_assumed = this->EvaporatingTemp - 0.1;

                    Label11:;
                        Pipe_m_ref = 0; // Total Ref Flow Rate( kg/s )

                        // Re-calculate Piping loss due to the Te and SH updates
                        Pipe_h_IU_out = 0;
                        Pipe_h_IU_out_i = 0;
                        Pipe_m_ref_i = 0;
                        Pipe_SH_merged = 0;
                        Pipe_Pe_assumed = GetSatPressureRefrig(state, this->RefrigerantName, Pipe_Te_assumed, RefrigerantIndex, RoutineName);

                        // Re-calculate total refrigerant flow rate, with updated SH
                        for (int NumTU = 1; NumTU <= NumTUInList; NumTU++) {
                            if (TerminalUnitList(TUListNum).TotalCoolLoad(NumTU) > 0) {
                                TUIndex = TerminalUnitList(TUListNum).ZoneTUPtr(NumTU);
                                CoolCoilIndex = VRFTU(TUIndex).CoolCoilIndex;

                                Tfs = this->EvaporatingTemp +
                                      (this->C3Te * pow_2(DXCoil(CoolCoilIndex).ActualSH) + this->C2Te * DXCoil(CoolCoilIndex).ActualSH + this->C1Te);

                                // Modifi_SH is the updated SH for a specific IU
                                if (this->C3Te == 0)
                                    Modifi_SHin = -(this->C1Te - Tfs + Pipe_Te_assumed) / this->C2Te; // 150130 Modifi_SH>Modifi_SHin
                                else
                                    Modifi_SHin =
                                        (-this->C2Te + std::pow((pow_2(this->C2Te) - 4 * (this->C1Te - Tfs + Pipe_Te_assumed) * this->C3Te), 0.5)) /
                                        (2 * this->C3Te);

                                RefTSat = GetSatTemperatureRefrig(state,
                                    this->RefrigerantName, max(min(Pipe_Pe_assumed, RefPHigh), RefPLow), RefrigerantIndex, RoutineName);
                                Pipe_h_IU_out_i = GetSupHeatEnthalpyRefrig(state,
                                                                           this->RefrigerantName,
                                                                           max(RefTSat, Pipe_Te_assumed + Modifi_SHin),
                                                                           max(min(Pipe_Pe_assumed, RefPHigh), RefPLow),
                                                                           RefrigerantIndex,
                                                                           RoutineName);

                                if (Pipe_h_IU_out_i > Pipe_h_IU_in) {
                                    Pipe_m_ref_i = (TerminalUnitList(TUListNum).TotalCoolLoad(NumTU) <= 0.0)
                                                       ? 0.0
                                                       : (TerminalUnitList(TUListNum).TotalCoolLoad(NumTU) / (Pipe_h_IU_out_i - Pipe_h_IU_in));
                                    Pipe_m_ref = Pipe_m_ref + Pipe_m_ref_i;
                                    Pipe_SH_merged = Pipe_SH_merged + Pipe_m_ref_i * Modifi_SHin;
                                    Pipe_h_IU_out = Pipe_h_IU_out + Pipe_m_ref_i * Pipe_h_IU_out_i;
                                }
                            }
                        }
                        if (Pipe_m_ref > 0) {
                            Pipe_h_IU_out = Pipe_h_IU_out / Pipe_m_ref;
                            Pipe_SH_merged = Pipe_SH_merged / Pipe_m_ref;
                        } else {
                            Pipe_SH_merged = this->SH;
                            RefTSat = GetSatTemperatureRefrig(state,
                                this->RefrigerantName, max(min(Pipe_Pe_assumed, RefPHigh), RefPLow), RefrigerantIndex, RoutineName);
                            Pipe_h_IU_out = GetSupHeatEnthalpyRefrig(state,
                                                                     this->RefrigerantName,
                                                                     max(RefTSat, Pipe_Te_assumed + Pipe_SH_merged),
                                                                     max(min(Pipe_Pe_assumed, RefPHigh), RefPLow),
                                                                     RefrigerantIndex,
                                                                     RoutineName);
                        }

                        // Re-calculate piping loss
                        this->VRFOU_PipeLossC(state,
                                              Pipe_m_ref,
                                              max(min(Pipe_Pe_assumed, RefPHigh), RefPLow),
                                              Pipe_h_IU_out,
                                              Pipe_SH_merged,
                                              OutdoorDryBulb,
                                              Pipe_Q,
                                              Pipe_DeltP,
                                              Pipe_h_comp_in);

                        T_suction = GetSatTemperatureRefrig(state,
                            this->RefrigerantName, max(min(Pipe_Pe_assumed - Pipe_DeltP, RefPHigh), RefPLow), RefrigerantIndex, RoutineName);

                        if ((std::abs(T_suction - SmallLoadTe) > 0.5) && (Pipe_Te_assumed < this->EvaporatingTemp) &&
                            (Pipe_Te_assumed > SmallLoadTe) && (NumIteTe < MaxNumIteTe)) {
                            Pipe_Te_assumed = Pipe_Te_assumed - 0.1;
                            NumIteTe = NumIteTe + 1;
                            goto Label11;
                        }

                        if (std::abs(T_suction - SmallLoadTe) > 0.5) {
                            NumIteTe = 999;
                            T_suction = SmallLoadTe;
                            Pipe_SH_merged = 3.0;
                            Pipe_Te_assumed = SmallLoadTe + 1;
                        }
                        // Iteration_Te End
                    }

                    // Perform iteration to calculate Pipe_T_comp_in( Te'+SH' )
                    Pipe_T_comp_in = GetSupHeatTempRefrig(state,
                                                          this->RefrigerantName,
                                                          max(min(Pipe_Pe_assumed - Pipe_DeltP, RefPHigh), RefPLow),
                                                          Pipe_h_comp_in,
                                                          T_suction + 3,
                                                          T_suction + 30,
                                                          RefrigerantIndex,
                                                          RoutineName);

                    Modifi_SH = Pipe_T_comp_in - T_suction;
                    P_suction = Pipe_Pe_assumed - Pipe_DeltP;
                    OUCondHeatRelease = TU_load + Pipe_Q + Ncomp; // Pipe_Q is changed when T_suction is changed -> Tc is also changed

                    // *VRF OU Tc calculations
                    this->VRFOU_TeTc(FlagCondMode,
                                     OUCondHeatRelease,
                                     this->SC,
                                     this->OUAirFlowRate * RhoAir,
                                     OutdoorDryBulb,
                                     OutdoorHumRat,
                                     OutdoorPressure,
                                     Tfs,
                                     T_discharge);
                    T_discharge = min(MaxOutdoorUnitTc, T_discharge);

                    // *Calculate capacity modification factor
                    C_cap_operation = this->VRFOU_CapModFactor(
                        state, Pipe_h_comp_in, Pipe_h_IU_in, max(min(P_suction, RefPHigh), RefPLow), T_suction + Modifi_SH, T_suction + 8, T_discharge - 5);

                    Cap_Eva0 = (TU_load + Pipe_Q) * C_cap_operation; // New Pipe_Q & C_cap_operation
                    Cap_Eva1 = this->CoffEvapCap * this->RatedEvapCapacity *
                               CurveValue(state, this->OUCoolingCAPFT(CounterCompSpdTemp), T_discharge, T_suction); // New Tc
                    CapDiff = std::abs(Cap_Eva1 - Cap_Eva0);

                    if ((CapDiff > (Tolerance * Cap_Eva0)) && (NumIteCcap < 30)) {
                        Pipe_Q0 = Pipe_Q;
                        C_cap_operation0 = C_cap_operation;
                        T_discharge_new = T_discharge;
                        NumIteCcap = NumIteCcap + 1;
                        goto Label13;
                    }

                    if (CapDiff > (Tolerance * Cap_Eva0)) NumIteCcap = 999;

                    Ncomp = this->RatedCompPower * CurveValue(state, this->OUCoolingPWRFT(CounterCompSpdTemp), T_discharge, T_suction);

                    this->CondensingTemp = T_discharge; // OU Tc' is updated due to OUCondHeatRelease updates, which is caused by IU Te' updates
                                                        // during low load conditions

                    break; // EXIT DoName1

                } // End: if( CounterCompSpdTemp <= 1 ) Low load modification

            } // End: if( Q_evap_req <= CompEvaporatingCAPSpd( CounterCompSpdTemp ) )

        } // End: Iteration DoName1

        if (CounterCompSpdTemp > NumOfCompSpdInput) {
            // Required load is beyond the maximum system capacity
            CompEvaporatingCAPSpd(NumOfCompSpdInput) =
                this->CoffEvapCap * this->RatedEvapCapacity * CurveValue(state, this->OUCoolingCAPFT(NumOfCompSpdInput), T_discharge, T_suction);
            OUCondHeatRelease = Ncomp + CompEvaporatingCAPSpd(NumOfCompSpdInput);
            CompSpdActual = this->CompressorSpeed(NumOfCompSpdInput);
            Ncomp = CompEvaporatingPWRSpd(NumOfCompSpdInput);
        }
    }

    void VRFCondenserEquipment::VRFOU_CalcCompH(
        EnergyPlusData &state,
        Real64 TU_load,            // Indoor unit heating load [W]
        Real64 T_suction,          // Compressor suction temperature Te' [C]
        Real64 T_discharge,        // Compressor discharge temperature Tc' [C]
        Real64 Pipe_h_out_ave,     // Average Enthalpy of the refrigerant leaving IUs [kJ/kg]
        Real64 IUMaxCondTemp,      // VRV IU condensing temperature, max among all indoor units [C]
        Real64 MinOutdoorUnitTe,   // The minimum temperature that OU Te can be at cooling mode (only used for calculating Min capacity)
        Real64 Tfs,                // Temperature of the air at the OU evaporator coil surface [C]]
        Real64 Pipe_Q,             // Piping Loss Algorithm Parameter: Heat loss [W]
        Real64 &OUEvapHeatExtract, // Condenser heat release (cooling mode) [W]
        Real64 &CompSpdActual,     // Actual compressor running speed [rps]
        Real64 &Ncomp              // Compressor power [W]
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Xiufeng Pang
        //       DATE WRITTEN   Feb 2014
        //       MODIFIED       Rongpeng Zhang, Jan 2016
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine simulates the compressor performance at given proportional conditions (heating mode). More specifically, it specifies
        // the compressor speed to provide sufficient evaporative capacity, and calculate the power of the compressor running at the specified
        // speed. Note that it may be needed to manipulate the operational conditions to further adjust system capacity at low load conditions.
        // The low load modification logics are different for cooling mode and heating mode.

        // METHODOLOGY EMPLOYED:
        // This is part of the VRF-FluidTCtrl Model.

        using CurveManager::CurveValue;
        using DataEnvironment::OutBaroPress;
        using DataEnvironment::OutDryBulbTemp;
        using DataEnvironment::OutHumRat;
        using DXCoils::DXCoil;
        using FluidProperties::FindRefrigerant;
        using FluidProperties::GetSatEnthalpyRefrig;
        using FluidProperties::GetSatPressureRefrig;
        using FluidProperties::GetSatTemperatureRefrig;
        using FluidProperties::GetSupHeatEnthalpyRefrig;
        using FluidProperties::GetSupHeatTempRefrig;
        using FluidProperties::RefrigData;
        using General::SolveRoot;
        using TempSolveRoot::SolveRoot;

        int CounterCompSpdTemp;                // Index for the compressor speed level[-]
        int CompSpdLB;                         // index for Compressor speed low bound [-]
        int CompSpdUB;                         // index for Compressor speed up bound [-]
        int MaxIter(500);                      // max iteration number allowed [-]
        int NumOfCompSpdInput;                 // Number of compressor speed input by the user [-]
        int NumIteCcap;                        // counter for Ccap calculation iterations [-]
        int NumTUInList;                       // number of terminal units is list
        int RefrigerantIndex;                  // Index of the refrigerant [-]
        int SolFla;                            // Solve flag for SolveRoot [-]
        int TUListNum;                         // index to TU List
        Real64 Cap_Eva0;                       // Evaporating capacity calculated based on physics model, used in the iterations [W]
        Real64 Cap_Eva1;                       // Evaporating capacity calculated by curves, used in the iterations [W]
        Real64 CapDiff;                        // Evaporating capacity difference used in the iterations [W]
        Real64 C_cap_operation;                // Compressor capacity modification algorithm_modified Cap [-]
        Real64 SmallLoadTe;                    // Updated suction temperature at small load conditions (Te') [C]
        Real64 Modifi_SH;                      // Temperature between compressor inlet temperature and evaporative temperature Te' [C]
        Real64 MinOutdoorUnitPe;               // The minimum pressure that Pe can be at cooling mode (only used for calculating Min capacity)
        Real64 Modifi_Pe;                      // Compressor power modification algorithm_modified Pe [Pa]
        Real64 Pipe_h_comp_in;                 // Piping Loss Algorithm Parameter: Enthalpy after piping loss (compressor inlet) [kJ/kg]
        Real64 Q_evap_req;                     // Required evaporative capacity [W]
        Real64 RefTSat;                        // Saturated temperature of the refrigerant [C]
        Real64 RefPLow;                        // Low Pressure Value for Ps (>0.0) [Pa]
        Real64 RefPHigh;                       // High Pressure Value for Ps (max in tables) [Pa]
        Real64 Tolerance(0.05);                // Tolerance for condensing temperature calculation [C}
        Array1D<Real64> CompEvaporatingPWRSpd; // Array for the compressor power at certain speed [W]
        Array1D<Real64> CompEvaporatingCAPSpd; // Array for the evaporating capacity at certain speed [W]
        Array1D<Real64> Par(3);                // Array for the parameters [-]

        static std::string const RoutineName("VRFOU_CalcCompH");

        // variable initializations
        NumOfCompSpdInput = this->CompressorSpeed.size();
        CompEvaporatingPWRSpd.dimension(NumOfCompSpdInput);
        CompEvaporatingCAPSpd.dimension(NumOfCompSpdInput);
        Q_evap_req = TU_load + Pipe_Q - Ncomp;

        TUListNum = this->ZoneTUListPtr;
        RefrigerantIndex = FindRefrigerant(state, this->RefrigerantName);
        RefPLow = RefrigData(RefrigerantIndex).PsLowPresValue;
        RefPHigh = RefrigData(RefrigerantIndex).PsHighPresValue;
        NumTUInList = TerminalUnitList(TUListNum).NumTUInList;

        // Calculate capacity modification factor
        MinOutdoorUnitPe = GetSatPressureRefrig(state, this->RefrigerantName, T_suction, RefrigerantIndex, RoutineName);
        RefTSat = GetSatTemperatureRefrig(state, this->RefrigerantName, max(min(MinOutdoorUnitPe, RefPHigh), RefPLow), RefrigerantIndex, RoutineName);
        Pipe_h_comp_in = GetSupHeatEnthalpyRefrig(
            state, this->RefrigerantName, max(RefTSat, T_suction + this->SH), max(min(MinOutdoorUnitPe, RefPHigh), RefPLow), RefrigerantIndex, RoutineName);
        C_cap_operation = this->VRFOU_CapModFactor(
            state, Pipe_h_comp_in, Pipe_h_out_ave, max(min(MinOutdoorUnitPe, RefPHigh), RefPLow), T_suction + this->SH, T_suction + 8, IUMaxCondTemp - 5);

        // Perform iterations to find the compressor speed that can meet the required heating load, Iteration DoName2
        for (CounterCompSpdTemp = 1; CounterCompSpdTemp <= NumOfCompSpdInput; CounterCompSpdTemp++) {

            CompEvaporatingPWRSpd(CounterCompSpdTemp) =
                this->RatedCompPower * CurveValue(state, this->OUCoolingPWRFT(CounterCompSpdTemp), T_discharge, T_suction);
            CompEvaporatingCAPSpd(CounterCompSpdTemp) =
                this->CoffEvapCap * this->RatedEvapCapacity * CurveValue(state, this->OUCoolingCAPFT(CounterCompSpdTemp), T_discharge, T_suction);

            if ((Q_evap_req * C_cap_operation) <= CompEvaporatingCAPSpd(CounterCompSpdTemp)) {
                // Compressor Capacity is greater than the required, finish Iteration DoName2

                if (CounterCompSpdTemp > 1) {
                    // Compressor runs at higher speed than min speed
                    CompSpdLB = CounterCompSpdTemp - 1;
                    CompSpdUB = CounterCompSpdTemp;

                    CompSpdActual = this->CompressorSpeed(CompSpdLB) + (this->CompressorSpeed(CompSpdUB) - this->CompressorSpeed(CompSpdLB)) /
                                                                           (CompEvaporatingCAPSpd(CompSpdUB) - CompEvaporatingCAPSpd(CompSpdLB)) *
                                                                           (Q_evap_req * C_cap_operation - CompEvaporatingCAPSpd(CompSpdLB));
                    Modifi_SH = this->SH;
                    Ncomp = CompEvaporatingPWRSpd(CompSpdLB) + (CompEvaporatingPWRSpd(CompSpdUB) - CompEvaporatingPWRSpd(CompSpdLB)) /
                                                                   (this->CompressorSpeed(CompSpdUB) - this->CompressorSpeed(CompSpdLB)) *
                                                                   (CompSpdActual - this->CompressorSpeed(CompSpdLB));

                    break; // EXIT DoName2

                } else {
                    // Compressor runs at the min speed
                    // Low Load Modifications

                    NumIteCcap = 1;
                Label19:;
                    Q_evap_req = max(0.0, TU_load + Pipe_Q - Ncomp);

                    // Update Te'( SmallLoadTe ) to meet the required evaporator capacity
                    CompSpdActual = this->CompressorSpeed(1);
                    Par(1) = T_discharge;
                    Par(2) = Q_evap_req * C_cap_operation / this->RatedEvapCapacity;
                    Par(3) = this->OUCoolingCAPFT(CounterCompSpdTemp);

                    TempSolveRoot::SolveRoot(state, 1.0e-3, MaxIter, SolFla, SmallLoadTe, CompResidual_FluidTCtrl, MinOutdoorUnitTe, T_suction, Par);
                    if (SolFla < 0) SmallLoadTe = MinOutdoorUnitTe;

                    T_suction = SmallLoadTe;

                    // Update SH and Pe to calculate Modification Factor, which is used to update rps to for N_comp calculations
                    if (this->C3Te == 0)
                        Modifi_SH = -(this->C1Te - Tfs + T_suction) / this->C2Te;
                    else
                        Modifi_SH =
                            (-this->C2Te + std::pow((pow_2(this->C2Te) - 4 * (this->C1Te - Tfs + T_suction) * this->C3Te), 0.5)) / (2 * this->C3Te);

                    Modifi_Pe = GetSatPressureRefrig(state, this->RefrigerantName, T_suction, RefrigerantIndex, RoutineName);

                    // Calculate capacity modification factor
                    RefTSat = GetSatTemperatureRefrig(state, this->RefrigerantName, max(min(Modifi_Pe, RefPHigh), RefPLow), RefrigerantIndex, RoutineName);
                    Pipe_h_comp_in = GetSupHeatEnthalpyRefrig(state,
                                                              this->RefrigerantName,
                                                              max(RefTSat, T_suction + Modifi_SH),
                                                              max(min(Modifi_Pe, RefPHigh), RefPLow),
                                                              RefrigerantIndex,
                                                              RoutineName);
                    C_cap_operation = this->VRFOU_CapModFactor(state,
                                                               Pipe_h_comp_in,
                                                               Pipe_h_out_ave,
                                                               max(min(Modifi_Pe, RefPHigh), RefPLow),
                                                               T_suction + Modifi_SH,
                                                               T_suction + 8,
                                                               IUMaxCondTemp - 5);

                    Cap_Eva0 = Q_evap_req * C_cap_operation;
                    Cap_Eva1 =
                        this->CoffEvapCap * this->RatedEvapCapacity * CurveValue(state, this->OUCoolingCAPFT(CounterCompSpdTemp), T_discharge, T_suction);
                    CapDiff = std::abs(Cap_Eva1 - Cap_Eva0);

                    if ((CapDiff > (Tolerance * Cap_Eva0)) && (NumIteCcap < 30)) {
                        NumIteCcap = NumIteCcap + 1;
                        goto Label19;
                    }
                    if (CapDiff > (Tolerance * Cap_Eva0)) NumIteCcap = 999;

                    Ncomp = this->RatedCompPower * CurveValue(state, this->OUCoolingPWRFT(CounterCompSpdTemp), T_discharge, T_suction);

                    break; // EXIT DoName2

                } // End: if( CounterCompSpdTemp <= 1 ) Low load modification

            } // End: if( Q_evap_req <= CompEvaporatingCAPSpd( CounterCompSpdTemp ) )

        } // End: Iteration DoName2

        if (CounterCompSpdTemp > NumOfCompSpdInput) {
            // Required heating load is beyond the maximum system capacity
            CompEvaporatingCAPSpd(NumOfCompSpdInput) =
                this->CoffEvapCap * this->RatedEvapCapacity * CurveValue(state, this->OUCoolingCAPFT(NumOfCompSpdInput), T_discharge, T_suction);
            OUEvapHeatExtract = CompEvaporatingCAPSpd(NumOfCompSpdInput);
            CompSpdActual = this->CompressorSpeed(NumOfCompSpdInput);
            Ncomp = CompEvaporatingPWRSpd(NumOfCompSpdInput);
        }
    }

    void
    VRFCondenserEquipment::VRFHR_OU_HR_Mode(EnergyPlusData &state,
                                            Real64 const h_IU_evap_in, // enthalpy of IU evaporator at inlet [kJ/kg]
                                            Real64 const h_comp_out,   // enthalpy of refrigerant at compressor outlet [kJ/kg]
                                            Real64 const Q_c_TU_PL,    // IU evaporator load, including piping loss [W]
                                            Real64 const Q_h_TU_PL,    // IU condenser load, including piping loss [W]
                                            Real64 const Tdischarge,   // VRF Compressor discharge refrigerant temperature [C]
                                            Real64 &Tsuction,          // VRF compressor suction refrigerant temperature [C]
                                            Real64 &Te_update,         // updated evaporating temperature, only updated when Tsuction is updated [C]
                                            Real64 &h_comp_in,         // enthalpy of refrigerant at compressor inlet [kJ/kg]
                                            Real64 &h_IU_PLc_out,  // enthalpy of refrigerant at the outlet of IU evaporator side main pipe [kJ/kg]
                                            Real64 &Pipe_Q_c,      // IU evaporator side piping loss [W]
                                            Real64 &Q_c_OU,        // OU evaporator load [W]
                                            Real64 &Q_h_OU,        // OU condenser load [W]
                                            Real64 &m_ref_IU_evap, // mass flow rate of Refrigerant through IU evaporators [kg/s]
                                            Real64 &m_ref_OU_evap, // mass flow rate of Refrigerant through OU evaporator [kg/s]
                                            Real64 &m_ref_OU_cond, // mass flow rate of Refrigerant through OU condenser [kg/s]
                                            Real64 &N_fan_OU,      // outdoor unit fan power [W]
                                            Real64 &CompSpdActual, // Actual compressor running speed [rps]
                                            Real64 &Ncomp          // compressor power [W]
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Rongpeng Zhang, LBNL
        //       DATE WRITTEN   Jan 2016
        //       MODIFIED       na
        //
        //       RE-ENGINEERED  na
        //
        // PURPOSE OF THIS SUBROUTINE:
        //        Determine the operational mode of the VRF-HR system, given the terminal unit side load conditions.
        //        Compressor and OU hex performance are analysed for each mode.
        //        A number of OU side operational parameters are also calculated here, including:
        //        (1) OU evaporator load Q_c_OU (2) OU condenser load Q_h_OU (3) OU fan energy consumption
        //        (4) OU compressor speed and energy consumption
        //        Note that Te and Te' may be updated here, and thus IU evaporator side piping loss recalculations.
        //        Then a number of operational parameters need to be updated, including:
        //        (1) IU evaporating temperature Te (2) OU evaporating temperature Te' etc.
        //
        // METHODOLOGY EMPLOYED:
        //        This is part of the physics based VRF model applicable for Fluid Temperature Control.

        using DataEnvironment::OutBaroPress;
        using DataEnvironment::OutDryBulbTemp;
        using DataEnvironment::OutHumRat;
        using FluidProperties::FindRefrigerant;
        using FluidProperties::GetSatEnthalpyRefrig;
        using FluidProperties::GetSatPressureRefrig;
        using FluidProperties::GetSupHeatEnthalpyRefrig;
        using FluidProperties::RefrigData;
        using General::SolveRoot;
        using General::TrimSigDigits;
        using TempSolveRoot::SolveRoot;

        Array1D<Real64> Par(7);     // Parameters passed to RegulaFalsi
        int const FlagCondMode(0);  // Flag for running as condenser [-]
        int const FlagEvapMode(1);  // Flag for running as evaporator [-]
        Real64 const ErrorTol(0.1); // tolerance for RegulaFalsi iterations
        int const MaxIte(100);      // maximum number of iterations
        int HRMode(0);              // HR operational mode [W]
        int HRMode_sub(0);          // HR operational mode (sub) [W]
        int RefrigerantIndex;       // Index of the refrigerant [-]
        int SolFla;                 // Flag of RegulaFalsi solver
        Real64 C_OU_HexRatio;       // capacity ratio between the OU condenser and OU evaporator [-]
        Real64 m_air_rated;         // OU coil air mass flow rate [kg/s]
        Real64 m_air_evap;          // OU evaporator air mass flow rate [kg/s]
        Real64 m_air_cond;          // OU condenser air mass flow rate [kg/s]
        Real64 m_air_evap_rated;    // Rated OU evaporator air mass flow rate [kg/s]
        Real64 N_fan_OU_evap(0);    // OU evaporator air mass flow rate [kg/s]
        Real64 N_fan_OU_cond(0);    // OU condenser air mass flow rate [kg/s]
        Real64 RhoAir;              // outdoor air density [kg/m3]
        Real64 Q_c_tot;             // Total evaporator capacity [W]
        Real64 Q_h_tot;             // Total condenser capacity [W]
        Real64 Pipe_Q_c_new;        // IU evaporator side piping loss (new), updated because of Te update [W]
        Real64 rps1_evap;           // compressor speed satisfying IU cooling load
        Real64 rps2_cond;           // compressor speed satisfying IU heating load
        Real64 RefPLow;             // Low Pressure Value for Ps (>0.0) [Pa]
        Real64 RefPHigh;            // High Pressure Value for Ps (max in tables) [Pa]
        Real64 Tfs;                 // temperature of the air at coil surface [C]
        Real64 Tolerance(0.05);     // Tolerance for condensing temperature calculation [C}
        Real64 Tsuction_new;        // VRF compressor suction refrigerant temperature (new) [C]

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        static std::string const RoutineName("VRFHR_OU_Mode");

        // Initialization: operational parameters
        RhoAir = PsyRhoAirFnPbTdbW(OutBaroPress, OutDryBulbTemp, OutHumRat);
        m_air_rated = this->OUAirFlowRate * RhoAir;
        C_OU_HexRatio = this->HROUHexRatio;

        // Initializations: component index
        RefrigerantIndex = FindRefrigerant(state, this->RefrigerantName);
        RefPLow = RefrigData(RefrigerantIndex).PsLowPresValue;
        RefPHigh = RefrigData(RefrigerantIndex).PsHighPresValue;

        // **Q_OU: HR mode determination
        //	 HRMode-1. Cooling Only
        //	 HRMode-2. Cooling Dominant w/o HR Loss
        //	 HRMode-3. Cooling Dominant w/ HR Loss
        //	 HRMode-4. Heating Dominant w/ HR Loss
        //	 HRMode-5. Heating Dominant w/o HR Loss
        //	 HRMode-6. Heating Only
        //	 HRMode-7. OU Hex not running
        {

            bool FlagMode5;   // true if compressor speed satisfying IU cooling load < that satisfying IU heating load
            bool FlagToLower; // true if To-5 is lower than the Tsuction determined by IU part
            Real64 temp_Tsuction;

            // Determine FlagToLower
            if (OutDryBulbTemp - this->DiffOUTeTo < Tsuction) {
                temp_Tsuction = OutDryBulbTemp - this->DiffOUTeTo;
                FlagToLower = true;
            } else {
                temp_Tsuction = Tsuction;
                FlagToLower = false;
            }

            // Calculate compressor speed satisfying IU loads: rps1_evap & rps2_cond
            this->VRFOU_CompSpd(state, Q_c_TU_PL, FlagEvapMode, temp_Tsuction, Tdischarge, h_IU_evap_in, h_IU_PLc_out, rps1_evap);
            this->VRFOU_CompSpd(state, Q_h_TU_PL, FlagCondMode, temp_Tsuction, Tdischarge, h_IU_evap_in, h_IU_PLc_out, rps2_cond);

            // Determine FlagMode5
            if (rps1_evap <= rps2_cond) {
                FlagMode5 = true;
            } else {
                FlagMode5 = false;
            }

            // Determine HR Mode
            if (FlagMode5) {
                HRMode = 5;
                if (FlagToLower)
                    HRMode_sub = 1;
                else
                    HRMode_sub = 2;
            } else {

                if (FlagToLower)
                    HRMode = 3; // Mode 3&4 share the same logics below
                else
                    HRMode = 2;
            }

            this->VRFOperationSimPath = HRMode * 10 + HRMode_sub;
        }

        // **Simulate outdoor unit and compressor performance, including
        // (1) compressor spd/power (2) OU hex capacity (3) OU fan flow rate and power
        // Tsuction/Te may also need updates
        if (HRMode == 5 && HRMode_sub == 2) {

            CompSpdActual = rps2_cond; // constant in this mode
            // Tsuction = Te'_iu < OutDryBulbTemp - 5; constant in this mode

            // compressor: Ncomp & Q_c_tot
            this->VRFOU_CompCap(state, CompSpdActual, Tsuction, Tdischarge, h_IU_evap_in, h_comp_in, Q_c_tot, Ncomp);

            // OU hex capacity
            Q_c_OU = Q_c_tot - Q_c_TU_PL;
            Q_h_OU = 0;

            // OU fan flow rate and power
            m_air_evap = this->VRFOU_FlowRate(FlagEvapMode, Tsuction, this->SH, Q_c_OU, OutDryBulbTemp, OutHumRat);
            m_air_evap_rated = m_air_rated;
            N_fan_OU_evap = this->RatedOUFanPower * m_air_evap / m_air_evap_rated;
            N_fan_OU_cond = 0;

        } else if (HRMode == 5 && HRMode_sub == 1) {

            // local parameters
            int Counter_Iter_Ncomp;
            bool Flag_Iter_Ncomp(true); // Flag to perform iterations
            Real64 Ncomp_ini;
            Real64 Ncomp_new;
            Real64 Q_c_tot_temp;
            Real64 Q_c_OU_temp;

            //===**Ncomp Iterations

            // initialization: Ncomp_ini, CompSpdActual
            Counter_Iter_Ncomp = 1;
            CompSpdActual = rps2_cond;
            Tsuction_new = OutDryBulbTemp - this->DiffOUTeTo;
            Pipe_Q_c_new = Pipe_Q_c;

            this->VRFOU_CompCap(state, CompSpdActual, Tsuction_new, Tdischarge, h_IU_evap_in, h_comp_in, Q_c_tot, Ncomp_ini);

            while (Flag_Iter_Ncomp) {

                Q_c_tot_temp = Q_h_TU_PL - Ncomp_ini; // Q_h_OU = 0
                Q_c_OU_temp = Q_c_tot_temp - Q_c_TU_PL;

                // Tsuction_new updated based on OU evaporator air-side calculations (Tsuction_new < To)
                m_air_evap_rated = m_air_rated;
                this->VRFOU_TeTc(FlagEvapMode, Q_c_OU_temp, this->SH, m_air_evap_rated, OutDryBulbTemp, OutHumRat, OutBaroPress, Tfs, Tsuction_new);
                Tsuction_new = min(Tsuction_new, Tsuction); // should be lower than Tsuction_IU

                // Calculate updated rps corresponding to updated Tsuction_new and Q_c_tot_temp
                this->VRFOU_CompSpd(state, Q_c_tot_temp, FlagEvapMode, Tsuction_new, Tdischarge, h_IU_evap_in, h_comp_in, CompSpdActual);

                // Calculate Ncomp_new, using updated CompSpdActual and Tsuction_new
                this->VRFOU_CompCap(state, CompSpdActual, Tsuction_new, Tdischarge, h_IU_evap_in, h_comp_in, Q_c_tot_temp, Ncomp_new);

                if ((std::abs(Ncomp_new - Ncomp_ini) > (Tolerance * Ncomp_ini)) && (Counter_Iter_Ncomp < 30)) {
                    Ncomp_ini = 0.5 * Ncomp_ini + 0.5 * Ncomp_new;
                    Counter_Iter_Ncomp = Counter_Iter_Ncomp + 1;
                    continue;
                }

                Flag_Iter_Ncomp = false;
            }

            // Ncomp Iterations Update
            Ncomp = Ncomp_new;
            Q_c_tot = Q_c_tot_temp;

            if (Tsuction_new < Tsuction) {
                // Need to update the Tsuction, and thus update Te_update & Pipe_Q_c_new.
                // Iteration continues.

                // temporary parameters
                Real64 Pe_update;
                Real64 Pipe_SH_merged;
                Real64 Pipe_DeltP;
                Real64 Pipe_h_IU_out;

                // Get an updated Te (Te_update) corresponding to the updated Te' (Tsuction_new). PL_c is re-performed.
                this->VRFOU_TeModification(state,
                                           this->EvaporatingTemp,
                                           Tsuction_new,
                                           h_IU_evap_in,
                                           OutDryBulbTemp,
                                           Te_update,
                                           Pe_update,
                                           m_ref_IU_evap,
                                           Pipe_h_IU_out,
                                           Pipe_SH_merged);

                // Re-calculate piping loss, update Pipe_Q_c_new
                this->VRFOU_PipeLossC(
                    state, m_ref_IU_evap, Pe_update, Pipe_h_IU_out, Pipe_SH_merged, OutDryBulbTemp, Pipe_Q_c_new, Pipe_DeltP, h_IU_PLc_out);

                Tsuction = Tsuction_new;
                Pipe_Q_c = Pipe_Q_c_new;
            }

            // No need to update the Tsuction.

            //===**Ncomp Iteration Ends (Label200)

            // OU hex capacity
            Q_c_OU = Q_c_tot - Q_c_TU_PL;
            Q_h_OU = 0;

            // OU fan power
            N_fan_OU_evap = this->RatedOUFanPower;
            N_fan_OU_cond = 0;

        } else if (HRMode == 3) { // Mode3 & Mode4 share the same algorithm

            // local parameters
            Real64 Ncomp_new;
            Real64 Q_c_tot_temp;
            Real64 Q_c_OU_temp;
            Real64 Tsuction_new;
            Real64 Tsuction_LB = OutDryBulbTemp - this->DiffOUTeTo;
            Real64 Tsuction_HB = Tsuction;

            // compressor speed is fixed in this mode
            CompSpdActual = rps1_evap; // constant in this mode
            m_air_evap_rated = m_air_rated * (1 - C_OU_HexRatio);
            m_air_evap = m_air_evap_rated; // may be updated

            // perform iterations to calculate Te at the given compressor speed and operational conditions
            {
                Par(1) = VRFTU(TerminalUnitList(this->ZoneTUListPtr).ZoneTUPtr(1)).VRFSysNum; // VRFCond;
                Par(2) = CompSpdActual;
                Par(3) = Tdischarge;
                Par(4) = h_IU_evap_in;
                Par(5) = h_comp_in;
                Par(6) = Q_c_TU_PL;
                Par(7) = m_air_evap_rated;

                TempSolveRoot::SolveRoot(state, ErrorTol, MaxIte, SolFla, Tsuction_new, VRFOUTeResidual_FluidTCtrl, Tsuction_LB, Tsuction_HB, Par);
                if (SolFla < 0) Tsuction_new = Tsuction_LB;

                // Update Q_c_tot_temp using updated Tsuction_new
                this->VRFOU_CompCap(state, CompSpdActual, Tsuction_new, Tdischarge, h_IU_evap_in, h_comp_in, Q_c_tot_temp, Ncomp_new);
                Q_c_OU_temp = Q_c_tot_temp - Q_c_TU_PL;

                // Iterations_Te Update
                Ncomp = Ncomp_new;
                Tsuction = Tsuction_new;
                Q_c_tot = Q_c_tot_temp;
                Q_c_OU = Q_c_OU_temp;
            }

            if (Tsuction >= Tsuction_HB) {
                // modify m_air_evap to adjust OU evaporator capacity;
                // update Ncomp, Q_c_OU, m_air_evap

                Tsuction = Tsuction_HB;

                // Q_c_tot
                this->VRFOU_CompCap(state, CompSpdActual, Tsuction_new, Tdischarge, h_IU_evap_in, h_comp_in, Q_c_tot, Ncomp);
                Q_c_OU = Q_c_tot - Q_c_TU_PL;

                // OU evaporator fan flow rate and power
                m_air_evap = this->VRFOU_FlowRate(FlagEvapMode, Tsuction, this->SH, Q_c_OU_temp, OutDryBulbTemp, OutHumRat);

            } else {
                // Need to update Te_update & Pipe_Q_c_new, corresponding to Tsuction update.

                // temporary parameters
                Real64 Pe_update;
                Real64 Pipe_SH_merged;
                Real64 Pipe_DeltP;
                Real64 Pipe_h_IU_out;

                // Get an updated Te (Te_update) corresponding to the updated Te' (Tsuction_new). PL_c is re-performed.
                this->VRFOU_TeModification(state, this->EvaporatingTemp,
                                           Tsuction_new,
                                           h_IU_evap_in,
                                           OutDryBulbTemp,
                                           Te_update,
                                           Pe_update,
                                           m_ref_IU_evap,
                                           Pipe_h_IU_out,
                                           Pipe_SH_merged);

                // Re-calculate piping loss, update Pipe_Q_c_new
                this->VRFOU_PipeLossC(
                    state, m_ref_IU_evap, Pe_update, Pipe_h_IU_out, Pipe_SH_merged, OutDryBulbTemp, Pipe_Q_c_new, Pipe_DeltP, h_IU_PLc_out);
                Pipe_Q_c = Pipe_Q_c_new;
            }

            // Q_h_ou
            Q_h_tot = Q_c_tot + Ncomp;
            Q_h_OU = Q_h_tot - Q_h_TU_PL;

            // OU condenser fan flow rate and power
            m_air_cond = this->VRFOU_FlowRate(FlagCondMode, Tdischarge, this->SC, Q_h_OU, OutDryBulbTemp, OutHumRat);

            // OU fan power
            N_fan_OU_evap = this->RatedOUFanPower * m_air_evap / m_air_rated;
            N_fan_OU_cond = this->RatedOUFanPower * m_air_cond / m_air_rated;

        } else if (HRMode == 2) {

            CompSpdActual = rps1_evap; // constant in this mode
            // Tsuction = Te'_iu < OutDryBulbTemp - 5; constant in this mode

            // compressor: Ncomp & Q_c_tot
            this->VRFOU_CompCap(state, CompSpdActual, Tsuction, Tdischarge, h_IU_evap_in, h_comp_in, Q_c_tot, Ncomp);

            // OU hex capacity
            Q_h_tot = Q_c_tot + Ncomp;
            Q_h_OU = Q_h_tot - Q_h_TU_PL;
            Q_c_OU = 0;

            // OU fan flow rate and power
            m_air_cond = this->VRFOU_FlowRate(FlagCondMode, Tdischarge, this->SC, Q_h_OU, OutDryBulbTemp, OutHumRat);
            N_fan_OU_cond = this->RatedOUFanPower * m_air_cond / m_air_rated;
            N_fan_OU_evap = 0;

        } else {
            Ncomp = 0;
            CompSpdActual = 0;
            Q_c_OU = 0;
            Q_h_OU = 0;
            N_fan_OU_evap = 0;
            N_fan_OU_cond = 0;
        }

        // OU fan power
        N_fan_OU = N_fan_OU_evap + N_fan_OU_cond;

        // Calculate the m_ref_OU_evap & m_ref_OU_cond, with updated Tsuction
        {
            Real64 h_OU_evap_in;  // enthalpy of OU evaporator at inlet [kJ/kg]
            Real64 h_OU_evap_out; // enthalpy of OU evaporator at outlet [kJ/kg]
            Real64 h_OU_cond_in;  // enthalpy of OU condenser at inlet [kJ/kg]
            Real64 h_OU_cond_out; // enthalpy of OU condenser at outlet [kJ/kg]

            Real64 Psuction = GetSatPressureRefrig(state, this->RefrigerantName, Tsuction, RefrigerantIndex, RoutineName);

            // enthalpy of OU evaporator/condenser inlets and outlets
            h_OU_evap_in = h_IU_evap_in;
            h_OU_cond_in = h_comp_out;
            h_OU_evap_out = GetSupHeatEnthalpyRefrig(
                state, this->RefrigerantName, Tsuction + this->SH, max(min(Psuction, RefPHigh), RefPLow), RefrigerantIndex, RoutineName);
            h_OU_cond_out = GetSatEnthalpyRefrig(state, this->RefrigerantName, Tdischarge - this->SC, 0.0, RefrigerantIndex, RoutineName);

            if ((Q_c_OU == 0) || (h_OU_evap_out - h_OU_evap_in) <= 0) {
                m_ref_OU_evap = 0;
            } else {
                m_ref_OU_evap = Q_c_OU / (h_OU_evap_out - h_OU_evap_in);
            }

            if ((Q_h_OU == 0) || (h_OU_cond_in - h_OU_cond_out <= 0)) {
                m_ref_OU_cond = 0;
            } else {
                m_ref_OU_cond = Q_h_OU / (h_OU_cond_in - h_OU_cond_out);
            }

            // Calculate the parameters of refrigerant at compressor inlet, which is
            // a combination of refrigerant from IU evaporators and OU evaporator
            if ((m_ref_OU_evap + m_ref_IU_evap) > 0) {
                h_comp_in = (m_ref_OU_evap * h_OU_evap_out + m_ref_IU_evap * h_IU_PLc_out) / (m_ref_OU_evap + m_ref_IU_evap);
            }
        }
    }

    void VRFCondenserEquipment::VRFOU_PipeLossC(
        EnergyPlusData &state,
        Real64 const Pipe_m_ref,     // Refrigerant mass flow rate [kg/s]
        Real64 const Pevap,          // VRF evaporating pressure [Pa]
        Real64 const Pipe_h_IU_out,  // Enthalpy of IU at outlet [kJ/kg]
        Real64 const Pipe_SH_merged, // Average super heating degrees after the indoor units [C]
        Real64 const OutdoorDryBulb, // outdoor dry-bulb temperature (C)
        Real64 &Pipe_Q,              // unit part load ratio
        Real64 &Pipe_DeltP,          // ratio of compressor ON airflow to AVERAGE airflow over timestep
        Real64 &Pipe_h_comp_in       // Piping Loss Algorithm Parameter: Enthalpy after piping loss (compressor inlet) [kJ/kg]
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Rongpeng Zhang
        //       DATE WRITTEN   Nov 2015
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Determine the piping loss of the refrigerant, including both the heat loss and pressure drop.
        // This happens at VRF cooling mode, within the Main Pipe connecting Outdoor Unit to Indoor Units.

        // METHODOLOGY EMPLOYED:
        // Use a physics based piping loss model.

        using DataGlobals::Pi;
        using DXCoils::DXCoil;
        using FluidProperties::FindRefrigerant;
        using FluidProperties::GetSupHeatDensityRefrig;
        using FluidProperties::RefrigData;
        using General::SolveRoot;
        using TempSolveRoot::SolveRoot;

        int TUListNum;        // index to TU List
        int TUIndex;          // Index to terminal unit
        int CoilIndex;        // index to coil in terminal unit
        int NumTUInList;      // number of terminal units is list
        int NumIUActivated;   // number of the used indoor units [-]
        int RefrigerantIndex; // Index of the refrigerant [-]

        Real64 Pipe_v_ref;            // Piping Loss Algorithm Parameter: Refrigerant velocity [m/s]
        Real64 Pipe_T_room;           // Piping Loss Algorithm Parameter: Average Room Temperature [C]
        Real64 Pipe_Num_Re;           // Piping Loss Algorithm Parameter: refrigerant Re Number [-]
        Real64 Pipe_Num_Pr;           // Piping Loss Algorithm Parameter: refrigerant Pr Number [-]
        Real64 Pipe_Num_Nu;           // Piping Loss Algorithm Parameter: refrigerant Nu Number [-]
        Real64 Pipe_Num_St;           // Piping Loss Algorithm Parameter: refrigerant St Number [-]
        Real64 Pipe_Coe_k1;           // Piping Loss Algorithm Parameter: coefficients [-]
        Real64 Pipe_Coe_k2;           // Piping Loss Algorithm Parameter: coefficients [-]
        Real64 Pipe_Coe_k3;           // Piping Loss Algorithm Parameter: coefficients [-]
        Real64 Pipe_cp_ref;           // Piping Loss Algorithm_[kJ/kg/K]
        Real64 Pipe_conductivity_ref; // Piping Loss Algorithm: refrigerant conductivity [W/m/K]
        Real64 Pipe_viscosity_ref;    // Piping Loss Algorithm Parameter: refrigerant viscosity [MuPa*s]
        Real64 Ref_Coe_v1;            // Piping Loss Algorithm Parameter: coefficient to calculate Pipe_viscosity_ref [-]
        Real64 Ref_Coe_v2;            // Piping Loss Algorithm Parameter: coefficient to calculate Pipe_viscosity_ref [-]
        Real64 Ref_Coe_v3;            // Piping Loss Algorithm Parameter: coefficient to calculate Pipe_viscosity_ref [-]
        Real64 RefPipInsH;            // Heat transfer coefficient for calculating piping loss [W/m2K]

        static std::string const RoutineName("VRFOU_PipeLossC");

        TUListNum = this->ZoneTUListPtr;
        NumTUInList = TerminalUnitList(TUListNum).NumTUInList;
        Pipe_conductivity_ref = this->RefPipInsCon;

        RefPipInsH = 9.3;
        Pipe_cp_ref = 1.6;

        // Refrigerant data
        RefrigerantIndex = FindRefrigerant(state, this->RefrigerantName);
        Real64 RefPLow = RefrigData(RefrigerantIndex).PsLowPresValue;   // Low Pressure Value for Ps (>0.0)
        Real64 RefPHigh = RefrigData(RefrigerantIndex).PsHighPresValue; // High Pressure Value for Ps (max in tables)

        // Calculate Pipe_T_room
        Pipe_T_room = 0;
        NumIUActivated = 0;
        for (int NumTU = 1; NumTU <= NumTUInList; ++NumTU) {
            TUIndex = TerminalUnitList(TUListNum).ZoneTUPtr(NumTU);
            CoilIndex = VRFTU(TUIndex).CoolCoilIndex;

            if (DXCoil(CoilIndex).TotalCoolingEnergyRate > 0.0) {
                Pipe_T_room = Pipe_T_room + DXCoil(CoilIndex).InletAirTemp;
                NumIUActivated = NumIUActivated + 1;
            }
        }
        if (NumIUActivated > 0)
            Pipe_T_room = Pipe_T_room / NumIUActivated;
        else
            Pipe_T_room = 24;

        if (Pipe_m_ref > 0) {
            if (this->RefPipDiaSuc <= 0) this->RefPipDiaSuc = 0.025;

            Ref_Coe_v1 = Pevap / 1000000 / 4.926;
            Ref_Coe_v2 = Pipe_h_IU_out / 383.5510343;
            Ref_Coe_v3 = (this->EvaporatingTemp + Pipe_SH_merged + 273.15) / 344.39;

            Pipe_viscosity_ref = 4.302 * Ref_Coe_v1 + 0.81622 * pow_2(Ref_Coe_v1) - 120.98 * Ref_Coe_v2 + 139.17 * pow_2(Ref_Coe_v2) +
                                 118.76 * Ref_Coe_v3 + 81.04 * pow_2(Ref_Coe_v3) + 5.7858 * Ref_Coe_v1 * Ref_Coe_v2 -
                                 8.3817 * Ref_Coe_v1 * Ref_Coe_v3 - 218.48 * Ref_Coe_v2 * Ref_Coe_v3 + 21.58;
            if (Pipe_viscosity_ref <= 0) Pipe_viscosity_ref = 16.26; // default superheated vapor viscosity data (MuPa*s) at T=353.15 K, P=2MPa

            Pipe_v_ref =
                Pipe_m_ref / (Pi * pow_2(this->RefPipDiaSuc) * 0.25) /
                GetSupHeatDensityRefrig(
                    state, this->RefrigerantName, this->EvaporatingTemp + Pipe_SH_merged, max(min(Pevap, RefPHigh), RefPLow), RefrigerantIndex, RoutineName);
            Pipe_Num_Re = Pipe_m_ref / (Pi * pow_2(this->RefPipDiaSuc) * 0.25) * this->RefPipDiaSuc / Pipe_viscosity_ref * 1000000;
            Pipe_Num_Pr = Pipe_viscosity_ref * Pipe_cp_ref * 0.001 / Pipe_conductivity_ref;
            Pipe_Num_Nu = 0.023 * std::pow(Pipe_Num_Re, 0.8) * std::pow(Pipe_Num_Pr, 0.3);
            Pipe_Num_St = Pipe_Num_Nu / Pipe_Num_Re / Pipe_Num_Pr;

            Pipe_DeltP = max(0.0,
                             8 * Pipe_Num_St * std::pow(Pipe_Num_Pr, 0.6667) * this->RefPipEquLen / this->RefPipDiaSuc *
                                     GetSupHeatDensityRefrig(state,
                                                             this->RefrigerantName,
                                                             this->EvaporatingTemp + Pipe_SH_merged,
                                                             max(min(Pevap, RefPHigh), RefPLow),
                                                             RefrigerantIndex,
                                                             RoutineName) *
                                     pow_2(Pipe_v_ref) / 2 -
                                 this->RefPipHei *
                                     GetSupHeatDensityRefrig(state,
                                                             this->RefrigerantName,
                                                             this->EvaporatingTemp + Pipe_SH_merged,
                                                             max(min(Pevap, RefPHigh), RefPLow),
                                                             RefrigerantIndex,
                                                             RoutineName) *
                                     9.80665);

            Pipe_Coe_k1 = Pipe_Num_Nu * Pipe_viscosity_ref;
            Pipe_Coe_k3 = RefPipInsH * (this->RefPipDiaSuc + 2 * this->RefPipInsThi);
            if (this->RefPipInsThi >= 0.0) {
                Pipe_Coe_k2 = 2 * this->RefPipInsCon / std::log(1.0 + 2 * this->RefPipInsThi / this->RefPipDiaSuc);
            } else {
                Pipe_Coe_k2 = 9999.9;
            }

            Pipe_Q = max(0.0,
                         (Pi * this->RefPipLen) * (OutdoorDryBulb / 2 + Pipe_T_room / 2 - this->EvaporatingTemp - Pipe_SH_merged) /
                             (1 / Pipe_Coe_k1 + 1 / Pipe_Coe_k2 + 1 / Pipe_Coe_k3));

            Pipe_h_comp_in = Pipe_h_IU_out + Pipe_Q / Pipe_m_ref;

        } else {
            Pipe_DeltP = 0;
            Pipe_Q = 0;
            Pipe_h_comp_in = Pipe_h_IU_out;
        }
    }

    void VRFCondenserEquipment::VRFOU_PipeLossH(
        EnergyPlusData &state,
        Real64 const Pipe_m_ref,     // Refrigerant mass flow rate [kg/s]
        Real64 const Pcond,          // VRF condensing pressure [Pa]
        Real64 const Pipe_h_IU_in,   // Enthalpy of IU at outlet [kJ/kg]
        Real64 const OutdoorDryBulb, // outdoor dry-bulb temperature (C)
        Real64 &Pipe_Q,              // unit part load ratio
        Real64 &Pipe_DeltP,          // ratio of compressor ON airflow to AVERAGE airflow over timestep
        Real64 &Pipe_h_comp_out      // Piping Loss Algorithm Parameter: Enthalpy before piping loss (compressor outlet) [kJ/kg]
    ) const
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Rongpeng Zhang
        //       DATE WRITTEN   Nov 2015
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Determine the piping loss of the refrigerant, including both the heat loss and pressure drop.
        // This happens at VRF cooling mode, within the Main Pipe connecting Outdoor Unit to Indoor Units.

        // METHODOLOGY EMPLOYED:
        // Use a physics based piping loss model.

        using DataGlobals::Pi;
        using DXCoils::DXCoil;
        using FluidProperties::FindRefrigerant;
        using FluidProperties::GetSatTemperatureRefrig;
        using FluidProperties::GetSupHeatDensityRefrig;
        using FluidProperties::GetSupHeatEnthalpyRefrig;
        using FluidProperties::GetSupHeatTempRefrig;
        using FluidProperties::RefrigData;
        using General::SolveRoot;
        using TempSolveRoot::SolveRoot;

        int TUListNum;        // index to TU List
        int TUIndex;          // Index to terminal unit
        int CoilIndex;        // index to coil in terminal unit
        int NumTUInList;      // number of terminal units is list
        int NumIUActivated;   // number of the used indoor units [-]
        int RefrigerantIndex; // Index of the refrigerant [-]

        Real64 Pipe_v_ref;            // Piping Loss Algorithm Parameter: Refrigerant velocity [m/s]
        Real64 Pipe_T_room;           // Piping Loss Algorithm Parameter: Average Room Temperature [C]
        Real64 Pipe_T_IU_in;          // Piping Loss Algorithm Parameter: Average Refrigerant Temperature [C]
        Real64 Pipe_Num_Re;           // Piping Loss Algorithm Parameter: refrigerant Re Number [-]
        Real64 Pipe_Num_Pr;           // Piping Loss Algorithm Parameter: refrigerant Pr Number [-]
        Real64 Pipe_Num_Nu;           // Piping Loss Algorithm Parameter: refrigerant Nu Number [-]
        Real64 Pipe_Num_St;           // Piping Loss Algorithm Parameter: refrigerant St Number [-]
        Real64 Pipe_Coe_k1;           // Piping Loss Algorithm Parameter: coefficients [-]
        Real64 Pipe_Coe_k2;           // Piping Loss Algorithm Parameter: coefficients [-]
        Real64 Pipe_Coe_k3;           // Piping Loss Algorithm Parameter: coefficients [-]
        Real64 Pipe_cp_ref;           // Piping Loss Algorithm_[kJ/kg/K]
        Real64 Pipe_conductivity_ref; // Piping Loss Algorithm: refrigerant conductivity [W/m/K]
        Real64 Pipe_viscosity_ref;    // Piping Loss Algorithm Parameter: refrigerant viscosity [MuPa*s]
        Real64 Ref_Coe_v1;            // Piping Loss Algorithm Parameter: coefficient to calculate Pipe_viscosity_ref [-]
        Real64 Ref_Coe_v2;            // Piping Loss Algorithm Parameter: coefficient to calculate Pipe_viscosity_ref [-]
        Real64 Ref_Coe_v3;            // Piping Loss Algorithm Parameter: coefficient to calculate Pipe_viscosity_ref [-]
        Real64 RefPipInsH;            // Heat transfer coefficient for calculating piping loss [W/m2K]

        static std::string const RoutineName("VRFOU_PipeLossH");

        TUListNum = this->ZoneTUListPtr;
        NumTUInList = TerminalUnitList(TUListNum).NumTUInList;
        Pipe_conductivity_ref = this->RefPipInsCon;

        RefPipInsH = 9.3;
        Pipe_cp_ref = 1.6;

        // Refrigerant data
        RefrigerantIndex = FindRefrigerant(state, this->RefrigerantName);
        Real64 RefTHigh = RefrigData(RefrigerantIndex).PsHighTempValue; // High Temperature Value for Ps (max in tables)
        Real64 RefPLow = RefrigData(RefrigerantIndex).PsLowPresValue;   // Low Pressure Value for Ps (>0.0)
        Real64 RefPHigh = RefrigData(RefrigerantIndex).PsHighPresValue; // High Pressure Value for Ps (max in tables)
        Real64 RefTSat = GetSatTemperatureRefrig(state, this->RefrigerantName, max(min(Pcond, RefPHigh), RefPLow), RefrigerantIndex, RoutineName);

        // Perform iteration to calculate Pipe_T_IU_in, given P and h
        Pipe_T_IU_in = GetSupHeatTempRefrig(state,
                                            this->RefrigerantName,
                                            max(min(Pcond, RefPHigh), RefPLow),
                                            Pipe_h_IU_in,
                                            max(this->IUCondensingTemp, RefTSat),
                                            min(this->IUCondensingTemp + 50, RefTHigh),
                                            RefrigerantIndex,
                                            RoutineName);
        Pipe_T_IU_in = min(RefTHigh, Pipe_T_IU_in);

        // Calculate average room temperature
        Pipe_T_room = 0;
        NumIUActivated = 0;
        for (int NumTU = 1; NumTU <= NumTUInList; ++NumTU) {
            TUIndex = TerminalUnitList(TUListNum).ZoneTUPtr(NumTU);
            CoilIndex = VRFTU(TUIndex).HeatCoilIndex;

            if (DXCoil(CoilIndex).TotalHeatingEnergyRate > 0.0) {
                Pipe_T_room = Pipe_T_room + DXCoil(CoilIndex).InletAirTemp;
                NumIUActivated = NumIUActivated + 1;
            }
        }
        if (NumIUActivated > 0)
            Pipe_T_room = Pipe_T_room / NumIUActivated;
        else
            Pipe_T_room = 18;

        // Calculate piping loss
        if (Pipe_m_ref > 0) {
            Ref_Coe_v1 = Pcond / 1000000 / 4.926;
            Ref_Coe_v2 = Pipe_h_IU_in / 383.5510343;
            Ref_Coe_v3 = (Pipe_T_IU_in + 273.15) / 344.39;
            Pipe_viscosity_ref = 4.302 * Ref_Coe_v1 + 0.81622 * pow_2(Ref_Coe_v1) - 120.98 * Ref_Coe_v2 + 139.17 * pow_2(Ref_Coe_v2) +
                                 118.76 * Ref_Coe_v3 + 81.04 * pow_2(Ref_Coe_v3) + 5.7858 * Ref_Coe_v1 * Ref_Coe_v2 -
                                 8.3817 * Ref_Coe_v1 * Ref_Coe_v3 - 218.48 * Ref_Coe_v2 * Ref_Coe_v3 + 21.58;
            if (Pipe_viscosity_ref <= 0) Pipe_viscosity_ref = 16.26; // default superheated vapor viscosity data (MuPa*s) at T=353.15 K, P=2MPa

            Pipe_v_ref =
                Pipe_m_ref / (Pi * pow_2(this->RefPipDiaDis) * 0.25) /
                GetSupHeatDensityRefrig(state, this->RefrigerantName, Pipe_T_IU_in, max(min(Pcond, RefPHigh), RefPLow), RefrigerantIndex, RoutineName);
            Pipe_Num_Re = Pipe_m_ref / (Pi * pow_2(this->RefPipDiaDis) * 0.25) * this->RefPipDiaDis / Pipe_viscosity_ref * 1000000;
            Pipe_Num_Pr = Pipe_viscosity_ref * Pipe_cp_ref * 0.001 / Pipe_conductivity_ref;
            Pipe_Num_Nu = 0.023 * std::pow(Pipe_Num_Re, 0.8) * std::pow(Pipe_Num_Pr, 0.4);
            Pipe_Num_St = Pipe_Num_Nu / Pipe_Num_Re / Pipe_Num_Pr;

            Pipe_Coe_k1 = Pipe_Num_Nu * Pipe_viscosity_ref;
            Pipe_Coe_k2 = this->RefPipInsCon * (this->RefPipDiaDis + this->RefPipInsThi) / this->RefPipInsThi;
            Pipe_Coe_k3 = RefPipInsH * (this->RefPipDiaDis + 2 * this->RefPipInsThi);

            Pipe_Q = max(0.0,
                         (Pi * this->RefPipLen) * (Pipe_T_IU_in - OutdoorDryBulb / 2 - Pipe_T_room / 2) /
                             (1 / Pipe_Coe_k1 + 1 / Pipe_Coe_k2 + 1 / Pipe_Coe_k3)); // [W]
            Pipe_DeltP = max(0.0,
                             8 * Pipe_Num_St * std::pow(Pipe_Num_Pr, 0.6667) * this->RefPipEquLen / this->RefPipDiaDis *
                                     GetSupHeatDensityRefrig(
                                         state, this->RefrigerantName, Pipe_T_IU_in, max(min(Pcond, RefPHigh), RefPLow), RefrigerantIndex, RoutineName) *
                                     pow_2(Pipe_v_ref) / 2 -
                                 this->RefPipHei *
                                     GetSupHeatDensityRefrig(
                                         state, this->RefrigerantName, Pipe_T_IU_in, max(min(Pcond, RefPHigh), RefPLow), RefrigerantIndex, RoutineName) *
                                     9.80665);

            Pipe_h_comp_out = Pipe_h_IU_in + Pipe_Q / Pipe_m_ref;

        } else {
            Pipe_DeltP = 0;
            Pipe_Q = 0;
            Pipe_h_comp_out = Pipe_h_IU_in;
        }
    }

    void VRFTerminalUnitEquipment::CalcVRFSuppHeatingCoil(EnergyPlusData &state,
                                                          int const VRFTUNum,            // index of vrf terminal unit
                                                          bool const FirstHVACIteration, // True when first HVAC iteration
                                                          Real64 const PartLoadRatio,    // coil operating part-load ratio
                                                          Real64 &SuppCoilLoad           // supp heating coil load max (W)
    )
    {

        // PURPOSE OF THIS SUBROUTINE:
        // Manages VRF terminal unit supplemental heaters simulation.

        // Locals
        // subroutine parameter definitions:
        int const MaxIte(500);   // Maximum number of iterations for solver
        Real64 const Acc(1.e-3); // Accuracy of solver result

        // local variable declaration:
        std::vector<Real64> Par; // Parameter array passed to solver
        int SolFla;              // Flag of solver, num iterations if >0, else error index
        Real64 SuppHeatCoilLoad; // load passed to supplemental heating coil (W)
        Real64 QActual;          // actual coil output (W)
        Real64 PartLoadFrac;     // temporary PLR variable

        Par.resize(4);
        QActual = 0.0;
        PartLoadFrac = 0.0;
        SuppHeatCoilLoad = 0.0;

        // simulate gas, electric, hot water, and steam heating coils
        if (DataEnvironment::OutDryBulbTemp <= this->MaxOATSuppHeatingCoil) {
            SuppHeatCoilLoad = SuppCoilLoad;
        } else {
            SuppHeatCoilLoad = 0.0;
        }

        {
            auto const SELECT_CASE_var(this->SuppHeatCoilType_Num);

            if ((SELECT_CASE_var == DataHVACGlobals::Coil_HeatingGasOrOtherFuel) || (SELECT_CASE_var == DataHVACGlobals::Coil_HeatingElectric)) {
                HeatingCoils::SimulateHeatingCoilComponents(state,
                                                            this->SuppHeatCoilName,
                                                            FirstHVACIteration,
                                                            SuppHeatCoilLoad,
                                                            this->SuppHeatCoilIndex,
                                                            QActual,
                                                            true,
                                                            this->OpMode,
                                                            PartLoadRatio);
                SuppHeatCoilLoad = QActual;
            } else if (SELECT_CASE_var == DataHVACGlobals::Coil_HeatingWater) {
                if (SuppHeatCoilLoad > DataHVACGlobals::SmallLoad) {
                    //     see if HW coil has enough capacity to meet the load
                    Real64 mdot = this->SuppHeatCoilFluidMaxFlow;
                    DataLoopNode::Node(this->SuppHeatCoilFluidInletNode).MassFlowRate = mdot;
                    //     simulate hot water coil to find the full flow operating capacity
                    WaterCoils::SimulateWaterCoilComponents(
                        state, this->SuppHeatCoilName, FirstHVACIteration, this->SuppHeatCoilIndex, QActual, this->OpMode, PartLoadRatio);
                    if (QActual > SuppHeatCoilLoad) {
                        Par[1] = double(VRFTUNum);
                        if (FirstHVACIteration) {
                            Par[2] = 1.0;
                        } else {
                            Par[2] = 0.0;
                        }
                        Par[3] = SuppHeatCoilLoad;

                        TempSolveRoot::SolveRoot(state, Acc, MaxIte, SolFla, PartLoadFrac, this->HotWaterHeatingCoilResidual, 0.0, 1.0, Par);
                        this->SuppHeatPartLoadRatio = PartLoadFrac;
                    } else {
                        this->SuppHeatPartLoadRatio = 1.0;
                        SuppHeatCoilLoad = QActual;
                    }
                } else {
                    this->SuppHeatPartLoadRatio = 0.0;
                    Real64 mdot = 0.0;
                    SuppHeatCoilLoad = 0.0;
                    PlantUtilities::SetComponentFlowRate(mdot,
                                                         this->SuppHeatCoilFluidInletNode,
                                                         this->SuppHeatCoilFluidOutletNode,
                                                         this->SuppHeatCoilLoopNum,
                                                         this->SuppHeatCoilLoopSide,
                                                         this->SuppHeatCoilBranchNum,
                                                         this->SuppHeatCoilCompNum);
                }
                //     simulate water heating coil
                WaterCoils::SimulateWaterCoilComponents(state,
                                                        this->SuppHeatCoilName,
                                                        FirstHVACIteration,
                                                        this->SuppHeatCoilIndex,
                                                        SuppHeatCoilLoad,
                                                        this->OpMode,
                                                        this->SuppHeatPartLoadRatio);

            } else if (SELECT_CASE_var == DataHVACGlobals::Coil_HeatingSteam) {
                //     simulate steam heating coil
                Real64 mdot = this->SuppHeatCoilFluidMaxFlow * PartLoadRatio;
                DataLoopNode::Node(this->SuppHeatCoilFluidInletNode).MassFlowRate = mdot;
                SteamCoils::SimulateSteamCoilComponents(state,
                                                        this->SuppHeatCoilName,
                                                        FirstHVACIteration,
                                                        this->SuppHeatCoilIndex,
                                                        SuppHeatCoilLoad,
                                                        QActual,
                                                        this->OpMode,
                                                        PartLoadRatio);
                SuppHeatCoilLoad = QActual;
            }
            SuppCoilLoad = SuppHeatCoilLoad;
        }
    }

    Real64 VRFTerminalUnitEquipment::HotWaterHeatingCoilResidual(EnergyPlusData &state,
                                                                 Real64 const PartLoadFrac,     // water heating coil part-load ratio
                                                                 std::vector<Real64> const &Par // par(1) = index to current VRF terminal unit
    )
    {

        // PURPOSE OF THIS FUNCTION:
        // Calculates supplemental hot water heating coils load fraction residual [(QActual - Load)/Load]
        // hot water Coil output depends on the part load ratio which is being varied to drive the load
        // fraction residual to zero.

        // METHODOLOGY EMPLOYED:
        // runs Coil:Heating:Water component object to get the actual heating load deleivered [W] at a
        // given part load ratio and calculates the residual as defined above

        // Return value
        Real64 Residuum; // Residual to be minimized to zero

        // local variables declaration:
        int VRFTUNum = int(Par[1]);       // index to current terminal unit simulated
        bool FirstHVACIteration = Par[2]; // 0 flag if it first HVAC iteration, or else 1
        Real64 SuppHeatCoilLoad = Par[3]; // supplemental heating coil load to be met [W]
        Real64 QActual = 0.0;             // actual heating load deleivered [W]

        // Real64 mdot = min(DataLoopNode::Node(VRFTU(VRFTUNum).SuppHeatCoilFluidOutletNode).MassFlowRateMaxAvail,
        //                  VRFTU(VRFTUNum).SuppHeatCoilFluidMaxFlow * PartLoadFrac);

        Real64 mdot = HVACVariableRefrigerantFlow::VRFTU(VRFTUNum).SuppHeatCoilFluidMaxFlow * PartLoadFrac;
        DataLoopNode::Node(VRFTU(VRFTUNum).SuppHeatCoilFluidInletNode).MassFlowRate = mdot;
        WaterCoils::SimulateWaterCoilComponents(state,
                                                VRFTU(VRFTUNum).SuppHeatCoilName,
                                                FirstHVACIteration,
                                                VRFTU(VRFTUNum).SuppHeatCoilIndex,
                                                QActual,
                                                VRFTU(VRFTUNum).OpMode,
                                                PartLoadFrac);

        if (std::abs(SuppHeatCoilLoad) == 0.0) {
            Residuum = (QActual - SuppHeatCoilLoad) / 100.0;
        } else {
            Residuum = (QActual - SuppHeatCoilLoad) / SuppHeatCoilLoad;
        }

        return Residuum;
    }

    Real64 VRFTerminalUnitEquipment::HeatingCoilCapacityLimit(
        Real64 const HeatCoilAirInletNode, // supplemental heating coil air inlet node
        Real64 const HeatCoilMaxSATAllowed // supplemental heating coil maximum supply air temperature allowed [C]
    )
    {
        // PURPOSE OF THIS FUNCTION:
        // Calculates supplemental heating coils maximum heating capacity allowed based on the maximum
        // supply air temperature limit specified.

        // METHODOLOGY EMPLOYED:
        // ( m_dot_air * Cp_air_avg * DeltaT_air_across_heating_coil) [W]

        // Return value
        Real64 HeatCoilCapacityAllowed; // heating coil maximum capacity that can be deleivered at current time [W]

        Real64 MDotAir = DataLoopNode::Node(HeatCoilAirInletNode).MassFlowRate;
        Real64 CpAirIn = Psychrometrics::PsyCpAirFnW(DataLoopNode::Node(HeatCoilAirInletNode).HumRat);
        Real64 HCDeltaT = max(0.0, HeatCoilMaxSATAllowed - DataLoopNode::Node(HeatCoilAirInletNode).Temp);
        HeatCoilCapacityAllowed = MDotAir * CpAirIn * HCDeltaT;

        return HeatCoilCapacityAllowed;
    }

    // Clears the global data in HVACVariableRefrigerantFlow.
    // Needed for unit tests, should not be normally called.
    void clear_state()
    {
        NumVRFCond = 0;
        NumVRFTU = 0;
        NumVRFTULists = 0;
        CompOnMassFlow = 0.0;
        OACompOnMassFlow = 0.0;
        CompOffMassFlow = 0.0;
        OACompOffMassFlow = 0.0;
        CompOnFlowRatio = 0.0;
        CompOffFlowRatio = 0.0;
        FanSpeedRatio = 0.0;
        LoopDXCoolCoilRTF = 0.0;
        LoopDXHeatCoilRTF = 0.0;
        CondenserWaterMassFlowRate = 0.0;

        GetVRFInputFlag = true;
        MyOneTimeFlag = true;
        MyOneTimeSizeFlag = true;
        ZoneEquipmentListNotChecked = true;

        VRF.deallocate();
        VrfUniqueNames.clear();
        VRFTU.deallocate();
        TerminalUnitList.deallocate();
        VRFTUNumericFields.deallocate();
        MaxCoolingCapacity.deallocate();
        MaxHeatingCapacity.deallocate();
        CoolCombinationRatio.deallocate();
        HeatCombinationRatio.deallocate();
        MaxDeltaT.deallocate();
        MinDeltaT.deallocate();
        LastModeCooling.deallocate();
        LastModeHeating.deallocate();
        HeatingLoad.deallocate();
        CoolingLoad.deallocate();
        NumCoolingLoads.deallocate();
        SumCoolingLoads.deallocate();
        NumHeatingLoads.deallocate();
        SumHeatingLoads.deallocate();
        CheckEquipName.deallocate();
        MyEnvrnFlag.deallocate();
        MySizeFlag.deallocate();
        MyBeginTimeStepFlag.deallocate();
        MyVRFFlag.deallocate();
        MyVRFCondFlag.deallocate();
        MyZoneEqFlag.deallocate();
    }

} // namespace HVACVariableRefrigerantFlow

} // namespace EnergyPlus
