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
#include <cmath>

// ObjexxFCL Headers
#include <ObjexxFCL/Array.functions.hh>
#include <ObjexxFCL/Fmath.hh>

// EnergyPlus Headers
#include <EnergyPlus/Autosizing/Base.hh>
#include <EnergyPlus/BranchNodeConnections.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataAirSystems.hh>
#include <EnergyPlus/DataContaminantBalance.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/DataPrecisionGlobals.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/Fans.hh>
#include <EnergyPlus/FluidProperties.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/GeneralRoutines.hh>
#include <EnergyPlus/GlobalNames.hh>
#include <EnergyPlus/HVACFan.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/NodeInputManager.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/OutputReportPredefined.hh>
#include <EnergyPlus/Plant/DataPlant.hh>
#include <EnergyPlus/PlantUtilities.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/ReportCoilSelection.hh>
#include <EnergyPlus/UtilityRoutines.hh>
#include <EnergyPlus/WaterThermalTanks.hh>
#include <EnergyPlus/WaterToAirHeatPumpSimple.hh>

namespace EnergyPlus {

namespace WaterToAirHeatPumpSimple {

    // Module containing the Water to Air Heat Pump simulation routines

    // MODULE INFORMATION:
    //       AUTHOR         Arun Shenoy
    //       DATE WRITTEN   Nov 2003
    //       MODIFIED       Brent Griffith, Sept 2010 plant upgrades
    //       RE-ENGINEERED  Kenneth Tang (Jan 2005)

    // PURPOSE OF THIS MODULE:
    // To encapsulate the data and algorithms required to
    // manage the Water to Air Heat Pump Simple Component

    // METHODOLOGY EMPLOYED:

    // REFERENCES:
    // (1) Lash.T.A.,1992.Simulation and Analysis of a Water Loop Heat Pump System.
    // M.S. Thesis, University of Illinois at Urbana Champaign.
    // (2) Shenoy, Arun. 2004. Simulation, Modeling and Analysis of Water to Air Heat Pump.
    // State Energy Simulation Program. M.S. Thesis, Department of Mechanical and Aerospace Engineering,
    // Oklahoma State University. (downloadable from www.hvac.okstate.edu)
    // (3) Tang,C.C.. 2005. Modeling Packaged Heat Pumps in a Quasi-Steady
    // State Energy Simulation Program. M.S. Thesis, Department of Mechanical and Aerospace Engineering,
    // Oklahoma State University. (downloadable from www.hvac.okstate.edu)

    // OTHER NOTES:

    // USE STATEMENTS:
    // Use statements for data only modules
    // Use statements for access to subroutines in other modules
    // Using/Aliasing
    using namespace DataPrecisionGlobals;
    using namespace DataLoopNode;
    using namespace DataGlobals;
    using namespace DataSizing;
    using DataEnvironment::OutBaroPress;
    using DataEnvironment::StdBaroPress;
    using DataHeatBalance::HeatReclaimSimple_WAHPCoil;
    using DataHVACGlobals::ContFanCycCoil;
    using DataHVACGlobals::Cooling;
    using DataHVACGlobals::CycFanCycCoil;
    using DataHVACGlobals::Heating;
    using DataHVACGlobals::TimeStepSys;
    using DataHVACGlobals::WaterConstant;
    using DataHVACGlobals::WaterConstantOnDemand;
    using DataHVACGlobals::WaterCycling;
    using DataPlant::TypeOf_CoilWAHPCoolingEquationFit;
    using DataPlant::TypeOf_CoilWAHPHeatingEquationFit;

    static std::string const BlankString;

    void SimWatertoAirHPSimple(EnergyPlusData &state, std::string const &CompName,   // Coil Name
                               int &CompIndex,                // Index for Component name
                               Real64 const SensLoad,         // Sensible demand load [W]
                               Real64 const LatentLoad,       // Latent demand load [W]
                               int const CyclingScheme,       // Continuous fan OR cycling compressor
                               Real64 const RuntimeFrac,      // Compressor run time fraction  or
                               Real64 &MaxONOFFCyclesperHour, // Maximum cycling rate of heat pump [cycles/hr]
                               Real64 &HPTimeConstant,        // Heat pump time constant [s]
                               Real64 &FanDelayTime,          // Fan delay time, time delay for the HP's fan to
                               int const CompOp,
                               Real64 const PartLoadRatio,
                               bool const FirstHVACIteration,
                               Optional<Real64 const> OnOffAirFlowRat // ratio of comp on to comp off air flow rate
    )
    {

        //       AUTHOR         Arun Shenoy
        //       DATE WRITTEN   Nov 2003
        //       MODIFIED       na
        //       RE-ENGINEERED  Kenneth Tang (Jan 2005)

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine manages Simple Water to Air Heat Pump component simulation.

        // REFERENCES:
        // (1) Lash.T.A.,1992.Simulation and Analysis of a Water Loop Heat Pump System.
        // M.S. Thesis, University of Illinois at Urbana Champaign.
        // (2) Shenoy, Arun. 2004. Simulation, Modeling and Analysis of Water to Air Heat Pump.
        // State Energy Simulation Program. M.S. Thesis, Department of Mechanical and Aerospace Engineering,
        // Oklahoma State University. (downloadable from www.hvac.okstate.edu)
        // (3) Tang,C.C.. 2005. Modeling Packaged Heat Pumps in a Quasi-Steady
        // State Energy Simulation Program. M.S. Thesis, Department of Mechanical and Aerospace Engineering,
        // Oklahoma State University. (downloadable from www.hvac.okstate.edu)

        // Using/Aliasing
        using FluidProperties::FindGlycol;
        using General::TrimSigDigits;

        // percent on-time (on-time/cycle time)
        // shut off after compressor cycle off  [s]

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int HPNum;                // The WatertoAirHP that you are currently loading input into
        Real64 OnOffAirFlowRatio; // ratio of comp on to comp off air flow rate

        // FLOW:

        // Obtains and Allocates WatertoAirHP related parameters from input file
        if (state.dataWaterToAirHeatPumpSimple->GetCoilsInputFlag) { // First time subroutine has been entered
            GetSimpleWatertoAirHPInput(state);
            //    WaterIndex=FindGlycol('WATER') !Initialize the WaterIndex once
            state.dataWaterToAirHeatPumpSimple->GetCoilsInputFlag = false;
        }

        if (CompIndex == 0) {
            HPNum = UtilityRoutines::FindItemInList(CompName, state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP);
            if (HPNum == 0) {
                ShowFatalError("WaterToAirHPSimple not found=" + CompName);
            }
            CompIndex = HPNum;
        } else {
            HPNum = CompIndex;
            if (HPNum > state.dataWaterToAirHeatPumpSimple->NumWatertoAirHPs || HPNum < 1) {
                ShowFatalError("SimWatertoAirHPSimple: Invalid CompIndex passed=" + TrimSigDigits(HPNum) +
                               ", Number of Water to Air HPs=" + TrimSigDigits(state.dataWaterToAirHeatPumpSimple->NumWatertoAirHPs) + ", WaterToAir HP name=" + CompName);
            }
            if (!CompName.empty() && CompName != state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Name) {
                ShowFatalError("SimWatertoAirHPSimple: Invalid CompIndex passed=" + TrimSigDigits(HPNum) + ", WaterToAir HP name=" + CompName +
                               ", stored WaterToAir HP Name for that index=" + state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Name);
            }
        }

        if (present(OnOffAirFlowRat)) {
            OnOffAirFlowRatio = OnOffAirFlowRat;
        } else {
            OnOffAirFlowRatio = 1.0;
        }

        if (state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WAHPPlantTypeOfNum == TypeOf_CoilWAHPCoolingEquationFit) {
            // Cooling mode
            InitSimpleWatertoAirHP(state,
                                   HPNum,
                                   MaxONOFFCyclesperHour,
                                   HPTimeConstant,
                                   FanDelayTime,
                                   SensLoad,
                                   LatentLoad,
                                   CyclingScheme,
                                   OnOffAirFlowRatio,
                                   FirstHVACIteration);
            CalcHPCoolingSimple(state, HPNum, CyclingScheme, RuntimeFrac, SensLoad, LatentLoad, CompOp, PartLoadRatio, OnOffAirFlowRatio);
            UpdateSimpleWatertoAirHP(state, HPNum);
        } else if (state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WAHPPlantTypeOfNum == TypeOf_CoilWAHPHeatingEquationFit) {
            // Heating mode
            InitSimpleWatertoAirHP(state,
                                   HPNum,
                                   MaxONOFFCyclesperHour,
                                   HPTimeConstant,
                                   FanDelayTime,
                                   SensLoad,
                                   constant_zero,
                                   CyclingScheme,
                                   OnOffAirFlowRatio,
                                   FirstHVACIteration);
            CalcHPHeatingSimple(state, HPNum, CyclingScheme, RuntimeFrac, SensLoad, CompOp, PartLoadRatio, OnOffAirFlowRatio);
            UpdateSimpleWatertoAirHP(state, HPNum);
        } else {
            ShowFatalError("SimWatertoAirHPSimple: WatertoAir heatpump not in either HEATING or COOLING mode");
        }
    }

    // MODULE SUBROUTINES:
    //*************************************************************************

    void GetSimpleWatertoAirHPInput(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Arun Shenoy
        //       DATE WRITTEN   Nov 2003
        //       MODIFIED       na
        //       RE-ENGINEERED  Kenneth Tang (Jan 2005)

        // PURPOSE OF THIS SUBROUTINE:
        // Obtains input data for HPs and stores it in HP data structures

        // METHODOLOGY EMPLOYED:
        // Uses "Get" routines to read in data.

        // REFERENCES:
        // (1) Lash.T.A.,1992.Simulation and Analysis of a Water loop Heat Pump System.
        // M.S. Thesis, University of Illinois at Urbana Champaign.
        // (2) Shenoy, Arun. 2004. Simulation, Modeling and Analysis of Water to Air Heat Pump.
        // State Energy Simulation Program. M.S. Thesis, Department of Mechanical and Aerospace Engineering,
        // Oklahoma State University. (downloadable from www.hvac.okstate.edu)
        // (3) Tang,C.C.. 2005. Modeling Packaged Heat Pumps in a Quasi-Steady
        // State Energy Simulation Program. M.S. Thesis, Department of Mechanical and Aerospace Engineering,
        // Oklahoma State University. (downloadable from www.hvac.okstate.edu)

        // Using/Aliasing
        using namespace NodeInputManager;
        using BranchNodeConnections::TestCompSet;
        using GlobalNames::VerifyUniqueCoilName;
        using namespace OutputReportPredefined;

        // SUBROUTINE PARAMETER DEFINITIONS:
        static std::string const RoutineName("GetSimpleWatertoAirHPInput: "); // include trailing blank space

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int HPNum;               // The Water to Air HP that you are currently loading input into
        int NumCool;             // Counter for cooling coil
        int NumHeat;             // Counter for heating coil
        int WatertoAirHPNum;     // Counter
        int NumAlphas;           // Number of variables in String format
        int NumNums;             // Number of variables in Numeric format
        int NumParams;           // Total number of input fields
        static int MaxNums(0);   // Maximum number of numeric input fields
        static int MaxAlphas(0); // Maximum number of alpha input fields
        int IOStat;
        static bool ErrorsFound(false);  // If errors detected in input
        std::string CurrentModuleObject; // for ease in getting objects
        Array1D_string AlphArray;        // Alpha input items for object
        Array1D_string cAlphaFields;     // Alpha field names
        Array1D_string cNumericFields;   // Numeric field names
        Array1D<Real64> NumArray;        // Numeric input items for object
        Array1D_bool lAlphaBlanks;       // Logical array, alpha field input BLANK = .TRUE.
        Array1D_bool lNumericBlanks;     // Logical array, numeric field input BLANK = .TRUE.

        NumCool = inputProcessor->getNumObjectsFound("Coil:Cooling:WaterToAirHeatPump:EquationFit");
        NumHeat = inputProcessor->getNumObjectsFound("Coil:Heating:WaterToAirHeatPump:EquationFit");
        state.dataWaterToAirHeatPumpSimple->NumWatertoAirHPs = NumCool + NumHeat;
        HPNum = 0;

        if (state.dataWaterToAirHeatPumpSimple->NumWatertoAirHPs <= 0) {
            ShowSevereError("No Equipment found in SimWatertoAirHPSimple");
            ErrorsFound = true;
        }

        // Allocate Arrays
        if (state.dataWaterToAirHeatPumpSimple->NumWatertoAirHPs > 0) {
            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP.allocate(state.dataWaterToAirHeatPumpSimple->NumWatertoAirHPs);
            state.dataWaterToAirHeatPumpSimple->SimpleHPTimeStepFlag.dimension(state.dataWaterToAirHeatPumpSimple->NumWatertoAirHPs, true);
            DataHeatBalance::HeatReclaimSimple_WAHPCoil.allocate(state.dataWaterToAirHeatPumpSimple->NumWatertoAirHPs);
        }

        inputProcessor->getObjectDefMaxArgs("Coil:Cooling:WaterToAirHeatPump:EquationFit", NumParams, NumAlphas, NumNums);
        MaxNums = max(MaxNums, NumNums);
        MaxAlphas = max(MaxAlphas, NumAlphas);
        inputProcessor->getObjectDefMaxArgs("Coil:Heating:WaterToAirHeatPump:EquationFit", NumParams, NumAlphas, NumNums);
        MaxNums = max(MaxNums, NumNums);
        MaxAlphas = max(MaxAlphas, NumAlphas);
        AlphArray.allocate(MaxAlphas);
        cAlphaFields.allocate(MaxAlphas);
        lAlphaBlanks.dimension(MaxAlphas, true);
        cNumericFields.allocate(MaxNums);
        lNumericBlanks.dimension(MaxNums, true);
        NumArray.dimension(MaxNums, 0.0);

        // Get the data for cooling coil
        CurrentModuleObject = "Coil:Cooling:WaterToAirHeatPump:EquationFit";

        for (WatertoAirHPNum = 1; WatertoAirHPNum <= NumCool; ++WatertoAirHPNum) {

            ++HPNum;

            inputProcessor->getObjectItem(state,
                                          CurrentModuleObject,
                                          HPNum,
                                          AlphArray,
                                          NumAlphas,
                                          NumArray,
                                          NumNums,
                                          IOStat,
                                          lNumericBlanks,
                                          lAlphaBlanks,
                                          cAlphaFields,
                                          cNumericFields);
            // ErrorsFound will be set to True if problem was found, left untouched otherwise
            VerifyUniqueCoilName(CurrentModuleObject, AlphArray(1), ErrorsFound, CurrentModuleObject + " Name");
            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Name = AlphArray(1);
            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WatertoAirHPType = "COOLING";
            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WAHPPlantTypeOfNum = TypeOf_CoilWAHPCoolingEquationFit;
            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).RatedAirVolFlowRate = NumArray(1);
            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).RatedWaterVolFlowRate = NumArray(2);
            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).RatedCapCoolTotal = NumArray(3);
            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).RatedCapCoolSens = NumArray(4);
            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).RatedCOPCool = NumArray(5);
            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).TotalCoolCap1 = NumArray(6);
            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).TotalCoolCap2 = NumArray(7);
            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).TotalCoolCap3 = NumArray(8);
            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).TotalCoolCap4 = NumArray(9);
            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).TotalCoolCap5 = NumArray(10);
            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).SensCoolCap1 = NumArray(11);
            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).SensCoolCap2 = NumArray(12);
            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).SensCoolCap3 = NumArray(13);
            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).SensCoolCap4 = NumArray(14);
            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).SensCoolCap5 = NumArray(15);
            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).SensCoolCap6 = NumArray(16);
            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).CoolPower1 = NumArray(17);
            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).CoolPower2 = NumArray(18);
            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).CoolPower3 = NumArray(19);
            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).CoolPower4 = NumArray(20);
            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).CoolPower5 = NumArray(21);
            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Twet_Rated = NumArray(22);
            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Gamma_Rated = NumArray(23);
            DataHeatBalance::HeatReclaimSimple_WAHPCoil(WatertoAirHPNum).Name = state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Name;
            DataHeatBalance::HeatReclaimSimple_WAHPCoil(WatertoAirHPNum).SourceType = CurrentModuleObject;

            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WaterInletNodeNum = GetOnlySingleNode(state,
                AlphArray(2), ErrorsFound, CurrentModuleObject, AlphArray(1), NodeType_Water, NodeConnectionType_Inlet, 2, ObjectIsNotParent);
            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WaterOutletNodeNum = GetOnlySingleNode(state,
                AlphArray(3), ErrorsFound, CurrentModuleObject, AlphArray(1), NodeType_Water, NodeConnectionType_Outlet, 2, ObjectIsNotParent);
            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).AirInletNodeNum = GetOnlySingleNode(state,
                AlphArray(4), ErrorsFound, CurrentModuleObject, AlphArray(1), NodeType_Air, NodeConnectionType_Inlet, 1, ObjectIsNotParent);
            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).AirOutletNodeNum = GetOnlySingleNode(state,
                AlphArray(5), ErrorsFound, CurrentModuleObject, AlphArray(1), NodeType_Air, NodeConnectionType_Outlet, 1, ObjectIsNotParent);

            TestCompSet(CurrentModuleObject, AlphArray(1), AlphArray(2), AlphArray(3), "Water Nodes");
            TestCompSet(CurrentModuleObject, AlphArray(1), AlphArray(4), AlphArray(5), "Air Nodes");

            // Setup Report variables for the cooling coil
            // CurrentModuleObject = "Coil:Cooling:WaterToAirHeatPump:EquationFit"
            SetupOutputVariable(state, "Cooling Coil Electricity Energy",
                                OutputProcessor::Unit::J,
                                state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Energy,
                                "System",
                                "Summed",
                                state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Name,
                                _,
                                "Electricity",
                                "Cooling",
                                _,
                                "System");
            SetupOutputVariable(state, "Cooling Coil Total Cooling Energy",
                                OutputProcessor::Unit::J,
                                state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).EnergyLoadTotal,
                                "System",
                                "Summed",
                                state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Name,
                                _,
                                "ENERGYTRANSFER",
                                "COOLINGCOILS",
                                _,
                                "System");
            SetupOutputVariable(state, "Cooling Coil Sensible Cooling Energy",
                                OutputProcessor::Unit::J,
                                state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).EnergySensible,
                                "System",
                                "Summed",
                                state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Name);
            SetupOutputVariable(state, "Cooling Coil Latent Cooling Energy",
                                OutputProcessor::Unit::J,
                                state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).EnergyLatent,
                                "System",
                                "Summed",
                                state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Name);
            SetupOutputVariable(state, "Cooling Coil Source Side Heat Transfer Energy",
                                OutputProcessor::Unit::J,
                                state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).EnergySource,
                                "System",
                                "Summed",
                                state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Name,
                                _,
                                "PLANTLOOPCOOLINGDEMAND",
                                "COOLINGCOILS",
                                _,
                                "System");

            // create predefined report entries
            PreDefTableEntry(pdchCoolCoilType, state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Name, CurrentModuleObject);
            PreDefTableEntry(pdchCoolCoilTotCap, state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Name, state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).RatedCapCoolTotal);
            PreDefTableEntry(pdchCoolCoilSensCap, state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Name, state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).RatedCapCoolSens);
            PreDefTableEntry(pdchCoolCoilLatCap,
                             state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Name,
                             state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).RatedCapCoolTotal - state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).RatedCapCoolSens);
            PreDefTableEntry(pdchCoolCoilSHR,
                             state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Name,
                             state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).RatedCapCoolSens / state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).RatedCapCoolTotal);
            PreDefTableEntry(pdchCoolCoilNomEff,
                             state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Name,
                             state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).RatedPowerCool / state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).RatedCapCoolTotal);
        }

        // Get the data for heating coil
        CurrentModuleObject = "Coil:Heating:WaterToAirHeatPump:EquationFit";

        for (WatertoAirHPNum = 1; WatertoAirHPNum <= NumHeat; ++WatertoAirHPNum) {

            ++HPNum;

            inputProcessor->getObjectItem(state,
                                          CurrentModuleObject,
                                          WatertoAirHPNum,
                                          AlphArray,
                                          NumAlphas,
                                          NumArray,
                                          NumNums,
                                          IOStat,
                                          lNumericBlanks,
                                          lAlphaBlanks,
                                          cAlphaFields,
                                          cNumericFields);
            // ErrorsFound will be set to True if problem was found, left untouched otherwise
            VerifyUniqueCoilName(CurrentModuleObject, AlphArray(1), ErrorsFound, CurrentModuleObject + " Name");

            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Name = AlphArray(1);
            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WatertoAirHPType = "HEATING";
            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WAHPPlantTypeOfNum = TypeOf_CoilWAHPHeatingEquationFit;
            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).RatedAirVolFlowRate = NumArray(1);
            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).RatedWaterVolFlowRate = NumArray(2);
            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).RatedCapHeat = NumArray(3);
            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).RatedCOPHeat = NumArray(4);
            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).HeatCap1 = NumArray(5);
            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).HeatCap2 = NumArray(6);
            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).HeatCap3 = NumArray(7);
            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).HeatCap4 = NumArray(8);
            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).HeatCap5 = NumArray(9);
            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).HeatPower1 = NumArray(10);
            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).HeatPower2 = NumArray(11);
            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).HeatPower3 = NumArray(12);
            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).HeatPower4 = NumArray(13);
            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).HeatPower5 = NumArray(14);

            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WaterInletNodeNum = GetOnlySingleNode(state,
                AlphArray(2), ErrorsFound, CurrentModuleObject, AlphArray(1), NodeType_Water, NodeConnectionType_Inlet, 2, ObjectIsNotParent);
            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WaterOutletNodeNum = GetOnlySingleNode(state,
                AlphArray(3), ErrorsFound, CurrentModuleObject, AlphArray(1), NodeType_Water, NodeConnectionType_Outlet, 2, ObjectIsNotParent);
            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).AirInletNodeNum = GetOnlySingleNode(state,
                AlphArray(4), ErrorsFound, CurrentModuleObject, AlphArray(1), NodeType_Air, NodeConnectionType_Inlet, 1, ObjectIsNotParent);
            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).AirOutletNodeNum = GetOnlySingleNode(state,
                AlphArray(5), ErrorsFound, CurrentModuleObject, AlphArray(1), NodeType_Air, NodeConnectionType_Outlet, 1, ObjectIsNotParent);

            TestCompSet(CurrentModuleObject, AlphArray(1), AlphArray(2), AlphArray(3), "Water Nodes");
            TestCompSet(CurrentModuleObject, AlphArray(1), AlphArray(4), AlphArray(5), "Air Nodes");

            // CurrentModuleObject = "Coil:Cooling:WaterToAirHeatPump:EquationFit"
            SetupOutputVariable(state, "Heating Coil Electricity Energy",
                                OutputProcessor::Unit::J,
                                state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Energy,
                                "System",
                                "Summed",
                                state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Name,
                                _,
                                "Electricity",
                                "Heating",
                                _,
                                "System");
            SetupOutputVariable(state, "Heating Coil Heating Energy",
                                OutputProcessor::Unit::J,
                                state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).EnergyLoadTotal,
                                "System",
                                "Summed",
                                state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Name,
                                _,
                                "ENERGYTRANSFER",
                                "HEATINGCOILS",
                                _,
                                "System");
            SetupOutputVariable(state, "Heating Coil Source Side Heat Transfer Energy",
                                OutputProcessor::Unit::J,
                                state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).EnergySource,
                                "System",
                                "Summed",
                                state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Name,
                                _,
                                "PLANTLOOPHEATINGDEMAND",
                                "HEATINGCOILS",
                                _,
                                "System");

            // create predefined report entries
            PreDefTableEntry(pdchHeatCoilType, state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Name, CurrentModuleObject);
            PreDefTableEntry(pdchHeatCoilNomCap, state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Name, state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).RatedCapHeat);
            PreDefTableEntry(pdchHeatCoilNomEff,
                             state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Name,
                             state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).RatedPowerHeat / state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).RatedCapHeat);
        }

        AlphArray.deallocate();
        cAlphaFields.deallocate();
        lAlphaBlanks.deallocate();
        cNumericFields.deallocate();
        lNumericBlanks.deallocate();
        NumArray.deallocate();

        if (ErrorsFound) {
            ShowFatalError(RoutineName + "Errors found getting input. Program terminates.");
        }

        for (HPNum = 1; HPNum <= state.dataWaterToAirHeatPumpSimple->NumWatertoAirHPs; ++HPNum) {

            if (state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WAHPPlantTypeOfNum == TypeOf_CoilWAHPCoolingEquationFit) {
                // COOLING COIL  Setup Report variables for the Heat Pump
                SetupOutputVariable(state, "Cooling Coil Electricity Rate",
                                    OutputProcessor::Unit::W,
                                    state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Power,
                                    "System",
                                    "Average",
                                    state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Name);
                SetupOutputVariable(state, "Cooling Coil Total Cooling Rate",
                                    OutputProcessor::Unit::W,
                                    state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).QLoadTotal,
                                    "System",
                                    "Average",
                                    state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Name);
                SetupOutputVariable(state, "Cooling Coil Sensible Cooling Rate",
                                    OutputProcessor::Unit::W,
                                    state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).QSensible,
                                    "System",
                                    "Average",
                                    state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Name);
                SetupOutputVariable(state, "Cooling Coil Latent Cooling Rate",
                                    OutputProcessor::Unit::W,
                                    state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).QLatent,
                                    "System",
                                    "Average",
                                    state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Name);
                SetupOutputVariable(state, "Cooling Coil Source Side Heat Transfer Rate",
                                    OutputProcessor::Unit::W,
                                    state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).QSource,
                                    "System",
                                    "Average",
                                    state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Name);
                SetupOutputVariable(state, "Cooling Coil Part Load Ratio",
                                    OutputProcessor::Unit::None,
                                    state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).PartLoadRatio,
                                    "System",
                                    "Average",
                                    state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Name);
                SetupOutputVariable(state, "Cooling Coil Runtime Fraction",
                                    OutputProcessor::Unit::None,
                                    state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).RunFrac,
                                    "System",
                                    "Average",
                                    state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Name);

                SetupOutputVariable(state, "Cooling Coil Air Mass Flow Rate",
                                    OutputProcessor::Unit::kg_s,
                                    state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).AirMassFlowRate,
                                    "System",
                                    "Average",
                                    state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Name);
                SetupOutputVariable(state, "Cooling Coil Air Inlet Temperature",
                                    OutputProcessor::Unit::C,
                                    state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).InletAirDBTemp,
                                    "System",
                                    "Average",
                                    state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Name);
                SetupOutputVariable(state, "Cooling Coil Air Inlet Humidity Ratio",
                                    OutputProcessor::Unit::kgWater_kgDryAir,
                                    state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).InletAirHumRat,
                                    "System",
                                    "Average",
                                    state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Name);
                SetupOutputVariable(state, "Cooling Coil Air Outlet Temperature",
                                    OutputProcessor::Unit::C,
                                    state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).OutletAirDBTemp,
                                    "System",
                                    "Average",
                                    state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Name);
                SetupOutputVariable(state, "Cooling Coil Air Outlet Humidity Ratio",
                                    OutputProcessor::Unit::kgWater_kgDryAir,
                                    state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).OutletAirHumRat,
                                    "System",
                                    "Average",
                                    state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Name);
                SetupOutputVariable(state, "Cooling Coil Source Side Mass Flow Rate",
                                    OutputProcessor::Unit::kg_s,
                                    state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WaterMassFlowRate,
                                    "System",
                                    "Average",
                                    state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Name);
                SetupOutputVariable(state, "Cooling Coil Source Side Inlet Temperature",
                                    OutputProcessor::Unit::C,
                                    state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).InletWaterTemp,
                                    "System",
                                    "Average",
                                    state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Name);
                SetupOutputVariable(state, "Cooling Coil Source Side Outlet Temperature",
                                    OutputProcessor::Unit::C,
                                    state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).OutletWaterTemp,
                                    "System",
                                    "Average",
                                    state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Name);

            } else if (state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WAHPPlantTypeOfNum == TypeOf_CoilWAHPHeatingEquationFit) {
                // HEATING COIL Setup Report variables for the Heat Pump
                SetupOutputVariable(state, "Heating Coil Electricity Rate",
                                    OutputProcessor::Unit::W,
                                    state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Power,
                                    "System",
                                    "Average",
                                    state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Name);
                SetupOutputVariable(state, "Heating Coil Heating Rate",
                                    OutputProcessor::Unit::W,
                                    state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).QLoadTotal,
                                    "System",
                                    "Average",
                                    state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Name);
                SetupOutputVariable(state, "Heating Coil Sensible Heating Rate",
                                    OutputProcessor::Unit::W,
                                    state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).QSensible,
                                    "System",
                                    "Average",
                                    state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Name);

                SetupOutputVariable(state, "Heating Coil Source Side Heat Transfer Rate",
                                    OutputProcessor::Unit::W,
                                    state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).QSource,
                                    "System",
                                    "Average",
                                    state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Name);
                SetupOutputVariable(state, "Heating Coil Part Load Ratio",
                                    OutputProcessor::Unit::None,
                                    state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).PartLoadRatio,
                                    "System",
                                    "Average",
                                    state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Name);
                SetupOutputVariable(state, "Heating Coil Runtime Fraction",
                                    OutputProcessor::Unit::None,
                                    state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).RunFrac,
                                    "System",
                                    "Average",
                                    state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Name);

                SetupOutputVariable(state, "Heating Coil Air Mass Flow Rate",
                                    OutputProcessor::Unit::kg_s,
                                    state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).AirMassFlowRate,
                                    "System",
                                    "Average",
                                    state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Name);
                SetupOutputVariable(state, "Heating Coil Air Inlet Temperature",
                                    OutputProcessor::Unit::C,
                                    state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).InletAirDBTemp,
                                    "System",
                                    "Average",
                                    state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Name);
                SetupOutputVariable(state, "Heating Coil Air Inlet Humidity Ratio",
                                    OutputProcessor::Unit::kgWater_kgDryAir,
                                    state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).InletAirHumRat,
                                    "System",
                                    "Average",
                                    state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Name);
                SetupOutputVariable(state, "Heating Coil Air Outlet Temperature",
                                    OutputProcessor::Unit::C,
                                    state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).OutletAirDBTemp,
                                    "System",
                                    "Average",
                                    state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Name);
                SetupOutputVariable(state, "Heating Coil Air Outlet Humidity Ratio",
                                    OutputProcessor::Unit::kgWater_kgDryAir,
                                    state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).OutletAirHumRat,
                                    "System",
                                    "Average",
                                    state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Name);
                SetupOutputVariable(state, "Heating Coil Source Side Mass Flow Rate",
                                    OutputProcessor::Unit::kg_s,
                                    state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WaterMassFlowRate,
                                    "System",
                                    "Average",
                                    state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Name);
                SetupOutputVariable(state, "Heating Coil Source Side Inlet Temperature",
                                    OutputProcessor::Unit::C,
                                    state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).InletWaterTemp,
                                    "System",
                                    "Average",
                                    state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Name);
                SetupOutputVariable(state, "Heating Coil Source Side Outlet Temperature",
                                    OutputProcessor::Unit::C,
                                    state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).OutletWaterTemp,
                                    "System",
                                    "Average",
                                    state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Name);
            }
        }
    }

    // Beginning Initialization Section of the Module
    //******************************************************************************

    void InitSimpleWatertoAirHP(EnergyPlusData &state,
                                int const HPNum,                           // Current HPNum under simulation
                                Real64 const MaxONOFFCyclesperHour,        // Maximum cycling rate of heat pump [cycles/hr]
                                Real64 const HPTimeConstant,               // Heat pump time constant [s]
                                Real64 const FanDelayTime,                 // Fan delay time, time delay for the HP's fan to
                                Real64 const SensLoad,                     // Control zone sensible load[W]
                                Real64 const LatentLoad,                   // Control zone latent load[W]
                                int const EP_UNUSED(CyclingScheme),        // fan operating mode
                                Real64 const EP_UNUSED(OnOffAirFlowRatio), // ratio of compressor on flow to average flow over time step
                                bool const FirstHVACIteration              // Iteration flag
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Arun Shenoy
        //       DATE WRITTEN   Nov 2003
        //       MODIFIED       na
        //       RE-ENGINEERED  Kenneth Tang (Jan 2005)

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine is for initializations of the Simple Water to Air HP Components.

        // METHODOLOGY EMPLOYED:
        // Uses the status flags to trigger initializations.

        // REFERENCES:

        // Using/Aliasing
        using DataGlobals::SysSizingCalc;
        using DataPlant::PlantLoop;
        using FluidProperties::GetDensityGlycol;
        using FluidProperties::GetSpecificHeatGlycol;
        using PlantUtilities::InitComponentNodes;
        using PlantUtilities::ScanPlantLoopsForObject;
        using PlantUtilities::SetComponentFlowRate;
        using Psychrometrics::PsyRhoAirFnPbTdbW;

        // Locals
        static Array1D_bool MySizeFlag; // used for sizing PTHP inputs one time

        // SUBROUTINE ARGUMENT DEFINITIONS:

        // shut off after compressor cycle off  [s]

        // SUBROUTINE PARAMETER DEFINITIONS:
        static std::string const RoutineName("InitSimpleWatertoAirHP");

        // INTERFACE BLOCK SPECIFICATIONS
        // na

        // DERIVED TYPE DEFINITIONS
        // na

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int AirInletNode;                // Node Number of the air inlet
        int WaterInletNode;              // Node Number of the Water inlet
        static Array1D_bool MyEnvrnFlag; // used for initializations each begin environment flag
        static Array1D_bool MyPlantScanFlag;
        Real64 rho; // local fluid density
        Real64 Cp;  // local fluid specific heat
        bool errFlag;

        if (state.dataWaterToAirHeatPumpSimple->MyOneTimeFlag) {
            // initialize the environment and sizing flags
            MySizeFlag.allocate(state.dataWaterToAirHeatPumpSimple->NumWatertoAirHPs);
            MyEnvrnFlag.allocate(state.dataWaterToAirHeatPumpSimple->NumWatertoAirHPs);
            MyPlantScanFlag.allocate(state.dataWaterToAirHeatPumpSimple->NumWatertoAirHPs);
            MySizeFlag = true;
            MyEnvrnFlag = true;
            MyPlantScanFlag = true;
            state.dataWaterToAirHeatPumpSimple->MyOneTimeFlag = false;
        }

        if (MyPlantScanFlag(HPNum) && allocated(PlantLoop)) {
            errFlag = false;
            ScanPlantLoopsForObject(state,
                                    state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Name,
                                    state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WAHPPlantTypeOfNum,
                                    state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).LoopNum,
                                    state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).LoopSide,
                                    state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).BranchNum,
                                    state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).CompNum,
                                    errFlag,
                                    _,
                                    _,
                                    _,
                                    _,
                                    _);
            if (errFlag) {
                ShowFatalError("InitSimpleWatertoAirHP: Program terminated for previous conditions.");
            }
            MyPlantScanFlag(HPNum) = false;
        }

        if (!SysSizingCalc && MySizeFlag(HPNum) && !MyPlantScanFlag(HPNum)) {
            // for each furnace, do the sizing once.
            SizeHVACWaterToAir(state, HPNum);

            MySizeFlag(HPNum) = false;
        }

        if (FirstHVACIteration) {
            if (state.dataWaterToAirHeatPumpSimple->SimpleHPTimeStepFlag(HPNum)) {
                if (state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WAHPPlantTypeOfNum == TypeOf_CoilWAHPCoolingEquationFit) {
                    if (state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).CompanionHeatingCoilNum > 0) {
                        if (state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WaterFlowMode) {
                            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).LastOperatingMode = Cooling;
                            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).CompanionHeatingCoilNum).LastOperatingMode = Cooling;
                        } else if (state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).CompanionHeatingCoilNum).WaterFlowMode) {
                            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).LastOperatingMode = Heating;
                            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).CompanionHeatingCoilNum).LastOperatingMode = Heating;
                        }
                        state.dataWaterToAirHeatPumpSimple->SimpleHPTimeStepFlag(state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).CompanionHeatingCoilNum) = false;
                    } else {
                        if (state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WaterFlowMode) {
                            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).LastOperatingMode = Cooling;
                        }
                    }
                    state.dataWaterToAirHeatPumpSimple->SimpleHPTimeStepFlag(HPNum) = false;
                } else {
                    // it is a heating coil
                    if (state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).CompanionCoolingCoilNum > 0) {
                        if (state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WaterFlowMode) {
                            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).LastOperatingMode = Heating;
                            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).CompanionCoolingCoilNum).LastOperatingMode = Heating;
                        } else if (state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).CompanionCoolingCoilNum).WaterFlowMode) {
                            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).LastOperatingMode = Cooling;
                            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).CompanionCoolingCoilNum).LastOperatingMode = Cooling;
                        }
                        state.dataWaterToAirHeatPumpSimple->SimpleHPTimeStepFlag(state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).CompanionCoolingCoilNum) = false;
                    } else {
                        if (state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WaterFlowMode) {
                            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).LastOperatingMode = Heating;
                        }
                    }
                    state.dataWaterToAirHeatPumpSimple->SimpleHPTimeStepFlag(HPNum) = false;
                }
            }
        } else {
            state.dataWaterToAirHeatPumpSimple->SimpleHPTimeStepFlag(HPNum) = true;
            if (state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WAHPPlantTypeOfNum == TypeOf_CoilWAHPCoolingEquationFit) {
                if (state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).CompanionHeatingCoilNum > 0)
                    state.dataWaterToAirHeatPumpSimple->SimpleHPTimeStepFlag(state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).CompanionHeatingCoilNum) = true;
            } else {
                if (state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).CompanionCoolingCoilNum > 0)
                    state.dataWaterToAirHeatPumpSimple->SimpleHPTimeStepFlag(state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).CompanionCoolingCoilNum) = true;
            }
        }

        // Do the Begin Environment initializations
        if (BeginEnvrnFlag && MyEnvrnFlag(HPNum) && !MyPlantScanFlag(HPNum)) {
            // Do the initializations to start simulation

            AirInletNode = state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).AirInletNodeNum;
            WaterInletNode = state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WaterInletNodeNum;

            // Initialize all report variables to a known state at beginning of simulation
            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).AirVolFlowRate = 0.0;
            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).InletAirDBTemp = 0.0;
            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).InletAirHumRat = 0.0;
            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).OutletAirDBTemp = 0.0;
            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).OutletAirHumRat = 0.0;
            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WaterVolFlowRate = 0.0;
            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WaterMassFlowRate = 0.0;
            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).InletWaterTemp = 0.0;
            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).InletWaterEnthalpy = 0.0;
            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).OutletWaterEnthalpy = 0.0;
            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).OutletWaterTemp = 0.0;
            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Power = 0.0;
            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).QLoadTotal = 0.0;
            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).QSensible = 0.0;
            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).QLatent = 0.0;
            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).QSource = 0.0;
            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Energy = 0.0;
            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).EnergyLoadTotal = 0.0;
            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).EnergySensible = 0.0;
            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).EnergyLatent = 0.0;
            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).EnergySource = 0.0;
            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).COP = 0.0;
            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).RunFrac = 0.0;
            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).PartLoadRatio = 0.0;

            rho = GetDensityGlycol(state, PlantLoop(state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).LoopNum).FluidName,
                                   InitConvTemp,
                                   PlantLoop(state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).LoopNum).FluidIndex,
                                   RoutineName);
            Cp = GetSpecificHeatGlycol(state, PlantLoop(state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).LoopNum).FluidName,
                                       InitConvTemp,
                                       PlantLoop(state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).LoopNum).FluidIndex,
                                       RoutineName);

            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).DesignWaterMassFlowRate = rho * state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).RatedWaterVolFlowRate;
            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).MaxONOFFCyclesperHour = MaxONOFFCyclesperHour;
            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).HPTimeConstant = HPTimeConstant;
            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).FanDelayTime = FanDelayTime;

            InitComponentNodes(0.0,
                               state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).DesignWaterMassFlowRate,
                               state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WaterInletNodeNum,
                               state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WaterOutletNodeNum,
                               state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).LoopNum,
                               state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).LoopSide,
                               state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).BranchNum,
                               state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).CompNum);

            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).SimFlag = true;

            MyEnvrnFlag(HPNum) = false;

        } // End If for the Begin Environment initializations

        if (!BeginEnvrnFlag) {
            MyEnvrnFlag(HPNum) = true;
        }

        // Do the following initializations (every time step): This should be the info from
        // the previous components outlets or the node data in this section.
        // First set the conditions for the air into the heat pump model

        // Set water and air inlet nodes

        AirInletNode = state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).AirInletNodeNum;
        WaterInletNode = state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WaterInletNodeNum;

        if ((SensLoad != 0.0 || LatentLoad != 0.0) && (Node(AirInletNode).MassFlowRate > 0.0)) {

            // Model requires the values to be calculated at full design flow rate for air and then scaled to part load ratio.
            // So always start the calculations by setting the air flow rate to design flow rate.

            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WaterMassFlowRate = state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).DesignWaterMassFlowRate;

            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).AirMassFlowRate =
                state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).RatedAirVolFlowRate *
                PsyRhoAirFnPbTdbW(StdBaroPress, Node(AirInletNode).Temp, Node(AirInletNode).HumRat, RoutineName);
            // If air flow is less than 25% rated flow. Then set air flow to the 25% of rated conditions
            if (state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).AirMassFlowRate <
                0.25 * state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).RatedAirVolFlowRate *
                    PsyRhoAirFnPbTdbW(StdBaroPress, Node(AirInletNode).Temp, Node(AirInletNode).HumRat, RoutineName)) {
                state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).AirMassFlowRate =
                    0.25 * state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).RatedAirVolFlowRate *
                    PsyRhoAirFnPbTdbW(StdBaroPress, Node(AirInletNode).Temp, Node(AirInletNode).HumRat, RoutineName);
            }
            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WaterFlowMode = true;
        } else { // heat pump is off
            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WaterFlowMode = false;
            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WaterMassFlowRate = 0.0;
            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).AirMassFlowRate = 0.0;
            if ((state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WaterCyclingMode) == WaterConstant) {
                if (state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WAHPPlantTypeOfNum == TypeOf_CoilWAHPCoolingEquationFit) {
                    if (state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).CompanionHeatingCoilNum > 0) {
                        if (state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).CompanionHeatingCoilNum).QLoadTotal > 0.0) {
                            // do nothing, there will be flow through this coil
                        } else if (state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).LastOperatingMode == Cooling) {
                            // set the flow rate to full design flow
                            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WaterMassFlowRate = state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).DesignWaterMassFlowRate;
                        }
                    } else {
                        if (state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).LastOperatingMode == Cooling) {
                            // set the flow rate to full design flow
                            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WaterMassFlowRate = state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).DesignWaterMassFlowRate;
                        }
                    }
                } else if (state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WAHPPlantTypeOfNum == TypeOf_CoilWAHPHeatingEquationFit) {
                    // It's a heating coil
                    if (state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).CompanionCoolingCoilNum > 0) {
                        if (state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).CompanionCoolingCoilNum).QLoadTotal > 0.0) {
                            // do nothing, there will be flow through this coil
                        } else if (state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).LastOperatingMode == Heating) {
                            // set the flow rate to full design flow
                            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WaterMassFlowRate = state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).DesignWaterMassFlowRate;
                        }
                    } else {
                        if (state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).LastOperatingMode == Heating) {
                            // set the flow rate to full design flow
                            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WaterMassFlowRate = state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).DesignWaterMassFlowRate;
                        }
                    }
                }
            }
        }

        SetComponentFlowRate(state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WaterMassFlowRate,
                             state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WaterInletNodeNum,
                             state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WaterOutletNodeNum,
                             state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).LoopNum,
                             state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).LoopSide,
                             state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).BranchNum,
                             state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).CompNum);

        state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).InletAirDBTemp = Node(AirInletNode).Temp;
        state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).InletAirHumRat = Node(AirInletNode).HumRat;
        state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).InletAirEnthalpy = Node(AirInletNode).Enthalpy;
        state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).InletWaterTemp = Node(WaterInletNode).Temp;
        state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).InletWaterEnthalpy = Node(WaterInletNode).Enthalpy;
        state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).OutletWaterTemp = state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).InletWaterTemp;
        state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).OutletWaterEnthalpy = state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).InletWaterEnthalpy;

        state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).MaxONOFFCyclesperHour = MaxONOFFCyclesperHour;
        state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).HPTimeConstant = HPTimeConstant;
        state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).FanDelayTime = FanDelayTime;

        // Outlet variables
        state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Power = 0.0;
        state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).QLoadTotal = 0.0;
        state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).QSensible = 0.0;
        state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).QLatent = 0.0;
        state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).QSource = 0.0;
        state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Energy = 0.0;
        state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).EnergyLoadTotal = 0.0;
        state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).EnergySensible = 0.0;
        state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).EnergyLatent = 0.0;
        state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).EnergySource = 0.0;
        state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).COP = 0.0;
        DataHeatBalance::HeatReclaimSimple_WAHPCoil(HPNum).AvailCapacity = 0.0;
    }

    void SizeHVACWaterToAir(EnergyPlusData &state, int const HPNum)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Richard Raustad, FSEC
        //       DATE WRITTEN   June 2009
        //       MODIFIED       August 2013 Daeho Kang, add component sizing table entries
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine is for sizing WSHP Components for which nominal capacities
        // and flow rates have not been specified in the input

        // METHODOLOGY EMPLOYED:
        // Obtains heating capacities and flow rates from the zone or system sizing arrays.
        // NOTE: For WSHP's we are sizing the heating capacity to be
        // equal to the cooling capacity.  Thus the cooling and
        // and heating capacities of a DX heat pump system will be identical. In real life the ARI
        // heating and cooling capacities are close but not identical.

        // REFERENCES:
        // na

        // Using/Aliasing
        using namespace Psychrometrics;
        using DataAirSystems::PrimaryAirSystem;
        using DataHVACGlobals::SmallAirVolFlow;
        using DataHVACGlobals::SmallLoad;
        using DataPlant::PlantLoop;
        using General::RoundSigDigits;
        using General::TrimSigDigits;
        using PlantUtilities::MyPlantSizingIndex;
        using PlantUtilities::RegisterPlantCompDesignFlow;
        using namespace OutputReportPredefined;
        using FluidProperties::GetDensityGlycol;
        using FluidProperties::GetSpecificHeatGlycol;

        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:

        // SUBROUTINE PARAMETER DEFINITIONS:
        static std::string const RoutineName("SizeWaterToAirCoil");
        static std::string const RoutineNameAlt("SizeHVACWaterToAir");

        // INTERFACE BLOCK SPECIFICATIONS
        // na

        // DERIVED TYPE DEFINITIONS
        // na

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 rhoair;
        Real64 MixTemp;
        Real64 MixHumRat;
        Real64 MixEnth;
        Real64 MixWetBulb;
        Real64 SupTemp;
        Real64 SupHumRat;
        Real64 SupEnth;
        Real64 OutTemp;
        Real64 ratioTDB;
        Real64 ratioTWB;
        Real64 ratioTS;
        Real64 OutAirFrac;
        Real64 VolFlowRate;
        Real64 CoolCapAtPeak;
        Real64 TotCapTempModFac = 1.0;
        Real64 SensCapAtPeak;
        Real64 SensCapTempModFac;
        Real64 TotalCapCoeff1; // 1st coefficient of the total cooling capacity performance curve
        Real64 TotalCapCoeff2; // 2nd coefficient of the total cooling capacity performance curve
        Real64 TotalCapCoeff3; // 3rd coefficient of the total cooling capacity performance curve
        Real64 TotalCapCoeff4; // 4th coefficient of the total cooling capacity performance curve
        Real64 TotalCapCoeff5; // 5th coefficient of the total cooling capacity performance curve
        Real64 SensCapCoeff1;  // 1st coefficient of the sensible cooling capacity performance curve
        Real64 SensCapCoeff2;  // 2nd coefficient of the sensible cooling capacity performance curve
        Real64 SensCapCoeff3;  // 3rd coefficient of the sensible cooling capacity performance curve
        Real64 SensCapCoeff4;  // 4th coefficient of the sensible cooling capacity performance curve
        Real64 SensCapCoeff5;  // 5th coefficient of the sensible cooling capacity performance curve
        Real64 SensCapCoeff6;  // 6th coefficient of the sensible cooling capacity performance curve
        int TimeStepNumAtMax;
        int DDNum;
        int PltSizNum;
        bool RatedCapCoolTotalAutoSized;
        bool RatedCapCoolSensAutoSized;
        bool ErrorsFound;
        Real64 SystemCapacity;
        Real64 rho;
        Real64 Cp;
        bool IsAutoSize;                  // Indicator to autosize
        bool HardSizeNoDesRun;            // Indicator to hardsize and no sizing run
        Real64 RatedAirVolFlowRateDes;    // Autosized rated air flow for reporting
        Real64 RatedAirVolFlowRateUser;   // Hardsized rated air flow for reporting
        Real64 RatedCapCoolTotalDes;      // Autosized rated cooling capacity for reporting
        Real64 RatedCapCoolTotalUser;     // Hardsized rated cooling capacity for reporting
        Real64 RatedCapCoolSensDes;       // Autosized rated sensible cooling capacity for reporting
        Real64 RatedCapCoolSensUser;      // Hardsized rated sensible cooling capacity for reporting
        Real64 RatedCapHeatDes;           // Autosized rated heating capacity for reporting
        Real64 RatedCapHeatUser;          // Hardsized rated heating capacity for reporting
        Real64 RatedWaterVolFlowRateDes;  // Autosized rated water flow rate for reporting
        Real64 RatedWaterVolFlowRateUser; // Hardsized rated water flow rate for reporting
        bool SizingDesRunThisAirSys;      // true if a particular air system had a Sizing:System object and system sizing done
        bool SizingDesRunThisZone;        // true if a particular zone had a Sizing:Zone object and zone sizing was done

        PltSizNum = 0;
        ErrorsFound = false;
        IsAutoSize = false;
        if (SysSizingRunDone || ZoneSizingRunDone) {
            HardSizeNoDesRun = false;
        } else {
            HardSizeNoDesRun = true;
        }
        if (CurSysNum > 0) {
            CheckThisAirSystemForSizing(CurSysNum, SizingDesRunThisAirSys);
        } else {
            SizingDesRunThisAirSys = false;
        }
        if (CurZoneEqNum > 0) {
            CheckThisZoneForSizing(CurZoneEqNum, SizingDesRunThisZone);
        } else {
            SizingDesRunThisZone = false;
        }
        RatedAirVolFlowRateDes = 0.0;
        RatedAirVolFlowRateUser = 0.0;
        RatedCapCoolTotalDes = 0.0;
        RatedCapCoolTotalUser = 0.0;
        RatedCapCoolSensDes = 0.0;
        RatedCapCoolSensUser = 0.0;
        RatedCapHeatDes = 0.0;
        RatedCapHeatUser = 0.0;
        RatedWaterVolFlowRateDes = 0.0;
        RatedWaterVolFlowRateUser = 0.0;
        std::string CompType = "COIL:" + state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WatertoAirHPType + ":WATERTOAIRHEATPUMP:EQUATIONFIT";

        if (state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).RatedAirVolFlowRate == AutoSize) {
            IsAutoSize = true;
        }
        if (CurSysNum > 0) {
            if (!IsAutoSize && !SizingDesRunThisAirSys) { // Simulation continue
                HardSizeNoDesRun = true;
                if (state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).RatedAirVolFlowRate > 0.0) {
                    BaseSizer::reportSizerOutput("COIL:" + state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WatertoAirHPType + ":WATERTOAIRHEATPUMP:EQUATIONFIT",
                                                 state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Name,
                                                 "User-Specified Rated Air Flow Rate [m3/s]",
                                                 state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).RatedAirVolFlowRate);
                }
            } else {
                CheckSysSizing("COIL:" + state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WatertoAirHPType + ":WATERTOAIRHEATPUMP:EQUATIONFIT",
                               state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Name);
                if (FinalSysSizing(CurSysNum).DesMainVolFlow >= SmallAirVolFlow) {
                    RatedAirVolFlowRateDes = FinalSysSizing(CurSysNum).DesMainVolFlow;
                } else {
                    RatedAirVolFlowRateDes = 0.0;
                }
            }
        } else if (CurZoneEqNum > 0) {
            if (!IsAutoSize && !SizingDesRunThisZone) { // Simulation continue
                HardSizeNoDesRun = true;
                if (state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).RatedAirVolFlowRate > 0.0) {
                    BaseSizer::reportSizerOutput("COIL:" + state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WatertoAirHPType + ":WATERTOAIRHEATPUMP:EQUATIONFIT",
                                                 state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Name,
                                                 "User-Specified Rated Air Flow Rate [m3/s]",
                                                 state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).RatedAirVolFlowRate);
                }
            } else {
                CheckZoneSizing("COIL:" + state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WatertoAirHPType + ":WATERTOAIRHEATPUMP:EQUATIONFIT",
                                state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Name);
                RatedAirVolFlowRateDes = max(FinalZoneSizing(CurZoneEqNum).DesCoolVolFlow, FinalZoneSizing(CurZoneEqNum).DesHeatVolFlow);
                if (RatedAirVolFlowRateDes < SmallAirVolFlow) {
                    RatedAirVolFlowRateDes = 0.0;
                }
            }
        }
        if (!HardSizeNoDesRun) {
            if (IsAutoSize) {
                state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).RatedAirVolFlowRate = RatedAirVolFlowRateDes;
                BaseSizer::reportSizerOutput("COIL:" + state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WatertoAirHPType + ":WATERTOAIRHEATPUMP:EQUATIONFIT",
                                             state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Name,
                                             "Design Size Rated Air Flow Rate [m3/s]",
                                             RatedAirVolFlowRateDes);
            } else {
                if (state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).RatedAirVolFlowRate > 0.0 && RatedAirVolFlowRateDes > 0.0 && !HardSizeNoDesRun) {
                    RatedAirVolFlowRateUser = state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).RatedAirVolFlowRate;
                    BaseSizer::reportSizerOutput("COIL:" + state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WatertoAirHPType + ":WATERTOAIRHEATPUMP:EQUATIONFIT",
                                                 state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Name,
                                                 "Design Size Rated Air Flow Rate [m3/s]",
                                                 RatedAirVolFlowRateDes,
                                                 "User-Specified Rated Air Flow Rate [m3/s]",
                                                 RatedAirVolFlowRateUser);
                    if (DisplayExtraWarnings) {
                        if ((std::abs(RatedAirVolFlowRateDes - RatedAirVolFlowRateUser) / RatedAirVolFlowRateUser) > AutoVsHardSizingThreshold) {
                            ShowMessage("SizeHVACWaterToAir: Potential issue with equipment sizing for coil " +
                                        state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WatertoAirHPType + ":WATERTOAIRHEATPUMP:EQUATIONFIT \"" +
                                        state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Name + "\"");
                            ShowContinueError("User-Specified Rated Air Volume Flow Rate of " + RoundSigDigits(RatedAirVolFlowRateUser, 5) +
                                              " [m3/s]");
                            ShowContinueError("differs from Design Size Rated Air Volume Flow Rate of " + RoundSigDigits(RatedAirVolFlowRateDes, 5) +
                                              " [m3/s]");
                            ShowContinueError("This may, or may not, indicate mismatched component sizes.");
                            ShowContinueError("Verify that the value entered is intended and is consistent with other components.");
                        }
                    }
                }
            }
        }

        RatedCapCoolTotalAutoSized = false;
        RatedCapCoolSensAutoSized = false;

        Real64 FanCoolLoad = 0.0;
        if (state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WatertoAirHPType == "COOLING") {
            // size rated total cooling capacity
            if (state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).RatedCapCoolTotal == AutoSize && state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WatertoAirHPType == "COOLING") {
                RatedCapCoolTotalAutoSized = true;
            }
            if (SizingDesRunThisAirSys || SizingDesRunThisZone) HardSizeNoDesRun = false;
            if (CurSysNum > 0) {
                if (!RatedCapCoolTotalAutoSized && !SizingDesRunThisAirSys) { // Simulation continue
                    HardSizeNoDesRun = true;
                    if (state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).RatedCapCoolTotal > 0.0) {
                        BaseSizer::reportSizerOutput("COIL:" + state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WatertoAirHPType + ":WATERTOAIRHEATPUMP:EQUATIONFIT",
                                                     state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Name,
                                                     "User-Specified Rated Total Cooling Capacity [W]",
                                                     state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).RatedCapCoolTotal);
                    }
                } else {
                    CheckSysSizing("COIL:" + state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WatertoAirHPType + ":WATERTOAIRHEATPUMP:EQUATIONFIT",
                                   state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Name);
                    VolFlowRate = state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).RatedAirVolFlowRate;
                    if (VolFlowRate >= SmallAirVolFlow) {
                        if (CurOASysNum > 0) { // coil is in the OA stream
                            MixTemp = FinalSysSizing(CurSysNum).OutTempAtCoolPeak;
                            MixHumRat = FinalSysSizing(CurSysNum).OutHumRatAtCoolPeak;
                            SupTemp = FinalSysSizing(CurSysNum).PrecoolTemp;
                            SupHumRat = FinalSysSizing(CurSysNum).PrecoolHumRat;
                        } else { // coil is on the main air loop
                            SupTemp = FinalSysSizing(CurSysNum).CoolSupTemp;
                            SupHumRat = FinalSysSizing(CurSysNum).CoolSupHumRat;
                            if (PrimaryAirSystem(CurSysNum).NumOACoolCoils == 0) { // there is no precooling of the OA stream
                                MixTemp = FinalSysSizing(CurSysNum).MixTempAtCoolPeak;
                                MixHumRat = FinalSysSizing(CurSysNum).MixHumRatAtCoolPeak;
                            } else { // there is precooling of OA stream
                                if (VolFlowRate > 0.0) {
                                    OutAirFrac = FinalSysSizing(CurSysNum).DesOutAirVolFlow / VolFlowRate;
                                } else {
                                    OutAirFrac = 1.0;
                                }
                                OutAirFrac = min(1.0, max(0.0, OutAirFrac));
                                MixTemp = OutAirFrac * FinalSysSizing(CurSysNum).PrecoolTemp +
                                          (1.0 - OutAirFrac) * FinalSysSizing(CurSysNum).RetTempAtCoolPeak;
                                MixHumRat = OutAirFrac * FinalSysSizing(CurSysNum).PrecoolHumRat +
                                            (1.0 - OutAirFrac) * FinalSysSizing(CurSysNum).RetHumRatAtCoolPeak;
                            }
                        }
                        // supply air condition is capped with that of mixed air to avoid SHR > 1.0
                        SupTemp = min(MixTemp, SupTemp);
                        SupHumRat = min(MixHumRat, SupHumRat);
                        OutTemp = FinalSysSizing(CurSysNum).OutTempAtCoolPeak;
                        rhoair = PsyRhoAirFnPbTdbW(StdBaroPress, MixTemp, MixHumRat, RoutineName);
                        MixEnth = PsyHFnTdbW(MixTemp, MixHumRat);
                        SupEnth = PsyHFnTdbW(SupTemp, SupHumRat);
                        TotalCapCoeff1 = state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).TotalCoolCap1;
                        TotalCapCoeff2 = state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).TotalCoolCap2;
                        TotalCapCoeff3 = state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).TotalCoolCap3;
                        TotalCapCoeff4 = state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).TotalCoolCap4;
                        TotalCapCoeff5 = state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).TotalCoolCap5;
                        Real64 FanCoolLoad = 0.0;
                        if (DataFanEnumType > -1 && DataFanIndex > -1) { // add fan heat to coil load
                            switch (DataFanEnumType) {
                            case DataAirSystems::structArrayLegacyFanModels: {
                                FanCoolLoad = Fans::FanDesHeatGain(state, DataFanIndex, VolFlowRate);
                                break;
                            }
                            case DataAirSystems::objectVectorOOFanSystemModel: {
                                FanCoolLoad = HVACFan::fanObjs[DataFanIndex]->getFanDesignHeatGain(state, VolFlowRate);
                                break;
                            }
                            case DataAirSystems::fanModelTypeNotYetSet: {
                                // do nothing
                                break;
                            }
                            } // end switch
                            Real64 CpAir = PsyCpAirFnW(MixHumRat);
                            if (PrimaryAirSystem(CurSysNum).supFanLocation == DataAirSystems::fanPlacement::BlowThru) {
                                MixTemp += FanCoolLoad / (CpAir * rhoair * VolFlowRate);
                            } else if (PrimaryAirSystem(CurSysNum).supFanLocation == DataAirSystems::fanPlacement::DrawThru) {
                                SupTemp -= FanCoolLoad / (CpAir * rhoair * VolFlowRate);
                            }
                        }
                        CoolCapAtPeak = (rhoair * VolFlowRate * (MixEnth - SupEnth)) + FanCoolLoad;
                        CoolCapAtPeak = max(0.0, CoolCapAtPeak);
                        MixWetBulb = PsyTwbFnTdbWPb(MixTemp, MixHumRat, StdBaroPress, RoutineName);
                        ratioTWB = (MixWetBulb + 273.15) / 283.15;
                        // rated condenser water inlet temperature of 85F
                        ratioTS = (((85.0 - 32.0) / 1.8) + 273.15) / 283.15;
                        TotCapTempModFac = TotalCapCoeff1 + (ratioTWB * TotalCapCoeff2) + (ratioTS * TotalCapCoeff3) + (1.0 * TotalCapCoeff4) +
                                           (1.0 * TotalCapCoeff5);
                        if (TotCapTempModFac > 0.0) {
                            RatedCapCoolTotalDes = CoolCapAtPeak / TotCapTempModFac;
                        } else {
                            RatedCapCoolTotalDes = CoolCapAtPeak;
                        }
                        coilSelectionReportObj->setCoilEntAirTemp(state, state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Name, CompType, MixTemp, CurSysNum, CurZoneEqNum);
                        coilSelectionReportObj->setCoilEntAirHumRat(state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Name, CompType, MixHumRat);
                        coilSelectionReportObj->setCoilLvgAirTemp(state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Name, CompType, SupTemp);
                        coilSelectionReportObj->setCoilLvgAirHumRat(state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Name, CompType, SupHumRat);
                    } else {
                        RatedCapCoolTotalDes = 0.0;
                    }
                }
            } else if (CurZoneEqNum > 0) {
                if (!RatedCapCoolTotalAutoSized && !SizingDesRunThisZone) { // Simulation continue
                    HardSizeNoDesRun = true;
                    if (state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).RatedCapCoolTotal > 0.0) {
                        BaseSizer::reportSizerOutput("COIL:" + state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WatertoAirHPType + ":WATERTOAIRHEATPUMP:EQUATIONFIT",
                                                     state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Name,
                                                     "User-Specified Rated Total Cooling Capacity [W]",
                                                     state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).RatedCapCoolTotal);
                    }
                } else {
                    CheckZoneSizing("COIL:" + state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WatertoAirHPType + ":WATERTOAIRHEATPUMP:EQUATIONFIT",
                                    state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Name);
                    VolFlowRate = state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).RatedAirVolFlowRate;
                    if (VolFlowRate >= SmallAirVolFlow) {
                        if (ZoneEqDXCoil) {
                            if (ZoneEqSizing(CurZoneEqNum).OAVolFlow > 0.0) {
                                MixTemp = FinalZoneSizing(CurZoneEqNum).DesCoolCoilInTemp;
                                MixHumRat = FinalZoneSizing(CurZoneEqNum).DesCoolCoilInHumRat;
                            } else {
                                MixTemp = FinalZoneSizing(CurZoneEqNum).ZoneRetTempAtCoolPeak;
                                MixHumRat = FinalZoneSizing(CurZoneEqNum).ZoneHumRatAtCoolPeak;
                            }
                        } else {
                            MixTemp = FinalZoneSizing(CurZoneEqNum).DesCoolCoilInTemp;
                            MixHumRat = FinalZoneSizing(CurZoneEqNum).DesCoolCoilInHumRat;
                        }
                        SupTemp = FinalZoneSizing(CurZoneEqNum).CoolDesTemp;
                        SupHumRat = FinalZoneSizing(CurZoneEqNum).CoolDesHumRat;
                        // supply air condition is capped with that of mixed air to avoid SHR > 1.0
                        SupTemp = min(MixTemp, SupTemp);
                        SupHumRat = min(MixHumRat, SupHumRat);
                        TimeStepNumAtMax = FinalZoneSizing(CurZoneEqNum).TimeStepNumAtCoolMax;
                        DDNum = FinalZoneSizing(CurZoneEqNum).CoolDDNum;
                        if (DDNum > 0 && TimeStepNumAtMax > 0) {
                            OutTemp = DesDayWeath(DDNum).Temp(TimeStepNumAtMax);
                        } else {
                            OutTemp = 0.0;
                        }
                        rhoair = PsyRhoAirFnPbTdbW(StdBaroPress, MixTemp, MixHumRat, RoutineName);
                        MixEnth = PsyHFnTdbW(MixTemp, MixHumRat);
                        SupEnth = PsyHFnTdbW(SupTemp, SupHumRat);
                        TotalCapCoeff1 = state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).TotalCoolCap1;
                        TotalCapCoeff2 = state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).TotalCoolCap2;
                        TotalCapCoeff3 = state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).TotalCoolCap3;
                        TotalCapCoeff4 = state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).TotalCoolCap4;
                        TotalCapCoeff5 = state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).TotalCoolCap5;
                        if (DataFanEnumType > -1 && DataFanIndex > -1) { // add fan heat to coil load
                            switch (DataFanEnumType) {
                            case DataAirSystems::structArrayLegacyFanModels: {
                                FanCoolLoad = Fans::FanDesHeatGain(state, DataFanIndex, VolFlowRate);
                                break;
                            }
                            case DataAirSystems::objectVectorOOFanSystemModel: {
                                FanCoolLoad = HVACFan::fanObjs[DataFanIndex]->getFanDesignHeatGain(state, VolFlowRate);
                                break;
                            }
                            case DataAirSystems::fanModelTypeNotYetSet: {
                                // do nothing
                                break;
                            }
                            } // end switch
                            Real64 CpAir = PsyCpAirFnW(MixHumRat);
                            if (DataSizing::DataFanPlacement == DataSizing::zoneFanPlacement::zoneBlowThru) {
                                MixTemp += FanCoolLoad / (CpAir * rhoair * VolFlowRate);
                            } else {
                                SupTemp -= FanCoolLoad / (CpAir * rhoair * VolFlowRate);
                            }
                        }
                        CoolCapAtPeak = (rhoair * VolFlowRate * (MixEnth - SupEnth)) + FanCoolLoad;
                        CoolCapAtPeak = max(0.0, CoolCapAtPeak);
                        MixWetBulb = PsyTwbFnTdbWPb(MixTemp, MixHumRat, StdBaroPress, RoutineName);
                        ratioTWB = (MixWetBulb + 273.15) / 283.15;
                        // rated condenser water inlet temperature of 85F
                        ratioTS = (((85.0 - 32.0) / 1.8) + 273.15) / 283.15;
                        TotCapTempModFac = TotalCapCoeff1 + (ratioTWB * TotalCapCoeff2) + (ratioTS * TotalCapCoeff3) + (1.0 * TotalCapCoeff4) +
                                           (1.0 * TotalCapCoeff5);
                        if (TotCapTempModFac > 0.0) {
                            RatedCapCoolTotalDes = CoolCapAtPeak / TotCapTempModFac;
                        } else {
                            RatedCapCoolTotalDes = CoolCapAtPeak;
                        }
                        coilSelectionReportObj->setCoilEntAirTemp(state, state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Name, CompType, MixTemp, CurSysNum, CurZoneEqNum);
                        coilSelectionReportObj->setCoilEntAirHumRat(state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Name, CompType, MixHumRat);
                        coilSelectionReportObj->setCoilLvgAirTemp(state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Name, CompType, SupTemp);
                        coilSelectionReportObj->setCoilLvgAirHumRat(state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Name, CompType, SupHumRat);
                    } else {
                        RatedCapCoolTotalDes = 0.0;
                    }
                }
                if (RatedCapCoolTotalDes < SmallLoad) {
                    RatedCapCoolTotalDes = 0.0;
                }
            }
            // size rated sensible cooling capacity
            if (state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).RatedCapCoolSens == AutoSize && state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WatertoAirHPType == "COOLING") {
                RatedCapCoolSensAutoSized = true;
            }
            if (SizingDesRunThisAirSys || SizingDesRunThisZone) HardSizeNoDesRun = false;
            if (CurSysNum > 0) {
                if (!RatedCapCoolSensAutoSized && !SizingDesRunThisAirSys) { // Simulation continue
                    HardSizeNoDesRun = true;
                    if (state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).RatedCapCoolSens > 0.0) {
                        BaseSizer::reportSizerOutput("COIL:" + state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WatertoAirHPType + ":WATERTOAIRHEATPUMP:EQUATIONFIT",
                                                     state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Name,
                                                     "User-Specified Rated Sensible Cooling Capacity [W]",
                                                     state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).RatedCapCoolSens);
                    }
                } else {
                    CheckSysSizing("COIL:" + state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WatertoAirHPType + ":WATERTOAIRHEATPUMP:EQUATIONFIT",
                                   state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Name);
                    VolFlowRate = state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).RatedAirVolFlowRate;
                    if (VolFlowRate >= SmallAirVolFlow) {
                        if (CurOASysNum > 0) { // coil is in the OA stream
                            MixTemp = FinalSysSizing(CurSysNum).OutTempAtCoolPeak;
                            MixHumRat = FinalSysSizing(CurSysNum).OutHumRatAtCoolPeak;
                            SupTemp = FinalSysSizing(CurSysNum).PrecoolTemp;
                            SupHumRat = FinalSysSizing(CurSysNum).PrecoolHumRat;
                        } else { // coil is on the main air loop
                            SupTemp = FinalSysSizing(CurSysNum).CoolSupTemp;
                            SupHumRat = FinalSysSizing(CurSysNum).CoolSupHumRat;
                            if (PrimaryAirSystem(CurSysNum).NumOACoolCoils == 0) { // there is no precooling of the OA stream
                                MixTemp = FinalSysSizing(CurSysNum).MixTempAtCoolPeak;
                                MixHumRat = FinalSysSizing(CurSysNum).MixHumRatAtCoolPeak;
                            } else { // there is precooling of OA stream
                                if (VolFlowRate > 0.0) {
                                    OutAirFrac = FinalSysSizing(CurSysNum).DesOutAirVolFlow / VolFlowRate;
                                } else {
                                    OutAirFrac = 1.0;
                                }
                                OutAirFrac = min(1.0, max(0.0, OutAirFrac));
                                MixTemp = OutAirFrac * FinalSysSizing(CurSysNum).PrecoolTemp +
                                          (1.0 - OutAirFrac) * FinalSysSizing(CurSysNum).RetTempAtCoolPeak;
                                MixHumRat = OutAirFrac * FinalSysSizing(CurSysNum).PrecoolHumRat +
                                            (1.0 - OutAirFrac) * FinalSysSizing(CurSysNum).RetHumRatAtCoolPeak;
                            }
                        }
                        // supply air condition is capped with that of mixed air to avoid SHR > 1.0
                        SupTemp = min(MixTemp, SupTemp);
                        SupHumRat = min(MixHumRat, SupHumRat);
                        OutTemp = FinalSysSizing(CurSysNum).OutTempAtCoolPeak;
                        rhoair = PsyRhoAirFnPbTdbW(StdBaroPress, MixTemp, MixHumRat, RoutineName);
                        MixEnth = PsyHFnTdbW(MixTemp, MixHumRat);
                        SupEnth = PsyHFnTdbW(SupTemp, MixHumRat);
                        SensCapCoeff1 = state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).SensCoolCap1;
                        SensCapCoeff2 = state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).SensCoolCap2;
                        SensCapCoeff3 = state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).SensCoolCap3;
                        SensCapCoeff4 = state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).SensCoolCap4;
                        SensCapCoeff5 = state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).SensCoolCap5;
                        SensCapCoeff6 = state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).SensCoolCap6;
                        Real64 FanCoolLoad = 0.0;
                        if (DataFanEnumType > -1 && DataFanIndex > -1) { // add fan heat to coil load
                            switch (DataFanEnumType) {
                            case DataAirSystems::structArrayLegacyFanModels: {
                                FanCoolLoad = Fans::FanDesHeatGain(state, DataFanIndex, VolFlowRate);
                                break;
                            }
                            case DataAirSystems::objectVectorOOFanSystemModel: {
                                FanCoolLoad = HVACFan::fanObjs[DataFanIndex]->getFanDesignHeatGain(state, VolFlowRate);
                                break;
                            }
                            case DataAirSystems::fanModelTypeNotYetSet: {
                                // do nothing
                                break;
                            }
                            } // end switch
                            Real64 CpAir = PsyCpAirFnW(MixHumRat);
                            if (PrimaryAirSystem(CurSysNum).supFanLocation == DataAirSystems::fanPlacement::BlowThru) {
                                MixTemp += FanCoolLoad / (CpAir * rhoair * VolFlowRate);
                            } else if (PrimaryAirSystem(CurSysNum).supFanLocation == DataAirSystems::fanPlacement::DrawThru) {
                                SupTemp -= FanCoolLoad / (CpAir * rhoair * VolFlowRate);
                            }
                        }
                        // Sensible capacity is calculated from enthalpy difference with constant humidity ratio, i.e.,
                        // there is only temperature difference between entering and leaving air enthalpy. Previously
                        // it was calculated using m.cp.dT
                        SensCapAtPeak = (rhoair * VolFlowRate * (MixEnth - SupEnth)) + FanCoolLoad;
                        SensCapAtPeak = max(0.0, SensCapAtPeak);
                        MixWetBulb = PsyTwbFnTdbWPb(MixTemp, MixHumRat, StdBaroPress, RoutineName);
                        ratioTDB = (MixTemp + 273.15) / 283.15;
                        ratioTWB = (MixWetBulb + 273.15) / 283.15;
                        // rated condenser water inlet temperature of 85F
                        ratioTS = (((85.0 - 32.0) / 1.8) + 273.15) / 283.15;
                        SensCapTempModFac = SensCapCoeff1 + (ratioTDB * SensCapCoeff2) + (ratioTWB * SensCapCoeff3) + (ratioTS * SensCapCoeff4) +
                                            (1.0 * SensCapCoeff5) + (1.0 * SensCapCoeff6);
                        RatedCapCoolSensDes = SensCapAtPeak / SensCapTempModFac;
                    } else {
                        RatedCapCoolSensDes = 0.0;
                    }
                }
            } else if (CurZoneEqNum > 0) {
                if (!RatedCapCoolSensAutoSized && !SizingDesRunThisZone) { // Simulation continue
                    HardSizeNoDesRun = true;
                    if (state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).RatedCapCoolSens > 0.0) {
                        BaseSizer::reportSizerOutput("COIL:" + state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WatertoAirHPType + ":WATERTOAIRHEATPUMP:EQUATIONFIT",
                                                     state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Name,
                                                     "User-Specified Rated Sensible Cooling Capacity [W]",
                                                     state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).RatedCapCoolSens);
                    }
                } else {
                    CheckZoneSizing("COIL:" + state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WatertoAirHPType + ":WATERTOAIRHEATPUMP:EQUATIONFIT",
                                    state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Name);
                    VolFlowRate = state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).RatedAirVolFlowRate;
                    if (VolFlowRate >= SmallAirVolFlow) {
                        if (ZoneEqDXCoil) {
                            if (ZoneEqSizing(CurZoneEqNum).OAVolFlow > 0.0) {
                                MixTemp = FinalZoneSizing(CurZoneEqNum).DesCoolCoilInTemp;
                                MixHumRat = FinalZoneSizing(CurZoneEqNum).DesCoolCoilInHumRat;
                            } else {
                                MixTemp = FinalZoneSizing(CurZoneEqNum).ZoneRetTempAtCoolPeak;
                                MixHumRat = FinalZoneSizing(CurZoneEqNum).ZoneHumRatAtCoolPeak;
                            }
                        } else {
                            MixTemp = FinalZoneSizing(CurZoneEqNum).DesCoolCoilInTemp;
                            MixHumRat = FinalZoneSizing(CurZoneEqNum).DesCoolCoilInHumRat;
                        }
                        SupTemp = FinalZoneSizing(CurZoneEqNum).CoolDesTemp;
                        SupHumRat = FinalZoneSizing(CurZoneEqNum).CoolDesHumRat;
                        // supply air condition is capped with that of mixed air to avoid SHR > 1.0
                        SupTemp = min(MixTemp, SupTemp);
                        SupHumRat = min(MixHumRat, SupHumRat);
                        TimeStepNumAtMax = FinalZoneSizing(CurZoneEqNum).TimeStepNumAtCoolMax;
                        DDNum = FinalZoneSizing(CurZoneEqNum).CoolDDNum;
                        if (DDNum > 0 && TimeStepNumAtMax > 0) {
                            OutTemp = DesDayWeath(DDNum).Temp(TimeStepNumAtMax);
                        } else {
                            OutTemp = 0.0;
                        }
                        rhoair = PsyRhoAirFnPbTdbW(StdBaroPress, MixTemp, MixHumRat, RoutineName);
                        MixEnth = PsyHFnTdbW(MixTemp, MixHumRat);
                        SupEnth = PsyHFnTdbW(SupTemp, MixHumRat);
                        SensCapCoeff1 = state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).SensCoolCap1;
                        SensCapCoeff2 = state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).SensCoolCap2;
                        SensCapCoeff3 = state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).SensCoolCap3;
                        SensCapCoeff4 = state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).SensCoolCap4;
                        SensCapCoeff5 = state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).SensCoolCap5;
                        SensCapCoeff6 = state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).SensCoolCap6;
                        Real64 FanCoolLoad = 0.0;
                        if (DataFanEnumType > -1 && DataFanIndex > -1) { // add fan heat to coil load
                            switch (DataFanEnumType) {
                            case DataAirSystems::structArrayLegacyFanModels: {
                                FanCoolLoad = Fans::FanDesHeatGain(state, DataFanIndex, VolFlowRate);
                                break;
                            }
                            case DataAirSystems::objectVectorOOFanSystemModel: {
                                FanCoolLoad = HVACFan::fanObjs[DataFanIndex]->getFanDesignHeatGain(state, VolFlowRate);
                                break;
                            }
                            case DataAirSystems::fanModelTypeNotYetSet: {
                                // do nothing
                                break;
                            }
                            } // end switch
                            Real64 CpAir = PsyCpAirFnW(MixHumRat);
                            if (DataSizing::DataFanPlacement == DataSizing::zoneFanPlacement::zoneBlowThru) {
                                MixTemp += FanCoolLoad / (CpAir * rhoair * VolFlowRate);
                            } else {
                                SupTemp -= FanCoolLoad / (CpAir * rhoair * VolFlowRate);
                            }
                        }
                        // Sensible capacity is calculated from enthalpy difference with constant humidity ratio, i.e.,
                        // there is only temperature difference between entering and leaving air enthalpy. Previously
                        // it was calculated using m.cp.dT
                        SensCapAtPeak = (rhoair * VolFlowRate * (MixEnth - SupEnth)) + FanCoolLoad;
                        SensCapAtPeak = max(0.0, SensCapAtPeak);
                        MixWetBulb = PsyTwbFnTdbWPb(MixTemp, MixHumRat, StdBaroPress, RoutineName);
                        ratioTDB = (MixTemp + 273.15) / 283.15;
                        ratioTWB = (MixWetBulb + 273.15) / 283.15;
                        // rated condenser water inlet temperature of 85F
                        ratioTS = (((85.0 - 32.0) / 1.8) + 273.15) / 283.15;
                        SensCapTempModFac = SensCapCoeff1 + (ratioTDB * SensCapCoeff2) + (ratioTWB * SensCapCoeff3) + (ratioTS * SensCapCoeff4) +
                                            (1.0 * SensCapCoeff5) + (1.0 * SensCapCoeff6);
                        if (SensCapTempModFac > 0.0) {
                            RatedCapCoolSensDes = SensCapAtPeak / SensCapTempModFac;
                        } else {
                            RatedCapCoolSensDes = SensCapAtPeak;
                        }
                    } else {
                        RatedCapCoolSensDes = 0.0;
                    }
                }
            }
            if (RatedCapCoolSensDes < SmallLoad) {
                RatedCapCoolSensDes = 0.0;
            }
            if (RatedCapCoolTotalAutoSized && RatedCapCoolSensAutoSized) {
                if (RatedCapCoolSensDes > RatedCapCoolTotalDes) {
                    RatedCapCoolTotalDes = RatedCapCoolSensDes;
                }
            }
            if (!HardSizeNoDesRun) {
                if (RatedCapCoolTotalAutoSized) {
                    state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).RatedCapCoolTotal = RatedCapCoolTotalDes;
                    BaseSizer::reportSizerOutput("COIL:" + state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WatertoAirHPType + ":WATERTOAIRHEATPUMP:EQUATIONFIT",
                                                 state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Name,
                                                 "Design Size Rated Total Cooling Capacity [W]",
                                                 RatedCapCoolTotalDes);
                    PreDefTableEntry(pdchCoolCoilTotCap, state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Name, state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).RatedCapCoolTotal);
                    PreDefTableEntry(pdchCoolCoilLatCap,
                                     state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Name,
                                     state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).RatedCapCoolTotal - state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).RatedCapCoolSens);
                    if (state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).RatedCapCoolTotal != 0.0) {
                        PreDefTableEntry(pdchCoolCoilSHR,
                                         state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Name,
                                         state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).RatedCapCoolSens / state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).RatedCapCoolTotal);
                        PreDefTableEntry(pdchCoolCoilNomEff,
                                         state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Name,
                                         state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).RatedPowerCool / state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).RatedCapCoolTotal);
                    } else {
                        PreDefTableEntry(pdchCoolCoilSHR, state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Name, 0.0);
                        PreDefTableEntry(pdchCoolCoilNomEff, state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Name, 0.0);
                    }
                } else { // Hardsized with sizing data
                    if (state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).RatedCapCoolTotal > 0.0 && RatedCapCoolTotalDes > 0.0 && !HardSizeNoDesRun) {
                        RatedCapCoolTotalUser = state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).RatedCapCoolTotal;
                        BaseSizer::reportSizerOutput("COIL:" + state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WatertoAirHPType + ":WATERTOAIRHEATPUMP:EQUATIONFIT",
                                                     state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Name,
                                                     "Design Size Rated Total Cooling Capacity [W]",
                                                     RatedCapCoolTotalDes,
                                                     "User-Specified Rated Total Cooling Capacity [W]",
                                                     RatedCapCoolTotalUser);
                        if (DisplayExtraWarnings) {
                            if ((std::abs(RatedCapCoolTotalDes - RatedCapCoolTotalUser) / RatedCapCoolTotalUser) > AutoVsHardSizingThreshold) {
                                ShowMessage("SizeHVACWaterToAir: Potential issue with equipment sizing for coil " +
                                            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WatertoAirHPType + ":WATERTOAIRHEATPUMP:EQUATIONFIT \"" +
                                            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Name + "\"");
                                ShowContinueError("User-Specified Rated Total Cooling Capacity of " + RoundSigDigits(RatedCapCoolTotalUser, 2) +
                                                  " [W]");
                                ShowContinueError("differs from Design Size Rated Total Cooling Capacity of " +
                                                  RoundSigDigits(RatedCapCoolTotalDes, 2) + " [W]");
                                ShowContinueError("This may, or may not, indicate mismatched component sizes.");
                                ShowContinueError("Verify that the value entered is intended and is consistent with other components.");
                            }
                        }
                    }
                }
            }
            coilSelectionReportObj->setCoilCoolingCapacity(state,
                                                           state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Name,
                                                           CompType,
                                                           state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).RatedCapCoolTotal,
                                                           RatedCapCoolTotalAutoSized,
                                                           CurSysNum,
                                                           CurZoneEqNum,
                                                           CurOASysNum,
                                                           FanCoolLoad,
                                                           TotCapTempModFac,
                                                           -999.0,
                                                           -999.0);
            if (!HardSizeNoDesRun) {
                if (RatedCapCoolSensAutoSized) {
                    state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).RatedCapCoolSens = RatedCapCoolSensDes;
                    BaseSizer::reportSizerOutput("COIL:" + state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WatertoAirHPType + ":WATERTOAIRHEATPUMP:EQUATIONFIT",
                                                 state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Name,
                                                 "Design Size Rated Sensible Cooling Capacity [W]",
                                                 RatedCapCoolSensDes);
                    PreDefTableEntry(pdchCoolCoilSensCap, state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Name, state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).RatedCapCoolSens);
                    PreDefTableEntry(pdchCoolCoilLatCap,
                                     state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Name,
                                     state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).RatedCapCoolTotal - state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).RatedCapCoolSens);
                    if (state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).RatedCapCoolTotal != 0.0) {
                        PreDefTableEntry(pdchCoolCoilSHR,
                                         state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Name,
                                         state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).RatedCapCoolSens / state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).RatedCapCoolTotal);
                    } else {
                        PreDefTableEntry(pdchCoolCoilSHR, state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Name, 0.0);
                    }
                } else {
                    if (state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).RatedCapCoolSens > 0.0 && RatedCapCoolSensDes > 0.0) {
                        RatedCapCoolSensUser = state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).RatedCapCoolSens;
                        BaseSizer::reportSizerOutput("COIL:" + state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WatertoAirHPType + ":WATERTOAIRHEATPUMP:EQUATIONFIT",
                                                     state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Name,
                                                     "Design Size Rated Sensible Cooling Capacity [W]",
                                                     RatedCapCoolSensDes,
                                                     "User-Specified Rated Sensible Cooling Capacity [W]",
                                                     RatedCapCoolSensUser);
                        if (DisplayExtraWarnings) {
                            if ((std::abs(RatedCapCoolSensDes - RatedCapCoolSensUser) / RatedCapCoolSensUser) > AutoVsHardSizingThreshold) {
                                ShowMessage("SizeHVACWaterToAir: Potential issue with equipment sizing for coil " +
                                            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WatertoAirHPType + ":WATERTOAIRHEATPUMP:EQUATIONFIT \"" +
                                            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Name + "\"");
                                ShowContinueError("User-Specified Rated Sensible Cooling Capacity of " + RoundSigDigits(RatedCapCoolSensUser, 2) +
                                                  " [W]");
                                ShowContinueError("differs from Design Size Rated Sensible Cooling Capacity of " +
                                                  RoundSigDigits(RatedCapCoolSensDes, 2) + " [W]");
                                ShowContinueError("This may, or may not, indicate mismatched component sizes.");
                                ShowContinueError("Verify that the value entered is intended and is consistent with other components.");
                            }
                        }
                    }
                }
            }
            // Set the global DX cooling coil capacity variable for use by other objects
            if (state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WatertoAirHPType == "COOLING") {
                DXCoolCap = state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).RatedCapCoolTotal;
            }
            // test autosized sensible and total cooling capacity for total > sensible
            if ((RatedCapCoolSensAutoSized && RatedCapCoolTotalAutoSized) || RatedCapCoolSensAutoSized) {
                if (state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).RatedCapCoolSens > state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).RatedCapCoolTotal) {
                    ShowWarningError("COIL:" + state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WatertoAirHPType + ":WATERTOAIRHEATPUMP:EQUATIONFIT \"" +
                                     state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Name + "\"");
                    ShowContinueError(RoutineName + ": Rated Sensible Cooling Capacity > Rated Total Cooling Capacity");
                    ShowContinueError("Each of these capacity inputs have been autosized.");
                    ShowContinueError("Rated Sensible Cooling Capacity = " + TrimSigDigits(state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).RatedCapCoolSens, 2) + " W");
                    ShowContinueError("Rated Total Cooling Capacity    = " + TrimSigDigits(state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).RatedCapCoolTotal, 2) + " W");
                    ShowContinueError("See eio file for further details.");
                    ShowContinueError("Check Total and Sensible Cooling Capacity Coefficients to ensure they are accurate.");
                    ShowContinueError("Check Zone and System Sizing objects to verify sizing inputs.");
                    ShowContinueError("Sizing statistics:");
                    ShowContinueError("Entering Air Dry-Bulb Temperature = " + TrimSigDigits(MixTemp, 3) + " C");
                    ShowContinueError("Entering Air Wet-Bulb Temperature = " + TrimSigDigits(MixWetBulb, 3) + " C");
                    ShowContinueError("Entering Condenser Water Temperature used = 24.4444 C");
                    ShowContinueError("Used design air and water flow rates (i.e., used 1 for ratioVL and ratioVS)");
                    ShowContinueError("ratioTDB = " + TrimSigDigits(((MixTemp + 283.15) / 273.15), 3));
                    ShowContinueError("ratioTWB = " + TrimSigDigits(((MixWetBulb + 283.15) / 273.15), 3));
                    ShowContinueError("ratioTS  = " + TrimSigDigits(((85.0 + 283.15) / 273.15), 3));
                    ShowContinueError("Sensible Cooling Capacity Modifier = " + TrimSigDigits(SensCapTempModFac, 5));
                    ShowContinueError("...Rated Sensible Cooling Capacity = Sensible Design Load / Sensible Cooling Capacity Modifier");
                    ShowContinueError("Total Cooling Capacity Modifier = " + TrimSigDigits(TotCapTempModFac, 5));
                    ShowContinueError("...Rated Total Cooling Capacity = Total Design Load / Total Cooling Capacity Modifier");
                    ShowContinueError("Carefully review the Load Side Total, Sensible, and Latent heat transfer rates");
                    ShowContinueError("... to ensure they meet the expected manufacturers performance specifications.");
                }
            } else if (RatedCapCoolTotalAutoSized) {
                if (state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).RatedCapCoolSens > state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).RatedCapCoolTotal) {
                    ShowWarningError("COIL:" + state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WatertoAirHPType + ":WATERTOAIRHEATPUMP:EQUATIONFIT \"" +
                                     state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Name + "\"");
                    ShowContinueError(RoutineName + ": Rated Sensible Cooling Capacity > Rated Total Cooling Capacity");
                    ShowContinueError("Only the rated total capacity input is autosized, consider autosizing both inputs.");
                    ShowContinueError("Rated Sensible Cooling Capacity = " + TrimSigDigits(state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).RatedCapCoolSens, 2) + " W");
                    ShowContinueError("Rated Total Cooling Capacity    = " + TrimSigDigits(state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).RatedCapCoolTotal, 2) + " W");
                    ShowContinueError("See eio file for further details.");
                    ShowContinueError("Check Total and Sensible Cooling Capacity Coefficients to ensure they are accurate.");
                    ShowContinueError("Check Zone and System Sizing objects to verify sizing inputs.");
                    ShowContinueError("Sizing statistics for Total Cooling Capacity:");
                    ShowContinueError("Entering Air Wet-Bulb Temperature = " + TrimSigDigits(MixWetBulb, 3) + " C");
                    ShowContinueError("Entering Condenser Water Temperature used = 24.4444 C");
                    ShowContinueError("Used design air and water flow rates (i.e., used 1 for ratioVL and ratioVS)");
                    ShowContinueError("ratioTWB = " + TrimSigDigits(((MixWetBulb + 283.15) / 273.15), 3));
                    ShowContinueError("ratioTS  = " + TrimSigDigits(((85.0 + 283.15) / 273.15), 3));
                    ShowContinueError("Sensible Cooling Capacity Modifier = " + TrimSigDigits(SensCapTempModFac, 5));
                    ShowContinueError("...Rated Sensible Cooling Capacity = Sensible Design Load / Sensible Cooling Capacity Modifier");
                    ShowContinueError("Carefully review the Load Side Total, Sensible, and Latent heat transfer rates");
                    ShowContinueError("... to ensure they meet the expected manufacturers performance specifications.");
                }
            }

        } // Cooling Coild

        if (state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WatertoAirHPType == "HEATING") {
            // size rated heating capacity
            IsAutoSize = false;
            if (state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).RatedCapHeat == AutoSize && state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WatertoAirHPType == "HEATING") {
                IsAutoSize = true;
            }
            if (SizingDesRunThisAirSys || SizingDesRunThisZone) HardSizeNoDesRun = false;
            //   simply set heating capacity equal to the cooling capacity
            if (state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WatertoAirHPType == "HEATING") {
                RatedCapHeatDes = DXCoolCap;
                if (RatedCapHeatDes == AutoSize) {
                    ShowWarningError("COIL:" + state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WatertoAirHPType + ":WATERTOAIRHEATPUMP:EQUATIONFIT \"" +
                                     state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Name + "\"");
                    ShowContinueError(RoutineName + ": Heating coil could not be autosized since cooling coil was not previously sized.");
                    ShowContinueError("... Cooling coil must be upstream of heating coil.");
                    ShowContinueError("... Manually sizing this heating coil will be required.");
                }
                if (RatedCapHeatDes < SmallLoad) {
                    RatedCapHeatDes = 0.0;
                }
            }
            if (IsAutoSize) {
                state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).RatedCapHeat = RatedCapHeatDes;
                BaseSizer::reportSizerOutput("COIL:" + state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WatertoAirHPType + ":WATERTOAIRHEATPUMP:EQUATIONFIT",
                                             state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Name,
                                             "Design Size Rated Heating Capacity [W]",
                                             RatedCapHeatDes);
                PreDefTableEntry(pdchHeatCoilNomCap, state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Name, state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).RatedCapHeat);
                if (state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).RatedCapHeat != 0.0) {
                    PreDefTableEntry(pdchHeatCoilNomEff,
                                     state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Name,
                                     state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).RatedPowerHeat / state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).RatedCapHeat);
                } else {
                    PreDefTableEntry(pdchHeatCoilNomEff, state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Name, 0.0);
                }
            } else {
                if (state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).RatedCapHeat > 0.0 && RatedCapHeatDes > 0.0 && !HardSizeNoDesRun) {
                    RatedCapHeatUser = state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).RatedCapHeat;
                    BaseSizer::reportSizerOutput("COIL:" + state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WatertoAirHPType + ":WATERTOAIRHEATPUMP:EQUATIONFIT",
                                                 state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Name,
                                                 "Design Size Rated Heating Capacity [W]",
                                                 RatedCapHeatDes,
                                                 "User-Specified Rated Heating Capacity [W]",
                                                 RatedCapHeatUser);
                    if (DisplayExtraWarnings) {
                        if ((std::abs(RatedCapHeatDes - RatedCapHeatUser) / RatedCapHeatUser) > AutoVsHardSizingThreshold) {
                            ShowMessage("SizeHVACWaterToAir: Potential issue with equipment sizing for coil " +
                                        state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WatertoAirHPType + ":WATERTOAIRHEATPUMP:EQUATIONFIT \"" +
                                        state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Name + "\"");
                            ShowContinueError("User-Specified Rated Heating Capacity of " + RoundSigDigits(RatedCapHeatUser, 2) + " [W]");
                            ShowContinueError("differs from Design Size Rated Heating Capacity of " + RoundSigDigits(RatedCapHeatDes, 2) + " [W]");
                            ShowContinueError("This may, or may not, indicate mismatched component sizes.");
                            ShowContinueError("Verify that the value entered is intended and is consistent with other components.");
                        }
                    }
                } else {
                    if (state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).RatedCapHeat > 0.0) {
                        RatedCapHeatUser = state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).RatedCapHeat;
                        BaseSizer::reportSizerOutput("COIL:" + state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WatertoAirHPType + ":WATERTOAIRHEATPUMP:EQUATIONFIT",
                                                     state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Name,
                                                     "User-Specified Rated Heating Capacity [W]",
                                                     RatedCapHeatUser);
                    }
                }
            }

            // Check that heat pump heating capacity is within 20% of cooling capacity. Check only for heating coil and report both.
            if (state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WatertoAirHPType == "HEATING" && state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).CompanionCoolingCoilNum > 0) {

                if (state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).CompanionCoolingCoilNum).RatedCapCoolTotal > 0.0) {

                    if (std::abs(state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).CompanionCoolingCoilNum).RatedCapCoolTotal -
                                 state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).RatedCapHeat) /
                            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).CompanionCoolingCoilNum).RatedCapCoolTotal >
                        0.2) {

                        ShowWarningError("COIL:" + state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WatertoAirHPType + ":WATERTOAIRHEATPUMP:EQUATIONFIT \"" +
                                         state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Name + "\"");
                        ShowContinueError(
                            "...used with COIL:" + state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).CompanionCoolingCoilNum).WatertoAirHPType +
                            ":WATERTOAIRHEATPUMP:EQUATIONFIT \"" + state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).CompanionCoolingCoilNum).Name + "\"");
                        ShowContinueError("...heating capacity is disproportionate (> 20% different) to total cooling capacity");
                        ShowContinueError("...heating capacity = " + TrimSigDigits(state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).RatedCapHeat, 3) + " W");
                        ShowContinueError("...cooling capacity = " +
                                          TrimSigDigits(state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).CompanionCoolingCoilNum).RatedCapCoolTotal, 3) +
                                          " W");
                    }
                }
            }

            coilSelectionReportObj->setCoilHeatingCapacity(state,
                                                           state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Name,
                                                           CompType,
                                                           state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).RatedCapHeat,
                                                           IsAutoSize,
                                                           CurSysNum,
                                                           CurZoneEqNum,
                                                           CurOASysNum,
                                                           FanCoolLoad,
                                                           TotCapTempModFac,
                                                           -999.0,
                                                           -999.0);
        } // Heating

        // size rated power
        if (state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WatertoAirHPType == "COOLING") {

            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).RatedPowerCool = state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).RatedCapCoolTotal / state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).RatedCOPCool;

        } else if (state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WatertoAirHPType == "HEATING") {

            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).RatedPowerHeat = state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).RatedCapHeat / state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).RatedCOPHeat;
        }

        // Size water volumetric flow rate
        IsAutoSize = false;
        if (state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).RatedWaterVolFlowRate == AutoSize) {
            IsAutoSize = true;
        }

        //   WSHP condenser can be on either a plant loop or condenser loop. Test each to find plant sizing number.
        //   first check to see if coil is connected to a plant loop, no warning on this CALL
        if (IsAutoSize) {
            PltSizNum = MyPlantSizingIndex("COIL:" + state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WatertoAirHPType + ":WATERTOAIRHEATPUMP:EQUATIONFIT",
                                           state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Name,
                                           state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WaterInletNodeNum,
                                           state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WaterOutletNodeNum,
                                           ErrorsFound,
                                           false);

            //!   if not found on a plant loop, check condenser loop and warn user if not found
            //    IF(PltSizNum == 0) THEN
            //      PltSizNum = &
            //          MyCondPlantSizingIndex('COIL:'//TRIM(SimpleWatertoAirHP(HPNum)%WaterToAirHPType)//':WATERTOAIRHEATPUMP:EQUATIONFIT', &
            //                                 SimpleWatertoAirHP(HPNum)%Name, &
            //                                 SimpleWatertoAirHP(HPNum)%WaterInletNodeNum, &
            //                                 SimpleWatertoAirHP(HPNum)%WaterOutletNodeNum, ErrorsFound)
            //    END IF

            if (PltSizNum > 0) {
                rho = GetDensityGlycol(state, PlantLoop(state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).LoopNum).FluidName,
                                       PlantSizData(PltSizNum).ExitTemp,
                                       PlantLoop(state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).LoopNum).FluidIndex,
                                       RoutineNameAlt);
                Cp = GetSpecificHeatGlycol(state, PlantLoop(state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).LoopNum).FluidName,
                                           PlantSizData(PltSizNum).ExitTemp,
                                           PlantLoop(state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).LoopNum).FluidIndex,
                                           RoutineNameAlt);

                if (state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WatertoAirHPType == "HEATING") {

                    RatedWaterVolFlowRateDes = state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).RatedCapHeat / (PlantSizData(PltSizNum).DeltaT * Cp * rho);
                } else if (state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WatertoAirHPType == "COOLING") {

                    //       use companion heating coil capacity to calculate volumetric flow rate
                    if (state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).CompanionCoolingCoilNum > 0) {
                        SystemCapacity = state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).CompanionCoolingCoilNum).RatedCapHeat;
                    } else {
                        SystemCapacity = state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).RatedCapCoolTotal;
                    }

                    RatedWaterVolFlowRateDes = SystemCapacity / (PlantSizData(PltSizNum).DeltaT * Cp * rho);
                }
            } else {
                ShowSevereError("Autosizing of water flow requires a loop Sizing:Plant object");
                ShowContinueError("Autosizing also requires physical connection to a plant or condenser loop.");
                ShowContinueError("Occurs in COIL:" + state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WatertoAirHPType +
                                  ":WATERTOAIRHEATPUMP:EQUATIONFIT Object=" + state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Name);
                ErrorsFound = true;
            }
        }
        if (IsAutoSize) {
            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).RatedWaterVolFlowRate = RatedWaterVolFlowRateDes;
            BaseSizer::reportSizerOutput("COIL:" + state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WatertoAirHPType + ":WATERTOAIRHEATPUMP:EQUATIONFIT",
                                         state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Name,
                                         "Design Size Rated Water Flow Rate [m3/s]",
                                         RatedWaterVolFlowRateDes);
        } else {
            if (state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).RatedWaterVolFlowRate > 0.0 && RatedWaterVolFlowRateDes > 0.0) {
                RatedWaterVolFlowRateUser = state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).RatedWaterVolFlowRate;
                BaseSizer::reportSizerOutput("COIL:" + state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WatertoAirHPType + ":WATERTOAIRHEATPUMP:EQUATIONFIT",
                                             state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Name,
                                             "Design Size Rated Water Flow Rate [m3/s]",
                                             RatedWaterVolFlowRateDes,
                                             "User-Specified Rated Water Flow Rate [m3/s]",
                                             RatedWaterVolFlowRateUser);
                if (DisplayExtraWarnings) {
                    if ((std::abs(RatedWaterVolFlowRateDes - RatedWaterVolFlowRateUser) / RatedWaterVolFlowRateUser) > AutoVsHardSizingThreshold) {
                        ShowMessage("SizeHVACWaterToAir: Potential issue with equipment sizing for coil " +
                                    state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WatertoAirHPType + ":WATERTOAIRHEATPUMP:EQUATIONFIT \"" +
                                    state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Name + "\"");
                        ShowContinueError("User-Specified Rated Water Flow Rate of " + RoundSigDigits(RatedWaterVolFlowRateUser, 5) + " [m3/s]");
                        ShowContinueError("differs from Design Size Rated Water Flow Rate of " + RoundSigDigits(RatedWaterVolFlowRateDes, 5) +
                                          " [m3/s]");
                        ShowContinueError("This may, or may not, indicate mismatched component sizes.");
                        ShowContinueError("Verify that the value entered is intended and is consistent with other components.");
                    }
                }
            }
        }

        // Save component design water volumetric flow rate.
        // Use 1/2 flow since both cooling and heating coil will save flow yet only 1 will operate at a time
        if (state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).RatedWaterVolFlowRate > 0.0) {
            RegisterPlantCompDesignFlow(state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WaterInletNodeNum, 0.5 * state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).RatedWaterVolFlowRate);
        }
    }

    void CalcHPCoolingSimple(EnergyPlusData &state, int const HPNum,                          // Heat Pump Number
                             int const CyclingScheme,                  // Fan/Compressor cycling scheme indicator
                             Real64 const RuntimeFrac,                 // Runtime Fraction of compressor or percent on time (on-time/cycle time)
                             Real64 const EP_UNUSED(SensDemand),       // Cooling Sensible Demand [W] !unused1208
                             Real64 const EP_UNUSED(LatentDemand),     // Cooling Latent Demand [W]
                             int const CompOp,                         // compressor operation flag
                             Real64 const PartLoadRatio,               // compressor part load ratio
                             Real64 const EP_UNUSED(OnOffAirFlowRatio) // ratio of compressor on flow to average flow over time step
    )
    {

        //       AUTHOR         Arun Shenoy
        //       DATE WRITTEN   Jan 2004
        //       MODIFIED       na
        //       RE-ENGINEERED  Kenneth Tang (Jan 2005)

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine is for simulating the cooling mode of the Water to Air HP Simple

        // METHODOLOGY EMPLOYED:
        // Simulate the heat pump performance using the coefficients and rated conditions
        // If the LatDegradModelSimFlag is enabled, the coil will be simulated twice:
        // (1)first simulation at the rated conditions (2) second simulation at the
        // actual operating conditions. Then call CalcEffectiveSHR and the effective SHR
        // is adjusted.
        // If the LatDegradModelSimFlag is disabled, the cooling coil is only simulated
        // once at the actual operating conditions.
        // Finally, adjust the heat pump outlet conditions based on the PartLoadRatio
        // and RuntimeFrac.

        // REFERENCES:
        // (1) Lash.T.A.,1992.Simulation and Analysis of a Water Loop Heat Pump System.
        // M.S. Thesis, University of Illinois at Urbana Champaign.
        // (2) Shenoy, Arun. 2004. Simulation, Modeling and Analysis of Water to Air Heat Pump.
        // State Energy Simulation Program. M.S. Thesis, Department of Mechanical and Aerospace Engineering,
        // Oklahoma State University. (downloadable from www.hvac.okstate.edu)
        // (3) Tang,C.C.. 2005. Modeling Packaged Heat Pumps in a Quasi-Steady
        // State Energy Simulation Program. M.S. Thesis, Department of Mechanical and Aerospace Engineering,
        // Oklahoma State University. (downloadable from www.hvac.okstate.edu)
        // (4) Henderson, H.I., K. Rengarajan.1996. A Model to Predict the Latent
        // Capacity of Air Conditioners and Heat Pumps at Part-Load Conditions
        // with Constant Fan Operation ASHRAE Transactions 102 (1), pp. 266-274.

        // Using/Aliasing
        using DataHVACGlobals::DXElecCoolingPower;
        using DataHVACGlobals::TimeStepSys;
        using DataPlant::PlantLoop;
        using FluidProperties::GetSpecificHeatGlycol;
        using PlantUtilities::SetComponentFlowRate;
        using Psychrometrics::PsyCpAirFnW;
        using Psychrometrics::PsyHFnTdbW;
        using Psychrometrics::PsyRhoAirFnPbTdbW;
        using Psychrometrics::PsyTdbFnHW;
        using Psychrometrics::PsyTwbFnTdbWPb;
        using Psychrometrics::PsyWFnTdbH;
        using Psychrometrics::PsyWFnTdbTwbPb;

        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:

        // SUBROUTINE PARAMETER DEFINITIONS:
        Real64 const Tref(283.15); // Reference Temperature for performance curves,10C [K]
        static std::string const RoutineName("CalcHPCoolingSimple");
        static std::string const RoutineNameSourceSideInletTemp("CalcHPCoolingSimple:SourceSideInletTemp");

        // INTERFACE BLOCK SPECIFICATIONS
        // na

        // DERIVED TYPE DEFINITIONS
        // na

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:

        Real64 TotalCapRated;         // Rated Total Cooling Capacity [W]
        Real64 SensCapRated;          // Rated Sensible Cooling Capacity [W]
        Real64 CoolPowerRated;        // Rated Cooling Power Input[W]
        Real64 AirVolFlowRateRated;   // Rated Air Volumetric Flow Rate [m3/s]
        Real64 WaterVolFlowRateRated; // Rated Water Volumetric Flow Rate [m3/s]
        Real64 TotalCapCoeff1;        // 1st coefficient of the total cooling capacity performance curve
        Real64 TotalCapCoeff2;        // 2nd coefficient of the total cooling capacity performance curve
        Real64 TotalCapCoeff3;        // 3rd coefficient of the total cooling capacity performance curve
        Real64 TotalCapCoeff4;        // 4th coefficient of the total cooling capacity performance curve
        Real64 TotalCapCoeff5;        // 5th coefficient of the total cooling capacity performance curve
        Real64 SensCapCoeff1;         // 1st coefficient of the sensible cooling capacity performance curve
        Real64 SensCapCoeff2;         // 2nd coefficient of the sensible cooling capacity performance curve
        Real64 SensCapCoeff3;         // 3rd coefficient of the sensible cooling capacity performance curve
        Real64 SensCapCoeff4;         // 4th coefficient of the sensible cooling capacity performance curve
        Real64 SensCapCoeff5;         // 5th coefficient of the sensible cooling capacity performance curve
        Real64 SensCapCoeff6;         // 6th coefficient of the sensible cooling capacity performance curve
        Real64 CoolPowerCoeff1;       // 1st coefficient of the cooling power consumption curve
        Real64 CoolPowerCoeff2;       // 2nd coefficient of the cooling power consumption curve
        Real64 CoolPowerCoeff3;       // 3rd coefficient of the cooling power consumption curve
        Real64 CoolPowerCoeff4;       // 4th coefficient of the cooling power consumption curve
        Real64 CoolPowerCoeff5;       // 5th coefficient of the cooling power consumption curve
        Real64 Twet_Rated;            // Twet at rated conditions (coil air flow rate and air temperatures), sec
        Real64 Gamma_Rated;           // Gamma at rated conditions (coil air flow rate and air temperatures)

        Real64 SHRss;  // Sensible heat ratio at steady state
        Real64 SHReff; // Effective sensible heat ratio at part-load condition
        //  REAL(r64) :: PartLoadRatio          ! Part load ratio

        Real64 ratioTDB; // Ratio of the inlet air dry bulb temperature to the rated conditions
        Real64 ratioTWB; // Ratio of the inlet air wet bulb temperature to the rated conditions
        Real64 ratioTS;  // Ratio of the source side(water) inlet temperature to the rated conditions
        Real64 ratioVL;  // Ratio of the air flow rate to the rated conditions
        Real64 ratioVS;  // Ratio of the water flow rate to the rated conditions
        Real64 CpWater;  // Specific heat of water [J/kg_C]
        Real64 CpAir;    // Specific heat of air [J/kg_C]
        Real64 ReportingConstant;

        bool LatDegradModelSimFlag;             // Latent degradation model simulation flag
        int NumIteration;                       // Iteration Counter
        static int Count(0);                    // No idea what this is for.
        static Real64 LoadSideInletDBTemp_Init; // rated conditions
        static Real64 LoadSideInletWBTemp_Init; // rated conditions
        static Real64 LoadSideInletHumRat_Init; // rated conditions
        static Real64 LoadSideInletEnth_Init;   // rated conditions
        static Real64 CpAir_Init;               // rated conditions
        Real64 LoadSideInletDBTemp_Unit;        // calc conditions for unit
        Real64 LoadSideInletWBTemp_Unit;        // calc conditions for unit
        Real64 LoadSideInletHumRat_Unit;        // calc conditions for unit
        Real64 LoadSideInletEnth_Unit;          // calc conditions for unit
        Real64 CpAir_Unit;                      // calc conditions for unit

        if (state.dataWaterToAirHeatPumpSimple->firstTime) {
            // Set indoor air conditions to the rated condition
            LoadSideInletDBTemp_Init = 26.7;
            LoadSideInletHumRat_Init = 0.0111;
            LoadSideInletEnth_Init = PsyHFnTdbW(LoadSideInletDBTemp_Init, LoadSideInletHumRat_Init);
            CpAir_Init = PsyCpAirFnW(LoadSideInletHumRat_Init);
            state.dataWaterToAirHeatPumpSimple->firstTime = false;
        }
        LoadSideInletWBTemp_Init = PsyTwbFnTdbWPb(LoadSideInletDBTemp_Init, LoadSideInletHumRat_Init, OutBaroPress, RoutineName);

        //  LOAD LOCAL VARIABLES FROM DATA STRUCTURE (for code readability)

        TotalCapRated = state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).RatedCapCoolTotal;
        SensCapRated = state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).RatedCapCoolSens;
        CoolPowerRated = state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).RatedPowerCool;
        AirVolFlowRateRated = state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).RatedAirVolFlowRate;
        WaterVolFlowRateRated = state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).RatedWaterVolFlowRate;

        TotalCapCoeff1 = state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).TotalCoolCap1;
        TotalCapCoeff2 = state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).TotalCoolCap2;
        TotalCapCoeff3 = state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).TotalCoolCap3;
        TotalCapCoeff4 = state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).TotalCoolCap4;
        TotalCapCoeff5 = state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).TotalCoolCap5;
        SensCapCoeff1 = state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).SensCoolCap1;
        SensCapCoeff2 = state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).SensCoolCap2;
        SensCapCoeff3 = state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).SensCoolCap3;
        SensCapCoeff4 = state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).SensCoolCap4;
        SensCapCoeff5 = state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).SensCoolCap5;
        SensCapCoeff6 = state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).SensCoolCap6;
        CoolPowerCoeff1 = state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).CoolPower1;
        CoolPowerCoeff2 = state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).CoolPower2;
        CoolPowerCoeff3 = state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).CoolPower3;
        CoolPowerCoeff4 = state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).CoolPower4;
        CoolPowerCoeff5 = state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).CoolPower5;
        Twet_Rated = state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Twet_Rated;
        Gamma_Rated = state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Gamma_Rated;

        state.dataWaterToAirHeatPumpSimple->LoadSideMassFlowRate = state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).AirMassFlowRate;
        state.dataWaterToAirHeatPumpSimple->SourceSideMassFlowRate = state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WaterMassFlowRate;
        state.dataWaterToAirHeatPumpSimple->SourceSideInletTemp = state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).InletWaterTemp;
        state.dataWaterToAirHeatPumpSimple->SourceSideInletEnth = state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).InletWaterEnthalpy;
        CpWater = GetSpecificHeatGlycol(state, PlantLoop(state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).LoopNum).FluidName,
                                        state.dataWaterToAirHeatPumpSimple->SourceSideInletTemp,
                                        PlantLoop(state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).LoopNum).FluidIndex,
                                        RoutineNameSourceSideInletTemp);

        // Check for flows, do not perform simulation if no flow in load side or source side.
        if (state.dataWaterToAirHeatPumpSimple->SourceSideMassFlowRate <= 0.0 || state.dataWaterToAirHeatPumpSimple->LoadSideMassFlowRate <= 0.0) {
            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).SimFlag = false;
            return;
        } else {
            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).SimFlag = true;
        }

        if (CompOp == 0) {
            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).SimFlag = false;
            return;
        }

        // Loop the calculation at least once depending whether the latent degradation model
        // is enabled. 1st iteration to calculate the QLatent(rated) at (TDB,TWB)indoorair=(26.7C,19.4C)
        // and 2nd iteration to calculate the  QLatent(actual)
        if ((RuntimeFrac >= 1.0) || (Twet_Rated <= 0.0) || (Gamma_Rated <= 0.0)) {
            LatDegradModelSimFlag = false;
            // Set NumIteration=1 so that latent model would quit after 1 simulation with the actual condition
            NumIteration = 1;
        } else {
            LatDegradModelSimFlag = true;
            // Set NumIteration=0 so that latent model would simulate twice with rated and actual condition
            NumIteration = 0;
        }

        // Set indoor air conditions to the actual condition
        LoadSideInletDBTemp_Unit = state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).InletAirDBTemp;
        LoadSideInletHumRat_Unit = state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).InletAirHumRat;
        LoadSideInletWBTemp_Unit = PsyTwbFnTdbWPb(LoadSideInletDBTemp_Unit, LoadSideInletHumRat_Unit, OutBaroPress, RoutineName);
        LoadSideInletEnth_Unit = state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).InletAirEnthalpy;
        CpAir_Unit = PsyCpAirFnW(LoadSideInletHumRat_Unit);

        while (true) {
            ++NumIteration;
            if (NumIteration == 1) {
                // Set indoor air conditions to the rated conditions
                state.dataWaterToAirHeatPumpSimple->LoadSideInletDBTemp = LoadSideInletDBTemp_Init;
                state.dataWaterToAirHeatPumpSimple->LoadSideInletHumRat = LoadSideInletHumRat_Init;
                state.dataWaterToAirHeatPumpSimple->LoadSideInletWBTemp = LoadSideInletWBTemp_Init;
                state.dataWaterToAirHeatPumpSimple->LoadSideInletEnth = LoadSideInletEnth_Init;
                CpAir = CpAir_Init;
            } else {
                // Set indoor air conditions to the actual condition
                state.dataWaterToAirHeatPumpSimple->LoadSideInletDBTemp = LoadSideInletDBTemp_Unit;
                state.dataWaterToAirHeatPumpSimple->LoadSideInletHumRat = LoadSideInletHumRat_Unit;
                state.dataWaterToAirHeatPumpSimple->LoadSideInletWBTemp = LoadSideInletWBTemp_Unit;
                state.dataWaterToAirHeatPumpSimple->LoadSideInletEnth = LoadSideInletEnth_Unit;
                CpAir = CpAir_Unit;
            }

            ratioTDB = ((state.dataWaterToAirHeatPumpSimple->LoadSideInletDBTemp + state.dataWaterToAirHeatPumpSimple->CelsiustoKelvin) / Tref);
            ratioTWB = ((state.dataWaterToAirHeatPumpSimple->LoadSideInletWBTemp + state.dataWaterToAirHeatPumpSimple->CelsiustoKelvin) / Tref);
            ratioTS = ((state.dataWaterToAirHeatPumpSimple->SourceSideInletTemp + state.dataWaterToAirHeatPumpSimple->CelsiustoKelvin) / Tref);
            ratioVL = (state.dataWaterToAirHeatPumpSimple->LoadSideMassFlowRate /
                       (AirVolFlowRateRated * PsyRhoAirFnPbTdbW(StdBaroPress, state.dataWaterToAirHeatPumpSimple->LoadSideInletDBTemp, state.dataWaterToAirHeatPumpSimple->LoadSideInletHumRat, RoutineName)));

            if (state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).DesignWaterMassFlowRate > 0.0) {
                ratioVS = (state.dataWaterToAirHeatPumpSimple->SourceSideMassFlowRate) / (state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).DesignWaterMassFlowRate);
            } else {
                ratioVS = 0.0;
            }

            state.dataWaterToAirHeatPumpSimple->QLoadTotal = TotalCapRated * (TotalCapCoeff1 + (ratioTWB * TotalCapCoeff2) + (ratioTS * TotalCapCoeff3) + (ratioVL * TotalCapCoeff4) +
                                          (ratioVS * TotalCapCoeff5));
            state.dataWaterToAirHeatPumpSimple->QSensible = SensCapRated * (SensCapCoeff1 + (ratioTDB * SensCapCoeff2) + (ratioTWB * SensCapCoeff3) + (ratioTS * SensCapCoeff4) +
                                        (ratioVL * SensCapCoeff5) + (ratioVS * SensCapCoeff6));
            state.dataWaterToAirHeatPumpSimple->Winput = CoolPowerRated * (CoolPowerCoeff1 + (ratioTWB * CoolPowerCoeff2) + (ratioTS * CoolPowerCoeff3) + (ratioVL * CoolPowerCoeff4) +
                                       (ratioVS * CoolPowerCoeff5));

            // Check if the Sensible Load is greater than the Total Cooling Load
            if (state.dataWaterToAirHeatPumpSimple->QSensible > state.dataWaterToAirHeatPumpSimple->QLoadTotal) {
                state.dataWaterToAirHeatPumpSimple->QSensible = state.dataWaterToAirHeatPumpSimple->QLoadTotal;
            }

            if (LatDegradModelSimFlag) {
                // Calculate for SHReff using the Latent Degradation Model
                if (NumIteration == 1) {
                    state.dataWaterToAirHeatPumpSimple->QLatRated = state.dataWaterToAirHeatPumpSimple->QLoadTotal - state.dataWaterToAirHeatPumpSimple->QSensible;
                } else if (NumIteration == 2) {
                    state.dataWaterToAirHeatPumpSimple->QLatActual = state.dataWaterToAirHeatPumpSimple->QLoadTotal - state.dataWaterToAirHeatPumpSimple->QSensible;
                    SHRss = state.dataWaterToAirHeatPumpSimple->QSensible / state.dataWaterToAirHeatPumpSimple->QLoadTotal;
                    SHReff =
                        CalcEffectiveSHR(state, HPNum, SHRss, CyclingScheme, RuntimeFrac, state.dataWaterToAirHeatPumpSimple->QLatRated, state.dataWaterToAirHeatPumpSimple->QLatActual, state.dataWaterToAirHeatPumpSimple->LoadSideInletDBTemp, state.dataWaterToAirHeatPumpSimple->LoadSideInletWBTemp);
                    //       Update sensible capacity based on effective SHR
                    state.dataWaterToAirHeatPumpSimple->QSensible = state.dataWaterToAirHeatPumpSimple->QLoadTotal * SHReff;
                    break;
                }
            } else {
                // Assume SHReff=SHRss
                SHReff = state.dataWaterToAirHeatPumpSimple->QSensible / state.dataWaterToAirHeatPumpSimple->QLoadTotal;
                break;
            }
        }

        // calculate coil outlet state variables
        state.dataWaterToAirHeatPumpSimple->LoadSideOutletEnth = state.dataWaterToAirHeatPumpSimple->LoadSideInletEnth - state.dataWaterToAirHeatPumpSimple->QLoadTotal / state.dataWaterToAirHeatPumpSimple->LoadSideMassFlowRate;
        state.dataWaterToAirHeatPumpSimple->LoadSideOutletDBTemp = state.dataWaterToAirHeatPumpSimple->LoadSideInletDBTemp - state.dataWaterToAirHeatPumpSimple->QSensible / (state.dataWaterToAirHeatPumpSimple->LoadSideMassFlowRate * CpAir);
        state.dataWaterToAirHeatPumpSimple->LoadSideOutletHumRat = PsyWFnTdbH(state.dataWaterToAirHeatPumpSimple->LoadSideOutletDBTemp, state.dataWaterToAirHeatPumpSimple->LoadSideOutletEnth, RoutineName);
        ++Count;
        // Actual outlet conditions are "average" for time step
        if (CyclingScheme == ContFanCycCoil) {
            // continuous fan, cycling compressor
            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).OutletAirEnthalpy = PartLoadRatio * state.dataWaterToAirHeatPumpSimple->LoadSideOutletEnth + (1.0 - PartLoadRatio) * state.dataWaterToAirHeatPumpSimple->LoadSideInletEnth;
            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).OutletAirHumRat = PartLoadRatio * state.dataWaterToAirHeatPumpSimple->LoadSideOutletHumRat + (1.0 - PartLoadRatio) * state.dataWaterToAirHeatPumpSimple->LoadSideInletHumRat;
            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).OutletAirDBTemp =
                PsyTdbFnHW(state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).OutletAirEnthalpy, state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).OutletAirHumRat);
            state.dataWaterToAirHeatPumpSimple->PLRCorrLoadSideMdot = state.dataWaterToAirHeatPumpSimple->LoadSideMassFlowRate;
        } else {
            // default to cycling fan, cycling compressor
            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).OutletAirEnthalpy = state.dataWaterToAirHeatPumpSimple->LoadSideOutletEnth;
            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).OutletAirHumRat = state.dataWaterToAirHeatPumpSimple->LoadSideOutletHumRat;
            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).OutletAirDBTemp = state.dataWaterToAirHeatPumpSimple->LoadSideOutletDBTemp;
            state.dataWaterToAirHeatPumpSimple->PLRCorrLoadSideMdot = state.dataWaterToAirHeatPumpSimple->LoadSideMassFlowRate * PartLoadRatio;
        }

        // scale heat transfer rates to PLR and power to RTF
        state.dataWaterToAirHeatPumpSimple->QLoadTotal *= PartLoadRatio;
        state.dataWaterToAirHeatPumpSimple->QSensible *= PartLoadRatio;
        state.dataWaterToAirHeatPumpSimple->Winput *= RuntimeFrac;
        state.dataWaterToAirHeatPumpSimple->QSource = state.dataWaterToAirHeatPumpSimple->QLoadTotal + state.dataWaterToAirHeatPumpSimple->Winput;
        DataHeatBalance::HeatReclaimSimple_WAHPCoil(HPNum).AvailCapacity = state.dataWaterToAirHeatPumpSimple->QSource;

        //  Add power to global variable so power can be summed by parent object
        DXElecCoolingPower = state.dataWaterToAirHeatPumpSimple->Winput;

        ReportingConstant = TimeStepSys * SecInHour;
        DataHeatBalance::HeatReclaimDataBase &HeatReclaim = HeatReclaimSimple_WAHPCoil(HPNum);
        HeatReclaim.WaterHeatingDesuperheaterReclaimedHeatTotal = 0.0;
        if (allocated(HeatReclaim.WaterHeatingDesuperheaterReclaimedHeat)) {
            for (auto &num : HeatReclaim.WaterHeatingDesuperheaterReclaimedHeat)
                HeatReclaim.WaterHeatingDesuperheaterReclaimedHeatTotal += num;
        }
        state.dataWaterToAirHeatPumpSimple->QSource -= HeatReclaim.WaterHeatingDesuperheaterReclaimedHeatTotal;

        // Update heat pump data structure
        state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Power = state.dataWaterToAirHeatPumpSimple->Winput;
        state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).QLoadTotal = state.dataWaterToAirHeatPumpSimple->QLoadTotal;
        state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).QSensible = state.dataWaterToAirHeatPumpSimple->QSensible;
        state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).QLatent = state.dataWaterToAirHeatPumpSimple->QLoadTotal - state.dataWaterToAirHeatPumpSimple->QSensible;
        state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).QSource = state.dataWaterToAirHeatPumpSimple->QSource;
        state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Energy = state.dataWaterToAirHeatPumpSimple->Winput * ReportingConstant;
        state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).EnergyLoadTotal = state.dataWaterToAirHeatPumpSimple->QLoadTotal * ReportingConstant;
        state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).EnergySensible = state.dataWaterToAirHeatPumpSimple->QSensible * ReportingConstant;
        state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).EnergyLatent = (state.dataWaterToAirHeatPumpSimple->QLoadTotal - state.dataWaterToAirHeatPumpSimple->QSensible) * ReportingConstant;
        state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).EnergySource = state.dataWaterToAirHeatPumpSimple->QSource * ReportingConstant;
        if (RuntimeFrac == 0.0) {
            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).COP = 0.0;
        } else {
            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).COP = state.dataWaterToAirHeatPumpSimple->QLoadTotal / state.dataWaterToAirHeatPumpSimple->Winput;
        }
        state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).RunFrac = RuntimeFrac;
        state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).PartLoadRatio = PartLoadRatio;
        state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).AirMassFlowRate = state.dataWaterToAirHeatPumpSimple->PLRCorrLoadSideMdot;

        if ((state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WaterCyclingMode) == WaterCycling) {
            // plant can lock flow at coil water inlet node, use design flow multiplied by PLR to calculate water mass flow rate
            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WaterMassFlowRate = state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).DesignWaterMassFlowRate * PartLoadRatio;
            SetComponentFlowRate(state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WaterMassFlowRate,
                                 state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WaterInletNodeNum,
                                 state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WaterOutletNodeNum,
                                 state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).LoopNum,
                                 state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).LoopSide,
                                 state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).BranchNum,
                                 state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).CompNum);
            if (state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WaterMassFlowRate > 0.0) {
                state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).OutletWaterTemp = state.dataWaterToAirHeatPumpSimple->SourceSideInletTemp + state.dataWaterToAirHeatPumpSimple->QSource / (state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WaterMassFlowRate * CpWater);
                state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).OutletWaterEnthalpy = state.dataWaterToAirHeatPumpSimple->SourceSideInletEnth + state.dataWaterToAirHeatPumpSimple->QSource / state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WaterMassFlowRate;
            }
        } else {
            if ((state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WaterCyclingMode) == WaterConstant) {
                if (state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WaterFlowMode) {
                    state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WaterMassFlowRate = state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).DesignWaterMassFlowRate;
                    SetComponentFlowRate(state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WaterMassFlowRate,
                                         state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WaterInletNodeNum,
                                         state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WaterOutletNodeNum,
                                         state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).LoopNum,
                                         state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).LoopSide,
                                         state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).BranchNum,
                                         state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).CompNum);
                } else {
                    state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WaterMassFlowRate = state.dataWaterToAirHeatPumpSimple->SourceSideMassFlowRate;
                }
            } else {
                state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WaterMassFlowRate = state.dataWaterToAirHeatPumpSimple->SourceSideMassFlowRate;
            }
            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).OutletWaterTemp = state.dataWaterToAirHeatPumpSimple->SourceSideInletTemp + state.dataWaterToAirHeatPumpSimple->QSource / (state.dataWaterToAirHeatPumpSimple->SourceSideMassFlowRate * CpWater);
            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).OutletWaterEnthalpy = state.dataWaterToAirHeatPumpSimple->SourceSideInletEnth + state.dataWaterToAirHeatPumpSimple->QSource / state.dataWaterToAirHeatPumpSimple->SourceSideMassFlowRate;
        }
    }

    void CalcHPHeatingSimple(EnergyPlusData &state, int const HPNum,                          // Heat Pump Number
                             int const CyclingScheme,                  // Fan/Compressor cycling scheme indicator
                             Real64 const RuntimeFrac,                 // Runtime Fraction of compressor
                             Real64 const EP_UNUSED(SensDemand),       // Cooling Sensible Demand [W] !unused1208
                             int const CompOp,                         // compressor operation flag
                             Real64 const PartLoadRatio,               // compressor part load ratio
                             Real64 const EP_UNUSED(OnOffAirFlowRatio) // ratio of compressor on flow to average flow over time step
    )
    {

        //       AUTHOR         Arun Shenoy
        //       DATE WRITTEN   Jan 2004
        //       MODIFIED       na
        //       RE-ENGINEERED  Kenneth Tang (Jan 2005)

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine is for simulating the heating mode of the Water to Air HP Simple

        // METHODOLOGY EMPLOYED:
        // Simulate the heat pump performance using the coefficients and rated conditions
        // Finally, adjust the heat pump outlet conditions based on the PartLoadRatio
        // and RuntimeFrac.

        // REFERENCES:
        // (1) Lash.T.A.,1992.Simulation and Analysis of a Water Loop Heat Pump System.
        // M.S. Thesis, University of Illinois at Urbana Champaign.
        // (2) Shenoy, Arun. 2004. Simulation, Modeling and Analysis of Water to Air Heat Pump.
        // State Energy Simulation Program. M.S. Thesis, Department of Mechanical and Aerospace Engineering,
        // Oklahoma State University. (downloadable from www.hvac.okstate.edu)
        // (3) Tang,C.C.. 2005. Modeling Packaged Heat Pumps in a Quasi-Steady
        // State Energy Simulation Program. M.S. Thesis, Department of Mechanical and Aerospace Engineering,
        // Oklahoma State University. (downloadable from www.hvac.okstate.edu)

        // Using/Aliasing
        using DataHVACGlobals::DXElecHeatingPower;
        using DataHVACGlobals::TimeStepSys;
        using DataPlant::PlantLoop;
        using FluidProperties::GetSpecificHeatGlycol;
        using PlantUtilities::SetComponentFlowRate;
        using Psychrometrics::PsyCpAirFnW;
        using Psychrometrics::PsyRhoAirFnPbTdbW;
        using Psychrometrics::PsyTdbFnHW;
        using Psychrometrics::PsyTwbFnTdbWPb;
        using Psychrometrics::PsyWFnTdbH;
        using Psychrometrics::PsyWFnTdbTwbPb;

        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:

        // SUBROUTINE PARAMETER DEFINITIONS:
        Real64 const Tref(283.15); // Reference Temperature for performance curves,10C [K]
        static std::string const RoutineName("CalcHPHeatingSimple");
        static std::string const RoutineNameSourceSideInletTemp("CalcHPHeatingSimple:SourceSideInletTemp");

        // INTERFACE BLOCK SPECIFICATIONS
        // na

        // DERIVED TYPE DEFINITIONS
        // na

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:

        Real64 HeatCapRated;          // Rated Heating Capacity [W]
        Real64 HeatPowerRated;        // Rated Heating Power Input[W]
        Real64 AirVolFlowRateRated;   // Rated Air Volumetric Flow Rate [m3/s]
        Real64 WaterVolFlowRateRated; // Rated Water Volumetric Flow Rate [m3/s]
        Real64 HeatCapCoeff1;         // 1st coefficient of the heating capacity performance curve
        Real64 HeatCapCoeff2;         // 2nd coefficient of the heating capacity performance curve
        Real64 HeatCapCoeff3;         // 3rd coefficient of the heating capacity performance curve
        Real64 HeatCapCoeff4;         // 4th coefficient of the heating capacity performance curve
        Real64 HeatCapCoeff5;         // 5th coefficient of the heating capacity performance curve
        Real64 HeatPowerCoeff1;       // 1st coefficient of the heating power consumption curve
        Real64 HeatPowerCoeff2;       // 2nd coefficient of the heating power consumption curve
        Real64 HeatPowerCoeff3;       // 3rd coefficient of the heating power consumption curve
        Real64 HeatPowerCoeff4;       // 4th coefficient of the heating power consumption curve
        Real64 HeatPowerCoeff5;       // 5th coefficient of the heating power consumption curve

        //  REAL(r64) :: PartLoadRatio          ! Part load ratio
        Real64 ratioTDB; // Ratio of the inlet air dry bulb temperature to the rated conditions
        Real64 ratioTS;  // Ratio of the source side (water) inlet temperature to the rated conditions
        Real64 ratioVL;  // Ratio of the load side flow rate to the rated conditions
        Real64 ratioVS;  // Ratio of the source side flow rate to the rated conditions
        Real64 CpWater;  // Specific heat of water [J/kg_C]
        Real64 CpAir;    // Specific heat of air [J/kg_C]
        Real64 ReportingConstant;

        //  LOAD LOCAL VARIABLES FROM DATA STRUCTURE (for code readability)

        HeatCapRated = state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).RatedCapHeat;
        HeatPowerRated = state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).RatedPowerHeat;
        AirVolFlowRateRated = state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).RatedAirVolFlowRate;
        WaterVolFlowRateRated = state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).RatedWaterVolFlowRate;
        HeatCapCoeff1 = state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).HeatCap1;
        HeatCapCoeff2 = state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).HeatCap2;
        HeatCapCoeff3 = state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).HeatCap3;
        HeatCapCoeff4 = state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).HeatCap4;
        HeatCapCoeff5 = state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).HeatCap5;
        HeatPowerCoeff1 = state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).HeatPower1;
        HeatPowerCoeff2 = state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).HeatPower2;
        HeatPowerCoeff3 = state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).HeatPower3;
        HeatPowerCoeff4 = state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).HeatPower4;
        HeatPowerCoeff5 = state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).HeatPower5;

        state.dataWaterToAirHeatPumpSimple->LoadSideMassFlowRate = state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).AirMassFlowRate;
        state.dataWaterToAirHeatPumpSimple->LoadSideInletDBTemp = state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).InletAirDBTemp;
        state.dataWaterToAirHeatPumpSimple->LoadSideInletHumRat = state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).InletAirHumRat;

        state.dataWaterToAirHeatPumpSimple->LoadSideInletWBTemp = PsyTwbFnTdbWPb(state.dataWaterToAirHeatPumpSimple->LoadSideInletDBTemp, state.dataWaterToAirHeatPumpSimple->LoadSideInletHumRat, OutBaroPress, RoutineName);
        state.dataWaterToAirHeatPumpSimple->LoadSideInletEnth = state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).InletAirEnthalpy;
        CpAir = PsyCpAirFnW(state.dataWaterToAirHeatPumpSimple->LoadSideInletHumRat);
        state.dataWaterToAirHeatPumpSimple->SourceSideMassFlowRate = state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WaterMassFlowRate;
        state.dataWaterToAirHeatPumpSimple->SourceSideInletTemp = state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).InletWaterTemp;
        state.dataWaterToAirHeatPumpSimple->SourceSideInletEnth = state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).InletWaterEnthalpy;
        CpWater = GetSpecificHeatGlycol(state, PlantLoop(state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).LoopNum).FluidName,
            state.dataWaterToAirHeatPumpSimple->SourceSideInletTemp,
                                        PlantLoop(state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).LoopNum).FluidIndex,
                                        RoutineNameSourceSideInletTemp);

        // Check for flows, do not perform simulation if no flow in load side or source side.
        if (state.dataWaterToAirHeatPumpSimple->SourceSideMassFlowRate <= 0.0 || state.dataWaterToAirHeatPumpSimple->LoadSideMassFlowRate <= 0.0) {
            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).SimFlag = false;
            return;
        } else {
            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).SimFlag = true;
        }

        if (CompOp == 0) {
            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).SimFlag = false;
            return;
        }

        ratioTDB = ((state.dataWaterToAirHeatPumpSimple->LoadSideInletDBTemp + state.dataWaterToAirHeatPumpSimple->CelsiustoKelvin) / Tref);
        ratioTS = ((state.dataWaterToAirHeatPumpSimple->SourceSideInletTemp + state.dataWaterToAirHeatPumpSimple->CelsiustoKelvin) / Tref);
        ratioVL =
            (state.dataWaterToAirHeatPumpSimple->LoadSideMassFlowRate / (AirVolFlowRateRated * PsyRhoAirFnPbTdbW(StdBaroPress, state.dataWaterToAirHeatPumpSimple->LoadSideInletDBTemp, state.dataWaterToAirHeatPumpSimple->LoadSideInletHumRat, RoutineName)));
        if (state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).DesignWaterMassFlowRate > 0.0) {
            ratioVS = (state.dataWaterToAirHeatPumpSimple->SourceSideMassFlowRate) / (state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).DesignWaterMassFlowRate);
        } else {
            ratioVS = 0.0;
        }

        state.dataWaterToAirHeatPumpSimple->QLoadTotal = HeatCapRated *
                     (HeatCapCoeff1 + (ratioTDB * HeatCapCoeff2) + (ratioTS * HeatCapCoeff3) + (ratioVL * HeatCapCoeff4) + (ratioVS * HeatCapCoeff5));
        state.dataWaterToAirHeatPumpSimple->QSensible = state.dataWaterToAirHeatPumpSimple->QLoadTotal;
        state.dataWaterToAirHeatPumpSimple->Winput = HeatPowerRated * (HeatPowerCoeff1 + (ratioTDB * HeatPowerCoeff2) + (ratioTS * HeatPowerCoeff3) + (ratioVL * HeatPowerCoeff4) +
                                   (ratioVS * HeatPowerCoeff5));

        // calculate coil outlet state variables
        state.dataWaterToAirHeatPumpSimple->LoadSideOutletEnth = state.dataWaterToAirHeatPumpSimple->LoadSideInletEnth + state.dataWaterToAirHeatPumpSimple->QLoadTotal / state.dataWaterToAirHeatPumpSimple->LoadSideMassFlowRate;
        state.dataWaterToAirHeatPumpSimple->LoadSideOutletDBTemp = state.dataWaterToAirHeatPumpSimple->LoadSideInletDBTemp + state.dataWaterToAirHeatPumpSimple->QSensible / (state.dataWaterToAirHeatPumpSimple->LoadSideMassFlowRate * CpAir);
        state.dataWaterToAirHeatPumpSimple->LoadSideOutletHumRat = PsyWFnTdbH(state.dataWaterToAirHeatPumpSimple->LoadSideOutletDBTemp, state.dataWaterToAirHeatPumpSimple->LoadSideOutletEnth, RoutineName);

        // Actual outlet conditions are "average" for time step
        if (CyclingScheme == ContFanCycCoil) {
            // continuous fan, cycling compressor
            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).OutletAirEnthalpy = PartLoadRatio * state.dataWaterToAirHeatPumpSimple->LoadSideOutletEnth + (1.0 - PartLoadRatio) * state.dataWaterToAirHeatPumpSimple->LoadSideInletEnth;
            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).OutletAirHumRat = PartLoadRatio * state.dataWaterToAirHeatPumpSimple->LoadSideOutletHumRat + (1.0 - PartLoadRatio) * state.dataWaterToAirHeatPumpSimple->LoadSideInletHumRat;
            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).OutletAirDBTemp =
                PsyTdbFnHW(state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).OutletAirEnthalpy, state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).OutletAirHumRat);
            state.dataWaterToAirHeatPumpSimple->PLRCorrLoadSideMdot = state.dataWaterToAirHeatPumpSimple->LoadSideMassFlowRate;
        } else {
            // default to cycling fan, cycling compressor
            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).OutletAirEnthalpy = state.dataWaterToAirHeatPumpSimple->LoadSideOutletEnth;
            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).OutletAirHumRat = state.dataWaterToAirHeatPumpSimple->LoadSideOutletHumRat;
            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).OutletAirDBTemp = state.dataWaterToAirHeatPumpSimple->LoadSideOutletDBTemp;
            state.dataWaterToAirHeatPumpSimple->PLRCorrLoadSideMdot = state.dataWaterToAirHeatPumpSimple->LoadSideMassFlowRate * PartLoadRatio;
        }

        // scale heat transfer rates to PLR and power to RTF
        state.dataWaterToAirHeatPumpSimple->QLoadTotal *= PartLoadRatio;
        state.dataWaterToAirHeatPumpSimple->QSensible *= PartLoadRatio;
        state.dataWaterToAirHeatPumpSimple->Winput *= RuntimeFrac;
        state.dataWaterToAirHeatPumpSimple->QSource = state.dataWaterToAirHeatPumpSimple->QLoadTotal - state.dataWaterToAirHeatPumpSimple->Winput;

        //  Add power to global variable so power can be summed by parent object
        DXElecHeatingPower = state.dataWaterToAirHeatPumpSimple->Winput;

        ReportingConstant = TimeStepSys * SecInHour;
        // Update heat pump data structure
        state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Power = state.dataWaterToAirHeatPumpSimple->Winput;
        state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).QLoadTotal = state.dataWaterToAirHeatPumpSimple->QLoadTotal;
        state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).QSensible = state.dataWaterToAirHeatPumpSimple->QSensible;
        state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).QSource = state.dataWaterToAirHeatPumpSimple->QSource;
        state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Energy = state.dataWaterToAirHeatPumpSimple->Winput * ReportingConstant;
        state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).EnergyLoadTotal = state.dataWaterToAirHeatPumpSimple->QLoadTotal * ReportingConstant;
        state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).EnergySensible = state.dataWaterToAirHeatPumpSimple->QSensible * ReportingConstant;
        state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).EnergyLatent = 0.0;
        state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).EnergySource = state.dataWaterToAirHeatPumpSimple->QSource * ReportingConstant;
        if (RuntimeFrac == 0.0) {
            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).COP = 0.0;
        } else {
            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).COP = state.dataWaterToAirHeatPumpSimple->QLoadTotal / state.dataWaterToAirHeatPumpSimple->Winput;
        }
        state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).RunFrac = RuntimeFrac;
        state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).PartLoadRatio = PartLoadRatio;
        state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).AirMassFlowRate = state.dataWaterToAirHeatPumpSimple->PLRCorrLoadSideMdot;

        if ((state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WaterCyclingMode) == WaterCycling) {
            // plant can lock flow at coil water inlet node, use design flow multiplied by PLR to calculate water mass flow rate
            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WaterMassFlowRate = state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).DesignWaterMassFlowRate * PartLoadRatio;
            SetComponentFlowRate(state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WaterMassFlowRate,
                                 state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WaterInletNodeNum,
                                 state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WaterOutletNodeNum,
                                 state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).LoopNum,
                                 state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).LoopSide,
                                 state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).BranchNum,
                                 state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).CompNum);
            if (state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WaterMassFlowRate > 0.0) {
                state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).OutletWaterTemp = state.dataWaterToAirHeatPumpSimple->SourceSideInletTemp - state.dataWaterToAirHeatPumpSimple->QSource / (state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WaterMassFlowRate * CpWater);
                state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).OutletWaterEnthalpy = state.dataWaterToAirHeatPumpSimple->SourceSideInletEnth - state.dataWaterToAirHeatPumpSimple->QSource / state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WaterMassFlowRate;
            }
        } else {
            if ((state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WaterCyclingMode) == WaterConstant) {
                if (state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WaterFlowMode) {
                    state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WaterMassFlowRate = state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).DesignWaterMassFlowRate;
                    SetComponentFlowRate(state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WaterMassFlowRate,
                                         state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WaterInletNodeNum,
                                         state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WaterOutletNodeNum,
                                         state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).LoopNum,
                                         state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).LoopSide,
                                         state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).BranchNum,
                                         state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).CompNum);
                } else {
                    state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WaterMassFlowRate = state.dataWaterToAirHeatPumpSimple->SourceSideMassFlowRate;
                }
            } else {
                state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WaterMassFlowRate = state.dataWaterToAirHeatPumpSimple->SourceSideMassFlowRate;
            }
            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).OutletWaterTemp = state.dataWaterToAirHeatPumpSimple->SourceSideInletTemp - state.dataWaterToAirHeatPumpSimple->QSource / (state.dataWaterToAirHeatPumpSimple->SourceSideMassFlowRate * CpWater);
            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).OutletWaterEnthalpy = state.dataWaterToAirHeatPumpSimple->SourceSideInletEnth - state.dataWaterToAirHeatPumpSimple->QSource / state.dataWaterToAirHeatPumpSimple->SourceSideMassFlowRate;
        }
    }

    void UpdateSimpleWatertoAirHP(EnergyPlusData &state, int const HPNum)
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Arun Shenoy
        //       DATE WRITTEN   Jan 2004
        //       MODIFIED       na
        //       RE-ENGINEERED  Kenneth Tang (Jan 2005)

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine updates the Water to Air Heat Pump outlet nodes.

        // METHODOLOGY EMPLOYED:
        // Data is moved from the HP data structure to the HP outlet nodes.

        // REFERENCES:
        // na

        // Using/Aliasing
        using DataContaminantBalance::Contaminant;
        using DataHVACGlobals::TimeStepSys;
        using PlantUtilities::SafeCopyPlantNode;

        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:

        // SUBROUTINE PARAMETER DEFINITIONS:
        // na

        // INTERFACE BLOCK SPECIFICATIONS
        // na

        // DERIVED TYPE DEFINITIONS
        // na

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int AirInletNode;
        int WaterInletNode;
        int AirOutletNode;
        int WaterOutletNode;
        Real64 ReportingConstant;

        // WatertoAirHP(HPNum)%SimFlag=.FALSE.
        if (!state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).SimFlag) {
            // Heatpump is off; just pass through conditions
            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Power = 0.0;
            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).QLoadTotal = 0.0;
            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).QSensible = 0.0;
            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).QLatent = 0.0;
            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).QSource = 0.0;
            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Energy = 0.0;
            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).EnergyLoadTotal = 0.0;
            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).EnergySensible = 0.0;
            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).EnergyLatent = 0.0;
            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).EnergySource = 0.0;
            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).COP = 0.0;
            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).RunFrac = 0.0;
            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).PartLoadRatio = 0.0;

            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).OutletAirDBTemp = state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).InletAirDBTemp;
            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).OutletAirHumRat = state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).InletAirHumRat;
            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).OutletAirEnthalpy = state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).InletAirEnthalpy;
            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).OutletWaterTemp = state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).InletWaterTemp;
            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).OutletWaterEnthalpy = state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).InletWaterEnthalpy;
        }

        AirInletNode = state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).AirInletNodeNum;
        WaterInletNode = state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WaterInletNodeNum;
        AirOutletNode = state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).AirOutletNodeNum;
        WaterOutletNode = state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WaterOutletNodeNum;

        // Set the air outlet  nodes of the WatertoAirHPSimple
        Node(AirOutletNode).MassFlowRate = Node(AirInletNode).MassFlowRate; // LoadSideMassFlowRate
        Node(AirOutletNode).Temp = state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).OutletAirDBTemp;
        Node(AirOutletNode).HumRat = state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).OutletAirHumRat;
        Node(AirOutletNode).Enthalpy = state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).OutletAirEnthalpy;

        // Set the air outlet nodes for properties that just pass through & not used
        Node(AirOutletNode).Quality = Node(AirInletNode).Quality;
        Node(AirOutletNode).Press = Node(AirInletNode).Press;
        Node(AirOutletNode).MassFlowRateMin = Node(AirInletNode).MassFlowRateMin;
        Node(AirOutletNode).MassFlowRateMax = Node(AirInletNode).MassFlowRateMax; // LoadSideMassFlowRate
        Node(AirOutletNode).MassFlowRateMinAvail = Node(AirInletNode).MassFlowRateMinAvail;
        Node(AirOutletNode).MassFlowRateMaxAvail = Node(AirInletNode).MassFlowRateMaxAvail; // LoadSideMassFlowRate

        // Set the water outlet node of the WatertoAirHPSimple
        // Set the water outlet nodes for properties that just pass through & not used
        SafeCopyPlantNode(WaterInletNode, WaterOutletNode);

        Node(WaterOutletNode).Temp = state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).OutletWaterTemp;
        Node(WaterOutletNode).Enthalpy = state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).OutletWaterEnthalpy;

        ReportingConstant = TimeStepSys * SecInHour;
        state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Energy = state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Power * ReportingConstant;
        state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).EnergyLoadTotal = state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).QLoadTotal * ReportingConstant;
        state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).EnergySensible = state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).QSensible * ReportingConstant;
        state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).EnergyLatent = state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).QLatent * ReportingConstant;
        state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).EnergySource = state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).QSource * ReportingConstant;

        if (Contaminant.CO2Simulation) {
            Node(AirOutletNode).CO2 = Node(AirInletNode).CO2;
        }
        if (Contaminant.GenericContamSimulation) {
            Node(AirOutletNode).GenContam = Node(AirInletNode).GenContam;
        }

        if (state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).reportCoilFinalSizes) {
            if (!DataGlobals::WarmupFlag && !DataGlobals::DoingHVACSizingSimulations && !DataGlobals::DoingSizing) {

                if (UtilityRoutines::SameString(state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WatertoAirHPType, "COOLING")) { // cooling
                    coilSelectionReportObj->setCoilFinalSizes(state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Name,
                                                              "Coil:" + state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WatertoAirHPType +
                                                                  ":WaterToAirHeatPump:EquationFit",
                                                              state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).RatedCapCoolTotal,
                                                              state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).RatedCapCoolSens,
                                                              state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).RatedAirVolFlowRate,
                                                              state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).RatedWaterVolFlowRate);
                } else if (UtilityRoutines::SameString(state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WatertoAirHPType, "HEATING")) { // heating
                    coilSelectionReportObj->setCoilFinalSizes(state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Name,
                                                              "Coil:" + state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WatertoAirHPType +
                                                                  ":WaterToAirHeatPump:EquationFit",
                                                              state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).RatedCapHeat,
                                                              state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).RatedCapHeat,
                                                              state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).RatedAirVolFlowRate,
                                                              state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).RatedWaterVolFlowRate);
                }
                state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).reportCoilFinalSizes = false;
            }
        }
    }

    //        End of Update subroutines for the WatertoAirHP Module
    // *****************************************************************************

    Real64 CalcEffectiveSHR(EnergyPlusData &state, int const HPNum,         // Index number for cooling coil
                            Real64 const SHRss,      // Steady-state sensible heat ratio
                            int const CyclingScheme, // Fan/compressor cycling scheme indicator
                            Real64 const RTF,        // Compressor run-time fraction
                            Real64 const QLatRated,  // Rated latent capacity
                            Real64 const QLatActual, // Actual latent capacity
                            Real64 const EnteringDB, // Entering air dry-bulb temperature
                            Real64 const EnteringWB  // Entering air wet-bulb temperature
    )
    {

        // FUNCTION INFORMATION:
        //    AUTHOR         Richard Raustad, FSEC
        //    DATE WRITTEN   September 2003
        //    MODIFIED       Kenneth Tang (Aug 2004) Added capability for simulating CycFanCycCoil
        //    RE-ENGINEERED  na

        // PURPOSE OF THIS FUNCTION:
        //    Adjust sensible heat ratio to account for degradation of DX coil latent
        //    capacity at part-load (cycling) conditions.

        // METHODOLOGY EMPLOYED:
        //    With model parameters entered by the user, the part-load latent performance
        //    of a DX cooling coil is determined for a constant air flow system with
        //    a cooling coil that cycles on/off. The model calculates the time
        //    required for condensate to begin falling from the cooling coil.
        //    Runtimes greater than this are integrated to a "part-load" latent
        //    capacity which is used to determine the "part-load" sensible heat ratio.
        //    See reference below for additional details (linear decay model, Eq. 8b).

        //    For cycling fan operation, a modified version of Henderson and Rengarajan (1996)
        //    model is used by ultilizing the fan delay time as the time-off (or time duration
        //    for the re-evaporation of moisture from time coil). Refer to Tang, C.C. (2005)

        // REFERENCES:
        //    (1) Henderson, H.I., K. Rengarajan.1996. A Model to Predict the Latent
        //    Capacity of Air Conditioners and Heat Pumps at Part-Load Conditions
        //    with Constant Fan Operation ASHRAE Transactions 102 (1), pp. 266-274.
        //    (2) Tang,C.C.. 2005. Modeling Packaged Heat Pumps in a Quasi-Steady
        //    State Energy Simulation Program. M.S. Thesis, Department of Mechanical and Aerospace Engineering,
        //    Oklahoma State University. (downloadable from www.hvac.okstate.edu)

        // Return value
        Real64 SHReff; // Effective sensible heat ratio, includes degradation due to cycling effects

        // Locals
        // FUNCTION ARGUMENT DEFINITIONS:

        // FUNCTION PARAMETER DEFINITIONS:
        // na

        // INTERFACE BLOCK SPECIFICATIONS
        // na

        // DERIVED TYPE DEFINITIONS
        // na

        // FUNCTION LOCAL VARIABLE DECLARATIONS:
        Real64 Twet; // Nominal time for condensate to begin leaving the coil's condensate drain line
        // at the current operating conditions (sec)
        Real64 Gamma; // Initial moisture evaporation rate divided by steady-state AC latent capacity
        // at the current operating conditions
        Real64 Twet_Rated;            // Twet at rated conditions (coil air flow rate and air temperatures), sec
        Real64 Gamma_Rated;           // Gamma at rated conditions (coil air flow rate and air temperatures)
        Real64 Twet_max;              // Maximum allowed value for Twet
        Real64 MaxONOFFCyclesperHour; // Maximum cycling rate of heat pump [cycles/hr]
        Real64 HPTimeConstant;        // Heat pump time constant [s]
        Real64 FanDelayTime;          // Fan delay time, time delay for the HP's fan to
        // shut off after compressor cycle off  [s]
        Real64 Ton;     // Coil on time (sec)
        Real64 Toff;    // Coil off time (sec)
        Real64 Toffa;   // Actual coil off time (sec). Equations valid for Toff <= (2.0 * Twet/Gamma)
        Real64 aa;      // Intermediate variable
        Real64 To1;     // Intermediate variable (first guess at To). To = time to the start of moisture removal
        Real64 To2;     // Intermediate variable (second guess at To). To = time to the start of moisture removal
        Real64 Error;   // Error for iteration (DO) loop
        Real64 LHRmult; // Latent Heat Ratio (LHR) multiplier. The effective latent heat ratio LHR = (1-SHRss)*LHRmult

        Twet_Rated = state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Twet_Rated;
        Gamma_Rated = state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Gamma_Rated;
        MaxONOFFCyclesperHour = state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).MaxONOFFCyclesperHour;
        HPTimeConstant = state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).HPTimeConstant;
        FanDelayTime = state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).FanDelayTime;

        //  No moisture evaporation (latent degradation) occurs for runtime fraction of 1.0
        //  All latent degradation model parameters cause divide by 0.0 if not greater than 0.0
        //  Latent degradation model parameters initialize to 0.0 meaning no evaporation model used.
        if ((RTF >= 1.0) || (QLatRated == 0.0) || (QLatActual == 0.0) || (Twet_Rated <= 0.0) || (Gamma_Rated <= 0.0) ||
            (MaxONOFFCyclesperHour <= 0.0) || (HPTimeConstant <= 0.0) || (RTF <= 0.0)) {
            SHReff = SHRss;
            return SHReff;
        }

        Twet_max = 9999.0; // high limit for Twet

        //  Calculate the model parameters at the actual operating conditions
        Twet = min(Twet_Rated * QLatRated / (QLatActual + 1.e-10), Twet_max);
        Gamma = Gamma_Rated * QLatRated * (EnteringDB - EnteringWB) / ((26.7 - 19.4) * QLatActual + 1.e-10);

        //  Calculate the compressor on and off times using a converntional thermostat curve
        Ton = 3600.0 / (4.0 * MaxONOFFCyclesperHour * (1.0 - RTF)); // duration of cooling coil on-cycle (sec)

        if ((CyclingScheme == CycFanCycCoil) && (FanDelayTime != 0.0)) {
            // For CycFanCycCoil, moisture is evaporated from the cooling coil back to the air stream
            // until the fan cycle off. Assume no evaporation from the coil after the fan shuts off.
            Toff = FanDelayTime;
        } else {
            // For ContFanCycCoil, moisture is evaporated from the cooling coil back to the air stream
            // for the entire heat pump off-cycle.
            Toff = 3600.0 / (4.0 * MaxONOFFCyclesperHour * RTF); // duration of cooling coil off-cycle (sec)
        }

        //  Cap Toff to meet the equation restriction
        if (Gamma > 0.0) {
            Toffa = min(Toff, 2.0 * Twet / Gamma);
        } else {
            Toffa = Toff;
        }

        //  Use sucessive substitution to solve for To
        aa = (Gamma * Toffa) - (0.25 / Twet) * pow_2(Gamma) * pow_2(Toffa);

        To1 = aa + HPTimeConstant;
        Error = 1.0;
        while (Error > 0.001) {
            To2 = aa - HPTimeConstant * (std::exp(-To1 / HPTimeConstant) - 1.0);
            Error = std::abs((To2 - To1) / To1);
            To1 = To2;
        }

        //  Adjust Sensible Heat Ratio (SHR) using Latent Heat Ratio (LHR) multiplier
        //  Floating underflow errors occur when -Ton/HPTimeConstant is a large negative number.
        //  Cap lower limit at -700 to avoid the underflow errors.
        aa = std::exp(max(-700.0, -Ton / HPTimeConstant));
        //  Calculate latent heat ratio multiplier
        LHRmult = max(((Ton - To2) / (Ton + HPTimeConstant * (aa - 1.0))), 0.0);

        //  Calculate part-load or "effective" sensible heat ratio
        SHReff = 1.0 - (1.0 - SHRss) * LHRmult;

        if (SHReff < SHRss) SHReff = SHRss; // Effective SHR can be less than the steady-state SHR
        if (SHReff > 1.0) SHReff = 1.0;     // Effective sensible heat ratio can't be greater than 1.0

        return SHReff;
    }

    int GetCoilIndex(EnergyPlusData &state, std::string const &CoilType, // must match coil types in this module
                     std::string const &CoilName, // must match coil names for the coil type
                     bool &ErrorsFound            // set to true if problem
    )
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         R. Raustad
        //       DATE WRITTEN   August 2007
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS FUNCTION:
        // This function looks up the coil capacity for the given coil and returns it.  If
        // incorrect coil type or name is given, ErrorsFound is returned as true and index is returned
        // as zero.

        // Using/Aliasing
        using FluidProperties::FindGlycol;

        // Return value
        int IndexNum; // returned index of matched coil

        // Obtains and Allocates WatertoAirHP related parameters from input file
        if (state.dataWaterToAirHeatPumpSimple->GetCoilsInputFlag) { // First time subroutine has been entered
            GetSimpleWatertoAirHPInput(state);
            //    WaterIndex=FindGlycol('WATER') !Initialize the WaterIndex once
            state.dataWaterToAirHeatPumpSimple->GetCoilsInputFlag = false;
        }

        IndexNum = UtilityRoutines::FindItemInList(CoilName, state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP);

        if (IndexNum == 0) {
            ShowSevereError("Could not find CoilType=\"" + CoilType + "\" with Name=\"" + CoilName + "\"");
            ErrorsFound = true;
        }

        return IndexNum;
    }

    Real64 GetCoilCapacity(EnergyPlusData &state,
                           std::string const &CoilType, // must match coil types in this module
                           std::string const &CoilName, // must match coil names for the coil type
                           bool &ErrorsFound            // set to true if problem
    )
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         Linda Lawrie
        //       DATE WRITTEN   February 2006
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS FUNCTION:
        // This function looks up the coil capacity for the given coil and returns it.  If
        // incorrect coil type or name is given, ErrorsFound is returned as true and capacity is returned
        // as negative.

        // Using/Aliasing
        using FluidProperties::FindGlycol;

        // Return value
        Real64 CoilCapacity; // returned capacity of matched coil

        // FUNCTION LOCAL VARIABLE DECLARATIONS:
        int WhichCoil;

        // Obtains and Allocates WatertoAirHP related parameters from input file
        if (state.dataWaterToAirHeatPumpSimple->GetCoilsInputFlag) { // First time subroutine has been entered
            GetSimpleWatertoAirHPInput(state);
            //    WaterIndex=FindGlycol('WATER') !Initialize the WaterIndex once
            state.dataWaterToAirHeatPumpSimple->GetCoilsInputFlag = false;
        }

        if (UtilityRoutines::SameString(CoilType, "COIL:COOLING:WATERTOAIRHEATPUMP:EQUATIONFIT") ||
            UtilityRoutines::SameString(CoilType, "COIL:HEATING:WATERTOAIRHEATPUMP:EQUATIONFIT")) {
            WhichCoil = UtilityRoutines::FindItemInList(CoilName, state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP);
            if (WhichCoil != 0) {
                if (UtilityRoutines::SameString(CoilType, "COIL:HEATING:WATERTOAIRHEATPUMP:EQUATIONFIT")) {
                    CoilCapacity = state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(WhichCoil).RatedCapHeat;
                } else {
                    CoilCapacity = state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(WhichCoil).RatedCapCoolTotal;
                }
            }
        } else {
            WhichCoil = 0;
        }

        if (WhichCoil == 0) {
            ShowSevereError("Could not find CoilType=\"" + CoilType + "\" with Name=\"" + CoilName + "\"");
            ErrorsFound = true;
            CoilCapacity = -1000.0;
        }

        return CoilCapacity;
    }

    Real64 GetCoilAirFlowRate(EnergyPlusData &state, std::string const &CoilType, // must match coil types in this module
                              std::string const &CoilName, // must match coil names for the coil type
                              bool &ErrorsFound            // set to true if problem
    )
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         Richard Raustad, FSEC
        //       DATE WRITTEN   October 2011
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS FUNCTION:
        // This function looks up the coil air flow rate for the given coil and returns it.  If
        // incorrect coil type or name is given, ErrorsFound is returned as true and capacity is returned
        // as negative.

        // Return value
        Real64 CoilAirFlowRate; // returned air volume flow rate of matched coil

        // FUNCTION LOCAL VARIABLE DECLARATIONS:
        int WhichCoil;

        // Obtains and Allocates WatertoAirHP related parameters from input file
        if (state.dataWaterToAirHeatPumpSimple->GetCoilsInputFlag) { // First time subroutine has been entered
            GetSimpleWatertoAirHPInput(state);
            //    WaterIndex=FindGlycol('WATER') !Initialize the WaterIndex once
            state.dataWaterToAirHeatPumpSimple->GetCoilsInputFlag = false;
        }

        if (CoilType == "COIL:COOLING:WATERTOAIRHEATPUMP:EQUATIONFIT" || CoilType == "COIL:HEATING:WATERTOAIRHEATPUMP:EQUATIONFIT") {
            WhichCoil = UtilityRoutines::FindItemInList(CoilName, state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP);
            if (WhichCoil != 0) {
                CoilAirFlowRate = state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(WhichCoil).RatedAirVolFlowRate;
            }
        } else {
            WhichCoil = 0;
        }

        if (WhichCoil == 0) {
            ShowSevereError("Could not find CoilType=\"" + CoilType + "\" with Name=\"" + CoilName + "\"");
            ErrorsFound = true;
            CoilAirFlowRate = -1000.0;
        }

        return CoilAirFlowRate;
    }

    int GetCoilInletNode(EnergyPlusData &state,
                         std::string const &CoilType, // must match coil types in this module
                         std::string const &CoilName, // must match coil names for the coil type
                         bool &ErrorsFound            // set to true if problem
    )
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         Linda Lawrie
        //       DATE WRITTEN   February 2006
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS FUNCTION:
        // This function looks up the given coil and returns the inlet node.  If
        // incorrect coil type or name is given, ErrorsFound is returned as true and value is returned
        // as zero.

        // Using/Aliasing
        using FluidProperties::FindGlycol;

        // Return value
        int NodeNumber; // returned outlet node of matched coil

        // FUNCTION LOCAL VARIABLE DECLARATIONS:
        int WhichCoil;

        // Obtains and Allocates WatertoAirHP related parameters from input file
        if (state.dataWaterToAirHeatPumpSimple->GetCoilsInputFlag) { // First time subroutine has been entered
            GetSimpleWatertoAirHPInput(state);
            //    WaterIndex=FindGlycol('WATER') !Initialize the WaterIndex once
            state.dataWaterToAirHeatPumpSimple->GetCoilsInputFlag = false;
        }

        WhichCoil = UtilityRoutines::FindItemInList(CoilName, state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP);
        if (WhichCoil != 0) {
            NodeNumber = state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(WhichCoil).AirInletNodeNum;
        }

        if (WhichCoil == 0) {
            ShowSevereError("Could not find CoilType=\"" + CoilType + "\" with Name=\"" + CoilName + "\"");
            ErrorsFound = true;
            NodeNumber = 0;
        }

        return NodeNumber;
    }

    int GetCoilOutletNode(EnergyPlusData &state,
                          std::string const &CoilType, // must match coil types in this module
                          std::string const &CoilName, // must match coil names for the coil type
                          bool &ErrorsFound            // set to true if problem
    )
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         R. Raustad
        //       DATE WRITTEN   July 2007
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS FUNCTION:
        // This function looks up the given coil and returns the outlet node.  If
        // incorrect coil type or name is given, ErrorsFound is returned as true and value is returned
        // as zero.

        // Using/Aliasing
        using FluidProperties::FindGlycol;

        // Return value
        int NodeNumber; // returned outlet node of matched coil

        // FUNCTION LOCAL VARIABLE DECLARATIONS:
        int WhichCoil;

        // Obtains and Allocates WatertoAirHP related parameters from input file
        if (state.dataWaterToAirHeatPumpSimple->GetCoilsInputFlag) { // First time subroutine has been entered
            GetSimpleWatertoAirHPInput(state);
            //    WaterIndex=FindGlycol('WATER') !Initialize the WaterIndex once
            state.dataWaterToAirHeatPumpSimple->GetCoilsInputFlag = false;
        }

        WhichCoil = UtilityRoutines::FindItemInList(CoilName, state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP);
        if (WhichCoil != 0) {
            NodeNumber = state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(WhichCoil).AirOutletNodeNum;
        }

        if (WhichCoil == 0) {
            ShowSevereError("Could not find CoilType=\"" + CoilType + "\" with Name=\"" + CoilName + "\"");
            ErrorsFound = true;
            NodeNumber = 0;
        }

        return NodeNumber;
    }

    void SetSimpleWSHPData(EnergyPlusData &state, int const SimpleWSHPNum,              // Number of OA Controller
                           bool &ErrorsFound,                    // Set to true if certain errors found
                           int const WaterCyclingMode,           // the coil water flow mode (cycling, constant or constantondemand)
                           Optional_int CompanionCoolingCoilNum, // Index to cooling coil for heating coil = SimpleWSHPNum
                           Optional_int CompanionHeatingCoilNum  // Index to heating coil for cooling coil = SimpleWSHPNum
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Richard Raustad
        //       DATE WRITTEN   June 2009
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This routine was designed to "push" information from a parent object to
        // this WSHP coil object.

        // Using/Aliasing
        using FluidProperties::FindGlycol;
        using General::TrimSigDigits;

        // Obtains and Allocates WatertoAirHP related parameters from input file
        if (state.dataWaterToAirHeatPumpSimple->GetCoilsInputFlag) { // First time subroutine has been entered
            GetSimpleWatertoAirHPInput(state);
            //    WaterIndex=FindGlycol('WATER') !Initialize the WaterIndex once
            state.dataWaterToAirHeatPumpSimple->GetCoilsInputFlag = false;
        }

        if (SimpleWSHPNum <= 0 || SimpleWSHPNum > state.dataWaterToAirHeatPumpSimple->NumWatertoAirHPs) {
            ShowSevereError("SetSimpleWSHPData: called with WSHP Coil Number out of range=" + TrimSigDigits(SimpleWSHPNum) + " should be >0 and <" +
                            TrimSigDigits(state.dataWaterToAirHeatPumpSimple->NumWatertoAirHPs));
            ErrorsFound = true;
            return;
        }

        state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(SimpleWSHPNum).WaterCyclingMode = WaterCyclingMode;
        if (present(CompanionCoolingCoilNum)) {
            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(SimpleWSHPNum).CompanionCoolingCoilNum = CompanionCoolingCoilNum;
            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(CompanionCoolingCoilNum).CompanionHeatingCoilNum = SimpleWSHPNum;
            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(CompanionCoolingCoilNum).WaterCyclingMode = WaterCyclingMode;
        }

        if (present(CompanionHeatingCoilNum)) {
            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(SimpleWSHPNum).CompanionHeatingCoilNum = CompanionHeatingCoilNum;
            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(CompanionHeatingCoilNum).CompanionCoolingCoilNum = SimpleWSHPNum;
            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(CompanionHeatingCoilNum).WaterCyclingMode = WaterCyclingMode;
        }
    }

} // namespace WaterToAirHeatPumpSimple

} // namespace EnergyPlus
