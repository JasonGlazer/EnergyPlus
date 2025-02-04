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
#include <vector>

// ObjexxFCL Headers
#include <ObjexxFCL/gio.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataErrorTracking.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/DataReportingFlags.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/DataSystemVariables.hh>
#include <EnergyPlus/DisplayRoutines.hh>
#include <EnergyPlus/EMSManager.hh>
#include <EnergyPlus/ExteriorEnergyUse.hh>
#include <EnergyPlus/FluidProperties.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/HVACSizingSimulationManager.hh>
#include <EnergyPlus/HeatBalanceManager.hh>
#include <EnergyPlus/Plant/DataPlant.hh>
#include <EnergyPlus/PlantPipingSystemsManager.hh>
#include <EnergyPlus/SQLiteProcedures.hh>
#include <EnergyPlus/SimulationManager.hh>
#include <EnergyPlus/UtilityRoutines.hh>
#include <EnergyPlus/WeatherManager.hh>

namespace EnergyPlus {

void HVACSizingSimulationManager::DetermineSizingAnalysesNeeded(EnergyPlusData &state)
{
    using DataSizing::Coincident;
    using DataSizing::NumPltSizInput;
    using DataSizing::PlantSizData;

    // currently the only type of advanced sizing analysis available is for coincident plant sizing
    // expect more specialized sizing analysis objects to be added, so minimizing code here and jump to a worker method once we know an instance is to
    // be created.

    // Loop over PlantSizData struct and find those plant loops that are to use coincident sizing
    for (int i = 1; i <= NumPltSizInput; ++i) {

        if (PlantSizData(i).ConcurrenceOption == Coincident) {

            // create an instance of analysis object for each loop
            CreateNewCoincidentPlantAnalysisObject(state, PlantSizData(i).PlantLoopName, i);
        }
    }
}

void HVACSizingSimulationManager::CreateNewCoincidentPlantAnalysisObject(EnergyPlusData &state, std::string const &PlantLoopName, int const PlantSizingIndex)
{
    using DataPlant::PlantLoop;
    using DataPlant::SupplySide;
    using DataPlant::TotNumLoops;
    using namespace FluidProperties;
    using DataSizing::PlantSizData;

    Real64 density;
    Real64 cp;

    // find plant loop number
    for (int i = 1; i <= TotNumLoops; ++i) {
        if (PlantLoopName == PlantLoop(i).Name) { // found it

            density = GetDensityGlycol(
                state, PlantLoop(i).FluidName, DataGlobals::CWInitConvTemp, PlantLoop(i).FluidIndex, "createNewCoincidentPlantAnalysisObject");
            cp = GetSpecificHeatGlycol(
                state, PlantLoop(i).FluidName, DataGlobals::CWInitConvTemp, PlantLoop(i).FluidIndex, "createNewCoincidentPlantAnalysisObject");

            plantCoincAnalyObjs.emplace_back(PlantLoopName,
                                             i,
                                             PlantLoop(i).LoopSide(SupplySide).NodeNumIn,
                                             density,
                                             cp,
                                             PlantSizData(PlantSizingIndex).NumTimeStepsInAvg,
                                             PlantSizingIndex);
        }
    }
}

void HVACSizingSimulationManager::SetupSizingAnalyses(EnergyPlusData &state)
{
    using DataLoopNode::Node;
    using DataSizing::CondenserLoop;
    using DataSizing::CoolingLoop;
    using DataSizing::HeatingLoop;
    using DataSizing::PlantSizData;
    using DataSizing::SteamLoop;

    for (auto &P : plantCoincAnalyObjs) {
        // call setup log routine for each coincident plant analysis object
        P.supplyInletNodeFlow_LogIndex = sizingLogger.SetupVariableSizingLog(state, Node(P.supplySideInletNodeNum).MassFlowRate, P.numTimeStepsInAvg);
        P.supplyInletNodeTemp_LogIndex = sizingLogger.SetupVariableSizingLog(state, Node(P.supplySideInletNodeNum).Temp, P.numTimeStepsInAvg);
        if (PlantSizData(P.plantSizingIndex).LoopType == HeatingLoop || PlantSizData(P.plantSizingIndex).LoopType == SteamLoop) {
            P.loopDemand_LogIndex = sizingLogger.SetupVariableSizingLog(state, DataPlant::PlantLoop(P.plantLoopIndex).HeatingDemand, P.numTimeStepsInAvg);
        } else if (PlantSizData(P.plantSizingIndex).LoopType == CoolingLoop || PlantSizData(P.plantSizingIndex).LoopType == CondenserLoop) {
            P.loopDemand_LogIndex = sizingLogger.SetupVariableSizingLog(state, DataPlant::PlantLoop(P.plantLoopIndex).CoolingDemand, P.numTimeStepsInAvg);
        }
    }
}

void HVACSizingSimulationManager::PostProcessLogs()
{
    // this function calls methods on log objects to do general processing on all the logged data in the framework
    for (auto &L : sizingLogger.logObjs) {
        L.AverageSysTimeSteps();   // collapse subtimestep data into zone step data
        L.ProcessRunningAverage(); // apply zone step moving average
    }
}

void HVACSizingSimulationManager::ProcessCoincidentPlantSizeAdjustments(EnergyPlusData &state, int const HVACSizingIterCount)
{
    using namespace DataPlant;
    using namespace PlantManager;
    using namespace DataSizing;
    using DataGlobals::FinalSizingHVACSizingSimIteration;

    // first pass through coincident plant objects to check new sizes and see if more iteration needed
    plantCoinAnalyRequestsAnotherIteration = false;
    for (auto &P : plantCoincAnalyObjs) {
        // step 1 find maximum flow rate on concurrent return temp and load
        P.newFoundMassFlowRateTimeStamp = sizingLogger.logObjs[P.supplyInletNodeFlow_LogIndex].GetLogVariableDataMax();
        P.peakMdotCoincidentDemand = sizingLogger.logObjs[P.loopDemand_LogIndex].GetLogVariableDataAtTimestamp(P.newFoundMassFlowRateTimeStamp);
        P.peakMdotCoincidentReturnTemp =
            sizingLogger.logObjs[P.supplyInletNodeTemp_LogIndex].GetLogVariableDataAtTimestamp(P.newFoundMassFlowRateTimeStamp);

        // step 2 find maximum load and concurrent flow and return temp
        P.NewFoundMaxDemandTimeStamp = sizingLogger.logObjs[P.loopDemand_LogIndex].GetLogVariableDataMax();
        P.peakDemandMassFlow = sizingLogger.logObjs[P.supplyInletNodeFlow_LogIndex].GetLogVariableDataAtTimestamp(P.NewFoundMaxDemandTimeStamp);
        P.peakDemandReturnTemp = sizingLogger.logObjs[P.supplyInletNodeTemp_LogIndex].GetLogVariableDataAtTimestamp(P.NewFoundMaxDemandTimeStamp);

        P.ResolveDesignFlowRate(state, HVACSizingIterCount);
        if (P.anotherIterationDesired) {
            plantCoinAnalyRequestsAnotherIteration = true;
        }
    }

    // as more sizing adjustments are added this will need to change to consider all not just plant coincident
    FinalSizingHVACSizingSimIteration = plantCoinAnalyRequestsAnotherIteration;
}
void HVACSizingSimulationManager::RedoKickOffAndResize(EnergyPlusData &state)
{
    using DataGlobals::KickOffSimulation;
    using DataGlobals::RedoSizesHVACSimulation;
    using namespace WeatherManager;
    using namespace SimulationManager;
    bool ErrorsFound(false);
    KickOffSimulation = true;
    RedoSizesHVACSimulation = true;

    ResetEnvironmentCounter(state);
    SetupSimulation(state, ErrorsFound);

    KickOffSimulation = false;
    RedoSizesHVACSimulation = false;
}

void HVACSizingSimulationManager::UpdateSizingLogsZoneStep(EnergyPlusData &state)
{
    sizingLogger.UpdateSizingLogValuesZoneStep(state);
}

void HVACSizingSimulationManager::UpdateSizingLogsSystemStep(EnergyPlusData &state)
{
    sizingLogger.UpdateSizingLogValuesSystemStep(state);
}

std::unique_ptr<HVACSizingSimulationManager> hvacSizingSimulationManager;

void ManageHVACSizingSimulation(EnergyPlusData &state, bool &ErrorsFound)
{
    using DataEnvironment::CurMnDy;
    using DataEnvironment::CurrentOverallSimDay;
    using DataEnvironment::EnvironmentName;
    using DataEnvironment::TotalOverallSimDays;
    using DataErrorTracking::ExitDuringSimulations;
    using DataSystemVariables::ReportDuringHVACSizingSimulation;
    using EMSManager::ManageEMS;
    using ExteriorEnergyUse::ManageExteriorEnergyUse;
    using General::TrimSigDigits;
    using PlantPipingSystemsManager::SimulateGroundDomains;

    using namespace WeatherManager;
    using namespace DataGlobals;
    using namespace DataReportingFlags;
    using namespace HeatBalanceManager;

    hvacSizingSimulationManager = std::unique_ptr<HVACSizingSimulationManager>(new HVACSizingSimulationManager());

    bool Available; // an environment is available to process
    int HVACSizingIterCount;

    hvacSizingSimulationManager->DetermineSizingAnalysesNeeded(state);

    hvacSizingSimulationManager->SetupSizingAnalyses(state);

    DisplayString("Beginning HVAC Sizing Simulation");
    DoingHVACSizingSimulations = true;
    DoOutputReporting = true;

    ResetEnvironmentCounter(state);

    // iterations over set of sizing periods for HVAC sizing Simulation, will break out if no more are needed
    for (HVACSizingIterCount = 1; HVACSizingIterCount <= HVACSizingSimMaxIterations; ++HVACSizingIterCount) {

        // need to extend Environment structure array to distinguish the HVAC Sizing Simulations from the regular run of that sizing period, repeats
        // for each set
        AddDesignSetToEnvironmentStruct(state, HVACSizingIterCount);

        WarmupFlag = true;
        Available = true;
        for (int i = 1; i <= state.dataWeatherManager->NumOfEnvrn; ++i) { // loop over environments

            GetNextEnvironment(state, Available, ErrorsFound);
            if (ErrorsFound) break;
            if (!Available) continue;

            hvacSizingSimulationManager->sizingLogger.SetupSizingLogsNewEnvironment(state);

            //	if (!DoDesDaySim) continue; // not sure about this, may need to force users to set this on input for this method, but maybe not
            if (KindOfSim == ksRunPeriodWeather) continue;
            if (KindOfSim == ksDesignDay) continue;
            if (KindOfSim == ksRunPeriodDesign) continue;

            if (state.dataWeatherManager->Environment(state.dataWeatherManager->Envrn).HVACSizingIterationNum != HVACSizingIterCount) continue;

            if (ReportDuringHVACSizingSimulation) {
                if (sqlite) {
                    sqlite->sqliteBegin();
                    sqlite->createSQLiteEnvironmentPeriodRecord(
                        DataEnvironment::CurEnvirNum, DataEnvironment::EnvironmentName, DataGlobals::KindOfSim);
                    sqlite->sqliteCommit();
                }
            }
            ExitDuringSimulations = true;

            DisplayString("Initializing New Environment Parameters, HVAC Sizing Simulation");

            BeginEnvrnFlag = true;
            if ((KindOfSim == ksHVACSizeDesignDay) && (state.dataWeatherManager->DesDayInput(state.dataWeatherManager->Environment(state.dataWeatherManager->Envrn).DesignDayNum).suppressBegEnvReset)) {
                // user has input in SizingPeriod:DesignDay directing to skip begin environment rests, for accuracy-with-speed as zones can more
                // easily converge fewer warmup days are allowed
                DisplayString("Suppressing Initialization of New Environment Parameters");
                DataGlobals::beginEnvrnWarmStartFlag = true;
            } else {
                DataGlobals::beginEnvrnWarmStartFlag = false;
            }
            EndEnvrnFlag = false;
            // EndMonthFlag = false;
            WarmupFlag = true;
            DayOfSim = 0;
            state.dataGlobal->DayOfSimChr = "0";
            NumOfWarmupDays = 0;

            bool anyEMSRan;
            ManageEMS(state, DataGlobals::emsCallFromBeginNewEvironment, anyEMSRan, ObjexxFCL::Optional_int_const()); // calling point

            while ((DayOfSim < NumOfDayInEnvrn) || (WarmupFlag)) { // Begin day loop ...

                if (ReportDuringHVACSizingSimulation) {
                    if (sqlite) sqlite->sqliteBegin(); // setup for one transaction per day
                }
                ++DayOfSim;
                state.dataGlobal->DayOfSimChr = fmt::to_string(DayOfSim);
                if (!WarmupFlag) {
                    ++CurrentOverallSimDay;
                    DisplaySimDaysProgress(CurrentOverallSimDay, TotalOverallSimDays);
                } else {
                    state.dataGlobal->DayOfSimChr = "0";
                }
                BeginDayFlag = true;
                EndDayFlag = false;

                if (WarmupFlag) {
                    ++NumOfWarmupDays;
                    cWarmupDay = TrimSigDigits(NumOfWarmupDays);
                    DisplayString("Warming up {" + cWarmupDay + '}');
                } else if (DayOfSim == 1) {
                    DisplayString("Starting HVAC Sizing Simulation at " + CurMnDy + " for " + EnvironmentName);
                    static constexpr auto Format_700("Environment:WarmupDays,{:3}\n");
                    print(state.files.eio, Format_700, NumOfWarmupDays);
                } else if (DisplayPerfSimulationFlag) {
                    DisplayString("Continuing Simulation at " + CurMnDy + " for " + EnvironmentName);
                    DisplayPerfSimulationFlag = false;
                }

                for (HourOfDay = 1; HourOfDay <= 24; ++HourOfDay) { // Begin hour loop ...

                    BeginHourFlag = true;
                    EndHourFlag = false;

                    for (TimeStep = 1; TimeStep <= NumOfTimeStepInHour; ++TimeStep) {
                        if (AnySlabsInModel || AnyBasementsInModel) {
                            SimulateGroundDomains(state, false);
                        }

                        BeginTimeStepFlag = true;

                        // Set the End__Flag variables to true if necessary.  Note that
                        // each flag builds on the previous level.  EndDayFlag cannot be
                        // .TRUE. unless EndHourFlag is also .TRUE., etc.  Note that the
                        // EndEnvrnFlag and the EndSimFlag cannot be set during warmup.
                        // Note also that BeginTimeStepFlag, EndTimeStepFlag, and the
                        // SubTimeStepFlags can/will be set/reset in the HVAC Manager.

                        if (TimeStep == NumOfTimeStepInHour) {
                            EndHourFlag = true;
                            if (HourOfDay == 24) {
                                EndDayFlag = true;
                                if (!WarmupFlag && (DayOfSim == NumOfDayInEnvrn)) {
                                    EndEnvrnFlag = true;
                                }
                            }
                        }

                        ManageWeather(state);

                        ManageExteriorEnergyUse(state);

                        ManageHeatBalance(state);

                        BeginHourFlag = false;
                        BeginDayFlag = false;
                        BeginEnvrnFlag = false;
                        BeginSimFlag = false;
                        BeginFullSimFlag = false;

                    } // TimeStep loop

                    PreviousHour = HourOfDay;

                } // ... End hour loop.
                if (ReportDuringHVACSizingSimulation) {
                    if (sqlite) sqlite->sqliteCommit(); // one transaction per day
                }
            } // ... End day loop.

        } // ... End environment loop.

        if (ErrorsFound) {
            ShowFatalError("Error condition occurred.  Previous Severe Errors cause termination.");
        }

        hvacSizingSimulationManager->PostProcessLogs();

        hvacSizingSimulationManager->ProcessCoincidentPlantSizeAdjustments(state, HVACSizingIterCount);

        hvacSizingSimulationManager->RedoKickOffAndResize(state);

        if (!hvacSizingSimulationManager->plantCoinAnalyRequestsAnotherIteration) {
            // jump out of for loop, or change for to a while
            break;
        }

        hvacSizingSimulationManager->sizingLogger.IncrementSizingPeriodSet();

    } // End HVAC Sizing Iteration loop

    WarmupFlag = false;
    DoOutputReporting = true;
    DoingHVACSizingSimulations = false;
    hvacSizingSimulationManager.reset(); // delete/reset unique_ptr
}
} // namespace EnergyPlus
