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
#include <ostream>

// ObjexxFCL Headers
#include <ObjexxFCL/numeric.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/IOFiles.hh>

namespace EnergyPlus {

namespace DataGlobals {

    // MODULE INFORMATION:
    //       AUTHOR         Rick Strand
    //       DATE WRITTEN   January 1997
    //       MODIFIED       May 1997 (RKS) Added Weather Variables
    //       MODIFIED       December 1997 (RKS,DF,LKL) Split into DataGlobals and DataEnvironment
    //       MODIFIED       February 1999 (FW) Added NextHour, WGTNEXT, WGTNOW
    //       MODIFIED       September 1999 (LKL) Rename WGTNEXT,WGTNOW for clarity
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS MODULE:
    // This data-only module is a repository for all variables which are considered
    // to be "global" in nature in EnergyPlus.

    // METHODOLOGY EMPLOYED:
    // na

    // REFERENCES:
    // na

    // OTHER NOTES:
    // na

    // Using/Aliasing

    // Data
    // -only module should be available to other modules and routines.
    // Thus, all variables in this module must be PUBLIC.
//    bool runReadVars(false);
//    bool DDOnlySimulation(false);
//    bool outputEpJSONConversion(false);
//    bool outputEpJSONConversionOnly(false);
//    bool isEpJSON(false);
//    bool isCBOR(false);
//    bool isMsgPack(false);
//    bool isUBJSON(false);
//    bool isBSON(false);
//    bool preserveIDFOrder(true);
//    bool stopSimulation(false);
    std::function<void (void *)> externalHVACManager;
    bool externalHVACManagerInitialized(false);

    // MODULE PARAMETER DEFINITIONS:
    int const BeginDay(1);
    int const DuringDay(2);
    int const EndDay(3);
    int const EndSysSizingCalc(5);

    // Parameters for KindOfSim
    int const ksDesignDay(1);
    int const ksRunPeriodDesign(2);
    int const ksRunPeriodWeather(3);
    int const ksHVACSizeDesignDay(4);       // a regular design day run during HVAC Sizing Simulation
    int const ksHVACSizeRunPeriodDesign(5); // a weather period design day run during HVAC Sizing Simulation
    int const ksReadAllWeatherData(6);      // a weather period for reading all weather data prior to the simulation

    Real64 const MaxEXPArg(709.78);       // maximum exponent in EXP() function
    Real64 const Pi(3.14159265358979324); // Pi 3.1415926535897932384626435
    Real64 const PiOvr2(Pi / 2.0);        // Pi/2
    Real64 const TwoPi(2.0 * Pi);         // 2*Pi 6.2831853071795864769252868
    Real64 const GravityConstant(9.807);
    Real64 const DegToRadians(Pi / 180.0);           // Conversion for Degrees to Radians
    Real64 const RadToDeg(180.0 / Pi);               // Conversion for Radians to Degrees
    Real64 const SecInHour(3600.0);                  // Conversion for hours to seconds
    Real64 const HoursInDay(24.0);                   // Number of Hours in Day
    Real64 const SecsInDay(SecInHour *HoursInDay);   // Number of seconds in Day
    Real64 const BigNumber(HUGE_(1.0));              // Max Number real used for initializations
    Real64 const rTinyValue(EPSILON(1.0));           // Tiny value to replace use of TINY(x)
    std::string::size_type const MaxNameLength(100); // Maximum Name Length in Characters -- should be the same
    // as MaxAlphaArgLength in InputProcessor module

    Real64 const KelvinConv(273.15);       // Conversion factor for C to K and K to C
    Real64 const InitConvTemp(5.05);       // [deg C], standard init vol to mass flow conversion temp
    Real64 const AutoCalculate(-99999.0);  // automatically calculate some fields.
    Real64 const CWInitConvTemp(5.05);     // [deg C], standard init chilled water vol to mass flow conversion temp
    Real64 const HWInitConvTemp(60.0);     // [deg C], standard init hot water vol to mass flow conversion temp
    Real64 const SteamInitConvTemp(100.0); // [deg C], standard init steam vol to mass flow conversion temp

    Real64 const StefanBoltzmann(5.6697E-8);     // Stefan-Boltzmann constant in W/(m2*K4)
    Real64 const UniversalGasConst(8314.462175); // (J/mol*K)

    Real64 const convertJtoGJ(1.0E-9); // Conversion factor for J to GJ

    // Parameters for EMS Calling Points
    int const emsCallFromZoneSizing(1);                           // Identity where EMS called from
    int const emsCallFromSystemSizing(2);                         // Identity where EMS called from
    int const emsCallFromBeginNewEvironment(3);                   // Identity where EMS called from
    int const emsCallFromBeginNewEvironmentAfterWarmUp(4);        // Identity where EMS called from
    int const emsCallFromBeginTimestepBeforePredictor(5);         // Identity where EMS called from
    int const emsCallFromBeforeHVACManagers(6);                   // Identity where EMS called from
    int const emsCallFromAfterHVACManagers(7);                    // Identity where EMS called from
    int const emsCallFromHVACIterationLoop(8);                    // Identity where EMS called from
    int const emsCallFromEndSystemTimestepBeforeHVACReporting(9); // Identity where EMS called from
    int const emsCallFromEndSystemTimestepAfterHVACReporting(10); // Identity where EMS called from
    int const emsCallFromEndZoneTimestepBeforeZoneReporting(11);  // Identity where EMS called from
    int const emsCallFromEndZoneTimestepAfterZoneReporting(12);   // Identity where EMS called from
    int const emsCallFromSetupSimulation(13);                     // identify where EMS called from,
    // this is for input processing only
    int const emsCallFromExternalInterface(14);                        // Identity where EMS called from
    int const emsCallFromComponentGetInput(15);                        // EMS called from end of get input for a component
    int const emsCallFromUserDefinedComponentModel(16);                // EMS called from inside a custom user component model
    int const emsCallFromUnitarySystemSizing(17);                      // EMS called from unitary system compound component
    int const emsCallFromBeginZoneTimestepBeforeInitHeatBalance(18);   // Identity where EMS called from
    int const emsCallFromBeginZoneTimestepAfterInitHeatBalance(19);    // Identity where EMS called from
    int const emsCallFromBeginZoneTimestepBeforeSetCurrentWeather(20); // Identity where EMS called from

    int const ScheduleAlwaysOn(-1); // Value when passed to schedule routines gives back 1.0 (on)

    // DERIVED TYPE DEFINITIONS:
    // na

    // INTERFACE BLOCK SPECIFICATIONS:
    // see DataOmterfaces fpr global interface statements

    // MODULE VARIABLE DECLARATIONS:

    bool BeginDayFlag(false);           // True at the start of each day, False after first time step in day
    bool BeginEnvrnFlag(false);         // True at the start of each environment, False after first time step in environ
    bool beginEnvrnWarmStartFlag(false); // Sizing Speed Up
    bool BeginHourFlag(false);          // True at the start of each hour, False after first time step in hour
    bool BeginSimFlag(false);           // True until any actual simulation (full or sizing) has begun, False after first time step
    bool BeginFullSimFlag(false);       // True until full simulation has begun, False after first time step
    bool BeginTimeStepFlag(false);      // True at the start of each time step, False after first subtime step of time step
    int DayOfSim(0);                    // Counter for days (during the simulation)
    int CalendarYear(0);                // Calendar year of the current day of simulation
    std::string CalendarYearChr;        // Calendar year of the current day of simulation (character -- for reporting)
    bool EndEnvrnFlag(false);           // True at the end of each environment (last time step of last hour of last day of environ)
    bool EndDesignDayEnvrnsFlag(false); // True at the end of the last design day environment
    // (last time step of last hour of last day of environ which is a design day)
    bool EndDayFlag(false);            // True at the end of each day (last time step of last hour of day)
    bool EndHourFlag(false);           // True at the end of each hour (last time step of hour)
    int PreviousHour(0);               // Previous Hour Index
    int HourOfDay(0);                  // Counter for hours in a simulation day
    Real64 WeightPreviousHour(0.0);    // Weighting of value for previous hour
    Real64 WeightNow(0.0);             // Weighting of value for current hour
    int NumOfDayInEnvrn(0);            // Number of days in the simulation for a particular environment
    int NumOfTimeStepInHour(0);        // Number of time steps in each hour of the simulation
    int NumOfZones(0);                 // Total number of Zones for simulation
    int TimeStep(0);                   // Counter for time steps (fractional hours)
    Real64 TimeStepZone(0.0);          // Zone time step in fractional hours
    bool WarmupFlag(false);            // True during the warmup portion of a simulation
    int StdOutputRecordCount(0);                     // Count of Standard output records
    int StdMeterRecordCount(0);                      // Count of Meter output records
    bool ZoneSizingCalc(false);                      // TRUE if zone sizing calculation
    bool SysSizingCalc(false);                       // TRUE if system sizing calculation
    bool DoZoneSizing(false);                        // User input in SimulationControl object
    bool DoSystemSizing(false);                      // User input in SimulationControl object
    bool DoPlantSizing(false);                       // User input in SimulationControl object
    bool DoDesDaySim(false);                         // User input in SimulationControl object
    bool DoWeathSim(false);                          // User input in SimulationControl object
    bool DoHVACSizingSimulation(false);              // User input in SimulationControl object
    int HVACSizingSimMaxIterations(0);               // User input in SimulationControl object
    bool WeathSimReq(false);                         // Input has a RunPeriod request
    int KindOfSim(0);                                // See parameters. (ksDesignDay, ksRunPeriodDesign, ksRunPeriodWeather)
    bool DoOutputReporting(false);                   // TRUE if variables to be written out
    bool DoingSizing(false);                         // TRUE when "sizing" is being performed (some error messages won't be displayed)
    bool DoingHVACSizingSimulations(false);          // true when HVAC Sizing Simulations are being performed.
    bool DoingInputProcessing(false);                // TRUE when "IP" is being performed (some error messages are cached)
    bool DisplayAllWarnings(false);                  // True when selection for  "DisplayAllWarnings" is entered (turns on other warning flags)
    bool DisplayExtraWarnings(false);                // True when selection for  "DisplayExtraWarnings" is entered
    bool DisplayUnusedObjects(false);                // True when selection for  "DisplayUnusedObjects" is entered
    bool DisplayUnusedSchedules(false);              // True when selection for  "DisplayUnusedSchedules" is entered
    bool DisplayAdvancedReportVariables(false);      // True when selection for  "DisplayAdvancedReportVariables" is entered
    bool DisplayZoneAirHeatBalanceOffBalance(false); // True when selection for  "DisplayZoneAirHeatBalanceOffBalance" is entered
    bool DisplayInputInAudit(false);                 // True when environmental variable "DisplayInputInAudit" is used
    bool CreateMinimalSurfaceVariables(false);       // True when selection for  "CreateMinimalSurfaceVariables" is entered
    Real64 CurrentTime(0.0);                         // CurrentTime, in fractional hours, from start of day. Uses Loads time step.
    int SimTimeSteps(0);                             // Number of (Loads) timesteps since beginning of run period (environment).
    int MinutesPerTimeStep(0);                       // Minutes per time step calculated from NumTimeStepInHour (number of minutes per load time step)
    Real64 TimeStepZoneSec(0.0);                     // Seconds per time step
    bool MetersHaveBeenInitialized(false);
    bool KickOffSimulation(false);                 // Kick off simulation -- meaning run each environment for 1 or 2 time steps.
    bool KickOffSizing(false);                     // Kick off sizing -- meaning run each environment for 1 or 2 time steps.
    bool RedoSizesHVACSimulation(false);           // doing kick off simulation for redoing sizes as part of sizing
    bool FinalSizingHVACSizingSimIteration(false); // when doing HVAC sizing Simulation
    bool AnyEnergyManagementSystemInModel(false);  // true if there is any EMS or Erl in model.  otherwise false
    bool AnyLocalEnvironmentsInModel(false);       // true if there is any local environmental data objected defined in model, otherwise false
    bool AnyPlantInModel(false);                   // true if there are any plant or condenser loops in model, otherwise false
    bool AnyIdealCondEntSetPointInModel(false);    // true if there is any ideal condenser entering set point manager in model.
    bool RunOptCondEntTemp(false);                 // true if the ideal condenser entering set point optimization is running
    bool CompLoadReportIsReq(false);               // true if the extra sizing calcs are performed to create a "pulse" for the load component report
    bool isPulseZoneSizing(false);                 // true during the set of zone sizing calcs that include the "pulse" for the load component report
    bool doLoadComponentPulseNow(false); // true for the time step that is the "pulse" for the load component report
    bool ShowDecayCurvesInEIO(false);    // true if the Radiant to Convective Decay Curves should appear in the EIO file
    bool AnySlabsInModel(false);         // true if there are any zone-coupled ground domains in the input file
    bool AnyBasementsInModel(false);     // true if there are any basements in the input file
    // Performance tradeoff globals
    bool DoCoilDirectSolutions(false);       //true if use coil direction solutions
    bool createPerfLog(false); //true if the _perflog.csv file should be created and a PerformancePrecisionTradeoffs object is used

    int Progress(0); // current progress (0-100)
    void (*fProgressPtr)(int const);
    void (*fMessagePtr)(std::string const &);
    std::function<void(int const)> progressCallback;
    std::function<void(const std::string &)> messageCallback;
    std::function<void(EnergyPlus::Error e, const std::string &)> errorCallback;

    bool eplusRunningViaAPI;

    // Clears the global data in DataGlobals.
    // Needed for unit tests, should not be normally called.
    void clear_state(IOFiles &ioFiles)
    {
        externalHVACManager = nullptr;
        externalHVACManagerInitialized = false;
        BeginDayFlag = false;
        BeginEnvrnFlag = false;
        BeginHourFlag = false;
        BeginSimFlag = false;
        BeginFullSimFlag = false;
        BeginTimeStepFlag = false;
        DayOfSim = 0;
        CalendarYear = 0;
        CalendarYearChr = "0";
        EndEnvrnFlag = false;
        EndDesignDayEnvrnsFlag = false;
        EndDayFlag = false;
        EndHourFlag = false;
        PreviousHour = 0;
        HourOfDay = 0;
        WeightPreviousHour = 0.0;
        WeightNow = 0.0;
        NumOfDayInEnvrn = 0;
        NumOfTimeStepInHour = 0;
        NumOfZones = 0;
        TimeStep = 0;
        TimeStepZone = 0.0;
        WarmupFlag = false;
        ioFiles.eso.close();
        ioFiles.err_stream.reset();
        StdOutputRecordCount = 0;
        ioFiles.debug.close();
        ioFiles.zsz.close();
        ioFiles.ssz.close();
        ioFiles.mtr.close();
        ioFiles.shade.close();
        StdMeterRecordCount = 0;
        ZoneSizingCalc = false;
        SysSizingCalc = false;
        DoZoneSizing = false;
        DoSystemSizing = false;
        DoPlantSizing = false;
        DoDesDaySim = false;
        DoWeathSim = false;
        DoHVACSizingSimulation = false;
        HVACSizingSimMaxIterations = 0;
        WeathSimReq = false;
        KindOfSim = 0;
        DoOutputReporting = false;
        DoingSizing = false;
        DoingHVACSizingSimulations = false;
        DoingInputProcessing = false;
        DisplayAllWarnings = false;
        DisplayExtraWarnings = false;
        DisplayUnusedObjects = false;
        DisplayUnusedSchedules = false;
        DisplayAdvancedReportVariables = false;
        DisplayZoneAirHeatBalanceOffBalance = false;
        DisplayInputInAudit = false;
        CreateMinimalSurfaceVariables = false;
        CurrentTime = 0.0;
        SimTimeSteps = 0;
        MinutesPerTimeStep = 0;
        TimeStepZoneSec = 0.0;
        MetersHaveBeenInitialized = false;
        KickOffSimulation = false;
        KickOffSizing = false;
        RedoSizesHVACSimulation = false;
        FinalSizingHVACSizingSimIteration = false;
        AnyEnergyManagementSystemInModel = false;
        AnyLocalEnvironmentsInModel = false;
        AnyPlantInModel = false;
        AnyIdealCondEntSetPointInModel = false;
        RunOptCondEntTemp = false;
        CompLoadReportIsReq = false;
        isPulseZoneSizing = false;
        doLoadComponentPulseNow = false;
        ShowDecayCurvesInEIO = false;
        AnySlabsInModel = false;
        AnyBasementsInModel = false;
        DoCoilDirectSolutions = false;
        Progress = 0;
        fProgressPtr = nullptr;
        fMessagePtr = nullptr;
        progressCallback = nullptr;
        messageCallback = nullptr;
        errorCallback = nullptr;
        ioFiles.mtr.close();
        ioFiles.err_stream.reset();
        eplusRunningViaAPI = false;
    }

} // namespace DataGlobals

} // namespace EnergyPlus
