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
#include <algorithm>
#include <cassert>
#include <cmath>
#include <string>
#include <unordered_map>

// ObjexxFCL Headers
#include <ObjexxFCL/Array.functions.hh>
#include <ObjexxFCL/Fmath.hh>
#include <ObjexxFCL/gio.hh>

// EnergyPlus Headers
#include <EnergyPlus/BranchNodeConnections.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataAirLoop.hh>
#include <EnergyPlus/DataAirSystems.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataGlobalConstants.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataHeatBalFanSys.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/DataZoneEnergyDemands.hh>
#include <EnergyPlus/DataZoneEquipment.hh>
#include <EnergyPlus/FanCoilUnits.hh>
#include <EnergyPlus/HVACStandAloneERV.hh>
#include <EnergyPlus/HVACVariableRefrigerantFlow.hh>
#include <EnergyPlus/HybridUnitaryAirConditioners.hh>
#include <EnergyPlus/OutdoorAirUnit.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/PackagedTerminalHeatPump.hh>
#include <EnergyPlus/Plant/DataPlant.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/PurchasedAirManager.hh>
#include <EnergyPlus/SystemReports.hh>
#include <EnergyPlus/UnitVentilator.hh>
#include <EnergyPlus/UtilityRoutines.hh>
#include <EnergyPlus/WindowAC.hh>

namespace EnergyPlus {

namespace SystemReports {

    // Module containing the routines dealing with Mechanical Ventilation Loads and Energy Reporting (Outside Air)

    // MODULE INFORMATION:
    //       AUTHOR         Mike Witte, Linda Lawrie, Dan Fisher
    //       DATE WRITTEN   Apr-Jul 2005
    //       MODIFIED       22Aug2010 Craig Wray - added Fan:ComponentModel
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS MODULE:
    // This module embodies the scheme(s) for reporting ventilation loads and energy use.

    // METHODOLOGY EMPLOYED:
    // na

    // REFERENCES:
    // na

    // OTHER NOTES:
    // na

    // Using/Aliasing
    using namespace DataLoopNode;
    using namespace DataAirLoop;
    using namespace DataGlobals;
    using namespace DataHVACGlobals;
    using namespace DataPlant;
    using namespace DataZoneEquipment;
    using namespace DataAirSystems;

    // Data
    // MODULE PARAMETER DEFINITIONS:
    int const NoHeatNoCool(0);
    int const CoolingOnly(1);
    int const HeatingOnly(2);
    int const HeatAndCool(3);
    int const MaxSetBackCount(3);

    static std::string const BlankString;

    // DERIVED TYPE DEFINITIONS:

    // MODULE VARIABLE DECLARATIONS:
    // Ventilation Report Variables
    Array1D<Real64> MaxCoolingLoadMetByVent;
    Array1D<Real64> MaxCoolingLoadAddedByVent;
    Array1D<Real64> MaxOvercoolingByVent;
    Array1D<Real64> MaxHeatingLoadMetByVent;
    Array1D<Real64> MaxHeatingLoadAddedByVent;
    Array1D<Real64> MaxOverheatingByVent;
    Array1D<Real64> MaxNoLoadHeatingByVent;
    Array1D<Real64> MaxNoLoadCoolingByVent;

    Array1D<Real64> RemMaxCoolingLoadMetByVent;
    Array1D<Real64> RemMaxCoolingLoadAddedByVent;
    Array1D<Real64> RemMaxOvercoolingByVent;
    Array1D<Real64> RemMaxHeatingLoadMetByVent;
    Array1D<Real64> RemMaxHeatingLoadAddedByVent;
    Array1D<Real64> RemMaxOverheatingByVent;
    Array1D<Real64> RemMaxNoLoadHeatingByVent;
    Array1D<Real64> RemMaxNoLoadCoolingByVent;

    Array1D<Real64> LastMaxCoolingLoadMetByVent;
    Array1D<Real64> LastMaxCoolingLoadAddedByVent;
    Array1D<Real64> LastMaxOvercoolingByVent;
    Array1D<Real64> LastMaxHeatingLoadMetByVent;
    Array1D<Real64> LastMaxHeatingLoadAddedByVent;
    Array1D<Real64> LastMaxOverheatingByVent;
    Array1D<Real64> LastMaxNoLoadHeatingByVent;
    Array1D<Real64> LastMaxNoLoadCoolingByVent;

    Array1D<Real64> SysTotZoneLoadHTNG;
    Array1D<Real64> SysTotZoneLoadCLNG;
    Array1D<Real64> SysOALoadHTNG;
    Array1D<Real64> SysOALoadCLNG;
    Array1D<Real64> SysTotHTNG;
    Array1D<Real64> SysTotCLNG;

    Array1D<Real64> SysTotH2OHOT;
    Array1D<Real64> SysTotH2OCOLD;
    Array1D<Real64> SysTotElec;
    Array1D<Real64> SysTotNaturalGas;
    Array1D<Real64> SysTotPropane;
    Array1D<Real64> SysTotSteam;

    Array1D<Real64> SysHumidHTNG;
    Array1D<Real64> SysHumidElec;
    Array1D<Real64> SysHumidNaturalGas;
    Array1D<Real64> SysHumidPropane;
    Array1D<Real64> SysEvapCLNG;
    Array1D<Real64> SysEvapElec;
    Array1D<Real64> SysHeatExHTNG;
    Array1D<Real64> SysHeatExCLNG;
    Array1D<Real64> DesDehumidCLNG;
    Array1D<Real64> DesDehumidElec;
    Array1D<Real64> SysSolarCollectHeating;
    Array1D<Real64> SysSolarCollectCooling;
    Array1D<Real64> SysUserDefinedTerminalHeating;
    Array1D<Real64> SysUserDefinedTerminalCooling;

    Array1D<Real64> SysFANCompHTNG;
    Array1D<Real64> SysFANCompElec;
    Array1D<Real64> SysCCCompCLNG;
    Array1D<Real64> SysCCCompH2OCOLD;
    Array1D<Real64> SysCCCompElec;
    Array1D<Real64> SysHCCompH2OHOT;
    Array1D<Real64> SysHCCompElec;
    Array1D<Real64> SysHCCompElecRes;
    Array1D<Real64> SysHCCompHTNG;
    Array1D<Real64> SysHCCompNaturalGas;
    Array1D<Real64> SysHCCompPropane;
    Array1D<Real64> SysHCCompSteam;
    Array1D<Real64> SysDomesticH2O;

    Array1D<Real64> ZoneOAMassFlow;       // zone mech vent mass flow rate {kg/s}
    Array1D<Real64> ZoneOAMass;           // zone mech vent total mass for time {kg}
    Array1D<Real64> ZoneOAVolFlowStdRho;  // zone mech vent volume flow rate at standard density {m3/s}
    Array1D<Real64> ZoneOAVolStdRho;      // zone mech vent total volume OA at standard density {m3/s}
    Array1D<Real64> ZoneOAVolFlowCrntRho; // zone mech vent volume flow rate at current density {m3/s}
    Array1D<Real64> ZoneOAVolCrntRho;     // zone mech vent total volume OA at current density {m3/s}
    Array1D<Real64> ZoneMechACH;          // zone mech vent air changes per hour {ACH}

    bool AirLoopLoadsReportEnabled(true);
    bool VentLoadsReportEnabled(true);
    bool VentEnergyReportEnabled(false);
    bool VentReportStructureCreated(false);
    int TotalLoopConnects(0); // Total number of loop connections
    int MaxLoopArraySize(100);
    int MaxCompArraySize(500);
    int DBFlag(0);

    Array1D_int SetBackCounter;
    Array1D_int HeatCoolFlag;
    Array1D_int FirstHeatCoolFlag;
    Array1D_int FirstHeatCoolHour;
    Array1D_int LastHeatCoolFlag;
    Array1D_int LastHeatCoolHour;
    Array1D_bool AirLoopCalcDone;
    Array1D_bool NoLoadFlag;
    Array1D_bool UnmetLoadFlag;

    // SUBROUTINE SPECIFICATIONS FOR MODULE SystemReports

    // Reporting Initialization

    // Reporting routines for module

    // Object Data
    Array1D<SummarizeLoads> Vent;

    // MODULE SUBROUTINES:
    //*************************************************************************

    // Functions

    void InitEnergyReports(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Dan Fisher
        //       DATE WRITTEN   April 2005
        //       MODIFIED
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Initializes the energy components of the data structures

        // METHODOLOGY EMPLOYED:
        // Once all compsets have been established (second iteration) find all components
        // subcomponents, etc.

        // Using/Aliasing
        using Psychrometrics::PsyHFnTdbW;
        using Psychrometrics::PsyRhoAirFnPbTdbW;
        using namespace DataGlobalConstants;

        // SUBROUTINE PARAMETER DEFINITIONS:
        int const EnergyTransfer(1);

        int AirDistUnitNum;
        int MatchLoop;
        int MatchLoopType;
        int MatchBranch;
        int MatchComp;
        int AirLoopNum;
        int BranchNum;
        int ZoneInletNodeNum;
        int CompNum;
        int VarNum;
        int SubCompNum;
        int SubSubCompNum;
        int EquipNum;
        int SubEquipNum;
        int SubSubEquipNum;
        int CtrlZoneNum;
        int NodeIndex;
        int Idx;
        int ListNum;
        int SAPNum;
        int SAPOutNode;
        int MainBranchNum;
        int SupplyCoolBranchNum;
        int SupplyHeatBranchNum;
        int VarType;
        int VarIndex;
        int OutNum;
        int NodeCount;
        int PlantLoopNum;
        int NumZoneConnectComps;
        int NumZoneConnectSubComps;
        int NumZoneConnectSubSubComps;
        int NumAirSysConnectComps;
        int NumAirSysConnectSubComps;
        int NumAirSysConnectSubSubComps;
        int ArrayCount;
        int LoopType;
        int LoopNum;
        int FirstIndex;
        int LastIndex;
        int LoopCount;
        std::string CompType;
        std::string CompName;
        bool MatchFound;
        static bool OneTimeFlag(true); // Flag set to make sure you initialize reports one time
        bool ConnectionFlag(false);

        if (!VentReportStructureCreated) return;

        if (OneTimeFlag) {

            // ***I think we need to preprocess the main components on the branch to get them in order***
            // This needs to be done before we start in on the component loop
            // GetChildrenData will put all of the subcomponents in order for us

            for (CtrlZoneNum = 1; CtrlZoneNum <= NumOfZones; ++CtrlZoneNum) {
                if (!ZoneEquipConfig(CtrlZoneNum).IsControlled) continue;
                ZoneEquipConfig(CtrlZoneNum).EquipListIndex =
                    UtilityRoutines::FindItemInList(ZoneEquipConfig(CtrlZoneNum).EquipListName, ZoneEquipList);
                ListNum = ZoneEquipConfig(CtrlZoneNum).EquipListIndex;
                for (ZoneInletNodeNum = 1; ZoneInletNodeNum <= ZoneEquipConfig(CtrlZoneNum).NumInletNodes; ++ZoneInletNodeNum) {
                    AirLoopNum = ZoneEquipConfig(CtrlZoneNum).InletNodeAirLoopNum(ZoneInletNodeNum);
                    for (CompNum = 1; CompNum <= ZoneEquipList(ListNum).NumOfEquipTypes; ++CompNum) {
                        for (NodeCount = 1; NodeCount <= ZoneEquipList(ListNum).EquipData(CompNum).NumOutlets; ++NodeCount) {
                            if (ZoneEquipList(ListNum).EquipData(CompNum).OutletNodeNums(NodeCount) ==
                                ZoneEquipConfig(CtrlZoneNum).AirDistUnitCool(ZoneInletNodeNum).OutNode) {
                                ZoneEquipConfig(CtrlZoneNum).AirDistUnitCool(ZoneInletNodeNum).AirDistUnitIndex = CompNum;
                                if (ZoneEquipConfig(CtrlZoneNum).AirDistUnitCool(ZoneInletNodeNum).SupplyAirPathExists) {
                                    for (SAPNum = 1; SAPNum <= NumSupplyAirPaths; ++SAPNum) {
                                        for (SAPOutNode = 1; SAPOutNode <= SupplyAirPath(SAPNum).NumOutletNodes; ++SAPOutNode) {
                                            if (ZoneEquipConfig(CtrlZoneNum).AirDistUnitCool(ZoneInletNodeNum).InNode ==
                                                SupplyAirPath(SAPNum).OutletNode(SAPOutNode)) {
                                                ZoneEquipConfig(CtrlZoneNum).AirDistUnitCool(ZoneInletNodeNum).SupplyAirPathIndex = SAPNum;
                                                for (OutNum = 1; OutNum <= state.dataAirLoop->AirToZoneNodeInfo(AirLoopNum).NumSupplyNodes; ++OutNum) {
                                                    if (state.dataAirLoop->AirToZoneNodeInfo(AirLoopNum).ZoneEquipSupplyNodeNum(OutNum) ==
                                                        SupplyAirPath(SAPNum).InletNodeNum) {
                                                        ZoneEquipConfig(CtrlZoneNum).AirDistUnitCool(ZoneInletNodeNum).SupplyBranchIndex =
                                                            PrimaryAirSystem(AirLoopNum).OutletBranchNum(OutNum);
                                                        if (PrimaryAirSystem(AirLoopNum).Splitter.Exists) {
                                                            for (MainBranchNum = 1; MainBranchNum <= PrimaryAirSystem(AirLoopNum).NumBranches;
                                                                 ++MainBranchNum) {
                                                                if (PrimaryAirSystem(AirLoopNum).Branch(MainBranchNum).NodeNumOut ==
                                                                    PrimaryAirSystem(AirLoopNum).Splitter.NodeNumIn) {
                                                                    ZoneEquipConfig(CtrlZoneNum).AirDistUnitCool(ZoneInletNodeNum).MainBranchIndex =
                                                                        MainBranchNum;
                                                                }
                                                            }
                                                        } else { // no splitter
                                                            ZoneEquipConfig(CtrlZoneNum).AirDistUnitCool(ZoneInletNodeNum).MainBranchIndex =
                                                                ZoneEquipConfig(CtrlZoneNum).AirDistUnitCool(ZoneInletNodeNum).SupplyBranchIndex;
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                } else { // no supply air path
                                    if (AirLoopNum > 0) {
                                        for (NodeIndex = 1; NodeIndex <= state.dataAirLoop->AirToZoneNodeInfo(AirLoopNum).NumSupplyNodes; ++NodeIndex) {
                                            if (state.dataAirLoop->AirToZoneNodeInfo(AirLoopNum).ZoneEquipSupplyNodeNum(NodeIndex) ==
                                                ZoneEquipConfig(CtrlZoneNum).AirDistUnitCool(ZoneInletNodeNum).InNode) {
                                                for (BranchNum = 1; BranchNum <= PrimaryAirSystem(AirLoopNum).NumBranches; ++BranchNum) {
                                                    if (PrimaryAirSystem(AirLoopNum).Branch(BranchNum).NodeNumOut ==
                                                        state.dataAirLoop->AirToZoneNodeInfo(AirLoopNum).AirLoopSupplyNodeNum(NodeIndex)) {
                                                        ZoneEquipConfig(CtrlZoneNum).AirDistUnitCool(ZoneInletNodeNum).SupplyBranchIndex = BranchNum;
                                                        if (PrimaryAirSystem(AirLoopNum).Splitter.Exists) {
                                                            for (MainBranchNum = 1; MainBranchNum <= PrimaryAirSystem(AirLoopNum).NumBranches;
                                                                 ++MainBranchNum) {
                                                                if (PrimaryAirSystem(AirLoopNum).Branch(MainBranchNum).NodeNumOut ==
                                                                    PrimaryAirSystem(AirLoopNum).Splitter.NodeNumIn) {
                                                                    ZoneEquipConfig(CtrlZoneNum).AirDistUnitCool(ZoneInletNodeNum).MainBranchIndex =
                                                                        MainBranchNum;
                                                                }
                                                            }
                                                        } else { // no splitter
                                                            ZoneEquipConfig(CtrlZoneNum).AirDistUnitCool(ZoneInletNodeNum).MainBranchIndex =
                                                                ZoneEquipConfig(CtrlZoneNum).AirDistUnitCool(ZoneInletNodeNum).SupplyAirPathIndex;
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            } else if (ZoneEquipList(ListNum).EquipData(CompNum).OutletNodeNums(NodeCount) ==
                                       ZoneEquipConfig(CtrlZoneNum).AirDistUnitHeat(ZoneInletNodeNum).InNode) {
                                ZoneEquipConfig(CtrlZoneNum).AirDistUnitHeat(ZoneInletNodeNum).AirDistUnitIndex = CompNum;
                                if (ZoneEquipConfig(CtrlZoneNum).AirDistUnitHeat(ZoneInletNodeNum).SupplyAirPathExists) {
                                    for (SAPNum = 1; SAPNum <= NumSupplyAirPaths; ++SAPNum) {
                                        for (NodeIndex = 1; NodeIndex <= state.dataAirLoop->AirToZoneNodeInfo(AirLoopNum).NumSupplyNodes; ++NodeIndex) {
                                            if (state.dataAirLoop->AirToZoneNodeInfo(AirLoopNum).ZoneEquipSupplyNodeNum(NodeIndex) ==
                                                SupplyAirPath(SAPNum).InletNodeNum) {
                                                for (BranchNum = 1; BranchNum <= PrimaryAirSystem(AirLoopNum).NumBranches; ++BranchNum) {
                                                    if (PrimaryAirSystem(AirLoopNum).Branch(BranchNum).NodeNumOut ==
                                                        state.dataAirLoop->AirToZoneNodeInfo(AirLoopNum).AirLoopSupplyNodeNum(NodeIndex)) {
                                                        ZoneEquipConfig(CtrlZoneNum).AirDistUnitHeat(ZoneInletNodeNum).SupplyBranchIndex = BranchNum;
                                                        if (PrimaryAirSystem(AirLoopNum).Splitter.Exists) {
                                                            for (MainBranchNum = 1; MainBranchNum <= PrimaryAirSystem(AirLoopNum).NumBranches;
                                                                 ++MainBranchNum) {
                                                                if (PrimaryAirSystem(AirLoopNum).Branch(MainBranchNum).NodeNumOut ==
                                                                    PrimaryAirSystem(AirLoopNum).Splitter.NodeNumIn) {
                                                                    ZoneEquipConfig(CtrlZoneNum).AirDistUnitHeat(ZoneInletNodeNum).MainBranchIndex =
                                                                        MainBranchNum;
                                                                }
                                                            }
                                                        } else { // no splitter
                                                            ZoneEquipConfig(CtrlZoneNum).AirDistUnitHeat(ZoneInletNodeNum).MainBranchIndex =
                                                                ZoneEquipConfig(CtrlZoneNum).AirDistUnitHeat(ZoneInletNodeNum).SupplyAirPathIndex;
                                                        }
                                                    }
                                                }
                                            }
                                        }

                                        for (SAPOutNode = 1; SAPOutNode <= SupplyAirPath(SAPNum).NumOutletNodes; ++SAPOutNode) {
                                            if (ZoneInletNodeNum == SupplyAirPath(SAPNum).OutletNode(SAPOutNode)) {
                                                ZoneEquipConfig(CtrlZoneNum).AirDistUnitHeat(ZoneInletNodeNum).SupplyAirPathIndex = SAPNum;
                                            }
                                        }
                                    }
                                } else { // no supply air path
                                    if (AirLoopNum > 0) {
                                        for (NodeIndex = 1; NodeIndex <= state.dataAirLoop->AirToZoneNodeInfo(AirLoopNum).NumSupplyNodes; ++NodeIndex) {
                                            if (state.dataAirLoop->AirToZoneNodeInfo(AirLoopNum).ZoneEquipSupplyNodeNum(NodeIndex) ==
                                                ZoneEquipConfig(CtrlZoneNum).AirDistUnitHeat(ZoneInletNodeNum).InNode) {
                                                for (BranchNum = 1; BranchNum <= PrimaryAirSystem(AirLoopNum).NumBranches; ++BranchNum) {
                                                    if (PrimaryAirSystem(AirLoopNum).Branch(BranchNum).NodeNumOut ==
                                                        state.dataAirLoop->AirToZoneNodeInfo(AirLoopNum).AirLoopSupplyNodeNum(NodeIndex)) {
                                                        ZoneEquipConfig(CtrlZoneNum).AirDistUnitHeat(ZoneInletNodeNum).SupplyBranchIndex = BranchNum;
                                                        if (PrimaryAirSystem(AirLoopNum).Splitter.Exists) {
                                                            for (MainBranchNum = 1; MainBranchNum <= PrimaryAirSystem(AirLoopNum).NumBranches;
                                                                 ++MainBranchNum) {
                                                                if (PrimaryAirSystem(AirLoopNum).Branch(MainBranchNum).NodeNumOut ==
                                                                    PrimaryAirSystem(AirLoopNum).Splitter.NodeNumIn) {
                                                                    ZoneEquipConfig(CtrlZoneNum).AirDistUnitHeat(ZoneInletNodeNum).MainBranchIndex =
                                                                        MainBranchNum;
                                                                }
                                                            }
                                                        } else { // no splitter
                                                            ZoneEquipConfig(CtrlZoneNum).AirDistUnitHeat(ZoneInletNodeNum).MainBranchIndex =
                                                                ZoneEquipConfig(CtrlZoneNum).AirDistUnitHeat(ZoneInletNodeNum).SupplyAirPathIndex;
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            } else {

                                // Can't tell if there's an error based on this code...need to check logical flags separately
                            }
                        }
                    }
                }
            }

            for (CtrlZoneNum = 1; CtrlZoneNum <= NumOfZones; ++CtrlZoneNum) {
                if (!ZoneEquipConfig(CtrlZoneNum).IsControlled) continue;
                ZoneEquipConfig(CtrlZoneNum).EquipListIndex =
                    UtilityRoutines::FindItemInList(ZoneEquipConfig(CtrlZoneNum).EquipListName, ZoneEquipList);
                ListNum = ZoneEquipConfig(CtrlZoneNum).EquipListIndex;
                // loop over the zone supply air path inlet nodes
                for (ZoneInletNodeNum = 1; ZoneInletNodeNum <= ZoneEquipConfig(CtrlZoneNum).NumInletNodes; ++ZoneInletNodeNum) {
                    AirLoopNum = ZoneEquipConfig(CtrlZoneNum).InletNodeAirLoopNum(ZoneInletNodeNum);

                    // 1. Find HVAC component plant loop connections
                    MainBranchNum = ZoneEquipConfig(CtrlZoneNum).AirDistUnitHeat(ZoneInletNodeNum).MainBranchIndex;
                    MainBranchNum = max(MainBranchNum, ZoneEquipConfig(CtrlZoneNum).AirDistUnitCool(ZoneInletNodeNum).MainBranchIndex);
                    if (MainBranchNum > 0) MatchPlantSys(AirLoopNum, MainBranchNum);
                    SupplyCoolBranchNum = ZoneEquipConfig(CtrlZoneNum).AirDistUnitCool(ZoneInletNodeNum).SupplyBranchIndex;
                    if (SupplyCoolBranchNum > 0 && (SupplyCoolBranchNum != MainBranchNum)) MatchPlantSys(AirLoopNum, SupplyCoolBranchNum);
                    SupplyHeatBranchNum = ZoneEquipConfig(CtrlZoneNum).AirDistUnitHeat(ZoneInletNodeNum).SupplyBranchIndex;
                    if (SupplyHeatBranchNum > 0 && (SupplyHeatBranchNum != MainBranchNum)) MatchPlantSys(AirLoopNum, SupplyHeatBranchNum);

                    AirDistUnitNum = ZoneEquipConfig(CtrlZoneNum).AirDistUnitCool(ZoneInletNodeNum).AirDistUnitIndex;
                    AirDistUnitNum = max(AirDistUnitNum, ZoneEquipConfig(CtrlZoneNum).AirDistUnitHeat(ZoneInletNodeNum).AirDistUnitIndex);
                    if (ListNum > 0 && AirDistUnitNum > 0) {
                        for (VarNum = 1; VarNum <= ZoneEquipList(ListNum).EquipData(AirDistUnitNum).NumMeteredVars; ++VarNum) {
                            if (ZoneEquipList(ListNum).EquipData(AirDistUnitNum).MeteredVar(VarNum).ResourceType == iRT_EnergyTransfer) {
                                ZoneEquipList(ListNum).EquipData(AirDistUnitNum).EnergyTransComp = EnergyTransfer;
                                CompType = ZoneEquipList(ListNum).EquipData(AirDistUnitNum).TypeOf;
                                CompName = ZoneEquipList(ListNum).EquipData(AirDistUnitNum).Name;
                                Idx = 0;
                                FindDemandSideMatch(CompType, CompName, MatchFound, MatchLoopType, MatchLoop, MatchBranch, MatchComp);
                                if (MatchFound)
                                    UpdateZoneCompPtrArray(Idx, ListNum, AirDistUnitNum, MatchLoopType, MatchLoop, MatchBranch, MatchComp);
                                ZoneEquipList(ListNum).EquipData(AirDistUnitNum).ZoneEqToPlantPtr = Idx;
                                break;
                            }
                        }
                        for (SubEquipNum = 1; SubEquipNum <= ZoneEquipList(ListNum).EquipData(AirDistUnitNum).NumSubEquip; ++SubEquipNum) {
                            for (VarNum = 1; VarNum <= ZoneEquipList(ListNum).EquipData(AirDistUnitNum).SubEquipData(SubEquipNum).NumMeteredVars;
                                 ++VarNum) {
                                if (ZoneEquipList(ListNum).EquipData(AirDistUnitNum).SubEquipData(SubEquipNum).MeteredVar(VarNum).ResourceType ==
                                    iRT_EnergyTransfer) {
                                    ZoneEquipList(ListNum).EquipData(AirDistUnitNum).SubEquipData(SubEquipNum).EnergyTransComp = EnergyTransfer;
                                    CompType = ZoneEquipList(ListNum).EquipData(AirDistUnitNum).SubEquipData(SubEquipNum).TypeOf;
                                    CompName = ZoneEquipList(ListNum).EquipData(AirDistUnitNum).SubEquipData(SubEquipNum).Name;
                                    Idx = 0;
                                    FindDemandSideMatch(CompType, CompName, MatchFound, MatchLoopType, MatchLoop, MatchBranch, MatchComp);
                                    if (MatchFound)
                                        UpdateZoneSubCompPtrArray(
                                            Idx, ListNum, AirDistUnitNum, SubEquipNum, MatchLoopType, MatchLoop, MatchBranch, MatchComp);
                                    ZoneEquipList(ListNum).EquipData(AirDistUnitNum).SubEquipData(SubEquipNum).ZoneEqToPlantPtr = Idx;
                                    break;
                                }
                            }
                            for (SubSubEquipNum = 1;
                                 SubSubEquipNum <= ZoneEquipList(ListNum).EquipData(AirDistUnitNum).SubEquipData(SubEquipNum).NumSubSubEquip;
                                 ++SubSubEquipNum) {
                                for (VarNum = 1; VarNum <= ZoneEquipList(ListNum)
                                                               .EquipData(AirDistUnitNum)
                                                               .SubEquipData(SubEquipNum)
                                                               .SubSubEquipData(SubSubEquipNum)
                                                               .NumMeteredVars;
                                     ++VarNum) {
                                    if (ZoneEquipList(ListNum)
                                            .EquipData(AirDistUnitNum)
                                            .SubEquipData(SubEquipNum)
                                            .SubSubEquipData(SubSubEquipNum)
                                            .MeteredVar(VarNum)
                                            .ResourceType == iRT_EnergyTransfer) {
                                        ZoneEquipList(ListNum)
                                            .EquipData(AirDistUnitNum)
                                            .SubEquipData(SubEquipNum)
                                            .SubSubEquipData(SubSubEquipNum)
                                            .EnergyTransComp = EnergyTransfer;
                                        CompType = ZoneEquipList(ListNum)
                                                       .EquipData(AirDistUnitNum)
                                                       .SubEquipData(SubEquipNum)
                                                       .SubSubEquipData(SubSubEquipNum)
                                                       .TypeOf;
                                        CompName = ZoneEquipList(ListNum)
                                                       .EquipData(AirDistUnitNum)
                                                       .SubEquipData(SubEquipNum)
                                                       .SubSubEquipData(SubSubEquipNum)
                                                       .Name;
                                        Idx = 0;
                                        FindDemandSideMatch(CompType, CompName, MatchFound, MatchLoopType, MatchLoop, MatchBranch, MatchComp);
                                        if (MatchFound)
                                            UpdateZoneSubSubCompPtrArray(Idx,
                                                                         ListNum,
                                                                         AirDistUnitNum,
                                                                         SubEquipNum,
                                                                         SubSubEquipNum,
                                                                         MatchLoopType,
                                                                         MatchLoop,
                                                                         MatchBranch,
                                                                         MatchComp);
                                        ZoneEquipList(ListNum)
                                            .EquipData(AirDistUnitNum)
                                            .SubEquipData(SubEquipNum)
                                            .SubSubEquipData(SubSubEquipNum)
                                            .ZoneEqToPlantPtr = Idx;
                                        break;
                                    }
                                }
                            }
                        }
                    }

                    // Eliminate duplicates in the connection arrays
                    if (allocated(ZoneCompToPlant)) {
                        EquipNum = isize(ZoneCompToPlant);
                    } else {
                        EquipNum = 0;
                    }
                    if (allocated(ZoneSubCompToPlant)) {
                        SubEquipNum = isize(ZoneSubCompToPlant);
                    } else {
                        SubEquipNum = 0;
                    }
                    if (allocated(ZoneSubSubCompToPlant)) {
                        SubSubEquipNum = isize(ZoneSubSubCompToPlant);
                    } else {
                        SubSubEquipNum = 0;
                    }
                    if (allocated(AirSysCompToPlant)) {
                        CompNum = isize(AirSysCompToPlant);
                    } else {
                        CompNum = 0;
                    }
                    if (allocated(AirSysSubCompToPlant)) {
                        SubCompNum = isize(AirSysSubCompToPlant);
                    } else {
                        SubCompNum = 0;
                    }
                    if (allocated(AirSysSubSubCompToPlant)) {
                        SubSubCompNum = isize(AirSysSubSubCompToPlant);
                    } else {
                        SubSubCompNum = 0;
                    }

                    if (EquipNum > 0) {
                        ArrayCount = 0;
                        for (int i = 1; i <= EquipNum; ++i) {
                            auto const &zi(ZoneCompToPlant(i));
                            bool duplicate(false);
                            for (int j = 1; j <= ArrayCount; ++j) {
                                auto const &zj(ZoneCompToPlant(j));
                                if ((zi.ZoneEqListNum == zj.ZoneEqListNum) && (zi.ZoneEqCompNum == zj.ZoneEqCompNum)) { // Duplicate
                                    duplicate = true;
                                    break;
                                }
                            }
                            if (!duplicate) {
                                ++ArrayCount;
                                if (i > ArrayCount) { // Copy to lower position
                                    auto &za(ZoneCompToPlant(ArrayCount));
                                    za.ZoneEqListNum = zi.ZoneEqListNum;
                                    za.ZoneEqCompNum = zi.ZoneEqCompNum;
                                    za.PlantLoopType = zi.PlantLoopType;
                                    za.PlantLoopNum = zi.PlantLoopNum;
                                    za.PlantLoopBranch = zi.PlantLoopBranch;
                                    za.PlantLoopComp = zi.PlantLoopComp;
                                    za.FirstDemandSidePtr = zi.FirstDemandSidePtr;
                                    za.LastDemandSidePtr = zi.LastDemandSidePtr;
                                }
                            }
                        }
                        for (int i = ArrayCount + 1; i <= EquipNum; ++i) { // Zero the now-unused entries
                            auto &zi(ZoneCompToPlant(i));
                            zi.ZoneEqListNum = 0;
                            zi.ZoneEqCompNum = 0;
                            zi.PlantLoopType = 0;
                            zi.PlantLoopNum = 0;
                            zi.PlantLoopBranch = 0;
                            zi.PlantLoopComp = 0;
                            zi.FirstDemandSidePtr = 0;
                            zi.LastDemandSidePtr = 0;
                        }
                    }

                    if (SubEquipNum > 0) {
                        ArrayCount = 0;
                        for (int i = 1; i <= SubEquipNum; ++i) {
                            auto const &zi(ZoneSubCompToPlant(i));
                            bool duplicate(false);
                            for (int j = 1; j <= ArrayCount; ++j) {
                                auto const &zj(ZoneSubCompToPlant(j));
                                if ((zi.ZoneEqListNum == zj.ZoneEqListNum) && (zi.ZoneEqCompNum == zj.ZoneEqCompNum) &&
                                    (zi.ZoneEqSubCompNum == zj.ZoneEqSubCompNum)) { // Duplicate
                                    duplicate = true;
                                    break;
                                }
                            }
                            if (!duplicate) {
                                ++ArrayCount;
                                if (i > ArrayCount) { // Copy to lower position
                                    auto &za(ZoneSubCompToPlant(ArrayCount));
                                    za.ZoneEqListNum = zi.ZoneEqListNum;
                                    za.ZoneEqCompNum = zi.ZoneEqCompNum;
                                    za.ZoneEqSubCompNum = zi.ZoneEqSubCompNum;
                                    za.PlantLoopType = zi.PlantLoopType;
                                    za.PlantLoopNum = zi.PlantLoopNum;
                                    za.PlantLoopBranch = zi.PlantLoopBranch;
                                    za.PlantLoopComp = zi.PlantLoopComp;
                                    za.FirstDemandSidePtr = zi.FirstDemandSidePtr;
                                    za.LastDemandSidePtr = zi.LastDemandSidePtr;
                                }
                            }
                        }
                        for (int i = ArrayCount + 1; i <= SubEquipNum; ++i) { // Zero the now-unused entries
                            auto &zi(ZoneSubCompToPlant(i));
                            zi.ZoneEqListNum = 0;
                            zi.ZoneEqCompNum = 0;
                            zi.ZoneEqSubCompNum = 0;
                            zi.PlantLoopType = 0;
                            zi.PlantLoopNum = 0;
                            zi.PlantLoopBranch = 0;
                            zi.PlantLoopComp = 0;
                            zi.FirstDemandSidePtr = 0;
                            zi.LastDemandSidePtr = 0;
                        }
                    }

                    if (SubSubEquipNum > 0) {
                        ArrayCount = 0;
                        for (int i = 1; i <= SubSubEquipNum; ++i) {
                            auto const &zi(ZoneSubSubCompToPlant(i));
                            bool duplicate(false);
                            for (int j = 1; j <= ArrayCount; ++j) {
                                auto const &zj(ZoneSubSubCompToPlant(j));
                                if ((zi.ZoneEqListNum == zj.ZoneEqListNum) && (zi.ZoneEqCompNum == zj.ZoneEqCompNum) &&
                                    (zi.ZoneEqSubCompNum == zj.ZoneEqSubCompNum) && (zi.ZoneEqSubSubCompNum == zj.ZoneEqSubSubCompNum)) { // Duplicate
                                    duplicate = true;
                                    break;
                                }
                            }
                            if (!duplicate) {
                                ++ArrayCount;
                                if (i > ArrayCount) { // Copy to lower position
                                    auto &za(ZoneSubSubCompToPlant(ArrayCount));
                                    za.ZoneEqListNum = zi.ZoneEqListNum;
                                    za.ZoneEqCompNum = zi.ZoneEqCompNum;
                                    za.ZoneEqSubCompNum = zi.ZoneEqSubCompNum;
                                    za.ZoneEqSubSubCompNum = zi.ZoneEqSubSubCompNum;
                                    za.PlantLoopType = zi.PlantLoopType;
                                    za.PlantLoopNum = zi.PlantLoopNum;
                                    za.PlantLoopBranch = zi.PlantLoopBranch;
                                    za.PlantLoopComp = zi.PlantLoopComp;
                                    za.FirstDemandSidePtr = zi.FirstDemandSidePtr;
                                    za.LastDemandSidePtr = zi.LastDemandSidePtr;
                                }
                            }
                        }
                        for (int i = ArrayCount + 1; i <= SubSubEquipNum; ++i) { // Zero the now-unused entries
                            auto &zi(ZoneSubSubCompToPlant(i));
                            zi.ZoneEqListNum = 0;
                            zi.ZoneEqCompNum = 0;
                            zi.ZoneEqSubCompNum = 0;
                            zi.ZoneEqSubSubCompNum = 0;
                            zi.PlantLoopType = 0;
                            zi.PlantLoopNum = 0;
                            zi.PlantLoopBranch = 0;
                            zi.PlantLoopComp = 0;
                            zi.FirstDemandSidePtr = 0;
                            zi.LastDemandSidePtr = 0;
                        }
                    }

                    if (CompNum > 0) {
                        ArrayCount = 0;
                        for (int i = 1; i <= CompNum; ++i) {
                            auto const &ai(AirSysCompToPlant(i));
                            bool duplicate(false);
                            for (int j = 1; j <= ArrayCount; ++j) {
                                auto const &aj(AirSysCompToPlant(j));
                                if ((ai.AirLoopNum == aj.AirLoopNum) && (ai.AirLoopBranch == aj.AirLoopBranch) &&
                                    (ai.AirLoopComp == aj.AirLoopComp)) { // Duplicate
                                    duplicate = true;
                                    break;
                                }
                            }
                            if (!duplicate) {
                                ++ArrayCount;
                                if (i > ArrayCount) { // Copy to lower position
                                    auto &aa(AirSysCompToPlant(ArrayCount));
                                    aa.AirLoopNum = ai.AirLoopNum;
                                    aa.AirLoopBranch = ai.AirLoopBranch;
                                    aa.AirLoopComp = ai.AirLoopComp;
                                    aa.PlantLoopType = ai.PlantLoopType;
                                    aa.PlantLoopNum = ai.PlantLoopNum;
                                    aa.PlantLoopBranch = ai.PlantLoopBranch;
                                    aa.PlantLoopComp = ai.PlantLoopComp;
                                    aa.FirstDemandSidePtr = ai.FirstDemandSidePtr;
                                    aa.LastDemandSidePtr = ai.LastDemandSidePtr;
                                }
                            }
                        }
                        for (int i = ArrayCount + 1; i <= CompNum; ++i) { // Zero the now-unused entries
                            auto &ai(AirSysCompToPlant(i));
                            ai.AirLoopNum = 0;
                            ai.AirLoopBranch = 0;
                            ai.AirLoopComp = 0;
                            ai.PlantLoopType = 0;
                            ai.PlantLoopNum = 0;
                            ai.PlantLoopBranch = 0;
                            ai.PlantLoopComp = 0;
                            ai.FirstDemandSidePtr = 0;
                            ai.LastDemandSidePtr = 0;
                        }
                    }

                    if (SubCompNum > 0) {
                        ArrayCount = 0;
                        for (int i = 1; i <= SubCompNum; ++i) {
                            auto const &ai(AirSysSubCompToPlant(i));
                            bool duplicate(false);
                            for (int j = 1; j <= ArrayCount; ++j) {
                                auto const &aj(AirSysSubCompToPlant(j));
                                if ((ai.AirLoopNum == aj.AirLoopNum) && (ai.AirLoopBranch == aj.AirLoopBranch) &&
                                    (ai.AirLoopComp == aj.AirLoopComp) && (ai.AirLoopSubComp == aj.AirLoopSubComp)) { // Duplicate
                                    duplicate = true;
                                    break;
                                }
                            }
                            if (!duplicate) {
                                ++ArrayCount;
                                if (i > ArrayCount) { // Copy to lower position
                                    auto &aa(AirSysSubCompToPlant(ArrayCount));
                                    aa.AirLoopNum = ai.AirLoopNum;
                                    aa.AirLoopBranch = ai.AirLoopBranch;
                                    aa.AirLoopComp = ai.AirLoopComp;
                                    aa.AirLoopSubComp = ai.AirLoopSubComp;
                                    aa.PlantLoopType = ai.PlantLoopType;
                                    aa.PlantLoopNum = ai.PlantLoopNum;
                                    aa.PlantLoopBranch = ai.PlantLoopBranch;
                                    aa.PlantLoopComp = ai.PlantLoopComp;
                                    aa.FirstDemandSidePtr = ai.FirstDemandSidePtr;
                                    aa.LastDemandSidePtr = ai.LastDemandSidePtr;
                                }
                            }
                        }
                        for (int i = ArrayCount + 1; i <= SubCompNum; ++i) { // Zero the now-unused entries
                            auto &ai(AirSysSubCompToPlant(i));
                            ai.AirLoopNum = 0;
                            ai.AirLoopBranch = 0;
                            ai.AirLoopComp = 0;
                            ai.AirLoopSubComp = 0;
                            ai.PlantLoopType = 0;
                            ai.PlantLoopNum = 0;
                            ai.PlantLoopBranch = 0;
                            ai.PlantLoopComp = 0;
                            ai.FirstDemandSidePtr = 0;
                            ai.LastDemandSidePtr = 0;
                        }
                    }

                    if (SubSubCompNum > 0) {
                        ArrayCount = 0;
                        for (int i = 1; i <= SubCompNum; ++i) {
                            auto const &ai(AirSysSubSubCompToPlant(i));
                            bool duplicate(false);
                            for (int j = 1; j <= ArrayCount; ++j) {
                                auto const &aj(AirSysSubSubCompToPlant(j));
                                if ((ai.AirLoopNum == aj.AirLoopNum) && (ai.AirLoopBranch == aj.AirLoopBranch) &&
                                    (ai.AirLoopComp == aj.AirLoopComp) && (ai.AirLoopSubComp == aj.AirLoopSubComp) &&
                                    (ai.AirLoopSubSubComp == aj.AirLoopSubSubComp)) { // Duplicate
                                    duplicate = true;
                                    break;
                                }
                            }
                            if (!duplicate) {
                                ++ArrayCount;
                                if (i > ArrayCount) { // Copy to lower position
                                    auto &aa(AirSysSubSubCompToPlant(ArrayCount));
                                    aa.AirLoopNum = ai.AirLoopNum;
                                    aa.AirLoopBranch = ai.AirLoopBranch;
                                    aa.AirLoopComp = ai.AirLoopComp;
                                    aa.AirLoopSubComp = ai.AirLoopSubComp;
                                    aa.AirLoopSubSubComp = ai.AirLoopSubSubComp;
                                    aa.PlantLoopType = ai.PlantLoopType;
                                    aa.PlantLoopNum = ai.PlantLoopNum;
                                    aa.PlantLoopBranch = ai.PlantLoopBranch;
                                    aa.PlantLoopComp = ai.PlantLoopComp;
                                    aa.FirstDemandSidePtr = ai.FirstDemandSidePtr;
                                    aa.LastDemandSidePtr = ai.LastDemandSidePtr;
                                }
                            }
                        }
                        for (int i = ArrayCount + 1; i <= SubCompNum; ++i) { // Zero the now-unused entries
                            auto &ai(AirSysSubSubCompToPlant(i));
                            ai.AirLoopNum = 0;
                            ai.AirLoopBranch = 0;
                            ai.AirLoopComp = 0;
                            ai.AirLoopSubComp = 0;
                            ai.AirLoopSubSubComp = 0;
                            ai.PlantLoopType = 0;
                            ai.PlantLoopNum = 0;
                            ai.PlantLoopBranch = 0;
                            ai.PlantLoopComp = 0;
                            ai.FirstDemandSidePtr = 0;
                            ai.LastDemandSidePtr = 0;
                        }
                    }

                    // 2. Find Supply Side loop for every demand side component
                    // The demand side components only need to know what supply side loop
                    // they are connected to.  The input and plant data structure will
                    // force the loop numbers to be the same.

                    // 3. Find Demand Side Component Corresponding to Supply Side Component
                    for (PlantLoopNum = 1; PlantLoopNum <= NumPlantLoops; ++PlantLoopNum) {
                        for (BranchNum = 1; BranchNum <= VentRepPlantSupplySide(PlantLoopNum).TotalBranches; ++BranchNum) {
                            for (CompNum = 1; CompNum <= VentRepPlantSupplySide(PlantLoopNum).Branch(BranchNum).TotalComponents; ++CompNum) {
                                {
                                    auto &thisVentRepComp(VentRepPlantSupplySide(PlantLoopNum).Branch(BranchNum).Comp(CompNum));
                                    CompType = thisVentRepComp.TypeOf;
                                    CompName = thisVentRepComp.Name;
                                    FindDemandSideMatch(CompType, CompName, MatchFound, MatchLoopType, MatchLoop, MatchBranch, MatchComp);
                                    thisVentRepComp.ConnectPlant.LoopType = MatchLoopType;
                                    thisVentRepComp.ConnectPlant.LoopNum = MatchLoop;
                                    thisVentRepComp.ConnectPlant.BranchNum = MatchBranch;
                                    thisVentRepComp.ConnectPlant.CompNum = MatchComp;
                                }
                            }
                        }
                    }

                    for (PlantLoopNum = 1; PlantLoopNum <= NumCondLoops; ++PlantLoopNum) {
                        for (BranchNum = 1; BranchNum <= VentRepCondSupplySide(PlantLoopNum).TotalBranches; ++BranchNum) {
                            for (CompNum = 1; CompNum <= VentRepCondSupplySide(PlantLoopNum).Branch(BranchNum).TotalComponents; ++CompNum) {
                                {
                                    auto &thisVentRepComp(VentRepCondSupplySide(PlantLoopNum).Branch(BranchNum).Comp(CompNum));
                                    CompType = thisVentRepComp.TypeOf;
                                    CompName = thisVentRepComp.Name;
                                    FindDemandSideMatch(CompType, CompName, MatchFound, MatchLoopType, MatchLoop, MatchBranch, MatchComp);
                                    thisVentRepComp.ConnectPlant.LoopType = MatchLoopType;
                                    thisVentRepComp.ConnectPlant.LoopNum = MatchLoop;
                                    thisVentRepComp.ConnectPlant.BranchNum = MatchBranch;
                                    thisVentRepComp.ConnectPlant.CompNum = MatchComp;
                                }
                            }
                        }
                    }
                }
            } // Controlled Zone Loop

            // 4.  Now Load all of the plant supply/demand side connections in a single array with pointers from the
            //    connection arrays (ZoneCompToPlant, ZoneSubCompToPlant, ZoneSubSubCompToPlant, AirSysCompToPlant, etc.)
            if (allocated(ZoneCompToPlant)) {
                NumZoneConnectComps = isize(ZoneCompToPlant);
            } else {
                NumZoneConnectComps = 0;
            }
            if (allocated(ZoneSubCompToPlant)) {
                NumZoneConnectSubComps = isize(ZoneSubCompToPlant);
            } else {
                NumZoneConnectSubComps = 0;
            }
            if (allocated(ZoneSubSubCompToPlant)) {
                NumZoneConnectSubSubComps = isize(ZoneSubSubCompToPlant);
            } else {
                NumZoneConnectSubSubComps = 0;
            }
            if (allocated(AirSysCompToPlant)) {
                NumAirSysConnectComps = isize(AirSysCompToPlant);
            } else {
                NumAirSysConnectComps = 0;
            }
            if (allocated(AirSysSubCompToPlant)) {
                NumAirSysConnectSubComps = isize(AirSysSubCompToPlant);
            } else {
                NumAirSysConnectSubComps = 0;
            }
            if (allocated(AirSysSubSubCompToPlant)) {
                NumAirSysConnectSubSubComps = isize(AirSysSubSubCompToPlant);
            } else {
                NumAirSysConnectSubSubComps = 0;
            }
            OneTimeFlag = false;

            ArrayCount = 0;
            for (CompNum = 1; CompNum <= NumZoneConnectComps; ++CompNum) {
                LoopType = ZoneCompToPlant(CompNum).PlantLoopType;
                LoopNum = ZoneCompToPlant(CompNum).PlantLoopNum;
                FirstIndex = ArrayCount + 1;
                LoopCount = 1;

                if (LoopType > 0 && LoopNum > 0) {
                    FindFirstLastPtr(state, LoopType, LoopNum, ArrayCount, LoopCount, ConnectionFlag);
                } else {
                    ConnectionFlag = false;
                }

                LastIndex = ArrayCount;
                if (FirstIndex > LastIndex) FirstIndex = LastIndex;
                if (ConnectionFlag) {
                    ZoneCompToPlant(CompNum).FirstDemandSidePtr = FirstIndex;
                    ZoneCompToPlant(CompNum).LastDemandSidePtr = LastIndex;
                }
            }

            for (SubCompNum = 1; SubCompNum <= NumZoneConnectSubComps; ++SubCompNum) {
                LoopType = ZoneSubCompToPlant(SubCompNum).PlantLoopType;
                LoopNum = ZoneSubCompToPlant(SubCompNum).PlantLoopNum;
                FirstIndex = ArrayCount + 1;
                LoopCount = 1;

                if (LoopType > 0 && LoopNum > 0) {
                    FindFirstLastPtr(state, LoopType, LoopNum, ArrayCount, LoopCount, ConnectionFlag);
                } else {
                    ConnectionFlag = false;
                }

                LastIndex = ArrayCount;
                if (FirstIndex > LastIndex) FirstIndex = LastIndex;
                if (ConnectionFlag) {
                    ZoneSubCompToPlant(SubCompNum).FirstDemandSidePtr = FirstIndex;
                    ZoneSubCompToPlant(SubCompNum).LastDemandSidePtr = LastIndex;
                }
            }

            for (SubSubCompNum = 1; SubSubCompNum <= NumZoneConnectSubSubComps; ++SubSubCompNum) {
                LoopType = ZoneSubSubCompToPlant(SubSubCompNum).PlantLoopType;
                LoopNum = ZoneSubSubCompToPlant(SubSubCompNum).PlantLoopNum;
                FirstIndex = ArrayCount + 1;
                LoopCount = 1;

                if (LoopType > 0 && LoopNum > 0) {
                    FindFirstLastPtr(state, LoopType, LoopNum, ArrayCount, LoopCount, ConnectionFlag);
                } else {
                    ConnectionFlag = false;
                }

                LastIndex = ArrayCount;
                if (FirstIndex > LastIndex) FirstIndex = LastIndex;
                if (ConnectionFlag) {
                    ZoneSubSubCompToPlant(SubSubCompNum).FirstDemandSidePtr = FirstIndex;
                    ZoneSubSubCompToPlant(SubSubCompNum).LastDemandSidePtr = LastIndex;
                }
            }
            for (CompNum = 1; CompNum <= NumAirSysConnectComps; ++CompNum) {
                LoopType = AirSysCompToPlant(CompNum).PlantLoopType;
                LoopNum = AirSysCompToPlant(CompNum).PlantLoopNum;
                FirstIndex = ArrayCount + 1;
                LoopCount = 1;

                if (LoopType > 0 && LoopNum > 0) {
                    FindFirstLastPtr(state, LoopType, LoopNum, ArrayCount, LoopCount, ConnectionFlag);
                } else {
                    ConnectionFlag = false;
                }

                LastIndex = ArrayCount;
                if (FirstIndex > LastIndex) FirstIndex = LastIndex;
                if (ConnectionFlag) {
                    AirSysCompToPlant(CompNum).FirstDemandSidePtr = FirstIndex;
                    AirSysCompToPlant(CompNum).LastDemandSidePtr = LastIndex;
                }
            }

            for (SubCompNum = 1; SubCompNum <= NumAirSysConnectSubComps; ++SubCompNum) {
                LoopType = AirSysSubCompToPlant(SubCompNum).PlantLoopType;
                LoopNum = AirSysSubCompToPlant(SubCompNum).PlantLoopNum;
                FirstIndex = ArrayCount + 1;
                LoopCount = 1;

                if (LoopType > 0 && LoopNum > 0) {
                    FindFirstLastPtr(state, LoopType, LoopNum, ArrayCount, LoopCount, ConnectionFlag);
                } else {
                    ConnectionFlag = false;
                }

                LastIndex = ArrayCount;
                if (FirstIndex > LastIndex) FirstIndex = LastIndex;
                if (ConnectionFlag) {
                    AirSysSubCompToPlant(SubCompNum).FirstDemandSidePtr = FirstIndex;
                    AirSysSubCompToPlant(SubCompNum).LastDemandSidePtr = LastIndex;
                }
            }

            for (SubSubCompNum = 1; SubSubCompNum <= NumAirSysConnectSubSubComps; ++SubSubCompNum) {
                LoopType = AirSysSubSubCompToPlant(SubSubCompNum).PlantLoopType;
                LoopNum = AirSysSubSubCompToPlant(SubSubCompNum).PlantLoopNum;
                FirstIndex = ArrayCount + 1;
                LoopCount = 1;

                if (LoopType > 0 && LoopNum > 0) {
                    FindFirstLastPtr(state, LoopType, LoopNum, ArrayCount, LoopCount, ConnectionFlag);
                } else {
                    ConnectionFlag = false;
                }

                LastIndex = ArrayCount;
                if (FirstIndex > LastIndex) FirstIndex = LastIndex;
                if (ConnectionFlag) {
                    AirSysSubSubCompToPlant(SubSubCompNum).FirstDemandSidePtr = FirstIndex;
                    AirSysSubSubCompToPlant(SubSubCompNum).LastDemandSidePtr = LastIndex;
                }
            }

            OneTimeFlag = false;
        }

        // On every iteration, load the air loop energy data
        for (AirLoopNum = 1; AirLoopNum <= NumPrimaryAirSys; ++AirLoopNum) {
            auto &pas = PrimaryAirSystem(AirLoopNum);
            for (BranchNum = 1; BranchNum <= pas.NumBranches; ++BranchNum) {
                auto &pasBranch = pas.Branch(BranchNum);
                for (CompNum = 1; CompNum <= pasBranch.TotalComponents; ++CompNum) {
                    auto &pasBranchComp = pasBranch.Comp(CompNum);
                    for (VarNum = 1; VarNum <= pasBranchComp.NumMeteredVars; ++VarNum) {
                        auto &pasBranchCompMeter = pasBranchComp.MeteredVar(VarNum);
                        VarType = pasBranchCompMeter.ReportVarType;
                        VarIndex = pasBranchCompMeter.ReportVarIndex;
                        pasBranchCompMeter.CurMeterReading = GetInternalVariableValue(VarType, VarIndex);
                    }
                    for (SubCompNum = 1; SubCompNum <= pasBranchComp.NumSubComps; ++SubCompNum) {
                        auto &pasBranchSubComp = pasBranchComp.SubComp(SubCompNum);
                        for (VarNum = 1; VarNum <= pasBranchSubComp.NumMeteredVars; ++VarNum) {
                            auto &pasBranchSubCompMeter = pasBranchSubComp.MeteredVar(VarNum);
                            VarType = pasBranchSubCompMeter.ReportVarType;
                            VarIndex = pasBranchSubCompMeter.ReportVarIndex;
                            pasBranchSubCompMeter.CurMeterReading = GetInternalVariableValue(VarType, VarIndex);
                        }
                        for (SubSubCompNum = 1; SubSubCompNum <= pasBranchSubComp.NumSubSubComps; ++SubSubCompNum) {
                            auto &pasBranchSubSubComp = pasBranchSubComp.SubSubComp(SubSubCompNum);
                            for (VarNum = 1; VarNum <= pasBranchSubSubComp.NumMeteredVars; ++VarNum) {
                                auto &pasBranchSubSubCompMeter = pasBranchSubSubComp.MeteredVar(VarNum);
                                VarType = pasBranchSubSubCompMeter.ReportVarType;
                                VarIndex = pasBranchSubSubCompMeter.ReportVarIndex;
                                pasBranchSubSubCompMeter.CurMeterReading = GetInternalVariableValue(VarType, VarIndex);
                            }
                        }
                    }
                }
            }
        }

        // On every iteration, load the zone equipment energy data
        for (ListNum = 1; ListNum <= NumOfZones; ++ListNum) {
            if (!ZoneEquipConfig(ListNum).IsControlled) continue;
            auto &zel = ZoneEquipList(ListNum);
            for (CompNum = 1; CompNum <= zel.NumOfEquipTypes; ++CompNum) {
                auto &zelEquipData = zel.EquipData(CompNum);
                for (VarNum = 1; VarNum <= zelEquipData.NumMeteredVars; ++VarNum) {
                    auto &zelEquipDataMeter = zelEquipData.MeteredVar(VarNum);
                    VarType = zelEquipDataMeter.ReportVarType;
                    VarIndex = zelEquipDataMeter.ReportVarIndex;
                    zelEquipDataMeter.CurMeterReading = GetInternalVariableValue(VarType, VarIndex);
                }
                for (SubCompNum = 1; SubCompNum <= zelEquipData.NumSubEquip; ++SubCompNum) {
                    auto &zelSubEquipData = zelEquipData.SubEquipData(SubCompNum);
                    for (VarNum = 1; VarNum <= zelSubEquipData.NumMeteredVars; ++VarNum) {
                        auto &zelSubEquipDataMeter = zelSubEquipData.MeteredVar(VarNum);
                        VarType = zelSubEquipDataMeter.ReportVarType;
                        VarIndex = zelSubEquipDataMeter.ReportVarIndex;
                        zelSubEquipDataMeter.CurMeterReading = GetInternalVariableValue(VarType, VarIndex);
                    }
                    for (SubSubCompNum = 1; SubSubCompNum <= zelSubEquipData.NumSubSubEquip; ++SubSubCompNum) {
                        auto &zelSubSubEquipData = zelSubEquipData.SubSubEquipData(SubSubCompNum);
                        for (VarNum = 1; VarNum <= zelSubSubEquipData.NumMeteredVars; ++VarNum) {
                            auto &zelSubSubEquipDataMeter = zelSubSubEquipData.MeteredVar(VarNum);
                            VarType = zelSubSubEquipDataMeter.ReportVarType;
                            VarIndex = zelSubSubEquipDataMeter.ReportVarIndex;
                            zelSubSubEquipDataMeter.CurMeterReading = GetInternalVariableValue(VarType, VarIndex); // Sankar Corrected zone array
                        }
                    }
                }
            }
        }

        // On every iteration, load the Plant Supply Side Data and load the Plant Demand Side Data
        for (PlantLoopNum = 1; PlantLoopNum <= NumPlantLoops; ++PlantLoopNum) {
            auto &vrpss = VentRepPlantSupplySide(PlantLoopNum);
            for (BranchNum = 1; BranchNum <= vrpss.TotalBranches; ++BranchNum) {
                auto &vrpssBranch = vrpss.Branch(BranchNum);
                for (CompNum = 1; CompNum <= vrpssBranch.TotalComponents; ++CompNum) {
                    auto &vrpssBranchComp = vrpssBranch.Comp(CompNum);
                    for (VarNum = 1; VarNum <= vrpssBranchComp.NumMeteredVars; ++VarNum) {
                        auto &vrpssBranchCompMeter = vrpssBranchComp.MeteredVar(VarNum);
                        VarType = vrpssBranchCompMeter.ReportVarType;
                        VarIndex = vrpssBranchCompMeter.ReportVarIndex;
                        vrpssBranchCompMeter.CurMeterReading = GetInternalVariableValue(VarType, VarIndex);
                    }
                }
            }
            auto &vrpds = VentRepPlantDemandSide(PlantLoopNum);
            for (BranchNum = 1; BranchNum <= vrpds.TotalBranches; ++BranchNum) {
                auto &vrpdsBranch = vrpds.Branch(BranchNum);
                for (CompNum = 1; CompNum <= vrpdsBranch.TotalComponents; ++CompNum) {
                    auto &vrpdsBranchComp = vrpdsBranch.Comp(CompNum);
                    for (VarNum = 1; VarNum <= vrpdsBranchComp.NumMeteredVars; ++VarNum) {
                        auto &vrpdsBranchCompMeter = vrpdsBranchComp.MeteredVar(VarNum);
                        VarType = vrpdsBranchCompMeter.ReportVarType;
                        VarIndex = vrpdsBranchCompMeter.ReportVarIndex;
                        vrpdsBranchCompMeter.CurMeterReading = GetInternalVariableValue(VarType, VarIndex);
                    }
                }
            }
        }

        // On every iteration, load the Condenser Supply Side Data and load the Condenser Demand Side Data
        for (PlantLoopNum = 1; PlantLoopNum <= NumCondLoops; ++PlantLoopNum) {
            auto &vrcss = VentRepCondSupplySide(PlantLoopNum);
            for (BranchNum = 1; BranchNum <= vrcss.TotalBranches; ++BranchNum) {
                auto &vrcssBranch = vrcss.Branch(BranchNum);
                for (CompNum = 1; CompNum <= vrcssBranch.TotalComponents; ++CompNum) {
                    auto &vrcssBranchComp = vrcssBranch.Comp(CompNum);
                    for (VarNum = 1; VarNum <= vrcssBranchComp.NumMeteredVars; ++VarNum) {
                        auto &vrcssBranchCompMeter = vrcssBranchComp.MeteredVar(VarNum);
                        VarType = vrcssBranchCompMeter.ReportVarType;
                        VarIndex = vrcssBranchCompMeter.ReportVarIndex;
                        vrcssBranchCompMeter.CurMeterReading = GetInternalVariableValue(VarType, VarIndex);
                    }
                }
            }
            auto &vrcds = VentRepCondSupplySide(PlantLoopNum);
            for (BranchNum = 1; BranchNum <= vrcds.TotalBranches; ++BranchNum) {
                auto &vrcdsBranch = vrcds.Branch(BranchNum);
                for (CompNum = 1; CompNum <= vrcdsBranch.TotalComponents; ++CompNum) {
                    auto &vrcdsBranchComp = vrcdsBranch.Comp(CompNum);
                    for (VarNum = 1; VarNum <= vrcdsBranchComp.NumMeteredVars; ++VarNum) {
                        auto &vrcdsBranchCompMeter = vrcdsBranchComp.MeteredVar(VarNum);
                        VarType = vrcdsBranchCompMeter.ReportVarType;
                        VarIndex = vrcdsBranchCompMeter.ReportVarIndex;
                        vrcdsBranchCompMeter.CurMeterReading = GetInternalVariableValue(VarType, VarIndex);
                    }
                }
            }
        }

        // initialize energy report variables
    }

    void FindFirstLastPtr(EnergyPlusData &state, int &LoopType, int &LoopNum, int &ArrayCount, int &LoopCount, bool &ConnectionFlag)
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Dan Fisher
        //       DATE WRITTEN   July 2005
        //       MODIFIED
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Initializes the energy components of the data structures

        // METHODOLOGY EMPLOYED:
        // Once all compsets have been established (second iteration) find all components
        // subcomponents, etc.

        // REFERENCES:
        // na

        // USE STATEMENTS:

        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS

        // SUBROUTINE PARAMETER DEFINITIONS:

        // INTERFACE BLOCK SPECIFICATIONS
        // na

        // DERIVED TYPE DEFINITIONS

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:

        int BranchNum;
        int Idx;
        int DemandSideLoopNum;
        int DemandSideBranchNum;
        int DemandSideCompNum;
        int SupplySideCompNum;
        int DemandSideLoopType;
        static bool OneTimeFlag(true); // Flag set to make sure you initialize reports one time
        bool found;
        //		int countloop;

        struct IdentifyLoop
        {
            // Members
            int LoopNum;
            int LoopType;

            // Default Constructor
            IdentifyLoop() : LoopNum(0), LoopType(0)
            {
            }
        };

        // Object Data
        static Array1D<IdentifyLoop> LoopStack;

        return; // Autodesk:? Is this routine now an intentional NOOP?

        if (OneTimeFlag) {
            LoopStack.allocate(MaxLoopArraySize);
            DemandSideConnect.allocate(MaxCompArraySize);

            OneTimeFlag = false;
        }
        for (auto &e : LoopStack) {
            e.LoopNum = 0;
            e.LoopType = 0;
        }

        ConnectionFlag = false;
        //    countloop=0
        //    write(outputfiledebug,*) '1228=lt,lc,lnum,cflag,arrcnt',looptype,loopcount,LoopNum,connectionflag,arraycount

        while (LoopCount > 0) {
            //        write(outputfiledebug,*) '1231==lt,lc,lnum,cflag,arrcnt',looptype,loopcount,LoopNum,connectionflag,arraycount
            //        write(outputfiledebug,*) 'loop=plname',TRIM(plantloop(LoopNum)%name)
            --LoopCount;
            //        countloop=countloop+1
            //        if (countloop > 100) exit
            if (LoopType == 1) {
                for (BranchNum = 1; BranchNum <= VentRepPlantSupplySide(LoopNum).TotalBranches; ++BranchNum) {
                    for (SupplySideCompNum = 1; SupplySideCompNum <= VentRepPlantSupplySide(LoopNum).Branch(BranchNum).TotalComponents;
                         ++SupplySideCompNum) {
                        {
                            auto &thisVentRepComp(VentRepPlantSupplySide(LoopNum).Branch(BranchNum).Comp(SupplySideCompNum));
                            DemandSideLoopType = thisVentRepComp.ConnectPlant.LoopType;
                            DemandSideLoopNum = thisVentRepComp.ConnectPlant.LoopNum;
                            DemandSideBranchNum = thisVentRepComp.ConnectPlant.BranchNum;
                            DemandSideCompNum = thisVentRepComp.ConnectPlant.CompNum;
                        }
                        // If the connection is valid load the connection array
                        if (DemandSideLoopType == 1 || DemandSideLoopType == 2) {
                            ConnectionFlag = true;
                            ++ArrayCount;
                            if (ArrayCount > MaxCompArraySize) {
                                DemandSideConnect.redimension(MaxCompArraySize += 100);
                            }
                            DemandSideConnect(ArrayCount).LoopType = DemandSideLoopType;
                            DemandSideConnect(ArrayCount).LoopNum = DemandSideLoopNum;
                            DemandSideConnect(ArrayCount).BranchNum = DemandSideBranchNum;
                            DemandSideConnect(ArrayCount).CompNum = DemandSideCompNum;

                            found = false;
                            print(state.files.debug, "1271=lstacksize {}\n", size(LoopStack));
                            for (Idx = 1; Idx <= isize(LoopStack); ++Idx) {
                                if (DemandSideLoopNum == LoopStack(Idx).LoopNum && DemandSideLoopType == LoopStack(Idx).LoopType) {
                                    found = true;
                                    break;
                                }
                            }
                            if (!found) {
                                ++LoopCount;
                                //       write(outputfiledebug,*) '1280=lc,mxsize',loopcount,maxlooparraysize
                                //       write(outputfiledebug,*) '1281=dsloopnum,dslooptype',DemandSideLoopNum,DemandSideLoopType
                                if (LoopCount > MaxLoopArraySize) {
                                    LoopStack.redimension(MaxLoopArraySize += 100);
                                }
                                //               write(outputfiledebug,*)
                                //               '1294=lcnt,dsloopnum,dslooptype',loopcount,DemandSideLoopNum,DemandSideLoopType
                                LoopStack(LoopCount).LoopNum = DemandSideLoopNum;
                                LoopStack(LoopCount).LoopType = DemandSideLoopType;
                            }
                        }
                    }
                }
            } else if (LoopType == 2) {
                for (BranchNum = 1; BranchNum <= VentRepCondSupplySide(LoopNum).TotalBranches; ++BranchNum) {
                    for (SupplySideCompNum = 1; SupplySideCompNum <= VentRepCondSupplySide(LoopNum).Branch(BranchNum).TotalComponents;
                         ++SupplySideCompNum) {
                        {
                            auto &thisVentRepComp(VentRepCondSupplySide(LoopNum).Branch(BranchNum).Comp(SupplySideCompNum));
                            DemandSideLoopType = thisVentRepComp.ConnectPlant.LoopType;
                            DemandSideLoopNum = thisVentRepComp.ConnectPlant.LoopNum;
                            DemandSideBranchNum = thisVentRepComp.ConnectPlant.BranchNum;
                            DemandSideCompNum = thisVentRepComp.ConnectPlant.CompNum;
                        }
                        // If the connection is valid load the connection array
                        if (DemandSideLoopType == 1 || DemandSideLoopType == 2) {
                            ConnectionFlag = true;
                            ++ArrayCount;
                            if (ArrayCount > MaxCompArraySize) {
                                DemandSideConnect.redimension(MaxCompArraySize += 100);
                            }
                            DemandSideConnect(ArrayCount).LoopType = DemandSideLoopType;
                            DemandSideConnect(ArrayCount).LoopNum = DemandSideLoopNum;
                            DemandSideConnect(ArrayCount).BranchNum = DemandSideBranchNum;
                            DemandSideConnect(ArrayCount).CompNum = DemandSideCompNum;

                            found = false;
                            for (Idx = 1; Idx <= isize(LoopStack); ++Idx) {
                                if (DemandSideLoopNum == LoopStack(Idx).LoopNum && DemandSideLoopType == LoopStack(Idx).LoopType) {
                                    found = true;
                                    break;
                                }
                            }
                            if (!found) {
                                ++LoopCount;
                                //       write(outputfiledebug,*) '1341=lcnt,arrsize',loopcount,maxlooparraysize
                                //       write(outputfiledebug,*) '1342=lsloopnum,dslooptype',DemandSideLoopNum,DemandSideLoopType
                                if (LoopCount > MaxLoopArraySize) {
                                    LoopStack.redimension(MaxLoopArraySize += 100);
                                }
                                LoopStack(LoopCount).LoopNum = DemandSideLoopNum;
                                LoopStack(LoopCount).LoopType = DemandSideLoopType;
                            }
                        }
                    }
                }
            } else {
                print(state.files.debug, "{}\n", "1361=error");
                // error
            }

            // now unload the LoopNum and LoopType arrays
            if (LoopCount > 0) {
                LoopType = LoopStack(LoopCount).LoopType;
                LoopNum = LoopStack(LoopCount).LoopNum;
            }

        } // While loop
    }

    void UpdateZoneCompPtrArray(int &Idx,
                                int const ListNum,
                                int const AirDistUnitNum,
                                int const PlantLoopType,
                                int const PlantLoop,
                                int const PlantBranch,
                                int const PlantComp)
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Dan Fisher
        //       DATE WRITTEN   June 2005
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Update Zone Component pointers

        // METHODOLOGY EMPLOYED:
        // na

        // REFERENCES:
        // na

        // USE STATEMENTS:
        // na

        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:
        // SUBROUTINE PARAMETER DEFINITIONS:
        // na

        // INTERFACE BLOCK SPECIFICATIONS:
        // na

        static bool OneTimeFlag(true);
        static int ArrayLimit(100);
        static int ArrayCounter(1);

        if (OneTimeFlag) {
            ZoneCompToPlant.allocate(ArrayLimit);
            for (auto &e : ZoneCompToPlant) {
                e.ZoneEqListNum = 0;
                e.ZoneEqCompNum = 0;
                e.PlantLoopType = 0;
                e.PlantLoopNum = 0;
                e.PlantLoopBranch = 0;
                e.PlantLoopComp = 0;
                e.FirstDemandSidePtr = 0;
                e.LastDemandSidePtr = 0;
            }

            OneTimeFlag = false;
        }

        if (ArrayCounter >= ArrayLimit) { // Redimension larger
            int const OldArrayLimit(ArrayLimit);
            ZoneCompToPlant.redimension(ArrayLimit *= 2);
            for (int i = OldArrayLimit + 1; i <= ArrayLimit; ++i) {
                auto &zctp(ZoneCompToPlant(i));
                zctp.ZoneEqListNum = 0;
                zctp.ZoneEqCompNum = 0;
                zctp.PlantLoopType = 0;
                zctp.PlantLoopNum = 0;
                zctp.PlantLoopBranch = 0;
                zctp.PlantLoopComp = 0;
                zctp.FirstDemandSidePtr = 0;
                zctp.LastDemandSidePtr = 0;
            }
        }

        Idx = ArrayCounter;
        auto &zctp(ZoneCompToPlant(Idx));
        zctp.ZoneEqListNum = ListNum;
        zctp.ZoneEqCompNum = AirDistUnitNum;
        zctp.PlantLoopType = PlantLoopType;
        zctp.PlantLoopNum = PlantLoop;
        zctp.PlantLoopBranch = PlantBranch;
        zctp.PlantLoopComp = PlantComp;
        ++ArrayCounter;
    }

    void UpdateZoneSubCompPtrArray(int &Idx,
                                   int const ListNum,
                                   int const AirDistUnitNum,
                                   int const SubCompNum,
                                   int const PlantLoopType,
                                   int const PlantLoop,
                                   int const PlantBranch,
                                   int const PlantComp)
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Dan Fisher
        //       DATE WRITTEN   June 2005
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Update Zone Sub Component Pointer Array

        // METHODOLOGY EMPLOYED:
        // na

        // REFERENCES:
        // na

        // USE STATEMENTS:
        // na

        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:
        // SUBROUTINE PARAMETER DEFINITIONS:
        // na

        // INTERFACE BLOCK SPECIFICATIONS:
        // na

        static bool OneTimeFlag(true);
        static int ArrayLimit(100);
        static int ArrayCounter(1);

        if (OneTimeFlag) {
            ZoneSubCompToPlant.allocate(ArrayLimit);
            for (auto &e : ZoneSubCompToPlant) {
                e.ZoneEqListNum = 0;
                e.ZoneEqCompNum = 0;
                e.ZoneEqSubCompNum = 0;
                e.PlantLoopType = 0;
                e.PlantLoopNum = 0;
                e.PlantLoopBranch = 0;
                e.PlantLoopComp = 0;
                e.FirstDemandSidePtr = 0;
                e.LastDemandSidePtr = 0;
            }

            OneTimeFlag = false;
        }

        if (ArrayCounter >= ArrayLimit) { // Redimension larger
            int const OldArrayLimit(ArrayLimit);
            ZoneSubCompToPlant.redimension(ArrayLimit *= 2);
            for (int i = OldArrayLimit + 1; i <= ArrayLimit; ++i) {
                auto &zctp(ZoneSubCompToPlant(i));
                zctp.ZoneEqListNum = 0;
                zctp.ZoneEqCompNum = 0;
                zctp.ZoneEqSubCompNum = 0;
                zctp.PlantLoopType = 0;
                zctp.PlantLoopNum = 0;
                zctp.PlantLoopBranch = 0;
                zctp.PlantLoopComp = 0;
                zctp.FirstDemandSidePtr = 0;
                zctp.LastDemandSidePtr = 0;
            }
        }

        Idx = ArrayCounter;
        auto &zctp(ZoneSubCompToPlant(Idx));
        zctp.ZoneEqListNum = ListNum;
        zctp.ZoneEqCompNum = AirDistUnitNum;
        zctp.ZoneEqSubCompNum = SubCompNum;
        zctp.PlantLoopType = PlantLoopType;
        zctp.PlantLoopNum = PlantLoop;
        zctp.PlantLoopBranch = PlantBranch;
        zctp.PlantLoopComp = PlantComp;
        ++ArrayCounter;
    }

    void UpdateZoneSubSubCompPtrArray(int &Idx,
                                      int const ListNum,
                                      int const AirDistUnitNum,
                                      int const SubCompNum,
                                      int const SubSubCompNum,
                                      int const PlantLoopType,
                                      int const PlantLoop,
                                      int const PlantBranch,
                                      int const PlantComp)
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Dan Fisher
        //       DATE WRITTEN   June 2005
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Update Zone Sub Component Pointer Array

        // METHODOLOGY EMPLOYED:
        // na

        // REFERENCES:
        // na

        // USE STATEMENTS:
        // na

        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:
        // SUBROUTINE PARAMETER DEFINITIONS:
        // na

        // INTERFACE BLOCK SPECIFICATIONS:
        // na

        static bool OneTimeFlag(true);
        static int ArrayLimit(100);
        static int ArrayCounter(1);

        if (OneTimeFlag) {
            ZoneSubSubCompToPlant.allocate(ArrayLimit);
            for (auto &e : ZoneSubSubCompToPlant) {
                e.ZoneEqListNum = 0;
                e.ZoneEqCompNum = 0;
                e.ZoneEqSubCompNum = 0;
                e.ZoneEqSubSubCompNum = 0;
                e.PlantLoopType = 0;
                e.PlantLoopNum = 0;
                e.PlantLoopBranch = 0;
                e.PlantLoopComp = 0;
                e.FirstDemandSidePtr = 0;
                e.LastDemandSidePtr = 0;
            }

            OneTimeFlag = false;
        }

        if (ArrayCounter >= ArrayLimit) { // Redimension larger
            int const OldArrayLimit(ArrayLimit);
            ZoneSubSubCompToPlant.redimension(ArrayLimit *= 2);
            for (int i = OldArrayLimit + 1; i <= ArrayLimit; ++i) {
                auto &zctp(ZoneSubSubCompToPlant(i));
                zctp.ZoneEqListNum = 0;
                zctp.ZoneEqCompNum = 0;
                zctp.ZoneEqSubCompNum = 0;
                zctp.ZoneEqSubSubCompNum = 0;
                zctp.PlantLoopType = 0;
                zctp.PlantLoopNum = 0;
                zctp.PlantLoopBranch = 0;
                zctp.PlantLoopComp = 0;
                zctp.FirstDemandSidePtr = 0;
                zctp.LastDemandSidePtr = 0;
            }
        }

        Idx = ArrayCounter;
        auto &zctp(ZoneSubSubCompToPlant(Idx));
        zctp.ZoneEqListNum = ListNum;
        zctp.ZoneEqCompNum = AirDistUnitNum;
        zctp.ZoneEqSubCompNum = SubCompNum;
        zctp.ZoneEqSubSubCompNum = SubSubCompNum;
        zctp.PlantLoopType = PlantLoopType;
        zctp.PlantLoopNum = PlantLoop;
        zctp.PlantLoopBranch = PlantBranch;
        zctp.PlantLoopComp = PlantComp;
        ++ArrayCounter;
    }

    void UpdateAirSysCompPtrArray(int &Idx,
                                  int const AirLoopNum,
                                  int const BranchNum,
                                  int const CompNum,
                                  int const PlantLoopType,
                                  int const PlantLoop,
                                  int const PlantBranch,
                                  int const PlantComp)
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Dan Fisher
        //       DATE WRITTEN   June 2005
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Update Air System Component Pointer Array

        // METHODOLOGY EMPLOYED:
        // na

        // REFERENCES:
        // na

        // USE STATEMENTS:
        // na

        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:
        // SUBROUTINE PARAMETER DEFINITIONS:
        // na

        // INTERFACE BLOCK SPECIFICATIONS:
        // na

        static bool OneTimeFlag(true);
        static int ArrayLimit(100);
        static int ArrayCounter(1);

        if (OneTimeFlag) {
            AirSysCompToPlant.allocate(ArrayLimit);
            for (auto &e : AirSysCompToPlant) {
                e.AirLoopNum = 0;
                e.AirLoopBranch = 0;
                e.AirLoopComp = 0;
                e.PlantLoopType = 0;
                e.PlantLoopNum = 0;
                e.PlantLoopBranch = 0;
                e.PlantLoopComp = 0;
                e.FirstDemandSidePtr = 0;
                e.LastDemandSidePtr = 0;
            }

            OneTimeFlag = false;
        }

        if (ArrayCounter >= ArrayLimit) { // Redimension larger
            int const OldArrayLimit(ArrayLimit);
            AirSysCompToPlant.redimension(ArrayLimit *= 2);
            for (int i = OldArrayLimit + 1; i <= ArrayLimit; ++i) {
                auto &actp(AirSysCompToPlant(i));
                actp.AirLoopNum = 0;
                actp.AirLoopBranch = 0;
                actp.AirLoopComp = 0;
                actp.PlantLoopType = 0;
                actp.PlantLoopNum = 0;
                actp.PlantLoopBranch = 0;
                actp.PlantLoopComp = 0;
                actp.FirstDemandSidePtr = 0;
                actp.LastDemandSidePtr = 0;
            }
        }

        Idx = ArrayCounter;
        auto &actp(AirSysCompToPlant(Idx));
        actp.AirLoopNum = AirLoopNum;
        actp.AirLoopBranch = BranchNum;
        actp.AirLoopComp = CompNum;
        actp.PlantLoopType = PlantLoopType;
        actp.PlantLoopNum = PlantLoop;
        actp.PlantLoopBranch = PlantBranch;
        actp.PlantLoopComp = PlantComp;
        ++ArrayCounter;
    }

    void UpdateAirSysSubCompPtrArray(int &Idx,
                                     int const AirLoopNum,
                                     int const BranchNum,
                                     int const CompNum,
                                     int const SubCompNum,
                                     int const PlantLoopType,
                                     int const PlantLoop,
                                     int const PlantBranch,
                                     int const PlantComp)
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Dan Fisher
        //       DATE WRITTEN   June 2005
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Update Air System Sub Component Pointer Array

        // METHODOLOGY EMPLOYED:
        // na

        // REFERENCES:
        // na

        // USE STATEMENTS:
        // na

        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:
        // SUBROUTINE PARAMETER DEFINITIONS:
        // na

        // INTERFACE BLOCK SPECIFICATIONS:
        // na

        static bool OneTimeFlag(true);
        static int ArrayLimit(100);
        static int ArrayCounter(1);

        if (OneTimeFlag) {
            AirSysSubCompToPlant.allocate(ArrayLimit);
            for (auto &e : AirSysSubCompToPlant) {
                e.AirLoopNum = 0;
                e.AirLoopBranch = 0;
                e.AirLoopComp = 0;
                e.AirLoopSubComp = 0;
                e.PlantLoopType = 0;
                e.PlantLoopNum = 0;
                e.PlantLoopBranch = 0;
                e.PlantLoopComp = 0;
                e.FirstDemandSidePtr = 0;
                e.LastDemandSidePtr = 0;
            }

            OneTimeFlag = false;
        }

        if (ArrayCounter >= ArrayLimit) { // Redimension larger
            int const OldArrayLimit(ArrayLimit);
            AirSysSubCompToPlant.redimension(ArrayLimit *= 2);
            for (int i = OldArrayLimit + 1; i <= ArrayLimit; ++i) {
                auto &actp(AirSysSubCompToPlant(i));
                actp.AirLoopNum = 0;
                actp.AirLoopBranch = 0;
                actp.AirLoopComp = 0;
                actp.AirLoopSubComp = 0;
                actp.PlantLoopType = 0;
                actp.PlantLoopNum = 0;
                actp.PlantLoopBranch = 0;
                actp.PlantLoopComp = 0;
                actp.FirstDemandSidePtr = 0;
                actp.LastDemandSidePtr = 0;
            }
        }

        Idx = ArrayCounter;
        auto &actp(AirSysSubCompToPlant(Idx));
        actp.AirLoopNum = AirLoopNum;
        actp.AirLoopBranch = BranchNum;
        actp.AirLoopComp = CompNum;
        actp.AirLoopSubComp = SubCompNum;
        actp.PlantLoopType = PlantLoopType;
        actp.PlantLoopNum = PlantLoop;
        actp.PlantLoopBranch = PlantBranch;
        actp.PlantLoopComp = PlantComp;
        ++ArrayCounter;
    }

    void UpdateAirSysSubSubCompPtrArray(int &Idx,
                                        int const AirLoopNum,
                                        int const BranchNum,
                                        int const CompNum,
                                        int const SubCompNum,
                                        int const SubSubCompNum,
                                        int const PlantLoopType,
                                        int const PlantLoop,
                                        int const PlantBranch,
                                        int const PlantComp)
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Dan Fisher
        //       DATE WRITTEN   June 2005
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Update Air System Sub Sub Component Pointer Array

        // METHODOLOGY EMPLOYED:
        // na

        // REFERENCES:
        // na

        // USE STATEMENTS:
        // na

        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:
        // SUBROUTINE PARAMETER DEFINITIONS:
        // na

        // INTERFACE BLOCK SPECIFICATIONS:
        // na

        static bool OneTimeFlag(true);
        static int ArrayLimit(100);
        static int ArrayCounter(1);

        if (OneTimeFlag) {
            AirSysSubSubCompToPlant.allocate(ArrayLimit);
            for (auto &e : AirSysSubSubCompToPlant) {
                e.AirLoopNum = 0;
                e.AirLoopBranch = 0;
                e.AirLoopComp = 0;
                e.AirLoopSubComp = 0;
                e.AirLoopSubSubComp = 0;
                e.PlantLoopType = 0;
                e.PlantLoopNum = 0;
                e.PlantLoopBranch = 0;
                e.PlantLoopComp = 0;
                e.FirstDemandSidePtr = 0;
                e.LastDemandSidePtr = 0;
            }

            OneTimeFlag = false;
        }

        if (ArrayCounter >= ArrayLimit) { // Redimension larger
            int const OldArrayLimit(ArrayLimit);
            AirSysSubSubCompToPlant.redimension(ArrayLimit *= 2);
            for (int i = OldArrayLimit + 1; i <= ArrayLimit; ++i) {
                auto &actp(AirSysSubSubCompToPlant(i));
                actp.AirLoopNum = 0;
                actp.AirLoopBranch = 0;
                actp.AirLoopComp = 0;
                actp.AirLoopSubComp = 0;
                actp.AirLoopSubSubComp = 0;
                actp.PlantLoopType = 0;
                actp.PlantLoopNum = 0;
                actp.PlantLoopBranch = 0;
                actp.PlantLoopComp = 0;
                actp.FirstDemandSidePtr = 0;
                actp.LastDemandSidePtr = 0;
            }
        }

        Idx = ArrayCounter;
        auto &actp(AirSysSubSubCompToPlant(Idx));
        actp.AirLoopNum = AirLoopNum;
        actp.AirLoopBranch = BranchNum;
        actp.AirLoopComp = CompNum;
        actp.AirLoopSubComp = SubCompNum;
        actp.AirLoopSubSubComp = SubSubCompNum;
        actp.PlantLoopType = PlantLoopType;
        actp.PlantLoopNum = PlantLoop;
        actp.PlantLoopBranch = PlantBranch;
        actp.PlantLoopComp = PlantComp;
        ++ArrayCounter;
    }

    void AllocateAndSetUpVentReports(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Sankaranarayanan K P
        //       DATE WRITTEN   July 2005
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Allocates Arrays and setup output variables related to Ventilation reports.

        // METHODOLOGY EMPLOYED:
        // na

        // REFERENCES:
        // na

        // Using/Aliasing
        using DataZoneEquipment::NumOfZones;
        using DataZoneEquipment::ZoneEquipConfig;

        // Subroutine Variable Declaration

        // Locals
        int ZoneIndex;
        int SysIndex;

        MaxCoolingLoadMetByVent.allocate(NumOfZones);
        MaxCoolingLoadAddedByVent.allocate(NumOfZones);
        MaxOvercoolingByVent.allocate(NumOfZones);
        MaxHeatingLoadMetByVent.allocate(NumOfZones);
        MaxHeatingLoadAddedByVent.allocate(NumOfZones);
        MaxOverheatingByVent.allocate(NumOfZones);
        MaxNoLoadHeatingByVent.allocate(NumOfZones);
        MaxNoLoadCoolingByVent.allocate(NumOfZones);

        ZoneOAMassFlow.allocate(NumOfZones);
        ZoneOAMass.allocate(NumOfZones);
        ZoneOAVolFlowStdRho.allocate(NumOfZones);
        ZoneOAVolStdRho.allocate(NumOfZones);
        ZoneOAVolFlowCrntRho.allocate(NumOfZones);
        ZoneOAVolCrntRho.allocate(NumOfZones);
        ZoneMechACH.allocate(NumOfZones);

        SysTotZoneLoadHTNG.allocate(NumPrimaryAirSys);
        SysTotZoneLoadCLNG.allocate(NumPrimaryAirSys);
        SysOALoadHTNG.allocate(NumPrimaryAirSys);
        SysOALoadCLNG.allocate(NumPrimaryAirSys);
        SysTotHTNG.allocate(NumPrimaryAirSys);
        SysTotCLNG.allocate(NumPrimaryAirSys);

        SysTotElec.allocate(NumPrimaryAirSys);
        SysTotNaturalGas.allocate(NumPrimaryAirSys);
        SysTotPropane.allocate(NumPrimaryAirSys);
        SysTotSteam.allocate(NumPrimaryAirSys);
        SysTotH2OCOLD.allocate(NumPrimaryAirSys);
        SysTotH2OHOT.allocate(NumPrimaryAirSys);

        SysHumidHTNG.allocate(NumPrimaryAirSys);
        SysHumidElec.allocate(NumPrimaryAirSys);
        SysHumidNaturalGas.allocate(NumPrimaryAirSys);
        SysHumidPropane.allocate(NumPrimaryAirSys);
        DesDehumidCLNG.allocate(NumPrimaryAirSys);
        DesDehumidElec.allocate(NumPrimaryAirSys);
        SysEvapCLNG.allocate(NumPrimaryAirSys);
        SysEvapElec.allocate(NumPrimaryAirSys);
        SysHeatExHTNG.allocate(NumPrimaryAirSys);
        SysHeatExCLNG.allocate(NumPrimaryAirSys);
        SysSolarCollectHeating.allocate(NumPrimaryAirSys);
        SysSolarCollectCooling.allocate(NumPrimaryAirSys);
        SysUserDefinedTerminalHeating.allocate(NumPrimaryAirSys);
        SysUserDefinedTerminalCooling.allocate(NumPrimaryAirSys);
        SysFANCompHTNG.allocate(NumPrimaryAirSys);
        SysFANCompElec.allocate(NumPrimaryAirSys);
        SysCCCompCLNG.allocate(NumPrimaryAirSys);
        SysCCCompH2OCOLD.allocate(NumPrimaryAirSys);
        SysCCCompElec.allocate(NumPrimaryAirSys);
        SysHCCompH2OHOT.allocate(NumPrimaryAirSys);
        SysHCCompElec.allocate(NumPrimaryAirSys);
        SysHCCompElecRes.allocate(NumPrimaryAirSys);
        SysHCCompHTNG.allocate(NumPrimaryAirSys);
        SysHCCompNaturalGas.allocate(NumPrimaryAirSys);
        SysHCCompPropane.allocate(NumPrimaryAirSys);
        SysHCCompSteam.allocate(NumPrimaryAirSys);
        SysDomesticH2O.allocate(NumPrimaryAirSys);

        SetBackCounter.allocate(NumOfZones);
        HeatCoolFlag.allocate(NumOfZones);
        LastHeatCoolFlag.allocate(NumOfZones);
        FirstHeatCoolFlag.allocate(NumOfZones);
        LastHeatCoolHour.allocate(NumOfZones);
        FirstHeatCoolHour.allocate(NumOfZones);
        NoLoadFlag.allocate(NumOfZones);
        UnmetLoadFlag.allocate(NumOfZones);

        UnmetLoadFlag = false;
        SetBackCounter = 0;
        HeatCoolFlag = 0;
        LastHeatCoolFlag = 0;
        FirstHeatCoolFlag = 0;
        LastHeatCoolHour = 0;
        FirstHeatCoolHour = 0;
        NoLoadFlag = false;

        MaxCoolingLoadMetByVent = 0.0;
        MaxCoolingLoadAddedByVent = 0.0;
        MaxOvercoolingByVent = 0.0;
        MaxHeatingLoadMetByVent = 0.0;
        MaxHeatingLoadAddedByVent = 0.0;
        MaxOverheatingByVent = 0.0;
        MaxNoLoadHeatingByVent = 0.0;
        MaxNoLoadCoolingByVent = 0.0;

        ZoneOAMassFlow = 0.0;
        ZoneOAMass = 0.0;
        ZoneOAVolFlowStdRho = 0.0;
        ZoneOAVolStdRho = 0.0;
        ZoneOAVolFlowCrntRho = 0.0;
        ZoneOAVolCrntRho = 0.0;
        ZoneMechACH = 0.0;

        // SYSTEM LOADS REPORT
        SysTotZoneLoadHTNG = 0.0;
        SysTotZoneLoadCLNG = 0.0;
        SysOALoadHTNG = 0.0;
        SysOALoadCLNG = 0.0;
        SysTotHTNG = 0.0;
        SysTotCLNG = 0.0;

        // SYSTEM ENERGY USE REPORT
        SysTotElec = 0.0;
        SysTotNaturalGas = 0.0;
        SysTotPropane = 0.0;
        SysTotSteam = 0.0;
        SysTotH2OCOLD = 0.0;
        SysTotH2OHOT = 0.0;

        // SYSTEM COMPONENT LOADS REPORT
        SysFANCompHTNG = 0.0;
        SysCCCompCLNG = 0.0;
        SysHCCompHTNG = 0.0;
        SysHeatExHTNG = 0.0;
        SysHeatExCLNG = 0.0;
        SysSolarCollectHeating = 0.0;
        SysSolarCollectCooling = 0.0;
        SysUserDefinedTerminalHeating = 0.0;
        SysUserDefinedTerminalCooling = 0.0;
        SysHumidHTNG = 0.0;
        SysEvapCLNG = 0.0;
        DesDehumidCLNG = 0.0;
        SysDomesticH2O = 0.0;

        // SYSTEM COMPONENT ENERGY REPORT
        SysFANCompElec = 0.0;
        SysHCCompH2OHOT = 0.0;
        SysCCCompH2OCOLD = 0.0;
        SysHCCompElec = 0.0;
        SysCCCompElec = 0.0;
        SysHCCompElecRes = 0.0;
        SysHCCompNaturalGas = 0.0;
        SysHCCompPropane = 0.0;
        SysHCCompSteam = 0.0;
        SysHumidElec = 0.0;
        SysHumidNaturalGas = 0.0;
        SysHumidPropane = 0.0;
        DesDehumidElec = 0.0;
        SysEvapElec = 0.0;

        if (AirLoopLoadsReportEnabled) {
            for (SysIndex = 1; SysIndex <= NumPrimaryAirSys; ++SysIndex) {

                // CurrentModuleObject='AirloopHVAC'
                // SYSTEM LOADS REPORT
                SetupOutputVariable(state, "Air System Total Heating Energy",
                                    OutputProcessor::Unit::J,
                                    SysTotHTNG(SysIndex),
                                    "HVAC",
                                    "Sum",
                                    PrimaryAirSystem(SysIndex).Name);

                SetupOutputVariable(state, "Air System Total Cooling Energy",
                                    OutputProcessor::Unit::J,
                                    SysTotCLNG(SysIndex),
                                    "HVAC",
                                    "Sum",
                                    PrimaryAirSystem(SysIndex).Name);

                // SYSTEM ENERGY USE REPORT
                SetupOutputVariable(state,
                    "Air System Hot Water Energy", OutputProcessor::Unit::J, SysTotH2OHOT(SysIndex), "HVAC", "Sum", PrimaryAirSystem(SysIndex).Name);

                SetupOutputVariable(state,
                    "Air System Steam Energy", OutputProcessor::Unit::J, SysTotSteam(SysIndex), "HVAC", "Sum", PrimaryAirSystem(SysIndex).Name);

                SetupOutputVariable(state, "Air System Chilled Water Energy",
                                    OutputProcessor::Unit::J,
                                    SysTotH2OCOLD(SysIndex),
                                    "HVAC",
                                    "Sum",
                                    PrimaryAirSystem(SysIndex).Name);

                SetupOutputVariable(state,
                    "Air System Electricity Energy", OutputProcessor::Unit::J, SysTotElec(SysIndex), "HVAC", "Sum", PrimaryAirSystem(SysIndex).Name);

                SetupOutputVariable(state,
                    "Air System NaturalGas Energy", OutputProcessor::Unit::J, SysTotNaturalGas(SysIndex), "HVAC", "Sum", PrimaryAirSystem(SysIndex).Name);

                SetupOutputVariable(state,
                    "Air System Propane Energy", OutputProcessor::Unit::J, SysTotPropane(SysIndex), "HVAC", "Sum", PrimaryAirSystem(SysIndex).Name);

                SetupOutputVariable(state,
                    "Air System Water Volume", OutputProcessor::Unit::m3, SysDomesticH2O(SysIndex), "HVAC", "Sum", PrimaryAirSystem(SysIndex).Name);

                // SYSTEM COMPONENT LOAD REPORT
                SetupOutputVariable(state, "Air System Fan Air Heating Energy",
                                    OutputProcessor::Unit::J,
                                    SysFANCompHTNG(SysIndex),
                                    "HVAC",
                                    "Sum",
                                    PrimaryAirSystem(SysIndex).Name);

                SetupOutputVariable(state, "Air System Cooling Coil Total Cooling Energy",
                                    OutputProcessor::Unit::J,
                                    SysCCCompCLNG(SysIndex),
                                    "HVAC",
                                    "Sum",
                                    PrimaryAirSystem(SysIndex).Name);

                SetupOutputVariable(state, "Air System Heating Coil Total Heating Energy",
                                    OutputProcessor::Unit::J,
                                    SysHCCompHTNG(SysIndex),
                                    "HVAC",
                                    "Sum",
                                    PrimaryAirSystem(SysIndex).Name);

                SetupOutputVariable(state, "Air System Heat Exchanger Total Heating Energy",
                                    OutputProcessor::Unit::J,
                                    SysHeatExHTNG(SysIndex),
                                    "HVAC",
                                    "Sum",
                                    PrimaryAirSystem(SysIndex).Name);

                SetupOutputVariable(state, "Air System Heat Exchanger Total Cooling Energy",
                                    OutputProcessor::Unit::J,
                                    SysHeatExCLNG(SysIndex),
                                    "HVAC",
                                    "Sum",
                                    PrimaryAirSystem(SysIndex).Name);

                SetupOutputVariable(state, "Air System Solar Collector Total Heating Energy",
                                    OutputProcessor::Unit::J,
                                    SysSolarCollectHeating(SysIndex),
                                    "HVAC",
                                    "Sum",
                                    PrimaryAirSystem(SysIndex).Name);

                SetupOutputVariable(state, "Air System Solar Collector Total Cooling Energy",
                                    OutputProcessor::Unit::J,
                                    SysSolarCollectCooling(SysIndex),
                                    "HVAC",
                                    "Sum",
                                    PrimaryAirSystem(SysIndex).Name);

                SetupOutputVariable(state, "Air System User Defined Air Terminal Total Heating Energy",
                                    OutputProcessor::Unit::J,
                                    SysUserDefinedTerminalHeating(SysIndex),
                                    "HVAC",
                                    "Sum",
                                    PrimaryAirSystem(SysIndex).Name);

                SetupOutputVariable(state, "Air System User Defined Air Terminal Total Cooling Energy",
                                    OutputProcessor::Unit::J,
                                    SysUserDefinedTerminalCooling(SysIndex),
                                    "HVAC",
                                    "Sum",
                                    PrimaryAirSystem(SysIndex).Name);

                SetupOutputVariable(state, "Air System Humidifier Total Heating Energy",
                                    OutputProcessor::Unit::J,
                                    SysHumidHTNG(SysIndex),
                                    "HVAC",
                                    "Sum",
                                    PrimaryAirSystem(SysIndex).Name);

                SetupOutputVariable(state, "Air System Evaporative Cooler Total Cooling Energy",
                                    OutputProcessor::Unit::J,
                                    SysEvapCLNG(SysIndex),
                                    "HVAC",
                                    "Sum",
                                    PrimaryAirSystem(SysIndex).Name);

                SetupOutputVariable(state, "Air System Desiccant Dehumidifier Total Cooling Energy",
                                    OutputProcessor::Unit::J,
                                    DesDehumidCLNG(SysIndex),
                                    "HVAC",
                                    "Sum",
                                    PrimaryAirSystem(SysIndex).Name);

                // SYSTEM COMPONENT ENERGY REPORT
                SetupOutputVariable(state, "Air System Fan Electricity Energy",
                                    OutputProcessor::Unit::J,
                                    SysFANCompElec(SysIndex),
                                    "HVAC",
                                    "Sum",
                                    PrimaryAirSystem(SysIndex).Name);

                SetupOutputVariable(state, "Air System Heating Coil Hot Water Energy",
                                    OutputProcessor::Unit::J,
                                    SysHCCompH2OHOT(SysIndex),
                                    "HVAC",
                                    "Sum",
                                    PrimaryAirSystem(SysIndex).Name);

                SetupOutputVariable(state, "Air System Cooling Coil Chilled Water Energy",
                                    OutputProcessor::Unit::J,
                                    SysCCCompH2OCOLD(SysIndex),
                                    "HVAC",
                                    "Sum",
                                    PrimaryAirSystem(SysIndex).Name);

                SetupOutputVariable(state, "Air System DX Heating Coil Electricity Energy",
                                    OutputProcessor::Unit::J,
                                    SysHCCompElec(SysIndex),
                                    "HVAC",
                                    "Sum",
                                    PrimaryAirSystem(SysIndex).Name);

                SetupOutputVariable(state, "Air System DX Cooling Coil Electricity Energy",
                                    OutputProcessor::Unit::J,
                                    SysCCCompElec(SysIndex),
                                    "HVAC",
                                    "Sum",
                                    PrimaryAirSystem(SysIndex).Name);

                SetupOutputVariable(state, "Air System Heating Coil Electricity Energy",
                                    OutputProcessor::Unit::J,
                                    SysHCCompElecRes(SysIndex),
                                    "HVAC",
                                    "Sum",
                                    PrimaryAirSystem(SysIndex).Name);

                SetupOutputVariable(state, "Air System Heating Coil NaturalGas Energy",
                                    OutputProcessor::Unit::J,
                                    SysHCCompNaturalGas(SysIndex),
                                    "HVAC",
                                    "Sum",
                                    PrimaryAirSystem(SysIndex).Name);

                SetupOutputVariable(state, "Air System Heating Coil Propane Energy",
                                    OutputProcessor::Unit::J,
                                    SysHCCompPropane(SysIndex),
                                    "HVAC",
                                    "Sum",
                                    PrimaryAirSystem(SysIndex).Name);

                SetupOutputVariable(state, "Air System Heating Coil Steam Energy",
                                    OutputProcessor::Unit::J,
                                    SysHCCompSteam(SysIndex),
                                    "HVAC",
                                    "Sum",
                                    PrimaryAirSystem(SysIndex).Name);

                SetupOutputVariable(state, "Air System Humidifier Electricity Energy",
                                    OutputProcessor::Unit::J,
                                    SysHumidElec(SysIndex),
                                    "HVAC",
                                    "Sum",
                                    PrimaryAirSystem(SysIndex).Name);

                SetupOutputVariable(state, "Air System Humidifier NaturalGas Energy",
                                    OutputProcessor::Unit::J,
                                    SysHumidNaturalGas(SysIndex),
                                    "HVAC",
                                    "Sum",
                                    PrimaryAirSystem(SysIndex).Name);

                SetupOutputVariable(state, "Air System Humidifier Propane Energy",
                                    OutputProcessor::Unit::J,
                                    SysHumidPropane(SysIndex),
                                    "HVAC",
                                    "Sum",
                                    PrimaryAirSystem(SysIndex).Name);

                SetupOutputVariable(state, "Air System Evaporative Cooler Electricity Energy",
                                    OutputProcessor::Unit::J,
                                    SysEvapElec(SysIndex),
                                    "HVAC",
                                    "Sum",
                                    PrimaryAirSystem(SysIndex).Name);

                SetupOutputVariable(state, "Air System Desiccant Dehumidifier Electricity Energy",
                                    OutputProcessor::Unit::J,
                                    DesDehumidElec(SysIndex),
                                    "HVAC",
                                    "Sum",
                                    PrimaryAirSystem(SysIndex).Name);
            }
        }
        for (ZoneIndex = 1; ZoneIndex <= NumOfZones; ++ZoneIndex) {
            if (!ZoneEquipConfig(ZoneIndex).IsControlled) continue;
            // CurrentModuleObject='Zones(Controlled)'
            if (VentLoadsReportEnabled) {
                // Cooling Loads
                SetupOutputVariable(state, "Zone Mechanical Ventilation No Load Heat Removal Energy",
                                    OutputProcessor::Unit::J,
                                    MaxNoLoadCoolingByVent(ZoneIndex),
                                    "HVAC",
                                    "Sum",
                                    ZoneEquipConfig(ZoneIndex).ZoneName);

                SetupOutputVariable(state, "Zone Mechanical Ventilation Cooling Load Increase Energy",
                                    OutputProcessor::Unit::J,
                                    MaxCoolingLoadAddedByVent(ZoneIndex),
                                    "HVAC",
                                    "Sum",
                                    ZoneEquipConfig(ZoneIndex).ZoneName);

                SetupOutputVariable(state, "Zone Mechanical Ventilation Cooling Load Increase Due to Overheating Energy",
                                    OutputProcessor::Unit::J,
                                    MaxOverheatingByVent(ZoneIndex),
                                    "HVAC",
                                    "Sum",
                                    ZoneEquipConfig(ZoneIndex).ZoneName);

                SetupOutputVariable(state, "Zone Mechanical Ventilation Cooling Load Decrease Energy",
                                    OutputProcessor::Unit::J,
                                    MaxCoolingLoadMetByVent(ZoneIndex),
                                    "HVAC",
                                    "Sum",
                                    ZoneEquipConfig(ZoneIndex).ZoneName);
                // Heating Loads
                SetupOutputVariable(state, "Zone Mechanical Ventilation No Load Heat Addition Energy",
                                    OutputProcessor::Unit::J,
                                    MaxNoLoadHeatingByVent(ZoneIndex),
                                    "HVAC",
                                    "Sum",
                                    ZoneEquipConfig(ZoneIndex).ZoneName);

                SetupOutputVariable(state, "Zone Mechanical Ventilation Heating Load Increase Energy",
                                    OutputProcessor::Unit::J,
                                    MaxHeatingLoadAddedByVent(ZoneIndex),
                                    "HVAC",
                                    "Sum",
                                    ZoneEquipConfig(ZoneIndex).ZoneName);

                SetupOutputVariable(state, "Zone Mechanical Ventilation Heating Load Increase Due to Overcooling Energy",
                                    OutputProcessor::Unit::J,
                                    MaxOvercoolingByVent(ZoneIndex),
                                    "HVAC",
                                    "Sum",
                                    ZoneEquipConfig(ZoneIndex).ZoneName);

                SetupOutputVariable(state, "Zone Mechanical Ventilation Heating Load Decrease Energy",
                                    OutputProcessor::Unit::J,
                                    MaxHeatingLoadMetByVent(ZoneIndex),
                                    "HVAC",
                                    "Sum",
                                    ZoneEquipConfig(ZoneIndex).ZoneName);
            }

            SetupOutputVariable(state, "Zone Mechanical Ventilation Mass Flow Rate",
                                OutputProcessor::Unit::kg_s,
                                ZoneOAMassFlow(ZoneIndex),
                                "HVAC",
                                "Average",
                                ZoneEquipConfig(ZoneIndex).ZoneName);

            SetupOutputVariable(state, "Zone Mechanical Ventilation Mass",
                                OutputProcessor::Unit::kg,
                                ZoneOAMass(ZoneIndex),
                                "HVAC",
                                "Sum",
                                ZoneEquipConfig(ZoneIndex).ZoneName);

            SetupOutputVariable(state, "Zone Mechanical Ventilation Standard Density Volume Flow Rate",
                                OutputProcessor::Unit::m3_s,
                                ZoneOAVolFlowStdRho(ZoneIndex),
                                "HVAC",
                                "Average",
                                ZoneEquipConfig(ZoneIndex).ZoneName);

            SetupOutputVariable(state, "Zone Mechanical Ventilation Standard Density Volume",
                                OutputProcessor::Unit::m3,
                                ZoneOAVolStdRho(ZoneIndex),
                                "HVAC",
                                "Sum",
                                ZoneEquipConfig(ZoneIndex).ZoneName);

            SetupOutputVariable(state, "Zone Mechanical Ventilation Current Density Volume Flow Rate",
                                OutputProcessor::Unit::m3_s,
                                ZoneOAVolFlowCrntRho(ZoneIndex),
                                "HVAC",
                                "Average",
                                ZoneEquipConfig(ZoneIndex).ZoneName);

            SetupOutputVariable(state, "Zone Mechanical Ventilation Current Density Volume",
                                OutputProcessor::Unit::m3,
                                ZoneOAVolCrntRho(ZoneIndex),
                                "HVAC",
                                "Sum",
                                ZoneEquipConfig(ZoneIndex).ZoneName);

            SetupOutputVariable(state, "Zone Mechanical Ventilation Air Changes per Hour",
                                OutputProcessor::Unit::ach,
                                ZoneMechACH(ZoneIndex),
                                "HVAC",
                                "Average",
                                ZoneEquipConfig(ZoneIndex).ZoneName);
        }
    }

    void CreateEnergyReportStructure()
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Dan Fisher/Linda Lawrie
        //       DATE WRITTEN   June 2005
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Creates the Energy Reporting Structure.  This routine is only called once --
        // so string compares have been left in.

        // METHODOLOGY EMPLOYED:
        // Once all compsets/nodes/connections have been established find all components
        // subcomponents, etc.

        // REFERENCES:
        // na

        // Using/Aliasing
        using BranchNodeConnections::GetChildrenData;
        using BranchNodeConnections::GetComponentData;
        using BranchNodeConnections::GetNumChildren;
        using BranchNodeConnections::IsParentObject;
        using namespace DataGlobalConstants;

        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:
        // na

        // SUBROUTINE PARAMETER DEFINITIONS:
        // na

        // INTERFACE BLOCK SPECIFICATIONS:

        // DERIVED TYPE DEFINITIONS:
        // na

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int AirLoopNum;
        int BranchNum;
        int CompNum;
        int SubCompNum;
        int SubSubCompNum;
        int VarNum;
        int VarNum1;
        int CtrlZoneNum;
        std::string TypeOfComp;
        std::string NameOfComp;
        bool ErrorsFound;
        bool ModeFlagOn;
        int NumInlets;
        int NumOutlets;
        int PlantLoopNum;

        // Dimension GetChildrenData arrays
        Array1D_string SubCompTypes;
        Array1D_string SubCompNames;
        Array1D_string InletNodeNames;
        Array1D_int InletNodeNumbers;
        Array1D_int InletFluidStreams;
        Array1D_string OutletNodeNames;
        Array1D_int OutletNodeNumbers;
        Array1D_int OutletFluidStreams;
        int NumChildren;
        int NumGrandChildren;
        bool IsParent;

        // Dimension GetMeteredVariables arrays
        Array1D_int VarIndexes;                            // Variable Numbers
        Array1D_int VarTypes;                              // Variable Types (1=integer, 2=real, 3=meter)
        Array1D_string UnitsStrings;                       // UnitsStrings for each variable
        Array1D<OutputProcessor::TimeStepType> IndexTypes; // Variable Idx Types (1=Zone,2=HVAC)
        Array1D<OutputProcessor::Unit> unitsForVar;        // units from enum for each variable
        Array1D_int ResourceTypes;                         // ResourceTypes for each variable
        Array1D_string EndUses;                            // EndUses for each variable
        Array1D_string Groups;                             // Groups for each variable
        Array1D_string Names;                              // Variable Names for each variable
        int NumFound;                                      // Number Found
        int NumVariables;
        int NumLeft; // Counter for deeper components

        // some variables for setting up the plant data structures
        int LoopSideNum;

        VentReportStructureCreated = true;
        for (AirLoopNum = 1; AirLoopNum <= NumPrimaryAirSys; ++AirLoopNum) {
            for (BranchNum = 1; BranchNum <= PrimaryAirSystem(AirLoopNum).NumBranches; ++BranchNum) {
                for (CompNum = 1; CompNum <= PrimaryAirSystem(AirLoopNum).Branch(BranchNum).TotalComponents; ++CompNum) {
                    TypeOfComp = PrimaryAirSystem(AirLoopNum).Branch(BranchNum).Comp(CompNum).TypeOf;
                    NameOfComp = PrimaryAirSystem(AirLoopNum).Branch(BranchNum).Comp(CompNum).Name;
                    // Get complete list of components for complex branches
                    if (IsParentObject(TypeOfComp, NameOfComp)) {

                        PrimaryAirSystem(AirLoopNum).Branch(BranchNum).Comp(CompNum).Parent = true;
                        NumChildren = GetNumChildren(TypeOfComp, NameOfComp);

                        SubCompTypes.allocate(NumChildren);
                        SubCompNames.allocate(NumChildren);
                        InletNodeNames.allocate(NumChildren);
                        InletNodeNumbers.allocate(NumChildren);
                        OutletNodeNames.allocate(NumChildren);
                        OutletNodeNumbers.allocate(NumChildren);
                        PrimaryAirSystem(AirLoopNum).Branch(BranchNum).Comp(CompNum).SubComp.allocate(NumChildren);

                        GetChildrenData(TypeOfComp,
                                        NameOfComp,
                                        NumChildren,
                                        SubCompTypes,
                                        SubCompNames,
                                        InletNodeNames,
                                        InletNodeNumbers,
                                        OutletNodeNames,
                                        OutletNodeNumbers,
                                        ErrorsFound);

                        for (SubCompNum = 1; SubCompNum <= NumChildren; ++SubCompNum) {
                            {
                                auto &thisSubComponent(PrimaryAirSystem(AirLoopNum).Branch(BranchNum).Comp(CompNum).SubComp(SubCompNum));
                                thisSubComponent.TypeOf = SubCompTypes(SubCompNum);
                                thisSubComponent.Name = SubCompNames(SubCompNum);
                                thisSubComponent.NodeNameIn = InletNodeNames(SubCompNum);
                                thisSubComponent.NodeNameOut = OutletNodeNames(SubCompNum);
                                thisSubComponent.NodeNumIn = InletNodeNumbers(SubCompNum);
                                thisSubComponent.NodeNumOut = OutletNodeNumbers(SubCompNum);
                            }
                        }

                        SubCompTypes.deallocate();
                        SubCompNames.deallocate();
                        InletNodeNames.deallocate();
                        InletNodeNumbers.deallocate();
                        OutletNodeNames.deallocate();
                        OutletNodeNumbers.deallocate();

                    } else {
                        NumChildren = 0;
                        PrimaryAirSystem(AirLoopNum).Branch(BranchNum).Comp(CompNum).Parent = false;
                    }
                    PrimaryAirSystem(AirLoopNum).Branch(BranchNum).Comp(CompNum).NumSubComps = NumChildren;

                    // check for 'grandchildren'
                    for (SubCompNum = 1; SubCompNum <= NumChildren; ++SubCompNum) {
                        TypeOfComp = PrimaryAirSystem(AirLoopNum).Branch(BranchNum).Comp(CompNum).SubComp(SubCompNum).TypeOf;
                        NameOfComp = PrimaryAirSystem(AirLoopNum).Branch(BranchNum).Comp(CompNum).SubComp(SubCompNum).Name;
                        if (IsParentObject(TypeOfComp, NameOfComp)) {
                            NumGrandChildren = GetNumChildren(TypeOfComp, NameOfComp);
                            SubCompTypes.allocate(NumGrandChildren);
                            SubCompNames.allocate(NumGrandChildren);
                            InletNodeNames.allocate(NumGrandChildren);
                            InletNodeNumbers.allocate(NumGrandChildren);
                            OutletNodeNames.allocate(NumGrandChildren);
                            OutletNodeNumbers.allocate(NumGrandChildren);
                            PrimaryAirSystem(AirLoopNum).Branch(BranchNum).Comp(CompNum).SubComp(SubCompNum).SubSubComp.allocate(NumGrandChildren);

                            GetChildrenData(TypeOfComp,
                                            NameOfComp,
                                            NumGrandChildren,
                                            SubCompTypes,
                                            SubCompNames,
                                            InletNodeNames,
                                            InletNodeNumbers,
                                            OutletNodeNames,
                                            OutletNodeNumbers,
                                            ErrorsFound);

                            for (SubSubCompNum = 1; SubSubCompNum <= NumGrandChildren; ++SubSubCompNum) {
                                {
                                    auto &thisSubSubComponent(
                                        PrimaryAirSystem(AirLoopNum).Branch(BranchNum).Comp(CompNum).SubComp(SubCompNum).SubSubComp(SubSubCompNum));
                                    thisSubSubComponent.TypeOf = SubCompTypes(SubSubCompNum);
                                    thisSubSubComponent.Name = SubCompNames(SubSubCompNum);
                                    thisSubSubComponent.NodeNameIn = InletNodeNames(SubSubCompNum);
                                    thisSubSubComponent.NodeNameOut = OutletNodeNames(SubSubCompNum);
                                    thisSubSubComponent.NodeNumIn = InletNodeNumbers(SubSubCompNum);
                                    thisSubSubComponent.NodeNumOut = OutletNodeNumbers(SubSubCompNum);
                                    NumLeft = GetNumChildren(SubCompTypes(SubSubCompNum), SubCompNames(SubSubCompNum));
                                    if (NumLeft > 0) {
                                        ShowSevereError("Hanging Children for component=" + SubCompTypes(SubSubCompNum) + ':' +
                                                        SubCompNames(SubSubCompNum));
                                    }
                                }
                            }

                            SubCompTypes.deallocate();
                            SubCompNames.deallocate();
                            InletNodeNames.deallocate();
                            InletNodeNumbers.deallocate();
                            OutletNodeNames.deallocate();
                            OutletNodeNumbers.deallocate();
                        } else {
                            NumGrandChildren = 0;
                            PrimaryAirSystem(AirLoopNum).Branch(BranchNum).Comp(CompNum).SubComp(SubCompNum).Parent = false;
                        }

                        PrimaryAirSystem(AirLoopNum).Branch(BranchNum).Comp(CompNum).SubComp(SubCompNum).NumSubSubComps = NumGrandChildren;
                    }
                }
            }
        }

        for (AirLoopNum = 1; AirLoopNum <= NumPrimaryAirSys; ++AirLoopNum) {
            for (BranchNum = 1; BranchNum <= PrimaryAirSystem(AirLoopNum).NumBranches; ++BranchNum) {
                for (CompNum = 1; CompNum <= PrimaryAirSystem(AirLoopNum).Branch(BranchNum).TotalComponents; ++CompNum) {
                    // Get complete list of components for complex branches
                    {
                        auto &thisComp(PrimaryAirSystem(AirLoopNum).Branch(BranchNum).Comp(CompNum));
                        TypeOfComp = thisComp.TypeOf;
                        NameOfComp = thisComp.Name;
                        NumVariables = GetNumMeteredVariables(TypeOfComp, NameOfComp);
                        if (NumVariables > 0) {
                            VarIndexes.allocate(NumVariables);
                            VarTypes.allocate(NumVariables);
                            IndexTypes.allocate(NumVariables);
                            unitsForVar.allocate(NumVariables);
                            ResourceTypes.allocate(NumVariables);
                            EndUses.allocate(NumVariables);
                            Groups.allocate(NumVariables);
                            Names.allocate(NumVariables);
                            thisComp.MeteredVar.allocate(NumVariables);

                            thisComp.NumMeteredVars = NumVariables;
                            GetMeteredVariables(TypeOfComp,
                                                NameOfComp,
                                                VarIndexes,
                                                VarTypes,
                                                IndexTypes,
                                                unitsForVar,
                                                ResourceTypes,
                                                EndUses,
                                                Groups,
                                                Names,
                                                NumFound);
                            ModeFlagOn = true;
                            for (VarNum = 1; VarNum <= NumVariables; ++VarNum) {
                                {
                                    auto &thisVar(thisComp.MeteredVar(VarNum));
                                    thisVar.ReportVarName = Names(VarNum);
                                    thisVar.ReportVarUnits = unitsForVar(VarNum);
                                    thisVar.ReportVarIndex = VarIndexes(VarNum);
                                    thisVar.ReportVarIndexType = IndexTypes(VarNum);
                                    thisVar.ReportVarType = VarTypes(VarNum);
                                    thisVar.ResourceType = ResourceTypes(VarNum);
                                    thisVar.EndUse = EndUses(VarNum);
                                    if (thisVar.EndUse == "HEATINGCOILS" && ModeFlagOn) {
                                        for (VarNum1 = 1; VarNum1 <= NumVariables; ++VarNum1) {
                                            thisComp.MeteredVar(VarNum1).EndUse_CompMode = HeatingOnly;
                                        }
                                        ModeFlagOn = false;
                                    } else if (thisVar.EndUse == "COOLINGCOILS" && ModeFlagOn) {
                                        for (VarNum1 = 1; VarNum1 <= NumVariables; ++VarNum1) {
                                            thisComp.MeteredVar(VarNum1).EndUse_CompMode = CoolingOnly;
                                        }
                                        ModeFlagOn = false;
                                    } else if (ModeFlagOn) {
                                        thisVar.EndUse_CompMode = NoHeatNoCool;
                                    }
                                    thisVar.Group = Groups(VarNum);
                                }
                            }

                            VarIndexes.deallocate();
                            VarTypes.deallocate();
                            IndexTypes.deallocate();
                            unitsForVar.deallocate();
                            ResourceTypes.deallocate();
                            EndUses.deallocate();
                            Groups.deallocate();
                            Names.deallocate();
                        }
                        for (SubCompNum = 1; SubCompNum <= thisComp.NumSubComps; ++SubCompNum) {
                            // Get complete list of components for complex branches
                            TypeOfComp = thisComp.SubComp(SubCompNum).TypeOf;
                            NameOfComp = thisComp.SubComp(SubCompNum).Name;
                            NumVariables = GetNumMeteredVariables(TypeOfComp, NameOfComp);
                            if (NumVariables > 0) {
                                VarIndexes.allocate(NumVariables);
                                VarTypes.allocate(NumVariables);
                                IndexTypes.allocate(NumVariables);
                                unitsForVar.allocate(NumVariables);
                                ResourceTypes.allocate(NumVariables);
                                EndUses.allocate(NumVariables);
                                Groups.allocate(NumVariables);
                                Names.allocate(NumVariables);
                                thisComp.SubComp(SubCompNum).MeteredVar.allocate(NumVariables);

                                GetMeteredVariables(TypeOfComp,
                                                    NameOfComp,
                                                    VarIndexes,
                                                    VarTypes,
                                                    IndexTypes,
                                                    unitsForVar,
                                                    ResourceTypes,
                                                    EndUses,
                                                    Groups,
                                                    Names,
                                                    NumFound);

                                ModeFlagOn = true;
                                for (VarNum = 1; VarNum <= NumVariables; ++VarNum) {
                                    {
                                        auto &thisVar(thisComp.SubComp(SubCompNum).MeteredVar(VarNum));
                                        thisVar.ReportVarName = Names(VarNum);
                                        thisVar.ReportVarUnits = unitsForVar(VarNum);
                                        thisVar.ReportVarIndex = VarIndexes(VarNum);
                                        thisVar.ReportVarIndexType = IndexTypes(VarNum);
                                        thisVar.ReportVarType = VarTypes(VarNum);
                                        thisVar.ResourceType = ResourceTypes(VarNum);
                                        thisVar.EndUse = EndUses(VarNum);
                                        if (thisVar.EndUse == "HEATINGCOILS" && ModeFlagOn) {
                                            for (VarNum1 = 1; VarNum1 <= NumVariables; ++VarNum1) {
                                                thisComp.SubComp(SubCompNum).MeteredVar(VarNum1).EndUse_CompMode = HeatingOnly;
                                            }
                                            ModeFlagOn = false;
                                        } else if (thisVar.EndUse == "COOLINGCOILS" && ModeFlagOn) {
                                            for (VarNum1 = 1; VarNum1 <= NumVariables; ++VarNum1) {
                                                thisComp.SubComp(SubCompNum).MeteredVar(VarNum1).EndUse_CompMode = CoolingOnly;
                                            }
                                            ModeFlagOn = false;
                                        } else if (ModeFlagOn) {
                                            thisVar.EndUse_CompMode = NoHeatNoCool;
                                        }
                                        thisVar.Group = Groups(VarNum);
                                    }
                                }

                                VarIndexes.deallocate();
                                VarTypes.deallocate();
                                IndexTypes.deallocate();
                                unitsForVar.deallocate();
                                ResourceTypes.deallocate();
                                EndUses.deallocate();
                                Groups.deallocate();
                                Names.deallocate();
                            }

                            thisComp.SubComp(SubCompNum).NumMeteredVars = NumVariables;

                            for (SubSubCompNum = 1; SubSubCompNum <= thisComp.SubComp(SubCompNum).NumSubSubComps; ++SubSubCompNum) {
                                // Get complete list of components for complex branches
                                TypeOfComp = thisComp.SubComp(SubCompNum).SubSubComp(SubSubCompNum).TypeOf;
                                NameOfComp = thisComp.SubComp(SubCompNum).SubSubComp(SubSubCompNum).Name;
                                NumVariables = GetNumMeteredVariables(TypeOfComp, NameOfComp);
                                if (NumVariables > 0) {
                                    VarIndexes.allocate(NumVariables);
                                    VarTypes.allocate(NumVariables);
                                    IndexTypes.allocate(NumVariables);
                                    unitsForVar.allocate(NumVariables);
                                    ResourceTypes.allocate(NumVariables);
                                    EndUses.allocate(NumVariables);
                                    Groups.allocate(NumVariables);
                                    Names.allocate(NumVariables);
                                    thisComp.SubComp(SubCompNum).SubSubComp(SubSubCompNum).MeteredVar.allocate(NumVariables);

                                    GetMeteredVariables(TypeOfComp,
                                                        NameOfComp,
                                                        VarIndexes,
                                                        VarTypes,
                                                        IndexTypes,
                                                        unitsForVar,
                                                        ResourceTypes,
                                                        EndUses,
                                                        Groups,
                                                        Names,
                                                        NumFound);

                                    ModeFlagOn = true;
                                    for (VarNum = 1; VarNum <= NumVariables; ++VarNum) {
                                        {
                                            auto &thisVar(thisComp.SubComp(SubCompNum).SubSubComp(SubSubCompNum).MeteredVar(VarNum));
                                            thisVar.ReportVarName = Names(VarNum);
                                            thisVar.ReportVarUnits = unitsForVar(VarNum);
                                            thisVar.ReportVarIndex = VarIndexes(VarNum);
                                            thisVar.ReportVarIndexType = IndexTypes(VarNum);
                                            thisVar.ReportVarType = VarTypes(VarNum);
                                            thisVar.ResourceType = ResourceTypes(VarNum);
                                            thisVar.EndUse = EndUses(VarNum);
                                            if (thisVar.EndUse == "HEATINGCOILS" && ModeFlagOn) {
                                                for (VarNum1 = 1; VarNum1 <= NumVariables; ++VarNum1) {
                                                    thisComp.SubComp(SubCompNum).SubSubComp(SubSubCompNum).MeteredVar(VarNum1).EndUse_CompMode =
                                                        HeatingOnly;
                                                }
                                                ModeFlagOn = false;
                                            } else if (thisVar.EndUse == "COOLINGCOILS" && ModeFlagOn) {
                                                for (VarNum1 = 1; VarNum1 <= NumVariables; ++VarNum1) {
                                                    thisComp.SubComp(SubCompNum).SubSubComp(SubSubCompNum).MeteredVar(VarNum1).EndUse_CompMode =
                                                        CoolingOnly;
                                                }
                                                ModeFlagOn = false;
                                            } else if (ModeFlagOn) {
                                                thisVar.EndUse_CompMode = NoHeatNoCool;
                                            }
                                            thisVar.Group = Groups(VarNum);
                                        }
                                    }

                                    VarIndexes.deallocate();
                                    VarTypes.deallocate();
                                    IndexTypes.deallocate();
                                    unitsForVar.deallocate();
                                    ResourceTypes.deallocate();
                                    EndUses.deallocate();
                                    Groups.deallocate();
                                    Names.deallocate();
                                }
                                thisComp.SubComp(SubCompNum).SubSubComp(SubSubCompNum).NumMeteredVars = NumVariables;
                            }
                        }
                    }
                }
            }
        }

        // Allocate the system serving zone equipment component arrays
        for (CtrlZoneNum = 1; CtrlZoneNum <= NumOfZones; ++CtrlZoneNum) {
            if (!ZoneEquipConfig(CtrlZoneNum).IsControlled) continue;
            // Set index of air loop serving zone
            for (CompNum = 1; CompNum <= ZoneEquipList(CtrlZoneNum).NumOfEquipTypes; ++CompNum) {
                TypeOfComp = ZoneEquipList(CtrlZoneNum).EquipType(CompNum);
                NameOfComp = ZoneEquipList(CtrlZoneNum).EquipName(CompNum);
                GetComponentData(TypeOfComp,
                                 NameOfComp,
                                 IsParent,
                                 NumInlets,
                                 InletNodeNames,
                                 InletNodeNumbers,
                                 InletFluidStreams,
                                 NumOutlets,
                                 OutletNodeNames,
                                 OutletNodeNumbers,
                                 OutletFluidStreams,
                                 ErrorsFound);
                {
                    auto &thisEquipData(ZoneEquipList(CtrlZoneNum).EquipData(CompNum));
                    thisEquipData.TypeOf = TypeOfComp;
                    thisEquipData.Name = NameOfComp;
                    thisEquipData.OutletNodeNums.allocate(NumOutlets);
                    thisEquipData.NumOutlets = NumOutlets;
                    thisEquipData.OutletNodeNums = OutletNodeNumbers;
                    thisEquipData.InletNodeNums.allocate(NumInlets);
                    thisEquipData.NumInlets = NumInlets;
                    thisEquipData.InletNodeNums = InletNodeNumbers;
                    thisEquipData.Parent = IsParent;
                    NumVariables = GetNumMeteredVariables(TypeOfComp, NameOfComp);
                    thisEquipData.NumMeteredVars = NumVariables;
                    if (NumVariables > 0) {
                        InletNodeNames.deallocate();
                        InletNodeNumbers.deallocate();
                        InletFluidStreams.deallocate();
                        OutletNodeNames.deallocate();
                        OutletNodeNumbers.deallocate();
                        OutletFluidStreams.deallocate();

                        VarIndexes.allocate(NumVariables);
                        VarTypes.allocate(NumVariables);
                        IndexTypes.allocate(NumVariables);
                        unitsForVar.allocate(NumVariables);
                        ResourceTypes.allocate(NumVariables);
                        EndUses.allocate(NumVariables);
                        Groups.allocate(NumVariables);
                        Names.allocate(NumVariables);
                        thisEquipData.MeteredVar.allocate(NumVariables);

                        GetMeteredVariables(
                            TypeOfComp, NameOfComp, VarIndexes, VarTypes, IndexTypes, unitsForVar, ResourceTypes, EndUses, Groups, Names, NumFound);

                        ModeFlagOn = true;
                        for (VarNum = 1; VarNum <= NumVariables; ++VarNum) {
                            {
                                auto &thisVar(thisEquipData.MeteredVar(VarNum));
                                thisVar.ReportVarName = Names(VarNum);
                                thisVar.ReportVarUnits = unitsForVar(VarNum);
                                thisVar.ReportVarIndex = VarIndexes(VarNum);
                                thisVar.ReportVarIndexType = IndexTypes(VarNum);
                                thisVar.ReportVarType = VarTypes(VarNum);
                                thisVar.ResourceType = ResourceTypes(VarNum);
                                thisVar.EndUse = EndUses(VarNum);
                                if (thisVar.EndUse == "HEATINGCOILS" && ModeFlagOn) {
                                    for (VarNum1 = 1; VarNum1 <= NumVariables; ++VarNum1) {
                                        thisEquipData.MeteredVar(VarNum1).EndUse_CompMode = HeatingOnly;
                                    }
                                    ModeFlagOn = false;
                                } else if (thisVar.EndUse == "COOLINGCOILS" && ModeFlagOn) {
                                    for (VarNum1 = 1; VarNum1 <= NumVariables; ++VarNum1) {
                                        thisEquipData.MeteredVar(VarNum1).EndUse_CompMode = CoolingOnly;
                                    }
                                    ModeFlagOn = false;
                                } else if (ModeFlagOn) {
                                    thisVar.EndUse_CompMode = NoHeatNoCool;
                                }
                                thisVar.Group = Groups(VarNum);
                            }
                        }

                        VarIndexes.deallocate();
                        VarTypes.deallocate();
                        IndexTypes.deallocate();
                        unitsForVar.deallocate();
                        ResourceTypes.deallocate();
                        EndUses.deallocate();
                        Groups.deallocate();
                        Names.deallocate();
                    }

                    if (IsParentObject(TypeOfComp, NameOfComp)) {
                        NumChildren = GetNumChildren(TypeOfComp, NameOfComp);
                        thisEquipData.NumSubEquip = NumChildren;

                        SubCompTypes.allocate(NumChildren);
                        SubCompNames.allocate(NumChildren);
                        InletNodeNames.allocate(NumChildren);
                        InletNodeNumbers.allocate(NumChildren);
                        OutletNodeNames.allocate(NumChildren);
                        OutletNodeNumbers.allocate(NumChildren);
                        thisEquipData.SubEquipData.allocate(NumChildren);

                        GetChildrenData(TypeOfComp,
                                        NameOfComp,
                                        NumChildren,
                                        SubCompTypes,
                                        SubCompNames,
                                        InletNodeNames,
                                        InletNodeNumbers,
                                        OutletNodeNames,
                                        OutletNodeNumbers,
                                        ErrorsFound);

                        for (SubCompNum = 1; SubCompNum <= NumChildren; ++SubCompNum) {
                            thisEquipData.SubEquipData(SubCompNum).TypeOf = SubCompTypes(SubCompNum);
                            thisEquipData.SubEquipData(SubCompNum).Name = SubCompNames(SubCompNum);
                            thisEquipData.SubEquipData(SubCompNum).OutletNodeNum = OutletNodeNumbers(SubCompNum);
                            thisEquipData.SubEquipData(SubCompNum).InletNodeNum = InletNodeNumbers(SubCompNum);
                        }

                        SubCompTypes.deallocate();
                        SubCompNames.deallocate();
                        InletNodeNames.deallocate();
                        InletNodeNumbers.deallocate();
                        OutletNodeNames.deallocate();
                        OutletNodeNumbers.deallocate();
                    } else {
                        NumChildren = 0;
                    }

                    for (SubCompNum = 1; SubCompNum <= NumChildren; ++SubCompNum) {
                        TypeOfComp = thisEquipData.SubEquipData(SubCompNum).TypeOf;
                        NameOfComp = thisEquipData.SubEquipData(SubCompNum).Name;
                        if (IsParentObject(TypeOfComp, NameOfComp)) {
                            NumGrandChildren = GetNumChildren(TypeOfComp, NameOfComp);
                            thisEquipData.SubEquipData(SubCompNum).NumSubSubEquip = NumGrandChildren;
                            SubCompTypes.allocate(NumGrandChildren);
                            SubCompNames.allocate(NumGrandChildren);
                            InletNodeNames.allocate(NumGrandChildren);
                            InletNodeNumbers.allocate(NumGrandChildren);
                            OutletNodeNames.allocate(NumGrandChildren);
                            OutletNodeNumbers.allocate(NumGrandChildren);
                            thisEquipData.SubEquipData(SubCompNum).SubSubEquipData.allocate(NumGrandChildren);
                            // Sankar added the array number for EquipData
                            GetChildrenData(TypeOfComp,
                                            NameOfComp,
                                            NumGrandChildren,
                                            SubCompTypes,
                                            SubCompNames,
                                            InletNodeNames,
                                            InletNodeNumbers,
                                            OutletNodeNames,
                                            OutletNodeNumbers,
                                            ErrorsFound);

                            for (SubSubCompNum = 1; SubSubCompNum <= NumGrandChildren; ++SubSubCompNum) {
                                thisEquipData.SubEquipData(SubCompNum).SubSubEquipData(SubSubCompNum).TypeOf = SubCompTypes(SubSubCompNum);
                                thisEquipData.SubEquipData(SubCompNum).SubSubEquipData(SubSubCompNum).Name = SubCompNames(SubSubCompNum);
                                thisEquipData.SubEquipData(SubCompNum).SubSubEquipData(SubSubCompNum).OutletNodeNum =
                                    OutletNodeNumbers(SubSubCompNum);
                                thisEquipData.SubEquipData(SubCompNum).SubSubEquipData(SubSubCompNum).InletNodeNum = InletNodeNumbers(SubSubCompNum);
                            }
                            SubCompTypes.deallocate();
                            SubCompNames.deallocate();
                            InletNodeNames.deallocate();
                            InletNodeNumbers.deallocate();
                            OutletNodeNames.deallocate();
                            OutletNodeNumbers.deallocate();
                        } else {
                            NumGrandChildren = 0;
                        }
                    }
                }
            }
        }

        for (CtrlZoneNum = 1; CtrlZoneNum <= NumOfZones; ++CtrlZoneNum) {
            if (!ZoneEquipConfig(CtrlZoneNum).IsControlled) continue;
            for (CompNum = 1; CompNum <= ZoneEquipList(CtrlZoneNum).NumOfEquipTypes; ++CompNum) {
                for (SubCompNum = 1; SubCompNum <= ZoneEquipList(CtrlZoneNum).EquipData(CompNum).NumSubEquip; ++SubCompNum) {
                    {
                        auto &thisSubEquipData(ZoneEquipList(CtrlZoneNum).EquipData(CompNum).SubEquipData(SubCompNum));
                        TypeOfComp = thisSubEquipData.TypeOf;
                        NameOfComp = thisSubEquipData.Name;

                        NumVariables = GetNumMeteredVariables(TypeOfComp, NameOfComp);
                        thisSubEquipData.NumMeteredVars = NumVariables; // Sankar added this line
                        if (NumVariables > 0) {
                            VarIndexes.allocate(NumVariables);
                            VarTypes.allocate(NumVariables);
                            IndexTypes.allocate(NumVariables);
                            unitsForVar.allocate(NumVariables);
                            ResourceTypes.allocate(NumVariables);
                            EndUses.allocate(NumVariables);
                            Groups.allocate(NumVariables);
                            Names.allocate(NumVariables);
                            thisSubEquipData.MeteredVar.allocate(NumVariables);

                            GetMeteredVariables(TypeOfComp,
                                                NameOfComp,
                                                VarIndexes,
                                                VarTypes,
                                                IndexTypes,
                                                unitsForVar,
                                                ResourceTypes,
                                                EndUses,
                                                Groups,
                                                Names,
                                                NumFound);

                            ModeFlagOn = true;
                            for (VarNum = 1; VarNum <= NumVariables; ++VarNum) {
                                {
                                    auto &thisVar(thisSubEquipData.MeteredVar(VarNum));
                                    thisVar.ReportVarName = Names(VarNum);
                                    thisVar.ReportVarUnits = unitsForVar(VarNum);
                                    thisVar.ReportVarIndex = VarIndexes(VarNum);
                                    thisVar.ReportVarIndexType = IndexTypes(VarNum);
                                    thisVar.ReportVarType = VarTypes(VarNum);
                                    thisVar.ResourceType = ResourceTypes(VarNum);
                                    thisVar.EndUse = EndUses(VarNum);
                                    if (thisVar.EndUse == "HEATINGCOILS" && ModeFlagOn) {
                                        for (VarNum1 = 1; VarNum1 <= NumVariables; ++VarNum1) {
                                            thisSubEquipData.MeteredVar(VarNum1).EndUse_CompMode = HeatingOnly;
                                        }
                                        ModeFlagOn = false;
                                    } else if (thisVar.EndUse == "COOLINGCOILS" && ModeFlagOn) {
                                        for (VarNum1 = 1; VarNum1 <= NumVariables; ++VarNum1) {
                                            thisSubEquipData.MeteredVar(VarNum1).EndUse_CompMode = CoolingOnly;
                                        }
                                        ModeFlagOn = false;
                                    } else if (ModeFlagOn) {
                                        thisVar.EndUse_CompMode = NoHeatNoCool;
                                    }
                                    thisVar.Group = Groups(VarNum);
                                }
                            }

                            VarIndexes.deallocate();
                            VarTypes.deallocate();
                            IndexTypes.deallocate();
                            unitsForVar.deallocate();
                            ResourceTypes.deallocate();
                            EndUses.deallocate();
                            Groups.deallocate();
                            Names.deallocate();
                        }

                        for (SubSubCompNum = 1; SubSubCompNum <= thisSubEquipData.NumSubSubEquip; ++SubSubCompNum) {
                            TypeOfComp = thisSubEquipData.SubSubEquipData(SubSubCompNum).TypeOf;
                            NameOfComp = thisSubEquipData.SubSubEquipData(SubSubCompNum).Name;

                            NumVariables = GetNumMeteredVariables(TypeOfComp, NameOfComp);
                            thisSubEquipData.SubSubEquipData(SubSubCompNum).NumMeteredVars = NumVariables; // Sankar added this line
                            if (NumVariables > 0) {
                                VarIndexes.allocate(NumVariables);
                                VarTypes.allocate(NumVariables);
                                IndexTypes.allocate(NumVariables);
                                unitsForVar.allocate(NumVariables);
                                ResourceTypes.allocate(NumVariables);
                                EndUses.allocate(NumVariables);
                                Groups.allocate(NumVariables);
                                Names.allocate(NumVariables);
                                thisSubEquipData.SubSubEquipData(SubSubCompNum).MeteredVar.allocate(NumVariables);

                                GetMeteredVariables(TypeOfComp,
                                                    NameOfComp,
                                                    VarIndexes,
                                                    VarTypes,
                                                    IndexTypes,
                                                    unitsForVar,
                                                    ResourceTypes,
                                                    EndUses,
                                                    Groups,
                                                    Names,
                                                    NumFound);

                                ModeFlagOn = true;
                                for (VarNum = 1; VarNum <= NumVariables; ++VarNum) {
                                    {
                                        auto &thisVar(thisSubEquipData.SubSubEquipData(SubSubCompNum).MeteredVar(VarNum));
                                        thisVar.ReportVarName = Names(VarNum);
                                        thisVar.ReportVarUnits = unitsForVar(VarNum);
                                        thisVar.ReportVarIndex = VarIndexes(VarNum);
                                        thisVar.ReportVarIndexType = IndexTypes(VarNum);
                                        thisVar.ReportVarType = VarTypes(VarNum);
                                        thisVar.ResourceType = ResourceTypes(VarNum);
                                        thisVar.EndUse = EndUses(VarNum);
                                        if (thisVar.EndUse == "HEATINGCOILS" && ModeFlagOn) {
                                            for (VarNum1 = 1; VarNum1 <= NumVariables; ++VarNum1) {
                                                thisSubEquipData.SubSubEquipData(SubSubCompNum).MeteredVar(VarNum1).EndUse_CompMode = HeatingOnly;
                                            }
                                            ModeFlagOn = false;
                                        } else if (thisVar.EndUse == "COOLINGCOILS" && ModeFlagOn) {
                                            for (VarNum1 = 1; VarNum1 <= NumVariables; ++VarNum1) {
                                                thisSubEquipData.SubSubEquipData(SubSubCompNum).MeteredVar(VarNum1).EndUse_CompMode = CoolingOnly;
                                            }
                                            ModeFlagOn = false;
                                        } else if (ModeFlagOn) {
                                            thisVar.EndUse_CompMode = NoHeatNoCool;
                                        }
                                        thisVar.Group = Groups(VarNum);
                                    }
                                }

                                VarIndexes.deallocate();
                                VarTypes.deallocate();
                                IndexTypes.deallocate();
                                unitsForVar.deallocate();
                                ResourceTypes.deallocate();
                                EndUses.deallocate();
                                Groups.deallocate();
                                Names.deallocate();
                            }
                        }
                    }
                }
            }
        }

        //***Plant Loops

        // previously, four separate huge DO loops all looking very very similar were used here
        // each individual block would operate on a single type of loop-side (plant demand, cond supply, etc.)
        // now, a bigger DO loop is applied iterating over all loops
        // a pointer (ThisReportData) is then directed to a particular item in the appropriate array
        // by operating on the pointer directly, we are actually operating on the item in the TARGET array item
        // in making this change, over 700 lines of code were dropped down to a single block

        for (PlantLoopNum = 1; PlantLoopNum <= NumPlantLoops + NumCondLoops; ++PlantLoopNum) {
            for (LoopSideNum = DemandSide; LoopSideNum <= SupplySide; ++LoopSideNum) {

                // Report selection
                ReportLoopData *select_ThisReportData(nullptr);

                if (PlantLoopNum <= NumPlantLoops) {
                    {
                        auto const SELECT_CASE_var(LoopSideNum);
                        if (SELECT_CASE_var == DemandSide) {
                            select_ThisReportData = &VentRepPlantDemandSide(PlantLoopNum);
                        } else if (SELECT_CASE_var == SupplySide) {
                            select_ThisReportData = &VentRepPlantSupplySide(PlantLoopNum);
                        } else {
                            assert(false);
                        }
                    }
                } else { // CondLoop
                    {
                        auto const SELECT_CASE_var(LoopSideNum);
                        if (SELECT_CASE_var == DemandSide) {
                            select_ThisReportData = &VentRepCondDemandSide(PlantLoopNum - NumPlantLoops);
                        } else if (SELECT_CASE_var == SupplySide) {
                            select_ThisReportData = &VentRepCondSupplySide(PlantLoopNum - NumPlantLoops);
                        } else {
                            assert(false);
                        }
                    }
                }

                // Object Data
                ReportLoopData &ThisReportData(*select_ThisReportData);

                for (BranchNum = 1; BranchNum <= ThisReportData.TotalBranches; ++BranchNum) {
                    for (CompNum = 1; CompNum <= ThisReportData.Branch(BranchNum).TotalComponents; ++CompNum) {
                        {
                            auto &thisComp(ThisReportData.Branch(BranchNum).Comp(CompNum));
                            TypeOfComp = thisComp.TypeOf;
                            NameOfComp = thisComp.Name;
                            // Get complete list of components for complex branches
                            if (IsParentObject(TypeOfComp, NameOfComp)) {

                                NumChildren = GetNumChildren(TypeOfComp, NameOfComp);

                                SubCompTypes.allocate(NumChildren);
                                SubCompNames.allocate(NumChildren);
                                InletNodeNames.allocate(NumChildren);
                                InletNodeNumbers.allocate(NumChildren);
                                OutletNodeNames.allocate(NumChildren);
                                OutletNodeNumbers.allocate(NumChildren);
                                thisComp.SubComp.allocate(NumChildren);

                                GetChildrenData(TypeOfComp,
                                                NameOfComp,
                                                NumChildren,
                                                SubCompTypes,
                                                SubCompNames,
                                                InletNodeNames,
                                                InletNodeNumbers,
                                                OutletNodeNames,
                                                OutletNodeNumbers,
                                                ErrorsFound);

                                for (SubCompNum = 1; SubCompNum <= NumChildren; ++SubCompNum) {
                                    thisComp.SubComp(SubCompNum).TypeOf = SubCompTypes(SubCompNum);
                                    thisComp.SubComp(SubCompNum).Name = SubCompNames(SubCompNum);
                                    thisComp.SubComp(SubCompNum).NodeNameIn = InletNodeNames(SubCompNum);
                                    thisComp.SubComp(SubCompNum).NodeNameOut = OutletNodeNames(SubCompNum);
                                    thisComp.SubComp(SubCompNum).NodeNumIn = InletNodeNumbers(SubCompNum);
                                    thisComp.SubComp(SubCompNum).NodeNumOut = OutletNodeNumbers(SubCompNum);
                                }

                                SubCompTypes.deallocate();
                                SubCompNames.deallocate();
                                InletNodeNames.deallocate();
                                InletNodeNumbers.deallocate();
                                OutletNodeNames.deallocate();
                                OutletNodeNumbers.deallocate();

                            } else {
                                NumChildren = 0;
                            }
                            thisComp.NumSubComps = NumChildren;

                            // check for 'grandchildren'
                            for (SubCompNum = 1; SubCompNum <= NumChildren; ++SubCompNum) {
                                TypeOfComp = thisComp.SubComp(SubCompNum).TypeOf;
                                NameOfComp = thisComp.SubComp(SubCompNum).Name;
                                if (IsParentObject(TypeOfComp, NameOfComp)) {
                                    NumGrandChildren = GetNumChildren(TypeOfComp, NameOfComp);
                                    SubCompTypes.allocate(NumGrandChildren);
                                    SubCompNames.allocate(NumGrandChildren);
                                    InletNodeNames.allocate(NumGrandChildren);
                                    InletNodeNumbers.allocate(NumGrandChildren);
                                    OutletNodeNames.allocate(NumGrandChildren);
                                    OutletNodeNumbers.allocate(NumGrandChildren);
                                    thisComp.SubComp(SubCompNum).SubSubComp.allocate(NumGrandChildren);

                                    GetChildrenData(TypeOfComp,
                                                    NameOfComp,
                                                    NumGrandChildren,
                                                    SubCompTypes,
                                                    SubCompNames,
                                                    InletNodeNames,
                                                    InletNodeNumbers,
                                                    OutletNodeNames,
                                                    OutletNodeNumbers,
                                                    ErrorsFound);

                                    for (SubSubCompNum = 1; SubSubCompNum <= NumGrandChildren; ++SubSubCompNum) {
                                        {
                                            auto &thisSubSubComp(thisComp.SubComp(SubCompNum).SubSubComp(SubSubCompNum));
                                            thisSubSubComp.TypeOf = SubCompTypes(SubSubCompNum);
                                            thisSubSubComp.Name = SubCompNames(SubSubCompNum);
                                            thisSubSubComp.NodeNameIn = InletNodeNames(SubSubCompNum);
                                            thisSubSubComp.NodeNameOut = OutletNodeNames(SubSubCompNum);
                                            thisSubSubComp.NodeNumIn = InletNodeNumbers(SubSubCompNum);
                                            thisSubSubComp.NodeNumOut = OutletNodeNumbers(SubSubCompNum);
                                        }
                                    }

                                    SubCompTypes.deallocate();
                                    SubCompNames.deallocate();
                                    InletNodeNames.deallocate();
                                    InletNodeNumbers.deallocate();
                                    OutletNodeNames.deallocate();
                                    OutletNodeNumbers.deallocate();
                                } else {
                                    NumGrandChildren = 0;
                                    thisComp.SubComp(SubCompNum).Parent = false;
                                }

                                thisComp.SubComp(SubCompNum).NumSubSubComps = NumGrandChildren;
                            }
                        }
                    }
                }
            }
        }

        for (PlantLoopNum = 1; PlantLoopNum <= NumPlantLoops + NumCondLoops; ++PlantLoopNum) {

            for (LoopSideNum = DemandSide; LoopSideNum <= SupplySide; ++LoopSideNum) {

                // Report selection
                ReportLoopData *select_ThisReportData(nullptr);

                if (PlantLoopNum <= NumPlantLoops) {
                    {
                        auto const SELECT_CASE_var(LoopSideNum);
                        if (SELECT_CASE_var == DemandSide) {
                            select_ThisReportData = &VentRepPlantDemandSide(PlantLoopNum);
                        } else if (SELECT_CASE_var == SupplySide) {
                            select_ThisReportData = &VentRepPlantSupplySide(PlantLoopNum);
                        } else {
                            assert(false);
                        }
                    }
                } else { // CondLoop
                    {
                        auto const SELECT_CASE_var(LoopSideNum);
                        if (SELECT_CASE_var == DemandSide) {
                            select_ThisReportData = &VentRepCondDemandSide(PlantLoopNum - NumPlantLoops);
                        } else if (SELECT_CASE_var == SupplySide) {
                            select_ThisReportData = &VentRepCondSupplySide(PlantLoopNum - NumPlantLoops);
                        } else {
                            assert(false);
                        }
                    }
                }

                // Object Data
                ReportLoopData &ThisReportData(*select_ThisReportData);

                for (BranchNum = 1; BranchNum <= ThisReportData.TotalBranches; ++BranchNum) {
                    for (CompNum = 1; CompNum <= ThisReportData.Branch(BranchNum).TotalComponents; ++CompNum) {
                        // Get complete list of components for complex branches
                        {
                            auto &thisComp(ThisReportData.Branch(BranchNum).Comp(CompNum));
                            TypeOfComp = thisComp.TypeOf;
                            NameOfComp = thisComp.Name;
                            NumVariables = GetNumMeteredVariables(TypeOfComp, NameOfComp);
                            if (NumVariables > 0) {
                                VarIndexes.allocate(NumVariables);
                                VarTypes.allocate(NumVariables);
                                IndexTypes.allocate(NumVariables);
                                unitsForVar.allocate(NumVariables);
                                ResourceTypes.allocate(NumVariables);
                                EndUses.allocate(NumVariables);
                                Groups.allocate(NumVariables);
                                Names.allocate(NumVariables);
                                thisComp.MeteredVar.allocate(NumVariables);

                                thisComp.NumMeteredVars = NumVariables;
                                GetMeteredVariables(TypeOfComp,
                                                    NameOfComp,
                                                    VarIndexes,
                                                    VarTypes,
                                                    IndexTypes,
                                                    unitsForVar,
                                                    ResourceTypes,
                                                    EndUses,
                                                    Groups,
                                                    Names,
                                                    NumFound);

                                ModeFlagOn = true;
                                for (VarNum = 1; VarNum <= NumVariables; ++VarNum) {
                                    {
                                        auto &thisVar(thisComp.MeteredVar(VarNum));
                                        thisVar.ReportVarName = Names(VarNum);
                                        thisVar.ReportVarUnits = unitsForVar(VarNum);
                                        thisVar.ReportVarIndex = VarIndexes(VarNum);
                                        thisVar.ReportVarIndexType = IndexTypes(VarNum);
                                        thisVar.ReportVarType = VarTypes(VarNum);
                                        thisVar.ResourceType = ResourceTypes(VarNum);
                                        thisVar.EndUse = EndUses(VarNum);
                                        if (thisVar.EndUse == "HEATINGCOILS" && ModeFlagOn) {
                                            for (VarNum1 = 1; VarNum1 <= NumVariables; ++VarNum1) {
                                                thisComp.MeteredVar(VarNum1).EndUse_CompMode = HeatingOnly;
                                            }
                                            ModeFlagOn = false;
                                        } else if (thisVar.EndUse == "COOLINGCOILS" && ModeFlagOn) {
                                            for (VarNum1 = 1; VarNum1 <= NumVariables; ++VarNum1) {
                                                thisComp.MeteredVar(VarNum1).EndUse_CompMode = CoolingOnly;
                                            }
                                            ModeFlagOn = false;
                                        } else if (ModeFlagOn) {
                                            thisVar.EndUse_CompMode = NoHeatNoCool;
                                        }
                                        thisVar.Group = Groups(VarNum);
                                    }
                                }

                                VarIndexes.deallocate();
                                VarTypes.deallocate();
                                IndexTypes.deallocate();
                                unitsForVar.deallocate();
                                ResourceTypes.deallocate();
                                EndUses.deallocate();
                                Groups.deallocate();
                                Names.deallocate();
                            }
                            for (SubCompNum = 1; SubCompNum <= thisComp.NumSubComps; ++SubCompNum) {
                                // Get complete list of components for complex branches
                                TypeOfComp = thisComp.SubComp(SubCompNum).TypeOf;
                                NameOfComp = thisComp.SubComp(SubCompNum).Name;
                                NumVariables = GetNumMeteredVariables(TypeOfComp, NameOfComp);
                                if (NumVariables > 0) {
                                    VarIndexes.allocate(NumVariables);
                                    VarTypes.allocate(NumVariables);
                                    IndexTypes.allocate(NumVariables);
                                    unitsForVar.allocate(NumVariables);
                                    ResourceTypes.allocate(NumVariables);
                                    EndUses.allocate(NumVariables);
                                    Groups.allocate(NumVariables);
                                    Names.allocate(NumVariables);
                                    thisComp.SubComp(SubCompNum).MeteredVar.allocate(NumVariables);

                                    GetMeteredVariables(TypeOfComp,
                                                        NameOfComp,
                                                        VarIndexes,
                                                        VarTypes,
                                                        IndexTypes,
                                                        unitsForVar,
                                                        ResourceTypes,
                                                        EndUses,
                                                        Groups,
                                                        Names,
                                                        NumFound);

                                    ModeFlagOn = true;
                                    for (VarNum = 1; VarNum <= NumVariables; ++VarNum) {
                                        {
                                            auto &thisVar(thisComp.SubComp(SubCompNum).MeteredVar(VarNum));
                                            thisVar.ReportVarName = Names(VarNum);
                                            thisVar.ReportVarUnits = unitsForVar(VarNum);
                                            thisVar.ReportVarIndex = VarIndexes(VarNum);
                                            thisVar.ReportVarIndexType = IndexTypes(VarNum);
                                            thisVar.ReportVarType = VarTypes(VarNum);
                                            thisVar.ResourceType = ResourceTypes(VarNum);
                                            thisVar.EndUse = EndUses(VarNum);
                                            if (thisVar.EndUse == "HEATINGCOILS" && ModeFlagOn) {
                                                for (VarNum1 = 1; VarNum1 <= NumVariables; ++VarNum1) {
                                                    thisComp.SubComp(SubCompNum).MeteredVar(VarNum1).EndUse_CompMode = HeatingOnly;
                                                }
                                                ModeFlagOn = false;
                                            } else if (thisVar.EndUse == "COOLINGCOILS" && ModeFlagOn) {
                                                for (VarNum1 = 1; VarNum1 <= NumVariables; ++VarNum1) {
                                                    thisComp.SubComp(SubCompNum).MeteredVar(VarNum1).EndUse_CompMode = CoolingOnly;
                                                }
                                                ModeFlagOn = false;
                                            } else if (ModeFlagOn) {
                                                thisVar.EndUse_CompMode = NoHeatNoCool;
                                            }
                                            thisVar.Group = Groups(VarNum);
                                        }
                                    }

                                    VarIndexes.deallocate();
                                    VarTypes.deallocate();
                                    IndexTypes.deallocate();
                                    unitsForVar.deallocate();
                                    ResourceTypes.deallocate();
                                    EndUses.deallocate();
                                    Groups.deallocate();
                                    Names.deallocate();
                                }
                                thisComp.SubComp(SubCompNum).NumMeteredVars = NumVariables;
                            }
                        }
                    }
                }
            }
        }
    }

    // End Initialization Section of the Module
    //******************************************************************************

    // Beginning of Reporting subroutines for the SimAir Module
    // *****************************************************************************

    void ReportSystemEnergyUse()
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Dan Fisher
        //       DATE WRITTEN   November 2005
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // calculate and report system loads and energy

        // METHODOLOGY EMPLOYED:
        // Accumulate meter data to appropriate report variables

        // REFERENCES:
        // na

        // Using/Aliasing
        using namespace DataZoneEnergyDemands;
        using Psychrometrics::PsyHFnTdbW;
        using namespace DataGlobalConstants;

        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:
        // na

        // SUBROUTINE PARAMETER DEFINITIONS:
        // na

        // INTERFACE BLOCK SPECIFICATIONS
        // na

        // DERIVED TYPE DEFINITIONS
        // na

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int Idx;          // loop counter
        int nodes;        // loop counter
        int CtrlZoneNum;  // ZONE counter
        int ZoneInNum;    // counter for zone air distribution inlets
        int AirLoopNum;   // counter for zone air distribution inlets
        int BranchNum;    // counter for zone air distribution inlets
        int EquipListNum; // counter for zone air distribution inlets
        int VarNum;       // counter for zone air distribution inlets
        int CompNum;
        int SubCompNum;
        int SubSubCompNum;
        int CompMode;
        int InletNodeNum;
        int OutletNodeNum;
        int ADUNum;
        int ADUCoolNum;
        int ADUHeatNum;
        int AirDistCoolInletNodeNum;
        int AirDistHeatInletNodeNum;
        int EnergyType;
        int ActualZoneNum;
        Real64 CompEnergyUse;
        Real64 ZoneLoad;
        Real64 CompLoad;
        Real64 ADUCoolFlowrate;
        Real64 ADUHeatFlowrate;
        bool CompLoadFlag;

        if (!AirLoopLoadsReportEnabled) return;

        // SYSTEM LOADS REPORT
        SysTotZoneLoadHTNG = 0.0;
        SysTotZoneLoadCLNG = 0.0;
        SysOALoadHTNG = 0.0;
        SysOALoadCLNG = 0.0;
        SysTotHTNG = 0.0;
        SysTotCLNG = 0.0;

        // SYSTEM ENERGY USE REPORT
        SysTotElec = 0.0;
        SysTotNaturalGas = 0.0;
        SysTotPropane = 0.0;
        SysTotSteam = 0.0;
        SysTotH2OCOLD = 0.0;
        SysTotH2OHOT = 0.0;

        // SYSTEM COMPONENT LOADS REPORT
        SysFANCompHTNG = 0.0;
        SysCCCompCLNG = 0.0;
        SysHCCompHTNG = 0.0;
        SysHeatExHTNG = 0.0;
        SysHeatExCLNG = 0.0;
        SysSolarCollectHeating = 0.0;
        SysSolarCollectCooling = 0.0;
        SysUserDefinedTerminalHeating = 0.0;
        SysUserDefinedTerminalCooling = 0.0;
        SysHumidHTNG = 0.0;
        SysEvapCLNG = 0.0;
        DesDehumidCLNG = 0.0;
        SysDomesticH2O = 0.0;

        // SYSTEM COMPONENT ENERGY REPORT
        SysFANCompElec = 0.0;
        SysHCCompH2OHOT = 0.0;
        SysCCCompH2OCOLD = 0.0;
        SysHCCompElec = 0.0;
        SysCCCompElec = 0.0;
        SysHCCompElecRes = 0.0;
        SysHCCompNaturalGas = 0.0;
        SysHCCompPropane = 0.0;
        SysHCCompSteam = 0.0;
        SysHumidElec = 0.0;
        SysHumidNaturalGas = 0.0;
        SysHumidPropane = 0.0;
        DesDehumidElec = 0.0;
        SysEvapElec = 0.0;

        for (AirLoopNum = 1; AirLoopNum <= NumPrimaryAirSys; ++AirLoopNum) {
            auto const &pas = PrimaryAirSystem(AirLoopNum);
            for (BranchNum = 1; BranchNum <= pas.NumBranches; ++BranchNum) {
                auto const &pasBranch = pas.Branch(BranchNum);
                if (Node(pasBranch.NodeNumOut).MassFlowRate <= 0.0) continue;
                for (CompNum = 1; CompNum <= pasBranch.TotalComponents; ++CompNum) {
                    auto const &pasBranchComp = pasBranch.Comp(CompNum);
                    InletNodeNum = pasBranchComp.NodeNumIn;
                    OutletNodeNum = pasBranchComp.NodeNumOut;
                    if (InletNodeNum <= 0 || OutletNodeNum <= 0) continue;
                    CompLoad = Node(OutletNodeNum).MassFlowRate * (PsyHFnTdbW(Node(InletNodeNum).Temp, Node(InletNodeNum).HumRat) -
                                                                   PsyHFnTdbW(Node(OutletNodeNum).Temp, Node(OutletNodeNum).HumRat));
                    CompLoad *= TimeStepSys * SecInHour;
                    CompEnergyUse = 0.0;
                    EnergyType = iRT_None;
                    CompLoadFlag = true;
                    CalcSystemEnergyUse(CompLoadFlag, AirLoopNum, pasBranchComp.TypeOf, EnergyType, CompLoad, CompEnergyUse);
                    CompLoadFlag = false;
                    for (VarNum = 1; VarNum <= pasBranchComp.NumMeteredVars; ++VarNum) {
                        auto const &pasBranchCompMeter = pasBranchComp.MeteredVar(VarNum);
                        CompMode = pasBranchCompMeter.EndUse_CompMode;
                        CompEnergyUse = pasBranchCompMeter.CurMeterReading;
                        EnergyType = pasBranchCompMeter.ResourceType;
                        CalcSystemEnergyUse(CompLoadFlag, AirLoopNum, pasBranchComp.TypeOf, EnergyType, CompLoad, CompEnergyUse);
                    }

                    for (SubCompNum = 1; SubCompNum <= pasBranchComp.NumSubComps; ++SubCompNum) {
                        auto const &pasBranchSubComp = pasBranchComp.SubComp(SubCompNum);
                        InletNodeNum = pasBranchSubComp.NodeNumIn;
                        OutletNodeNum = pasBranchSubComp.NodeNumOut;
                        if (InletNodeNum <= 0 || OutletNodeNum <= 0) continue;
                        CompLoad = Node(OutletNodeNum).MassFlowRate * (PsyHFnTdbW(Node(InletNodeNum).Temp, Node(InletNodeNum).HumRat) -
                                                                       PsyHFnTdbW(Node(OutletNodeNum).Temp, Node(OutletNodeNum).HumRat));
                        CompLoad *= TimeStepSys * SecInHour;
                        CompEnergyUse = 0.0;
                        EnergyType = iRT_None;
                        CompLoadFlag = true;
                        CalcSystemEnergyUse(CompLoadFlag, AirLoopNum, pasBranchSubComp.TypeOf, EnergyType, CompLoad, CompEnergyUse);
                        CompLoadFlag = false;
                        for (VarNum = 1; VarNum <= pasBranchSubComp.NumMeteredVars; ++VarNum) {
                            auto const &pasBranchSubCompMeter = pasBranchSubComp.MeteredVar(VarNum);
                            CompMode = pasBranchSubCompMeter.EndUse_CompMode;
                            CompEnergyUse = pasBranchSubCompMeter.CurMeterReading;
                            EnergyType = pasBranchSubCompMeter.ResourceType;
                            CalcSystemEnergyUse(CompLoadFlag, AirLoopNum, pasBranchSubComp.TypeOf, EnergyType, CompLoad, CompEnergyUse);
                        }

                        for (SubSubCompNum = 1; SubSubCompNum <= pasBranchSubComp.NumSubSubComps; ++SubSubCompNum) {
                            auto const &pasBranchSubSubComp = pasBranchSubComp.SubSubComp(SubSubCompNum);
                            InletNodeNum = pasBranchSubSubComp.NodeNumIn;
                            OutletNodeNum = pasBranchSubSubComp.NodeNumOut;
                            if (InletNodeNum <= 0 || OutletNodeNum <= 0) continue;
                            CompLoad = Node(OutletNodeNum).MassFlowRate * (PsyHFnTdbW(Node(InletNodeNum).Temp, Node(InletNodeNum).HumRat) -
                                                                           PsyHFnTdbW(Node(OutletNodeNum).Temp, Node(OutletNodeNum).HumRat));
                            CompLoad *= TimeStepSys * SecInHour;
                            CompEnergyUse = 0.0;
                            EnergyType = iRT_None;
                            CompLoadFlag = true;
                            CalcSystemEnergyUse(CompLoadFlag, AirLoopNum, pasBranchSubSubComp.TypeOf, EnergyType, CompLoad, CompEnergyUse);
                            CompLoadFlag = false;
                            for (VarNum = 1; VarNum <= pasBranchSubSubComp.NumMeteredVars; ++VarNum) {
                                auto const &pasBranchSubSubCompMeter = pasBranchSubSubComp.MeteredVar(VarNum);
                                CompMode = pasBranchSubSubCompMeter.EndUse_CompMode;
                                CompEnergyUse = pasBranchSubSubCompMeter.CurMeterReading;
                                EnergyType = pasBranchSubSubCompMeter.ResourceType;
                                CalcSystemEnergyUse(CompLoadFlag, AirLoopNum, pasBranchSubSubComp.TypeOf, EnergyType, CompLoad, CompEnergyUse);
                            }
                        }
                    }
                }
            }
        }

        for (CtrlZoneNum = 1; CtrlZoneNum <= NumOfZones; ++CtrlZoneNum) {
            auto const &zecCtrlZone = ZoneEquipConfig(CtrlZoneNum);
            if (!zecCtrlZone.IsControlled) continue;

            // retrieve the zone load for each zone
            ActualZoneNum = zecCtrlZone.ActualZoneNum;
            ZoneLoad = ZoneSysEnergyDemand(ActualZoneNum).TotalOutputRequired;

            // if system operating in deadband reset zone load
            if (DeadBandOrSetback(ActualZoneNum)) ZoneLoad = 0.0;

            // loop over the zone supply air path inlet nodes
            for (ZoneInNum = 1; ZoneInNum <= zecCtrlZone.NumInletNodes; ++ZoneInNum) {
                // retrieve air loop indexes
                AirLoopNum = zecCtrlZone.InletNodeAirLoopNum(ZoneInNum);
                if (AirLoopNum == 0) continue;

                // Zone cooling load - this will double count if there is more than one airloop serving the same zone - but not sure how to apportion
                if (ZoneLoad < -SmallLoad) {
                    SysTotZoneLoadCLNG(AirLoopNum) += std::abs(ZoneLoad);

                    // Zone heating load
                } else if (ZoneLoad > SmallLoad) {
                    SysTotZoneLoadHTNG(AirLoopNum) += std::abs(ZoneLoad);
                }
                auto const &zecCtrlZoneCool = zecCtrlZone.AirDistUnitCool(ZoneInNum);
                auto const &zecCtrlZoneHeat = zecCtrlZone.AirDistUnitHeat(ZoneInNum);

                AirDistCoolInletNodeNum = max(zecCtrlZoneCool.InNode, 0);
                AirDistHeatInletNodeNum = max(zecCtrlZoneHeat.InNode, 0);

                // Set for cooling or heating path
                if (AirDistCoolInletNodeNum > 0 && AirDistHeatInletNodeNum == 0) {
                    ADUCoolFlowrate = max(Node(zecCtrlZoneCool.InNode).MassFlowRate, 0.0);
                } else if (AirDistHeatInletNodeNum > 0 && AirDistCoolInletNodeNum == 0) {
                    ADUHeatFlowrate = max(Node(zecCtrlZoneHeat.InNode).MassFlowRate, 0.0);
                } else {
                    ADUCoolFlowrate = 0.0;
                    ADUHeatFlowrate = 0.0;
                }

                EquipListNum = zecCtrlZone.EquipListIndex;
                auto const &zel = ZoneEquipList(EquipListNum);

                for (Idx = 1; Idx <= 2; ++Idx) {
                    if (Idx == 1) {
                        ADUCoolNum = max(zecCtrlZoneCool.AirDistUnitIndex, 0);
                        if (ADUCoolNum == 0) continue;
                        ADUNum = ADUCoolNum;
                    } else { //(Idx =2)THEN
                        ADUHeatNum = max(zecCtrlZoneHeat.AirDistUnitIndex, 0);
                        if (ADUHeatNum == 0) continue;
                        ADUNum = ADUHeatNum;
                    }

                    auto const &zelEquipData = zel.EquipData(ADUNum);

                    CompLoad = 0.0;
                    if (zelEquipData.NumInlets > 0) {
                        for (nodes = 1; nodes <= zelEquipData.NumInlets; ++nodes) {
                            InletNodeNum = zelEquipData.InletNodeNums(Idx);
                            CompLoad += (PsyHFnTdbW(Node(InletNodeNum).Temp, Node(InletNodeNum).HumRat) * Node(InletNodeNum).MassFlowRate);
                        }
                        for (nodes = 1; nodes <= zelEquipData.NumOutlets; ++nodes) {
                            OutletNodeNum = zelEquipData.OutletNodeNums(Idx);
                            CompLoad -= (PsyHFnTdbW(Node(OutletNodeNum).Temp, Node(OutletNodeNum).HumRat) * Node(OutletNodeNum).MassFlowRate);
                        }
                    }
                    CompLoad *= TimeStepSys * SecInHour;
                    CompEnergyUse = 0.0;
                    EnergyType = iRT_None;
                    CompLoadFlag = true;
                    CalcSystemEnergyUse(CompLoadFlag, AirLoopNum, zelEquipData.TypeOf, EnergyType, CompLoad, CompEnergyUse);
                    CompLoadFlag = false;
                    for (VarNum = 1; VarNum <= zelEquipData.NumMeteredVars; ++VarNum) {
                        CompEnergyUse = zelEquipData.MeteredVar(VarNum).CurMeterReading;
                        EnergyType = zelEquipData.MeteredVar(VarNum).ResourceType;
                        CalcSystemEnergyUse(CompLoadFlag, AirLoopNum, zelEquipData.TypeOf, EnergyType, CompLoad, CompEnergyUse);
                    }

                    for (SubCompNum = 1; SubCompNum <= zelEquipData.NumSubEquip; ++SubCompNum) {
                        auto const &zelSubEquipData = zelEquipData.SubEquipData(SubCompNum);
                        InletNodeNum = zelSubEquipData.InletNodeNum;
                        OutletNodeNum = zelSubEquipData.OutletNodeNum;
                        if (InletNodeNum <= 0 || OutletNodeNum <= 0) continue;
                        CompLoad = Node(InletNodeNum).MassFlowRate * (PsyHFnTdbW(Node(InletNodeNum).Temp, Node(InletNodeNum).HumRat) -
                                                                      PsyHFnTdbW(Node(OutletNodeNum).Temp, Node(OutletNodeNum).HumRat));
                        CompLoad *= TimeStepSys * SecInHour;
                        CompEnergyUse = 0.0;
                        EnergyType = iRT_None;
                        CompLoadFlag = true;
                        CalcSystemEnergyUse(CompLoadFlag, AirLoopNum, zelSubEquipData.TypeOf, EnergyType, CompLoad, CompEnergyUse);
                        CompLoadFlag = false;
                        for (VarNum = 1; VarNum <= zelSubEquipData.NumMeteredVars; ++VarNum) {
                            CompEnergyUse = zelSubEquipData.MeteredVar(VarNum).CurMeterReading;
                            EnergyType = zelSubEquipData.MeteredVar(VarNum).ResourceType;
                            CalcSystemEnergyUse(CompLoadFlag, AirLoopNum, zelSubEquipData.TypeOf, EnergyType, CompLoad, CompEnergyUse);
                        }

                        for (SubSubCompNum = 1; SubSubCompNum <= zelSubEquipData.NumSubSubEquip; ++SubSubCompNum) {
                            auto const &zelSubSubEquipData = zelSubEquipData.SubSubEquipData(SubSubCompNum);
                            InletNodeNum = zelSubSubEquipData.InletNodeNum;
                            OutletNodeNum = zelSubSubEquipData.OutletNodeNum;
                            if (InletNodeNum <= 0 || OutletNodeNum <= 0) continue;
                            CompLoad = Node(InletNodeNum).MassFlowRate * (PsyHFnTdbW(Node(InletNodeNum).Temp, Node(InletNodeNum).HumRat) -
                                                                          PsyHFnTdbW(Node(OutletNodeNum).Temp, Node(OutletNodeNum).HumRat));
                            CompLoad *= TimeStepSys * SecInHour;
                            CompEnergyUse = 0.0;
                            EnergyType = iRT_None;
                            CompLoadFlag = true;
                            CalcSystemEnergyUse(CompLoadFlag, AirLoopNum, zelSubSubEquipData.TypeOf, EnergyType, CompLoad, CompEnergyUse);
                            CompLoadFlag = false;
                            for (VarNum = 1; VarNum <= zelSubSubEquipData.NumMeteredVars; ++VarNum) {
                                CompEnergyUse = zelSubSubEquipData.MeteredVar(VarNum).CurMeterReading;
                                EnergyType = zelSubSubEquipData.MeteredVar(VarNum).ResourceType;
                                CalcSystemEnergyUse(CompLoadFlag, AirLoopNum, zelSubSubEquipData.TypeOf, EnergyType, CompLoad, CompEnergyUse);
                            }
                        } // SubSubCompNum
                    }     // SubCompNum
                }         // Idx
            }             // ZoneInNum
        }                 // Controlled Zone Loop

        for (AirLoopNum = 1; AirLoopNum <= NumPrimaryAirSys; ++AirLoopNum) {
            SysTotHTNG(AirLoopNum) = SysFANCompHTNG(AirLoopNum) + SysHCCompHTNG(AirLoopNum) + SysHeatExHTNG(AirLoopNum) + SysHumidHTNG(AirLoopNum) +
                                     SysSolarCollectHeating(AirLoopNum) + SysUserDefinedTerminalHeating(AirLoopNum);
            SysTotCLNG(AirLoopNum) = SysCCCompCLNG(AirLoopNum) + SysHeatExCLNG(AirLoopNum) + SysEvapCLNG(AirLoopNum) + DesDehumidCLNG(AirLoopNum) +
                                     SysSolarCollectCooling(AirLoopNum) + SysUserDefinedTerminalCooling(AirLoopNum);
            SysTotElec(AirLoopNum) = SysFANCompElec(AirLoopNum) + SysHCCompElec(AirLoopNum) + SysCCCompElec(AirLoopNum) +
                                     SysHCCompElecRes(AirLoopNum) + SysHumidElec(AirLoopNum) + DesDehumidElec(AirLoopNum) + SysEvapElec(AirLoopNum);
            SysTotNaturalGas(AirLoopNum) = SysHCCompNaturalGas(AirLoopNum) + SysHumidNaturalGas(AirLoopNum);
            SysTotPropane(AirLoopNum) = SysHCCompPropane(AirLoopNum) + SysHumidPropane(AirLoopNum);
            SysTotSteam(AirLoopNum) = SysHCCompSteam(AirLoopNum);
            SysTotH2OCOLD(AirLoopNum) = SysCCCompH2OCOLD(AirLoopNum);
            SysTotH2OHOT(AirLoopNum) = SysHCCompH2OHOT(AirLoopNum);
        }
    }

    void CalcSystemEnergyUse(bool const CompLoadFlag,
                             int const AirLoopNum,
                             std::string const &CompType,
                             int const EnergyType,
                             Real64 const CompLoad,
                             Real64 const CompEnergy)
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Dan Fisher
        //       DATE WRITTEN   Nov. 2005
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // accumulate system loads and energy to report variables

        // METHODOLOGY EMPLOYED:

        // REFERENCES:
        // na

        // Using/Aliasing
        using Psychrometrics::PsyHFnTdbW;
        using namespace DataZoneEnergyDemands;
        using namespace DataGlobalConstants;

        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:

        // SUBROUTINE PARAMETER DEFINITIONS:

        // Tuned String comparisons were a big performance hit
        // ComponentTypes and component_strings must remain in sync
        enum ComponentTypes
        { // Using older enum style to avoid the name scoping cruft
            AIRLOOPHVAC_OUTDOORAIRSYSTEM,
            AIRLOOPHVAC_UNITARY_FURNACE_HEATCOOL,
            AIRLOOPHVAC_UNITARY_FURNACE_HEATONLY,
            AIRLOOPHVAC_UNITARYHEATCOOL,
            AIRLOOPHVAC_UNITARYHEATCOOL_VAVCHANGEOVERBYPASS,
            AIRLOOPHVAC_UNITARYHEATONLY,
            AIRLOOPHVAC_UNITARYHEATPUMP_AIRTOAIR,
            AIRLOOPHVAC_UNITARYHEATPUMP_AIRTOAIR_MULTISPEED,
            AIRLOOPHVAC_UNITARYHEATPUMP_WATERTOAIR,
            AIRLOOPHVAC_UNITARYSYSTEM,
            AIRTERMINAL_DUALDUCT_CONSTANTVOLUME_COOL,
            AIRTERMINAL_DUALDUCT_CONSTANTVOLUME_HEAT,
            AIRTERMINAL_DUALDUCT_VAV_COOL,
            AIRTERMINAL_DUALDUCT_VAV_HEAT,
            AIRTERMINAL_DUALDUCT_VAV_OUTDOORAIR_OUTDOORAIR,
            AIRTERMINAL_DUALDUCT_VAV_OUTDOORAIR_RECIRCULATEDAIR,
            AIRTERMINAL_SINGLEDUCT_CONSTANTVOLUME_COOLEDBEAM,
            AIRTERMINAL_SINGLEDUCT_CONSTANTVOLUME_FOURPIPEBEAM,
            AIRTERMINAL_SINGLEDUCT_CONSTANTVOLUME_FOURPIPEINDUCTION,
            AIRTERMINAL_SINGLEDUCT_CONSTANTVOLUME_REHEAT,
            AIRTERMINAL_SINGLEDUCT_CONSTANTVOLUME_NOREHEAT,
            AIRTERMINAL_SINGLEDUCT_MIXER,
            AIRTERMINAL_SINGLEDUCT_PARALLELPIU_REHEAT,
            AIRTERMINAL_SINGLEDUCT_SERIESPIU_REHEAT,
            AIRTERMINAL_SINGLEDUCT_USERDEFINED,
            AIRTERMINAL_SINGLEDUCT_VAV_HEATANDCOOL_NOREHEAT,
            AIRTERMINAL_SINGLEDUCT_VAV_HEATANDCOOL_REHEAT,
            AIRTERMINAL_SINGLEDUCT_VAV_NOREHEAT,
            AIRTERMINAL_SINGLEDUCT_VAV_REHEAT,
            AIRTERMINAL_SINGLEDUCT_VAV_REHEAT_VARIABLESPEEDFAN,
            COIL_COOLING_DX,
            COIL_COOLING_DX_MULTISPEED,
            COIL_COOLING_DX_SINGLESPEED,
            COIL_COOLING_DX_SINGLESPEED_THERMALSTORAGE,
            COIL_COOLING_DX_TWOSPEED,
            COIL_COOLING_DX_TWOSTAGEWITHHUMIDITYCONTROLMODE,
            COIL_COOLING_DX_VARIABLESPEED,
            COIL_INTEGRATED_DX_VARIABLESPEED,
            COIL_COOLING_WATER,
            COIL_COOLING_WATER_DETAILEDGEOMETRY,
            COIL_COOLING_WATERTOAIRHEATPUMP_EQUATIONFIT,
            COIL_COOLING_WATERTOAIRHEATPUMP_PARAMETERESTIMATION,
            COIL_COOLING_WATERTOAIRHEATPUMP_VARIABLESPEEDEQUATIONFIT,
            COIL_HEATING_DESUPERHEATER,
            COIL_HEATING_DX_MULTISPEED,
            COIL_HEATING_DX_SINGLESPEED,
            COIL_HEATING_DX_VARIABLESPEED,
            COIL_HEATING_ELECTRIC,
            COIL_HEATING_ELECTRIC_MULTISTAGE,
            COIL_HEATING_GAS,
            COIL_HEATING_GAS_MULTISTAGE,
            COIL_HEATING_STEAM,
            COIL_HEATING_WATER,
            COIL_HEATING_WATERTOAIRHEATPUMP_EQUATIONFIT,
            COIL_HEATING_WATERTOAIRHEATPUMP_PARAMETERESTIMATION,
            COIL_HEATING_WATERTOAIRHEATPUMP_VARIABLESPEEDEQUATIONFIT,
            COIL_WATERHEATING_AIRTOWATERHEATPUMP_VARIABLESPEED,
            COIL_USERDEFINED,
            COILSYSTEM_COOLING_DX,
            COILSYSTEM_COOLING_DX_HEATEXCHANGERASSISTED,
            COILSYSTEM_COOLING_WATER_HEATEXCHANGERASSISTED,
            COILSYSTEM_HEATING_DX,
            DEHUMIDIFIER_DESICCANT_NOFANS,
            DEHUMIDIFIER_DESICCANT_SYSTEM,
            DUCT,
            EVAPORATIVECOOLER_DIRECT_CELDEKPAD,
            EVAPORATIVECOOLER_DIRECT_RESEARCHSPECIAL,
            EVAPORATIVECOOLER_INDIRECT_CELDEKPAD,
            EVAPORATIVECOOLER_INDIRECT_RESEARCHSPECIAL,
            EVAPORATIVECOOLER_INDIRECT_WETCOIL,
            FAN_COMPONENTMODEL,
            FAN_SYSTEMMODEL,
            FAN_CONSTANTVOLUME,
            FAN_ONOFF,
            FAN_VARIABLEVOLUME,
            HEATEXCHANGER_AIRTOAIR_FLATPLATE,
            HEATEXCHANGER_AIRTOAIR_SENSIBLEANDLATENT,
            HEATEXCHANGER_DESICCANT_BALANCEDFLOW,
            HUMIDIFIER_STEAM_ELECTRIC,
            HUMIDIFIER_STEAM_GAS,
            OUTDOORAIR_MIXER,
            SOLARCOLLECTOR_FLATPLATE_PHOTOVOLTAICTHERMAL,
            SOLARCOLLECTOR_UNGLAZEDTRANSPIRED,
            ZONEHVAC_AIRDISTRIBUTIONUNIT,
            ZONEHVAC_TERMINALUNIT_VRF,
            COIL_COOLING_VRF,
            COIL_HEATING_VRF,
            COIL_COOLING_VRF_FTC,
            COIL_HEATING_VRF_FTC,
            n_ComponentTypes,
            Unknown_ComponentType
        };

        static std::unordered_map<std::string, ComponentTypes> const component_map = {
            {"AIRLOOPHVAC:OUTDOORAIRSYSTEM", AIRLOOPHVAC_OUTDOORAIRSYSTEM},
            {"AIRLOOPHVAC:UNITARY:FURNACE:HEATCOOL", AIRLOOPHVAC_UNITARY_FURNACE_HEATCOOL},
            {"AIRLOOPHVAC:UNITARY:FURNACE:HEATONLY", AIRLOOPHVAC_UNITARY_FURNACE_HEATONLY},
            {"AIRLOOPHVAC:UNITARYHEATCOOL", AIRLOOPHVAC_UNITARYHEATCOOL},
            {"AIRLOOPHVAC:UNITARYHEATCOOL:VAVCHANGEOVERBYPASS", AIRLOOPHVAC_UNITARYHEATCOOL_VAVCHANGEOVERBYPASS},
            {"AIRLOOPHVAC:UNITARYHEATONLY", AIRLOOPHVAC_UNITARYHEATONLY},
            {"AIRLOOPHVAC:UNITARYHEATPUMP:AIRTOAIR", AIRLOOPHVAC_UNITARYHEATPUMP_AIRTOAIR},
            {"AIRLOOPHVAC:UNITARYHEATPUMP:AIRTOAIR:MULTISPEED", AIRLOOPHVAC_UNITARYHEATPUMP_AIRTOAIR_MULTISPEED},
            {"AIRLOOPHVAC:UNITARYHEATPUMP:WATERTOAIR", AIRLOOPHVAC_UNITARYHEATPUMP_WATERTOAIR},
            {"AIRLOOPHVAC:UNITARYSYSTEM", AIRLOOPHVAC_UNITARYSYSTEM},
            {"AIRTERMINAL:DUALDUCT:CONSTANTVOLUME:COOL", AIRTERMINAL_DUALDUCT_CONSTANTVOLUME_COOL},
            {"AIRTERMINAL:DUALDUCT:CONSTANTVOLUME:HEAT", AIRTERMINAL_DUALDUCT_CONSTANTVOLUME_HEAT},
            {"AIRTERMINAL:DUALDUCT:VAV:COOL", AIRTERMINAL_DUALDUCT_VAV_COOL},
            {"AIRTERMINAL:DUALDUCT:VAV:HEAT", AIRTERMINAL_DUALDUCT_VAV_HEAT},
            {"AIRTERMINAL:DUALDUCT:VAV:OUTDOORAIR:OUTDOORAIR", AIRTERMINAL_DUALDUCT_VAV_OUTDOORAIR_OUTDOORAIR},
            {"AIRTERMINAL:DUALDUCT:VAV:OUTDOORAIR:RECIRCULATEDAIR", AIRTERMINAL_DUALDUCT_VAV_OUTDOORAIR_RECIRCULATEDAIR},
            {"AIRTERMINAL:SINGLEDUCT:CONSTANTVOLUME:COOLEDBEAM", AIRTERMINAL_SINGLEDUCT_CONSTANTVOLUME_COOLEDBEAM},
            {"AIRTERMINAL:SINGLEDUCT:CONSTANTVOLUME:FOURPIPEBEAM", AIRTERMINAL_SINGLEDUCT_CONSTANTVOLUME_FOURPIPEBEAM},
            {"AIRTERMINAL:SINGLEDUCT:CONSTANTVOLUME:FOURPIPEINDUCTION", AIRTERMINAL_SINGLEDUCT_CONSTANTVOLUME_FOURPIPEINDUCTION},
            {"AIRTERMINAL:SINGLEDUCT:CONSTANTVOLUME:REHEAT", AIRTERMINAL_SINGLEDUCT_CONSTANTVOLUME_REHEAT},
            {"AIRTERMINAL:SINGLEDUCT:CONSTANTVOLUME:NOREHEAT", AIRTERMINAL_SINGLEDUCT_CONSTANTVOLUME_NOREHEAT},
            {"AIRTERMINAL:SINGLEDUCT:MIXER", AIRTERMINAL_SINGLEDUCT_MIXER},
            {"AIRTERMINAL:SINGLEDUCT:PARALLELPIU:REHEAT", AIRTERMINAL_SINGLEDUCT_PARALLELPIU_REHEAT},
            {"AIRTERMINAL:SINGLEDUCT:SERIESPIU:REHEAT", AIRTERMINAL_SINGLEDUCT_SERIESPIU_REHEAT},
            {"AIRTERMINAL:SINGLEDUCT:USERDEFINED", AIRTERMINAL_SINGLEDUCT_USERDEFINED},
            {"AIRTERMINAL:SINGLEDUCT:VAV:HEATANDCOOL:NOREHEAT", AIRTERMINAL_SINGLEDUCT_VAV_HEATANDCOOL_NOREHEAT},
            {"AIRTERMINAL:SINGLEDUCT:VAV:HEATANDCOOL:REHEAT", AIRTERMINAL_SINGLEDUCT_VAV_HEATANDCOOL_REHEAT},
            {"AIRTERMINAL:SINGLEDUCT:VAV:NOREHEAT", AIRTERMINAL_SINGLEDUCT_VAV_NOREHEAT},
            {"AIRTERMINAL:SINGLEDUCT:VAV:REHEAT", AIRTERMINAL_SINGLEDUCT_VAV_REHEAT},
            {"AIRTERMINAL:SINGLEDUCT:VAV:REHEAT:VARIABLESPEEDFAN", AIRTERMINAL_SINGLEDUCT_VAV_REHEAT_VARIABLESPEEDFAN},
            {"COIL:COOLING:DX", COIL_COOLING_DX},
            {"COIL:COOLING:DX:MULTISPEED", COIL_COOLING_DX_MULTISPEED},
            {"COIL:COOLING:DX:SINGLESPEED", COIL_COOLING_DX_SINGLESPEED},
            {"COIL:COOLING:DX:SINGLESPEED:THERMALSTORAGE", COIL_COOLING_DX_SINGLESPEED_THERMALSTORAGE},
            {"COIL:COOLING:DX:TWOSPEED", COIL_COOLING_DX_TWOSPEED},
            {"COIL:COOLING:DX:TWOSTAGEWITHHUMIDITYCONTROLMODE", COIL_COOLING_DX_TWOSTAGEWITHHUMIDITYCONTROLMODE},
            {"COIL:COOLING:DX:VARIABLESPEED", COIL_COOLING_DX_VARIABLESPEED},
            {"COILSYSTEM:INTEGRATEDHEATPUMP:AIRSOURCE", COIL_INTEGRATED_DX_VARIABLESPEED},
            {"COIL:COOLING:WATER", COIL_COOLING_WATER},
            {"COIL:COOLING:WATER:DETAILEDGEOMETRY", COIL_COOLING_WATER_DETAILEDGEOMETRY},
            {"COIL:COOLING:WATERTOAIRHEATPUMP:EQUATIONFIT", COIL_COOLING_WATERTOAIRHEATPUMP_EQUATIONFIT},
            {"COIL:COOLING:WATERTOAIRHEATPUMP:PARAMETERESTIMATION", COIL_COOLING_WATERTOAIRHEATPUMP_PARAMETERESTIMATION},
            {"COIL:COOLING:WATERTOAIRHEATPUMP:VARIABLESPEEDEQUATIONFIT", COIL_COOLING_WATERTOAIRHEATPUMP_VARIABLESPEEDEQUATIONFIT},
            {"COIL:HEATING:DESUPERHEATER", COIL_HEATING_DESUPERHEATER},
            {"COIL:HEATING:DX:MULTISPEED", COIL_HEATING_DX_MULTISPEED},
            {"COIL:HEATING:DX:SINGLESPEED", COIL_HEATING_DX_SINGLESPEED},
            {"COIL:HEATING:DX:VARIABLESPEED", COIL_HEATING_DX_VARIABLESPEED},
            {"COIL:HEATING:ELECTRIC", COIL_HEATING_ELECTRIC},
            {"COIL:HEATING:ELECTRIC:MULTISTAGE", COIL_HEATING_ELECTRIC_MULTISTAGE},
            {"COIL:HEATING:FUEL", COIL_HEATING_GAS},
            {"COIL:HEATING:GAS:MULTISTAGE", COIL_HEATING_GAS_MULTISTAGE},
            {"COIL:HEATING:STEAM", COIL_HEATING_STEAM},
            {"COIL:HEATING:WATER", COIL_HEATING_WATER},
            {"COIL:HEATING:WATERTOAIRHEATPUMP:EQUATIONFIT", COIL_HEATING_WATERTOAIRHEATPUMP_EQUATIONFIT},
            {"COIL:HEATING:WATERTOAIRHEATPUMP:PARAMETERESTIMATION", COIL_HEATING_WATERTOAIRHEATPUMP_PARAMETERESTIMATION},
            {"COIL:HEATING:WATERTOAIRHEATPUMP:VARIABLESPEEDEQUATIONFIT", COIL_HEATING_WATERTOAIRHEATPUMP_VARIABLESPEEDEQUATIONFIT},
            {"COIL:WATERHEATING:AIRTOWATERHEATPUMP:VARIABLESPEED", COIL_WATERHEATING_AIRTOWATERHEATPUMP_VARIABLESPEED},
            {"COIL:USERDEFINED", COIL_USERDEFINED},
            {"COILSYSTEM:COOLING:DX", COILSYSTEM_COOLING_DX},
            {"COILSYSTEM:COOLING:DX:HEATEXCHANGERASSISTED", COILSYSTEM_COOLING_DX_HEATEXCHANGERASSISTED},
            {"COILSYSTEM:COOLING:WATER:HEATEXCHANGERASSISTED", COILSYSTEM_COOLING_WATER_HEATEXCHANGERASSISTED},
            {"COILSYSTEM:HEATING:DX", COILSYSTEM_HEATING_DX},
            {"DEHUMIDIFIER:DESICCANT:NOFANS", DEHUMIDIFIER_DESICCANT_NOFANS},
            {"DEHUMIDIFIER:DESICCANT:SYSTEM", DEHUMIDIFIER_DESICCANT_SYSTEM},
            {"DUCT", DUCT},
            {"EVAPORATIVECOOLER:DIRECT:CELDEKPAD", EVAPORATIVECOOLER_DIRECT_CELDEKPAD},
            {"EVAPORATIVECOOLER:DIRECT:RESEARCHSPECIAL", EVAPORATIVECOOLER_DIRECT_RESEARCHSPECIAL},
            {"EVAPORATIVECOOLER:INDIRECT:CELDEKPAD", EVAPORATIVECOOLER_INDIRECT_CELDEKPAD},
            {"EVAPORATIVECOOLER:INDIRECT:RESEARCHSPECIAL", EVAPORATIVECOOLER_INDIRECT_RESEARCHSPECIAL},
            {"EVAPORATIVECOOLER:INDIRECT:WETCOIL", EVAPORATIVECOOLER_INDIRECT_WETCOIL},
            {"FAN:COMPONENTMODEL", FAN_COMPONENTMODEL},
            {"FAN:SYSTEMMODEL", FAN_SYSTEMMODEL},
            {"FAN:CONSTANTVOLUME", FAN_CONSTANTVOLUME},
            {"FAN:ONOFF", FAN_ONOFF},
            {"FAN:VARIABLEVOLUME", FAN_VARIABLEVOLUME},
            {"HEATEXCHANGER:AIRTOAIR:FLATPLATE", HEATEXCHANGER_AIRTOAIR_FLATPLATE},
            {"HEATEXCHANGER:AIRTOAIR:SENSIBLEANDLATENT", HEATEXCHANGER_AIRTOAIR_SENSIBLEANDLATENT},
            {"HEATEXCHANGER:DESICCANT:BALANCEDFLOW", HEATEXCHANGER_DESICCANT_BALANCEDFLOW},
            {"HUMIDIFIER:STEAM:ELECTRIC", HUMIDIFIER_STEAM_ELECTRIC},
            {"HUMIDIFIER:STEAM:GAS", HUMIDIFIER_STEAM_GAS},
            {"OUTDOORAIR:MIXER", OUTDOORAIR_MIXER},
            {"SOLARCOLLECTOR:FLATPLATE:PHOTOVOLTAICTHERMAL", SOLARCOLLECTOR_FLATPLATE_PHOTOVOLTAICTHERMAL},
            {"SOLARCOLLECTOR:UNGLAZEDTRANSPIRED", SOLARCOLLECTOR_UNGLAZEDTRANSPIRED},
            {"ZONEHVAC:AIRDISTRIBUTIONUNIT", ZONEHVAC_AIRDISTRIBUTIONUNIT},
            {"ZONEHVAC:TERMINALUNIT:VARIABLEREFRIGERANTFLOW", ZONEHVAC_TERMINALUNIT_VRF},
            {"COIL:COOLING:DX:VARIABLEREFRIGERANTFLOW", COIL_COOLING_VRF},
            {"COIL:HEATING:DX:VARIABLEREFRIGERANTFLOW", COIL_HEATING_VRF},
            {"COIL:COOLING:DX:VARIABLEREFRIGERANTFLOW:FLUIDTEMPERATURECONTROL", COIL_COOLING_VRF_FTC},
            {"COIL:HEATING:DX:VARIABLEREFRIGERANTFLOW:FLUIDTEMPERATURECONTROL", COIL_HEATING_VRF_FTC}};
        assert(component_map.size() == n_ComponentTypes);

        // INTERFACE BLOCK SPECIFICATIONS
        // na

        // DERIVED TYPE DEFINITIONS

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        static int NumCompTypes(0);
        int found;

        struct CompTypeError
        {
            // Members
            std::string CompType;
            int CompErrIndex;

            // Default Constructor
            CompTypeError() : CompErrIndex(0)
            {
            }
        };

        // Object Data
        static Array1D<CompTypeError> CompTypeErrors(100);

        if (!AirLoopLoadsReportEnabled) return;

        // following for debug
        //    CHARACTER(len=60) :: cEnergyType

        //    cEnergyType=cRT_ValidTypes(EnergyType-ResourceTypeInitialOffset)

        // Find enum for the component type string
        ComponentTypes comp_type;
        auto const it = component_map.find(CompType);
        if (it != component_map.end()) {
            comp_type = it->second;
        } else {
            comp_type = Unknown_ComponentType;
        }

        switch (comp_type) {
        case AIRLOOPHVAC_OUTDOORAIRSYSTEM: // Outside Air System
            if (CompLoadFlag) {
                if (CompLoad > 0.0) {
                    SysOALoadCLNG(AirLoopNum) += std::abs(CompLoad);
                } else {
                    SysOALoadHTNG(AirLoopNum) += std::abs(CompLoad);
                }
            }
            break;
        case OUTDOORAIR_MIXER: // Outdoor Air Mixer
            // No energy transfers to account for
            break;
        case AIRTERMINAL_SINGLEDUCT_MIXER:
            // No energy transfers to account for

            break;
        case FAN_CONSTANTVOLUME:
        case FAN_VARIABLEVOLUME:
        case FAN_ONOFF:
        case FAN_SYSTEMMODEL:
        case FAN_COMPONENTMODEL:

            if (CompLoadFlag) SysFANCompHTNG(AirLoopNum) += std::abs(CompLoad);
            SysFANCompElec(AirLoopNum) += CompEnergy;

            // Cooling Coil Types for the air sys simulation
            break;
        case COILSYSTEM_COOLING_DX_HEATEXCHANGERASSISTED:
        case COIL_COOLING_DX_SINGLESPEED:
        case COIL_COOLING_DX_TWOSPEED:
        case COIL_COOLING_DX_TWOSTAGEWITHHUMIDITYCONTROLMODE:
        case COIL_COOLING_DX:
        case COIL_COOLING_DX_MULTISPEED:
        case COIL_COOLING_WATERTOAIRHEATPUMP_EQUATIONFIT:
        case COIL_COOLING_WATERTOAIRHEATPUMP_PARAMETERESTIMATION:
        case COIL_COOLING_WATERTOAIRHEATPUMP_VARIABLESPEEDEQUATIONFIT:
        case COIL_COOLING_DX_VARIABLESPEED:
        case COILSYSTEM_COOLING_WATER_HEATEXCHANGERASSISTED:
        case COIL_COOLING_WATER_DETAILEDGEOMETRY:
        case COIL_COOLING_WATER:
        case COIL_COOLING_DX_SINGLESPEED_THERMALSTORAGE:
        case COIL_COOLING_VRF:
        case COIL_COOLING_VRF_FTC:
        case COIL_WATERHEATING_AIRTOWATERHEATPUMP_VARIABLESPEED:

            if (CompLoadFlag) SysCCCompCLNG(AirLoopNum) += std::abs(CompLoad);
            if ((EnergyType == iRT_PlantLoopCoolingDemand) || (EnergyType == iRT_DistrictCooling)) {
                SysCCCompH2OCOLD(AirLoopNum) += CompEnergy;
            } else if (EnergyType == iRT_Electricity) {
                SysCCCompElec(AirLoopNum) += CompEnergy;
            }

            // Heating Coil Types for the air sys simulation
            break;
        case COIL_HEATING_WATER:
        case COIL_HEATING_DX_SINGLESPEED:
        case COIL_HEATING_DX_MULTISPEED:
        case COIL_HEATING_WATERTOAIRHEATPUMP_EQUATIONFIT:
        case COIL_HEATING_WATERTOAIRHEATPUMP_PARAMETERESTIMATION:
        case COIL_HEATING_WATERTOAIRHEATPUMP_VARIABLESPEEDEQUATIONFIT:
        case COIL_HEATING_DX_VARIABLESPEED:
        case COIL_HEATING_STEAM:
        case COIL_HEATING_GAS:
        case COIL_HEATING_GAS_MULTISTAGE:
        case COIL_HEATING_DESUPERHEATER:

            if (CompLoadFlag) SysHCCompHTNG(AirLoopNum) += std::abs(CompLoad);
            if ((EnergyType == iRT_PlantLoopHeatingDemand) || (EnergyType == iRT_DistrictHeating)) {
                SysHCCompH2OHOT(AirLoopNum) += CompEnergy;
            } else if (EnergyType == iRT_Steam) {
                SysHCCompSteam(AirLoopNum) += CompEnergy;
            } else if (EnergyType == iRT_Electricity) {
                SysHCCompElec(AirLoopNum) += CompEnergy;
            } else if (EnergyType == iRT_Natural_Gas) {
                SysHCCompNaturalGas(AirLoopNum) += CompEnergy;
            } else if (EnergyType == iRT_Propane) {
                SysHCCompPropane(AirLoopNum) += CompEnergy;
            }

            break;
        case COIL_HEATING_ELECTRIC:
        case COIL_HEATING_ELECTRIC_MULTISTAGE:

            if (CompLoadFlag) SysHCCompHTNG(AirLoopNum) += std::abs(CompLoad);
            if (EnergyType == iRT_Electricity) {
                SysHCCompElecRes(AirLoopNum) += CompEnergy;
            }

            break;
        case COIL_USERDEFINED:

            if (CompLoadFlag) {
                if (CompLoad > 0.0) {
                    SysCCCompCLNG(AirLoopNum) += std::abs(CompLoad);
                } else {
                    SysHCCompHTNG(AirLoopNum) += std::abs(CompLoad);
                }
            }
            if ((EnergyType == iRT_PlantLoopHeatingDemand) || (EnergyType == iRT_DistrictHeating)) {
                SysHCCompH2OHOT(AirLoopNum) += CompEnergy;
            } else if ((EnergyType == iRT_PlantLoopCoolingDemand) || (EnergyType == iRT_DistrictCooling)) {
                SysCCCompH2OCOLD(AirLoopNum) += CompEnergy;
            } else if (EnergyType == iRT_Steam) {
                SysHCCompSteam(AirLoopNum) += CompEnergy;
            } else if (EnergyType == iRT_Electricity) {
                if (CompLoad > 0.0) {
                    SysCCCompElec(AirLoopNum) += CompEnergy;
                } else {
                    SysHCCompElec(AirLoopNum) += CompEnergy;
                }
            } else if (EnergyType == iRT_Natural_Gas) {
                SysHCCompNaturalGas(AirLoopNum) += CompEnergy;
            } else if (EnergyType == iRT_Propane) {
                SysHCCompPropane(AirLoopNum) += CompEnergy;
            }

            // DX Systems
            break;
        case COIL_HEATING_VRF:
        case COIL_HEATING_VRF_FTC:
        case AIRLOOPHVAC_UNITARYSYSTEM:
            // All energy transfers accounted for in subcomponent models
            break;
        case AIRLOOPHVAC_UNITARYHEATPUMP_AIRTOAIR:
            // All energy transfers accounted for in subcomponent models
            break;
        case AIRLOOPHVAC_UNITARYHEATPUMP_WATERTOAIR:
            // All energy transfers accounted for in subcomponent models
            break;
        case COILSYSTEM_COOLING_DX:
            // All energy transfers accounted for in subcomponent models
            break;
        case COILSYSTEM_HEATING_DX:
            // All energy transfers accounted for in subcomponent models
            break;
        case AIRLOOPHVAC_UNITARY_FURNACE_HEATONLY:
            // All energy transfers accounted for in subcomponent models
            break;
        case AIRLOOPHVAC_UNITARY_FURNACE_HEATCOOL:
            // All energy transfers accounted for in subcomponent models
            break;
        case AIRLOOPHVAC_UNITARYHEATONLY:
            // All energy transfers accounted for in subcomponent models
            break;
        case AIRLOOPHVAC_UNITARYHEATCOOL:
            // All energy transfers accounted for in subcomponent models
            break;
        case AIRLOOPHVAC_UNITARYHEATCOOL_VAVCHANGEOVERBYPASS:
            // All energy transfers accounted for in subcomponent models
            break;
        case AIRLOOPHVAC_UNITARYHEATPUMP_AIRTOAIR_MULTISPEED:
            // All energy transfers accounted for in subcomponent models
            break;
        case ZONEHVAC_TERMINALUNIT_VRF:
            // All energy transfers accounted for in subcomponent models
            break;
            // Humidifier Types for the air system simulation
        case HUMIDIFIER_STEAM_GAS:
        case HUMIDIFIER_STEAM_ELECTRIC:
            if (CompLoadFlag) SysHumidHTNG(AirLoopNum) += std::abs(CompLoad);
            if (EnergyType == iRT_Water) {
                SysDomesticH2O(AirLoopNum) += std::abs(CompEnergy);
            } else if (EnergyType == iRT_Electricity) {
                SysHumidElec(AirLoopNum) += CompEnergy;
            } else if (EnergyType == iRT_Natural_Gas) {
                SysHumidNaturalGas(AirLoopNum) += CompEnergy;
            } else if (EnergyType == iRT_Propane) {
                SysHumidPropane(AirLoopNum) += CompEnergy;
            }

            // Evap Cooler Types for the air system simulation
            break;
        case EVAPORATIVECOOLER_DIRECT_CELDEKPAD:
        case EVAPORATIVECOOLER_INDIRECT_CELDEKPAD:
        case EVAPORATIVECOOLER_INDIRECT_WETCOIL:
        case EVAPORATIVECOOLER_DIRECT_RESEARCHSPECIAL:
        case EVAPORATIVECOOLER_INDIRECT_RESEARCHSPECIAL:
            if (CompLoadFlag) SysEvapCLNG(AirLoopNum) += std::abs(CompLoad);
            if (EnergyType == iRT_Water) {
                SysDomesticH2O(AirLoopNum) += std::abs(CompEnergy);
            } else if (EnergyType == iRT_Electricity) {
                SysEvapElec(AirLoopNum) += CompEnergy;
            }

            // Desiccant Dehumidifier Types for the air system simulation
            break;
        case DEHUMIDIFIER_DESICCANT_NOFANS:
        case DEHUMIDIFIER_DESICCANT_SYSTEM:
            if (CompLoadFlag) DesDehumidCLNG(AirLoopNum) += std::abs(CompLoad);
            if (EnergyType == iRT_Electricity) {
                DesDehumidElec(AirLoopNum) += CompEnergy;
            }

            // Heat Exchanger Types
            break;
        case HEATEXCHANGER_AIRTOAIR_FLATPLATE:
        case HEATEXCHANGER_AIRTOAIR_SENSIBLEANDLATENT:
        case HEATEXCHANGER_DESICCANT_BALANCEDFLOW:
            if (CompLoadFlag) {
                if (CompLoad > 0.0) {
                    SysHeatExCLNG(AirLoopNum) += std::abs(CompLoad);
                } else {
                    SysHeatExHTNG(AirLoopNum) += std::abs(CompLoad);
                }
            }

            // Air Terminal Types
            break;
        case AIRTERMINAL_DUALDUCT_CONSTANTVOLUME_COOL:
        case AIRTERMINAL_DUALDUCT_CONSTANTVOLUME_HEAT:
        case AIRTERMINAL_DUALDUCT_VAV_COOL:
        case AIRTERMINAL_DUALDUCT_VAV_HEAT:
        case AIRTERMINAL_DUALDUCT_VAV_OUTDOORAIR_OUTDOORAIR:
        case AIRTERMINAL_DUALDUCT_VAV_OUTDOORAIR_RECIRCULATEDAIR:
        case AIRTERMINAL_SINGLEDUCT_CONSTANTVOLUME_FOURPIPEINDUCTION:
        case AIRTERMINAL_SINGLEDUCT_CONSTANTVOLUME_REHEAT:
        case AIRTERMINAL_SINGLEDUCT_CONSTANTVOLUME_NOREHEAT:
        case AIRTERMINAL_SINGLEDUCT_PARALLELPIU_REHEAT:
        case AIRTERMINAL_SINGLEDUCT_SERIESPIU_REHEAT:
        case AIRTERMINAL_SINGLEDUCT_VAV_HEATANDCOOL_NOREHEAT:
        case AIRTERMINAL_SINGLEDUCT_VAV_HEATANDCOOL_REHEAT:
        case AIRTERMINAL_SINGLEDUCT_VAV_NOREHEAT:
        case AIRTERMINAL_SINGLEDUCT_VAV_REHEAT:
        case AIRTERMINAL_SINGLEDUCT_VAV_REHEAT_VARIABLESPEEDFAN:
        case AIRTERMINAL_SINGLEDUCT_CONSTANTVOLUME_COOLEDBEAM:
        case AIRTERMINAL_SINGLEDUCT_CONSTANTVOLUME_FOURPIPEBEAM:
        case ZONEHVAC_AIRDISTRIBUTIONUNIT:
            // All energy transfers accounted for in component models

            // Duct Types
            break;
        case DUCT:
            // duct losses should be accounted for here ???
            // requires addition of a new variable to sum duct losses
            // Example:
            //      IF(CompLoad > 0.0d0)THEN
            //        SysDuctHTNG(AirLoopNum) =  SysDuctHTNG(AirLoopNum) + ABS(CompLoad)
            //      ELSE
            //        SysDuctCLNG(AirLoopNum) =  SysDuctCLNG(AirLoopNum) + ABS(CompLoad)
            //      ENDIF

            // Solar Collector Types
            break;
        case SOLARCOLLECTOR_FLATPLATE_PHOTOVOLTAICTHERMAL:
        case SOLARCOLLECTOR_UNGLAZEDTRANSPIRED:
            if (CompLoadFlag) {
                if (CompLoad > 0.0) {
                    SysSolarCollectCooling(AirLoopNum) += std::abs(CompLoad);
                } else {
                    SysSolarCollectHeating(AirLoopNum) += std::abs(CompLoad);
                }
            }

            break;
        case AIRTERMINAL_SINGLEDUCT_USERDEFINED:
            // User component model energy use should be accounted for here
            if (CompLoadFlag) {
                if (CompLoad > 0.0) {
                    SysUserDefinedTerminalCooling(AirLoopNum) += std::abs(CompLoad);
                } else {
                    SysUserDefinedTerminalHeating(AirLoopNum) += std::abs(CompLoad);
                }
            }
            if ((EnergyType == iRT_PlantLoopHeatingDemand) || (EnergyType == iRT_DistrictHeating)) {
                SysHCCompH2OHOT(AirLoopNum) += CompEnergy;
            } else if ((EnergyType == iRT_PlantLoopCoolingDemand) || (EnergyType == iRT_DistrictCooling)) {
                SysCCCompH2OCOLD(AirLoopNum) += CompEnergy;
            } else if (EnergyType == iRT_Steam) {
                SysHCCompSteam(AirLoopNum) += CompEnergy;
            } else if (EnergyType == iRT_Electricity) {
                if (CompLoad > 0.0) {
                    SysCCCompElec(AirLoopNum) += CompEnergy;
                } else {
                    SysHCCompElec(AirLoopNum) += CompEnergy;
                }
            } else if (EnergyType == iRT_Natural_Gas) {
                SysHCCompNaturalGas(AirLoopNum) += CompEnergy;
            } else if (EnergyType == iRT_Propane) {
                SysHCCompPropane(AirLoopNum) += CompEnergy;
            }
            // Recurring warning for unaccounted equipment types
            // (should never happen, when this does happen enter appropriate equipment CASE statement above)
            break;
        case COIL_INTEGRATED_DX_VARIABLESPEED:
            // All energy transfers accounted for in component models
            break;
        default:
            found = 0;
            if (NumCompTypes > 0) {
                found = UtilityRoutines::FindItemInList(CompType, CompTypeErrors, &CompTypeError::CompType, NumCompTypes);
            }
            if (found == 0) {
                CompTypeErrors(++NumCompTypes).CompType = CompType;
                found = NumCompTypes;
            }
            ShowRecurringSevereErrorAtEnd("CalcSystemEnergyUse: Component Type=" + CompType + " not logged as one of allowable Component Types.",
                                          CompTypeErrors(found).CompErrIndex);
            break;
        } // switch
    }

    void ReportMaxVentilationLoads(EnergyPlusData &state)
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Dan Fisher (with minor assistance from RKS)
        //       DATE WRITTEN   July 2004
        //       MODIFIED       Dec. 2006, BG. reengineered to add zone forced air units to vent rates and loads
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // calculate and report zone ventilation loads

        // METHODOLOGY EMPLOYED:
        // calculate energy contribution of outside air through mixing box and pro-rate to
        // zones according to zone mass flow rates.

        // Using/Aliasing
        using Psychrometrics::PsyHFnTdbW;
        using Psychrometrics::PsyRhoAirFnPbTdbW;
        using namespace DataZoneEnergyDemands;
        using namespace DataGlobalConstants;
        using DataEnvironment::OutBaroPress;
        using DataEnvironment::StdRhoAir;
        using DataHeatBalance::ZnAirRpt;
        using DataHeatBalance::Zone;
        using DataHeatBalance::ZonePreDefRep;
        using DataHeatBalFanSys::MAT;
        using DataHeatBalFanSys::ZoneAirHumRatAvg;
        using FanCoilUnits::GetFanCoilMixedAirNode;
        using FanCoilUnits::GetFanCoilOutAirNode;
        using FanCoilUnits::GetFanCoilReturnAirNode;
        using FanCoilUnits::GetFanCoilZoneInletAirNode;
        using HVACStandAloneERV::GetStandAloneERVOutAirNode;
        using HVACStandAloneERV::GetStandAloneERVReturnAirNode;
        using HVACStandAloneERV::GetStandAloneERVZoneInletAirNode;
        using HVACVariableRefrigerantFlow::GetVRFTUMixedAirNode;
        using HVACVariableRefrigerantFlow::GetVRFTUOutAirNode;
        using HVACVariableRefrigerantFlow::GetVRFTUReturnAirNode;
        using HVACVariableRefrigerantFlow::GetVRFTUZoneInletAirNode;
        using HybridUnitaryAirConditioners::GetHybridUnitaryACOutAirNode;
        using HybridUnitaryAirConditioners::GetHybridUnitaryACReturnAirNode;
        using HybridUnitaryAirConditioners::GetHybridUnitaryACZoneInletNode;
        using OutdoorAirUnit::GetOutdoorAirUnitOutAirNode;
        using OutdoorAirUnit::GetOutdoorAirUnitReturnAirNode;
        using OutdoorAirUnit::GetOutdoorAirUnitZoneInletNode;
        using PackagedTerminalHeatPump::GetPTUnitMixedAirNode;
        using PackagedTerminalHeatPump::GetPTUnitOutAirNode;
        using PackagedTerminalHeatPump::GetPTUnitReturnAirNode;
        using PackagedTerminalHeatPump::GetPTUnitZoneInletAirNode;
        using PurchasedAirManager::GetPurchasedAirMixedAirHumRat;
        using PurchasedAirManager::GetPurchasedAirMixedAirTemp;
        using PurchasedAirManager::GetPurchasedAirOutAirMassFlow;
        using PurchasedAirManager::GetPurchasedAirReturnAirNode;
        using PurchasedAirManager::GetPurchasedAirZoneInletAirNode;
        using UnitVentilator::GetUnitVentilatorMixedAirNode;
        using UnitVentilator::GetUnitVentilatorOutAirNode;
        using UnitVentilator::GetUnitVentilatorReturnAirNode;
        using UnitVentilator::GetUnitVentilatorZoneInletAirNode;
        using WindowAC::GetWindowACMixedAirNode;
        using WindowAC::GetWindowACOutAirNode;
        using WindowAC::GetWindowACReturnAirNode;
        using WindowAC::GetWindowACZoneInletAirNode;

        // SUBROUTINE PARAMETER DEFINITIONS:
        Real64 const SmallLoad(0.1); // (W)

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int CtrlZoneNum;             // ZONE counter
        int ZoneInNum;               // counter for zone air distribution inlets
        int ReturnAirNode;           // node number for return node on primary air loop
        int MixedAirNode;            // mixed air node number (right after the mixing box) on primary air loop
        int AirLoopNum;              // index to AirloopHVAC
        int AirDistCoolInletNodeNum; // Air distribution unit inlet node number
        int AirDistHeatInletNodeNum; // Air distribution unit outlet node number

        Real64 ZFAUEnthReturnAir;  // Zone forced Air unit enthalpy of the return air [kJ/kgK]
        Real64 ZFAUTempMixedAir;   // Zone forced Air unit dry-bulb temperature of the mixed air [C]
        Real64 ZFAUHumRatMixedAir; // Zone forced Air unit humidity ratio of the mixed air [kg/kg]
        Real64 ZFAUEnthMixedAir;   // Zone forced Air unit enthalpy of the mixed air [kJ/kgK]
        Real64 ZFAUEnthOutdoorAir; // Zone forced Air unit enthalpy of the outdoor air [kJ/kgK]
        Real64 ZFAUFlowRate;       // Zone forced Air unit air mass flow rate [kg/s]
        Real64 ZFAUZoneVentLoad;   // ventilation load attributed to a particular zone from zone forced air units [J]
        Real64 ZFAUOutAirFlow;     // outside air flow rate for zone from zone forced air units.
        int ZoneInletAirNode;      // Zone forced Air unit zone inlet node number

        Real64 ZoneVentLoad;          // ventilation load attributed to a particular zone
        Real64 ZoneLoad;              // ventilation load attributed to a particular zone
        Real64 OutAirFlow;            // Total outside air mass flow from zone equipment and air loop equipment [kg/s]
        Real64 ZoneFlowFrac;          // fraction of mixed air flowing to a zone
        Real64 ZoneVolume;            // Volume of zone [m3]
        Real64 currentZoneAirDensity; // current zone air density (outside barometric pressure) [kg/m3]

        int ActualZoneNum;    // Zone forced Air zone number
        int OutAirNode;       // Zone forced Air unit outdoor air node number
        int thisZoneEquipNum; // loop counter

        //  CALL GetComponentEnergyUse
        if (!VentReportStructureCreated) return;
        if (!VentLoadsReportEnabled) return;
        // following inits are array assignments across all controlled zones.
        ZoneOAMassFlow = 0.0;
        ZoneOAMass = 0.0;
        ZoneOAVolFlowStdRho = 0.0;
        ZoneOAVolStdRho = 0.0;
        ZoneOAVolFlowCrntRho = 0.0;
        ZoneOAVolCrntRho = 0.0;
        ZoneMechACH = 0.0;
        MaxCoolingLoadMetByVent = 0.0;
        MaxCoolingLoadAddedByVent = 0.0;
        MaxOvercoolingByVent = 0.0;
        MaxHeatingLoadMetByVent = 0.0;
        MaxHeatingLoadAddedByVent = 0.0;
        MaxOverheatingByVent = 0.0;
        MaxNoLoadHeatingByVent = 0.0;
        MaxNoLoadCoolingByVent = 0.0;

        for (CtrlZoneNum = 1; CtrlZoneNum <= NumOfZones; ++CtrlZoneNum) {
            if (!ZoneEquipConfig(CtrlZoneNum).IsControlled) continue;
            Real64 ZAirSysZoneVentLoad = 0.0; // ventilation load attributed to a particular zone from all primary air systems serving the zone [J]
            Real64 ZAirSysOutAirFlow = 0.0;   // outside air flow rate for zone from all primary air systems serving thezone [kg/s]
            // first clear out working variables from previous zone.
            ZFAUFlowRate = 0.0;
            ZFAUZoneVentLoad = 0.0;
            ZFAUOutAirFlow = 0.0;
            OutAirFlow = 0.0;
            ZoneFlowFrac = 0.0;

            // retrieve the zone load for each zone
            ActualZoneNum = ZoneEquipConfig(CtrlZoneNum).ActualZoneNum;
            ZoneLoad = ZoneSysEnergyDemand(ActualZoneNum).TotalOutputRequired;
            ZoneVolume = Zone(ActualZoneNum).Volume * Zone(ActualZoneNum).Multiplier * Zone(ActualZoneNum).ListMultiplier; // CR 7170

            // if system operating in deadband reset zone load
            if (DeadBandOrSetback(ActualZoneNum)) ZoneLoad = 0.0;
            if (DeadBandOrSetback(ActualZoneNum)) {
                DBFlag = 1;
            } else {
                DBFlag = 0;
            }

            //  IF(AirLoopNum == 0 ) CYCLE   !orig line (BG 12-8-06 changed, zone forced air equipment seems to get excluded here...)

            // first deal with any (and all) Zone Forced Air Units that might have outside air.
            for (thisZoneEquipNum = 1; thisZoneEquipNum <= ZoneEquipList(ZoneEquipConfig(CtrlZoneNum).EquipListIndex).NumOfEquipTypes;
                 ++thisZoneEquipNum) {
                {
                    auto const SELECT_CASE_var(ZoneEquipList(ZoneEquipConfig(CtrlZoneNum).EquipListIndex).EquipType_Num(thisZoneEquipNum));
                    // case statement to cover all possible zone forced air units that could have outside air

                    if (SELECT_CASE_var == WindowAC_Num) { // Window Air Conditioner
                        OutAirNode = GetWindowACOutAirNode(state, ZoneEquipList(ZoneEquipConfig(CtrlZoneNum).EquipListIndex).EquipIndex(thisZoneEquipNum));
                        if (OutAirNode > 0) ZFAUOutAirFlow += Node(OutAirNode).MassFlowRate;

                        ZoneInletAirNode =
                            GetWindowACZoneInletAirNode(state, ZoneEquipList(ZoneEquipConfig(CtrlZoneNum).EquipListIndex).EquipIndex(thisZoneEquipNum));
                        if (ZoneInletAirNode > 0) ZFAUFlowRate = max(Node(ZoneInletAirNode).MassFlowRate, 0.0);
                        MixedAirNode =
                            GetWindowACMixedAirNode(state, ZoneEquipList(ZoneEquipConfig(CtrlZoneNum).EquipListIndex).EquipIndex(thisZoneEquipNum));
                        ReturnAirNode =
                            GetWindowACReturnAirNode(state, ZoneEquipList(ZoneEquipConfig(CtrlZoneNum).EquipListIndex).EquipIndex(thisZoneEquipNum));
                        if ((MixedAirNode > 0) && (ReturnAirNode > 0)) {
                            ZFAUEnthMixedAir = PsyHFnTdbW(Node(MixedAirNode).Temp, Node(MixedAirNode).HumRat);
                            ZFAUEnthReturnAir = PsyHFnTdbW(Node(ReturnAirNode).Temp, Node(ReturnAirNode).HumRat);
                            // Calculate the zone ventilation load for this supply air path (i.e. zone inlet)
                            ZFAUZoneVentLoad += (ZFAUFlowRate) * (ZFAUEnthMixedAir - ZFAUEnthReturnAir) * TimeStepSys * SecInHour; //*KJperJ
                        } else {
                            ZFAUZoneVentLoad += 0.0;
                        }

                    } else if (SELECT_CASE_var == VRFTerminalUnit_Num) {
                        OutAirNode = GetVRFTUOutAirNode(state, ZoneEquipList(ZoneEquipConfig(CtrlZoneNum).EquipListIndex).EquipIndex(thisZoneEquipNum));
                        if (OutAirNode > 0) ZFAUOutAirFlow += Node(OutAirNode).MassFlowRate;
                        ZoneInletAirNode =
                            GetVRFTUZoneInletAirNode(state, ZoneEquipList(ZoneEquipConfig(CtrlZoneNum).EquipListIndex).EquipIndex(thisZoneEquipNum));
                        if (ZoneInletAirNode > 0) ZFAUFlowRate = max(Node(ZoneInletAirNode).MassFlowRate, 0.0);
                        MixedAirNode = GetVRFTUMixedAirNode(state, ZoneEquipList(ZoneEquipConfig(CtrlZoneNum).EquipListIndex).EquipIndex(thisZoneEquipNum));
                        ReturnAirNode =
                            GetVRFTUReturnAirNode(state, ZoneEquipList(ZoneEquipConfig(CtrlZoneNum).EquipListIndex).EquipIndex(thisZoneEquipNum));
                        if ((MixedAirNode > 0) && (ReturnAirNode > 0)) {
                            ZFAUEnthMixedAir = PsyHFnTdbW(Node(MixedAirNode).Temp, Node(MixedAirNode).HumRat);
                            ZFAUEnthReturnAir = PsyHFnTdbW(Node(ReturnAirNode).Temp, Node(ReturnAirNode).HumRat);
                            // Calculate the zone ventilation load for this supply air path (i.e. zone inlet)
                            ZFAUZoneVentLoad += (ZFAUFlowRate) * (ZFAUEnthMixedAir - ZFAUEnthReturnAir) * TimeStepSys * SecInHour; //*KJperJ
                        } else {
                            ZFAUZoneVentLoad += 0.0;
                        }

                    } else if ((SELECT_CASE_var == PkgTermHPAirToAir_Num) || (SELECT_CASE_var == PkgTermACAirToAir_Num) ||
                               (SELECT_CASE_var == PkgTermHPWaterToAir_Num)) {
                        OutAirNode = GetPTUnitOutAirNode(state, ZoneEquipList(ZoneEquipConfig(CtrlZoneNum).EquipListIndex).EquipIndex(thisZoneEquipNum),
                                                         ZoneEquipList(ZoneEquipConfig(CtrlZoneNum).EquipListIndex).EquipType_Num(thisZoneEquipNum));
                        if (OutAirNode > 0) ZFAUOutAirFlow += Node(OutAirNode).MassFlowRate;

                        ZoneInletAirNode =
                            GetPTUnitZoneInletAirNode(state, ZoneEquipList(ZoneEquipConfig(CtrlZoneNum).EquipListIndex).EquipIndex(thisZoneEquipNum),
                                                      ZoneEquipList(ZoneEquipConfig(CtrlZoneNum).EquipListIndex).EquipType_Num(thisZoneEquipNum));
                        if (ZoneInletAirNode > 0) ZFAUFlowRate = max(Node(ZoneInletAirNode).MassFlowRate, 0.0);
                        MixedAirNode =
                            GetPTUnitMixedAirNode(state, ZoneEquipList(ZoneEquipConfig(CtrlZoneNum).EquipListIndex).EquipIndex(thisZoneEquipNum),
                                                  ZoneEquipList(ZoneEquipConfig(CtrlZoneNum).EquipListIndex).EquipType_Num(thisZoneEquipNum));
                        ReturnAirNode =
                            GetPTUnitReturnAirNode(state, ZoneEquipList(ZoneEquipConfig(CtrlZoneNum).EquipListIndex).EquipIndex(thisZoneEquipNum),
                                                   ZoneEquipList(ZoneEquipConfig(CtrlZoneNum).EquipListIndex).EquipType_Num(thisZoneEquipNum));
                        if ((MixedAirNode > 0) && (ReturnAirNode > 0)) {
                            ZFAUEnthMixedAir = PsyHFnTdbW(Node(MixedAirNode).Temp, Node(MixedAirNode).HumRat);
                            ZFAUEnthReturnAir = PsyHFnTdbW(Node(ReturnAirNode).Temp, Node(ReturnAirNode).HumRat);
                            // Calculate the zone ventilation load for this supply air path (i.e. zone inlet)
                            ZFAUZoneVentLoad += (ZFAUFlowRate) * (ZFAUEnthMixedAir - ZFAUEnthReturnAir) * TimeStepSys * SecInHour; //*KJperJ
                        } else {
                            ZFAUZoneVentLoad += 0.0;
                        }

                    } else if (SELECT_CASE_var == FanCoil4Pipe_Num) {
                        OutAirNode = GetFanCoilOutAirNode(state, ZoneEquipList(ZoneEquipConfig(CtrlZoneNum).EquipListIndex).EquipIndex(thisZoneEquipNum));
                        if (OutAirNode > 0) ZFAUOutAirFlow += Node(OutAirNode).MassFlowRate;

                        ZoneInletAirNode =
                            GetFanCoilZoneInletAirNode(state, ZoneEquipList(ZoneEquipConfig(CtrlZoneNum).EquipListIndex).EquipIndex(thisZoneEquipNum));
                        if (ZoneInletAirNode > 0) ZFAUFlowRate = max(Node(ZoneInletAirNode).MassFlowRate, 0.0);
                        MixedAirNode =
                            GetFanCoilMixedAirNode(state, ZoneEquipList(ZoneEquipConfig(CtrlZoneNum).EquipListIndex).EquipIndex(thisZoneEquipNum));
                        ReturnAirNode =
                            GetFanCoilReturnAirNode(state, ZoneEquipList(ZoneEquipConfig(CtrlZoneNum).EquipListIndex).EquipIndex(thisZoneEquipNum));
                        if ((MixedAirNode > 0) && (ReturnAirNode > 0)) {
                            ZFAUEnthMixedAir = PsyHFnTdbW(Node(MixedAirNode).Temp, Node(MixedAirNode).HumRat);
                            ZFAUEnthReturnAir = PsyHFnTdbW(Node(ReturnAirNode).Temp, Node(ReturnAirNode).HumRat);
                            // Calculate the zone ventilation load for this supply air path (i.e. zone inlet)
                            ZFAUZoneVentLoad += (ZFAUFlowRate) * (ZFAUEnthMixedAir - ZFAUEnthReturnAir) * TimeStepSys * SecInHour; //*KJperJ
                        } else {
                            ZFAUZoneVentLoad += 0.0;
                        }

                    } else if (SELECT_CASE_var == UnitVentilator_Num) {
                        OutAirNode =
                            GetUnitVentilatorOutAirNode(state, ZoneEquipList(ZoneEquipConfig(CtrlZoneNum).EquipListIndex).EquipIndex(thisZoneEquipNum));
                        if (OutAirNode > 0) ZFAUOutAirFlow += Node(OutAirNode).MassFlowRate;

                        ZoneInletAirNode = GetUnitVentilatorZoneInletAirNode(state,
                            ZoneEquipList(ZoneEquipConfig(CtrlZoneNum).EquipListIndex).EquipIndex(thisZoneEquipNum));
                        if (ZoneInletAirNode > 0) ZFAUFlowRate = max(Node(ZoneInletAirNode).MassFlowRate, 0.0);
                        MixedAirNode =
                            GetUnitVentilatorMixedAirNode(state, ZoneEquipList(ZoneEquipConfig(CtrlZoneNum).EquipListIndex).EquipIndex(thisZoneEquipNum));
                        ReturnAirNode =
                            GetUnitVentilatorReturnAirNode(state, ZoneEquipList(ZoneEquipConfig(CtrlZoneNum).EquipListIndex).EquipIndex(thisZoneEquipNum));
                        if ((MixedAirNode > 0) && (ReturnAirNode > 0)) {
                            ZFAUEnthMixedAir = PsyHFnTdbW(Node(MixedAirNode).Temp, Node(MixedAirNode).HumRat);
                            ZFAUEnthReturnAir = PsyHFnTdbW(Node(ReturnAirNode).Temp, Node(ReturnAirNode).HumRat);
                            // Calculate the zone ventilation load for this supply air path (i.e. zone inlet)
                            ZFAUZoneVentLoad += (ZFAUFlowRate) * (ZFAUEnthMixedAir - ZFAUEnthReturnAir) * TimeStepSys * SecInHour; //*KJperJ
                        } else {
                            ZFAUZoneVentLoad += 0.0;
                        }

                    } else if (SELECT_CASE_var == PurchasedAir_Num) {
                        ZFAUOutAirFlow +=
                            GetPurchasedAirOutAirMassFlow(state, ZoneEquipList(ZoneEquipConfig(CtrlZoneNum).EquipListIndex).EquipIndex(thisZoneEquipNum));
                        ZoneInletAirNode =
                            GetPurchasedAirZoneInletAirNode(state, ZoneEquipList(ZoneEquipConfig(CtrlZoneNum).EquipListIndex).EquipIndex(thisZoneEquipNum));
                        if (ZoneInletAirNode > 0) ZFAUFlowRate = max(Node(ZoneInletAirNode).MassFlowRate, 0.0);
                        ZFAUTempMixedAir =
                            GetPurchasedAirMixedAirTemp(state, ZoneEquipList(ZoneEquipConfig(CtrlZoneNum).EquipListIndex).EquipIndex(thisZoneEquipNum));
                        ZFAUHumRatMixedAir =
                            GetPurchasedAirMixedAirHumRat(state, ZoneEquipList(ZoneEquipConfig(CtrlZoneNum).EquipListIndex).EquipIndex(thisZoneEquipNum));
                        ReturnAirNode =
                            GetPurchasedAirReturnAirNode(state, ZoneEquipList(ZoneEquipConfig(CtrlZoneNum).EquipListIndex).EquipIndex(thisZoneEquipNum));
                        if ((ZFAUFlowRate > 0) && (ReturnAirNode > 0)) {
                            ZFAUEnthMixedAir = PsyHFnTdbW(ZFAUTempMixedAir, ZFAUHumRatMixedAir);
                            ZFAUEnthReturnAir = PsyHFnTdbW(Node(ReturnAirNode).Temp, Node(ReturnAirNode).HumRat);
                            // Calculate the zone ventilation load for this supply air path (i.e. zone inlet)
                            ZFAUZoneVentLoad += (ZFAUFlowRate) * (ZFAUEnthMixedAir - ZFAUEnthReturnAir) * TimeStepSys * SecInHour; //*KJperJ
                        } else {
                            ZFAUZoneVentLoad += 0.0;
                        }

                    } else if (SELECT_CASE_var == ERVStandAlone_Num) {
                        OutAirNode =
                            GetStandAloneERVOutAirNode(state, ZoneEquipList(ZoneEquipConfig(CtrlZoneNum).EquipListIndex).EquipIndex(thisZoneEquipNum));
                        if (OutAirNode > 0) ZFAUOutAirFlow += Node(OutAirNode).MassFlowRate;

                        ZoneInletAirNode =
                            GetStandAloneERVZoneInletAirNode(state, ZoneEquipList(ZoneEquipConfig(CtrlZoneNum).EquipListIndex).EquipIndex(thisZoneEquipNum));
                        if (ZoneInletAirNode > 0) ZFAUFlowRate = max(Node(ZoneInletAirNode).MassFlowRate, 0.0);
                        MixedAirNode = ZoneInletAirNode;
                        ReturnAirNode =
                            GetStandAloneERVReturnAirNode(state, ZoneEquipList(ZoneEquipConfig(CtrlZoneNum).EquipListIndex).EquipIndex(thisZoneEquipNum));
                        if ((MixedAirNode > 0) && (ReturnAirNode > 0)) {
                            ZFAUEnthMixedAir = PsyHFnTdbW(Node(MixedAirNode).Temp, Node(MixedAirNode).HumRat);
                            ZFAUEnthReturnAir = PsyHFnTdbW(Node(ReturnAirNode).Temp, Node(ReturnAirNode).HumRat);
                            // Calculate the zone ventilation load for this supply air path (i.e. zone inlet)
                            ZFAUZoneVentLoad += (ZFAUFlowRate) * (ZFAUEnthMixedAir - ZFAUEnthReturnAir) * TimeStepSys * SecInHour; //*KJperJ
                        } else {
                            ZFAUZoneVentLoad += 0.0;
                        }

                    } else if (SELECT_CASE_var == ZoneUnitarySys_Num) {
                        // add accounting for OA when unitary system is used as zone equipment

                    } else if (SELECT_CASE_var == OutdoorAirUnit_Num) {
                        OutAirNode =
                            GetOutdoorAirUnitOutAirNode(state, ZoneEquipList(ZoneEquipConfig(CtrlZoneNum).EquipListIndex).EquipIndex(thisZoneEquipNum));
                        if (OutAirNode > 0) ZFAUOutAirFlow += Node(OutAirNode).MassFlowRate;

                        ZoneInletAirNode =
                            GetOutdoorAirUnitZoneInletNode(state, ZoneEquipList(ZoneEquipConfig(CtrlZoneNum).EquipListIndex).EquipIndex(thisZoneEquipNum));
                        if (ZoneInletAirNode > 0) ZFAUFlowRate = max(Node(ZoneInletAirNode).MassFlowRate, 0.0);
                        ReturnAirNode =
                            GetOutdoorAirUnitReturnAirNode(state, ZoneEquipList(ZoneEquipConfig(CtrlZoneNum).EquipListIndex).EquipIndex(thisZoneEquipNum));
                        if ((OutAirNode > 0) && (ReturnAirNode > 0)) {
                            //						ZFAUEnthMixedAir = PsyHFnTdbW( Node( MixedAirNode ).Temp, Node( MixedAirNode
                            //).HumRat
                            //);
                            ZFAUEnthReturnAir = PsyHFnTdbW(Node(ReturnAirNode).Temp, Node(ReturnAirNode).HumRat);
                            ZFAUEnthOutdoorAir = PsyHFnTdbW(Node(OutAirNode).Temp, Node(OutAirNode).HumRat);
                            // Calculate the zone ventilation load for this supply air path (i.e. zone inlet)
                            ZFAUZoneVentLoad += (ZFAUFlowRate) * (ZFAUEnthOutdoorAir - ZFAUEnthReturnAir) * TimeStepSys * SecInHour; //*KJperJ
                        } else {
                            ZFAUZoneVentLoad += 0.0;
                        }

                    } else if (SELECT_CASE_var == ZoneHybridEvaporativeCooler_Num) {
                        OutAirNode =
                            GetHybridUnitaryACOutAirNode(state, ZoneEquipList(ZoneEquipConfig(CtrlZoneNum).EquipListIndex).EquipIndex(thisZoneEquipNum));
                        if (OutAirNode > 0) ZFAUOutAirFlow += Node(OutAirNode).MassFlowRate;

                        ZoneInletAirNode =
                            GetHybridUnitaryACZoneInletNode(state, ZoneEquipList(ZoneEquipConfig(CtrlZoneNum).EquipListIndex).EquipIndex(thisZoneEquipNum));
                        if (ZoneInletAirNode > 0) ZFAUFlowRate = max(Node(ZoneInletAirNode).MassFlowRate, 0.0);

                        ReturnAirNode =
                            GetHybridUnitaryACReturnAirNode(state, ZoneEquipList(ZoneEquipConfig(CtrlZoneNum).EquipListIndex).EquipIndex(thisZoneEquipNum));
                        if ((OutAirNode > 0) && (ReturnAirNode > 0)) {
                            //						ZFAUEnthMixedAir = PsyHFnTdbW( Node( MixedAirNode ).Temp, Node( MixedAirNode
                            //).HumRat
                            //);
                            ZFAUEnthReturnAir = PsyHFnTdbW(Node(ReturnAirNode).Temp, Node(ReturnAirNode).HumRat);
                            ZFAUEnthOutdoorAir = PsyHFnTdbW(Node(OutAirNode).Temp, Node(OutAirNode).HumRat);
                            // Calculate the zone ventilation load for this supply air path (i.e. zone inlet)

                            ZFAUZoneVentLoad += (ZFAUFlowRate) * (ZFAUEnthOutdoorAir - ZFAUEnthReturnAir) * TimeStepSys * SecInHour; //*KJperJ
                        } else {
                            ZFAUZoneVentLoad += 0.0;
                        }

                    } else if (SELECT_CASE_var == UnitHeater_Num || SELECT_CASE_var == VentilatedSlab_Num ||
                               //	ZoneHVAC:EvaporativeCoolerUnit ?????
                               SELECT_CASE_var == ZoneEvaporativeCoolerUnit_Num ||
                               SELECT_CASE_var == AirDistUnit_Num || SELECT_CASE_var == BBWaterConvective_Num ||
                               SELECT_CASE_var == BBElectricConvective_Num || SELECT_CASE_var == HiTempRadiant_Num ||
                               //	not sure how HeatExchanger:* could be used as zone equipment ?????
                               SELECT_CASE_var == LoTempRadiant_Num || SELECT_CASE_var == ZoneExhaustFan_Num || SELECT_CASE_var == HeatXchngr_Num ||
                               // HPWaterHeater can be used as zone equipment
                               SELECT_CASE_var == HPWaterHeater_Num || SELECT_CASE_var == BBWater_Num || SELECT_CASE_var == ZoneDXDehumidifier_Num ||
                               SELECT_CASE_var == BBSteam_Num || SELECT_CASE_var == BBElectric_Num ||
                               SELECT_CASE_var == RefrigerationAirChillerSet_Num || SELECT_CASE_var == UserDefinedZoneHVACForcedAir_Num ||
                               SELECT_CASE_var == CoolingPanel_Num) {
                        // do nothing, OA not included

                    } else {

                        ShowFatalError(
                            "ReportMaxVentilationLoads: Developer must either create accounting for OA or include in final else if to do nothing");
                    }
                }
            }

            // loop over the zone supply air path inlet nodes
            for (ZoneInNum = 1; ZoneInNum <= ZoneEquipConfig(CtrlZoneNum).NumInletNodes; ++ZoneInNum) {
                Real64 AirSysEnthReturnAir = 0.0;    // enthalpy of the return air (mixing box inlet node, return side) [kJ/kgK]
                Real64 AirSysEnthMixedAir = 0.0;     // enthalpy of the mixed air (mixing box outlet node, mixed air side) [kJ/kgK]
                Real64 AirSysZoneVentLoad = 0.0;     // ventilation load attributed to a particular zone from primary air system [J]
                Real64 ADUCoolFlowrate = 0.0;        // Air distribution unit cooling air mass flow rate [kg/s]
                Real64 ADUHeatFlowrate = 0.0;        // Air distribution unit heating air mass flow rate [kg/s]
                Real64 AirSysTotalMixFlowRate = 0.0; // Mixed air mass flow rate [kg/s]
                Real64 AirSysOutAirFlow = 0.0;       // outside air flow rate for zone from primary air system [kg/s]
                // retrieve air loop index
                AirLoopNum = ZoneEquipConfig(CtrlZoneNum).InletNodeAirLoopNum(ZoneInNum);
                MixedAirNode = 0;
                ReturnAirNode = 0;
                AirDistCoolInletNodeNum = 0;
                AirDistHeatInletNodeNum = 0;
                if (AirLoopNum != 0) { // deal with primary air system
                    AirDistCoolInletNodeNum = max(ZoneEquipConfig(CtrlZoneNum).AirDistUnitCool(ZoneInNum).InNode, 0);
                    AirDistHeatInletNodeNum = max(ZoneEquipConfig(CtrlZoneNum).AirDistUnitHeat(ZoneInNum).InNode, 0);
                    // Set for cooling or heating path
                    if (AirDistCoolInletNodeNum > 0 && AirDistHeatInletNodeNum == 0) {
                        ADUCoolFlowrate = max(Node(ZoneEquipConfig(CtrlZoneNum).AirDistUnitCool(ZoneInNum).InNode).MassFlowRate,
                                              0.0); // CR7244 need to accumulate flow across multiple inlets
                    } else if (AirDistHeatInletNodeNum > 0 && AirDistCoolInletNodeNum == 0) {
                        ADUHeatFlowrate = max(Node(ZoneEquipConfig(CtrlZoneNum).AirDistUnitHeat(ZoneInNum).InNode).MassFlowRate,
                                              0.0); // CR7244 need to accumulate flow across multiple inlets
                    } else if (AirDistCoolInletNodeNum > 0 && AirDistHeatInletNodeNum > 0 && AirDistCoolInletNodeNum != AirDistHeatInletNodeNum) {
                        // dual ducts! CR7244 need to accumulate flow across multiple inlets (don't count same inlet twice)
                        ADUHeatFlowrate = max(Node(ZoneEquipConfig(CtrlZoneNum).AirDistUnitHeat(ZoneInNum).InNode).MassFlowRate,
                                              0.0); // CR7244 need to accumulate flow across multiple inlets
                        ADUCoolFlowrate = max(Node(ZoneEquipConfig(CtrlZoneNum).AirDistUnitCool(ZoneInNum).InNode).MassFlowRate,
                                              0.0); // CR7244 need to accumulate flow across multiple inlets
                    } else if (AirDistCoolInletNodeNum > 0 && AirDistHeatInletNodeNum > 0) {
                        // dual ducts! CR7244 need to accumulate flow across multiple inlets (don't count same inlet twice)
                        ADUCoolFlowrate = max(Node(ZoneEquipConfig(CtrlZoneNum).AirDistUnitCool(ZoneInNum).InNode).MassFlowRate,
                                              0.0); // CR7244 need to accumulate flow across multiple inlets
                    } else {
                        // do nothing (already inits)
                    }
                    // Find the mixed air node and return air node of the system that supplies the zone
                    MixedAirNode = PrimaryAirSystem(AirLoopNum).OASysOutletNodeNum;
                    ReturnAirNode = PrimaryAirSystem(AirLoopNum).OASysInletNodeNum;
                }

                if (MixedAirNode == 0 || ReturnAirNode == 0) {
                    AirSysZoneVentLoad = 0.0;
                    AirSysOutAirFlow = 0.0;
                } else {
                    // Calculate return and mixed air ethalpies
                    AirSysEnthReturnAir = PsyHFnTdbW(Node(ReturnAirNode).Temp, Node(ReturnAirNode).HumRat);
                    AirSysEnthMixedAir = PsyHFnTdbW(Node(MixedAirNode).Temp, Node(MixedAirNode).HumRat);

                    if (PrimaryAirSystem(AirLoopNum).OASysExists) {
                        OutAirNode = PrimaryAirSystem(AirLoopNum).OAMixOAInNodeNum;
                        AirSysOutAirFlow = Node(OutAirNode).MassFlowRate;
                    } else {
                        AirSysOutAirFlow = 0.0;
                    }

                    AirSysTotalMixFlowRate = Node(MixedAirNode).MassFlowRate;

                    if (AirSysTotalMixFlowRate != 0.0) {
                        ZoneFlowFrac = (ADUCoolFlowrate + ADUHeatFlowrate) / AirSysTotalMixFlowRate;
                        AirSysOutAirFlow *= ZoneFlowFrac;
                    } else {
                        ZoneFlowFrac = 0.0;
                        AirSysOutAirFlow = 0.0;
                    }
                    // Calculate the zone ventilation load for this supply air path (i.e. zone inlet)
                    AirSysZoneVentLoad =
                        (ADUCoolFlowrate + ADUHeatFlowrate) * (AirSysEnthMixedAir - AirSysEnthReturnAir) * TimeStepSys * SecInHour; //*KJperJ
                }
                ZAirSysZoneVentLoad += AirSysZoneVentLoad;
                ZAirSysOutAirFlow += AirSysOutAirFlow;
            } // primary air system present

            // now combine OA flow from zone forced air units with primary air system
            OutAirFlow = ZAirSysOutAirFlow + ZFAUOutAirFlow;
            // assign report variables
            ZoneOAMassFlow(CtrlZoneNum) = OutAirFlow;
            ZoneOAMass(CtrlZoneNum) = ZoneOAMassFlow(CtrlZoneNum) * TimeStepSys * SecInHour;

            // determine volumetric values from mass flow using standard density (adjusted for elevation)
            ZoneOAVolFlowStdRho(CtrlZoneNum) = ZoneOAMassFlow(CtrlZoneNum) / StdRhoAir;
            ZoneOAVolStdRho(CtrlZoneNum) = ZoneOAVolFlowStdRho(CtrlZoneNum) * TimeStepSys * SecInHour;

            // determine volumetric values from mass flow using current air density for zone (adjusted for elevation)
            currentZoneAirDensity = PsyRhoAirFnPbTdbW(OutBaroPress, MAT(ActualZoneNum), ZoneAirHumRatAvg(ActualZoneNum));
            if (currentZoneAirDensity > 0.0) ZoneOAVolFlowCrntRho(CtrlZoneNum) = ZoneOAMassFlow(CtrlZoneNum) / currentZoneAirDensity;
            ZoneOAVolCrntRho(CtrlZoneNum) = ZoneOAVolFlowCrntRho(CtrlZoneNum) * TimeStepSys * SecInHour;
            if (ZoneVolume > 0.0) ZoneMechACH(CtrlZoneNum) = (ZoneOAVolCrntRho(CtrlZoneNum) / TimeStepSys) / ZoneVolume;

            // store data for predefined tabular report on outside air
            if (ZonePreDefRep(ActualZoneNum).isOccupied) {
                // accumulate the occupied time
                ZonePreDefRep(ActualZoneNum).TotTimeOcc += TimeStepSys;
                // mechnical ventilation
                ZonePreDefRep(ActualZoneNum).MechVentVolTotal += ZoneOAVolCrntRho(CtrlZoneNum);
                if ((ZoneOAVolCrntRho(CtrlZoneNum) / TimeStepSys) < ZonePreDefRep(ActualZoneNum).MechVentVolMin) {
                    ZonePreDefRep(ActualZoneNum).MechVentVolMin = ZoneOAVolCrntRho(CtrlZoneNum) / TimeStepSys;
                }
                // infiltration
                ZonePreDefRep(ActualZoneNum).InfilVolTotal += ZnAirRpt(ActualZoneNum).InfilVolumeCurDensity;
                if (ZnAirRpt(ActualZoneNum).InfilVolumeCurDensity < ZonePreDefRep(ActualZoneNum).InfilVolMin) {
                    ZonePreDefRep(ActualZoneNum).InfilVolMin = ZnAirRpt(ActualZoneNum).InfilVolumeCurDensity;
                }
                //'simple' mechanical ventilation
                ZonePreDefRep(ActualZoneNum).SimpVentVolTotal += ZnAirRpt(ActualZoneNum).VentilVolumeCurDensity;
                if (ZnAirRpt(ActualZoneNum).VentilVolumeCurDensity < ZonePreDefRep(ActualZoneNum).SimpVentVolMin) {
                    ZonePreDefRep(ActualZoneNum).SimpVentVolMin = ZnAirRpt(ActualZoneNum).VentilVolumeCurDensity;
                }
            }

            // now combine Vent load from zone forced air units with primary air system
            ZoneVentLoad = ZAirSysZoneVentLoad + ZFAUZoneVentLoad;
            // cycle if ZoneVentLoad is small
            if (std::abs(ZoneVentLoad) < SmallLoad) continue; // orig. had RETURN here, BG changed to CYCLE for next controlled zone in do loop.

            // Ventilation Heating
            if (ZoneVentLoad > SmallLoad) {
                // Zone cooling load
                if (ZoneLoad < -SmallLoad) {
                    MaxCoolingLoadAddedByVent(CtrlZoneNum) += std::abs(ZoneVentLoad);
                    // Zone heating load
                } else if (ZoneLoad > SmallLoad) {
                    if (ZoneVentLoad > ZoneLoad) {
                        MaxHeatingLoadMetByVent(CtrlZoneNum) += std::abs(ZoneLoad);
                        MaxOverheatingByVent(CtrlZoneNum) += (ZoneVentLoad - ZoneLoad);
                    } else {
                        MaxHeatingLoadMetByVent(CtrlZoneNum) += std::abs(ZoneVentLoad);
                    }
                    // No Zone Load
                } else {
                    MaxNoLoadHeatingByVent(CtrlZoneNum) += std::abs(ZoneVentLoad);
                }

                // Ventilation Cooling
            } else if (ZoneVentLoad < -SmallLoad) {
                // Zone cooling load
                if (ZoneLoad < -SmallLoad) {
                    if (ZoneVentLoad < ZoneLoad) {
                        MaxCoolingLoadMetByVent(CtrlZoneNum) += std::abs(ZoneLoad);
                        MaxOvercoolingByVent(CtrlZoneNum) += std::abs(ZoneVentLoad - ZoneLoad);
                    } else {
                        MaxCoolingLoadMetByVent(CtrlZoneNum) += std::abs(ZoneVentLoad);
                    }
                    // Zone heating load
                } else if (ZoneLoad > SmallLoad) {
                    MaxHeatingLoadAddedByVent(CtrlZoneNum) += std::abs(ZoneVentLoad);
                    // No Zone Load
                } else {
                    MaxNoLoadCoolingByVent(CtrlZoneNum) += std::abs(ZoneVentLoad);
                }

                // Ventilation No Load
            } else {
            }
        } // loop over controlled zones
    }

    void MatchPlantSys(int const AirLoopNum, // counter for zone air distribution inlets
                       int const BranchNum   // counter for zone air distribution inlets
    )
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Dan Fisher
        //       DATE WRITTEN   May 2005
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // calculate and report zone ventilation loads

        // METHODOLOGY EMPLOYED:
        // calculate energy contribution of outside air through mixing box and pro-rate to
        // zones according to zone mass flow rates.

        // REFERENCES:
        // na

        // Using/Aliasing
        using namespace DataGlobalConstants;

        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:

        // SUBROUTINE PARAMETER DEFINITIONS:
        int const EnergyTrans(1);

        // INTERFACE BLOCK SPECIFICATIONS
        // na

        // DERIVED TYPE DEFINITIONS
        // na

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        std::string CompType;
        std::string CompName;
        int CompNum; // counter for components on air loop branch connected to air distribution unit
        int VarNum;
        int SubCompNum;    // counter for components on air loop branch connected to air distribution unit
        int SubSubCompNum; // counter for components on air loop branch connected to air distribution unit
        bool MatchFound;   // Set to .TRUE. when a match is found
        int MatchLoop;     // Loop number of the match
        int MatchBranch;   // Branch number of the match
        int MatchComp;     // Component number of the match
        int MatchLoopType;
        int Idx;

        for (CompNum = 1; CompNum <= PrimaryAirSystem(AirLoopNum).Branch(BranchNum).TotalComponents; ++CompNum) {
            {
                auto &thisComp(PrimaryAirSystem(AirLoopNum).Branch(BranchNum).Comp(CompNum));
                for (VarNum = 1; VarNum <= thisComp.NumMeteredVars; ++VarNum) {
                    if (thisComp.MeteredVar(VarNum).ResourceType == iRT_EnergyTransfer) {
                        thisComp.EnergyTransComp = EnergyTrans;
                        CompType = thisComp.TypeOf;
                        CompName = thisComp.Name;
                        Idx = 0;
                        FindDemandSideMatch(CompType, CompName, MatchFound, MatchLoopType, MatchLoop, MatchBranch, MatchComp);
                        if (MatchFound)
                            UpdateAirSysCompPtrArray(Idx, AirLoopNum, BranchNum, CompNum, MatchLoopType, MatchLoop, MatchBranch, MatchComp);
                        thisComp.AirSysToPlantPtr = Idx;
                        break;
                    }
                }
                for (SubCompNum = 1; SubCompNum <= thisComp.NumSubComps; ++SubCompNum) {
                    //!!!!          IF(SysVentLoad == 0.0d0)EXIT
                    {
                        auto &thisSubComp(thisComp.SubComp(SubCompNum));
                        for (VarNum = 1; VarNum <= thisSubComp.NumMeteredVars; ++VarNum) {
                            if (thisSubComp.MeteredVar(VarNum).ResourceType == iRT_EnergyTransfer) {
                                thisSubComp.EnergyTransComp = EnergyTrans;
                                CompType = thisSubComp.TypeOf;
                                CompName = thisSubComp.Name;
                                Idx = 0;
                                FindDemandSideMatch(CompType, CompName, MatchFound, MatchLoopType, MatchLoop, MatchBranch, MatchComp);
                                if (MatchFound)
                                    UpdateAirSysSubCompPtrArray(
                                        Idx, AirLoopNum, BranchNum, CompNum, SubCompNum, MatchLoopType, MatchLoop, MatchBranch, MatchComp);
                                thisSubComp.AirSysToPlantPtr = Idx;
                                break;
                            }
                        }
                        for (SubSubCompNum = 1; SubSubCompNum <= thisSubComp.NumSubSubComps; ++SubSubCompNum) {
                            //!!!!            IF(SysVentLoad == 0.0d0)EXIT
                            {
                                auto &thisSubSubComp(thisSubComp.SubSubComp(SubSubCompNum));
                                for (VarNum = 1; VarNum <= thisSubSubComp.NumMeteredVars; ++VarNum) {
                                    if (thisSubSubComp.MeteredVar(VarNum).ResourceType == iRT_EnergyTransfer) {
                                        thisSubSubComp.EnergyTransComp = EnergyTrans;
                                        CompType = thisSubSubComp.TypeOf;
                                        CompName = thisSubSubComp.Name;
                                        Idx = 0;
                                        FindDemandSideMatch(CompType, CompName, MatchFound, MatchLoopType, MatchLoop, MatchBranch, MatchComp);
                                        if (MatchFound)
                                            UpdateAirSysSubSubCompPtrArray(Idx,
                                                                           AirLoopNum,
                                                                           BranchNum,
                                                                           CompNum,
                                                                           SubCompNum,
                                                                           SubSubCompNum,
                                                                           MatchLoopType,
                                                                           MatchLoop,
                                                                           MatchBranch,
                                                                           MatchComp);
                                        thisSubSubComp.AirSysToPlantPtr = Idx;
                                        break;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    void FindDemandSideMatch(std::string const &CompType, // Inlet node of the component to find the match of
                             std::string const &CompName, // Outlet node of the component to find the match of
                             bool &MatchFound,            // Set to .TRUE. when a match is found
                             int &MatchLoopType,          // Loop number of the match
                             int &MatchLoop,              // Loop number of the match
                             int &MatchBranch,            // Branch number of the match
                             int &MatchComp               // Component number of the match
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Rick Strand
        //       DATE WRITTEN   September 2004
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine intializes the connections between various loops.
        // Due to the fact that this requires numerous string compares, it
        // is much more efficient to find this information once and then
        // store it in module level variables (LoopConnect derived type).

        // METHODOLOGY EMPLOYED:
        // Simply cycles through the plant and condenser demand sides until
        // a component is found that matches the component type and name

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int PassBranchNum; // DO loop counter for branches
        int PassCompNum;   // DO loop counter for components
        int PassLoopNum;   // DO loop counter for loops or the top level of the hierarchy
        // FLOW:
        // Initialize all of the output variables

        MatchFound = false;
        MatchLoopType = 0;
        MatchLoop = 0;
        MatchLoop = 0;
        MatchBranch = 0;
        MatchComp = 0;

        // Now cycle through all of the demand side loops to see if we can find
        // a match for the component type and name.  Once a match is found,
        // record the type of loop and the loop, branch, and component numbers.
        if (!MatchFound) { // Go through the plant demand side loops
            for (PassLoopNum = 1; PassLoopNum <= NumPlantLoops; ++PassLoopNum) {
                for (PassBranchNum = 1; PassBranchNum <= VentRepPlantDemandSide(PassLoopNum).TotalBranches; ++PassBranchNum) {
                    for (PassCompNum = 1; PassCompNum <= VentRepPlantDemandSide(PassLoopNum).Branch(PassBranchNum).TotalComponents; ++PassCompNum) {
                        if (UtilityRoutines::SameString(CompType,
                                                        VentRepPlantDemandSide(PassLoopNum).Branch(PassBranchNum).Comp(PassCompNum).TypeOf) &&
                            UtilityRoutines::SameString(CompName, VentRepPlantDemandSide(PassLoopNum).Branch(PassBranchNum).Comp(PassCompNum).Name)) {
                            // Found a match on the plant demand side--increment the counter
                            MatchFound = true;
                            MatchLoopType = 1;
                            MatchLoop = PassLoopNum;
                            MatchBranch = PassBranchNum;
                            MatchComp = PassCompNum;
                            break; // PassCompNum DO loop
                        }
                    }
                    if (MatchFound) break; // PassBranchNum DO loop
                }
                if (MatchFound) break; // PassLoopNum DO loop
            }
        }

        if (!MatchFound) { // Go through the condenser demand side loops
            for (PassLoopNum = 1; PassLoopNum <= NumCondLoops; ++PassLoopNum) {
                for (PassBranchNum = 1; PassBranchNum <= VentRepCondDemandSide(PassLoopNum).TotalBranches; ++PassBranchNum) {
                    for (PassCompNum = 1; PassCompNum <= VentRepCondDemandSide(PassLoopNum).Branch(PassBranchNum).TotalComponents; ++PassCompNum) {
                        if (UtilityRoutines::SameString(CompType,
                                                        VentRepCondDemandSide(PassLoopNum).Branch(PassBranchNum).Comp(PassCompNum).TypeOf) &&
                            UtilityRoutines::SameString(CompName, VentRepCondDemandSide(PassLoopNum).Branch(PassBranchNum).Comp(PassCompNum).Name)) {
                            // Found a match on the plant demand side--increment the counter
                            MatchFound = true;
                            MatchLoopType = 2;
                            MatchLoop = PassLoopNum;
                            MatchBranch = PassBranchNum;
                            MatchComp = PassCompNum;
                            break; // PassCompNum DO loop
                        }
                    }
                    if (MatchFound) break; // PassBranchNum DO loop
                }
                if (MatchFound) break; // PassLoopNum DO loop
            }
        }
    }

    void ReportAirLoopConnections(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Michael J. Witte, Linda K. Lawrie
        //       DATE WRITTEN   February 2004 (moved from BranchInputManager ReportLoopConnections)
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Report air loop splitter connections to the BND file.

        // METHODOLOGY EMPLOYED:
        // Needs description, as appropriate.

        // REFERENCES:
        // na

        // Using/Aliasing
        using DataHeatBalance::Zone;
        using DataHVACGlobals::NumPrimaryAirSys;

        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:
        // na

        // SUBROUTINE PARAMETER DEFINITIONS:
        static std::string const errstring("**error**");

        // INTERFACE BLOCK SPECIFICATIONS
        // na

        // DERIVED TYPE DEFINITIONS
        // na

        // Formats
        static constexpr auto Format_706("! <#AirLoopHVACs>,<Number of AirLoopHVACs>");
        static constexpr auto Format_708(
            "! <AirLoopHVAC>,<Air Loop Name>,<# Return Nodes>,<# Supply Nodes>,<# Zones Cooled>,<# Zones Heated>,<Outdoor Air Used>");
        static constexpr auto Format_709("! <AirLoop Return Connections>,<Connection Count>,<AirLoopHVAC Name>,<Zn Eqp Return Node #>,<Zn Eqp Return "
                                         "Node Name>,<AirLoop Return Node #>,<Air Loop Return Node Name>");
        static constexpr auto Format_710("! <AirLoop Supply Connections>,<Connection Count>,<AirLoopHVAC Name>,<Zn Eqp Supply Node #>,<Zn Eqp Supply "
                                         "Node Name>,<AirLoop Supply Node #>,<Air Loop Supply Node Name>");
        static constexpr auto Format_711("! <Cooled Zone Info>,<Cooled Zone Count>,<Cooled Zone Name>,<Cooled Zone Inlet Node #>,<Cooled Zone Inlet "
                                         "Node Name>,<AirLoopHVAC Name>");
        static constexpr auto Format_712("! <Heated Zone Info>,<Heated Zone Count>,<Heated Zone Name>,<Heated Zone Inlet Node #>,<Heated Zone Inlet "
                                         "Node Name>,<AirLoopHVAC Name>");
        static constexpr auto Format_714("! <Outdoor Air Connections>,<OA Inlet Node #>,<OA Return Air Inlet Node Name>,<OA Outlet Node #>,<OA Mixed "
                                         "Air Outlet Node Name>,<AirLoopHVAC Name>");

        print(state.files.bnd, "{}\n", "! ===============================================================");
        print(state.files.bnd, "{}\n", Format_706);
        print(state.files.bnd, " #AirLoopHVACs,{}\n", NumPrimaryAirSys);
        print(state.files.bnd, "{}\n", Format_708);
        print(state.files.bnd, "{}\n", Format_709);
        print(state.files.bnd, "{}\n", Format_710);
        print(state.files.bnd, "{}\n", Format_711);
        print(state.files.bnd, "{}\n", Format_712);
        print(state.files.bnd, "{}\n", Format_714);
        print(state.files.bnd,
              "{}\n",
              "! <AirLoopHVAC Connector>,<Connector Type>,<Connector Name>,<Loop Name>,<Loop Type>,<Number of Inlets/Outlets>");
        print(state.files.bnd,
              "{}\n",
              "! <AirLoopHVAC Connector Branches>,<Connector Node Count>,<Connector Type>,<Connector Name>,<Inlet Branch>,<Outlet Branch>,<Loop "
              "Name>,<Loop Type>");
        print(state.files.bnd,
              "{}\n",
              "! <AirLoopHVAC Connector Nodes>,<Connector Node Count>,<Connector Type>,<Connector Name>,<Inlet Node>,<Outlet Node>,<Loop Name>,<Loop "
              "Type>");
        for (int Count = 1; Count <= NumPrimaryAirSys; ++Count) {
            const auto oaSysExists = [&]() {
                if (state.dataAirLoop->AirToOANodeInfo(Count).OASysExists) {
                    return "Yes";
                } else {
                    return "No";
                }
            }();

            print(state.files.bnd,
                  " AirLoopHVAC,{},{},{},{},{},{}\n",
                  state.dataAirLoop->AirToZoneNodeInfo(Count).AirLoopName,
                  state.dataAirLoop->AirToZoneNodeInfo(Count).NumReturnNodes,
                  state.dataAirLoop->AirToZoneNodeInfo(Count).NumSupplyNodes,
                  state.dataAirLoop->AirToZoneNodeInfo(Count).NumZonesCooled,
                  state.dataAirLoop->AirToZoneNodeInfo(Count).NumZonesHeated,
                  oaSysExists);
            for (int Count1 = 1; Count1 <= state.dataAirLoop->AirToZoneNodeInfo(Count).NumReturnNodes; ++Count1) {
                print(state.files.bnd, "   AirLoop Return Connections,{},{},", Count1, state.dataAirLoop->AirToZoneNodeInfo(Count).AirLoopName);
                if (state.dataAirLoop->AirToZoneNodeInfo(Count).ZoneEquipReturnNodeNum(Count1) > 0) {
                    print(state.files.bnd,
                          "{},{},",
                          state.dataAirLoop->AirToZoneNodeInfo(Count).ZoneEquipReturnNodeNum(Count1),
                          NodeID(state.dataAirLoop->AirToZoneNodeInfo(Count).ZoneEquipReturnNodeNum(Count1)));
                } else {
                    print(state.files.bnd, "{},{},", errstring, errstring);
                }
                if (state.dataAirLoop->AirToZoneNodeInfo(Count).AirLoopReturnNodeNum(Count1) > 0) {
                    print(state.files.bnd,
                          "{},{}\n",
                          state.dataAirLoop->AirToZoneNodeInfo(Count).AirLoopReturnNodeNum(Count1),
                          NodeID(state.dataAirLoop->AirToZoneNodeInfo(Count).AirLoopReturnNodeNum(Count1)));
                } else {
                    print(state.files.bnd, "{},{}\n", errstring, errstring);
                }
            }
            for (int Count1 = 1; Count1 <= state.dataAirLoop->AirToZoneNodeInfo(Count).NumSupplyNodes; ++Count1) {
                print(state.files.bnd, "   AirLoop Supply Connections,{},{},", Count1, state.dataAirLoop->AirToZoneNodeInfo(Count).AirLoopName);
                if (state.dataAirLoop->AirToZoneNodeInfo(Count).ZoneEquipSupplyNodeNum(Count1) > 0) {
                    print(state.files.bnd,
                          "{},{},",
                          state.dataAirLoop->AirToZoneNodeInfo(Count).ZoneEquipSupplyNodeNum(Count1),
                          NodeID(state.dataAirLoop->AirToZoneNodeInfo(Count).ZoneEquipSupplyNodeNum(Count1)));
                } else {
                    print(state.files.bnd, "{},{},", errstring, errstring);
                }
                if (state.dataAirLoop->AirToZoneNodeInfo(Count).AirLoopSupplyNodeNum(Count1) > 0) {
                    print(state.files.bnd,
                          "{},{}\n",
                          state.dataAirLoop->AirToZoneNodeInfo(Count).AirLoopSupplyNodeNum(Count1),
                          NodeID(state.dataAirLoop->AirToZoneNodeInfo(Count).AirLoopSupplyNodeNum(Count1)));
                } else {
                    print(state.files.bnd, "{},{}\n", errstring, errstring);
                }
            }

            for (int Count1 = 1; Count1 <= state.dataAirLoop->AirToZoneNodeInfo(Count).NumZonesCooled; ++Count1) {
                const auto CtrldZoneNum = state.dataAirLoop->AirToZoneNodeInfo(Count).CoolCtrlZoneNums(Count1);
                const auto ZoneNum = ZoneEquipConfig(CtrldZoneNum).ActualZoneNum;
                print(state.files.bnd, "   Cooled Zone Info,{},{},", Count1, Zone(ZoneNum).Name);
                if (state.dataAirLoop->AirToZoneNodeInfo(Count).CoolZoneInletNodes(Count1) > 0) {
                    print(state.files.bnd,
                          "{},{},{}\n",
                          state.dataAirLoop->AirToZoneNodeInfo(Count).CoolZoneInletNodes(Count1),
                          NodeID(state.dataAirLoop->AirToZoneNodeInfo(Count).CoolZoneInletNodes(Count1)),
                          state.dataAirLoop->AirToZoneNodeInfo(Count).AirLoopName);
                } else {
                    print(state.files.bnd, "{},{},{}\n", errstring, errstring, state.dataAirLoop->AirToZoneNodeInfo(Count).AirLoopName);
                }
            }
            for (int Count1 = 1; Count1 <= state.dataAirLoop->AirToZoneNodeInfo(Count).NumZonesHeated; ++Count1) {
                const auto CtrldZoneNum = state.dataAirLoop->AirToZoneNodeInfo(Count).HeatCtrlZoneNums(Count1);
                const auto ZoneNum = ZoneEquipConfig(CtrldZoneNum).ActualZoneNum;
                print(state.files.bnd, "   Heated Zone Info,{},{},", Count1, Zone(ZoneNum).Name);
                if (state.dataAirLoop->AirToZoneNodeInfo(Count).HeatZoneInletNodes(Count1) > 0) {
                    print(state.files.bnd,
                          "{},{},{}\n",
                          state.dataAirLoop->AirToZoneNodeInfo(Count).HeatZoneInletNodes(Count1),
                          NodeID(state.dataAirLoop->AirToZoneNodeInfo(Count).HeatZoneInletNodes(Count1)),
                          state.dataAirLoop->AirToZoneNodeInfo(Count).AirLoopName);
                } else {
                    print(state.files.bnd, "{},{},{}\n", errstring, errstring, state.dataAirLoop->AirToZoneNodeInfo(Count).AirLoopName);
                }
            }
            if (state.dataAirLoop->AirToOANodeInfo(Count).OASysExists) {
                std::string ChrOut;
                std::string ChrOut2;
                if (state.dataAirLoop->AirToOANodeInfo(Count).OASysInletNodeNum > 0) {
                    ChrOut = std::to_string(state.dataAirLoop->AirToOANodeInfo(Count).OASysInletNodeNum);
                } else {
                    ChrOut = errstring;
                }
                if (state.dataAirLoop->AirToOANodeInfo(Count).OASysOutletNodeNum > 0) {
                    ChrOut2 = std::to_string(state.dataAirLoop->AirToOANodeInfo(Count).OASysOutletNodeNum);
                } else {
                    ChrOut2 = errstring;
                }

                print(state.files.bnd, "   Outdoor Air Connections,{},", ChrOut);
                if (ChrOut != errstring) {
                    print(state.files.bnd, "{},", NodeID(state.dataAirLoop->AirToOANodeInfo(Count).OASysInletNodeNum));
                } else {
                    print(state.files.bnd, "{},", errstring);
                }
                if (ChrOut2 != errstring) {
                    print(state.files.bnd,
                          "{},{},{}\n",
                          ChrOut2,
                          NodeID(state.dataAirLoop->AirToOANodeInfo(Count).OASysOutletNodeNum),
                          state.dataAirLoop->AirToZoneNodeInfo(Count).AirLoopName);
                } else {
                    print(state.files.bnd, "{},{},{}\n", errstring, errstring, state.dataAirLoop->AirToZoneNodeInfo(Count).AirLoopName);
                }
            }
            //  Report HVAC Air Loop Splitter to BND file
            if (PrimaryAirSystem(Count).Splitter.Exists) {
                print(state.files.bnd,
                      "   AirLoopHVAC Connector,Splitter,{},{},Air,{}\n",
                      PrimaryAirSystem(Count).Splitter.Name,
                      PrimaryAirSystem(Count).Name,
                      PrimaryAirSystem(Count).Splitter.TotalOutletNodes);
                for (int Count1 = 1; Count1 <= PrimaryAirSystem(Count).Splitter.TotalOutletNodes; ++Count1) {
                    print(state.files.bnd, "     AirLoopHVAC Connector Branches,{},Splitter,{},", Count1, PrimaryAirSystem(Count).Splitter.Name);

                    if (PrimaryAirSystem(Count).Splitter.BranchNumIn <= 0) {
                        print(state.files.bnd, "{},", errstring);
                    } else {
                        print(state.files.bnd, "{},", PrimaryAirSystem(Count).Branch(PrimaryAirSystem(Count).Splitter.BranchNumIn).Name);
                    }

                    if (PrimaryAirSystem(Count).Splitter.BranchNumOut(Count1) <= 0) {
                        print(state.files.bnd, "{},{},Air\n", errstring, PrimaryAirSystem(Count).Name);
                    } else {
                        print(state.files.bnd,
                              "{},{},Air\n",
                              PrimaryAirSystem(Count).Branch(PrimaryAirSystem(Count).Splitter.BranchNumOut(Count1)).Name,
                              PrimaryAirSystem(Count).Name);
                    }

                    print(state.files.bnd,
                          "     AirLoopHVAC Connector Nodes,   {},Splitter,{},{},{},{},Air\n",
                          Count1,
                          PrimaryAirSystem(Count).Splitter.Name,
                          PrimaryAirSystem(Count).Splitter.NodeNameIn,
                          PrimaryAirSystem(Count).Splitter.NodeNameOut(Count1),
                          PrimaryAirSystem(Count).Name);
                }
            }
        }
    }

    //        End of Reporting subroutines for the SimAir Module
    // *****************************************************************************

} // namespace SystemReports

} // namespace EnergyPlus
