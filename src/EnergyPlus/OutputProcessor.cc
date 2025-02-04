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
#include <cstdio>
#include <cstring>
#include <fstream>
#include <memory>
#include <ostream>
#include <string>
#include <unordered_set>

// ObjexxFCL Headers
#include <ObjexxFCL/Array.functions.hh>
#include <ObjexxFCL/Fmath.hh>
#include <ObjexxFCL/gio.hh>
#include <ObjexxFCL/string.functions.hh>

// EnergyPlus Headers
#include "re2/re2.h"
#include <EnergyPlus/CommandLineInterface.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataGlobalConstants.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataIPShortCuts.hh>
#include <EnergyPlus/DataOutputs.hh>
#include <EnergyPlus/DataPrecisionGlobals.hh>
#include <EnergyPlus/DataStringGlobals.hh>
#include <EnergyPlus/DataSystemVariables.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/GlobalNames.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/OutputReportPredefined.hh>
#include <EnergyPlus/ResultsFramework.hh>
#include <EnergyPlus/SQLiteProcedures.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/SortAndStringUtilities.hh>
#include <EnergyPlus/UtilityRoutines.hh>
#include <milo/dtoa.h>
#include <milo/itoa.h>

namespace EnergyPlus {

namespace OutputProcessor {

    // MODULE INFORMATION:
    //       AUTHOR         Linda Lawrie
    //       DATE WRITTEN   December 1998
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS MODULE:
    // This module contains the major Output Processor routines.
    // In addition, in this file are several routines which can be called
    // without Useing the OutputProcessor Module

    // METHODOLOGY EMPLOYED:
    // Lots of pointers and other fancy data stuff.

    // REFERENCES:
    // EnergyPlus OutputProcessor specifications.

    // OTHER NOTES:
    // na

    // Using/Aliasing
    using namespace DataPrecisionGlobals;
    using DataEnvironment::DayOfMonth;
    using DataEnvironment::DayOfWeek;
    using DataEnvironment::DSTIndicator;
    using DataEnvironment::HolidayIndex;
    using DataEnvironment::Month;
    using DataEnvironment::Year;
    using DataGlobals::DayOfSim;
    using DataGlobals::HourOfDay;
    using DataGlobals::MaxNameLength;
    using DataGlobals::MinutesPerTimeStep;
    using DataGlobals::StdOutputRecordCount;
    using namespace DataGlobalConstants;

    // Data
    // in this file should obey a USE OutputProcessor, ONLY: rule.

    // MODULE PARAMETER DEFINITIONS:
    int const ReportVDD_No(0);  // Don't report the variable dictionaries in any form
    int const ReportVDD_Yes(1); // Report the variable dictionaries in "report format"
    int const ReportVDD_IDF(2); // Report the variable dictionaries in "IDF format"

    Real64 const MinSetValue(99999999999999.0);
    Real64 const MaxSetValue(-99999999999999.0);
    int const IMinSetValue(999999);
    int const IMaxSetValue(-999999);

    int const VarType_NotFound(0); // ref: GetVariableKeyCountandType, 0 = not found
    int const VarType_Integer(1);  // ref: GetVariableKeyCountandType, 1 = integer
    int const VarType_Real(2);     // ref: GetVariableKeyCountandType, 2 = real
    int const VarType_Meter(3);    // ref: GetVariableKeyCountandType, 3 = meter
    int const VarType_Schedule(4); // ref: GetVariableKeyCountandType, 4 = schedule

    int const MeterType_Normal(0);     // Type value for normal meters
    int const MeterType_Custom(1);     // Type value for custom meters
    int const MeterType_CustomDec(2);  // Type value for custom meters that decrement another meter
    int const MeterType_CustomDiff(3); // Type value for custom meters that difference another meter

    static ObjexxFCL::gio::Fmt fmtA("(A)");
    Array1D_string const DayTypes(12,
                                  {"Sunday",
                                   "Monday",
                                   "Tuesday",
                                   "Wednesday",
                                   "Thursday",
                                   "Friday",
                                   "Saturday",
                                   "Holiday",
                                   "SummerDesignDay",
                                   "WinterDesignDay",
                                   "CustomDay1",
                                   "CustomDay2"});

    std::vector<std::string> const endUseCategoryNames = {"HEATING",
                                                          "COOLING",
                                                          "INTERIORLIGHTS",
                                                          "EXTERIORLIGHTS",
                                                          "INTERIOREQUIPMENT",
                                                          "EXTERIOREQUIPMENT",
                                                          "FANS",
                                                          "PUMPS",
                                                          "HEATREJECTION",
                                                          "HUMIDIFIER",
                                                          "HEATRECOVERY",
                                                          "WATERSYSTEMS",
                                                          "REFRIGERATION",
                                                          "COGENERATION"};

    static std::string const BlankString;
    int const UnitsStringLength(16);

    int const RVarAllocInc(1000);
    int const LVarAllocInc(1000);
    int const IVarAllocInc(10);

    //  For IP Units (tabular reports) certain resources will be put in sub-tables
    // INTEGER, PARAMETER :: RT_IPUnits_Consumption=0
    int const RT_IPUnits_Electricity(1);
    int const RT_IPUnits_Gas(2);
    int const RT_IPUnits_Cooling(3);
    int const RT_IPUnits_Water(4);
    int const RT_IPUnits_OtherKG(5);
    int const RT_IPUnits_OtherM3(6);
    int const RT_IPUnits_OtherL(7);
    int const RT_IPUnits_OtherJ(0);

    // DERIVED TYPE DEFINITIONS:

    int InstMeterCacheSize(1000);    // the maximum size of the instant meter cache used in GetInstantMeterValue
    int InstMeterCacheSizeInc(1000); // the increment for the instant meter cache used in GetInstantMeterValue
    Array1D_int InstMeterCache;      // contains a list of RVariableTypes that make up a specific meter
    int InstMeterCacheLastUsed(0);   // the last item in the instant meter cache used

    // INTERFACE BLOCK SPECIFICATIONS:
    // na

    // MODULE VARIABLE DECLARATIONS:

    int CurrentReportNumber(0);
    int NumVariablesForOutput(0);
    int MaxVariablesForOutput(0);
    int NumOfRVariable_Setup(0);
    int NumTotalRVariable(0);
    int NumOfRVariable_Sum(0);
    int NumOfRVariable_Meter(0);
    int NumOfRVariable(0);
    int MaxRVariable(0);
    int NumOfIVariable_Setup(0);
    int NumTotalIVariable(0);
    int NumOfIVariable_Sum(0);
    int NumOfIVariable(0);
    int MaxIVariable(0);
    bool OutputInitialized(false);
    int ProduceReportVDD(ReportVDD_No);
    int NumHoursInDay(24);
    int NumHoursInMonth(0);
    int NumHoursInSim(0);
    Array1D_int ReportList;
    int NumReportList(0);
    int NumExtraVars(0);

    int NumOfReqVariables(0); // Current number of Requested Report Variables

    int NumVarMeterArrays(0); // Current number of Arrays pointing to meters

    int NumEnergyMeters(0);     // Current number of Energy Meters
    Array1D<Real64> MeterValue; // This holds the current timestep value for each meter.

    int TimeStepStampReportNbr;             // TimeStep and Hourly Report number
    std::string TimeStepStampReportChr;     // TimeStep and Hourly Report number (character -- for printing)
    bool TrackingHourlyVariables(false);    // Requested Hourly Report Variables
    int DailyStampReportNbr;                // Daily Report number
    std::string DailyStampReportChr;        // Daily Report number (character -- for printing)
    bool TrackingDailyVariables(false);     // Requested Daily Report Variables
    int MonthlyStampReportNbr;              // Monthly Report number
    std::string MonthlyStampReportChr;      // Monthly Report number (character -- for printing)
    bool TrackingMonthlyVariables(false);   // Requested Monthly Report Variables
    int YearlyStampReportNbr;               // Yearly Report number
    std::string YearlyStampReportChr;       // Yearly Report number (character -- for printing)
    bool TrackingYearlyVariables(false);    // Requested Yearly Report Variables
    int RunPeriodStampReportNbr;            // RunPeriod Report number
    std::string RunPeriodStampReportChr;    // RunPeriod Report number (character -- for printing)
    bool TrackingRunPeriodVariables(false); // Requested RunPeriod Report Variables
    Real64 TimeStepZoneSec;                 // Seconds from NumTimeStepInHour
    bool ErrorsLogged(false);
    bool ProduceVariableDictionary(false);

    int MaxNumSubcategories(1);
    bool isFinalYear(false);

    bool GetOutputInputFlag(true);

    ReportingFrequency minimumReportFrequency(ReportingFrequency::EachCall);
    std::vector<APIOutputVariableRequest> apiVarRequests;

    namespace {
        // These were static variables within different functions. They were pulled out into the namespace
        // to facilitate easier unit testing of those functions.
        // These are purposefully not in the header file as an extern variable. No one outside of OutputProcessor should
        // use these. They are cleared by clear_state() for use by unit tests, but normal simulations should be unaffected.
        // This is purposefully in an anonymous namespace so nothing outside this implementation file can use it.
        int ReportNumberCounter(0);        // The report number is used in output reports as a key.
        int LHourP(-1);                    // Helps set hours for timestamp output
        Real64 LStartMin(-1.0);            // Helps set minutes for timestamp output
        Real64 LEndMin(-1.0);              // Helps set minutes for timestamp output
        bool GetMeterIndexFirstCall(true); // trigger setup in GetMeterIndex
        bool InitFlag(true);
        Array1D_int keyVarIndexes; // Array index for specific key name
        int curKeyVarIndexLimit;   // current limit for keyVarIndexes
        Array1D_string varNames;  // stored variable names
        Array1D_int ivarNames;    // pointers for sorted information
        int numVarNames;          // number of variable names
    } // namespace

    // All routines should be listed here whether private or not
    // PUBLIC  ReallocateTVar
    // PUBLIC  SetReportNow

    // Object Data

    // Pointers to the actual TimeStep variables
    std::map<TimeStepType, TimeSteps> TimeValue;
    Array1D<RealVariableType> RVariableTypes;         // Variable Types structure (use NumOfRVariables to traverse)
    Array1D<IntegerVariableType> IVariableTypes;      // Variable Types structure (use NumOfIVariables to traverse)
    Array1D<VariableTypeForDDOutput> DDVariableTypes; // Variable Types structure (use NumVariablesForOutput to traverse)
    Array1D<ReqReportVariables> ReqRepVars;
    Array1D<MeterArrayType> VarMeterArrays;
    Array1D<MeterType> EnergyMeters;
    Array1D<EndUseCategoryType> EndUseCategory;
    std::unordered_map<std::string, std::string> UniqueMeterNames;

    // Routines tagged on the end of this module:
    //  AddToOutputVariableList
    //  AssignReportNumber
    //  GenOutputVariablesAuditReport
    //  GetCurrentMeterValue
    //  GetInstantMeterValue
    //  GetInternalVariableValue
    //  GetInternalVariableValueExternalInterface
    //  GetMeteredVariables
    //  GetMeterIndex
    //  GetMeterResourceType
    //  GetNumMeteredVariables
    //  GetVariableKeyCountandType
    //  GetVariableKeys
    //  InitPollutionMeterReporting
    //  ProduceRDDMDD
    //  ReportingThisVariable
    //  SetInitialMeterReportingAndOutputNames
    //  SetupOutputVariable
    //  UpdateDataandReport
    //  UpdateMeterReporting

    // Functions

    // Clears the global data in OutputProcessor.
    // Needed for unit tests, should not be normally called.
    void clear_state()
    {
        InstMeterCacheSize = 1000;
        InstMeterCacheSizeInc = 1000;
        InstMeterCache.deallocate();
        InstMeterCacheLastUsed = 0;
        CurrentReportNumber = 0;
        NumVariablesForOutput = 0;
        MaxVariablesForOutput = 0;
        NumOfRVariable_Setup = 0;
        NumTotalRVariable = 0;
        NumOfRVariable_Sum = 0;
        NumOfRVariable_Meter = 0;
        NumOfRVariable = 0;
        MaxRVariable = 0;
        NumOfIVariable_Setup = 0;
        NumTotalIVariable = 0;
        NumOfIVariable_Sum = 0;
        NumOfIVariable = 0;
        MaxIVariable = 0;
        OutputInitialized = false;
        GetOutputInputFlag = true;
        ProduceReportVDD = ReportVDD_No;
        NumHoursInDay = 24;
        NumHoursInMonth = 0;
        NumHoursInSim = 0;
        ReportList.deallocate();
        NumReportList = 0;
        NumExtraVars = 0;
        NumOfReqVariables = 0;
        NumVarMeterArrays = 0;
        NumEnergyMeters = 0;
        MeterValue.deallocate();
        TimeStepStampReportNbr = 0;
        TimeStepStampReportChr = "";
        TrackingHourlyVariables = false;
        DailyStampReportNbr = 0;
        DailyStampReportChr = "";
        TrackingDailyVariables = false;
        MonthlyStampReportNbr = 0;
        MonthlyStampReportChr = "";
        TrackingMonthlyVariables = false;
        YearlyStampReportNbr = 0;
        YearlyStampReportChr = "";
        TrackingYearlyVariables = false;
        RunPeriodStampReportNbr = 0;
        RunPeriodStampReportChr = "";
        TrackingRunPeriodVariables = false;
        TimeStepZoneSec = 0;
        ErrorsLogged = false;
        ProduceVariableDictionary = false;
        MaxNumSubcategories = 1;
        ReportNumberCounter = 0;
        LHourP = -1;
        LStartMin = -1.0;
        LEndMin = -1.0;
        GetMeterIndexFirstCall = true;
        InitFlag = true;
        TimeValue.clear();
        RVariableTypes.deallocate();
        IVariableTypes.deallocate();
        DDVariableTypes.deallocate();
        ReqRepVars.deallocate();
        VarMeterArrays.deallocate();
        EnergyMeters.deallocate();
        EndUseCategory.deallocate();
        UniqueMeterNames.clear();
        apiVarRequests.clear();
        keyVarIndexes.clear(); // Array index for specific key name
        curKeyVarIndexLimit = 0;   // current limit for keyVarIndexes
        varNames.clear();  // stored variable names
        ivarNames.clear();    // pointers for sorted information
        numVarNames = 0;          // number of variable names
    }

    void InitializeOutput(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda K. Lawrie
        //       DATE WRITTEN   December 1998
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine initializes the OutputProcessor data structures.

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

        RVariableTypes.allocate(RVarAllocInc);
        MaxRVariable = RVarAllocInc;

        IVariableTypes.allocate(IVarAllocInc);
        MaxIVariable = IVarAllocInc;

        // First index is the frequency designation (-1 = each call, etc)
        // Second index is the variable type (1=Average, 2=Sum)
        // Note, Meters always report like Average (with min/max, etc) for hourly and above
        // FreqNotice( 1, -1 ) = " !Each Call";
        // FreqNotice( 1, 0 ) = " !TimeStep";
        // FreqNotice( 1, 1 ) = " !Hourly";
        // FreqNotice( 1, 2 ) = " !Daily [Value,Min,Hour,Minute,Max,Hour,Minute]";
        // FreqNotice( 1, 3 ) = " !Monthly [Value,Min,Day,Hour,Minute,Max,Day,Hour,Minute]";
        // FreqNotice( 1, 4 ) = " !RunPeriod [Value,Min,Month,Day,Hour,Minute,Max,Month,Day,Hour,Minute]";
        // FreqNotice( 2, -1 ) = " !Each Call";
        // FreqNotice( 2, 0 ) = " !TimeStep";
        // FreqNotice( 2, 1 ) = " !Hourly";
        // FreqNotice( 2, 2 ) = " !Daily [Value,Min,Hour,Minute,Max,Hour,Minute]";
        // FreqNotice( 2, 3 ) = " !Monthly [Value,Min,Day,Hour,Minute,Max,Day,Hour,Minute]";
        // FreqNotice( 2, 4 ) = " !RunPeriod [Value,Min,Month,Day,Hour,Minute,Max,Month,Day,Hour,Minute]";

        ReportList.allocate(500);
        NumReportList = 500;
        ReportList = 0;
        NumExtraVars = 0;

        // Initialize end use category names - the indices must match up with endUseNames in OutputReportTabular
        EndUseCategory.allocate(NumEndUses);
        EndUseCategory(endUseHeating).Name = "Heating";
        EndUseCategory(endUseCooling).Name = "Cooling";
        EndUseCategory(endUseInteriorLights).Name = "InteriorLights";
        EndUseCategory(endUseExteriorLights).Name = "ExteriorLights";
        EndUseCategory(endUseInteriorEquipment).Name = "InteriorEquipment";
        EndUseCategory(endUseExteriorEquipment).Name = "ExteriorEquipment";
        EndUseCategory(endUseFans).Name = "Fans";
        EndUseCategory(endUsePumps).Name = "Pumps";
        EndUseCategory(endUseHeatRejection).Name = "HeatRejection";
        EndUseCategory(endUseHumidification).Name = "Humidifier";
        EndUseCategory(endUseHeatRecovery).Name = "HeatRecovery";
        EndUseCategory(endUseWaterSystem).Name = "WaterSystems";
        EndUseCategory(endUseRefrigeration).Name = "Refrigeration";
        EndUseCategory(endUseCogeneration).Name = "Cogeneration";

        // Initialize display names for output table - this could go away if end use key names are changed to match
        EndUseCategory(endUseHeating).DisplayName = "Heating";
        EndUseCategory(endUseCooling).DisplayName = "Cooling";
        EndUseCategory(endUseInteriorLights).DisplayName = "Interior Lighting";
        EndUseCategory(endUseExteriorLights).DisplayName = "Exterior Lighting";
        EndUseCategory(endUseInteriorEquipment).DisplayName = "Interior Equipment";
        EndUseCategory(endUseExteriorEquipment).DisplayName = "Exterior Equipment";
        EndUseCategory(endUseFans).DisplayName = "Fans";
        EndUseCategory(endUsePumps).DisplayName = "Pumps";
        EndUseCategory(endUseHeatRejection).DisplayName = "Heat Rejection";
        EndUseCategory(endUseHumidification).DisplayName = "Humidification";
        EndUseCategory(endUseHeatRecovery).DisplayName = "Heat Recovery";
        EndUseCategory(endUseWaterSystem).DisplayName = "Water Systems";
        EndUseCategory(endUseRefrigeration).DisplayName = "Refrigeration";
        EndUseCategory(endUseCogeneration).DisplayName = "Generators";

        OutputInitialized = true;

        TimeStepZoneSec = double(MinutesPerTimeStep) * 60.0;

        InitializeMeters(state);
    }

    void SetupTimePointers(std::string const &TimeStepTypeKey, // Which timestep is being set up, 'Zone'=1, 'HVAC'=2
                           Real64 &TimeStep                    // The timestep variable.  Used to get the address
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda K. Lawrie
        //       DATE WRITTEN   December 1998
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine sets up the derived type for the output processor that
        // contains pointers to the TimeStep values used in the simulation.

        // METHODOLOGY EMPLOYED:
        // Indicate that the TimeStep passed in is a target for the pointer
        // attributes in the derived types.

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        // ValidateTimeStepType will throw a Fatal if not valid
        TimeStepType timeStepType = ValidateTimeStepType(TimeStepTypeKey, "SetupTimePointers");

        TimeSteps tPtr;
        tPtr.TimeStep = &TimeStep;
        if (!TimeValue.insert(std::make_pair(timeStepType, tPtr)).second) {
            // The element was already present... shouldn't happen
            ShowFatalError("SetupTimePointers was already called for " + TimeStepTypeKey);
        }
    }

    void CheckReportVariable(EnergyPlusData &state, std::string const &KeyedValue, std::string const &VarName)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda K. Lawrie
        //       DATE WRITTEN   December 1998
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine will get the report variable information from input and
        // determine if this variable (KeyedValue and VariableName) should be reported
        // and, if so, what frequency to report.

        // This routine is called when SetupOutputVariable is called with no "optional"
        // Reporting Frequency.  It is expected that SetupOutputVariable would only be
        // called once for each keyed variable to be triggered for output (from the input
        // requests).  The optional report frequency would only be used for debugging
        // purposes.  Therefore, this routine will collect all occasions where this
        // passed variablename would be reported from the requested input.  It builds
        // a list of these requests (ReportList) so that the calling routine can propagate
        // the requests into the correct data structure.

        // METHODOLOGY EMPLOYED:
        // This instance being requested will always have a key associated with it.  Matching
        // instances (from input) may or may not have keys, but only one instance of a reporting
        // frequency per variable is allowed.  ReportList will be populated with ReqRepVars indices
        // of those extra things from input that satisfy this condition.

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int Item;
        int Loop;
        int Pos;
        int MinLook;
        int MaxLook;

        // Make sure that input has been read
        GetReportVariableInput(state);

        if (NumOfReqVariables > 0) {
            // Do a quick check
            Item = UtilityRoutines::FindItem(VarName, ReqRepVars, &ReqReportVariables::VarName);

            NumExtraVars = 0;
            ReportList = 0;
            MinLook = 999999999;
            MaxLook = -999999999;

            if (Item != 0) {
                Loop = Item;
                Pos = Item;
                MinLook = min(MinLook, Pos);
                MaxLook = max(MaxLook, Pos);
                while (Loop <= NumOfReqVariables && Pos != 0) {
                    //  Mark all with blank keys as used
                    if (ReqRepVars(Loop).Key.empty()) {
                        ReqRepVars(Loop).Used = true;
                    }
                    if (Loop < NumOfReqVariables) {
                        Pos = UtilityRoutines::FindItem(VarName, ReqRepVars({Loop + 1, NumOfReqVariables}), &ReqReportVariables::VarName);
                        if (Pos != 0) {
                            MinLook = min(MinLook, Loop + Pos);
                            MaxLook = max(MaxLook, Loop + Pos);
                        }
                    } else {
                        Pos = 1;
                    }
                    Loop += Pos;
                }
                BuildKeyVarList(KeyedValue, VarName, MinLook, MaxLook);
                AddBlankKeys(VarName, MinLook, MaxLook);
            }
        }
    }

    void BuildKeyVarList(std::string const &KeyedValue,   // Associated Key for this variable
                         std::string const &VariableName, // String Name of variable
                         int const MinIndx,               // Min number (from previous routine) for this variable
                         int const MaxIndx                // Max number (from previous routine) for this variable
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda K. Lawrie
        //       DATE WRITTEN   March 1999
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine builds an initial list (from ReqRepVars) of
        // pointers to that data structure for this KeyedValue and VariableName.

        // METHODOLOGY EMPLOYED:
        // Go through the ReqRepVars list and add those
        // that match (and dont duplicate ones already in the list).

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int Loop;
        int Loop1;
        bool Dup;

        for (Loop = MinIndx; Loop <= MaxIndx; ++Loop) {
            if (ReqRepVars(Loop).Key.empty()) continue;
            if (!UtilityRoutines::SameString(ReqRepVars(Loop).VarName, VariableName)) continue;
            if (!(UtilityRoutines::SameString(ReqRepVars(Loop).Key, KeyedValue) || RE2::FullMatch(KeyedValue, "(?i)" + ReqRepVars(Loop).Key)))
                continue;

            //   A match.  Make sure doesn't duplicate

            ReqRepVars(Loop).Used = true;
            Dup = false;
            for (Loop1 = 1; Loop1 <= NumExtraVars; ++Loop1) {
                if (ReqRepVars(ReportList(Loop1)).frequency == ReqRepVars(Loop).frequency) {
                    Dup = true;
                } else {
                    continue;
                }
                //  So Same Report Frequency
                if (ReqRepVars(ReportList(Loop1)).SchedPtr != ReqRepVars(Loop).SchedPtr) Dup = false;
            }

            if (!Dup) {
                ++NumExtraVars;
                if (NumExtraVars == NumReportList) {
                    ReportList.redimension(NumReportList += 100, 0);
                }
                ReportList(NumExtraVars) = Loop;
            }
        }
    }

    void AddBlankKeys(std::string const &VariableName, // String Name of variable
                      int const MinIndx,               // Min number (from previous routine) for this variable
                      int const MaxIndx                // Max number (from previous routine) for this variable
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda K. Lawrie
        //       DATE WRITTEN   March 1999
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine adds to the ReportList any report variables that have
        // been requested for all keys of that report variable (if it doesnt duplicate
        // a frequency already on the list).

        // METHODOLOGY EMPLOYED:
        // Go through the ReqRepVars list and add those
        // that match (and dont duplicate ones already in the list).

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int Loop;
        int Loop1;
        bool Dup;

        for (Loop = MinIndx; Loop <= MaxIndx; ++Loop) {
            if (!ReqRepVars(Loop).Key.empty()) continue;
            if (!UtilityRoutines::SameString(ReqRepVars(Loop).VarName, VariableName)) continue;

            //   A match.  Make sure doesnt duplicate

            Dup = false;
            for (Loop1 = 1; Loop1 <= NumExtraVars; ++Loop1) {
                // IF (ReqRepVars(ReportList(Loop1))%ReportFreq == ReqRepVars(Loop)%ReportFreq) Dup=.TRUE.
                if (ReqRepVars(ReportList(Loop1)).frequency == ReqRepVars(Loop).frequency) {
                    Dup = true;
                } else {
                    continue;
                }
                //  So Same Report Frequency
                if (ReqRepVars(ReportList(Loop1)).SchedPtr != ReqRepVars(Loop).SchedPtr) Dup = false;
            }

            if (!Dup) {
                ++NumExtraVars;
                if (NumExtraVars == NumReportList) {
                    ReportList.redimension(NumReportList += 100, 0);
                }
                ReportList(NumExtraVars) = Loop;
            }
        }
    }

    static std::string frequencyNotice(StoreType EP_UNUSED(storeType), ReportingFrequency reportingInterval)
    {
        switch (reportingInterval) {
        case ReportingFrequency::EachCall:
            return " !Each Call";
            break;
        case ReportingFrequency::TimeStep:
            return " !TimeStep";
            break;
        case ReportingFrequency::Hourly:
            return " !Hourly";
            break;
        case ReportingFrequency::Daily:
            return " !Daily [Value,Min,Hour,Minute,Max,Hour,Minute]";
            break;
        case ReportingFrequency::Monthly:
            return " !Monthly [Value,Min,Day,Hour,Minute,Max,Day,Hour,Minute]";
            break;
        case ReportingFrequency::Yearly:
            return " !Annual [Value,Min,Month,Day,Hour,Minute,Max,Month,Day,Hour,Minute]";
            break;
        case ReportingFrequency::Simulation:
            return " !RunPeriod [Value,Min,Month,Day,Hour,Minute,Max,Month,Day,Hour,Minute]";
            break;
        }
        return " !Hourly";
    }

    std::string reportingFrequency(ReportingFrequency reportingInterval)
    {
        switch (reportingInterval) {
        case ReportingFrequency::EachCall:
            return "Each Call";
            break;
        case ReportingFrequency::TimeStep:
            return "TimeStep";
            break;
        case ReportingFrequency::Hourly:
            return "Hourly";
            break;
        case ReportingFrequency::Daily:
            return "Daily";
            break;
        case ReportingFrequency::Monthly:
            return "Monthly";
            break;
        case ReportingFrequency::Yearly:
            return "Annual";
            break;
        case ReportingFrequency::Simulation:
            return "RunPeriod";
            break;
        }
        return "Hourly";
    }

    ReportingFrequency determineFrequency(const std::string &FreqString)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda K. Lawrie
        //       DATE WRITTEN   December 1998
        //       MODIFIED       December 2017; Jason DeGraw
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine looks at the passed in report frequency string and
        // determines the reporting frequency.

        // METHODOLOGY EMPLOYED:
        // na

        // REFERENCES:
        //       \field Reporting Frequency
        //       \type choice
        //       \key Detailed
        //       \note Detailed lists every instance (i.e. HVAC variable timesteps)
        //       \key Timestep
        //       \note Timestep refers to the zone Timestep/Number of Timesteps in hour value
        //       \note RunPeriod, Environment, and Annual are the same
        //       \key Hourly
        //       \key Daily
        //       \key Monthly
        //       \key RunPeriod
        //       \key Environment
        //       \key Annual
        //       \default Hourly
        //       \note RunPeriod and Environment are synonymous

        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:

        // SUBROUTINE PARAMETER DEFINITIONS:
        static std::vector<std::string> const PossibleFreq({"deta", "time", "hour", "dail", "mont", "runp", "envi", "annu"});
        //=(/'detail','Timestep','Hourly','Daily','Monthly','RunPeriod','Environment','Annual'/)
        static std::vector<std::string> const ExactFreqString(
            {"Detailed", "Timestep", "Hourly", "Daily", "Monthly", "RunPeriod", "Environment", "Annual"});
        // Vector of the result, was { -1, 0, 1, 2, 3, 4, 4, 4 } before the addition of Yearly;
        static std::vector<ReportingFrequency> const FreqValues({ReportingFrequency::EachCall,
                                                                 ReportingFrequency::TimeStep,
                                                                 ReportingFrequency::Hourly,
                                                                 ReportingFrequency::Daily,
                                                                 ReportingFrequency::Monthly,
                                                                 ReportingFrequency::Simulation,
                                                                 ReportingFrequency::Simulation,
                                                                 ReportingFrequency::Yearly});
        // note: runperiod and environment are synonomous

        // INTERFACE BLOCK SPECIFICATIONS:
        // na

        // DERIVED TYPE DEFINITIONS:
        // na

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:

        ReportingFrequency ReportFreq(ReportingFrequency::Hourly); // Default
        std::string::size_type const LenString = min(len(FreqString), static_cast<std::string::size_type>(4u));

        if (LenString < 4u) {
            return ReportFreq;
        }

        std::string const FreqStringTrim(FreqString.substr(0, LenString));
        for (unsigned Loop = 0; Loop < FreqValues.size(); ++Loop) {
            if (UtilityRoutines::SameString(FreqStringTrim, PossibleFreq[Loop])) {
                if (!UtilityRoutines::SameString(FreqString, ExactFreqString[Loop])) {
                    ShowWarningError("DetermineFrequency: Entered frequency=\"" + FreqString + "\" is not an exact match to key strings.");
                    ShowContinueError("Frequency=" + ExactFreqString[Loop] + " will be used.");
                }
                ReportFreq = std::max(FreqValues[Loop], minimumReportFrequency);
                break;
            }
        }
        return ReportFreq;
    }

    void GetReportVariableInput(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda K. Lawrie
        //       DATE WRITTEN   December 1998
        //       MODIFIED       December 2017; Jason DeGraw
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine gets the requested report variables from
        // the input file.
        // Report Variable,
        //        \memo each Report Variable command picks variables to be put onto the standard output file (.eso)
        //        \memo some variables may not be reported for every simulation
        //   A1 , \field Key_Value
        //        \note use '*' (without quotes) to apply this variable to all keys
        //   A2 , \field Variable_Name
        //   A3 , \field Reporting_Frequency
        //        \type choice
        //        \key detailed
        //        \key timestep
        //        \key hourly
        //        \key daily
        //        \key monthly
        //        \key runperiod
        //   A4 ; \field Schedule_Name
        //        \type object-list
        //        \object-list ScheduleNames

        // Using/Aliasing
        using DataSystemVariables::MinReportFrequency;
        using ScheduleManager::GetScheduleIndex;

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int Loop;
        int NumAlpha;
        int NumNumbers;
        int IOStat;
        static bool ErrorsFound(false); // If errors detected in input
        std::string cCurrentModuleObject;
        Array1D_string cAlphaArgs(4);
        Array1D_string cAlphaFieldNames(4);
        Array1D_bool lAlphaFieldBlanks(4);
        Array1D<Real64> rNumericArgs(1);
        Array1D_string cNumericFieldNames(1);
        Array1D_bool lNumericFieldBlanks(1);


        // Bail out if the input has already been read in
        if (!GetOutputInputFlag) {
            return;
        }
        GetOutputInputFlag = false;

        // First check environment variable to see of possible override for minimum reporting frequency
        if (MinReportFrequency != "") {
            // Formats
            static constexpr auto Format_800("! <Minimum Reporting Frequency (overriding input value)>, Value, Input Value\n");
            static constexpr auto Format_801(" Minimum Reporting Frequency, {},{}\n");
            minimumReportFrequency = determineFrequency(MinReportFrequency);
            print(state.files.eio, Format_800);
            print(state.files.eio, Format_801, frequencyNotice(StoreType::Averaged, minimumReportFrequency), MinReportFrequency);
        }

        cCurrentModuleObject = "Output:Variable";
        NumOfReqVariables = inputProcessor->getNumObjectsFound(cCurrentModuleObject);
        ReqRepVars.allocate(NumOfReqVariables);

        for (Loop = 1; Loop <= NumOfReqVariables; ++Loop) {

            inputProcessor->getObjectItem(state,
                                          cCurrentModuleObject,
                                          Loop,
                                          cAlphaArgs,
                                          NumAlpha,
                                          rNumericArgs,
                                          NumNumbers,
                                          IOStat,
                                          lNumericFieldBlanks,
                                          lAlphaFieldBlanks,
                                          cAlphaFieldNames,
                                          cNumericFieldNames);

            // Check for duplicates?

            ReqRepVars(Loop).Key = cAlphaArgs(1);
            if (ReqRepVars(Loop).Key == "*") {
                ReqRepVars(Loop).Key = BlankString;
            }

            std::string::size_type const lbpos = index(cAlphaArgs(2), '['); // Remove Units designation if user put it in
            if (lbpos != std::string::npos) {
                cAlphaArgs(2).erase(lbpos);
            }
            ReqRepVars(Loop).VarName = cAlphaArgs(2);

            ReqRepVars(Loop).frequency = determineFrequency(cAlphaArgs(3));

            // Schedule information
            ReqRepVars(Loop).SchedName = cAlphaArgs(4);
            if (not_blank(ReqRepVars(Loop).SchedName)) {
                ReqRepVars(Loop).SchedPtr = GetScheduleIndex(state, ReqRepVars(Loop).SchedName);
                if (ReqRepVars(Loop).SchedPtr == 0) {
                    ShowSevereError("GetReportVariableInput: " + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + ':' + ReqRepVars(Loop).VarName +
                                    "\" invalid " + cAlphaFieldNames(4) + "=\"" + ReqRepVars(Loop).SchedName + "\" - not found.");
                    ErrorsFound = true;
                }
            } else {
                ReqRepVars(Loop).SchedPtr = 0;
            }

            ReqRepVars(Loop).Used = false;
        }

        if (ErrorsFound) {
            ShowFatalError("GetReportVariableInput:" + cCurrentModuleObject + ": errors in input.");
        }
    }

    void ProduceMinMaxString(std::string &String,                // Current value
                             int const DateValue,                // Date of min/max
                             ReportingFrequency const ReportFreq // Reporting Frequency
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda K. Lawrie
        //       DATE WRITTEN   December 1998
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine produces the appropriate min/max string depending
        // on the reporting frequency.

        // METHODOLOGY EMPLOYED:
        // Prior to calling this routine, the basic value string will be
        // produced, but DecodeMonDayHrMin will not have been called.

        // REFERENCES:
        // na

        // Using/Aliasing
        using General::DecodeMonDayHrMin;

        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:

        // SUBROUTINE PARAMETER DEFINITIONS:
        static constexpr auto DayFormat("{},{:2},{:2}");
        static constexpr auto MonthFormat("{},{:2},{:2},{:2}");
        static constexpr auto EnvrnFormat("{},{:2},{:2},{:2},{:2}");

        // INTERFACE BLOCK SPECIFICATIONS:
        // na

        // DERIVED TYPE DEFINITIONS:
        // na

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int Mon;
        int Day;
        int Hour;
        int Minute;
        std::string StrOut;

        DecodeMonDayHrMin(DateValue, Mon, Day, Hour, Minute);

        switch (ReportFreq) {
        case ReportingFrequency::Daily:
            StrOut = format(DayFormat, strip(String), Hour, Minute);
            break;
        case ReportingFrequency::Monthly:
            StrOut = format(MonthFormat, strip(String), Day, Hour, Minute);
            break;
        case ReportingFrequency::Yearly:
            StrOut = format(EnvrnFormat, strip(String), Mon, Day, Hour, Minute);
            break;
        case ReportingFrequency::Simulation:
            StrOut = format(EnvrnFormat, strip(String), Mon, Day, Hour, Minute);
            break;
        default: // Each, TimeStep, Hourly dont have this
            StrOut = BlankString;
            break;
        }

        String = StrOut;
    }

    void ProduceMinMaxStringWStartMinute(std::string &String,                // Current value
                                         int const DateValue,                // Date of min/max
                                         ReportingFrequency const ReportFreq // Reporting Frequency
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda K. Lawrie
        //       DATE WRITTEN   January 2001
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine produces the appropriate min/max string depending
        // on the reporting frequency.  Used in Meter reporting.

        // METHODOLOGY EMPLOYED:
        // Prior to calling this routine, the basic value string will be
        // produced, but DecodeMonDayHrMin will not have been called.  Uses the MinutesPerTimeStep
        // value to set the StartMinute.

        // REFERENCES:
        // na

        // Using/Aliasing
        using General::DecodeMonDayHrMin;

        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:

        // SUBROUTINE PARAMETER DEFINITIONS:
        static constexpr auto HrFormat("{},{:02}:{:02}");
        static constexpr auto DayFormat("{},{:2},{:02}:{:02}");
        static constexpr auto MonthFormat("{},{:2},{:2},{:02}:{:02}");
        static constexpr auto EnvrnFormat("{},{:2},{:2},{:2},{:02}:{:02}");

        // INTERFACE BLOCK SPECIFICATIONS:
        // na

        // DERIVED TYPE DEFINITIONS:
        // na

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int Mon;
        int Day;
        int Hour;
        int Minute;
        int StartMinute;
        std::string StrOut;

        DecodeMonDayHrMin(DateValue, Mon, Day, Hour, Minute);

        switch (ReportFreq) {
        case ReportingFrequency::Hourly: // Hourly -- used in meters
            StartMinute = Minute - MinutesPerTimeStep + 1;
            StrOut = format(HrFormat, strip(String), StartMinute, Minute);
            break;

        case ReportingFrequency::Daily: // Daily
            StartMinute = Minute - MinutesPerTimeStep + 1;
            StrOut = format(DayFormat, strip(String), Hour, StartMinute, Minute);
            break;

        case ReportingFrequency::Monthly: // Monthly
            StartMinute = Minute - MinutesPerTimeStep + 1;
            StrOut = format(MonthFormat, strip(String), Day, Hour, StartMinute, Minute);
            break;

        case ReportingFrequency::Yearly: // Yearly
            StartMinute = Minute - MinutesPerTimeStep + 1;
            StrOut = format(EnvrnFormat, strip(String), Mon, Day, Hour, StartMinute, Minute);
            break;

        case ReportingFrequency::Simulation: // Environment
            StartMinute = Minute - MinutesPerTimeStep + 1;
            StrOut = format(EnvrnFormat, strip(String), Mon, Day, Hour, StartMinute, Minute);
            break;

        default: // Each, TimeStep, Hourly dont have this
            StrOut = BlankString;
            break;
        }

        String = StrOut;
    }

    TimeStepType ValidateTimeStepType(std::string const &TimeStepTypeKey, // Index type (Zone, HVAC) for variables
                                      std::string const &CalledFrom       // Routine called from (for error messages)
    )
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         Linda K. Lawrie
        //       DATE WRITTEN   December 1998
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS FUNCTION:
        // This function validates the requested "index" type and returns
        // the proper value for use inside the OutputProcessor.

        // METHODOLOGY EMPLOYED:
        // Look it up in a list of valid index types.

        // FUNCTION LOCAL VARIABLE DECLARATIONS:

        // TODO: , "HEATBALANCE", "HEAT BALANCE" are used nowhere aside from tests. Should we remove them?
        static std::vector<std::string> zoneIndexes({"ZONE", "HEATBALANCE", "HEAT BALANCE"});
        static std::vector<std::string> systemIndexes({"HVAC", "SYSTEM", "PLANT"});
        std::string uppercase(UtilityRoutines::MakeUPPERCase(TimeStepTypeKey));

        if (std::find(zoneIndexes.begin(), zoneIndexes.end(), uppercase) != zoneIndexes.end()) {
            return TimeStepType::TimeStepZone;
        }

        if (std::find(systemIndexes.begin(), systemIndexes.end(), uppercase) != systemIndexes.end()) {
            return TimeStepType::TimeStepSystem;
        }

        //  The following should never happen to a user!!!!
        ShowSevereError("OutputProcessor/ValidateTimeStepType: Invalid Index Key passed to ValidateTimeStepType=" + TimeStepTypeKey);
        ShowContinueError("..Should be \"ZONE\", \"SYSTEM\", \"HVAC\", or \"PLANT\"... was called from:" + CalledFrom);
        ShowFatalError("Preceding condition causes termination.");

        return TimeStepType::TimeStepZone;
    }

    std::string StandardTimeStepTypeKey(TimeStepType const timeStepType)
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         Linda K. Lawrie
        //       DATE WRITTEN   December 1998
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS FUNCTION:
        // This function gives the standard string for the index type
        // given.

        // METHODOLOGY EMPLOYED:
        // Look it up in a list of valid index types.

        // REFERENCES:
        // na

        // USE STATEMENTS:
        // na

        // Return value
        std::string StandardTimeStepTypeKey;

        // Locals
        // FUNCTION ARGUMENT DEFINITIONS:

        // FUNCTION PARAMETER DEFINITIONS:
        // na

        // INTERFACE BLOCK SPECIFICATIONS:
        // na

        // DERIVED TYPE DEFINITIONS:
        // na

        // FUNCTION LOCAL VARIABLE DECLARATIONS:
        // na

        if (timeStepType == TimeStepType::TimeStepZone) {
            StandardTimeStepTypeKey = "Zone";
        } else if (timeStepType == TimeStepType::TimeStepSystem) {
            StandardTimeStepTypeKey = "HVAC";
        } else {
            StandardTimeStepTypeKey = "UNKW";
        }

        return StandardTimeStepTypeKey;
    }

    StoreType validateVariableType(std::string const &VariableTypeKey)
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         Linda K. Lawrie
        //       DATE WRITTEN   December 1998
        //       MODIFIED       December 2017; Jason DeGraw
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS FUNCTION:
        // This function validates the VariableTypeKey passed to the SetupVariable
        // routine and assigns it the value used in the OutputProcessor.

        // METHODOLOGY EMPLOYED:
        // Look it up in a list of valid variable types.

        // Return value
        // na

        // FUNCTION LOCAL VARIABLE DECLARATIONS:
        static Array1D_string StateVariables(3);
        static std::vector<std::string> stateVariables({"STATE", "AVERAGE", "AVERAGED"});
        static Array1D_string NonStateVariables(4);
        static std::vector<std::string> nonStateVariables({"NON STATE", "NONSTATE", "SUM", "SUMMED"});
        std::string uppercase(UtilityRoutines::MakeUPPERCase(VariableTypeKey));

        auto iter = std::find(stateVariables.begin(), stateVariables.end(), uppercase);
        if (iter != stateVariables.end()) {
            return StoreType::Averaged;
        }

        iter = std::find(nonStateVariables.begin(), nonStateVariables.end(), uppercase);
        if (iter != nonStateVariables.end()) {
            return StoreType::Summed;
        }

        ShowSevereError("Invalid variable type requested=" + VariableTypeKey);

        return StoreType::Averaged;
    }

    std::string standardVariableTypeKey(StoreType const VariableType)
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         Linda K. Lawrie
        //       DATE WRITTEN   July 1999
        //       MODIFIED       December 2017; Jason DeGraw
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS FUNCTION:
        // This function gives the standard string for the variable type
        // given.

        // METHODOLOGY EMPLOYED:
        // From variable type value, produce proper string.

        // REFERENCES:
        // na

        // USE STATEMENTS:
        // na

        // Return value
        // na

        // Locals
        // FUNCTION ARGUMENT DEFINITIONS:

        // FUNCTION PARAMETER DEFINITIONS:
        // na

        // INTERFACE BLOCK SPECIFICATIONS:
        // na

        // DERIVED TYPE DEFINITIONS:
        // na

        // FUNCTION LOCAL VARIABLE DECLARATIONS:
        // na

        switch (VariableType) {
        case StoreType::Averaged:
            return "Average";
            break;
        case StoreType::Summed:
            return "Sum";
            break;
        }

        return "Unknown";
    }

    // *****************************************************************************
    // The following routines implement Energy Meters in EnergyPlus.
    // *****************************************************************************

    void InitializeMeters(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda Lawrie
        //       DATE WRITTEN   January 2001
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine creates the set of meters in EnergyPlus.  In this initial
        // implementation, it is a static set of meters.

        // METHODOLOGY EMPLOYED:
        // Allocate the static set.  Use "AddMeter" with appropriate arguments that will
        // allow expansion later.

        // REFERENCES:
        // na

        // USE STATEMENTS:
        // na

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

        state.files.mtd.ensure_open("InitializeMeters", state.files.outputControl.mtd);
    }

    void GetCustomMeterInput(EnergyPlusData &state, bool &ErrorsFound)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda Lawrie
        //       DATE WRITTEN   January 2006
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This routine will help implement "custom"/user defined meters.  However, it must be called after all
        // the other meters are set up and all report variables are established.

        // METHODOLOGY EMPLOYED:
        // na

        // REFERENCES:
        // Processes the objects:
        // Meter:Custom,
        //    \extensible:2 - repeat last two fields, remembering to remove ; from "inner" fields.
        //    \memo Used to allow users to combine specific variables and/or meters into
        //    \memo "custom" meter configurations.
        //    A1,  \field Name
        //         \required-field
        //         \reference CustomMeterNames
        //    A2,  \field Fuel Type
        //         \type choice
        //         \key Electricity
        //         \key NaturalGas
        //         \key PropaneGas
        //         \key FuelOilNo1
        //         \key FuelOilNo2
        //         \key Coal
        //         \key Diesel
        //         \key Gasoline
        //         \key Water
        //         \key Generic
        //         \key OtherFuel1
        //         \key OtherFuel2
        //    A3,  \field Key Name 1
        //         \required-field
        //         \begin-extensible
        //    A4,  \field Report Variable or Meter Name 1
        //         \required-field
        // <etc>
        // AND
        // Meter:CustomDecrement,
        //    \extensible:2 - repeat last two fields, remembering to remove ; from "inner" fields.
        //    \memo Used to allow users to combine specific variables and/or meters into
        //    \memo "custom" meter configurations.
        //    A1,  \field Name
        //         \required-field
        //         \reference CustomMeterNames
        //    A2,  \field Fuel Type
        //         \type choice
        //         \key Electricity
        //         \key NaturalGas
        //         \key PropaneGas
        //         \key FuelOilNo1
        //         \key FuelOilNo2
        //         \key Coal
        //         \key Diesel
        //         \key Gasoline
        //         \key Water
        //         \key Generic
        //         \key OtherFuel1
        //         \key OtherFuel2
        //    A3,  \field Source Meter Name
        //         \required-field
        //    A4,  \field Key Name 1
        //         \required-field
        //         \begin-extensible
        //    A5,  \field Report Variable or Meter Name 1
        //         \required-field
        // <etc>

        // Using/Aliasing
        using namespace DataIPShortCuts;

        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:

        // SUBROUTINE PARAMETER DEFINITIONS:
        // na

        // INTERFACE BLOCK SPECIFICATIONS:
        // na

        // DERIVED TYPE DEFINITIONS:
        // na

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int NumAlpha;
        int NumNumbers;
        int Loop;
        int IOStat;
        int NumCustomMeters;
        int NumCustomDecMeters;
        int fldIndex;
        bool KeyIsStar;
        Array1D_string NamesOfKeys;                                    // Specific key name
        Array1D_int IndexesForKeyVar;                                  // Array index
        OutputProcessor::Unit UnitsVar(OutputProcessor::Unit::None);   // Units enumeration
        OutputProcessor::Unit MeterUnits(OutputProcessor::Unit::None); // Units enumeration
        int KeyCount;
        int TypeVar;
        OutputProcessor::StoreType AvgSumVar;
        OutputProcessor::TimeStepType StepTypeVar;
        int iKey;
        int iKey1;
        bool MeterCreated;
        Array1D_int VarsOnCustomMeter;
        int MaxVarsOnCustomMeter;
        int NumVarsOnCustomMeter;
        Array1D_int VarsOnSourceMeter;
        int MaxVarsOnSourceMeter;
        int NumVarsOnSourceMeter;
        int iOnMeter;
        int WhichMeter;
        bool errFlag;
        bool BigErrorsFound;
        bool testa;
        bool testb;
        bool Tagged; // variable is appropriate to put on meter
        std::string::size_type lbrackPos;

        BigErrorsFound = false;

        cCurrentModuleObject = "Meter:Custom";
        NumCustomMeters = inputProcessor->getNumObjectsFound(cCurrentModuleObject);

        // make list of names for all Meter:Custom since they cannot refer to other Meter:Custom's
        std::unordered_set<std::string> namesOfMeterCustom;
        namesOfMeterCustom.reserve(NumCustomMeters);

        for (Loop = 1; Loop <= NumCustomMeters; ++Loop) {
            inputProcessor->getObjectItem(state,
                                          cCurrentModuleObject,
                                          Loop,
                                          cAlphaArgs,
                                          NumAlpha,
                                          rNumericArgs,
                                          NumNumbers,
                                          IOStat,
                                          lNumericFieldBlanks,
                                          lAlphaFieldBlanks,
                                          cAlphaFieldNames,
                                          cNumericFieldNames);
            namesOfMeterCustom.emplace(UtilityRoutines::MakeUPPERCase(cAlphaArgs(1)));
        }

        for (Loop = 1; Loop <= NumCustomMeters; ++Loop) {
            inputProcessor->getObjectItem(state,
                                          cCurrentModuleObject,
                                          Loop,
                                          cAlphaArgs,
                                          NumAlpha,
                                          rNumericArgs,
                                          NumNumbers,
                                          IOStat,
                                          lNumericFieldBlanks,
                                          lAlphaFieldBlanks,
                                          cAlphaFieldNames,
                                          cNumericFieldNames);
            lbrackPos = index(cAlphaArgs(1), '[');
            if (lbrackPos != std::string::npos) cAlphaArgs(1).erase(lbrackPos);
            MeterCreated = false;
            if (GlobalNames::VerifyUniqueInterObjectName(UniqueMeterNames, cAlphaArgs(1), cCurrentModuleObject, cAlphaFieldNames(1), ErrorsFound)) {
                continue;
            }
            if (allocated(VarsOnCustomMeter)) VarsOnCustomMeter.deallocate();
            VarsOnCustomMeter.allocate(1000);
            VarsOnCustomMeter = 0;
            MaxVarsOnCustomMeter = 1000;
            NumVarsOnCustomMeter = 0;
            // check if any fields reference another Meter:Custom
            int found = 0;
            for (fldIndex = 4; fldIndex <= NumAlpha; fldIndex += 2) {
                if (namesOfMeterCustom.find(UtilityRoutines::MakeUPPERCase(cAlphaArgs(fldIndex))) != namesOfMeterCustom.end()) {
                    found = fldIndex;
                    break;
                }
            }
            if (found != 0) {
                ShowWarningError(cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\", contains a reference to another " + cCurrentModuleObject +
                                 " in field: " + cAlphaFieldNames(found) + "=\"" + cAlphaArgs(found) + "\".");
                continue;
            }

            for (fldIndex = 3; fldIndex <= NumAlpha; fldIndex += 2) {
                if (cAlphaArgs(fldIndex) == "*" || lAlphaFieldBlanks(fldIndex)) {
                    KeyIsStar = true;
                    cAlphaArgs(fldIndex) = "*";
                } else {
                    KeyIsStar = false;
                }
                if (lAlphaFieldBlanks(fldIndex + 1)) {
                    ShowSevereError(cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\", blank " + cAlphaFieldNames(fldIndex + 1) + '.');
                    ShowContinueError("...cannot create custom meter.");
                    BigErrorsFound = true;
                    continue;
                }
                if (BigErrorsFound) continue;
                // Don't build/check things out if there were errors anywhere.  Use "GetVariableKeys" to map to actual variables...
                lbrackPos = index(cAlphaArgs(fldIndex + 1), '[');
                if (lbrackPos != std::string::npos) cAlphaArgs(fldIndex + 1).erase(lbrackPos);
                Tagged = false;
                GetVariableKeyCountandType(state, cAlphaArgs(fldIndex + 1), KeyCount, TypeVar, AvgSumVar, StepTypeVar, UnitsVar);
                if (TypeVar == VarType_NotFound) {
                    ShowWarningError(cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\", invalid " + cAlphaFieldNames(fldIndex + 1) + "=\"" +
                                     cAlphaArgs(fldIndex + 1) + "\".");
                    ShowContinueError("...will not be shown with the Meter results.");
                    continue;
                }
                if (!MeterCreated) {
                    MeterUnits = UnitsVar; // meter units are same as first variable on custom meter
                    AddMeter(cAlphaArgs(1), UnitsVar, BlankString, BlankString, BlankString, BlankString);
                    EnergyMeters(NumEnergyMeters).TypeOfMeter = MeterType_Custom;
                    // Can't use resource type in AddMeter cause it will confuse it with other meters.  So, now:
                    GetStandardMeterResourceType(EnergyMeters(NumEnergyMeters).ResourceType, UtilityRoutines::MakeUPPERCase(cAlphaArgs(2)), errFlag);
                    if (errFlag) {
                        ShowContinueError("..on " + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\".");
                        BigErrorsFound = true;
                    }
                    DetermineMeterIPUnits(EnergyMeters(NumEnergyMeters).RT_forIPUnits, EnergyMeters(NumEnergyMeters).ResourceType, UnitsVar, errFlag);
                    if (errFlag) {
                        ShowContinueError("..on " + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\".");
                        ShowContinueError("..requests for IP units from this meter will be ignored.");
                    }
                    //        EnergyMeters(NumEnergyMeters)%RT_forIPUnits=DetermineMeterIPUnits(EnergyMeters(NumEnergyMeters)%ResourceType,UnitsVar)
                    MeterCreated = true;
                }
                if (UnitsVar != MeterUnits) {
                    ShowWarningError(cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\", differing units in " + cAlphaFieldNames(fldIndex + 1) +
                                     "=\"" + cAlphaArgs(fldIndex + 1) + "\".");
                    ShowContinueError("...will not be shown with the Meter results; units for meter=" + unitEnumToString(MeterUnits) +
                                      ", units for this variable=" + unitEnumToString(UnitsVar) + '.');
                    continue;
                }
                if ((TypeVar == VarType_Real || TypeVar == VarType_Integer) && AvgSumVar == StoreType::Summed) {
                    Tagged = true;
                    NamesOfKeys.allocate(KeyCount);
                    IndexesForKeyVar.allocate(KeyCount);
                    GetVariableKeys(state, cAlphaArgs(fldIndex + 1), TypeVar, NamesOfKeys, IndexesForKeyVar);
                    iOnMeter = 0;
                    if (KeyIsStar) {
                        for (iKey = 1; iKey <= KeyCount; ++iKey) {
                            ++NumVarsOnCustomMeter;
                            if (NumVarsOnCustomMeter > MaxVarsOnCustomMeter) {
                                VarsOnCustomMeter.redimension(MaxVarsOnCustomMeter += 100, 0);
                            }
                            VarsOnCustomMeter(NumVarsOnCustomMeter) = IndexesForKeyVar(iKey);
                            iOnMeter = 1;
                        }
                        if (iOnMeter == 0) {
                            ShowSevereError(cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\", invalid (all keys) " +
                                            cAlphaFieldNames(fldIndex + 1) + "=\"" + cAlphaArgs(fldIndex + 1) + "\".");
                            ErrorsFound = true;
                        }
                    } else { // Key is not "*"
                        for (iKey = 1; iKey <= KeyCount; ++iKey) {
                            if (NamesOfKeys(iKey) != cAlphaArgs(fldIndex)) continue;
                            ++NumVarsOnCustomMeter;
                            if (NumVarsOnCustomMeter > MaxVarsOnCustomMeter) {
                                VarsOnCustomMeter.redimension(MaxVarsOnCustomMeter += 100, 0);
                            }
                            VarsOnCustomMeter(NumVarsOnCustomMeter) = IndexesForKeyVar(iKey);
                            iOnMeter = 1;
                        }
                        if (iOnMeter == 0) {
                            ShowSevereError(cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\", invalid " + cAlphaArgs(fldIndex) + ':' +
                                            cAlphaArgs(fldIndex + 1));
                            ErrorsFound = true;
                        }
                    }
                    NamesOfKeys.deallocate();
                    IndexesForKeyVar.deallocate();
                }
                if (TypeVar == VarType_Meter && AvgSumVar == StoreType::Summed) {
                    Tagged = true;
                    NamesOfKeys.allocate(KeyCount);
                    IndexesForKeyVar.allocate(KeyCount);
                    GetVariableKeys(state, cAlphaArgs(fldIndex + 1), TypeVar, NamesOfKeys, IndexesForKeyVar);
                    WhichMeter = IndexesForKeyVar(1);
                    NamesOfKeys.deallocate();
                    IndexesForKeyVar.deallocate();
                    // for meters there will only be one key...  but it has variables associated...
                    for (iOnMeter = 1; iOnMeter <= NumVarMeterArrays; ++iOnMeter) {
                        if (!any_eq(VarMeterArrays(iOnMeter).OnMeters, WhichMeter)) continue;
                        ++NumVarsOnCustomMeter;
                        if (NumVarsOnCustomMeter > MaxVarsOnCustomMeter) {
                            VarsOnCustomMeter.redimension(MaxVarsOnCustomMeter += 100, 0);
                        }
                        VarsOnCustomMeter(NumVarsOnCustomMeter) = VarMeterArrays(iOnMeter).RepVariable;
                    }
                }
                if (!Tagged) { // couldn't find place for this item on a meter
                    if (AvgSumVar != StoreType::Summed) {
                        ShowWarningError(cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\", variable not summed variable " +
                                         cAlphaFieldNames(fldIndex + 1) + "=\"" + cAlphaArgs(fldIndex + 1) + "\".");
                        ShowContinueError("...will not be shown with the Meter results; units for meter=" + unitEnumToString(MeterUnits) +
                                          ", units for this variable=" + unitEnumToString(UnitsVar) + '.');
                    }
                }
            }
            // Check for duplicates
            for (iKey = 1; iKey <= NumVarsOnCustomMeter; ++iKey) {
                if (VarsOnCustomMeter(iKey) == 0) continue;
                for (iKey1 = iKey + 1; iKey1 <= NumVarsOnCustomMeter; ++iKey1) {
                    if (iKey == iKey1) continue;
                    if (VarsOnCustomMeter(iKey) != VarsOnCustomMeter(iKey1)) continue;
                    ShowWarningError(cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\", duplicate name=\"" +
                                     RVariableTypes(VarsOnCustomMeter(iKey1)).VarName + "\".");
                    ShowContinueError("...only one value with this name will be shown with the Meter results.");
                    VarsOnCustomMeter(iKey1) = 0;
                }
            }
            for (iKey = 1; iKey <= NumVarsOnCustomMeter; ++iKey) {
                if (VarsOnCustomMeter(iKey) == 0) continue;
                auto & tmpVar = RVariableTypes(VarsOnCustomMeter(iKey)).VarPtr;
                AttachCustomMeters(VarsOnCustomMeter(iKey), tmpVar.MeterArrayPtr, NumEnergyMeters);
            }
            if (NumVarsOnCustomMeter == 0) {
                ShowWarningError(cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\", no items assigned ");
                ShowContinueError(
                    "...will not be shown with the Meter results. This may be caused by a Meter:Custom be assigned to another Meter:Custom.");
            }
        }

        cCurrentModuleObject = "Meter:CustomDecrement";
        NumCustomDecMeters = inputProcessor->getNumObjectsFound(cCurrentModuleObject);

        for (Loop = 1; Loop <= NumCustomDecMeters; ++Loop) {
            inputProcessor->getObjectItem(state,
                                          cCurrentModuleObject,
                                          Loop,
                                          cAlphaArgs,
                                          NumAlpha,
                                          rNumericArgs,
                                          NumNumbers,
                                          IOStat,
                                          lNumericFieldBlanks,
                                          lAlphaFieldBlanks,
                                          cAlphaFieldNames,
                                          cNumericFieldNames);
            lbrackPos = index(cAlphaArgs(1), '[');
            if (lbrackPos != std::string::npos) cAlphaArgs(1).erase(lbrackPos);
            MeterCreated = false;
            if (GlobalNames::VerifyUniqueInterObjectName(UniqueMeterNames, cAlphaArgs(1), cCurrentModuleObject, cAlphaFieldNames(1), ErrorsFound)) {
                continue;
            }
            if (allocated(VarsOnCustomMeter)) VarsOnCustomMeter.deallocate();
            VarsOnCustomMeter.allocate(1000);
            VarsOnCustomMeter = 0;
            MaxVarsOnCustomMeter = 1000;
            NumVarsOnCustomMeter = 0;

            lbrackPos = index(cAlphaArgs(3), '[');
            if (lbrackPos != std::string::npos) cAlphaArgs(1).erase(lbrackPos);
            WhichMeter = UtilityRoutines::FindItem(cAlphaArgs(3), EnergyMeters);
            if (WhichMeter == 0) {
                ShowSevereError(cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\", invalid " + cAlphaFieldNames(3) + "=\"" + cAlphaArgs(3) + "\".");
                ErrorsFound = true;
                continue;
            }
            //  Set up array of Vars that are on the source meter (for later validation).
            if (allocated(VarsOnSourceMeter)) VarsOnSourceMeter.deallocate();
            VarsOnSourceMeter.allocate(1000);
            VarsOnSourceMeter = 0;
            MaxVarsOnSourceMeter = 1000;
            NumVarsOnSourceMeter = 0;
            for (iKey = 1; iKey <= NumVarMeterArrays; ++iKey) {
                if (VarMeterArrays(iKey).NumOnMeters == 0 && VarMeterArrays(iKey).NumOnCustomMeters == 0) continue;
                //  On a meter
                if (any_eq(VarMeterArrays(iKey).OnMeters, WhichMeter)) {
                    ++NumVarsOnSourceMeter;
                    if (NumVarsOnSourceMeter > MaxVarsOnSourceMeter) {
                        VarsOnSourceMeter.redimension(MaxVarsOnSourceMeter += 100, 0);
                    }
                    VarsOnSourceMeter(NumVarsOnSourceMeter) = VarMeterArrays(iKey).RepVariable;
                    continue;
                }
                if (VarMeterArrays(iKey).NumOnCustomMeters == 0) continue;
                if (any_eq(VarMeterArrays(iKey).OnCustomMeters, WhichMeter)) {
                    ++NumVarsOnSourceMeter;
                    if (NumVarsOnSourceMeter > MaxVarsOnSourceMeter) {
                        VarsOnSourceMeter.redimension(MaxVarsOnSourceMeter += 100, 0);
                    }
                    VarsOnSourceMeter(NumVarsOnSourceMeter) = VarMeterArrays(iKey).RepVariable;
                    continue;
                }
            }

            for (fldIndex = 4; fldIndex <= NumAlpha; fldIndex += 2) {
                if (cAlphaArgs(fldIndex) == "*" || lAlphaFieldBlanks(fldIndex)) {
                    KeyIsStar = true;
                    cAlphaArgs(fldIndex) = "*";
                } else {
                    KeyIsStar = false;
                }
                if (lAlphaFieldBlanks(fldIndex + 1)) {
                    ShowSevereError(cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\", blank " + cAlphaFieldNames(fldIndex + 1) + '.');
                    ShowContinueError("...cannot create custom meter.");
                    BigErrorsFound = true;
                    continue;
                }
                if (BigErrorsFound) continue;
                Tagged = false;
                lbrackPos = index(cAlphaArgs(fldIndex + 1), '[');
                if (lbrackPos != std::string::npos) cAlphaArgs(fldIndex + 1).erase(lbrackPos);
                // Don't build/check things out if there were errors anywhere.  Use "GetVariableKeys" to map to actual variables...
                GetVariableKeyCountandType(state, cAlphaArgs(fldIndex + 1), KeyCount, TypeVar, AvgSumVar, StepTypeVar, UnitsVar);
                if (TypeVar == VarType_NotFound) {
                    ShowWarningError(cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\", invalid " + cAlphaFieldNames(fldIndex + 1) + "=\"" +
                                     cAlphaArgs(fldIndex + 1) + "\".");
                    ShowContinueError("...will not be shown with the Meter results.");
                    continue;
                }
                if (!MeterCreated) {
                    MeterUnits = UnitsVar;
                    AddMeter(cAlphaArgs(1), UnitsVar, BlankString, BlankString, BlankString, BlankString);
                    EnergyMeters(NumEnergyMeters).TypeOfMeter = MeterType_CustomDec;
                    EnergyMeters(NumEnergyMeters).SourceMeter = WhichMeter;

                    // Can't use resource type in AddMeter cause it will confuse it with other meters.  So, now:
                    GetStandardMeterResourceType(EnergyMeters(NumEnergyMeters).ResourceType, UtilityRoutines::MakeUPPERCase(cAlphaArgs(2)), errFlag);
                    if (errFlag) {
                        ShowContinueError("..on " + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\".");
                        BigErrorsFound = true;
                    }
                    DetermineMeterIPUnits(EnergyMeters(NumEnergyMeters).RT_forIPUnits, EnergyMeters(NumEnergyMeters).ResourceType, UnitsVar, errFlag);
                    if (errFlag) {
                        ShowContinueError("..on " + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\".");
                        ShowContinueError("..requests for IP units from this meter will be ignored.");
                    }
                    //        EnergyMeters(NumEnergyMeters)%RT_forIPUnits=DetermineMeterIPUnits(EnergyMeters(NumEnergyMeters)%ResourceType,UnitsVar)
                    MeterCreated = true;
                }
                if (UnitsVar != MeterUnits) {
                    ShowWarningError(cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\", differing units in " + cAlphaFieldNames(fldIndex + 1) +
                                     "=\"" + cAlphaArgs(fldIndex + 1) + "\".");
                    ShowContinueError("...will not be shown with the Meter results; units for meter=" + unitEnumToString(MeterUnits) +
                                      ", units for this variable=" + unitEnumToString(UnitsVar) + '.');
                    continue;
                }
                if ((TypeVar == VarType_Real || TypeVar == VarType_Integer) && AvgSumVar == StoreType::Summed) {
                    Tagged = true;
                    NamesOfKeys.allocate(KeyCount);
                    IndexesForKeyVar.allocate(KeyCount);
                    GetVariableKeys(state, cAlphaArgs(fldIndex + 1), TypeVar, NamesOfKeys, IndexesForKeyVar);
                    iOnMeter = 0;
                    if (KeyIsStar) {
                        for (iKey = 1; iKey <= KeyCount; ++iKey) {
                            ++NumVarsOnCustomMeter;
                            if (NumVarsOnCustomMeter > MaxVarsOnCustomMeter) {
                                VarsOnCustomMeter.redimension(MaxVarsOnCustomMeter += 100, 0);
                            }
                            VarsOnCustomMeter(NumVarsOnCustomMeter) = IndexesForKeyVar(iKey);
                            iOnMeter = 1;
                        }
                        if (iOnMeter == 0) {
                            ShowSevereError(cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\", invalid (all keys) " +
                                            cAlphaFieldNames(fldIndex + 1) + "=\"" + cAlphaArgs(fldIndex + 1) + "\".");
                            ErrorsFound = true;
                        }
                    } else {
                        for (iKey = 1; iKey <= KeyCount; ++iKey) {
                            if (NamesOfKeys(iKey) != cAlphaArgs(fldIndex)) continue;
                            ++NumVarsOnCustomMeter;
                            if (NumVarsOnCustomMeter > MaxVarsOnCustomMeter) {
                                VarsOnCustomMeter.redimension(MaxVarsOnCustomMeter += 100, 0);
                            }
                            VarsOnCustomMeter(NumVarsOnCustomMeter) = IndexesForKeyVar(iKey);
                            iOnMeter = 1;
                        }
                        if (iOnMeter == 0) {
                            ShowSevereError(cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\", invalid " + cAlphaArgs(fldIndex) + ':' +
                                            cAlphaArgs(fldIndex + 1));
                            ErrorsFound = true;
                        }
                    }
                    NamesOfKeys.deallocate();
                    IndexesForKeyVar.deallocate();
                }
                if (TypeVar == VarType_Meter && AvgSumVar == StoreType::Summed) {
                    Tagged = true;
                    NamesOfKeys.allocate(KeyCount);
                    IndexesForKeyVar.allocate(KeyCount);
                    GetVariableKeys(state, cAlphaArgs(fldIndex + 1), TypeVar, NamesOfKeys, IndexesForKeyVar);
                    WhichMeter = IndexesForKeyVar(1);
                    NamesOfKeys.deallocate();
                    IndexesForKeyVar.deallocate();
                    // for meters there will only be one key...  but it has variables associated...
                    for (iOnMeter = 1; iOnMeter <= NumVarMeterArrays; ++iOnMeter) {
                        testa = any_eq(VarMeterArrays(iOnMeter).OnMeters, WhichMeter);
                        testb = false;
                        if (VarMeterArrays(iOnMeter).NumOnCustomMeters > 0) {
                            testb = any_eq(VarMeterArrays(iOnMeter).OnCustomMeters, WhichMeter);
                        }
                        if (!(testa || testb)) continue;
                        ++NumVarsOnCustomMeter;
                        if (NumVarsOnCustomMeter > MaxVarsOnCustomMeter) {
                            VarsOnCustomMeter.redimension(MaxVarsOnCustomMeter += 100, 0);
                        }
                        VarsOnCustomMeter(NumVarsOnCustomMeter) = VarMeterArrays(iOnMeter).RepVariable;
                    }
                }
                if (!Tagged) { // couldn't find place for this item on a meter
                    if (AvgSumVar != StoreType::Summed) {
                        ShowWarningError(cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\", variable not summed variable " +
                                         cAlphaFieldNames(fldIndex + 1) + "=\"" + cAlphaArgs(fldIndex + 1) + "\".");
                        ShowContinueError("...will not be shown with the Meter results; units for meter=" + unitEnumToString(MeterUnits) +
                                          ", units for this variable=" + unitEnumToString(UnitsVar) + '.');
                    }
                }
            }
            // Check for duplicates
            for (iKey = 1; iKey <= NumVarsOnCustomMeter; ++iKey) {
                if (VarsOnCustomMeter(iKey) == 0) continue;
                for (iKey1 = iKey + 1; iKey1 <= NumVarsOnCustomMeter; ++iKey1) {
                    if (iKey == iKey1) continue;
                    if (VarsOnCustomMeter(iKey) != VarsOnCustomMeter(iKey1)) continue;
                    ShowWarningError(cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\", duplicate name=\"" +
                                     RVariableTypes(VarsOnCustomMeter(iKey1)).VarName + "\".");
                    ShowContinueError("...only one value with this name will be shown with the Meter results.");
                    VarsOnCustomMeter(iKey1) = 0;
                }
            }
            for (iKey = 1; iKey <= NumVarsOnCustomMeter; ++iKey) {
                if (VarsOnCustomMeter(iKey) == 0) continue;
                auto & tmpVar = RVariableTypes(VarsOnCustomMeter(iKey)).VarPtr;
                AttachCustomMeters(VarsOnCustomMeter(iKey), tmpVar.MeterArrayPtr, NumEnergyMeters);
            }

            errFlag = false;
            for (iKey = 1; iKey <= NumVarsOnCustomMeter; ++iKey) {
                for (iKey1 = 1; iKey1 <= NumVarsOnSourceMeter; ++iKey1) {
                    if (any_eq(VarsOnSourceMeter, VarsOnCustomMeter(iKey))) break;
                    if (!errFlag) {
                        ShowSevereError(cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\", invalid specification to " + cAlphaFieldNames(3) + "=\"" +
                                        cAlphaArgs(3) + "\".");
                        errFlag = true;
                    }
                    ShowContinueError("..Variable=" + RVariableTypes(VarsOnCustomMeter(iKey)).VarName);
                    ErrorsFound = true;
                    break;
                }
            }
            if (NumVarsOnCustomMeter == 0) {
                ShowWarningError(cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\", no items assigned ");
                ShowContinueError("...will not be shown with the Meter results");
            }

            VarsOnCustomMeter.deallocate();
            VarsOnSourceMeter.deallocate();
        }

        if (BigErrorsFound) ErrorsFound = true;
    }

    void GetStandardMeterResourceType(std::string &OutResourceType,
                                      std::string const &UserInputResourceType, // Passed uppercase
                                      bool &ErrorsFound)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda Lawrie
        //       DATE WRITTEN   April 2006
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This routine compares the user input resource type with valid ones and returns
        // the standard resource type.

        ErrorsFound = false;

        // Basic ResourceType for Meters
        {
            auto const meterType(UserInputResourceType);

            if (meterType == "ELECTRICITY") {
                OutResourceType = "Electricity";

            } else if (meterType == "NATURALGAS") {
                OutResourceType = "NaturalGas";

            } else if (meterType == "GASOLINE") {
                OutResourceType = "Gasoline";

            } else if (meterType == "DIESEL") {
                OutResourceType = "Diesel";

            } else if (meterType == "COAL") {
                OutResourceType = "Coal";

            } else if (meterType == "FUELOILNO1") {
                OutResourceType = "FuelOilNo1";

            } else if (meterType == "FUELOILNO2") {
                OutResourceType = "FuelOilNo2";

            } else if (meterType == "PROPANE") {
                OutResourceType = "Propane";

            } else if (meterType == "WATER" || meterType == "H2O") {
                OutResourceType = "Water"; // this is water "use"

            } else if (meterType == "ONSITEWATER" || meterType == "WATERPRODUCED" || meterType == "ONSITE WATER") {
                OutResourceType = "OnSiteWater"; // these are for supply record keeping

            } else if (meterType == "MAINSWATER" || meterType == "WATERSUPPLY") {
                OutResourceType = "MainsWater"; // record keeping

            } else if (meterType == "RAINWATER" || meterType == "PRECIPITATION") {
                OutResourceType = "RainWater"; // record keeping

            } else if (meterType == "WELLWATER" || meterType == "GROUNDWATER") {
                OutResourceType = "WellWater"; // record keeping

            } else if (meterType == "CONDENSATE") {
                OutResourceType = "Condensate"; // record keeping

            } else if (meterType == "ENERGYTRANSFER" || meterType == "ENERGYXFER" || meterType == "XFER") {
                OutResourceType = "EnergyTransfer";

            } else if (meterType == "STEAM") {
                OutResourceType = "Steam";

            } else if (meterType == "DISTRICTCOOLING") {
                OutResourceType = "DistrictCooling";

            } else if (meterType == "DISTRICTHEATING") {
                OutResourceType = "DistrictHeating";

            } else if (meterType == "ELECTRICITYPRODUCED") {
                OutResourceType = "ElectricityProduced";

            } else if (meterType == "ELECTRICITYPURCHASED") {
                OutResourceType = "ElectricityPurchased";

            } else if (meterType == "ELECTRICITYSURPLUSSOLD") {
                OutResourceType = "ElectricitySurplusSold";

            } else if (meterType == "ELECTRICITYNET") {
                OutResourceType = "ElectricityNet";

            } else if (meterType == "SOLARWATER") {
                OutResourceType = "SolarWater";

            } else if (meterType == "SOLARAIR") {
                OutResourceType = "SolarAir";

            } else if (meterType == "SO2") {
                OutResourceType = "SO2";

            } else if (meterType == "NOX") {
                OutResourceType = "NOx";

            } else if (meterType == "N2O") {
                OutResourceType = "N2O";

            } else if (meterType == "PM") {
                OutResourceType = "PM";

            } else if (meterType == "PM2.5") {
                OutResourceType = "PM2.5";

            } else if (meterType == "PM10") {
                OutResourceType = "PM10";

            } else if (meterType == "CO") {
                OutResourceType = "CO";

            } else if (meterType == "CO2") {
                OutResourceType = "CO2";

            } else if (meterType == "CH4") {
                OutResourceType = "CH4";

            } else if (meterType == "NH3") {
                OutResourceType = "NH3";

            } else if (meterType == "NMVOC") {
                OutResourceType = "NMVOC";

            } else if (meterType == "HG") {
                OutResourceType = "Hg";

            } else if (meterType == "PB") {
                OutResourceType = "Pb";

            } else if (meterType == "NUCLEAR HIGH") {
                OutResourceType = "Nuclear High";

            } else if (meterType == "NUCLEAR LOW") {
                OutResourceType = "Nuclear Low";

            } else if (meterType == "WATERENVIRONMENTALFACTORS") {
                OutResourceType = "WaterEnvironmentalFactors";

            } else if (meterType == "CARBON EQUIVALENT") {
                OutResourceType = "Carbon Equivalent";

            } else if (meterType == "SOURCE") {
                OutResourceType = "Source";

            } else if (meterType == "PLANTLOOPHEATINGDEMAND") {
                OutResourceType = "PlantLoopHeatingDemand";

            } else if (meterType == "PLANTLOOPCOOLINGDEMAND") {
                OutResourceType = "PlantLoopCoolingDemand";

            } else if (meterType == "GENERIC") { // only used by custom meters
                OutResourceType = "Generic";

            } else if (meterType == "OTHERFUEL1") { // other fuel type (defined by user)
                OutResourceType = "OtherFuel1";

            } else if (meterType == "OTHERFUEL2") { // other fuel type (defined by user)
                OutResourceType = "OtherFuel2";

            } else {
                ShowSevereError("GetStandardMeterResourceType: Illegal OutResourceType (for Meters) Entered=" + UserInputResourceType);
                ErrorsFound = true;
            }
        }
    }

    void AddMeter(std::string const &Name,               // Name for the meter
                  OutputProcessor::Unit const &MtrUnits, // Units for the meter
                  std::string const &ResourceType,       // ResourceType for the meter
                  std::string const &EndUse,             // EndUse for the meter
                  std::string const &EndUseSub,          // EndUse subcategory for the meter
                  std::string const &Group               // Group for the meter
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda Lawrie
        //       DATE WRITTEN   January 2001
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine adds a meter to the current definition set of meters.  If the maximum has
        // already been reached, a reallocation procedure begins.  This action needs to be done at the
        // start of the simulation, primarily before any output is stored.

        // Make sure this isn't already in the list of meter names
        int Found;
        if (NumEnergyMeters > 0) {
            Found = UtilityRoutines::FindItemInList(Name, EnergyMeters);
        } else {
            Found = 0;
        }

        if (Found == 0) {
            EnergyMeters.redimension(++NumEnergyMeters);
            EnergyMeters(NumEnergyMeters).Name = Name;
            EnergyMeters(NumEnergyMeters).ResourceType = ResourceType;
            EnergyMeters(NumEnergyMeters).EndUse = EndUse;
            EnergyMeters(NumEnergyMeters).EndUseSub = EndUseSub;
            EnergyMeters(NumEnergyMeters).Group = Group;
            EnergyMeters(NumEnergyMeters).Units = MtrUnits;
            EnergyMeters(NumEnergyMeters).TSValue = 0.0;
            EnergyMeters(NumEnergyMeters).CurTSValue = 0.0;
            EnergyMeters(NumEnergyMeters).RptTS = false;
            EnergyMeters(NumEnergyMeters).RptTSFO = false;
            AssignReportNumber(EnergyMeters(NumEnergyMeters).TSRptNum);
            EnergyMeters(NumEnergyMeters).TSRptNumChr = fmt::to_string(EnergyMeters(NumEnergyMeters).TSRptNum);
            EnergyMeters(NumEnergyMeters).HRValue = 0.0;
            EnergyMeters(NumEnergyMeters).HRMaxVal = MaxSetValue;
            EnergyMeters(NumEnergyMeters).HRMaxValDate = 0;
            EnergyMeters(NumEnergyMeters).HRMinVal = MinSetValue;
            EnergyMeters(NumEnergyMeters).HRMinValDate = 0;
            EnergyMeters(NumEnergyMeters).RptHR = false;
            EnergyMeters(NumEnergyMeters).RptHRFO = false;
            AssignReportNumber(EnergyMeters(NumEnergyMeters).HRRptNum);
            EnergyMeters(NumEnergyMeters).HRRptNumChr = fmt::to_string(EnergyMeters(NumEnergyMeters).HRRptNum);
            EnergyMeters(NumEnergyMeters).DYValue = 0.0;
            EnergyMeters(NumEnergyMeters).DYMaxVal = MaxSetValue;
            EnergyMeters(NumEnergyMeters).DYMaxValDate = 0;
            EnergyMeters(NumEnergyMeters).DYMinVal = MinSetValue;
            EnergyMeters(NumEnergyMeters).DYMinValDate = 0;
            EnergyMeters(NumEnergyMeters).RptDY = false;
            EnergyMeters(NumEnergyMeters).RptDYFO = false;
            AssignReportNumber(EnergyMeters(NumEnergyMeters).DYRptNum);
            EnergyMeters(NumEnergyMeters).DYRptNumChr = fmt::to_string(EnergyMeters(NumEnergyMeters).DYRptNum);
            EnergyMeters(NumEnergyMeters).MNValue = 0.0;
            EnergyMeters(NumEnergyMeters).MNMaxVal = MaxSetValue;
            EnergyMeters(NumEnergyMeters).MNMaxValDate = 0;
            EnergyMeters(NumEnergyMeters).MNMinVal = MinSetValue;
            EnergyMeters(NumEnergyMeters).MNMinValDate = 0;
            EnergyMeters(NumEnergyMeters).RptMN = false;
            EnergyMeters(NumEnergyMeters).RptMNFO = false;
            AssignReportNumber(EnergyMeters(NumEnergyMeters).MNRptNum);
            EnergyMeters(NumEnergyMeters).MNRptNumChr = fmt::to_string(EnergyMeters(NumEnergyMeters).MNRptNum);
            EnergyMeters(NumEnergyMeters).YRValue = 0.0;
            EnergyMeters(NumEnergyMeters).YRMaxVal = MaxSetValue;
            EnergyMeters(NumEnergyMeters).YRMaxValDate = 0;
            EnergyMeters(NumEnergyMeters).YRMinVal = MinSetValue;
            EnergyMeters(NumEnergyMeters).YRMinValDate = 0;
            EnergyMeters(NumEnergyMeters).RptYR = false;
            EnergyMeters(NumEnergyMeters).RptYRFO = false;
            AssignReportNumber(EnergyMeters(NumEnergyMeters).YRRptNum);
            EnergyMeters(NumEnergyMeters).YRRptNumChr = fmt::to_string(EnergyMeters(NumEnergyMeters).YRRptNum);
            EnergyMeters(NumEnergyMeters).SMValue = 0.0;
            EnergyMeters(NumEnergyMeters).SMMaxVal = MaxSetValue;
            EnergyMeters(NumEnergyMeters).SMMaxValDate = 0;
            EnergyMeters(NumEnergyMeters).SMMinVal = MinSetValue;
            EnergyMeters(NumEnergyMeters).SMMinValDate = 0;
            EnergyMeters(NumEnergyMeters).RptSM = false;
            EnergyMeters(NumEnergyMeters).RptSMFO = false;
            AssignReportNumber(EnergyMeters(NumEnergyMeters).SMRptNum);
            EnergyMeters(NumEnergyMeters).SMRptNumChr = fmt::to_string(EnergyMeters(NumEnergyMeters).SMRptNum);
            AssignReportNumber(EnergyMeters(NumEnergyMeters).TSAccRptNum);
            AssignReportNumber(EnergyMeters(NumEnergyMeters).HRAccRptNum);
            AssignReportNumber(EnergyMeters(NumEnergyMeters).DYAccRptNum);
            AssignReportNumber(EnergyMeters(NumEnergyMeters).MNAccRptNum);
            AssignReportNumber(EnergyMeters(NumEnergyMeters).YRAccRptNum);
            AssignReportNumber(EnergyMeters(NumEnergyMeters).SMAccRptNum);
            EnergyMeters(NumEnergyMeters).FinYrSMValue = 0.0;
            EnergyMeters(NumEnergyMeters).FinYrSMMaxVal = MaxSetValue;
            EnergyMeters(NumEnergyMeters).FinYrSMMaxValDate = 0;
            EnergyMeters(NumEnergyMeters).FinYrSMMinVal = MinSetValue;
            EnergyMeters(NumEnergyMeters).FinYrSMMinValDate = 0;
        } else {
            ShowFatalError("Requested to Add Meter which was already present=" + Name);
        }
        if (!ResourceType.empty()) {
            bool errFlag;
            DetermineMeterIPUnits(EnergyMeters(NumEnergyMeters).RT_forIPUnits, ResourceType, MtrUnits, errFlag);
            if (errFlag) {
                ShowContinueError("..on Meter=\"" + Name + "\".");
                ShowContinueError("..requests for IP units from this meter will be ignored.");
            }
        }
    }

    void AttachMeters(OutputProcessor::Unit const &MtrUnits, // Units for this meter
                      std::string &ResourceType,             // Electricity, Gas, etc.
                      std::string &EndUse,                   // End-use category (Lights, Heating, etc.)
                      std::string &EndUseSub,                // End-use subcategory (user-defined, e.g., General Lights, Task Lights, etc.)
                      std::string &Group,                    // Group key (Facility, Zone, Building, etc.)
                      std::string const &ZoneName,           // Zone key only applicable for Building group
                      int const RepVarNum,                   // Number of this report variable
                      int &MeterArrayPtr,                    // Output set of Pointers to Meters
                      bool &ErrorsFound                      // True if errors in this call
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda Lawrie
        //       DATE WRITTEN   January 2001
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine determines which meters this variable will be on (if any),
        // sets up the meter pointer arrays, and returns a index value to this array which
        // is stored with the variable.

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:

        if (UtilityRoutines::SameString(Group, "Building")) {
            ValidateNStandardizeMeterTitles(MtrUnits, ResourceType, EndUse, EndUseSub, Group, ErrorsFound, ZoneName);
        } else {
            ValidateNStandardizeMeterTitles(MtrUnits, ResourceType, EndUse, EndUseSub, Group, ErrorsFound);
        }

        VarMeterArrays.redimension(++NumVarMeterArrays);
        MeterArrayPtr = NumVarMeterArrays;
        VarMeterArrays(NumVarMeterArrays).NumOnMeters = 0;
        VarMeterArrays(NumVarMeterArrays).RepVariable = RepVarNum;
        VarMeterArrays(NumVarMeterArrays).OnMeters = 0;
        int Found = UtilityRoutines::FindItem(ResourceType + ":Facility", EnergyMeters);
        if (Found != 0) {
            ++VarMeterArrays(NumVarMeterArrays).NumOnMeters;
            VarMeterArrays(NumVarMeterArrays).OnMeters(VarMeterArrays(NumVarMeterArrays).NumOnMeters) = Found;
        }
        if (!Group.empty()) {
            Found = UtilityRoutines::FindItem(ResourceType + ':' + Group, EnergyMeters);
            if (Found != 0) {
                ++VarMeterArrays(NumVarMeterArrays).NumOnMeters;
                VarMeterArrays(NumVarMeterArrays).OnMeters(VarMeterArrays(NumVarMeterArrays).NumOnMeters) = Found;
            }
            if (UtilityRoutines::SameString(Group, "Building")) { // Match to Zone
                Found = UtilityRoutines::FindItem(ResourceType + ":Zone:" + ZoneName, EnergyMeters);
                if (Found != 0) {
                    ++VarMeterArrays(NumVarMeterArrays).NumOnMeters;
                    VarMeterArrays(NumVarMeterArrays).OnMeters(VarMeterArrays(NumVarMeterArrays).NumOnMeters) = Found;
                }
            }
        }

        //!! Following if EndUse is by ResourceType
        if (!EndUse.empty()) {
            Found = UtilityRoutines::FindItem(EndUse + ':' + ResourceType, EnergyMeters);
            if (Found != 0) {
                ++VarMeterArrays(NumVarMeterArrays).NumOnMeters;
                VarMeterArrays(NumVarMeterArrays).OnMeters(VarMeterArrays(NumVarMeterArrays).NumOnMeters) = Found;
            }
            if (UtilityRoutines::SameString(Group, "Building")) { // Match to Zone
                Found = UtilityRoutines::FindItem(EndUse + ':' + ResourceType + ":Zone:" + ZoneName, EnergyMeters);
                if (Found != 0) {
                    ++VarMeterArrays(NumVarMeterArrays).NumOnMeters;
                    VarMeterArrays(NumVarMeterArrays).OnMeters(VarMeterArrays(NumVarMeterArrays).NumOnMeters) = Found;
                }
            }

            // End use subcategory
            if (!EndUseSub.empty()) {
                Found = UtilityRoutines::FindItem(EndUseSub + ':' + EndUse + ':' + ResourceType, EnergyMeters);
                if (Found != 0) {
                    ++VarMeterArrays(NumVarMeterArrays).NumOnMeters;
                    VarMeterArrays(NumVarMeterArrays).OnMeters(VarMeterArrays(NumVarMeterArrays).NumOnMeters) = Found;

                    AddEndUseSubcategory(ResourceType, EndUse, EndUseSub);
                }
                if (UtilityRoutines::SameString(Group, "Building")) { // Match to Zone
                    Found = UtilityRoutines::FindItem(EndUseSub + ':' + EndUse + ':' + ResourceType + ":Zone:" + ZoneName, EnergyMeters);
                    if (Found != 0) {
                        ++VarMeterArrays(NumVarMeterArrays).NumOnMeters;
                        VarMeterArrays(NumVarMeterArrays).OnMeters(VarMeterArrays(NumVarMeterArrays).NumOnMeters) = Found;
                    }
                }
            }
        }
    }

    void AttachCustomMeters(int const RepVarNum, // Number of this report variable
                            int &MeterArrayPtr,  // Input/Output set of Pointers to Meters
                            int const MeterIndex // Which meter this is
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda Lawrie
        //       DATE WRITTEN   January 2006
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine determines which meters this variable will be on (if any),
        // sets up the meter pointer arrays, and returns a index value to this array which
        // is stored with the variable.

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:

        if (MeterArrayPtr == 0) {
            VarMeterArrays.redimension(++NumVarMeterArrays);
            MeterArrayPtr = NumVarMeterArrays;
            VarMeterArrays(NumVarMeterArrays).NumOnMeters = 0;
            VarMeterArrays(NumVarMeterArrays).RepVariable = RepVarNum;
            VarMeterArrays(NumVarMeterArrays).OnMeters = 0;
            VarMeterArrays(NumVarMeterArrays).OnCustomMeters.allocate(1);
            VarMeterArrays(NumVarMeterArrays).NumOnCustomMeters = 1;
        } else { // MeterArrayPtr set
            VarMeterArrays(MeterArrayPtr).OnCustomMeters.redimension(++VarMeterArrays(MeterArrayPtr).NumOnCustomMeters);
        }
        VarMeterArrays(MeterArrayPtr).OnCustomMeters(VarMeterArrays(MeterArrayPtr).NumOnCustomMeters) = MeterIndex;
    }

    void ValidateNStandardizeMeterTitles(OutputProcessor::Unit const &MtrUnits, // Units for the meter
                                         std::string &ResourceType,             // Electricity, Gas, etc.
                                         std::string &EndUse,                   // End Use Type (Lights, Heating, etc.)
                                         std::string &EndUseSub,                // End Use Sub Type (General Lights, Task Lights, etc.)
                                         std::string &Group,                    // Group key (Facility, Zone, Building, etc.)
                                         bool &ErrorsFound,                     // True if errors in this call
                                         Optional_string_const ZoneName         // ZoneName when Group=Building
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda Lawrie
        //       DATE WRITTEN   January 2001
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine uses the keys for the Energy Meters given to the SetupOutputVariable routines
        // and makes sure they are "standard" as well as creating meters which need to be added as this
        // is the first use of that kind of meter designation.

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int Found; // For checking whether meter is already defined
        bool LocalErrorsFound;
        std::string MeterName;

        LocalErrorsFound = false;
        // Basic ResourceType Meters
        GetStandardMeterResourceType(ResourceType, UtilityRoutines::MakeUPPERCase(ResourceType), LocalErrorsFound);

        if (!LocalErrorsFound) {
            if (NumEnergyMeters > 0) {
                Found = UtilityRoutines::FindItem(ResourceType + ":Facility", EnergyMeters);
            } else {
                Found = 0;
            }
            if (Found == 0) AddMeter(ResourceType + ":Facility", MtrUnits, ResourceType, "", "", "");
        }

        //!  Group Meters
        {
            auto const groupMeter(uppercased(Group));

            if (groupMeter.empty()) {

            } else if (groupMeter == "BUILDING") {
                Group = "Building";

            } else if (groupMeter == "HVAC" || groupMeter == "SYSTEM") {
                Group = "HVAC";

            } else if (groupMeter == "PLANT") {
                Group = "Plant";

            } else {
                ShowSevereError("Illegal Group (for Meters) Entered=" + Group);
                LocalErrorsFound = true;
            }
        }

        if (!LocalErrorsFound && !Group.empty()) {
            Found = UtilityRoutines::FindItem(ResourceType + ':' + Group, EnergyMeters);
            if (Found == 0) AddMeter(ResourceType + ':' + Group, MtrUnits, ResourceType, "", "", Group);
            if (Group == "Building") {
                Found = UtilityRoutines::FindItem(ResourceType + ":Zone:" + ZoneName, EnergyMeters);
                if (Found == 0) {
                    AddMeter(ResourceType + ":Zone:" + ZoneName, MtrUnits, ResourceType, "", "", "Zone");
                }
            }
        }

        //!!! EndUse Meters
        {
            auto const endUseMeter(uppercased(EndUse));

            if (endUseMeter.empty()) {

            } else if (endUseMeter == "INTERIOR LIGHTS" || endUseMeter == "INTERIORLIGHTS") {
                EndUse = "InteriorLights";

            } else if (endUseMeter == "EXTERIOR LIGHTS" || endUseMeter == "EXTERIORLIGHTS") {
                EndUse = "ExteriorLights";

            } else if (endUseMeter == "HEATING" || endUseMeter == "HTG") {
                EndUse = "Heating";

            } else if (endUseMeter == "HEATPRODUCED") {
                EndUse = "HeatProduced";

            } else if (endUseMeter == "COOLING" || endUseMeter == "CLG") {
                EndUse = "Cooling";

            } else if (endUseMeter == "DOMESTICHOTWATER" || endUseMeter == "DHW" || endUseMeter == "DOMESTIC HOT WATER") {
                EndUse = "WaterSystems";

            } else if (endUseMeter == "COGEN" || endUseMeter == "COGENERATION") {
                EndUse = "Cogeneration";

            } else if (endUseMeter == "INTERIOREQUIPMENT" || endUseMeter == "INTERIOR EQUIPMENT") {
                EndUse = "InteriorEquipment";

            } else if (endUseMeter == "EXTERIOREQUIPMENT" || endUseMeter == "EXTERIOR EQUIPMENT" || endUseMeter == "EXT EQ" ||
                       endUseMeter == "EXTERIOREQ") {
                EndUse = "ExteriorEquipment";

            } else if (endUseMeter == "EXTERIOR:WATEREQUIPMENT") {
                EndUse = "ExteriorEquipment";

            } else if (endUseMeter == "PURCHASEDHOTWATER" || endUseMeter == "DISTRICTHOTWATER" || endUseMeter == "PURCHASED HEATING") {
                EndUse = "DistrictHotWater";

            } else if (endUseMeter == "PURCHASEDCOLDWATER" || endUseMeter == "DISTRICTCHILLEDWATER" || endUseMeter == "PURCHASEDCHILLEDWATER" ||
                       endUseMeter == "PURCHASED COLD WATER" || endUseMeter == "PURCHASED COOLING") {
                EndUse = "DistrictChilledWater";

            } else if (endUseMeter == "FANS" || endUseMeter == "FAN") {
                EndUse = "Fans";

            } else if (endUseMeter == "HEATINGCOILS" || endUseMeter == "HEATINGCOIL" || endUseMeter == "HEATING COILS" ||
                       endUseMeter == "HEATING COIL") {
                EndUse = "HeatingCoils";

            } else if (endUseMeter == "COOLINGCOILS" || endUseMeter == "COOLINGCOIL" || endUseMeter == "COOLING COILS" ||
                       endUseMeter == "COOLING COIL") {
                EndUse = "CoolingCoils";

            } else if (endUseMeter == "PUMPS" || endUseMeter == "PUMP") {
                EndUse = "Pumps";

            } else if (endUseMeter == "FREECOOLING" || endUseMeter == "FREE COOLING") {
                EndUse = "Freecooling";

            } else if (endUseMeter == "LOOPTOLOOP") {
                EndUse = "LoopToLoop";

            } else if (endUseMeter == "CHILLERS" || endUseMeter == "CHILLER") {
                EndUse = "Chillers";

            } else if (endUseMeter == "BOILERS" || endUseMeter == "BOILER") {
                EndUse = "Boilers";

            } else if (endUseMeter == "BASEBOARD" || endUseMeter == "BASEBOARDS") {
                EndUse = "Baseboard";

            } else if (endUseMeter == "COOLINGPANEL" || endUseMeter == "COOLINGPANELS") {
                EndUse = "CoolingPanel";

            } else if (endUseMeter == "HEATREJECTION" || endUseMeter == "HEAT REJECTION") {
                EndUse = "HeatRejection";

            } else if (endUseMeter == "HUMIDIFIER" || endUseMeter == "HUMIDIFIERS") {
                EndUse = "Humidifier";

            } else if (endUseMeter == "HEATRECOVERY" || endUseMeter == "HEAT RECOVERY") {
                EndUse = "HeatRecovery";

            } else if (endUseMeter == "PHOTOVOLTAICS" || endUseMeter == "PV" || endUseMeter == "PHOTOVOLTAIC") {
                EndUse = "Photovoltaic";

            } else if (endUseMeter == "WINDTURBINES" || endUseMeter == "WT" || endUseMeter == "WINDTURBINE") {
                EndUse = "WindTurbine";

            } else if (endUseMeter == "ELECTRICSTORAGE") {
                EndUse = "ElectricStorage";

            } else if (endUseMeter == "POWERCONVERSION") {

                EndUse = "PowerConversion";

            } else if (endUseMeter == "HEAT RECOVERY FOR COOLING" || endUseMeter == "HEATRECOVERYFORCOOLING" ||
                       endUseMeter == "HEATRECOVERYCOOLING") {
                EndUse = "HeatRecoveryForCooling";

            } else if (endUseMeter == "HEAT RECOVERY FOR HEATING" || endUseMeter == "HEATRECOVERYFORHEATING" ||
                       endUseMeter == "HEATRECOVERYHEATING") {
                EndUse = "HeatRecoveryForHeating";

            } else if (endUseMeter == "ELECTRICEMISSIONS") {
                EndUse = "ElectricEmissions";

            } else if (endUseMeter == "PURCHASEDELECTRICEMISSIONS") {
                EndUse = "PurchasedElectricEmissions";

            } else if (endUseMeter == "SOLDELECTRICEMISSIONS") {
                EndUse = "SoldElectricEmissions";

            } else if (endUseMeter == "NATURALGASEMISSIONS") {
                EndUse = "NaturalGasEmissions";

            } else if (endUseMeter == "FUELOILNO1EMISSIONS") {
                EndUse = "FuelOilNo1Emissions";

            } else if (endUseMeter == "FUELOILNO2EMISSIONS") {
                EndUse = "FuelOilNo2Emissions";

            } else if (endUseMeter == "COALEMISSIONS") {
                EndUse = "CoalEmissions";

            } else if (endUseMeter == "GASOLINEEMISSIONS") {
                EndUse = "GasolineEmissions";

            } else if (endUseMeter == "PROPANEEMISSIONS") {
                EndUse = "PropaneEmissions";

            } else if (endUseMeter == "DIESELEMISSIONS") {
                EndUse = "DieselEmissions";

            } else if (endUseMeter == "OTHERFUEL1EMISSIONS") {
                EndUse = "OtherFuel1Emissions";

            } else if (endUseMeter == "OTHERFUEL2EMISSIONS") {
                EndUse = "OtherFuel2Emissions";

            } else if (endUseMeter == "CARBONEQUIVALENTEMISSIONS") {
                EndUse = "CarbonEquivalentEmissions";

            } else if (endUseMeter == "REFRIGERATION") {
                EndUse = "Refrigeration";

            } else if (endUseMeter == "COLDSTORAGECHARGE") {
                EndUse = "ColdStorageCharge";

            } else if (endUseMeter == "COLDSTORAGEDISCHARGE") {
                EndUse = "ColdStorageDischarge";

            } else if (endUseMeter == "WATERSYSTEMS" || endUseMeter == "WATERSYSTEM" || endUseMeter == "Water System") {
                EndUse = "WaterSystems";

            } else if (endUseMeter == "RAINWATER") {
                EndUse = "Rainwater";

            } else if (endUseMeter == "CONDENSATE") {
                EndUse = "Condensate";

            } else if (endUseMeter == "WELLWATER") {
                EndUse = "Wellwater";

            } else if (endUseMeter == "MAINSWATER" || endUseMeter == "PURCHASEDWATER") {
                EndUse = "MainsWater";

            } else {
                ShowSevereError("Illegal EndUse (for Meters) Entered=" + EndUse);
                LocalErrorsFound = true;
            }
        }

        //!! Following if we do EndUse by ResourceType
        if (!LocalErrorsFound && !EndUse.empty()) {
            Found = UtilityRoutines::FindItem(EndUse + ':' + ResourceType, EnergyMeters);
            if (Found == 0) AddMeter(EndUse + ':' + ResourceType, MtrUnits, ResourceType, EndUse, "", "");

            if (Group == "Building") { // Match to Zone
                Found = UtilityRoutines::FindItem(EndUse + ':' + ResourceType + ":Zone:" + ZoneName, EnergyMeters);
                if (Found == 0) {
                    AddMeter(EndUse + ':' + ResourceType + ":Zone:" + ZoneName, MtrUnits, ResourceType, EndUse, "", "Zone");
                }
            }
        } else if (LocalErrorsFound) {
            ErrorsFound = true;
        }

        // End-Use Subcategories
        if (!LocalErrorsFound && !EndUseSub.empty()) {
            MeterName = EndUseSub + ':' + EndUse + ':' + ResourceType;
            Found = UtilityRoutines::FindItem(MeterName, EnergyMeters);
            if (Found == 0) AddMeter(MeterName, MtrUnits, ResourceType, EndUse, EndUseSub, "");
        } else if (LocalErrorsFound) {
            ErrorsFound = true;
        }
    }

    void DetermineMeterIPUnits(int &CodeForIPUnits,                   // Output Code for IP Units
                               std::string const &ResourceType,       // Resource Type
                               OutputProcessor::Unit const &MtrUnits, // Meter units
                               bool &ErrorsFound                      // true if errors found during subroutine
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda Lawrie
        //       DATE WRITTEN   January 2012
        //       MODIFIED       September 2012; made into subroutine
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // In order to set up tabular reports for IP units, need to search on same strings
        // that tabular reports does for IP conversion.

        // REFERENCES:
        // OutputReportTabular looks for:
        // CONSUMP - not used in meters
        // ELEC - Electricity (kWH)
        // GAS - Gas (therm)
        // COOL - Cooling (ton)
        // and we need to add WATER (for m3/gal, etc)

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        std::string UC_ResourceType;

        ErrorsFound = false;
        UC_ResourceType = UtilityRoutines::MakeUPPERCase(ResourceType);

        CodeForIPUnits = RT_IPUnits_OtherJ;
        if (has(UC_ResourceType, "ELEC")) {
            CodeForIPUnits = RT_IPUnits_Electricity;
        } else if (has(UC_ResourceType, "GAS")) {
            CodeForIPUnits = RT_IPUnits_Gas;
        } else if (has(UC_ResourceType, "COOL")) {
            CodeForIPUnits = RT_IPUnits_Cooling;
        }
        if (MtrUnits == OutputProcessor::Unit::m3 && has(UC_ResourceType, "WATER")) {
            CodeForIPUnits = RT_IPUnits_Water;
        } else if (MtrUnits == OutputProcessor::Unit::m3) {
            CodeForIPUnits = RT_IPUnits_OtherM3;
        }
        if (MtrUnits == OutputProcessor::Unit::kg) {
            CodeForIPUnits = RT_IPUnits_OtherKG;
        }
        if (MtrUnits == OutputProcessor::Unit::L) {
            CodeForIPUnits = RT_IPUnits_OtherL;
        }
        //  write(outputfiledebug,*) 'resourcetype=',TRIM(resourcetype)
        //  write(outputfiledebug,*) 'ipunits type=',CodeForIPUnits
        if (!(MtrUnits == OutputProcessor::Unit::kg) && !(MtrUnits == OutputProcessor::Unit::J) && !(MtrUnits == OutputProcessor::Unit::m3) &&
            !(MtrUnits == OutputProcessor::Unit::L)) {
            ShowWarningError("DetermineMeterIPUnits: Meter units not recognized for IP Units conversion=[" + unitEnumToString(MtrUnits) + "].");
            ErrorsFound = true;
        }
    }

    void UpdateMeterValues(Real64 const TimeStepValue,                // Value of this variable at the current time step.
                           int const NumOnMeters,                     // Number of meters this variable is "on".
                           const Array1D_int &OnMeters                // Which meters this variable is on (index values)
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda Lawrie
        //       DATE WRITTEN   January 2001
        //       MODIFIED       Jason DeGraw 2/12/2020, de-optionalized
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine updates all the meter values in the lists with the current
        // time step value for this variable.

        // METHODOLOGY EMPLOYED:
        // Variables, as they are "setup", may or may not be on one or more meters.
        // All "metered" variables are on the "facility meter".  Index values will be
        // set from the variables to the appropriate meters.  Then, the updating of
        // the meter values is quite simple -- just add the time step value of the variable
        // (which is passed to this routine) to all the values being kept for the meter.
        // Reporting of the meters is taken care of in a different routine.  During reporting,
        // some values will also be reset (for example, after reporting the "hour", the new
        // "hour" value of the meter is reset to 0.0, etc.

        // REFERENCES:
        // na

        // USE STATEMENTS:
        // na

        // Argument array dimensioning

        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:

        // SUBROUTINE PARAMETER DEFINITIONS:
        // na

        // INTERFACE BLOCK SPECIFICATIONS:
        // na

        // DERIVED TYPE DEFINITIONS:
        // na

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int Meter; // Loop Control
        int Which; // Index value for the meter

        for (Meter = 1; Meter <= NumOnMeters; ++Meter) {
            Which = OnMeters(Meter);
            MeterValue(Which) += TimeStepValue;
        }

    }

    void UpdateMeterValues(Real64 const TimeStepValue,                // Value of this variable at the current time step.
                           int const NumOnMeters,                     // Number of meters this variable is "on".
                           const Array1D_int &OnMeters,                // Which meters this variable is on (index values)
                           int const NumOnCustomMeters,               // Number of custom meters this variable is "on".
                           const Array1D_int &OnCustomMeters // Which custom meters this variable is on (index values)
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda Lawrie
        //       DATE WRITTEN   January 2001
        //       MODIFIED       Jason DeGraw 2/12/2020, de-optionalized
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine updates all the meter values in the lists with the current
        // time step value for this variable.

        // METHODOLOGY EMPLOYED:
        // Variables, as they are "setup", may or may not be on one or more meters.
        // All "metered" variables are on the "facility meter".  Index values will be
        // set from the variables to the appropriate meters.  Then, the updating of
        // the meter values is quite simple -- just add the time step value of the variable
        // (which is passed to this routine) to all the values being kept for the meter.
        // Reporting of the meters is taken care of in a different routine.  During reporting,
        // some values will also be reset (for example, after reporting the "hour", the new
        // "hour" value of the meter is reset to 0.0, etc.

        // REFERENCES:
        // na

        // USE STATEMENTS:
        // na

        // Argument array dimensioning

        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:

        // SUBROUTINE PARAMETER DEFINITIONS:
        // na

        // INTERFACE BLOCK SPECIFICATIONS:
        // na

        // DERIVED TYPE DEFINITIONS:
        // na

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int Meter; // Loop Control
        int Which; // Index value for the meter

        for (Meter = 1; Meter <= NumOnMeters; ++Meter) {
            Which = OnMeters(Meter);
            MeterValue(Which) += TimeStepValue;
        }

        // This calculates the basic values for decrement/difference meters -- UpdateMeters then calculates the actual.
        for (Meter = 1; Meter <= NumOnCustomMeters; ++Meter) {
            Which = OnCustomMeters(Meter);
            MeterValue(Which) += TimeStepValue;
        }
    }

    void UpdateMeters(int const TimeStamp) // Current TimeStamp (for max/min)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda Lawrie
        //       DATE WRITTEN   April 2001
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine updates the meters with the current time step value
        // for each meter.  Also, sets min/max values for hourly...run period reporting.

        // METHODOLOGY EMPLOYED:
        // Goes thru the number of meters, setting min/max as appropriate.  Uses timestamp
        // from calling program.

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
        int Meter; // Loop Control

        for (Meter = 1; Meter <= NumEnergyMeters; ++Meter) {
            if (EnergyMeters(Meter).TypeOfMeter != MeterType_CustomDec && EnergyMeters(Meter).TypeOfMeter != MeterType_CustomDiff) {
                EnergyMeters(Meter).TSValue += MeterValue(Meter);
                EnergyMeters(Meter).HRValue += MeterValue(Meter);
                SetMinMax(EnergyMeters(Meter).TSValue,
                          TimeStamp,
                          EnergyMeters(Meter).HRMaxVal,
                          EnergyMeters(Meter).HRMaxValDate,
                          EnergyMeters(Meter).HRMinVal,
                          EnergyMeters(Meter).HRMinValDate);
                EnergyMeters(Meter).DYValue += MeterValue(Meter);
                SetMinMax(EnergyMeters(Meter).TSValue,
                          TimeStamp,
                          EnergyMeters(Meter).DYMaxVal,
                          EnergyMeters(Meter).DYMaxValDate,
                          EnergyMeters(Meter).DYMinVal,
                          EnergyMeters(Meter).DYMinValDate);
                EnergyMeters(Meter).MNValue += MeterValue(Meter);
                SetMinMax(EnergyMeters(Meter).TSValue,
                          TimeStamp,
                          EnergyMeters(Meter).MNMaxVal,
                          EnergyMeters(Meter).MNMaxValDate,
                          EnergyMeters(Meter).MNMinVal,
                          EnergyMeters(Meter).MNMinValDate);
                EnergyMeters(Meter).YRValue += MeterValue(Meter);
                SetMinMax(EnergyMeters(Meter).TSValue,
                          TimeStamp,
                          EnergyMeters(Meter).YRMaxVal,
                          EnergyMeters(Meter).YRMaxValDate,
                          EnergyMeters(Meter).YRMinVal,
                          EnergyMeters(Meter).YRMinValDate);
                EnergyMeters(Meter).SMValue += MeterValue(Meter);
                SetMinMax(EnergyMeters(Meter).TSValue,
                          TimeStamp,
                          EnergyMeters(Meter).SMMaxVal,
                          EnergyMeters(Meter).SMMaxValDate,
                          EnergyMeters(Meter).SMMinVal,
                          EnergyMeters(Meter).SMMinValDate);
                if (isFinalYear) {
                    EnergyMeters(Meter).FinYrSMValue += MeterValue(Meter);
                    SetMinMax(EnergyMeters(Meter).TSValue,
                              TimeStamp,
                              EnergyMeters(Meter).FinYrSMMaxVal,
                              EnergyMeters(Meter).FinYrSMMaxValDate,
                              EnergyMeters(Meter).FinYrSMMinVal,
                              EnergyMeters(Meter).FinYrSMMinValDate);
                }
            } else {
                EnergyMeters(Meter).TSValue = EnergyMeters(EnergyMeters(Meter).SourceMeter).TSValue - MeterValue(Meter);
                EnergyMeters(Meter).HRValue += EnergyMeters(Meter).TSValue;
                SetMinMax(EnergyMeters(Meter).TSValue,
                          TimeStamp,
                          EnergyMeters(Meter).HRMaxVal,
                          EnergyMeters(Meter).HRMaxValDate,
                          EnergyMeters(Meter).HRMinVal,
                          EnergyMeters(Meter).HRMinValDate);
                EnergyMeters(Meter).DYValue += EnergyMeters(Meter).TSValue;
                SetMinMax(EnergyMeters(Meter).TSValue,
                          TimeStamp,
                          EnergyMeters(Meter).DYMaxVal,
                          EnergyMeters(Meter).DYMaxValDate,
                          EnergyMeters(Meter).DYMinVal,
                          EnergyMeters(Meter).DYMinValDate);
                EnergyMeters(Meter).MNValue += EnergyMeters(Meter).TSValue;
                SetMinMax(EnergyMeters(Meter).TSValue,
                          TimeStamp,
                          EnergyMeters(Meter).MNMaxVal,
                          EnergyMeters(Meter).MNMaxValDate,
                          EnergyMeters(Meter).MNMinVal,
                          EnergyMeters(Meter).MNMinValDate);
                EnergyMeters(Meter).YRValue += EnergyMeters(Meter).TSValue;
                SetMinMax(EnergyMeters(Meter).TSValue,
                          TimeStamp,
                          EnergyMeters(Meter).YRMaxVal,
                          EnergyMeters(Meter).YRMaxValDate,
                          EnergyMeters(Meter).YRMinVal,
                          EnergyMeters(Meter).YRMinValDate);
                EnergyMeters(Meter).SMValue += EnergyMeters(Meter).TSValue;
                SetMinMax(EnergyMeters(Meter).TSValue,
                          TimeStamp,
                          EnergyMeters(Meter).SMMaxVal,
                          EnergyMeters(Meter).SMMaxValDate,
                          EnergyMeters(Meter).SMMinVal,
                          EnergyMeters(Meter).SMMinValDate);
                if (isFinalYear) {
                    EnergyMeters(Meter).FinYrSMValue += EnergyMeters(Meter).TSValue;
                    SetMinMax(EnergyMeters(Meter).TSValue,
                              TimeStamp,
                              EnergyMeters(Meter).FinYrSMMaxVal,
                              EnergyMeters(Meter).FinYrSMMaxValDate,
                              EnergyMeters(Meter).FinYrSMMinVal,
                              EnergyMeters(Meter).FinYrSMMinValDate);
                }
            }
        }

        MeterValue = 0.0; // Ready for next update
    }

    void ResetAccumulationWhenWarmupComplete()
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Jason Glazer
        //       DATE WRITTEN   June 2015
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Resets the accumulating meter values. Needed after warmup period is over to
        // reset the totals on meters so that they are not accumulated over the warmup period

        // METHODOLOGY EMPLOYED:
        // Cycle through the meters and reset all accumulating values

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
        int Meter; // Loop Control
        int Loop;  // Loop Variable

        for (Meter = 1; Meter <= NumEnergyMeters; ++Meter) {
            EnergyMeters(Meter).HRValue = 0.0;
            EnergyMeters(Meter).HRMaxVal = MaxSetValue;
            EnergyMeters(Meter).HRMaxValDate = 0;
            EnergyMeters(Meter).HRMinVal = MinSetValue;
            EnergyMeters(Meter).HRMinValDate = 0;

            EnergyMeters(Meter).DYValue = 0.0;
            EnergyMeters(Meter).DYMaxVal = MaxSetValue;
            EnergyMeters(Meter).DYMaxValDate = 0;
            EnergyMeters(Meter).DYMinVal = MinSetValue;
            EnergyMeters(Meter).DYMinValDate = 0;

            EnergyMeters(Meter).MNValue = 0.0;
            EnergyMeters(Meter).MNMaxVal = MaxSetValue;
            EnergyMeters(Meter).MNMaxValDate = 0;
            EnergyMeters(Meter).MNMinVal = MinSetValue;
            EnergyMeters(Meter).MNMinValDate = 0;

            EnergyMeters(Meter).YRValue = 0.0;
            EnergyMeters(Meter).YRMaxVal = MaxSetValue;
            EnergyMeters(Meter).YRMaxValDate = 0;
            EnergyMeters(Meter).YRMinVal = MinSetValue;
            EnergyMeters(Meter).YRMinValDate = 0;

            EnergyMeters(Meter).SMValue = 0.0;
            EnergyMeters(Meter).SMMaxVal = MaxSetValue;
            EnergyMeters(Meter).SMMaxValDate = 0;
            EnergyMeters(Meter).SMMinVal = MinSetValue;
            EnergyMeters(Meter).SMMinValDate = 0;

            EnergyMeters(Meter).FinYrSMValue = 0.0;
            EnergyMeters(Meter).FinYrSMMaxVal = MaxSetValue;
            EnergyMeters(Meter).FinYrSMMaxValDate = 0;
            EnergyMeters(Meter).FinYrSMMinVal = MinSetValue;
            EnergyMeters(Meter).FinYrSMMinValDate = 0;
        }

        for (Loop = 1; Loop <= NumOfRVariable; ++Loop) {
            auto &rVar(RVariableTypes(Loop).VarPtr);
            if (rVar.frequency == ReportingFrequency::Monthly || rVar.frequency == ReportingFrequency::Yearly ||
                rVar.frequency == ReportingFrequency::Simulation) {
                rVar.StoreValue = 0.0;
                rVar.NumStored = 0;
            }
        }

        for (Loop = 1; Loop <= NumOfIVariable; ++Loop) {
            auto &iVar(IVariableTypes(Loop).VarPtr);
            if (iVar.frequency == ReportingFrequency::Monthly || iVar.frequency == ReportingFrequency::Yearly ||
                iVar.frequency == ReportingFrequency::Simulation) {
                iVar.StoreValue = 0;
                iVar.NumStored = 0;
            }
        }
    }

    void SetMinMax(Real64 const TestValue, // Candidate new value
                   int const TimeStamp,    // TimeStamp to be stored if applicable
                   Real64 &CurMaxValue,    // Current Maximum Value
                   int &CurMaxValDate,     // Current Maximum Value Date Stamp
                   Real64 &CurMinValue,    // Current Minimum Value
                   int &CurMinValDate      // Current Minimum Value Date Stamp
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda Lawrie
        //       DATE WRITTEN   January 2001
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine looks at the current value, comparing against the current max and
        // min for this meter/variable and resets along with a timestamp if applicable.

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

        // DERIVED TYPE DEFINITIONS:
        // na

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        // na

        if (TestValue > CurMaxValue) {
            CurMaxValue = TestValue;
            CurMaxValDate = TimeStamp;
        }
        if (TestValue < CurMinValue) {
            CurMinValue = TestValue;
            CurMinValDate = TimeStamp;
        }
    }

    void ReportTSMeters(EnergyPlusData &state,
                        Real64 const StartMinute, // Start Minute for TimeStep
                        Real64 const EndMinute,   // End Minute for TimeStep
                        bool &PrintESOTimeStamp,  // True if the ESO Time Stamp also needs to be printed
                        bool PrintTimeStampToSQL  // Print Time Stamp to SQL file
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda Lawrie
        //       DATE WRITTEN   January 2001
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine reports on the meters that have been requested for
        // reporting on each time step.

        // METHODOLOGY EMPLOYED:
        // na

        // REFERENCES:
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
        int Loop; // Loop Control
        bool PrintTimeStamp;
        int CurDayType;
        static Real64 rDummy1(0.0);
        static Real64 rDummy2(0.0);
        static int iDummy1(0);
        static int iDummy2(0);

        if (!ResultsFramework::resultsFramework->TSMeters.rDataFrameEnabled()) {
            ResultsFramework::resultsFramework->initializeMeters(EnergyMeters, ReportingFrequency::TimeStep);
        }

        PrintTimeStamp = true;
        for (Loop = 1; Loop <= NumEnergyMeters; ++Loop) {
            EnergyMeters(Loop).CurTSValue = EnergyMeters(Loop).TSValue;
            if (!EnergyMeters(Loop).RptTS && !EnergyMeters(Loop).RptAccTS) continue;
            if (PrintTimeStamp) {
                CurDayType = DayOfWeek;
                if (HolidayIndex > 0) {
                    CurDayType = 7 + HolidayIndex;
                }
                WriteTimeStampFormatData(state.files.mtr,
                                         ReportingFrequency::EachCall,
                                         TimeStepStampReportNbr,
                                         TimeStepStampReportChr,
                                         DayOfSim,
                                         state.dataGlobal->DayOfSimChr,
                                         PrintTimeStamp && PrintTimeStampToSQL,
                                         Month,
                                         DayOfMonth,
                                         HourOfDay,
                                         EndMinute,
                                         StartMinute,
                                         DSTIndicator,
                                         DayTypes(CurDayType));
                if (ResultsFramework::resultsFramework->TSMeters.rDataFrameEnabled()) {
                    ResultsFramework::resultsFramework->TSMeters.newRow(Month, DayOfMonth, HourOfDay, EndMinute);
                }
                PrintTimeStamp = false;
                PrintTimeStampToSQL = false;
            }

            if (PrintESOTimeStamp && !EnergyMeters(Loop).RptTSFO && !EnergyMeters(Loop).RptAccTSFO) {
                CurDayType = DayOfWeek;
                if (HolidayIndex > 0) {
                    CurDayType = 7 + HolidayIndex;
                }
                WriteTimeStampFormatData(state.files.eso,
                                         ReportingFrequency::EachCall,
                                         TimeStepStampReportNbr,
                                         TimeStepStampReportChr,
                                         DayOfSim,
                                         state.dataGlobal->DayOfSimChr,
                                         PrintTimeStamp && PrintESOTimeStamp && PrintTimeStampToSQL,
                                         Month,
                                         DayOfMonth,
                                         HourOfDay,
                                         EndMinute,
                                         StartMinute,
                                         DSTIndicator,
                                         DayTypes(CurDayType));
                PrintESOTimeStamp = false;
            }

            if (EnergyMeters(Loop).RptTS) {
                WriteReportMeterData(state,
                                     EnergyMeters(Loop).TSRptNum,
                                     EnergyMeters(Loop).TSRptNumChr,
                                     EnergyMeters(Loop).TSValue,
                                     ReportingFrequency::TimeStep,
                                     rDummy1,
                                     iDummy1,
                                     rDummy2,
                                     iDummy2,
                                     EnergyMeters(Loop).RptTSFO);
                ResultsFramework::resultsFramework->TSMeters.pushVariableValue(EnergyMeters(Loop).TSRptNum, EnergyMeters(Loop).TSValue);
            }

            if (EnergyMeters(Loop).RptAccTS) {
                WriteCumulativeReportMeterData(state,
                                               EnergyMeters(Loop).TSAccRptNum,
                                               fmt::to_string(EnergyMeters(Loop).TSAccRptNum),
                                               EnergyMeters(Loop).SMValue,
                                               EnergyMeters(Loop).RptAccTSFO);
                ResultsFramework::resultsFramework->TSMeters.pushVariableValue(EnergyMeters(Loop).TSAccRptNum, EnergyMeters(Loop).SMValue);
            }
        }

        if (NumEnergyMeters > 0) {
            for (auto &e : EnergyMeters)
                e.TSValue = 0.0;
        }
    }

    void ReportHRMeters(EnergyPlusData &state, bool PrintTimeStampToSQL // Print Time Stamp to SQL file
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda Lawrie
        //       DATE WRITTEN   January 2001
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine reports on the meters that have been requested for
        // reporting on each hour.

        // METHODOLOGY EMPLOYED:
        // na

        // REFERENCES:
        // na

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
        int Loop; // Loop Control
        bool PrintTimeStamp;
        int CurDayType;
        static Real64 rDummy1(0.0);
        static Real64 rDummy2(0.0);
        static int iDummy1(0);
        static int iDummy2(0);

        if (!ResultsFramework::resultsFramework->HRMeters.rDataFrameEnabled()) {
            ResultsFramework::resultsFramework->initializeMeters(EnergyMeters, ReportingFrequency::Hourly);
        }

        PrintTimeStamp = true;
        for (Loop = 1; Loop <= NumEnergyMeters; ++Loop) {
            if (!EnergyMeters(Loop).RptHR && !EnergyMeters(Loop).RptAccHR) continue;
            if (PrintTimeStamp) {
                CurDayType = DayOfWeek;
                if (HolidayIndex > 0) {
                    CurDayType = 7 + HolidayIndex;
                }
                WriteTimeStampFormatData(state.files.mtr,
                                         ReportingFrequency::Hourly,
                                         TimeStepStampReportNbr,
                                         TimeStepStampReportChr,
                                         DayOfSim,
                                         state.dataGlobal->DayOfSimChr,
                                         PrintTimeStamp && PrintTimeStampToSQL,
                                         Month,
                                         DayOfMonth,
                                         HourOfDay,
                                         _,
                                         _,
                                         DSTIndicator,
                                         DayTypes(CurDayType));
                if (ResultsFramework::resultsFramework->HRMeters.rDataFrameEnabled()) {
                    ResultsFramework::resultsFramework->HRMeters.newRow(Month, DayOfMonth, HourOfDay, 0);
                }
                PrintTimeStamp = false;
                PrintTimeStampToSQL = false;
            }

            if (EnergyMeters(Loop).RptHR) {
                WriteReportMeterData(state,
                                     EnergyMeters(Loop).HRRptNum,
                                     EnergyMeters(Loop).HRRptNumChr,
                                     EnergyMeters(Loop).HRValue,
                                     ReportingFrequency::Hourly,
                                     rDummy1,
                                     iDummy1,
                                     rDummy2,
                                     iDummy2,
                                     EnergyMeters(Loop).RptHRFO); // EnergyMeters(Loop)%HRMinVal, EnergyMeters(Loop)%HRMinValDate, & |
                                                                  // EnergyMeters(Loop)%HRMaxVal, EnergyMeters(Loop)%HRMaxValDate, &
                ResultsFramework::resultsFramework->HRMeters.pushVariableValue(EnergyMeters(Loop).HRRptNum, EnergyMeters(Loop).HRValue);
                EnergyMeters(Loop).HRValue = 0.0;
                EnergyMeters(Loop).HRMinVal = MinSetValue;
                EnergyMeters(Loop).HRMaxVal = MaxSetValue;
            }

            if (EnergyMeters(Loop).RptAccHR) {
                WriteCumulativeReportMeterData(state,
                                               EnergyMeters(Loop).HRAccRptNum,
                                               fmt::to_string(EnergyMeters(Loop).HRAccRptNum),
                                               EnergyMeters(Loop).SMValue,
                                               EnergyMeters(Loop).RptAccHRFO);
                ResultsFramework::resultsFramework->HRMeters.pushVariableValue(EnergyMeters(Loop).HRAccRptNum, EnergyMeters(Loop).SMValue);
            }
        }
    }

    void ReportDYMeters(EnergyPlusData &state, bool PrintTimeStampToSQL // Print Time Stamp to SQL file
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda Lawrie
        //       DATE WRITTEN   January 2001
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine reports on the meters that have been requested for
        // reporting on each day.

        // METHODOLOGY EMPLOYED:
        // na

        // REFERENCES:
        // na

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
        int Loop; // Loop Control
        bool PrintTimeStamp;
        int CurDayType;

        if (!ResultsFramework::resultsFramework->DYMeters.rVariablesScanned()) {
            ResultsFramework::resultsFramework->initializeMeters(EnergyMeters, ReportingFrequency::Daily);
        }

        PrintTimeStamp = true;
        for (Loop = 1; Loop <= NumEnergyMeters; ++Loop) {
            if (!EnergyMeters(Loop).RptDY && !EnergyMeters(Loop).RptAccDY) continue;
            if (PrintTimeStamp) {
                CurDayType = DayOfWeek;
                if (HolidayIndex > 0) {
                    CurDayType = 7 + HolidayIndex;
                }
                WriteTimeStampFormatData(state.files.mtr,
                                         ReportingFrequency::Daily,
                                         DailyStampReportNbr,
                                         DailyStampReportChr,
                                         DayOfSim,
                                         state.dataGlobal->DayOfSimChr,
                                         PrintTimeStamp && PrintTimeStampToSQL,
                                         Month,
                                         DayOfMonth,
                                         _,
                                         _,
                                         _,
                                         DSTIndicator,
                                         DayTypes(CurDayType));
                if (ResultsFramework::resultsFramework->DYMeters.rDataFrameEnabled()) {
                    ResultsFramework::resultsFramework->DYMeters.newRow(Month, DayOfMonth, HourOfDay, 0);
                }
                PrintTimeStamp = false;
                PrintTimeStampToSQL = false;
            }

            if (EnergyMeters(Loop).RptDY) {
                WriteReportMeterData(state,
                                     EnergyMeters(Loop).DYRptNum,
                                     EnergyMeters(Loop).DYRptNumChr,
                                     EnergyMeters(Loop).DYValue,
                                     ReportingFrequency::Daily,
                                     EnergyMeters(Loop).DYMinVal,
                                     EnergyMeters(Loop).DYMinValDate,
                                     EnergyMeters(Loop).DYMaxVal,
                                     EnergyMeters(Loop).DYMaxValDate,
                                     EnergyMeters(Loop).RptDYFO);
                ResultsFramework::resultsFramework->DYMeters.pushVariableValue(EnergyMeters(Loop).DYRptNum, EnergyMeters(Loop).DYValue);
                EnergyMeters(Loop).DYValue = 0.0;
                EnergyMeters(Loop).DYMinVal = MinSetValue;
                EnergyMeters(Loop).DYMaxVal = MaxSetValue;
            }

            if (EnergyMeters(Loop).RptAccDY) {
                WriteCumulativeReportMeterData(state,
                                               EnergyMeters(Loop).DYAccRptNum,
                                               fmt::to_string(EnergyMeters(Loop).DYAccRptNum),
                                               EnergyMeters(Loop).SMValue,
                                               EnergyMeters(Loop).RptAccDYFO);
                ResultsFramework::resultsFramework->DYMeters.pushVariableValue(EnergyMeters(Loop).DYAccRptNum, EnergyMeters(Loop).SMValue);
            }
        }
    }

    void ReportMNMeters(EnergyPlusData &state, bool PrintTimeStampToSQL // Print Time Stamp to SQL file
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda Lawrie
        //       DATE WRITTEN   January 2001
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine reports on the meters that have been requested for
        // reporting on each month.

        // METHODOLOGY EMPLOYED:
        // na

        // REFERENCES:
        // na


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
        int Loop; // Loop Control
        bool PrintTimeStamp;

        if (!ResultsFramework::resultsFramework->MNMeters.rVariablesScanned()) {
            ResultsFramework::resultsFramework->initializeMeters(EnergyMeters, ReportingFrequency::Monthly);
        }

        PrintTimeStamp = true;
        for (Loop = 1; Loop <= NumEnergyMeters; ++Loop) {
            if (!EnergyMeters(Loop).RptMN && !EnergyMeters(Loop).RptAccMN) continue;
            if (PrintTimeStamp) {
                WriteTimeStampFormatData(state.files.mtr,
                                         ReportingFrequency::Monthly,
                                         MonthlyStampReportNbr,
                                         MonthlyStampReportChr,
                                         DayOfSim,
                                         state.dataGlobal->DayOfSimChr,
                                         PrintTimeStamp && PrintTimeStampToSQL,
                                         Month);
                if (ResultsFramework::resultsFramework->MNMeters.rDataFrameEnabled()) {
                    ResultsFramework::resultsFramework->MNMeters.newRow(Month, DayOfMonth, HourOfDay, 0);
                }
                PrintTimeStamp = false;
                PrintTimeStampToSQL = false;
            }

            if (EnergyMeters(Loop).RptMN) {
                WriteReportMeterData(state,
                                     EnergyMeters(Loop).MNRptNum,
                                     EnergyMeters(Loop).MNRptNumChr,
                                     EnergyMeters(Loop).MNValue,
                                     ReportingFrequency::Monthly,
                                     EnergyMeters(Loop).MNMinVal,
                                     EnergyMeters(Loop).MNMinValDate,
                                     EnergyMeters(Loop).MNMaxVal,
                                     EnergyMeters(Loop).MNMaxValDate,
                                     EnergyMeters(Loop).RptMNFO);
                ResultsFramework::resultsFramework->MNMeters.pushVariableValue(EnergyMeters(Loop).MNRptNum, EnergyMeters(Loop).MNValue);
                EnergyMeters(Loop).MNValue = 0.0;
                EnergyMeters(Loop).MNMinVal = MinSetValue;
                EnergyMeters(Loop).MNMaxVal = MaxSetValue;
            }

            if (EnergyMeters(Loop).RptAccMN) {
                WriteCumulativeReportMeterData(state,
                                               EnergyMeters(Loop).MNAccRptNum,
                                               fmt::to_string(EnergyMeters(Loop).MNAccRptNum),
                                               EnergyMeters(Loop).SMValue,
                                               EnergyMeters(Loop).RptAccMNFO);
                ResultsFramework::resultsFramework->MNMeters.pushVariableValue(EnergyMeters(Loop).MNAccRptNum, EnergyMeters(Loop).SMValue);
            }
        }
    }

    void ReportYRMeters(EnergyPlusData &state, bool PrintTimeStampToSQL)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Jason DeGraw
        //       DATE WRITTEN   January 2018
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine reports on the meters that have been requested for
        // reporting on each year.

        // METHODOLOGY EMPLOYED:
        // na

        // REFERENCES:
        // na

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
        int Loop; // Loop Control
        bool PrintTimeStamp;

        if (!ResultsFramework::resultsFramework->YRMeters.rVariablesScanned()) {
            ResultsFramework::resultsFramework->initializeMeters(EnergyMeters, ReportingFrequency::Yearly);
        }

        PrintTimeStamp = true;
        for (Loop = 1; Loop <= NumEnergyMeters; ++Loop) {
            if (!EnergyMeters(Loop).RptYR && !EnergyMeters(Loop).RptAccYR) continue;
            if (PrintTimeStamp) {
                WriteYearlyTimeStamp(
                    state.files.mtr, YearlyStampReportChr, DataGlobals::CalendarYearChr, PrintTimeStamp && PrintTimeStampToSQL);
                if (ResultsFramework::resultsFramework->YRMeters.rDataFrameEnabled()) {
                    ResultsFramework::resultsFramework->YRMeters.newRow(Month, DayOfMonth, HourOfDay, 0);
                }
                PrintTimeStamp = false;
                PrintTimeStampToSQL = false;
            }

            if (EnergyMeters(Loop).RptYR) {
                WriteReportMeterData(state,
                                     EnergyMeters(Loop).YRRptNum,
                                     EnergyMeters(Loop).YRRptNumChr,
                                     EnergyMeters(Loop).YRValue,
                                     ReportingFrequency::Yearly,
                                     EnergyMeters(Loop).YRMinVal,
                                     EnergyMeters(Loop).YRMinValDate,
                                     EnergyMeters(Loop).YRMaxVal,
                                     EnergyMeters(Loop).YRMaxValDate,
                                     EnergyMeters(Loop).RptYRFO);
                ResultsFramework::resultsFramework->YRMeters.pushVariableValue(EnergyMeters(Loop).YRRptNum, EnergyMeters(Loop).YRValue);
                EnergyMeters(Loop).YRValue = 0.0;
                EnergyMeters(Loop).YRMinVal = MinSetValue;
                EnergyMeters(Loop).YRMaxVal = MaxSetValue;
            }

            if (EnergyMeters(Loop).RptAccYR) {
                WriteCumulativeReportMeterData(state,
                                               EnergyMeters(Loop).YRAccRptNum,
                                               fmt::to_string(EnergyMeters(Loop).YRAccRptNum),
                                               EnergyMeters(Loop).YRValue,
                                               EnergyMeters(Loop).RptAccYRFO);
                ResultsFramework::resultsFramework->YRMeters.pushVariableValue(EnergyMeters(Loop).YRAccRptNum, EnergyMeters(Loop).SMValue);
            }
        }
    }

    void ReportSMMeters(EnergyPlusData &state, bool PrintTimeStampToSQL // Print Time Stamp to SQL file
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda Lawrie
        //       DATE WRITTEN   January 2001
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine reports on the meters that have been requested for
        // reporting on each environment/run period.

        // METHODOLOGY EMPLOYED:
        // na

        // REFERENCES:
        // na

        // Using/Aliasing
        // using namespace OutputReportPredefined;

        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:
        // na

        // SUBROUTINE PARAMETER DEFINITIONS:

        // INTERFACE BLOCK SPECIFICATIONS:
        // na

        // DERIVED TYPE DEFINITIONS:
        // na

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int Loop; // Loop Control
        bool PrintTimeStamp;

        if (!ResultsFramework::resultsFramework->SMMeters.rVariablesScanned()) {
            ResultsFramework::resultsFramework->initializeMeters(EnergyMeters, ReportingFrequency::Simulation);
        }

        PrintTimeStamp = true;
        for (Loop = 1; Loop <= NumEnergyMeters; ++Loop) {
            EnergyMeters(Loop).LastSMValue = EnergyMeters(Loop).SMValue;
            EnergyMeters(Loop).LastSMMinVal = EnergyMeters(Loop).SMMinVal;
            EnergyMeters(Loop).LastSMMinValDate = EnergyMeters(Loop).SMMinValDate;
            EnergyMeters(Loop).LastSMMaxVal = EnergyMeters(Loop).SMMaxVal;
            EnergyMeters(Loop).LastSMMaxValDate = EnergyMeters(Loop).SMMaxValDate;
            if (!EnergyMeters(Loop).RptSM && !EnergyMeters(Loop).RptAccSM) continue;
            if (PrintTimeStamp) {
                WriteTimeStampFormatData(state.files.mtr,
                                         ReportingFrequency::Simulation,
                                         RunPeriodStampReportNbr,
                                         RunPeriodStampReportChr,
                                         DayOfSim,
                                         state.dataGlobal->DayOfSimChr,
                                         PrintTimeStamp && PrintTimeStampToSQL);
                if (ResultsFramework::resultsFramework->SMMeters.rDataFrameEnabled()) {
                    ResultsFramework::resultsFramework->SMMeters.newRow(Month, DayOfMonth, HourOfDay, 0);
                }
                PrintTimeStamp = false;
                PrintTimeStampToSQL = false;
            }

            if (EnergyMeters(Loop).RptSM) {
                WriteReportMeterData(state,
                                     EnergyMeters(Loop).SMRptNum,
                                     EnergyMeters(Loop).SMRptNumChr,
                                     EnergyMeters(Loop).SMValue,
                                     ReportingFrequency::Simulation,
                                     EnergyMeters(Loop).SMMinVal,
                                     EnergyMeters(Loop).SMMinValDate,
                                     EnergyMeters(Loop).SMMaxVal,
                                     EnergyMeters(Loop).SMMaxValDate,
                                     EnergyMeters(Loop).RptSMFO);
                ResultsFramework::resultsFramework->SMMeters.pushVariableValue(EnergyMeters(Loop).SMRptNum, EnergyMeters(Loop).SMValue);
            }

            if (EnergyMeters(Loop).RptAccSM) {
                WriteCumulativeReportMeterData(state,
                                               EnergyMeters(Loop).SMAccRptNum,
                                               fmt::to_string(EnergyMeters(Loop).SMAccRptNum),
                                               EnergyMeters(Loop).SMValue,
                                               EnergyMeters(Loop).RptAccSMFO);
                ResultsFramework::resultsFramework->SMMeters.pushVariableValue(EnergyMeters(Loop).SMAccRptNum, EnergyMeters(Loop).SMValue);
            }
        }

        if (NumEnergyMeters > 0) {
            for (auto &e : EnergyMeters) {
                e.SMValue = 0.0;
                e.SMMinVal = MinSetValue;
                e.SMMaxVal = MaxSetValue;
            }
        }
    }

    void ReportForTabularReports()
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda Lawrie
        //       DATE WRITTEN   August 2013
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine is called after all the simulation is done and before
        // tabular reports in order to reduce the number of calls to the predefined routine
        // for SM (Simulation period) meters, the value of the last calculation is stored
        // in the data structure.

        // METHODOLOGY EMPLOYED:
        // na

        // REFERENCES:
        // na

        // Using/Aliasing
        using namespace OutputReportPredefined;

        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:
        // na

        // SUBROUTINE PARAMETER DEFINITIONS:
        Real64 const convertJtoGJ(1.0 / 1000000000.0);

        // INTERFACE BLOCK SPECIFICATIONS:
        // na

        // DERIVED TYPE DEFINITIONS:
        // na

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int Loop; // Loop Control

        for (Loop = 1; Loop <= NumEnergyMeters; ++Loop) {
            int const RT_forIPUnits(EnergyMeters(Loop).RT_forIPUnits);
            if (RT_forIPUnits == RT_IPUnits_Electricity) {
                PreDefTableEntry(pdchEMelecannual, EnergyMeters(Loop).Name, EnergyMeters(Loop).FinYrSMValue * convertJtoGJ);
                PreDefTableEntry(pdchEMelecminvalue, EnergyMeters(Loop).Name, EnergyMeters(Loop).FinYrSMMinVal / TimeStepZoneSec);
                PreDefTableEntry(pdchEMelecminvaluetime, EnergyMeters(Loop).Name, DateToStringWithMonth(EnergyMeters(Loop).FinYrSMMinValDate));
                PreDefTableEntry(pdchEMelecmaxvalue, EnergyMeters(Loop).Name, EnergyMeters(Loop).FinYrSMMaxVal / TimeStepZoneSec);
                PreDefTableEntry(pdchEMelecmaxvaluetime, EnergyMeters(Loop).Name, DateToStringWithMonth(EnergyMeters(Loop).FinYrSMMaxValDate));
            } else if (RT_forIPUnits == RT_IPUnits_Gas) {
                PreDefTableEntry(pdchEMgasannual, EnergyMeters(Loop).Name, EnergyMeters(Loop).FinYrSMValue * convertJtoGJ);
                PreDefTableEntry(pdchEMgasminvalue, EnergyMeters(Loop).Name, EnergyMeters(Loop).FinYrSMMinVal / TimeStepZoneSec);
                PreDefTableEntry(pdchEMgasminvaluetime, EnergyMeters(Loop).Name, DateToStringWithMonth(EnergyMeters(Loop).FinYrSMMinValDate));
                PreDefTableEntry(pdchEMgasmaxvalue, EnergyMeters(Loop).Name, EnergyMeters(Loop).FinYrSMMaxVal / TimeStepZoneSec);
                PreDefTableEntry(pdchEMgasmaxvaluetime, EnergyMeters(Loop).Name, DateToStringWithMonth(EnergyMeters(Loop).FinYrSMMaxValDate));
            } else if (RT_forIPUnits == RT_IPUnits_Cooling) {
                PreDefTableEntry(pdchEMcoolannual, EnergyMeters(Loop).Name, EnergyMeters(Loop).FinYrSMValue * convertJtoGJ);
                PreDefTableEntry(pdchEMcoolminvalue, EnergyMeters(Loop).Name, EnergyMeters(Loop).FinYrSMMinVal / TimeStepZoneSec);
                PreDefTableEntry(pdchEMcoolminvaluetime, EnergyMeters(Loop).Name, DateToStringWithMonth(EnergyMeters(Loop).FinYrSMMinValDate));
                PreDefTableEntry(pdchEMcoolmaxvalue, EnergyMeters(Loop).Name, EnergyMeters(Loop).FinYrSMMaxVal / TimeStepZoneSec);
                PreDefTableEntry(pdchEMcoolmaxvaluetime, EnergyMeters(Loop).Name, DateToStringWithMonth(EnergyMeters(Loop).FinYrSMMaxValDate));
            } else if (RT_forIPUnits == RT_IPUnits_Water) {
                PreDefTableEntry(pdchEMwaterannual, EnergyMeters(Loop).Name, EnergyMeters(Loop).FinYrSMValue);
                PreDefTableEntry(pdchEMwaterminvalue, EnergyMeters(Loop).Name, EnergyMeters(Loop).FinYrSMMinVal / TimeStepZoneSec);
                PreDefTableEntry(pdchEMwaterminvaluetime, EnergyMeters(Loop).Name, DateToStringWithMonth(EnergyMeters(Loop).FinYrSMMinValDate));
                PreDefTableEntry(pdchEMwatermaxvalue, EnergyMeters(Loop).Name, EnergyMeters(Loop).FinYrSMMaxVal / TimeStepZoneSec);
                PreDefTableEntry(pdchEMwatermaxvaluetime, EnergyMeters(Loop).Name, DateToStringWithMonth(EnergyMeters(Loop).FinYrSMMaxValDate));
            } else if (RT_forIPUnits == RT_IPUnits_OtherKG) {
                PreDefTableEntry(pdchEMotherKGannual, EnergyMeters(Loop).Name, EnergyMeters(Loop).FinYrSMValue);
                PreDefTableEntry(pdchEMotherKGminvalue, EnergyMeters(Loop).Name, EnergyMeters(Loop).FinYrSMMinVal / TimeStepZoneSec, 3);
                PreDefTableEntry(pdchEMotherKGminvaluetime, EnergyMeters(Loop).Name, DateToStringWithMonth(EnergyMeters(Loop).FinYrSMMinValDate));
                PreDefTableEntry(pdchEMotherKGmaxvalue, EnergyMeters(Loop).Name, EnergyMeters(Loop).FinYrSMMaxVal / TimeStepZoneSec, 3);
                PreDefTableEntry(pdchEMotherKGmaxvaluetime, EnergyMeters(Loop).Name, DateToStringWithMonth(EnergyMeters(Loop).FinYrSMMaxValDate));
            } else if (RT_forIPUnits == RT_IPUnits_OtherM3) {
                PreDefTableEntry(pdchEMotherM3annual, EnergyMeters(Loop).Name, EnergyMeters(Loop).FinYrSMValue, 3);
                PreDefTableEntry(pdchEMotherM3minvalue, EnergyMeters(Loop).Name, EnergyMeters(Loop).FinYrSMMinVal / TimeStepZoneSec, 3);
                PreDefTableEntry(pdchEMotherM3minvaluetime, EnergyMeters(Loop).Name, DateToStringWithMonth(EnergyMeters(Loop).FinYrSMMinValDate));
                PreDefTableEntry(pdchEMotherM3maxvalue, EnergyMeters(Loop).Name, EnergyMeters(Loop).FinYrSMMaxVal / TimeStepZoneSec, 3);
                PreDefTableEntry(pdchEMotherM3maxvaluetime, EnergyMeters(Loop).Name, DateToStringWithMonth(EnergyMeters(Loop).FinYrSMMaxValDate));
            } else if (RT_forIPUnits == RT_IPUnits_OtherL) {
                PreDefTableEntry(pdchEMotherLannual, EnergyMeters(Loop).Name, EnergyMeters(Loop).FinYrSMValue, 3);
                PreDefTableEntry(pdchEMotherLminvalue, EnergyMeters(Loop).Name, EnergyMeters(Loop).FinYrSMMinVal / TimeStepZoneSec, 3);
                PreDefTableEntry(pdchEMotherLminvaluetime, EnergyMeters(Loop).Name, DateToStringWithMonth(EnergyMeters(Loop).FinYrSMMinValDate));
                PreDefTableEntry(pdchEMotherLmaxvalue, EnergyMeters(Loop).Name, EnergyMeters(Loop).FinYrSMMaxVal / TimeStepZoneSec, 3);
                PreDefTableEntry(pdchEMotherLmaxvaluetime, EnergyMeters(Loop).Name, DateToStringWithMonth(EnergyMeters(Loop).FinYrSMMaxValDate));
            } else {
                PreDefTableEntry(pdchEMotherJannual, EnergyMeters(Loop).Name, EnergyMeters(Loop).FinYrSMValue * convertJtoGJ);
                PreDefTableEntry(pdchEMotherJminvalue, EnergyMeters(Loop).Name, EnergyMeters(Loop).FinYrSMMinVal / TimeStepZoneSec);
                PreDefTableEntry(pdchEMotherJminvaluetime, EnergyMeters(Loop).Name, DateToStringWithMonth(EnergyMeters(Loop).FinYrSMMinValDate));
                PreDefTableEntry(pdchEMotherJmaxvalue, EnergyMeters(Loop).Name, EnergyMeters(Loop).FinYrSMMaxVal / TimeStepZoneSec);
                PreDefTableEntry(pdchEMotherJmaxvaluetime, EnergyMeters(Loop).Name, DateToStringWithMonth(EnergyMeters(Loop).FinYrSMMaxValDate));
            }
        }
    }

    std::string DateToStringWithMonth(int const codedDate) // word containing encoded month, day, hour, minute
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Jason Glazer
        //       DATE WRITTEN   August 2003
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        //   Convert the coded date format into a usable
        //   string

        if (codedDate == 0) return "-";

        static constexpr auto DateFmt("{:02}-{:3}-{:02}:{:02}");

        // ((month*100 + day)*100 + hour)*100 + minute
        int Month;  // month in integer format (1-12)
        int Day;    // day in integer format (1-31)
        int Hour;   // hour in integer format (1-24)
        int Minute; // minute in integer format (0:59)

        General::DecodeMonDayHrMin(codedDate, Month, Day, Hour, Minute);

        if (Month < 1 || Month > 12) return "-";
        if (Day < 1 || Day > 31) return "-";
        if (Hour < 1 || Hour > 24) return "-";
        if (Minute < 0 || Minute > 60) return "-";

        --Hour;
        if (Minute == 60) {
            ++Hour;
            Minute = 0;
        }

        std::string monthName;
        switch (Month) {
        case 1:
            monthName = "JAN";
            break;
        case 2:
            monthName = "FEB";
            break;
        case 3:
            monthName = "MAR";
            break;
        case 4:
            monthName = "APR";
            break;
        case 5:
            monthName = "MAY";
            break;
        case 6:
            monthName = "JUN";
            break;
        case 7:
            monthName = "JUL";
            break;
        case 8:
            monthName = "AUG";
            break;
        case 9:
            monthName = "SEP";
            break;
        case 10:
            monthName = "OCT";
            break;
        case 11:
            monthName = "NOV";
            break;
        case 12:
            monthName = "DEC";
            break;
        }

        const std::string StringOut = format(DateFmt, Day, monthName, Hour, Minute);
        return StringOut;
    }

    void ReportMeterDetails(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda Lawrie
        //       DATE WRITTEN   January 2006
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Writes the meter details report.  This shows which variables are on
        // meters as well as the meter contents.

        // METHODOLOGY EMPLOYED:
        // na

        // REFERENCES:
        // na

        // USE STATEMENTS:
        // na

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

        for (int VarMeter = 1; VarMeter <= NumVarMeterArrays; ++VarMeter) {

            const std::string mtrUnitString = unitEnumToStringBrackets(RVariableTypes(VarMeterArrays(VarMeter).RepVariable).units);

            std::string Multipliers = "";
            const auto ZoneMult = RVariableTypes(VarMeterArrays(VarMeter).RepVariable).VarPtr.ZoneMult;
            const auto ZoneListMult = RVariableTypes(VarMeterArrays(VarMeter).RepVariable).VarPtr.ZoneListMult;

            if (ZoneMult > 1 || ZoneListMult > 1) {
                Multipliers = format(" * {}  (Zone Multiplier = {}, Zone List Multiplier = {})", ZoneMult * ZoneListMult, ZoneMult, ZoneListMult);
            }

            print(state.files.mtd, "\n Meters for {},{}{}{}\n", RVariableTypes(VarMeterArrays(VarMeter).RepVariable).VarPtr.ReportIDChr,
                       RVariableTypes(VarMeterArrays(VarMeter).RepVariable).VarName, mtrUnitString, Multipliers);

            for (int I = 1; I <= VarMeterArrays(VarMeter).NumOnMeters; ++I) {
                print(state.files.mtd, "  OnMeter={}{}\n", EnergyMeters(VarMeterArrays(VarMeter).OnMeters(I)).Name, mtrUnitString);
            }

            for (int I = 1; I <= VarMeterArrays(VarMeter).NumOnCustomMeters; ++I) {
                print(state.files.mtd, "  OnCustomMeter={}{}\n", EnergyMeters(VarMeterArrays(VarMeter).OnCustomMeters(I)).Name, mtrUnitString);
            }
        }

        for (int Meter = 1; Meter <= NumEnergyMeters; ++Meter) {
            print(state.files.mtd, "\n For Meter={}{}", EnergyMeters(Meter).Name, unitEnumToStringBrackets(EnergyMeters(Meter).Units));
            if (EnergyMeters(Meter).ResourceType != "") {
                print(state.files.mtd, ", ResourceType={}", EnergyMeters(Meter).ResourceType);
            }
            if (EnergyMeters(Meter).EndUse != "") {
                print(state.files.mtd, ", EndUse={}", EnergyMeters(Meter).EndUse);
            }
            if (EnergyMeters(Meter).Group != "") {
                print(state.files.mtd, ", Group={}", EnergyMeters(Meter).Group);
            }
            print(state.files.mtd, ", contents are:\n");

            bool CustDecWritten = false;

            for (int VarMeter = 1; VarMeter <= NumVarMeterArrays; ++VarMeter) {
                if (EnergyMeters(Meter).TypeOfMeter == MeterType_Normal) {
                    if (any_eq(VarMeterArrays(VarMeter).OnMeters, Meter)) {
                        for (int VarMeter1 = 1; VarMeter1 <= VarMeterArrays(VarMeter).NumOnMeters; ++VarMeter1) {
                            if (VarMeterArrays(VarMeter).OnMeters(VarMeter1) != Meter) continue;

                            std::string Multipliers = "";
                            const auto ZoneMult = RVariableTypes(VarMeterArrays(VarMeter).RepVariable).VarPtr.ZoneMult;
                            const auto ZoneListMult = RVariableTypes(VarMeterArrays(VarMeter).RepVariable).VarPtr.ZoneListMult;

                            if (ZoneMult > 1 || ZoneListMult > 1) {
                                Multipliers = format(
                                    " * {}  (Zone Multiplier = {}, Zone List Multiplier = {})", ZoneMult * ZoneListMult, ZoneMult, ZoneListMult);
                            }

                            print(state.files.mtd, "  {}{}\n", RVariableTypes(VarMeterArrays(VarMeter).RepVariable).VarName, Multipliers);
                        }
                    }
                }
                if (EnergyMeters(Meter).TypeOfMeter != MeterType_Normal) {
                    if (VarMeterArrays(VarMeter).NumOnCustomMeters > 0) {
                        if (any_eq(VarMeterArrays(VarMeter).OnCustomMeters, Meter)) {
                            if (!CustDecWritten && EnergyMeters(Meter).TypeOfMeter == MeterType_CustomDec) {
                                print(state.files.mtd,  " Values for this meter will be Source Meter={}; but will be decremented by:\n", EnergyMeters(EnergyMeters(Meter).SourceMeter).Name);
                                CustDecWritten = true;
                            }
                            for (int VarMeter1 = 1; VarMeter1 <= VarMeterArrays(VarMeter).NumOnCustomMeters; ++VarMeter1) {
                                if (VarMeterArrays(VarMeter).OnCustomMeters(VarMeter1) != Meter) continue;

                                std::string Multipliers;
                                const auto ZoneMult = RVariableTypes(VarMeterArrays(VarMeter).RepVariable).VarPtr.ZoneMult;
                                const auto ZoneListMult = RVariableTypes(VarMeterArrays(VarMeter).RepVariable).VarPtr.ZoneListMult;

                                if (ZoneMult > 1 || ZoneListMult > 1) {
                                    Multipliers = format(
                                        " * {}  (Zone Multiplier = {}, Zone List Multiplier = {})", ZoneMult * ZoneListMult, ZoneMult, ZoneListMult);
                                }

                                print(state.files.mtd, "  {}{}\n", RVariableTypes(VarMeterArrays(VarMeter).RepVariable).VarName, Multipliers);
                            }
                        }
                    }
                }
            }
        }
    }

    // *****************************************************************************
    // End of routines for Energy Meters implementation in EnergyPlus.
    // *****************************************************************************

    void AddEndUseSubcategory(std::string const &EP_UNUSED(ResourceName), std::string const &EndUseName, std::string const &EndUseSubName)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Peter Graham Ellis
        //       DATE WRITTEN   February 2006
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine manages the list of subcategories for each end-use category.

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int EndUseNum;
        int EndUseSubNum;
        int NumSubs;

        bool Found = false;
        for (EndUseNum = 1; EndUseNum <= NumEndUses; ++EndUseNum) {
            if (UtilityRoutines::SameString(EndUseCategory(EndUseNum).Name, EndUseName)) {

                for (EndUseSubNum = 1; EndUseSubNum <= EndUseCategory(EndUseNum).NumSubcategories; ++EndUseSubNum) {
                    if (UtilityRoutines::SameString(EndUseCategory(EndUseNum).SubcategoryName(EndUseSubNum), EndUseSubName)) {
                        // Subcategory already exists, no further action required
                        Found = true;
                        break;
                    }
                }

                if (!Found) {
                    // Add the subcategory by reallocating the array
                    NumSubs = EndUseCategory(EndUseNum).NumSubcategories;
                    EndUseCategory(EndUseNum).SubcategoryName.redimension(NumSubs + 1);

                    EndUseCategory(EndUseNum).NumSubcategories = NumSubs + 1;
                    EndUseCategory(EndUseNum).SubcategoryName(NumSubs + 1) = EndUseSubName;

                    if (EndUseCategory(EndUseNum).NumSubcategories > MaxNumSubcategories) {
                        MaxNumSubcategories = EndUseCategory(EndUseNum).NumSubcategories;
                    }

                    Found = true;
                }
                break;
            }
        }

        if (!Found) {
            ShowSevereError("Nonexistent end use passed to AddEndUseSubcategory=" + EndUseName);
        }
    }
    void WriteTimeStampFormatData(
        InputOutputFile &outputFile,
        ReportingFrequency const reportingInterval, // See Module Parameter Definitons for ReportEach, ReportTimeStep, ReportHourly, etc.
        int const reportID,                         // The ID of the time stamp
        std::string const &reportIDString,          // The ID of the time stamp
        int const DayOfSim,                         // the number of days simulated so far
        std::string const &DayOfSimChr,             // the number of days simulated so far
        bool writeToSQL,
        Optional_int_const Month,           // the month of the reporting interval
        Optional_int_const DayOfMonth,      // The day of the reporting interval
        Optional_int_const Hour,            // The hour of the reporting interval
        Optional<Real64 const> EndMinute,   // The last minute in the reporting interval
        Optional<Real64 const> StartMinute, // The starting minute of the reporting interval
        Optional_int_const DST,             // A flag indicating whether daylight savings time is observed
        Optional_string_const DayType       // The day tied for the data (e.g., Monday)
    )
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         Greg Stark
        //       DATE WRITTEN   July 2008
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS FUNCTION:
        // This function reports the timestamp data for the output processor
        // Much of the code in this function was embedded in earlier versions of EnergyPlus
        // and was moved to this location to simplify maintenance and to allow for data output
        // to the SQL database

        // METHODOLOGY EMPLOYED:
        // na

        // REFERENCES:
        // na

        // Using/Aliasing
        using namespace DataPrecisionGlobals;

        // Locals
        // FUNCTION ARGUMENT DEFINITIONS:

        // FUNCTION PARAMETER DEFINITIONS:
        // na

        // INTERFACE BLOCK SPECIFICATIONS:
        // na

        // DERIVED TYPE DEFINITIONS:
        // na

        // FUNCTION LOCAL VARIABLE DECLARATIONS:
        static int const N(100);
        static char stamp[N];
        assert(reportIDString.length() + DayOfSimChr.length() + (DayType.present() ? DayType().length() : 0u) + 26 <
               N); // Check will fit in stamp size

        if (!outputFile.good()) return;

        switch (reportingInterval) {
        case ReportingFrequency::EachCall:
        case ReportingFrequency::TimeStep:
            std::sprintf(stamp,
                         "%s,%s,%2d,%2d,%2d,%2d,%5.2f,%5.2f,%s",
                         reportIDString.c_str(),
                         DayOfSimChr.c_str(),
                         Month(),
                         DayOfMonth(),
                         DST(),
                         Hour(),
                         StartMinute(),
                         EndMinute(),
                         DayType().c_str());
            print(outputFile, "{}\n", stamp);
            if (writeToSQL && sqlite) {
                sqlite->createSQLiteTimeIndexRecord(static_cast<int>(reportingInterval),
                                                    reportID,
                                                    DayOfSim,
                                                    DataEnvironment::CurEnvirNum,
                                                    DataGlobals::CalendarYear,
                                                    Month,
                                                    DayOfMonth,
                                                    Hour,
                                                    EndMinute,
                                                    StartMinute,
                                                    DST,
                                                    DayType,
                                                    DataGlobals::WarmupFlag);
            }
            break;
        case ReportingFrequency::Hourly:
            std::sprintf(stamp,
                         "%s,%s,%2d,%2d,%2d,%2d,%5.2f,%5.2f,%s",
                         reportIDString.c_str(),
                         DayOfSimChr.c_str(),
                         Month(),
                         DayOfMonth(),
                         DST(),
                         Hour(),
                         0.0,
                         60.0,
                         DayType().c_str());
            print(outputFile, "{}\n", stamp);
            if (writeToSQL && sqlite) {
                sqlite->createSQLiteTimeIndexRecord(static_cast<int>(reportingInterval),
                                                    reportID,
                                                    DayOfSim,
                                                    DataEnvironment::CurEnvirNum,
                                                    DataGlobals::CalendarYear,
                                                    Month,
                                                    DayOfMonth,
                                                    Hour,
                                                    _,
                                                    _,
                                                    DST,
                                                    DayType,
                                                    DataGlobals::WarmupFlag);
            }
            break;
        case ReportingFrequency::Daily:
            std::sprintf(stamp, "%s,%s,%2d,%2d,%2d,%s", reportIDString.c_str(), DayOfSimChr.c_str(), Month(), DayOfMonth(), DST(), DayType().c_str());
            print(outputFile, "{}\n", stamp);
            if (writeToSQL && sqlite) {
                sqlite->createSQLiteTimeIndexRecord(static_cast<int>(reportingInterval),
                                                    reportID,
                                                    DayOfSim,
                                                    DataEnvironment::CurEnvirNum,
                                                    DataGlobals::CalendarYear,
                                                    Month,
                                                    DayOfMonth,
                                                    _,
                                                    _,
                                                    _,
                                                    DST,
                                                    DayType,
                                                    DataGlobals::WarmupFlag);
            }
            break;
        case ReportingFrequency::Monthly:
            std::sprintf(stamp, "%s,%s,%2d", reportIDString.c_str(), DayOfSimChr.c_str(), Month());
            print(outputFile, "{}\n", stamp);
            if (writeToSQL && sqlite) {
                sqlite->createSQLiteTimeIndexRecord(
                    static_cast<int>(reportingInterval), reportID, DayOfSim, DataEnvironment::CurEnvirNum, DataGlobals::CalendarYear, Month);
            }
            break;
        case ReportingFrequency::Simulation:
            std::sprintf(stamp, "%s,%s", reportIDString.c_str(), DayOfSimChr.c_str());
            print(outputFile, "{}\n", stamp);
            if (writeToSQL && sqlite) {
                sqlite->createSQLiteTimeIndexRecord(
                    static_cast<int>(reportingInterval), reportID, DayOfSim, DataEnvironment::CurEnvirNum, DataGlobals::CalendarYear);
            }
            break;
        default:
            if (sqlite) {
                std::string str("Illegal reportingInterval passed to WriteTimeStampFormatData: " +
                                fmt::to_string(static_cast<int>(reportingInterval)));
                sqlite->sqliteWriteMessage(str);
            }
            break;
        }
    }

    void WriteYearlyTimeStamp(InputOutputFile &outputFile,
                              std::string const &reportIDString,   // The ID of the time stamp
                              std::string const &yearOfSimChr,     // the year of the simulation
                              bool writeToSQL)
    {
        print(outputFile, "{},{}\n", reportIDString, yearOfSimChr);
        if (writeToSQL && sqlite) {
            sqlite->createYearlyTimeIndexRecord(DataGlobals::CalendarYear, DataEnvironment::CurEnvirNum);
        }
    }

    void WriteReportVariableDictionaryItem(EnergyPlusData &state,
                                           ReportingFrequency const reportingInterval, // The reporting interval (e.g., hourly, daily)
                                           StoreType const storeType,
                                           int const reportID,                 // The reporting ID for the data
                                           int const EP_UNUSED(indexGroupKey), // The reporting group (e.g., Zone, Plant Loop, etc.)
                                           std::string const &indexGroup,      // The reporting group (e.g., Zone, Plant Loop, etc.)
                                           std::string const &reportIDChr,     // The reporting ID for the data
                                           std::string const &keyedValue,      // The key name for the data
                                           std::string const &variableName,    // The variable's actual name
                                           TimeStepType const timeStepType,
                                           OutputProcessor::Unit const &unitsForVar, // The variables units
                                           Optional_string_const customUnitName,
                                           Optional_string_const ScheduleName)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Greg Stark
        //       DATE WRITTEN   August 2008
        //       MODIFIED       April 2011; Linda Lawrie
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine writes the ESO data dictionary information to the output files
        // and the SQL database

        // METHODOLOGY EMPLOYED:

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
        std::string FreqString;

        FreqString = frequencyNotice(storeType, reportingInterval);

        if (present(ScheduleName)) {
            FreqString += "," + ScheduleName;
        }

        std::string UnitsString;
        if (unitsForVar == OutputProcessor::Unit::customEMS && present(customUnitName)) {
            UnitsString = customUnitName;
        } else {
            UnitsString = unitEnumToString(unitsForVar);
        }

        const auto write = [&](InputOutputFile &file, const int interval) {
            if (file.good()) {
                print(file, "{},{},{},{} [{}]{}\n", reportIDChr, interval, keyedValue, variableName, UnitsString, FreqString);
            }
        };
        switch (reportingInterval) {
        case ReportingFrequency::EachCall:
        case ReportingFrequency::TimeStep:
            write(state.files.eso, 1);
            break;
        case ReportingFrequency::Hourly:
            TrackingHourlyVariables = true;
            write(state.files.eso, 1);
            break;
        case ReportingFrequency::Daily:
            TrackingDailyVariables = true;
            write(state.files.eso, 7);
            break;
        case ReportingFrequency::Monthly:
            TrackingMonthlyVariables = true;
            write(state.files.eso, 9);
            break;
        case ReportingFrequency::Simulation:
            TrackingRunPeriodVariables = true;
            write(state.files.eso, 11);
            break;
        case ReportingFrequency::Yearly:
            TrackingYearlyVariables = true;
            write(state.files.eso, 11);
            break;
            // No default available?
        }

        if (sqlite) {
            sqlite->createSQLiteReportDictionaryRecord(reportID,
                                                       static_cast<int>(storeType),
                                                       indexGroup,
                                                       keyedValue,
                                                       variableName,
                                                       static_cast<int>(timeStepType),
                                                       UnitsString,
                                                       static_cast<int>(reportingInterval),
                                                       false,
                                                       ScheduleName);
        }

        ResultsFramework::resultsFramework->addReportVariable(keyedValue, variableName, UnitsString, reportingInterval);

        // add to ResultsFramework for output variable list, need to check RVI/MVI later
    }

    void WriteMeterDictionaryItem(EnergyPlusData &state,
                                  ReportingFrequency const reportingInterval, // The reporting interval (e.g., hourly, daily)
                                  StoreType const storeType,
                                  int const reportID,                 // The reporting ID in for the variable
                                  int const EP_UNUSED(indexGroupKey), // The reporting group for the variable
                                  std::string const &indexGroup,      // The reporting group for the variable
                                  std::string const &reportIDChr,     // The reporting ID in for the variable
                                  std::string const &meterName,       // The variable's meter name
                                  OutputProcessor::Unit const &unit,  // The variables units
                                  bool const cumulativeMeterFlag,     // A flag indicating cumulative data
                                  bool const meterFileOnlyFlag        // A flag indicating whether the data is to be written to standard output
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Greg Stark
        //       DATE WRITTEN   August 2008
        //       MODIFIED       April 2011; Linda Lawrie
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // The subroutine writes meter data dictionary information to the output files
        // and the SQL database. Much of the code here was embedded in other subroutines
        // and was moved here for the purposes of ease of maintenance and to allow easy
        // data reporting to the SQL database

        // METHODOLOGY EMPLOYED:
        // na

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
        std::string UnitsString = unitEnumToString(unit);

        std::string const FreqString(frequencyNotice(storeType, reportingInterval));

        const auto print_meter = [&](EnergyPlusData &state, const int frequency) {
            const auto out = [&](InputOutputFile &of) {
                if (of.good()) {
                    if (cumulativeMeterFlag) {
                        static constexpr auto fmt{"{},{},Cumulative {} [{}]{}\n"};
                        const auto lenString = index(FreqString, '[');
                        print(of, fmt, reportIDChr, 1, meterName, UnitsString, FreqString.substr(0, lenString));
                    } else {
                        static constexpr auto fmt{"{},{},{} [{}]{}\n"};
                        print(of, fmt, reportIDChr, frequency, meterName, UnitsString, FreqString);
                    }
                }
            };

            out(state.files.mtr);
            if (!meterFileOnlyFlag) {
                out(state.files.eso);
            }
        };

        switch (reportingInterval) {
        case ReportingFrequency::EachCall:
        case ReportingFrequency::TimeStep:
        case ReportingFrequency::Hourly: // -1, 0, 1
            print_meter(state, 1);
            break;
        case ReportingFrequency::Daily: //  2
            print_meter(state, 7);
            break;
        case ReportingFrequency::Monthly: //  3
            print_meter(state, 9);
            break;
        case ReportingFrequency::Yearly: //  5
            print_meter(state, 11);
            break;
        case ReportingFrequency::Simulation: //  4
            print_meter(state, 11);
            break;
        }

        static std::string const keyedValueStringCum("Cumulative ");
        static std::string const keyedValueStringNon;
        std::string const &keyedValueString(cumulativeMeterFlag ? keyedValueStringCum : keyedValueStringNon);

        if (sqlite) {
            sqlite->createSQLiteReportDictionaryRecord(reportID,
                                                       static_cast<int>(storeType),
                                                       indexGroup,
                                                       keyedValueString,
                                                       meterName,
                                                       1,
                                                       UnitsString,
                                                       static_cast<int>(reportingInterval),
                                                       true);
        }

        ResultsFramework::resultsFramework->addReportMeter(meterName, UnitsString, reportingInterval);
        // add to ResultsFramework for output variable list, need to check RVI/MVI later
    }

    void WriteRealVariableOutput(EnergyPlusData &state,
                                 RealVariables &realVar,             // Real variable to write out
                                 ReportingFrequency const reportType // The report type or interval (e.g., hourly)
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Greg Stark
        //       DATE WRITTEN   August 2008
        //       MODIFIED       April 2011; Linda Lawrie, December 2017; Jason DeGraw
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine writes real report variable data to the output file and
        // SQL database. Much of the code here was an included in earlier versions
        // of the UpdateDataandReport subroutine. The code was moved to facilitate
        // easier maintenance and writing of data to the SQL database.

        // METHODOLOGY EMPLOYED:
        // na

        // REFERENCES:
        // na

        // USE STATEMENTS:

        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:

        // SUBROUTINE PARAMETER DEFINITIONS:
        // na

        // INTERFACE BLOCK SPECIFICATIONS:
        // na

        // DERIVED TYPE DEFINITIONS:
        // na

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        // na

        if (realVar.Report && realVar.frequency == reportType && realVar.Stored) {
            if (realVar.NumStored > 0.0) {
                WriteReportRealData(state,
                                    realVar.ReportID,
                                    realVar.ReportIDChr,
                                    realVar.StoreValue,
                                    realVar.storeType,
                                    realVar.NumStored,
                                    realVar.frequency,
                                    realVar.MinValue,
                                    realVar.minValueDate,
                                    realVar.MaxValue,
                                    realVar.maxValueDate);
                ++StdOutputRecordCount;
            }

            realVar.StoreValue = 0.0;
            realVar.NumStored = 0.0;
            realVar.MinValue = MinSetValue;
            realVar.MaxValue = MaxSetValue;
            realVar.Stored = false;
        }
    }

    void WriteReportRealData(EnergyPlusData &state,
                             int const reportID,
                             std::string const &creportID,
                             Real64 const repValue,
                             StoreType const storeType,
                             Real64 const numOfItemsStored,
                             ReportingFrequency const reportingInterval,
                             Real64 const minValue,
                             int const minValueDate,
                             Real64 const MaxValue,
                             int const maxValueDate)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Greg Stark
        //       DATE WRITTEN   July 2008
        //       MODIFIED       April 2011; Linda Lawrie
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine writes the average real data to the output files and
        // SQL database. It supports the WriteRealVariableOutput subroutine.
        // Much of the code here was an included in earlier versions
        // of the UpdateDataandReport subroutine. The code was moved to facilitate
        // easier maintenance and writing of data to the SQL database.

        static char s[129];

        std::string NumberOut;   // Character for producing "number out"
        Real64 repVal(repValue); // The variable's value

        if (storeType == StoreType::Averaged) {
            repVal /= numOfItemsStored;
        }
        if (repVal == 0.0) {
            NumberOut = "0.0";
        } else {
            dtoa(repVal, s);
            NumberOut = std::string(s);
        }

        if (ResultsFramework::resultsFramework->timeSeriesEnabled()) {
            //// The others (<= hourly) are handled inline with the code
            // add to daily TS data store
            if (reportingInterval == ReportingFrequency::Daily) {
                ResultsFramework::resultsFramework->RIDailyTSData.pushVariableValue(reportID, repVal);
            }
            // add to monthly TS data store
            if (reportingInterval == ReportingFrequency::Monthly) {
                ResultsFramework::resultsFramework->RIMonthlyTSData.pushVariableValue(reportID, repVal);
            }
            // add to run period TS data store
            if (reportingInterval == ReportingFrequency::Simulation) {
                ResultsFramework::resultsFramework->RIRunPeriodTSData.pushVariableValue(reportID, repVal);
            }
            // add to annual TS data store
            if (reportingInterval == ReportingFrequency::Yearly) {
                ResultsFramework::resultsFramework->RIYearlyTSData.pushVariableValue(reportID, repVal);
            }
        }

        if (sqlite) {
            sqlite->createSQLiteReportDataRecord(
                reportID, repVal, static_cast<int>(reportingInterval), minValue, minValueDate, MaxValue, maxValueDate);
        }

        if ((reportingInterval == ReportingFrequency::EachCall) || (reportingInterval == ReportingFrequency::TimeStep) ||
            (reportingInterval == ReportingFrequency::Hourly)) { // -1, 0, 1
            if (state.files.eso.good()) {
                print(state.files.eso, "{},{}\n", creportID, NumberOut);
            }

        } else { // if ( ( reportingInterval == ReportingFrequency::Daily ) || ( reportingInterval == ReportingFrequency::Monthly ) || (
                 // reportingInterval == ReportingFrequency::Simulation ) ) { //  2, 3, 4, 5
            std::string MaxOut; // Character for Max out string
            std::string MinOut; // Character for Min out string

            if (MaxValue == 0.0) {
                MaxOut = "0.0";
            } else {
                dtoa(MaxValue, s);
                MaxOut = std::string(s);
            }

            if (minValue == 0.0) {
                MinOut = "0.0";
            } else {
                dtoa(minValue, s);
                MinOut = std::string(s);
            }

            // Append the min and max strings with date information
            ProduceMinMaxString(MinOut, minValueDate, reportingInterval);
            ProduceMinMaxString(MaxOut, maxValueDate, reportingInterval);

            if (state.files.eso.good()) {
                print(state.files.eso, "{},{},{},{}\n", creportID, NumberOut, MinOut, MaxOut);
            }
        }
    }

    void WriteCumulativeReportMeterData(EnergyPlusData &state,
                                        int const reportID,           // The variable's report ID
                                        std::string const &creportID, // variable ID in characters
                                        Real64 const repValue,        // The variable's value
                                        bool const meterOnlyFlag      // A flag that indicates if the data should be written to standard output
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Greg Stark
        //       DATE WRITTEN   July 2008
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine writes the cumulative meter data to the output files and
        // SQL database.

        static char s[129];
        std::string NumberOut; // Character for producing "number out"

        if (repValue == 0.0) {
            NumberOut = "0.0";
        } else {
            dtoa(repValue, s);
            NumberOut = std::string(s);
        }

        if (sqlite) {
            sqlite->createSQLiteReportDataRecord(reportID, repValue);
        }

        if (state.files.mtr.good()) print(state.files.mtr, "{},{}\n", creportID, NumberOut);
        ++DataGlobals::StdMeterRecordCount;

        if (!meterOnlyFlag) {
            if (state.files.eso.good()) print(state.files.eso, "{},{}\n", creportID, NumberOut);
            ++DataGlobals::StdOutputRecordCount;
        }
    }

    void WriteReportMeterData(EnergyPlusData &state,
                              int const reportID,                         // The variable's report ID
                              std::string const &creportID,               // variable ID in characters
                              Real64 const repValue,                      // The variable's value
                              ReportingFrequency const reportingInterval, // The variable's reporting interval (e.g., hourly)
                              Real64 const minValue,                      // The variable's minimum value during the reporting interval
                              int const minValueDate,                     // The date the minimum value occurred
                              Real64 const MaxValue,                      // The variable's maximum value during the reporting interval
                              int const maxValueDate,                     // The date of the maximum value
                              bool const meterOnlyFlag                    // Indicates whether the data is for the meter file only
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Greg Stark
        //       DATE WRITTEN   July 2008
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine writes for the non-cumulative meter data to the output files and
        // SQL database.

        // METHODOLOGY EMPLOYED:
        // na

        // REFERENCES:
        // na

        // Using/Aliasing
        using DataGlobals::StdMeterRecordCount;
        using DataGlobals::StdOutputRecordCount;

        static char s[129];
        std::string NumberOut; // Character for producing "number out"

        if (repValue == 0.0) {
            NumberOut = "0.0";
        } else {
            dtoa(repValue, s);
            NumberOut = std::string(s);
        }

        if (sqlite) {
            sqlite->createSQLiteReportDataRecord(
                reportID, repValue, static_cast<int>(reportingInterval), minValue, minValueDate, MaxValue, maxValueDate, MinutesPerTimeStep);
        }

        if ((reportingInterval == ReportingFrequency::EachCall) || (reportingInterval == ReportingFrequency::TimeStep) ||
            (reportingInterval == ReportingFrequency::Hourly)) { // -1, 0, 1
            if (state.files.mtr.good()) {
                print(state.files.mtr, "{},{}\n", creportID, NumberOut);
            }
            ++StdMeterRecordCount;
            if (state.files.eso.good() && !meterOnlyFlag) {
                print(state.files.eso, "{},{}\n", creportID, NumberOut);
                ++StdOutputRecordCount;
            }
        } else { // if ( ( reportingInterval == ReportDaily ) || ( reportingInterval == ReportMonthly ) || ( reportingInterval == ReportSim ) ) { //
                 // 2, 3, 4
            std::string MaxOut; // Character for Max out string
            std::string MinOut; // Character for Min out string

            if (MaxValue == 0.0) {
                MaxOut = "0.0";
            } else {
                dtoa(MaxValue, s);
                MaxOut = std::string(s);
            }

            if (minValue == 0.0) {
                MinOut = "0.0";
            } else {
                dtoa(minValue, s);
                MinOut = std::string(s);
            }

            // Append the min and max strings with date information
            ProduceMinMaxString(MinOut, minValueDate, reportingInterval);
            ProduceMinMaxString(MaxOut, maxValueDate, reportingInterval);

            if (state.files.mtr.good()) {
                print(state.files.mtr, "{},{},{},{}\n", creportID, NumberOut, MinOut, MaxOut);
            }

            ++StdMeterRecordCount;
            if (state.files.eso.good() && !meterOnlyFlag) {
                print(state.files.eso, "{},{},{},{}\n", creportID, NumberOut, MinOut, MaxOut);
                ++StdOutputRecordCount;
            }
        }
    }

    void WriteNumericData(EnergyPlusData &state,
                          int const reportID,           // The variable's reporting ID
                          std::string const &creportID, // variable ID in characters
                          Real64 const repValue         // The variable's value
    )
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Mark Adams
        //       DATE WRITTEN   May 2016
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE:
        // This subroutine writes real data to the output files and
        // SQL database.
        // This is a refactor of WriteRealData.
        //
        // Much of the code here was an included in earlier versions
        // of the UpdateDataandReport subroutine. The code was moved to facilitate
        // easier maintenance and writing of data to the SQL database.

        static char s[129];

        if (DataSystemVariables::UpdateDataDuringWarmupExternalInterface && !DataSystemVariables::ReportDuringWarmup) return;

        dtoa(repValue, s);

        if (sqlite) {
            sqlite->createSQLiteReportDataRecord(reportID, repValue);
        }

        if (state.files.eso.good()) {
            print(state.files.eso, "{},{}\n", creportID, s);
        }
    }

    void WriteNumericData(EnergyPlusData &state,
                          int const reportID,           // The variable's reporting ID
                          std::string const &creportID, // variable ID in characters
                          int32_t const repValue        // The variable's value
    )
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Mark Adams
        //       DATE WRITTEN   May 2016
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE:
        // This subroutine writes real data to the output files and
        // SQL database.
        // This is a refactor of WriteIntegerData.
        //
        // Much of the code here was an included in earlier versions
        // of the UpdateDataandReport subroutine. The code was moved to facilitate
        // easier maintenance and writing of data to the SQL database.

        static char s[129];

        i32toa(repValue, s);

        if (sqlite) {
            sqlite->createSQLiteReportDataRecord(reportID, repValue);
        }

        if (state.files.eso.good()) {
            print(state.files.eso, "{},{}\n", creportID, s);
        }
    }

    void WriteNumericData(EnergyPlusData &state,
                          int const reportID,           // The variable's reporting ID
                          std::string const &creportID, // variable ID in characters
                          int64_t const repValue        // The variable's value
    )
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Mark Adams
        //       DATE WRITTEN   May 2016
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE:
        // This subroutine writes real data to the output files and
        // SQL database.
        // This is a refactor of WriteIntegerData.
        //
        // Much of the code here was an included in earlier versions
        // of the UpdateDataandReport subroutine. The code was moved to facilitate
        // easier maintenance and writing of data to the SQL database.

        static char s[129];

        i64toa(repValue, s);

        if (sqlite) {
            sqlite->createSQLiteReportDataRecord(reportID, repValue);
        }

        if (state.files.eso.good()) {
            print(state.files.eso, "{},{}\n", creportID, s);
        }
    }

    void WriteIntegerVariableOutput(EnergyPlusData &state,
                                    IntegerVariables &intVar,           // Integer variable to write out
                                    ReportingFrequency const reportType // The report type (i.e., the reporting interval)
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Greg Stark
        //       DATE WRITTEN   August 2008
        //       MODIFIED       April 2011; Linda Lawrie, December 2017; Jason DeGraw
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine writes integer report variable data to the output file and
        // SQL database. Much of the code here was an included in earlier versions
        // of the UpdateDataandReport subroutine. The code was moved to facilitate
        // easier maintenance and writing of data to the SQL database.

        // METHODOLOGY EMPLOYED:
        // na

        // REFERENCES:
        // na

        // Using/Aliasing
        using DataSystemVariables::ReportDuringWarmup;
        using DataSystemVariables::UpdateDataDuringWarmupExternalInterface;

        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:

        // SUBROUTINE PARAMETER DEFINITIONS:
        // na

        // INTERFACE BLOCK SPECIFICATIONS:
        // na

        // DERIVED TYPE DEFINITIONS:
        // na

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        // na

        if (UpdateDataDuringWarmupExternalInterface && !ReportDuringWarmup) return;

        if (intVar.Report && intVar.frequency == reportType && intVar.Stored) {
            if (intVar.NumStored > 0.0) {
                WriteReportIntegerData(state,
                                       intVar.ReportID,
                                       intVar.ReportIDChr,
                                       intVar.StoreValue,
                                       intVar.storeType,
                                       intVar.NumStored,
                                       intVar.frequency,
                                       intVar.MinValue,
                                       intVar.minValueDate,
                                       intVar.MaxValue,
                                       intVar.maxValueDate);
                ++StdOutputRecordCount;
            }

            intVar.StoreValue = 0.0;
            intVar.NumStored = 0.0;
            intVar.MinValue = IMinSetValue;
            intVar.MaxValue = IMaxSetValue;
            intVar.Stored = false;
        }
    }

    void WriteReportIntegerData(EnergyPlusData &state,
                                int const reportID,                         // The variable's reporting ID
                                std::string const &reportIDString,          // The variable's reporting ID (character)
                                Real64 const repValue,                      // The variable's value
                                StoreType const storeType,                  // Type of item (averaged or summed)
                                Real64 const numOfItemsStored,              // The number of items (hours or timesteps) of data stored
                                ReportingFrequency const reportingInterval, // The reporting interval (e.g., monthly)
                                int const minValue,                         // The variable's minimum value during the reporting interval
                                int const minValueDate,                     // The date the minimum value occurred
                                int const MaxValue,                         // The variable's maximum value during the reporting interval
                                int const maxValueDate                      // The date the maximum value occurred
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Greg Stark
        //       DATE WRITTEN   July 2008
        //       MODIFIED       April 2011; Linda Lawrie
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine writes averaged integer data to the output files and
        // SQL database. It supports the WriteIntegerVariableOutput subroutine.
        // Much of the code here was an included in earlier versions
        // of the UpdateDataandReport subroutine. The code was moved to facilitate
        // easier maintenance and writing of data to the SQL database.

        // METHODOLOGY EMPLOYED:
        // na

        // REFERENCES:
        // na

        // Using/Aliasing
        using namespace DataPrecisionGlobals;
        using General::strip_trailing_zeros;

        // Locals

        // SUBROUTINE ARGUMENT DEFINITIONS:

        // SUBROUTINE PARAMETER DEFINITIONS:

        // INTERFACE BLOCK SPECIFICATIONS:
        // na

        // DERIVED TYPE DEFINITIONS:
        // na

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        std::string NumberOut; // Character for producing "number out"
        std::string MaxOut;    // Character for Max out string
        std::string MinOut;    // Character for Min out string
        Real64 rmaxValue;
        Real64 rminValue;
        Real64 repVal; // The variable's value

        repVal = repValue;
        if (storeType == StoreType::Averaged) {
            repVal /= numOfItemsStored;
        }
        if (repValue == 0.0) {
            NumberOut = "0.0";
        } else {
            NumberOut = format("{:N}", repVal);
            strip_trailing_zeros(trim(NumberOut));
        }

        // Append the min and max strings with date information
        MinOut = fmt::to_string(minValue);
        MaxOut = fmt::to_string(MaxValue);
        ProduceMinMaxString(MinOut, minValueDate, reportingInterval);
        ProduceMinMaxString(MaxOut, maxValueDate, reportingInterval);

        if (ResultsFramework::resultsFramework->timeSeriesEnabled()) {
            // add to daily TS data store
            if (reportingInterval == ReportingFrequency::Daily) {
                ResultsFramework::resultsFramework->RIDailyTSData.pushVariableValue(reportID, repVal);
            }
            // add to monthly TS data store
            if (reportingInterval == ReportingFrequency::Monthly) {
                ResultsFramework::resultsFramework->RIMonthlyTSData.pushVariableValue(reportID, repVal);
            }
            // add to run period TS data store
            if (reportingInterval == ReportingFrequency::Simulation) {
                ResultsFramework::resultsFramework->RIRunPeriodTSData.pushVariableValue(reportID, repVal);
            }
            // add to annual TS data store
            if (reportingInterval == ReportingFrequency::Yearly) {
                ResultsFramework::resultsFramework->RIYearlyTSData.pushVariableValue(reportID, repVal);
            }
        }

        rminValue = minValue;
        rmaxValue = MaxValue;
        if (sqlite) {
            sqlite->createSQLiteReportDataRecord(
                reportID, repVal, static_cast<int>(reportingInterval), rminValue, minValueDate, rmaxValue, maxValueDate);
        }

        if ((reportingInterval == ReportingFrequency::EachCall) || (reportingInterval == ReportingFrequency::TimeStep) ||
            (reportingInterval == ReportingFrequency::Hourly)) { // -1, 0, 1
            if (state.files.eso.good()) {
                print(state.files.eso, "{},{}\n", reportIDString, NumberOut);
            }
        } else { // if ( ( reportingInterval == ReportDaily ) || ( reportingInterval == ReportMonthly ) || ( reportingInterval == ReportSim ) ) { //
                 // 2, 3, 4
            if (state.files.eso.good()) {
                print(state.files.eso, "{},{},{},{}\n", reportIDString, NumberOut, MinOut, MaxOut);
            }
        }
    }

    int DetermineIndexGroupKeyFromMeterName(std::string const &meterName) // the meter name
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         Greg Stark
        //       DATE WRITTEN   May 2009
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS FUNCTION:
        // This function attemps to guess determine how a meter variable should be
        // grouped.  It does this by parsing the meter name and then assigns a
        // indexGroupKey based on the name

        // METHODOLOGY EMPLOYED:
        // na

        // REFERENCES:
        // na

        // USE STATEMENTS:
        // na

        // Return value
        int DetermineIndexGroupKeyFromMeterName;

        // Locals
        // FUNCTION ARGUMENT DEFINITIONS:

        // FUNCTION LOCAL VARIABLE DECLARATIONS:
        static int indexGroupKey(-1);

        // Facility indices are in the 100s
        if (has(meterName, "Electricity:Facility")) {
            indexGroupKey = 100;
        } else if (has(meterName, "NaturalGas:Facility")) {
            indexGroupKey = 101;
        } else if (has(meterName, "DistricHeating:Facility")) {
            indexGroupKey = 102;
        } else if (has(meterName, "DistricCooling:Facility")) {
            indexGroupKey = 103;
        } else if (has(meterName, "ElectricityNet:Facility")) {
            indexGroupKey = 104;

            // Building indices are in the 200s
        } else if (has(meterName, "Electricity:Building")) {
            indexGroupKey = 201;
        } else if (has(meterName, "NaturalGas:Building")) {
            indexGroupKey = 202;

            // HVAC indices are in the 300s
        } else if (has(meterName, "Electricity:HVAC")) {
            indexGroupKey = 301;

            // InteriorLights:Electricity:Zone indices are in the 500s
        } else if (has(meterName, "InteriorLights:Electricity:Zone")) {
            indexGroupKey = 501;

            // InteriorLights:Electricity indices are in the 400s
        } else if (has(meterName, "InteriorLights:Electricity")) {
            indexGroupKey = 401;

            // Unknown items have negative indices
        } else {
            indexGroupKey = -11;
        }

        DetermineIndexGroupKeyFromMeterName = indexGroupKey;

        return DetermineIndexGroupKeyFromMeterName;
    }

    std::string DetermineIndexGroupFromMeterGroup(MeterType const &meter) // the meter
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         Greg Stark
        //       DATE WRITTEN   May 2009
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS FUNCTION:
        // This function attemps to determine how a meter variable should be
        // grouped.  It does this by parsing the meter group

        // METHODOLOGY EMPLOYED:
        // na

        // REFERENCES:
        // na

        // USE STATEMENTS:
        // na

        // Return value
        std::string indexGroup;

        // Locals
        // FUNCTION ARGUMENT DEFINITIONS:

        // FUNCTION LOCAL VARIABLE DECLARATIONS:

        if (len(meter.Group) > 0) {
            indexGroup = meter.Group;
        } else {
            indexGroup = "Facility";
        }

        if (len(meter.ResourceType) > 0) {
            indexGroup += ":" + meter.ResourceType;
        }

        if (len(meter.EndUse) > 0) {
            indexGroup += ":" + meter.EndUse;
        }

        if (len(meter.EndUseSub) > 0) {
            indexGroup += ":" + meter.EndUseSub;
        }

        return indexGroup;
    }

    void SetInternalVariableValue(int const varType,       // 1=integer, 2=real, 3=meter
                                  int const keyVarIndex,   // Array index
                                  Real64 const SetRealVal, // real value to set, if type is real or meter
                                  int const SetIntVal      // integer value to set if type is integer
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         B. Griffith
        //       DATE WRITTEN   August 2012
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This is a simple set routine for output pointers
        // It is intended for special use to reinitializations those pointers used for EMS sensors

        // METHODOLOGY EMPLOYED:
        // given a variable type and variable index,
        // assign the pointers the values passed in.

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

        if (varType == 1) { // Integer
            *IVariableTypes(keyVarIndex).VarPtr.Which = SetIntVal;
        } else if (varType == 2) { // real
            *RVariableTypes(keyVarIndex).VarPtr.Which = SetRealVal;
        } else if (varType == 3) { // meter
            EnergyMeters(keyVarIndex).CurTSValue = SetRealVal;
        }
    }

    // returns the string corresponding to the OutputProcessor::Unit enum in brackets
    std::string unitEnumToStringBrackets(Unit const &unitIn)
    {
        // J.Glazer - August/September 2017
        return " [" + unitEnumToString(unitIn) + "]";
    }

    // returns the unit string for a DDVariableTypes item and custom string when customEMS is used
    std::string unitStringFromDDitem(int const ddItemPtr // index provided for DDVariableTypes
    )
    {
        // J.Glazer - August/September 2017
        OutputProcessor::Unit ddUnit = DDVariableTypes(ddItemPtr).units;
        if (ddUnit != OutputProcessor::Unit::customEMS) {
            return unitEnumToStringBrackets(ddUnit);
        } else {
            return " [" + DDVariableTypes(ddItemPtr).unitNameCustomEMS + "]";
        }
    }

    // returns the string corresponding to the OutputProcessor::Unit enum
    std::string unitEnumToString(Unit const &unitIn)
    {
        // J.Glazer - August/September 2017
        switch (unitIn) {
        case OutputProcessor::Unit::J:
            return "J";
            break;
        case OutputProcessor::Unit::W:
            return "W";
            break;
        case OutputProcessor::Unit::C:
            return "C";
            break;
        case OutputProcessor::Unit::None:
            return "";
            break;
        case OutputProcessor::Unit::kg:
            return "kg";
            break;
        case OutputProcessor::Unit::W_m2:
            return "W/m2";
            break;
        case OutputProcessor::Unit::m3:
            return "m3";
            break;
        case OutputProcessor::Unit::hr:
            return "hr";
            break;
        case OutputProcessor::Unit::kg_s:
            return "kg/s";
            break;
        case OutputProcessor::Unit::deg:
            return "deg";
            break;
        case OutputProcessor::Unit::m3_s:
            return "m3/s";
            break;
        case OutputProcessor::Unit::W_m2K:
            return "W/m2-K";
            break;
        case OutputProcessor::Unit::kgWater_kgDryAir:
            return "kgWater/kgDryAir";
            break;
        case OutputProcessor::Unit::Perc:
            return "%";
            break;
        case OutputProcessor::Unit::m_s:
            return "m/s";
            break;
        case OutputProcessor::Unit::lux:
            return "lux";
            break;
        case OutputProcessor::Unit::kgWater_s:
            return "kgWater/s";
            break;
        case OutputProcessor::Unit::rad:
            return "rad";
            break;
        case OutputProcessor::Unit::Pa:
            return "Pa";
            break;
        case OutputProcessor::Unit::J_kg:
            return "J/kg";
            break;
        case OutputProcessor::Unit::m:
            return "m";
            break;
        case OutputProcessor::Unit::lum_W:
            return "lum/W";
            break;
        case OutputProcessor::Unit::kg_m3:
            return "kg/m3";
            break;
        case OutputProcessor::Unit::L:
            return "L";
            break;
        case OutputProcessor::Unit::ach:
            return "ach";
            break;
        case OutputProcessor::Unit::m2:
            return "m2";
            break;
        case OutputProcessor::Unit::deltaC:
            return "deltaC";
            break;
        case OutputProcessor::Unit::J_kgK:
            return "J/kg-K";
            break;
        case OutputProcessor::Unit::W_W:
            return "W/W";
            break;
        case OutputProcessor::Unit::clo:
            return "clo";
            break;
        case OutputProcessor::Unit::W_K:
            return "W/K";
            break;
        case OutputProcessor::Unit::K_W:
            return "K/W";
            break;
        case OutputProcessor::Unit::ppm:
            return "ppm";
            break;
        case OutputProcessor::Unit::kg_kg:
            return "kg/kg";
            break;
        case OutputProcessor::Unit::s:
            return "s";
            break;
        case OutputProcessor::Unit::cd_m2:
            return "cd/m2";
            break;
        case OutputProcessor::Unit::kmol_s:
            return "kmol/s";
            break;
        case OutputProcessor::Unit::K_m:
            return "K/m";
            break;
        case OutputProcessor::Unit::min:
            return "min";
            break;
        case OutputProcessor::Unit::J_kgWater:
            return "J/kgWater";
            break;
        case OutputProcessor::Unit::rev_min:
            return "rev/min";
            break;
        case OutputProcessor::Unit::kg_m2s:
            return "kg/m2-s";
            break;
        case OutputProcessor::Unit::J_m2:
            return "J/m2";
            break;
        case OutputProcessor::Unit::A:
            return "A";
            break;
        case OutputProcessor::Unit::V:
            return "V";
            break;
        case OutputProcessor::Unit::W_m2C:
            return "W/m2-C";
            break;
        case OutputProcessor::Unit::Ah:
            return "Ah";
            break;
        case OutputProcessor::Unit::Btu_h_W:
            return "Btu/h-W";
            break;
        default:
            return "unknown";
            break;
        }
    }

    // returns the OutputProcessor::Unit enum value when a string containing the units is provided without brackets
    OutputProcessor::Unit unitStringToEnum(std::string const &unitIn)
    {
        // J.Glazer - August/September 2017
        std::string unitUpper = UtilityRoutines::MakeUPPERCase(unitIn);
        if (unitUpper == "J") {
            return OutputProcessor::Unit::J;
        } else if (unitUpper == "DELTAC") {
            return OutputProcessor::Unit::deltaC;
        } else if (unitUpper == "") {
            return OutputProcessor::Unit::None;
        } else if (unitUpper == "W") {
            return OutputProcessor::Unit::W;
        } else if (unitUpper == "C") {
            return OutputProcessor::Unit::C;
        } else if (unitUpper == "KG/S") {
            return OutputProcessor::Unit::kg_s;
        } else if (unitUpper == "KGWATER/KGDRYAIR") {
            return OutputProcessor::Unit::kgWater_kgDryAir;
        } else if (unitUpper == "PPM") {
            return OutputProcessor::Unit::ppm;
        } else if (unitUpper == "PA") {
            return OutputProcessor::Unit::Pa;
        } else if (unitUpper == "M3/S") {
            return OutputProcessor::Unit::m3_s;
        } else if (unitUpper == "MIN") {
            return OutputProcessor::Unit::min;
        } else if (unitUpper == "M3") {
            return OutputProcessor::Unit::m3;
        } else if (unitUpper == "KG") {
            return OutputProcessor::Unit::kg;
        } else if (unitUpper == "ACH") {
            return OutputProcessor::Unit::ach;
        } else if (unitUpper == "W/W") {
            return OutputProcessor::Unit::W_W;
        } else if (unitUpper == "LUX") {
            return OutputProcessor::Unit::lux;
        } else if (unitUpper == "LUM/W") {
            return OutputProcessor::Unit::lum_W;
        } else if (unitUpper == "HR") {
            return OutputProcessor::Unit::hr;
        } else if (unitUpper == "CD/M2") {
            return OutputProcessor::Unit::cd_m2;
        } else if (unitUpper == "J/KGWATER") {
            return OutputProcessor::Unit::J_kgWater;
        } else if (unitUpper == "M/S") {
            return OutputProcessor::Unit::m_s;
        } else if (unitUpper == "W/M2") {
            return OutputProcessor::Unit::W_m2;
        } else if (unitUpper == "M") {
            return OutputProcessor::Unit::m;
        } else if (unitUpper == "AH") {
            return OutputProcessor::Unit::Ah;
        } else if (unitUpper == "A") {
            return OutputProcessor::Unit::A;
        } else if (unitUpper == "V") {
            return OutputProcessor::Unit::V;
        } else if (unitUpper == "KMOL/S") {
            return OutputProcessor::Unit::kmol_s;
        } else if (unitUpper == "KG/S") {
            return OutputProcessor::Unit::rev_min;
        } else if (unitUpper == "W/M2-K") {
            return OutputProcessor::Unit::W_m2K;
        } else if (unitUpper == "J/KG") {
            return OutputProcessor::Unit::J_kg;
        } else if (unitUpper == "KG/KG") {
            return OutputProcessor::Unit::kg_kg;
        } else if (unitUpper == "%") {
            return OutputProcessor::Unit::Perc;
        } else if (unitUpper == "DEG") {
            return OutputProcessor::Unit::deg;
        } else if (unitUpper == "S") {
            return OutputProcessor::Unit::s;
        } else if (unitUpper == "KG/M3") {
            return OutputProcessor::Unit::kg_m3;
        } else if (unitUpper == "KG/M2-S") {
            return OutputProcessor::Unit::kg_m2s;
        } else if (unitUpper == "J/KG-K") {
            return OutputProcessor::Unit::J_kgK;
        } else if (unitUpper == "L") {
            return OutputProcessor::Unit::L;
        } else if (unitUpper == "K/M") {
            return OutputProcessor::Unit::K_m;
        } else if (unitUpper == "M2") {
            return OutputProcessor::Unit::m2;
        } else if (unitUpper == "W/M2-C") {
            return OutputProcessor::Unit::W_m2C;
        } else if (unitUpper == "RAD") {
            return OutputProcessor::Unit::rad;
        } else if (unitUpper == "J/M2") {
            return OutputProcessor::Unit::J_m2;
        } else if (unitUpper == "CLO") {
            return OutputProcessor::Unit::clo;
        } else if (unitUpper == "W/K") {
            return OutputProcessor::Unit::W_K;
        } else if (unitUpper == "K/W") {
            return OutputProcessor::Unit::K_W;
        } else if (unitUpper == "KGWATER/S") {
            return OutputProcessor::Unit::kgWater_s;
        } else {
            return OutputProcessor::Unit::unknown;
        }
    }

} // namespace OutputProcessor

//==============================================================================================
// *****************************************************************************
// These routines are available outside the OutputProcessor Module (i.e. calling
// routines do not have to "USE OutputProcessor".  But each of these routines
// will use the OutputProcessor and take advantage that everything is PUBLIC
// within the OutputProcessor.
// *****************************************************************************

void SetupOutputVariable(EnergyPlusData &state,
                         std::string const &VariableName,           // String Name of variable (with units)
                         OutputProcessor::Unit const &VariableUnit, // Actual units corresponding to the actual variable
                         Real64 &ActualVariable,                    // Actual Variable, used to set up pointer
                         std::string const &TimeStepTypeKey,        // Zone, HeatBalance=1, HVAC, System, Plant=2
                         std::string const &VariableTypeKey,        // State, Average=1, NonState, Sum=2
                         std::string const &KeyedValue,             // Associated Key for this variable
                         Optional_string_const ReportFreq,          // Internal use -- causes reporting at this freqency
                         Optional_string_const ResourceTypeKey,     // Meter Resource Type (Electricity, Gas, etc)
                         Optional_string_const EndUseKey,           // Meter End Use Key (Lights, Heating, Cooling, etc)
                         Optional_string_const EndUseSubKey,        // Meter End Use Sub Key (General Lights, Task Lights, etc)
                         Optional_string_const GroupKey,            // Meter Super Group Key (Building, System, Plant)
                         Optional_string_const ZoneKey,             // Meter Zone Key (zone name)
                         Optional_int_const ZoneMult,               // Zone Multiplier, defaults to 1
                         Optional_int_const ZoneListMult,           // Zone List Multiplier, defaults to 1
                         Optional_int_const indexGroupKey,          // Group identifier for SQL output
                         Optional_string_const customUnitName       // the custom name for the units from EMS definition of units
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Linda K. Lawrie
    //       DATE WRITTEN   December 1998
    //       MODIFIED       January 2001; Implement Meters
    //                      August 2008; Implement SQL output
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine sets up the variable data structure that will be used
    // to track values of the output variables of EnergyPlus.

    // METHODOLOGY EMPLOYED:
    // Pointers (as pointers), pointers (as indices), and lots of other KEWL data stuff.

    // Using/Aliasing
    using namespace DataPrecisionGlobals;
    using namespace OutputProcessor;
    using General::TrimSigDigits;

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    int CV;
    TimeStepType TimeStepType; // 1=TimeStepZone, 2=TimeStepSys
    StoreType VariableType;    // 1=Average, 2=Sum, 3=Min/Max
    int Loop;
    ReportingFrequency RepFreq(ReportingFrequency::Hourly);
    std::string ResourceType;       // Will hold value of ResourceTypeKey
    std::string EndUse;             // Will hold value of EndUseKey
    std::string EndUseSub;          // Will hold value of EndUseSubKey
    std::string Group;              // Will hold value of GroupKey
    std::string ZoneName;           // Will hold value of ZoneKey
    static bool ErrorsFound(false); // True if Errors Found
    int localIndexGroupKey;

    if (!OutputInitialized) InitializeOutput(state);

    // Variable name without units
    const std::string& VarName = VariableName;

    // Determine whether to Report or not
    CheckReportVariable(state, KeyedValue, VarName);

    if (NumExtraVars == 0) {
        NumExtraVars = 1;
        ReportList = -1;
    }

    // If ReportFreq present, overrides input
    if (present(ReportFreq)) {
        RepFreq = determineFrequency(ReportFreq);
        NumExtraVars = 1;
        ReportList = 0;
    }

    // DataOutputs::OutputVariablesForSimulation is case-insensitive
    bool const ThisOneOnTheList = DataOutputs::FindItemInVariableList(KeyedValue, VarName);
    bool OnMeter = false; // True if this variable is on a meter

    for (Loop = 1; Loop <= NumExtraVars; ++Loop) {

        if (Loop == 1) ++NumOfRVariable_Setup;

        if (Loop == 1) {
            OnMeter = false;
            if (present(ResourceTypeKey)) {
                ResourceType = ResourceTypeKey;
                OnMeter = true;
            } else {
                ResourceType = "";
            }
            if (present(EndUseKey)) {
                EndUse = EndUseKey;
                OnMeter = true;
            } else {
                EndUse = "";
            }
            if (present(EndUseSubKey)) {
                EndUseSub = EndUseSubKey;
                OnMeter = true;
            } else {
                 EndUseSub = "";
                 if (present(EndUseKey)) {
                    if (std::find(endUseCategoryNames.begin(), endUseCategoryNames.end(), UtilityRoutines::MakeUPPERCase(EndUseKey)) != endUseCategoryNames.end()) {
                        EndUseSub = "General";
                    }
                }
            }
            if (present(GroupKey)) {
                Group = GroupKey;
                OnMeter = true;
            } else {
                Group = "";
            }
            if (present(ZoneKey)) {
                ZoneName = ZoneKey;
                OnMeter = true;
            } else {
                ZoneName = "";
            }
        }

        TimeStepType = ValidateTimeStepType(TimeStepTypeKey, "SetupOutputVariable");
        VariableType = validateVariableType(VariableTypeKey);

        if (present(customUnitName)) {
            AddToOutputVariableList(VarName, TimeStepType, VariableType, VarType_Real, VariableUnit, customUnitName);
        } else {
            AddToOutputVariableList(VarName, TimeStepType, VariableType, VarType_Real, VariableUnit);
        }
        ++NumTotalRVariable;

        if (!OnMeter && !ThisOneOnTheList) continue;

        ++NumOfRVariable;
        if (Loop == 1 && VariableType == StoreType::Summed) {
            ++NumOfRVariable_Sum;
            if (present(ResourceTypeKey)) {
                if (!ResourceTypeKey().empty()) ++NumOfRVariable_Meter;
            }
        }
        if (NumOfRVariable > MaxRVariable) {
            ReallocateRVar();
        }
        CV = NumOfRVariable;
        auto &thisRvar = RVariableTypes(CV);
        thisRvar.timeStepType = TimeStepType;
        thisRvar.storeType = VariableType;
        thisRvar.VarName = KeyedValue + ':' + VarName;
        thisRvar.VarNameOnly = VarName;
        thisRvar.VarNameOnlyUC = UtilityRoutines::MakeUPPERCase(VarName);
        thisRvar.VarNameUC = UtilityRoutines::MakeUPPERCase(thisRvar.VarName);
        thisRvar.KeyNameOnlyUC = UtilityRoutines::MakeUPPERCase(KeyedValue);
        thisRvar.units = VariableUnit;
        if (VariableUnit == OutputProcessor::Unit::customEMS) {
            thisRvar.unitNameCustomEMS = customUnitName;
        }
        AssignReportNumber(CurrentReportNumber);
        const auto IDOut = fmt::to_string(CurrentReportNumber);
        thisRvar.ReportID = CurrentReportNumber;
        auto &thisVarPtr = thisRvar.VarPtr;
        thisVarPtr.Value = 0.0;
        thisVarPtr.TSValue = 0.0;
        thisVarPtr.StoreValue = 0.0;
        thisVarPtr.NumStored = 0.0;
        thisVarPtr.MaxValue = MaxSetValue;
        thisVarPtr.maxValueDate = 0;
        thisVarPtr.MinValue = MinSetValue;
        thisVarPtr.minValueDate = 0;
        thisVarPtr.Which = &ActualVariable;
        thisVarPtr.ReportID = CurrentReportNumber;
        thisVarPtr.ReportIDChr = IDOut.substr(0, 15);
        thisVarPtr.storeType = VariableType;
        thisVarPtr.Stored = false;
        thisVarPtr.Report = false;
        thisVarPtr.frequency = ReportingFrequency::Hourly;
        thisVarPtr.SchedPtr = 0;
        thisVarPtr.MeterArrayPtr = 0;
        thisVarPtr.ZoneMult = 1;
        thisVarPtr.ZoneListMult = 1;
        if (present(ZoneMult) && present(ZoneListMult)) {
            thisVarPtr.ZoneMult = ZoneMult;
            thisVarPtr.ZoneListMult = ZoneListMult;
        }

        if (Loop == 1) {
            if (OnMeter) {
                if (VariableType == StoreType::Averaged) {
                    ShowSevereError("Meters can only be \"Summed\" variables");
                    ShowContinueError("..reference variable=" + KeyedValue + ':' + VariableName);
                    ErrorsFound = true;
                } else {
                    Unit mtrUnits = RVariableTypes(CV).units;
                    ErrorsFound = false;
                    AttachMeters(mtrUnits, ResourceType, EndUse, EndUseSub, Group, ZoneName, CV, thisVarPtr.MeterArrayPtr, ErrorsFound);
                    if (ErrorsFound) {
                        ShowContinueError("Invalid Meter spec for variable=" + KeyedValue + ':' + VariableName);
                        ErrorsLogged = true;
                    }
                }
            }
        }

        if (ReportList(Loop) == -1) continue;

        thisVarPtr.Report = true;

        if (ReportList(Loop) == 0) {
            thisVarPtr.frequency = RepFreq;
            thisVarPtr.SchedPtr = 0;
        } else {
            thisVarPtr.frequency = ReqRepVars(ReportList(Loop)).frequency;
            thisVarPtr.SchedPtr = ReqRepVars(ReportList(Loop)).SchedPtr;
        }

        if (thisVarPtr.Report) {
            if (present(indexGroupKey)) {
                localIndexGroupKey = indexGroupKey;
            } else {
                localIndexGroupKey = -999; // Unknown Group
            }

            if (thisVarPtr.SchedPtr != 0) {
                WriteReportVariableDictionaryItem(state,
                                                  thisVarPtr.frequency,
                                                  thisVarPtr.storeType,
                                                  thisVarPtr.ReportID,
                                                  localIndexGroupKey,
                                                  TimeStepTypeKey,
                                                  thisVarPtr.ReportIDChr,
                                                  KeyedValue,
                                                  VarName,
                                                  thisRvar.timeStepType,
                                                  thisRvar.units,
                                                  thisRvar.unitNameCustomEMS,
                                                  ReqRepVars(ReportList(Loop)).SchedName);
            } else {
                WriteReportVariableDictionaryItem(state,
                                                  thisVarPtr.frequency,
                                                  thisVarPtr.storeType,
                                                  thisVarPtr.ReportID,
                                                  localIndexGroupKey,
                                                  TimeStepTypeKey,
                                                  thisVarPtr.ReportIDChr,
                                                  KeyedValue,
                                                  VarName,
                                                  thisRvar.timeStepType,
                                                  thisRvar.units,
                                                  thisRvar.unitNameCustomEMS);
            }
        }
    }
}

void SetupOutputVariable(EnergyPlusData &state,
                         std::string const &VariableName,           // String Name of variable
                         OutputProcessor::Unit const &VariableUnit, // Actual units corresponding to the actual variable
                         int &ActualVariable,                       // Actual Variable, used to set up pointer
                         std::string const &TimeStepTypeKey,        // Zone, HeatBalance=1, HVAC, System, Plant=2
                         std::string const &VariableTypeKey,        // State, Average=1, NonState, Sum=2
                         std::string const &KeyedValue,             // Associated Key for this variable
                         Optional_string_const ReportFreq,          // Internal use -- causes reporting at this freqency
                         Optional_int_const indexGroupKey           // Group identifier for SQL output
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Linda K. Lawrie
    //       DATE WRITTEN   December 1998
    //       MODIFIED       August 2008; Added SQL output capability
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine sets up the variable data structure that will be used
    // to track values of the output variables of EnergyPlus.

    // METHODOLOGY EMPLOYED:
    // Pointers (as pointers), pointers (as indices), and lots of other KEWL data stuff.

    // Using/Aliasing
    using namespace DataPrecisionGlobals;
    using namespace OutputProcessor;
    using General::TrimSigDigits;

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    int CV;
    TimeStepType TimeStepType; // 1=TimeStepZone, 2=TimeStepSys
    StoreType VariableType;    // 1=Average, 2=Sum, 3=Min/Max
    int localIndexGroupKey;
    int Loop;
    ReportingFrequency RepFreq(ReportingFrequency::Hourly);

    if (!OutputInitialized) InitializeOutput(state);

    // Variable name without units
    const std::string& VarName = VariableName;

    // Determine whether to Report or not
    CheckReportVariable(state, KeyedValue, VarName);

    if (NumExtraVars == 0) {
        NumExtraVars = 1;
        ReportList = -1;
    }

    // If ReportFreq present, overrides input
    if (present(ReportFreq)) {
        RepFreq = determineFrequency(ReportFreq);
        NumExtraVars = 1;
        ReportList = 0;
    }

    // DataOutputs::OutputVariablesForSimulation is case-insentitive
    bool const ThisOneOnTheList = DataOutputs::FindItemInVariableList(KeyedValue, VarName);

    for (Loop = 1; Loop <= NumExtraVars; ++Loop) {

        if (Loop == 1) ++NumOfIVariable_Setup;

        TimeStepType = ValidateTimeStepType(TimeStepTypeKey, "SetupOutputVariable");
        VariableType = validateVariableType(VariableTypeKey);

        AddToOutputVariableList(VarName, TimeStepType, VariableType, VarType_Integer, VariableUnit);
        ++NumTotalIVariable;

        if (!ThisOneOnTheList) continue;

        ++NumOfIVariable;
        if (Loop == 1 && VariableType == StoreType::Summed) {
            ++NumOfIVariable_Sum;
        }
        if (NumOfIVariable > MaxIVariable) {
            ReallocateIVar();
        }

        CV = NumOfIVariable;
        auto &thisIVar = IVariableTypes(CV);
        thisIVar.timeStepType = TimeStepType;
        thisIVar.storeType = VariableType;
        thisIVar.VarName = KeyedValue + ':' + VarName;
        thisIVar.VarNameOnly = VarName;
        thisIVar.VarNameOnlyUC = UtilityRoutines::MakeUPPERCase(VarName);
        thisIVar.VarNameUC = UtilityRoutines::MakeUPPERCase(thisIVar.VarName);
        thisIVar.KeyNameOnlyUC = UtilityRoutines::MakeUPPERCase(KeyedValue);
        thisIVar.units = VariableUnit;
        AssignReportNumber(CurrentReportNumber);
        const auto IDOut = fmt::to_string(CurrentReportNumber);
        thisIVar.ReportID = CurrentReportNumber;
        auto &thisVarPtr = thisIVar.VarPtr;
        thisVarPtr.Value = 0.0;
        thisVarPtr.StoreValue = 0.0;
        thisVarPtr.TSValue = 0.0;
        thisVarPtr.NumStored = 0.0;
        //    IVariable%LastTSValue=0
        thisVarPtr.MaxValue = IMaxSetValue;
        thisVarPtr.maxValueDate = 0;
        thisVarPtr.MinValue = IMinSetValue;
        thisVarPtr.minValueDate = 0;
        thisVarPtr.Which = &ActualVariable;
        thisVarPtr.ReportID = CurrentReportNumber;
        thisVarPtr.ReportIDChr = IDOut.substr(0, 15);
        thisVarPtr.storeType = VariableType;
        thisVarPtr.Stored = false;
        thisVarPtr.Report = false;
        thisVarPtr.frequency = ReportingFrequency::Hourly;
        thisVarPtr.SchedPtr = 0;

        if (ReportList(Loop) == -1) continue;

        thisVarPtr.Report = true;

        if (ReportList(Loop) == 0) {
            thisVarPtr.frequency = RepFreq;
            thisVarPtr.SchedPtr = 0;
        } else {
            thisVarPtr.frequency = ReqRepVars(ReportList(Loop)).frequency;
            thisVarPtr.SchedPtr = ReqRepVars(ReportList(Loop)).SchedPtr;
        }

        if (thisVarPtr.Report) {
            if (present(indexGroupKey)) {
                localIndexGroupKey = indexGroupKey;
            } else {
                localIndexGroupKey = -999; // Unknown Group
            }

            if (thisVarPtr.SchedPtr != 0) {
                WriteReportVariableDictionaryItem(state,
                                                  thisVarPtr.frequency,
                                                  thisVarPtr.storeType,
                                                  thisVarPtr.ReportID,
                                                  localIndexGroupKey,
                                                  TimeStepTypeKey,
                                                  thisVarPtr.ReportIDChr,
                                                  KeyedValue,
                                                  VarName,
                                                  thisIVar.timeStepType,
                                                  thisIVar.units,
                                                  ReqRepVars(ReportList(Loop)).SchedName);
            } else {
                WriteReportVariableDictionaryItem(state,
                                                  thisVarPtr.frequency,
                                                  thisVarPtr.storeType,
                                                  thisVarPtr.ReportID,
                                                  localIndexGroupKey,
                                                  TimeStepTypeKey,
                                                  thisVarPtr.ReportIDChr,
                                                  KeyedValue,
                                                  VarName,
                                                  thisIVar.timeStepType,
                                                  thisIVar.units);
            }
        }
    }
}

void SetupOutputVariable(EnergyPlusData &state,
                         std::string const &VariableName,           // String Name of variable
                         OutputProcessor::Unit const &VariableUnit, // Actual units corresponding to the actual variable
                         Real64 &ActualVariable,                    // Actual Variable, used to set up pointer
                         std::string const &TimeStepTypeKey,        // Zone, HeatBalance=1, HVAC, System, Plant=2
                         std::string const &VariableTypeKey,        // State, Average=1, NonState, Sum=2
                         int const KeyedValue,                      // Associated Key for this variable
                         Optional_string_const ReportFreq,          // Internal use -- causes reporting at this freqency
                         Optional_string_const ResourceTypeKey,     // Meter Resource Type (Electricity, Gas, etc)
                         Optional_string_const EndUseKey,           // Meter End Use Key (Lights, Heating, Cooling, etc)
                         Optional_string_const EndUseSubKey,        // Meter End Use Sub Key (General Lights, Task Lights, etc)
                         Optional_string_const GroupKey,            // Meter Super Group Key (Building, System, Plant)
                         Optional_string_const ZoneKey,             // Meter Zone Key (zone name)
                         Optional_int_const ZoneMult,               // Zone Multiplier, defaults to 1
                         Optional_int_const ZoneListMult,           // Zone List Multiplier, defaults to 1
                         Optional_int_const indexGroupKey           // Group identifier for SQL output
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Linda K. Lawrie
    //       DATE WRITTEN   February 1999
    //       MODIFIED       January 2001; Implement Meters
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine allows an integer key for a variable.  Changes this to a
    // standard character variable and passes everything to SetupOutputVariable.

    // METHODOLOGY EMPLOYED:
    // Pointers (as pointers), pointers (as indices), and lots of other KEWL data stuff.

    // REFERENCES:
    // na

    // Using/Aliasing
    using namespace DataPrecisionGlobals;

    // Locals
    // SUBROUTINE ARGUMENT DEFINITIONS:

    // SUBROUTINE PARAMETER DEFINITIONS:

    // INTERFACE BLOCK SPECIFICATIONS:
    // na

    // DERIVED TYPE DEFINITIONS:
    // na

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:

    // Not checking for valid number

    SetupOutputVariable(state,
                        VariableName,
                        VariableUnit,
                        ActualVariable,
                        TimeStepTypeKey,
                        VariableTypeKey,
                        fmt::to_string(KeyedValue),
                        ReportFreq,
                        ResourceTypeKey,
                        EndUseKey,
                        EndUseSubKey,
                        GroupKey,
                        ZoneKey,
                        ZoneMult,
                        ZoneListMult,
                        indexGroupKey);
}

void UpdateDataandReport(EnergyPlusData &state, OutputProcessor::TimeStepType const t_TimeStepTypeKey) // What kind of data to update (Zone, HVAC)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Linda K. Lawrie
    //       DATE WRITTEN   December 1998
    //       MODIFIED       January 2001; Resolution integrated at the Zone TimeStep intervals
    //       MODIFIED       August 2008; Added SQL output capability
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine writes the actual report variable (for user requested
    // Report Variables) strings to the standard output file.

    // METHODOLOGY EMPLOYED:
    // na

    // REFERENCES:
    // na

    // Using/Aliasing
    using namespace DataPrecisionGlobals;
    using namespace OutputProcessor;
    using DataEnvironment::EndMonthFlag;
    using DataEnvironment::EndYearFlag;
    using DataGlobals::EndDayFlag;
    using DataGlobals::EndEnvrnFlag;
    using DataGlobals::EndHourFlag;
    using DataGlobals::HourOfDay;
    using General::EncodeMonDayHrMin;
    using ScheduleManager::GetCurrentScheduleValue;

    // Locals
    // SUBROUTINE ARGUMENT DEFINITIONS:

    // SUBROUTINE PARAMETER DEFINITIONS:
    // na

    // INTERFACE BLOCK SPECIFICATIONS:
    // na

    // DERIVED TYPE DEFINITIONS:
    // na

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    int Loop;             // Loop Variable
    Real64 CurVal;        // Current value for real variables
    Real64 ICurVal;       // Current value for integer variables
    int MDHM;             // Month,Day,Hour,Minute
    bool TimePrint(true); // True if the time needs to be printed
    Real64 StartMinute;   // StartMinute for UpdateData call
    Real64 MinuteNow;     // What minute it is now
    bool ReportNow;       // True if this variable should be reported now
    int CurDayType;       // What kind of day it is (weekday (sunday, etc) or holiday)
    //////////// hoisted into namespace ////////////////////////////////////////////////
    // static int LHourP( -1 ); // Helps set hours for timestamp output
    // static Real64 LStartMin( -1.0 ); // Helps set minutes for timestamp output
    // static Real64 LEndMin( -1.0 ); // Helps set minutes for timestamp output
    ////////////////////////////////////////////////////////////////////////////////////
    static bool EndTimeStepFlag(false); // True when it's the end of the Zone Time Step
    Real64 rxTime;                      // (MinuteNow-StartMinute)/REAL(MinutesPerTimeStep,r64) - for execution time

    if (t_TimeStepTypeKey != TimeStepType::TimeStepZone && t_TimeStepTypeKey != TimeStepType::TimeStepSystem) {
        ShowFatalError("Invalid reporting requested -- UpdateDataAndReport");
    }

    // Basic record keeping and report out if "detailed"
    StartMinute = TimeValue.at(t_TimeStepTypeKey).CurMinute;
    TimeValue.at(t_TimeStepTypeKey).CurMinute += (*TimeValue.at(t_TimeStepTypeKey).TimeStep) * 60.0;
    if (t_TimeStepTypeKey == TimeStepType::TimeStepSystem &&
        (TimeValue.at(TimeStepType::TimeStepSystem).CurMinute == TimeValue.at(TimeStepType::TimeStepZone).CurMinute)) {
        EndTimeStepFlag = true;
    } else if (t_TimeStepTypeKey == TimeStepType::TimeStepZone) {
        EndTimeStepFlag = true;
    } else {
        EndTimeStepFlag = false;
    }
    MinuteNow = TimeValue.at(t_TimeStepTypeKey).CurMinute;

    EncodeMonDayHrMin(MDHM, Month, DayOfMonth, HourOfDay, int(MinuteNow));
    TimePrint = true;

    rxTime = (MinuteNow - StartMinute) / double(MinutesPerTimeStep);

    if (ResultsFramework::resultsFramework->timeSeriesEnabled()) {
        // R and I data frames for TimeStepType::TimeStepZone
        if (t_TimeStepTypeKey == TimeStepType::TimeStepZone && !ResultsFramework::resultsFramework->RIDetailedZoneTSData.rVariablesScanned()) {
            ResultsFramework::resultsFramework->initializeRTSDataFrame(
                ReportingFrequency::EachCall, RVariableTypes, NumOfRVariable, TimeStepType::TimeStepZone);
        }
        if (t_TimeStepTypeKey == TimeStepType::TimeStepZone && !ResultsFramework::resultsFramework->RIDetailedZoneTSData.iVariablesScanned()) {
            ResultsFramework::resultsFramework->initializeITSDataFrame(
                ReportingFrequency::EachCall, IVariableTypes, NumOfIVariable, TimeStepType::TimeStepZone);
        }

        // R and I data frames for TimeStepType::TimeStepSystem
        if (t_TimeStepTypeKey == TimeStepType::TimeStepSystem && !ResultsFramework::resultsFramework->RIDetailedHVACTSData.rVariablesScanned()) {
            ResultsFramework::resultsFramework->initializeRTSDataFrame(
                ReportingFrequency::EachCall, RVariableTypes, NumOfRVariable, TimeStepType::TimeStepSystem);
        }
        if (t_TimeStepTypeKey == TimeStepType::TimeStepSystem && !ResultsFramework::resultsFramework->RIDetailedHVACTSData.iVariablesScanned()) {
            ResultsFramework::resultsFramework->initializeITSDataFrame(
                ReportingFrequency::EachCall, IVariableTypes, NumOfIVariable, TimeStepType::TimeStepSystem);
        }
    }

    if (ResultsFramework::resultsFramework->timeSeriesEnabled()) {
        if (t_TimeStepTypeKey == TimeStepType::TimeStepZone) {
            ResultsFramework::resultsFramework->RIDetailedZoneTSData.newRow(
                Month, DayOfMonth, HourOfDay, TimeValue.at(TimeStepType::TimeStepZone).CurMinute);
        }
        if (t_TimeStepTypeKey == TimeStepType::TimeStepSystem) {
            // TODO this was an error probably, was using TimeValue(1)
            ResultsFramework::resultsFramework->RIDetailedHVACTSData.newRow(
                Month, DayOfMonth, HourOfDay, TimeValue.at(TimeStepType::TimeStepSystem).CurMinute);
        }
    }

    // Main "Record Keeping" Loops for R and I variables
    for (Loop = 1; Loop <= NumOfRVariable; ++Loop) {
        if (RVariableTypes(Loop).timeStepType != t_TimeStepTypeKey) continue;

        // Act on the RVariables variable
        auto &rVar(RVariableTypes(Loop).VarPtr);
        rVar.Stored = true;
        if (rVar.storeType == StoreType::Averaged) {
            CurVal = (*rVar.Which) * rxTime;
            //        CALL SetMinMax(RVar%Which,MDHM,RVar%MaxValue,RVar%maxValueDate,RVar%MinValue,RVar%minValueDate)
            if ((*rVar.Which) > rVar.MaxValue) {
                rVar.MaxValue = (*rVar.Which);
                rVar.maxValueDate = MDHM;
            }
            if ((*rVar.Which) < rVar.MinValue) {
                rVar.MinValue = (*rVar.Which);
                rVar.minValueDate = MDHM;
            }
            rVar.TSValue += CurVal;
            rVar.EITSValue = rVar.TSValue; // CR - 8481 fix - 09/06/2011
        } else {
            //        CurVal=RVar%Which
            if ((*rVar.Which) > rVar.MaxValue) {
                rVar.MaxValue = (*rVar.Which);
                rVar.maxValueDate = MDHM;
            }
            if ((*rVar.Which) < rVar.MinValue) {
                rVar.MinValue = (*rVar.Which);
                rVar.minValueDate = MDHM;
            }
            rVar.TSValue += (*rVar.Which);
            rVar.EITSValue = rVar.TSValue; // CR - 8481 fix - 09/06/2011
        }

        // End of "record keeping"  Report if applicable
        if (!rVar.Report) continue;
        ReportNow = true;
        if (rVar.SchedPtr > 0) ReportNow = (GetCurrentScheduleValue(rVar.SchedPtr) != 0.0); // SetReportNow(RVar%SchedPtr)
        if (!ReportNow) continue;
        rVar.tsStored = true;
        if (!rVar.thisTSStored) {
            ++rVar.thisTSCount;
            rVar.thisTSStored = true;
        }

        if (rVar.frequency == ReportingFrequency::EachCall) {
            if (TimePrint) {
                if (LHourP != HourOfDay || std::abs(LStartMin - StartMinute) > 0.001 ||
                    std::abs(LEndMin - TimeValue.at(t_TimeStepTypeKey).CurMinute) > 0.001) {
                    CurDayType = DayOfWeek;
                    if (HolidayIndex > 0) {
                        CurDayType = 7 + HolidayIndex;
                    }
                    WriteTimeStampFormatData(state.files.eso,
                                             ReportingFrequency::EachCall,
                                             TimeStepStampReportNbr,
                                             TimeStepStampReportChr,
                                             DayOfSim,
                                             state.dataGlobal->DayOfSimChr,
                                             true,
                                             Month,
                                             DayOfMonth,
                                             HourOfDay,
                                             TimeValue.at(t_TimeStepTypeKey).CurMinute,
                                             StartMinute,
                                             DSTIndicator,
                                             DayTypes(CurDayType));
                    LHourP = HourOfDay;
                    LStartMin = StartMinute;
                    LEndMin = TimeValue.at(t_TimeStepTypeKey).CurMinute;
                }
                TimePrint = false;
            }
            WriteNumericData(state, rVar.ReportID, rVar.ReportIDChr, *rVar.Which);
            ++StdOutputRecordCount;

            if (ResultsFramework::resultsFramework->timeSeriesEnabled()) {
                if (t_TimeStepTypeKey == TimeStepType::TimeStepZone) {
                    ResultsFramework::resultsFramework->RIDetailedZoneTSData.pushVariableValue(rVar.ReportID, *rVar.Which);
                }
                if (t_TimeStepTypeKey == TimeStepType::TimeStepSystem) {
                    ResultsFramework::resultsFramework->RIDetailedHVACTSData.pushVariableValue(rVar.ReportID, *rVar.Which);
                }
            }
        }
    }

    for (Loop = 1; Loop <= NumOfIVariable; ++Loop) {
        if (IVariableTypes(Loop).timeStepType != t_TimeStepTypeKey) continue;

        // Act on the IVariables variable
        auto &iVar(IVariableTypes(Loop).VarPtr);
        iVar.Stored = true;
        //      ICurVal=IVar%Which
        if (iVar.storeType == StoreType::Averaged) {
            ICurVal = (*iVar.Which) * rxTime;
            iVar.TSValue += ICurVal;
            iVar.EITSValue = iVar.TSValue; // CR - 8481 fix - 09/06/2011
            if (nint(ICurVal) > iVar.MaxValue) {
                iVar.MaxValue = nint(ICurVal); // Record keeping for date and time go here too
                iVar.maxValueDate = MDHM;      //+ TimeValue.at(t_TimeStepTypeKey)%TimeStep
            }
            if (nint(ICurVal) < iVar.MinValue) {
                iVar.MinValue = nint(ICurVal);
                iVar.minValueDate = MDHM; //+ TimeValue.at(t_TimeStepTypeKey)%TimeStep
            }
        } else {
            if ((*iVar.Which) > iVar.MaxValue) {
                iVar.MaxValue = (*iVar.Which); // Record keeping for date and time go here too
                iVar.maxValueDate = MDHM;   //+ TimeValue(TimeStepType)%TimeStep
            }
            if ((*iVar.Which) < iVar.MinValue) {
                iVar.MinValue = (*iVar.Which);
                iVar.minValueDate = MDHM; //+ TimeValue(TimeStepType)%TimeStep
            }
            iVar.TSValue += (*iVar.Which);
            iVar.EITSValue = iVar.TSValue; // CR - 8481 fix - 09/06/2011
        }

        if (!iVar.Report) continue;
        ReportNow = true;
        if (iVar.SchedPtr > 0) ReportNow = (GetCurrentScheduleValue(iVar.SchedPtr) != 0.0); // SetReportNow(IVar%SchedPtr)
        if (!ReportNow) continue;
        iVar.tsStored = true;
        if (!iVar.thisTSStored) {
            ++iVar.thisTSCount;
            iVar.thisTSStored = true;
        }

        if (iVar.frequency == ReportingFrequency::EachCall) {
            if (TimePrint) {
                if (LHourP != HourOfDay || std::abs(LStartMin - StartMinute) > 0.001 ||
                    std::abs(LEndMin - TimeValue.at(t_TimeStepTypeKey).CurMinute) > 0.001) {
                    CurDayType = DayOfWeek;
                    if (HolidayIndex > 0) {
                        CurDayType = 7 + HolidayIndex;
                    }
                    WriteTimeStampFormatData(state.files.eso,
                                             ReportingFrequency::EachCall,
                                             TimeStepStampReportNbr,
                                             TimeStepStampReportChr,
                                             DayOfSim,
                                             state.dataGlobal->DayOfSimChr,
                                             true,
                                             Month,
                                             DayOfMonth,
                                             HourOfDay,
                                             TimeValue.at(t_TimeStepTypeKey).CurMinute,
                                             StartMinute,
                                             DSTIndicator,
                                             DayTypes(CurDayType));
                    LHourP = HourOfDay;
                    LStartMin = StartMinute;
                    LEndMin = TimeValue.at(t_TimeStepTypeKey).CurMinute;
                }
                TimePrint = false;
            }
            // only time integer vars actual report as integer only is "detailed"
            WriteNumericData(state, iVar.ReportID, iVar.ReportIDChr, *iVar.Which);
            ++StdOutputRecordCount;

            if (ResultsFramework::resultsFramework->timeSeriesEnabled()) {
                if (t_TimeStepTypeKey == TimeStepType::TimeStepZone) {
                    ResultsFramework::resultsFramework->RIDetailedZoneTSData.pushVariableValue(iVar.ReportID, *iVar.Which);
                }
                if (t_TimeStepTypeKey == TimeStepType::TimeStepSystem) {
                    ResultsFramework::resultsFramework->RIDetailedHVACTSData.pushVariableValue(iVar.ReportID, *iVar.Which);
                }
            }
        }
    }

    if (t_TimeStepTypeKey == TimeStepType::TimeStepSystem) return; // All other stuff happens at the "zone" time step call to this routine.

    // TimeStep Block (Report on Zone TimeStep)

    if (EndTimeStepFlag) {
        if (ResultsFramework::resultsFramework->timeSeriesEnabled()) {
            if (!ResultsFramework::resultsFramework->RITimestepTSData.rVariablesScanned()) {
                ResultsFramework::resultsFramework->initializeRTSDataFrame(ReportingFrequency::TimeStep, RVariableTypes, NumOfRVariable);
            }
            if (!ResultsFramework::resultsFramework->RITimestepTSData.iVariablesScanned()) {
                ResultsFramework::resultsFramework->initializeITSDataFrame(ReportingFrequency::TimeStep, IVariableTypes, NumOfIVariable);
            }
            ResultsFramework::resultsFramework->RITimestepTSData.newRow(Month, DayOfMonth, HourOfDay, TimeValue.at(TimeStepType::TimeStepZone).CurMinute);
        }

        for (auto &thisTimeStepType : {TimeStepType::TimeStepZone, TimeStepType::TimeStepSystem}) { // Zone, HVAC
            for (Loop = 1; Loop <= NumOfRVariable; ++Loop) {
                if (RVariableTypes(Loop).timeStepType != thisTimeStepType) continue;
                auto &rVar(RVariableTypes(Loop).VarPtr);
                // Update meters on the TimeStep  (Zone)
                if (rVar.MeterArrayPtr != 0) {
                    if (VarMeterArrays(rVar.MeterArrayPtr).NumOnCustomMeters <= 0) {
                        UpdateMeterValues(rVar.TSValue * rVar.ZoneMult * rVar.ZoneListMult,
                                          VarMeterArrays(rVar.MeterArrayPtr).NumOnMeters,
                                          VarMeterArrays(rVar.MeterArrayPtr).OnMeters);
                    } else {
                        UpdateMeterValues(rVar.TSValue * rVar.ZoneMult * rVar.ZoneListMult,
                                          VarMeterArrays(rVar.MeterArrayPtr).NumOnMeters,
                                          VarMeterArrays(rVar.MeterArrayPtr).OnMeters,
                                          VarMeterArrays(rVar.MeterArrayPtr).NumOnCustomMeters,
                                          VarMeterArrays(rVar.MeterArrayPtr).OnCustomMeters);
                    }
                }
                ReportNow = true;
                if (rVar.SchedPtr > 0) ReportNow = (GetCurrentScheduleValue(rVar.SchedPtr) != 0.0); // SetReportNow(RVar%SchedPtr)
                if (!ReportNow || !rVar.Report) {
                    rVar.TSValue = 0.0;
                }
                //        IF (RVar%StoreType == AveragedVar) THEN
                //          RVar%Value=RVar%Value+RVar%TSValue/NumOfTimeStepInHour
                //        ELSE
                rVar.Value += rVar.TSValue;
                //        ENDIF

                if (!ReportNow || !rVar.Report) continue;

                if (rVar.frequency == ReportingFrequency::TimeStep) {
                    if (TimePrint) {
                        if (LHourP != HourOfDay || std::abs(LStartMin - StartMinute) > 0.001 ||
                            std::abs(LEndMin - TimeValue.at(thisTimeStepType).CurMinute) > 0.001) {
                            CurDayType = DayOfWeek;
                            if (HolidayIndex > 0) {
                                CurDayType = 7 + HolidayIndex;
                            }
                            WriteTimeStampFormatData(state.files.eso,
                                                     ReportingFrequency::EachCall,
                                                     TimeStepStampReportNbr,
                                                     TimeStepStampReportChr,
                                                     DayOfSim,
                                                     state.dataGlobal->DayOfSimChr,
                                                     true,
                                                     Month,
                                                     DayOfMonth,
                                                     HourOfDay,
                                                     TimeValue.at(thisTimeStepType).CurMinute,
                                                     StartMinute,
                                                     DSTIndicator,
                                                     DayTypes(CurDayType));
                            LHourP = HourOfDay;
                            LStartMin = StartMinute;
                            LEndMin = TimeValue.at(thisTimeStepType).CurMinute;
                        }
                        TimePrint = false;
                    }

                    WriteNumericData(state, rVar.ReportID, rVar.ReportIDChr, rVar.TSValue);
                    ++StdOutputRecordCount;

                    if (ResultsFramework::resultsFramework->timeSeriesEnabled()) {
                        ResultsFramework::resultsFramework->RITimestepTSData.pushVariableValue(rVar.ReportID, rVar.TSValue);
                    }
                }
                rVar.TSValue = 0.0;
                rVar.thisTSStored = false;
            } // Number of R Variables

            for (Loop = 1; Loop <= NumOfIVariable; ++Loop) {
                if (IVariableTypes(Loop).timeStepType != thisTimeStepType) continue;
                auto &iVar(IVariableTypes(Loop).VarPtr);
                ReportNow = true;
                if (iVar.SchedPtr > 0) ReportNow = (GetCurrentScheduleValue(iVar.SchedPtr) != 0.0); // SetReportNow(IVar%SchedPtr)
                if (!ReportNow) {
                    iVar.TSValue = 0.0;
                }
                //        IF (IVar%StoreType == AveragedVar) THEN
                //          IVar%Value=IVar%Value+REAL(IVar%TSValue,r64)/REAL(NumOfTimeStepInHour,r64)
                //        ELSE
                iVar.Value += iVar.TSValue;
                //        ENDIF

                if (!ReportNow || !iVar.Report) continue;

                if (iVar.frequency == ReportingFrequency::TimeStep) {
                    if (TimePrint) {
                        if (LHourP != HourOfDay || std::abs(LStartMin - StartMinute) > 0.001 ||
                            std::abs(LEndMin - TimeValue.at(thisTimeStepType).CurMinute) > 0.001) {
                            CurDayType = DayOfWeek;
                            if (HolidayIndex > 0) {
                                CurDayType = 7 + HolidayIndex;
                            }
                            WriteTimeStampFormatData(state.files.eso,
                                                     ReportingFrequency::EachCall,
                                                     TimeStepStampReportNbr,
                                                     TimeStepStampReportChr,
                                                     DayOfSim,
                                                     state.dataGlobal->DayOfSimChr,
                                                     true,
                                                     Month,
                                                     DayOfMonth,
                                                     HourOfDay,
                                                     TimeValue.at(thisTimeStepType).CurMinute,
                                                     StartMinute,
                                                     DSTIndicator,
                                                     DayTypes(CurDayType));
                            LHourP = HourOfDay;
                            LStartMin = StartMinute;
                            LEndMin = TimeValue.at(thisTimeStepType).CurMinute;
                        }
                        TimePrint = false;
                    }

                    WriteNumericData(state, iVar.ReportID, iVar.ReportIDChr, iVar.TSValue);
                    ++StdOutputRecordCount;

                    if (ResultsFramework::resultsFramework->timeSeriesEnabled()) {
                        ResultsFramework::resultsFramework->RITimestepTSData.pushVariableValue(iVar.ReportID, iVar.TSValue);
                    }
                }
                iVar.TSValue = 0.0;
                iVar.thisTSStored = false;
            } // Number of I Variables
        }     // Index Type (Zone or HVAC)

        UpdateMeters(MDHM);

        ReportTSMeters(state, StartMinute, TimeValue.at(TimeStepType::TimeStepZone).CurMinute, TimePrint, TimePrint);

    } // TimeStep Block

    // Hour Block
    if (EndHourFlag) {
        if (TrackingHourlyVariables) {
            CurDayType = DayOfWeek;
            if (HolidayIndex > 0) {
                CurDayType = 7 + HolidayIndex;
            }
            WriteTimeStampFormatData(state.files.eso,
                                     ReportingFrequency::Hourly,
                                     TimeStepStampReportNbr,
                                     TimeStepStampReportChr,
                                     DayOfSim,
                                     state.dataGlobal->DayOfSimChr,
                                     true,
                                     Month,
                                     DayOfMonth,
                                     HourOfDay,
                                     _,
                                     _,
                                     DSTIndicator,
                                     DayTypes(CurDayType));
            TimePrint = false;
        }

        if (ResultsFramework::resultsFramework->timeSeriesEnabled()) {
            if (!ResultsFramework::resultsFramework->RIHourlyTSData.rVariablesScanned()) {
                ResultsFramework::resultsFramework->initializeRTSDataFrame(ReportingFrequency::Hourly, RVariableTypes, NumOfRVariable);
            }
            if (!ResultsFramework::resultsFramework->RIHourlyTSData.iVariablesScanned()) {
                ResultsFramework::resultsFramework->initializeITSDataFrame(ReportingFrequency::Hourly, IVariableTypes, NumOfIVariable);
            }
            ResultsFramework::resultsFramework->RIHourlyTSData.newRow(Month, DayOfMonth, HourOfDay, 0);
        }

        for (auto &thisTimeStepType : {TimeStepType::TimeStepZone, TimeStepType::TimeStepSystem}) { // Zone, HVAC
            TimeValue.at(thisTimeStepType).CurMinute = 0.0;
            for (Loop = 1; Loop <= NumOfRVariable; ++Loop) {
                if (RVariableTypes(Loop).timeStepType != thisTimeStepType) continue;
                auto &rVar(RVariableTypes(Loop).VarPtr);
                //        ReportNow=.TRUE.
                //        IF (RVar%SchedPtr > 0) &
                //          ReportNow=(GetCurrentScheduleValue(RVar%SchedPtr) /= 0.0)  !SetReportNow(RVar%SchedPtr)

                //        IF (ReportNow) THEN
                if (rVar.tsStored) {
                    if (rVar.storeType == StoreType::Averaged) {
                        rVar.Value /= double(rVar.thisTSCount);
                    }
                    if (rVar.Report && rVar.frequency == ReportingFrequency::Hourly && rVar.Stored) {
                        WriteNumericData(state, rVar.ReportID, rVar.ReportIDChr, rVar.Value);
                        ++StdOutputRecordCount;
                        rVar.Stored = false;
                        // add time series value for hourly to data store
                        if (ResultsFramework::resultsFramework->timeSeriesEnabled()) {
                            ResultsFramework::resultsFramework->RIHourlyTSData.pushVariableValue(rVar.ReportID, rVar.Value);
                        }
                    }
                    rVar.StoreValue += rVar.Value;
                    ++rVar.NumStored;
                }
                rVar.tsStored = false;
                rVar.thisTSStored = false;
                rVar.thisTSCount = 0;
                rVar.Value = 0.0;
            } // Number of R Variables

            for (Loop = 1; Loop <= NumOfIVariable; ++Loop) {
                if (IVariableTypes(Loop).timeStepType != thisTimeStepType) continue;
                auto &iVar(IVariableTypes(Loop).VarPtr);
                //        ReportNow=.TRUE.
                //        IF (IVar%SchedPtr > 0) &
                //          ReportNow=(GetCurrentScheduleValue(IVar%SchedPtr) /= 0.0)  !SetReportNow(IVar%SchedPtr)
                //        IF (ReportNow) THEN
                if (iVar.tsStored) {
                    if (iVar.storeType == StoreType::Averaged) {
                        iVar.Value /= double(iVar.thisTSCount);
                    }
                    if (iVar.Report && iVar.frequency == ReportingFrequency::Hourly && iVar.Stored) {
                        WriteNumericData(state, iVar.ReportID, iVar.ReportIDChr, iVar.Value);
                        ++StdOutputRecordCount;
                        iVar.Stored = false;
                        if (ResultsFramework::resultsFramework->timeSeriesEnabled()) {
                            ResultsFramework::resultsFramework->RIHourlyTSData.pushVariableValue(iVar.ReportID, iVar.Value);
                        }
                    }
                    iVar.StoreValue += iVar.Value;
                    ++iVar.NumStored;
                }
                iVar.tsStored = false;
                iVar.thisTSStored = false;
                iVar.thisTSCount = 0;
                iVar.Value = 0.0;
            } // Number of I Variables
        }     // thisTimeStepType (Zone or HVAC)

        ReportHRMeters(state, TimePrint);

    } // Hour Block

    if (!EndHourFlag) return;

    // Day Block
    if (EndDayFlag) {
        if (TrackingDailyVariables) {
            CurDayType = DayOfWeek;
            if (HolidayIndex > 0) {
                CurDayType = 7 + HolidayIndex;
            }
            WriteTimeStampFormatData(state.files.eso,
                                     ReportingFrequency::Daily,
                                     DailyStampReportNbr,
                                     DailyStampReportChr,
                                     DayOfSim,
                                     state.dataGlobal->DayOfSimChr,
                                     true,
                                     Month,
                                     DayOfMonth,
                                     _,
                                     _,
                                     _,
                                     DSTIndicator,
                                     DayTypes(CurDayType));
            TimePrint = false;
        }
        if (ResultsFramework::resultsFramework->timeSeriesEnabled()) {
            if (!ResultsFramework::resultsFramework->RIDailyTSData.rVariablesScanned()) {
                ResultsFramework::resultsFramework->initializeRTSDataFrame(ReportingFrequency::Daily, RVariableTypes, NumOfRVariable);
            }
            if (!ResultsFramework::resultsFramework->RIDailyTSData.iVariablesScanned()) {
                ResultsFramework::resultsFramework->initializeITSDataFrame(ReportingFrequency::Daily, IVariableTypes, NumOfIVariable);
            }
            ResultsFramework::resultsFramework->RIDailyTSData.newRow(Month, DayOfMonth, HourOfDay, 0);
        }

        NumHoursInMonth += 24;
        for (auto &thisTimeStepType : {TimeStepType::TimeStepZone, TimeStepType::TimeStepSystem}) { // Zone, HVAC
            for (Loop = 1; Loop <= NumOfRVariable; ++Loop) {
                if (RVariableTypes(Loop).timeStepType == thisTimeStepType) {
                    WriteRealVariableOutput(state, RVariableTypes(Loop).VarPtr, ReportingFrequency::Daily);
                }
            } // Number of R Variables

            for (Loop = 1; Loop <= NumOfIVariable; ++Loop) {
                if (IVariableTypes(Loop).timeStepType == thisTimeStepType) {
                    WriteIntegerVariableOutput(state, IVariableTypes(Loop).VarPtr, ReportingFrequency::Daily);
                }
            } // Number of I Variables
        }     // thisTimeStepType (Zone or HVAC)

        ReportDYMeters(state, TimePrint);

    } // Day Block

    // Only continue if EndDayFlag is set
    if (!EndDayFlag) return;

    // Month Block
    if (EndMonthFlag || EndEnvrnFlag) {
        if (TrackingMonthlyVariables) {
            WriteTimeStampFormatData(state.files.eso,
                                     ReportingFrequency::Monthly,
                                     MonthlyStampReportNbr,
                                     MonthlyStampReportChr,
                                     DayOfSim,
                                     state.dataGlobal->DayOfSimChr,
                                     true,
                                     Month);
            TimePrint = false;
        }

        if (ResultsFramework::resultsFramework->timeSeriesEnabled()) {
            if (!ResultsFramework::resultsFramework->RIMonthlyTSData.rVariablesScanned()) {
                ResultsFramework::resultsFramework->initializeRTSDataFrame(ReportingFrequency::Monthly, RVariableTypes, NumOfRVariable);
            }
            if (!ResultsFramework::resultsFramework->RIMonthlyTSData.iVariablesScanned()) {
                ResultsFramework::resultsFramework->initializeITSDataFrame(ReportingFrequency::Monthly, IVariableTypes, NumOfIVariable);
            }
            ResultsFramework::resultsFramework->RIMonthlyTSData.newRow(Month, DayOfMonth, HourOfDay, 0);
        }

        NumHoursInSim += NumHoursInMonth;
        EndMonthFlag = false;
        for (auto &thisTimeStepType : {TimeStepType::TimeStepZone, TimeStepType::TimeStepSystem}) { // Zone, HVAC
            for (Loop = 1; Loop <= NumOfRVariable; ++Loop) {
                if (RVariableTypes(Loop).timeStepType == thisTimeStepType) {
                    WriteRealVariableOutput(state, RVariableTypes(Loop).VarPtr, ReportingFrequency::Monthly);
                }
            } // Number of R Variables

            for (Loop = 1; Loop <= NumOfIVariable; ++Loop) {
                if (IVariableTypes(Loop).timeStepType == thisTimeStepType) {
                    WriteIntegerVariableOutput(state, IVariableTypes(Loop).VarPtr, ReportingFrequency::Monthly);
                }
            } // Number of I Variables
        }     // thisTimeStepType (Zone, HVAC)

        ReportMNMeters(state, TimePrint);

        NumHoursInMonth = 0;
    } // Month Block

    // Sim/Environment Block
    if (EndEnvrnFlag) {
        if (TrackingRunPeriodVariables) {
            WriteTimeStampFormatData(state.files.eso,
                                     ReportingFrequency::Simulation,
                                     RunPeriodStampReportNbr,
                                     RunPeriodStampReportChr,
                                     DayOfSim,
                                     state.dataGlobal->DayOfSimChr,
                                     true);
            TimePrint = false;
        }

        if (ResultsFramework::resultsFramework->timeSeriesEnabled()) {
            if (!ResultsFramework::resultsFramework->RIRunPeriodTSData.rVariablesScanned()) {
                ResultsFramework::resultsFramework->initializeRTSDataFrame(ReportingFrequency::Simulation, RVariableTypes, NumOfRVariable);
            }
            if (!ResultsFramework::resultsFramework->RIRunPeriodTSData.iVariablesScanned()) {
                ResultsFramework::resultsFramework->initializeITSDataFrame(ReportingFrequency::Simulation, IVariableTypes, NumOfIVariable);
            }
            ResultsFramework::resultsFramework->RIRunPeriodTSData.newRow(Month, DayOfMonth, HourOfDay, 0);
        }
        for (auto &thisTimeStepType : {TimeStepType::TimeStepZone, TimeStepType::TimeStepSystem}) { // Zone, HVAC
            for (Loop = 1; Loop <= NumOfRVariable; ++Loop) {
                if (RVariableTypes(Loop).timeStepType == thisTimeStepType) {
                    WriteRealVariableOutput(state, RVariableTypes(Loop).VarPtr, ReportingFrequency::Simulation);
                }
            } // Number of R Variables

            for (Loop = 1; Loop <= NumOfIVariable; ++Loop) {
                if (IVariableTypes(Loop).timeStepType == thisTimeStepType) {
                    WriteIntegerVariableOutput(state, IVariableTypes(Loop).VarPtr, ReportingFrequency::Simulation);
                }
            } // Number of I Variables
        }     // thisTimeStepType (Zone, HVAC)

        ReportSMMeters(state, TimePrint);

        NumHoursInSim = 0;
    }

    // Yearly Block
    if (EndYearFlag) {
        if (TrackingYearlyVariables) {
            WriteYearlyTimeStamp(state.files.eso, YearlyStampReportChr, DataGlobals::CalendarYearChr, true);
            TimePrint = false;
        }
        if (ResultsFramework::resultsFramework->timeSeriesEnabled()) {
            if (!ResultsFramework::resultsFramework->RIYearlyTSData.rVariablesScanned()) {
                ResultsFramework::resultsFramework->initializeRTSDataFrame(ReportingFrequency::Yearly, RVariableTypes, NumOfRVariable);
            }
            if (!ResultsFramework::resultsFramework->RIYearlyTSData.iVariablesScanned()) {
                ResultsFramework::resultsFramework->initializeITSDataFrame(ReportingFrequency::Yearly, IVariableTypes, NumOfIVariable);
            }
            ResultsFramework::resultsFramework->RIYearlyTSData.newRow(Month, DayOfMonth, HourOfDay, 0);
        }
        for (auto &thisTimeStepType : {TimeStepType::TimeStepZone, TimeStepType::TimeStepSystem}) { // Zone, HVAC
            for (Loop = 1; Loop <= NumOfRVariable; ++Loop) {
                if (RVariableTypes(Loop).timeStepType == thisTimeStepType) {
                    WriteRealVariableOutput(state, RVariableTypes(Loop).VarPtr, ReportingFrequency::Yearly);
                }
            } // Number of R Variables

            for (Loop = 1; Loop <= NumOfIVariable; ++Loop) {
                if (IVariableTypes(Loop).timeStepType == thisTimeStepType) {
                    WriteIntegerVariableOutput(state, IVariableTypes(Loop).VarPtr, ReportingFrequency::Yearly);
                }
            } // Number of I Variables
        }     // thisTimeStepType (Zone, HVAC)

        ReportYRMeters(state, TimePrint);

        DataGlobals::CalendarYear += 1;
        DataGlobals::CalendarYearChr = fmt::to_string(DataGlobals::CalendarYear);
    }
}

void AssignReportNumber(int &ReportNumber)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Linda K. Lawrie
    //       DATE WRITTEN   December 1997
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine returns the next report number available.  The report number
    // is used in output reports as a key.

    // METHODOLOGY EMPLOYED:
    // Use internal ReportNumberCounter to maintain current report numbers.

    // REFERENCES:
    // na

    // USE STATEMENTS:
    using namespace OutputProcessor;

    // Locals
    // SUBROUTINE ARGUMENT DEFINITIONS:

    // FUNCTION PARAMETER DEFINITIONS:
    // na

    // INTERFACE BLOCK SPECIFICATIONS:
    // na

    // DERIVED TYPE DEFINITIONS:
    // na

    // FUNCTION LOCAL VARIABLE DECLARATIONS:
    //////////// hoisted into namespace ////////////
    // static int ReportNumberCounter( 0 );
    ////////////////////////////////////////////////

    ++ReportNumberCounter;
    ReportNumber = ReportNumberCounter;
}

void GenOutputVariablesAuditReport()
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Linda Lawrie
    //       DATE WRITTEN   February 2000
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine reports (to the .err file) any report variables
    // which were requested but not "setup" during the run.  These will
    // either be items that were not used in the IDF file or misspellings
    // of report variable names.

    // METHODOLOGY EMPLOYED:
    // Use flagged data structure in OutputProcessor.

    // REFERENCES:
    // na

    // Using/Aliasing
    using namespace DataPrecisionGlobals;
    using namespace OutputProcessor;
    using DataGlobals::DisplayAdvancedReportVariables;

    // Locals
    // SUBROUTINE ARGUMENT DEFINITIONS:
    // na

    // SUBROUTINE PARAMETER DEFINITIONS:
    static std::map<ReportingFrequency, std::string> reportFrequency({{ReportingFrequency::EachCall, "Detailed"},
                                                                      {ReportingFrequency::TimeStep, "TimeStep"},
                                                                      {ReportingFrequency::Hourly, "Hourly"},
                                                                      {ReportingFrequency::Daily, "Daily"},
                                                                      {ReportingFrequency::Monthly, "Monthly"},
                                                                      {ReportingFrequency::Yearly, "Annual"}});

    // INTERFACE BLOCK SPECIFICATIONS:
    // na

    // DERIVED TYPE DEFINITIONS:
    // na

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    static bool Rept(false);
    int Loop;
    static bool OpaqSurfWarned(false);

    for (Loop = 1; Loop <= NumOfReqVariables; ++Loop) {
        if (ReqRepVars(Loop).Used) continue;
        if (ReqRepVars(Loop).Key.empty()) ReqRepVars(Loop).Key = "*";
        if (has(ReqRepVars(Loop).VarName, "OPAQUE SURFACE INSIDE FACE CONDUCTION") && !DisplayAdvancedReportVariables && !OpaqSurfWarned) {
            ShowWarningError("Variables containing \"Opaque Surface Inside Face Conduction\" are now \"advanced\" variables.");
            ShowContinueError("You must enter the \"Output:Diagnostics,DisplayAdvancedReportVariables;\" statement to view.");
            ShowContinueError("First, though, read cautionary statements in the \"InputOutputReference\" document.");
            OpaqSurfWarned = true;
        }
        if (!Rept) {
            ShowWarningError("The following Report Variables were requested but not generated -- check.rdd file");
            ShowContinueError("Either the IDF did not contain these elements, the variable name is misspelled,");
            ShowContinueError("or the requested variable is an advanced output which requires Output : Diagnostics, DisplayAdvancedReportVariables;");
            Rept = true;
        }
        ShowMessage("Key=" + ReqRepVars(Loop).Key + ", VarName=" + ReqRepVars(Loop).VarName +
                    ", Frequency=" + reportFrequency[ReqRepVars(Loop).frequency]);
    }
}

void UpdateMeterReporting(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Linda Lawrie
    //       DATE WRITTEN   January 2001
    //       MODIFIED       February 2007 -- add cumulative meter reporting
    //                      January 2012 -- add predefined tabular meter reporting
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine is called at the end of the first HVAC iteration and
    // sets up the reporting for the Energy Meters.  It also may show a fatal error
    // if errors occurred during initial SetupOutputVariable processing.  It "gets"
    // the Report Meter input:
    // Report Meter,
    //        \memo Meters requested here show up on eplusout.eso and eplusout.mtr
    //   A1 , \field Meter_Name
    //        \required-field
    //        \note Form is EnergyUseType:..., e.g. Electricity:* for all Electricity meters
    //        \note or EndUse:..., e.g. InteriorLights:* for all interior lights
    //        \note Report MeterFileOnly puts results on the eplusout.mtr file only
    //   A2 ; \field Reporting_Frequency
    //        \type choice
    //        \key timestep
    //        \note timestep refers to the zone timestep/timestep in hour value
    //        \note runperiod, environment, and annual are the same
    //        \key hourly
    //        \key daily
    //        \key monthly
    //        \key runperiod
    //        \key environment
    //        \key annual
    //        \note runperiod, environment, and annual are synonymous
    // Report MeterFileOnly,
    //        \memo same reporting as Report Meter -- goes to eplusout.mtr only
    //   A1 , \field Meter_Name
    //        \required-field
    //        \note Form is EnergyUseType:..., e.g. Electricity:* for all Electricity meters
    //        \note or EndUse:..., e.g. InteriorLights:* for all interior lights
    //        \note Report MeterFileOnly puts results on the eplusout.mtr file only
    //   A2 ; \field Reporting_Frequency
    //        \type choice
    //        \key timestep
    //        \note timestep refers to the zone timestep/timestep in hour value
    //        \note runperiod, environment, and annual are the same
    //        \key hourly
    //        \key daily
    //        \key monthly
    //        \key runperiod
    //        \key environment
    //        \key annual
    //        \note runperiod, environment, and annual are synonymous

    // Using/Aliasing
    using namespace DataIPShortCuts;
    using namespace DataPrecisionGlobals;
    using namespace OutputProcessor;

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    int Loop;
    Array1D_string Alphas(2);
    Array1D<Real64> Numbers(1);
    int NumAlpha;
    int NumNumbers;
    int IOStat;
    std::string::size_type WildCard;
    std::string::size_type TestLen(0);
    std::string::size_type varnameLen;
    int NumReqMeters;
    int NumReqMeterFOs;
    int Meter;
    ReportingFrequency ReportFreq;
    bool NeverFound;

    static bool ErrorsFound(false); // If errors detected in input

    GetCustomMeterInput(state, ErrorsFound);
    if (ErrorsFound) {
        ErrorsLogged = true;
    }

    cCurrentModuleObject = "Output:Meter";
    NumReqMeters = inputProcessor->getNumObjectsFound(cCurrentModuleObject);

    for (Loop = 1; Loop <= NumReqMeters; ++Loop) {

        inputProcessor->getObjectItem(state,
                                      cCurrentModuleObject,
                                      Loop,
                                      Alphas,
                                      NumAlpha,
                                      Numbers,
                                      NumNumbers,
                                      IOStat,
                                      lNumericFieldBlanks,
                                      lAlphaFieldBlanks,
                                      cAlphaFieldNames,
                                      cNumericFieldNames);

        varnameLen = index(Alphas(1), '[');
        if (varnameLen != std::string::npos) Alphas(1).erase(varnameLen);

        WildCard = index(Alphas(1), '*');
        if (WildCard != std::string::npos) {
            TestLen = WildCard;
        }

        ReportFreq = determineFrequency(Alphas(2));

        if (WildCard == std::string::npos) {
            Meter = UtilityRoutines::FindItem(Alphas(1), EnergyMeters);
            if (Meter == 0) {
                ShowWarningError(cCurrentModuleObject + ": invalid " + cAlphaFieldNames(1) + "=\"" + Alphas(1) + "\" - not found.");
                continue;
            }

            SetInitialMeterReportingAndOutputNames(state, Meter, false, ReportFreq, false);

        } else { // Wildcard input
            NeverFound = true;
            for (Meter = 1; Meter <= NumEnergyMeters; ++Meter) {
                if (!UtilityRoutines::SameString(EnergyMeters(Meter).Name.substr(0, TestLen), Alphas(1).substr(0, TestLen))) continue;
                NeverFound = false;

                SetInitialMeterReportingAndOutputNames(state, Meter, false, ReportFreq, false);
            }
            if (NeverFound) {
                ShowWarningError(cCurrentModuleObject + ": invalid " + cAlphaFieldNames(1) + "=\"" + Alphas(1) + "\" - not found.");
            }
        }
    }

    cCurrentModuleObject = "Output:Meter:MeterFileOnly";
    NumReqMeterFOs = inputProcessor->getNumObjectsFound(cCurrentModuleObject);
    for (Loop = 1; Loop <= NumReqMeterFOs; ++Loop) {

        inputProcessor->getObjectItem(state,
                                      cCurrentModuleObject,
                                      Loop,
                                      Alphas,
                                      NumAlpha,
                                      Numbers,
                                      NumNumbers,
                                      IOStat,
                                      lNumericFieldBlanks,
                                      lAlphaFieldBlanks,
                                      cAlphaFieldNames,
                                      cNumericFieldNames);

        varnameLen = index(Alphas(1), '[');
        if (varnameLen != std::string::npos) Alphas(1).erase(varnameLen);

        WildCard = index(Alphas(1), '*');
        if (WildCard != std::string::npos) {
            TestLen = WildCard;
        }

        ReportFreq = determineFrequency(Alphas(2));

        if (WildCard == std::string::npos) {
            Meter = UtilityRoutines::FindItem(Alphas(1), EnergyMeters);
            if (Meter == 0) {
                ShowWarningError(cCurrentModuleObject + ": invalid " + cAlphaFieldNames(1) + "=\"" + Alphas(1) + "\" - not found.");
                continue;
            }

            SetInitialMeterReportingAndOutputNames(state, Meter, true, ReportFreq, false);

        } else { // Wildcard input
            NeverFound = true;
            for (Meter = 1; Meter <= NumEnergyMeters; ++Meter) {
                if (!UtilityRoutines::SameString(EnergyMeters(Meter).Name.substr(0, TestLen), Alphas(1).substr(0, TestLen))) continue;
                NeverFound = false;

                SetInitialMeterReportingAndOutputNames(state, Meter, true, ReportFreq, false);
            }
            if (NeverFound) {
                ShowWarningError(cCurrentModuleObject + ": invalid " + cAlphaFieldNames(1) + "=\"" + Alphas(1) + "\" - not found.");
            }
        }
    }

    cCurrentModuleObject = "Output:Meter:Cumulative";
    NumReqMeters = inputProcessor->getNumObjectsFound(cCurrentModuleObject);

    for (Loop = 1; Loop <= NumReqMeters; ++Loop) {

        inputProcessor->getObjectItem(state,
                                      cCurrentModuleObject,
                                      Loop,
                                      Alphas,
                                      NumAlpha,
                                      Numbers,
                                      NumNumbers,
                                      IOStat,
                                      lNumericFieldBlanks,
                                      lAlphaFieldBlanks,
                                      cAlphaFieldNames,
                                      cNumericFieldNames);

        varnameLen = index(Alphas(1), '[');
        if (varnameLen != std::string::npos) Alphas(1).erase(varnameLen);

        WildCard = index(Alphas(1), '*');
        if (WildCard != std::string::npos) {
            TestLen = WildCard;
        }

        ReportFreq = determineFrequency(Alphas(2));

        if (WildCard == std::string::npos) {
            Meter = UtilityRoutines::FindItem(Alphas(1), EnergyMeters);
            if (Meter == 0) {
                ShowWarningError(cCurrentModuleObject + ": invalid " + cAlphaFieldNames(1) + "=\"" + Alphas(1) + "\" - not found.");
                continue;
            }

            SetInitialMeterReportingAndOutputNames(state, Meter, false, ReportFreq, true);

        } else { // Wildcard input
            NeverFound = true;
            for (Meter = 1; Meter <= NumEnergyMeters; ++Meter) {
                if (!UtilityRoutines::SameString(EnergyMeters(Meter).Name.substr(0, TestLen), Alphas(1).substr(0, TestLen))) continue;
                NeverFound = false;

                SetInitialMeterReportingAndOutputNames(state, Meter, false, ReportFreq, true);
            }
            if (NeverFound) {
                ShowWarningError(cCurrentModuleObject + ": invalid " + cAlphaFieldNames(1) + "=\"" + Alphas(1) + "\" - not found.");
            }
        }
    }

    cCurrentModuleObject = "Output:Meter:Cumulative:MeterFileOnly";
    NumReqMeterFOs = inputProcessor->getNumObjectsFound(cCurrentModuleObject);
    for (Loop = 1; Loop <= NumReqMeterFOs; ++Loop) {

        inputProcessor->getObjectItem(state,
                                      cCurrentModuleObject,
                                      Loop,
                                      Alphas,
                                      NumAlpha,
                                      Numbers,
                                      NumNumbers,
                                      IOStat,
                                      lNumericFieldBlanks,
                                      lAlphaFieldBlanks,
                                      cAlphaFieldNames,
                                      cNumericFieldNames);

        varnameLen = index(Alphas(1), '[');
        if (varnameLen != std::string::npos) Alphas(1).erase(varnameLen);

        WildCard = index(Alphas(1), '*');
        if (WildCard != std::string::npos) {
            TestLen = WildCard;
        }

        ReportFreq = determineFrequency(Alphas(2));

        if (WildCard == std::string::npos) {
            Meter = UtilityRoutines::FindItem(Alphas(1), EnergyMeters);
            if (Meter == 0) {
                ShowWarningError(cCurrentModuleObject + ": invalid " + cAlphaFieldNames(1) + "=\"" + Alphas(1) + "\" - not found.");
                continue;
            }

            SetInitialMeterReportingAndOutputNames(state, Meter, true, ReportFreq, true);

        } else { // Wildcard input
            NeverFound = true;
            for (Meter = 1; Meter <= NumEnergyMeters; ++Meter) {
                if (!UtilityRoutines::SameString(EnergyMeters(Meter).Name.substr(0, TestLen), Alphas(1).substr(0, TestLen))) continue;
                NeverFound = false;

                SetInitialMeterReportingAndOutputNames(state, Meter, true, ReportFreq, true);
            }
            if (NeverFound) {
                ShowWarningError(cCurrentModuleObject + ": invalid " + cAlphaFieldNames(1) + "=\"" + Alphas(1) + "\" - not found.");
            }
        }
    }

    ReportMeterDetails(state);

    if (ErrorsLogged) {
        ShowFatalError("UpdateMeterReporting: Previous Meter Specification errors cause program termination.");
    }

    MeterValue.dimension(NumEnergyMeters, 0.0);
}

void SetInitialMeterReportingAndOutputNames(EnergyPlusData &state,
                                            int const WhichMeter,              // Which meter number
                                            bool const MeterFileOnlyIndicator, // true if this is a meter file only reporting
                                            OutputProcessor::ReportingFrequency const FrequencyIndicator, // at what frequency is the meter reported
                                            bool const CumulativeIndicator // true if this is a Cumulative meter reporting
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Linda Lawrie
    //       DATE WRITTEN   February 2007
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // Set values and output initial names to output files.

    // METHODOLOGY EMPLOYED:
    // na

    // REFERENCES:
    // na

    // Using/Aliasing
    using namespace DataPrecisionGlobals;
    using namespace OutputProcessor;
    using General::TrimSigDigits;

    // Locals
    // SUBROUTINE ARGUMENT DEFINITIONS:

    // SUBROUTINE PARAMETER DEFINITIONS:
    // na

    // INTERFACE BLOCK SPECIFICATIONS:
    // na

    // DERIVED TYPE DEFINITIONS:
    // na

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    int indexGroupKey;
    std::string indexGroup;

    if ((FrequencyIndicator == ReportingFrequency::EachCall) ||
        (FrequencyIndicator == ReportingFrequency::TimeStep)) { // roll "detailed" into TimeStep
        if (!CumulativeIndicator) {
            if (MeterFileOnlyIndicator) {
                if (EnergyMeters(WhichMeter).RptTS) {
                    ShowWarningError("Output:Meter:MeterFileOnly requested for \"" + EnergyMeters(WhichMeter).Name +
                                     "\" (TimeStep), already on \"Output:Meter\". Will report to both " + state.files.eso.fileName +
                                     " and " + state.files.mtr.fileName);
                }
            }
            if (!EnergyMeters(WhichMeter).RptTS) {
                EnergyMeters(WhichMeter).RptTS = true;
                if (MeterFileOnlyIndicator) EnergyMeters(WhichMeter).RptTSFO = true;
                indexGroupKey = DetermineIndexGroupKeyFromMeterName(EnergyMeters(WhichMeter).Name);
                indexGroup = DetermineIndexGroupFromMeterGroup(EnergyMeters(WhichMeter));
                WriteMeterDictionaryItem(state,
                                         FrequencyIndicator,
                                         StoreType::Summed,
                                         EnergyMeters(WhichMeter).TSRptNum,
                                         indexGroupKey,
                                         indexGroup,
                                         EnergyMeters(WhichMeter).TSRptNumChr,
                                         EnergyMeters(WhichMeter).Name,
                                         EnergyMeters(WhichMeter).Units,
                                         false,
                                         MeterFileOnlyIndicator);
            }
        } else {
            if (MeterFileOnlyIndicator) {
                if (EnergyMeters(WhichMeter).RptAccTS) {
                    ShowWarningError("Output:Meter:MeterFileOnly requested for \"Cumulative " + EnergyMeters(WhichMeter).Name +
                                     "\" (TimeStep), already on \"Output:Meter\". Will report to both " + state.files.eso.fileName +
                                     " and " + state.files.mtr.fileName);
                }
            }
            if (!EnergyMeters(WhichMeter).RptAccTS) {
                EnergyMeters(WhichMeter).RptAccTS = true;
                if (MeterFileOnlyIndicator) EnergyMeters(WhichMeter).RptAccTSFO = true;
                indexGroupKey = DetermineIndexGroupKeyFromMeterName(EnergyMeters(WhichMeter).Name);
                indexGroup = DetermineIndexGroupFromMeterGroup(EnergyMeters(WhichMeter));
                WriteMeterDictionaryItem(state,
                                         FrequencyIndicator,
                                         StoreType::Summed,
                                         EnergyMeters(WhichMeter).TSAccRptNum,
                                         indexGroupKey,
                                         indexGroup,
                                         TrimSigDigits(EnergyMeters(WhichMeter).TSAccRptNum),
                                         EnergyMeters(WhichMeter).Name,
                                         EnergyMeters(WhichMeter).Units,
                                         true,
                                         MeterFileOnlyIndicator);
            }
        }
    } else if (FrequencyIndicator == ReportingFrequency::Hourly) {
        if (!CumulativeIndicator) {
            if (MeterFileOnlyIndicator) {
                if (EnergyMeters(WhichMeter).RptHR) {
                    ShowWarningError("Output:Meter:MeterFileOnly requested for \"" + EnergyMeters(WhichMeter).Name +
                                     "\" (Hourly), already on \"Output:Meter\". Will report to both " + state.files.eso.fileName +
                                     " and " + state.files.mtr.fileName);
                }
            }
            if (!EnergyMeters(WhichMeter).RptHR) {
                EnergyMeters(WhichMeter).RptHR = true;
                if (MeterFileOnlyIndicator) EnergyMeters(WhichMeter).RptHRFO = true;
                if (!MeterFileOnlyIndicator) TrackingHourlyVariables = true;
                indexGroupKey = DetermineIndexGroupKeyFromMeterName(EnergyMeters(WhichMeter).Name);
                indexGroup = DetermineIndexGroupFromMeterGroup(EnergyMeters(WhichMeter));
                WriteMeterDictionaryItem(state,
                                         FrequencyIndicator,
                                         StoreType::Summed,
                                         EnergyMeters(WhichMeter).HRRptNum,
                                         indexGroupKey,
                                         indexGroup,
                                         EnergyMeters(WhichMeter).HRRptNumChr,
                                         EnergyMeters(WhichMeter).Name,
                                         EnergyMeters(WhichMeter).Units,
                                         false,
                                         MeterFileOnlyIndicator);
            }
        } else {
            if (MeterFileOnlyIndicator) {
                if (EnergyMeters(WhichMeter).RptAccHR) {
                    ShowWarningError("Output:Meter:MeterFileOnly requested for \"Cumulative " + EnergyMeters(WhichMeter).Name +
                                     "\" (Hourly), already on \"Output:Meter\". Will report to both " + state.files.eso.fileName +
                                     " and " + state.files.mtr.fileName);
                }
            }
            if (!EnergyMeters(WhichMeter).RptAccHR) {
                EnergyMeters(WhichMeter).RptAccHR = true;
                if (MeterFileOnlyIndicator) EnergyMeters(WhichMeter).RptAccHRFO = true;
                if (!MeterFileOnlyIndicator) TrackingHourlyVariables = true;
                indexGroupKey = DetermineIndexGroupKeyFromMeterName(EnergyMeters(WhichMeter).Name);
                indexGroup = DetermineIndexGroupFromMeterGroup(EnergyMeters(WhichMeter));
                WriteMeterDictionaryItem(state,
                                         FrequencyIndicator,
                                         StoreType::Summed,
                                         EnergyMeters(WhichMeter).HRAccRptNum,
                                         indexGroupKey,
                                         indexGroup,
                                         TrimSigDigits(EnergyMeters(WhichMeter).HRAccRptNum),
                                         EnergyMeters(WhichMeter).Name,
                                         EnergyMeters(WhichMeter).Units,
                                         true,
                                         MeterFileOnlyIndicator);
            }
        }
    } else if (FrequencyIndicator == ReportingFrequency::Daily) {
        if (!CumulativeIndicator) {
            if (MeterFileOnlyIndicator) {
                if (EnergyMeters(WhichMeter).RptDY) {
                    ShowWarningError("Output:Meter:MeterFileOnly requested for \"" + EnergyMeters(WhichMeter).Name +
                                     "\" (Daily), already on \"Output:Meter\". Will report to both " + state.files.eso.fileName +
                                     " and " + state.files.mtr.fileName);
                }
            }
            if (!EnergyMeters(WhichMeter).RptDY) {
                EnergyMeters(WhichMeter).RptDY = true;
                if (MeterFileOnlyIndicator) EnergyMeters(WhichMeter).RptDYFO = true;
                if (!MeterFileOnlyIndicator) TrackingDailyVariables = true;
                indexGroupKey = DetermineIndexGroupKeyFromMeterName(EnergyMeters(WhichMeter).Name);
                indexGroup = DetermineIndexGroupFromMeterGroup(EnergyMeters(WhichMeter));
                WriteMeterDictionaryItem(state,
                                         FrequencyIndicator,
                                         StoreType::Summed,
                                         EnergyMeters(WhichMeter).DYRptNum,
                                         indexGroupKey,
                                         indexGroup,
                                         EnergyMeters(WhichMeter).DYRptNumChr,
                                         EnergyMeters(WhichMeter).Name,
                                         EnergyMeters(WhichMeter).Units,
                                         false,
                                         MeterFileOnlyIndicator);
            }
        } else {
            if (MeterFileOnlyIndicator) {
                if (EnergyMeters(WhichMeter).RptAccDY) {
                    ShowWarningError("Output:Meter:MeterFileOnly requested for \"Cumulative " + EnergyMeters(WhichMeter).Name +
                                     "\" (Hourly), already on \"Output:Meter\". Will report to both " + state.files.eso.fileName +
                                     " and " + state.files.mtr.fileName);
                }
            }
            if (!EnergyMeters(WhichMeter).RptAccDY) {
                EnergyMeters(WhichMeter).RptAccDY = true;
                if (MeterFileOnlyIndicator) EnergyMeters(WhichMeter).RptAccDYFO = true;
                if (!MeterFileOnlyIndicator) TrackingDailyVariables = true;
                indexGroupKey = DetermineIndexGroupKeyFromMeterName(EnergyMeters(WhichMeter).Name);
                indexGroup = DetermineIndexGroupFromMeterGroup(EnergyMeters(WhichMeter));
                WriteMeterDictionaryItem(state,
                                         FrequencyIndicator,
                                         StoreType::Summed,
                                         EnergyMeters(WhichMeter).DYAccRptNum,
                                         indexGroupKey,
                                         indexGroup,
                                         TrimSigDigits(EnergyMeters(WhichMeter).DYAccRptNum),
                                         EnergyMeters(WhichMeter).Name,
                                         EnergyMeters(WhichMeter).Units,
                                         true,
                                         MeterFileOnlyIndicator);
            }
        }
    } else if (FrequencyIndicator == ReportingFrequency::Monthly) {
        if (!CumulativeIndicator) {
            if (MeterFileOnlyIndicator) {
                if (EnergyMeters(WhichMeter).RptMN) {
                    ShowWarningError("Output:Meter:MeterFileOnly requested for \"" + EnergyMeters(WhichMeter).Name +
                                     "\" (Monthly), already on \"Output:Meter\". Will report to both " + state.files.eso.fileName +
                                     " and " + state.files.mtr.fileName);
                }
            }
            if (!EnergyMeters(WhichMeter).RptMN) {
                EnergyMeters(WhichMeter).RptMN = true;
                if (MeterFileOnlyIndicator) EnergyMeters(WhichMeter).RptMNFO = true;
                if (!MeterFileOnlyIndicator) TrackingMonthlyVariables = true;
                indexGroupKey = DetermineIndexGroupKeyFromMeterName(EnergyMeters(WhichMeter).Name);
                indexGroup = DetermineIndexGroupFromMeterGroup(EnergyMeters(WhichMeter));
                WriteMeterDictionaryItem(state,
                                         FrequencyIndicator,
                                         StoreType::Summed,
                                         EnergyMeters(WhichMeter).MNRptNum,
                                         indexGroupKey,
                                         indexGroup,
                                         EnergyMeters(WhichMeter).MNRptNumChr,
                                         EnergyMeters(WhichMeter).Name,
                                         EnergyMeters(WhichMeter).Units,
                                         false,
                                         MeterFileOnlyIndicator);
            }
        } else {
            if (MeterFileOnlyIndicator) {
                if (EnergyMeters(WhichMeter).RptAccMN) {
                    ShowWarningError("Output:Meter:MeterFileOnly requested for \"Cumulative " + EnergyMeters(WhichMeter).Name +
                                     "\" (Monthly), already on \"Output:Meter\". Will report to both " + state.files.eso.fileName +
                                     " and " + state.files.mtr.fileName);
                }
            }
            if (!EnergyMeters(WhichMeter).RptAccMN) {
                EnergyMeters(WhichMeter).RptAccMN = true;
                if (MeterFileOnlyIndicator) EnergyMeters(WhichMeter).RptAccMNFO = true;
                if (!MeterFileOnlyIndicator) TrackingMonthlyVariables = true;
                indexGroupKey = DetermineIndexGroupKeyFromMeterName(EnergyMeters(WhichMeter).Name);
                indexGroup = DetermineIndexGroupFromMeterGroup(EnergyMeters(WhichMeter));
                WriteMeterDictionaryItem(state,
                                         FrequencyIndicator,
                                         StoreType::Summed,
                                         EnergyMeters(WhichMeter).MNAccRptNum,
                                         indexGroupKey,
                                         indexGroup,
                                         TrimSigDigits(EnergyMeters(WhichMeter).MNAccRptNum),
                                         EnergyMeters(WhichMeter).Name,
                                         EnergyMeters(WhichMeter).Units,
                                         true,
                                         MeterFileOnlyIndicator);
            }
        }
    } else if (FrequencyIndicator == ReportingFrequency::Yearly) {
        if (!CumulativeIndicator) {
            if (MeterFileOnlyIndicator) {
                if (EnergyMeters(WhichMeter).RptYR) {
                    ShowWarningError("Output:Meter:MeterFileOnly requested for \"" + EnergyMeters(WhichMeter).Name +
                                     "\" (Annual), already on \"Output:Meter\". Will report to both " + state.files.eso.fileName +
                                     " and " + state.files.mtr.fileName);
                }
            }
            if (!EnergyMeters(WhichMeter).RptYR) {
                EnergyMeters(WhichMeter).RptYR = true;
                if (MeterFileOnlyIndicator) EnergyMeters(WhichMeter).RptYRFO = true;
                if (!MeterFileOnlyIndicator) TrackingYearlyVariables = true;
                indexGroupKey = DetermineIndexGroupKeyFromMeterName(EnergyMeters(WhichMeter).Name);
                indexGroup = DetermineIndexGroupFromMeterGroup(EnergyMeters(WhichMeter));
                WriteMeterDictionaryItem(state,
                                         FrequencyIndicator,
                                         StoreType::Summed,
                                         EnergyMeters(WhichMeter).YRRptNum,
                                         indexGroupKey,
                                         indexGroup,
                                         EnergyMeters(WhichMeter).YRRptNumChr,
                                         EnergyMeters(WhichMeter).Name,
                                         EnergyMeters(WhichMeter).Units,
                                         false,
                                         MeterFileOnlyIndicator);
            }
        } else {
            if (MeterFileOnlyIndicator) {
                if (EnergyMeters(WhichMeter).RptAccYR) {
                    ShowWarningError("Output:Meter:MeterFileOnly requested for \"Cumulative " + EnergyMeters(WhichMeter).Name +
                                     "\" (Annual), already on \"Output:Meter\". Will report to both " + state.files.eso.fileName +
                                     " and " + state.files.mtr.fileName);
                }
            }
            if (!EnergyMeters(WhichMeter).RptAccYR) {
                EnergyMeters(WhichMeter).RptAccYR = true;
                if (MeterFileOnlyIndicator) EnergyMeters(WhichMeter).RptAccYRFO = true;
                if (!MeterFileOnlyIndicator) TrackingYearlyVariables = true;
                indexGroupKey = DetermineIndexGroupKeyFromMeterName(EnergyMeters(WhichMeter).Name);
                indexGroup = DetermineIndexGroupFromMeterGroup(EnergyMeters(WhichMeter));
                WriteMeterDictionaryItem(state,
                                         FrequencyIndicator,
                                         StoreType::Summed,
                                         EnergyMeters(WhichMeter).YRAccRptNum,
                                         indexGroupKey,
                                         indexGroup,
                                         TrimSigDigits(EnergyMeters(WhichMeter).YRAccRptNum),
                                         EnergyMeters(WhichMeter).Name,
                                         EnergyMeters(WhichMeter).Units,
                                         true,
                                         MeterFileOnlyIndicator);
            }
        }
    } else if (FrequencyIndicator == ReportingFrequency::Simulation) {
        if (!CumulativeIndicator) {
            if (MeterFileOnlyIndicator) {
                if (EnergyMeters(WhichMeter).RptSM) {
                    ShowWarningError("Output:Meter:MeterFileOnly requested for \"" + EnergyMeters(WhichMeter).Name +
                                     "\" (RunPeriod), already on \"Output:Meter\". Will report to both " + state.files.eso.fileName +
                                     " and " + state.files.mtr.fileName);
                }
            }
            if (!EnergyMeters(WhichMeter).RptSM) {
                EnergyMeters(WhichMeter).RptSM = true;
                if (MeterFileOnlyIndicator) EnergyMeters(WhichMeter).RptSMFO = true;
                if (!MeterFileOnlyIndicator) TrackingRunPeriodVariables = true;
                indexGroupKey = DetermineIndexGroupKeyFromMeterName(EnergyMeters(WhichMeter).Name);
                indexGroup = DetermineIndexGroupFromMeterGroup(EnergyMeters(WhichMeter));
                WriteMeterDictionaryItem(state,
                                         FrequencyIndicator,
                                         StoreType::Summed,
                                         EnergyMeters(WhichMeter).SMRptNum,
                                         indexGroupKey,
                                         indexGroup,
                                         EnergyMeters(WhichMeter).SMRptNumChr,
                                         EnergyMeters(WhichMeter).Name,
                                         EnergyMeters(WhichMeter).Units,
                                         false,
                                         MeterFileOnlyIndicator);
            }
        } else {
            if (MeterFileOnlyIndicator) {
                if (EnergyMeters(WhichMeter).RptAccSM) {
                    ShowWarningError("Output:Meter:MeterFileOnly requested for \"Cumulative " + EnergyMeters(WhichMeter).Name +
                                     "\" (RunPeriod), already on \"Output:Meter\". Will report to both " + state.files.eso.fileName +
                                     " and " + state.files.mtr.fileName);
                }
            }
            if (!EnergyMeters(WhichMeter).RptAccSM) {
                EnergyMeters(WhichMeter).RptAccSM = true;
                if (MeterFileOnlyIndicator) EnergyMeters(WhichMeter).RptAccSMFO = true;
                if (!MeterFileOnlyIndicator) TrackingRunPeriodVariables = true;
                indexGroupKey = DetermineIndexGroupKeyFromMeterName(EnergyMeters(WhichMeter).Name);
                indexGroup = DetermineIndexGroupFromMeterGroup(EnergyMeters(WhichMeter));
                WriteMeterDictionaryItem(state,
                                         FrequencyIndicator,
                                         StoreType::Summed,
                                         EnergyMeters(WhichMeter).SMAccRptNum,
                                         indexGroupKey,
                                         indexGroup,
                                         TrimSigDigits(EnergyMeters(WhichMeter).SMAccRptNum),
                                         EnergyMeters(WhichMeter).Name,
                                         EnergyMeters(WhichMeter).Units,
                                         true,
                                         MeterFileOnlyIndicator);
            }
        }
    } else {
    }
}

int GetMeterIndex(std::string const &MeterName)
{

    // FUNCTION INFORMATION:
    //       AUTHOR         Linda K. Lawrie
    //       DATE WRITTEN   August 2002
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS FUNCTION:
    // This function returns a index to the meter "number" (aka assigned report number)
    // for the meter name.  If none active for this run, a zero is returned.  This is used later to
    // obtain a meter "value".

    // Using/Aliasing
    using namespace DataPrecisionGlobals;
    using namespace OutputProcessor;
    using SortAndStringUtilities::SetupAndSort;

    // Return value
    int MeterIndex;

    // FUNCTION LOCAL VARIABLE DECLARATIONS:
    // Valid Meter names because matching case insensitive
    static Array1D_string ValidMeterNames;
    static Array1D_int iValidMeterNames;
    static int NumValidMeters(0);
    //////////// hoisted into namespace changed to GetMeterIndexFirstCall////////////
    // static bool FirstCall( true );
    ////////////////////////////////////////////////
    int Found;

    if (GetMeterIndexFirstCall || (NumValidMeters != NumEnergyMeters)) {
        NumValidMeters = NumEnergyMeters;
        ValidMeterNames.allocate(NumValidMeters);
        for (Found = 1; Found <= NumValidMeters; ++Found) {
            ValidMeterNames(Found) = UtilityRoutines::MakeUPPERCase(EnergyMeters(Found).Name);
        }
        iValidMeterNames.allocate(NumValidMeters);
        SetupAndSort(ValidMeterNames, iValidMeterNames);
        GetMeterIndexFirstCall = false;
    }

    MeterIndex = UtilityRoutines::FindItemInSortedList(MeterName, ValidMeterNames, NumValidMeters);
    if (MeterIndex != 0) MeterIndex = iValidMeterNames(MeterIndex);

    return MeterIndex;
}

std::string GetMeterResourceType(int const MeterNumber) // Which Meter Number (from GetMeterIndex)
{

    // FUNCTION INFORMATION:
    //       AUTHOR         Linda K. Lawrie
    //       DATE WRITTEN   August 2002
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS FUNCTION:
    // This function returns the character string of Resource Type for the
    // given meter number/index. If MeterNumber is 0, ResourceType=Invalid/Unknown.

    // METHODOLOGY EMPLOYED:
    // na

    // REFERENCES:
    // na

    // Using/Aliasing
    using namespace DataPrecisionGlobals;
    using namespace OutputProcessor;

    // Return value
    std::string ResourceType;

    // Locals
    // FUNCTION ARGUMENT DEFINITIONS:

    // FUNCTION PARAMETER DEFINITIONS:
    // na

    // INTERFACE BLOCK SPECIFICATIONS:
    // na

    // DERIVED TYPE DEFINITIONS:
    // na

    // FUNCTION LOCAL VARIABLE DECLARATIONS:
    if (MeterNumber > 0) {
        ResourceType = EnergyMeters(MeterNumber).ResourceType;
    } else {
        ResourceType = "Invalid/Unknown";
    }

    return ResourceType;
}

Real64 GetCurrentMeterValue(int const MeterNumber) // Which Meter Number (from GetMeterIndex)
{

    // FUNCTION INFORMATION:
    //       AUTHOR         Linda K. Lawrie
    //       DATE WRITTEN   August 2002
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS FUNCTION:
    // This function returns the current meter value (timestep) for the meter number indicated.

    // METHODOLOGY EMPLOYED:
    // Uses internal EnergyMeters structure to get value.

    // REFERENCES:
    // na

    // Using/Aliasing
    using namespace DataPrecisionGlobals;
    using namespace OutputProcessor;

    // Return value
    Real64 CurrentMeterValue;

    // Locals
    // FUNCTION ARGUMENT DEFINITIONS:

    // FUNCTION PARAMETER DEFINITIONS:
    // na

    // INTERFACE BLOCK SPECIFICATIONS:
    // na

    // DERIVED TYPE DEFINITIONS:
    // na

    // FUNCTION LOCAL VARIABLE DECLARATIONS:
    // na

    if (MeterNumber > 0) {
        CurrentMeterValue = EnergyMeters(MeterNumber).CurTSValue;
    } else {
        CurrentMeterValue = 0.0;
    }

    return CurrentMeterValue;
}

Real64 GetInstantMeterValue(int const MeterNumber,                             // Which Meter Number (from GetMeterIndex)
                            OutputProcessor::TimeStepType const t_timeStepType // Whether this is zone of HVAC
)
{

    // FUNCTION INFORMATION:
    //       AUTHOR         Richard Liesen
    //       DATE WRITTEN   February 2003
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS FUNCTION:
    // This function returns the Instantaneous meter value (timestep) for the meter number indicated
    //  using TimeStepType to differentiate between Zone and HVAC values.

    // METHODOLOGY EMPLOYED:
    // Uses internal EnergyMeters structure to get value.

    // REFERENCES:
    // na

    // Using/Aliasing
    using namespace DataPrecisionGlobals;
    using namespace OutputProcessor;

    // Return value
    Real64 InstantMeterValue(0.0);

    // Locals
    // FUNCTION ARGUMENT DEFINITIONS:

    // FUNCTION PARAMETER DEFINITIONS:
    // na

    // INTERFACE BLOCK SPECIFICATIONS:
    // na

    // DERIVED TYPE DEFINITIONS:
    // na

    // FUNCTION LOCAL VARIABLE DECLARATIONS:

    //      EnergyMeters(Meter)%TSValue=EnergyMeters(EnergyMeters(Meter)%SourceMeter)%TSValue-MeterValue(Meter)

    if (MeterNumber == 0) return InstantMeterValue;

    auto &energy_meter(EnergyMeters(MeterNumber));
    auto &cache_beg(energy_meter.InstMeterCacheStart);
    auto &cache_end(energy_meter.InstMeterCacheEnd);
    if (energy_meter.TypeOfMeter != MeterType_CustomDec) {
        // section added to speed up the execution of this routine
        // instead of looping through all the VarMeterArrays to see if a RVariableType is used for a
        // specific meter, create a list of all the indexes for RVariableType that are used for that
        // meter.
        if (cache_beg == 0) { // not yet added to the cache
            for (int Loop = 1; Loop <= NumVarMeterArrays; ++Loop) {
                auto const &var_meter_on(VarMeterArrays(Loop).OnMeters);
                for (int Meter = 1, Meter_end = VarMeterArrays(Loop).NumOnMeters; Meter <= Meter_end; ++Meter) {
                    if (var_meter_on(Meter) == MeterNumber) {
                        IncrementInstMeterCache();
                        cache_end = InstMeterCacheLastUsed;
                        if (cache_beg == 0) cache_beg = InstMeterCacheLastUsed;
                        InstMeterCache(InstMeterCacheLastUsed) = VarMeterArrays(Loop).RepVariable;
                        break;
                    }
                }
                auto const &var_meter_on_custom(VarMeterArrays(Loop).OnCustomMeters);
                for (int Meter = 1, Meter_end = VarMeterArrays(Loop).NumOnCustomMeters; Meter <= Meter_end; ++Meter) {
                    if (var_meter_on_custom(Meter) == MeterNumber) {
                        IncrementInstMeterCache();
                        cache_end = InstMeterCacheLastUsed;
                        if (cache_beg == 0) cache_beg = InstMeterCacheLastUsed;
                        InstMeterCache(InstMeterCacheLastUsed) = VarMeterArrays(Loop).RepVariable;
                        break;
                    }
                } // End Number of Meters Loop
            }
        }
        for (int Loop = cache_beg; Loop <= cache_end; ++Loop) {
            auto &r_var_loop(RVariableTypes(InstMeterCache(Loop)));
            // Separate the Zone variables from the HVAC variables using TimeStepType
            if (r_var_loop.timeStepType == t_timeStepType) {
                auto &rVar(r_var_loop.VarPtr);
                // Add to the total all of the appropriate variables
                InstantMeterValue += (*rVar.Which) * rVar.ZoneMult * rVar.ZoneListMult;
            }
        }
    } else { // MeterType_CustomDec
        // Get Source Meter value
        // Loop through all report meters to find correct report variables to add to instant meter total
        for (int Loop = 1; Loop <= NumVarMeterArrays; ++Loop) {
            auto &r_var_loop(RVariableTypes(VarMeterArrays(Loop).RepVariable));

            auto const &var_meter_on(VarMeterArrays(Loop).OnMeters);
            for (int Meter = 1, Meter_end = VarMeterArrays(Loop).NumOnMeters; Meter <= Meter_end; ++Meter) {
                if (var_meter_on(Meter) == energy_meter.SourceMeter) {
                    // Separate the Zone variables from the HVAC variables using TimeStepType
                    if (r_var_loop.timeStepType == t_timeStepType) {
                        auto &rVar(r_var_loop.VarPtr);
                        // Add to the total all of the appropriate variables
                        InstantMeterValue += (*rVar.Which) * rVar.ZoneMult * rVar.ZoneListMult;
                        break;
                    }
                }
            }

            auto const &var_meter_on_custom(VarMeterArrays(Loop).OnCustomMeters);
            for (int Meter = 1, Meter_end = VarMeterArrays(Loop).NumOnCustomMeters; Meter <= Meter_end; ++Meter) {
                if (var_meter_on_custom(Meter) == energy_meter.SourceMeter) {
                    // Separate the Zone variables from the HVAC variables using TimeStepType
                    if (r_var_loop.timeStepType == t_timeStepType) {
                        auto &rVar(r_var_loop.VarPtr);
                        // Add to the total all of the appropriate variables
                        InstantMeterValue += (*rVar.Which) * rVar.ZoneMult * rVar.ZoneListMult;
                        break;
                    }
                }
            }

        } // End Number of Meters Loop
        for (int Loop = 1; Loop <= NumVarMeterArrays; ++Loop) {
            auto &r_var_loop(RVariableTypes(VarMeterArrays(Loop).RepVariable));

            auto const &var_meter_on(VarMeterArrays(Loop).OnMeters);
            for (int Meter = 1, Meter_end = VarMeterArrays(Loop).NumOnMeters; Meter <= Meter_end; ++Meter) {
                if (var_meter_on(Meter) == MeterNumber) {
                    // Separate the Zone variables from the HVAC variables using TimeStepType
                    if (r_var_loop.timeStepType == t_timeStepType) {
                        auto &rVar(r_var_loop.VarPtr);
                        // Add to the total all of the appropriate variables
                        InstantMeterValue -= (*rVar.Which) * rVar.ZoneMult * rVar.ZoneListMult;
                        break;
                    }
                }
            }

            auto const &var_meter_on_custom(VarMeterArrays(Loop).OnCustomMeters);
            for (int Meter = 1, Meter_end = VarMeterArrays(Loop).NumOnCustomMeters; Meter <= Meter_end; ++Meter) {
                if (var_meter_on_custom(Meter) == MeterNumber) {
                    // Separate the Zone variables from the HVAC variables using TimeStepType
                    if (r_var_loop.timeStepType == t_timeStepType) {
                        auto &rVar(r_var_loop.VarPtr);
                        // Add to the total all of the appropriate variables
                        InstantMeterValue -= (*rVar.Which) * rVar.ZoneMult * rVar.ZoneListMult;
                        break;
                    }
                }
            }

        } // End Number of Meters Loop
    }

    return InstantMeterValue;
}

void IncrementInstMeterCache()
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         Jason Glazer
    //       DATE WRITTEN   January 2013
    //       MODIFIED
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS FUNCTION:
    // Manage the InstMeterCache array

    // METHODOLOGY EMPLOYED:
    // When the array grows to large, double it.

    // REFERENCES:
    // na

    // Using/Aliasing
    using namespace OutputProcessor;

    if (!allocated(InstMeterCache)) {
        InstMeterCache.dimension(InstMeterCacheSizeInc, 0); // zero the entire array
        InstMeterCacheLastUsed = 1;
    } else {
        ++InstMeterCacheLastUsed;
        // if larger than current size grow the array
        if (InstMeterCacheLastUsed > InstMeterCacheSize) {
            InstMeterCache.redimension(InstMeterCacheSize += InstMeterCacheSizeInc, 0);
        }
    }
}

Real64 GetInternalVariableValue(int const varType,    // 1=integer, 2=real, 3=meter
                                int const keyVarIndex // Array index
)
{
    // FUNCTION INFORMATION:
    //       AUTHOR         Linda K. Lawrie
    //       DATE WRITTEN   December 2000
    //       MODIFIED       August 2003, M. J. Witte
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS FUNCTION:
    // This function returns the current value of the Internal Variable assigned to
    // the varType and keyVarIndex.  Values may be accessed for REAL(r64) and integer
    // report variables and meter variables.  The variable type (varType) may be
    // determined by calling subroutine and GetVariableKeyCountandType.  The
    // index (keyVarIndex) may be determined by calling subroutine GetVariableKeys.

    // METHODOLOGY EMPLOYED:
    // Uses Internal OutputProcessor data structure to return value.

    // REFERENCES:
    // na

    // Using/Aliasing
    using namespace DataPrecisionGlobals;
    using namespace OutputProcessor;
    using ScheduleManager::GetCurrentScheduleValue;

    // Return value
    Real64 resultVal; // value returned

    // Locals
    // FUNCTION ARGUMENT DEFINITIONS:

    // FUNCTION PARAMETER DEFINITIONS:
    // na

    // INTERFACE BLOCK SPECIFICATIONS:
    // na

    // DERIVED TYPE DEFINITIONS:
    // na

    // FUNCTION LOCAL VARIABLE DECLARATIONS:
    // na

    // Select based on variable type:  integer, real, or meter
    if (varType == 0) { // Variable not a found variable
        resultVal = 0.0;
    } else if (varType == 1) { // Integer
        if (keyVarIndex > NumOfIVariable) {
            ShowFatalError("GetInternalVariableValue: Integer variable passed index beyond range of array.");
            ShowContinueError("Index = " + General::TrimSigDigits(keyVarIndex) +
                              " Number of integer variables = " + General::TrimSigDigits(NumOfIVariable));
        }
        if (keyVarIndex < 1) {
            ShowFatalError("GetInternalVariableValue: Integer variable passed index <1. Index = " + General::TrimSigDigits(keyVarIndex));
        }

        // must use %Which, %Value is always zero if variable is not a requested report variable
        resultVal = double(*IVariableTypes(keyVarIndex).VarPtr.Which);
    } else if (varType == 2) { // real
        if (keyVarIndex > NumOfRVariable) {
            ShowFatalError("GetInternalVariableValue: Real variable passed index beyond range of array.");
            ShowContinueError("Index = " + General::TrimSigDigits(keyVarIndex) +
                              " Number of real variables = " + General::TrimSigDigits(NumOfRVariable));
        }
        if (keyVarIndex < 1) {
            ShowFatalError("GetInternalVariableValue: Integer variable passed index <1. Index = " + General::TrimSigDigits(keyVarIndex));
        }

        // must use %Which, %Value is always zero if variable is not a requested report variable
        resultVal = *RVariableTypes(keyVarIndex).VarPtr.Which;
    } else if (varType == 3) { // Meter
        resultVal = GetCurrentMeterValue(keyVarIndex);
    } else if (varType == 4) { // Schedule
        resultVal = GetCurrentScheduleValue(keyVarIndex);
    } else {
        resultVal = 0.0;
    }

    return resultVal;
}

Real64 GetInternalVariableValueExternalInterface(int const varType,    // 1=integer, 2=REAL(r64), 3=meter
                                                 int const keyVarIndex // Array index
)
{
    // FUNCTION INFORMATION:
    //       AUTHOR         Thierry S. Nouidui
    //       DATE WRITTEN   August 2011
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS FUNCTION:
    // This function returns the last zone-timestep value of the Internal Variable assigned to
    // the varType and keyVarIndex.  Values may be accessed for REAL(r64) and integer
    // report variables and meter variables.  The variable type (varType) may be
    // determined by calling subroutine and GetVariableKeyCountandType.  The
    // index (keyVarIndex) may be determined by calling subroutine GetVariableKeys.

    // METHODOLOGY EMPLOYED:
    // Uses Internal OutputProcessor data structure to return value.

    // REFERENCES:
    // na

    // Using/Aliasing
    using namespace DataPrecisionGlobals;
    using namespace OutputProcessor;
    using ScheduleManager::GetCurrentScheduleValue;

    // Return value
    Real64 resultVal; // value returned

    // Locals
    // FUNCTION ARGUMENT DEFINITIONS:

    // FUNCTION PARAMETER DEFINITIONS:
    // na

    // INTERFACE BLOCK SPECIFICATIONS:
    // na

    // DERIVED TYPE DEFINITIONS:
    // na

    // FUNCTION LOCAL VARIABLE DECLARATIONS:
    // na

    // Select based on variable type:  integer, REAL(r64), or meter
    if (varType == 0) { // Variable not a found variable
        resultVal = 0.0;
    } else if (varType == 1) { // Integer
        if (keyVarIndex > NumOfIVariable) {
            ShowFatalError("GetInternalVariableValueExternalInterface: passed index beyond range of array.");
        }
        if (keyVarIndex < 1) {
            ShowFatalError("GetInternalVariableValueExternalInterface: passed index beyond range of array.");
        }

        // must use %EITSValue, %This is the last-zonetimestep value
        resultVal = double(IVariableTypes(keyVarIndex).VarPtr.EITSValue);
    } else if (varType == 2) { // REAL(r64)
        if (keyVarIndex > NumOfRVariable) {
            ShowFatalError("GetInternalVariableValueExternalInterface: passed index beyond range of array.");
        }
        if (keyVarIndex < 1) {
            ShowFatalError("GetInternalVariableValueExternalInterface: passed index beyond range of array.");
        }

        // must use %EITSValue, %This is the last-zonetimestep value
        resultVal = RVariableTypes(keyVarIndex).VarPtr.EITSValue;
    } else if (varType == 3) { // Meter
        resultVal = GetCurrentMeterValue(keyVarIndex);
    } else if (varType == 4) { // Schedule
        resultVal = GetCurrentScheduleValue(keyVarIndex);
    } else {
        resultVal = 0.0;
    }

    return resultVal;
}

int GetNumMeteredVariables(std::string const &EP_UNUSED(ComponentType), // Given Component Type
                           std::string const &ComponentName             // Given Component Name (user defined)
)
{

    // FUNCTION INFORMATION:
    //       AUTHOR         Linda Lawrie
    //       DATE WRITTEN   May 2005
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS FUNCTION:
    // This function counts the number of metered variables associated with the
    // given ComponentType/Name.   This resultant number would then be used to
    // allocate arrays for a call the GetMeteredVariables routine.

    // METHODOLOGY EMPLOYED:
    // na

    // REFERENCES:
    // na

    // Using/Aliasing
    using namespace DataPrecisionGlobals;
    using namespace OutputProcessor;

    // Return value
    int NumVariables;

    // Locals
    // FUNCTION ARGUMENT DEFINITIONS:

    // FUNCTION PARAMETER DEFINITIONS:
    // na

    // INTERFACE BLOCK SPECIFICATIONS:
    // na

    // DERIVED TYPE DEFINITIONS:
    // na

    // FUNCTION LOCAL VARIABLE DECLARATIONS:
    int Loop;

    NumVariables = 0;
    for (Loop = 1; Loop <= NumOfRVariable; ++Loop) {
        //    Pos=INDEX(RVariableTypes(Loop)%VarName,':')
        //    IF (ComponentName /= RVariableTypes(Loop)%VarNameUC(1:Pos-1)) CYCLE
        if (ComponentName != RVariableTypes(Loop).KeyNameOnlyUC) continue;
        auto &rVar(RVariableTypes(Loop).VarPtr);
        if (rVar.MeterArrayPtr == 0) {
            continue;
        }
        if (VarMeterArrays(rVar.MeterArrayPtr).NumOnMeters > 0) {
            ++NumVariables;
        }
    }

    return NumVariables;
}

void GetMeteredVariables(std::string const &ComponentType,                      // Given Component Type
                         std::string const &ComponentName,                      // Given Component Name (user defined)
                         Array1D_int &VarIndexes,                               // Variable Numbers
                         Array1D_int &VarTypes,                                 // Variable Types (1=integer, 2=real, 3=meter)
                         Array1D<OutputProcessor::TimeStepType> &TimeStepTypes, // Variable Index Types (1=Zone,2=HVAC)
                         Array1D<OutputProcessor::Unit> &unitsForVar,           // units from enum for each variable
                         Array1D_int &ResourceTypes,                            // ResourceTypes for each variable
                         Array1D_string &EndUses,                               // EndUses for each variable
                         Array1D_string &Groups,                                // Groups for each variable
                         Array1D_string &Names,                                 // Variable Names for each variable
                         int &NumFound                                          // Number Found
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Linda Lawrie
    //       DATE WRITTEN   May 2005
    //       MODIFIED       Jason DeGraw 2/12/2020, de-optionalized
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // This routine gets the variable names and other associated information
    // for metered variables associated with the given ComponentType/Name.

    // Using/Aliasing
    using namespace DataPrecisionGlobals;
    using namespace DataGlobalConstants;
    using namespace OutputProcessor;

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    int Loop;
    int NumVariables;
    int MeterPtr;
    int NumOnMeterPtr;
    int MeterNum;

    NumVariables = 0;

    for (Loop = 1; Loop <= NumOfRVariable; ++Loop) {
        //    Pos=INDEX(RVariableTypes(Loop)%VarName,':')
        //    IF (ComponentName /= RVariableTypes(Loop)%VarNameUC(1:Pos-1)) CYCLE
        if (ComponentName != RVariableTypes(Loop).KeyNameOnlyUC) continue;
        auto &rVar(RVariableTypes(Loop).VarPtr);
        if (rVar.MeterArrayPtr == 0) continue;
        NumOnMeterPtr = VarMeterArrays(rVar.MeterArrayPtr).NumOnMeters;
        MeterPtr = VarMeterArrays(rVar.MeterArrayPtr).OnMeters(1);
        if (MeterPtr) {
            ++NumVariables;
            VarIndexes(NumVariables) = Loop;
            VarTypes(NumVariables) = 2;
            TimeStepTypes(NumVariables) = RVariableTypes(Loop).timeStepType;
            unitsForVar(NumVariables) = RVariableTypes(Loop).units;

            ResourceTypes(NumVariables) = AssignResourceTypeNum(UtilityRoutines::MakeUPPERCase(EnergyMeters(MeterPtr).ResourceType));

            Names(NumVariables) = RVariableTypes(Loop).VarNameUC;

            for (MeterNum = 1; MeterNum <= NumOnMeterPtr; ++MeterNum) {
                MeterPtr = VarMeterArrays(rVar.MeterArrayPtr).OnMeters(MeterNum);
                if (!EnergyMeters(MeterPtr).EndUse.empty()) {
                    EndUses(NumVariables) = UtilityRoutines::MakeUPPERCase(EnergyMeters(MeterPtr).EndUse);
                    break;
                }
            }

            for (MeterNum = 1; MeterNum <= NumOnMeterPtr; ++MeterNum) {
                MeterPtr = VarMeterArrays(rVar.MeterArrayPtr).OnMeters(MeterNum);
                if (!EnergyMeters(MeterPtr).Group.empty()) {
                    Groups(NumVariables) = UtilityRoutines::MakeUPPERCase(EnergyMeters(MeterPtr).Group);
                    break;
                }
            }

        } else {
            ShowWarningError("Referenced variable or meter used in the wrong context \"" + ComponentName + "\" of type \"" + ComponentType + "\"");
        }
    }

    NumFound = NumVariables; // Should just return this
}

void GetMeteredVariables(std::string const &ComponentType,                      // Given Component Type
                         std::string const &ComponentName,                      // Given Component Name (user defined)
                         Array1D_int &VarIndexes,                               // Variable Numbers
                         Array1D_int &VarTypes,                                 // Variable Types (1=integer, 2=real, 3=meter)
                         Array1D<OutputProcessor::TimeStepType> &TimeStepTypes, // Variable Index Types (1=Zone,2=HVAC)
                         Array1D<OutputProcessor::Unit> &unitsForVar,           // units from enum for each variable
                         Array1D_int &ResourceTypes,                            // ResourceTypes for each variable
                         Array1D_string &EndUses,                               // EndUses for each variable
                         Array1D_string &Groups,                                // Groups for each variable
                         Array1D_string &Names,                                 // Variable Names for each variable
                         Array1D_int &VarIDs                                    // Variable Report Numbers
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Linda Lawrie
    //       DATE WRITTEN   May 2005
    //       MODIFIED       Jason DeGraw 2/12/2020, de-optionalized
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // This routine gets the variable names and other associated information
    // for metered variables associated with the given ComponentType/Name.

    // Using/Aliasing
    using namespace DataPrecisionGlobals;
    using namespace DataGlobalConstants;
    using namespace OutputProcessor;

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    int Loop;
    int NumVariables;
    int MeterPtr;
    int NumOnMeterPtr;
    int MeterNum;

    NumVariables = 0;

    for (Loop = 1; Loop <= NumOfRVariable; ++Loop) {
        //    Pos=INDEX(RVariableTypes(Loop)%VarName,':')
        //    IF (ComponentName /= RVariableTypes(Loop)%VarNameUC(1:Pos-1)) CYCLE
        if (ComponentName != RVariableTypes(Loop).KeyNameOnlyUC) continue;
        auto &rVar(RVariableTypes(Loop).VarPtr);
        if (rVar.MeterArrayPtr == 0) continue;
        NumOnMeterPtr = VarMeterArrays(rVar.MeterArrayPtr).NumOnMeters;
        MeterPtr = VarMeterArrays(rVar.MeterArrayPtr).OnMeters(1);
        if (MeterPtr) {
            ++NumVariables;
            VarIndexes(NumVariables) = Loop;
            VarTypes(NumVariables) = 2;
            TimeStepTypes(NumVariables) = RVariableTypes(Loop).timeStepType;
            unitsForVar(NumVariables) = RVariableTypes(Loop).units;

            ResourceTypes(NumVariables) = AssignResourceTypeNum(UtilityRoutines::MakeUPPERCase(EnergyMeters(MeterPtr).ResourceType));
            Names(NumVariables) = RVariableTypes(Loop).VarNameUC;

            for (MeterNum = 1; MeterNum <= NumOnMeterPtr; ++MeterNum) {
                MeterPtr = VarMeterArrays(rVar.MeterArrayPtr).OnMeters(MeterNum);
                if (!EnergyMeters(MeterPtr).EndUse.empty()) {
                    EndUses(NumVariables) = UtilityRoutines::MakeUPPERCase(EnergyMeters(MeterPtr).EndUse);
                    break;
                }
            }

           for (MeterNum = 1; MeterNum <= NumOnMeterPtr; ++MeterNum) {
                MeterPtr = VarMeterArrays(rVar.MeterArrayPtr).OnMeters(MeterNum);
                if (!EnergyMeters(MeterPtr).Group.empty()) {
                    Groups(NumVariables) = UtilityRoutines::MakeUPPERCase(EnergyMeters(MeterPtr).Group);
                    break;
                }
            }

            VarIDs(NumVariables) = rVar.ReportID;

        } else {
            ShowWarningError("Referenced variable or meter used in the wrong context \"" + ComponentName + "\" of type \"" + ComponentType + "\"");
        }
    }

}

void GetVariableKeyCountandType(EnergyPlusData &state,
                                std::string const &varName,                 // Standard variable name
                                int &numKeys,                               // Number of keys found
                                int &varType,                               // 0=not found, 1=integer, 2=real, 3=meter
                                OutputProcessor::StoreType &varAvgSum,      // Variable  is Averaged=1 or Summed=2
                                OutputProcessor::TimeStepType &varStepType, // Variable time step is Zone=1 or HVAC=2
                                OutputProcessor::Unit &varUnits             // Units enumeration
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Michael J. Witte
    //       DATE WRITTEN   August 2003
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine returns the variable TYPE (Real, integer, meter, schedule, etc.)
    // (varType) whether it is an averaged or summed variable (varAvgSum),
    // whether it is a zone or HVAC time step (varStepType),
    // and the number of keynames for a given report variable or report meter name
    // (varName).  The variable type (varType) and number of keys (numKeys) are
    // used when calling subroutine GetVariableKeys to obtain a list of the
    // keynames for a particular variable and a corresponding list of indexes.

    // METHODOLOGY EMPLOYED:
    // Uses Internal OutputProcessor data structure to search for varName
    // in each of the three output data arrays:
    //       RVariableTypes - real report variables
    //       IVariableTypes - integer report variables
    //       EnergyMeters   - report meters (via GetMeterIndex function)
    //       Schedules      - specific schedule values
    // When the variable is found, the variable type (varType) is set and the
    // number of associated keys is counted.
    // varType is assigned as follows:
    //       0 = not found
    //       1 = integer
    //       2 = real
    //       3 = meter
    //       4 = schedule
    //  varAvgSum is assigned as follows:
    //       1 = averaged
    //       2 = summed
    //  varStepType is assigned as follows:
    //       1 = zone time step
    //       2 = HVAC time step

    // Using/Aliasing
    using namespace DataPrecisionGlobals;
    using namespace OutputProcessor;
    using ScheduleManager::GetScheduleIndex;
    using ScheduleManager::GetScheduleType;
    using SortAndStringUtilities::SetupAndSort;

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    //////////// hoisted into namespace ///////////////////////////////////////////////
    // static bool InitFlag( true ); // for initting the keyVarIndexes array
    ////////////////////////////////////////////////////////////////////////////////////
    int Loop; // Loop counters
    int Loop2;
    std::string::size_type Position; // Starting point of search string
    int VFound;                      // Found integer/real variable attributes
    bool Found;                      // True if varName is found
    bool Duplicate;                  // True if keyname is a duplicate
    std::string VarKeyPlusName;      // Full variable name including keyname and units
    std::string varNameUpper;        // varName pushed to all upper case

    // INITIALIZATIONS
    if (InitFlag) {
        curKeyVarIndexLimit = 1000;
        keyVarIndexes.allocate(curKeyVarIndexLimit);
        numVarNames = NumVariablesForOutput;
        varNames.allocate(numVarNames);
        for (Loop = 1; Loop <= NumVariablesForOutput; ++Loop) {
            varNames(Loop) = UtilityRoutines::MakeUPPERCase(DDVariableTypes(Loop).VarNameOnly);
        }
        ivarNames.allocate(numVarNames);
        SetupAndSort(varNames, ivarNames);
        InitFlag = false;
    }

    if (numVarNames != NumVariablesForOutput) {
        numVarNames = NumVariablesForOutput;
        varNames.allocate(numVarNames);
        for (Loop = 1; Loop <= NumVariablesForOutput; ++Loop) {
            varNames(Loop) = UtilityRoutines::MakeUPPERCase(DDVariableTypes(Loop).VarNameOnly);
        }
        ivarNames.allocate(numVarNames);
        SetupAndSort(varNames, ivarNames);
    }

    keyVarIndexes = 0;
    varType = VarType_NotFound;
    numKeys = 0;
    varAvgSum = StoreType::Averaged;
    varStepType = TimeStepType::TimeStepZone;
    varUnits = OutputProcessor::Unit::None;
    Found = false;
    Duplicate = false;
    varNameUpper = varName;

    // Search Variable List First
    VFound = UtilityRoutines::FindItemInSortedList(varNameUpper, varNames, numVarNames);
    if (VFound != 0) {
        varType = DDVariableTypes(ivarNames(VFound)).VariableType;
    }

    if (varType == VarType_Integer) {
        // Search Integer Variables
        for (Loop = 1; Loop <= NumOfIVariable; ++Loop) {
            VarKeyPlusName = IVariableTypes(Loop).VarNameUC;
            Position = index(VarKeyPlusName, ':' + varNameUpper, true);
            if (Position != std::string::npos) {
                if (VarKeyPlusName.substr(Position + 1) == varNameUpper) {
                    Found = true;
                    varType = VarType_Integer;
                    Duplicate = false;
                    // Check if duplicate - duplicates happen if the same report variable/key name
                    // combination is requested more than once in the idf at different reporting
                    // frequencies
                    for (Loop2 = 1; Loop2 <= numKeys; ++Loop2) {
                        if (VarKeyPlusName == IVariableTypes(keyVarIndexes(Loop2)).VarNameUC) Duplicate = true;
                    }
                    if (!Duplicate) {
                        ++numKeys;
                        if (numKeys > curKeyVarIndexLimit) {
                            keyVarIndexes.redimension(curKeyVarIndexLimit += 500, 0);
                        }
                        keyVarIndexes(numKeys) = Loop;
                        varAvgSum = DDVariableTypes(ivarNames(VFound)).storeType;
                        varStepType = DDVariableTypes(ivarNames(VFound)).timeStepType;
                        varUnits = DDVariableTypes(ivarNames(VFound)).units;
                    }
                }
            }
        }
    } else if (varType == VarType_Real) {
        // Search real Variables Next
        for (Loop = 1; Loop <= NumOfRVariable; ++Loop) {
            if (RVariableTypes(Loop).VarNameOnlyUC == varNameUpper) {
                Found = true;
                varType = VarType_Real;
                Duplicate = false;
                // Check if duplicate - duplicates happen if the same report variable/key name
                // combination is requested more than once in the idf at different reporting
                // frequencies
                VarKeyPlusName = RVariableTypes(Loop).VarNameUC;
                for (Loop2 = 1; Loop2 <= numKeys; ++Loop2) {
                    if (VarKeyPlusName == RVariableTypes(keyVarIndexes(Loop2)).VarNameUC) Duplicate = true;
                }
                if (!Duplicate) {
                    ++numKeys;
                    if (numKeys > curKeyVarIndexLimit) {
                        keyVarIndexes.redimension(curKeyVarIndexLimit += 500, 0);
                    }
                    keyVarIndexes(numKeys) = Loop;
                    varAvgSum = DDVariableTypes(ivarNames(VFound)).storeType;
                    varStepType = DDVariableTypes(ivarNames(VFound)).timeStepType;
                    varUnits = DDVariableTypes(ivarNames(VFound)).units;
                }
            }
        }
    }

    // Search Meters if not found in integers or reals
    // Use the GetMeterIndex function
    // Meters do not have keys, so only one will be found
    if (!Found) {
        keyVarIndexes(1) = GetMeterIndex(varName);
        if (keyVarIndexes(1) > 0) {
            Found = true;
            numKeys = 1;
            varType = VarType_Meter;
            varUnits = EnergyMeters(keyVarIndexes(1)).Units;
            varAvgSum = StoreType::Summed;
            varStepType = TimeStepType::TimeStepZone;
        }
    }

    // Search schedules if not found in integers, reals, or meters
    // Use the GetScheduleIndex function
    // Schedules do not have keys, so only one will be found
    if (!Found) {
        keyVarIndexes(1) = GetScheduleIndex(state, varName);
        if (keyVarIndexes(1) > 0) {
            Found = true;
            numKeys = 1;
            varType = VarType_Schedule;
            varUnits = unitStringToEnum(GetScheduleType(state, keyVarIndexes(1)));
            varAvgSum = StoreType::Averaged;
            varStepType = TimeStepType::TimeStepZone;
        }
    }
}

void GetVariableKeys(EnergyPlusData &state,
                     std::string const &varName, // Standard variable name
                     int const varType,          // 1=integer, 2=real, 3=meter
                     Array1D_string &keyNames,   // Specific key name
                     Array1D_int &keyVarIndexes  // Array index for
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Michael J. Witte
    //       DATE WRITTEN   August 2003
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine returns a list of keynames and indexes associated
    // with a particular report variable or report meter name (varName).
    // This routine assumes that the variable TYPE (Real, integer, meter, etc.)
    // may be determined by calling GetVariableKeyCountandType.  The variable type
    // and index can then be used with function GetInternalVariableValue to
    // to retrieve the current value of a particular variable/keyname combination.

    // METHODOLOGY EMPLOYED:
    // Uses Internal OutputProcessor data structure to search for varName
    // and build list of keynames and indexes.  The indexes are the array index
    // in the data array for the

    // Using/Aliasing
    using namespace DataPrecisionGlobals;
    using namespace OutputProcessor;
    using ScheduleManager::GetScheduleIndex;

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    int Loop; // Loop counters
    int Loop2;
    std::string::size_type Position; // Starting point of search string
    bool Duplicate;                  // True if keyname is a duplicate
    int maxKeyNames;                 // Max allowable # of key names=size of keyNames array
    int maxkeyVarIndexes;            // Max allowable # of key indexes=size of keyVarIndexes array
    int numKeys;                     // Number of keys found
    std::string VarKeyPlusName;      // Full variable name including keyname and units
    std::string varNameUpper;        // varName pushed to all upper case

    // INITIALIZATIONS
    keyNames = "";
    keyVarIndexes = 0;
    numKeys = 0;
    Duplicate = false;
    maxKeyNames = size(keyNames);
    maxkeyVarIndexes = size(keyVarIndexes);
    varNameUpper = UtilityRoutines::MakeUPPERCase(varName);

    // Select based on variable type:  integer, real, or meter
    if (varType == VarType_Integer) { // Integer
        for (Loop = 1; Loop <= NumOfIVariable; ++Loop) {
            VarKeyPlusName = IVariableTypes(Loop).VarNameUC;
            Position = index(VarKeyPlusName, ':' + varNameUpper, true);
            if (Position != std::string::npos) {
                if (VarKeyPlusName.substr(Position + 1) == varNameUpper) {
                    Duplicate = false;
                    // Check if duplicate - duplicates happen if the same report variable/key name
                    // combination is requested more than once in the idf at different reporting
                    // frequencies
                    for (Loop2 = 1; Loop2 <= numKeys; ++Loop2) {
                        if (VarKeyPlusName == IVariableTypes(keyVarIndexes(Loop2)).VarNameUC) Duplicate = true;
                    }
                    if (!Duplicate) {
                        ++numKeys;
                        if ((numKeys > maxKeyNames) || (numKeys > maxkeyVarIndexes)) {
                            ShowFatalError("Invalid array size in GetVariableKeys");
                        }
                        keyNames(numKeys) = IVariableTypes(Loop).VarNameUC.substr(0, Position);
                        keyVarIndexes(numKeys) = Loop;
                    }
                }
            }
        }
    } else if (varType == VarType_Real) { // Real
        for (Loop = 1; Loop <= NumOfRVariable; ++Loop) {
            if (RVariableTypes(Loop).VarNameOnlyUC == varNameUpper) {
                Duplicate = false;
                // Check if duplicate - duplicates happen if the same report variable/key name
                // combination is requested more than once in the idf at different reporting
                // frequencies
                VarKeyPlusName = RVariableTypes(Loop).VarNameUC;
                for (Loop2 = 1; Loop2 <= numKeys; ++Loop2) {
                    if (VarKeyPlusName == RVariableTypes(keyVarIndexes(Loop2)).VarNameUC) Duplicate = true;
                }
                if (!Duplicate) {
                    ++numKeys;
                    if ((numKeys > maxKeyNames) || (numKeys > maxkeyVarIndexes)) {
                        ShowFatalError("Invalid array size in GetVariableKeys");
                    }
                    keyNames(numKeys) = RVariableTypes(Loop).KeyNameOnlyUC;
                    keyVarIndexes(numKeys) = Loop;
                }
            }
        }
    } else if (varType == VarType_Meter) { // Meter
        numKeys = 1;
        if ((numKeys > maxKeyNames) || (numKeys > maxkeyVarIndexes)) {
            ShowFatalError("Invalid array size in GetVariableKeys");
        }
        keyNames(1) = "Meter";
        keyVarIndexes(1) = GetMeterIndex(varName);
    } else if (varType == VarType_Schedule) { // Schedule
        numKeys = 1;
        if ((numKeys > maxKeyNames) || (numKeys > maxkeyVarIndexes)) {
            ShowFatalError("Invalid array size in GetVariableKeys");
        }
        keyNames(1) = "Environment";
        keyVarIndexes(1) = GetScheduleIndex(state, varName);
    } else {
        // do nothing
    }
}

bool ReportingThisVariable(std::string const &RepVarName)
{

    // FUNCTION INFORMATION:
    //       AUTHOR         Linda Lawrie
    //       DATE WRITTEN   October 2008
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS FUNCTION:
    // This function scans the report variables and reports back
    // if user has requested this variable be reported.

    // METHODOLOGY EMPLOYED:
    // na

    // REFERENCES:
    // na

    // Using/Aliasing
    using namespace OutputProcessor;

    // Return value
    bool BeingReported;

    // Locals
    // FUNCTION ARGUMENT DEFINITIONS:

    // FUNCTION PARAMETER DEFINITIONS:
    // na

    // INTERFACE BLOCK SPECIFICATIONS:
    // na

    // DERIVED TYPE DEFINITIONS:
    // na

    // FUNCTION LOCAL VARIABLE DECLARATIONS:
    int Found;

    BeingReported = false;
    Found = UtilityRoutines::FindItem(RepVarName, ReqRepVars, &ReqReportVariables::VarName);
    if (Found > 0) {
        BeingReported = true;
    }

    if (!BeingReported) { // check meter names too
        Found = UtilityRoutines::FindItem(RepVarName, EnergyMeters);
        if (Found > 0) {
            if (EnergyMeters(Found).RptTS || EnergyMeters(Found).RptHR || EnergyMeters(Found).RptDY || EnergyMeters(Found).RptMN ||
                EnergyMeters(Found).RptSM || EnergyMeters(Found).RptTSFO || EnergyMeters(Found).RptHRFO || EnergyMeters(Found).RptDYFO ||
                EnergyMeters(Found).RptMNFO || EnergyMeters(Found).RptSMFO || EnergyMeters(Found).RptAccTS || EnergyMeters(Found).RptAccHR ||
                EnergyMeters(Found).RptAccDY || EnergyMeters(Found).RptAccMN || EnergyMeters(Found).RptAccSM || EnergyMeters(Found).RptAccTSFO ||
                EnergyMeters(Found).RptAccHRFO || EnergyMeters(Found).RptAccDYFO || EnergyMeters(Found).RptAccMNFO ||
                EnergyMeters(Found).RptAccSMFO) {
                BeingReported = true;
            }
        }
    }

    return BeingReported;
}

void InitPollutionMeterReporting(EnergyPlusData &state, std::string const &ReportFreqName)
{

    // SUBROUTINE INFORMATION:Richard Liesen
    //       DATE WRITTEN   July 2002
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine is called at the end of the first HVAC iteration and
    // sets up the reporting for the Pollution Meters.
    // ReportPollutionOutput,
    //   A1 ; \field Reporting_Frequency
    //        \type choice
    //        \key timestep
    //        \key hourly
    //        \key daily
    //        \key monthly
    //        \key runperiod
    // METHODOLOGY EMPLOYED:
    // The program tries to setup all of the following meters if the Pollution Report is initiated.
    //       Electricity:Facility [J]
    //       Diesel:Facility [J]
    //       DistrictCooling:Facility [J]
    //       DistrictHeating:Facility [J]
    //       Gas:Facility [J]
    //       GASOLINE:Facility [J]
    //       COAL:Facility [J]
    //       FuelOilNo1:Facility [J]
    //       FuelOilNo2:Facility [J]
    //       Propane:Facility [J]
    //       ElectricityProduced:Facility [J]
    //       Pollutant:CO2
    //       Pollutant:CO
    //       Pollutant:CH4
    //       Pollutant:NOx
    //       Pollutant:N2O
    //       Pollutant:SO2
    //       Pollutant:PM
    //       Pollutant:PM10
    //       Pollutant:PM2.5
    //       Pollutant:NH3
    //       Pollutant:NMVOC
    //       Pollutant:Hg
    //       Pollutant:Pb
    //       Pollutant:WaterEnvironmentalFactors
    //       Pollutant:Nuclear High
    //       Pollutant:Nuclear Low
    //       Pollutant:Carbon Equivalent

    // Using/Aliasing
    using namespace DataPrecisionGlobals;
    using namespace OutputProcessor;
    // SUBROUTINE PARAMETER DEFINITIONS:
    //             Now for the Pollution Meters
    static Array1D_string const PollutionMeters({1, 29},
                                                {"Electricity:Facility",
                                                 "Diesel:Facility",
                                                 "DistrictCooling:Facility",
                                                 "DistrictHeating:Facility",
                                                 "NaturalGas:Facility",
                                                 "GASOLINE:Facility",
                                                 "COAL:Facility",
                                                 "FuelOilNo1:Facility",
                                                 "FuelOilNo2:Facility",
                                                 "Propane:Facility",
                                                 "ElectricityProduced:Facility",
                                                 "Steam:Facility",
                                                 "CO2:Facility",
                                                 "CO:Facility",
                                                 "CH4:Facility",
                                                 "NOx:Facility",
                                                 "N2O:Facility",
                                                 "SO2:Facility",
                                                 "PM:Facility",
                                                 "PM10:Facility",
                                                 "PM2.5:Facility",
                                                 "NH3:Facility",
                                                 "NMVOC:Facility",
                                                 "Hg:Facility",
                                                 "Pb:Facility",
                                                 "WaterEnvironmentalFactors:Facility",
                                                 "Nuclear High:Facility",
                                                 "Nuclear Low:Facility",
                                                 "Carbon Equivalent:Facility"});

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    int Loop;
    int NumReqMeters;
    int Meter;
    ReportingFrequency ReportFreq(determineFrequency(ReportFreqName));

    int indexGroupKey;
    std::string indexGroup;

    NumReqMeters = 29;

    for (Loop = 1; Loop <= NumReqMeters; ++Loop) {

        Meter = UtilityRoutines::FindItem(PollutionMeters(Loop), EnergyMeters);
        if (Meter > 0) { // All the active meters for this run are set, but all are still searched for.

            indexGroupKey = DetermineIndexGroupKeyFromMeterName(EnergyMeters(Meter).Name);
            indexGroup = DetermineIndexGroupFromMeterGroup(EnergyMeters(Meter));
            // All of the specified meters are checked and the headers printed to the meter file if this
            //  has not been done previously
            if (ReportFreq == ReportingFrequency::TimeStep) {
                if (EnergyMeters(Meter).RptTS) {
                    EnergyMeters(Meter).RptTS = true;
                } else {
                    EnergyMeters(Meter).RptTS = true;
                    WriteMeterDictionaryItem(state,
                                             ReportFreq,
                                             StoreType::Summed,
                                             EnergyMeters(Meter).TSRptNum,
                                             indexGroupKey,
                                             indexGroup,
                                             EnergyMeters(Meter).TSRptNumChr,
                                             EnergyMeters(Meter).Name,
                                             EnergyMeters(Meter).Units,
                                             false,
                                             false);
                }
            } else if (ReportFreq == ReportingFrequency::Hourly) {
                if (EnergyMeters(Meter).RptHR) {
                    EnergyMeters(Meter).RptHR = true;
                    TrackingHourlyVariables = true;
                } else {
                    EnergyMeters(Meter).RptHR = true;
                    TrackingHourlyVariables = true;
                    WriteMeterDictionaryItem(state,
                                             ReportFreq,
                                             StoreType::Summed,
                                             EnergyMeters(Meter).HRRptNum,
                                             indexGroupKey,
                                             indexGroup,
                                             EnergyMeters(Meter).HRRptNumChr,
                                             EnergyMeters(Meter).Name,
                                             EnergyMeters(Meter).Units,
                                             false,
                                             false);
                }
            } else if (ReportFreq == ReportingFrequency::Daily) {
                if (EnergyMeters(Meter).RptDY) {
                    EnergyMeters(Meter).RptDY = true;
                    TrackingDailyVariables = true;
                } else {
                    EnergyMeters(Meter).RptDY = true;
                    TrackingDailyVariables = true;
                    WriteMeterDictionaryItem(state,
                                             ReportFreq,
                                             StoreType::Summed,
                                             EnergyMeters(Meter).DYRptNum,
                                             indexGroupKey,
                                             indexGroup,
                                             EnergyMeters(Meter).DYRptNumChr,
                                             EnergyMeters(Meter).Name,
                                             EnergyMeters(Meter).Units,
                                             false,
                                             false);
                }
            } else if (ReportFreq == ReportingFrequency::Monthly) {
                if (EnergyMeters(Meter).RptMN) {
                    EnergyMeters(Meter).RptMN = true;
                    TrackingMonthlyVariables = true;
                } else {
                    EnergyMeters(Meter).RptMN = true;
                    TrackingMonthlyVariables = true;
                    WriteMeterDictionaryItem(state,
                                             ReportFreq,
                                             StoreType::Summed,
                                             EnergyMeters(Meter).MNRptNum,
                                             indexGroupKey,
                                             indexGroup,
                                             EnergyMeters(Meter).MNRptNumChr,
                                             EnergyMeters(Meter).Name,
                                             EnergyMeters(Meter).Units,
                                             false,
                                             false);
                }
            } else if (ReportFreq == ReportingFrequency::Yearly) {
                if (EnergyMeters(Meter).RptYR) {
                    EnergyMeters(Meter).RptYR = true;
                    TrackingYearlyVariables = true;
                } else {
                    EnergyMeters(Meter).RptYR = true;
                    TrackingMonthlyVariables = true;
                    WriteMeterDictionaryItem(state,
                                             ReportFreq,
                                             StoreType::Summed,
                                             EnergyMeters(Meter).YRRptNum,
                                             indexGroupKey,
                                             indexGroup,
                                             EnergyMeters(Meter).YRRptNumChr,
                                             EnergyMeters(Meter).Name,
                                             EnergyMeters(Meter).Units,
                                             false,
                                             false);
                }
            } else if (ReportFreq == ReportingFrequency::Simulation) {
                if (EnergyMeters(Meter).RptSM) {
                    EnergyMeters(Meter).RptSM = true;
                    TrackingRunPeriodVariables = true;
                } else {
                    EnergyMeters(Meter).RptSM = true;
                    TrackingRunPeriodVariables = true;
                    WriteMeterDictionaryItem(state,
                                             ReportFreq,
                                             StoreType::Summed,
                                             EnergyMeters(Meter).SMRptNum,
                                             indexGroupKey,
                                             indexGroup,
                                             EnergyMeters(Meter).SMRptNumChr,
                                             EnergyMeters(Meter).Name,
                                             EnergyMeters(Meter).Units,
                                             false,
                                             false);
                }
            } else {
            }
        }
    }
}

void ProduceRDDMDD(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Linda Lawrie
    //       DATE WRITTEN   March 2009
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // provide a single call for writing out the Report Data Dictionary and Meter Data Dictionary.

    // Using/Aliasing
    using DataStringGlobals::IDDVerString;
    using DataStringGlobals::VerString;
    using namespace OutputProcessor;
    using General::ScanForReports;
    using SortAndStringUtilities::SetupAndSort;

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    std::string VarOption1;
    std::string VarOption2;
    bool DoReport;
    int Item;
    bool SortByName;
    int ItemPtr;

    struct VariableTypes
    {
        // Members
        int RealIntegerType; // Real= 1, Integer=2
        int VarPtr;          // pointer to real/integer VariableTypes structures
        int TimeStepType;
        int StoreType;
        std::string UnitsString;

        // Default Constructor
        VariableTypes() : RealIntegerType(0), VarPtr(0), TimeStepType(0), StoreType(0)
        {
        }
    };

    //  See if Report Variables should be turned on

    SortByName = false;
    ScanForReports(state, "VariableDictionary", DoReport, _, VarOption1, VarOption2);
    //  IF (.not. DoReport) RETURN

    if (DoReport) {
        ProduceReportVDD = ReportVDD_Yes;
        if (VarOption1 == std::string("IDF")) {
            ProduceReportVDD = ReportVDD_IDF;
        }
        if (!VarOption2.empty()) {
            if (UtilityRoutines::SameString(VarOption2, "Name") || UtilityRoutines::SameString(VarOption2, "AscendingName")) {
                SortByName = true;
            }
        }
    }

    state.files.rdd.ensure_open("ProduceRDDMDD", state.files.outputControl.rdd);
    state.files.mdd.ensure_open("ProduceRDDMDD", state.files.outputControl.mdd);
    if (ProduceReportVDD == ReportVDD_Yes) {
        print(state.files.rdd, "Program Version,{},{}{}", VerString, IDDVerString, '\n');
        print(state.files.rdd, "Var Type (reported time step),Var Report Type,Variable Name [Units]{}", '\n');

        print(state.files.mdd, "Program Version,{},{}{}", VerString, IDDVerString, '\n');
        print(state.files.mdd, "Var Type (reported time step),Var Report Type,Variable Name [Units]{}", '\n');
    } else if (ProduceReportVDD == ReportVDD_IDF) {
        print(state.files.rdd, "! Program Version,{},{}{}", VerString, IDDVerString, '\n');
        print(state.files.rdd, "! Output:Variable Objects (applicable to this run){}", '\n');

        print(state.files.mdd, "! Program Version,{},{}{}", VerString, IDDVerString, '\n');
        print(state.files.mdd, "! Output:Meter Objects (applicable to this run){}", '\n');
    }

    Array1D_string VariableNames(NumVariablesForOutput);
    for (int i = 1; i <= NumVariablesForOutput; ++i)
        VariableNames(i) = DDVariableTypes(i).VarNameOnly;
    Array1D_int iVariableNames(NumVariablesForOutput);

    if (SortByName) {
        SetupAndSort(VariableNames, iVariableNames);
    } else {
        for (Item = 1; Item <= NumVariablesForOutput; ++Item) {
            iVariableNames(Item) = Item;
        }
    }

    for (Item = 1; Item <= NumVariablesForOutput; ++Item) {
        if (ProduceReportVDD == ReportVDD_Yes) {
            ItemPtr = iVariableNames(Item);
            if (!DDVariableTypes(ItemPtr).ReportedOnDDFile) {
                print(state.files.rdd, "{},{},{}{}{}", StandardTimeStepTypeKey(DDVariableTypes(ItemPtr).timeStepType), standardVariableTypeKey(DDVariableTypes(ItemPtr).storeType), VariableNames(Item), unitStringFromDDitem(ItemPtr), '\n');
                ResultsFramework::resultsFramework->RDD.push_back(StandardTimeStepTypeKey(DDVariableTypes(ItemPtr).timeStepType) + "," +
                                                                  standardVariableTypeKey(DDVariableTypes(ItemPtr).storeType) + "," +
                                                                  VariableNames(Item) + unitStringFromDDitem(ItemPtr));
                DDVariableTypes(ItemPtr).ReportedOnDDFile = true;
                while (DDVariableTypes(ItemPtr).Next != 0) {
                    if (SortByName) {
                        ++ItemPtr;
                    } else {
                        ItemPtr = DDVariableTypes(ItemPtr).Next;
                    }
                    print(state.files.rdd, "{},{},{}{}{}", StandardTimeStepTypeKey(DDVariableTypes(ItemPtr).timeStepType), standardVariableTypeKey(DDVariableTypes(ItemPtr).storeType), VariableNames(Item), unitStringFromDDitem(ItemPtr), '\n');
                    ResultsFramework::resultsFramework->RDD.push_back(StandardTimeStepTypeKey(DDVariableTypes(ItemPtr).timeStepType) + "," +
                                                                      standardVariableTypeKey(DDVariableTypes(ItemPtr).storeType) + "," +
                                                                      VariableNames(Item) + unitStringFromDDitem(ItemPtr));
                    DDVariableTypes(ItemPtr).ReportedOnDDFile = true;
                }
            }
        } else if (ProduceReportVDD == ReportVDD_IDF) {
            ItemPtr = iVariableNames(Item);
            if (!DDVariableTypes(ItemPtr).ReportedOnDDFile) {
                print(state.files.rdd, "Output:Variable,*,{},hourly; !- {} {}{}{}", VariableNames(Item), StandardTimeStepTypeKey(DDVariableTypes(ItemPtr).timeStepType), standardVariableTypeKey(DDVariableTypes(ItemPtr).storeType), unitStringFromDDitem(ItemPtr), '\n');
                ResultsFramework::resultsFramework->RDD.push_back(StandardTimeStepTypeKey(DDVariableTypes(ItemPtr).timeStepType) + "," +
                                                                  standardVariableTypeKey(DDVariableTypes(ItemPtr).storeType) + "," +
                                                                  VariableNames(Item) + unitStringFromDDitem(ItemPtr));
                DDVariableTypes(ItemPtr).ReportedOnDDFile = true;
                while (DDVariableTypes(ItemPtr).Next != 0) {
                    if (SortByName) {
                        ++ItemPtr;
                    } else {
                        ItemPtr = DDVariableTypes(ItemPtr).Next;
                    }
                    print(state.files.rdd, "Output:Variable,*,{},hourly; !- {} {}{}{}", VariableNames(Item), StandardTimeStepTypeKey(DDVariableTypes(ItemPtr).timeStepType), standardVariableTypeKey(DDVariableTypes(ItemPtr).storeType), unitStringFromDDitem(ItemPtr), '\n');
                    ResultsFramework::resultsFramework->RDD.push_back(StandardTimeStepTypeKey(DDVariableTypes(ItemPtr).timeStepType) + "," +
                                                                      standardVariableTypeKey(DDVariableTypes(ItemPtr).storeType) + "," +
                                                                      VariableNames(Item) + unitStringFromDDitem(ItemPtr));
                    DDVariableTypes(ItemPtr).ReportedOnDDFile = true;
                }
            }
        }
    }
    state.files.rdd.close();

    //  Now EnergyMeter variables
    VariableNames.allocate(NumEnergyMeters);
    iVariableNames.allocate(NumEnergyMeters);
    if (SortByName) {
        for (Item = 1; Item <= NumEnergyMeters; ++Item) {
            VariableNames(Item) = EnergyMeters(Item).Name;
        }
        SetupAndSort(VariableNames, iVariableNames);
    } else {
        for (Item = 1; Item <= NumEnergyMeters; ++Item) {
            VariableNames(Item) = EnergyMeters(Item).Name;
            iVariableNames(Item) = Item;
        }
    }

    for (Item = 1; Item <= NumEnergyMeters; ++Item) {
        ItemPtr = iVariableNames(Item);
        if (ProduceReportVDD == ReportVDD_Yes) {
            print(state.files.mdd, "Zone,Meter,{}{}{}", EnergyMeters(ItemPtr).Name, unitEnumToStringBrackets(EnergyMeters(ItemPtr).Units), '\n');
            ResultsFramework::resultsFramework->MDD.push_back("Zone,Meter," + EnergyMeters(ItemPtr).Name +
                                                              unitEnumToStringBrackets(EnergyMeters(ItemPtr).Units));
        } else if (ProduceReportVDD == ReportVDD_IDF) {
            print(state.files.mdd, "Output:Meter,{},hourly; !-{}{}", EnergyMeters(ItemPtr).Name, unitEnumToStringBrackets(EnergyMeters(ItemPtr).Units), '\n');
            ResultsFramework::resultsFramework->MDD.push_back("Output:Meter," + EnergyMeters(ItemPtr).Name +
                                                              unitEnumToStringBrackets(EnergyMeters(ItemPtr).Units));
            print(state.files.mdd, "Output:Meter:Cumulative,{},hourly; !-{}{}", EnergyMeters(ItemPtr).Name, unitEnumToStringBrackets(EnergyMeters(ItemPtr).Units), '\n');
            ResultsFramework::resultsFramework->MDD.push_back("Output:Meter:Cumulative," + EnergyMeters(ItemPtr).Name +
                                                              unitEnumToStringBrackets(EnergyMeters(ItemPtr).Units));
        }
    }
    state.files.mdd.close();
}

void AddToOutputVariableList(std::string const &VarName, // Variable Name
                             OutputProcessor::TimeStepType const TimeStepType,
                             OutputProcessor::StoreType const StateType,
                             int const VariableType,
                             OutputProcessor::Unit const unitsForVar,
                             Optional_string_const customUnitName // the custom name for the units from EMS definition of units
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Linda Lawrie
    //       DATE WRITTEN   August 2010
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // This routine maintains a unique list of Output Variables for the
    // Variable Dictionary output.

    // METHODOLOGY EMPLOYED:
    // na

    // REFERENCES:
    // na

    // Using/Aliasing
    using namespace OutputProcessor;

    // Locals
    // SUBROUTINE ARGUMENT DEFINITIONS:

    // SUBROUTINE PARAMETER DEFINITIONS:
    // na

    // INTERFACE BLOCK SPECIFICATIONS:
    // na

    // DERIVED TYPE DEFINITIONS:
    // na

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:

    int dup = 0; // for duplicate variable name
    if (NumVariablesForOutput > 0) {
        dup = UtilityRoutines::FindItemInList(VarName, DDVariableTypes, &VariableTypeForDDOutput::VarNameOnly, NumVariablesForOutput);
    } else {
        DDVariableTypes.allocate(LVarAllocInc);
        MaxVariablesForOutput = LVarAllocInc;
    }
    if (dup == 0) {
        ++NumVariablesForOutput;
        if (NumVariablesForOutput > MaxVariablesForOutput) {
            DDVariableTypes.redimension(MaxVariablesForOutput += LVarAllocInc);
        }
        DDVariableTypes(NumVariablesForOutput).timeStepType = TimeStepType;
        DDVariableTypes(NumVariablesForOutput).storeType = StateType;
        DDVariableTypes(NumVariablesForOutput).VariableType = VariableType;
        DDVariableTypes(NumVariablesForOutput).VarNameOnly = VarName;
        DDVariableTypes(NumVariablesForOutput).units = unitsForVar;
        if (present(customUnitName) && unitsForVar == OutputProcessor::Unit::customEMS) {
            DDVariableTypes(NumVariablesForOutput).unitNameCustomEMS = customUnitName;
        }
    } else if (unitsForVar != DDVariableTypes(dup).units) { // not the same as first units
        int dup2 = 0;                                       // for duplicate variable name
        while (DDVariableTypes(dup).Next != 0) {
            if (unitsForVar != DDVariableTypes(DDVariableTypes(dup).Next).units) {
                dup = DDVariableTypes(dup).Next;
                continue;
            }
            dup2 = DDVariableTypes(dup).Next;
            break;
        }
        if (dup2 == 0) {
            ++NumVariablesForOutput;
            if (NumVariablesForOutput > MaxVariablesForOutput) {
                DDVariableTypes.redimension(MaxVariablesForOutput += LVarAllocInc);
            }
            DDVariableTypes(NumVariablesForOutput).timeStepType = TimeStepType;
            DDVariableTypes(NumVariablesForOutput).storeType = StateType;
            DDVariableTypes(NumVariablesForOutput).VariableType = VariableType;
            DDVariableTypes(NumVariablesForOutput).VarNameOnly = VarName;
            DDVariableTypes(NumVariablesForOutput).units = unitsForVar;
            if (present(customUnitName) && unitsForVar == OutputProcessor::Unit::customEMS) {
                DDVariableTypes(NumVariablesForOutput).unitNameCustomEMS = customUnitName;
            }
            DDVariableTypes(dup).Next = NumVariablesForOutput;
        }
    }
}

int initErrorFile(EnergyPlusData &state)
{
    state.files.err_stream = std::unique_ptr<std::ostream>(new std::ofstream(state.files.outputErrFileName));
    if (state.files.err_stream->bad()) {
        DisplayString("ERROR: Could not open file " + state.files.outputErrFileName + " for output (write).");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

} // namespace EnergyPlus
