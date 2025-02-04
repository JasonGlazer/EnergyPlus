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
#include <string>

// ObjexxFCL Headers
#include <ObjexxFCL/Array.functions.hh>
#include <ObjexxFCL/Fmath.hh>

// EnergyPlus Headers
#include <EnergyPlus/AirflowNetworkBalanceManager.hh>
#include <EnergyPlus/HVACManager.hh>
#include <AirflowNetwork/Elements.hpp>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataAirLoop.hh>
#include <EnergyPlus/DataAirSystems.hh>
#include <EnergyPlus/DataContaminantBalance.hh>
#include <EnergyPlus/DataConvergParams.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataErrorTracking.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataHeatBalFanSys.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/DataPrecisionGlobals.hh>
#include <EnergyPlus/DataReportingFlags.hh>
#include <EnergyPlus/DataSurfaces.hh>
#include <EnergyPlus/DataSystemVariables.hh>
#include <EnergyPlus/DataZoneEquipment.hh>
#include <EnergyPlus/DemandManager.hh>
#include <EnergyPlus/DisplayRoutines.hh>
#include <EnergyPlus/ElectricPowerServiceManager.hh>
#include <EnergyPlus/EMSManager.hh>
#include <EnergyPlus/Fans.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/HVACSizingSimulationManager.hh>
#include <EnergyPlus/IceThermalStorage.hh>
#include <EnergyPlus/InternalHeatGains.hh>
#include <EnergyPlus/NodeInputManager.hh>
#include <EnergyPlus/NonZoneEquipmentManager.hh>
#include <EnergyPlus/OutAirNodeManager.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/OutputReportTabular.hh>
#include <EnergyPlus/Plant/DataPlant.hh>
#include <EnergyPlus/Plant/PlantManager.hh>
#include <EnergyPlus/PlantCondLoopOperation.hh>
#include <EnergyPlus/PlantLoopHeatPumpEIR.hh>
#include <EnergyPlus/PlantUtilities.hh>
#include <EnergyPlus/PollutionModule.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/RefrigeratedCase.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/SetPointManager.hh>
#include <EnergyPlus/SimAirServingZones.hh>
#include <EnergyPlus/SizingManager.hh>
#include <EnergyPlus/SystemAvailabilityManager.hh>
#include <EnergyPlus/SystemReports.hh>
#include <EnergyPlus/UtilityRoutines.hh>
#include <EnergyPlus/WaterManager.hh>
#include <EnergyPlus/ZoneContaminantPredictorCorrector.hh>
#include <EnergyPlus/ZoneEquipmentManager.hh>
#include <EnergyPlus/ZoneTempPredictorCorrector.hh>

namespace EnergyPlus {

namespace HVACManager {

    // PURPOSE OF THIS MODULE:
    // This module contains the high level HVAC control
    // subroutines.  Subroutine ManageHVAC, which is called from the heat balance,
    // calls the HVAC simulation and is the most probable insertion point for
    // connections to other HVAC engines.  ManageHVAC also controls the system
    // timestep, automatically shortening the timestep to meet convergence criteria.

    // METHODOLOGY EMPLOYED:
    // The basic solution technique is iteration with lagging.
    // The timestep is shortened using a bisection method.

    // REFERENCES:

    // Using/Aliasing
    using namespace DataPrecisionGlobals;
    using DataGlobals::AnyEnergyManagementSystemInModel;
    using DataGlobals::AnyIdealCondEntSetPointInModel;
    using DataGlobals::BeginDayFlag;
    using DataGlobals::BeginEnvrnFlag;
    using DataGlobals::BeginTimeStepFlag;
    using DataGlobals::DayOfSim;
    using DataGlobals::DisplayExtraWarnings;
    using DataGlobals::DoOutputReporting;
    using DataGlobals::DuringDay;
    using DataGlobals::emsCallFromAfterHVACManagers;
    using DataGlobals::emsCallFromBeforeHVACManagers;
    using DataGlobals::emsCallFromBeginTimestepBeforePredictor;
    using DataGlobals::emsCallFromEndSystemTimestepAfterHVACReporting;
    using DataGlobals::emsCallFromEndSystemTimestepBeforeHVACReporting;
    using DataGlobals::emsCallFromHVACIterationLoop;
    using DataGlobals::EndHourFlag;
    using DataGlobals::HourOfDay;
    using DataGlobals::isPulseZoneSizing;
    using DataGlobals::KickOffSimulation;
    using DataGlobals::MetersHaveBeenInitialized;
    using DataGlobals::NumOfZones;
    using DataGlobals::RunOptCondEntTemp;
    using DataGlobals::SecInHour;
    using DataGlobals::SysSizingCalc;
    using DataGlobals::TimeStep;
    using DataGlobals::TimeStepZone;
    using DataGlobals::WarmupFlag;
    using DataGlobals::ZoneSizingCalc;
    using namespace DataEnvironment;

    using DataHeatBalFanSys::iCorrectStep;
    using DataHeatBalFanSys::iGetZoneSetPoints;
    using DataHeatBalFanSys::iPredictStep;
    using DataHeatBalFanSys::iPushSystemTimestepHistories;
    using DataHeatBalFanSys::iPushZoneTimestepHistories;
    using DataHeatBalFanSys::iRevertZoneTimestepHistories;
    using DataHeatBalFanSys::MAT;
    using DataHeatBalFanSys::ZoneAirHumRat;
    using DataHeatBalFanSys::ZoneAirHumRatAvg;
    using DataHeatBalFanSys::ZT;
    using DataHeatBalFanSys::ZTAV;
    using namespace DataHVACGlobals;
    using namespace DataLoopNode;
    using namespace DataAirLoop;
    using namespace DataConvergParams;
    using namespace DataReportingFlags;

    // Data
    // MODULE PARAMETER DEFINITIONS:
    static std::string const BlankString;

    // MODULE VARIABLE DECLARATIONS:

    int HVACManageIteration(0); // counts iterations to enforce maximum iteration limit
    int RepIterAir(0);

    namespace {
        // These were static variables within different functions. They were pulled out into the namespace
        // to facilitate easier unit testing of those functions.
        // These are purposefully not in the header file as an extern variable. No one outside of this should
        // use these. They are cleared by clear_state() for use by unit tests, but normal simulations should be unaffected.
        // This is purposefully in an anonymous namespace so nothing outside this implementation file can use it.
        bool SimHVACIterSetup(false);
        bool TriggerGetAFN(true);
        bool ReportAirHeatBalanceFirstTimeFlag(true);
        bool MyOneTimeFlag(true);
        bool PrintedWarmup(false);
        bool MyEnvrnFlag(true);
        bool DebugNamesReported(false);
        bool MySetPointInit(true);
        bool MyEnvrnFlag2(true);
        bool FlowMaxAvailAlreadyReset(false);
        bool FlowResolutionNeeded(false);
    } // namespace
    // SUBROUTINE SPECIFICATIONS FOR MODULE PrimaryPlantLoops
    // and zone equipment simulations

    // MODULE SUBROUTINES:

    // Functions
    void clear_state()
    {
        HVACManageIteration = 0;
        RepIterAir = 0;
        SimHVACIterSetup = false;
        TriggerGetAFN = true;
        ReportAirHeatBalanceFirstTimeFlag = true;
        MyOneTimeFlag = true;
        PrintedWarmup = false;
        MyEnvrnFlag = true;
        DebugNamesReported = false;
        MySetPointInit = true;
        MyEnvrnFlag2 = true;
        FlowMaxAvailAlreadyReset = false;
        FlowResolutionNeeded = false;
    }

    void ManageHVAC(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHORS:  Russ Taylor, Dan Fisher
        //       DATE WRITTEN:  Jan. 1998
        //       MODIFIED       Jul 2003 (CC) added a subroutine call for air models
        //       RE-ENGINEERED  May 2008, Brent Griffith, revised variable time step method and zone conditions history

        // PURPOSE OF THIS SUBROUTINE:
        // This routine effectively replaces the IBLAST
        // "SystemDriver" routine.  The main function of the routine
        // is to set the system timestep, "TimeStepSys", call the models related to zone
        // air temperatures, and .

        // METHODOLOGY EMPLOYED:
        //  manage calls to Predictor and Corrector and other updates in ZoneTempPredictorCorrector
        //  manage variable time step and when zone air histories are updated.

        // REFERENCES:

        // Using/Aliasing
        using DataConvergParams::MaxZoneTempDiff;
        using DataConvergParams::MinTimeStepSys; // =0.0166667     != 1 minute | 0.3 C = (1% OF 300 C) =max allowable diff between ZoneAirTemp at
                                                 // Time=T & T-1

        using ZoneTempPredictorCorrector::DetectOscillatingZoneTemp;
        using ZoneTempPredictorCorrector::ManageZoneAirUpdates;

        using AirflowNetworkBalanceManager::ManageAirflowNetworkBalance;
        using DataContaminantBalance::Contaminant;
        using DataContaminantBalance::OutdoorCO2;
        using DataContaminantBalance::OutdoorGC;
        using DataContaminantBalance::ZoneAirCO2;
        using DataContaminantBalance::ZoneAirCO2Avg;
        using DataContaminantBalance::ZoneAirCO2Temp;
        using DataContaminantBalance::ZoneAirGC;
        using DataContaminantBalance::ZoneAirGCAvg;
        using DataContaminantBalance::ZoneAirGCTemp;
        using DataGlobals::CompLoadReportIsReq;
        using DataGlobals::KindOfSim;
        using DataGlobals::ksHVACSizeDesignDay;
        using DataGlobals::ksHVACSizeRunPeriodDesign;
        using DataHeatBalFanSys::QRadSurfAFNDuct;
        using DataHeatBalFanSys::SysDepZoneLoads;
        using DataHeatBalFanSys::SysDepZoneLoadsLagged;
        using DataHeatBalFanSys::ZoneAirHumRatAvgComf;
        using DataHeatBalFanSys::ZoneThermostatSetPointHi;
        using DataHeatBalFanSys::ZoneThermostatSetPointHiAver;
        using DataHeatBalFanSys::ZoneThermostatSetPointLo;
        using DataHeatBalFanSys::ZoneThermostatSetPointLoAver;
        using DataHeatBalFanSys::ZTAVComf;
        using DataSystemVariables::ReportDuringWarmup; // added for FMI
        using DataSystemVariables::UpdateDataDuringWarmupExternalInterface;
        using DemandManager::ManageDemand;
        using DemandManager::UpdateDemandManagers;
        using EMSManager::ManageEMS;
        using IceThermalStorage::UpdateIceFractions;
        using InternalHeatGains::UpdateInternalGainValues;
        using NodeInputManager::CalcMoreNodeInfo;
        using OutAirNodeManager::SetOutAirNodes;
        using OutputReportTabular::GatherComponentLoadsHVAC;
        using OutputReportTabular::UpdateTabularReports; // added for writing tabular output reports
        using PlantManager::UpdateNodeThermalHistory;
        using PollutionModule::CalculatePollution;
        using RefrigeratedCase::ManageRefrigeratedCaseRacks;
        using ScheduleManager::GetCurrentScheduleValue;
        using SizingManager::UpdateFacilitySizing;
        using SystemAvailabilityManager::ManageHybridVentilation;
        using SystemReports::InitEnergyReports;
        using SystemReports::ReportMaxVentilationLoads;
        using SystemReports::ReportSystemEnergyUse;
        using WaterManager::ManageWater;
        using WaterManager::ManageWaterInits;
        using ZoneContaminantPredictorCorrector::ManageZoneContaminanUpdates;
        using ZoneEquipmentManager::CalcAirFlowSimple;
        using ZoneEquipmentManager::UpdateZoneSizing;
        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:
        // na

        // SUBROUTINE PARAMETER DEFINITIONS:
        static constexpr auto EndOfHeaderString("End of Data Dictionary");          // End of data dictionary marker
        static constexpr auto EnvironmentStampFormatStr("{},{},{:7.2F},{:7.2F},{:7.2F},{:7.2F}\n"); // Format descriptor for environ stamp


        // INTERFACE BLOCK SPECIFICATIONS:
        // na

        // DERIVED TYPE DEFINITIONS:
        // na

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 PriorTimeStep;       // magnitude of time step for previous history terms
        Real64 ZoneTempChange(0.0); // change in zone air temperature from timestep t-1 to t
        int NodeNum;
        bool ReportDebug;
        int ZoneNum;

        static int ZTempTrendsNumSysSteps(0);
        static int SysTimestepLoop(0);
        bool DummyLogical;

        // Formats

        // SYSTEM INITIALIZATION
        if (TriggerGetAFN) {
            TriggerGetAFN = false;
            DisplayString("Initializing HVAC");
            ManageAirflowNetworkBalance(state); // first call only gets input and returns.
        }

        ZT = MAT;
        // save for use with thermal comfort control models (Fang, Pierce, and KSU)
        ZTAVComf = ZTAV;
        ZoneAirHumRatAvgComf = ZoneAirHumRatAvg;
        ZTAV = 0.0;
        ZoneThermostatSetPointHiAver = 0.0;
        ZoneThermostatSetPointLoAver = 0.0;
        ZoneAirHumRatAvg = 0.0;
        PrintedWarmup = false;
        if (Contaminant.CO2Simulation) {
            OutdoorCO2 = GetCurrentScheduleValue(Contaminant.CO2OutdoorSchedPtr);
            ZoneAirCO2Avg = 0.0;
        }
        if (Contaminant.GenericContamSimulation) {
            OutdoorGC = GetCurrentScheduleValue(Contaminant.GenericContamOutdoorSchedPtr);
            if (allocated(ZoneAirGCAvg)) ZoneAirGCAvg = 0.0;
        }

        if (BeginEnvrnFlag && MyEnvrnFlag) {
            AirLoopsSimOnce = false;
            MyEnvrnFlag = false;
            NumOfSysTimeStepsLastZoneTimeStep = 1;
            PreviousTimeStep = TimeStepZone;
        }
        if (!BeginEnvrnFlag) {
            MyEnvrnFlag = true;
        }

        QRadSurfAFNDuct = 0.0;
        SysTimeElapsed = 0.0;
        TimeStepSys = TimeStepZone;
        FirstTimeStepSysFlag = true;
        ShortenTimeStepSys = false;
        UseZoneTimeStepHistory = true;
        PriorTimeStep = TimeStepZone;
        NumOfSysTimeSteps = 1;
        FracTimeStepZone = TimeStepSys / TimeStepZone;

        bool anyEMSRan;
        ManageEMS(state, emsCallFromBeginTimestepBeforePredictor, anyEMSRan, ObjexxFCL::Optional_int_const()); // calling point

        SetOutAirNodes(state);

        ManageRefrigeratedCaseRacks(state);

        // ZONE INITIALIZATION  'Get Zone Setpoints'
        ManageZoneAirUpdates(state, iGetZoneSetPoints, ZoneTempChange, ShortenTimeStepSys, UseZoneTimeStepHistory, PriorTimeStep);
        if (Contaminant.SimulateContaminants)
            ManageZoneContaminanUpdates(state, iGetZoneSetPoints, ShortenTimeStepSys, UseZoneTimeStepHistory, PriorTimeStep);

        ManageHybridVentilation(state);

        CalcAirFlowSimple(state);
        if (AirflowNetwork::SimulateAirflowNetwork > AirflowNetwork::AirflowNetworkControlSimple) {
            AirflowNetwork::RollBackFlag = false;
            ManageAirflowNetworkBalance(state, false);
        }

        SetHeatToReturnAirFlag(state);

        SysDepZoneLoadsLagged = SysDepZoneLoads;

        UpdateInternalGainValues(true, true);

        ManageZoneAirUpdates(state, iPredictStep, ZoneTempChange, ShortenTimeStepSys, UseZoneTimeStepHistory, PriorTimeStep);

        if (Contaminant.SimulateContaminants) ManageZoneContaminanUpdates(state, iPredictStep, ShortenTimeStepSys, UseZoneTimeStepHistory, PriorTimeStep);

        SimHVAC(state);

        if (AnyIdealCondEntSetPointInModel && MetersHaveBeenInitialized && !WarmupFlag) {
            RunOptCondEntTemp = true;
            while (RunOptCondEntTemp) {
                SimHVAC(state);
            }
        }

        ManageWaterInits(state);

        // Only simulate once per zone timestep; must be after SimHVAC
        if (FirstTimeStepSysFlag && MetersHaveBeenInitialized) {
            ManageDemand(state);
        }

        BeginTimeStepFlag = false; // At this point, we have been through the first pass through SimHVAC so this needs to be set

        ManageZoneAirUpdates(state, iCorrectStep, ZoneTempChange, ShortenTimeStepSys, UseZoneTimeStepHistory, PriorTimeStep);
        if (Contaminant.SimulateContaminants) ManageZoneContaminanUpdates(state, iCorrectStep, ShortenTimeStepSys, UseZoneTimeStepHistory, PriorTimeStep);

        if (ZoneTempChange > MaxZoneTempDiff && !KickOffSimulation) {
            // determine value of adaptive system time step
            // model how many system timesteps we want in zone timestep
            ZTempTrendsNumSysSteps = int(ZoneTempChange / MaxZoneTempDiff + 1.0); // add 1 for truncation
            NumOfSysTimeSteps = min(ZTempTrendsNumSysSteps, LimitNumSysSteps);
            // then determine timestep length for even distribution, protect div by zero
            if (NumOfSysTimeSteps > 0) TimeStepSys = TimeStepZone / NumOfSysTimeSteps;
            TimeStepSys = max(TimeStepSys, MinTimeStepSys);
            UseZoneTimeStepHistory = false;
            ShortenTimeStepSys = true;
        } else {
            NumOfSysTimeSteps = 1;
            UseZoneTimeStepHistory = true;
        }

        if (UseZoneTimeStepHistory) PreviousTimeStep = TimeStepZone;
        for (SysTimestepLoop = 1; SysTimestepLoop <= NumOfSysTimeSteps; ++SysTimestepLoop) {
            if (state.dataGlobal->stopSimulation) break;

            if (TimeStepSys < TimeStepZone) {

                ManageHybridVentilation(state);
                CalcAirFlowSimple(state, SysTimestepLoop);
                if (AirflowNetwork::SimulateAirflowNetwork > AirflowNetwork::AirflowNetworkControlSimple) {
                    AirflowNetwork::RollBackFlag = false;
                    ManageAirflowNetworkBalance(state, false);
                }

                UpdateInternalGainValues(true, true);

                ManageZoneAirUpdates(state, iPredictStep, ZoneTempChange, ShortenTimeStepSys, UseZoneTimeStepHistory,
                                     PriorTimeStep);

                if (Contaminant.SimulateContaminants)
                    ManageZoneContaminanUpdates(state, iPredictStep, ShortenTimeStepSys, UseZoneTimeStepHistory,
                                                PriorTimeStep);
                SimHVAC(state);

                if (AnyIdealCondEntSetPointInModel && MetersHaveBeenInitialized && !WarmupFlag) {
                    RunOptCondEntTemp = true;
                    while (RunOptCondEntTemp) {
                        SimHVAC(state);
                    }
                }

                ManageWaterInits(state);

                // Need to set the flag back since we do not need to shift the temps back again in the correct step.
                ShortenTimeStepSys = false;

                ManageZoneAirUpdates(state, iCorrectStep, ZoneTempChange, ShortenTimeStepSys, UseZoneTimeStepHistory,
                                     PriorTimeStep);
                if (Contaminant.SimulateContaminants)
                    ManageZoneContaminanUpdates(state, iCorrectStep, ShortenTimeStepSys, UseZoneTimeStepHistory,
                                                PriorTimeStep);

                ManageZoneAirUpdates(state, iPushSystemTimestepHistories, ZoneTempChange, ShortenTimeStepSys,
                                     UseZoneTimeStepHistory, PriorTimeStep);
                if (Contaminant.SimulateContaminants)
                    ManageZoneContaminanUpdates(state, iPushSystemTimestepHistories, ShortenTimeStepSys,
                                                UseZoneTimeStepHistory, PriorTimeStep);
                PreviousTimeStep = TimeStepSys;
            }

            FracTimeStepZone = TimeStepSys / TimeStepZone;

            for (ZoneNum = 1; ZoneNum <= NumOfZones; ++ZoneNum) {
                ZTAV(ZoneNum) += ZT(ZoneNum) * FracTimeStepZone;
                ZoneAirHumRatAvg(ZoneNum) += ZoneAirHumRat(ZoneNum) * FracTimeStepZone;
                if (Contaminant.CO2Simulation) ZoneAirCO2Avg(ZoneNum) += ZoneAirCO2(ZoneNum) * FracTimeStepZone;
                if (Contaminant.GenericContamSimulation) ZoneAirGCAvg(ZoneNum) += ZoneAirGC(ZoneNum) * FracTimeStepZone;
                if (state.dataZoneTempPredictorCorrector->NumOnOffCtrZone > 0) {
                    ZoneThermostatSetPointHiAver(ZoneNum) += ZoneThermostatSetPointHi(ZoneNum) * FracTimeStepZone;
                    ZoneThermostatSetPointLoAver(ZoneNum) += ZoneThermostatSetPointLo(ZoneNum) * FracTimeStepZone;
                }
            }

            DetectOscillatingZoneTemp(state);
            UpdateZoneListAndGroupLoads(); // Must be called before UpdateDataandReport(OutputProcessor::TimeStepType::TimeStepSystem)
            UpdateIceFractions();          // Update fraction of ice stored in TES
            ManageWater(state);
            // update electricity data for net, purchased, sold etc.
            DummyLogical = false;
            facilityElectricServiceObj->manageElectricPowerService(state, false, DummyLogical, true);

            // Update the plant and condenser loop capacitance model temperature history.
            UpdateNodeThermalHistory();

            if (OutputReportTabular::displayHeatEmissionsSummary) {
                OutputReportTabular::CalcHeatEmissionReport(state);
            }

            ManageEMS(state, emsCallFromEndSystemTimestepBeforeHVACReporting, anyEMSRan, ObjexxFCL::Optional_int_const()); // EMS calling point

            // This is where output processor data is updated for System Timestep reporting
            if (!WarmupFlag) {
                if (DoOutputReporting) {
                    CalcMoreNodeInfo(state);
                    CalculatePollution();
                    InitEnergyReports(state);
                    ReportSystemEnergyUse();
                }
                if (DoOutputReporting || (ZoneSizingCalc && CompLoadReportIsReq)) {
                    ReportAirHeatBalance(state);
                    if (ZoneSizingCalc) GatherComponentLoadsHVAC();
                }
                if (DoOutputReporting) {
                    ReportMaxVentilationLoads(state);
                    UpdateDataandReport(state, OutputProcessor::TimeStepType::TimeStepSystem);
                    if (KindOfSim == ksHVACSizeDesignDay || KindOfSim == ksHVACSizeRunPeriodDesign) {
                        if (hvacSizingSimulationManager) hvacSizingSimulationManager->UpdateSizingLogsSystemStep(state);
                    }
                    UpdateTabularReports(state, OutputProcessor::TimeStepType::TimeStepSystem);
                }
                if (ZoneSizingCalc) {
                    UpdateZoneSizing(state, DuringDay);
                    UpdateFacilitySizing(state, DuringDay);
                }
                EIRPlantLoopHeatPumps::EIRPlantLoopHeatPump::checkConcurrentOperation();
            } else if (!KickOffSimulation && DoOutputReporting && ReportDuringWarmup) {
                if (BeginDayFlag && !PrintEnvrnStampWarmupPrinted) {
                    PrintEnvrnStampWarmup = true;
                    PrintEnvrnStampWarmupPrinted = true;
                }
                if (!BeginDayFlag) PrintEnvrnStampWarmupPrinted = false;
                if (PrintEnvrnStampWarmup) {
                    if (PrintEndDataDictionary && DoOutputReporting && !PrintedWarmup) {
                        print(state.files.eso, "{}\n", EndOfHeaderString);
                        print(state.files.mtr, "{}\n", EndOfHeaderString);
                        PrintEndDataDictionary = false;
                    }
                    if (DoOutputReporting && !PrintedWarmup) {

                        print(state.files.eso,
                              EnvironmentStampFormatStr,
                              "1",
                              "Warmup {" + cWarmupDay + "} " + EnvironmentName,
                              Latitude,
                              Longitude,
                              TimeZoneNumber,
                              Elevation);
                        print(state.files.mtr,
                              EnvironmentStampFormatStr,
                              "1",
                              "Warmup {" + cWarmupDay + "} " + EnvironmentName,
                              Latitude,
                              Longitude,
                              TimeZoneNumber,
                              Elevation);
                        PrintEnvrnStampWarmup = false;
                    }
                    PrintedWarmup = true;
                }
                CalcMoreNodeInfo(state);
                UpdateDataandReport(state, OutputProcessor::TimeStepType::TimeStepSystem);
                if (KindOfSim == ksHVACSizeDesignDay || KindOfSim == ksHVACSizeRunPeriodDesign) {
                    if (hvacSizingSimulationManager) hvacSizingSimulationManager->UpdateSizingLogsSystemStep(state);
                }
            } else if (UpdateDataDuringWarmupExternalInterface) { // added for FMI
                if (BeginDayFlag && !PrintEnvrnStampWarmupPrinted) {
                    PrintEnvrnStampWarmup = true;
                    PrintEnvrnStampWarmupPrinted = true;
                }
                if (!BeginDayFlag) PrintEnvrnStampWarmupPrinted = false;
                if (PrintEnvrnStampWarmup) {
                    if (PrintEndDataDictionary && DoOutputReporting && !PrintedWarmup) {
                        print(state.files.eso, "{}\n", EndOfHeaderString);
                        print(state.files.mtr, "{}\n", EndOfHeaderString);
                        PrintEndDataDictionary = false;
                    }
                    if (DoOutputReporting && !PrintedWarmup) {
                        print(state.files.eso,
                              EnvironmentStampFormatStr,
                              "1",
                              "Warmup {" + cWarmupDay + "} " + EnvironmentName,
                              Latitude,
                              Longitude,
                              TimeZoneNumber,
                              Elevation);
                        print(state.files.mtr,
                              EnvironmentStampFormatStr,
                              "1",
                              "Warmup {" + cWarmupDay + "} " + EnvironmentName,
                              Latitude,
                              Longitude,
                              TimeZoneNumber,
                              Elevation);
                        PrintEnvrnStampWarmup = false;
                    }
                    PrintedWarmup = true;
                }
                UpdateDataandReport(state, OutputProcessor::TimeStepType::TimeStepSystem);
            }
            ManageEMS(state, emsCallFromEndSystemTimestepAfterHVACReporting, anyEMSRan, ObjexxFCL::Optional_int_const()); // EMS calling point
            // UPDATE SYSTEM CLOCKS
            SysTimeElapsed += TimeStepSys;

            FirstTimeStepSysFlag = false;
        } // system time step  loop (loops once if no downstepping)

        ManageZoneAirUpdates(state, iPushZoneTimestepHistories, ZoneTempChange, ShortenTimeStepSys, UseZoneTimeStepHistory, PriorTimeStep);
        if (Contaminant.SimulateContaminants)
            ManageZoneContaminanUpdates(state, iPushZoneTimestepHistories, ShortenTimeStepSys, UseZoneTimeStepHistory, PriorTimeStep);

        NumOfSysTimeStepsLastZoneTimeStep = NumOfSysTimeSteps;

        UpdateDemandManagers(state);

        // DO FINAL UPDATE OF RECORD KEEPING VARIABLES
        // Report the Node Data to Aid in Debugging
        if (DebugOutput) {
            if (EvenDuringWarmup) {
                ReportDebug = true;
            } else {
                ReportDebug = !WarmupFlag;
            }
            if ((ReportDebug) && (DayOfSim > 0)) { // Report the node data
                if (size(Node) > 0 && !DebugNamesReported) {
                    print(state.files.debug, "{}\n", "node #   Name");
                    for (NodeNum = 1; NodeNum <= isize(Node); ++NodeNum) {
                        print(state.files.debug, " {:3}     {}\n", NodeNum, NodeID(NodeNum));
                    }
                    DebugNamesReported = true;
                }
                if (size(Node) > 0) {
                    print(state.files.debug, "\n\n Day of Sim     Hour of Day    Time\n");
                    print(state.files.debug, "{:12}{:12} {:22.15N} \n", DayOfSim, HourOfDay, TimeStep * TimeStepZone);
                    print(state.files.debug,
                          "{}\n",
                          "node #   Temp   MassMinAv  MassMaxAv TempSP      MassFlow       MassMin       MassMax        MassSP    Press        "
                          "Enthal     HumRat Fluid Type");
                }
                for (NodeNum = 1; NodeNum <= isize(Node); ++NodeNum) {
                    static constexpr auto Format_20{
                        " {:3} {:8.2F}  {:8.3F}  {:8.3F}  {:8.2F} {:13.2F} {:13.2F} {:13.2F} {:13.2F}  {:#8.0F}  {:11.2F}  {:9.5F}  {}\n"};

                    print(state.files.debug,
                          Format_20,
                          NodeNum,
                          Node(NodeNum).Temp,
                          Node(NodeNum).MassFlowRateMinAvail,
                          Node(NodeNum).MassFlowRateMaxAvail,
                          Node(NodeNum).TempSetPoint,
                          Node(NodeNum).MassFlowRate,
                          Node(NodeNum).MassFlowRateMin,
                          Node(NodeNum).MassFlowRateMax,
                          Node(NodeNum).MassFlowRateSetPoint,
                          Node(NodeNum).Press,
                          Node(NodeNum).Enthalpy,
                          Node(NodeNum).HumRat,
                          ValidNodeFluidTypes(Node(NodeNum).FluidType));
                }
            }
        }
    }

    void SimHVAC(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR:          Dan Fisher
        //       DATE WRITTEN:    April 1997
        //       DATE MODIFIED:   May 1998 (RKS,RDT)

        // PURPOSE OF THIS SUBROUTINE: Selects and calls the HVAC loop managers

        // METHODOLOGY EMPLOYED: Each loop manager is called or passed over
        // in succession based on the logical flags associated with the manager.
        // The logical flags are set in the manager routines and passed
        // as parameters to this routine.  Each loop manager potentially
        // affects a different set of other loop managers.

        // Future development could involve specifying any number of user
        // selectable control schemes based on the logical flags used in
        // this default control algorithm.

        // REFERENCES: none

        // Using/Aliasing
        using namespace DataConvergParams;
        using DataEnvironment::CurMnDy;
        using DataEnvironment::EnvironmentName;
        using DataGlobals::AnyPlantInModel;
        using DataPlant::ConvergenceHistoryARR;
        using DataPlant::DemandSide;
        using DataPlant::NumConvergenceHistoryTerms;
        using DataPlant::PlantLoop;
        using DataPlant::PlantManageHalfLoopCalls;
        using DataPlant::PlantManageSubIterations;
        using DataPlant::square_sum_ConvergenceHistoryARR;
        using DataPlant::sum_ConvergenceHistoryARR;
        using DataPlant::sum_square_ConvergenceHistoryARR;
        using DataPlant::SupplySide;
        using DataPlant::TotNumLoops;
        using EMSManager::ManageEMS;
        using General::CreateSysTimeIntervalString;
        using General::RoundSigDigits;
        using NonZoneEquipmentManager::ManageNonZoneEquipment;
        using PlantCondLoopOperation::SetupPlantEMSActuators;
        using PlantManager::GetPlantInput;
        using PlantManager::GetPlantLoopData;
        using PlantManager::InitOneTimePlantSizingInfo;
        using PlantManager::ReInitPlantLoopsAtFirstHVACIteration;
        using PlantManager::SetupBranchControlTypes;
        using PlantManager::SetupInitialPlantCallingOrder;
        using PlantManager::SetupReports;
        using PlantUtilities::AnyPlantSplitterMixerLacksContinuity;
        using PlantUtilities::CheckForRunawayPlantTemps;
        using PlantUtilities::CheckPlantMixerSplitterConsistency;
        using PlantUtilities::SetAllPlantSimFlagsToValue;
        using SetPointManager::ManageSetPoints;
        using SystemAvailabilityManager::ManageSystemAvailability;
        using ZoneEquipmentManager::ManageZoneEquipment;

        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:
        // na

        // SUBROUTINE PARAMETER DEFINITIONS:
        bool const SimWithPlantFlowUnlocked(false);
        bool const SimWithPlantFlowLocked(true);

        // INTERFACE BLOCK SPECIFICATIONS:
        // na

        // DERIVED TYPE DEFINITIONS:
        // na

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        bool FirstHVACIteration; // True when solution technique on first iteration
        static int ErrCount(0); // Number of times that the maximum iterations was exceeded
        static int MaxErrCount(0);
        static std::string ErrEnvironmentName;
        int LoopNum;
        int LoopSide;
        int ThisLoopSide;

        int AirSysNum;
        int StackDepth;
        std::string HistoryTrace;
        Real64 SlopeHumRat;
        Real64 SlopeMdot;
        Real64 SlopeTemps;
        Real64 AvgValue;
        bool FoundOscillationByDuplicate;
        int ZoneNum;
        int NodeIndex;
        bool MonotonicIncreaseFound;
        bool MonotonicDecreaseFound;

        // Initialize all of the simulation flags to true for the first iteration
        SimZoneEquipmentFlag = true;
        SimNonZoneEquipmentFlag = true;
        SimAirLoopsFlag = true;
        SimPlantLoopsFlag = true;
        SimElecCircuitsFlag = true;
        FirstHVACIteration = true;

        if (state.dataAirLoop->AirLoopInputsFilled) {
            for (auto &e : state.dataAirLoop->AirLoopControlInfo) {
                // Reset air loop control info for cooling coil active flag (used in TU's for reheat air flow control)
                e.CoolingActiveFlag = false;
                // Reset air loop control info for heating coil active flag (used in OA controller for HX control)
                e.HeatingActiveFlag = false;
                // reset outside air system HX to off first time through
                e.HeatRecoveryBypass = true;
                // set HX check status flag to check for custom control in MixedAir.cc
                e.CheckHeatRecoveryBypassStatus = true;
                // set OA comp simulated flag to false
                e.OASysComponentsSimulated = false;
                // set economizer flow locked flag to false, will reset if custom HX control is used
                e.EconomizerFlowLocked = false;
                // set air loop resim flags for when heat recovery is used and air loop needs another iteration
                e.HeatRecoveryResimFlag = true;
                e.HeatRecoveryResimFlag2 = false;
                e.ResimAirLoopFlag = false;
            }
        }

        // This setups the reports for the Iteration variable that limits how many times
        //  it goes through all of the HVAC managers before moving on.
        // The plant loop 'get inputs' and initialization are also done here in order to allow plant loop connected components
        // simulated by managers other than the plant manager to run correctly.
        HVACManageIteration = 0;
        PlantManageSubIterations = 0;
        PlantManageHalfLoopCalls = 0;
        SetAllPlantSimFlagsToValue(true);
        if (!SimHVACIterSetup) {
            SetupOutputVariable(state, "HVAC System Solver Iteration Count", OutputProcessor::Unit::None, HVACManageIteration, "HVAC", "Sum", "SimHVAC");
            SetupOutputVariable(state, "Air System Solver Iteration Count", OutputProcessor::Unit::None, RepIterAir, "HVAC", "Sum", "SimHVAC");
            SetupOutputVariable(state, "Air System Relief Air Total Heat Loss Energy",
                                OutputProcessor::Unit::J,
                                DataHeatBalance::SysTotalHVACReliefHeatLoss,
                                "HVAC",
                                "Sum",
                                "SimHVAC");
            SetupOutputVariable(state, "HVAC System Total Heat Rejection Energy",
                                OutputProcessor::Unit::J,
                                DataHeatBalance::SysTotalHVACRejectHeatLoss,
                                "HVAC",
                                "Sum",
                                "SimHVAC");
            ManageSetPoints(state); // need to call this before getting plant loop data so setpoint checks can complete okay
            GetPlantLoopData(state);
            GetPlantInput(state);
            SetupInitialPlantCallingOrder();
            SetupBranchControlTypes(); // new routine to do away with input for branch control type
            //    CALL CheckPlantLoopData
            SetupReports(state);
            if (AnyEnergyManagementSystemInModel) {
                SetupPlantEMSActuators();
            }

            if (TotNumLoops > 0) {
                SetupOutputVariable(state,
                    "Plant Solver Sub Iteration Count", OutputProcessor::Unit::None, PlantManageSubIterations, "HVAC", "Sum", "SimHVAC");
                SetupOutputVariable(state,
                    "Plant Solver Half Loop Calls Count", OutputProcessor::Unit::None, PlantManageHalfLoopCalls, "HVAC", "Sum", "SimHVAC");
                for (LoopNum = 1; LoopNum <= TotNumLoops; ++LoopNum) {
                    // init plant sizing numbers in main plant data structure
                    InitOneTimePlantSizingInfo(LoopNum);
                }
            }
            SimHVACIterSetup = true;
        }

        if (ZoneSizingCalc) {
            ManageZoneEquipment(state, FirstHVACIteration, SimZoneEquipmentFlag, SimAirLoopsFlag);
            // need to call non zone equipment so water use zone gains can be included in sizing calcs
            ManageNonZoneEquipment(state, FirstHVACIteration, SimNonZoneEquipmentFlag);
            facilityElectricServiceObj->manageElectricPowerService(state, FirstHVACIteration, SimElecCircuitsFlag, false);
            return;
        }

        // Before the HVAC simulation, reset control flags and specified flow
        // rates that might have been set by the set point and availability
        // managers.

        ResetHVACControl(state);

        // Before the HVAC simulation, call ManageSetPoints to set all the HVAC
        // node setpoints
        bool anyEMSRan = false;
        ManageEMS(state, emsCallFromBeforeHVACManagers, anyEMSRan, ObjexxFCL::Optional_int_const()); // calling point

        ManageSetPoints(state);

        // re-initialize plant loop and nodes.
        ReInitPlantLoopsAtFirstHVACIteration(state);

        // Before the HVAC simulation, call ManageSystemAvailability to set
        // the system on/off flags
        ManageSystemAvailability(state);

        ManageEMS(state, emsCallFromAfterHVACManagers, anyEMSRan, ObjexxFCL::Optional_int_const()); // calling point
        ManageEMS(state, emsCallFromHVACIterationLoop, anyEMSRan, ObjexxFCL::Optional_int_const()); // calling point id

        // first explicitly call each system type with FirstHVACIteration,

        // Manages the various component simulations
        SimSelectedEquipment(state, SimAirLoopsFlag,
                             SimZoneEquipmentFlag,
                             SimNonZoneEquipmentFlag,
                             SimPlantLoopsFlag,
                             SimElecCircuitsFlag,
                             FirstHVACIteration,
                             SimWithPlantFlowUnlocked);

        // Eventually, when all of the flags are set to false, the
        // simulation has converged for this system time step.

        SimPlantLoopsFlag = true;
        SetAllPlantSimFlagsToValue(true); // set so loop to simulate at least once on non-first hvac

        FirstHVACIteration = false;

        // then iterate among all systems after first HVAC iteration is over

        // Main iteration loop for HVAC.  If any of the simulation flags are
        // true, then specific components must be resimulated.
        while ((SimAirLoopsFlag || SimZoneEquipmentFlag || SimNonZoneEquipmentFlag || SimPlantLoopsFlag || SimElecCircuitsFlag) &&
               (HVACManageIteration <= MaxIter)) {

            if (state.dataGlobal->stopSimulation) break;

            ManageEMS(state, emsCallFromHVACIterationLoop, anyEMSRan, ObjexxFCL::Optional_int_const()); // calling point id

            // Manages the various component simulations
            SimSelectedEquipment(state, SimAirLoopsFlag,
                                 SimZoneEquipmentFlag,
                                 SimNonZoneEquipmentFlag,
                                 SimPlantLoopsFlag,
                                 SimElecCircuitsFlag,
                                 FirstHVACIteration,
                                 SimWithPlantFlowUnlocked);

            // Eventually, when all of the flags are set to false, the
            // simulation has converged for this system time step.

            UpdateZoneInletConvergenceLog();

            ++HVACManageIteration; // Increment the iteration counter

            if (anyEMSRan && HVACManageIteration <= 2) {
                // the calling point emsCallFromHVACIterationLoop is only effective for air loops if this while loop runs at least twice
                SimAirLoopsFlag = true;
            }
            if (HVACManageIteration < MinAirLoopIterationsAfterFirst) {
                // sequenced zone loads for airloops may require extra iterations depending upon zone equipment order and load distribution type
                SimAirLoopsFlag = true;
                SimZoneEquipmentFlag = true;
            }
        }
        if (AnyPlantInModel) {
            if (AnyPlantSplitterMixerLacksContinuity()) {
                // rerun systems in a "Final flow lock/last iteration" mode
                // now call for one second to last plant simulation
                SimAirLoopsFlag = false;
                SimZoneEquipmentFlag = false;
                SimNonZoneEquipmentFlag = false;
                SimPlantLoopsFlag = true;
                SimElecCircuitsFlag = false;
                SimSelectedEquipment(state, SimAirLoopsFlag,
                                     SimZoneEquipmentFlag,
                                     SimNonZoneEquipmentFlag,
                                     SimPlantLoopsFlag,
                                     SimElecCircuitsFlag,
                                     FirstHVACIteration,
                                     SimWithPlantFlowUnlocked);
                // now call for all non-plant simulation, but with plant flow lock on
                SimAirLoopsFlag = true;
                SimZoneEquipmentFlag = true;
                SimNonZoneEquipmentFlag = true;
                SimPlantLoopsFlag = false;
                SimElecCircuitsFlag = true;
                SimSelectedEquipment(state, SimAirLoopsFlag,
                                     SimZoneEquipmentFlag,
                                     SimNonZoneEquipmentFlag,
                                     SimPlantLoopsFlag,
                                     SimElecCircuitsFlag,
                                     FirstHVACIteration,
                                     SimWithPlantFlowLocked);
                UpdateZoneInletConvergenceLog();
                // now call for a last plant simulation
                SimAirLoopsFlag = false;
                SimZoneEquipmentFlag = false;
                SimNonZoneEquipmentFlag = false;
                SimPlantLoopsFlag = true;
                SimElecCircuitsFlag = false;
                SimSelectedEquipment(state, SimAirLoopsFlag,
                                     SimZoneEquipmentFlag,
                                     SimNonZoneEquipmentFlag,
                                     SimPlantLoopsFlag,
                                     SimElecCircuitsFlag,
                                     FirstHVACIteration,
                                     SimWithPlantFlowUnlocked);
                // now call for a last all non-plant simulation, but with plant flow lock on
                SimAirLoopsFlag = true;
                SimZoneEquipmentFlag = true;
                SimNonZoneEquipmentFlag = true;
                SimPlantLoopsFlag = false;
                SimElecCircuitsFlag = true;
                SimSelectedEquipment(state, SimAirLoopsFlag,
                                     SimZoneEquipmentFlag,
                                     SimNonZoneEquipmentFlag,
                                     SimPlantLoopsFlag,
                                     SimElecCircuitsFlag,
                                     FirstHVACIteration,
                                     SimWithPlantFlowLocked);
                UpdateZoneInletConvergenceLog();
            }
        }

        // DSU  Test plant loop for errors
        for (LoopNum = 1; LoopNum <= TotNumLoops; ++LoopNum) {
            for (LoopSide = DemandSide; LoopSide <= SupplySide; ++LoopSide) {
                CheckPlantMixerSplitterConsistency(LoopNum, LoopSide, FirstHVACIteration);
                CheckForRunawayPlantTemps(LoopNum, LoopSide);
            }
        }

        if ((HVACManageIteration > MaxIter) && (!WarmupFlag)) {
            ++ErrCount;
            if (ErrCount < 15) {
                ErrEnvironmentName = EnvironmentName;
                ShowWarningError("SimHVAC: Maximum iterations (" + fmt::to_string(MaxIter) + ") exceeded for all HVAC loops, at " + EnvironmentName + ", " +
                                 CurMnDy + ' ' + CreateSysTimeIntervalString());
                if (SimAirLoopsFlag) {
                    ShowContinueError("The solution for one or more of the Air Loop HVAC systems did not appear to converge");
                }
                if (SimZoneEquipmentFlag) {
                    ShowContinueError("The solution for zone HVAC equipment did not appear to converge");
                }
                if (SimNonZoneEquipmentFlag) {
                    ShowContinueError("The solution for non-zone equipment did not appear to converge");
                }
                if (SimPlantLoopsFlag) {
                    ShowContinueError("The solution for one or more plant systems did not appear to converge");
                }
                if (SimElecCircuitsFlag) {
                    ShowContinueError("The solution for on-site electric generators did not appear to converge");
                }
                if (ErrCount == 1 && !DisplayExtraWarnings) {
                    ShowContinueError("...use Output:Diagnostics,DisplayExtraWarnings; to show more details on each max iteration exceeded.");
                }
                if (DisplayExtraWarnings) {

                    for (AirSysNum = 1; AirSysNum <= NumPrimaryAirSys; ++AirSysNum) {

                        if (any(AirLoopConvergence(AirSysNum).HVACMassFlowNotConverged)) {

                            ShowContinueError("Air System Named = " + state.dataAirLoop->AirToZoneNodeInfo(AirSysNum).AirLoopName +
                                              " did not converge for mass flow rate");
                            ShowContinueError("Check values should be zero. Most Recent values listed first.");
                            HistoryTrace = "";
                            for (StackDepth = 1; StackDepth <= ConvergLogStackDepth; ++StackDepth) {
                                HistoryTrace += RoundSigDigits(AirLoopConvergence(AirSysNum).HVACFlowDemandToSupplyTolValue(StackDepth), 6) + ',';
                            }

                            ShowContinueError("Demand-to-Supply interface mass flow rate check value iteration history trace: " + HistoryTrace);
                            HistoryTrace = "";
                            for (StackDepth = 1; StackDepth <= ConvergLogStackDepth; ++StackDepth) {
                                HistoryTrace +=
                                    RoundSigDigits(AirLoopConvergence(AirSysNum).HVACFlowSupplyDeck1ToDemandTolValue(StackDepth), 6) + ',';
                            }
                            ShowContinueError("Supply-to-demand interface deck 1 mass flow rate check value iteration history trace: " +
                                              HistoryTrace);

                            if (state.dataAirLoop->AirToZoneNodeInfo(AirSysNum).NumSupplyNodes >= 2) {
                                HistoryTrace = "";
                                for (StackDepth = 1; StackDepth <= ConvergLogStackDepth; ++StackDepth) {
                                    HistoryTrace +=
                                        RoundSigDigits(AirLoopConvergence(AirSysNum).HVACFlowSupplyDeck2ToDemandTolValue(StackDepth), 6) + ',';
                                }
                                ShowContinueError("Supply-to-demand interface deck 2 mass flow rate check value iteration history trace: " +
                                                  HistoryTrace);
                            }
                        } // mass flow rate not converged

                        if (any(AirLoopConvergence(AirSysNum).HVACHumRatNotConverged)) {

                            ShowContinueError("Air System Named = " + state.dataAirLoop->AirToZoneNodeInfo(AirSysNum).AirLoopName +
                                              " did not converge for humidity ratio");
                            ShowContinueError("Check values should be zero. Most Recent values listed first.");
                            HistoryTrace = "";
                            for (StackDepth = 1; StackDepth <= ConvergLogStackDepth; ++StackDepth) {
                                HistoryTrace += RoundSigDigits(AirLoopConvergence(AirSysNum).HVACHumDemandToSupplyTolValue(StackDepth), 6) + ',';
                            }
                            ShowContinueError("Demand-to-Supply interface humidity ratio check value iteration history trace: " + HistoryTrace);
                            HistoryTrace = "";
                            for (StackDepth = 1; StackDepth <= ConvergLogStackDepth; ++StackDepth) {
                                HistoryTrace += RoundSigDigits(AirLoopConvergence(AirSysNum).HVACHumSupplyDeck1ToDemandTolValue(StackDepth), 6) + ',';
                            }
                            ShowContinueError("Supply-to-demand interface deck 1 humidity ratio check value iteration history trace: " +
                                              HistoryTrace);

                            if (state.dataAirLoop->AirToZoneNodeInfo(AirSysNum).NumSupplyNodes >= 2) {
                                HistoryTrace = "";
                                for (StackDepth = 1; StackDepth <= ConvergLogStackDepth; ++StackDepth) {
                                    HistoryTrace +=
                                        RoundSigDigits(AirLoopConvergence(AirSysNum).HVACHumSupplyDeck2ToDemandTolValue(StackDepth), 6) + ',';
                                }
                                ShowContinueError("Supply-to-demand interface deck 2 humidity ratio check value iteration history trace: " +
                                                  HistoryTrace);
                            }
                        } // humidity ratio not converged

                        if (any(AirLoopConvergence(AirSysNum).HVACTempNotConverged)) {

                            ShowContinueError("Air System Named = " + state.dataAirLoop->AirToZoneNodeInfo(AirSysNum).AirLoopName + " did not converge for temperature");
                            ShowContinueError("Check values should be zero. Most Recent values listed first.");
                            HistoryTrace = "";
                            for (StackDepth = 1; StackDepth <= ConvergLogStackDepth; ++StackDepth) {
                                HistoryTrace += RoundSigDigits(AirLoopConvergence(AirSysNum).HVACTempDemandToSupplyTolValue(StackDepth), 6) + ',';
                            }
                            ShowContinueError("Demand-to-Supply interface temperature check value iteration history trace: " + HistoryTrace);
                            HistoryTrace = "";
                            for (StackDepth = 1; StackDepth <= ConvergLogStackDepth; ++StackDepth) {
                                HistoryTrace +=
                                    RoundSigDigits(AirLoopConvergence(AirSysNum).HVACTempSupplyDeck1ToDemandTolValue(StackDepth), 6) + ',';
                            }
                            ShowContinueError("Supply-to-demand interface deck 1 temperature check value iteration history trace: " + HistoryTrace);

                            if (state.dataAirLoop->AirToZoneNodeInfo(AirSysNum).NumSupplyNodes >= 2) {
                                HistoryTrace = "";
                                for (StackDepth = 1; StackDepth <= ConvergLogStackDepth; ++StackDepth) {
                                    HistoryTrace +=
                                        RoundSigDigits(AirLoopConvergence(AirSysNum).HVACTempSupplyDeck1ToDemandTolValue(StackDepth), 6) + ',';
                                }
                                ShowContinueError("Supply-to-demand interface deck 2 temperature check value iteration history trace: " +
                                                  HistoryTrace);
                            }
                        } // Temps not converged
                        if (any(AirLoopConvergence(AirSysNum).HVACEnergyNotConverged)) {

                            ShowContinueError("Air System Named = " + state.dataAirLoop->AirToZoneNodeInfo(AirSysNum).AirLoopName + " did not converge for energy");
                            ShowContinueError("Check values should be zero. Most Recent values listed first.");
                            HistoryTrace = "";
                            for (StackDepth = 1; StackDepth <= ConvergLogStackDepth; ++StackDepth) {
                                HistoryTrace += RoundSigDigits(AirLoopConvergence(AirSysNum).HVACEnergyDemandToSupplyTolValue(StackDepth), 6) + ',';
                            }
                            ShowContinueError("Demand-to-Supply interface energy check value iteration history trace: " + HistoryTrace);
                            HistoryTrace = "";
                            for (StackDepth = 1; StackDepth <= ConvergLogStackDepth; ++StackDepth) {
                                HistoryTrace +=
                                    RoundSigDigits(AirLoopConvergence(AirSysNum).HVACEnergySupplyDeck1ToDemandTolValue(StackDepth), 6) + ',';
                            }
                            ShowContinueError("Supply-to-demand interface deck 1 energy check value iteration history trace: " + HistoryTrace);

                            if (state.dataAirLoop->AirToZoneNodeInfo(AirSysNum).NumSupplyNodes >= 2) {
                                HistoryTrace = "";
                                for (StackDepth = 1; StackDepth <= ConvergLogStackDepth; ++StackDepth) {
                                    HistoryTrace +=
                                        RoundSigDigits(AirLoopConvergence(AirSysNum).HVACEnergySupplyDeck2ToDemandTolValue(StackDepth), 6) + ',';
                                }
                                ShowContinueError("Supply-to-demand interface deck 2 energy check value iteration history trace: " + HistoryTrace);
                            }
                        } // energy not converged

                    } // loop over air loop systems

                    // loop over zones and check for issues with zone inlet nodes
                    for (ZoneNum = 1; ZoneNum <= NumOfZones; ++ZoneNum) {

                        for (NodeIndex = 1; NodeIndex <= ZoneInletConvergence(ZoneNum).NumInletNodes; ++NodeIndex) {

                            // Check humidity ratio
                            FoundOscillationByDuplicate = false;
                            MonotonicDecreaseFound = false;
                            MonotonicIncreaseFound = false;
                            // check for evidence of oscillation by indentify duplicates when latest value not equal to average
                            AvgValue = sum(ZoneInletConvergence(ZoneNum).InletNode(NodeIndex).HumidityRatio) / double(ConvergLogStackDepth);
                            if (std::abs(ZoneInletConvergence(ZoneNum).InletNode(NodeIndex).HumidityRatio(1) - AvgValue) >
                                HVACHumRatOscillationToler) { // last iterate differs from average
                                FoundOscillationByDuplicate = false;
                                for (StackDepth = 2; StackDepth <= ConvergLogStackDepth; ++StackDepth) {
                                    if (std::abs(ZoneInletConvergence(ZoneNum).InletNode(NodeIndex).HumidityRatio(1) -
                                                 ZoneInletConvergence(ZoneNum).InletNode(NodeIndex).HumidityRatio(StackDepth)) <
                                        HVACHumRatOscillationToler) {
                                        FoundOscillationByDuplicate = true;
                                        ShowContinueError("Node named " + NodeID(ZoneInletConvergence(ZoneNum).InletNode(NodeIndex).NodeNum) +
                                                          " shows oscillating humidity ratio across iterations with a repeated value of " +
                                                          RoundSigDigits(ZoneInletConvergence(ZoneNum).InletNode(NodeIndex).HumidityRatio(1), 6));
                                        break;
                                    }
                                }
                                if (!FoundOscillationByDuplicate) {
                                    SlopeHumRat = (sum_ConvergLogStackARR * sum(ZoneInletConvergence(ZoneNum).InletNode(NodeIndex).HumidityRatio) -
                                                   double(ConvergLogStackDepth) *
                                                       sum((ConvergLogStackARR * ZoneInletConvergence(ZoneNum).InletNode(NodeIndex).HumidityRatio))) /
                                                  (square_sum_ConvergLogStackARR - double(ConvergLogStackDepth) * sum_square_ConvergLogStackARR);
                                    if (std::abs(SlopeHumRat) > HVACHumRatSlopeToler) {

                                        if (SlopeHumRat < 0.0) { // check for monotic decrease
                                            MonotonicDecreaseFound = true;
                                            for (StackDepth = 2; StackDepth <= ConvergLogStackDepth; ++StackDepth) {
                                                if (ZoneInletConvergence(ZoneNum).InletNode(NodeIndex).HumidityRatio(StackDepth - 1) >
                                                    ZoneInletConvergence(ZoneNum).InletNode(NodeIndex).HumidityRatio(StackDepth)) {
                                                    MonotonicDecreaseFound = false;
                                                    break;
                                                }
                                            }
                                            if (MonotonicDecreaseFound) {
                                                ShowContinueError(
                                                    "Node named " + NodeID(ZoneInletConvergence(ZoneNum).InletNode(NodeIndex).NodeNum) +
                                                    " shows monotonically decreasing humidity ratio with a trend rate across iterations of " +
                                                    RoundSigDigits(SlopeHumRat, 6) + " [ kg-water/kg-dryair/iteration]");
                                            }
                                        } else { // check for monotic incrase
                                            MonotonicIncreaseFound = true;
                                            for (StackDepth = 2; StackDepth <= ConvergLogStackDepth; ++StackDepth) {
                                                if (ZoneInletConvergence(ZoneNum).InletNode(NodeIndex).HumidityRatio(StackDepth - 1) <
                                                    ZoneInletConvergence(ZoneNum).InletNode(NodeIndex).HumidityRatio(StackDepth)) {
                                                    MonotonicIncreaseFound = false;
                                                    break;
                                                }
                                            }
                                            if (MonotonicIncreaseFound) {
                                                ShowContinueError(
                                                    "Node named " + NodeID(ZoneInletConvergence(ZoneNum).InletNode(NodeIndex).NodeNum) +
                                                    " shows monotonically increasing humidity ratio with a trend rate across iterations of " +
                                                    RoundSigDigits(SlopeHumRat, 6) + " [ kg-water/kg-dryair/iteration]");
                                            }
                                        }
                                    } // significant slope in iterates
                                }     // no osciallation
                            }         // last value does not equal average of stack.

                            if (MonotonicDecreaseFound || MonotonicIncreaseFound || FoundOscillationByDuplicate) {
                                HistoryTrace = "";
                                for (StackDepth = 1; StackDepth <= ConvergLogStackDepth; ++StackDepth) {
                                    HistoryTrace +=
                                        RoundSigDigits(ZoneInletConvergence(ZoneNum).InletNode(NodeIndex).HumidityRatio(StackDepth), 6) + ',';
                                }
                                ShowContinueError(
                                    "Node named " + NodeID(ZoneInletConvergence(ZoneNum).InletNode(NodeIndex).NodeNum) +
                                    " humidity ratio [kg-water/kg-dryair] iteration history trace (most recent first): " + HistoryTrace);
                            } // need to report trace
                            // end humidity ratio

                            // Check Mass flow rate
                            FoundOscillationByDuplicate = false;
                            MonotonicDecreaseFound = false;
                            MonotonicIncreaseFound = false;
                            // check for evidence of oscillation by indentify duplicates when latest value not equal to average
                            AvgValue = sum(ZoneInletConvergence(ZoneNum).InletNode(NodeIndex).MassFlowRate) / double(ConvergLogStackDepth);
                            if (std::abs(ZoneInletConvergence(ZoneNum).InletNode(NodeIndex).MassFlowRate(1) - AvgValue) >
                                HVACFlowRateOscillationToler) { // last iterate differs from average
                                FoundOscillationByDuplicate = false;
                                for (StackDepth = 2; StackDepth <= ConvergLogStackDepth; ++StackDepth) {
                                    if (std::abs(ZoneInletConvergence(ZoneNum).InletNode(NodeIndex).MassFlowRate(1) -
                                                 ZoneInletConvergence(ZoneNum).InletNode(NodeIndex).MassFlowRate(StackDepth)) <
                                        HVACFlowRateOscillationToler) {
                                        FoundOscillationByDuplicate = true;
                                        ShowContinueError("Node named " + NodeID(ZoneInletConvergence(ZoneNum).InletNode(NodeIndex).NodeNum) +
                                                          " shows oscillating mass flow rate across iterations with a repeated value of " +
                                                          RoundSigDigits(ZoneInletConvergence(ZoneNum).InletNode(NodeIndex).MassFlowRate(1), 6));
                                        break;
                                    }
                                }
                                if (!FoundOscillationByDuplicate) {
                                    SlopeMdot = (sum_ConvergLogStackARR * sum(ZoneInletConvergence(ZoneNum).InletNode(NodeIndex).MassFlowRate) -
                                                 double(ConvergLogStackDepth) *
                                                     sum((ConvergLogStackARR * ZoneInletConvergence(ZoneNum).InletNode(NodeIndex).MassFlowRate))) /
                                                (square_sum_ConvergLogStackARR - double(ConvergLogStackDepth) * sum_square_ConvergLogStackARR);
                                    if (std::abs(SlopeMdot) > HVACFlowRateSlopeToler) {
                                        if (SlopeMdot < 0.0) { // check for monotic decrease
                                            MonotonicDecreaseFound = true;
                                            for (StackDepth = 2; StackDepth <= ConvergLogStackDepth; ++StackDepth) {
                                                if (ZoneInletConvergence(ZoneNum).InletNode(NodeIndex).MassFlowRate(StackDepth - 1) >
                                                    ZoneInletConvergence(ZoneNum).InletNode(NodeIndex).MassFlowRate(StackDepth)) {
                                                    MonotonicDecreaseFound = false;
                                                    break;
                                                }
                                            }
                                            if (MonotonicDecreaseFound) {
                                                ShowContinueError(
                                                    "Node named " + NodeID(ZoneInletConvergence(ZoneNum).InletNode(NodeIndex).NodeNum) +
                                                    " shows monotonically decreasing mass flow rate with a trend rate across iterations of " +
                                                    RoundSigDigits(SlopeMdot, 6) + " [kg/s/iteration]");
                                            }
                                        } else { // check for monotic incrase
                                            MonotonicIncreaseFound = true;
                                            for (StackDepth = 2; StackDepth <= ConvergLogStackDepth; ++StackDepth) {
                                                if (ZoneInletConvergence(ZoneNum).InletNode(NodeIndex).MassFlowRate(StackDepth - 1) <
                                                    ZoneInletConvergence(ZoneNum).InletNode(NodeIndex).MassFlowRate(StackDepth)) {
                                                    MonotonicIncreaseFound = false;
                                                    break;
                                                }
                                            }
                                            if (MonotonicIncreaseFound) {
                                                ShowContinueError(
                                                    "Node named " + NodeID(ZoneInletConvergence(ZoneNum).InletNode(NodeIndex).NodeNum) +
                                                    " shows monotonically increasing mass flow rate with a trend rate across iterations of " +
                                                    RoundSigDigits(SlopeMdot, 6) + " [kg/s/iteration]");
                                            }
                                        }
                                    } // significant slope in iterates
                                }     // no osciallation
                            }         // last value does not equal average of stack.

                            if (MonotonicDecreaseFound || MonotonicIncreaseFound || FoundOscillationByDuplicate) {
                                HistoryTrace = "";
                                for (StackDepth = 1; StackDepth <= ConvergLogStackDepth; ++StackDepth) {
                                    HistoryTrace +=
                                        RoundSigDigits(ZoneInletConvergence(ZoneNum).InletNode(NodeIndex).MassFlowRate(StackDepth), 6) + ',';
                                }
                                ShowContinueError("Node named " + NodeID(ZoneInletConvergence(ZoneNum).InletNode(NodeIndex).NodeNum) +
                                                  " mass flow rate [kg/s] iteration history trace (most recent first): " + HistoryTrace);
                            } // need to report trace
                            // end mass flow rate

                            // Check Temperatures
                            FoundOscillationByDuplicate = false;
                            MonotonicDecreaseFound = false;
                            MonotonicIncreaseFound = false;
                            // check for evidence of oscillation by indentify duplicates when latest value not equal to average
                            AvgValue = sum(ZoneInletConvergence(ZoneNum).InletNode(NodeIndex).Temperature) / double(ConvergLogStackDepth);
                            if (std::abs(ZoneInletConvergence(ZoneNum).InletNode(NodeIndex).Temperature(1) - AvgValue) >
                                HVACTemperatureOscillationToler) { // last iterate differs from average
                                FoundOscillationByDuplicate = false;
                                for (StackDepth = 2; StackDepth <= ConvergLogStackDepth; ++StackDepth) {
                                    if (std::abs(ZoneInletConvergence(ZoneNum).InletNode(NodeIndex).Temperature(1) -
                                                 ZoneInletConvergence(ZoneNum).InletNode(NodeIndex).Temperature(StackDepth)) <
                                        HVACTemperatureOscillationToler) {
                                        FoundOscillationByDuplicate = true;
                                        ShowContinueError("Node named " + NodeID(ZoneInletConvergence(ZoneNum).InletNode(NodeIndex).NodeNum) +
                                                          " shows oscillating temperatures across iterations with a repeated value of " +
                                                          RoundSigDigits(ZoneInletConvergence(ZoneNum).InletNode(NodeIndex).Temperature(1), 6));
                                        break;
                                    }
                                }
                                if (!FoundOscillationByDuplicate) {
                                    SlopeTemps = (sum_ConvergLogStackARR * sum(ZoneInletConvergence(ZoneNum).InletNode(NodeIndex).Temperature) -
                                                  double(ConvergLogStackDepth) *
                                                      sum((ConvergLogStackARR * ZoneInletConvergence(ZoneNum).InletNode(NodeIndex).Temperature))) /
                                                 (square_sum_ConvergLogStackARR - double(ConvergLogStackDepth) * sum_square_ConvergLogStackARR);
                                    if (std::abs(SlopeTemps) > HVACTemperatureSlopeToler) {
                                        if (SlopeTemps < 0.0) { // check for monotic decrease
                                            MonotonicDecreaseFound = true;
                                            for (StackDepth = 2; StackDepth <= ConvergLogStackDepth; ++StackDepth) {
                                                if (ZoneInletConvergence(ZoneNum).InletNode(NodeIndex).Temperature(StackDepth - 1) >
                                                    ZoneInletConvergence(ZoneNum).InletNode(NodeIndex).Temperature(StackDepth)) {
                                                    MonotonicDecreaseFound = false;
                                                    break;
                                                }
                                            }
                                            if (MonotonicDecreaseFound) {
                                                ShowContinueError(
                                                    "Node named " + NodeID(ZoneInletConvergence(ZoneNum).InletNode(NodeIndex).NodeNum) +
                                                    " shows monotonically decreasing temperature with a trend rate across iterations of " +
                                                    RoundSigDigits(SlopeTemps, 4) + " [C/iteration]");
                                            }
                                        } else { // check for monotic incrase
                                            MonotonicIncreaseFound = true;
                                            for (StackDepth = 2; StackDepth <= ConvergLogStackDepth; ++StackDepth) {
                                                if (ZoneInletConvergence(ZoneNum).InletNode(NodeIndex).Temperature(StackDepth - 1) <
                                                    ZoneInletConvergence(ZoneNum).InletNode(NodeIndex).Temperature(StackDepth)) {
                                                    MonotonicIncreaseFound = false;
                                                    break;
                                                }
                                            }
                                            if (MonotonicIncreaseFound) {
                                                ShowContinueError(
                                                    "Node named " + NodeID(ZoneInletConvergence(ZoneNum).InletNode(NodeIndex).NodeNum) +
                                                    " shows monotonically increasing temperatures with a trend rate across iterations of " +
                                                    RoundSigDigits(SlopeTemps, 4) + " [C/iteration]");
                                            }
                                        }
                                    } // significant slope in iterates
                                }     // no osciallation
                            }         // last value does not equal average of stack.

                            if (MonotonicDecreaseFound || MonotonicIncreaseFound || FoundOscillationByDuplicate) {
                                HistoryTrace = "";
                                for (StackDepth = 1; StackDepth <= ConvergLogStackDepth; ++StackDepth) {
                                    HistoryTrace +=
                                        RoundSigDigits(ZoneInletConvergence(ZoneNum).InletNode(NodeIndex).Temperature(StackDepth), 6) + ',';
                                }
                                ShowContinueError("Node named " + NodeID(ZoneInletConvergence(ZoneNum).InletNode(NodeIndex).NodeNum) +
                                                  " temperature [C] iteration history trace (most recent first): " + HistoryTrace);
                            } // need to report trace
                              // end Temperature checks

                        } // loop over zone inlet nodes
                    }     // loop over zones

                    for (LoopNum = 1; LoopNum <= TotNumLoops; ++LoopNum) {

                        if (PlantConvergence(LoopNum).PlantMassFlowNotConverged) {
                            ShowContinueError("Plant System Named = " + PlantLoop(LoopNum).Name + " did not converge for mass flow rate");
                            ShowContinueError("Check values should be zero. Most Recent values listed first.");
                            HistoryTrace = "";
                            for (StackDepth = 1; StackDepth <= ConvergLogStackDepth; ++StackDepth) {
                                HistoryTrace += RoundSigDigits(PlantConvergence(LoopNum).PlantFlowDemandToSupplyTolValue(StackDepth), 6) + ',';
                            }
                            ShowContinueError("Demand-to-Supply interface mass flow rate check value iteration history trace: " + HistoryTrace);
                            HistoryTrace = "";
                            for (StackDepth = 1; StackDepth <= ConvergLogStackDepth; ++StackDepth) {
                                HistoryTrace += RoundSigDigits(PlantConvergence(LoopNum).PlantFlowSupplyToDemandTolValue(StackDepth), 6) + ',';
                            }
                            ShowContinueError("Supply-to-Demand interface mass flow rate check value iteration history trace: " + HistoryTrace);

                            // now work with history logs for mass flow to detect issues
                            for (ThisLoopSide = 1; ThisLoopSide <= isize(PlantLoop(LoopNum).LoopSide); ++ThisLoopSide) {
                                // loop side inlet node
                                FoundOscillationByDuplicate = false;
                                MonotonicDecreaseFound = false;
                                MonotonicIncreaseFound = false;
                                AvgValue =
                                    sum(PlantLoop(LoopNum).LoopSide(ThisLoopSide).InletNode.MassFlowRateHistory) / double(NumConvergenceHistoryTerms);
                                if (std::abs(PlantLoop(LoopNum).LoopSide(ThisLoopSide).InletNode.MassFlowRateHistory(1) - AvgValue) >
                                    PlantFlowRateOscillationToler) {
                                    FoundOscillationByDuplicate = false;
                                    for (StackDepth = 2; StackDepth <= NumConvergenceHistoryTerms; ++StackDepth) {
                                        if (std::abs(PlantLoop(LoopNum).LoopSide(ThisLoopSide).InletNode.MassFlowRateHistory(1) -
                                                     PlantLoop(LoopNum).LoopSide(ThisLoopSide).InletNode.MassFlowRateHistory(StackDepth)) <
                                            PlantFlowRateOscillationToler) {
                                            FoundOscillationByDuplicate = true;
                                            ShowContinueError(
                                                "Node named " + PlantLoop(LoopNum).LoopSide(ThisLoopSide).NodeNameIn +
                                                " shows oscillating flow rates across iterations with a repeated value of " +
                                                RoundSigDigits(PlantLoop(LoopNum).LoopSide(ThisLoopSide).InletNode.MassFlowRateHistory(1), 7));
                                            break;
                                        }
                                    }
                                }
                                if (!FoundOscillationByDuplicate) {
                                    SlopeMdot =
                                        (sum_ConvergenceHistoryARR * sum(PlantLoop(LoopNum).LoopSide(ThisLoopSide).InletNode.MassFlowRateHistory) -
                                         double(NumConvergenceHistoryTerms) *
                                             sum((ConvergenceHistoryARR * PlantLoop(LoopNum).LoopSide(ThisLoopSide).InletNode.MassFlowRateHistory))) /
                                        (square_sum_ConvergenceHistoryARR - double(NumConvergenceHistoryTerms) * sum_square_ConvergenceHistoryARR);
                                    if (std::abs(SlopeMdot) > PlantFlowRateSlopeToler) {
                                        if (SlopeMdot < 0.0) { // check for monotonic decrease
                                            MonotonicDecreaseFound = true;
                                            for (StackDepth = 2; StackDepth <= NumConvergenceHistoryTerms; ++StackDepth) {
                                                if (PlantLoop(LoopNum).LoopSide(ThisLoopSide).InletNode.MassFlowRateHistory(StackDepth - 1) >
                                                    PlantLoop(LoopNum).LoopSide(ThisLoopSide).InletNode.MassFlowRateHistory(StackDepth)) {
                                                    MonotonicDecreaseFound = false;
                                                    break;
                                                }
                                            }
                                            if (MonotonicDecreaseFound) {
                                                ShowContinueError(
                                                    "Node named " + PlantLoop(LoopNum).LoopSide(ThisLoopSide).NodeNameIn +
                                                    " shows monotonically decreasing mass flow rate with a trend rate across iterations of " +
                                                    RoundSigDigits(SlopeMdot, 7) + " [kg/s/iteration]");
                                            }
                                        } else { // check for monotonic incrase
                                            MonotonicIncreaseFound = true;
                                            for (StackDepth = 2; StackDepth <= NumConvergenceHistoryTerms; ++StackDepth) {
                                                if (PlantLoop(LoopNum).LoopSide(ThisLoopSide).InletNode.MassFlowRateHistory(StackDepth - 1) <
                                                    PlantLoop(LoopNum).LoopSide(ThisLoopSide).InletNode.MassFlowRateHistory(StackDepth)) {
                                                    MonotonicIncreaseFound = false;
                                                    break;
                                                }
                                            }
                                            if (MonotonicIncreaseFound) {
                                                ShowContinueError(
                                                    "Node named " + PlantLoop(LoopNum).LoopSide(ThisLoopSide).NodeNameIn +
                                                    " shows monotonically increasing mass flow rate with a trend rate across iterations of " +
                                                    RoundSigDigits(SlopeMdot, 7) + " [kg/s/iteration]");
                                            }
                                        }
                                    } // significant slope found
                                }     // no oscillation found

                                if (MonotonicDecreaseFound || MonotonicIncreaseFound || FoundOscillationByDuplicate) {
                                    HistoryTrace = "";
                                    for (StackDepth = 1; StackDepth <= NumConvergenceHistoryTerms; ++StackDepth) {
                                        HistoryTrace +=
                                            RoundSigDigits(PlantLoop(LoopNum).LoopSide(ThisLoopSide).InletNode.MassFlowRateHistory(StackDepth), 7) +
                                            ',';
                                    }
                                    ShowContinueError("Node named " + PlantLoop(LoopNum).LoopSide(ThisLoopSide).NodeNameIn +
                                                      " mass flow rate [kg/s] iteration history trace (most recent first): " + HistoryTrace);
                                } // need to report trace
                                // end of inlet node

                                // loop side outlet node
                                FoundOscillationByDuplicate = false;
                                MonotonicDecreaseFound = false;
                                MonotonicIncreaseFound = false;
                                AvgValue = sum(PlantLoop(LoopNum).LoopSide(ThisLoopSide).OutletNode.MassFlowRateHistory) /
                                           double(NumConvergenceHistoryTerms);
                                if (std::abs(PlantLoop(LoopNum).LoopSide(ThisLoopSide).OutletNode.MassFlowRateHistory(1) - AvgValue) >
                                    PlantFlowRateOscillationToler) {
                                    FoundOscillationByDuplicate = false;
                                    for (StackDepth = 2; StackDepth <= NumConvergenceHistoryTerms; ++StackDepth) {
                                        if (std::abs(PlantLoop(LoopNum).LoopSide(ThisLoopSide).OutletNode.MassFlowRateHistory(1) -
                                                     PlantLoop(LoopNum).LoopSide(ThisLoopSide).OutletNode.MassFlowRateHistory(StackDepth)) <
                                            PlantFlowRateOscillationToler) {
                                            FoundOscillationByDuplicate = true;
                                            ShowContinueError(
                                                "Node named " + PlantLoop(LoopNum).LoopSide(ThisLoopSide).NodeNameOut +
                                                " shows oscillating flow rates across iterations with a repeated value of " +
                                                RoundSigDigits(PlantLoop(LoopNum).LoopSide(ThisLoopSide).OutletNode.MassFlowRateHistory(1), 7));
                                            break;
                                        }
                                    }
                                }
                                if (!FoundOscillationByDuplicate) {
                                    SlopeMdot =
                                        (sum_ConvergenceHistoryARR * sum(PlantLoop(LoopNum).LoopSide(ThisLoopSide).OutletNode.MassFlowRateHistory) -
                                         double(NumConvergenceHistoryTerms) *
                                             sum((ConvergenceHistoryARR *
                                                  PlantLoop(LoopNum).LoopSide(ThisLoopSide).OutletNode.MassFlowRateHistory))) /
                                        (square_sum_ConvergenceHistoryARR - double(NumConvergenceHistoryTerms) * sum_square_ConvergenceHistoryARR);
                                    if (std::abs(SlopeMdot) > PlantFlowRateSlopeToler) {
                                        if (SlopeMdot < 0.0) { // check for monotonic decrease
                                            MonotonicDecreaseFound = true;
                                            for (StackDepth = 2; StackDepth <= NumConvergenceHistoryTerms; ++StackDepth) {
                                                if (PlantLoop(LoopNum).LoopSide(ThisLoopSide).OutletNode.MassFlowRateHistory(StackDepth - 1) >
                                                    PlantLoop(LoopNum).LoopSide(ThisLoopSide).OutletNode.MassFlowRateHistory(StackDepth)) {
                                                    MonotonicDecreaseFound = false;
                                                    break;
                                                }
                                            }
                                            if (MonotonicDecreaseFound) {
                                                ShowContinueError(
                                                    "Node named " + PlantLoop(LoopNum).LoopSide(ThisLoopSide).NodeNameOut +
                                                    " shows monotonically decreasing mass flow rate with a trend rate across iterations of " +
                                                    RoundSigDigits(SlopeMdot, 7) + " [kg/s/iteration]");
                                            }
                                        } else { // check for monotonic incrase
                                            MonotonicIncreaseFound = true;
                                            for (StackDepth = 2; StackDepth <= NumConvergenceHistoryTerms; ++StackDepth) {
                                                if (PlantLoop(LoopNum).LoopSide(ThisLoopSide).OutletNode.MassFlowRateHistory(StackDepth - 1) <
                                                    PlantLoop(LoopNum).LoopSide(ThisLoopSide).OutletNode.MassFlowRateHistory(StackDepth)) {
                                                    MonotonicIncreaseFound = false;
                                                    break;
                                                }
                                            }
                                            if (MonotonicIncreaseFound) {
                                                ShowContinueError(
                                                    "Node named " + PlantLoop(LoopNum).LoopSide(ThisLoopSide).NodeNameOut +
                                                    " shows monotonically increasing mass flow rate with a trend rate across iterations of " +
                                                    RoundSigDigits(SlopeMdot, 7) + " [kg/s/iteration]");
                                            }
                                        }
                                    } // significant slope found
                                }     // no oscillation found

                                if (MonotonicDecreaseFound || MonotonicIncreaseFound || FoundOscillationByDuplicate) {
                                    HistoryTrace = "";
                                    for (StackDepth = 1; StackDepth <= NumConvergenceHistoryTerms; ++StackDepth) {
                                        HistoryTrace +=
                                            RoundSigDigits(PlantLoop(LoopNum).LoopSide(ThisLoopSide).OutletNode.MassFlowRateHistory(StackDepth), 7) +
                                            ',';
                                    }
                                    ShowContinueError("Node named " + PlantLoop(LoopNum).LoopSide(ThisLoopSide).NodeNameOut +
                                                      " mass flow rate [kg/s] iteration history trace (most recent first): " + HistoryTrace);
                                } // need to report trace
                                  // end of Outlet node

                            } // plant loop sides

                        } // mass flow not converged

                        if (PlantConvergence(LoopNum).PlantTempNotConverged) {
                            ShowContinueError("Plant System Named = " + PlantLoop(LoopNum).Name + " did not converge for temperature");
                            ShowContinueError("Check values should be zero. Most Recent values listed first.");
                            HistoryTrace = "";
                            for (StackDepth = 1; StackDepth <= ConvergLogStackDepth; ++StackDepth) {
                                HistoryTrace += RoundSigDigits(PlantConvergence(LoopNum).PlantTempDemandToSupplyTolValue(StackDepth), 6) + ',';
                            }
                            ShowContinueError("Demand-to-Supply interface temperature check value iteration history trace: " + HistoryTrace);
                            HistoryTrace = "";
                            for (StackDepth = 1; StackDepth <= ConvergLogStackDepth; ++StackDepth) {
                                HistoryTrace += RoundSigDigits(PlantConvergence(LoopNum).PlantTempSupplyToDemandTolValue(StackDepth), 6) + ',';
                            }
                            ShowContinueError("Supply-to-Demand interface temperature check value iteration history trace: " + HistoryTrace);

                            // now work with history logs for mass flow to detect issues
                            for (ThisLoopSide = 1; ThisLoopSide <= isize(PlantLoop(LoopNum).LoopSide); ++ThisLoopSide) {
                                // loop side inlet node
                                FoundOscillationByDuplicate = false;
                                MonotonicDecreaseFound = false;
                                MonotonicIncreaseFound = false;
                                AvgValue =
                                    sum(PlantLoop(LoopNum).LoopSide(ThisLoopSide).InletNode.TemperatureHistory) / double(NumConvergenceHistoryTerms);
                                if (std::abs(PlantLoop(LoopNum).LoopSide(ThisLoopSide).InletNode.TemperatureHistory(1) - AvgValue) >
                                    PlantTemperatureOscillationToler) {
                                    FoundOscillationByDuplicate = false;
                                    for (StackDepth = 2; StackDepth <= NumConvergenceHistoryTerms; ++StackDepth) {
                                        if (std::abs(PlantLoop(LoopNum).LoopSide(ThisLoopSide).InletNode.TemperatureHistory(1) -
                                                     PlantLoop(LoopNum).LoopSide(ThisLoopSide).InletNode.TemperatureHistory(StackDepth)) <
                                            PlantTemperatureOscillationToler) {
                                            FoundOscillationByDuplicate = true;
                                            ShowContinueError(
                                                "Node named " + PlantLoop(LoopNum).LoopSide(ThisLoopSide).NodeNameIn +
                                                " shows oscillating temperatures across iterations with a repeated value of " +
                                                RoundSigDigits(PlantLoop(LoopNum).LoopSide(ThisLoopSide).InletNode.TemperatureHistory(1), 5));
                                            break;
                                        }
                                    }
                                }
                                if (!FoundOscillationByDuplicate) {
                                    SlopeTemps =
                                        (sum_ConvergenceHistoryARR * sum(PlantLoop(LoopNum).LoopSide(ThisLoopSide).InletNode.TemperatureHistory) -
                                         double(NumConvergenceHistoryTerms) *
                                             sum((ConvergenceHistoryARR * PlantLoop(LoopNum).LoopSide(ThisLoopSide).InletNode.TemperatureHistory))) /
                                        (square_sum_ConvergenceHistoryARR - double(NumConvergenceHistoryTerms) * sum_square_ConvergenceHistoryARR);
                                    if (std::abs(SlopeTemps) > PlantTemperatureSlopeToler) {
                                        if (SlopeTemps < 0.0) { // check for monotic decrease
                                            MonotonicDecreaseFound = true;
                                            for (StackDepth = 2; StackDepth <= NumConvergenceHistoryTerms; ++StackDepth) {
                                                if (PlantLoop(LoopNum).LoopSide(ThisLoopSide).InletNode.TemperatureHistory(StackDepth - 1) >
                                                    PlantLoop(LoopNum).LoopSide(ThisLoopSide).InletNode.TemperatureHistory(StackDepth)) {
                                                    MonotonicDecreaseFound = false;
                                                    break;
                                                }
                                            }
                                            if (MonotonicDecreaseFound) {
                                                ShowContinueError(
                                                    "Node named " + PlantLoop(LoopNum).LoopSide(ThisLoopSide).NodeNameIn +
                                                    " shows monotonically decreasing temperatures with a trend rate across iterations of " +
                                                    RoundSigDigits(SlopeTemps, 5) + " [C/iteration]");
                                            }
                                        } else { // check for monotic incrase
                                            MonotonicIncreaseFound = true;
                                            for (StackDepth = 2; StackDepth <= NumConvergenceHistoryTerms; ++StackDepth) {
                                                if (PlantLoop(LoopNum).LoopSide(ThisLoopSide).InletNode.TemperatureHistory(StackDepth - 1) <
                                                    PlantLoop(LoopNum).LoopSide(ThisLoopSide).InletNode.TemperatureHistory(StackDepth)) {
                                                    MonotonicIncreaseFound = false;
                                                    break;
                                                }
                                            }
                                            if (MonotonicIncreaseFound) {
                                                ShowContinueError(
                                                    "Node named " + PlantLoop(LoopNum).LoopSide(ThisLoopSide).NodeNameIn +
                                                    " shows monotonically increasing temperatures with a trend rate across iterations of " +
                                                    RoundSigDigits(SlopeTemps, 5) + " [C/iteration]");
                                            }
                                        }
                                    } // significant slope found
                                }     // no oscillation found

                                if (MonotonicDecreaseFound || MonotonicIncreaseFound || FoundOscillationByDuplicate) {
                                    HistoryTrace = "";
                                    for (StackDepth = 1; StackDepth <= NumConvergenceHistoryTerms; ++StackDepth) {
                                        HistoryTrace +=
                                            RoundSigDigits(PlantLoop(LoopNum).LoopSide(ThisLoopSide).InletNode.TemperatureHistory(StackDepth), 5) +
                                            ',';
                                    }
                                    ShowContinueError("Node named " + PlantLoop(LoopNum).LoopSide(ThisLoopSide).NodeNameIn +
                                                      " temperature [C] iteration history trace (most recent first): " + HistoryTrace);
                                } // need to report trace
                                // end of inlet node

                                // loop side outlet node
                                FoundOscillationByDuplicate = false;
                                MonotonicDecreaseFound = false;
                                MonotonicIncreaseFound = false;
                                AvgValue =
                                    sum(PlantLoop(LoopNum).LoopSide(ThisLoopSide).OutletNode.TemperatureHistory) / double(NumConvergenceHistoryTerms);
                                if (std::abs(PlantLoop(LoopNum).LoopSide(ThisLoopSide).OutletNode.TemperatureHistory(1) - AvgValue) >
                                    PlantTemperatureOscillationToler) {
                                    FoundOscillationByDuplicate = false;
                                    for (StackDepth = 2; StackDepth <= NumConvergenceHistoryTerms; ++StackDepth) {
                                        if (std::abs(PlantLoop(LoopNum).LoopSide(ThisLoopSide).OutletNode.TemperatureHistory(1) -
                                                     PlantLoop(LoopNum).LoopSide(ThisLoopSide).OutletNode.TemperatureHistory(StackDepth)) <
                                            PlantTemperatureOscillationToler) {
                                            FoundOscillationByDuplicate = true;
                                            ShowContinueError(
                                                "Node named " + PlantLoop(LoopNum).LoopSide(ThisLoopSide).NodeNameOut +
                                                " shows oscillating temperatures across iterations with a repeated value of " +
                                                RoundSigDigits(PlantLoop(LoopNum).LoopSide(ThisLoopSide).OutletNode.TemperatureHistory(1), 5));
                                            break;
                                        }
                                    }
                                }
                                if (!FoundOscillationByDuplicate) {
                                    SlopeTemps =
                                        (sum_ConvergenceHistoryARR * sum(PlantLoop(LoopNum).LoopSide(ThisLoopSide).OutletNode.TemperatureHistory) -
                                         double(NumConvergenceHistoryTerms) *
                                             sum((ConvergenceHistoryARR * PlantLoop(LoopNum).LoopSide(ThisLoopSide).OutletNode.TemperatureHistory))) /
                                        (square_sum_ConvergenceHistoryARR - double(NumConvergenceHistoryTerms) * sum_square_ConvergenceHistoryARR);
                                    if (std::abs(SlopeTemps) > PlantFlowRateSlopeToler) {
                                        if (SlopeTemps < 0.0) { // check for monotic decrease
                                            MonotonicDecreaseFound = true;
                                            for (StackDepth = 2; StackDepth <= NumConvergenceHistoryTerms; ++StackDepth) {
                                                if (PlantLoop(LoopNum).LoopSide(ThisLoopSide).OutletNode.TemperatureHistory(StackDepth - 1) >
                                                    PlantLoop(LoopNum).LoopSide(ThisLoopSide).OutletNode.TemperatureHistory(StackDepth)) {
                                                    MonotonicDecreaseFound = false;
                                                    break;
                                                }
                                            }
                                            if (MonotonicDecreaseFound) {
                                                ShowContinueError(
                                                    "Node named " + PlantLoop(LoopNum).LoopSide(ThisLoopSide).NodeNameOut +
                                                    " shows monotonically decreasing temperatures with a trend rate across iterations of " +
                                                    RoundSigDigits(SlopeTemps, 5) + " [C/iteration]");
                                            }
                                        } else { // check for monotic incrase
                                            MonotonicIncreaseFound = true;
                                            for (StackDepth = 2; StackDepth <= NumConvergenceHistoryTerms; ++StackDepth) {
                                                if (PlantLoop(LoopNum).LoopSide(ThisLoopSide).OutletNode.TemperatureHistory(StackDepth - 1) <
                                                    PlantLoop(LoopNum).LoopSide(ThisLoopSide).OutletNode.TemperatureHistory(StackDepth)) {
                                                    MonotonicIncreaseFound = false;
                                                    break;
                                                }
                                            }
                                            if (MonotonicIncreaseFound) {
                                                ShowContinueError(
                                                    "Node named " + PlantLoop(LoopNum).LoopSide(ThisLoopSide).NodeNameOut +
                                                    " shows monotonically increasing temperatures with a trend rate across iterations of " +
                                                    RoundSigDigits(SlopeTemps, 5) + " [C/iteration]");
                                            }
                                        }
                                    } // significant slope found
                                }     // no oscillation found

                                if (MonotonicDecreaseFound || MonotonicIncreaseFound || FoundOscillationByDuplicate) {
                                    HistoryTrace = "";
                                    for (StackDepth = 1; StackDepth <= NumConvergenceHistoryTerms; ++StackDepth) {
                                        HistoryTrace +=
                                            RoundSigDigits(PlantLoop(LoopNum).LoopSide(ThisLoopSide).OutletNode.TemperatureHistory(StackDepth), 5) +
                                            ',';
                                    }
                                    ShowContinueError("Node named " + PlantLoop(LoopNum).LoopSide(ThisLoopSide).NodeNameOut +
                                                      " temperature [C] iteration history trace (most recent first): " + HistoryTrace);
                                } // need to report trace
                                  // end of Outlet node

                            } // plant loop sides

                        } // temperature not converged
                    }     // loop over plant loop systems
                }
            } else {
                if (EnvironmentName == ErrEnvironmentName) {
                    ShowRecurringWarningErrorAtEnd(
                        "SimHVAC: Exceeding Maximum iterations for all HVAC loops, during " + EnvironmentName + " continues", MaxErrCount);
                } else {
                    MaxErrCount = 0;
                    ErrEnvironmentName = EnvironmentName;
                    ShowRecurringWarningErrorAtEnd(
                        "SimHVAC: Exceeding Maximum iterations for all HVAC loops, during " + EnvironmentName + " continues", MaxErrCount);
                }
            }
        }

        CheckAirLoopFlowBalance(state);

        // Set node setpoints to a flag value so that controllers can check whether their sensed nodes
        // have a setpoint
        if (!ZoneSizingCalc && !SysSizingCalc) {
            if (MySetPointInit) {
                if (NumOfNodes > 0) {
                    for (auto &e : Node) {
                        e.TempSetPoint = SensedNodeFlagValue;
                        e.HumRatSetPoint = SensedNodeFlagValue;
                        e.HumRatMin = SensedNodeFlagValue;
                        e.HumRatMax = SensedNodeFlagValue;
                        e.MassFlowRateSetPoint = SensedNodeFlagValue; // BG 5-26-2009 (being checked in HVACControllers.cc)
                    }
                    DefaultNodeValues.TempSetPoint = SensedNodeFlagValue;
                    DefaultNodeValues.HumRatSetPoint = SensedNodeFlagValue;
                    DefaultNodeValues.HumRatMin = SensedNodeFlagValue;
                    DefaultNodeValues.HumRatMax = SensedNodeFlagValue;
                    DefaultNodeValues.MassFlowRateSetPoint = SensedNodeFlagValue; // BG 5-26-2009 (being checked in HVACControllers.cc)
                }
                MySetPointInit = false;
                DoSetPointTest = true;
            } else {
                DoSetPointTest = false;
            }
        }
        if (SetPointErrorFlag) {
            ShowFatalError("Previous severe set point errors cause program termination");
        }
    }

    void SimSelectedEquipment(EnergyPlusData &state, bool &SimAirLoops,         // True when the air loops need to be (re)simulated
                              bool &SimZoneEquipment,    // True when zone equipment components need to be (re)simulated
                              bool &SimNonZoneEquipment, // True when non-zone equipment components need to be (re)simulated
                              bool &SimPlantLoops,       // True when the main plant loops need to be (re)simulated
                              bool &SimElecCircuits,     // True when electric circuits need to be (re)simulated
                              bool &FirstHVACIteration,  // True when solution technique on first iteration
                              bool const LockPlantFlows)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Russ Taylor, Rick Strand
        //       DATE WRITTEN   May 1998
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine receives the flags from SimHVAC which determines
        // which middle-level managers must be called.

        // METHODOLOGY EMPLOYED:
        // Each flag is checked and the appropriate manager is then called.

        // REFERENCES:
        // na

        // Using/Aliasing
        using AirflowNetworkBalanceManager::ManageAirflowNetworkBalance;
        using DataErrorTracking::AskForPlantCheckOnAbort;
        using DataPlant::FlowLocked;
        using DataPlant::FlowUnlocked;
        using NonZoneEquipmentManager::ManageNonZoneEquipment;
        using PlantManager::ManagePlantLoops;
        using PlantUtilities::AnyPlantLoopSidesNeedSim;
        using PlantUtilities::ResetAllPlantInterConnectFlags;
        using PlantUtilities::SetAllFlowLocks;
        using SimAirServingZones::ManageAirLoops;
        using ZoneEquipmentManager::ManageZoneEquipment;

        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:
        bool ResimulateAirZone; // True when solution technique on third iteration used in AirflowNetwork

        // SUBROUTINE PARAMETER DEFINITIONS:
        int const MaxAir(5); // Iteration Max for Air Simulation Iterations

        // INTERFACE BLOCK SPECIFICATIONS:
        // na

        // DERIVED TYPE DEFINITIONS:
        // na

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int IterAir; // counts iterations to enforce maximum iteration limit

        // FLOW:

        IterAir = 0;

        // Set all plant flow locks to UNLOCKED to allow air side components to operate properly
        // This requires that the plant flow resolver carefully set the min/max avail limits on
        //  air side components to ensure they request within bounds.
        if (LockPlantFlows) {
            SetAllFlowLocks(FlowLocked);
        } else {
            SetAllFlowLocks(FlowUnlocked);
        }
        ResetAllPlantInterConnectFlags();

        if (BeginEnvrnFlag && MyEnvrnFlag2) {
            // Following comment is incorrect!  (LKL) Even the first time through this does more than read in data.
            // Zone equipment data needs to be read in before air loop data to allow the
            // determination of which zones are connected to which air loops.
            // This call of ManageZoneEquipment does nothing except force the
            // zone equipment data to be read in.
            ManageZoneEquipment(state, FirstHVACIteration, SimZoneEquipment, SimAirLoops);
            MyEnvrnFlag2 = false;
        }
        if (!BeginEnvrnFlag) {
            MyEnvrnFlag2 = true;
        }

        if (FirstHVACIteration) {
            RepIterAir = 0;
            // Call AirflowNetwork simulation to calculate air flows and pressures
            if (AirflowNetwork::SimulateAirflowNetwork > AirflowNetwork::AirflowNetworkControlSimple) {
                ManageAirflowNetworkBalance(state, FirstHVACIteration);
            }
            ManageAirLoops(state, FirstHVACIteration, SimAirLoops, SimZoneEquipment);
            state.dataAirLoop->AirLoopInputsFilled = true; // all air loop inputs have been read in
            SimAirLoops = true;         // Need to make sure that SimAirLoop is simulated at min twice to calculate PLR in some air loop equipment
            AirLoopsSimOnce = true;     // air loops simulated once for this environment
            ResetTerminalUnitFlowLimits(state);
            FlowMaxAvailAlreadyReset = true;
            ManageZoneEquipment(state, FirstHVACIteration, SimZoneEquipment, SimAirLoops);
            SimZoneEquipment = true; // needs to be simulated at least twice for flow resolution to propagate to this routine
            ManageNonZoneEquipment(state, FirstHVACIteration, SimNonZoneEquipment);
            facilityElectricServiceObj->manageElectricPowerService(state, FirstHVACIteration, SimElecCircuitsFlag, false);

            ManagePlantLoops(state, FirstHVACIteration, SimAirLoops, SimZoneEquipment, SimNonZoneEquipment, SimPlantLoops, SimElecCircuits);

            AskForPlantCheckOnAbort = true; // need to make a first pass through plant calcs before this check make sense
            facilityElectricServiceObj->manageElectricPowerService(state, FirstHVACIteration, SimElecCircuitsFlag, false);
        } else {
            FlowResolutionNeeded = false;
            while ((SimAirLoops || SimZoneEquipment) && (IterAir <= MaxAir)) {
                ++IterAir; // Increment the iteration counter
                // Call AirflowNetwork simulation to calculate air flows and pressures
                ResimulateAirZone = false;
                if (AirflowNetwork::SimulateAirflowNetwork > AirflowNetwork::AirflowNetworkControlSimple) {
                    ManageAirflowNetworkBalance(state, FirstHVACIteration, IterAir, ResimulateAirZone);
                }
                if (SimAirLoops) {
                    ManageAirLoops(state, FirstHVACIteration, SimAirLoops, SimZoneEquipment);
                    SimElecCircuits = true; // If this was simulated there are possible electric changes that need to be simulated
                }

                // make sure flow resolution gets done
                if (FlowResolutionNeeded) {
                    SimZoneEquipment = true;
                }
                if (SimZoneEquipment) {
                    if ((IterAir == 1) && (!FlowMaxAvailAlreadyReset)) { // don't do reset if already done in FirstHVACIteration
                        // ResetTerminalUnitFlowLimits(); // don't do reset at all - interferes with convergence and terminal unit flow controls
                        FlowResolutionNeeded = true;
                    } else {
                        ResolveAirLoopFlowLimits(state);
                        FlowResolutionNeeded = false;
                    }
                    ManageZoneEquipment(state, FirstHVACIteration, SimZoneEquipment, SimAirLoops);
                    SimElecCircuits = true; // If this was simulated there are possible electric changes that need to be simulated
                }
                FlowMaxAvailAlreadyReset = false;

                //      IterAir = IterAir + 1   ! Increment the iteration counter
                if (AirflowNetwork::SimulateAirflowNetwork > AirflowNetwork::AirflowNetworkControlSimple) {
                    if (ResimulateAirZone) { // Need to make sure that SimAirLoop and SimZoneEquipment are simulated
                        SimAirLoops = true;  // at min three times using ONOFF fan with the AirflowNetwork model
                        SimZoneEquipment = true;
                    }
                }
            }

            RepIterAir += IterAir;
            if (IterAir > MaxAir) {
                AirLoopConvergFail = 1;
            } else {
                AirLoopConvergFail = 0;
            }
            // Check to see if any components have been locked out. If so, SimAirLoops will be reset to TRUE.
            ResolveLockoutFlags(state, SimAirLoops);

            if (SimNonZoneEquipment) {
                ManageNonZoneEquipment(state, FirstHVACIteration, SimNonZoneEquipment);
                SimElecCircuits = true; // If this was simulated there are possible electric changes that need to be simulated
            }

            if (SimElecCircuits) {
                facilityElectricServiceObj->manageElectricPowerService(state, FirstHVACIteration, SimElecCircuitsFlag, false);
            }

            if (!SimPlantLoops) {
                // check to see if any air side component may have requested plant resim
                if (AnyPlantLoopSidesNeedSim()) {
                    SimPlantLoops = true;
                }
            }

            if (SimPlantLoops) {
                ManagePlantLoops(state, FirstHVACIteration, SimAirLoops, SimZoneEquipment, SimNonZoneEquipment, SimPlantLoops, SimElecCircuits);
            }

            if (SimElecCircuits) {
                facilityElectricServiceObj->manageElectricPowerService(state, FirstHVACIteration, SimElecCircuitsFlag, false);
            }
        }
    }

    void ResetTerminalUnitFlowLimits(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Fred Buhl
        //       DATE WRITTEN   Feb 2010
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Reset the max flow available limits at the inlet nodes of terminal units

        // METHODOLOGY EMPLOYED:
        // Loops through all air loops, finds the inlet nodes of the terminal units
        // served by each air loop, and resets the node MassFlowRateMaxAvail (and MinAvail) to
        // the hard max and mins.

        // REFERENCES:
        // na

        // Using/Aliasing

        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:

        // SUBROUTINE PARAMETER DEFINITIONS:
        // na

        // INTERFACE BLOCK SPECIFICATIONS:
        // na

        // DERIVED TYPE DEFINITIONS:
        // na

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int AirLoopIndex;
        int ZonesCooledIndex;
        int ZonesHeatedIndex;
        int TermInletNode;

        for (AirLoopIndex = 1; AirLoopIndex <= NumPrimaryAirSys; ++AirLoopIndex) { // loop over the primary air loops
            for (ZonesCooledIndex = 1; ZonesCooledIndex <= state.dataAirLoop->AirToZoneNodeInfo(AirLoopIndex).NumZonesCooled;
                 ++ZonesCooledIndex) { // loop over the zones cooled by this air loop
                TermInletNode = state.dataAirLoop->AirToZoneNodeInfo(AirLoopIndex).TermUnitCoolInletNodes(ZonesCooledIndex);
                // reset the max avail flow rate at the terminal unit cold air inlet to the max
                Node(TermInletNode).MassFlowRateMaxAvail = Node(TermInletNode).MassFlowRateMax;
                Node(TermInletNode).MassFlowRateMinAvail = Node(TermInletNode).MassFlowRateMin;
            }
            for (ZonesHeatedIndex = 1; ZonesHeatedIndex <= state.dataAirLoop->AirToZoneNodeInfo(AirLoopIndex).NumZonesHeated;
                 ++ZonesHeatedIndex) { // loop over the zones heated by this air loop
                TermInletNode = state.dataAirLoop->AirToZoneNodeInfo(AirLoopIndex).TermUnitHeatInletNodes(ZonesHeatedIndex);
                // reset the max avail flow rate at the terminal unit hot air inlet to the max
                Node(TermInletNode).MassFlowRateMaxAvail = Node(TermInletNode).MassFlowRateMax;
                Node(TermInletNode).MassFlowRateMinAvail = Node(TermInletNode).MassFlowRateMin;
            }
        }
    }

    void ResolveAirLoopFlowLimits(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Fred Buhl
        //       DATE WRITTEN   August 2003
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine is for resolving hard flow mismatches between zone equipment and
        // the primary air loop. Such a mismatch can occur when the air terminal units are
        // requsting more air than the central air system can supply.

        // METHODOLOGY EMPLOYED:
        // Sets the MassFlowRateMaxAvail on the terminal unit inlet nodes to match the
        // maximum available from the primary air loop.

        // REFERENCES:
        // na

        // Using/Aliasing
        using DataConvergParams::HVACFlowRateToler;

        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:

        // SUBROUTINE PARAMETER DEFINITIONS:
        // na

        // INTERFACE BLOCK SPECIFICATIONS:
        // na

        // DERIVED TYPE DEFINITIONS:
        // na

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int AirLoopIndex;
        int ZonesCooledIndex;
        int ZonesHeatedIndex;
        int TermInletNode;
        int SupplyIndex;
        int SupplyNode;
        Real64 FlowRatio;

        for (AirLoopIndex = 1; AirLoopIndex <= NumPrimaryAirSys; ++AirLoopIndex) { // loop over the primary air loops
            for (SupplyIndex = 1; SupplyIndex <= state.dataAirLoop->AirToZoneNodeInfo(AirLoopIndex).NumSupplyNodes;
                 ++SupplyIndex) {                                                             // loop over the air loop supply outlets
                if (state.dataAirLoop->AirToZoneNodeInfo(AirLoopIndex).SupplyDuctType(SupplyIndex) == Cooling) { // check for cooling duct
                    // check if terminal units requesting more air than air loop can supply; if so, set terminal unit inlet
                    // node mass flow max avail to what air loop can supply
                    SupplyNode = state.dataAirLoop->AirToZoneNodeInfo(AirLoopIndex).AirLoopSupplyNodeNum(SupplyIndex);
                    if (Node(SupplyNode).MassFlowRate > 0.0) {
                        // must include bypass flow for ChangeoverBypass system so that terminal units are not restricted (e.g., MaxAvail is lowered)
                        if ((Node(SupplyNode).MassFlowRateSetPoint - Node(SupplyNode).MassFlowRate - state.dataAirLoop->AirLoopFlow(AirLoopIndex).BypassMassFlow) >
                            HVACFlowRateToler * 0.01) {
                            FlowRatio = Node(SupplyNode).MassFlowRate / Node(SupplyNode).MassFlowRateSetPoint;
                            for (ZonesCooledIndex = 1; ZonesCooledIndex <= state.dataAirLoop->AirToZoneNodeInfo(AirLoopIndex).NumZonesCooled; ++ZonesCooledIndex) {
                                TermInletNode = state.dataAirLoop->AirToZoneNodeInfo(AirLoopIndex).TermUnitCoolInletNodes(ZonesCooledIndex);
                                Node(TermInletNode).MassFlowRateMaxAvail = Node(TermInletNode).MassFlowRate * FlowRatio;
                                Node(TermInletNode).MassFlowRateMinAvail =
                                    min(Node(TermInletNode).MassFlowRateMaxAvail, Node(TermInletNode).MassFlowRateMinAvail);
                            }
                        }
                        if ((Node(SupplyNode).MassFlowRateSetPoint - Node(SupplyNode).MassFlowRate - state.dataAirLoop->AirLoopFlow(AirLoopIndex).BypassMassFlow) <
                            -HVACFlowRateToler * 0.01) {
                            if (Node(SupplyNode).MassFlowRateSetPoint == 0.0) {
                                //               CALL ShowFatalError('ResolveAirLoopFlowLimits: Node MassFlowRateSetPoint = 0.0, Node='//  &
                                //                                   TRIM(NodeID(SupplyNode))//  &
                                //                                   ', check for Node Connection Errors in the following messages.')
                                for (ZonesCooledIndex = 1; ZonesCooledIndex <= state.dataAirLoop->AirToZoneNodeInfo(AirLoopIndex).NumZonesCooled; ++ZonesCooledIndex) {
                                    TermInletNode = state.dataAirLoop->AirToZoneNodeInfo(AirLoopIndex).TermUnitCoolInletNodes(ZonesCooledIndex);
                                    Node(TermInletNode).MassFlowRateMaxAvail = Node(TermInletNode).MassFlowRateMax;
                                    Node(TermInletNode).MassFlowRateMinAvail =
                                        Node(SupplyNode).MassFlowRate / double(state.dataAirLoop->AirToZoneNodeInfo(AirLoopIndex).NumZonesCooled);
                                }
                            } else {
                                FlowRatio = Node(SupplyNode).MassFlowRate / Node(SupplyNode).MassFlowRateSetPoint;
                                for (ZonesCooledIndex = 1; ZonesCooledIndex <= state.dataAirLoop->AirToZoneNodeInfo(AirLoopIndex).NumZonesCooled; ++ZonesCooledIndex) {
                                    TermInletNode = state.dataAirLoop->AirToZoneNodeInfo(AirLoopIndex).TermUnitCoolInletNodes(ZonesCooledIndex);
                                    Node(TermInletNode).MassFlowRateMinAvail = Node(TermInletNode).MassFlowRate * FlowRatio;
                                    Node(TermInletNode).MassFlowRateMaxAvail =
                                        max(Node(TermInletNode).MassFlowRateMaxAvail, Node(TermInletNode).MassFlowRateMinAvail);
                                }
                            }
                        }
                    }
                }
            }
            for (SupplyIndex = 1; SupplyIndex <= state.dataAirLoop->AirToZoneNodeInfo(AirLoopIndex).NumSupplyNodes;
                 ++SupplyIndex) {                                                             // loop over the air loop supply outlets
                if (state.dataAirLoop->AirToZoneNodeInfo(AirLoopIndex).SupplyDuctType(SupplyIndex) == Heating) { // check for heating duct
                    // check if terminal units requesting more air than air loop can supply; if so, set terminal unit inlet
                    // node mass flow max avail to what air loop can supply
                    SupplyNode = state.dataAirLoop->AirToZoneNodeInfo(AirLoopIndex).AirLoopSupplyNodeNum(SupplyIndex);
                    if (Node(SupplyNode).MassFlowRate > 0.0) {
                        // must include bypass flow for ChangeoverBypass system so that terminal units are not restricted (e.g., MaxAvail is lowered)
                        if ((Node(SupplyNode).MassFlowRateSetPoint - Node(SupplyNode).MassFlowRate - state.dataAirLoop->AirLoopFlow(AirLoopIndex).BypassMassFlow) >
                            HVACFlowRateToler * 0.01) {
                            FlowRatio = Node(SupplyNode).MassFlowRate / Node(SupplyNode).MassFlowRateSetPoint;
                            for (ZonesHeatedIndex = 1; ZonesHeatedIndex <= state.dataAirLoop->AirToZoneNodeInfo(AirLoopIndex).NumZonesHeated; ++ZonesHeatedIndex) {
                                TermInletNode = state.dataAirLoop->AirToZoneNodeInfo(AirLoopIndex).TermUnitHeatInletNodes(ZonesHeatedIndex);
                                Node(TermInletNode).MassFlowRateMaxAvail = Node(TermInletNode).MassFlowRate * FlowRatio;
                                Node(TermInletNode).MassFlowRateMinAvail =
                                    min(Node(TermInletNode).MassFlowRateMaxAvail, Node(TermInletNode).MassFlowRateMinAvail);
                            }
                        }
                        if ((Node(SupplyNode).MassFlowRateSetPoint - Node(SupplyNode).MassFlowRate - state.dataAirLoop->AirLoopFlow(AirLoopIndex).BypassMassFlow) <
                            -HVACFlowRateToler * 0.01) {
                            if (Node(SupplyNode).MassFlowRateSetPoint == 0.0) {
                                // CALL ShowFatalError('ResolveAirLoopFlowLimits: Node MassFlowRateSetPoint = 0.0, Node='//  &
                                // TRIM(NodeID(SupplyNode))//  &
                                // ', check for Node Connection Errors in the following messages.')
                                for (ZonesHeatedIndex = 1; ZonesHeatedIndex <= state.dataAirLoop->AirToZoneNodeInfo(AirLoopIndex).NumZonesHeated; ++ZonesHeatedIndex) {
                                    TermInletNode = state.dataAirLoop->AirToZoneNodeInfo(AirLoopIndex).TermUnitHeatInletNodes(ZonesHeatedIndex);
                                    Node(TermInletNode).MassFlowRateMaxAvail = Node(TermInletNode).MassFlowRateMax;
                                    Node(TermInletNode).MassFlowRateMinAvail =
                                        Node(SupplyNode).MassFlowRate / double(state.dataAirLoop->AirToZoneNodeInfo(AirLoopIndex).NumZonesCooled);
                                }
                            } else {
                                FlowRatio = Node(SupplyNode).MassFlowRate / Node(SupplyNode).MassFlowRateSetPoint;
                                for (ZonesHeatedIndex = 1; ZonesHeatedIndex <= state.dataAirLoop->AirToZoneNodeInfo(AirLoopIndex).NumZonesHeated; ++ZonesHeatedIndex) {
                                    TermInletNode = state.dataAirLoop->AirToZoneNodeInfo(AirLoopIndex).TermUnitHeatInletNodes(ZonesHeatedIndex);
                                    Node(TermInletNode).MassFlowRateMinAvail = Node(TermInletNode).MassFlowRate * FlowRatio;
                                    Node(TermInletNode).MassFlowRateMaxAvail =
                                        max(Node(TermInletNode).MassFlowRateMaxAvail, Node(TermInletNode).MassFlowRateMinAvail);
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    void ResolveLockoutFlags(EnergyPlusData &state, bool &SimAir) // TRUE means air loops must be (re)simulated
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Fred Buhl
        //       DATE WRITTEN   December 2003
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine checks for components lockout flags and asks for air loop resimulation
        // if any components have been locked out

        // METHODOLOGY EMPLOYED:
        // Checks if loop lockout flags are .TRUE.; if so, sets SimAirLoops to .TRUE.

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

        // DERIVED TYPE DEFINITIONS:
        // na

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int AirLoopIndex;

        for (AirLoopIndex = 1; AirLoopIndex <= NumPrimaryAirSys; ++AirLoopIndex) { // loop over the primary air loops
            // check if economizer ia active and if there is a request that it be locked out
            if (state.dataAirLoop->AirLoopControlInfo(AirLoopIndex).EconoActive &&
                (state.dataAirLoop->AirLoopControlInfo(AirLoopIndex).ReqstEconoLockoutWithCompressor || state.dataAirLoop->AirLoopControlInfo(AirLoopIndex).ReqstEconoLockoutWithHeating)) {
                state.dataAirLoop->AirLoopControlInfo(AirLoopIndex).EconoLockout = true;
                SimAir = true;
            }
        }
    }

    void ResetHVACControl(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Fred Buhl
        //       DATE WRITTEN   December 2004
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine resets loop control flags and specified flow rates that may
        // have been set by the set point and availability managers in the previous
        // time step

        // METHODOLOGY EMPLOYED:

        // REFERENCES:
        // na

        // USE STATEMENTS:
        // na

        // SUBROUTINE ARGUMENT DEFINITIONS:

        // SUBROUTINE PARAMETER DEFINITIONS:
        // na

        // INTERFACE BLOCK SPECIFICATIONS:
        // na

        // DERIVED TYPE DEFINITIONS:
        // na

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:

        if (NumPrimaryAirSys == 0) return;
        for (auto &e : state.dataAirLoop->AirLoopControlInfo) {
            e.NightVent = false;
            e.LoopFlowRateSet = false;
        }
        for (auto &e : state.dataAirLoop->AirLoopFlow)
            e.ReqSupplyFrac = 1.0;
    }

    void ResetNodeData()
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda Lawrie
        //       DATE WRITTEN   March 2005
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This routine resets all node data to "initial" conditions.

        // METHODOLOGY EMPLOYED:
        // na

        // REFERENCES:
        // na

        // USE STATEMENTS:
        // na

        // SUBROUTINE ARGUMENT DEFINITIONS:
        // na

        // SUBROUTINE PARAMETER DEFINITIONS:
        // na

        // INTERFACE BLOCK SPECIFICATIONS:
        // na

        // DERIVED TYPE DEFINITIONS:
        // na

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        // na
        if (NumOfNodes <= 0) return;

        for (auto &e : Node) {
            e.Temp = DefaultNodeValues.Temp;
            e.TempMin = DefaultNodeValues.TempMin;
            e.TempMax = DefaultNodeValues.TempMax;
            e.TempSetPoint = DefaultNodeValues.TempSetPoint;
            e.MassFlowRate = DefaultNodeValues.MassFlowRate;
            e.MassFlowRateMin = DefaultNodeValues.MassFlowRateMin;
            e.MassFlowRateMax = DefaultNodeValues.MassFlowRateMax;
            e.MassFlowRateMinAvail = DefaultNodeValues.MassFlowRateMinAvail;
            e.MassFlowRateMaxAvail = DefaultNodeValues.MassFlowRateMaxAvail;
            e.MassFlowRateSetPoint = DefaultNodeValues.MassFlowRateSetPoint;
            e.Quality = DefaultNodeValues.Quality;
            e.Press = DefaultNodeValues.Press;
            e.Enthalpy = DefaultNodeValues.Enthalpy;
            e.HumRat = DefaultNodeValues.HumRat;
            e.HumRatMin = DefaultNodeValues.HumRatMin;
            e.HumRatMax = DefaultNodeValues.HumRatMax;
            e.HumRatSetPoint = DefaultNodeValues.HumRatSetPoint;
            e.TempSetPointHi = DefaultNodeValues.TempSetPointHi;
            e.TempSetPointLo = DefaultNodeValues.TempSetPointLo;
        }

        if (allocated(MoreNodeInfo)) {
            for (auto &e : MoreNodeInfo) {
                e.WetBulbTemp = DefaultNodeValues.Temp;
                e.RelHumidity = 0.0;
                e.ReportEnthalpy = DefaultNodeValues.Enthalpy;
                e.VolFlowRateStdRho = 0.0;
                e.VolFlowRateCrntRho = 0.0;
                e.Density = 0.0;
            }
        }
    }

    void UpdateZoneListAndGroupLoads()
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Apparently someone who doesn't believe in documenting.
        //       DATE WRITTEN   ???
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Don't know.

        // METHODOLOGY EMPLOYED:
        // na

        // REFERENCES:
        // na

        // Using/Aliasing
        using namespace DataHeatBalance;

        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:
        // na

        // SUBROUTINE PARAMETER DEFINITIONS:
        // na

        // INTERFACE BLOCK SPECIFICATIONS:
        // na

        // DERIVED TYPE DEFINITIONS:
        // na

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int ZoneNum;
        int ListNum;
        int GroupNum;
        int Mult;

        // FLOW:
        // Sum ZONE LIST and ZONE GROUP report variables
        ListSNLoadHeatEnergy = 0.0;
        ListSNLoadCoolEnergy = 0.0;
        ListSNLoadHeatRate = 0.0;
        ListSNLoadCoolRate = 0.0;

        for (ListNum = 1; ListNum <= NumOfZoneLists; ++ListNum) {
            for (ZoneNum = 1; ZoneNum <= ZoneList(ListNum).NumOfZones; ++ZoneNum) {
                Mult = Zone(ZoneNum).Multiplier;
                ListSNLoadHeatEnergy(ListNum) += SNLoadHeatEnergy(ZoneList(ListNum).Zone(ZoneNum)) * Mult;
                ListSNLoadCoolEnergy(ListNum) += SNLoadCoolEnergy(ZoneList(ListNum).Zone(ZoneNum)) * Mult;
                ListSNLoadHeatRate(ListNum) += SNLoadHeatRate(ZoneList(ListNum).Zone(ZoneNum)) * Mult;
                ListSNLoadCoolRate(ListNum) += SNLoadCoolRate(ZoneList(ListNum).Zone(ZoneNum)) * Mult;
            } // ZoneNum
        }     // ListNum

        for (GroupNum = 1; GroupNum <= NumOfZoneGroups; ++GroupNum) {
            Mult = ZoneGroup(GroupNum).Multiplier;
            GroupSNLoadHeatEnergy(GroupNum) = ListSNLoadHeatEnergy(ZoneGroup(GroupNum).ZoneList) * Mult;
            GroupSNLoadCoolEnergy(GroupNum) = ListSNLoadCoolEnergy(ZoneGroup(GroupNum).ZoneList) * Mult;
            GroupSNLoadHeatRate(GroupNum) = ListSNLoadHeatRate(ZoneGroup(GroupNum).ZoneList) * Mult;
            GroupSNLoadCoolRate(GroupNum) = ListSNLoadCoolRate(ZoneGroup(GroupNum).ZoneList) * Mult;
        } // GroupNum
    }

    void ReportAirHeatBalance(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda Lawrie
        //       DATE WRITTEN   July 2000
        //       MODIFIED       Shirey, Jan 2008 (MIXING/CROSS MIXING outputs)
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine updates the report variables for the AirHeatBalance.

        // METHODOLOGY EMPLOYED:
        // na

        // REFERENCES:
        // na

        // Using/Aliasing
        using DataEnvironment::OutBaroPress;
        using DataEnvironment::OutHumRat;
        using DataEnvironment::StdRhoAir;
        using DataGlobals::SecInHour;
        using DataHeatBalance::AirBalanceQuadrature;
        using DataHeatBalance::CrossMixing;
        using DataHeatBalance::Mixing;
        using DataHeatBalance::RefDoorMixing;
        using DataHeatBalance::TotCrossMixing;
        using DataHeatBalance::TotMixing;
        using DataHeatBalance::TotRefDoorMixing;
        using DataHeatBalance::TotVentilation;
        using DataHeatBalance::TotZoneAirBalance;
        using DataHeatBalance::Ventilation;
        using DataHeatBalance::ZnAirRpt;
        using DataHeatBalance::Zone;
        using DataHeatBalance::ZoneAirBalance;
        using DataHeatBalance::ZoneTotalExfiltrationHeatLoss;
        using DataHeatBalance::ZoneTotalExhaustHeatLoss;
        using DataHeatBalFanSys::MCPI; // , MCPTI, MCPTV, MCPM, MCPTM, MixingMassFlowZone
        using DataHeatBalFanSys::MCPV;
        using DataHeatBalFanSys::MDotCPOA;
        using DataHeatBalFanSys::MDotOA;
        using DataHVACGlobals::CycleOn;
        using DataHVACGlobals::CycleOnZoneFansOnly;
        using Psychrometrics::PsyCpAirFnW;
        using Psychrometrics::PsyHgAirFnWTdb;
        using Psychrometrics::PsyRhoAirFnPbTdbW;

        using AirflowNetworkBalanceManager::ReportAirflowNetwork;
        using DataZoneEquipment::ZoneEquipAvail;

        using DataZoneEquipment::CrossMixingReportFlag;
        using DataZoneEquipment::MixingReportFlag;
        using DataZoneEquipment::VentMCP;

        using DataZoneEquipment::ZoneEquipConfig;

        using DataHVACGlobals::FanType_ZoneExhaust;
        using Fans::Fan;

        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:
        // na

        // SUBROUTINE PARAMETER DEFINITIONS:
        static std::string const RoutineName3("ReportAirHeatBalance:3");
        // na

        // INTERFACE BLOCK SPECIFICATIONS:
        // na

        // DERIVED TYPE DEFINITIONS:
        // na

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int ZoneLoop;                      // Counter for the # of zones (nz)
        int ZoneA;                         // Mated zone number for pair pf zones sharing refrigeration door opening
        int ZoneB;                         // Mated zone number for pair pf zones sharing refrigeration door opening
        int VentNum;                       // Counter for ventilation statements
        int FanNum;                        // Counter for exhaust fans
        Real64 AirDensity;                 // Density of air (kg/m^3)
        Real64 CpAir;                      // Heat capacity of air (J/kg-C)
        Real64 ADSCorrectionFactor;        // Correction factor of air flow model values when ADS is simulated
        Real64 H2OHtOfVap;                 // Heat of vaporization of air
        Real64 TotalLoad;                  // Total loss or gain
        int MixNum;                        // Counter for MIXING and Cross Mixing statements
        static Array1D<Real64> MixSenLoad; // Mixing sensible loss or gain
        static Array1D<Real64> MixLatLoad; // Mixing latent loss or gain
        int j;                             // Index in a do-loop
        int VentZoneNum;                   // Number of ventilation object per zone
        Real64 VentZoneMassflow;           // Total mass flow rate per zone
        Real64 VentZoneAirTemp;            // Average Zone inlet temperature

        ZoneTotalExfiltrationHeatLoss = 0.0;
        ZoneTotalExhaustHeatLoss = 0.0;

        // Ensure no airflownetwork and simple calculations
        if (AirflowNetwork::SimulateAirflowNetwork == 0) return;

        if (AirflowNetwork::SimulateAirflowNetwork > AirflowNetwork::AirflowNetworkControlSimple) ReportAirflowNetwork(state);

        // Reports zone exhaust loss by exhaust fans
        for (ZoneLoop = 1; ZoneLoop <= NumOfZones; ++ZoneLoop) { // Start of zone loads report variable update loop ...
            CpAir = PsyCpAirFnW(OutHumRat);
            H2OHtOfVap = PsyHgAirFnWTdb(OutHumRat, Zone(ZoneLoop).OutDryBulbTemp);
            ADSCorrectionFactor = 1.0;
            if (AirflowNetwork::SimulateAirflowNetwork == AirflowNetwork::AirflowNetworkControlSimpleADS) {
                if ((ZoneEquipAvail(ZoneLoop) == CycleOn || ZoneEquipAvail(ZoneLoop) == CycleOnZoneFansOnly) &&
                    AirflowNetwork::AirflowNetworkZoneFlag(ZoneLoop)) {
                    ADSCorrectionFactor = 0.0;
                }
            }

            ZnAirRpt(ZoneLoop).ExhTotalLoss = 0;
            ZnAirRpt(ZoneLoop).ExhSensiLoss = 0;

            for (FanNum = 1; FanNum <= state.dataFans->NumFans; ++FanNum) {
                //  Add reportable vars
                if (Fan(FanNum).FanType_Num == FanType_ZoneExhaust) {
                    for (int ExhNum = 1; ExhNum <= ZoneEquipConfig(ZoneLoop).NumExhaustNodes; ExhNum++) {
                        if (Fan(FanNum).InletNodeNum == ZoneEquipConfig(ZoneLoop).ExhaustNode(ExhNum)) {
                            ZnAirRpt(ZoneLoop).ExhTotalLoss +=
                                Fan(FanNum).OutletAirMassFlowRate * (Fan(FanNum).OutletAirEnthalpy - OutEnthalpy) * ADSCorrectionFactor;
                            ZnAirRpt(ZoneLoop).ExhSensiLoss += Fan(FanNum).OutletAirMassFlowRate * CpAir *
                                                               (Fan(FanNum).OutletAirTemp - Zone(ZoneLoop).OutDryBulbTemp) * ADSCorrectionFactor;
                            break;
                        }
                    }
                }
            }

            ZnAirRpt(ZoneLoop).ExhLatentLoss = ZnAirRpt(ZoneLoop).ExhTotalLoss - ZnAirRpt(ZoneLoop).ExhSensiLoss;
        }

        // Report results for SIMPLE option only
        if (!(AirflowNetwork::SimulateAirflowNetwork == AirflowNetwork::AirflowNetworkControlSimple ||
              AirflowNetwork::SimulateAirflowNetwork == AirflowNetwork::AirflowNetworkControlSimpleADS))
            return;

        if (ReportAirHeatBalanceFirstTimeFlag) {
            MixSenLoad.allocate(NumOfZones);
            MixLatLoad.allocate(NumOfZones);
            ReportAirHeatBalanceFirstTimeFlag = false;
        }

        for (ZoneLoop = 1; ZoneLoop <= NumOfZones; ++ZoneLoop) { // Start of zone loads report variable update loop ...

            // Break the infiltration load into heat gain and loss components
            ADSCorrectionFactor = 1.0;

            if (AirflowNetwork::SimulateAirflowNetwork == AirflowNetwork::AirflowNetworkControlSimpleADS) {
                // CR7608 IF (TurnFansOn .AND. AirflowNetworkZoneFlag(ZoneLoop)) ADSCorrectionFactor=0
                if ((ZoneEquipAvail(ZoneLoop) == CycleOn || ZoneEquipAvail(ZoneLoop) == CycleOnZoneFansOnly) &&
                    AirflowNetwork::AirflowNetworkZoneFlag(ZoneLoop))
                    ADSCorrectionFactor = 0.0;
            }

            if (MAT(ZoneLoop) > Zone(ZoneLoop).OutDryBulbTemp) {

                ZnAirRpt(ZoneLoop).InfilHeatLoss =
                    0.001 * MCPI(ZoneLoop) * (MAT(ZoneLoop) - Zone(ZoneLoop).OutDryBulbTemp) * TimeStepSys * SecInHour * 1000.0 * ADSCorrectionFactor;
                ZnAirRpt(ZoneLoop).InfilHeatGain = 0.0;

            } else if (MAT(ZoneLoop) <= Zone(ZoneLoop).OutDryBulbTemp) {

                ZnAirRpt(ZoneLoop).InfilHeatGain =
                    0.001 * MCPI(ZoneLoop) * (Zone(ZoneLoop).OutDryBulbTemp - MAT(ZoneLoop)) * TimeStepSys * SecInHour * 1000.0 * ADSCorrectionFactor;
                ZnAirRpt(ZoneLoop).InfilHeatLoss = 0.0;
            }
            // Report infiltration latent gains and losses
            CpAir = PsyCpAirFnW(OutHumRat);
            H2OHtOfVap = PsyHgAirFnWTdb(ZoneAirHumRat(ZoneLoop), MAT(ZoneLoop));
            if (ZoneAirHumRat(ZoneLoop) > OutHumRat) {

                ZnAirRpt(ZoneLoop).InfilLatentLoss = 0.001 * MCPI(ZoneLoop) / CpAir * (ZoneAirHumRat(ZoneLoop) - OutHumRat) * H2OHtOfVap *
                                                     TimeStepSys * SecInHour * 1000.0 * ADSCorrectionFactor;
                ZnAirRpt(ZoneLoop).InfilLatentGain = 0.0;

            } else if (ZoneAirHumRat(ZoneLoop) <= OutHumRat) {

                ZnAirRpt(ZoneLoop).InfilLatentGain = 0.001 * MCPI(ZoneLoop) / CpAir * (OutHumRat - ZoneAirHumRat(ZoneLoop)) * H2OHtOfVap *
                                                     TimeStepSys * SecInHour * 1000.0 * ADSCorrectionFactor;
                ZnAirRpt(ZoneLoop).InfilLatentLoss = 0.0;
            }
            // Total infiltration losses and gains
            TotalLoad = ZnAirRpt(ZoneLoop).InfilHeatGain + ZnAirRpt(ZoneLoop).InfilLatentGain - ZnAirRpt(ZoneLoop).InfilHeatLoss -
                        ZnAirRpt(ZoneLoop).InfilLatentLoss;
            if (TotalLoad > 0) {
                ZnAirRpt(ZoneLoop).InfilTotalGain = TotalLoad * ADSCorrectionFactor;
                ZnAirRpt(ZoneLoop).InfilTotalLoss = 0.0;
            } else {
                ZnAirRpt(ZoneLoop).InfilTotalGain = 0.0;
                ZnAirRpt(ZoneLoop).InfilTotalLoss = -TotalLoad * ADSCorrectionFactor;
            }

            // first calculate mass flows using outside air heat capacity for consistency with input to heat balance
            CpAir = PsyCpAirFnW(OutHumRat);
            ZnAirRpt(ZoneLoop).InfilMass = (MCPI(ZoneLoop) / CpAir) * TimeStepSys * SecInHour * ADSCorrectionFactor;
            ZnAirRpt(ZoneLoop).InfilMdot = (MCPI(ZoneLoop) / CpAir) * ADSCorrectionFactor;
            ZnAirRpt(ZoneLoop).VentilMass = (MCPV(ZoneLoop) / CpAir) * TimeStepSys * SecInHour * ADSCorrectionFactor;
            ZnAirRpt(ZoneLoop).VentilMdot = (MCPV(ZoneLoop) / CpAir) * ADSCorrectionFactor;

            // CR7751  second, calculate using indoor conditions for density property
            AirDensity = PsyRhoAirFnPbTdbW(OutBaroPress, MAT(ZoneLoop), ZoneAirHumRatAvg(ZoneLoop), RoutineName3);
            ZnAirRpt(ZoneLoop).InfilVolumeCurDensity = (MCPI(ZoneLoop) / CpAir / AirDensity) * TimeStepSys * SecInHour * ADSCorrectionFactor;
            ZnAirRpt(ZoneLoop).InfilAirChangeRate = ZnAirRpt(ZoneLoop).InfilVolumeCurDensity / (TimeStepSys * Zone(ZoneLoop).Volume);
            ZnAirRpt(ZoneLoop).InfilVdotCurDensity = (MCPI(ZoneLoop) / CpAir / AirDensity) * ADSCorrectionFactor;
            ZnAirRpt(ZoneLoop).VentilVolumeCurDensity = (MCPV(ZoneLoop) / CpAir / AirDensity) * TimeStepSys * SecInHour * ADSCorrectionFactor;
            ZnAirRpt(ZoneLoop).VentilAirChangeRate = ZnAirRpt(ZoneLoop).VentilVolumeCurDensity / (TimeStepSys * Zone(ZoneLoop).Volume);
            ZnAirRpt(ZoneLoop).VentilVdotCurDensity = (MCPV(ZoneLoop) / CpAir / AirDensity) * ADSCorrectionFactor;

            // CR7751 third, calculate using standard dry air at nominal elevation
            AirDensity = StdRhoAir;
            ZnAirRpt(ZoneLoop).InfilVolumeStdDensity = (MCPI(ZoneLoop) / CpAir / AirDensity) * TimeStepSys * SecInHour * ADSCorrectionFactor;
            ZnAirRpt(ZoneLoop).InfilVdotStdDensity = (MCPI(ZoneLoop) / CpAir / AirDensity) * ADSCorrectionFactor;
            ZnAirRpt(ZoneLoop).VentilVolumeStdDensity = (MCPV(ZoneLoop) / CpAir / AirDensity) * TimeStepSys * SecInHour * ADSCorrectionFactor;
            ZnAirRpt(ZoneLoop).VentilVdotStdDensity = (MCPV(ZoneLoop) / CpAir / AirDensity) * ADSCorrectionFactor;

            //    ZnAirRpt(ZoneLoop)%VentilFanElec = 0.0
            ZnAirRpt(ZoneLoop).VentilAirTemp = 0.0;
            ZnAirRpt(ZoneLoop).VentilHeatLoss = 0.0;
            ZnAirRpt(ZoneLoop).VentilHeatGain = 0.0;
            VentZoneNum = 0;
            VentZoneMassflow = 0.0;
            VentZoneAirTemp = 0.0;

            for (VentNum = 1; VentNum <= TotVentilation; ++VentNum) {
                if (Ventilation(VentNum).ZonePtr == ZoneLoop) {
                    // moved into CalcAirFlowSimple
                    //        ZnAirRpt(ZoneLoop)%VentilFanElec  = ZnAirRpt(ZoneLoop)%VentilFanElec+Ventilation(VentNum)%FanPower*TimeStepSys*SecInHour
                    //        &
                    //          *ADSCorrectionFactor
                    if (ADSCorrectionFactor > 0) {
                        ZnAirRpt(ZoneLoop).VentilAirTemp += Ventilation(VentNum).AirTemp * VentMCP(VentNum);
                        VentZoneMassflow += VentMCP(VentNum);
                        VentZoneAirTemp += Ventilation(VentNum).AirTemp;
                    } else {
                        ZnAirRpt(ZoneLoop).VentilAirTemp = Zone(ZoneLoop).OutDryBulbTemp;
                    }
                    // Break the ventilation load into heat gain and loss components
                    if (MAT(ZoneLoop) > Ventilation(VentNum).AirTemp) {
                        ZnAirRpt(ZoneLoop).VentilHeatLoss +=
                            VentMCP(VentNum) * (MAT(ZoneLoop) - Ventilation(VentNum).AirTemp) * TimeStepSys * SecInHour * ADSCorrectionFactor;
                    } else if (MAT(ZoneLoop) <= Ventilation(VentNum).AirTemp) {
                        ZnAirRpt(ZoneLoop).VentilHeatGain +=
                            VentMCP(VentNum) * (Ventilation(VentNum).AirTemp - MAT(ZoneLoop)) * TimeStepSys * SecInHour * ADSCorrectionFactor;
                    }

                    ++VentZoneNum;
                    if (VentZoneNum > 1) continue;

                    // Report ventilation latent gains and losses
                    H2OHtOfVap = PsyHgAirFnWTdb(ZoneAirHumRat(ZoneLoop), MAT(ZoneLoop));
                    if (ZoneAirHumRat(ZoneLoop) > OutHumRat) {
                        ZnAirRpt(ZoneLoop).VentilLatentLoss = 0.001 * MCPV(ZoneLoop) / CpAir * (ZoneAirHumRat(ZoneLoop) - OutHumRat) * H2OHtOfVap *
                                                              TimeStepSys * SecInHour * 1000.0 * ADSCorrectionFactor;
                        ZnAirRpt(ZoneLoop).VentilLatentGain = 0.0;
                    } else if (ZoneAirHumRat(ZoneLoop) <= OutHumRat) {
                        ZnAirRpt(ZoneLoop).VentilLatentGain = 0.001 * MCPV(ZoneLoop) / CpAir * (OutHumRat - ZoneAirHumRat(ZoneLoop)) * H2OHtOfVap *
                                                              TimeStepSys * SecInHour * 1000.0 * ADSCorrectionFactor;
                        ZnAirRpt(ZoneLoop).VentilLatentLoss = 0.0;
                    }
                    // Total ventilation losses and gains
                    TotalLoad = ZnAirRpt(ZoneLoop).VentilHeatGain + ZnAirRpt(ZoneLoop).VentilLatentGain - ZnAirRpt(ZoneLoop).VentilHeatLoss -
                                ZnAirRpt(ZoneLoop).VentilLatentLoss;
                    if (TotalLoad > 0) {
                        ZnAirRpt(ZoneLoop).VentilTotalGain = TotalLoad * ADSCorrectionFactor;
                        ZnAirRpt(ZoneLoop).VentilTotalLoss = 0.0;
                    } else {
                        ZnAirRpt(ZoneLoop).VentilTotalGain = 0.0;
                        ZnAirRpt(ZoneLoop).VentilTotalLoss = -TotalLoad * ADSCorrectionFactor;
                    }
                }
            }

            if (ADSCorrectionFactor > 0 && VentZoneNum > 1 && VentZoneMassflow > 0.0) {
                ZnAirRpt(ZoneLoop).VentilAirTemp /= VentZoneMassflow;
            } else if (ADSCorrectionFactor > 0 && VentZoneNum == 1) {
                ZnAirRpt(ZoneLoop).VentilAirTemp = VentZoneAirTemp;
            } else { // Just in case
                ZnAirRpt(ZoneLoop).VentilAirTemp = Zone(ZoneLoop).OutDryBulbTemp;
            }

            // Report mixing sensible and latent loads
            MixSenLoad = 0.0; // Initialize arrays to zero before starting to sum
            MixLatLoad = 0.0;
            ZnAirRpt(ZoneLoop).MixVolume = 0.0;         // zero reported volume prior to summations below
            ZnAirRpt(ZoneLoop).MixVdotCurDensity = 0.0; // zero reported volume flow rate prior to summations below
            ZnAirRpt(ZoneLoop).MixVdotStdDensity = 0.0; // zero reported volume flow rate prior to summations below
            ZnAirRpt(ZoneLoop).MixMass = 0.0;           // ! zero reported mass prior to summations below
            ZnAirRpt(ZoneLoop).MixMdot = 0.0;           // ! zero reported mass flow rate prior to summations below
            //    MixingLoad = 0.0d0

            for (MixNum = 1; MixNum <= TotMixing; ++MixNum) {
                if ((Mixing(MixNum).ZonePtr == ZoneLoop) && MixingReportFlag(MixNum)) {
                    //        MixSenLoad(ZoneLoop) = MixSenLoad(ZoneLoop)+MCPM(ZoneLoop)*MAT(Mixing(MixNum)%FromZone)
                    //        H2OHtOfVap = PsyHgAirFnWTdb(ZoneAirHumRat(ZoneLoop), MAT(ZoneLoop))
                    //        Per Jan 17, 2008 conference call, agreed to use average conditions for Rho, Cp and Hfg
                    //           and to recalculate the report variable using end of time step temps and humrats
                    AirDensity = PsyRhoAirFnPbTdbW(OutBaroPress,
                                                   (MAT(ZoneLoop) + MAT(Mixing(MixNum).FromZone)) / 2.0,
                                                   (ZoneAirHumRat(ZoneLoop) + ZoneAirHumRat(Mixing(MixNum).FromZone)) / 2.0,
                                                   BlankString);
                    CpAir = PsyCpAirFnW((ZoneAirHumRat(ZoneLoop) + ZoneAirHumRat(Mixing(MixNum).FromZone)) / 2.0);
                    ZnAirRpt(ZoneLoop).MixVolume += Mixing(MixNum).DesiredAirFlowRate * TimeStepSys * SecInHour * ADSCorrectionFactor;
                    ZnAirRpt(ZoneLoop).MixVdotCurDensity += Mixing(MixNum).DesiredAirFlowRate * ADSCorrectionFactor;
                    ZnAirRpt(ZoneLoop).MixMass += Mixing(MixNum).DesiredAirFlowRate * AirDensity * TimeStepSys * SecInHour * ADSCorrectionFactor;
                    ZnAirRpt(ZoneLoop).MixMdot += Mixing(MixNum).DesiredAirFlowRate * AirDensity * ADSCorrectionFactor;
                    ZnAirRpt(ZoneLoop).MixVdotStdDensity += Mixing(MixNum).DesiredAirFlowRate * (AirDensity / StdRhoAir) * ADSCorrectionFactor;
                    MixSenLoad(ZoneLoop) += Mixing(MixNum).DesiredAirFlowRate * AirDensity * CpAir * (MAT(ZoneLoop) - MAT(Mixing(MixNum).FromZone));
                    H2OHtOfVap = PsyHgAirFnWTdb((ZoneAirHumRat(ZoneLoop) + ZoneAirHumRat(Mixing(MixNum).FromZone)) / 2.0,
                                                (MAT(ZoneLoop) + MAT(Mixing(MixNum).FromZone)) / 2.0);
                    //        MixLatLoad(ZoneLoop) = MixLatLoad(ZoneLoop)+MixingMassFlowZone(ZoneLoop)*(ZoneAirHumRat(ZoneLoop)- &
                    //                     ZoneAirHumRat(Mixing(MixNum)%FromZone))*H2OHtOfVap
                    MixLatLoad(ZoneLoop) += Mixing(MixNum).DesiredAirFlowRate * AirDensity *
                                            (ZoneAirHumRat(ZoneLoop) - ZoneAirHumRat(Mixing(MixNum).FromZone)) * H2OHtOfVap;
                }
            }

            for (MixNum = 1; MixNum <= TotCrossMixing; ++MixNum) {
                if ((CrossMixing(MixNum).ZonePtr == ZoneLoop) && CrossMixingReportFlag(MixNum)) {
                    //        MixSenLoad(ZoneLoop) = MixSenLoad(ZoneLoop)+MCPM(ZoneLoop)*MAT(CrossMixing(MixNum)%FromZone)
                    //        Per Jan 17, 2008 conference call, agreed to use average conditions for Rho, Cp and Hfg
                    //           and to recalculate the report variable using end of time step temps and humrats
                    AirDensity = PsyRhoAirFnPbTdbW(OutBaroPress,
                                                   (MAT(ZoneLoop) + MAT(CrossMixing(MixNum).FromZone)) / 2.0,
                                                   (ZoneAirHumRat(ZoneLoop) + ZoneAirHumRat(CrossMixing(MixNum).FromZone)) / 2.0,
                                                   BlankString);
                    CpAir = PsyCpAirFnW((ZoneAirHumRat(ZoneLoop) + ZoneAirHumRat(CrossMixing(MixNum).FromZone)) / 2.0);
                    ZnAirRpt(ZoneLoop).MixVolume += CrossMixing(MixNum).DesiredAirFlowRate * TimeStepSys * SecInHour * ADSCorrectionFactor;
                    ZnAirRpt(ZoneLoop).MixVdotCurDensity += CrossMixing(MixNum).DesiredAirFlowRate * ADSCorrectionFactor;
                    ZnAirRpt(ZoneLoop).MixMass += CrossMixing(MixNum).DesiredAirFlowRate * AirDensity * TimeStepSys * SecInHour * ADSCorrectionFactor;
                    ZnAirRpt(ZoneLoop).MixMdot += CrossMixing(MixNum).DesiredAirFlowRate * AirDensity * ADSCorrectionFactor;
                    ZnAirRpt(ZoneLoop).MixVdotStdDensity += CrossMixing(MixNum).DesiredAirFlowRate * (AirDensity / StdRhoAir) * ADSCorrectionFactor;
                    MixSenLoad(ZoneLoop) +=
                        CrossMixing(MixNum).DesiredAirFlowRate * AirDensity * CpAir * (MAT(ZoneLoop) - MAT(CrossMixing(MixNum).FromZone));
                    H2OHtOfVap = PsyHgAirFnWTdb((ZoneAirHumRat(ZoneLoop) + ZoneAirHumRat(CrossMixing(MixNum).FromZone)) / 2.0,
                                                (MAT(ZoneLoop) + MAT(CrossMixing(MixNum).FromZone)) / 2.0);
                    //       MixLatLoad(ZoneLoop) = MixLatLoad(ZoneLoop)+MixingMassFlowZone(ZoneLoop)*(ZoneAirHumRat(ZoneLoop)- &
                    //                     ZoneAirHumRat(CrossMixing(MixNum)%FromZone))*H2OHtOfVap
                    MixLatLoad(ZoneLoop) += CrossMixing(MixNum).DesiredAirFlowRate * AirDensity *
                                            (ZoneAirHumRat(ZoneLoop) - ZoneAirHumRat(CrossMixing(MixNum).FromZone)) * H2OHtOfVap;
                }
                if ((CrossMixing(MixNum).FromZone == ZoneLoop) && CrossMixingReportFlag(MixNum)) {
                    AirDensity = PsyRhoAirFnPbTdbW(OutBaroPress,
                                                   (MAT(ZoneLoop) + MAT(CrossMixing(MixNum).ZonePtr)) / 2.0,
                                                   (ZoneAirHumRat(ZoneLoop) + ZoneAirHumRat(CrossMixing(MixNum).ZonePtr)) / 2.0,
                                                   BlankString);
                    CpAir = PsyCpAirFnW((ZoneAirHumRat(ZoneLoop) + ZoneAirHumRat(CrossMixing(MixNum).ZonePtr)) / 2.0);
                    ZnAirRpt(ZoneLoop).MixVolume += CrossMixing(MixNum).DesiredAirFlowRate * TimeStepSys * SecInHour * ADSCorrectionFactor;
                    ZnAirRpt(ZoneLoop).MixVdotCurDensity += CrossMixing(MixNum).DesiredAirFlowRate * ADSCorrectionFactor;
                    ZnAirRpt(ZoneLoop).MixMass += CrossMixing(MixNum).DesiredAirFlowRate * AirDensity * TimeStepSys * SecInHour * ADSCorrectionFactor;
                    ZnAirRpt(ZoneLoop).MixMdot += CrossMixing(MixNum).DesiredAirFlowRate * AirDensity * ADSCorrectionFactor;
                    ZnAirRpt(ZoneLoop).MixVdotStdDensity += CrossMixing(MixNum).DesiredAirFlowRate * (AirDensity / StdRhoAir) * ADSCorrectionFactor;
                    MixSenLoad(ZoneLoop) +=
                        CrossMixing(MixNum).DesiredAirFlowRate * AirDensity * CpAir * (MAT(ZoneLoop) - MAT(CrossMixing(MixNum).ZonePtr));
                    H2OHtOfVap = PsyHgAirFnWTdb((ZoneAirHumRat(ZoneLoop) + ZoneAirHumRat(CrossMixing(MixNum).ZonePtr)) / 2.0,
                                                (MAT(ZoneLoop) + MAT(CrossMixing(MixNum).ZonePtr)) / 2.0);
                    MixLatLoad(ZoneLoop) += CrossMixing(MixNum).DesiredAirFlowRate * AirDensity *
                                            (ZoneAirHumRat(ZoneLoop) - ZoneAirHumRat(CrossMixing(MixNum).ZonePtr)) * H2OHtOfVap;
                }
            }

            if (TotRefDoorMixing > 0) {
                // IF(ZoneLoop .NE. NumOfZones)THEN  !Refrigeration Door Mixing
                // Note - do each Pair a Single time, so must do increment reports for both zones
                //       Can't have a pair that has ZoneA zone number = NumOfZones because organized
                //       in input with lowest zone # first no matter how input in idf
                if (RefDoorMixing(ZoneLoop).RefDoorMixFlag) { // .TRUE. for both zoneA and zoneB
                    if (RefDoorMixing(ZoneLoop).ZonePtr == ZoneLoop) {
                        for (j = 1; j <= RefDoorMixing(ZoneLoop).NumRefDoorConnections; ++j) {
                            //    Capture impact when zoneloop is the 'primary zone'
                            //    that is, the zone of a pair with the lower zone number
                            if (RefDoorMixing(ZoneLoop).VolRefDoorFlowRate(j) > 0.0) {
                                ZoneB = RefDoorMixing(ZoneLoop).MateZonePtr(j);
                                AirDensity = PsyRhoAirFnPbTdbW(OutBaroPress,
                                                               (MAT(ZoneLoop) + MAT(ZoneB)) / 2.0,
                                                               (ZoneAirHumRat(ZoneLoop) + ZoneAirHumRat(ZoneB)) / 2.0,
                                                               BlankString);
                                CpAir = PsyCpAirFnW((ZoneAirHumRat(ZoneLoop) + ZoneAirHumRat(ZoneB)) / 2.0);
                                H2OHtOfVap =
                                    PsyHgAirFnWTdb((ZoneAirHumRat(ZoneLoop) + ZoneAirHumRat(ZoneB)) / 2.0, (MAT(ZoneLoop) + MAT(ZoneB)) / 2.0);
                                ZnAirRpt(ZoneLoop).MixVolume +=
                                    RefDoorMixing(ZoneLoop).VolRefDoorFlowRate(j) * TimeStepSys * SecInHour * ADSCorrectionFactor;
                                ZnAirRpt(ZoneLoop).MixVdotCurDensity += RefDoorMixing(ZoneLoop).VolRefDoorFlowRate(j) * ADSCorrectionFactor;
                                ZnAirRpt(ZoneLoop).MixMass +=
                                    RefDoorMixing(ZoneLoop).VolRefDoorFlowRate(j) * AirDensity * TimeStepSys * SecInHour * ADSCorrectionFactor;
                                ZnAirRpt(ZoneLoop).MixMdot += RefDoorMixing(ZoneLoop).VolRefDoorFlowRate(j) * AirDensity * ADSCorrectionFactor;
                                ZnAirRpt(ZoneLoop).MixVdotStdDensity +=
                                    RefDoorMixing(ZoneLoop).VolRefDoorFlowRate(j) * (AirDensity / StdRhoAir) * ADSCorrectionFactor;
                                MixSenLoad(ZoneLoop) +=
                                    RefDoorMixing(ZoneLoop).VolRefDoorFlowRate(j) * AirDensity * CpAir * (MAT(ZoneLoop) - MAT(ZoneB));
                                MixLatLoad(ZoneLoop) += RefDoorMixing(ZoneLoop).VolRefDoorFlowRate(j) * AirDensity *
                                                        (ZoneAirHumRat(ZoneLoop) - ZoneAirHumRat(ZoneB)) * H2OHtOfVap;
                            } // flow > 0
                        }     // J-1, numref connections
                    }         // zone A (zoneptr = zoneloop)
                    for (ZoneA = 1; ZoneA <= (ZoneLoop - 1); ++ZoneA) {
                        //    Capture impact when zoneloop is the 'mating zone'
                        //    that is, the zone of a pair with the higher zone number(matezoneptr = zoneloop)
                        if (RefDoorMixing(ZoneA).RefDoorMixFlag) {
                            for (j = 1; j <= RefDoorMixing(ZoneA).NumRefDoorConnections; ++j) {
                                if (RefDoorMixing(ZoneA).MateZonePtr(j) == ZoneLoop) {
                                    if (RefDoorMixing(ZoneA).VolRefDoorFlowRate(j) > 0.0) {
                                        AirDensity = PsyRhoAirFnPbTdbW(OutBaroPress,
                                                                       (MAT(ZoneLoop) + MAT(ZoneA)) / 2.0,
                                                                       (ZoneAirHumRat(ZoneLoop) + ZoneAirHumRat(ZoneA)) / 2.0,
                                                                       BlankString);
                                        CpAir = PsyCpAirFnW((ZoneAirHumRat(ZoneLoop) + ZoneAirHumRat(ZoneA)) / 2.0);
                                        H2OHtOfVap = PsyHgAirFnWTdb((ZoneAirHumRat(ZoneLoop) + ZoneAirHumRat(ZoneA)) / 2.0,
                                                                    (MAT(ZoneLoop) + MAT(ZoneA)) / 2.0);
                                        ZnAirRpt(ZoneLoop).MixVolume +=
                                            RefDoorMixing(ZoneA).VolRefDoorFlowRate(j) * TimeStepSys * SecInHour * ADSCorrectionFactor;
                                        ZnAirRpt(ZoneLoop).MixVdotCurDensity += RefDoorMixing(ZoneA).VolRefDoorFlowRate(j) * ADSCorrectionFactor;
                                        ZnAirRpt(ZoneLoop).MixMass +=
                                            RefDoorMixing(ZoneA).VolRefDoorFlowRate(j) * AirDensity * TimeStepSys * SecInHour * ADSCorrectionFactor;
                                        ZnAirRpt(ZoneLoop).MixMdot += RefDoorMixing(ZoneA).VolRefDoorFlowRate(j) * AirDensity * ADSCorrectionFactor;
                                        ZnAirRpt(ZoneLoop).MixVdotStdDensity +=
                                            RefDoorMixing(ZoneA).VolRefDoorFlowRate(j) * (AirDensity / StdRhoAir) * ADSCorrectionFactor;
                                        MixSenLoad(ZoneLoop) +=
                                            RefDoorMixing(ZoneA).VolRefDoorFlowRate(j) * AirDensity * CpAir * (MAT(ZoneLoop) - MAT(ZoneA));
                                        MixLatLoad(ZoneLoop) += RefDoorMixing(ZoneA).VolRefDoorFlowRate(j) * AirDensity *
                                                                (ZoneAirHumRat(ZoneLoop) - ZoneAirHumRat(ZoneA)) * H2OHtOfVap;
                                    } // volflowrate > 0
                                }     // matezoneptr (zoneB) = Zonelooop
                            }         // NumRefDoorConnections
                        }             // Refdoormix flag on ZoneA
                    }                 // zone A from 1 to (zoneloop - 1)
                }                     // Refdoormix flag on zoneloop
            }                         //(TotRefDoorMixing .GT. 0)
            // end refrigeration door mixing reports

            //    MixingLoad(ZoneLoop) = MCPM(ZoneLoop)*MAT(ZoneLoop) - MixSenLoad(ZoneLoop)
            if (MixSenLoad(ZoneLoop) > 0.0) {
                ZnAirRpt(ZoneLoop).MixHeatLoss = MixSenLoad(ZoneLoop) * TimeStepSys * SecInHour * ADSCorrectionFactor;
                ZnAirRpt(ZoneLoop).MixHeatGain = 0.0;
            } else {
                ZnAirRpt(ZoneLoop).MixHeatLoss = 0.0;
                ZnAirRpt(ZoneLoop).MixHeatGain = -MixSenLoad(ZoneLoop) * TimeStepSys * SecInHour * ADSCorrectionFactor;
            }
            // Report mixing latent loads
            //    MixingLoad(ZoneLoop) = MixLatLoad(ZoneLoop)
            if (MixLatLoad(ZoneLoop) > 0.0) {
                ZnAirRpt(ZoneLoop).MixLatentLoss = MixLatLoad(ZoneLoop) * TimeStepSys * SecInHour * ADSCorrectionFactor;
                ZnAirRpt(ZoneLoop).MixLatentGain = 0.0;
            } else {
                ZnAirRpt(ZoneLoop).MixLatentLoss = 0.0;
                ZnAirRpt(ZoneLoop).MixLatentGain = -MixLatLoad(ZoneLoop) * TimeStepSys * SecInHour * ADSCorrectionFactor;
            }
            // Total Mixing losses and gains
            TotalLoad =
                ZnAirRpt(ZoneLoop).MixHeatGain + ZnAirRpt(ZoneLoop).MixLatentGain - ZnAirRpt(ZoneLoop).MixHeatLoss - ZnAirRpt(ZoneLoop).MixLatentLoss;
            if (TotalLoad > 0) {
                ZnAirRpt(ZoneLoop).MixTotalGain = TotalLoad * ADSCorrectionFactor;
                ZnAirRpt(ZoneLoop).MixTotalLoss = 0.0;
            } else {
                ZnAirRpt(ZoneLoop).MixTotalGain = 0.0;
                ZnAirRpt(ZoneLoop).MixTotalLoss = -TotalLoad * ADSCorrectionFactor;
            }

            // Reporting combined outdoor air flows
            for (j = 1; j <= TotZoneAirBalance; ++j) {
                if (ZoneAirBalance(j).BalanceMethod == AirBalanceQuadrature && ZoneLoop == ZoneAirBalance(j).ZonePtr) {
                    if (MAT(ZoneLoop) > Zone(ZoneLoop).OutDryBulbTemp) {
                        ZnAirRpt(ZoneLoop).OABalanceHeatLoss =
                            MDotCPOA(ZoneLoop) * (MAT(ZoneLoop) - Zone(ZoneLoop).OutDryBulbTemp) * TimeStepSys * SecInHour * ADSCorrectionFactor;
                        ZnAirRpt(ZoneLoop).OABalanceHeatGain = 0.0;
                    } else {
                        ZnAirRpt(ZoneLoop).OABalanceHeatLoss = 0.0;
                        ZnAirRpt(ZoneLoop).OABalanceHeatGain =
                            -MDotCPOA(ZoneLoop) * (MAT(ZoneLoop) - Zone(ZoneLoop).OutDryBulbTemp) * TimeStepSys * SecInHour * ADSCorrectionFactor;
                    }
                    H2OHtOfVap = PsyHgAirFnWTdb(OutHumRat, Zone(ZoneLoop).OutDryBulbTemp);
                    if (ZoneAirHumRat(ZoneLoop) > OutHumRat) {
                        ZnAirRpt(ZoneLoop).OABalanceLatentLoss = 0.001 * MDotOA(ZoneLoop) * (ZoneAirHumRat(ZoneLoop) - OutHumRat) * H2OHtOfVap *
                                                                 TimeStepSys * SecInHour * 1000.0 * ADSCorrectionFactor;
                        ZnAirRpt(ZoneLoop).OABalanceLatentGain = 0.0;
                    } else if (ZoneAirHumRat(ZoneLoop) <= OutHumRat) {
                        ZnAirRpt(ZoneLoop).OABalanceLatentGain = 0.001 * MDotOA(ZoneLoop) * (OutHumRat - ZoneAirHumRat(ZoneLoop)) * H2OHtOfVap *
                                                                 TimeStepSys * SecInHour * 1000.0 * ADSCorrectionFactor;
                        ZnAirRpt(ZoneLoop).OABalanceLatentLoss = 0.0;
                    }
                    // Total ventilation losses and gains
                    TotalLoad = ZnAirRpt(ZoneLoop).OABalanceHeatGain + ZnAirRpt(ZoneLoop).OABalanceLatentGain - ZnAirRpt(ZoneLoop).OABalanceHeatLoss -
                                ZnAirRpt(ZoneLoop).OABalanceLatentLoss;
                    if (TotalLoad > 0) {
                        ZnAirRpt(ZoneLoop).OABalanceTotalGain = TotalLoad * ADSCorrectionFactor;
                        ZnAirRpt(ZoneLoop).OABalanceTotalLoss = 0.0;
                    } else {
                        ZnAirRpt(ZoneLoop).OABalanceTotalGain = 0.0;
                        ZnAirRpt(ZoneLoop).OABalanceTotalLoss = -TotalLoad * ADSCorrectionFactor;
                    }
                    ZnAirRpt(ZoneLoop).OABalanceMass = (MDotOA(ZoneLoop)) * TimeStepSys * SecInHour * ADSCorrectionFactor;
                    ZnAirRpt(ZoneLoop).OABalanceMdot = (MDotOA(ZoneLoop)) * ADSCorrectionFactor;
                    AirDensity = PsyRhoAirFnPbTdbW(OutBaroPress, MAT(ZoneLoop), ZoneAirHumRatAvg(ZoneLoop), BlankString);
                    ZnAirRpt(ZoneLoop).OABalanceVolumeCurDensity = (MDotOA(ZoneLoop) / AirDensity) * TimeStepSys * SecInHour * ADSCorrectionFactor;
                    ZnAirRpt(ZoneLoop).OABalanceAirChangeRate = ZnAirRpt(ZoneLoop).OABalanceVolumeCurDensity / (TimeStepSys * Zone(ZoneLoop).Volume);
                    ZnAirRpt(ZoneLoop).OABalanceVdotCurDensity = (MDotOA(ZoneLoop) / AirDensity) * ADSCorrectionFactor;
                    AirDensity = StdRhoAir;
                    ZnAirRpt(ZoneLoop).OABalanceVolumeStdDensity = (MDotOA(ZoneLoop) / AirDensity) * TimeStepSys * SecInHour * ADSCorrectionFactor;
                    ZnAirRpt(ZoneLoop).OABalanceVdotStdDensity = (MDotOA(ZoneLoop) / AirDensity) * ADSCorrectionFactor;
                    ZnAirRpt(ZoneLoop).OABalanceFanElec = ZnAirRpt(ZoneLoop).VentilFanElec;
                }
            }
            // Reports exfiltration loss
            H2OHtOfVap = PsyHgAirFnWTdb(OutHumRat, Zone(ZoneLoop).OutDryBulbTemp);
            ZnAirRpt(ZoneLoop).SysInletMass = 0;
            ZnAirRpt(ZoneLoop).SysOutletMass = 0;
            if (!ZoneEquipConfig(ZoneLoop).IsControlled) {
                for (int j = 1; j <= ZoneEquipConfig(ZoneLoop).NumInletNodes; ++j) {
                    ZnAirRpt(ZoneLoop).SysInletMass +=
                        Node(ZoneEquipConfig(ZoneLoop).InletNode(j)).MassFlowRate * TimeStepSys * SecInHour * ADSCorrectionFactor;
                }
                for (int j = 1; j <= ZoneEquipConfig(ZoneLoop).NumExhaustNodes; ++j) {
                    ZnAirRpt(ZoneLoop).SysOutletMass +=
                        Node(ZoneEquipConfig(ZoneLoop).ExhaustNode(j)).MassFlowRate * TimeStepSys * SecInHour * ADSCorrectionFactor;
                }
                for (int j = 1; j <= ZoneEquipConfig(ZoneLoop).NumReturnNodes; ++j) {
                    ZnAirRpt(ZoneLoop).SysOutletMass +=
                        Node(ZoneEquipConfig(ZoneLoop).ReturnNode(j)).MassFlowRate * TimeStepSys * SecInHour * ADSCorrectionFactor;
                }
            }

            ZnAirRpt(ZoneLoop).ExfilMass = ZnAirRpt(ZoneLoop).InfilMass + ZnAirRpt(ZoneLoop).VentilMass + ZnAirRpt(ZoneLoop).MixMass +
                                           ZnAirRpt(ZoneLoop).OABalanceMass + ZnAirRpt(ZoneLoop).SysInletMass -
                                           ZnAirRpt(ZoneLoop).SysOutletMass; // kg
            ZnAirRpt(ZoneLoop).ExfilSensiLoss =
                ZnAirRpt(ZoneLoop).ExfilMass / (TimeStepSys * SecInHour) * (MAT(ZoneLoop) - Zone(ZoneLoop).OutDryBulbTemp) * CpAir; // W
            ZnAirRpt(ZoneLoop).ExfilLatentLoss =
                ZnAirRpt(ZoneLoop).ExfilMass / (TimeStepSys * SecInHour) * (ZoneAirHumRat(ZoneLoop) - OutHumRat) * H2OHtOfVap;
            ZnAirRpt(ZoneLoop).ExfilTotalLoss = ZnAirRpt(ZoneLoop).ExfilLatentLoss + ZnAirRpt(ZoneLoop).ExfilSensiLoss;

            ZoneTotalExfiltrationHeatLoss += ZnAirRpt(ZoneLoop).ExfilTotalLoss * TimeStepSys * SecInHour;
            ZoneTotalExhaustHeatLoss += ZnAirRpt(ZoneLoop).ExhTotalLoss * TimeStepSys * SecInHour;
        }
    }

    void SetHeatToReturnAirFlag(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Fred Buhl
        //       DATE WRITTEN   February 2008
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This sets some flags at the air loop and zone level: these flags indicate
        // whether an air loop represents a "unitary" system, and whether the system is operating
        // in a on/off (cycling fan) mode. At the zone level flags are set to indicate whether
        // the zone is served by a zonal system only, and whether the air loop serving the zone (idf any)
        // is in cycling fan mode. Using this information, the subroutine sets a flag at the zone level
        // to tell ManageZoneAirUpdates (predict and correct) what to do with the heat to return air.

        // METHODOLOGY EMPLOYED:
        // Uses program data structures state.dataAirLoop->AirLoopControlInfo and ZoneEquipInfo

        // REFERENCES:
        // na

        // Using/Aliasing
        using DataHeatBalance::Lights;
        using DataHeatBalance::TotLights;
        using DataHeatBalance::Zone;
        using DataHVACGlobals::NumPrimaryAirSys;
        using DataSurfaces::AirFlowWindow_Destination_ReturnAir;
        using DataZoneEquipment::ZoneEquipConfig;
        using ScheduleManager::CheckScheduleValue;
        using ScheduleManager::GetCurrentScheduleValue;
        using ScheduleManager::GetScheduleMaxValue;

        // Locals
        // SUBROUTINE ARGUMENTS:

        // SUBROUTINE PARAMETER DEFINITIONS:
        // na

        // INTERFACE BLOCK SPECIFICATIONS:
        // na

        // DERIVED TYPE DEFINITIONS:
        // na

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        static int AirLoopNum(0); // the air loop index
        int ControlledZoneNum;    // controlled zone index
        bool CyclingFan(false);   // TRUE means air loop operates in cycling fan mode at some point
        static int ZoneNum(0);           // zone index
        int LightNum;                    // Lights object index
        int SurfNum;                     // Surface index
        static Real64 CycFanMaxVal(0.0); // max value of cycling fan schedule

        if (!AirLoopsSimOnce) return;

        if (MyOneTimeFlag) {
            // set the air loop Any Continuous Fan flag
            for (AirLoopNum = 1; AirLoopNum <= NumPrimaryAirSys; ++AirLoopNum) {
                if (state.dataAirLoop->AirLoopControlInfo(AirLoopNum).UnitarySys) { // for unitary systems check the cycling fan schedule
                    if (state.dataAirLoop->AirLoopControlInfo(AirLoopNum).CycFanSchedPtr > 0) {
                        CycFanMaxVal = GetScheduleMaxValue(state.dataAirLoop->AirLoopControlInfo(AirLoopNum).CycFanSchedPtr);
                        if (CycFanMaxVal > 0.0) {
                            state.dataAirLoop->AirLoopControlInfo(AirLoopNum).AnyContFan = true;
                        } else {
                            state.dataAirLoop->AirLoopControlInfo(AirLoopNum).AnyContFan = false;
                        }
                    } else { // no schedule means always cycling fan
                        state.dataAirLoop->AirLoopControlInfo(AirLoopNum).AnyContFan = false;
                    }
                } else { // for nonunitary (central) all systems are continuous fan
                    state.dataAirLoop->AirLoopControlInfo(AirLoopNum).AnyContFan = true;
                }
            }
            // check to see if a controlled zone is served exclusively by a zonal system
            for (ControlledZoneNum = 1; ControlledZoneNum <= NumOfZones; ++ControlledZoneNum) {
                ZoneNum = ZoneEquipConfig(ControlledZoneNum).ActualZoneNum;
                bool airLoopFound = false;
                for (int zoneInNode = 1; zoneInNode <= ZoneEquipConfig(ControlledZoneNum).NumInletNodes; ++zoneInNode) {
                    if (ZoneEquipConfig(ControlledZoneNum).InletNodeAirLoopNum(zoneInNode) > 0) {
                        airLoopFound = true;
                    }
                }
                if (!airLoopFound && ZoneEquipConfig(ControlledZoneNum).NumInletNodes == ZoneEquipConfig(ControlledZoneNum).NumExhaustNodes) {
                    ZoneEquipConfig(ControlledZoneNum).ZonalSystemOnly = true;
                }
            }
            // issue warning messages if zone is served by a zonal system or a cycling system and the input calls for
            // heat gain to return air
            for (ControlledZoneNum = 1; ControlledZoneNum <= NumOfZones; ++ControlledZoneNum) {
                if (!ZoneEquipConfig(ControlledZoneNum).IsControlled) continue;
                ZoneNum = ZoneEquipConfig(ControlledZoneNum).ActualZoneNum;
                CyclingFan = false;
                for (int zoneInNode = 1; zoneInNode <= ZoneEquipConfig(ControlledZoneNum).NumInletNodes; ++zoneInNode) {
                    AirLoopNum = ZoneEquipConfig(ControlledZoneNum).InletNodeAirLoopNum(zoneInNode);
                    if (AirLoopNum > 0) {
                        if (state.dataAirLoop->AirLoopControlInfo(AirLoopNum).CycFanSchedPtr > 0) {
                            CyclingFan = CheckScheduleValue(state.dataAirLoop->AirLoopControlInfo(AirLoopNum).CycFanSchedPtr, 0.0);
                        }
                    }
                }
                if (ZoneEquipConfig(ControlledZoneNum).ZonalSystemOnly || CyclingFan) {
                    if (Zone(ZoneNum).RefrigCaseRA) {
                        ShowWarningError("For zone=" + Zone(ZoneNum).Name +
                                         " return air cooling by refrigerated cases will be applied to the zone air.");
                        ShowContinueError("  This zone has no return air or is served by an on/off HVAC system.");
                    }
                    for (LightNum = 1; LightNum <= TotLights; ++LightNum) {
                        if (Lights(LightNum).ZonePtr != ZoneNum) continue;
                        if (Lights(LightNum).FractionReturnAir > 0.0) {
                            ShowWarningError("For zone=" + Zone(ZoneNum).Name + " return air heat gain from lights will be applied to the zone air.");
                            ShowContinueError("  This zone has no return air or is served by an on/off HVAC system.");
                            break;
                        }
                    }
                    for (SurfNum = Zone(ZoneNum).SurfaceFirst; SurfNum <= Zone(ZoneNum).SurfaceLast; ++SurfNum) {
                        if (DataSurfaces::SurfWinAirflowDestination(SurfNum) == AirFlowWindow_Destination_ReturnAir) {
                            ShowWarningError("For zone=" + Zone(ZoneNum).Name +
                                             " return air heat gain from air flow windows will be applied to the zone air.");
                            ShowContinueError("  This zone has no return air or is served by an on/off HVAC system.");
                        }
                    }
                }
            }
            MyOneTimeFlag = false;
        }

        // set the air loop fan operation mode
        for (AirLoopNum = 1; AirLoopNum <= NumPrimaryAirSys; ++AirLoopNum) {
            if (state.dataAirLoop->AirLoopControlInfo(AirLoopNum).CycFanSchedPtr > 0) {
                if (GetCurrentScheduleValue(state.dataAirLoop->AirLoopControlInfo(AirLoopNum).CycFanSchedPtr) == 0.0) {
                    state.dataAirLoop->AirLoopControlInfo(AirLoopNum).FanOpMode = CycFanCycCoil;
                } else {
                    state.dataAirLoop->AirLoopControlInfo(AirLoopNum).FanOpMode = ContFanCycCoil;
                }
            }
        }
        // set the zone level NoHeatToReturnAir flag
        // if any air loop in the zone is continuous fan, then set NoHeatToReturnAir = false and sort it out node-by-node
        for (ControlledZoneNum = 1; ControlledZoneNum <= NumOfZones; ++ControlledZoneNum) {
            if (!ZoneEquipConfig(ControlledZoneNum).IsControlled) continue;
            ZoneNum = ZoneEquipConfig(ControlledZoneNum).ActualZoneNum;
            Zone(ZoneNum).NoHeatToReturnAir = true;
            if (!ZoneEquipConfig(ControlledZoneNum).ZonalSystemOnly) {
                for (int zoneInNode = 1; zoneInNode <= ZoneEquipConfig(ControlledZoneNum).NumInletNodes; ++zoneInNode) {
                    AirLoopNum = ZoneEquipConfig(ControlledZoneNum).InletNodeAirLoopNum(zoneInNode);
                    if (AirLoopNum > 0) {
                        if (state.dataAirLoop->AirLoopControlInfo(AirLoopNum).FanOpMode == ContFanCycCoil) {
                            Zone(ZoneNum).NoHeatToReturnAir = false;
                            break;
                        }
                    }
                }
            }
        }
    }

    void UpdateZoneInletConvergenceLog()
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         <author>
        //       DATE WRITTEN   <date_written>
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // <description>

        // METHODOLOGY EMPLOYED:
        // <description>

        // REFERENCES:
        // na

        // Using/Aliasing
        using DataConvergParams::ConvergLogStackDepth;
        using DataConvergParams::ZoneInletConvergence;
        using DataGlobals::NumOfZones;
        using DataLoopNode::Node;

        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:
        // na

        // SUBROUTINE PARAMETER DEFINITIONS:
        // na

        // INTERFACE BLOCK SPECIFICATIONS:
        // na

        // DERIVED TYPE DEFINITIONS:
        // na

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int ZoneNum;
        int NodeIndex;
        int NodeNum;
        Array1D<Real64> tmpRealARR(ConvergLogStackDepth);

        for (ZoneNum = 1; ZoneNum <= NumOfZones; ++ZoneNum) {

            for (NodeIndex = 1; NodeIndex <= ZoneInletConvergence(ZoneNum).NumInletNodes; ++NodeIndex) {
                NodeNum = ZoneInletConvergence(ZoneNum).InletNode(NodeIndex).NodeNum;

                tmpRealARR = ZoneInletConvergence(ZoneNum).InletNode(NodeIndex).HumidityRatio;
                ZoneInletConvergence(ZoneNum).InletNode(NodeIndex).HumidityRatio(1) = Node(NodeNum).HumRat;
                ZoneInletConvergence(ZoneNum).InletNode(NodeIndex).HumidityRatio({2, ConvergLogStackDepth}) =
                    tmpRealARR({1, ConvergLogStackDepth - 1});

                tmpRealARR = ZoneInletConvergence(ZoneNum).InletNode(NodeIndex).MassFlowRate;
                ZoneInletConvergence(ZoneNum).InletNode(NodeIndex).MassFlowRate(1) = Node(NodeNum).MassFlowRate;
                ZoneInletConvergence(ZoneNum).InletNode(NodeIndex).MassFlowRate({2, ConvergLogStackDepth}) =
                    tmpRealARR({1, ConvergLogStackDepth - 1});

                tmpRealARR = ZoneInletConvergence(ZoneNum).InletNode(NodeIndex).Temperature;
                ZoneInletConvergence(ZoneNum).InletNode(NodeIndex).Temperature(1) = Node(NodeNum).Temp;
                ZoneInletConvergence(ZoneNum).InletNode(NodeIndex).Temperature({2, ConvergLogStackDepth}) = tmpRealARR({1, ConvergLogStackDepth - 1});
            }
        }
    }

    void CheckAirLoopFlowBalance(EnergyPlusData &state)
    {
        // Check for unbalanced airloop
        if (!DataGlobals::WarmupFlag && AirLoopsSimOnce) {
            for (int AirLoopNum = 1; AirLoopNum <= NumPrimaryAirSys; ++AirLoopNum) {
                auto &thisAirLoopFlow(state.dataAirLoop->AirLoopFlow(AirLoopNum));
                if (!thisAirLoopFlow.FlowError) {
                    Real64 unbalancedExhaustDelta = thisAirLoopFlow.SupFlow - thisAirLoopFlow.OAFlow - thisAirLoopFlow.SysRetFlow;
                    if (unbalancedExhaustDelta > SmallMassFlow) {
                        ShowSevereError("CheckAirLoopFlowBalance: AirLoopHVAC " + DataAirSystems::PrimaryAirSystem(AirLoopNum).Name +
                                        " is unbalanced. Supply is > return plus outdoor air.");
                        ShowContinueErrorTimeStamp("");
                        ShowContinueError("  Flows [m3/s at standard density]: Supply=" +
                                          General::RoundSigDigits(thisAirLoopFlow.SupFlow / DataEnvironment::StdRhoAir, 6) +
                                          "  Return=" + General::RoundSigDigits(thisAirLoopFlow.SysRetFlow / DataEnvironment::StdRhoAir, 6) +
                                          "  Outdoor Air=" + General::RoundSigDigits(thisAirLoopFlow.OAFlow / DataEnvironment::StdRhoAir, 6));
                        ShowContinueError("  Imbalance=" + General::RoundSigDigits(unbalancedExhaustDelta / DataEnvironment::StdRhoAir, 6));
                        ShowContinueError("  This error will only be reported once per system.");
                        thisAirLoopFlow.FlowError = true;
                    }
                }
            }
        }
    }

} // namespace HVACManager

} // namespace EnergyPlus
