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
#include <cmath>

// ObjexxFCL Headers
#include <ObjexxFCL/Array.functions.hh>
#include <ObjexxFCL/Array2D.hh>
#include <ObjexxFCL/Fmath.hh>

// EnergyPlus Headers
#include <EnergyPlus/AirflowNetworkBalanceManager.hh>
#include <EnergyPlus/CurveManager.hh>
#include <EnergyPlus/DataAirLoop.hh>
#include <EnergyPlus/DataAirSystems.hh>
#include <AirflowNetwork/Elements.hpp>
#include <EnergyPlus/DataContaminantBalance.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataHeatBalFanSys.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataIPShortCuts.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/Plant/DataPlant.hh>
#include <EnergyPlus/DataZoneControls.hh>
#include <EnergyPlus/DataZoneEquipment.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/NodeInputManager.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/SystemAvailabilityManager.hh>
#include <EnergyPlus/ThermalComfort.hh>
#include <EnergyPlus/UtilityRoutines.hh>

namespace EnergyPlus {

namespace SystemAvailabilityManager {

    // Module containing the System Availability Manager routines

    // MODULE INFORMATION:
    //       AUTHOR         Fred Buhl
    //       DATE WRITTEN   August 2001
    //       MODIFIED       February 2004, PGE: Added plant managers.
    //       MODIFIED       March 2007, LG: Added hybrid ventilation control.
    //                      August 2008, R. Raustad - FSEC: added 2 new scheduled sys avail managers
    //                      March 2011, Chandan Sharma - FSEC: Added zone sys avail managers
    //                      August 2013, Xiufeng Pang (XP) - added algorithms for optimal start
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS MODULE
    // To encapsulate the data and algorithms required to
    // determine system (loop) availability and "cycle on" status.

    // METHODOLOGY EMPLOYED:
    // Previous time step node data and current zone thermostat setpoints are used
    // in a set of fixed, precoded algorithms to determine the current time step
    // on/off status of systems and loops.

    // USE STATEMENTS:
    // Use statements for data only modules
    using namespace DataGlobals;
    using namespace DataHVACGlobals;
    using namespace ScheduleManager;
    using DataAirSystems::PrimaryAirSystem;
    using DataHeatBalance::ZoneList;

    // Data
    // MODULE PARAMETER DEFINITIONS
    static std::string const BlankString;

    int const MaxDayTypes(12);
    int const StayOff(0);
    int const CycleOnAny(1);
    int const CycleOnControlZone(2);
    int const ZoneFansOnly(3);
    int const CycleOnAnyCoolingOrHeatingZone(4);
    int const CycleOnAnyCoolingZone(5);
    int const CycleOnAnyHeatingZone(6);
    int const CycleOnAnyHeatingZoneFansOnly(7);

    // Cycling Run Time Control Type
    int const FixedRunTime(1);
    int const Thermostat(2);
    int const ThermostatWithMinimumRunTime(3);

    // Optimum start parameter definations
    int const ControlZone(4);
    int const MaximumOfZoneList(5);

    int const ConstantTemperatureGradient(0);
    int const AdaptiveTemperatureGradient(1);
    int const AdaptiveASHRAE(2);
    int const ConstantStartTime(3);

    // Hybrid Ventilation parameters
    int const HybridVentMode_No(0);       // No hybrid ventilation control
    int const HybridVentMode_Temp(1);     // Temperature control
    int const HybridVentMode_Enth(2);     // Enthalpy control
    int const HybridVentMode_DewPoint(3); // Dew point control
    int const HybridVentMode_OA(4);       // Outdoor air control
    int const HybridVentMode_OperT80(5);  // Operative temperature control with 80% acceptability limits
    int const HybridVentMode_OperT90(6);  // Operative temperature control with 90% acceptability limits
    int const HybridVentMode_CO2(7);      // CO2 control

    int const HybridVentCtrl_NoAction(0); // No hybrid ventilation control
    int const HybridVentCtrl_Open(1);     // Open windows or doors
    int const HybridVentCtrl_Close(2);    // Close windows or doors

    int const NumValidSysAvailManagerTypes(12);
    Array1D_string const cValidSysAvailManagerTypes(NumValidSysAvailManagerTypes,
                                                    {"AvailabilityManager:Scheduled",
                                                     "AvailabilityManager:ScheduledOn",
                                                     "AvailabilityManager:ScheduledOff",
                                                     "AvailabilityManager:NightCycle",
                                                     "AvailabilityManager:DifferentialThermostat",
                                                     "AvailabilityManager:HighTemperatureTurnOff",
                                                     "AvailabilityManager:HighTemperatureTurnOn",
                                                     "AvailabilityManager:LowTemperatureTurnOff",
                                                     "AvailabilityManager:LowTemperatureTurnOn",
                                                     "AvailabilityManager:NightVentilation",
                                                     "AvailabilityManager:HybridVentilation",
                                                     "AvailabilityManager:OptimumStart"});
    int const SysAvailMgr_Scheduled(1);
    int const SysAvailMgr_ScheduledOn(2);
    int const SysAvailMgr_ScheduledOff(3);
    int const SysAvailMgr_NightCycle(4);
    int const SysAvailMgr_DiffThermo(5);
    int const SysAvailMgr_HiTempTOff(6);
    int const SysAvailMgr_HiTempTOn(7);
    int const SysAvailMgr_LoTempTOff(8);
    int const SysAvailMgr_LoTempTOn(9);
    int const SysAvailMgr_NightVent(10);
    int const SysAvailMgr_HybridVent(11);

    int const SysAvailMgr_OptimumStart(12);
    Array1D_int const ValidSysAvailManagerTypes(NumValidSysAvailManagerTypes,
                                                {SysAvailMgr_Scheduled,
                                                 SysAvailMgr_ScheduledOn,
                                                 SysAvailMgr_ScheduledOff,
                                                 SysAvailMgr_NightCycle,
                                                 SysAvailMgr_DiffThermo,
                                                 SysAvailMgr_HiTempTOff,
                                                 SysAvailMgr_HiTempTOn,
                                                 SysAvailMgr_LoTempTOff,
                                                 SysAvailMgr_LoTempTOn,
                                                 SysAvailMgr_NightVent,
                                                 SysAvailMgr_HybridVent,
                                                 SysAvailMgr_OptimumStart});
    // DERIVED TYPE DEFINITIONS

    // Not used yet

    // MODULE VARIABLE DECLARATIONS

    int NumSchedSysAvailMgrs(0);
    int NumSchedOnSysAvailMgrs(0);
    int NumSchedOffSysAvailMgrs(0);
    int NumNCycSysAvailMgrs(0);
    int NumDiffTSysAvailMgrs(0);
    int NumHiTurnOffSysAvailMgrs(0);
    int NumHiTurnOnSysAvailMgrs(0);
    int NumLoTurnOffSysAvailMgrs(0);
    int NumLoTurnOnSysAvailMgrs(0);
    int NumNVentSysAvailMgrs(0);
    int NumAvailManagerLists(0);
    bool GetAvailListsInput(true);
    bool GetAvailMgrInputFlag(true); // First time, input is "gotten"
    bool GetHybridInputFlag(true);   // Flag set to make sure you get input once
    int NumOptStartSysAvailMgrs(0);
    bool BeginOfDayResetFlag(true);

    Real64 CurrentEndTime(0.0);     // Current end time
    Real64 CurrentEndTimeLast(0.0); // last end time
    Real64 TimeStepSysLast(0.0);    // last system time step

    namespace {
        // These were static variables within different functions. They were pulled out into the namespace
        // to facilitate easier unit testing of those functions.
        // These are purposefully not in the header file as an extern variable. No one outside of this should
        // use these. They are cleared by clear_state() for use by unit tests, but normal simulations should be unaffected.
        // This is purposefully in an anonymous namespace so nothing outside this implementation file can use it.
        bool InitSysAvailManagers_MyOneTimeFlag(true);
        bool CalcNCycSysAvailMgr_OneTimeFlag(true);
        Array1D<Real64> OptStart_AdaTempGradTrdHeat; // Heating temp gradient for previous days - used in CalcOptStartSysAvailMgr
        Array1D<Real64> OptStart_AdaTempGradTrdCool; // Cooling temp gradient for previous days - used in CalcOptStartSysAvailMgr
    }                                                // namespace

    // SUBROUTINE SPECIFICATIONS FOR MODULE

    // Object Data
    Array1D<DefineSchedSysAvailManager> SchedSysAvailMgrData;
    Array1D<DefineSchedOnSysAvailManager> SchedOnSysAvailMgrData;
    Array1D<DefineSchedOffSysAvailManager> SchedOffSysAvailMgrData;
    Array1D<DefineNightCycSysAvailManager> NCycSysAvailMgrData;
    Array1D<DefineDiffTSysAvailManager> DiffTSysAvailMgrData;
    Array1D<DefineHiLoSysAvailManager> HiTurnOffSysAvailMgrData;
    Array1D<DefineHiLoSysAvailManager> HiTurnOnSysAvailMgrData;
    Array1D<DefineHiLoSysAvailManager> LoTurnOffSysAvailMgrData;
    Array1D<DefineHiLoSysAvailManager> LoTurnOnSysAvailMgrData;
    Array1D<DefineNightVentSysAvailManager> NVentSysAvailMgrData;
    Array1D<DefineHybridVentSysAvailManager> HybridVentSysAvailMgrData;
    Array1D<SysAvailManagerList> SysAvailMgrListData;
    Array1D<DefineOptStartSysAvailManager> OptStartSysAvailMgrData;
    Array1D<DefineASHRAEAdaptiveOptimumStartCoeffs> ASHRAEOptSCoeffCooling;
    Array1D<DefineASHRAEAdaptiveOptimumStartCoeffs> ASHRAEOptSCoeffHeating;

    // Functions
    void clear_state()
    {
        NumSchedSysAvailMgrs = 0;
        NumSchedOnSysAvailMgrs = 0;
        NumSchedOffSysAvailMgrs = 0;
        NumNCycSysAvailMgrs = 0;
        NumDiffTSysAvailMgrs = 0;
        NumHiTurnOffSysAvailMgrs = 0;
        NumHiTurnOnSysAvailMgrs = 0;
        NumLoTurnOffSysAvailMgrs = 0;
        NumLoTurnOnSysAvailMgrs = 0;
        NumNVentSysAvailMgrs = 0;
        NumAvailManagerLists = 0;
        GetAvailListsInput = true;
        GetAvailMgrInputFlag = true;
        GetHybridInputFlag = true;
        InitSysAvailManagers_MyOneTimeFlag = true;
        CalcNCycSysAvailMgr_OneTimeFlag = true;
        NumOptStartSysAvailMgrs = 0;
        SchedSysAvailMgrData.deallocate();
        SchedOnSysAvailMgrData.deallocate();
        SchedOffSysAvailMgrData.deallocate();
        NCycSysAvailMgrData.deallocate();
        DiffTSysAvailMgrData.deallocate();
        HiTurnOffSysAvailMgrData.deallocate();
        HiTurnOnSysAvailMgrData.deallocate();
        LoTurnOffSysAvailMgrData.deallocate();
        LoTurnOnSysAvailMgrData.deallocate();
        NVentSysAvailMgrData.deallocate();
        HybridVentSysAvailMgrData.deallocate();
        SysAvailMgrListData.deallocate();
        OptStartSysAvailMgrData.deallocate();
        ASHRAEOptSCoeffCooling.deallocate();
        ASHRAEOptSCoeffHeating.deallocate();
        BeginOfDayResetFlag = true;
        OptStart_AdaTempGradTrdHeat.deallocate();
        OptStart_AdaTempGradTrdCool.deallocate();
    }

    void ManageSystemAvailability(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Fred Buhl
        //       DATE WRITTEN   August 2001
        //       MODIFIED       L. Gu, April, 2007. Added hybrid ventilation control
        //                      Chandan Sharma, March 2011/July 2012 - FSEC: Added zone sys avail managers
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Manage the simulation of the System Availability Managers

        // METHODOLOGY EMPLOYED:
        // NA

        // REFERENCES:
        // NA

        // Using/Aliasing
        using DataZoneEquipment::NumValidSysAvailZoneComponents;
        using DataZoneEquipment::ZoneEquipAvail;
        using namespace DataLoopNode;
        using namespace DataAirLoop;
        using namespace DataPlant;

        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:
        // None

        // SUBROUTINE PARAMETER DEFINITIONS:
        // na

        // INTERFACE BLOCK SPECIFICATIONS:
        // na

        // DERIVED TYPE DEFINITIONS:
        // na

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int PriAirSysNum;         // Primary Air System index
        int PriAirSysAvailMgrNum; // Index of Sys Avail Manager in a Primary Air System
        int PlantNum;             // Plant Loop index
        int PlantAvailMgrNum;     // Index of Plant Avail Manager in a Plant Loop
        int AvailStatus;
        int PreviousStatus;
        int ZoneInSysNum;
        int CtrldZoneNum;
        int HybridVentNum;           // Hybrid ventilation control number
        int ZoneEquipType;           // Type of ZoneHVAC:* component
        int CompNum;                 // Index of ZoneHVAC:* component
        int ZoneCompAvailMgrNum;     // Index of availability manager associated with the ZoneHVAC:* component
        static int DummyArgument(1); // This variable is used when SimSysAvailManager is called for a ZoneHVAC:* component

        if (GetAvailMgrInputFlag) {
            GetSysAvailManagerInputs(state);
            GetAvailMgrInputFlag = false;
            return;
        }

        InitSysAvailManagers();

        for (PriAirSysNum = 1; PriAirSysNum <= NumPrimaryAirSys; ++PriAirSysNum) { // loop over the primary air systems

            PreviousStatus = state.dataAirLoop->PriAirSysAvailMgr(PriAirSysNum).AvailStatus; // Save the previous status for differential thermostat
            state.dataAirLoop->PriAirSysAvailMgr(PriAirSysNum).AvailStatus = NoAction;       // initialize the availability to "take no action"

            for (PriAirSysAvailMgrNum = 1; PriAirSysAvailMgrNum <= state.dataAirLoop->PriAirSysAvailMgr(PriAirSysNum).NumAvailManagers;
                 ++PriAirSysAvailMgrNum) { // loop over the avail managers in system

                SimSysAvailManager(state, state.dataAirLoop->PriAirSysAvailMgr(PriAirSysNum).AvailManagerType(PriAirSysAvailMgrNum),
                                   state.dataAirLoop->PriAirSysAvailMgr(PriAirSysNum).AvailManagerName(PriAirSysAvailMgrNum),
                                   state.dataAirLoop->PriAirSysAvailMgr(PriAirSysNum).AvailManagerNum(PriAirSysAvailMgrNum),
                                   PriAirSysNum,
                                   PreviousStatus,
                                   AvailStatus);

                if (AvailStatus == ForceOff) {
                    state.dataAirLoop->PriAirSysAvailMgr(PriAirSysNum).AvailStatus = ForceOff;
                    break; // Fans forced off takes precedence
                } else if (AvailStatus == CycleOnZoneFansOnly) {
                    state.dataAirLoop->PriAirSysAvailMgr(PriAirSysNum).AvailStatus = CycleOnZoneFansOnly; // zone fans only takes next precedence
                } else if ((AvailStatus == CycleOn) && (state.dataAirLoop->PriAirSysAvailMgr(PriAirSysNum).AvailStatus == NoAction)) {
                    state.dataAirLoop->PriAirSysAvailMgr(PriAirSysNum).AvailStatus = CycleOn; // cycle on is lowest precedence
                }

            } // end of availability manager loop

            // Add hybrid ventilation control
            if (NumHybridVentSysAvailMgrs > 0) {
                for (HybridVentNum = 1; HybridVentNum <= NumHybridVentSysAvailMgrs; ++HybridVentNum) {
                    if (HybridVentSysAvailMgrData(HybridVentNum).AirLoopNum == PriAirSysNum &&
                        HybridVentSysAvailMgrData(HybridVentNum).VentilationCtrl == HybridVentCtrl_Open) {
                        state.dataAirLoop->PriAirSysAvailMgr(PriAirSysNum).AvailStatus = ForceOff; // Force the system off
                    }
                }
            }

            // loop over the zones served by the system and set the zone equipment availability
            for (ZoneInSysNum = 1; ZoneInSysNum <= state.dataAirLoop->AirToZoneNodeInfo(PriAirSysNum).NumZonesCooled; ++ZoneInSysNum) {

                CtrldZoneNum = state.dataAirLoop->AirToZoneNodeInfo(PriAirSysNum).CoolCtrlZoneNums(ZoneInSysNum);
                ZoneEquipAvail(CtrldZoneNum) = state.dataAirLoop->PriAirSysAvailMgr(PriAirSysNum).AvailStatus;
            }

        } // end of primary air system loop

        for (PlantNum = 1; PlantNum <= NumPlantLoops; ++PlantNum) {

            PreviousStatus = PlantAvailMgr(PlantNum).AvailStatus; // Save the previous status for differential thermostat
            PlantAvailMgr(PlantNum).AvailStatus = NoAction;       // Initialize the availability to "take no action"

            for (PlantAvailMgrNum = 1; PlantAvailMgrNum <= PlantAvailMgr(PlantNum).NumAvailManagers;
                 ++PlantAvailMgrNum) { // loop over the avail managers in plant

                SimSysAvailManager(state,
                                   PlantAvailMgr(PlantNum).AvailManagerType(PlantAvailMgrNum),
                                   PlantAvailMgr(PlantNum).AvailManagerName(PlantAvailMgrNum),
                                   PlantAvailMgr(PlantNum).AvailManagerNum(PlantAvailMgrNum),
                                   PlantNum,
                                   PreviousStatus,
                                   AvailStatus);

                if (AvailStatus != NoAction) {
                    PlantAvailMgr(PlantNum).AvailStatus = AvailStatus;
                    break; // First manager to do anything other than "NoAction" gets to set the availability
                }

            } // end of availability manager loop

        } // end of plant loop

        for (ZoneEquipType = 1; ZoneEquipType <= NumValidSysAvailZoneComponents;
             ++ZoneEquipType) { // loop over the zone equipment types which allow system avail managers
            if (allocated(ZoneComp)) {
                if (ZoneComp(ZoneEquipType).TotalNumComp > 0) {
                    for (CompNum = 1; CompNum <= ZoneComp(ZoneEquipType).TotalNumComp; ++CompNum) {
                        if (allocated(ZoneComp(ZoneEquipType).ZoneCompAvailMgrs)) {
                            if (ZoneComp(ZoneEquipType).ZoneCompAvailMgrs(CompNum).NumAvailManagers > 0) {
                                // Save the previous status for differential thermostat
                                PreviousStatus = ZoneComp(ZoneEquipType).ZoneCompAvailMgrs(CompNum).AvailStatus;
                                // initialize the availability to "take no action"
                                ZoneComp(ZoneEquipType).ZoneCompAvailMgrs(CompNum).AvailStatus = NoAction;
                                for (ZoneCompAvailMgrNum = 1;
                                     ZoneCompAvailMgrNum <= ZoneComp(ZoneEquipType).ZoneCompAvailMgrs(CompNum).NumAvailManagers;
                                     ++ZoneCompAvailMgrNum) {
                                    // loop over the avail managers in ZoneHVAC:* components
                                    SimSysAvailManager(state,
                                                       ZoneComp(ZoneEquipType).ZoneCompAvailMgrs(CompNum).AvailManagerType(ZoneCompAvailMgrNum),
                                                       ZoneComp(ZoneEquipType).ZoneCompAvailMgrs(CompNum).AvailManagerName(ZoneCompAvailMgrNum),
                                                       ZoneComp(ZoneEquipType).ZoneCompAvailMgrs(CompNum).AvailManagerNum(ZoneCompAvailMgrNum),
                                                       DummyArgument,
                                                       PreviousStatus,
                                                       AvailStatus,
                                                       ZoneEquipType,
                                                       CompNum);
                                    if (AvailStatus == ForceOff) {
                                        ZoneComp(ZoneEquipType).ZoneCompAvailMgrs(CompNum).AvailStatus = ForceOff;
                                        break; // Fans forced off takes precedence
                                    } else if ((AvailStatus == CycleOn) &&
                                               (ZoneComp(ZoneEquipType).ZoneCompAvailMgrs(CompNum).AvailStatus == NoAction)) {
                                        // cycle on is next precedence
                                        ZoneComp(ZoneEquipType).ZoneCompAvailMgrs(CompNum).AvailStatus = CycleOn;
                                    }
                                } // end of availability manager loop
                            }
                        } else {
                            ZoneComp(ZoneEquipType).ZoneCompAvailMgrs(CompNum).AvailStatus = NoAction;
                        }
                        if (ZoneComp(ZoneEquipType).ZoneCompAvailMgrs(CompNum).ZoneNum > 0) {
                            if (NumHybridVentSysAvailMgrs > 0) {
                                for (HybridVentNum = 1; HybridVentNum <= NumHybridVentSysAvailMgrs; ++HybridVentNum) {
                                    if (!HybridVentSysAvailMgrData(HybridVentNum).HybridVentMgrConnectedToAirLoop) {
                                        if (HybridVentSysAvailMgrData(HybridVentNum).ActualZoneNum ==
                                            ZoneComp(ZoneEquipType).ZoneCompAvailMgrs(CompNum).ZoneNum) {
                                            if (HybridVentSysAvailMgrData(HybridVentNum).VentilationCtrl == HybridVentCtrl_Open) {
                                                ZoneComp(ZoneEquipType).ZoneCompAvailMgrs(CompNum).AvailStatus = ForceOff;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        } // end of zone equip types
    }

    void GetSysAvailManagerInputs(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Fred Buhl
        //       DATE WRITTEN   August 2001
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Obtains input data for System Availability Managers and stores it in
        // appropriate data structures.

        // METHODOLOGY EMPLOYED:
        // Uses InputProcessor "Get" routines to obtain data.

        // Using/Aliasing
        using DataHeatBalance::NumOfZoneLists;
        using DataHeatBalance::Zone;
        using DataHeatBalance::ZoneList;
        using NodeInputManager::GetOnlySingleNode;
        using NodeInputManager::MarkNode;
        using namespace DataLoopNode;
        using DataZoneEquipment::cValidSysAvailManagerCompTypes;
        using DataZoneEquipment::NumValidSysAvailZoneComponents;

        // SUBROUTINE PARAMETER DEFINITIONS:
        static std::string const RoutineName("GetSysAvailManagerInputs: "); // include trailing blank

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Array1D_string cAlphaFieldNames;
        Array1D_string cNumericFieldNames;
        Array1D_bool lNumericFieldBlanks;
        Array1D_bool lAlphaFieldBlanks;
        Array1D_string cAlphaArgs;
        Array1D<Real64> rNumericArgs;
        std::string cCurrentModuleObject;
        int NumAlphas;                  // Number of Alphas for each GetObjectItem call
        int NumNumbers;                 // Number of Numbers for each GetObjectItem call
        int maxAlphas;                  // maximum number of alphas for this set of objects
        int maxNumbers;                 // maximum number of numbers for this set of objects
        int numArgs;                    // maximum number of arguments for this set of objects
        int IOStatus;                   // Used in GetObjectItem
        static bool ErrorsFound(false); // Set to true if errors in input, fatal at end of routine
        int SysAvailNum;                // DO loop index for all System Availability Managers
        int CyclingTimeSteps;
        int ZoneEquipType;
        int TotalNumComp;
        int ZoneListNum;
        int ZoneNumInList;

        // Get the number of occurrences of each type of manager and read in data
        cCurrentModuleObject = "AvailabilityManager:Scheduled";
        inputProcessor->getObjectDefMaxArgs(cCurrentModuleObject, numArgs, NumAlphas, NumNumbers);
        maxNumbers = NumNumbers;
        maxAlphas = NumAlphas;
        cCurrentModuleObject = "AvailabilityManager:ScheduledOn";
        inputProcessor->getObjectDefMaxArgs(cCurrentModuleObject, numArgs, NumAlphas, NumNumbers);
        maxNumbers = max(maxNumbers, NumNumbers);
        maxAlphas = max(maxAlphas, NumAlphas);
        cCurrentModuleObject = "AvailabilityManager:ScheduledOff";
        inputProcessor->getObjectDefMaxArgs(cCurrentModuleObject, numArgs, NumAlphas, NumNumbers);
        maxNumbers = max(maxNumbers, NumNumbers);
        maxAlphas = max(maxAlphas, NumAlphas);
        cCurrentModuleObject = "AvailabilityManager:NightCycle";
        inputProcessor->getObjectDefMaxArgs(cCurrentModuleObject, numArgs, NumAlphas, NumNumbers);
        maxNumbers = max(maxNumbers, NumNumbers);
        maxAlphas = max(maxAlphas, NumAlphas);
        cCurrentModuleObject = "AvailabilityManager:DifferentialThermostat";
        inputProcessor->getObjectDefMaxArgs(cCurrentModuleObject, numArgs, NumAlphas, NumNumbers);
        maxNumbers = max(maxNumbers, NumNumbers);
        maxAlphas = max(maxAlphas, NumAlphas);
        cCurrentModuleObject = "AvailabilityManager:HighTemperatureTurnOff";
        inputProcessor->getObjectDefMaxArgs(cCurrentModuleObject, numArgs, NumAlphas, NumNumbers);
        maxNumbers = max(maxNumbers, NumNumbers);
        maxAlphas = max(maxAlphas, NumAlphas);
        cCurrentModuleObject = "AvailabilityManager:HighTemperatureTurnOn";
        inputProcessor->getObjectDefMaxArgs(cCurrentModuleObject, numArgs, NumAlphas, NumNumbers);
        maxNumbers = max(maxNumbers, NumNumbers);
        maxAlphas = max(maxAlphas, NumAlphas);
        cCurrentModuleObject = "AvailabilityManager:LowTemperatureTurnOff";
        inputProcessor->getObjectDefMaxArgs(cCurrentModuleObject, numArgs, NumAlphas, NumNumbers);
        maxNumbers = max(maxNumbers, NumNumbers);
        maxAlphas = max(maxAlphas, NumAlphas);
        cCurrentModuleObject = "AvailabilityManager:LowTemperatureTurnOn";
        inputProcessor->getObjectDefMaxArgs(cCurrentModuleObject, numArgs, NumAlphas, NumNumbers);
        maxNumbers = max(maxNumbers, NumNumbers);
        maxAlphas = max(maxAlphas, NumAlphas);
        cCurrentModuleObject = "AvailabilityManager:NightVentilation";
        inputProcessor->getObjectDefMaxArgs(cCurrentModuleObject, numArgs, NumAlphas, NumNumbers);
        maxNumbers = max(maxNumbers, NumNumbers);
        maxAlphas = max(maxAlphas, NumAlphas);
        cCurrentModuleObject = "AvailabilityManager:OptimumStart";
        inputProcessor->getObjectDefMaxArgs(cCurrentModuleObject, numArgs, NumAlphas, NumNumbers);
        maxNumbers = max(maxNumbers, NumNumbers);
        maxAlphas = max(maxAlphas, NumAlphas);

        cAlphaFieldNames.allocate(maxAlphas);
        cAlphaArgs.allocate(maxAlphas);
        lAlphaFieldBlanks.dimension(maxAlphas, false);
        cNumericFieldNames.allocate(maxNumbers);
        rNumericArgs.dimension(maxNumbers, 0.0);
        lNumericFieldBlanks.dimension(maxNumbers, false);

        if (!allocated(ZoneComp)) {
            ZoneComp.allocate(NumValidSysAvailZoneComponents);
        }

        for (ZoneEquipType = 1; ZoneEquipType <= NumValidSysAvailZoneComponents; ++ZoneEquipType) {
            if (!allocated(ZoneComp(ZoneEquipType).ZoneCompAvailMgrs)) {
                TotalNumComp = inputProcessor->getNumObjectsFound(cValidSysAvailManagerCompTypes(ZoneEquipType));
                ZoneComp(ZoneEquipType).TotalNumComp = TotalNumComp;
                if (TotalNumComp > 0) {
                    ZoneComp(ZoneEquipType).ZoneCompAvailMgrs.allocate(TotalNumComp);
                }
            }
        }

        cCurrentModuleObject = "AvailabilityManager:Scheduled";
        NumSchedSysAvailMgrs = inputProcessor->getNumObjectsFound(cCurrentModuleObject);

        if (NumSchedSysAvailMgrs > 0) {

            SchedSysAvailMgrData.allocate(NumSchedSysAvailMgrs);

            for (SysAvailNum = 1; SysAvailNum <= NumSchedSysAvailMgrs; ++SysAvailNum) {

                inputProcessor->getObjectItem(state,
                                              cCurrentModuleObject,
                                              SysAvailNum,
                                              cAlphaArgs,
                                              NumAlphas,
                                              rNumericArgs,
                                              NumNumbers,
                                              IOStatus,
                                              lNumericFieldBlanks,
                                              lAlphaFieldBlanks,
                                              cAlphaFieldNames,
                                              cNumericFieldNames);
                UtilityRoutines::IsNameEmpty(cAlphaArgs(1), cCurrentModuleObject, ErrorsFound);
                SchedSysAvailMgrData(SysAvailNum).Name = cAlphaArgs(1);
                SchedSysAvailMgrData(SysAvailNum).MgrType = SysAvailMgr_Scheduled;

                SchedSysAvailMgrData(SysAvailNum).SchedPtr = GetScheduleIndex(state, cAlphaArgs(2));
                if (SchedSysAvailMgrData(SysAvailNum).SchedPtr == 0) {
                    ShowSevereError(RoutineName + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\", invalid");
                    ShowContinueError("not found: " + cAlphaFieldNames(2) + "=\"" + cAlphaArgs(2) + "\".");
                    ErrorsFound = true;
                }

                SetupOutputVariable(state, "Availability Manager Scheduled Control Status",
                                    OutputProcessor::Unit::None,
                                    SchedSysAvailMgrData(SysAvailNum).AvailStatus,
                                    "System",
                                    "Average",
                                    SchedSysAvailMgrData(SysAvailNum).Name);

            } // SysAvailNum
        }

        cCurrentModuleObject = "AvailabilityManager:ScheduledOn";
        NumSchedOnSysAvailMgrs = inputProcessor->getNumObjectsFound(cCurrentModuleObject);

        if (NumSchedOnSysAvailMgrs > 0) {

            SchedOnSysAvailMgrData.allocate(NumSchedOnSysAvailMgrs);

            for (SysAvailNum = 1; SysAvailNum <= NumSchedOnSysAvailMgrs; ++SysAvailNum) {

                inputProcessor->getObjectItem(state, cCurrentModuleObject,
                                              SysAvailNum,
                                              cAlphaArgs,
                                              NumAlphas,
                                              rNumericArgs,
                                              NumNumbers,
                                              IOStatus,
                                              lNumericFieldBlanks,
                                              lAlphaFieldBlanks,
                                              cAlphaFieldNames,
                                              cNumericFieldNames);
                UtilityRoutines::IsNameEmpty(cAlphaArgs(1), cCurrentModuleObject, ErrorsFound);
                SchedOnSysAvailMgrData(SysAvailNum).Name = cAlphaArgs(1);
                SchedOnSysAvailMgrData(SysAvailNum).MgrType = SysAvailMgr_ScheduledOn;

                SchedOnSysAvailMgrData(SysAvailNum).SchedPtr = GetScheduleIndex(state, cAlphaArgs(2));
                if (SchedOnSysAvailMgrData(SysAvailNum).SchedPtr == 0) {
                    ShowSevereError(RoutineName + cCurrentModuleObject + " = \"" + cAlphaArgs(1) + "\", invalid");
                    ShowContinueError("not found: " + cAlphaFieldNames(2) + "=\"" + cAlphaArgs(2) + "\".");
                    ErrorsFound = true;
                }

                SetupOutputVariable(state, "Availability Manager Scheduled On Control Status",
                                    OutputProcessor::Unit::None,
                                    SchedOnSysAvailMgrData(SysAvailNum).AvailStatus,
                                    "System",
                                    "Average",
                                    SchedOnSysAvailMgrData(SysAvailNum).Name);

            } // SysAvailNum
        }

        cCurrentModuleObject = "AvailabilityManager:ScheduledOff";
        NumSchedOffSysAvailMgrs = inputProcessor->getNumObjectsFound(cCurrentModuleObject);

        if (NumSchedOffSysAvailMgrs > 0) {

            SchedOffSysAvailMgrData.allocate(NumSchedOffSysAvailMgrs);

            for (SysAvailNum = 1; SysAvailNum <= NumSchedOffSysAvailMgrs; ++SysAvailNum) {

                inputProcessor->getObjectItem(state, cCurrentModuleObject,
                                              SysAvailNum,
                                              cAlphaArgs,
                                              NumAlphas,
                                              rNumericArgs,
                                              NumNumbers,
                                              IOStatus,
                                              lNumericFieldBlanks,
                                              lAlphaFieldBlanks,
                                              cAlphaFieldNames,
                                              cNumericFieldNames);
                UtilityRoutines::IsNameEmpty(cAlphaArgs(1), cCurrentModuleObject, ErrorsFound);
                SchedOffSysAvailMgrData(SysAvailNum).Name = cAlphaArgs(1);
                SchedOffSysAvailMgrData(SysAvailNum).MgrType = SysAvailMgr_ScheduledOff;

                SchedOffSysAvailMgrData(SysAvailNum).SchedPtr = GetScheduleIndex(state, cAlphaArgs(2));
                if (SchedOffSysAvailMgrData(SysAvailNum).SchedPtr == 0) {
                    ShowSevereError(RoutineName + cCurrentModuleObject + " = \"" + cAlphaArgs(1) + "\", invalid");
                    ShowContinueError("not found: " + cAlphaFieldNames(2) + "=\"" + cAlphaArgs(2) + "\".");
                    ErrorsFound = true;
                }

                SetupOutputVariable(state, "Availability Manager Scheduled Off Control Status",
                                    OutputProcessor::Unit::None,
                                    SchedOffSysAvailMgrData(SysAvailNum).AvailStatus,
                                    "System",
                                    "Average",
                                    SchedOffSysAvailMgrData(SysAvailNum).Name);

            } // SysAvailNum
        }

        cCurrentModuleObject = "AvailabilityManager:NightCycle";
        NumNCycSysAvailMgrs = inputProcessor->getNumObjectsFound(cCurrentModuleObject);
        CyclingTimeSteps = 0;

        if (NumNCycSysAvailMgrs > 0) {

            NCycSysAvailMgrData.allocate(NumNCycSysAvailMgrs);

            for (SysAvailNum = 1; SysAvailNum <= NumNCycSysAvailMgrs; ++SysAvailNum) {

                inputProcessor->getObjectItem(state, cCurrentModuleObject,
                                              SysAvailNum,
                                              cAlphaArgs,
                                              NumAlphas,
                                              rNumericArgs,
                                              NumNumbers,
                                              IOStatus,
                                              lNumericFieldBlanks,
                                              lAlphaFieldBlanks,
                                              cAlphaFieldNames,
                                              cNumericFieldNames);
                UtilityRoutines::IsNameEmpty(cAlphaArgs(1), cCurrentModuleObject, ErrorsFound);
                NCycSysAvailMgrData(SysAvailNum).Name = cAlphaArgs(1);
                NCycSysAvailMgrData(SysAvailNum).MgrType = SysAvailMgr_NightCycle;
                NCycSysAvailMgrData(SysAvailNum).TempTolRange = rNumericArgs(1);
                CyclingTimeSteps = nint((rNumericArgs(2) / SecInHour) * double(NumOfTimeStepInHour));
                CyclingTimeSteps = max(1, CyclingTimeSteps);
                NCycSysAvailMgrData(SysAvailNum).CyclingTimeSteps = CyclingTimeSteps;
                NCycSysAvailMgrData(SysAvailNum).SchedPtr = GetScheduleIndex(state, cAlphaArgs(2));
                if (NCycSysAvailMgrData(SysAvailNum).SchedPtr == 0) {
                    ShowSevereError(RoutineName + cCurrentModuleObject + " = \"" + cAlphaArgs(1) + "\", invalid");
                    ShowContinueError("not found: " + cAlphaFieldNames(2) + "=\"" + cAlphaArgs(2) + "\".");
                    ErrorsFound = true;
                }
                NCycSysAvailMgrData(SysAvailNum).FanSched = cAlphaArgs(3);
                NCycSysAvailMgrData(SysAvailNum).FanSchedPtr = GetScheduleIndex(state, cAlphaArgs(3));
                if (NCycSysAvailMgrData(SysAvailNum).FanSchedPtr == 0) {
                    ShowSevereError(RoutineName + cCurrentModuleObject + " = \"" + cAlphaArgs(1) + "\", invalid");
                    ShowContinueError("not found: " + cAlphaFieldNames(3) + "=\"" + cAlphaArgs(3) + "\".");
                    ErrorsFound = true;
                }

                {
                    auto const SELECT_CASE_var(UtilityRoutines::MakeUPPERCase(cAlphaArgs(4)));
                    if (SELECT_CASE_var == "STAYOFF") {
                        NCycSysAvailMgrData(SysAvailNum).CtrlType = StayOff;
                    } else if (SELECT_CASE_var == "CYCLEONANY") {
                        NCycSysAvailMgrData(SysAvailNum).CtrlType = CycleOnAny;
                    } else if (SELECT_CASE_var == "CYCLEONCONTROLZONE") {
                        NCycSysAvailMgrData(SysAvailNum).CtrlType = CycleOnControlZone;
                    } else if (SELECT_CASE_var == "CYCLEONANYZONEFANSONLY") {
                        NCycSysAvailMgrData(SysAvailNum).CtrlType = ZoneFansOnly;
                    } else if (SELECT_CASE_var == "CYCLEONANYCOOLINGORHEATINGZONE") {
                        NCycSysAvailMgrData(SysAvailNum).CtrlType = CycleOnAnyCoolingOrHeatingZone;
                    } else if (SELECT_CASE_var == "CYCLEONANYCOOLINGZONE") {
                        NCycSysAvailMgrData(SysAvailNum).CtrlType = CycleOnAnyCoolingZone;
                    } else if (SELECT_CASE_var == "CYCLEONANYHEATINGZONE") {
                        NCycSysAvailMgrData(SysAvailNum).CtrlType = CycleOnAnyHeatingZone;
                    } else if (SELECT_CASE_var == "CYCLEONANYHEATINGZONEFANSONLY") {
                        NCycSysAvailMgrData(SysAvailNum).CtrlType = CycleOnAnyHeatingZoneFansOnly;
                    } else {
                        ShowSevereError(RoutineName + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\", invalid");
                        ShowSevereError(RoutineName + "incorrect value: " + cAlphaFieldNames(4) + "=\"" + cAlphaArgs(4) + "\".");
                        ErrorsFound = true;
                    }
                }

                // Cycling Run Time Control Type
                if (!lAlphaFieldBlanks(5)) {
                    {
                        auto const SELECT_CASE_var(UtilityRoutines::MakeUPPERCase(cAlphaArgs(5)));
                        if (SELECT_CASE_var == "FIXEDRUNTIME") {
                            NCycSysAvailMgrData(SysAvailNum).CycRunTimeCntrlType = FixedRunTime;
                        } else if (SELECT_CASE_var == "THERMOSTAT") {
                            NCycSysAvailMgrData(SysAvailNum).CycRunTimeCntrlType = Thermostat;
                        } else if (SELECT_CASE_var == "THERMOSTATWITHMINIMUMRUNTIME") {
                            NCycSysAvailMgrData(SysAvailNum).CycRunTimeCntrlType = ThermostatWithMinimumRunTime;
                        } else {
                            ShowSevereError(RoutineName + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\", invalid");
                            ShowSevereError(RoutineName + "incorrect value: " + cAlphaFieldNames(5) + "=\"" + cAlphaArgs(5) + "\".");
                            ErrorsFound = true;
                        }
                    }
                }

                // Control zone or zonelist
                if (!lAlphaFieldBlanks(6)) {
                    NCycSysAvailMgrData(SysAvailNum).CtrlZoneListName = cAlphaArgs(6);
                    int ZoneNum = UtilityRoutines::FindItemInList(cAlphaArgs(6), Zone);
                    if (ZoneNum > 0) {
                        NCycSysAvailMgrData(SysAvailNum).NumOfCtrlZones = 1;
                        NCycSysAvailMgrData(SysAvailNum).CtrlZonePtrs.allocate(1);
                        NCycSysAvailMgrData(SysAvailNum).CtrlZonePtrs(1) = ZoneNum;
                    } else {
                        int ZoneListNum = 0;
                        if (NumOfZoneLists > 0) ZoneListNum = UtilityRoutines::FindItemInList(cAlphaArgs(6), ZoneList);
                        if (ZoneListNum > 0) {
                            int NumZones = ZoneList(ZoneListNum).NumOfZones;
                            NCycSysAvailMgrData(SysAvailNum).NumOfCtrlZones = NumZones;
                            NCycSysAvailMgrData(SysAvailNum).CtrlZonePtrs.allocate(NumZones);
                            for (int ZoneNumInList = 1; ZoneNumInList <= NumZones; ++ZoneNumInList) {
                                NCycSysAvailMgrData(SysAvailNum).CtrlZonePtrs(ZoneNumInList) = ZoneList(ZoneListNum).Zone(ZoneNumInList);
                            }
                        } else {
                            ShowSevereError(RoutineName + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\" invalid " + cAlphaFieldNames(6) + "=\"" +
                                            cAlphaArgs(6) + "\" not found.");
                            ErrorsFound = true;
                        }
                    }
                }

                // Cooling zone or zonelist
                if (!lAlphaFieldBlanks(7)) {
                    NCycSysAvailMgrData(SysAvailNum).CoolingZoneListName = cAlphaArgs(7);
                    int ZoneNum = UtilityRoutines::FindItemInList(cAlphaArgs(7), Zone);
                    if (ZoneNum > 0) {
                        NCycSysAvailMgrData(SysAvailNum).NumOfCoolingZones = 1;
                        NCycSysAvailMgrData(SysAvailNum).CoolingZonePtrs.allocate(1);
                        NCycSysAvailMgrData(SysAvailNum).CoolingZonePtrs(1) = ZoneNum;
                    } else {
                        int ZoneListNum = 0;
                        if (NumOfZoneLists > 0) ZoneListNum = UtilityRoutines::FindItemInList(cAlphaArgs(7), ZoneList);
                        if (ZoneListNum > 0) {
                            int NumZones = ZoneList(ZoneListNum).NumOfZones;
                            NCycSysAvailMgrData(SysAvailNum).NumOfCoolingZones = NumZones;
                            NCycSysAvailMgrData(SysAvailNum).CoolingZonePtrs.allocate(NumZones);
                            for (int ZoneNumInList = 1; ZoneNumInList <= NumZones; ++ZoneNumInList) {
                                NCycSysAvailMgrData(SysAvailNum).CoolingZonePtrs(ZoneNumInList) = ZoneList(ZoneListNum).Zone(ZoneNumInList);
                            }
                        } else {
                            ShowSevereError(RoutineName + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\" invalid " + cAlphaFieldNames(7) + "=\"" +
                                            cAlphaArgs(7) + "\" not found.");
                            ErrorsFound = true;
                        }
                    }
                }

                // Heating zone or zonelist
                if (!lAlphaFieldBlanks(8)) {
                    NCycSysAvailMgrData(SysAvailNum).HeatingZoneListName = cAlphaArgs(8);
                    int ZoneNum = UtilityRoutines::FindItemInList(cAlphaArgs(8), Zone);
                    if (ZoneNum > 0) {
                        NCycSysAvailMgrData(SysAvailNum).NumOfHeatingZones = 1;
                        NCycSysAvailMgrData(SysAvailNum).HeatingZonePtrs.allocate(1);
                        NCycSysAvailMgrData(SysAvailNum).HeatingZonePtrs(1) = ZoneNum;
                    } else {
                        int ZoneListNum = 0;
                        if (NumOfZoneLists > 0) ZoneListNum = UtilityRoutines::FindItemInList(cAlphaArgs(8), ZoneList);
                        if (ZoneListNum > 0) {
                            int NumZones = ZoneList(ZoneListNum).NumOfZones;
                            NCycSysAvailMgrData(SysAvailNum).NumOfHeatingZones = NumZones;
                            NCycSysAvailMgrData(SysAvailNum).HeatingZonePtrs.allocate(NumZones);
                            for (int ZoneNumInList = 1; ZoneNumInList <= NumZones; ++ZoneNumInList) {
                                NCycSysAvailMgrData(SysAvailNum).HeatingZonePtrs(ZoneNumInList) = ZoneList(ZoneListNum).Zone(ZoneNumInList);
                            }
                        } else {
                            ShowSevereError(RoutineName + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\" invalid " + cAlphaFieldNames(8) + "=\"" +
                                            cAlphaArgs(8) + "\" not found.");
                            ErrorsFound = true;
                        }
                    }
                }

                // HeatZnFan zone or zonelist
                if (!lAlphaFieldBlanks(9)) {
                    NCycSysAvailMgrData(SysAvailNum).HeatZnFanZoneListName = cAlphaArgs(9);
                    int ZoneNum = UtilityRoutines::FindItemInList(cAlphaArgs(9), Zone);
                    if (ZoneNum > 0) {
                        NCycSysAvailMgrData(SysAvailNum).NumOfHeatZnFanZones = 1;
                        NCycSysAvailMgrData(SysAvailNum).HeatZnFanZonePtrs.allocate(1);
                        NCycSysAvailMgrData(SysAvailNum).HeatZnFanZonePtrs(1) = ZoneNum;
                    } else {
                        int ZoneListNum = 0;
                        if (NumOfZoneLists > 0) ZoneListNum = UtilityRoutines::FindItemInList(cAlphaArgs(9), ZoneList);
                        if (ZoneListNum > 0) {
                            int NumZones = ZoneList(ZoneListNum).NumOfZones;
                            NCycSysAvailMgrData(SysAvailNum).NumOfHeatZnFanZones = NumZones;
                            NCycSysAvailMgrData(SysAvailNum).HeatZnFanZonePtrs.allocate(NumZones);
                            for (int ZoneNumInList = 1; ZoneNumInList <= NumZones; ++ZoneNumInList) {
                                NCycSysAvailMgrData(SysAvailNum).HeatZnFanZonePtrs(ZoneNumInList) = ZoneList(ZoneListNum).Zone(ZoneNumInList);
                            }
                        } else {
                            ShowSevereError(RoutineName + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\" invalid " + cAlphaFieldNames(9) + "=\"" +
                                            cAlphaArgs(9) + "\" not found.");
                            ErrorsFound = true;
                        }
                    }
                }

                SetupOutputVariable(state, "Availability Manager Night Cycle Control Status",
                                    OutputProcessor::Unit::None,
                                    NCycSysAvailMgrData(SysAvailNum).AvailStatus,
                                    "System",
                                    "Average",
                                    NCycSysAvailMgrData(SysAvailNum).Name);

            } // SysAvailNum
        }

        cCurrentModuleObject = "AvailabilityManager:OptimumStart";
        NumOptStartSysAvailMgrs = inputProcessor->getNumObjectsFound(cCurrentModuleObject);
        CyclingTimeSteps = 0;

        if (NumOptStartSysAvailMgrs > 0) {
            // Array size of variable type OptStartSysAvailMgrData is updated
            OptStartSysAvailMgrData.allocate(NumOptStartSysAvailMgrs);

            for (SysAvailNum = 1; SysAvailNum <= NumOptStartSysAvailMgrs; ++SysAvailNum) {

                inputProcessor->getObjectItem(state, cCurrentModuleObject,
                                              SysAvailNum,
                                              cAlphaArgs,
                                              NumAlphas,
                                              rNumericArgs,
                                              NumNumbers,
                                              IOStatus,
                                              lNumericFieldBlanks,
                                              lAlphaFieldBlanks,
                                              cAlphaFieldNames,
                                              cNumericFieldNames);
                UtilityRoutines::IsNameEmpty(cAlphaArgs(1), cCurrentModuleObject, ErrorsFound);
                OptStartSysAvailMgrData(SysAvailNum).Name = cAlphaArgs(1);
                OptStartSysAvailMgrData(SysAvailNum).MgrType = SysAvailMgr_OptimumStart;
                OptStartSysAvailMgrData(SysAvailNum).SchedPtr = GetScheduleIndex(state, cAlphaArgs(2));
                if (OptStartSysAvailMgrData(SysAvailNum).SchedPtr == 0) {
                    ShowSevereError(RoutineName + cCurrentModuleObject + " = \"" + cAlphaArgs(1) + "\", invalid");
                    ShowContinueError("not found: " + cAlphaFieldNames(2) + "=\"" + cAlphaArgs(2) + "\".");
                    ErrorsFound = true;
                }
                OptStartSysAvailMgrData(SysAvailNum).FanSched = cAlphaArgs(3);
                OptStartSysAvailMgrData(SysAvailNum).FanSchedPtr = GetScheduleIndex(state, cAlphaArgs(3));
                if (OptStartSysAvailMgrData(SysAvailNum).FanSchedPtr == 0) {
                    ShowSevereError(RoutineName + cCurrentModuleObject + " = \"" + cAlphaArgs(1) + "\", invalid");
                    ShowContinueError("not found: " + cAlphaFieldNames(3) + "=\"" + cAlphaArgs(3) + "\".");
                    ErrorsFound = true;
                }

                OptStartSysAvailMgrData(SysAvailNum).MaxOptStartTime = rNumericArgs(1);

                {
                    auto const SELECT_CASE_var(UtilityRoutines::MakeUPPERCase(cAlphaArgs(4)));
                    if (SELECT_CASE_var == "STAYOFF") {
                        OptStartSysAvailMgrData(SysAvailNum).CtrlType = StayOff;
                    } else if (SELECT_CASE_var == "CONTROLZONE") {
                        OptStartSysAvailMgrData(SysAvailNum).CtrlType = ControlZone;
                    } else if (SELECT_CASE_var == "MAXIMUMOFZONELIST") {
                        OptStartSysAvailMgrData(SysAvailNum).CtrlType = MaximumOfZoneList;
                    } else {
                        OptStartSysAvailMgrData(SysAvailNum).CtrlType = ControlZone;
                        ShowSevereError(RoutineName + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\", invalid");
                        ShowSevereError(RoutineName + "incorrect value: " + cAlphaFieldNames(4) + "=\"" + cAlphaArgs(4) + "\".");
                        ErrorsFound = true;
                    }
                }

                if (OptStartSysAvailMgrData(SysAvailNum).CtrlType == ControlZone) {
                    OptStartSysAvailMgrData(SysAvailNum).CtrlZoneName = cAlphaArgs(5);
                    OptStartSysAvailMgrData(SysAvailNum).ZoneNum = UtilityRoutines::FindItemInList(cAlphaArgs(5), Zone);
                    if (OptStartSysAvailMgrData(SysAvailNum).ZoneNum == 0) {
                        ShowSevereError(RoutineName + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\", invalid");
                        ShowSevereError("not found: " + cAlphaFieldNames(5) + "=\"" + cAlphaArgs(5) + "\".");
                        ErrorsFound = true;
                    }
                }

                if (OptStartSysAvailMgrData(SysAvailNum).CtrlType == MaximumOfZoneList) {
                    OptStartSysAvailMgrData(SysAvailNum).ZoneListName = cAlphaArgs(6);
                    for (ZoneListNum = 1; ZoneListNum <= NumOfZoneLists; ++ZoneListNum) {
                        if (ZoneList(ZoneListNum).Name == cAlphaArgs(6)) {
                            OptStartSysAvailMgrData(SysAvailNum).NumOfZones = ZoneList(ZoneListNum).NumOfZones;
                            OptStartSysAvailMgrData(SysAvailNum).ZonePtrs.allocate(ZoneList(ZoneListNum).NumOfZones);
                            for (ZoneNumInList = 1; ZoneNumInList <= ZoneList(ZoneListNum).NumOfZones; ++ZoneNumInList) {
                                OptStartSysAvailMgrData(SysAvailNum).ZonePtrs(ZoneNumInList) = ZoneList(ZoneListNum).Zone(ZoneNumInList);
                            }
                        }
                    }
                    OptStartSysAvailMgrData(SysAvailNum).NumOfZones = UtilityRoutines::FindItemInList(cAlphaArgs(6), ZoneList);
                    if (OptStartSysAvailMgrData(SysAvailNum).NumOfZones == 0) {
                        ShowSevereError(RoutineName + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\", invalid");
                        ShowSevereError("not found: " + cAlphaFieldNames(6) + "=\"" + cAlphaArgs(6) + "\".");
                        ErrorsFound = true;
                    }
                }

                {
                    auto const SELECT_CASE_var(UtilityRoutines::MakeUPPERCase(cAlphaArgs(7)));
                    if (SELECT_CASE_var == "CONSTANTTEMPERATUREGRADIENT") {
                        OptStartSysAvailMgrData(SysAvailNum).CtrlAlgType = ConstantTemperatureGradient;
                    } else if (SELECT_CASE_var == "ADAPTIVETEMPERATUREGRADIENT") {
                        OptStartSysAvailMgrData(SysAvailNum).CtrlAlgType = AdaptiveTemperatureGradient;
                    } else if (SELECT_CASE_var == "ADAPTIVEASHRAE") {
                        OptStartSysAvailMgrData(SysAvailNum).CtrlAlgType = AdaptiveASHRAE;
                    } else if (SELECT_CASE_var == "CONSTANTSTARTTIME") {
                        OptStartSysAvailMgrData(SysAvailNum).CtrlAlgType = ConstantStartTime;
                    } else {
                        OptStartSysAvailMgrData(SysAvailNum).CtrlAlgType = AdaptiveASHRAE;
                        ShowSevereError(RoutineName + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\", invalid");
                        ShowSevereError(RoutineName + "incorrect value: " + cAlphaFieldNames(7) + "=\"" + cAlphaArgs(7) + "\".");
                        ErrorsFound = true;
                    }
                }

                if (OptStartSysAvailMgrData(SysAvailNum).CtrlAlgType == ConstantTemperatureGradient) {
                    OptStartSysAvailMgrData(SysAvailNum).ConstTGradCool = rNumericArgs(2);
                }

                if (OptStartSysAvailMgrData(SysAvailNum).CtrlAlgType == ConstantTemperatureGradient) {
                    OptStartSysAvailMgrData(SysAvailNum).ConstTGradHeat = rNumericArgs(3);
                }

                if (OptStartSysAvailMgrData(SysAvailNum).CtrlAlgType == AdaptiveTemperatureGradient) {
                    OptStartSysAvailMgrData(SysAvailNum).InitTGradCool = rNumericArgs(4);
                }

                if (OptStartSysAvailMgrData(SysAvailNum).CtrlAlgType == AdaptiveTemperatureGradient) {
                    OptStartSysAvailMgrData(SysAvailNum).InitTGradHeat = rNumericArgs(5);
                }

                if (OptStartSysAvailMgrData(SysAvailNum).CtrlAlgType == ConstantStartTime) {
                    OptStartSysAvailMgrData(SysAvailNum).ConstStartTime = rNumericArgs(6);
                }

                if (OptStartSysAvailMgrData(SysAvailNum).CtrlAlgType == AdaptiveTemperatureGradient) {
                    OptStartSysAvailMgrData(SysAvailNum).NumPreDays = rNumericArgs(7);
                }

                SetupOutputVariable(state, "Availability Manager Optimum Start Control Status",
                                    OutputProcessor::Unit::None,
                                    OptStartSysAvailMgrData(SysAvailNum).AvailStatus,
                                    "System",
                                    "Average",
                                    OptStartSysAvailMgrData(SysAvailNum).Name);

                // add
                SetupOutputVariable(state, "Availability Manager Optimum Start Time Before Occupancy",
                                    OutputProcessor::Unit::hr,
                                    OptStartSysAvailMgrData(SysAvailNum).NumHoursBeforeOccupancy,
                                    "System",
                                    "Average",
                                    OptStartSysAvailMgrData(SysAvailNum).Name,
                                    "Daily");
            }
        }

        cCurrentModuleObject = "AvailabilityManager:DifferentialThermostat";
        NumDiffTSysAvailMgrs = inputProcessor->getNumObjectsFound(cCurrentModuleObject);

        if (NumDiffTSysAvailMgrs > 0) {

            DiffTSysAvailMgrData.allocate(NumDiffTSysAvailMgrs);

            for (SysAvailNum = 1; SysAvailNum <= NumDiffTSysAvailMgrs; ++SysAvailNum) {

                inputProcessor->getObjectItem(state, cCurrentModuleObject,
                                              SysAvailNum,
                                              cAlphaArgs,
                                              NumAlphas,
                                              rNumericArgs,
                                              NumNumbers,
                                              IOStatus,
                                              lNumericFieldBlanks,
                                              lAlphaFieldBlanks,
                                              cAlphaFieldNames,
                                              cNumericFieldNames);
                UtilityRoutines::IsNameEmpty(cAlphaArgs(1), cCurrentModuleObject, ErrorsFound);
                DiffTSysAvailMgrData(SysAvailNum).Name = cAlphaArgs(1);
                DiffTSysAvailMgrData(SysAvailNum).MgrType = SysAvailMgr_DiffThermo;

                DiffTSysAvailMgrData(SysAvailNum).HotNode = GetOnlySingleNode(state, cAlphaArgs(2),
                                                                              ErrorsFound,
                                                                              cCurrentModuleObject,
                                                                              cAlphaArgs(1),
                                                                              NodeType_Unknown,
                                                                              NodeConnectionType_Sensor,
                                                                              1,
                                                                              ObjectIsNotParent);
                MarkNode(DiffTSysAvailMgrData(SysAvailNum).HotNode, cCurrentModuleObject, cAlphaArgs(1), "Hot Node");
                DiffTSysAvailMgrData(SysAvailNum).ColdNode = GetOnlySingleNode(state, cAlphaArgs(3),
                                                                               ErrorsFound,
                                                                               cCurrentModuleObject,
                                                                               cAlphaArgs(1),
                                                                               NodeType_Unknown,
                                                                               NodeConnectionType_Sensor,
                                                                               1,
                                                                               ObjectIsNotParent);
                MarkNode(DiffTSysAvailMgrData(SysAvailNum).ColdNode, cCurrentModuleObject, cAlphaArgs(1), "Cold Node");

                DiffTSysAvailMgrData(SysAvailNum).TempDiffOn = rNumericArgs(1);

                if (NumNumbers > 1) {
                    DiffTSysAvailMgrData(SysAvailNum).TempDiffOff = rNumericArgs(2);
                } else {
                    DiffTSysAvailMgrData(SysAvailNum).TempDiffOff = DiffTSysAvailMgrData(SysAvailNum).TempDiffOn;
                }

                if (DiffTSysAvailMgrData(SysAvailNum).TempDiffOff > DiffTSysAvailMgrData(SysAvailNum).TempDiffOn) {
                    ShowSevereError(RoutineName + cCurrentModuleObject + " = \"" + cAlphaArgs(1) + "\", invalid");
                    ShowContinueError("The " + cNumericFieldNames(2) + " is greater than the " + cNumericFieldNames(1) + '.');
                    ErrorsFound = true;
                }

                SetupOutputVariable(state, "Availability Manager Differential Thermostat Control Status",
                                    OutputProcessor::Unit::None,
                                    DiffTSysAvailMgrData(SysAvailNum).AvailStatus,
                                    "System",
                                    "Average",
                                    DiffTSysAvailMgrData(SysAvailNum).Name);

            } // SysAvailNum
        }

        cCurrentModuleObject = "AvailabilityManager:HighTemperatureTurnOff";
        NumHiTurnOffSysAvailMgrs = inputProcessor->getNumObjectsFound(cCurrentModuleObject);

        if (NumHiTurnOffSysAvailMgrs > 0) {
            HiTurnOffSysAvailMgrData.allocate(NumHiTurnOffSysAvailMgrs);

            for (SysAvailNum = 1; SysAvailNum <= NumHiTurnOffSysAvailMgrs; ++SysAvailNum) {

                inputProcessor->getObjectItem(state,
                                              cCurrentModuleObject,
                                              SysAvailNum,
                                              cAlphaArgs,
                                              NumAlphas,
                                              rNumericArgs,
                                              NumNumbers,
                                              IOStatus,
                                              lNumericFieldBlanks,
                                              lAlphaFieldBlanks,
                                              cAlphaFieldNames,
                                              cNumericFieldNames);
                UtilityRoutines::IsNameEmpty(cAlphaArgs(1), cCurrentModuleObject, ErrorsFound);
                HiTurnOffSysAvailMgrData(SysAvailNum).Name = cAlphaArgs(1);
                HiTurnOffSysAvailMgrData(SysAvailNum).MgrType = SysAvailMgr_HiTempTOff;

                HiTurnOffSysAvailMgrData(SysAvailNum).Node = GetOnlySingleNode(state, cAlphaArgs(2),
                                                                               ErrorsFound,
                                                                               cCurrentModuleObject,
                                                                               cAlphaArgs(1),
                                                                               NodeType_Unknown,
                                                                               NodeConnectionType_Sensor,
                                                                               1,
                                                                               ObjectIsNotParent);
                MarkNode(HiTurnOffSysAvailMgrData(SysAvailNum).Node, cCurrentModuleObject, cAlphaArgs(1), "Sensor Node");

                HiTurnOffSysAvailMgrData(SysAvailNum).Temp = rNumericArgs(1);

                SetupOutputVariable(state, "Availability Manager High Temperature Turn Off Control Status",
                                    OutputProcessor::Unit::None,
                                    HiTurnOffSysAvailMgrData(SysAvailNum).AvailStatus,
                                    "System",
                                    "Average",
                                    HiTurnOffSysAvailMgrData(SysAvailNum).Name);

            } // SysAvailNum
        }

        cCurrentModuleObject = "AvailabilityManager:HighTemperatureTurnOn";
        NumHiTurnOnSysAvailMgrs = inputProcessor->getNumObjectsFound(cCurrentModuleObject);

        if (NumHiTurnOnSysAvailMgrs > 0) {

            HiTurnOnSysAvailMgrData.allocate(NumHiTurnOnSysAvailMgrs);

            for (SysAvailNum = 1; SysAvailNum <= NumHiTurnOnSysAvailMgrs; ++SysAvailNum) {

                inputProcessor->getObjectItem(state, cCurrentModuleObject,
                                              SysAvailNum,
                                              cAlphaArgs,
                                              NumAlphas,
                                              rNumericArgs,
                                              NumNumbers,
                                              IOStatus,
                                              lNumericFieldBlanks,
                                              lAlphaFieldBlanks,
                                              cAlphaFieldNames,
                                              cNumericFieldNames);
                UtilityRoutines::IsNameEmpty(cAlphaArgs(1), cCurrentModuleObject, ErrorsFound);
                HiTurnOnSysAvailMgrData(SysAvailNum).Name = cAlphaArgs(1);
                HiTurnOnSysAvailMgrData(SysAvailNum).MgrType = SysAvailMgr_HiTempTOn;

                HiTurnOnSysAvailMgrData(SysAvailNum).Node = GetOnlySingleNode(state, cAlphaArgs(2),
                                                                              ErrorsFound,
                                                                              cCurrentModuleObject,
                                                                              cAlphaArgs(1),
                                                                              NodeType_Unknown,
                                                                              NodeConnectionType_Sensor,
                                                                              1,
                                                                              ObjectIsNotParent);
                MarkNode(HiTurnOnSysAvailMgrData(SysAvailNum).Node, cCurrentModuleObject, cAlphaArgs(1), "Sensor Node");

                HiTurnOnSysAvailMgrData(SysAvailNum).Temp = rNumericArgs(1);

                SetupOutputVariable(state, "Availability Manager High Temperature Turn On Control Status",
                                    OutputProcessor::Unit::None,
                                    HiTurnOnSysAvailMgrData(SysAvailNum).AvailStatus,
                                    "System",
                                    "Average",
                                    HiTurnOnSysAvailMgrData(SysAvailNum).Name);

            } // SysAvailNum
        }

        cCurrentModuleObject = "AvailabilityManager:LowTemperatureTurnOff";
        NumLoTurnOffSysAvailMgrs = inputProcessor->getNumObjectsFound(cCurrentModuleObject);

        if (NumLoTurnOffSysAvailMgrs > 0) {

            LoTurnOffSysAvailMgrData.allocate(NumLoTurnOffSysAvailMgrs);

            for (SysAvailNum = 1; SysAvailNum <= NumLoTurnOffSysAvailMgrs; ++SysAvailNum) {

                inputProcessor->getObjectItem(state, cCurrentModuleObject,
                                              SysAvailNum,
                                              cAlphaArgs,
                                              NumAlphas,
                                              rNumericArgs,
                                              NumNumbers,
                                              IOStatus,
                                              lNumericFieldBlanks,
                                              lAlphaFieldBlanks,
                                              cAlphaFieldNames,
                                              cNumericFieldNames);
                UtilityRoutines::IsNameEmpty(cAlphaArgs(1), cCurrentModuleObject, ErrorsFound);
                LoTurnOffSysAvailMgrData(SysAvailNum).Name = cAlphaArgs(1);
                LoTurnOffSysAvailMgrData(SysAvailNum).MgrType = SysAvailMgr_LoTempTOff;

                LoTurnOffSysAvailMgrData(SysAvailNum).Node = GetOnlySingleNode(state, cAlphaArgs(2),
                                                                               ErrorsFound,
                                                                               cCurrentModuleObject,
                                                                               cAlphaArgs(1),
                                                                               NodeType_Unknown,
                                                                               NodeConnectionType_Sensor,
                                                                               1,
                                                                               ObjectIsNotParent);
                MarkNode(LoTurnOffSysAvailMgrData(SysAvailNum).Node, cCurrentModuleObject, cAlphaArgs(1), "Sensor Node");

                LoTurnOffSysAvailMgrData(SysAvailNum).Temp = rNumericArgs(1);

                if (!lAlphaFieldBlanks(3)) {
                    LoTurnOffSysAvailMgrData(SysAvailNum).SchedPtr = GetScheduleIndex(state, cAlphaArgs(3));
                    if (LoTurnOffSysAvailMgrData(SysAvailNum).SchedPtr == 0) {
                        ShowSevereError(RoutineName + cAlphaFieldNames(3) + "=\"" + cAlphaArgs(3) + "\" not found.");
                        ShowContinueError("Occurs in " + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\".");
                        ErrorsFound = true;
                    }
                } else {
                    LoTurnOffSysAvailMgrData(SysAvailNum).SchedPtr = 0;
                }

                SetupOutputVariable(state, "Availability Manager Low Temperature Turn Off Control Status",
                                    OutputProcessor::Unit::None,
                                    LoTurnOffSysAvailMgrData(SysAvailNum).AvailStatus,
                                    "System",
                                    "Average",
                                    LoTurnOffSysAvailMgrData(SysAvailNum).Name);

            } // SysAvailNum
        }

        cCurrentModuleObject = "AvailabilityManager:LowTemperatureTurnOn";
        NumLoTurnOnSysAvailMgrs = inputProcessor->getNumObjectsFound(cCurrentModuleObject);

        if (NumLoTurnOnSysAvailMgrs > 0) {

            LoTurnOnSysAvailMgrData.allocate(NumLoTurnOnSysAvailMgrs);

            for (SysAvailNum = 1; SysAvailNum <= NumLoTurnOnSysAvailMgrs; ++SysAvailNum) {

                inputProcessor->getObjectItem(state, cCurrentModuleObject,
                                              SysAvailNum,
                                              cAlphaArgs,
                                              NumAlphas,
                                              rNumericArgs,
                                              NumNumbers,
                                              IOStatus,
                                              lNumericFieldBlanks,
                                              lAlphaFieldBlanks,
                                              cAlphaFieldNames,
                                              cNumericFieldNames);
                UtilityRoutines::IsNameEmpty(cAlphaArgs(1), cCurrentModuleObject, ErrorsFound);
                LoTurnOnSysAvailMgrData(SysAvailNum).Name = cAlphaArgs(1);
                LoTurnOnSysAvailMgrData(SysAvailNum).MgrType = SysAvailMgr_LoTempTOn;

                LoTurnOnSysAvailMgrData(SysAvailNum).Node = GetOnlySingleNode(state, cAlphaArgs(2),
                                                                              ErrorsFound,
                                                                              cCurrentModuleObject,
                                                                              cAlphaArgs(1),
                                                                              NodeType_Unknown,
                                                                              NodeConnectionType_Sensor,
                                                                              1,
                                                                              ObjectIsNotParent);
                MarkNode(LoTurnOnSysAvailMgrData(SysAvailNum).Node, cCurrentModuleObject, cAlphaArgs(1), "Sensor Node");

                LoTurnOnSysAvailMgrData(SysAvailNum).Temp = rNumericArgs(1);

                SetupOutputVariable(state, "Availability Manager Low Temperature Turn On Control Status",
                                    OutputProcessor::Unit::None,
                                    LoTurnOnSysAvailMgrData(SysAvailNum).AvailStatus,
                                    "System",
                                    "Average",
                                    LoTurnOnSysAvailMgrData(SysAvailNum).Name);

            } // SysAvailNum
        }

        cCurrentModuleObject = "AvailabilityManager:NightVentilation";
        NumNVentSysAvailMgrs = inputProcessor->getNumObjectsFound(cCurrentModuleObject);

        if (NumNVentSysAvailMgrs > 0) {

            NVentSysAvailMgrData.allocate(NumNVentSysAvailMgrs);

            for (SysAvailNum = 1; SysAvailNum <= NumNVentSysAvailMgrs; ++SysAvailNum) {

                inputProcessor->getObjectItem(state, cCurrentModuleObject,
                                              SysAvailNum,
                                              cAlphaArgs,
                                              NumAlphas,
                                              rNumericArgs,
                                              NumNumbers,
                                              IOStatus,
                                              lNumericFieldBlanks,
                                              lAlphaFieldBlanks,
                                              cAlphaFieldNames,
                                              cNumericFieldNames);
                UtilityRoutines::IsNameEmpty(cAlphaArgs(1), cCurrentModuleObject, ErrorsFound);
                NVentSysAvailMgrData(SysAvailNum).Name = cAlphaArgs(1);
                NVentSysAvailMgrData(SysAvailNum).MgrType = SysAvailMgr_NightVent;

                NVentSysAvailMgrData(SysAvailNum).SchedPtr = GetScheduleIndex(state, cAlphaArgs(2));
                if (NVentSysAvailMgrData(SysAvailNum).SchedPtr == 0) {
                    ShowSevereError(RoutineName + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\", invalid");
                    ShowContinueError("not found: " + cAlphaFieldNames(2) + "=\"" + cAlphaArgs(2) + "\".");
                    ErrorsFound = true;
                }
                NVentSysAvailMgrData(SysAvailNum).FanSched = cAlphaArgs(3);
                NVentSysAvailMgrData(SysAvailNum).FanSchedPtr = GetScheduleIndex(state, cAlphaArgs(3));
                if (NVentSysAvailMgrData(SysAvailNum).FanSchedPtr == 0) {
                    ShowSevereError(RoutineName + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\", invalid");
                    ShowContinueError("not found: " + cAlphaFieldNames(3) + "=\"" + cAlphaArgs(3) + "\".");
                    ErrorsFound = true;
                }
                NVentSysAvailMgrData(SysAvailNum).VentTempSched = cAlphaArgs(4);
                NVentSysAvailMgrData(SysAvailNum).VentTempSchedPtr = GetScheduleIndex(state, cAlphaArgs(4));
                if (NVentSysAvailMgrData(SysAvailNum).VentTempSchedPtr == 0) {
                    ShowSevereError(RoutineName + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\", invalid");
                    ShowContinueError("not found: " + cAlphaFieldNames(4) + "=\"" + cAlphaArgs(4) + "\".");
                    ErrorsFound = true;
                }
                NVentSysAvailMgrData(SysAvailNum).VentDelT = rNumericArgs(1);
                NVentSysAvailMgrData(SysAvailNum).VentTempLowLim = rNumericArgs(2);
                NVentSysAvailMgrData(SysAvailNum).VentFlowFrac = rNumericArgs(3);
                NVentSysAvailMgrData(SysAvailNum).CtrlZoneName = cAlphaArgs(5);
                NVentSysAvailMgrData(SysAvailNum).ZoneNum = UtilityRoutines::FindItemInList(cAlphaArgs(5), Zone);
                if (NVentSysAvailMgrData(SysAvailNum).ZoneNum == 0) {
                    ShowSevereError(RoutineName + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\", invalid");
                    ShowContinueError("not found: " + cAlphaFieldNames(5) + "=\"" + cAlphaArgs(5) + "\".");
                    ErrorsFound = true;
                }

                SetupOutputVariable(state, "Availability Manager Night Ventilation Control Status",
                                    OutputProcessor::Unit::None,
                                    NVentSysAvailMgrData(SysAvailNum).AvailStatus,
                                    "System",
                                    "Average",
                                    NVentSysAvailMgrData(SysAvailNum).Name);

            } // SysAvailNum
        }

        cAlphaFieldNames.deallocate();
        cAlphaArgs.deallocate();
        lAlphaFieldBlanks.deallocate();
        cNumericFieldNames.deallocate();
        rNumericArgs.deallocate();
        lNumericFieldBlanks.deallocate();

        if (ErrorsFound) {
            ShowFatalError(RoutineName + "Errors found in input.  Preceding condition(s) cause termination.");
        }
    }

    void GetSysAvailManagerListInputs(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda Lawrie
        //       DATE WRITTEN   August 2007
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This routine gets the System Availability Manager List object input and stores
        // it for later retrieval of items from the Plant and Air Loops.

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Array1D_string cAlphaFieldNames;
        Array1D_string cNumericFieldNames;
        Array1D_bool lNumericFieldBlanks;
        Array1D_bool lAlphaFieldBlanks;
        Array1D_string cAlphaArgs;
        Array1D<Real64> rNumericArgs;
        std::string cCurrentModuleObject;
        int NumAlphas;
        int NumNumbers;
        int numArgs;
        int Item;
        int IOStatus;
        bool ErrorsFound;
        int list;
        int itemnum;

        if (GetAvailMgrInputFlag) {
            GetSysAvailManagerInputs(state);
            GetAvailMgrInputFlag = false;
        }

        ErrorsFound = false;

        cCurrentModuleObject = "AvailabilityManagerAssignmentList";
        inputProcessor->getObjectDefMaxArgs(cCurrentModuleObject, numArgs, NumAlphas, NumNumbers);
        cAlphaFieldNames.allocate(NumAlphas);
        cAlphaArgs.allocate(NumAlphas);
        lAlphaFieldBlanks.dimension(NumAlphas, false);
        cNumericFieldNames.allocate(NumNumbers);
        rNumericArgs.dimension(NumNumbers, 0.0);
        lNumericFieldBlanks.dimension(NumNumbers, false);

        cCurrentModuleObject = "AvailabilityManagerAssignmentList";
        NumAvailManagerLists = inputProcessor->getNumObjectsFound(cCurrentModuleObject);

        if (NumAvailManagerLists > 0) {

            SysAvailMgrListData.allocate(NumAvailManagerLists);

            for (Item = 1; Item <= NumAvailManagerLists; ++Item) {
                inputProcessor->getObjectItem(state, cCurrentModuleObject,
                                              Item,
                                              cAlphaArgs,
                                              NumAlphas,
                                              rNumericArgs,
                                              NumNumbers,
                                              IOStatus,
                                              lNumericFieldBlanks,
                                              lAlphaFieldBlanks,
                                              cAlphaFieldNames,
                                              cNumericFieldNames);
                UtilityRoutines::IsNameEmpty(cAlphaArgs(1), cCurrentModuleObject, ErrorsFound);
                SysAvailMgrListData(Item).Name = cAlphaArgs(1);

                SysAvailMgrListData(Item).NumItems = (NumAlphas - 1) / 2; // Subtract off the list name first
                SysAvailMgrListData(Item).AvailManagerName.allocate(SysAvailMgrListData(Item).NumItems);
                SysAvailMgrListData(Item).AvailManagerName = "";
                SysAvailMgrListData(Item).cAvailManagerType.allocate(SysAvailMgrListData(Item).NumItems);
                SysAvailMgrListData(Item).cAvailManagerType = "";
                SysAvailMgrListData(Item).AvailManagerType.allocate(SysAvailMgrListData(Item).NumItems);
                SysAvailMgrListData(Item).AvailManagerType = 0;

                // retrieve data

                itemnum = 1;
                for (list = 1; list <= SysAvailMgrListData(Item).NumItems; ++list) {
                    ++itemnum;
                    SysAvailMgrListData(Item).cAvailManagerType(list) = cAlphaArgs(itemnum);
                    SysAvailMgrListData(Item).AvailManagerType(list) = ValidateAndSetSysAvailabilityManagerType(cAlphaArgs(itemnum));
                    // these are validated individually in the GetPlant, GetSystem and GetZoneEq lists
                    ++itemnum;
                    SysAvailMgrListData(Item).AvailManagerName(list) = cAlphaArgs(itemnum);
                } // End of retrieving items
            }
        }

        cAlphaFieldNames.deallocate();
        cAlphaArgs.deallocate();
        lAlphaFieldBlanks.deallocate();
        cNumericFieldNames.deallocate();
        rNumericArgs.deallocate();
        lNumericFieldBlanks.deallocate();

        if (ErrorsFound) {
            ShowFatalError("GetSysAvailManagerListInputs: Program terminates due to preceding conditions.");
        }
    }

    void GetPlantAvailabilityManager(EnergyPlusData &state,
                                     std::string const &AvailabilityListName, // name that should be an Availability Manager List Name
                                     int const Loop,                          // which loop this is
                                     int const NumPlantLoops,                 // Total number of plant loops
                                     bool &ErrorsFound                        // true if certain errors are detected here
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda Lawrie
        //       DATE WRITTEN   August 2007
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine gets the plant availability manager data for the indicated
        // loop.  If the PlantAvailMgr structure has not been allocated, it will be allocated
        // to "number of plant loops".

        // METHODOLOGY EMPLOYED:
        // <description>

        // REFERENCES:
        // na

        // Using/Aliasing
        using namespace DataPlant;

        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:

        // SUBROUTINE PARAMETER DEFINITIONS:
        // na

        // INTERFACE BLOCK SPECIFICATIONS:
        // na

        // DERIVED TYPE DEFINITIONS:
        // na

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int Found;
        int Num;

        if (GetAvailListsInput) {
            GetSysAvailManagerListInputs(state);
            GetAvailListsInput = false;
        }

        if (!allocated(PlantAvailMgr)) {
            PlantAvailMgr.allocate(NumPlantLoops);
        }

        Found = 0;
        if (NumAvailManagerLists > 0) Found = UtilityRoutines::FindItemInList(AvailabilityListName, SysAvailMgrListData);

        if (Found != 0) {
            PlantAvailMgr(Loop).NumAvailManagers = SysAvailMgrListData(Found).NumItems;
            PlantAvailMgr(Loop).AvailStatus = NoAction;
            PlantAvailMgr(Loop).StartTime = 0;
            PlantAvailMgr(Loop).StopTime = 0;
            PlantAvailMgr(Loop).AvailManagerName.allocate(PlantAvailMgr(Loop).NumAvailManagers);
            PlantAvailMgr(Loop).AvailManagerType.allocate(PlantAvailMgr(Loop).NumAvailManagers);
            PlantAvailMgr(Loop).AvailManagerNum.allocate(PlantAvailMgr(Loop).NumAvailManagers);
            for (Num = 1; Num <= PlantAvailMgr(Loop).NumAvailManagers; ++Num) {
                PlantAvailMgr(Loop).AvailManagerName(Num) = SysAvailMgrListData(Found).AvailManagerName(Num);
                PlantAvailMgr(Loop).AvailManagerNum(Num) = 0;
                PlantAvailMgr(Loop).AvailManagerType(Num) = SysAvailMgrListData(Found).AvailManagerType(Num);
                if (PlantAvailMgr(Loop).AvailManagerType(Num) == 0) {
                    ShowSevereError("GetPlantLoopData/GetPlantAvailabilityManager: Invalid System Availability Manager Type entered=\"" +
                                    SysAvailMgrListData(Found).cAvailManagerType(Num) + "\".");
                    ShowContinueError("Occurs in AvailabilityManagerAssignmentList=\"" + AvailabilityListName + "\".");
                    ErrorsFound = true;
                }
                if (SysAvailMgrListData(Found).AvailManagerType(Num) == SysAvailMgr_DiffThermo && Num != PlantAvailMgr(Loop).NumAvailManagers) {
                    ShowWarningError("GetPlantLoopData/GetPlantAvailabilityManager: AvailabilityManager:DifferentialThermostat=\"" +
                                     SysAvailMgrListData(Found).AvailManagerName(Num) + "\".");
                    ShowContinueError(
                        "...is not the last manager on the AvailabilityManagerAssignmentList.  Any remaining managers will not be used.");
                    ShowContinueError("Occurs in AvailabilityManagerAssignmentList =\"" + AvailabilityListName + "\".");
                }
                if (SysAvailMgrListData(Found).AvailManagerType(Num) == SysAvailMgr_NightVent ||
                    SysAvailMgrListData(Found).AvailManagerType(Num) == SysAvailMgr_NightCycle) {
                    ShowSevereError("GetPlantLoopData/GetPlantAvailabilityManager: Invalid System Availability Manager Type entered=\"" +
                                    SysAvailMgrListData(Found).cAvailManagerType(Num) + "\".");
                    ShowContinueError("...this manager is not used in a Plant Loop.");
                    ShowContinueError("Occurs in AvailabilityManagerAssignmentList=\"" + AvailabilityListName + "\".");
                    ErrorsFound = true;
                }
            } // End of Num Loop

        } else {
            if (AvailabilityListName != "") {
                ShowWarningError("GetPlantLoopData/GetPlantAvailabilityManager: AvailabilityManagerAssignmentList=" + AvailabilityListName +
                                 " not found in lists.  No availability will be used.");
            }
            PlantAvailMgr(Loop).NumAvailManagers = 0;
            PlantAvailMgr(Loop).AvailStatus = NoAction;
            PlantAvailMgr(Loop).AvailManagerName.allocate(PlantAvailMgr(Loop).NumAvailManagers);
            PlantAvailMgr(Loop).AvailManagerType.allocate(PlantAvailMgr(Loop).NumAvailManagers);
            PlantAvailMgr(Loop).AvailManagerNum.allocate(PlantAvailMgr(Loop).NumAvailManagers);
        }
    }

    void GetAirLoopAvailabilityManager(EnergyPlusData &state,
                                       std::string const &AvailabilityListName, // name that should be an Availability Manager List Name
                                       int const Loop,                          // which loop this is
                                       int const NumAirLoops,                   // Total number of air loops
                                       bool &ErrorsFound                        // true if certain errors are detected here
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda Lawrie
        //       DATE WRITTEN   August 2007
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine gets the availability manager data for the indicated air
        // loop or for the indicated type of zone equipment component.
        // If the PriAirSysAvailMgr structure has not been allocated, it will be allocated
        // to "number of air loops".

        // METHODOLOGY EMPLOYED:
        // na

        // REFERENCES:
        // na

        // Using/Aliasing
        using namespace DataAirLoop;

        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:

        // SUBROUTINE PARAMETER DEFINITIONS:
        // na

        // INTERFACE BLOCK SPECIFICATIONS:
        // na

        // DERIVED TYPE DEFINITIONS:
        // na

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int Found;
        int Num;
        //  INTEGER :: CompNumAvailManagers ! Number of availability managers associated with a ZoneHVAC:* component

        if (GetAvailListsInput) {
            GetSysAvailManagerListInputs(state);
            GetAvailListsInput = false;
        }

        if (!allocated(state.dataAirLoop->PriAirSysAvailMgr)) {
            state.dataAirLoop->PriAirSysAvailMgr.allocate(NumAirLoops);
        }

        Found = 0;
        if (NumAvailManagerLists > 0) Found = UtilityRoutines::FindItemInList(AvailabilityListName, SysAvailMgrListData);

        if (Found != 0) {
            state.dataAirLoop->PriAirSysAvailMgr(Loop).NumAvailManagers = SysAvailMgrListData(Found).NumItems;
            state.dataAirLoop->PriAirSysAvailMgr(Loop).AvailStatus = NoAction;
            state.dataAirLoop->PriAirSysAvailMgr(Loop).StartTime = 0;
            state.dataAirLoop->PriAirSysAvailMgr(Loop).StopTime = 0;
            state.dataAirLoop->PriAirSysAvailMgr(Loop).ReqSupplyFrac = 1.0;
            state.dataAirLoop->PriAirSysAvailMgr(Loop).AvailManagerName.allocate(state.dataAirLoop->PriAirSysAvailMgr(Loop).NumAvailManagers);
            state.dataAirLoop->PriAirSysAvailMgr(Loop).AvailManagerType.allocate(state.dataAirLoop->PriAirSysAvailMgr(Loop).NumAvailManagers);
            state.dataAirLoop->PriAirSysAvailMgr(Loop).AvailManagerNum.allocate(state.dataAirLoop->PriAirSysAvailMgr(Loop).NumAvailManagers);
            for (Num = 1; Num <= state.dataAirLoop->PriAirSysAvailMgr(Loop).NumAvailManagers; ++Num) {
                state.dataAirLoop->PriAirSysAvailMgr(Loop).AvailManagerName(Num) = SysAvailMgrListData(Found).AvailManagerName(Num);
                state.dataAirLoop->PriAirSysAvailMgr(Loop).AvailManagerNum(Num) = 0;
                state.dataAirLoop->PriAirSysAvailMgr(Loop).AvailManagerType(Num) = SysAvailMgrListData(Found).AvailManagerType(Num);
                if (state.dataAirLoop->PriAirSysAvailMgr(Loop).AvailManagerType(Num) == 0) {
                    ShowSevereError("GetAirPathData/GetAirLoopAvailabilityManager: Invalid AvailabilityManagerAssignmentList Type entered=\"" +
                                    SysAvailMgrListData(Found).cAvailManagerType(Num) + "\".");
                    ShowContinueError("Occurs in AvailabilityManagerAssignmentList=\"" + SysAvailMgrListData(Found).AvailManagerName(Num) + "\".");
                    ErrorsFound = true;
                }
                if (SysAvailMgrListData(Found).AvailManagerType(Num) == SysAvailMgr_DiffThermo && Num != state.dataAirLoop->PriAirSysAvailMgr(Loop).NumAvailManagers) {
                    ShowWarningError("GetAirPathData/GetAirLoopAvailabilityManager: AvailabilityManager:DifferentialThermostat=\"" +
                                     SysAvailMgrListData(Found).AvailManagerName(Num) + "\".");
                    ShowContinueError(
                        "...is not the last manager on the AvailabilityManagerAssignmentList.  Any remaining managers will not be used.");
                    ShowContinueError("Occurs in AvailabilityManagerAssignmentList=\"" + SysAvailMgrListData(Found).AvailManagerName(Num) + "\".");
                }
            } // End of Num Loop

        } else {
            if (AvailabilityListName != "") {
                ShowWarningError("GetAirPathData/GetAirLoopAvailabilityManager: AvailabilityManagerAssignmentList=" + AvailabilityListName +
                                 " not found in lists.  No availability will be used.");
            }
            state.dataAirLoop->PriAirSysAvailMgr(Loop).NumAvailManagers = 0;
            state.dataAirLoop->PriAirSysAvailMgr(Loop).AvailStatus = NoAction;
            state.dataAirLoop->PriAirSysAvailMgr(Loop).AvailManagerName.allocate(state.dataAirLoop->PriAirSysAvailMgr(Loop).NumAvailManagers);
            state.dataAirLoop->PriAirSysAvailMgr(Loop).AvailManagerType.allocate(state.dataAirLoop->PriAirSysAvailMgr(Loop).NumAvailManagers);
            state.dataAirLoop->PriAirSysAvailMgr(Loop).AvailManagerNum.allocate(state.dataAirLoop->PriAirSysAvailMgr(Loop).NumAvailManagers);
        }
    }

    void GetZoneEqAvailabilityManager(EnergyPlusData &state,
                                      int const ZoneEquipType, // Type of ZoneHVAC:* component
                                      int const CompNum,       // Index of a particular ZoneHVAC:* component
                                      bool &ErrorsFound        // true if certain errors are detected here
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda Lawrie
        //       DATE WRITTEN   April 2011
        //       MODIFIED       Chandan Sharma, March 2011/July 2012 - FSEC: Added zone sys avail managers
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine gets the availability manager data for the indicated type of zone
        // equipment component.
        // If not allocated, ZoneComp structure will be allocated to "Total num of zone equip types" and
        // ZoneCompAvailMgrs structure will be allocated to "Total number of components of the indicated type".

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        std::string AvailabilityListName; // name that should be an Availability Manager List Name
        int Found;
        int Num;
        int CompNumAvailManagers; // Number of availability managers associated with a ZoneHVAC:* component

        if (GetAvailListsInput) {
            GetSysAvailManagerListInputs(state);
            GetAvailListsInput = false;
        }

        if (ZoneComp(ZoneEquipType).ZoneCompAvailMgrs(CompNum).Input) { // when both air loop and zone eq avail managers are present, zone avail mngrs
                                                                        // list name has not been read in first time through here (see end of if
                                                                        // block)
            AvailabilityListName = ZoneComp(ZoneEquipType).ZoneCompAvailMgrs(CompNum).AvailManagerListName;
            Found = 0;
            if (NumAvailManagerLists > 0) Found = UtilityRoutines::FindItemInList(AvailabilityListName, SysAvailMgrListData);
            if (Found != 0) {
                ZoneComp(ZoneEquipType).ZoneCompAvailMgrs(CompNum).NumAvailManagers = SysAvailMgrListData(Found).NumItems;
                CompNumAvailManagers = ZoneComp(ZoneEquipType).ZoneCompAvailMgrs(CompNum).NumAvailManagers;
                ZoneComp(ZoneEquipType).ZoneCompAvailMgrs(CompNum).AvailStatus = NoAction;
                ZoneComp(ZoneEquipType).ZoneCompAvailMgrs(CompNum).StartTime = 0;
                ZoneComp(ZoneEquipType).ZoneCompAvailMgrs(CompNum).StopTime = 0;
                if (!allocated(ZoneComp(ZoneEquipType).ZoneCompAvailMgrs(CompNum).AvailManagerName)) {
                    ZoneComp(ZoneEquipType).ZoneCompAvailMgrs(CompNum).AvailManagerName.allocate(CompNumAvailManagers);
                    ZoneComp(ZoneEquipType).ZoneCompAvailMgrs(CompNum).AvailManagerType.allocate(CompNumAvailManagers);
                    ZoneComp(ZoneEquipType).ZoneCompAvailMgrs(CompNum).AvailManagerNum.allocate(CompNumAvailManagers);
                }
                for (Num = 1; Num <= ZoneComp(ZoneEquipType).ZoneCompAvailMgrs(CompNum).NumAvailManagers; ++Num) {
                    ZoneComp(ZoneEquipType).ZoneCompAvailMgrs(CompNum).AvailManagerName(Num) = SysAvailMgrListData(Found).AvailManagerName(Num);
                    ZoneComp(ZoneEquipType).ZoneCompAvailMgrs(CompNum).AvailManagerNum(Num) = 0;
                    ZoneComp(ZoneEquipType).ZoneCompAvailMgrs(CompNum).AvailManagerType(Num) = SysAvailMgrListData(Found).AvailManagerType(Num);
                    if (ZoneComp(ZoneEquipType).ZoneCompAvailMgrs(CompNum).AvailManagerType(Num) == 0) {
                        ShowSevereError("GetZoneEqAvailabilityManager: Invalid AvailabilityManagerAssignmentList Type entered=\"" +
                                        SysAvailMgrListData(Found).cAvailManagerType(Num) + "\".");
                        ShowContinueError("Occurs in AvailabilityManagerAssignmentList=\"" + SysAvailMgrListData(Found).AvailManagerName(Num) +
                                          "\".");
                        ErrorsFound = true;
                    }
                    if (SysAvailMgrListData(Found).AvailManagerType(Num) == SysAvailMgr_DiffThermo &&
                        Num != ZoneComp(ZoneEquipType).ZoneCompAvailMgrs(CompNum).NumAvailManagers) {
                        ShowWarningError("GetZoneEqAvailabilityManager: AvailabilityManager:DifferentialThermostat=\"" +
                                         SysAvailMgrListData(Found).AvailManagerName(Num) + "\".");
                        ShowContinueError(
                            "...is not the last manager on the AvailabilityManagerAssignmentList.  Any remaining managers will not be used.");
                        ShowContinueError("Occurs in AvailabilityManagerAssignmentList=\"" + SysAvailMgrListData(Found).AvailManagerName(Num) +
                                          "\".");
                    }
                } // End of Num Loop
            }
            if (ZoneComp(ZoneEquipType).ZoneCompAvailMgrs(CompNum).Count > 0 || Found > 0)
                ZoneComp(ZoneEquipType).ZoneCompAvailMgrs(CompNum).Input = false;
            ZoneComp(ZoneEquipType).ZoneCompAvailMgrs(CompNum).Count += 1;
        }
    }

    void InitSysAvailManagers()
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Fred Buhl
        //       DATE WRITTEN   August 2001
        //       MODIFIED       Brent Griffith, CR8376 initialize to NoAction every timestep
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine is for initializations of the System Availability Manager objects.

        // METHODOLOGY EMPLOYED:
        // Uses the status flags to trigger initializations.

        // REFERENCES:
        // na

        // Using/Aliasing
        using DataZoneEquipment::NumValidSysAvailZoneComponents;
        using DataZoneEquipment::ZoneEquipConfig;

        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:
        // NA

        // SUBROUTINE PARAMETER DEFINITIONS:
        // na

        // INTERFACE BLOCK SPECIFICATIONS:
        // na

        // DERIVED TYPE DEFINITIONS:
        // na

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int SysAvailNum; // DO loop indes for Sys Avail Manager objects
        int ZoneEquipType;
        int ZoneListNum;
        int ScanZoneListNum;
        int ZoneNum;
        // One time initializations

        if (InitSysAvailManagers_MyOneTimeFlag) {

            for (SysAvailNum = 1; SysAvailNum <= NumNCycSysAvailMgrs; ++SysAvailNum) {
                if (NCycSysAvailMgrData(SysAvailNum).CtrlType == CycleOnControlZone) {
                    // set the controlled zone numbers
                    for (int index = 1; index <= NCycSysAvailMgrData(SysAvailNum).NumOfCtrlZones; ++index) {
                        for (int ControlledZoneNum = 1; ControlledZoneNum <= NumOfZones; ++ControlledZoneNum) {
                            if (allocated(ZoneEquipConfig)) {
                                if (ZoneEquipConfig(ControlledZoneNum).ActualZoneNum == NCycSysAvailMgrData(SysAvailNum).CtrlZonePtrs(index)) {
                                    NCycSysAvailMgrData(SysAvailNum).CtrlZonePtrs(index) = ControlledZoneNum;
                                    break;
                                }
                            }
                        }
                    }
                }
            }

            for (SysAvailNum = 1; SysAvailNum <= NumOptStartSysAvailMgrs; ++SysAvailNum) {
                {
                    auto const SELECT_CASE_var(OptStartSysAvailMgrData(SysAvailNum).CtrlType);
                    if (SELECT_CASE_var == ControlZone) {
                        // set the controlled zone numbers
                        for (int ControlledZoneNum = 1; ControlledZoneNum <= NumOfZones; ++ControlledZoneNum) {
                            if (allocated(ZoneEquipConfig)) {
                                if (ZoneEquipConfig(ControlledZoneNum).ActualZoneNum == OptStartSysAvailMgrData(SysAvailNum).ZoneNum) {
                                    OptStartSysAvailMgrData(SysAvailNum).ControlledZoneNum = ControlledZoneNum;
                                    break;
                                }
                            }
                        }
                    } else if (SELECT_CASE_var == MaximumOfZoneList) {
                        // a zone list
                        ZoneListNum = UtilityRoutines::FindItemInList(OptStartSysAvailMgrData(SysAvailNum).ZoneListName, ZoneList);
                        if (ZoneListNum > 0) {
                            OptStartSysAvailMgrData(SysAvailNum).NumOfZones = ZoneList(ZoneListNum).NumOfZones;
                            if (!allocated(OptStartSysAvailMgrData(SysAvailNum).ZonePtrs)) {
                                OptStartSysAvailMgrData(SysAvailNum).ZonePtrs.allocate({1, ZoneList(ZoneListNum).NumOfZones});
                            }
                            for (ScanZoneListNum = 1; ScanZoneListNum <= ZoneList(ZoneListNum).NumOfZones; ++ScanZoneListNum) {
                                ZoneNum = ZoneList(ZoneListNum).Zone(ScanZoneListNum);
                                OptStartSysAvailMgrData(SysAvailNum).ZonePtrs(ScanZoneListNum) = ZoneNum;
                            }
                        }
                    }
                }
            }

            for (SysAvailNum = 1; SysAvailNum <= NumNVentSysAvailMgrs; ++SysAvailNum) {
                // set the controlled zone numbers
                for (int ControlledZoneNum = 1; ControlledZoneNum <= NumOfZones; ++ControlledZoneNum) {
                    if (allocated(ZoneEquipConfig)) {
                        if (ZoneEquipConfig(ControlledZoneNum).ActualZoneNum == NVentSysAvailMgrData(SysAvailNum).ZoneNum) {
                            NVentSysAvailMgrData(SysAvailNum).ControlledZoneNum = ControlledZoneNum;
                            break;
                        }
                    }
                }
            }

            InitSysAvailManagers_MyOneTimeFlag = false;

        } // end 1 time initializations

        // initialize individual availability managers to no action (CR 8376 reporting issue)
        if (allocated(SchedSysAvailMgrData))
            for (auto &e : SchedSysAvailMgrData)
                e.AvailStatus = NoAction;
        if (allocated(SchedOnSysAvailMgrData))
            for (auto &e : SchedOnSysAvailMgrData)
                e.AvailStatus = NoAction;
        if (allocated(SchedOffSysAvailMgrData))
            for (auto &e : SchedOffSysAvailMgrData)
                e.AvailStatus = NoAction;
        if (allocated(NCycSysAvailMgrData))
            for (auto &e : NCycSysAvailMgrData)
                e.AvailStatus = NoAction;
        if (allocated(NVentSysAvailMgrData))
            for (auto &e : NVentSysAvailMgrData)
                e.AvailStatus = NoAction;
        if (allocated(DiffTSysAvailMgrData))
            for (auto &e : DiffTSysAvailMgrData)
                e.AvailStatus = NoAction;
        if (allocated(HiTurnOffSysAvailMgrData))
            for (auto &e : HiTurnOffSysAvailMgrData)
                e.AvailStatus = NoAction;
        if (allocated(HiTurnOnSysAvailMgrData))
            for (auto &e : HiTurnOnSysAvailMgrData)
                e.AvailStatus = NoAction;
        if (allocated(LoTurnOffSysAvailMgrData))
            for (auto &e : LoTurnOffSysAvailMgrData)
                e.AvailStatus = NoAction;
        if (allocated(LoTurnOnSysAvailMgrData))
            for (auto &e : LoTurnOnSysAvailMgrData)
                e.AvailStatus = NoAction;
        if (allocated(OptStartSysAvailMgrData)) {
            for (auto &e : OptStartSysAvailMgrData) {
                e.AvailStatus = NoAction;
                e.isSimulated = false;
            }
        }
        //  HybridVentSysAvailMgrData%AvailStatus= NoAction
        for (ZoneEquipType = 1; ZoneEquipType <= NumValidSysAvailZoneComponents; ++ZoneEquipType) { // loop over the zone equipment types
            if (allocated(ZoneComp)) {
                if (ZoneComp(ZoneEquipType).TotalNumComp > 0)
                    for (auto &e : ZoneComp(ZoneEquipType).ZoneCompAvailMgrs)
                        e.AvailStatus = NoAction;
            }
        }
    }

    void SimSysAvailManager(EnergyPlusData &state,
                            int const SysAvailType,
                            std::string const &SysAvailName,
                            int &SysAvailNum,
                            int const PriAirSysNum, // Primary Air System index. If being called for a ZoneHVAC:* component
                            int const PreviousStatus,
                            int &AvailStatus,
                            Optional_int_const ZoneEquipType, // Type of ZoneHVAC:* equipment component
                            Optional_int_const CompNum        // Index of ZoneHVAC:* equipment component
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Fred Buhl
        //       DATE WRITTEN   August 2001
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE
        // Loop over all the System Availability Managers and invoke the correct
        // System Availability Manager algorithm.

        // Using/Aliasing
        using General::TrimSigDigits;

        {
            auto const SELECT_CASE_var(SysAvailType);
            if (SELECT_CASE_var == SysAvailMgr_Scheduled) { // 'AvailabilityManager:Scheduled'
                if (SysAvailNum == 0) {
                    SysAvailNum = UtilityRoutines::FindItemInList(SysAvailName, SchedSysAvailMgrData);
                }
                if (SysAvailNum > 0) {
                    CalcSchedSysAvailMgr(SysAvailNum, AvailStatus);
                } else {
                    ShowFatalError("SimSysAvailManager: AvailabilityManager:Scheduled not found: " + SysAvailName);
                }

            } else if (SELECT_CASE_var == SysAvailMgr_ScheduledOn) { // 'AvailabilityManager:ScheduledOn'
                if (SysAvailNum == 0) {
                    SysAvailNum = UtilityRoutines::FindItemInList(SysAvailName, SchedOnSysAvailMgrData);
                }
                if (SysAvailNum > 0) {
                    CalcSchedOnSysAvailMgr(SysAvailNum, AvailStatus);
                } else {
                    ShowFatalError("SimSysAvailManager: AvailabilityManager:ScheduledOn not found: " + SysAvailName);
                }

            } else if (SELECT_CASE_var == SysAvailMgr_ScheduledOff) { // 'AvailabilityManager:ScheduledOff'
                if (SysAvailNum == 0) {
                    SysAvailNum = UtilityRoutines::FindItemInList(SysAvailName, SchedOffSysAvailMgrData);
                }
                if (SysAvailNum > 0) {
                    CalcSchedOffSysAvailMgr(SysAvailNum, AvailStatus);
                } else {
                    ShowFatalError("SimSysAvailManager: AvailabilityManager:ScheduledOff not found: " + SysAvailName);
                }

            } else if (SELECT_CASE_var == SysAvailMgr_NightCycle) { // 'AvailabilityManager:NightCycle'
                if (SysAvailNum == 0) {
                    SysAvailNum = UtilityRoutines::FindItemInList(SysAvailName, NCycSysAvailMgrData);
                }
                if (SysAvailNum > 0) {
                    CalcNCycSysAvailMgr(state, SysAvailNum, PriAirSysNum, AvailStatus, ZoneEquipType, CompNum);
                } else {
                    ShowFatalError("SimSysAvailManager: AvailabilityManager:NightCycle not found: " + SysAvailName);
                }

            } else if (SELECT_CASE_var == SysAvailMgr_OptimumStart) { // 'AvailabilityManager:OptimumStart'
                if (SysAvailNum == 0) {
                    SysAvailNum = UtilityRoutines::FindItemInList(SysAvailName, OptStartSysAvailMgrData);
                }
                if (SysAvailNum > 0) {
                    CalcOptStartSysAvailMgr(state, SysAvailNum, PriAirSysNum, AvailStatus, ZoneEquipType, CompNum);
                } else {
                    ShowFatalError("SimSysAvailManager: AvailabilityManager:OptimumStart not found: " + SysAvailName);
                }

            } else if (SELECT_CASE_var == SysAvailMgr_NightVent) { // 'AvailabilityManager:NightVentilation'
                if (SysAvailNum == 0) {
                    SysAvailNum = UtilityRoutines::FindItemInList(SysAvailName, NVentSysAvailMgrData);
                }
                if (SysAvailNum > 0) {
                    CalcNVentSysAvailMgr(state, SysAvailNum, PriAirSysNum, AvailStatus, ZoneEquipType);
                } else {
                    ShowFatalError("SimSysAvailManager: AvailabilityManager:NightVentilation not found: " + SysAvailName);
                }

            } else if (SELECT_CASE_var == SysAvailMgr_DiffThermo) { // 'AvailabilityManager:DifferentialThermostat'
                if (SysAvailNum == 0) {
                    SysAvailNum = UtilityRoutines::FindItemInList(SysAvailName, DiffTSysAvailMgrData);
                }
                if (SysAvailNum > 0) {
                    CalcDiffTSysAvailMgr(SysAvailNum, PreviousStatus, AvailStatus);
                } else {
                    ShowFatalError("SimSysAvailManager: AvailabilityManager:DifferentialThermostat not found: " + SysAvailName);
                }

            } else if (SELECT_CASE_var == SysAvailMgr_HiTempTOff) { // 'AvailabilityManager:HighTemperatureTurnOff'
                if (SysAvailNum == 0) {
                    SysAvailNum = UtilityRoutines::FindItemInList(SysAvailName, HiTurnOffSysAvailMgrData);
                }
                if (SysAvailNum > 0) {
                    CalcHiTurnOffSysAvailMgr(SysAvailNum, AvailStatus);
                } else {
                    ShowFatalError("SimSysAvailManager: AvailabilityManager:HighTemperatureTurnOff not found: " + SysAvailName);
                }

            } else if (SELECT_CASE_var == SysAvailMgr_HiTempTOn) { // 'AvailabilityManager:HighTemperatureTurnOn'
                if (SysAvailNum == 0) {
                    SysAvailNum = UtilityRoutines::FindItemInList(SysAvailName, HiTurnOnSysAvailMgrData);
                }
                if (SysAvailNum > 0) {
                    CalcHiTurnOnSysAvailMgr(SysAvailNum, AvailStatus);
                } else {
                    ShowFatalError("SimSysAvailManager: AvailabilityManager:HighTemperatureTurnOn not found: " + SysAvailName);
                }

            } else if (SELECT_CASE_var == SysAvailMgr_LoTempTOff) { // 'AvailabilityManager:LowTemperatureTurnOff'
                if (SysAvailNum == 0) {
                    SysAvailNum = UtilityRoutines::FindItemInList(SysAvailName, LoTurnOffSysAvailMgrData);
                }
                if (SysAvailNum > 0) {
                    CalcLoTurnOffSysAvailMgr(SysAvailNum, AvailStatus);
                } else {
                    ShowFatalError("SimSysAvailManager: AvailabilityManager:LowTemperatureTurnOff not found: " + SysAvailName);
                }

            } else if (SELECT_CASE_var == SysAvailMgr_LoTempTOn) { // 'AvailabilityManager:LowTemperatureTurnOn'
                if (SysAvailNum == 0) {
                    SysAvailNum = UtilityRoutines::FindItemInList(SysAvailName, LoTurnOnSysAvailMgrData);
                }
                if (SysAvailNum > 0) {
                    CalcLoTurnOnSysAvailMgr(SysAvailNum, AvailStatus);
                } else {
                    ShowFatalError("SimSysAvailManager: AvailabilityManager:LowTemperatureTurnOn not found: " + SysAvailName);
                }

            } else {
                ShowSevereError("AvailabilityManager Type not found: " + TrimSigDigits(SysAvailType));
                ShowContinueError("Occurs in Manager=" + SysAvailName);
                ShowFatalError("Preceding condition causes termination.");
            }
        }
    }

    void CalcSchedSysAvailMgr(int const SysAvailNum, // number of the current scheduled system availability manager
                              int &AvailStatus       // System status indicator
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Fred Buhl
        //       DATE WRITTEN   August 2001
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Set AvailStatus indicator for a primary air loop, plant loop or ZoneHVAC component.

        // METHODOLOGY EMPLOYED:
        // Looks at the System Availability Manager schedule and sets the
        // AvailStatus indicator accordingly. Mostly a useless algorithm
        // since the fan schedules can do the same thing.

        // REFERENCES:
        // na

        // USE STATEMENTS:

        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:

        // SUBROUTINE PARAMETER DEFINITIONS:

        // INTERFACE BLOCK SPECIFICATIONS:

        // DERIVED TYPE DEFINITIONS:
        // na

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:

        if (GetCurrentScheduleValue(SchedSysAvailMgrData(SysAvailNum).SchedPtr) > 0.0) {
            AvailStatus = CycleOn;
        } else {
            AvailStatus = ForceOff;
        }

        SchedSysAvailMgrData(SysAvailNum).AvailStatus = AvailStatus;
    }

    void CalcSchedOnSysAvailMgr(int const SysAvailNum, // number of the current scheduled on system availability manager
                                int &AvailStatus       // System status indicator
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         R. Raustad - FSEC
        //       DATE WRITTEN   August 2008
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Set AvailStatus indicator for a primary air loop, plant loop or ZoneHVAC component.

        // METHODOLOGY EMPLOYED:
        // Looks at the System Availability Manager schedule and sets the
        // AvailStatus indicator accordingly. If the schedule value is > 0
        // the availability status is CycleOn, ELSE the status is NoAction.

        // REFERENCES:
        // na

        // USE STATEMENTS:

        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:

        // SUBROUTINE PARAMETER DEFINITIONS:

        // INTERFACE BLOCK SPECIFICATIONS:

        // DERIVED TYPE DEFINITIONS:
        // na

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:

        if (GetCurrentScheduleValue(SchedOnSysAvailMgrData(SysAvailNum).SchedPtr) > 0.0) {
            AvailStatus = CycleOn;
        } else {
            AvailStatus = NoAction;
        }

        SchedOnSysAvailMgrData(SysAvailNum).AvailStatus = AvailStatus;
    }

    void CalcSchedOffSysAvailMgr(int const SysAvailNum, // number of the current scheduled off system availability manager
                                 int &AvailStatus       // System status indicator
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         R. Raustad - FSEC
        //       DATE WRITTEN   August 2008
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Set AvailStatus indicator for a primary air loop, plant loop or ZoneHVAC component.

        // METHODOLOGY EMPLOYED:
        // Looks at the System Availability Manager schedule and sets the
        // AvailStatus indicator accordingly.  If the schedule value is = 0
        // the availability status is ForceOff, ELSE the status is NoAction.

        // REFERENCES:
        // na

        // USE STATEMENTS:

        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:

        // SUBROUTINE PARAMETER DEFINITIONS:

        // INTERFACE BLOCK SPECIFICATIONS:

        // DERIVED TYPE DEFINITIONS:
        // na

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:

        if (GetCurrentScheduleValue(SchedOffSysAvailMgrData(SysAvailNum).SchedPtr) == 0.0) {
            AvailStatus = ForceOff;
        } else {
            AvailStatus = NoAction;
        }

        SchedOffSysAvailMgrData(SysAvailNum).AvailStatus = AvailStatus;
    }

    void CalcNCycSysAvailMgr(EnergyPlusData &state,
                             int const SysAvailNum,            // number of the current scheduled system availability manager
                             int const PriAirSysNum,           // number of the primary air system affected by this Avail. Manager
                             int &AvailStatus,                 // System status indicator
                             Optional_int_const ZoneEquipType, // Type of ZoneHVAC equipment component
                             Optional_int_const CompNum        // Index of ZoneHVAC equipment component
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Fred Buhl
        //       DATE WRITTEN   August 2001
        //       MODIFIED       March 2011, Chandan Sharma - FSEC: Allowed night cycle
        //                             availability manager to work for ZoneHVAC component
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Set AvailStatus indicator for a primary air loop or ZoneHVAC component.

        // METHODOLOGY EMPLOYED:
        // For air loop, depending on the type of control, looks at 1 named zone or all the zones
        // attached to a primary air system, compares zone temperature to the setup
        // or setback thermostat setpoint, and sets the AvailStaus indicator according
        // to whether the system needs to be cycled on or not.
        // For ZoneHVAC component, uses the exact same method as above but only looks at the
        // zone where component is located.

        // REFERENCES:
        // na

        // Using/Aliasing
        using namespace DataAirLoop;
        using DataHeatBalFanSys::TempControlType;
        using DataHeatBalFanSys::TempTstatAir;
        using DataHeatBalFanSys::TempZoneThermostatSetPoint;
        using DataHeatBalFanSys::ZoneThermostatSetPointHi;
        using DataHeatBalFanSys::ZoneThermostatSetPointLo;
        using DataZoneEquipment::ZoneEquipConfig;

        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:
        // SUBROUTINE PARAMETER DEFINITIONS:

        // INTERFACE BLOCK SPECIFICATIONS:

        // DERIVED TYPE DEFINITIONS:
        // na

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int StartTime;
        int StopTime;
        int ZoneInSysNum;
        int CtrldZoneNum;
        int ZoneNum;
        Real64 TempTol;
        static Array1D_bool ZoneCompNCControlType;
        int CyclingRunTimeControlType;

        if (present(ZoneEquipType)) {
            if (WarmupFlag && BeginDayFlag) {
                // reset start/stop times at beginning of each day during warmup to prevent non-convergence due to rotating start times
                ZoneComp(ZoneEquipType).ZoneCompAvailMgrs(CompNum).StartTime = SimTimeSteps;
                ZoneComp(ZoneEquipType).ZoneCompAvailMgrs(CompNum).StopTime = SimTimeSteps;
            }

            StartTime = ZoneComp(ZoneEquipType).ZoneCompAvailMgrs(CompNum).StartTime;
            StopTime = ZoneComp(ZoneEquipType).ZoneCompAvailMgrs(CompNum).StopTime;
            if (CalcNCycSysAvailMgr_OneTimeFlag) {
                ZoneCompNCControlType.dimension(NumNCycSysAvailMgrs, true);
                CalcNCycSysAvailMgr_OneTimeFlag = false;
            }
        } else {
            if (WarmupFlag && BeginDayFlag) {
                // reset start/stop times at beginning of each day during warmup to prevent non-convergence due to rotating start times
                state.dataAirLoop->PriAirSysAvailMgr(PriAirSysNum).StartTime = SimTimeSteps;
                state.dataAirLoop->PriAirSysAvailMgr(PriAirSysNum).StopTime = SimTimeSteps;
            }

            StartTime = state.dataAirLoop->PriAirSysAvailMgr(PriAirSysNum).StartTime;
            StopTime = state.dataAirLoop->PriAirSysAvailMgr(PriAirSysNum).StopTime;
        }
        // CR 7913 changed to allow during warmup
        if ((GetCurrentScheduleValue(NCycSysAvailMgrData(SysAvailNum).SchedPtr) <= 0.0) ||
            (GetCurrentScheduleValue(NCycSysAvailMgrData(SysAvailNum).FanSchedPtr) > 0.0)) {
            AvailStatus = NoAction;
            NCycSysAvailMgrData(SysAvailNum).AvailStatus = AvailStatus; // CR 8358
            return;
        }

        CyclingRunTimeControlType = NCycSysAvailMgrData(SysAvailNum).CycRunTimeCntrlType;

        if (CyclingRunTimeControlType == FixedRunTime) {
            TempTol = 0.5 * NCycSysAvailMgrData(SysAvailNum).TempTolRange;
        } else {
            TempTol = 0.05;
        }

        if (present(ZoneEquipType)) {
            if (SimTimeSteps >= StartTime && SimTimeSteps < StopTime &&
                (CyclingRunTimeControlType == FixedRunTime || CyclingRunTimeControlType == ThermostatWithMinimumRunTime)) { // if cycled on
                AvailStatus = CycleOn;
            } else if (SimTimeSteps == StopTime && CyclingRunTimeControlType == FixedRunTime) { // if end of cycle run time, shut down if fan off
                AvailStatus = NoAction;
            } else {

                {
                    auto const SELECT_CASE_var(NCycSysAvailMgrData(SysAvailNum).CtrlType); // select type of night cycle control

                    if (SELECT_CASE_var == StayOff) {
                        AvailStatus = NoAction;

                    } else if (SELECT_CASE_var == CycleOnControlZone) {

                        ZoneNum = NCycSysAvailMgrData(SysAvailNum).CtrlZonePtrs(1);

                        {
                            auto const SELECT_CASE_var1(TempControlType(ZoneNum)); // select on thermostat control

                            if (SELECT_CASE_var1 == SingleHeatingSetPoint) {
                                if (TempTstatAir(ZoneNum) < TempZoneThermostatSetPoint(ZoneNum) - TempTol) {
                                    AvailStatus = CycleOn;
                                } else {
                                    AvailStatus = NoAction;
                                }

                            } else if (SELECT_CASE_var1 == SingleCoolingSetPoint) {
                                if (TempTstatAir(ZoneNum) > TempZoneThermostatSetPoint(ZoneNum) + TempTol) {
                                    AvailStatus = CycleOn;
                                } else {
                                    AvailStatus = NoAction;
                                }

                            } else if (SELECT_CASE_var1 == SingleHeatCoolSetPoint) {
                                if ((TempTstatAir(ZoneNum) < TempZoneThermostatSetPoint(ZoneNum) - TempTol) ||
                                    (TempTstatAir(ZoneNum) > TempZoneThermostatSetPoint(ZoneNum) + TempTol)) {
                                    AvailStatus = CycleOn;
                                } else {
                                    AvailStatus = NoAction;
                                }

                            } else if (SELECT_CASE_var1 == DualSetPointWithDeadBand) {
                                if ((TempTstatAir(ZoneNum) < ZoneThermostatSetPointLo(ZoneNum) - TempTol) ||
                                    (TempTstatAir(ZoneNum) > ZoneThermostatSetPointHi(ZoneNum) + TempTol)) {
                                    AvailStatus = CycleOn;
                                } else {
                                    AvailStatus = NoAction;
                                }

                            } else {
                                AvailStatus = NoAction;
                            }
                        } // end select on thermostat control

                    } else if ((SELECT_CASE_var == CycleOnAny) || (SELECT_CASE_var == ZoneFansOnly)) {
                        if (ZoneCompNCControlType(SysAvailNum)) {
                            ShowWarningError("AvailabilityManager:NightCycle = " + NCycSysAvailMgrData(SysAvailNum).Name +
                                             ", is specified for a ZoneHVAC component.");
                            ShowContinueError("The only valid Control Types for ZoneHVAC components are CycleOnControlZone and StayOff.");
                            ShowContinueError("Night Cycle operation will not be modeled for ZoneHVAC components that reference this manager.");
                            ZoneCompNCControlType(SysAvailNum) = false;
                        }
                        AvailStatus = NoAction;

                    } else {
                        AvailStatus = NoAction;
                    }
                } // end select type of night cycle control

                if (AvailStatus == CycleOn) {                      // reset the start and stop times
                    if (CyclingRunTimeControlType == Thermostat) { // Cycling Run Time is ignored
                        ZoneComp(ZoneEquipType).ZoneCompAvailMgrs(CompNum).StartTime = SimTimeSteps;
                        ZoneComp(ZoneEquipType).ZoneCompAvailMgrs(CompNum).StopTime = SimTimeSteps;
                    } else {
                        ZoneComp(ZoneEquipType).ZoneCompAvailMgrs(CompNum).StartTime = SimTimeSteps;
                        ZoneComp(ZoneEquipType).ZoneCompAvailMgrs(CompNum).StopTime =
                            SimTimeSteps + NCycSysAvailMgrData(SysAvailNum).CyclingTimeSteps;
                    }
                }
            }
        } else {
            if (SimTimeSteps >= StartTime && SimTimeSteps < StopTime &&
                (CyclingRunTimeControlType == FixedRunTime || CyclingRunTimeControlType == ThermostatWithMinimumRunTime)) { // if cycled on
                AvailStatus = NCycSysAvailMgrData(SysAvailNum).PriorAvailStatus;
                if (NCycSysAvailMgrData(SysAvailNum).CtrlType == ZoneFansOnly) AvailStatus = CycleOnZoneFansOnly;
            } else if (SimTimeSteps == StopTime && CyclingRunTimeControlType == FixedRunTime) { // if end of cycle run time, shut down if fan off
                AvailStatus = NoAction;
            } else {

                {
                    auto const SELECT_CASE_var(NCycSysAvailMgrData(SysAvailNum).CtrlType); // select type of night cycle control

                    if (SELECT_CASE_var == StayOff) {
                        AvailStatus = NoAction;

                    } else if ((SELECT_CASE_var == CycleOnAny) || (SELECT_CASE_var == ZoneFansOnly)) {

                        // If no zones cooled, Availstatus could be "unknown"
                        AvailStatus = NoAction;

                        for (ZoneInSysNum = 1; ZoneInSysNum <= state.dataAirLoop->AirToZoneNodeInfo(PriAirSysNum).NumZonesCooled;
                             ++ZoneInSysNum) { // loop over zones in system

                            CtrldZoneNum = state.dataAirLoop->AirToZoneNodeInfo(PriAirSysNum).CoolCtrlZoneNums(ZoneInSysNum);
                            ZoneNum = ZoneEquipConfig(CtrldZoneNum).ActualZoneNum;

                            {
                                auto const SELECT_CASE_var1(TempControlType(ZoneNum)); // select on thermostat control

                                if (SELECT_CASE_var1 == SingleHeatingSetPoint) {
                                    if (TempTstatAir(ZoneNum) < TempZoneThermostatSetPoint(ZoneNum) - TempTol) {
                                        AvailStatus = CycleOn;
                                        break;
                                    } else {
                                        AvailStatus = NoAction;
                                    }

                                } else if (SELECT_CASE_var1 == SingleCoolingSetPoint) {
                                    if (TempTstatAir(ZoneNum) > TempZoneThermostatSetPoint(ZoneNum) + TempTol) {
                                        AvailStatus = CycleOn;
                                        break;
                                    } else {
                                        AvailStatus = NoAction;
                                    }

                                } else if (SELECT_CASE_var1 == SingleHeatCoolSetPoint) {
                                    if ((TempTstatAir(ZoneNum) < TempZoneThermostatSetPoint(ZoneNum) - TempTol) ||
                                        (TempTstatAir(ZoneNum) > TempZoneThermostatSetPoint(ZoneNum) + TempTol)) {
                                        AvailStatus = CycleOn;
                                        break;
                                    } else {
                                        AvailStatus = NoAction;
                                    }

                                } else if (SELECT_CASE_var1 == DualSetPointWithDeadBand) {
                                    if ((TempTstatAir(ZoneNum) < ZoneThermostatSetPointLo(ZoneNum) - TempTol) ||
                                        (TempTstatAir(ZoneNum) > ZoneThermostatSetPointHi(ZoneNum) + TempTol)) {
                                        AvailStatus = CycleOn;
                                        break;
                                    } else {
                                        AvailStatus = NoAction;
                                    }

                                } else {
                                    AvailStatus = NoAction;
                                }
                            } // end select on thermostat control

                        } // end loop over zones in system

                    } else if (SELECT_CASE_var == CycleOnControlZone) {
                        AvailStatus = NoAction;
                        if (CoolingZoneOutOfTolerance(
                                NCycSysAvailMgrData(SysAvailNum).CtrlZonePtrs, NCycSysAvailMgrData(SysAvailNum).NumOfCtrlZones, TempTol))
                            AvailStatus = CycleOn;
                        if (HeatingZoneOutOfTolerance(
                                NCycSysAvailMgrData(SysAvailNum).CtrlZonePtrs, NCycSysAvailMgrData(SysAvailNum).NumOfCtrlZones, TempTol))
                            AvailStatus = CycleOn;
                    } else if (SELECT_CASE_var == CycleOnAnyCoolingOrHeatingZone) {
                        if (CoolingZoneOutOfTolerance(
                                NCycSysAvailMgrData(SysAvailNum).CoolingZonePtrs, NCycSysAvailMgrData(SysAvailNum).NumOfCoolingZones, TempTol)) {
                            AvailStatus = CycleOn;
                        } else if (HeatingZoneOutOfTolerance(NCycSysAvailMgrData(SysAvailNum).HeatingZonePtrs,
                                                             NCycSysAvailMgrData(SysAvailNum).NumOfHeatingZones,
                                                             TempTol)) {
                            AvailStatus = CycleOn;
                        } else if (HeatingZoneOutOfTolerance(NCycSysAvailMgrData(SysAvailNum).HeatZnFanZonePtrs,
                                                             NCycSysAvailMgrData(SysAvailNum).NumOfHeatZnFanZones,
                                                             TempTol)) {
                            AvailStatus = CycleOnZoneFansOnly;
                        } else {
                            AvailStatus = NoAction;
                        }
                    } else if (SELECT_CASE_var == CycleOnAnyCoolingZone) {
                        if (CoolingZoneOutOfTolerance(
                                NCycSysAvailMgrData(SysAvailNum).CoolingZonePtrs, NCycSysAvailMgrData(SysAvailNum).NumOfCoolingZones, TempTol)) {
                            AvailStatus = CycleOn;
                        } else {
                            AvailStatus = NoAction;
                        }
                    } else if (SELECT_CASE_var == CycleOnAnyHeatingZone) {
                        if (HeatingZoneOutOfTolerance(
                                NCycSysAvailMgrData(SysAvailNum).HeatingZonePtrs, NCycSysAvailMgrData(SysAvailNum).NumOfHeatingZones, TempTol)) {
                            AvailStatus = CycleOn;
                        } else if (HeatingZoneOutOfTolerance(NCycSysAvailMgrData(SysAvailNum).HeatZnFanZonePtrs,
                                                             NCycSysAvailMgrData(SysAvailNum).NumOfHeatZnFanZones,
                                                             TempTol)) {
                            AvailStatus = CycleOnZoneFansOnly;
                        } else {
                            AvailStatus = NoAction;
                        }
                    } else if (SELECT_CASE_var == CycleOnControlZone) {
                        if (HeatingZoneOutOfTolerance(
                                NCycSysAvailMgrData(SysAvailNum).HeatZnFanZonePtrs, NCycSysAvailMgrData(SysAvailNum).NumOfHeatZnFanZones, TempTol)) {
                            AvailStatus = CycleOnZoneFansOnly;
                        } else {
                            AvailStatus = NoAction;
                        }
                    } else {
                        AvailStatus = NoAction;
                    }
                } // end select type of night cycle control

                if ((AvailStatus == CycleOn) || (AvailStatus == CycleOnZoneFansOnly)) { // reset the start and stop times
                    if (NCycSysAvailMgrData(SysAvailNum).CtrlType == ZoneFansOnly) AvailStatus = CycleOnZoneFansOnly;
                    // issue #6151
                    if (CyclingRunTimeControlType == Thermostat) { // Cycling Run Time is ignored
                        state.dataAirLoop->PriAirSysAvailMgr(PriAirSysNum).StartTime = SimTimeSteps;
                        state.dataAirLoop->PriAirSysAvailMgr(PriAirSysNum).StopTime = SimTimeSteps;
                    } else {
                        state.dataAirLoop->PriAirSysAvailMgr(PriAirSysNum).StartTime = SimTimeSteps;
                        state.dataAirLoop->PriAirSysAvailMgr(PriAirSysNum).StopTime = SimTimeSteps + NCycSysAvailMgrData(SysAvailNum).CyclingTimeSteps;
                    }
                }
            }
        }
        NCycSysAvailMgrData(SysAvailNum).AvailStatus = AvailStatus;
        NCycSysAvailMgrData(SysAvailNum).PriorAvailStatus = AvailStatus;
    }

    bool CoolingZoneOutOfTolerance(Array1D_int const ZonePtrList, // list of controlled zone pointers
                                   int const NumZones,            // number of zones in list
                                   Real64 const TempTolerance     // temperature tolerance
    )
    {
        // Check if any zone temperature is above the cooling setpoint plus tolerance
        for (int Index = 1; Index <= NumZones; ++Index) { // loop over zones in list
            int ZoneNum = ZonePtrList(Index);
            {
                auto const tstatType(DataHeatBalFanSys::TempControlType(ZoneNum));

                if ((tstatType == SingleCoolingSetPoint) || (tstatType == SingleHeatCoolSetPoint)) {
                    if (DataHeatBalFanSys::TempTstatAir(ZoneNum) > DataHeatBalFanSys::TempZoneThermostatSetPoint(ZoneNum) + TempTolerance) {
                        return true; // return on the first zone found
                    }
                } else if (tstatType == DualSetPointWithDeadBand) {
                    if (DataHeatBalFanSys::TempTstatAir(ZoneNum) > DataHeatBalFanSys::ZoneThermostatSetPointHi(ZoneNum) + TempTolerance) {
                        return true; // return on the first zone found
                    }
                }
            }
        }
        return false;
    }

    bool HeatingZoneOutOfTolerance(Array1D_int const ZonePtrList, // list of controlled zone pointers
                                   int const NumZones,            // number of zones in list
                                   Real64 const TempTolerance     // temperature tolerance
    )
    {
        // Check if any zone temperature is below the heating setpoint less tolerance
        for (int Index = 1; Index <= NumZones; ++Index) { // loop over zones in list
            int ZoneNum = ZonePtrList(Index);
            {
                auto const tstatType(DataHeatBalFanSys::TempControlType(ZoneNum));

                if ((tstatType == SingleHeatingSetPoint) || (tstatType == SingleHeatCoolSetPoint)) {
                    if (DataHeatBalFanSys::TempTstatAir(ZoneNum) < DataHeatBalFanSys::TempZoneThermostatSetPoint(ZoneNum) - TempTolerance) {
                        return true; // return on the first zone found
                    }
                } else if (tstatType == DualSetPointWithDeadBand) {
                    if (DataHeatBalFanSys::TempTstatAir(ZoneNum) < DataHeatBalFanSys::ZoneThermostatSetPointLo(ZoneNum) - TempTolerance) {
                        return true; // return on the first zone found
                    }
                }
            }
        }
        return false;
    }

    void CalcOptStartSysAvailMgr(EnergyPlusData &state,
                                 int const SysAvailNum,                       // number of the current scheduled system availability manager
                                 int const PriAirSysNum,                      // number of the primary air system affected by this Avail. Manager
                                 int &AvailStatus,                            // System status indicator
                                 Optional_int_const EP_UNUSED(ZoneEquipType), // Type of ZoneHVAC equipment component
                                 Optional_int_const EP_UNUSED(CompNum)        // Index of ZoneHVAC equipment component
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR            Xiufeng Pang (XP)
        //       DATE WRITTEN      August 2013
        //       MODIFIED
        //       RE-ENGINEERED

        // PURPOSE OF THIS SUBROUTINE:
        // Set AvailStatus indicator for a primary air loop, plant loop or ZoneHVAC component

        // METHODOLOGY EMPLOYED:
        // Sets the AvailStatus indicator according to the
        // optimum start algorithm

        // Using/Aliasing
        using namespace DataAirLoop;
        using DataEnvironment::DayOfWeekTomorrow;
        using DataEnvironment::DayOfYear;
        using DataEnvironment::DSTIndicator;
        using DataHeatBalFanSys::TempControlType;
        using DataHeatBalFanSys::TempTstatAir;
        using DataHeatBalFanSys::TempZoneThermostatSetPoint;
        using DataHeatBalFanSys::ZoneThermostatSetPointHi;
        using DataHeatBalFanSys::ZoneThermostatSetPointLo;
        using DataZoneControls::OccRoomTSetPointCool;
        using DataZoneControls::OccRoomTSetPointHeat;
        using DataZoneEquipment::ZoneEquipConfig;

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:

        int ScheduleIndex;
        Array2D<Real64> DayValues;
        Array2D<Real64> DayValuesTmr;
        int JDay;
        int TmrJDay;
        int TmrDayOfWeek;
        int ZoneNum;
        Real64 FanStartTime;
        Real64 FanStartTimeTmr;
        Real64 PreStartTime;
        Real64 PreStartTimeTmr;
        Real64 DeltaTime;
        Real64 TempDiff;
        Real64 TempDiffHi;
        Real64 TempDiffLo;
        bool FirstTimeATGFlag(true);
        bool OverNightStartFlag(false); // Flag to indicate the optimum start starts before mid night.
        bool CycleOnFlag(false);
        bool OSReportVarFlag(true);
        int NumPreDays;
        int NumOfZonesInList;
        Real64 AdaTempGradHeat;
        Real64 AdaTempGradCool;
        Real64 ATGUpdateTime1(0.0);
        Real64 ATGUpdateTime2(0.0);
        Real64 ATGUpdateTemp1(0.0);
        Real64 ATGUpdateTemp2(0.0);
        bool ATGUpdateFlag1(false);
        bool ATGUpdateFlag2(false);
        int ATGCounter;
        int ATGWCZoneNumHi;
        int ATGWCZoneNumLo;
        Real64 NumHoursBeforeOccupancy; // Variable to store the number of hours before occupancy in optimum start period
        bool exitLoop;                  // exit loop on found data

        auto &OptStartMgr(OptStartSysAvailMgrData(SysAvailNum));

        // some avail managers may be used in air loop and plant availability manager lists, if so they only need be simulated once
        if (OptStartMgr.isSimulated) {
            AvailStatus = OptStartMgr.AvailStatus;
            return;
        }
        OptStartMgr.isSimulated = true;

        // update air loop specific data
        TempDiffLo = OptStartMgr.TempDiffLo;
        TempDiffHi = OptStartMgr.TempDiffHi;
        ATGWCZoneNumLo = OptStartMgr.ATGWCZoneNumLo;
        ATGWCZoneNumHi = OptStartMgr.ATGWCZoneNumHi;
        CycleOnFlag = OptStartMgr.CycleOnFlag;
        ATGUpdateFlag1 = OptStartMgr.ATGUpdateFlag1;
        ATGUpdateFlag2 = OptStartMgr.ATGUpdateFlag2;
        NumHoursBeforeOccupancy = OptStartMgr.NumHoursBeforeOccupancy;
        FirstTimeATGFlag = OptStartMgr.FirstTimeATGFlag;
        OverNightStartFlag = OptStartMgr.OverNightStartFlag;
        OSReportVarFlag = OptStartMgr.OSReportVarFlag;

        if (OptStartMgr.CtrlAlgType == AdaptiveTemperatureGradient) {
            NumPreDays = OptStartMgr.NumPreDays;
            if (!allocated(OptStart_AdaTempGradTrdHeat)) {
                OptStart_AdaTempGradTrdHeat.allocate(NumPreDays);
                OptStart_AdaTempGradTrdCool.allocate(NumPreDays);
            }
            if (!allocated(OptStartMgr.AdaTempGradTrdHeat)) {
                OptStartMgr.AdaTempGradTrdHeat.allocate(NumPreDays);
                OptStartMgr.AdaTempGradTrdHeat = 0.0;
                OptStartMgr.AdaTempGradTrdCool.allocate(NumPreDays);
                OptStartMgr.AdaTempGradTrdCool = 0.0;
            }
            OptStart_AdaTempGradTrdHeat = OptStartMgr.AdaTempGradTrdHeat;
            OptStart_AdaTempGradTrdCool = OptStartMgr.AdaTempGradTrdCool;
            AdaTempGradHeat = OptStartMgr.AdaTempGradHeat;
            AdaTempGradCool = OptStartMgr.AdaTempGradCool;
            ATGUpdateTime1 = OptStartMgr.ATGUpdateTime1;
            ATGUpdateTime2 = OptStartMgr.ATGUpdateTime2;
            ATGUpdateTemp1 = OptStartMgr.ATGUpdateTemp1;
            ATGUpdateTemp2 = OptStartMgr.ATGUpdateTemp2;
        }

        // add or use a new variable OptStartSysAvailMgrData(SysAvailNum)%FanSchIndex
        if (KickOffSimulation) {
            AvailStatus = NoAction;
        } else {
            ScheduleIndex = GetScheduleIndex(state, OptStartMgr.FanSched);
            JDay = DayOfYear;
            TmrJDay = JDay + 1;
            TmrDayOfWeek = DayOfWeekTomorrow;

            DayValues.allocate(NumOfTimeStepInHour, 24);
            DayValuesTmr.allocate(NumOfTimeStepInHour, 24);
            if (!allocated(OptStartData.OptStartFlag)) {
                OptStartData.OptStartFlag.allocate(NumOfZones);
                OptStartData.OccStartTime.allocate(NumOfZones);
            }
            if (!allocated(OptStartData.ActualZoneNum)) OptStartData.ActualZoneNum.allocate(NumOfZones);

            // OptStartFlag needs to be reset each timestep to not stay set to true post-occupancy
            OptStartData.OptStartFlag = false;

            // reset OptStartData once per beginning of day
            if (BeginDayFlag) {
                NumHoursBeforeOccupancy = 0.0; // Initialize the hours of optimum start period. This variable is for reporting purpose.
                if (BeginOfDayResetFlag) {
                    OptStartData.OccStartTime = 22.99; // initialize the zone occupancy start time
                    BeginOfDayResetFlag = false;
                }
            }
            if (!BeginDayFlag) BeginOfDayResetFlag = true;

            GetScheduleValuesForDay(state, ScheduleIndex, DayValues);
            GetScheduleValuesForDay(state, ScheduleIndex, DayValuesTmr, TmrJDay, TmrDayOfWeek);

            FanStartTime = 0.0;
            FanStartTimeTmr = 0.0;
            exitLoop = false;
            for (int I = 1; I <= 24; ++I) {
                for (int J = 1; J <= NumOfTimeStepInHour; ++J) {
                    if (DayValues(J, I) <= 0.0) continue;
                    FanStartTime = I - 1 + 1.0 / NumOfTimeStepInHour * J - 0.01;
                    exitLoop = true;
                    break;
                }
                if (exitLoop) break;
            }

            exitLoop = false;
            for (int I = 1; I <= 24; ++I) {
                for (int J = 1; J <= NumOfTimeStepInHour; ++J) {
                    if (DayValuesTmr(J, I) <= 0.0) continue;
                    FanStartTimeTmr = I - 1 + 1.0 / NumOfTimeStepInHour * J - 0.01;
                    exitLoop = true;
                    break;
                }
                if (exitLoop) break;
            }

            if (FanStartTimeTmr == 0.0) FanStartTimeTmr = 24.0;

            // Pass the start time to ZoneTempPredictorCorrector
            for (int counter = 1; counter <= state.dataAirLoop->AirToZoneNodeInfo(PriAirSysNum).NumZonesCooled; ++counter) {
                int actZoneNum = ZoneEquipConfig(state.dataAirLoop->AirToZoneNodeInfo(PriAirSysNum).CoolCtrlZoneNums(counter)).ActualZoneNum;
                OptStartData.OccStartTime(actZoneNum) = FanStartTime;
                OptStartData.ActualZoneNum(actZoneNum) = actZoneNum;
            }
            for (int counter = 1; counter <= state.dataAirLoop->AirToZoneNodeInfo(PriAirSysNum).NumZonesHeated; ++counter) {
                int actZoneNum = ZoneEquipConfig(state.dataAirLoop->AirToZoneNodeInfo(PriAirSysNum).HeatCtrlZoneNums(counter)).ActualZoneNum;
                OptStartData.OccStartTime(actZoneNum) = FanStartTime;
                OptStartData.ActualZoneNum(actZoneNum) = actZoneNum;
            }

            if (DSTIndicator > 0) {
                --FanStartTime;
                --FanStartTimeTmr;
            }

            {
                auto const SELECT_CASE_var(OptStartMgr.CtrlAlgType);
                if (SELECT_CASE_var == ConstantStartTime) {
                    if (OptStartMgr.CtrlType == StayOff) {
                        AvailStatus = NoAction;
                    } else {
                        DeltaTime = OptStartMgr.ConstStartTime;
                        if (DeltaTime > OptStartMgr.MaxOptStartTime) {
                            DeltaTime = OptStartMgr.MaxOptStartTime;
                        }
                        PreStartTime = FanStartTime - DeltaTime;
                        if (PreStartTime < 0.0) PreStartTime = -0.1;
                        PreStartTimeTmr = FanStartTimeTmr - DeltaTime;
                        if (PreStartTimeTmr < 0.0) {
                            PreStartTimeTmr += 24.0;
                            OverNightStartFlag = true;
                        } else {
                            OverNightStartFlag = false;
                        }
                        if (!OverNightStartFlag) {
                            if (FanStartTime == 0.0 || PreviousHour > FanStartTime) {
                                AvailStatus = NoAction;
                                OSReportVarFlag = true;
                            } else if (PreStartTime < CurrentTime) {
                                if (OSReportVarFlag) {
                                    NumHoursBeforeOccupancy = DeltaTime;
                                    OSReportVarFlag = false;
                                }
                                AvailStatus = CycleOn;
                                OptStartMgr.SetOptStartFlag(state, PriAirSysNum);
                            } else {
                                AvailStatus = NoAction;
                                OSReportVarFlag = true;
                            }
                        } else {
                            if (FanStartTime == 0.0 || (HourOfDay > FanStartTime && CurrentTime <= PreStartTimeTmr)) {
                                AvailStatus = NoAction;
                                OSReportVarFlag = true;
                            } else if (PreStartTime < CurrentTime || PreStartTimeTmr < CurrentTime) {
                                if (OSReportVarFlag) {
                                    NumHoursBeforeOccupancy = DeltaTime;
                                    OSReportVarFlag = false;
                                }
                                AvailStatus = CycleOn;
                                OptStartMgr.SetOptStartFlag(state, PriAirSysNum);
                            } else {
                                AvailStatus = NoAction;
                                OSReportVarFlag = true;
                            }
                        }
                    }

                } else if (SELECT_CASE_var == ConstantTemperatureGradient) {
                    if (OptStartMgr.CtrlType == ControlZone) {
                        ZoneNum = OptStartMgr.ZoneNum;
                        if ((!allocated(TempTstatAir)) || (!allocated(ZoneThermostatSetPointLo)) || (!allocated(ZoneThermostatSetPointHi))) {
                            TempDiff = 0.0;
                        } else {
                            if (!CycleOnFlag) {
                                if (allocated(OccRoomTSetPointHeat) && allocated(OccRoomTSetPointCool)) {
                                    TempDiffHi = TempTstatAir(ZoneNum) - OccRoomTSetPointCool(ZoneNum);
                                    TempDiffLo = TempTstatAir(ZoneNum) - OccRoomTSetPointHeat(ZoneNum);
                                } else {
                                    TempDiffHi = 0.0;
                                    TempDiffLo = 0.0;
                                }
                            }
                        }

                        if (TempDiffHi < 0.0) {
                            TempDiff = TempDiffLo;
                            if (TempDiff < 0.0) { // Heating Mode
                                TempDiff = std::abs(TempDiff);
                                DeltaTime = TempDiff / OptStartMgr.ConstTGradHeat;
                                if (DeltaTime > OptStartMgr.MaxOptStartTime) {
                                    DeltaTime = OptStartMgr.MaxOptStartTime;
                                }
                                PreStartTime = FanStartTime - DeltaTime;
                                if (PreStartTime < 0) PreStartTime = -0.1;
                                PreStartTimeTmr = FanStartTimeTmr - DeltaTime;
                                if (PreStartTimeTmr < 0) {
                                    PreStartTimeTmr += 24.0;
                                    OverNightStartFlag = true;
                                } else {
                                    OverNightStartFlag = false;
                                }
                                if (!OverNightStartFlag) {
                                    if (FanStartTime == 0.0 || CurrentTime > FanStartTime) {
                                        CycleOnFlag = false;
                                        OSReportVarFlag = true;
                                    } else if (CycleOnFlag) {
                                        AvailStatus = CycleOn;
                                        OptStartMgr.SetOptStartFlag(state, PriAirSysNum);
                                        if (CurrentTime > FanStartTime) CycleOnFlag = false;
                                    } else if (PreStartTime < CurrentTime) {
                                        AvailStatus = CycleOn;
                                        CycleOnFlag = true;
                                        if (OSReportVarFlag) {
                                            NumHoursBeforeOccupancy = DeltaTime;
                                            OSReportVarFlag = false;
                                        }
                                        OptStartMgr.SetOptStartFlag(state, PriAirSysNum);
                                    } else {
                                        AvailStatus = NoAction;
                                        CycleOnFlag = false;
                                        OSReportVarFlag = true;
                                    }
                                } else {
                                    if (FanStartTime == 0.0 || (CurrentTime > FanStartTime && CurrentTime <= PreStartTimeTmr)) {
                                        AvailStatus = NoAction;
                                        CycleOnFlag = false;
                                        OSReportVarFlag = true;
                                    } else if (CycleOnFlag) {
                                        AvailStatus = CycleOn;
                                        OptStartMgr.SetOptStartFlag(state, PriAirSysNum);
                                        if (CurrentTime > FanStartTime && CurrentTime < PreStartTimeTmr) CycleOnFlag = false;
                                    } else if (PreStartTime < CurrentTime || PreStartTimeTmr < CurrentTime) {
                                        if (OSReportVarFlag) {
                                            NumHoursBeforeOccupancy = DeltaTime;
                                            OSReportVarFlag = false;
                                        }
                                        AvailStatus = CycleOn;
                                        CycleOnFlag = true;
                                        OptStartMgr.SetOptStartFlag(state, PriAirSysNum);
                                    } else {
                                        AvailStatus = NoAction;
                                        CycleOnFlag = false;
                                        OSReportVarFlag = true;
                                    }
                                }
                            } else {
                                AvailStatus = NoAction;
                                CycleOnFlag = false;
                            }
                        } else if (OccRoomTSetPointCool(ZoneNum) < 50.0) { // Cooling Mode
                            TempDiff = TempDiffHi;
                            DeltaTime = TempDiff / OptStartMgr.ConstTGradCool;
                            if (DeltaTime > OptStartMgr.MaxOptStartTime) {
                                DeltaTime = OptStartMgr.MaxOptStartTime;
                            }
                            PreStartTime = FanStartTime - DeltaTime;
                            if (PreStartTime < 0) PreStartTime = -0.1;
                            PreStartTimeTmr = FanStartTimeTmr - DeltaTime;
                            if (PreStartTimeTmr < 0) {
                                PreStartTimeTmr += 24.0;
                                OverNightStartFlag = true;
                            } else {
                                OverNightStartFlag = false;
                            }
                            if (!OverNightStartFlag) {
                                if (FanStartTime == 0.0 || CurrentTime > FanStartTime) {
                                    AvailStatus = NoAction;
                                    CycleOnFlag = false;
                                    OSReportVarFlag = true;
                                } else if (CycleOnFlag) {
                                    AvailStatus = CycleOn;
                                    OptStartMgr.SetOptStartFlag(state, PriAirSysNum);
                                } else if (PreStartTime < CurrentTime) {
                                    if (OSReportVarFlag) {
                                        NumHoursBeforeOccupancy = DeltaTime;
                                        OSReportVarFlag = false;
                                    }
                                    AvailStatus = CycleOn;
                                    CycleOnFlag = true;
                                    OptStartMgr.SetOptStartFlag(state, PriAirSysNum);
                                } else {
                                    AvailStatus = NoAction;
                                    CycleOnFlag = false;
                                    OSReportVarFlag = true;
                                }
                            } else {
                                if (FanStartTime == 0.0 || (CurrentTime > FanStartTime && CurrentTime <= PreStartTimeTmr)) {
                                    AvailStatus = NoAction;
                                    CycleOnFlag = false;
                                    OSReportVarFlag = true;
                                } else if (CycleOnFlag) {
                                    AvailStatus = CycleOn;
                                    OptStartMgr.SetOptStartFlag(state, PriAirSysNum);
                                } else if (PreStartTime < CurrentTime || PreStartTimeTmr < CurrentTime) {
                                    if (OSReportVarFlag) {
                                        NumHoursBeforeOccupancy = DeltaTime;
                                        OSReportVarFlag = false;
                                    }
                                    AvailStatus = CycleOn;
                                    CycleOnFlag = true;
                                    OptStartMgr.SetOptStartFlag(state, PriAirSysNum);
                                } else {
                                    AvailStatus = NoAction;
                                    CycleOnFlag = false;
                                    OSReportVarFlag = true;
                                }
                            }
                        } else {
                            AvailStatus = NoAction;
                            CycleOnFlag = false;
                        }
                    } else if (OptStartMgr.CtrlType == MaximumOfZoneList) {

                        NumOfZonesInList = OptStartMgr.NumOfZones;
                        if ((!allocated(TempTstatAir)) || (!allocated(ZoneThermostatSetPointLo)) || (!allocated(ZoneThermostatSetPointHi))) {
                            TempDiff = 0.0;
                        } else {
                            if (!CycleOnFlag) {
                                if (allocated(OccRoomTSetPointHeat) && allocated(OccRoomTSetPointCool)) {
                                    TempDiffHi = 0.0;
                                    TempDiffLo = 0.0;
                                    for (ZoneNum = 1; ZoneNum <= NumOfZonesInList; ++ZoneNum) {
                                        TempDiff = TempTstatAir(OptStartMgr.ZonePtrs(ZoneNum)) - OccRoomTSetPointCool(OptStartMgr.ZonePtrs(ZoneNum));
                                        TempDiffHi = max(TempDiffHi, TempDiff);
                                        TempDiff = TempTstatAir(OptStartMgr.ZonePtrs(ZoneNum)) - OccRoomTSetPointHeat(OptStartMgr.ZonePtrs(ZoneNum));
                                        TempDiffLo = min(TempDiffLo, TempDiff);
                                    }
                                } else {
                                    TempDiffHi = 0.0;
                                    TempDiffLo = 0.0;
                                }
                            }
                        }
                        if ((TempDiffHi < 0.0 && TempDiffLo < 0.0) ||
                            (std::abs(TempDiffLo) > std::abs(TempDiffHi) && TempDiffLo < 0)) { // Heating Mode
                            TempDiff = TempDiffLo;
                            TempDiff = std::abs(TempDiff);
                            DeltaTime = TempDiff / OptStartMgr.ConstTGradHeat;
                            if (DeltaTime > OptStartMgr.MaxOptStartTime) {
                                DeltaTime = OptStartMgr.MaxOptStartTime;
                            }
                            PreStartTime = FanStartTime - DeltaTime;
                            if (PreStartTime < 0) PreStartTime = -0.1;
                            PreStartTimeTmr = FanStartTimeTmr - DeltaTime;
                            if (PreStartTimeTmr < 0) {
                                PreStartTimeTmr += 24.0;
                                OverNightStartFlag = true;
                            } else {
                                OverNightStartFlag = false;
                            }
                            if (!OverNightStartFlag) {
                                if (FanStartTime == 0.0 || CurrentTime > FanStartTime) {
                                    AvailStatus = NoAction;
                                    CycleOnFlag = false;
                                    OSReportVarFlag = true;
                                } else if (CycleOnFlag) {
                                    AvailStatus = CycleOn;
                                    OptStartMgr.SetOptStartFlag(state, PriAirSysNum);
                                    if (CurrentTime > FanStartTime) CycleOnFlag = false;
                                } else if (PreStartTime < CurrentTime) {
                                    if (OSReportVarFlag) {
                                        NumHoursBeforeOccupancy = DeltaTime;
                                        OSReportVarFlag = false;
                                    }
                                    AvailStatus = CycleOn;
                                    CycleOnFlag = true;
                                    OptStartMgr.SetOptStartFlag(state, PriAirSysNum);
                                } else {
                                    AvailStatus = NoAction;
                                    CycleOnFlag = false;
                                    OSReportVarFlag = true;
                                }
                            } else {
                                if (FanStartTime == 0.0 || (CurrentTime > FanStartTime && CurrentTime <= PreStartTimeTmr)) {
                                    AvailStatus = NoAction;
                                    CycleOnFlag = false;
                                    OSReportVarFlag = true;
                                } else if (CycleOnFlag) {
                                    AvailStatus = CycleOn;
                                    OptStartMgr.SetOptStartFlag(state, PriAirSysNum);
                                    if (CurrentTime > FanStartTime && CurrentTime < PreStartTimeTmr) CycleOnFlag = false;
                                } else if (PreStartTime < CurrentTime || PreStartTimeTmr < CurrentTime) {
                                    if (OSReportVarFlag) {
                                        NumHoursBeforeOccupancy = DeltaTime;
                                        OSReportVarFlag = false;
                                    }
                                    AvailStatus = CycleOn;
                                    CycleOnFlag = true;
                                    OptStartMgr.SetOptStartFlag(state, PriAirSysNum);
                                } else {
                                    AvailStatus = NoAction;
                                    CycleOnFlag = false;
                                    OSReportVarFlag = true;
                                }
                            }
                        } else if (TempDiffHi <= 0.0 && TempDiffLo >= 0.0) { // not heating and not cooling
                            AvailStatus = NoAction;
                            CycleOnFlag = false;
                            TempDiffHi = 0.0;
                            TempDiffLo = 0.0;
                        } else if (TempDiffHi < 30.0) { // Cooling Mode
                            TempDiff = TempDiffHi;
                            DeltaTime = TempDiff / OptStartMgr.ConstTGradCool;
                            if (DeltaTime > OptStartMgr.MaxOptStartTime) {
                                DeltaTime = OptStartMgr.MaxOptStartTime;
                            }
                            PreStartTime = FanStartTime - DeltaTime;
                            if (PreStartTime < 0) PreStartTime = -0.1;
                            PreStartTimeTmr = FanStartTimeTmr - DeltaTime;
                            if (PreStartTimeTmr < 0) {
                                PreStartTimeTmr += 24.0;
                                OverNightStartFlag = true;
                            } else {
                                OverNightStartFlag = false;
                            }
                            if (!OverNightStartFlag) {
                                if (FanStartTime == 0.0 || CurrentTime > FanStartTime) {
                                    AvailStatus = NoAction;
                                    CycleOnFlag = false;
                                    OSReportVarFlag = true;
                                } else if (CycleOnFlag) {
                                    AvailStatus = CycleOn;
                                    OptStartMgr.SetOptStartFlag(state, PriAirSysNum);
                                } else if (PreStartTime < CurrentTime) {
                                    if (OSReportVarFlag) {
                                        NumHoursBeforeOccupancy = DeltaTime;
                                        OSReportVarFlag = false;
                                    }
                                    AvailStatus = CycleOn;
                                    CycleOnFlag = true;
                                    OptStartMgr.SetOptStartFlag(state, PriAirSysNum);
                                } else {
                                    AvailStatus = NoAction;
                                    CycleOnFlag = false;
                                    OSReportVarFlag = true;
                                }
                            } else {
                                if (FanStartTime == 0.0 || (CurrentTime > FanStartTime && CurrentTime <= PreStartTimeTmr)) {
                                    AvailStatus = NoAction;
                                    CycleOnFlag = false;
                                    OSReportVarFlag = true;
                                } else if (CycleOnFlag) {
                                    AvailStatus = CycleOn;
                                    OptStartMgr.SetOptStartFlag(state, PriAirSysNum);
                                } else if (PreStartTime < CurrentTime || PreStartTimeTmr < CurrentTime) {
                                    if (OSReportVarFlag) {
                                        NumHoursBeforeOccupancy = DeltaTime;
                                        OSReportVarFlag = false;
                                    }
                                    AvailStatus = CycleOn;
                                    CycleOnFlag = true;
                                    OptStartMgr.SetOptStartFlag(state, PriAirSysNum);
                                } else {
                                    AvailStatus = NoAction;
                                    CycleOnFlag = false;
                                    OSReportVarFlag = true;
                                }
                            }
                        } else {
                            AvailStatus = NoAction;
                            CycleOnFlag = false;
                        }
                    } else {
                        AvailStatus = NoAction;
                    }

                } else if (SELECT_CASE_var == AdaptiveTemperatureGradient) {

                    if (OptStartMgr.CtrlType == ControlZone) {
                        ZoneNum = OptStartMgr.ZoneNum;
                        if ((!allocated(TempTstatAir)) || (!allocated(ZoneThermostatSetPointLo)) || (!allocated(ZoneThermostatSetPointHi))) {
                            TempDiff = 0.0;
                        } else {
                            if (!CycleOnFlag) {
                                if (allocated(OccRoomTSetPointHeat) && allocated(OccRoomTSetPointCool)) {
                                    TempDiffHi = TempTstatAir(ZoneNum) - OccRoomTSetPointCool(ZoneNum);
                                    TempDiffLo = TempTstatAir(ZoneNum) - OccRoomTSetPointHeat(ZoneNum);
                                } else {
                                    TempDiffHi = 0.0;
                                    TempDiffLo = 0.0;
                                }
                            }
                        }
                        // Store adaptive temperature gradients for previous days and calculate the adaptive temp gradients
                        //-----------------------------------------------------------------------------
                        if (WarmupFlag) {
                            AdaTempGradHeat = OptStartMgr.InitTGradHeat;
                            AdaTempGradCool = OptStartMgr.InitTGradCool;
                        } else if (DayOfSim == BeginDay && BeginDayFlag) {
                            OptStart_AdaTempGradTrdHeat = OptStartMgr.InitTGradHeat;
                            AdaTempGradHeat = OptStartMgr.InitTGradHeat;
                            OptStart_AdaTempGradTrdCool = OptStartMgr.InitTGradCool;
                            AdaTempGradCool = OptStartMgr.InitTGradCool;
                        } else {
                            if (BeginDayFlag && FirstTimeATGFlag) {
                                FirstTimeATGFlag = false;
                                AdaTempGradHeat += OptStart_AdaTempGradTrdHeat(NumPreDays) / NumPreDays - OptStart_AdaTempGradTrdHeat(1) / NumPreDays;
                                AdaTempGradCool += OptStart_AdaTempGradTrdCool(NumPreDays) / NumPreDays - OptStart_AdaTempGradTrdCool(1) / NumPreDays;
                                if (FanStartTime > 0) {
                                    for (ATGCounter = 1; ATGCounter <= NumPreDays - 1; ++ATGCounter) {
                                        OptStart_AdaTempGradTrdHeat(ATGCounter) = OptStart_AdaTempGradTrdHeat(ATGCounter + 1);
                                        OptStart_AdaTempGradTrdCool(ATGCounter) = OptStart_AdaTempGradTrdCool(ATGCounter + 1);
                                    }
                                }
                            }
                        }

                        if (CurrentTime >= 1.0) FirstTimeATGFlag = true;
                        //------------------------------------------------------------------------------

                        if (TempDiffHi < 0.0) {
                            TempDiff = TempDiffLo;
                            if (TempDiff < 0.0) { // Heating Mode
                                TempDiff = std::abs(TempDiff);
                                DeltaTime = TempDiff / AdaTempGradHeat;
                                if (DeltaTime > OptStartMgr.MaxOptStartTime) {
                                    DeltaTime = OptStartMgr.MaxOptStartTime;
                                }
                                PreStartTime = FanStartTime - DeltaTime;
                                if (PreStartTime < 0.0) PreStartTime = -0.1;
                                PreStartTimeTmr = FanStartTimeTmr - DeltaTime;
                                if (PreStartTimeTmr < 0.0) {
                                    PreStartTimeTmr += 24.0;
                                    OverNightStartFlag = true;
                                } else {
                                    OverNightStartFlag = false;
                                }
                                if (!OverNightStartFlag) {
                                    if (FanStartTime == 0.0 || CurrentTime > FanStartTime) {
                                        AvailStatus = NoAction;
                                        CycleOnFlag = false;
                                        OSReportVarFlag = true;
                                    } else if (CycleOnFlag) {
                                        AvailStatus = CycleOn;
                                        OptStartMgr.SetOptStartFlag(state, PriAirSysNum);
                                        if (CurrentTime > FanStartTime) CycleOnFlag = false;
                                        // Calculate the current day actual temperature gradient --------------------------
                                        if (!WarmupFlag) {
                                            if (ATGUpdateFlag1) {
                                                ATGUpdateTime1 = CurrentTime;
                                                ATGUpdateTemp1 = TempTstatAir(ZoneNum);
                                                ATGUpdateFlag1 = false;
                                            }
                                            if (TempTstatAir(ZoneNum) >= OccRoomTSetPointHeat(ZoneNum) && ATGUpdateFlag2) {
                                                ATGUpdateTime2 = CurrentTime;
                                                ATGUpdateTemp2 = TempTstatAir(ZoneNum);
                                                ATGUpdateFlag2 = false;
                                                if (std::abs(ATGUpdateTime2 - ATGUpdateTime1) > 1.e-10) {
                                                    OptStart_AdaTempGradTrdHeat(NumPreDays) =
                                                        (ATGUpdateTemp2 - ATGUpdateTemp1) / (ATGUpdateTime2 - ATGUpdateTime1);
                                                } else {
                                                    OptStart_AdaTempGradTrdHeat(NumPreDays) = (ATGUpdateTemp2 - ATGUpdateTemp1) * NumOfTimeStepInHour;
                                                }
                                            }
                                        }
                                        //---------------------------------------------------------------------------------
                                    } else if (PreStartTime < CurrentTime) {
                                        if (OSReportVarFlag) {
                                            NumHoursBeforeOccupancy = DeltaTime;
                                            OSReportVarFlag = false;
                                        }
                                        AvailStatus = CycleOn;
                                        CycleOnFlag = true;
                                        ATGUpdateFlag1 = true;
                                        ATGUpdateFlag2 = true;
                                        OptStartMgr.SetOptStartFlag(state, PriAirSysNum);
                                    } else {
                                        AvailStatus = NoAction;
                                        CycleOnFlag = false;
                                        OSReportVarFlag = true;
                                    }
                                } else {
                                    if (FanStartTime == 0.0 || (CurrentTime > FanStartTime && CurrentTime <= PreStartTimeTmr)) {
                                        AvailStatus = NoAction;
                                        CycleOnFlag = false;
                                        OSReportVarFlag = true;
                                    } else if (CycleOnFlag) {
                                        AvailStatus = CycleOn;
                                        OptStartMgr.SetOptStartFlag(state, PriAirSysNum);
                                        if (CurrentTime > FanStartTime && CurrentTime < PreStartTimeTmr) CycleOnFlag = false;
                                        // Calculate the current day actual temperature gradient --------------------------
                                        if (!WarmupFlag) {
                                            if (ATGUpdateFlag1) {
                                                ATGUpdateTime1 = CurrentTime;
                                                ATGUpdateTemp1 = TempTstatAir(ZoneNum);
                                                ATGUpdateFlag1 = false;
                                            }
                                            if (TempTstatAir(ZoneNum) >= OccRoomTSetPointHeat(ZoneNum) && ATGUpdateFlag2) {
                                                ATGUpdateTime2 = CurrentTime;
                                                ATGUpdateTemp2 = TempTstatAir(ZoneNum);
                                                ATGUpdateFlag2 = false;
                                                if (std::abs(ATGUpdateTime2 - ATGUpdateTime1 + 24.0) > 1.e-10) {
                                                    OptStart_AdaTempGradTrdHeat(NumPreDays) =
                                                        (ATGUpdateTemp2 - ATGUpdateTemp1) / (ATGUpdateTime2 - ATGUpdateTime1 + 24.0);
                                                } else {
                                                    OptStart_AdaTempGradTrdHeat(NumPreDays) = (ATGUpdateTemp2 - ATGUpdateTemp1) * NumOfTimeStepInHour;
                                                }
                                            }
                                        }
                                        //---------------------------------------------------------------------------------
                                    } else if (PreStartTime < CurrentTime || PreStartTimeTmr < CurrentTime) {
                                        if (OSReportVarFlag) {
                                            NumHoursBeforeOccupancy = DeltaTime;
                                            OSReportVarFlag = false;
                                        }
                                        AvailStatus = CycleOn;
                                        CycleOnFlag = true;
                                        ATGUpdateFlag1 = true;
                                        ATGUpdateFlag2 = true;
                                        OptStartMgr.SetOptStartFlag(state, PriAirSysNum);
                                    } else {
                                        AvailStatus = NoAction;
                                        CycleOnFlag = false;
                                        OSReportVarFlag = true;
                                    }
                                }
                            } else {
                                AvailStatus = NoAction;
                                CycleOnFlag = false;
                            }
                        } else if (OccRoomTSetPointCool(ZoneNum) < 50.0) { // Cooling Mode
                            TempDiff = TempDiffHi;
                            DeltaTime = TempDiff / AdaTempGradCool;
                            if (DeltaTime > OptStartMgr.MaxOptStartTime) {
                                DeltaTime = OptStartMgr.MaxOptStartTime;
                            }
                            PreStartTime = FanStartTime - DeltaTime;
                            if (PreStartTime < 0.0) PreStartTime = -0.1;
                            PreStartTimeTmr = FanStartTimeTmr - DeltaTime;
                            if (PreStartTimeTmr < 0.0) {
                                PreStartTimeTmr += 24.0;
                                OverNightStartFlag = true;
                            } else {
                                OverNightStartFlag = false;
                            }
                            if (!OverNightStartFlag) {
                                if (FanStartTime == 0.0 || CurrentTime > FanStartTime) {
                                    AvailStatus = NoAction;
                                    CycleOnFlag = false;
                                    OSReportVarFlag = true;
                                } else if (CycleOnFlag) {
                                    if (OSReportVarFlag) {
                                        NumHoursBeforeOccupancy = DeltaTime;
                                        OSReportVarFlag = false;
                                    }
                                    AvailStatus = CycleOn;
                                    OptStartMgr.SetOptStartFlag(state, PriAirSysNum);
                                    if (!WarmupFlag) {
                                        if (ATGUpdateFlag1) {
                                            ATGUpdateTime1 = CurrentTime;
                                            ATGUpdateTemp1 = TempTstatAir(ZoneNum);
                                            ATGUpdateFlag1 = false;
                                        }
                                        if (TempTstatAir(ZoneNum) <= OccRoomTSetPointCool(ZoneNum) && ATGUpdateFlag2) {
                                            ATGUpdateTime2 = CurrentTime;
                                            ATGUpdateTemp2 = TempTstatAir(ZoneNum);
                                            ATGUpdateFlag2 = false;
                                            if (std::abs(ATGUpdateTime2 - ATGUpdateTime1) > 1.e-10) {
                                                OptStart_AdaTempGradTrdCool(NumPreDays) =
                                                    (ATGUpdateTemp1 - ATGUpdateTemp2) / (ATGUpdateTime2 - ATGUpdateTime1);
                                            } else {
                                                OptStart_AdaTempGradTrdCool(NumPreDays) = (ATGUpdateTemp1 - ATGUpdateTemp2) * NumOfTimeStepInHour;
                                            }
                                        }
                                    }
                                } else if (PreStartTime < CurrentTime) {
                                    AvailStatus = CycleOn;
                                    CycleOnFlag = true;
                                    ATGUpdateFlag1 = true;
                                    ATGUpdateFlag2 = true;
                                    OptStartMgr.SetOptStartFlag(state, PriAirSysNum);
                                } else {
                                    AvailStatus = NoAction;
                                    CycleOnFlag = false;
                                    OSReportVarFlag = true;
                                }
                            } else {
                                if (FanStartTime == 0.0 || (CurrentTime > FanStartTime && CurrentTime <= PreStartTimeTmr)) {
                                    AvailStatus = NoAction;
                                    CycleOnFlag = false;
                                    OSReportVarFlag = true;
                                } else if (CycleOnFlag) {
                                    AvailStatus = CycleOn;
                                    if (!WarmupFlag) {
                                        if (ATGUpdateFlag1) {
                                            ATGUpdateTime1 = CurrentTime;
                                            ATGUpdateTemp1 = TempTstatAir(ZoneNum);
                                            ATGUpdateFlag1 = false;
                                        }
                                        if (TempTstatAir(ZoneNum) <= OccRoomTSetPointCool(ZoneNum) && ATGUpdateFlag2) {
                                            ATGUpdateTime2 = CurrentTime;
                                            ATGUpdateTemp2 = TempTstatAir(ZoneNum);
                                            ATGUpdateFlag2 = false;
                                            if (std::abs(ATGUpdateTime2 - ATGUpdateTime1 + 24.0) > 1.e-10) {
                                                OptStart_AdaTempGradTrdCool(NumPreDays) =
                                                    (ATGUpdateTemp1 - ATGUpdateTemp2) / (ATGUpdateTime2 - ATGUpdateTime1 + 24.0);
                                            } else {
                                                OptStart_AdaTempGradTrdCool(NumPreDays) = (ATGUpdateTemp1 - ATGUpdateTemp2) * NumOfTimeStepInHour;
                                            }
                                        }
                                    }
                                    OptStartMgr.SetOptStartFlag(state, PriAirSysNum);
                                } else if (PreStartTime < CurrentTime || PreStartTimeTmr < CurrentTime) {
                                    if (OSReportVarFlag) {
                                        NumHoursBeforeOccupancy = DeltaTime;
                                        OSReportVarFlag = false;
                                    }
                                    AvailStatus = CycleOn;
                                    CycleOnFlag = true;
                                    ATGUpdateFlag1 = true;
                                    ATGUpdateFlag2 = true;
                                    OptStartMgr.SetOptStartFlag(state, PriAirSysNum);
                                } else {
                                    AvailStatus = NoAction;
                                    CycleOnFlag = false;
                                    OSReportVarFlag = true;
                                }
                            }
                        } else { // Not heating nor cooling mode
                            AvailStatus = NoAction;
                            CycleOnFlag = false;
                        }
                    } else if (OptStartMgr.CtrlType == MaximumOfZoneList) {

                        NumOfZonesInList = OptStartMgr.NumOfZones;
                        ATGWCZoneNumHi = OptStartMgr.ZonePtrs(1);
                        ATGWCZoneNumLo = OptStartMgr.ZonePtrs(1);
                        if ((!allocated(TempTstatAir)) || (!allocated(ZoneThermostatSetPointLo)) || (!allocated(ZoneThermostatSetPointHi))) {
                            TempDiff = 0.0;
                        } else {
                            if (!CycleOnFlag) {
                                if (allocated(OccRoomTSetPointHeat) && allocated(OccRoomTSetPointCool)) {
                                    TempDiffHi = 0.0;
                                    TempDiffLo = 0.0;
                                    ATGWCZoneNumHi = OptStartMgr.ZonePtrs(1);
                                    ATGWCZoneNumLo = OptStartMgr.ZonePtrs(1);
                                    for (ZoneNum = 1; ZoneNum <= NumOfZonesInList; ++ZoneNum) {
                                        TempDiff = TempTstatAir(OptStartMgr.ZonePtrs(ZoneNum)) - OccRoomTSetPointCool(OptStartMgr.ZonePtrs(ZoneNum));
                                        TempDiffHi = max(TempDiffHi, TempDiff);
                                        // Store the worse case zone number for actual temperature gradient calculation
                                        if (TempDiff == TempDiffHi) {
                                            ATGWCZoneNumHi = OptStartMgr.ZonePtrs(ZoneNum);
                                        }
                                        TempDiff = TempTstatAir(OptStartMgr.ZonePtrs(ZoneNum)) - OccRoomTSetPointHeat(OptStartMgr.ZonePtrs(ZoneNum));
                                        TempDiffLo = min(TempDiffLo, TempDiff);
                                        if (TempDiff == TempDiffLo) {
                                            ATGWCZoneNumLo = OptStartMgr.ZonePtrs(ZoneNum);
                                        }
                                    }
                                } else {
                                    TempDiffHi = 0.0;
                                    TempDiffLo = 0.0;
                                }
                            }
                        }
                        // Store adaptive temperature gradients for previous days and calculate the adaptive temp gradients
                        //-----------------------------------------------------------------------------
                        if (WarmupFlag) {
                            AdaTempGradHeat = OptStartMgr.InitTGradHeat;
                            AdaTempGradCool = OptStartMgr.InitTGradCool;
                        } else if (DayOfSim == BeginDay && BeginDayFlag) {
                            OptStart_AdaTempGradTrdHeat = OptStartMgr.InitTGradHeat;
                            AdaTempGradHeat = OptStartMgr.InitTGradHeat;
                            OptStart_AdaTempGradTrdCool = OptStartMgr.InitTGradCool;
                            AdaTempGradCool = OptStartMgr.InitTGradCool;
                        } else {
                            if (BeginDayFlag && FirstTimeATGFlag) {
                                FirstTimeATGFlag = false;
                                AdaTempGradHeat += OptStart_AdaTempGradTrdHeat(NumPreDays) / NumPreDays - OptStart_AdaTempGradTrdHeat(1) / NumPreDays;
                                AdaTempGradCool += OptStart_AdaTempGradTrdCool(NumPreDays) / NumPreDays - OptStart_AdaTempGradTrdCool(1) / NumPreDays;
                                if (FanStartTime > 0) {
                                    for (ATGCounter = 1; ATGCounter <= NumPreDays - 1; ++ATGCounter) {
                                        OptStart_AdaTempGradTrdHeat(ATGCounter) = OptStart_AdaTempGradTrdHeat(ATGCounter + 1);
                                        OptStart_AdaTempGradTrdCool(ATGCounter) = OptStart_AdaTempGradTrdCool(ATGCounter + 1);
                                    }
                                }
                            }
                        }

                        if (CurrentTime >= 1.0) FirstTimeATGFlag = true;
                        //------------------------------------------------------------------------------

                        if ((TempDiffHi < 0.0 && TempDiffLo < 0.0) ||
                            (std::abs(TempDiffLo) > std::abs(TempDiffHi) && TempDiffLo < 0.0)) { // Heating Mode
                            TempDiff = TempDiffLo;
                            TempDiff = std::abs(TempDiff);
                            DeltaTime = TempDiff / AdaTempGradHeat;
                            if (DeltaTime > OptStartMgr.MaxOptStartTime) {
                                DeltaTime = OptStartMgr.MaxOptStartTime;
                            }
                            PreStartTime = FanStartTime - DeltaTime;
                            if (PreStartTime < 0.0) PreStartTime = -0.1;
                            PreStartTimeTmr = FanStartTimeTmr - DeltaTime;
                            if (PreStartTimeTmr < 0.0) {
                                PreStartTimeTmr += 24.0;
                                OverNightStartFlag = true;
                            } else {
                                OverNightStartFlag = false;
                            }
                            if (!OverNightStartFlag) {
                                if (FanStartTime == 0.0 || CurrentTime > FanStartTime) {
                                    OSReportVarFlag = true;
                                    AvailStatus = NoAction;
                                    CycleOnFlag = false;
                                } else if (CycleOnFlag) {
                                    AvailStatus = CycleOn;
                                    OptStartMgr.SetOptStartFlag(state, PriAirSysNum);
                                    if (CurrentTime > FanStartTime) CycleOnFlag = false;
                                    // Calculate the current day actual temperature gradient --------------------------
                                    if (!WarmupFlag) {
                                        if (ATGUpdateFlag1) {
                                            ATGUpdateTime1 = CurrentTime;
                                            ATGUpdateTemp1 = TempTstatAir(ATGWCZoneNumLo);
                                            ATGUpdateFlag1 = false;
                                        }
                                        if (TempTstatAir(ATGWCZoneNumLo) >= OccRoomTSetPointHeat(ATGWCZoneNumLo) && ATGUpdateFlag2) {
                                            ATGUpdateTime2 = CurrentTime;
                                            ATGUpdateTemp2 = TempTstatAir(ATGWCZoneNumLo);
                                            ATGUpdateFlag2 = false;
                                            if (std::abs(ATGUpdateTime2 - ATGUpdateTime1) > 1.e-10) {
                                                OptStart_AdaTempGradTrdHeat(NumPreDays) =
                                                    (ATGUpdateTemp2 - ATGUpdateTemp1) / (ATGUpdateTime2 - ATGUpdateTime1);
                                            } else {
                                                OptStart_AdaTempGradTrdHeat(NumPreDays) = (ATGUpdateTemp2 - ATGUpdateTemp1) * NumOfTimeStepInHour;
                                            }
                                        }
                                    }
                                    //---------------------------------------------------------------------------------
                                } else if (PreStartTime < CurrentTime) {
                                    if (OSReportVarFlag) {
                                        NumHoursBeforeOccupancy = DeltaTime;
                                        OSReportVarFlag = false;
                                    }
                                    AvailStatus = CycleOn;
                                    CycleOnFlag = true;
                                    ATGUpdateFlag1 = true;
                                    ATGUpdateFlag2 = true;
                                    OptStartMgr.SetOptStartFlag(state, PriAirSysNum);
                                } else {
                                    AvailStatus = NoAction;
                                    CycleOnFlag = false;
                                    OSReportVarFlag = true;
                                }
                            } else {
                                if (FanStartTime == 0.0 || (CurrentTime > FanStartTime && CurrentTime <= PreStartTimeTmr)) {
                                    AvailStatus = NoAction;
                                    CycleOnFlag = false;
                                    OSReportVarFlag = true;
                                } else if (CycleOnFlag) {
                                    AvailStatus = CycleOn;
                                    // Calculate the current day actual temperature gradient --------------------------
                                    if (!WarmupFlag) {
                                        if (ATGUpdateFlag1) {
                                            ATGUpdateTime1 = CurrentTime;
                                            ATGUpdateTemp1 = TempTstatAir(ATGWCZoneNumLo);
                                            ATGUpdateFlag1 = false;
                                        }
                                        if (TempTstatAir(ATGWCZoneNumLo) >= OccRoomTSetPointHeat(ATGWCZoneNumLo) && ATGUpdateFlag2) {
                                            ATGUpdateTime2 = CurrentTime;
                                            ATGUpdateTemp2 = TempTstatAir(ATGWCZoneNumLo);
                                            ATGUpdateFlag2 = false;
                                            if (std::abs(ATGUpdateTime2 - ATGUpdateTime1 + 24.0) > 1.e-10) {
                                                OptStart_AdaTempGradTrdHeat(NumPreDays) =
                                                    (ATGUpdateTemp2 - ATGUpdateTemp1) / (ATGUpdateTime2 - ATGUpdateTime1 + 24.0);
                                            } else {
                                                OptStart_AdaTempGradTrdHeat(NumPreDays) = (ATGUpdateTemp2 - ATGUpdateTemp1) * NumOfTimeStepInHour;
                                            }
                                        }
                                    }
                                    //---------------------------------------------------------------------------------
                                    OptStartMgr.SetOptStartFlag(state, PriAirSysNum);
                                    if (CurrentTime > FanStartTime && CurrentTime < PreStartTimeTmr) CycleOnFlag = false;
                                } else if (PreStartTime < CurrentTime || PreStartTimeTmr < CurrentTime) {
                                    if (OSReportVarFlag) {
                                        NumHoursBeforeOccupancy = DeltaTime;
                                        OSReportVarFlag = false;
                                    }
                                    AvailStatus = CycleOn;
                                    CycleOnFlag = true;
                                    ATGUpdateFlag1 = true;
                                    ATGUpdateFlag2 = true;
                                    OptStartMgr.SetOptStartFlag(state, PriAirSysNum);
                                } else {
                                    AvailStatus = NoAction;
                                    CycleOnFlag = false;
                                    OSReportVarFlag = true;
                                }
                            }
                        } else if (TempDiffHi <= 0.0 && TempDiffLo >= 0.0) { // not heating and not cooling
                            AvailStatus = NoAction;
                            CycleOnFlag = false;
                            TempDiffHi = 0.0;
                            TempDiffLo = 0.0;
                        } else if (TempDiffHi < 30.0) { // Cooling Mode
                            TempDiff = TempDiffHi;
                            DeltaTime = TempDiff / AdaTempGradCool;
                            if (DeltaTime > OptStartMgr.MaxOptStartTime) {
                                DeltaTime = OptStartMgr.MaxOptStartTime;
                            }
                            PreStartTime = FanStartTime - DeltaTime;
                            if (PreStartTime < 0) PreStartTime = -0.1;
                            PreStartTimeTmr = FanStartTimeTmr - DeltaTime;
                            if (PreStartTimeTmr < 0) {
                                PreStartTimeTmr += 24.0;
                                OverNightStartFlag = true;
                            } else {
                                OverNightStartFlag = false;
                            }
                            if (!OverNightStartFlag) {
                                if (FanStartTime == 0.0 || CurrentTime > FanStartTime) {
                                    AvailStatus = NoAction;
                                    CycleOnFlag = false;
                                    OSReportVarFlag = true;
                                } else if (CycleOnFlag) {
                                    AvailStatus = CycleOn;
                                    // Calculate the current day actual temperature gradient --------------------------
                                    if (!WarmupFlag) {
                                        if (ATGUpdateFlag1) {
                                            ATGUpdateTime1 = CurrentTime;
                                            ATGUpdateTemp1 = TempTstatAir(ATGWCZoneNumHi);
                                            ATGUpdateFlag1 = false;
                                        }
                                        if (TempTstatAir(ATGWCZoneNumHi) <= OccRoomTSetPointCool(ATGWCZoneNumHi) && ATGUpdateFlag2) {
                                            ATGUpdateTime2 = CurrentTime;
                                            ATGUpdateTemp2 = TempTstatAir(ATGWCZoneNumHi);
                                            ATGUpdateFlag2 = false;
                                            if (std::abs(ATGUpdateTime2 - ATGUpdateTime1) > 1.e-10) {
                                                OptStart_AdaTempGradTrdCool(NumPreDays) =
                                                    (ATGUpdateTemp1 - ATGUpdateTemp2) / (ATGUpdateTime2 - ATGUpdateTime1);
                                            } else {
                                                OptStart_AdaTempGradTrdCool(NumPreDays) = (ATGUpdateTemp1 - ATGUpdateTemp2) * NumOfTimeStepInHour;
                                            }
                                        }
                                    }
                                    //---------------------------------------------------------------------------------
                                    OptStartMgr.SetOptStartFlag(state, PriAirSysNum);
                                } else if (PreStartTime < CurrentTime) {
                                    if (OSReportVarFlag) {
                                        NumHoursBeforeOccupancy = DeltaTime;
                                        OSReportVarFlag = false;
                                    }
                                    AvailStatus = CycleOn;
                                    CycleOnFlag = true;
                                    ATGUpdateFlag1 = true;
                                    ATGUpdateFlag2 = true;
                                    OptStartMgr.SetOptStartFlag(state, PriAirSysNum);
                                } else {
                                    AvailStatus = NoAction;
                                    CycleOnFlag = false;
                                    OSReportVarFlag = true;
                                }
                            } else {
                                if (FanStartTime == 0.0 || (CurrentTime > FanStartTime && CurrentTime <= PreStartTimeTmr)) {
                                    AvailStatus = NoAction;
                                    CycleOnFlag = false;
                                    OSReportVarFlag = true;
                                } else if (CycleOnFlag) {
                                    AvailStatus = CycleOn;
                                    // Calculate the current day actual temperature gradient --------------------------
                                    if (!WarmupFlag) {
                                        if (ATGUpdateFlag1) {
                                            ATGUpdateTime1 = CurrentTime;
                                            ATGUpdateTemp1 = TempTstatAir(ATGWCZoneNumHi);
                                            ATGUpdateFlag1 = false;
                                        }
                                        if (TempTstatAir(ATGWCZoneNumHi) <= OccRoomTSetPointCool(ATGWCZoneNumHi) && ATGUpdateFlag2) {
                                            ATGUpdateTime2 = CurrentTime;
                                            ATGUpdateTemp2 = TempTstatAir(ATGWCZoneNumHi);
                                            ATGUpdateFlag2 = false;
                                            if (std::abs(ATGUpdateTime2 - ATGUpdateTime1 + 24.0) > 1.e-10) {
                                                OptStart_AdaTempGradTrdCool(NumPreDays) =
                                                    (ATGUpdateTemp1 - ATGUpdateTemp2) / (ATGUpdateTime2 - ATGUpdateTime1 + 24.0);
                                            } else {
                                                OptStart_AdaTempGradTrdCool(NumPreDays) = (ATGUpdateTemp1 - ATGUpdateTemp2) * NumOfTimeStepInHour;
                                            }
                                        }
                                    }
                                    //---------------------------------------------------------------------------------
                                    OptStartMgr.SetOptStartFlag(state, PriAirSysNum);
                                } else if (PreStartTime < CurrentTime || PreStartTimeTmr < CurrentTime) {
                                    if (OSReportVarFlag) {
                                        NumHoursBeforeOccupancy = DeltaTime;
                                        OSReportVarFlag = false;
                                    }
                                    AvailStatus = CycleOn;
                                    CycleOnFlag = true;
                                    ATGUpdateFlag2 = true;
                                    ATGUpdateFlag1 = true;
                                    OptStartMgr.SetOptStartFlag(state, PriAirSysNum);
                                } else {
                                    AvailStatus = NoAction;
                                    CycleOnFlag = false;
                                    OSReportVarFlag = true;
                                }
                            }
                        } else {
                            AvailStatus = NoAction;
                            CycleOnFlag = false;
                        }
                    } else {
                        AvailStatus = NoAction;
                    }

                } else if (SELECT_CASE_var == AdaptiveASHRAE) {
                    AvailStatus = NoAction;
                }
            }
        }

        OptStartMgr.AvailStatus = AvailStatus;
        OptStartMgr.NumHoursBeforeOccupancy = NumHoursBeforeOccupancy;
        OptStartMgr.TempDiffLo = TempDiffLo;
        OptStartMgr.TempDiffHi = TempDiffHi;
        OptStartMgr.ATGWCZoneNumLo = ATGWCZoneNumLo;
        OptStartMgr.ATGWCZoneNumHi = ATGWCZoneNumHi;
        OptStartMgr.CycleOnFlag = CycleOnFlag;
        OptStartMgr.ATGUpdateFlag1 = ATGUpdateFlag1;
        OptStartMgr.ATGUpdateFlag2 = ATGUpdateFlag2;
        OptStartMgr.FirstTimeATGFlag = FirstTimeATGFlag;
        OptStartMgr.OverNightStartFlag = OverNightStartFlag;
        OptStartMgr.OSReportVarFlag = OSReportVarFlag;
        if (OptStartMgr.CtrlAlgType == AdaptiveTemperatureGradient) {
            OptStartMgr.AdaTempGradTrdHeat = OptStart_AdaTempGradTrdHeat;
            OptStartMgr.AdaTempGradTrdCool = OptStart_AdaTempGradTrdCool;
            OptStartMgr.AdaTempGradHeat = AdaTempGradHeat;
            OptStartMgr.AdaTempGradCool = AdaTempGradCool;
            OptStartMgr.ATGUpdateTime1 = ATGUpdateTime1;
            OptStartMgr.ATGUpdateTime2 = ATGUpdateTime2;
            OptStartMgr.ATGUpdateTemp1 = ATGUpdateTemp1;
            OptStartMgr.ATGUpdateTemp2 = ATGUpdateTemp2;
        }
    }

    void DefineOptStartSysAvailManager::SetOptStartFlag(EnergyPlusData &state, int const AirLoopNum)
    {
        // Set the OptStartFlag true for all zones on the air loop
        auto const &thisAirToZoneNodeInfo(state.dataAirLoop->AirToZoneNodeInfo(AirLoopNum));
        for (int counter = 1; counter <= thisAirToZoneNodeInfo.NumZonesCooled; ++counter) {
            int actZoneNum = DataZoneEquipment::ZoneEquipConfig(thisAirToZoneNodeInfo.CoolCtrlZoneNums(counter)).ActualZoneNum;
            OptStartData.OptStartFlag(actZoneNum) = true;
        }
        for (int counter = 1; counter <= thisAirToZoneNodeInfo.NumZonesHeated; ++counter) {
            int actZoneNum = DataZoneEquipment::ZoneEquipConfig(thisAirToZoneNodeInfo.HeatCtrlZoneNums(counter)).ActualZoneNum;
            OptStartData.OptStartFlag(actZoneNum) = true;
        }
    }
    void CalcNVentSysAvailMgr(EnergyPlusData &state,
                              int const SysAvailNum,           // number of the current scheduled system availability manager
                              int const PriAirSysNum,          // number of the primary air system affected by this Avail. Manager
                              int &AvailStatus,                // System status indicator
                              Optional_int_const ZoneEquipType // Type of zone equipment component
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Fred Buhl
        //       DATE WRITTEN   December 2004
        //       MODIFIED       March 2011, Chandan Sharma - FSEC: Allowed night ventilation
        //                             availability manager to work for zone component
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Set AvailStatus indicator for a primary air loop and ZoneHVAC component and sets a specified flow
        // rate fraction for the air loop for use during night ventilation.

        // METHODOLOGY EMPLOYED:
        // Looks at outside and indoor conditions to determine if night ventilation
        // is beneficial. If it is and it is scheduled on the AvailStatus is set to cycle
        // on and the loop flow rate fractionis set to the specified night ventilation
        // value.

        // REFERENCES:
        // na

        // Using/Aliasing
        using namespace DataAirLoop;
        using DataEnvironment::OutDryBulbTemp;
        using DataHeatBalFanSys::TempControlType;
        using DataHeatBalFanSys::TempTstatAir;
        using DataHeatBalFanSys::TempZoneThermostatSetPoint;
        using DataHeatBalFanSys::ZoneThermostatSetPointHi;
        using DataHeatBalFanSys::ZoneThermostatSetPointLo;
        using DataZoneEquipment::ZoneEquipConfig;

        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:
        // SUBROUTINE PARAMETER DEFINITIONS:

        // INTERFACE BLOCK SPECIFICATIONS:

        // DERIVED TYPE DEFINITIONS:
        // na

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int ZoneInSysNum;
        int CtrldZoneNum;
        int ZoneNum;
        bool TempCheck;     // TRUE if one zone's temperature is above the value of the vent temp sched
        bool DelTCheck;     // TRUE if the control zone temperature - outside temperature > VentDelT
        bool LowLimCheck;   // TRUE if one zones's air temperature is below this value
        Real64 VentTemp;    // value of the ventilation temperature schedule
        int ControlZoneNum; // actual zone number of the control zone

        TempCheck = false;
        DelTCheck = false;
        LowLimCheck = false;
        // check if night venting allowed: not allowed if avail sched is off or fan sched is on
        // CR 7913 changed to allow during warmup
        if ((GetCurrentScheduleValue(NVentSysAvailMgrData(SysAvailNum).SchedPtr) <= 0.0) ||
            (GetCurrentScheduleValue(NVentSysAvailMgrData(SysAvailNum).FanSchedPtr) > 0.0)) {
            AvailStatus = NoAction;
        } else {

            VentTemp = GetCurrentScheduleValue(NVentSysAvailMgrData(SysAvailNum).VentTempSchedPtr);
            ControlZoneNum = NVentSysAvailMgrData(SysAvailNum).ZoneNum;

            if (present(ZoneEquipType)) {
                // if the room temperature is greater than the vent temp sched value, set the vent temp check to TRUE
                if (TempTstatAir(ControlZoneNum) > VentTemp) {
                    TempCheck = true;
                }
                // if the room temperature is less than the low limit set the low limit check to TRUE
                if (TempTstatAir(ControlZoneNum) < NVentSysAvailMgrData(SysAvailNum).VentTempLowLim) {
                    LowLimCheck = true;
                }
            } else {
                for (ZoneInSysNum = 1; ZoneInSysNum <= state.dataAirLoop->AirToZoneNodeInfo(PriAirSysNum).NumZonesCooled; ++ZoneInSysNum) { // loop over zones in system

                    CtrldZoneNum = state.dataAirLoop->AirToZoneNodeInfo(PriAirSysNum).CoolCtrlZoneNums(ZoneInSysNum);
                    ZoneNum = ZoneEquipConfig(CtrldZoneNum).ActualZoneNum;
                    // if the room temperature is greater than the vent temp sched value, set the vent temp check to TRUE
                    if (TempTstatAir(ZoneNum) > VentTemp) {
                        TempCheck = true;
                    }
                    // if the room temperature is less than the low limit set the low limit check to TRUE
                    if (TempTstatAir(ZoneNum) < NVentSysAvailMgrData(SysAvailNum).VentTempLowLim) {
                        LowLimCheck = true;
                    }
                }
            }
            // If the difference between the control zone temperature and the outside temperature is greater than
            // the specified night venting delta T then set the delta T check to TRUE
            if ((TempTstatAir(ControlZoneNum) - OutDryBulbTemp) > NVentSysAvailMgrData(SysAvailNum).VentDelT) {
                DelTCheck = true;
            }
            // If the limit requirements are met turn on night ventilation
            if (TempCheck && DelTCheck && !LowLimCheck) {
                AvailStatus = CycleOn;
            } else {
                AvailStatus = NoAction;
            }
        }

        if (!present(ZoneEquipType)) {
            if (AvailStatus == CycleOn) {
                state.dataAirLoop->AirLoopControlInfo(PriAirSysNum).LoopFlowRateSet = true;
                state.dataAirLoop->AirLoopControlInfo(PriAirSysNum).NightVent = true;
                state.dataAirLoop->AirLoopFlow(PriAirSysNum).ReqSupplyFrac = NVentSysAvailMgrData(SysAvailNum).VentFlowFrac;
            }
        }

        NVentSysAvailMgrData(SysAvailNum).AvailStatus = AvailStatus;
    }

    void CalcDiffTSysAvailMgr(int const SysAvailNum,    // Number of the current scheduled system availability manager
                              int const PreviousStatus, // System status for the previous timestep
                              int &AvailStatus          // System status indicator
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Peter Graham Ellis
        //       DATE WRITTEN   February 2004
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Set AvailStatus indicator for a plant loop, primary air loop or ZoneHVAC component.

        // METHODOLOGY EMPLOYED:

        // Using/Aliasing
        using DataLoopNode::Node;
        using DataPlant::PlantAvailMgr;

        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 DeltaTemp;

        // FLOW:
        DeltaTemp = Node(DiffTSysAvailMgrData(SysAvailNum).HotNode).Temp - Node(DiffTSysAvailMgrData(SysAvailNum).ColdNode).Temp;

        if (DeltaTemp >= DiffTSysAvailMgrData(SysAvailNum).TempDiffOn) {
            AvailStatus = CycleOn;
        } else if (DeltaTemp <= DiffTSysAvailMgrData(SysAvailNum).TempDiffOff) {
            AvailStatus = ForceOff;
        } else {

            if (PreviousStatus == NoAction) {
                AvailStatus = ForceOff;
            } else {
                AvailStatus = PreviousStatus; // No change, but not "NoAction"; it should always be on or off.
            }
        }

        DiffTSysAvailMgrData(SysAvailNum).AvailStatus = AvailStatus;
    }

    void CalcHiTurnOffSysAvailMgr(int const SysAvailNum, // Number of the current scheduled system availability manager
                                  int &AvailStatus       // System status indicator
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Peter Graham Ellis
        //       DATE WRITTEN   February 2004
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Set AvailStatus indicator for a plant loop, primary air loop or ZoneHVAC component.

        // METHODOLOGY EMPLOYED:

        // Using/Aliasing
        using DataLoopNode::Node;

        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:

        // FLOW:
        if (Node(HiTurnOffSysAvailMgrData(SysAvailNum).Node).Temp >= HiTurnOffSysAvailMgrData(SysAvailNum).Temp) {
            AvailStatus = ForceOff;
        } else {
            AvailStatus = NoAction;
        }

        HiTurnOffSysAvailMgrData(SysAvailNum).AvailStatus = AvailStatus;
    }

    void CalcHiTurnOnSysAvailMgr(int const SysAvailNum, // Number of the current scheduled system availability manager
                                 int &AvailStatus       // System status indicator
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Peter Graham Ellis
        //       DATE WRITTEN   February 2004
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Set AvailStatus indicator for a plant loop, primary air loop or ZoneHVAC component.

        // METHODOLOGY EMPLOYED:

        // Using/Aliasing
        using DataLoopNode::Node;

        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:

        // FLOW:
        if (Node(HiTurnOnSysAvailMgrData(SysAvailNum).Node).Temp >= HiTurnOnSysAvailMgrData(SysAvailNum).Temp) {
            AvailStatus = CycleOn;
        } else {
            AvailStatus = NoAction;
        }

        HiTurnOnSysAvailMgrData(SysAvailNum).AvailStatus = AvailStatus;
    }

    void CalcLoTurnOffSysAvailMgr(int const SysAvailNum, // Number of the current scheduled system availability manager
                                  int &AvailStatus       // System status indicator
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Peter Graham Ellis
        //       DATE WRITTEN   February 2004
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Set AvailStatus indicator for a plant loop, primary air loop or ZoneHVAC component.

        // METHODOLOGY EMPLOYED:

        // Using/Aliasing
        using DataLoopNode::Node;

        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:

        // FLOW:

        // If applicability schedule is off, then availability manager is inactive, return no action
        if (LoTurnOffSysAvailMgrData(SysAvailNum).SchedPtr > 0) {
            if (GetCurrentScheduleValue(LoTurnOffSysAvailMgrData(SysAvailNum).SchedPtr) <= 0.0) {
                AvailStatus = NoAction;
                LoTurnOffSysAvailMgrData(SysAvailNum).AvailStatus = AvailStatus;
                return;
            }
        }

        // Availability manager is active, check temperature limit
        if (Node(LoTurnOffSysAvailMgrData(SysAvailNum).Node).Temp <= LoTurnOffSysAvailMgrData(SysAvailNum).Temp) {
            AvailStatus = ForceOff;
        } else {
            AvailStatus = NoAction;
        }

        LoTurnOffSysAvailMgrData(SysAvailNum).AvailStatus = AvailStatus;
    }

    void CalcLoTurnOnSysAvailMgr(int const SysAvailNum, // Number of the current scheduled system availability manager
                                 int &AvailStatus       // System status indicator
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Peter Graham Ellis
        //       DATE WRITTEN   February 2004
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Set AvailStatus indicator for a plant loop, primary air loop or ZoneHVAC component.

        // METHODOLOGY EMPLOYED:

        // Using/Aliasing
        using DataLoopNode::Node;

        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:

        // FLOW:
        if (Node(LoTurnOnSysAvailMgrData(SysAvailNum).Node).Temp <= LoTurnOnSysAvailMgrData(SysAvailNum).Temp) {
            AvailStatus = CycleOn;
        } else {
            AvailStatus = NoAction;
        }

        LoTurnOnSysAvailMgrData(SysAvailNum).AvailStatus = AvailStatus;
    }

    int ValidateAndSetSysAvailabilityManagerType(std::string const &AvailMgrName) // name to validate
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         Linda Lawrie
        //       DATE WRITTEN   April 2005
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS FUNCTION:
        // This function returns true for a valid System Availability Manager Type
        // and false if not.

        // Return value
        int ValidType; // result of validation

        // FUNCTION LOCAL VARIABLE DECLARATIONS:
        int Found;

        Found = UtilityRoutines::FindItem(AvailMgrName, cValidSysAvailManagerTypes, NumValidSysAvailManagerTypes);
        if (Found > 0) {
            //   Hybrid ventilation must not be specified in a list
            if (ValidSysAvailManagerTypes(Found) != SysAvailMgr_HybridVent) {
                ValidType = ValidSysAvailManagerTypes(Found);
            } else {
                ValidType = 0;
            }
        } else {
            ValidType = 0;
        }

        return ValidType;
    }

    void ManageHybridVentilation(EnergyPlusData &state)
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Lixing Gu
        //       DATE WRITTEN   March 2007
        //       MODIFIED       July 2012, Chandan Sharma - FSEC: Added zone sys avail managers
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Manage the simulation of the Hybrid Ventilation Control System Availability Managers

        // METHODOLOGY EMPLOYED:
        // NA

        // REFERENCES:
        // NA

        // Using/Aliasing
        using DataZoneEquipment::ZoneEquipConfig;
        using namespace DataLoopNode;
        using namespace DataAirLoop;

        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:
        // None

        // SUBROUTINE PARAMETER DEFINITIONS:
        // na

        // INTERFACE BLOCK SPECIFICATIONS:
        // na

        // DERIVED TYPE DEFINITIONS:
        // na

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int PriAirSysNum; // Primary Air System index
        int SysAvailNum;

        if (GetHybridInputFlag) {
            GetHybridVentilationInputs(state);
            GetHybridInputFlag = false;
        }

        if (NumHybridVentSysAvailMgrs == 0) return;

        InitHybridVentSysAvailMgr();

        for (SysAvailNum = 1; SysAvailNum <= NumHybridVentSysAvailMgrs; ++SysAvailNum) {
            if (HybridVentSysAvailMgrData(SysAvailNum).HybridVentMgrConnectedToAirLoop) {
                for (PriAirSysNum = 1; PriAirSysNum <= NumPrimaryAirSys; ++PriAirSysNum) {
                    if (HybridVentSysAvailMgrData(SysAvailNum).AirLoopNum == PriAirSysNum) CalcHybridVentSysAvailMgr(state, SysAvailNum, PriAirSysNum);
                }
            } else {
                // Hybrid ventilation manager is applied to zone component
                if (HybridVentSysAvailMgrData(SysAvailNum).SimHybridVentSysAvailMgr) {
                    CalcHybridVentSysAvailMgr(state, SysAvailNum);
                }
            }
        }
    }

    void GetHybridVentilationInputs(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Lixing Gu
        //       DATE WRITTEN   March 2007
        //       MODIFIED       L. GU, 6/23/08, Added more controls, including simple airflow objects
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Obtains input data for Hybrid Ventilation Control System Availability Managers and stores it in
        // appropriate data structures.

        // METHODOLOGY EMPLOYED:
        // Uses InputProcessor "Get" routines to obtain data.

        // Using/Aliasing
        using DataHeatBalance::TotVentilation;
        using DataHeatBalance::Ventilation;
        using DataHeatBalance::Zone;
        using NodeInputManager::GetOnlySingleNode;
        using NodeInputManager::MarkNode;
        using namespace DataLoopNode;
        using General::TrimSigDigits;
        using namespace DataIPShortCuts;
        using CurveManager::CurveValue;
        using CurveManager::GetCurveIndex;
        using CurveManager::GetCurveMinMaxValues;
        using DataContaminantBalance::Contaminant;

        // SUBROUTINE PARAMETER DEFINITIONS:
        static std::string const RoutineName("GetHybridVentilationInputs: "); // include trailing blank

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int NumAlphas;                  // Number of Alphas for each GetObjectItem call
        int NumNumbers;                 // Number of Numbers for each GetObjectItem call
        int IOStatus;                   // Used in GetObjectItem
        static bool ErrorsFound(false); // Set to true if errors in input, fatal at end of routine
        int SysAvailNum;                // DO loop index for all System Availability Managers
        Real64 SchedMin;                // Minimum value specified in a schedule
        Real64 SchedMax;                // Maximum value specified in a schedule
        Real64 CurveMin;                // Minimum value specified in a curve
        Real64 CurveMax;                // Maximum value specified in a curve
        Real64 CurveVal;                // Curve value

        // Get the number of occurrences of each type of System Availability Manager
        cCurrentModuleObject = "AvailabilityManager:HybridVentilation";
        NumHybridVentSysAvailMgrs = inputProcessor->getNumObjectsFound(cCurrentModuleObject);

        if (NumHybridVentSysAvailMgrs == 0) return;

        // Allocate the data arrays
        HybridVentSysAvailMgrData.allocate(NumHybridVentSysAvailMgrs);
        HybridVentSysAvailAirLoopNum.allocate(NumHybridVentSysAvailMgrs);
        HybridVentSysAvailActualZoneNum.allocate(NumHybridVentSysAvailMgrs);
        HybridVentSysAvailVentCtrl.allocate(NumHybridVentSysAvailMgrs);
        HybridVentSysAvailANCtrlStatus.allocate(NumHybridVentSysAvailMgrs);
        HybridVentSysAvailMaster.allocate(NumHybridVentSysAvailMgrs);
        HybridVentSysAvailWindModifier.allocate(NumHybridVentSysAvailMgrs);
        HybridVentSysAvailANCtrlStatus = 0;
        HybridVentSysAvailMaster = 0;

        for (SysAvailNum = 1; SysAvailNum <= NumHybridVentSysAvailMgrs; ++SysAvailNum) {

            inputProcessor->getObjectItem(state,
                                          cCurrentModuleObject,
                                          SysAvailNum,
                                          cAlphaArgs,
                                          NumAlphas,
                                          rNumericArgs,
                                          NumNumbers,
                                          IOStatus,
                                          lNumericFieldBlanks,
                                          lAlphaFieldBlanks,
                                          cAlphaFieldNames,
                                          cNumericFieldNames);
            UtilityRoutines::IsNameEmpty(cAlphaArgs(1), cCurrentModuleObject, ErrorsFound);
            HybridVentSysAvailMgrData(SysAvailNum).Name = cAlphaArgs(1);
            HybridVentSysAvailMgrData(SysAvailNum).MgrType = SysAvailMgr_HybridVent;

            HybridVentSysAvailMgrData(SysAvailNum).AirLoopName = cAlphaArgs(2);

            if (lAlphaFieldBlanks(2)) { // Hybrid ventilation manager applied to zone
                HybridVentSysAvailMgrData(SysAvailNum).HybridVentMgrConnectedToAirLoop = false;
            }
            HybridVentSysAvailMgrData(SysAvailNum).ControlZoneName = cAlphaArgs(3);
            // Check zone number
            HybridVentSysAvailMgrData(SysAvailNum).ActualZoneNum = UtilityRoutines::FindItemInList(cAlphaArgs(3), Zone);
            if (HybridVentSysAvailMgrData(SysAvailNum).ActualZoneNum == 0) {
                ShowSevereError(RoutineName + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\" invalid");
                ShowContinueError("not found: " + cAlphaFieldNames(3) + "=\"" + cAlphaArgs(3) + "\".");
                ErrorsFound = true;
            }

            HybridVentSysAvailMgrData(SysAvailNum).ControlModeSchedPtr = GetScheduleIndex(state, cAlphaArgs(4));
            if (HybridVentSysAvailMgrData(SysAvailNum).ControlModeSchedPtr == 0) {
                ShowSevereError(RoutineName + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\" invalid");
                ShowContinueError("not found: " + cAlphaFieldNames(4) + "=\"" + cAlphaArgs(4) + "\".");
                ErrorsFound = true;
            }

            // Check schedule values
            SchedMin = GetScheduleMinValue(HybridVentSysAvailMgrData(SysAvailNum).ControlModeSchedPtr);
            SchedMax = GetScheduleMaxValue(HybridVentSysAvailMgrData(SysAvailNum).ControlModeSchedPtr);
            if (SchedMin == 0 && SchedMax == 0) {
                ShowWarningError(RoutineName + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\"");
                ShowContinueError(cAlphaFieldNames(4) + "=\"" + cAlphaArgs(4) + "\" specifies control mode 0 for all entries.");
                ShowContinueError("All zones using this " + cAlphaFieldNames(4) + " have no hybrid ventilation control.");
            }
            if (SchedMax > 7.0) {
                ShowSevereError(RoutineName + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\"");
                ShowContinueError(cAlphaFieldNames(4) + "=\"" + cAlphaArgs(4) + "\", the maximum schedule value should be 7. However, ");
                ShowContinueError("the maximum entered value in the schedule is " + TrimSigDigits(SchedMax, 1));
                ErrorsFound = true;
            }
            if (SchedMin < 0.0) {
                ShowSevereError(RoutineName + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\"");
                ShowContinueError(cAlphaFieldNames(4) + "=\"" + cAlphaArgs(4) + "the minimum schedule value should be 0. However, ");
                ShowContinueError("the minimum entered value in the schedule is " + TrimSigDigits(SchedMin, 1));
                ErrorsFound = true;
            }
            if (SchedMax == 7.0 && !Contaminant.CO2Simulation) {
                ShowSevereError(RoutineName + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\"");
                ShowContinueError(cAlphaFieldNames(4) + "=\"" + cAlphaArgs(4) +
                                  "\", When the schedule value is 7, carbon dioxide (CO2) control is requested. ");
                ShowContinueError("However, CO2 simulation is not enabled. Please use ZoneAirContaminantBalance object to simulate CO2.");
                ErrorsFound = true;
            }
            // Read use weather rain indicator
            if (UtilityRoutines::SameString(cAlphaArgs(5), "YES")) {
                HybridVentSysAvailMgrData(SysAvailNum).UseRainIndicator = true;
            } else if (UtilityRoutines::SameString(cAlphaArgs(5), "NO")) {
                HybridVentSysAvailMgrData(SysAvailNum).UseRainIndicator = false;
            } else {
                ShowSevereError(RoutineName + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\"");
                ShowContinueError("..invalid value: " + cAlphaFieldNames(5) + "=\"" + cAlphaArgs(5) + "\".");
                ShowContinueError("Valid choices are Yes or No.");
                ErrorsFound = true;
            }

            // Check max wind speed
            if (NumNumbers > 0) {
                HybridVentSysAvailMgrData(SysAvailNum).MaxWindSpeed = rNumericArgs(1);
                if (rNumericArgs(1) > 40.0 || rNumericArgs(1) < 0.0) {
                    ShowSevereError(RoutineName + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\"");
                    ShowContinueError(cNumericFieldNames(1) + " is beyond the range.");
                    ShowContinueError("The input value is " + TrimSigDigits(rNumericArgs(1), 0) + ". The allowed value must be >= 0 and <= 40 m/s");
                    ErrorsFound = true;
                }
            }

            // Read Max and Min outdoor temperature
            if (NumNumbers > 1) {
                HybridVentSysAvailMgrData(SysAvailNum).MinOutdoorTemp = rNumericArgs(2);
                if (rNumericArgs(2) > 100.0 || rNumericArgs(2) < -100.0) {
                    ShowSevereError(RoutineName + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\"");
                    ShowContinueError(cNumericFieldNames(2) + " is beyond the range.");
                    ShowContinueError("The input value is " + TrimSigDigits(rNumericArgs(2), 0) +
                                      ". The allowed value must be between -100 C and +100 C");
                    ErrorsFound = true;
                }
            }
            if (NumNumbers > 2) {
                HybridVentSysAvailMgrData(SysAvailNum).MaxOutdoorTemp = rNumericArgs(3);
                if (rNumericArgs(3) > 100.0 || rNumericArgs(3) < -100.0) {
                    ShowSevereError(RoutineName + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\"");
                    ShowContinueError(cNumericFieldNames(3) + " is beyond the range.");
                    ShowContinueError("The input value is " + TrimSigDigits(rNumericArgs(3), 0) +
                                      ". The allowed value must be between -100 C and +100 C");
                    ErrorsFound = true;
                }
            }
            // Ensure MaxTemp >= MinTemp
            if (rNumericArgs(2) >= rNumericArgs(3)) {
                ShowSevereError(RoutineName + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\" The " + cNumericFieldNames(2) +
                                " must be less than the " + cNumericFieldNames(3));
                ShowContinueError("The " + cNumericFieldNames(2) + " is " + TrimSigDigits(rNumericArgs(2), 0) + ". The " + cNumericFieldNames(3) +
                                  " is " + TrimSigDigits(rNumericArgs(3), 0) + '.');
                ErrorsFound = true;
            }

            // Read Max and Min outdoor enthalpy
            if (NumNumbers > 3) {
                HybridVentSysAvailMgrData(SysAvailNum).MinOutdoorEnth = rNumericArgs(4);
                if (rNumericArgs(4) > 300000.0 || rNumericArgs(4) < 0.0) {
                    ShowSevereError(RoutineName + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\"");
                    ShowContinueError(cNumericFieldNames(4) + " is beyond the range.");
                    ShowContinueError("The input value is " + TrimSigDigits(rNumericArgs(4), 0) +
                                      ". The allowed value must be between 0 and 300000 J/kg");
                    ErrorsFound = true;
                }
            }
            if (NumNumbers > 4) {
                HybridVentSysAvailMgrData(SysAvailNum).MaxOutdoorEnth = rNumericArgs(5);
                if (rNumericArgs(5) > 300000.0 || rNumericArgs(5) < 0.0) {
                    ShowSevereError(RoutineName + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\"");
                    ShowContinueError(cNumericFieldNames(5) + " is beyond the range.");
                    ShowContinueError("The input value is " + TrimSigDigits(rNumericArgs(5), 0) +
                                      ". The allowed value must be between 0 and 300000 J/kg");
                    ErrorsFound = true;
                }
            }
            // Ensure MaxEnth >= MiniEnth
            if (rNumericArgs(4) >= rNumericArgs(5)) {
                ShowSevereError(RoutineName + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\" The " + cNumericFieldNames(4) +
                                " must be less than the " + cNumericFieldNames(5));
                ShowContinueError("The " + cNumericFieldNames(4) + " is " + TrimSigDigits(rNumericArgs(4), 0) + ". The " + cNumericFieldNames(5) +
                                  " is " + TrimSigDigits(rNumericArgs(5), 0) + '.');
                ErrorsFound = true;
            }

            // Read Max and Min outdoor dew point
            if (NumNumbers > 5) {
                HybridVentSysAvailMgrData(SysAvailNum).MinOutdoorDewPoint = rNumericArgs(6);
                if (rNumericArgs(6) > 100.0 || rNumericArgs(6) < -100.0) {
                    ShowSevereError(RoutineName + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\"");
                    ShowContinueError(cNumericFieldNames(6) + " is beyond the range.");
                    ShowContinueError("The input value is " + TrimSigDigits(rNumericArgs(6), 0) +
                                      ". The allowed value must be between -100 C and +100 C");
                    ErrorsFound = true;
                }
            }
            if (NumNumbers > 6) {
                HybridVentSysAvailMgrData(SysAvailNum).MaxOutdoorDewPoint = rNumericArgs(7);
                if (rNumericArgs(7) > 100.0 || rNumericArgs(7) < -100.0) {
                    ShowSevereError(RoutineName + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\"");
                    ShowContinueError(cNumericFieldNames(7) + " is beyond the range.");
                    ShowContinueError("The input value is " + TrimSigDigits(rNumericArgs(7), 0) +
                                      ". The allowed value must be between -100 C and +100 C");
                    ErrorsFound = true;
                }
            }
            // Ensure MaxTemp >= MinTemp
            if (rNumericArgs(6) >= rNumericArgs(7)) {
                ShowSevereError(RoutineName + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\" The " + cNumericFieldNames(6) +
                                " must be less than the " + cNumericFieldNames(7));
                ShowContinueError("The " + cNumericFieldNames(6) + " is " + TrimSigDigits(rNumericArgs(6), 0) + ". The " + cNumericFieldNames(7) +
                                  " is " + TrimSigDigits(rNumericArgs(7), 0) + '.');
                ErrorsFound = true;
            }

            HybridVentSysAvailMgrData(SysAvailNum).MinOASched = cAlphaArgs(6);
            HybridVentSysAvailMgrData(SysAvailNum).MinOASchedPtr = GetScheduleIndex(state, cAlphaArgs(6));
            if (HybridVentSysAvailMgrData(SysAvailNum).MinOASchedPtr == 0) {
                ShowSevereError(RoutineName + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\", invalid");
                ShowContinueError("..not found: " + cAlphaFieldNames(6) + "=\"" + cAlphaArgs(6) + "\".");
                ErrorsFound = true;
            }
            SchedMin = GetScheduleMinValue(HybridVentSysAvailMgrData(SysAvailNum).MinOASchedPtr);
            if (SchedMin < 0.0) {
                ShowSevereError(RoutineName + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\", Schedule value must be >= 0 in " +
                                cAlphaFieldNames(6) + "=\"" + cAlphaArgs(6) + "\".");
                ShowContinueError("The minimum schedule value is " + TrimSigDigits(SchedMin, 1));
                ErrorsFound = true;
            }

            if (!lAlphaFieldBlanks(7)) {
                HybridVentSysAvailMgrData(SysAvailNum).OpeningFactorFWS = GetCurveIndex(state, cAlphaArgs(7));
                if (HybridVentSysAvailMgrData(SysAvailNum).OpeningFactorFWS <= 0) {
                    ShowSevereError(RoutineName + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\"");
                    ShowContinueError(" not found: " + cAlphaFieldNames(7) + "=\"" + cAlphaArgs(7) + "\".");
                    ErrorsFound = true;
                } else {
                    GetCurveMinMaxValues(state, HybridVentSysAvailMgrData(SysAvailNum).OpeningFactorFWS, CurveMin, CurveMax);
                    if (CurveMin < 0.0) {
                        ShowSevereError(RoutineName + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\"");
                        ShowContinueError("The minimum wind speed used in " + cAlphaFieldNames(7) + "=\"" + cAlphaArgs(7) +
                                          "should be greater than or equal to 0.0 (m/s)");
                        ShowContinueError("Curve minimum value appears to be less than 0.");
                        ErrorsFound = true;
                    }
                    CurveVal = CurveValue(state, HybridVentSysAvailMgrData(SysAvailNum).OpeningFactorFWS, CurveMin);
                    if (CurveVal < 0.0) {
                        ShowSevereError(RoutineName + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\"");
                        ShowContinueError("The minimum value of " + cAlphaFieldNames(7) +
                                          " must be greater than or equal to 0.0 at the minimum value of wind speed.");
                        ShowContinueError(cAlphaFieldNames(7) + "=\"" + cAlphaArgs(7) + "\".");
                        ShowContinueError("Curve output at the minimum wind speed = " + TrimSigDigits(CurveVal, 3));
                        ErrorsFound = true;
                    }
                    CurveVal = CurveValue(state, HybridVentSysAvailMgrData(SysAvailNum).OpeningFactorFWS, CurveMax);
                    if (CurveVal > 1.0) {
                        ShowSevereError(RoutineName + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\"");
                        ShowContinueError("The maximum value of " + cAlphaFieldNames(7) +
                                          " must be less than or equal to 1.0 at the maximum value of wind speed.");
                        ShowContinueError(cAlphaFieldNames(7) + "=\"" + cAlphaArgs(7) + "\".");
                        ShowContinueError("Curve output at the maximum wind speed = " + TrimSigDigits(CurveVal, 3));
                        ErrorsFound = true;
                    }
                    // Check curve type
                    ErrorsFound |= CurveManager::CheckCurveDims(state,
                        HybridVentSysAvailMgrData(SysAvailNum).OpeningFactorFWS,   // Curve index
                        {1},                            // Valid dimensions
                        RoutineName,                    // Routine name
                        cCurrentModuleObject,            // Object Type
                        HybridVentSysAvailMgrData(SysAvailNum).Name,     // Object Name
                        cAlphaFieldNames(7));    // Field Name
                }
            }

            HybridVentSysAvailMgrData(SysAvailNum).ANControlTypeSchedPtr = GetScheduleIndex(state, cAlphaArgs(8));
            if (HybridVentSysAvailMgrData(SysAvailNum).ANControlTypeSchedPtr > 0) {
                HybridVentSysAvailMaster(SysAvailNum) = HybridVentSysAvailMgrData(SysAvailNum).ActualZoneNum;
                // Check schedule values
                SchedMin = GetScheduleMinValue(HybridVentSysAvailMgrData(SysAvailNum).ANControlTypeSchedPtr);
                SchedMax = GetScheduleMaxValue(HybridVentSysAvailMgrData(SysAvailNum).ANControlTypeSchedPtr);
                HybridVentSysAvailANCtrlStatus(SysAvailNum) = HybridVentSysAvailMgrData(SysAvailNum).ANControlTypeSchedPtr;
                if (SchedMax > 1.0) {
                    ShowSevereError(RoutineName + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\"");
                    ShowContinueError(" For " + cAlphaFieldNames(8) + "=\"" + cAlphaArgs(8) + "\",");
                    ShowContinueError("the maximum schedule value should be 1. However, ");
                    ShowContinueError("the maximum entered value in the schedule is " + TrimSigDigits(SchedMax, 1));
                    ErrorsFound = true;
                }
                if (SchedMin < 0.0) {
                    ShowSevereError(RoutineName + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\"");
                    ShowContinueError("For " + cAlphaFieldNames(8) + "=\"" + cAlphaArgs(8) + "\",");
                    ShowContinueError("the minimum schedule value should be 0. However, ");
                    ShowContinueError("the minimum entered value in the schedule is " + TrimSigDigits(SchedMin, 1));
                    ErrorsFound = true;
                }
            }

            HybridVentSysAvailMgrData(SysAvailNum).SimpleControlTypeSchedPtr = GetScheduleIndex(state, cAlphaArgs(9));
            if (HybridVentSysAvailMgrData(SysAvailNum).SimpleControlTypeSchedPtr > 0 &&
                HybridVentSysAvailMgrData(SysAvailNum).ANControlTypeSchedPtr > 0) {
                ShowWarningError(RoutineName + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\"");
                ShowContinueError("The inputs for" + cAlphaFieldNames(8) + " and " + cAlphaFieldNames(9) + " are valid.");
                ShowContinueError("But both objects cannot work at the same time. The Simple Airflow Control is disabled");
                HybridVentSysAvailMgrData(SysAvailNum).SimpleControlTypeSchedPtr = 0;
            } else if (HybridVentSysAvailMgrData(SysAvailNum).SimpleControlTypeSchedPtr > 0) {
                // Check schedule values
                SchedMin = GetScheduleMinValue(HybridVentSysAvailMgrData(SysAvailNum).SimpleControlTypeSchedPtr);
                SchedMax = GetScheduleMaxValue(HybridVentSysAvailMgrData(SysAvailNum).SimpleControlTypeSchedPtr);
                if (SchedMax > 1.0) {
                    ShowSevereError(RoutineName + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\"");
                    ShowContinueError("For " + cAlphaFieldNames(9) + "=\"" + cAlphaArgs(9) + "\",");
                    ShowContinueError("the maximum schedule value should be 1. However, ");
                    ShowContinueError("the maximum entered value in the schedule is " + TrimSigDigits(SchedMax, 1));
                    ErrorsFound = true;
                }
                if (SchedMin < 0.0) {
                    ShowSevereError(RoutineName + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\"");
                    ShowContinueError("For " + cAlphaFieldNames(9) + "=\"" + cAlphaArgs(9) + "\",");
                    ShowContinueError("the minimum schedule value should be 0. However, ");
                    ShowContinueError("the minimum entered value in the schedule is " + TrimSigDigits(SchedMin, 1));
                    ErrorsFound = true;
                }
            }

            if (HybridVentSysAvailMgrData(SysAvailNum).SimpleControlTypeSchedPtr > 0) {
                HybridVentSysAvailMgrData(SysAvailNum).VentilationName = cAlphaArgs(10);
                if (TotVentilation > 0) {
                    HybridVentSysAvailMgrData(SysAvailNum).VentilationPtr = UtilityRoutines::FindItemInList(cAlphaArgs(10), Ventilation);
                    HybridVentSysAvailMaster(SysAvailNum) = HybridVentSysAvailMgrData(SysAvailNum).VentilationPtr;
                    SchedMax = GetScheduleMaxValue(HybridVentSysAvailMgrData(SysAvailNum).SimpleControlTypeSchedPtr);
                    if (HybridVentSysAvailMgrData(SysAvailNum).VentilationPtr <= 0 && int(SchedMax) == 1) {
                        ShowSevereError(RoutineName + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\"");
                        ShowContinueError(cAlphaFieldNames(10) + "=\"" + cAlphaArgs(10) + "\" is required and not found.");
                        ErrorsFound = true;
                    } // Otherwise check later
                }
            }

            // Check simple airflow object
            if (HybridVentSysAvailMgrData(SysAvailNum).SimpleControlTypeSchedPtr > 0 && HybridVentSysAvailMgrData(SysAvailNum).VentilationPtr > 0) {
                if (HybridVentSysAvailMgrData(SysAvailNum).ActualZoneNum !=
                    Ventilation(HybridVentSysAvailMgrData(SysAvailNum).VentilationPtr).ZonePtr) {
                    ShowSevereError(RoutineName + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\"");
                    ShowContinueError("The Zone name specified in the Ventilation object " +
                                      Zone(Ventilation(HybridVentSysAvailMgrData(SysAvailNum).VentilationPtr).ZonePtr).Name);
                    ShowContinueError("is not equal to the " + cAlphaFieldNames(3) + "=\"" + cAlphaArgs(3) + "\".");
                    ErrorsFound = true;
                }
            }

            if (HybridVentSysAvailMgrData(SysAvailNum).SimpleControlTypeSchedPtr > 0 &&
                AirflowNetwork::SimulateAirflowNetwork > AirflowNetwork::AirflowNetworkControlSimple) {
                ShowSevereError(RoutineName + cCurrentModuleObject + "=\"" + HybridVentSysAvailMgrData(SysAvailNum).Name + "\"");
                ShowContinueError("The simple airflow objects are used for natural ventilation calculation.");
                ShowContinueError("The Airflow Network model is not allowed to perform. Please set the control type = NoMultizoneOrDistribution");
                ErrorsFound = true;
            }

            if (HybridVentSysAvailMgrData(SysAvailNum).SimpleControlTypeSchedPtr == 0) {
                if (AirflowNetwork::SimulateAirflowNetwork <= AirflowNetwork::AirflowNetworkControlSimple) {
                    ShowWarningError(RoutineName + cCurrentModuleObject + "=\"" + HybridVentSysAvailMgrData(SysAvailNum).Name + "\"");
                    ShowContinueError("The Airflow Network model is not available for Hybrid Ventilation Control.");
                } else if (AirflowNetwork::SimulateAirflowNetwork == AirflowNetwork::AirflowNetworkControlSimpleADS) {
                    ShowWarningError(RoutineName + cCurrentModuleObject + "=\"" + HybridVentSysAvailMgrData(SysAvailNum).Name + "\"");
                    ShowContinueError("Please check the AirflowNetwork Control field in the AirflowNetwork:SimulationControl object.");
                    ShowContinueError("The suggested choices are MultizoneWithDistribution or MultizoneWithoutDistribution.");
                }
            }

            // Disallow combination of simple control and OA control mode
            SchedMax = GetScheduleMaxValue(HybridVentSysAvailMgrData(SysAvailNum).ControlModeSchedPtr);
            if (HybridVentSysAvailMgrData(SysAvailNum).SimpleControlTypeSchedPtr > 0 && SchedMax == 4.0) {
                ShowSevereError(RoutineName + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\"");
                ShowContinueError("The outdoor ventilation air control type defined in " + cAlphaArgs(4) + " cannot work together with " +
                                  cAlphaFieldNames(9));
                ErrorsFound = true;
            }

            if (!lNumericFieldBlanks(8)) {
                HybridVentSysAvailMgrData(SysAvailNum).MinOperTime = rNumericArgs(8);
            }
            if (!lNumericFieldBlanks(9)) {
                HybridVentSysAvailMgrData(SysAvailNum).MinVentTime = rNumericArgs(9);
            }

        } // SysAvailNum

        if (NumHybridVentSysAvailMgrs > 1) {
            for (SysAvailNum = 2; SysAvailNum <= NumHybridVentSysAvailMgrs; ++SysAvailNum) {
                if (HybridVentSysAvailMgrData(SysAvailNum - 1).ANControlTypeSchedPtr > 0) {
                    if (HybridVentSysAvailMgrData(SysAvailNum).SimpleControlTypeSchedPtr > 0) {
                        ShowSevereError("The AirflowNetwork model is used for natural ventilation calculation in " + cCurrentModuleObject + "=\"" +
                                        HybridVentSysAvailMgrData(SysAvailNum - 1).Name + "\"");
                        ShowContinueError("The simple airflow objects are used for natural ventilation calculation in " + cCurrentModuleObject +
                                          "=\"" + HybridVentSysAvailMgrData(SysAvailNum).Name + "\"");
                        ShowContinueError("The hybrid ventilation control requires the same models to calculate natural ventilation");
                        ErrorsFound = true;
                    }
                }
                if (HybridVentSysAvailMgrData(SysAvailNum - 1).SimpleControlTypeSchedPtr > 0) {
                    if (HybridVentSysAvailMgrData(SysAvailNum).ANControlTypeSchedPtr > 0) {
                        ShowSevereError("The Airflow Network model is used for natural ventilation calculation in " + cCurrentModuleObject + "=\"" +
                                        HybridVentSysAvailMgrData(SysAvailNum).Name + "\"");
                        ShowContinueError("The simple airflow objects are used for natural ventilation calculation in " + cCurrentModuleObject +
                                          "=\"" + HybridVentSysAvailMgrData(SysAvailNum - 1).Name + "\"");
                        ShowContinueError("The hybrid ventilation control requires the same models to calculate natural ventilation");
                        ErrorsFound = true;
                    }
                }
            } // SysAvailNum
        }

        if (ErrorsFound) {
            ShowFatalError(RoutineName + "Errors found in input.  Preceding condition(s) cause termination.");
        }

        // Set up output variables
        for (SysAvailNum = 1; SysAvailNum <= NumHybridVentSysAvailMgrs; ++SysAvailNum) {
            if (HybridVentSysAvailMgrData(SysAvailNum).HybridVentMgrConnectedToAirLoop) {
                SetupOutputVariable(state, "Availability Manager Hybrid Ventilation Control Status",
                                    OutputProcessor::Unit::None,
                                    HybridVentSysAvailMgrData(SysAvailNum).VentilationCtrl,
                                    "System",
                                    "Average",
                                    HybridVentSysAvailMgrData(SysAvailNum).AirLoopName);
                SetupOutputVariable(state, "Availability Manager Hybrid Ventilation Control Mode",
                                    OutputProcessor::Unit::None,
                                    HybridVentSysAvailMgrData(SysAvailNum).ControlMode,
                                    "System",
                                    "Average",
                                    HybridVentSysAvailMgrData(SysAvailNum).AirLoopName);
            } else {
                SetupOutputVariable(state, "Availability Manager Hybrid Ventilation Control Status",
                                    OutputProcessor::Unit::None,
                                    HybridVentSysAvailMgrData(SysAvailNum).VentilationCtrl,
                                    "System",
                                    "Average",
                                    HybridVentSysAvailMgrData(SysAvailNum).ControlZoneName);
                SetupOutputVariable(state, "Availability Manager Hybrid Ventilation Control Mode",
                                    OutputProcessor::Unit::None,
                                    HybridVentSysAvailMgrData(SysAvailNum).ControlMode,
                                    "System",
                                    "Average",
                                    HybridVentSysAvailMgrData(SysAvailNum).ControlZoneName);
            }

            if (HybridVentSysAvailMgrData(SysAvailNum).MinOperTime > 0) {
                SetupOutputVariable(state, "Hybrid Ventilation Control HVAC System Operation Elapsed Time",
                                    OutputProcessor::Unit::min,
                                    HybridVentSysAvailMgrData(SysAvailNum).TimeOperDuration,
                                    "System",
                                    "Average",
                                    HybridVentSysAvailMgrData(SysAvailNum).Name);
            }

            if (HybridVentSysAvailMgrData(SysAvailNum).MinVentTime > 0) {
                SetupOutputVariable(state, "Hybrid Ventilation Control Natural Ventilation Elapsed Time",
                                    OutputProcessor::Unit::min,
                                    HybridVentSysAvailMgrData(SysAvailNum).TimeVentDuration,
                                    "System",
                                    "Average",
                                    HybridVentSysAvailMgrData(SysAvailNum).Name);
            }

            if (CheckScheduleValue(HybridVentSysAvailMgrData(SysAvailNum).ControlModeSchedPtr, HybridVentMode_OperT80) ||
                CheckScheduleValue(HybridVentSysAvailMgrData(SysAvailNum).ControlModeSchedPtr, HybridVentMode_OperT90)) {
                SetupOutputVariable(state, "Hybrid Ventilation Operative Temperature",
                                    OutputProcessor::Unit::C,
                                    HybridVentSysAvailMgrData(SysAvailNum).OperativeTemp,
                                    "System",
                                    "Average",
                                    HybridVentSysAvailMgrData(SysAvailNum).Name);
                SetupOutputVariable(state, "Hybrid Ventilation Lower Limit Operative Temperature",
                                    OutputProcessor::Unit::C,
                                    HybridVentSysAvailMgrData(SysAvailNum).minAdaTem,
                                    "System",
                                    "Average",
                                    HybridVentSysAvailMgrData(SysAvailNum).Name);
                SetupOutputVariable(state, "Hybrid Ventilation Upper Limit Operative Temperature",
                                    OutputProcessor::Unit::C,
                                    HybridVentSysAvailMgrData(SysAvailNum).maxAdaTem,
                                    "System",
                                    "Average",
                                    HybridVentSysAvailMgrData(SysAvailNum).Name);
            }

            if (CheckScheduleValue(HybridVentSysAvailMgrData(SysAvailNum).ControlModeSchedPtr, HybridVentMode_CO2)) {
                SetupOutputVariable(state, "Hybrid Ventilation CO2 Concentration",
                                    OutputProcessor::Unit::ppm,
                                    HybridVentSysAvailMgrData(SysAvailNum).CO2,
                                    "System",
                                    "Average",
                                    HybridVentSysAvailMgrData(SysAvailNum).Name);
            }
        }
    }

    void InitHybridVentSysAvailMgr()
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Lixing Gu
        //       DATE WRITTEN   March 2007
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine is for initializations of the Hybrid Ventilation Control System Availability Manager

        // METHODOLOGY EMPLOYED:
        // Uses the status flags to trigger initializations.

        // Using/Aliasing
        using DataHeatBalance::AdaptiveComfortRequested_ASH55;
        using DataHeatBalance::TotVentilation;
        using DataHeatBalance::Ventilation;
        using DataZoneEquipment::NumValidSysAvailZoneComponents;
        using DataZoneEquipment::ZoneEquipConfig;

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        static bool MyOneTimeFlag(true); // One time flag
        static bool MyEnvrnFlag(true);
        int SysAvailNum;                // DO loop index for Sys Avail Manager objects
        int ControlledZoneNum;          // Index into the ZoneEquipConfig array
        static bool ErrorsFound(false); // Set to true if errors in input, fatal at end of routine
        int AirLoopNum;                 // Air loop number
        int ControlMode;                // Hybrid control mode
        int AirLoopCount;               // Air loop name count
        Real64 SchedMax;                // Maximum value specified in a schedule
        int SysAvailIndex;              // Hybrid Ventilation Sys Avail Manager index
        int ZoneEquipType;
        int HybridVentNum;

        // One time initializations
        if (MyOneTimeFlag && allocated(ZoneEquipConfig) && allocated(PrimaryAirSystem)) {

            // Ensure the controlled zone is listed and defined in an HVAC Air Loop
            for (SysAvailNum = 1; SysAvailNum <= NumHybridVentSysAvailMgrs; ++SysAvailNum) {
                if (HybridVentSysAvailMgrData(SysAvailNum).SimpleControlTypeSchedPtr > 0 && TotVentilation > 0 &&
                    HybridVentSysAvailMgrData(SysAvailNum).VentilationPtr == 0) {
                    HybridVentSysAvailMgrData(SysAvailNum).VentilationPtr =
                        UtilityRoutines::FindItemInList(HybridVentSysAvailMgrData(SysAvailNum).VentilationName, Ventilation);
                    HybridVentSysAvailMaster(SysAvailNum) = HybridVentSysAvailMgrData(SysAvailNum).VentilationPtr;
                    SchedMax = GetScheduleMaxValue(HybridVentSysAvailMgrData(SysAvailNum).SimpleControlTypeSchedPtr);
                    if (HybridVentSysAvailMgrData(SysAvailNum).VentilationPtr <= 0 && int(SchedMax) == 1) {
                        ShowSevereError("ZoneVentilation Object Name=\"" + HybridVentSysAvailMgrData(SysAvailNum).VentilationName +
                                        "\" is required and not found.");
                        ShowContinueError("Occurs in AvailabilityManager:HybridVentilation=\"" + HybridVentSysAvailMgrData(SysAvailNum).Name + "\".");
                        ErrorsFound = true;
                    }
                }
                // Check air loop number
                for (AirLoopNum = 1; AirLoopNum <= NumPrimaryAirSys; ++AirLoopNum) { // loop over the primary air systems
                    if (UtilityRoutines::SameString(PrimaryAirSystem(AirLoopNum).Name, HybridVentSysAvailMgrData(SysAvailNum).AirLoopName)) {
                        HybridVentSysAvailMgrData(SysAvailNum).AirLoopNum = AirLoopNum;
                    }
                }
                HybridVentSysAvailAirLoopNum(SysAvailNum) = HybridVentSysAvailMgrData(SysAvailNum).AirLoopNum;
                HybridVentSysAvailActualZoneNum(SysAvailNum) = HybridVentSysAvailMgrData(SysAvailNum).ActualZoneNum;

                // set the controlled zone numbers
                for (ControlledZoneNum = 1; ControlledZoneNum <= NumOfZones; ++ControlledZoneNum) {
                    if (ZoneEquipConfig(ControlledZoneNum).ActualZoneNum == HybridVentSysAvailMgrData(SysAvailNum).ActualZoneNum) {
                        HybridVentSysAvailMgrData(SysAvailNum).ControlledZoneNum = ControlledZoneNum;
                        bool zoneFound = false;
                        if (HybridVentSysAvailMgrData(SysAvailNum).HybridVentMgrConnectedToAirLoop) {
                            if (HybridVentSysAvailMgrData(SysAvailNum).ControlledZoneNum > 0) {
                                for (int zoneInNode = 1; zoneInNode <= ZoneEquipConfig(ControlledZoneNum).NumInletNodes; ++zoneInNode) {
                                    if (ZoneEquipConfig(ControlledZoneNum).InletNodeAirLoopNum(zoneInNode) ==
                                        HybridVentSysAvailMgrData(SysAvailNum).AirLoopNum) {
                                        zoneFound = true;
                                        break;
                                    }
                                }
                                if (!zoneFound) {
                                    ShowSevereError(cValidSysAvailManagerTypes(HybridVentSysAvailMgrData(SysAvailNum).MgrType) +
                                                    ", The controlled zone =" + HybridVentSysAvailMgrData(SysAvailNum).ControlZoneName +
                                                    " is not served by this Air Loop=" + HybridVentSysAvailMgrData(SysAvailNum).AirLoopName);
                                    ErrorsFound = true;
                                }
                            }
                            break;
                        }
                    }
                    if (std::any_of(
                            HybridVentSysAvailMgrData.begin(),
                            HybridVentSysAvailMgrData.end(),
                            [](SystemAvailabilityManager::DefineHybridVentSysAvailManager const &e) { return e.HybridVentMgrConnectedToAirLoop; })) {
                        for (int zoneInNode = 1; zoneInNode <= ZoneEquipConfig(ControlledZoneNum).NumInletNodes; ++zoneInNode) {
                            if (ZoneEquipConfig(ControlledZoneNum).InletNodeAirLoopNum(zoneInNode) ==
                                    HybridVentSysAvailMgrData(SysAvailNum).AirLoopNum &&
                                HybridVentSysAvailMgrData(SysAvailNum).AirLoopNum > 0) {
                                for (HybridVentNum = 1; HybridVentNum <= NumHybridVentSysAvailMgrs; ++HybridVentNum) {
                                    if (!HybridVentSysAvailMgrData(HybridVentNum).HybridVentMgrConnectedToAirLoop && (HybridVentNum != SysAvailNum)) {
                                        if (ZoneEquipConfig(ControlledZoneNum).ActualZoneNum ==
                                                HybridVentSysAvailMgrData(HybridVentNum).ActualZoneNum &&
                                            ZoneEquipConfig(ControlledZoneNum).ActualZoneNum > 0) {
                                            ShowWarningError("AvailabilityManager:HybridVentilation = \"" +
                                                             HybridVentSysAvailMgrData(HybridVentNum).Name + "\" has the controlled zone name = \"" +
                                                             HybridVentSysAvailMgrData(HybridVentNum).ControlZoneName + "\".");
                                            ShowContinueError(
                                                "This controlled zone already has hybrid ventilation control through this air loop = \"" +
                                                HybridVentSysAvailMgrData(SysAvailNum).AirLoopName + "\".");
                                            ShowContinueError("Only AvailabilityManager:HybridVentilation = \"" +
                                                              HybridVentSysAvailMgrData(SysAvailNum).Name +
                                                              "\" will be simulated. Simulation continues...");
                                        } else {
                                            HybridVentSysAvailMgrData(HybridVentNum).SimHybridVentSysAvailMgr = true;
                                        }
                                    }
                                }
                            }
                        }
                    } else {
                        for (auto &e : HybridVentSysAvailMgrData)
                            e.SimHybridVentSysAvailMgr = true;
                    }
                }

                if (HybridVentSysAvailMgrData(SysAvailNum).ControlledZoneNum == 0) {
                    ShowSevereError(cValidSysAvailManagerTypes(HybridVentSysAvailMgrData(SysAvailNum).MgrType) +
                                    ", The controlled zone is not defined correctly =" + HybridVentSysAvailMgrData(SysAvailNum).ControlZoneName);
                    ErrorsFound = true;
                }
                // check schedule value for adaptive temperature control
                if (CheckScheduleValue(HybridVentSysAvailMgrData(SysAvailNum).ControlModeSchedPtr, 5.0) ||
                    CheckScheduleValue(HybridVentSysAvailMgrData(SysAvailNum).ControlModeSchedPtr, 6.0)) {
                    if (!AdaptiveComfortRequested_ASH55) {
                        ShowSevereError("GetHybridVentilationInputs: AvailabilityManager:HybridVentilation =\"" +
                                        HybridVentSysAvailMgrData(SysAvailNum).Name + "\"");
                        ShowContinueError("Ventilation Control Mode Schedule Name =\"" +
                                          Schedule(HybridVentSysAvailMgrData(SysAvailNum).ControlModeSchedPtr).Name +
                                          "\", When the schedule value is 5 or 6, operative temperature control is requested. ");
                        ShowContinueError("However, AdaptiveASH55 is not entered in the Thermal Comfort Model Type fields in the People object.");
                        ErrorsFound = true;
                    }
                }
            }

            // Ensure an airloop name is not used more than once in the hybrid ventilation control objects
            for (AirLoopNum = 1; AirLoopNum <= NumPrimaryAirSys; ++AirLoopNum) { // loop over the primary air systems
                AirLoopCount = 0;
                for (SysAvailNum = 1; SysAvailNum <= NumHybridVentSysAvailMgrs; ++SysAvailNum) {
                    if (UtilityRoutines::SameString(PrimaryAirSystem(AirLoopNum).Name, HybridVentSysAvailMgrData(SysAvailNum).AirLoopName)) {
                        ++AirLoopCount;
                        if (AirLoopCount > 1) SysAvailIndex = SysAvailNum;
                    }
                }
                if (AirLoopCount > 1) {
                    ShowSevereError(cValidSysAvailManagerTypes(HybridVentSysAvailMgrData(SysAvailIndex).MgrType) +
                                    ", The AirLoopHVAC name found more than once=" + PrimaryAirSystem(AirLoopNum).Name);
                    ShowContinueError("Each AirLoopHVAC allows one hybrid ventilation control object.");
                    ErrorsFound = true;
                }
            }

            if (ErrorsFound) {
                ShowFatalError("Errors found in getting AvailabilityManager:* inputs");
            }

            MyOneTimeFlag = false;

        } // end 1 time initializations

        for (SysAvailNum = 1; SysAvailNum <= NumHybridVentSysAvailMgrs; ++SysAvailNum) {
            ControlMode = GetCurrentScheduleValue(HybridVentSysAvailMgrData(SysAvailNum).ControlModeSchedPtr);
            HybridVentSysAvailMgrData(SysAvailNum).ControlMode = ControlMode;
            // -1 means that the value will be determined inside CalcHybridVentSysAvailMgr.
            // IF the value is still -1, the program will stop.
            HybridVentSysAvailVentCtrl(SysAvailNum) = -1;
            HybridVentSysAvailWindModifier(SysAvailNum) = -1.0;
        }

        if (allocated(HybridVentSysAvailMgrData))
            for (auto &e : HybridVentSysAvailMgrData)
                e.AvailStatus = NoAction;

        for (ZoneEquipType = 1; ZoneEquipType <= NumValidSysAvailZoneComponents; ++ZoneEquipType) { // loop over the zone equipment types
            if (allocated(ZoneComp)) {
                if (ZoneComp(ZoneEquipType).TotalNumComp > 0)
                    for (auto &e : ZoneComp(ZoneEquipType).ZoneCompAvailMgrs)
                        e.AvailStatus = NoAction;
            }
        }

        if (BeginEnvrnFlag && MyEnvrnFlag) {
            for (SysAvailNum = 1; SysAvailNum <= NumHybridVentSysAvailMgrs; ++SysAvailNum) {
                HybridVentSysAvailMgrData(SysAvailNum).TimeVentDuration = 0.0;
                HybridVentSysAvailMgrData(SysAvailNum).TimeOperDuration = 0.0;
            }
            MyEnvrnFlag = false;
        }
        if (!BeginEnvrnFlag) {
            MyEnvrnFlag = true;
        }
        // check minimum operation time
        CurrentEndTime = CurrentTime + SysTimeElapsed;
        if (CurrentEndTime > CurrentEndTimeLast && TimeStepSys >= TimeStepSysLast) {
            for (SysAvailNum = 1; SysAvailNum <= NumHybridVentSysAvailMgrs; ++SysAvailNum) {
                if (HybridVentSysAvailMgrData(SysAvailNum).VentilationCtrl == HybridVentCtrl_NoAction) {
                    HybridVentSysAvailMgrData(SysAvailNum).TimeOperDuration = 0.0;
                    HybridVentSysAvailMgrData(SysAvailNum).TimeVentDuration = 0.0;
                }
                if (HybridVentSysAvailMgrData(SysAvailNum).MinVentTime > 0.0) {
                    if (HybridVentSysAvailMgrData(SysAvailNum).VentilationCtrl == HybridVentCtrl_Open) {
                        HybridVentSysAvailMgrData(SysAvailNum).TimeVentDuration += (CurrentEndTime - CurrentEndTimeLast) * 60.0;
                        HybridVentSysAvailMgrData(SysAvailNum).TimeOperDuration = 0.0;
                    }
                }
                if (HybridVentSysAvailMgrData(SysAvailNum).MinOperTime > 0.0) {
                    if (HybridVentSysAvailMgrData(SysAvailNum).VentilationCtrl == HybridVentCtrl_Close) {
                        HybridVentSysAvailMgrData(SysAvailNum).TimeOperDuration += (CurrentEndTime - CurrentEndTimeLast) * 60.0;
                        HybridVentSysAvailMgrData(SysAvailNum).TimeVentDuration = 0.0;
                    }
                }
            }
        }
        TimeStepSysLast = TimeStepSys;
        CurrentEndTimeLast = CurrentEndTime;
    }

    void CalcHybridVentSysAvailMgr(EnergyPlusData &state,
                                   int const SysAvailNum,          // number of the current scheduled system availability manager
                                   Optional_int_const PriAirSysNum // number of the primary air system affected by this Avail. Manager
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Lixing Gu
        //       DATE WRITTEN   March 2007
        //       MODIFIED       July 2012, Chandan Sharma - FSEC: Added zone sys avail managers
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Set AvailStatus indicator for a primary air loop and AirflowNetwork model to prevent
        // windows or doors open during HVAC system operation

        // METHODOLOGY EMPLOYED:
        // Looks at outside and indoor conditions to determine if hybrid ventilation
        // is beneficial. If it is and it is scheduled on the AvailStatus is set to cycle
        // on and open windows or doors.

        // REFERENCES:
        // na

        // Using/Aliasing
        using namespace DataAirLoop;
        using AirflowNetworkBalanceManager::GetZoneInfilAirChangeRate;
        using AirflowNetworkBalanceManager::ManageAirflowNetworkBalance;
        using CurveManager::CurveValue;
        using DataContaminantBalance::ZoneAirCO2;
        using DataContaminantBalance::ZoneCO2SetPoint;
        using DataEnvironment::IsRain;
        using DataEnvironment::OutBaroPress;
        using DataEnvironment::OutDewPointTemp;
        using DataEnvironment::OutEnthalpy;
        using DataEnvironment::OutHumRat;
        using DataHeatBalance::HybridControlTypeClose;
        using DataHeatBalance::HybridControlTypeGlobal;
        using DataHeatBalance::HybridControlTypeIndiv;
        using DataHeatBalance::Mixing;
        using DataHeatBalance::MRT;
        using DataHeatBalance::TotMixing;
        using DataHeatBalance::TotVentilation;
        using DataHeatBalance::Ventilation;
        using DataHeatBalance::Zone;
        using DataHeatBalFanSys::MAT;
        using DataHeatBalFanSys::TempControlType;
        using DataHeatBalFanSys::TempZoneThermostatSetPoint;
        using DataHeatBalFanSys::ZoneAirHumRat;
        using DataHeatBalFanSys::ZoneThermostatSetPointHi;
        using DataHeatBalFanSys::ZoneThermostatSetPointLo;
        using DataZoneControls::HumidityControlZone;
        using DataZoneControls::NumHumidityControlZones;
        using DataZoneEquipment::NumValidSysAvailZoneComponents;
        using DataZoneEquipment::ZoneEquipConfig;
        using Psychrometrics::PsyHFnTdbW;
        using Psychrometrics::PsyRhFnTdbWPb;
        using Psychrometrics::PsyTdpFnWPb;
        using Psychrometrics::PsyWFnTdbRhPb;
        using ThermalComfort::runningAverageASH;

        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:

        // SUBROUTINE PARAMETER DEFINITIONS:

        // INTERFACE BLOCK SPECIFICATIONS:

        // DERIVED TYPE DEFINITIONS:
        // na

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int ZoneNum;            // actual zone number of the control zone
        int ControlMode;        // Hybrid control mode
        int HStatZoneNum;       // Humidity control zone number
        Real64 ZoneAirEnthalpy; // Zone air enthalpy
        Real64 ZoneAirDewPoint; // Zone air dew point temperature
        Real64 ZoneAirRH;       // Zone air relative humidity
        Real64 TempExt;         // Outdoor dry bulb temperature at zone height
        Real64 WindExt;         // Outdoor wind spped at zone height
        // unused  REAL(r64)    :: RHSetPoint      ! RH setpoint from a given schedule
        Real64 WSetPoint;                   // Humidity ratio setpoint from a given RH setpoint schedule
        Real64 OASetPoint;                  // Outdoor air setpoint from a given OA setpoint schedule
        Real64 ACH;                         // Zone air change per hour
        bool found;                         // Used for humidistat object
        bool HybridVentModeOA;              // USed to check whether HybridVentModeOA is allowed
        Real64 ZoneRHHumidifyingSetPoint;   // Zone humidifying setpoint (%)
        Real64 ZoneRHDehumidifyingSetPoint; // Zone dehumidifying setpoint (%)
        int ControlledZoneNum;              // Index into the ZoneEquipConfig array
        int SimpleControlType;              // Simple control type from a schedule: 0 individual, 1 global
        int i;                              // Array index
        Real64 minAdaTem;                   // minimum adaptive temperature for adaptive temperature control
        Real64 maxAdaTem;                   // maximum adaptive temperature for adaptive temperature control
        bool KeepStatus;                    // true, if minimum time operation is needed
        int ZoneEquipType;
        int ZoneCompNum;
        int AirLoopNum;
        int Num;
        int AvailStatus;

        KeepStatus = false;
        if (HybridVentSysAvailMgrData(SysAvailNum).TimeVentDuration > 0.0 &&
            HybridVentSysAvailMgrData(SysAvailNum).TimeVentDuration <= HybridVentSysAvailMgrData(SysAvailNum).MinVentTime) {
            KeepStatus = true;
        }
        if (HybridVentSysAvailMgrData(SysAvailNum).TimeOperDuration > 0.0 &&
            HybridVentSysAvailMgrData(SysAvailNum).TimeOperDuration <= HybridVentSysAvailMgrData(SysAvailNum).MinOperTime) {
            KeepStatus = true;
        }

        ControlMode = HybridVentSysAvailMgrData(SysAvailNum).ControlMode;

        ZoneNum = HybridVentSysAvailMgrData(SysAvailNum).ActualZoneNum;
        if (!KeepStatus) HybridVentSysAvailMgrData(SysAvailNum).VentilationCtrl = HybridVentCtrl_NoAction;
        TempExt = Zone(ZoneNum).OutDryBulbTemp;
        WindExt = Zone(ZoneNum).WindSpeed;
        HybridVentSysAvailMgrData(SysAvailNum).OperativeTemp = 0.0;
        HybridVentSysAvailMgrData(SysAvailNum).minAdaTem = 0.0;
        HybridVentSysAvailMgrData(SysAvailNum).maxAdaTem = 0.0;

        if (!KeepStatus) {
            {
                auto const SELECT_CASE_var(ControlMode);

                if (SELECT_CASE_var == HybridVentMode_No) {
                    HybridVentSysAvailMgrData(SysAvailNum).VentilationCtrl = HybridVentCtrl_NoAction;

                    // Temperature control
                } else if (SELECT_CASE_var == HybridVentMode_Temp) {
                    if (TempExt >= HybridVentSysAvailMgrData(SysAvailNum).MinOutdoorTemp &&
                        TempExt <= HybridVentSysAvailMgrData(SysAvailNum).MaxOutdoorTemp) {
                        HybridVentSysAvailMgrData(SysAvailNum).VentilationCtrl = HybridVentCtrl_Open;
                    } else {
                        HybridVentSysAvailMgrData(SysAvailNum).VentilationCtrl = HybridVentCtrl_Close;
                    }

                    // Enthalpy control
                } else if (SELECT_CASE_var == HybridVentMode_Enth) {
                    ZoneAirEnthalpy = PsyHFnTdbW(MAT(ZoneNum), ZoneAirHumRat(ZoneNum));
                    if (OutEnthalpy >= HybridVentSysAvailMgrData(SysAvailNum).MinOutdoorEnth &&
                        OutEnthalpy <= HybridVentSysAvailMgrData(SysAvailNum).MaxOutdoorEnth) {
                        HybridVentSysAvailMgrData(SysAvailNum).VentilationCtrl = HybridVentCtrl_Open;
                    } else {
                        HybridVentSysAvailMgrData(SysAvailNum).VentilationCtrl = HybridVentCtrl_Close;
                    }

                    // Dew point control
                } else if (SELECT_CASE_var == HybridVentMode_DewPoint) {
                    if (OutDewPointTemp >= HybridVentSysAvailMgrData(SysAvailNum).MinOutdoorDewPoint &&
                        OutDewPointTemp <= HybridVentSysAvailMgrData(SysAvailNum).MaxOutdoorDewPoint) {
                        HybridVentSysAvailMgrData(SysAvailNum).VentilationCtrl = HybridVentCtrl_Open;
                    } else {
                        HybridVentSysAvailMgrData(SysAvailNum).VentilationCtrl = HybridVentCtrl_Close;
                    }

                } else if (SELECT_CASE_var == HybridVentMode_OA) {
                    OASetPoint = GetCurrentScheduleValue(HybridVentSysAvailMgrData(SysAvailNum).MinOASchedPtr);
                    ACH = 0.0;
                    HybridVentModeOA = true;
                    if (!HybridVentSysAvailMgrData(SysAvailNum).HybridVentMgrConnectedToAirLoop) {
                        if (AirflowNetwork::SimulateAirflowNetwork <= AirflowNetwork::AirflowNetworkControlSimple) {
                            HybridVentModeOA = false;
                        }
                    }

                    if (HybridVentSysAvailMgrData(SysAvailNum).ANControlTypeSchedPtr > 0 && HybridVentModeOA) {
                        ManageAirflowNetworkBalance(state, true);
                        ACH = GetZoneInfilAirChangeRate(state, ZoneNum);
                    }
                    if (ACH > OASetPoint) {
                        HybridVentSysAvailMgrData(SysAvailNum).VentilationCtrl = HybridVentCtrl_Open;
                    } else {
                        HybridVentSysAvailMgrData(SysAvailNum).VentilationCtrl = HybridVentCtrl_Close;
                    }

                } else if (SELECT_CASE_var == HybridVentMode_OperT80) {
                    if (runningAverageASH >= 10.0 && runningAverageASH <= 33.5) {
                        HybridVentSysAvailMgrData(SysAvailNum).OperativeTemp = 0.5 * (MAT(ZoneNum) + MRT(ZoneNum));
                        minAdaTem = 0.31 * runningAverageASH + 14.3;
                        maxAdaTem = 0.31 * runningAverageASH + 21.3;
                        HybridVentSysAvailMgrData(SysAvailNum).minAdaTem = minAdaTem;
                        HybridVentSysAvailMgrData(SysAvailNum).maxAdaTem = maxAdaTem;
                        if (HybridVentSysAvailMgrData(SysAvailNum).OperativeTemp <= maxAdaTem &&
                            HybridVentSysAvailMgrData(SysAvailNum).OperativeTemp >= minAdaTem) {
                            HybridVentSysAvailMgrData(SysAvailNum).VentilationCtrl = HybridVentCtrl_Open;
                        } else {
                            HybridVentSysAvailMgrData(SysAvailNum).VentilationCtrl = HybridVentCtrl_Close;
                        }
                    } else {
                        HybridVentSysAvailMgrData(SysAvailNum).VentilationCtrl = HybridVentCtrl_Close;
                    }

                } else if (SELECT_CASE_var == HybridVentMode_OperT90) {
                    if (runningAverageASH >= 10.0 && runningAverageASH <= 33.5) {
                        HybridVentSysAvailMgrData(SysAvailNum).OperativeTemp = 0.5 * (MAT(ZoneNum) + MRT(ZoneNum));
                        minAdaTem = 0.31 * runningAverageASH + 15.3;
                        maxAdaTem = 0.31 * runningAverageASH + 20.3;
                        HybridVentSysAvailMgrData(SysAvailNum).minAdaTem = minAdaTem;
                        HybridVentSysAvailMgrData(SysAvailNum).maxAdaTem = maxAdaTem;
                        if (HybridVentSysAvailMgrData(SysAvailNum).OperativeTemp <= maxAdaTem &&
                            HybridVentSysAvailMgrData(SysAvailNum).OperativeTemp >= minAdaTem) {
                            HybridVentSysAvailMgrData(SysAvailNum).VentilationCtrl = HybridVentCtrl_Open;
                        } else {
                            HybridVentSysAvailMgrData(SysAvailNum).VentilationCtrl = HybridVentCtrl_Close;
                        }
                    } else {
                        HybridVentSysAvailMgrData(SysAvailNum).VentilationCtrl = HybridVentCtrl_Close;
                    }

                } else if (SELECT_CASE_var == HybridVentMode_CO2) {
                    HybridVentSysAvailMgrData(SysAvailNum).CO2 = ZoneAirCO2(ZoneNum);
                    if (ZoneAirCO2(ZoneNum) > ZoneCO2SetPoint(ZoneNum)) {
                        if (HybridVentSysAvailMgrData(SysAvailNum).HybridVentMgrConnectedToAirLoop) {
                            AirLoopNum = HybridVentSysAvailMgrData(SysAvailNum).AirLoopNum;
                            for (Num = 1; Num <= state.dataAirLoop->PriAirSysAvailMgr(HybridVentSysAvailMgrData(SysAvailNum).AirLoopNum).NumAvailManagers; ++Num) {
                                SimSysAvailManager(state,
                                                   state.dataAirLoop->PriAirSysAvailMgr(AirLoopNum).AvailManagerType(Num),
                                                   state.dataAirLoop->PriAirSysAvailMgr(AirLoopNum).AvailManagerName(Num),
                                                   state.dataAirLoop->PriAirSysAvailMgr(AirLoopNum).AvailManagerNum(Num),
                                                   AirLoopNum,
                                                   state.dataAirLoop->PriAirSysAvailMgr(AirLoopNum).AvailStatus,
                                                   AvailStatus);
                            }
                            if (AvailStatus == CycleOn) {
                                HybridVentSysAvailMgrData(SysAvailNum).VentilationCtrl = HybridVentCtrl_Close;
                            } else {
                                HybridVentSysAvailMgrData(SysAvailNum).VentilationCtrl = HybridVentCtrl_Open;
                            }
                        } else if (HybridVentSysAvailMgrData(SysAvailNum).SimHybridVentSysAvailMgr) {
                            HybridVentSysAvailMgrData(SysAvailNum).VentilationCtrl = HybridVentCtrl_Open;
                            for (ZoneEquipType = 1; ZoneEquipType <= NumValidSysAvailZoneComponents; ++ZoneEquipType) {
                                for (ZoneCompNum = 1; ZoneCompNum <= ZoneComp(ZoneEquipType).TotalNumComp; ++ZoneCompNum) {
                                    if (ZoneComp(ZoneEquipType).ZoneCompAvailMgrs(ZoneCompNum).AvailStatus == CycleOn) {
                                        HybridVentSysAvailMgrData(SysAvailNum).VentilationCtrl = HybridVentCtrl_Close;
                                        break;
                                    }
                                }
                            }
                        } else {
                            HybridVentSysAvailMgrData(SysAvailNum).VentilationCtrl = HybridVentCtrl_Open;
                        }
                    }
                } else {
                    ShowSevereError(cValidSysAvailManagerTypes(HybridVentSysAvailMgrData(SysAvailNum).MgrType) +
                                    ": incorrect Control Type: " + HybridVentSysAvailMgrData(SysAvailNum).AirLoopName);
                    ShowFatalError("Errors found in getting " + cValidSysAvailManagerTypes(HybridVentSysAvailMgrData(SysAvailNum).MgrType) +
                                   " Control mode value");
                }
            }

            if (HybridVentSysAvailMgrData(SysAvailNum).VentilationCtrl == HybridVentCtrl_Open) {

                // Temperature and enthalpy control
                if (HybridVentSysAvailMgrData(SysAvailNum).ControlMode == HybridVentMode_Temp ||
                    HybridVentSysAvailMgrData(SysAvailNum).ControlMode == HybridVentMode_Enth) {

                    {
                        auto const SELECT_CASE_var(TempControlType(ZoneNum)); // select on thermostat control

                        if (SELECT_CASE_var == SingleHeatingSetPoint) {
                            if (MAT(ZoneNum) < TempZoneThermostatSetPoint(ZoneNum)) {
                                HybridVentSysAvailMgrData(SysAvailNum).VentilationCtrl = HybridVentCtrl_Close;
                            }

                        } else if (SELECT_CASE_var == SingleCoolingSetPoint) {
                            if (MAT(ZoneNum) > TempZoneThermostatSetPoint(ZoneNum)) {
                                HybridVentSysAvailMgrData(SysAvailNum).VentilationCtrl = HybridVentCtrl_Close;
                            }

                        } else if (SELECT_CASE_var == SingleHeatCoolSetPoint) {
                            HybridVentSysAvailMgrData(SysAvailNum).VentilationCtrl = HybridVentCtrl_Close;
                            ++HybridVentSysAvailMgrData(SysAvailNum).SingleHCErrCount;
                            if (HybridVentSysAvailMgrData(SysAvailNum).SingleHCErrCount < 2) {
                                ShowWarningError("Hybrid ventilation control: " + HybridVentSysAvailMgrData(SysAvailNum).AirLoopName +
                                                 ": The zone temperature control type is ThermostatSetpoint:SingleHeatingOrCooling. Natural "
                                                 "ventilation is not allowed.");
                                ShowContinueErrorTimeStamp("");
                            } else {
                                ShowRecurringWarningErrorAtEnd(
                                    "Hybrid ventilation control: " + HybridVentSysAvailMgrData(SysAvailNum).AirLoopName +
                                        ": No natural ventilation continues with a ThermostatSetpoint:SingleHeatingOrCooling type...",
                                    HybridVentSysAvailMgrData(SysAvailNum).SingleHCErrIndex,
                                    double(HybridVentSysAvailMgrData(SysAvailNum).ControlMode),
                                    double(HybridVentSysAvailMgrData(SysAvailNum).ControlMode));
                            }

                        } else if (SELECT_CASE_var == DualSetPointWithDeadBand) {
                            if ((MAT(ZoneNum) < ZoneThermostatSetPointLo(ZoneNum)) || (MAT(ZoneNum) > ZoneThermostatSetPointHi(ZoneNum))) {
                                HybridVentSysAvailMgrData(SysAvailNum).VentilationCtrl = HybridVentCtrl_Close;
                            }

                        } else {
                        }
                    } // end select on thermostat control
                }

                // Dew point control mode
                if (HybridVentSysAvailMgrData(SysAvailNum).ControlMode == HybridVentMode_DewPoint) {
                    ZoneAirRH = PsyRhFnTdbWPb(MAT(ZoneNum), ZoneAirHumRat(ZoneNum), OutBaroPress) * 100.0;
                    ZoneAirDewPoint = PsyTdpFnWPb(ZoneAirHumRat(ZoneNum), OutBaroPress);
                    if (NumHumidityControlZones == 0) {
                        ++HybridVentSysAvailMgrData(SysAvailNum).DewPointNoRHErrCount;
                        if (HybridVentSysAvailMgrData(SysAvailNum).DewPointNoRHErrCount < 2) {
                            ShowWarningError("Hybrid ventilation control: Dew point control mode is selected, but no ZoneControl:Humidistat object=" +
                                             HybridVentSysAvailMgrData(SysAvailNum).AirLoopName);
                            ShowContinueError("The hybrid ventilation control is triggered by outdoor min and max dewpoint only.");
                            ShowContinueError("HVAC system may turn off when outdoor dewpoint is between min and max dewpoint.");
                            ShowContinueErrorTimeStamp("");
                        } else {
                            ShowRecurringWarningErrorAtEnd("Hybrid ventilation control: " + HybridVentSysAvailMgrData(SysAvailNum).AirLoopName +
                                                               ": no ZoneControl:Humidistat object continues...",
                                                           HybridVentSysAvailMgrData(SysAvailNum).DewPointNoRHErrIndex,
                                                           double(HybridVentSysAvailMgrData(SysAvailNum).ControlMode),
                                                           double(HybridVentSysAvailMgrData(SysAvailNum).ControlMode));
                        }
                    }
                    found = false;
                    for (HStatZoneNum = 1; HStatZoneNum <= NumHumidityControlZones; ++HStatZoneNum) {
                        if (HumidityControlZone(HStatZoneNum).ActualZoneNum == ZoneNum) {
                            found = true;
                            ZoneRHHumidifyingSetPoint = GetCurrentScheduleValue(HumidityControlZone(HStatZoneNum).HumidifyingSchedIndex);
                            ZoneRHDehumidifyingSetPoint = GetCurrentScheduleValue(HumidityControlZone(HStatZoneNum).DehumidifyingSchedIndex);
                            if (ZoneAirRH > ZoneRHDehumidifyingSetPoint) { // Need dehumidification
                                WSetPoint = PsyWFnTdbRhPb(MAT(ZoneNum), (ZoneRHDehumidifyingSetPoint / 100.0), OutBaroPress);
                                if (WSetPoint < OutHumRat) HybridVentSysAvailMgrData(SysAvailNum).VentilationCtrl = HybridVentCtrl_Close;
                            } else if (ZoneAirRH < ZoneRHHumidifyingSetPoint) { // Need humidification
                                WSetPoint = PsyWFnTdbRhPb(MAT(ZoneNum), (ZoneRHHumidifyingSetPoint / 100.0), OutBaroPress);
                                if (WSetPoint > OutHumRat) HybridVentSysAvailMgrData(SysAvailNum).VentilationCtrl = HybridVentCtrl_Close;
                            } else {
                                HybridVentSysAvailMgrData(SysAvailNum).VentilationCtrl = HybridVentCtrl_Close;
                            }
                        }
                    }
                    if (!found && NumHumidityControlZones > 0) {
                        ++HybridVentSysAvailMgrData(SysAvailNum).DewPointErrCount;
                        if (HybridVentSysAvailMgrData(SysAvailNum).DewPointErrCount < 2) {
                            ShowWarningError("Hybrid ventilation control: The zone for dew point control mode is different from the zone for "
                                             "ZoneControl:Humidistat=" +
                                             HybridVentSysAvailMgrData(SysAvailNum).AirLoopName);
                            ShowContinueError("The Zone name for hybrid control is " + Zone(ZoneNum).Name + ". Humidistat has no impact");
                            ShowContinueError("HVAC system may turn off when outdoor dewpoint is between min and max dewpoint.");
                            ShowContinueErrorTimeStamp("");
                        } else {
                            ShowRecurringWarningErrorAtEnd("Hybrid ventilation control: " + HybridVentSysAvailMgrData(SysAvailNum).AirLoopName +
                                                               " No humidistat control impact continues...",
                                                           HybridVentSysAvailMgrData(SysAvailNum).DewPointErrIndex,
                                                           double(HybridVentSysAvailMgrData(SysAvailNum).ControlMode),
                                                           double(HybridVentSysAvailMgrData(SysAvailNum).ControlMode));
                        }
                    }
                }

                // Outdoor ventilation air control mode
                if (HybridVentSysAvailMgrData(SysAvailNum).ControlMode == HybridVentMode_OA) {
                }
            }
        }

        if (WindExt > HybridVentSysAvailMgrData(SysAvailNum).MaxWindSpeed) {
            HybridVentSysAvailMgrData(SysAvailNum).VentilationCtrl = HybridVentCtrl_Close;
        }

        if (IsRain && HybridVentSysAvailMgrData(SysAvailNum).UseRainIndicator) {
            HybridVentSysAvailMgrData(SysAvailNum).VentilationCtrl = HybridVentCtrl_Close;
        }
        // Sent a signal to the AirflowNetwork to ensure large onpenings are close or open based on this logic
        HybridVentSysAvailVentCtrl(SysAvailNum) = HybridVentSysAvailMgrData(SysAvailNum).VentilationCtrl;
        if (HybridVentSysAvailVentCtrl(SysAvailNum) < 0) {
            // Fatal error
            ShowFatalError(
                "Hybrid ventilation control: the ventilation control status is beyond the range. Please check input of control mode schedule");
        }

        if (HybridVentSysAvailMgrData(SysAvailNum).HybridVentMgrConnectedToAirLoop) {
            if (HybridVentSysAvailMgrData(SysAvailNum).VentilationCtrl == HybridVentCtrl_Close) {
                state.dataAirLoop->PriAirSysAvailMgr(PriAirSysNum).AvailStatus = CycleOn;
            }
        }

        if (HybridVentSysAvailMgrData(SysAvailNum).VentilationCtrl == HybridVentCtrl_Open &&
            HybridVentSysAvailMgrData(SysAvailNum).ANControlTypeSchedPtr > 0 && HybridVentSysAvailMgrData(SysAvailNum).OpeningFactorFWS > 0) {
            HybridVentSysAvailWindModifier(SysAvailNum) = CurveValue(state, HybridVentSysAvailMgrData(SysAvailNum).OpeningFactorFWS, WindExt);
        }

        // Set up flags to control simple airflow objects
        if (HybridVentSysAvailMgrData(SysAvailNum).AirLoopNum > 0 && HybridVentSysAvailMgrData(SysAvailNum).SimpleControlTypeSchedPtr > 0) {
            SimpleControlType = GetCurrentScheduleValue(HybridVentSysAvailMgrData(SysAvailNum).SimpleControlTypeSchedPtr);
            for (ControlledZoneNum = 1; ControlledZoneNum <= NumOfZones; ++ControlledZoneNum) {
                for (int zoneInNode = 1; zoneInNode <= ZoneEquipConfig(ControlledZoneNum).NumInletNodes; ++zoneInNode) {
                    if (HybridVentSysAvailMgrData(SysAvailNum).AirLoopNum == ZoneEquipConfig(ControlledZoneNum).InletNodeAirLoopNum(zoneInNode)) {
                        // Setup flag for ventilation objects
                        for (i = 1; i <= TotVentilation; ++i) {
                            if (Ventilation(i).ZonePtr == ZoneEquipConfig(ControlledZoneNum).ActualZoneNum) {
                                Ventilation(i).HybridControlType = HybridControlTypeIndiv;
                                if (HybridVentSysAvailMgrData(SysAvailNum).VentilationCtrl == HybridVentCtrl_Close) {
                                    Ventilation(i).HybridControlType = HybridControlTypeClose;
                                } else {
                                    if (SimpleControlType == 1) {
                                        Ventilation(i).HybridControlType = HybridControlTypeGlobal;
                                        Ventilation(i).HybridControlMasterNum = HybridVentSysAvailMgrData(SysAvailNum).VentilationPtr;
                                    }
                                }
                            }
                        }
                        // Setup flag for Mixing objects
                        for (i = 1; i <= TotMixing; ++i) {
                            if (Mixing(i).ZonePtr == ZoneEquipConfig(ControlledZoneNum).ActualZoneNum) {
                                Mixing(i).HybridControlType = HybridControlTypeIndiv;
                                if (HybridVentSysAvailMgrData(SysAvailNum).VentilationCtrl == HybridVentCtrl_Close) {
                                    Mixing(i).HybridControlType = HybridControlTypeClose;
                                } else {
                                    if (SimpleControlType == 1) {
                                        Mixing(i).HybridControlType = HybridControlTypeGlobal;
                                        Mixing(i).HybridControlMasterNum = HybridVentSysAvailMgrData(SysAvailNum).VentilationPtr;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        } else if (HybridVentSysAvailMgrData(SysAvailNum).SimpleControlTypeSchedPtr > 0) {
            SimpleControlType = GetCurrentScheduleValue(HybridVentSysAvailMgrData(SysAvailNum).SimpleControlTypeSchedPtr);
            // Hybrid ventilation manager is applied to zone component
            // setup flag for ventilation objects
            for (i = 1; i <= TotVentilation; ++i) {
                if (Ventilation(i).ZonePtr == HybridVentSysAvailMgrData(SysAvailNum).ActualZoneNum) {
                    Ventilation(i).HybridControlType = HybridControlTypeIndiv;
                    if (HybridVentSysAvailMgrData(SysAvailNum).VentilationCtrl == HybridVentCtrl_Close) {
                        Ventilation(i).HybridControlType = HybridControlTypeClose;
                    } else {
                        if (SimpleControlType == 1) {
                            Ventilation(i).HybridControlType = HybridControlTypeGlobal;
                            Ventilation(i).HybridControlMasterNum = HybridVentSysAvailMgrData(SysAvailNum).VentilationPtr;
                        }
                    }
                }
            }
            // Setup flag for Mixing objects
            for (i = 1; i <= TotMixing; ++i) {
                if (Mixing(i).ZonePtr == HybridVentSysAvailMgrData(SysAvailNum).ActualZoneNum) {
                    Mixing(i).HybridControlType = HybridControlTypeIndiv;
                    if (HybridVentSysAvailMgrData(SysAvailNum).VentilationCtrl == HybridVentCtrl_Close) {
                        Mixing(i).HybridControlType = HybridControlTypeClose;
                    } else {
                        if (SimpleControlType == 1) {
                            Mixing(i).HybridControlType = HybridControlTypeGlobal;
                            Mixing(i).HybridControlMasterNum = HybridVentSysAvailMgrData(SysAvailNum).VentilationPtr;
                        }
                    }
                }
            }
        }
    }

    bool GetHybridVentilationControlStatus(EnergyPlusData &state, int const ZoneNum) // Index of zone
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Lixing Gu
        //       DATE WRITTEN   July 2010
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This routine was designed to find whether this zone is controlled by hybrid ventilation
        // ventilation control option.

        // METHODOLOGY EMPLOYED:
        // na

        // REFERENCES:
        // na

        // Using/Aliasing
        using General::TrimSigDigits;

        // Return value
        bool VentControl; // Set to true if ventilation control in the same zone

        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:
        int SysAvailNum; // index to system availability manager number

        // SUBROUTINE PARAMETER DEFINITIONS:
        // na

        // INTERFACE BLOCK SPECIFICATIONS:
        // na

        // DERIVED TYPE DEFINITIONS:
        // na

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:

        // Obtains inputs of hybrid ventilation objects
        if (GetHybridInputFlag) { // First time subroutine has been entered
            GetHybridVentilationInputs(state);
            GetHybridInputFlag = false;
        }

        VentControl = false;

        for (SysAvailNum = 1; SysAvailNum <= NumHybridVentSysAvailMgrs; ++SysAvailNum) {
            if (HybridVentSysAvailMgrData(SysAvailNum).ActualZoneNum == ZoneNum) {
                if (HybridVentSysAvailMgrData(SysAvailNum).SimpleControlTypeSchedPtr > 0) {
                    VentControl = true;
                }
            }
        }

        return VentControl;
    }

} // namespace SystemAvailabilityManager

} // namespace EnergyPlus
