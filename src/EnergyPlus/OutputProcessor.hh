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

#ifndef OutputProcessor_hh_INCLUDED
#define OutputProcessor_hh_INCLUDED

// C++ Headers
#include <iosfwd>
#include <map>

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>
#include <ObjexxFCL/Array2D.hh>
#include <ObjexxFCL/Optional.hh>
#include <ObjexxFCL/Reference.hh>

// EnergyPlus Headers
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/EnergyPlus.hh>
#include <EnergyPlus/DisplayRoutines.hh>

namespace EnergyPlus {

// Forward declarations
class InputOutputFile;
struct EnergyPlusData;

namespace OutputProcessor {

    // Using/Aliasing

    // Data
    // in this file should obey a USE OutputProcessor, ONLY: rule.

    // MODULE PARAMETER DEFINITIONS:
    extern int const ReportVDD_No;  // Don't report the variable dictionaries in any form
    extern int const ReportVDD_Yes; // Report the variable dictionaries in "report format"
    extern int const ReportVDD_IDF; // Report the variable dictionaries in "IDF format"

    extern Real64 const MinSetValue;
    extern Real64 const MaxSetValue;
    extern int const IMinSetValue;
    extern int const IMaxSetValue;

    extern int const VarType_NotFound; // ref: GetVariableKeyCountandType, 0 = not found
    extern int const VarType_Integer;  // ref: GetVariableKeyCountandType, 1 = integer
    extern int const VarType_Real;     // ref: GetVariableKeyCountandType, 2 = real
    extern int const VarType_Meter;    // ref: GetVariableKeyCountandType, 3 = meter
    extern int const VarType_Schedule; // ref: GetVariableKeyCountandType, 4 = schedule

    extern int const MeterType_Normal;     // Type value for normal meters
    extern int const MeterType_Custom;     // Type value for custom meters
    extern int const MeterType_CustomDec;  // Type value for custom meters that decrement another meter
    extern int const MeterType_CustomDiff; // Type value for custom meters that difference another meter

    extern Array1D_string const DayTypes;
    extern int const UnitsStringLength;

    extern int const RVarAllocInc;
    extern int const LVarAllocInc;
    extern int const IVarAllocInc;

    //  For IP Units (tabular reports) certain resources will be put in sub-tables
    // INTEGER, PARAMETER :: RT_IPUnits_Consumption=0
    extern int const RT_IPUnits_Electricity;
    extern int const RT_IPUnits_Gas;
    extern int const RT_IPUnits_Cooling;
    extern int const RT_IPUnits_Water;
    extern int const RT_IPUnits_OtherKG;
    extern int const RT_IPUnits_OtherM3;
    extern int const RT_IPUnits_OtherL;
    extern int const RT_IPUnits_OtherJ;

    // DERIVED TYPE DEFINITIONS:

    extern int InstMeterCacheSize;     // the maximum size of the instant meter cache used in GetInstantMeterValue
    extern int InstMeterCacheSizeInc;  // the increment for the instant meter cache used in GetInstantMeterValue
    extern Array1D_int InstMeterCache; // contains a list of RVariableTypes that make up a specific meter
    extern int InstMeterCacheLastUsed; // the last item in the instant meter cache used

    // INTERFACE BLOCK SPECIFICATIONS:
    // na

    // MODULE VARIABLE DECLARATIONS:

    extern int CurrentReportNumber;
    extern int NumVariablesForOutput;
    extern int MaxVariablesForOutput;
    extern int NumOfRVariable_Setup;
    extern int NumTotalRVariable;
    extern int NumOfRVariable_Sum;
    extern int NumOfRVariable_Meter;
    extern int NumOfRVariable;
    extern int MaxRVariable;
    extern int NumOfIVariable_Setup;
    extern int NumTotalIVariable;
    extern int NumOfIVariable_Sum;
    extern int NumOfIVariable;
    extern int MaxIVariable;
    extern bool OutputInitialized;
    extern int ProduceReportVDD;
    extern int NumHoursInDay;
    extern int NumHoursInMonth;
    extern int NumHoursInSim;
    extern Array1D_int ReportList;
    extern int NumReportList;
    extern int NumExtraVars;

    extern int NumOfReqVariables; // Current number of Requested Report Variables

    extern int NumVarMeterArrays; // Current number of Arrays pointing to meters

    extern int NumEnergyMeters;        // Current number of Energy Meters
    extern Array1D<Real64> MeterValue; // This holds the current timestep value for each meter.

    extern int TimeStepStampReportNbr;          // TimeStep and Hourly Report number
    extern std::string TimeStepStampReportChr;  // TimeStep and Hourly Report number (character -- for printing)
    extern bool TrackingHourlyVariables;        // Requested Hourly Report Variables
    extern int DailyStampReportNbr;             // Daily Report number
    extern std::string DailyStampReportChr;     // Daily Report number (character -- for printing)
    extern bool TrackingDailyVariables;         // Requested Daily Report Variables
    extern int MonthlyStampReportNbr;           // Monthly Report number
    extern std::string MonthlyStampReportChr;   // Monthly Report number (character -- for printing)
    extern bool TrackingMonthlyVariables;       // Requested Monthly Report Variables
    extern int YearlyStampReportNbr;            // Monthly Report number
    extern std::string YearlyStampReportChr;    // Monthly Report number (character -- for printing)
    extern bool TrackingYearlyVariables;        // Requested Yearly Report Variables
    extern int RunPeriodStampReportNbr;         // RunPeriod Report number
    extern std::string RunPeriodStampReportChr; // RunPeriod Report number (character -- for printing)
    extern bool TrackingRunPeriodVariables;     // Requested RunPeriod Report Variables
    extern Real64 TimeStepZoneSec;              // Seconds from NumTimeStepInHour
    extern bool ErrorsLogged;
    extern bool ProduceVariableDictionary;

    extern int MaxNumSubcategories;
    extern bool isFinalYear;

    extern bool GetOutputInputFlag; // First time, input is "gotten"

    // All routines should be listed here whether private or not
    // PUBLIC  ReallocateTVar
    // PUBLIC  SetReportNow

    // Types
    enum class Unit
    {
        kg_s,
        C,
        kgWater_kgDryAir,
        ppm,
        Pa,
        m3_s,
        None,
        min,
        W,
        J,
        m3,
        kg,
        ach,
        W_W,
        lux,
        lum_W,
        hr,
        cd_m2,
        J_kgWater,
        m_s,
        W_m2,
        m,
        Ah,
        A,
        V,
        deltaC,
        kmol_s,
        rev_min,
        Btu_h_W,
        W_m2K,
        J_kg,
        kg_kg,
        Perc,
        deg,
        s,
        kg_m3,
        kg_m2s,
        J_kgK,
        L,
        K_m,
        m2,
        W_m2C,
        rad,
        J_m2,
        clo,
        W_K,
        K_W,
        kgWater_s,
        unknown,
        customEMS
    };

    enum class ReportingFrequency
    {
        EachCall = -1, // Write out each time UpdatedataandReport is called
        TimeStep,      // Write out at 'EndTimeStepFlag'
        Hourly,        // Write out at 'EndHourFlag'
        Daily,         // Write out at 'EndDayFlag'
        Monthly,       // Write out at end of month (must be determined)
        Simulation,    // Write out once per environment 'EndEnvrnFlag'
        Yearly         // Write out at 'EndYearFlag'
    };

    extern ReportingFrequency minimumReportFrequency;

    enum class StoreType
    {
        Averaged = 1, // Type value for "averaged" variables
        Summed        // Type value for "summed" variables
    };

    enum class TimeStepType
    {
        TimeStepZone = 1,      // Type value for "zone" timestep variables
        TimeStepSystem = 2,    // Type value for "system" timestep variables
    };

    struct TimeSteps
    {
        // Members
        Real64 * TimeStep; // fortran POINTER Pointer to the Actual Time Step Variable (Zone or HVAC)
        Real64 CurMinute;           // Current minute (decoded from real Time Step Value)

        // Default Constructor
        TimeSteps()
            : CurMinute(0.0)
        {
        }
    };

    struct RealVariables
    {
        // Members
        Real64 * Which; // The POINTER to the actual variable holding the value
        Real64 Value;            // Current Value of the variable (to resolution of Zone Time Step)
        Real64 TSValue;          // Value of this variable at the Zone Time Step
        Real64 EITSValue;        // Value of this variable at the Zone Time Step for external interface
        Real64 StoreValue;       // At end of Zone Time Step, value is placed here for later reporting
        Real64 NumStored;        // Number of hours stored
        StoreType storeType;     // Variable Type (Summed/Non-Static or Average/Static)
        bool Stored;             // True when value is stored
        bool Report;             // User has requested reporting of this variable in the IDF
        bool tsStored;           // if stored for this zone timestep
        bool thisTSStored;       // if stored for this zone timestep
        int thisTSCount;
        ReportingFrequency frequency; // How often to report this variable
        Real64 MaxValue;              // Maximum reporting (only for Averaged variables, and those greater than Time Step)
        int maxValueDate;             // Date stamp of maximum
        Real64 MinValue;              // Minimum reporting (only for Averaged variables, and those greater than Time Step)
        int minValueDate;             // Date stamp of minimum
        int ReportID;                 // Report variable ID number
        std::string ReportIDChr;      // Report variable ID number (character -- for printing)
        int SchedPtr;                 // If scheduled, this points to the schedule
        int MeterArrayPtr;            // If metered, this points to an array of applicable meters
        int ZoneMult;                 // If metered, Zone Multiplier is applied
        int ZoneListMult;             // If metered, Zone List Multiplier is applied

        // Default Constructor
        RealVariables()
            : Value(0.0), TSValue(0.0), EITSValue(0.0), StoreValue(0.0), NumStored(0.0), storeType(StoreType::Averaged), Stored(false), Report(false),
              tsStored(false), thisTSStored(false), thisTSCount(0), frequency(ReportingFrequency::Hourly), MaxValue(-9999.0), maxValueDate(0),
              MinValue(9999.0), minValueDate(0), ReportID(0), SchedPtr(0), MeterArrayPtr(0), ZoneMult(1), ZoneListMult(1)
        {
        }
    };

    struct IntegerVariables
    {
        // Members
        int * Which; // The POINTER to the actual variable holding the value
        Real64 Value;        // Current Value of the variable (to resolution of Zone Time Step)
        Real64 TSValue;      // Value of this variable at the Zone Time Step
        Real64 EITSValue;    // Value of this variable at the Zone Time Step for external interface
        Real64 StoreValue;   // At end of Zone Time Step, value is placed here for later reporting
        Real64 NumStored;    // Number of hours stored
        StoreType storeType; // Variable Type (Summed/Non-Static or Average/Static)
        bool Stored;         // True when value is stored
        bool Report;         // User has requested reporting of this variable in the IDF
        bool tsStored;       // if stored for this zone timestep
        bool thisTSStored;   // if stored for this zone timestep
        int thisTSCount;
        ReportingFrequency frequency; // How often to report this variable
        int MaxValue;                 // Maximum reporting (only for Averaged variables, and those greater than Time Step)
        int maxValueDate;             // Date stamp of maximum
        int MinValue;                 // Minimum reporting (only for Averaged variables, and those greater than Time Step)
        int minValueDate;             // Date stamp of minimum
        int ReportID;                 // Report variable ID number
        std::string ReportIDChr;      // Report variable ID number (character -- for printing)
        int SchedPtr;                 // If scheduled, this points to the schedule

        // Default Constructor
        IntegerVariables()
            : Value(0.0), TSValue(0.0), EITSValue(0.0), StoreValue(0.0), NumStored(0.0), storeType(StoreType::Averaged), Stored(false), Report(false),
              tsStored(false), thisTSStored(false), thisTSCount(0), frequency(ReportingFrequency::Hourly), MaxValue(-9999), maxValueDate(0),
              MinValue(9999), minValueDate(0), ReportID(0), SchedPtr(0)
        {
        }
    };

    struct VariableTypeForDDOutput
    {
        // Members
        TimeStepType timeStepType;        // Type whether Zone or HVAC
        StoreType storeType;           // Variable Type (Summed/Non-Static or Average/Static)
        int VariableType;              // Integer, Real.
        int Next;                      // Next variable of same name (different units)
        bool ReportedOnDDFile;         // true after written to .rdd/.mdd file
        std::string VarNameOnly;       // Name of Variable
        OutputProcessor::Unit units;   // Units for Variable
        std::string unitNameCustomEMS; // name of units when customEMS is used for EMS variables that are unusual

        // Default Constructor
        VariableTypeForDDOutput()
            : timeStepType(TimeStepType::TimeStepZone), storeType(StoreType::Averaged), VariableType(VarType_NotFound), Next(0), ReportedOnDDFile(false),
              units(OutputProcessor::Unit::None)
        {
        }
    };

    struct RealVariableType
    {
        // Members
        TimeStepType timeStepType;          // Type whether Zone or HVAC
        StoreType storeType;             // Variable Type (Summed/Non-Static or Average/Static)
        int ReportID;                    // Report variable ID number
        std::string VarName;             // Name of Variable key:variable
        std::string VarNameUC;           // Name of Variable (Uppercase)
        std::string VarNameOnly;         // Name of Variable
        std::string VarNameOnlyUC;       // Name of Variable with out key in uppercase
        std::string KeyNameOnlyUC;       // Name of key only witht out variable in uppercase
        OutputProcessor::Unit units;     // Units for Variable
        std::string unitNameCustomEMS;   // name of units when customEMS is used for EMS variables that are unusual
        RealVariables VarPtr;            // Pointer used to real Variables structure

        // Default Constructor
        RealVariableType() : timeStepType(TimeStepType::TimeStepZone), storeType(StoreType::Averaged), ReportID(0), units(OutputProcessor::Unit::None)
        {
        }
    };

    struct IntegerVariableType
    {
        // Members
        TimeStepType timeStepType;             // Type whether Zone or HVAC
        StoreType storeType;                // Variable Type (Summed/Non-Static or Average/Static)
        int ReportID;                       // Report variable ID number
        std::string VarName;                // Name of Variable
        std::string VarNameUC;              // Name of Variable
        std::string VarNameOnly;            // Name of Variable
        std::string VarNameOnlyUC;       // Name of Variable with out key in uppercase
        std::string KeyNameOnlyUC;       // Name of key only witht out variable in uppercase
        OutputProcessor::Unit units;        // Units for Variable
        IntegerVariables VarPtr;            // Pointer used to integer Variables structure

        // Default Constructor
        IntegerVariableType() : timeStepType(TimeStepType::TimeStepZone), storeType(StoreType::Averaged), ReportID(0), units(OutputProcessor::Unit::None)
        {
        }
    };

    struct ReqReportVariables // Structure for requested Report Variables
    {
        // Members
        std::string Key;              // Could be blank or "*"
        std::string VarName;          // Name of Variable
        ReportingFrequency frequency; // Reporting Frequency
        int SchedPtr;                 // Index of the Schedule
        std::string SchedName;        // Schedule Name
        bool Used;                    // True when this combination (key, varname, frequency) has been set

        // Default Constructor
        ReqReportVariables() : frequency(ReportingFrequency::Hourly), SchedPtr(0), Used(false)
        {
        }
    };

    struct MeterArrayType
    {
        // Members
        int NumOnMeters;            // Number of OnMeter Entries for variable
        int RepVariable;            // Backwards pointer to real Variable
        Array1D_int OnMeters;       // Forward pointer to Meter Numbers
        int NumOnCustomMeters;      // Number of OnCustomMeter Entries for variable
        Array1D_int OnCustomMeters; // Forward pointer to Custom Meter Numbers

        // Default Constructor
        MeterArrayType() : NumOnMeters(0), RepVariable(0), OnMeters(6, 0), NumOnCustomMeters(0)
        {
        }
    };

    struct MeterType
    {
        // Members
        std::string Name;            // Name of the meter
        std::string ResourceType;    // Resource Type of the meter
        std::string EndUse;          // End Use of the meter
        std::string EndUseSub;       // End Use subcategory of the meter
        std::string Group;           // Group of the meter
        OutputProcessor::Unit Units; // Units for the Meter
        int RT_forIPUnits;           // Resource type number for IP Units (tabular) reporting
        int TypeOfMeter;             // type of meter
        int SourceMeter;             // for custom decrement meters, this is the meter number for the subtraction

        Real64 TSValue;          // TimeStep Value
        Real64 CurTSValue;       // Current TimeStep Value (internal access)
        bool RptTS;              // Report at End of TimeStep (Zone)
        bool RptTSFO;            // Report at End of TimeStep (Zone) -- meter file only
        int TSRptNum;            // Report Number for TS Values
        std::string TSRptNumChr; // Report Number for TS Values (character -- for printing)

        Real64 HRValue;          // Hourly Value
        bool RptHR;              // Report at End of Hour
        bool RptHRFO;            // Report at End of Hour -- meter file only
        Real64 HRMaxVal;         // Maximum Value (Hour)
        int HRMaxValDate;        // Date stamp of maximum
        Real64 HRMinVal;         // Minimum Value (Hour)
        int HRMinValDate;        // Date stamp of minimum
        int HRRptNum;            // Report Number for HR Values
        std::string HRRptNumChr; // Report Number for HR Values (character -- for printing)

        Real64 DYValue;          // Daily Value
        bool RptDY;              // Report at End of Day
        bool RptDYFO;            // Report at End of Day -- meter file only
        Real64 DYMaxVal;         // Maximum Value (Day)
        int DYMaxValDate;        // Date stamp of maximum
        Real64 DYMinVal;         // Minimum Value (Day)
        int DYMinValDate;        // Date stamp of minimum
        int DYRptNum;            // Report Number for DY Values
        std::string DYRptNumChr; // Report Number for DY Values (character -- for printing)

        Real64 MNValue;          // Monthly Value
        bool RptMN;              // Report at End of Month
        bool RptMNFO;            // Report at End of Month -- meter file only
        Real64 MNMaxVal;         // Maximum Value (Month)
        int MNMaxValDate;        // Date stamp of maximum
        Real64 MNMinVal;         // Minimum Value (Month)
        int MNMinValDate;        // Date stamp of minimum
        int MNRptNum;            // Report Number for MN Values
        std::string MNRptNumChr; // Report Number for MN Values (character -- for printing)

        Real64 YRValue;          // Yearly Value
        bool RptYR;              // Report at End of Year
        bool RptYRFO;            // Report at End of Year
        Real64 YRMaxVal;         // Maximum Value (Yearly)
        int YRMaxValDate;        // Date stamp of maximum
        Real64 YRMinVal;         // Minimum Value (Yearly)
        int YRMinValDate;        // Date stamp of minimum
        int YRRptNum;            // Report Number for YR Values
        std::string YRRptNumChr; // Report Number for YR Values (character -- for printing)

        Real64 SMValue;          // Simulation Value
        bool RptSM;              // Report at End of Environment/Simulation
        bool RptSMFO;            // Report at End of Environment/Simulation -- meter file only
        Real64 SMMaxVal;         // Maximum Value (Sim)
        int SMMaxValDate;        // Date stamp of maximum
        Real64 SMMinVal;         // Minimum Value (Sim)
        int SMMinValDate;        // Date stamp of minimum
        int SMRptNum;            // Report Number for SM Values
        std::string SMRptNumChr; // Report Number for SM Values (character -- for printing)

        Real64 LastSMValue;      // Simulation Value
        Real64 LastSMMaxVal;     // Maximum Value (Sim)
        int LastSMMaxValDate;    // Date stamp of maximum
        Real64 LastSMMinVal;     // Minimum Value (Sim)
        int LastSMMinValDate;    // Date stamp of minimum
        Real64 FinYrSMValue;     // Final Year Simulation Value
        Real64 FinYrSMMaxVal;    // Maximum Value (Sim)
        int FinYrSMMaxValDate;   // Date stamp of maximum
        Real64 FinYrSMMinVal;    // Minimum Value (Sim)
        int FinYrSMMinValDate;   // Date stamp of minimum
        bool RptAccTS;           // Report Cumulative Meter at Time Step
        bool RptAccTSFO;         // Report Cumulative Meter at Time Step -- meter file only
        bool RptAccHR;           // Report Cumulative Meter at Hour
        bool RptAccHRFO;         // Report Cumulative Meter at Hour -- meter file only
        bool RptAccDY;           // Report Cumulative Meter at Day
        bool RptAccDYFO;         // Report Cumulative Meter at Day -- meter file only
        bool RptAccMN;           // Report Cumulative Meter at Month
        bool RptAccMNFO;         // Report Cumulative Meter at Month -- meter file only
        bool RptAccYR;           // Report Cumulative Meter at Year
        bool RptAccYRFO;         // Report Cumulative Meter at Year -- meter file only
        bool RptAccSM;           // Report Cumulative Meter at Run Period
        bool RptAccSMFO;         // Report Cumulative Meter at Run Period -- meter file only
        int TSAccRptNum;         // Report Number for Acc Values
        int HRAccRptNum;         // Report Number for Acc Values
        int DYAccRptNum;         // Report Number for Acc Values
        int MNAccRptNum;         // Report Number for Acc Values
        int YRAccRptNum;         // Report Number for Acc Values
        int SMAccRptNum;         // Report Number for Acc Values
        int InstMeterCacheStart; // index of the beginning of the instant meter cache
        int InstMeterCacheEnd;   // index of the end of the instant meter cache

        // Default Constructor
        MeterType()
            : Units(OutputProcessor::Unit::None), RT_forIPUnits(0), TypeOfMeter(MeterType_Normal), SourceMeter(0), TSValue(0.0), CurTSValue(0.0),
              RptTS(false), RptTSFO(false), TSRptNum(0), HRValue(0.0), RptHR(false), RptHRFO(false), HRMaxVal(-99999.0), HRMaxValDate(0),
              HRMinVal(99999.0), HRMinValDate(0), HRRptNum(0), DYValue(0.0), RptDY(false), RptDYFO(false), DYMaxVal(-99999.0), DYMaxValDate(0),
              DYMinVal(99999.0), DYMinValDate(0), DYRptNum(0), MNValue(0.0), RptMN(false), RptMNFO(false), MNMaxVal(-99999.0), MNMaxValDate(0),
              MNMinVal(99999.0), MNMinValDate(0), MNRptNum(0), YRValue(0.0), RptYR(false), RptYRFO(false), YRMaxVal(-99999.0), YRMaxValDate(0),
              YRMinVal(99999.0), YRMinValDate(0), YRRptNum(0), SMValue(0.0), RptSM(false), RptSMFO(false), SMMaxVal(-99999.0), SMMaxValDate(0),
              SMMinVal(99999.0), SMMinValDate(0), SMRptNum(0), LastSMValue(0.0), LastSMMaxVal(-99999.0), LastSMMaxValDate(0), LastSMMinVal(99999.0),
              LastSMMinValDate(0), FinYrSMValue(0.0), FinYrSMMaxVal(-99999.0), FinYrSMMaxValDate(0), FinYrSMMinVal(99999.0), FinYrSMMinValDate(0),
              RptAccTS(false), RptAccTSFO(false), RptAccHR(false), RptAccHRFO(false), RptAccDY(false), RptAccDYFO(false), RptAccMN(false),
              RptAccMNFO(false), RptAccYR(false), RptAccYRFO(false), RptAccSM(false), RptAccSMFO(false), TSAccRptNum(0), HRAccRptNum(0),
              DYAccRptNum(0), MNAccRptNum(0), YRAccRptNum(0), SMAccRptNum(0), InstMeterCacheStart(0), InstMeterCacheEnd(0)
        {
        }
    };

    struct EndUseCategoryType
    {
        // Members
        std::string Name;        // End use category name
        std::string DisplayName; // Display name for output table
        int NumSubcategories;
        Array1D_string SubcategoryName; // Array of subcategory names

        // Default Constructor
        EndUseCategoryType() : NumSubcategories(0)
        {
        }
    };

    // Object Data
    extern std::map<TimeStepType, TimeSteps> TimeValue;      // Pointers to the actual TimeStep variables
    extern Array1D<RealVariableType> RVariableTypes;         // Variable Types structure (use NumOfRVariables to traverse)
    extern Array1D<IntegerVariableType> IVariableTypes;      // Variable Types structure (use NumOfIVariables to traverse)
    extern Array1D<VariableTypeForDDOutput> DDVariableTypes; // Variable Types structure (use NumVariablesForOutput to traverse)
    extern Array1D<ReqReportVariables> ReqRepVars;
    extern Array1D<MeterArrayType> VarMeterArrays;
    extern Array1D<MeterType> EnergyMeters;
    extern Array1D<EndUseCategoryType> EndUseCategory;

    // Functions

    // Clears the global data in OutputProcessor.
    // Needed for unit tests, should not be normally called.
    void clear_state();

    void InitializeOutput(EnergyPlusData &state);

    void SetupTimePointers(std::string const &IndexKey, // Which timestep is being set up, 'Zone'=1, 'HVAC'=2
                           Real64 &TimeStep             // The timestep variable.  Used to get the address
    );

    void CheckReportVariable(EnergyPlusData &state,
                             std::string const &KeyedValue, // Associated Key for this variable
                             std::string const &VarName     // String Name of variable (without units)
    );

    void BuildKeyVarList(std::string const &KeyedValue,   // Associated Key for this variable
                         std::string const &VariableName, // String Name of variable
                         int const MinIndx,               // Min number (from previous routine) for this variable
                         int const MaxIndx                // Max number (from previous routine) for this variable
    );

    void AddBlankKeys(std::string const &VariableName, // String Name of variable
                      int const MinIndx,               // Min number (from previous routine) for this variable
                      int const MaxIndx                // Max number (from previous routine) for this variable
    );

    void GetReportVariableInput(EnergyPlusData &state);

    ReportingFrequency determineFrequency(std::string const &FreqString);

    std::string reportingFrequency(ReportingFrequency reportingInterval);

    void ProduceMinMaxString(std::string &String,                // Current value
                             int const DateValue,                // Date of min/max
                             ReportingFrequency const ReportFreq // Reporting Frequency
    );

    void ProduceMinMaxStringWStartMinute(std::string &String,                // Current value
                                         int const DateValue,                // Date of min/max
                                         ReportingFrequency const ReportFreq // Reporting Frequency
    );

    // TODO: GET RID OF THESE REDIMENSIONS
    inline void ReallocateIntegerArray(Array1D_int &Array,
                                       int &ArrayMax,     // Current and resultant dimension for Array
                                       int const ArrayInc // increment for redimension
    )
    {
        Array.redimension(ArrayMax += ArrayInc, 0);
    }

    inline void ReallocateRVar()
    {
        RVariableTypes.redimension(MaxRVariable += RVarAllocInc);
    }

    inline void ReallocateIVar()
    {
        IVariableTypes.redimension(MaxIVariable += IVarAllocInc);
    }

    TimeStepType ValidateTimeStepType(std::string const &TimeStepTypeKey, // Index type (Zone, HVAC) for variables
                                      std::string const &CalledFrom    // Routine called from (for error messages)
    );

    std::string StandardTimeStepTypeKey(TimeStepType const timeStepType);

    StoreType validateVariableType(std::string const &VariableTypeKey);

    std::string standardVariableTypeKey(StoreType const VariableType);

    // *****************************************************************************
    // The following routines implement Energy Meters in EnergyPlus.
    // *****************************************************************************

    void InitializeMeters(EnergyPlusData &state);

    void GetCustomMeterInput(EnergyPlusData &state, bool &ErrorsFound);

    void GetStandardMeterResourceType(std::string &OutResourceType, std::string const &UserInputResourceType, bool &ErrorsFound);

    void AddMeter(std::string const &Name,               // Name for the meter
                  OutputProcessor::Unit const &MtrUnits, // Units for the meter
                  std::string const &ResourceType,       // ResourceType for the meter
                  std::string const &EndUse,             // EndUse for the meter
                  std::string const &EndUseSub,          // EndUse subcategory for the meter
                  std::string const &Group               // Group for the meter
    );

    void AttachMeters(Unit const &MtrUnits,        // Units for this meter
                      std::string &ResourceType,   // Electricity, Gas, etc.
                      std::string &EndUse,         // End-use category (Lights, Heating, etc.)
                      std::string &EndUseSub,      // End-use subcategory (user-defined, e.g., General Lights, Task Lights, etc.)
                      std::string &Group,          // Group key (Facility, Zone, Building, etc.)
                      std::string const &ZoneName, // Zone key only applicable for Building group
                      int const RepVarNum,         // Number of this report variable
                      int &MeterArrayPtr,          // Output set of Pointers to Meters
                      bool &ErrorsFound            // True if errors in this call
    );

    void AttachCustomMeters(int const RepVarNum, // Number of this report variable
                            int &MeterArrayPtr,  // Input/Output set of Pointers to Meters
                            int const MeterIndex // Which meter this is
    );

    void ValidateNStandardizeMeterTitles(OutputProcessor::Unit const &MtrUnits, // Units for the meter
                                         std::string &ResourceType,             // Electricity, Gas, etc.
                                         std::string &EndUse,                   // End Use Type (Lights, Heating, etc.)
                                         std::string &EndUseSub,                // End Use Sub Type (General Lights, Task Lights, etc.)
                                         std::string &Group,                    // Group key (Facility, Zone, Building, etc.)
                                         bool &ErrorsFound,                     // True if errors in this call
                                         Optional_string_const ZoneName = _     // ZoneName when Group=Building
    );

    void DetermineMeterIPUnits(int &CodeForIPUnits,                   // Output Code for IP Units
                               std::string const &ResourceType,       // Resource Type
                               OutputProcessor::Unit const &MtrUnits, // Meter units
                               bool &ErrorsFound                      // true if errors found during subroutine
    );

    void UpdateMeterValues(Real64 const TimeStepValue,                    // Value of this variable at the current time step.
                           int const NumOnMeters,                         // Number of meters this variable is "on".
                           const Array1D_int &OnMeters                     // Which meters this variable is on (index values)
    );

    void UpdateMeterValues(Real64 const TimeStepValue,                    // Value of this variable at the current time step.
                           int const NumOnMeters,                         // Number of meters this variable is "on".
                           const Array1D_int &OnMeters,                    // Which meters this variable is on (index values)
                           int const NumOnCustomMeters,                   // Number of custom meters this variable is "on".
                           const Array1D_int &OnCustomMeters              // Which custom meters this variable is on (index values)
    );

    void UpdateMeters(int const TimeStamp); // Current TimeStamp (for max/min)

    void ResetAccumulationWhenWarmupComplete();

    void SetMinMax(Real64 const TestValue, // Candidate new value
                   int const TimeStamp,    // TimeStamp to be stored if applicable
                   Real64 &CurMaxValue,    // Current Maximum Value
                   int &CurMaxValDate,     // Current Maximum Value Date Stamp
                   Real64 &CurMinValue,    // Current Minimum Value
                   int &CurMinValDate      // Current Minimum Value Date Stamp
    );

    void ReportTSMeters(EnergyPlusData &state, Real64 const StartMinute, // Start Minute for TimeStep
                        Real64 const EndMinute,   // End Minute for TimeStep
                        bool &PrintESOTimeStamp,  // True if the ESO Time Stamp also needs to be printed
                        bool PrintTimeStampToSQL  // Print Time Stamp to SQL file
    );

    void ReportHRMeters(EnergyPlusData &state, bool PrintTimeStampToSQL // Print Time Stamp to SQL file
    );

    void ReportDYMeters(EnergyPlusData &state, bool PrintTimeStampToSQL // Print Time Stamp to SQL file
    );

    void ReportMNMeters(EnergyPlusData &state, bool PrintTimeStampToSQL // Print Time Stamp to SQL file
    );

    void ReportSMMeters(EnergyPlusData &state, bool PrintTimeStampToSQL // Print Time Stamp to SQL file
    );

    void ReportYRMeters(EnergyPlusData &state, bool PrintTimeStampToSQL);

    void ReportForTabularReports();

    std::string DateToStringWithMonth(int const codedDate); // word containing encoded month, day, hour, minute

    void ReportMeterDetails(EnergyPlusData &state);

    // *****************************************************************************
    // End of routines for Energy Meters implementation in EnergyPlus.
    // *****************************************************************************

    void AddEndUseSubcategory(std::string const &ResourceName, std::string const &EndUseName, std::string const &EndUseSubName);

    void WriteTimeStampFormatData(InputOutputFile &outputFile,
                                  ReportingFrequency const reportingInterval, // Reporting frequency.
                                  int const reportID,                         // The ID of the time stamp
                                  std::string const &reportIDString,          // The ID of the time stamp
                                  int const DayOfSim,                         // the number of days simulated so far
                                  std::string const &DayOfSimChr,             // the number of days simulated so far
                                  bool writeToSQL,                            // write to SQLite
                                  Optional_int_const Month = _,               // the month of the reporting interval
                                  Optional_int_const DayOfMonth = _,          // The day of the reporting interval
                                  Optional_int_const Hour = _,                // The hour of the reporting interval
                                  Optional<Real64 const> EndMinute = _,       // The last minute in the reporting interval
                                  Optional<Real64 const> StartMinute = _,     // The starting minute of the reporting interval
                                  Optional_int_const DST = _,                 // A flag indicating whether daylight savings time is observed
                                  Optional_string_const DayType = _           // The day tied for the data (e.g., Monday)
    );

    void WriteYearlyTimeStamp(InputOutputFile &outputFile,
                              std::string const &reportIDString,    // The ID of the time stamp
                              std::string const &yearOfSimChr,      // the year of the simulation
                              bool writeToSQL);

    void WriteYearlyTimeStamp(std::ostream *out_stream_p,        // Output stream pointer
                              std::string const &reportIDString, // The ID of the time stamp
                              std::string const &yearOfSimChr,   // the year of the simulation
                              bool writeToSQL);

    void WriteReportVariableDictionaryItem(EnergyPlusData &state,
                                           ReportingFrequency const reportingInterval, // The reporting interval (e.g., hourly, daily)
                                           StoreType const storeType,
                                           int const reportID,              // The reporting ID for the data
                                           int const indexGroupKey,         // The reporting group (e.g., Zone, Plant Loop, etc.)
                                           std::string const &indexGroup,   // The reporting group (e.g., Zone, Plant Loop, etc.)
                                           std::string const &reportIDChr,  // The reporting ID for the data
                                           std::string const &keyedValue,   // The key name for the data
                                           std::string const &variableName, // The variable's actual name
                                           TimeStepType const timeStepType,
                                           OutputProcessor::Unit const &unitsForVar, // The variables units
                                           Optional_string_const customUnitName = _,
                                           Optional_string_const ScheduleName = _);

    void WriteMeterDictionaryItem(EnergyPlusData &state,
                                  ReportingFrequency const reportingInterval, // The reporting interval (e.g., hourly, daily)
                                  StoreType const storeType,
                                  int const reportID,                // The reporting ID in for the variable
                                  int const indexGroupKey,           // The reporting group for the variable
                                  std::string const &indexGroup,     // The reporting group for the variable
                                  std::string const &reportIDChr,    // The reporting ID in for the variable
                                  std::string const &meterName,      // The variable's meter name
                                  OutputProcessor::Unit const &unit, // The variables units
                                  bool const cumulativeMeterFlag,    // A flag indicating cumulative data
                                  bool const meterFileOnlyFlag       // A flag indicating whether the data is to be written to standard output
    );

    void WriteRealVariableOutput(EnergyPlusData &state,
                                 RealVariables &realVar,             // Real variable to write out
                                 ReportingFrequency const reportType // The report type or interval (e.g., hourly)
    );

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
                             int const maxValueDate);

    void WriteCumulativeReportMeterData(EnergyPlusData &state,
                                        int const reportID,           // The variable's report ID
                                        std::string const &creportID, // variable ID in characters
                                        Real64 const repValue,        // The variable's value
                                        bool const meterOnlyFlag      // A flag that indicates if the data should be written to standard output
    );

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
    );

    void WriteNumericData(EnergyPlusData &state,
                          int const reportID,           // The variable's reporting ID
                          std::string const &creportID, // variable ID in characters
                          Real64 const repValue         // The variable's value
    );

    void WriteNumericData(EnergyPlusData &state,
                          int const reportID,           // The variable's reporting ID
                          std::string const &creportID, // variable ID in characters
                          int32_t const repValue        // The variable's value
    );

    void WriteNumericData(EnergyPlusData &state,
                          int const reportID,           // The variable's reporting ID
                          std::string const &creportID, // variable ID in characters
                          int64_t const repValue        // The variable's value
    );

    void WriteIntegerVariableOutput(EnergyPlusData &state,
                                    IntegerVariables &intVar,           // Integer variable to write out
                                    ReportingFrequency const reportType // The report type (i.e., the reporting interval)
    );

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
    );

    int DetermineIndexGroupKeyFromMeterName(std::string const &meterName); // the meter name

    std::string DetermineIndexGroupFromMeterGroup(MeterType const &meter); // the meter

    void SetInternalVariableValue(int const varType,       // 1=integer, 2=real, 3=meter
                                  int const keyVarIndex,   // Array index
                                  Real64 const SetRealVal, // real value to set, if type is real or meter
                                  int const SetIntVal      // integer value to set if type is integer
    );

    std::string unitEnumToStringBrackets(Unit const &unitIn);

    std::string unitEnumToString(OutputProcessor::Unit const &unitIn);

    OutputProcessor::Unit unitStringToEnum(std::string const &unitIn);

    std::string unitStringFromDDitem(int const ddItemPtr // index provided for DDVariableTypes
    );

    std::string timeStepTypeEnumToString(OutputProcessor::TimeStepType const &t_timeStepType);

    struct APIOutputVariableRequest {
        std::string varName = "";
        std::string varKey = "";
    };
    extern std::vector<APIOutputVariableRequest> apiVarRequests;

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
                         std::string const &TimeStepTypeKey,           // Zone, HeatBalance=1, HVAC, System, Plant=2
                         std::string const &VariableTypeKey,        // State, Average=1, NonState, Sum=2
                         std::string const &KeyedValue,             // Associated Key for this variable
                         Optional_string_const ReportFreq = _,      // Internal use -- causes reporting at this freqency
                         Optional_string_const ResourceTypeKey = _, // Meter Resource Type (Electricity, Gas, etc)
                         Optional_string_const EndUseKey = _,       // Meter End Use Key (Lights, Heating, Cooling, etc)
                         Optional_string_const EndUseSubKey = _,    // Meter End Use Sub Key (General Lights, Task Lights, etc)
                         Optional_string_const GroupKey = _,        // Meter Super Group Key (Building, System, Plant)
                         Optional_string_const ZoneKey = _,         // Meter Zone Key (zone name)
                         Optional_int_const ZoneMult = _,           // Zone Multiplier, defaults to 1
                         Optional_int_const ZoneListMult = _,       // Zone List Multiplier, defaults to 1
                         Optional_int_const indexGroupKey = _,      // Group identifier for SQL output
                         Optional_string_const customUnitName = _   // the custom name for the units from EMS definition of units
);

void SetupOutputVariable(EnergyPlusData &state,
                         std::string const &VariableName,           // String Name of variable
                         OutputProcessor::Unit const &VariableUnit, // Actual units corresponding to the actual variable
                         int &ActualVariable,                       // Actual Variable, used to set up pointer
                         std::string const &TimeStepTypeKey,           // Zone, HeatBalance=1, HVAC, System, Plant=2
                         std::string const &VariableTypeKey,        // State, Average=1, NonState, Sum=2
                         std::string const &KeyedValue,             // Associated Key for this variable
                         Optional_string_const ReportFreq = _,      // Internal use -- causes reporting at this freqency
                         Optional_int_const indexGroupKey = _       // Group identifier for SQL output
);

void SetupOutputVariable(std::string const &VariableName,           // String Name of variable
                         OutputProcessor::Unit const &VariableUnit, // Actual units corresponding to the actual variable
                         Real64 &ActualVariable,                    // Actual Variable, used to set up pointer
                         std::string const &TimeStepTypeKey,           // Zone, HeatBalance=1, HVAC, System, Plant=2
                         std::string const &VariableTypeKey,        // State, Average=1, NonState, Sum=2
                         int const KeyedValue,                      // Associated Key for this variable
                         Optional_string_const ReportFreq = _,      // Internal use -- causes reporting at this freqency
                         Optional_string_const ResourceTypeKey = _, // Meter Resource Type (Electricity, Gas, etc)
                         Optional_string_const EndUseKey = _,       // Meter End Use Key (Lights, Heating, Cooling, etc)
                         Optional_string_const EndUseSubKey = _,    // Meter End Use Sub Key (General Lights, Task Lights, etc)
                         Optional_string_const GroupKey = _,        // Meter Super Group Key (Building, System, Plant)
                         Optional_string_const ZoneKey = _,         // Meter Zone Key (zone name)
                         Optional_int_const ZoneMult = _,           // Zone Multiplier, defaults to 1
                         Optional_int_const ZoneListMult = _,       // Zone List Multiplier, defaults to 1
                         Optional_int_const indexGroupKey = _       // Group identifier for SQL output
);

void UpdateDataandReport(EnergyPlusData &state, OutputProcessor::TimeStepType const TimeStepTypeKey); // What kind of data to update (Zone, HVAC)

void AssignReportNumber(int &ReportNumber);

void GenOutputVariablesAuditReport();

void UpdateMeterReporting(EnergyPlusData &state);

void SetInitialMeterReportingAndOutputNames(EnergyPlusData &state,
                                            int const WhichMeter,              // Which meter number
                                            bool const MeterFileOnlyIndicator, // true if this is a meter file only reporting
                                            OutputProcessor::ReportingFrequency const FrequencyIndicator, // at what frequency is the meter reported
                                            bool const CumulativeIndicator // true if this is a Cumulative meter reporting
);

int GetMeterIndex(std::string const &MeterName);

std::string GetMeterResourceType(int const MeterNumber); // Which Meter Number (from GetMeterIndex)

Real64 GetCurrentMeterValue(int const MeterNumber); // Which Meter Number (from GetMeterIndex)

Real64 GetInstantMeterValue(int const MeterNumber, // Which Meter Number (from GetMeterIndex)
                            OutputProcessor::TimeStepType const TimeStepType    // Whether this is zone of HVAC
);

void IncrementInstMeterCache();

Real64 GetInternalVariableValue(int const varType,    // 1=integer, 2=real, 3=meter
                                int const keyVarIndex // Array index
);

Real64 GetInternalVariableValueExternalInterface(int const varType,    // 1=integer, 2=REAL(r64), 3=meter
                                                 int const keyVarIndex // Array index
);

int GetNumMeteredVariables(std::string const &ComponentType, // Given Component Type
                           std::string const &ComponentName  // Given Component Name (user defined)
);

void GetMeteredVariables(std::string const &ComponentType,                      // Given Component Type
                         std::string const &ComponentName,                      // Given Component Name (user defined)
                         Array1D_int &VarIndexes,                               // Variable Numbers
                         Array1D_int &VarTypes,                                 // Variable Types (1=integer, 2=real, 3=meter)
                         Array1D<OutputProcessor::TimeStepType> &TimeStepTypes, // Variable Index Types (1=Zone,2=HVAC),
                         Array1D<OutputProcessor::Unit> &unitsForVar,           // units from enum for each variable
                         Array1D_int &ResourceTypes,                            // ResourceTypes for each variable
                         Array1D_string &EndUses,                               // EndUses for each variable
                         Array1D_string &Groups,                                // Groups for each variable
                         Array1D_string &Names,                                 // Variable Names for each variable
                         int &NumFound                                          // Number Found
);

void GetMeteredVariables(std::string const &ComponentType,                       // Given Component Type
                         std::string const &ComponentName,                       // Given Component Name (user defined)
                         Array1D_int &VarIndexes,                                // Variable Numbers
                         Array1D_int &VarTypes,                                  // Variable Types (1=integer, 2=real, 3=meter)
                         Array1D<OutputProcessor::TimeStepType> &TimeStepTypes,  // Variable Index Types (1=Zone,2=HVAC),
                         Array1D<OutputProcessor::Unit> &unitsForVar,            // units from enum for each variable
                         Array1D_int &ResourceTypes,                             // ResourceTypes for each variable
                         Array1D_string &EndUses,                                // EndUses for each variable
                         Array1D_string &Groups,                                 // Groups for each variable
                         Array1D_string &Names,                                  // Variable Names for each variable
                         Array1D_int &VarIDs                                     // Variable Report Numbers
);

void GetVariableKeyCountandType(EnergyPlusData &state,
                                std::string const &varName,            // Standard variable name
                                int &numKeys,                          // Number of keys found
                                int &varType,                          // 0=not found, 1=integer, 2=real, 3=meter
                                OutputProcessor::StoreType &varAvgSum, // Variable  is Averaged=1 or Summed=2
                                OutputProcessor::TimeStepType &varStepType,  // Variable time step is Zone=1 or HVAC=2
                                OutputProcessor::Unit &varUnits        // Units enumeration
);

void GetVariableKeys(EnergyPlusData &state,
                     std::string const &varName, // Standard variable name
                     int const varType,          // 1=integer, 2=real, 3=meter
                     Array1D_string &keyNames,   // Specific key name
                     Array1D_int &keyVarIndexes  // Array index for
);

bool ReportingThisVariable(std::string const &RepVarName);

void InitPollutionMeterReporting(EnergyPlusData &state, std::string const &ReportFreqName);

void ProduceRDDMDD(EnergyPlusData &state);

void AddToOutputVariableList(std::string const &VarName, // Variable Name
                             OutputProcessor::TimeStepType const TimeStepType,
                             OutputProcessor::StoreType const StateType,
                             int const VariableType,
                             OutputProcessor::Unit const unitsForVar,
                             Optional_string_const customUnitName = _ // the custom name for the units from EMS definition of units
);

int initErrorFile(EnergyPlusData &state);

} // namespace EnergyPlus

#endif
