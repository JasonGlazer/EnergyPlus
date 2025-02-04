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
#include <string>

// ObjexxFCL Headers
#include <ObjexxFCL/Array.functions.hh>
#include <ObjexxFCL/Array1D.hh>
#include <ObjexxFCL/Array2D.hh>
#include <ObjexxFCL/Fmath.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/DataHeatBalFanSys.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataIPShortCuts.hh>
#include <EnergyPlus/DataRoomAirModel.hh>
#include <EnergyPlus/DataZoneEquipment.hh>
#include <EnergyPlus/DataZoneControls.hh>
#include <EnergyPlus/EMSManager.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/GeneralRoutines.hh>
#include <EnergyPlus/GlobalNames.hh>
#include <EnergyPlus/HVACManager.hh>
#include <EnergyPlus/HeatBalanceAirManager.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/SystemAvailabilityManager.hh>
#include <EnergyPlus/UtilityRoutines.hh>
#include <EnergyPlus/ZoneTempPredictorCorrector.hh>

namespace EnergyPlus {

namespace HeatBalanceAirManager {
    // Module containing the air heat balance simulation routines
    // calculation (initialization) routines

    // MODULE INFORMATION:
    //       AUTHOR         Richard J. Liesen
    //       DATE WRITTEN   February 1998
    //       MODIFIED       May-July 2000 Joe Huang for Comis Link
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS MODULE:
    // To encapsulate the data and algorithms required to
    // manage the air simluation heat balance on the building.

    // METHODOLOGY EMPLOYED:

    // REFERENCES:
    // The heat balance method is outlined in the "Tarp Alogorithms Manual"
    // The methods are also summarized in many BSO Theses and papers.

    // OTHER NOTES:
    // This module was created from IBLAST subroutines

    // USE STATEMENTS:
    // Use statements for data only modules
    // Using/Aliasing
    using namespace DataGlobals;
    using namespace DataEnvironment;
    using namespace DataHeatBalFanSys;
    using namespace DataHeatBalance;
    using namespace DataSurfaces;

    // Use statements for access to subroutines in other modules
    using Psychrometrics::PsyCpAirFnW;
    using Psychrometrics::PsyHFnTdbW;
    using Psychrometrics::PsyRhoAirFnPbTdbW;
    using Psychrometrics::PsyTdbFnHW;

    // Data
    std::unordered_set<std::string> UniqueZoneNames;
    std::unordered_map<std::string, std::string> UniqueInfiltrationNames;
    // MODULE PARAMETER DEFINITIONS:
    static std::string const BlankString;

    namespace {
        // These were static variables within different functions. They were pulled out into the namespace
        // to facilitate easier unit testing of those functions.
        // These are purposefully not in the header file as an extern variable. No one outside of this should
        // use these. They are cleared by clear_state() for use by unit tests, but normal simulations should be unaffected.
        // This is purposefully in an anonymous namespace so nothing outside this implementation file can use it.
        bool ManageAirHeatBalanceGetInputFlag(true);
    } // namespace
    //         Subroutine Specifications for the Heat Balance Module
    // Driver Routines

    // Get Input routines for module

    // Initialization routines for module

    // Algorithms for the module
    // Reporting routines for module

    // MODULE SUBROUTINES:
    //*************************************************************************

    // Functions
    void clear_state()
    {
        ManageAirHeatBalanceGetInputFlag = true;
        UniqueZoneNames.clear();
        UniqueInfiltrationNames.clear();
    }

    void ManageAirHeatBalance(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Richard Liesen
        //       DATE WRITTEN   February 1998
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine manages the heat air balance method of calculating
        // building thermal loads.  It is called from the HeatBalanceManager
        // at the time step level.  This driver manages the calls to all of
        // the other drivers and simulation algorithms.

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

        // Obtains and Allocates heat balance related parameters from input file
        if (ManageAirHeatBalanceGetInputFlag) {
            GetAirHeatBalanceInput(state);
            ManageAirHeatBalanceGetInputFlag = false;
        }

        InitAirHeatBalance(); // Initialize all heat balance related parameters

        // Solve the zone heat balance 'Detailed' solution
        // Call the air surface heat balances
        CalcHeatBalanceAir(state);

        ReportZoneMeanAirTemp();
    }

    // Get Input Section of the Module
    //******************************************************************************

    void GetAirHeatBalanceInput(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Richard Liesen
        //       DATE WRITTEN   February 1998
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine is the main routine to call other input routines

        // METHODOLOGY EMPLOYED:
        // Uses the status flags to trigger events.

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
        bool ErrorsFound(false);

        // FLOW:

        GetAirFlowFlag(state, ErrorsFound);

        SetZoneMassConservationFlag();

        // get input parameters for modeling of room air flow
        GetRoomAirModelParameters(state, ErrorsFound);

        if (ErrorsFound) {
            ShowFatalError("GetAirHeatBalanceInput: Errors found in getting Air inputs");
        }
    }

    void GetAirFlowFlag(EnergyPlusData &state, bool &ErrorsFound) // Set to true if errors found
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Garrett Westmacott
        //       DATE WRITTEN   February 2000
        //       MODIFIED       Oct 2003, FCW: Change "Infiltration-Air Change Rate" from Sum to State
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine calls the routine to get simple air flow input data.

        // METHODOLOGY EMPLOYED:
        // Modelled after 'Modual Example' in Guide for Module Developers

        // Using/Aliasing
        using ScheduleManager::GetScheduleIndex;

        // Formats


        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:

        AirFlowFlag = UseSimpleAirFlow;

        GetSimpleAirModelInputs(state, ErrorsFound);
        if (TotInfiltration + TotVentilation + TotMixing + TotCrossMixing + TotRefDoorMixing > 0) {
            static constexpr auto Format_720("! <AirFlow Model>, Simple\n AirFlow Model, {}\n");
            print(state.files.eio, Format_720, "Simple");
        }
    }

    void SetZoneMassConservationFlag()
    {

        // SUBROUTINE INFORMATION :
        // AUTHOR         Bereket Nigusse, FSEC
        // DATE WRITTEN   February 2014
        // MODIFIED

        // RE - ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE :
        // This subroutine sets the zone mass conservation flag to true.

        // Using/Aliasing
        using DataHeatBalance::Mixing;
        using DataHeatBalance::TotMixing;
        using DataHeatBalance::ZoneAirMassFlow;
        using DataHeatBalFanSys::MixingMassFlowZone;
        using DataHeatBalFanSys::ZoneMassBalanceFlag;

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS :
        int Loop;

        // flow

        if (ZoneAirMassFlow.EnforceZoneMassBalance && ZoneAirMassFlow.BalanceMixing) {
            for (Loop = 1; Loop <= TotMixing; ++Loop) {
                ZoneMassBalanceFlag(Mixing(Loop).ZonePtr) = true;
                ZoneMassBalanceFlag(Mixing(Loop).FromZone) = true;
            }
        }
    }

    void GetSimpleAirModelInputs(EnergyPlusData &state, bool &ErrorsFound) // IF errors found in input
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda Lawrie
        //       DATE WRITTEN   July 2000
        //       MODIFIED       Oct 2003,FCW: change "Infiltration-Air Change Rate" from Sum to State
        //       MODIFIED       Jan 2008,LG: Allow multiple infiltration and ventilation objects per zone
        //                      May 2009, BG: added calls to setup for possible EMS override
        //                      August 2011, TKS: added refrigeration door mixing
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine gets the input for the "simple" air flow model.

        // REFERENCES:
        // IDD Statements
        // INFILTRATION,A1 [Zone Name],A2 [SCHEDULE Name],N1 [Design level KW],
        //     N2 [Constant Term Coefficient], N3 [Temperature Term Coefficient],
        //     N4 [Velocity Term Coefficient], N5 [Velocity Squared Term Coefficient];
        // MIXING,A1 [Zone Name],A2 [SCHEDULE Name],N1 [Design Level], A3 [Source Zone Name],
        //     N2 [Delta Temperature delta C];
        // CROSS MIXING,A1 [Zone Name],A2 [SCHEDULE Name],N1 [Design Level],
        //     A3 [Source Zone Name], N2 [Delta Temperature delta C];
        // REFRIGERATION DOOR MIXING,A1 [Zone Name],A2 [Mate Zone Name],N1 [Design Level],
        //     A3 [Source Zone Name], N2 [Delta Temperature delta C];

        // Using/Aliasing
        using DataGlobals::NumOfZones;
        using General::CheckCreatedZoneItemName;
        using General::RoundSigDigits;
        using ScheduleManager::CheckScheduleValueMinMax;
        using ScheduleManager::GetScheduleIndex;
        using ScheduleManager::GetScheduleMinValue;
        using ScheduleManager::GetScheduleName;
        using ScheduleManager::GetScheduleValuesForDay;
        using SystemAvailabilityManager::GetHybridVentilationControlStatus;

        // SUBROUTINE PARAMETER DEFINITIONS:
        Real64 const VentilTempLimit(100.0);                               // degrees Celsius
        Real64 const MixingTempLimit(100.0);                               // degrees Celsius
        Real64 const VentilWSLimit(40.0);                                  // m/s
        static std::string const RoutineName("GetSimpleAirModelInputs: "); // include trailing blank space
        // Refrigeration Door Mixing Protection types, factors used to moderate mixing flow.
        Real64 const RefDoorNone(0.0);
        Real64 const RefDoorAirCurtain(0.5);
        Real64 const RefDoorStripCurtain(0.9);

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Array2D<Real64> SVals1;
        Array2D<Real64> SVals2;
        int NumAlpha;  // Number of Alphas for each GetobjectItem call
        int NumNumber; // Number of Numbers for each GetobjectItem call
        int maxAlpha;  // max of Alphas for allocation
        int maxNumber; // max of Numbers for allocation
        int NumArgs;
        int IOStat;
        Array1D_string cAlphaFieldNames;
        Array1D_string cNumericFieldNames;
        Array1D_bool lNumericFieldBlanks;
        Array1D_bool lAlphaFieldBlanks;
        Array1D_string cAlphaArgs;
        Array1D<Real64> rNumericArgs;
        std::string cCurrentModuleObject;

        int i;
        int Loop;
        int Loop1;
        Array1D_bool RepVarSet;
        bool IsNotOK;

        int ZoneNum;
        std::string StringOut;
        std::string NameThisObject;
        int InfiltCount;
        int VentiCount;
        bool ControlFlag;
        int Item;
        int Item1;
        bool errFlag;
        int ZLItem;
        Array1D<Real64> TotInfilVentFlow;
        Array1D<Real64> TotMixingFlow;
        Array1D<Real64> ZoneMixingNum;
        int ConnectTest;
        int ConnectionNumber;
        int NumbNum;
        int AlphaNum;
        int Zone1Num;
        int Zone2Num;
        int ZoneNumA;
        int ZoneNumB;
        int SourceCount;
        int ReceivingCount;
        int IsSourceZone;

        // Formats
        static constexpr auto Format_720(" {} Airflow Stats Nominal, {},{},{},{:.2R},{:.1R},");
        static constexpr auto Format_721("! <{} Airflow Stats Nominal>,Name,Schedule Name,Zone Name, Zone Floor Area {{m2}}, # Zone Occupants,{}\n");
        static constexpr auto Format_722(" {}, {}\n");



        RepVarSet.dimension(NumOfZones, true);

        // Following used for reporting
        ZnAirRpt.allocate(NumOfZones);

        for (Loop = 1; Loop <= NumOfZones; ++Loop) {
            // CurrentModuleObject='Zone'
            SetupOutputVariable(state,
                "Zone Mean Air Temperature", OutputProcessor::Unit::C, ZnAirRpt(Loop).MeanAirTemp, "Zone", "Average", Zone(Loop).Name);
            SetupOutputVariable(state,
                "Zone Operative Temperature", OutputProcessor::Unit::C, ZnAirRpt(Loop).OperativeTemp, "Zone", "Average", Zone(Loop).Name);
            SetupOutputVariable(state, "Zone Mean Air Dewpoint Temperature",
                                OutputProcessor::Unit::C,
                                ZnAirRpt(Loop).MeanAirDewPointTemp,
                                "Zone",
                                "Average",
                                Zone(Loop).Name);
            SetupOutputVariable(state, "Zone Mean Air Humidity Ratio",
                                OutputProcessor::Unit::kgWater_kgDryAir,
                                ZnAirRpt(Loop).MeanAirHumRat,
                                "Zone",
                                "Average",
                                Zone(Loop).Name);
            SetupOutputVariable(state, "Zone Air Heat Balance Internal Convective Heat Gain Rate",
                                OutputProcessor::Unit::W,
                                ZnAirRpt(Loop).SumIntGains,
                                "System",
                                "Average",
                                Zone(Loop).Name);
            SetupOutputVariable(state, "Zone Air Heat Balance Surface Convection Rate",
                                OutputProcessor::Unit::W,
                                ZnAirRpt(Loop).SumHADTsurfs,
                                "System",
                                "Average",
                                Zone(Loop).Name);
            SetupOutputVariable(state, "Zone Air Heat Balance Interzone Air Transfer Rate",
                                OutputProcessor::Unit::W,
                                ZnAirRpt(Loop).SumMCpDTzones,
                                "System",
                                "Average",
                                Zone(Loop).Name);
            SetupOutputVariable(state, "Zone Air Heat Balance Outdoor Air Transfer Rate",
                                OutputProcessor::Unit::W,
                                ZnAirRpt(Loop).SumMCpDtInfil,
                                "System",
                                "Average",
                                Zone(Loop).Name);
            SetupOutputVariable(state, "Zone Air Heat Balance System Air Transfer Rate",
                                OutputProcessor::Unit::W,
                                ZnAirRpt(Loop).SumMCpDTsystem,
                                "System",
                                "Average",
                                Zone(Loop).Name);
            SetupOutputVariable(state, "Zone Air Heat Balance System Convective Heat Gain Rate",
                                OutputProcessor::Unit::W,
                                ZnAirRpt(Loop).SumNonAirSystem,
                                "System",
                                "Average",
                                Zone(Loop).Name);
            SetupOutputVariable(state, "Zone Air Heat Balance Air Energy Storage Rate",
                                OutputProcessor::Unit::W,
                                ZnAirRpt(Loop).CzdTdt,
                                "System",
                                "Average",
                                Zone(Loop).Name);
            if (DisplayAdvancedReportVariables) {
                SetupOutputVariable(state, "Zone Phase Change Material Melting Enthalpy",
                                    OutputProcessor::Unit::J_kg,
                                    ZnAirRpt(Loop).SumEnthalpyM,
                                    "Zone",
                                    "Average",
                                    Zone(Loop).Name);
                SetupOutputVariable(state, "Zone Phase Change Material Freezing Enthalpy",
                                    OutputProcessor::Unit::J_kg,
                                    ZnAirRpt(Loop).SumEnthalpyH,
                                    "Zone",
                                    "Average",
                                    Zone(Loop).Name);
                SetupOutputVariable(state,
                    "Zone Air Heat Balance Deviation Rate", OutputProcessor::Unit::W, ZnAirRpt(Loop).imBalance, "System", "Average", Zone(Loop).Name);
            }


            SetupOutputVariable(state, "Zone Exfiltration Heat Transfer Rate",
                                OutputProcessor::Unit::W,
                                ZnAirRpt(Loop).ExfilTotalLoss,
                                "System",
                                "Average",
                                Zone(Loop).Name);
            SetupOutputVariable(state, "Zone Exfiltration Sensible Heat Transfer Rate",
                                OutputProcessor::Unit::W,
                                ZnAirRpt(Loop).ExfilSensiLoss,
                                "System",
                                "Average",
                                Zone(Loop).Name);
            SetupOutputVariable(state, "Zone Exfiltration Latent Heat Transfer Rate",
                                OutputProcessor::Unit::W,
                                ZnAirRpt(Loop).ExfilLatentLoss,
                                "System",
                                "Average",
                                Zone(Loop).Name);
            SetupOutputVariable(state, "Zone Exhaust Air Heat Transfer Rate",
                                OutputProcessor::Unit::W,
                                ZnAirRpt(Loop).ExhTotalLoss,
                                "System",
                                "Average",
                                Zone(Loop).Name);
            SetupOutputVariable(state, "Zone Exhaust Air Sensible Heat Transfer Rate",
                                OutputProcessor::Unit::W,
                                ZnAirRpt(Loop).ExhSensiLoss,
                                "System",
                                "Average",
                                Zone(Loop).Name);
            SetupOutputVariable(state, "Zone Exhaust Air Latent Heat Transfer Rate",
                                OutputProcessor::Unit::W,
                                ZnAirRpt(Loop).ExhLatentLoss,
                                "System",
                                "Average",
                                Zone(Loop).Name);
        }

        SetupOutputVariable(state,
            "Site Total Zone Exfiltration Heat Loss", OutputProcessor::Unit::J, ZoneTotalExfiltrationHeatLoss, "System", "Sum", "Environment");
        SetupOutputVariable(state,
            "Site Total Zone Exhaust Air Heat Loss", OutputProcessor::Unit::J, ZoneTotalExhaustHeatLoss, "System", "Sum", "Environment");

        cCurrentModuleObject = "ZoneAirBalance:OutdoorAir";
        inputProcessor->getObjectDefMaxArgs(cCurrentModuleObject, NumArgs, NumAlpha, NumNumber);
        maxAlpha = NumAlpha;
        maxNumber = NumNumber;
        cCurrentModuleObject = "ZoneInfiltration:EffectiveLeakageArea";
        inputProcessor->getObjectDefMaxArgs(cCurrentModuleObject, NumArgs, NumAlpha, NumNumber);
        maxAlpha = max(NumAlpha, maxAlpha);
        maxNumber = max(NumNumber, maxNumber);
        cCurrentModuleObject = "ZoneInfiltration:FlowCoefficient";
        inputProcessor->getObjectDefMaxArgs(cCurrentModuleObject, NumArgs, NumAlpha, NumNumber);
        maxAlpha = max(NumAlpha, maxAlpha);
        maxNumber = max(NumNumber, maxNumber);
        cCurrentModuleObject = "ZoneInfiltration:DesignFlowRate";
        inputProcessor->getObjectDefMaxArgs(cCurrentModuleObject, NumArgs, NumAlpha, NumNumber);
        maxAlpha = max(NumAlpha, maxAlpha);
        maxNumber = max(NumNumber, maxNumber);
        cCurrentModuleObject = "ZoneVentilation:DesignFlowRate";
        inputProcessor->getObjectDefMaxArgs(cCurrentModuleObject, NumArgs, NumAlpha, NumNumber);
        maxAlpha = max(NumAlpha, maxAlpha);
        maxNumber = max(NumNumber, maxNumber);
        cCurrentModuleObject = "ZoneVentilation:WindandStackOpenArea";
        inputProcessor->getObjectDefMaxArgs(cCurrentModuleObject, NumArgs, NumAlpha, NumNumber);
        maxAlpha = max(NumAlpha, maxAlpha);
        maxNumber = max(NumNumber, maxNumber);
        cCurrentModuleObject = "ZoneMixing";
        inputProcessor->getObjectDefMaxArgs(cCurrentModuleObject, NumArgs, NumAlpha, NumNumber);
        maxAlpha = max(NumAlpha, maxAlpha);
        maxNumber = max(NumNumber, maxNumber);
        cCurrentModuleObject = "ZoneCrossMixing";
        inputProcessor->getObjectDefMaxArgs(cCurrentModuleObject, NumArgs, NumAlpha, NumNumber);
        maxAlpha = max(NumAlpha, maxAlpha);
        maxNumber = max(NumNumber, maxNumber);
        cCurrentModuleObject = "ZoneRefrigerationDoorMixing";
        inputProcessor->getObjectDefMaxArgs(cCurrentModuleObject, NumArgs, NumAlpha, NumNumber);
        maxAlpha = max(NumAlpha, maxAlpha);
        maxNumber = max(NumNumber, maxNumber);

        cAlphaArgs.allocate(maxAlpha);
        cAlphaFieldNames.allocate(maxAlpha);
        cNumericFieldNames.allocate(maxNumber);
        rNumericArgs.dimension(maxNumber, 0.0);
        lAlphaFieldBlanks.dimension(maxAlpha, true);
        lNumericFieldBlanks.dimension(maxNumber, true);

        cCurrentModuleObject = "ZoneAirBalance:OutdoorAir";
        TotZoneAirBalance = inputProcessor->getNumObjectsFound(cCurrentModuleObject);

        ZoneAirBalance.allocate(TotZoneAirBalance);

        for (Loop = 1; Loop <= TotZoneAirBalance; ++Loop) {
            inputProcessor->getObjectItem(state,
                                          cCurrentModuleObject,
                                          Loop,
                                          cAlphaArgs,
                                          NumAlpha,
                                          rNumericArgs,
                                          NumNumber,
                                          IOStat,
                                          lNumericFieldBlanks,
                                          lAlphaFieldBlanks,
                                          cAlphaFieldNames,
                                          cNumericFieldNames);
            IsNotOK = false;
            UtilityRoutines::IsNameEmpty(cAlphaArgs(1), cCurrentModuleObject, ErrorsFound);
            ZoneAirBalance(Loop).Name = cAlphaArgs(1);
            ZoneAirBalance(Loop).ZoneName = cAlphaArgs(2);
            ZoneAirBalance(Loop).ZonePtr = UtilityRoutines::FindItemInList(cAlphaArgs(2), Zone);
            if (ZoneAirBalance(Loop).ZonePtr == 0) {
                ShowSevereError(RoutineName + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\", invalid (not found) " + cAlphaFieldNames(2) +
                                "=\"" + cAlphaArgs(2) + "\".");
                ErrorsFound = true;
            }
            GlobalNames::IntraObjUniquenessCheck(cAlphaArgs(2), cCurrentModuleObject, cAlphaFieldNames(2), UniqueZoneNames, IsNotOK);
            if (IsNotOK) {
                ShowSevereError(RoutineName + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\", a duplicated object " + cAlphaFieldNames(2) +
                                "=\"" + cAlphaArgs(2) + "\" is found.");
                ShowContinueError("A zone can only have one " + cCurrentModuleObject + " object.");
                ErrorsFound = true;
            }

            {
                auto const SELECT_CASE_var(cAlphaArgs(3)); // Aie balance method type character input-->convert to integer
                if (SELECT_CASE_var == "QUADRATURE") {
                    ZoneAirBalance(Loop).BalanceMethod = AirBalanceQuadrature;
                } else if (SELECT_CASE_var == "NONE") {
                    ZoneAirBalance(Loop).BalanceMethod = AirBalanceNone;
                } else {
                    ZoneAirBalance(Loop).BalanceMethod = AirBalanceNone;
                    ShowWarningError(RoutineName + cAlphaFieldNames(3) + " = " + cAlphaArgs(3) + " not valid choice for " + cCurrentModuleObject +
                                     '=' + cAlphaArgs(1));
                    ShowContinueError("The default choice \"NONE\" is assigned");
                }
            }

            ZoneAirBalance(Loop).InducedAirRate = rNumericArgs(1);
            if (rNumericArgs(1) < 0.0) {
                ShowSevereError(
                    RoutineName + cCurrentModuleObject + "=\"" + cAlphaArgs(1) +
                    "\", invalid Induced Outdoor Air Due to Duct Leakage Unbalance specification [<0.0]=" + RoundSigDigits(rNumericArgs(1), 3));
                ErrorsFound = true;
            }

            ZoneAirBalance(Loop).InducedAirSchedPtr = GetScheduleIndex(state, cAlphaArgs(4));
            if (ZoneAirBalance(Loop).InducedAirSchedPtr == 0) {
                if (lAlphaFieldBlanks(4)) {
                    ShowSevereError(RoutineName + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\"," + cAlphaFieldNames(4) +
                                    " is required but field is blank.");
                } else {
                    ShowSevereError(RoutineName + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\", invalid (not found) " + cAlphaFieldNames(4) +
                                    "=\"" + cAlphaArgs(4) + "\".");
                }
                ErrorsFound = true;
            }
            if (!CheckScheduleValueMinMax(ZoneAirBalance(Loop).InducedAirSchedPtr, ">=", 0.0, "<=", 1.0)) {
                ShowSevereError(cCurrentModuleObject + " = " + ZoneAirBalance(Loop).Name + ":  Error found in " + cAlphaFieldNames(4) + " = " +
                                cAlphaArgs(4));
                ShowContinueError("Schedule values must be (>=0., <=1.)");
                ErrorsFound = true;
            }

            // Check whether this zone is also controleld by hybrid ventilation object with ventilation control option or not
            ControlFlag = GetHybridVentilationControlStatus(state, ZoneAirBalance(Loop).ZonePtr);
            if (ControlFlag && ZoneAirBalance(Loop).BalanceMethod == AirBalanceQuadrature) {
                ZoneAirBalance(Loop).BalanceMethod = AirBalanceNone;
                ShowWarningError(cCurrentModuleObject + " = " + ZoneAirBalance(Loop).Name + ": This Zone (" + cAlphaArgs(2) +
                                 ") is controlled by AvailabilityManager:HybridVentilation with Simple Airflow Control Type option.");
                ShowContinueError("Air balance method type QUADRATURE and Simple Airflow Control Type cannot co-exist. The NONE method is assigned");
            }

            if (ZoneAirBalance(Loop).BalanceMethod == AirBalanceQuadrature) {
                SetupOutputVariable(state, "Zone Combined Outdoor Air Sensible Heat Loss Energy",
                                    OutputProcessor::Unit::J,
                                    ZnAirRpt(ZoneAirBalance(Loop).ZonePtr).OABalanceHeatLoss,
                                    "System",
                                    "Sum",
                                    Zone(ZoneAirBalance(Loop).ZonePtr).Name);
                SetupOutputVariable(state, "Zone Combined Outdoor Air Sensible Heat Gain Energy",
                                    OutputProcessor::Unit::J,
                                    ZnAirRpt(ZoneAirBalance(Loop).ZonePtr).OABalanceHeatGain,
                                    "System",
                                    "Sum",
                                    Zone(ZoneAirBalance(Loop).ZonePtr).Name);
                SetupOutputVariable(state, "Zone Combined Outdoor Air Latent Heat Loss Energy",
                                    OutputProcessor::Unit::J,
                                    ZnAirRpt(ZoneAirBalance(Loop).ZonePtr).OABalanceLatentLoss,
                                    "System",
                                    "Sum",
                                    Zone(ZoneAirBalance(Loop).ZonePtr).Name);
                SetupOutputVariable(state, "Zone Combined Outdoor Air Latent Heat Gain Energy",
                                    OutputProcessor::Unit::J,
                                    ZnAirRpt(ZoneAirBalance(Loop).ZonePtr).OABalanceLatentGain,
                                    "System",
                                    "Sum",
                                    Zone(ZoneAirBalance(Loop).ZonePtr).Name);
                SetupOutputVariable(state, "Zone Combined Outdoor Air Total Heat Loss Energy",
                                    OutputProcessor::Unit::J,
                                    ZnAirRpt(ZoneAirBalance(Loop).ZonePtr).OABalanceTotalLoss,
                                    "System",
                                    "Sum",
                                    Zone(ZoneAirBalance(Loop).ZonePtr).Name);
                SetupOutputVariable(state, "Zone Combined Outdoor Air Total Heat Gain Energy",
                                    OutputProcessor::Unit::J,
                                    ZnAirRpt(ZoneAirBalance(Loop).ZonePtr).OABalanceTotalGain,
                                    "System",
                                    "Sum",
                                    Zone(ZoneAirBalance(Loop).ZonePtr).Name);
                SetupOutputVariable(state, "Zone Combined Outdoor Air Current Density Volume Flow Rate",
                                    OutputProcessor::Unit::m3_s,
                                    ZnAirRpt(ZoneAirBalance(Loop).ZonePtr).OABalanceVdotCurDensity,
                                    "System",
                                    "Sum",
                                    Zone(ZoneAirBalance(Loop).ZonePtr).Name);
                SetupOutputVariable(state, "Zone Combined Outdoor Air Standard Density Volume Flow Rate",
                                    OutputProcessor::Unit::m3_s,
                                    ZnAirRpt(ZoneAirBalance(Loop).ZonePtr).OABalanceVdotStdDensity,
                                    "System",
                                    "Sum",
                                    Zone(ZoneAirBalance(Loop).ZonePtr).Name);
                SetupOutputVariable(state, "Zone Combined Outdoor Air Current Density Volume",
                                    OutputProcessor::Unit::m3,
                                    ZnAirRpt(ZoneAirBalance(Loop).ZonePtr).OABalanceVolumeCurDensity,
                                    "System",
                                    "Sum",
                                    Zone(ZoneAirBalance(Loop).ZonePtr).Name);
                SetupOutputVariable(state, "Zone Combined Outdoor Air Standard Density Volume",
                                    OutputProcessor::Unit::m3,
                                    ZnAirRpt(ZoneAirBalance(Loop).ZonePtr).OABalanceVolumeStdDensity,
                                    "System",
                                    "Sum",
                                    Zone(ZoneAirBalance(Loop).ZonePtr).Name);
                SetupOutputVariable(state, "Zone Combined Outdoor Air Mass",
                                    OutputProcessor::Unit::kg,
                                    ZnAirRpt(ZoneAirBalance(Loop).ZonePtr).OABalanceMass,
                                    "System",
                                    "Sum",
                                    Zone(ZoneAirBalance(Loop).ZonePtr).Name);
                SetupOutputVariable(state, "Zone Combined Outdoor Air Mass Flow Rate",
                                    OutputProcessor::Unit::kg_s,
                                    ZnAirRpt(ZoneAirBalance(Loop).ZonePtr).OABalanceMdot,
                                    "System",
                                    "Average",
                                    Zone(ZoneAirBalance(Loop).ZonePtr).Name);
                SetupOutputVariable(state, "Zone Combined Outdoor Air Changes per Hour",
                                    OutputProcessor::Unit::ach,
                                    ZnAirRpt(ZoneAirBalance(Loop).ZonePtr).OABalanceAirChangeRate,
                                    "System",
                                    "Average",
                                    Zone(ZoneAirBalance(Loop).ZonePtr).Name);
                SetupOutputVariable(state, "Zone Combined Outdoor Air Fan Electricity Energy",
                                    OutputProcessor::Unit::J,
                                    ZnAirRpt(ZoneAirBalance(Loop).ZonePtr).OABalanceFanElec,
                                    "System",
                                    "Sum",
                                    Zone(ZoneAirBalance(Loop).ZonePtr).Name,
                                    _,
                                    "Electricity",
                                    "Fans",
                                    "Ventilation (simple)",
                                    "Building",
                                    Zone(ZoneAirBalance(Loop).ZonePtr).Name);
            }
        }

        cCurrentModuleObject = "ZoneInfiltration:EffectiveLeakageArea";
        TotShermGrimsInfiltration = inputProcessor->getNumObjectsFound(cCurrentModuleObject);

        cCurrentModuleObject = "ZoneInfiltration:FlowCoefficient";
        TotAIM2Infiltration = inputProcessor->getNumObjectsFound(cCurrentModuleObject);

        cCurrentModuleObject = "ZoneInfiltration:DesignFlowRate";
        NumInfiltrationStatements = inputProcessor->getNumObjectsFound(cCurrentModuleObject);

        InfiltrationObjects.allocate(NumInfiltrationStatements);

        TotDesignFlowInfiltration = 0;
        errFlag = false;
        for (Item = 1; Item <= NumInfiltrationStatements; ++Item) {
            inputProcessor->getObjectItem(state,
                                          cCurrentModuleObject,
                                          Item,
                                          cAlphaArgs,
                                          NumAlpha,
                                          rNumericArgs,
                                          NumNumber,
                                          IOStat,
                                          lNumericFieldBlanks,
                                          lAlphaFieldBlanks,
                                          cAlphaFieldNames,
                                          cNumericFieldNames);
            UtilityRoutines::IsNameEmpty(cAlphaArgs(1), cCurrentModuleObject, ErrorsFound);

            InfiltrationObjects(Item).Name = cAlphaArgs(1);
            Item1 = UtilityRoutines::FindItemInList(cAlphaArgs(2), Zone);
            ZLItem = 0;
            if (Item1 == 0 && NumOfZoneLists > 0) ZLItem = UtilityRoutines::FindItemInList(cAlphaArgs(2), ZoneList);
            if (Item1 > 0) {
                InfiltrationObjects(Item).StartPtr = TotDesignFlowInfiltration + 1;
                ++TotDesignFlowInfiltration;
                InfiltrationObjects(Item).NumOfZones = 1;
                InfiltrationObjects(Item).ZoneListActive = false;
                InfiltrationObjects(Item).ZoneOrZoneListPtr = Item1;
            } else if (ZLItem > 0) {
                InfiltrationObjects(Item).StartPtr = TotDesignFlowInfiltration + 1;
                TotDesignFlowInfiltration += ZoneList(ZLItem).NumOfZones;
                InfiltrationObjects(Item).NumOfZones = ZoneList(ZLItem).NumOfZones;
                InfiltrationObjects(Item).ZoneListActive = true;
                InfiltrationObjects(Item).ZoneOrZoneListPtr = ZLItem;
            } else {
                ShowSevereError(cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\" invalid " + cAlphaFieldNames(2) + "=\"" + cAlphaArgs(2) +
                                "\" not found.");
                ErrorsFound = true;
                errFlag = true;
            }
        }

        if (errFlag) {
            ShowSevereError(RoutineName + "Errors with invalid names in " + cCurrentModuleObject + " objects.");
            ShowContinueError("...These will not be read in.  Other errors may occur.");
            TotDesignFlowInfiltration = 0;
        }

        TotInfiltration = TotDesignFlowInfiltration + TotShermGrimsInfiltration + TotAIM2Infiltration;

        Infiltration.allocate(TotInfiltration);
        UniqueInfiltrationNames.reserve(static_cast<unsigned>(TotInfiltration));

        if (TotDesignFlowInfiltration > 0) {
            Loop = 0;
            cCurrentModuleObject = "ZoneInfiltration:DesignFlowRate";
            for (Item = 1; Item <= NumInfiltrationStatements; ++Item) {

                inputProcessor->getObjectItem(state,
                                              cCurrentModuleObject,
                                              Item,
                                              cAlphaArgs,
                                              NumAlpha,
                                              rNumericArgs,
                                              NumNumber,
                                              IOStat,
                                              lNumericFieldBlanks,
                                              lAlphaFieldBlanks,
                                              cAlphaFieldNames,
                                              cNumericFieldNames);

                for (Item1 = 1; Item1 <= InfiltrationObjects(Item).NumOfZones; ++Item1) {
                    ++Loop;
                    if (!InfiltrationObjects(Item).ZoneListActive) {
                        Infiltration(Loop).Name = cAlphaArgs(1);
                        Infiltration(Loop).ZonePtr = InfiltrationObjects(Item).ZoneOrZoneListPtr;
                    } else {
                        CheckCreatedZoneItemName(RoutineName,
                                                 cCurrentModuleObject,
                                                 Zone(ZoneList(InfiltrationObjects(Item).ZoneOrZoneListPtr).Zone(Item1)).Name,
                                                 ZoneList(InfiltrationObjects(Item).ZoneOrZoneListPtr).MaxZoneNameLength,
                                                 InfiltrationObjects(Item).Name,
                                                 Infiltration,
                                                 Loop - 1,
                                                 Infiltration(Loop).Name,
                                                 errFlag);
                        Infiltration(Loop).ZonePtr = ZoneList(InfiltrationObjects(Item).ZoneOrZoneListPtr).Zone(Item1);
                        if (errFlag) ErrorsFound = true;
                    }

                    Infiltration(Loop).ModelType = InfiltrationDesignFlowRate;
                    Infiltration(Loop).SchedPtr = GetScheduleIndex(state, cAlphaArgs(3));
                    if (Infiltration(Loop).SchedPtr == 0) {
                        if (Item1 == 1) {
                            if (lAlphaFieldBlanks(3)) {
                                ShowSevereError(RoutineName + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\"," + cAlphaFieldNames(3) +
                                                " is required but field is blank.");
                            } else {
                                ShowSevereError(RoutineName + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\", invalid (not found) " +
                                                cAlphaFieldNames(3) + "=\"" + cAlphaArgs(3) + "\".");
                            }
                            ErrorsFound = true;
                        }
                    }

                    // setup a flag if the outdoor air balance method is applied
                    if (Infiltration(Loop).ZonePtr > 0 && TotZoneAirBalance > 0) {
                        for (i = 1; i <= TotZoneAirBalance; ++i) {
                            if (Infiltration(Loop).ZonePtr == ZoneAirBalance(i).ZonePtr) {
                                if (ZoneAirBalance(i).BalanceMethod == AirBalanceQuadrature) {
                                    Infiltration(Loop).QuadratureSum = true;
                                    Infiltration(Loop).OABalancePtr = i;
                                    break;
                                }
                            }
                        }
                    }

                    // Infiltration equipment design level calculation method.
                    {
                        auto const SELECT_CASE_var(cAlphaArgs(4));
                        if ((SELECT_CASE_var == "FLOW") || (SELECT_CASE_var == "FLOW/ZONE")) {
                            Infiltration(Loop).DesignLevel = rNumericArgs(1);
                            if (lNumericFieldBlanks(1)) {
                                ShowWarningError(RoutineName + cCurrentModuleObject + "=\"" + Infiltration(Loop).Name + "\", " + cAlphaFieldNames(4) +
                                                 " specifies " + cNumericFieldNames(1) + ", but that field is blank.  0 Infiltration will result.");
                            }

                        } else if (SELECT_CASE_var == "FLOW/AREA") {
                            if (Infiltration(Loop).ZonePtr != 0) {
                                if (rNumericArgs(2) >= 0.0) {
                                    Infiltration(Loop).DesignLevel = rNumericArgs(2) * Zone(Infiltration(Loop).ZonePtr).FloorArea;
                                    if (Infiltration(Loop).ZonePtr > 0) {
                                        if (Zone(Infiltration(Loop).ZonePtr).FloorArea <= 0.0) {
                                            ShowWarningError(RoutineName + cCurrentModuleObject + "=\"" + Infiltration(Loop).Name + "\", " +
                                                             cAlphaFieldNames(4) + " specifies " + cNumericFieldNames(2) +
                                                             ", but Zone Floor Area = 0.  0 Infiltration will result.");
                                        }
                                    }
                                } else {
                                    ShowSevereError(RoutineName + cCurrentModuleObject + "=\"" + Infiltration(Loop).Name +
                                                    "\", invalid flow/area specification [<0.0]=" + RoundSigDigits(rNumericArgs(2), 3));
                                    ErrorsFound = true;
                                }
                            }
                            if (lNumericFieldBlanks(2)) {
                                ShowWarningError(RoutineName + cCurrentModuleObject + "=\"" + Infiltration(Loop).Name + "\", " + cAlphaFieldNames(4) +
                                                 " specifies " + cNumericFieldNames(2) + ", but that field is blank.  0 Infiltration will result.");
                            }

                        } else if (SELECT_CASE_var == "FLOW/EXTERIORAREA") {
                            if (Infiltration(Loop).ZonePtr != 0) {
                                if (rNumericArgs(3) >= 0.0) {
                                    Infiltration(Loop).DesignLevel = rNumericArgs(3) * Zone(Infiltration(Loop).ZonePtr).ExteriorTotalSurfArea;
                                    if (Zone(Infiltration(Loop).ZonePtr).ExteriorTotalSurfArea <= 0.0) {
                                        ShowWarningError(RoutineName + cCurrentModuleObject + "=\"" + Infiltration(Loop).Name + "\", " +
                                                         cAlphaFieldNames(4) + " specifies " + cNumericFieldNames(3) +
                                                         ", but Exterior Surface Area = 0.  0 Infiltration will result.");
                                    }
                                } else {
                                    ShowSevereError(RoutineName + cCurrentModuleObject + " = \"" + Infiltration(Loop).Name +
                                                    "\", invalid flow/exteriorarea specification [<0.0]=" + RoundSigDigits(rNumericArgs(3), 3));
                                    ErrorsFound = true;
                                }
                            }
                            if (lNumericFieldBlanks(3)) {
                                ShowWarningError(RoutineName + cCurrentModuleObject + "=\"" + Infiltration(Loop).Name + "\", " + cAlphaFieldNames(4) +
                                                 " specifies " + cNumericFieldNames(3) + ", but that field is blank.  0 Infiltration will result.");
                            }
                        } else if (SELECT_CASE_var == "FLOW/EXTERIORWALLAREA") {
                            if (Infiltration(Loop).ZonePtr != 0) {
                                if (rNumericArgs(3) >= 0.0) {
                                    Infiltration(Loop).DesignLevel = rNumericArgs(3) * Zone(Infiltration(Loop).ZonePtr).ExtGrossWallArea;
                                    if (Zone(Infiltration(Loop).ZonePtr).ExtGrossWallArea <= 0.0) {
                                        ShowWarningError(RoutineName + cCurrentModuleObject + "=\"" + Infiltration(Loop).Name + "\", " +
                                                         cAlphaFieldNames(4) + " specifies " + cNumericFieldNames(3) +
                                                         ", but Exterior Wall Area = 0.  0 Infiltration will result.");
                                    }
                                } else {
                                    ShowSevereError(RoutineName + cCurrentModuleObject + " = \"" + Infiltration(Loop).Name +
                                                    "\", invalid flow/exteriorwallarea specification [<0.0]=" + RoundSigDigits(rNumericArgs(3), 3));
                                    ErrorsFound = true;
                                }
                            }
                            if (lNumericFieldBlanks(3)) {
                                ShowWarningError(RoutineName + cCurrentModuleObject + "=\"" + Infiltration(Loop).Name + "\", " + cAlphaFieldNames(4) +
                                                 " specifies " + cNumericFieldNames(3) + ", but that field is blank.  0 Infiltration will result.");
                            }
                        } else if (SELECT_CASE_var == "AIRCHANGES/HOUR") {
                            if (Infiltration(Loop).ZonePtr != 0) {
                                if (rNumericArgs(4) >= 0.0) {
                                    Infiltration(Loop).DesignLevel = rNumericArgs(4) * Zone(Infiltration(Loop).ZonePtr).Volume / SecInHour;
                                    if (Zone(Infiltration(Loop).ZonePtr).Volume <= 0.0) {
                                        ShowWarningError(RoutineName + cCurrentModuleObject + "=\"" + Infiltration(Loop).Name + "\", " +
                                                         cAlphaFieldNames(4) + " specifies " + cNumericFieldNames(4) +
                                                         ", but Zone Volume = 0.  0 Infiltration will result.");
                                    }
                                } else {
                                    ShowSevereError(
                                        RoutineName + "In " + cCurrentModuleObject + " = \"" + Infiltration(Loop).Name +
                                        "\", invalid ACH (air changes per hour) specification [<0.0]=" + RoundSigDigits(rNumericArgs(4), 3));
                                    ErrorsFound = true;
                                }
                            }
                            if (lNumericFieldBlanks(4)) {
                                ShowWarningError(RoutineName + cCurrentModuleObject + "=\"" + Infiltration(Loop).Name + "\", " + cAlphaFieldNames(4) +
                                                 " specifies " + cNumericFieldNames(4) + ", but that field is blank.  0 Infiltration will result.");
                            }

                        } else {
                            if (Item1 == 1) {
                                ShowSevereError(RoutineName + cCurrentModuleObject + "=\"" + cAlphaArgs(1) +
                                                "\", invalid calculation method=" + cAlphaArgs(4));
                                ErrorsFound = true;
                            }
                        }
                    }

                    if (!lNumericFieldBlanks(5)) {
                        Infiltration(Loop).ConstantTermCoef = rNumericArgs(5);
                    } else {
                        Infiltration(Loop).ConstantTermCoef = 1.0;
                    }
                    if (!lNumericFieldBlanks(6)) {
                        Infiltration(Loop).TemperatureTermCoef = rNumericArgs(6);
                    } else {
                        Infiltration(Loop).TemperatureTermCoef = 0.0;
                    }
                    if (!lNumericFieldBlanks(7)) {
                        Infiltration(Loop).VelocityTermCoef = rNumericArgs(7);
                    } else {
                        Infiltration(Loop).VelocityTermCoef = 0.0;
                    }
                    if (!lNumericFieldBlanks(8)) {
                        Infiltration(Loop).VelocitySQTermCoef = rNumericArgs(8);
                    } else {
                        Infiltration(Loop).VelocitySQTermCoef = 0.0;
                    }

                    if (Infiltration(Loop).ConstantTermCoef == 0.0 && Infiltration(Loop).TemperatureTermCoef == 0.0 &&
                        Infiltration(Loop).VelocityTermCoef == 0.0 && Infiltration(Loop).VelocitySQTermCoef == 0.0) {
                        if (Item1 == 1) {
                            ShowWarningError(RoutineName + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\", in " + cAlphaFieldNames(2) + "=\"" +
                                             cAlphaArgs(2) + "\".");
                            ShowContinueError("Infiltration Coefficients are all zero.  No Infiltration will be reported.");
                        }
                    }
                }
            }
        }

        cCurrentModuleObject = "ZoneInfiltration:EffectiveLeakageArea";
        InfiltCount = TotDesignFlowInfiltration;
        for (Loop = 1; Loop <= TotShermGrimsInfiltration; ++Loop) {
            inputProcessor->getObjectItem(state,
                                          cCurrentModuleObject,
                                          Loop,
                                          cAlphaArgs,
                                          NumAlpha,
                                          rNumericArgs,
                                          NumNumber,
                                          IOStat,
                                          lNumericFieldBlanks,
                                          lAlphaFieldBlanks,
                                          cAlphaFieldNames,
                                          cNumericFieldNames);
            ++InfiltCount;
            GlobalNames::VerifyUniqueInterObjectName(UniqueInfiltrationNames, cAlphaArgs(1), cCurrentModuleObject, cAlphaFieldNames(1), ErrorsFound);
            Infiltration(InfiltCount).Name = cAlphaArgs(1);
            Infiltration(InfiltCount).ModelType = InfiltrationShermanGrimsrud;
            Infiltration(InfiltCount).ZonePtr = UtilityRoutines::FindItemInList(cAlphaArgs(2), Zone);
            if (Infiltration(InfiltCount).ZonePtr == 0) {
                ShowSevereError(RoutineName + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\", invalid (not found) " + cAlphaFieldNames(2) +
                                "=\"" + cAlphaArgs(2) + "\".");
                ErrorsFound = true;
            }

            // setup a flag if the outdoor air balance method is applied
            if (Infiltration(Loop).ZonePtr > 0 && TotZoneAirBalance > 0) {
                for (i = 1; i <= TotZoneAirBalance; ++i) {
                    if (Infiltration(Loop).ZonePtr == ZoneAirBalance(i).ZonePtr) {
                        if (ZoneAirBalance(i).BalanceMethod == AirBalanceQuadrature) {
                            Infiltration(Loop).QuadratureSum = true;
                            Infiltration(Loop).OABalancePtr = i;
                            break;
                        }
                    }
                }
            }

            Infiltration(InfiltCount).SchedPtr = GetScheduleIndex(state, cAlphaArgs(3));
            if (Infiltration(InfiltCount).SchedPtr == 0) {
                if (lAlphaFieldBlanks(3)) {
                    ShowSevereError(RoutineName + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\"," + cAlphaFieldNames(3) +
                                    " is required but field is blank.");
                } else {
                    ShowSevereError(RoutineName + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\", invalid (not found) " + cAlphaFieldNames(3) +
                                    "=\"" + cAlphaArgs(3) + "\".");
                }
                ErrorsFound = true;
            }
            Infiltration(InfiltCount).LeakageArea = rNumericArgs(1);
            Infiltration(InfiltCount).BasicStackCoefficient = rNumericArgs(2);
            Infiltration(InfiltCount).BasicWindCoefficient = rNumericArgs(3);

            // check if zone has exterior surfaces
            if (Infiltration(InfiltCount).ZonePtr > 0) {
                if (Zone(Infiltration(InfiltCount).ZonePtr).ExteriorTotalSurfArea <= 0.0) {
                    ShowWarningError(RoutineName + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\", " + cAlphaFieldNames(2) + "=\"" +
                                     cAlphaArgs(2) + "\" does not have surfaces exposed to outdoors.");
                    ShowContinueError("Infiltration model is appropriate for exterior zones not interior zones, simulation continues.");
                }
            }
        }

        cCurrentModuleObject = "ZoneInfiltration:FlowCoefficient";
        for (Loop = 1; Loop <= TotAIM2Infiltration; ++Loop) {
            inputProcessor->getObjectItem(state,
                                          cCurrentModuleObject,
                                          Loop,
                                          cAlphaArgs,
                                          NumAlpha,
                                          rNumericArgs,
                                          NumNumber,
                                          IOStat,
                                          lNumericFieldBlanks,
                                          lAlphaFieldBlanks,
                                          cAlphaFieldNames,
                                          cNumericFieldNames);
            ++InfiltCount;
            GlobalNames::VerifyUniqueInterObjectName(UniqueInfiltrationNames, cAlphaArgs(1), cCurrentModuleObject, cAlphaFieldNames(1), ErrorsFound);
            Infiltration(InfiltCount).Name = cAlphaArgs(1);
            Infiltration(InfiltCount).ModelType = InfiltrationAIM2;
            Infiltration(InfiltCount).ZonePtr = UtilityRoutines::FindItemInList(cAlphaArgs(2), Zone);
            if (Infiltration(InfiltCount).ZonePtr == 0) {
                ShowSevereError(RoutineName + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\", invalid (not found) " + cAlphaFieldNames(2) +
                                "=\"" + cAlphaArgs(2) + "\".");
                ErrorsFound = true;
            }

            // setup a flag if the outdoor air balance method is applied
            if (Infiltration(Loop).ZonePtr > 0 && TotZoneAirBalance > 0) {
                for (i = 1; i <= TotZoneAirBalance; ++i) {
                    if (Infiltration(Loop).ZonePtr == ZoneAirBalance(i).ZonePtr) {
                        if (ZoneAirBalance(i).BalanceMethod == AirBalanceQuadrature) {
                            Infiltration(Loop).QuadratureSum = true;
                            Infiltration(Loop).OABalancePtr = i;
                            break;
                        }
                    }
                }
            }

            Infiltration(InfiltCount).SchedPtr = GetScheduleIndex(state, cAlphaArgs(3));
            if (Infiltration(InfiltCount).SchedPtr == 0) {
                if (lAlphaFieldBlanks(3)) {
                    ShowSevereError(RoutineName + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\"," + cAlphaFieldNames(3) +
                                    " is required but field is blank.");
                } else {
                    ShowSevereError(RoutineName + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\", invalid (not found) " + cAlphaFieldNames(3) +
                                    "=\"" + cAlphaArgs(3) + "\".");
                }
                ErrorsFound = true;
            }
            Infiltration(InfiltCount).FlowCoefficient = rNumericArgs(1);
            Infiltration(InfiltCount).AIM2StackCoefficient = rNumericArgs(2);
            Infiltration(InfiltCount).PressureExponent = rNumericArgs(3);
            Infiltration(InfiltCount).AIM2WindCoefficient = rNumericArgs(4);
            Infiltration(InfiltCount).ShelterFactor = rNumericArgs(5);

            // check if zone has exterior surfaces
            if (Infiltration(InfiltCount).ZonePtr > 0) {
                if (Zone(Infiltration(InfiltCount).ZonePtr).ExteriorTotalSurfArea <= 0.0) {
                    ShowWarningError(RoutineName + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\", " + cAlphaFieldNames(2) + "=\"" +
                                     cAlphaArgs(2) + "\" does not have surfaces exposed to outdoors.");
                    ShowContinueError("Infiltration model is appropriate for exterior zones not interior zones, simulation continues.");
                }
            }
        }

        // setup zone-level infiltration reports
        for (Loop = 1; Loop <= TotInfiltration; ++Loop) {
            if (Infiltration(Loop).ZonePtr > 0 && !Infiltration(Loop).QuadratureSum) {
                if (RepVarSet(Infiltration(Loop).ZonePtr)) {
                    RepVarSet(Infiltration(Loop).ZonePtr) = false;
                    SetupOutputVariable(state, "Zone Infiltration Sensible Heat Loss Energy",
                                        OutputProcessor::Unit::J,
                                        ZnAirRpt(Infiltration(Loop).ZonePtr).InfilHeatLoss,
                                        "System",
                                        "Sum",
                                        Zone(Infiltration(Loop).ZonePtr).Name);
                    SetupOutputVariable(state, "Zone Infiltration Sensible Heat Gain Energy",
                                        OutputProcessor::Unit::J,
                                        ZnAirRpt(Infiltration(Loop).ZonePtr).InfilHeatGain,
                                        "System",
                                        "Sum",
                                        Zone(Infiltration(Loop).ZonePtr).Name);
                    SetupOutputVariable(state, "Zone Infiltration Latent Heat Loss Energy",
                                        OutputProcessor::Unit::J,
                                        ZnAirRpt(Infiltration(Loop).ZonePtr).InfilLatentLoss,
                                        "System",
                                        "Sum",
                                        Zone(Infiltration(Loop).ZonePtr).Name);
                    SetupOutputVariable(state, "Zone Infiltration Latent Heat Gain Energy",
                                        OutputProcessor::Unit::J,
                                        ZnAirRpt(Infiltration(Loop).ZonePtr).InfilLatentGain,
                                        "System",
                                        "Sum",
                                        Zone(Infiltration(Loop).ZonePtr).Name);
                    SetupOutputVariable(state, "Zone Infiltration Total Heat Loss Energy",
                                        OutputProcessor::Unit::J,
                                        ZnAirRpt(Infiltration(Loop).ZonePtr).InfilTotalLoss,
                                        "System",
                                        "Sum",
                                        Zone(Infiltration(Loop).ZonePtr).Name);
                    SetupOutputVariable(state, "Zone Infiltration Total Heat Gain Energy",
                                        OutputProcessor::Unit::J,
                                        ZnAirRpt(Infiltration(Loop).ZonePtr).InfilTotalGain,
                                        "System",
                                        "Sum",
                                        Zone(Infiltration(Loop).ZonePtr).Name);
                    SetupOutputVariable(state, "Zone Infiltration Current Density Volume Flow Rate",
                                        OutputProcessor::Unit::m3_s,
                                        ZnAirRpt(Infiltration(Loop).ZonePtr).InfilVdotCurDensity,
                                        "System",
                                        "Average",
                                        Zone(Infiltration(Loop).ZonePtr).Name);
                    SetupOutputVariable(state, "Zone Infiltration Standard Density Volume Flow Rate",
                                        OutputProcessor::Unit::m3_s,
                                        ZnAirRpt(Infiltration(Loop).ZonePtr).InfilVdotStdDensity,
                                        "System",
                                        "Average",
                                        Zone(Infiltration(Loop).ZonePtr).Name);
                    SetupOutputVariable(state, "Zone Infiltration Current Density Volume",
                                        OutputProcessor::Unit::m3,
                                        ZnAirRpt(Infiltration(Loop).ZonePtr).InfilVolumeCurDensity,
                                        "System",
                                        "Sum",
                                        Zone(Infiltration(Loop).ZonePtr).Name);
                    SetupOutputVariable(state, "Zone Infiltration Standard Density Volume",
                                        OutputProcessor::Unit::m3,
                                        ZnAirRpt(Infiltration(Loop).ZonePtr).InfilVolumeStdDensity,
                                        "System",
                                        "Sum",
                                        Zone(Infiltration(Loop).ZonePtr).Name);
                    SetupOutputVariable(state, "Zone Infiltration Mass",
                                        OutputProcessor::Unit::kg,
                                        ZnAirRpt(Infiltration(Loop).ZonePtr).InfilMass,
                                        "System",
                                        "Sum",
                                        Zone(Infiltration(Loop).ZonePtr).Name);
                    SetupOutputVariable(state, "Zone Infiltration Mass Flow Rate",
                                        OutputProcessor::Unit::kg_s,
                                        ZnAirRpt(Infiltration(Loop).ZonePtr).InfilMdot,
                                        "System",
                                        "Average",
                                        Zone(Infiltration(Loop).ZonePtr).Name);
                    SetupOutputVariable(state, "Zone Infiltration Air Change Rate",
                                        OutputProcessor::Unit::ach,
                                        ZnAirRpt(Infiltration(Loop).ZonePtr).InfilAirChangeRate,
                                        "System",
                                        "Average",
                                        Zone(Infiltration(Loop).ZonePtr).Name);
                }
            }

            if (AnyEnergyManagementSystemInModel) {
                SetupEMSActuator("Zone Infiltration",
                                 Infiltration(Loop).Name,
                                 "Air Exchange Flow Rate",
                                 "[m3/s]",
                                 Infiltration(Loop).EMSOverrideOn,
                                 Infiltration(Loop).EMSAirFlowRateValue);
            }
        }
        // VENTILATION Section: The following section is responsible for obtaining the simple ventilation
        // from the user's input file.
        RepVarSet = true;

        cCurrentModuleObject = "ZoneVentilation:DesignFlowRate";
        NumVentilationStatements = inputProcessor->getNumObjectsFound(cCurrentModuleObject);

        cCurrentModuleObject = "ZoneVentilation:WindandStackOpenArea";
        TotWindAndStackVentilation = inputProcessor->getNumObjectsFound(cCurrentModuleObject);

        VentilationObjects.allocate(NumVentilationStatements);

        TotDesignFlowVentilation = 0;
        errFlag = false;
        cCurrentModuleObject = "ZoneVentilation:DesignFlowRate";
        for (Item = 1; Item <= NumVentilationStatements; ++Item) {
            inputProcessor->getObjectItem(state,
                                          cCurrentModuleObject,
                                          Item,
                                          cAlphaArgs,
                                          NumAlpha,
                                          rNumericArgs,
                                          NumNumber,
                                          IOStat,
                                          lNumericFieldBlanks,
                                          lAlphaFieldBlanks,
                                          cAlphaFieldNames,
                                          cNumericFieldNames);
            UtilityRoutines::IsNameEmpty(cAlphaArgs(1), cCurrentModuleObject, ErrorsFound);
            errFlag = ErrorsFound;

            VentilationObjects(Item).Name = cAlphaArgs(1);

            Item1 = UtilityRoutines::FindItemInList(cAlphaArgs(2), Zone);
            ZLItem = 0;
            if (Item1 == 0 && NumOfZoneLists > 0) ZLItem = UtilityRoutines::FindItemInList(cAlphaArgs(2), ZoneList);
            if (Item1 > 0) {
                VentilationObjects(Item).StartPtr = TotDesignFlowVentilation + 1;
                ++TotDesignFlowVentilation;
                VentilationObjects(Item).NumOfZones = 1;
                VentilationObjects(Item).ZoneListActive = false;
                VentilationObjects(Item).ZoneOrZoneListPtr = Item1;
            } else if (ZLItem > 0) {
                VentilationObjects(Item).StartPtr = TotDesignFlowVentilation + 1;
                TotDesignFlowVentilation += ZoneList(ZLItem).NumOfZones;
                VentilationObjects(Item).NumOfZones = ZoneList(ZLItem).NumOfZones;
                VentilationObjects(Item).ZoneListActive = true;
                VentilationObjects(Item).ZoneOrZoneListPtr = ZLItem;
            } else {
                ShowSevereError(cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\" invalid " + cAlphaFieldNames(2) + "=\"" + cAlphaArgs(2) +
                                "\" not found.");
                ErrorsFound = true;
                errFlag = true;
            }
        }

        if (errFlag) {
            ShowSevereError(RoutineName + "Errors with invalid names in " + cCurrentModuleObject + " objects.");
            ShowContinueError("...These will not be read in.  Other errors may occur.");
            TotDesignFlowVentilation = 0;
        }

        TotVentilation = TotDesignFlowVentilation + TotWindAndStackVentilation;
        Ventilation.allocate(TotVentilation);

        if (TotDesignFlowVentilation > 0) {
            Loop = 0;
            cCurrentModuleObject = "ZoneVentilation:DesignFlowRate";
            for (Item = 1; Item <= NumVentilationStatements; ++Item) {

                inputProcessor->getObjectItem(state,
                                              cCurrentModuleObject,
                                              Item,
                                              cAlphaArgs,
                                              NumAlpha,
                                              rNumericArgs,
                                              NumNumber,
                                              IOStat,
                                              lNumericFieldBlanks,
                                              lAlphaFieldBlanks,
                                              cAlphaFieldNames,
                                              cNumericFieldNames);

                for (Item1 = 1; Item1 <= VentilationObjects(Item).NumOfZones; ++Item1) {
                    ++Loop;
                    if (!VentilationObjects(Item).ZoneListActive) {
                        Ventilation(Loop).Name = cAlphaArgs(1);
                        Ventilation(Loop).ZonePtr = VentilationObjects(Item).ZoneOrZoneListPtr;
                    } else {
                        CheckCreatedZoneItemName(RoutineName,
                                                 cCurrentModuleObject,
                                                 Zone(ZoneList(VentilationObjects(Item).ZoneOrZoneListPtr).Zone(Item1)).Name,
                                                 ZoneList(VentilationObjects(Item).ZoneOrZoneListPtr).MaxZoneNameLength,
                                                 VentilationObjects(Item).Name,
                                                 Ventilation,
                                                 Loop - 1,
                                                 Ventilation(Loop).Name,
                                                 errFlag);
                        Ventilation(Loop).ZonePtr = ZoneList(VentilationObjects(Item).ZoneOrZoneListPtr).Zone(Item1);
                        if (errFlag) ErrorsFound = true;
                    }

                    // setup a flag if the outdoor air balance method is applied
                    if (Ventilation(Loop).ZonePtr > 0 && TotZoneAirBalance > 0) {
                        for (i = 1; i <= TotZoneAirBalance; ++i) {
                            if (Ventilation(Loop).ZonePtr == ZoneAirBalance(i).ZonePtr) {
                                if (ZoneAirBalance(i).BalanceMethod == AirBalanceQuadrature) {
                                    Ventilation(Loop).QuadratureSum = true;
                                    Ventilation(Loop).OABalancePtr = i;
                                    break;
                                }
                            }
                        }
                    }

                    Ventilation(Loop).ModelType = VentilationDesignFlowRate;
                    Ventilation(Loop).SchedPtr = GetScheduleIndex(state, cAlphaArgs(3));
                    if (Ventilation(Loop).SchedPtr == 0) {
                        if (Item1 == 1) {
                            if (lAlphaFieldBlanks(3)) {
                                ShowSevereError(RoutineName + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\"," + cAlphaFieldNames(3) +
                                                " is required but field is blank.");
                            } else {
                                ShowSevereError(RoutineName + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\", invalid (not found) " +
                                                cAlphaFieldNames(3) + "=\"" + cAlphaArgs(3) + "\".");
                            }
                        }
                        ErrorsFound = true;
                    }

                    // Ventilation equipment design level calculation method
                    {
                        auto const SELECT_CASE_var(cAlphaArgs(4));
                        if ((SELECT_CASE_var == "FLOW") || (SELECT_CASE_var == "FLOW/ZONE")) {
                            Ventilation(Loop).DesignLevel = rNumericArgs(1);
                            if (lNumericFieldBlanks(1)) {
                                ShowWarningError(RoutineName + cCurrentModuleObject + "=\"" + Ventilation(Loop).Name + "\", " + cAlphaFieldNames(4) +
                                                 " specifies " + cNumericFieldNames(1) + ", but that field is blank.  0 Ventilation will result.");
                            }

                        } else if (SELECT_CASE_var == "FLOW/AREA") {
                            if (Ventilation(Loop).ZonePtr != 0) {
                                if (rNumericArgs(2) >= 0.0) {
                                    Ventilation(Loop).DesignLevel = rNumericArgs(2) * Zone(Ventilation(Loop).ZonePtr).FloorArea;
                                    if (Zone(Ventilation(Loop).ZonePtr).FloorArea <= 0.0) {
                                        ShowWarningError(RoutineName + cCurrentModuleObject + "=\"" + Ventilation(Loop).Name + "\", " +
                                                         cAlphaFieldNames(4) + " specifies " + cNumericFieldNames(2) +
                                                         ", but Zone Floor Area = 0.  0 Ventilation will result.");
                                    }
                                } else {
                                    ShowSevereError(RoutineName + cCurrentModuleObject + "=\"" + Ventilation(Loop).Name +
                                                    "\", invalid flow/area specification [<0.0]=" + RoundSigDigits(rNumericArgs(2), 3));
                                    ErrorsFound = true;
                                }
                            }
                            if (lNumericFieldBlanks(2)) {
                                ShowWarningError(RoutineName + cCurrentModuleObject + "=\"" + Ventilation(Loop).Name + "\", " + cAlphaFieldNames(4) +
                                                 " specifies " + cNumericFieldNames(2) + ", but that field is blank.  0 Ventilation will result.");
                            }

                        } else if (SELECT_CASE_var == "FLOW/PERSON") {
                            if (Ventilation(Loop).ZonePtr != 0) {
                                if (rNumericArgs(3) >= 0.0) {
                                    Ventilation(Loop).DesignLevel = rNumericArgs(3) * Zone(Ventilation(Loop).ZonePtr).TotOccupants;
                                    if (Zone(Ventilation(Loop).ZonePtr).TotOccupants <= 0.0) {
                                        ShowWarningError(RoutineName + cCurrentModuleObject + "=\"" + Ventilation(Loop).Name + "\", " +
                                                         cAlphaFieldNames(4) + " specifies " + cNumericFieldNames(3) +
                                                         ", but Zone Total Occupants = 0.  0 Ventilation will result.");
                                    }
                                } else {
                                    ShowSevereError(RoutineName + cCurrentModuleObject + "=\"" + Ventilation(Loop).Name +
                                                    "\", invalid flow/person specification [<0.0]=" + RoundSigDigits(rNumericArgs(3), 3));
                                    ErrorsFound = true;
                                }
                            }
                            if (lNumericFieldBlanks(3)) {
                                ShowWarningError(RoutineName + cCurrentModuleObject + "=\"" + Ventilation(Loop).Name + "\", " + cAlphaFieldNames(4) +
                                                 "specifies " + cNumericFieldNames(3) + ", but that field is blank.  0 Ventilation will result.");
                            }

                        } else if (SELECT_CASE_var == "AIRCHANGES/HOUR") {
                            if (Ventilation(Loop).ZonePtr != 0) {
                                if (rNumericArgs(4) >= 0.0) {
                                    Ventilation(Loop).DesignLevel = rNumericArgs(4) * Zone(Ventilation(Loop).ZonePtr).Volume / SecInHour;
                                    if (Zone(Ventilation(Loop).ZonePtr).Volume <= 0.0) {
                                        ShowWarningError(RoutineName + cCurrentModuleObject + "=\"" + Ventilation(Loop).Name + "\", " +
                                                         cAlphaFieldNames(4) + " specifies " + cNumericFieldNames(4) +
                                                         ", but Zone Volume = 0.  0 Ventilation will result.");
                                    }
                                } else {
                                    ShowSevereError(
                                        RoutineName + cCurrentModuleObject + "=\"" + Ventilation(Loop).Name +
                                        "\", invalid ACH (air changes per hour) specification [<0.0]=" + RoundSigDigits(rNumericArgs(5), 3));
                                    ErrorsFound = true;
                                }
                            }
                            if (lNumericFieldBlanks(4)) {
                                ShowWarningError(RoutineName + cCurrentModuleObject + "=\"" + Ventilation(Loop).Name + "\", " + cAlphaFieldNames(4) +
                                                 " specifies " + cNumericFieldNames(4) + ", but that field is blank.  0 Ventilation will result.");
                            }

                        } else {
                            if (Item1 == 1) {
                                ShowSevereError(RoutineName + cCurrentModuleObject + "=\"" + cAlphaArgs(1) +
                                                "\", invalid calculation method=" + cAlphaArgs(4));
                                ErrorsFound = true;
                            }
                        }
                    }

                    {
                        auto const SELECT_CASE_var(cAlphaArgs(5)); // Fan type character input-->convert to integer
                        if (SELECT_CASE_var == "EXHAUST") {
                            Ventilation(Loop).FanType = ExhaustVentilation;
                        } else if (SELECT_CASE_var == "INTAKE") {
                            Ventilation(Loop).FanType = IntakeVentilation;
                        } else if ((SELECT_CASE_var == "NATURAL") || (SELECT_CASE_var == "NONE") || (SELECT_CASE_var == BlankString)) {
                            Ventilation(Loop).FanType = NaturalVentilation;
                        } else if (SELECT_CASE_var == "BALANCED") {
                            Ventilation(Loop).FanType = BalancedVentilation;
                        } else {
                            if (Item1 == 1) {
                                ShowSevereError(RoutineName + cCurrentModuleObject + "=\"" + Ventilation(Loop).Name + "\". invalid " +
                                                cAlphaFieldNames(5) + "=\"" + cAlphaArgs(5) + "\".");
                                ErrorsFound = true;
                            }
                        }
                    }

                    Ventilation(Loop).FanPressure = rNumericArgs(5);
                    if (Ventilation(Loop).FanPressure < 0.0) {
                        if (Item1 == 1) {
                            ShowSevereError(RoutineName + cCurrentModuleObject + "=\"" + Ventilation(Loop).Name + "\", " + cNumericFieldNames(5) +
                                            " must be >=0");
                            ErrorsFound = true;
                        }
                    }

                    Ventilation(Loop).FanEfficiency = rNumericArgs(6);
                    if ((Ventilation(Loop).FanEfficiency <= 0.0) || (Ventilation(Loop).FanEfficiency > 1.0)) {
                        if (Item1 == 1) {
                            ShowSevereError(RoutineName + cCurrentModuleObject + "=\"" + Ventilation(Loop).Name + "\"," + cNumericFieldNames(6) +
                                            " must be in range >0 and <= 1");
                            ErrorsFound = true;
                        }
                    }

                    // Override any user input for cases where natural ventilation is being used
                    if (Ventilation(Loop).FanType == NaturalVentilation) {
                        Ventilation(Loop).FanPressure = 0.0;
                        Ventilation(Loop).FanEfficiency = 1.0;
                    }

                    if (!lNumericFieldBlanks(7)) {
                        Ventilation(Loop).ConstantTermCoef = rNumericArgs(7);
                    } else {
                        Ventilation(Loop).ConstantTermCoef = 1.0;
                    }
                    if (!lNumericFieldBlanks(8)) {
                        Ventilation(Loop).TemperatureTermCoef = rNumericArgs(8);
                    } else {
                        Ventilation(Loop).TemperatureTermCoef = 0.0;
                    }
                    if (!lNumericFieldBlanks(9)) {
                        Ventilation(Loop).VelocityTermCoef = rNumericArgs(9);
                    } else {
                        Ventilation(Loop).VelocityTermCoef = 0.0;
                    }
                    if (!lNumericFieldBlanks(10)) {
                        Ventilation(Loop).VelocitySQTermCoef = rNumericArgs(10);
                    } else {
                        Ventilation(Loop).VelocitySQTermCoef = 0.0;
                    }

                    if (Ventilation(Loop).ConstantTermCoef == 0.0 && Ventilation(Loop).TemperatureTermCoef == 0.0 &&
                        Ventilation(Loop).VelocityTermCoef == 0.0 && Ventilation(Loop).VelocitySQTermCoef == 0.0) {
                        if (Item1 == 1) {
                            ShowWarningError(RoutineName + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\", in " + cAlphaFieldNames(2) + "=\"" +
                                             cAlphaArgs(2) + "\".");
                            ShowContinueError("Ventilation Coefficients are all zero.  No Ventilation will be reported.");
                        }
                    }

                    if (!lNumericFieldBlanks(11)) {
                        Ventilation(Loop).MinIndoorTemperature = rNumericArgs(11);
                    } else {
                        Ventilation(Loop).MinIndoorTemperature = -VentilTempLimit;
                    }
                    //    Ventilation(Loop)%MinIndoorTemperature = rNumericArgs(11)
                    if ((Ventilation(Loop).MinIndoorTemperature < -VentilTempLimit) || (Ventilation(Loop).MinIndoorTemperature > VentilTempLimit)) {
                        if (Item1 == 1) {
                            ShowSevereError(RoutineName + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\" must have " + cNumericFieldNames(11) +
                                            " between -100C and 100C.");
                            ShowContinueError("...value entered=[" + RoundSigDigits(rNumericArgs(11), 2) + "].");
                            ErrorsFound = true;
                        }
                    }

                    Ventilation(Loop).MinIndoorTempSchedPtr = GetScheduleIndex(state, cAlphaArgs(6));
                    if (Ventilation(Loop).MinIndoorTempSchedPtr > 0) {
                        if (Item1 == 1) {
                            if (!lNumericFieldBlanks(11))
                                ShowWarningError(
                                    RoutineName +
                                    "The Minimum Indoor Temperature value and schedule are provided. The scheduled temperature will be used in the " +
                                    cCurrentModuleObject + " object = " + cAlphaArgs(1));
                            // Check min and max values in the schedule to ensure both values are within the range
                            if (!CheckScheduleValueMinMax(Ventilation(Loop).MinIndoorTempSchedPtr, ">=", -VentilTempLimit, "<=", VentilTempLimit)) {
                                ShowSevereError(
                                    RoutineName + cCurrentModuleObject + " statement = " + cAlphaArgs(1) +
                                    " must have a minimum indoor temperature between -100C and 100C defined in the schedule = " + cAlphaArgs(6));
                                ErrorsFound = true;
                            }
                        }
                    }
                    if (Ventilation(Loop).MinIndoorTempSchedPtr == 0 && lNumericFieldBlanks(11) && (!lAlphaFieldBlanks(6))) {
                        if (Item1 == 1) {
                            ShowWarningError(RoutineName + cNumericFieldNames(11) +
                                             ": the value field is blank and schedule field is invalid. The default value will be used (" +
                                             RoundSigDigits(-VentilTempLimit, 1) + ") ");
                            ShowContinueError("in the " + cCurrentModuleObject + " object = " + cAlphaArgs(1) + " and the simulation continues...");
                        }
                    }
                    // Check Minimum indoor temperature value and schedule fields
                    if (!lNumericFieldBlanks(11) && (!cAlphaArgs(6).empty() && Ventilation(Loop).MinIndoorTempSchedPtr == 0)) {
                        if (Item1 == 1) {
                            ShowWarningError(RoutineName + cAlphaFieldNames(6) + " = " + cAlphaArgs(6) +
                                             " is invalid. The constant value will be used at " + RoundSigDigits(rNumericArgs(11), 1) +
                                             " degrees C ");
                            ShowContinueError("in the " + cCurrentModuleObject + " object = " + cAlphaArgs(1) + " and the simulation continues...");
                        }
                    }

                    if (!lNumericFieldBlanks(12)) {
                        Ventilation(Loop).MaxIndoorTemperature = rNumericArgs(12);
                    } else {
                        Ventilation(Loop).MaxIndoorTemperature = VentilTempLimit;
                    }
                    if ((Ventilation(Loop).MaxIndoorTemperature < -VentilTempLimit) || (Ventilation(Loop).MaxIndoorTemperature > VentilTempLimit)) {
                        if (Item1 == 1) {
                            ShowSevereError(RoutineName + cCurrentModuleObject + " = " + cAlphaArgs(1) +
                                            " must have a maximum indoor temperature between -100C and 100C");
                            ErrorsFound = true;
                        }
                    }

                    Ventilation(Loop).MaxIndoorTempSchedPtr = GetScheduleIndex(state, cAlphaArgs(7));
                    if (Ventilation(Loop).MaxIndoorTempSchedPtr > 0) {
                        if (Item1 == 1) {
                            if (!lNumericFieldBlanks(12))
                                ShowWarningError(
                                    RoutineName +
                                    "The Maximum Indoor Temperature value and schedule are provided. The scheduled temperature will be used in the " +
                                    cCurrentModuleObject + " object = " + cAlphaArgs(1));
                            // Check min and max values in the schedule to ensure both values are within the range
                            if (!CheckScheduleValueMinMax(Ventilation(Loop).MaxIndoorTempSchedPtr, ">=", -VentilTempLimit, "<=", VentilTempLimit)) {
                                ShowSevereError(
                                    cCurrentModuleObject + " = " + cAlphaArgs(1) +
                                    " must have a maximum indoor temperature between -100C and 100C defined in the schedule = " + cAlphaArgs(7));
                                ErrorsFound = true;
                            }
                        }
                    }
                    if (Ventilation(Loop).MaxIndoorTempSchedPtr == 0 && lNumericFieldBlanks(12) && (!lAlphaFieldBlanks(7))) {
                        if (Item1 == 1) {
                            ShowWarningError(RoutineName + cNumericFieldNames(12) +
                                             ": the value field is blank and schedule field is invalid. The default value will be used (" +
                                             RoundSigDigits(VentilTempLimit, 1) + ") ");
                            ShowContinueError("in the " + cCurrentModuleObject + " object = " + cAlphaArgs(1) + " and the simulation continues...");
                        }
                    }
                    // Check Maximum indoor temperature value and schedule fields
                    if (!lNumericFieldBlanks(12) && ((!lAlphaFieldBlanks(7)) && Ventilation(Loop).MaxIndoorTempSchedPtr == 0)) {
                        if (Item1 == 1) {
                            ShowWarningError(RoutineName + cAlphaFieldNames(7) + " = " + cAlphaArgs(7) +
                                             " is invalid. The constant value will be used at " + RoundSigDigits(rNumericArgs(12), 1) +
                                             " degrees C ");
                            ShowContinueError("in the " + cCurrentModuleObject + " object = " + cAlphaArgs(1) + " and the simulation continues...");
                        }
                    }

                    if (!lNumericFieldBlanks(13)) {
                        Ventilation(Loop).DelTemperature = rNumericArgs(13);
                    } else {
                        Ventilation(Loop).DelTemperature = -VentilTempLimit;
                    }
                    //    Ventilation(Loop)%DelTemperature = rNumericArgs(13)  !  3/12/03  Negative del temp now allowed COP

                    Ventilation(Loop).DeltaTempSchedPtr = GetScheduleIndex(state, cAlphaArgs(8));
                    if (Ventilation(Loop).DeltaTempSchedPtr > 0) {
                        if (Item1 == 1) {
                            if (!lNumericFieldBlanks(13))
                                ShowWarningError(
                                    RoutineName +
                                    "The Delta Temperature value and schedule are provided. The scheduled temperature will be used in the " +
                                    cCurrentModuleObject + " object = " + cAlphaArgs(1));
                            // Check min value in the schedule to ensure both values are within the range
                            if (GetScheduleMinValue(Ventilation(Loop).DeltaTempSchedPtr) < -VentilTempLimit) {
                                ShowSevereError(RoutineName + cCurrentModuleObject + " statement = " + cAlphaArgs(1) +
                                                " must have a delta temperature equal to or above -100C defined in the schedule = " + cAlphaArgs(8));
                                ErrorsFound = true;
                            }
                        }
                    }
                    if (Ventilation(Loop).DeltaTempSchedPtr == 0 && lNumericFieldBlanks(13) && (!lAlphaFieldBlanks(8))) {
                        if (Item1 == 1) {
                            ShowWarningError(RoutineName + cNumericFieldNames(13) +
                                             ": the value field is blank and schedule field is invalid. The default value will be used (" +
                                             RoundSigDigits(VentilTempLimit, 1) + ") ");
                            ShowContinueError("in the " + cCurrentModuleObject + " object = " + cAlphaArgs(1) + " and the simulation continues...");
                        }
                    }

                    // Check delta temperature value and schedule fields
                    //    IF (lNumericFieldBlanks(13) .AND. cAlphaArgs(8) .EQ. BlankString) THEN
                    //      CALL ShowWarningError(RoutineName//'Both the delta temperature value and delta schedule are blank. ')
                    //      CALL ShowContinueError('Will set the temperature to a constant value of '//TRIM(RoundSigDigits(-VentilTempLimit,1)) &
                    //           //' degrees C ')
                    //      CALL ShowContinueError('in the Ventilation object = '//TRIM(cAlphaArgs(1))//' and the simulation continues...')
                    //    END IF
                    if (!lNumericFieldBlanks(13) && ((!lAlphaFieldBlanks(8)) && Ventilation(Loop).DeltaTempSchedPtr == 0)) {
                        if (Item1 == 1) {
                            ShowWarningError(RoutineName + cAlphaFieldNames(8) + " = " + cAlphaArgs(8) +
                                             " is invalid. The constant value will be used at " + RoundSigDigits(rNumericArgs(13), 1) +
                                             " degrees C ");
                            ShowContinueError("in the " + cCurrentModuleObject + " object = " + cAlphaArgs(1) + " and the simulation continues...");
                        }
                    }

                    if (!lNumericFieldBlanks(14)) {
                        Ventilation(Loop).MinOutdoorTemperature = rNumericArgs(14);
                    } else {
                        Ventilation(Loop).MinOutdoorTemperature = -VentilTempLimit;
                    }
                    if ((Ventilation(Loop).MinOutdoorTemperature < -VentilTempLimit) || (Ventilation(Loop).MinOutdoorTemperature > VentilTempLimit)) {
                        if (Item1 == 1) {
                            ShowSevereError(RoutineName + cCurrentModuleObject + " statement = " + cAlphaArgs(1) + " must have " +
                                            cNumericFieldNames(14) + " between -100C and 100C");
                            ErrorsFound = true;
                        }
                    }

                    Ventilation(Loop).MinOutdoorTempSchedPtr = GetScheduleIndex(state, cAlphaArgs(9));
                    if (Item1 == 1) {
                        if (Ventilation(Loop).MinOutdoorTempSchedPtr > 0) {
                            if (!lNumericFieldBlanks(14))
                                ShowWarningError(RoutineName +
                                                 "The Minimum Outdoor Temperature value and schedule are provided. The scheduled temperature will be "
                                                 "used in the " +
                                                 cCurrentModuleObject + " object = " + cAlphaArgs(1));
                            // Check min and max values in the schedule to ensure both values are within the range
                            if (!CheckScheduleValueMinMax(Ventilation(Loop).MinOutdoorTempSchedPtr, ">=", -VentilTempLimit, "<=", VentilTempLimit)) {
                                ShowSevereError(
                                    RoutineName + cCurrentModuleObject + " statement = " + cAlphaArgs(1) +
                                    " must have a minimum outdoor temperature between -100C and 100C defined in the schedule = " + cAlphaArgs(9));
                                ErrorsFound = true;
                            }
                        }
                        if (Ventilation(Loop).MinOutdoorTempSchedPtr == 0 && lNumericFieldBlanks(14) && (!lAlphaFieldBlanks(9))) {
                            ShowWarningError(RoutineName +
                                             "Minimum Outdoor Temperature: the value field is blank and schedule field is invalid. The default value "
                                             "will be used (" +
                                             RoundSigDigits(-VentilTempLimit, 1) + ") ");
                            ShowContinueError("in the " + cCurrentModuleObject + " object = " + cAlphaArgs(1) + " and the simulation continues...");
                        }
                        // Check Minimum outdoor temperature value and schedule fields
                        if (!lNumericFieldBlanks(14) && ((!lAlphaFieldBlanks(9)) && Ventilation(Loop).MinOutdoorTempSchedPtr == 0)) {
                            ShowWarningError(RoutineName + cAlphaFieldNames(9) + " = " + cAlphaArgs(9) +
                                             " is invalid. The constant value will be used at " + RoundSigDigits(rNumericArgs(14), 1) +
                                             " degrees C ");
                            ShowContinueError("in the " + cCurrentModuleObject + " object = " + cAlphaArgs(1) + " and the simulation continues...");
                        }
                    }

                    if (!lNumericFieldBlanks(15)) {
                        Ventilation(Loop).MaxOutdoorTemperature = rNumericArgs(15);
                    } else {
                        Ventilation(Loop).MaxOutdoorTemperature = VentilTempLimit;
                    }
                    if (Item1 == 1) {
                        if ((Ventilation(Loop).MaxOutdoorTemperature < -VentilTempLimit) ||
                            (Ventilation(Loop).MaxOutdoorTemperature > VentilTempLimit)) {
                            ShowSevereError(RoutineName + cCurrentModuleObject + " statement = " + cAlphaArgs(1) + " must have a " +
                                            cNumericFieldNames(15) + " between -100C and 100C");
                            ErrorsFound = true;
                        }
                    }

                    Ventilation(Loop).MaxOutdoorTempSchedPtr = GetScheduleIndex(state, cAlphaArgs(10));
                    if (Item1 == 1) {
                        if (Ventilation(Loop).MaxOutdoorTempSchedPtr > 0) {
                            if (!lNumericFieldBlanks(15))
                                ShowWarningError(RoutineName +
                                                 "The Maximum Outdoor Temperature value and schedule are provided. The scheduled temperature will be "
                                                 "used in the " +
                                                 cCurrentModuleObject + " object = " + cAlphaArgs(1));
                            if (!CheckScheduleValueMinMax(Ventilation(Loop).MaxOutdoorTempSchedPtr, ">=", -VentilTempLimit, "<=", VentilTempLimit)) {
                                ShowSevereError(
                                    RoutineName + cCurrentModuleObject + " statement = " + cAlphaArgs(1) +
                                    " must have a maximum outdoor temperature between -100C and 100C defined in the schedule = " + cAlphaArgs(10));
                                ErrorsFound = true;
                            }
                        }
                        if (Ventilation(Loop).MaxOutdoorTempSchedPtr == 0 && lNumericFieldBlanks(15) && (!lAlphaFieldBlanks(10))) {
                            ShowWarningError(RoutineName + cNumericFieldNames(15) +
                                             ": the value field is blank and schedule field is invalid. The default value will be used (" +
                                             RoundSigDigits(VentilTempLimit, 1) + ") ");
                            ShowContinueError("in the " + cCurrentModuleObject + " object = " + cAlphaArgs(1) + " and the simulation continues...");
                        }
                        // Check Maximum outdoor temperature value and schedule fields
                        if (!lNumericFieldBlanks(15) && ((!lAlphaFieldBlanks(10)) && Ventilation(Loop).MaxOutdoorTempSchedPtr == 0)) {
                            ShowWarningError(RoutineName + cAlphaFieldNames(10) + " = " + cAlphaArgs(10) +
                                             "is invalid. The constant value will be used at " + RoundSigDigits(rNumericArgs(15), 1) + " degrees C ");
                            ShowContinueError("in the " + cCurrentModuleObject + " object = " + cAlphaArgs(1) + " and the simulation continues...");
                        }
                    }

                    if (!lNumericFieldBlanks(16)) {
                        Ventilation(Loop).MaxWindSpeed = rNumericArgs(16);
                    } else {
                        Ventilation(Loop).MaxWindSpeed = VentilWSLimit;
                    }
                    if (Item1 == 1) {
                        if ((Ventilation(Loop).MaxWindSpeed < -VentilWSLimit) || (Ventilation(Loop).MaxWindSpeed > VentilWSLimit)) {
                            ShowSevereError(RoutineName + cCurrentModuleObject + " statement = " + cAlphaArgs(1) +
                                            " must have a maximum wind speed between -40 m/s and 40 m/s");
                            ErrorsFound = true;
                        }
                    }

                    // Report variables should be added for individual VENTILATION objects, in addition to zone totals below

                    if (Ventilation(Loop).ZonePtr > 0) {
                        if (RepVarSet(Ventilation(Loop).ZonePtr) && !Ventilation(Loop).QuadratureSum) {
                            RepVarSet(Ventilation(Loop).ZonePtr) = false;
                            SetupOutputVariable(state, "Zone Ventilation Sensible Heat Loss Energy",
                                                OutputProcessor::Unit::J,
                                                ZnAirRpt(Ventilation(Loop).ZonePtr).VentilHeatLoss,
                                                "System",
                                                "Sum",
                                                Zone(Ventilation(Loop).ZonePtr).Name);
                            SetupOutputVariable(state, "Zone Ventilation Sensible Heat Gain Energy",
                                                OutputProcessor::Unit::J,
                                                ZnAirRpt(Ventilation(Loop).ZonePtr).VentilHeatGain,
                                                "System",
                                                "Sum",
                                                Zone(Ventilation(Loop).ZonePtr).Name);
                            SetupOutputVariable(state, "Zone Ventilation Latent Heat Loss Energy",
                                                OutputProcessor::Unit::J,
                                                ZnAirRpt(Ventilation(Loop).ZonePtr).VentilLatentLoss,
                                                "System",
                                                "Sum",
                                                Zone(Ventilation(Loop).ZonePtr).Name);
                            SetupOutputVariable(state, "Zone Ventilation Latent Heat Gain Energy",
                                                OutputProcessor::Unit::J,
                                                ZnAirRpt(Ventilation(Loop).ZonePtr).VentilLatentGain,
                                                "System",
                                                "Sum",
                                                Zone(Ventilation(Loop).ZonePtr).Name);
                            SetupOutputVariable(state, "Zone Ventilation Total Heat Loss Energy",
                                                OutputProcessor::Unit::J,
                                                ZnAirRpt(Ventilation(Loop).ZonePtr).VentilTotalLoss,
                                                "System",
                                                "Sum",
                                                Zone(Ventilation(Loop).ZonePtr).Name);
                            SetupOutputVariable(state, "Zone Ventilation Total Heat Gain Energy",
                                                OutputProcessor::Unit::J,
                                                ZnAirRpt(Ventilation(Loop).ZonePtr).VentilTotalGain,
                                                "System",
                                                "Sum",
                                                Zone(Ventilation(Loop).ZonePtr).Name);
                            SetupOutputVariable(state, "Zone Ventilation Current Density Volume Flow Rate",
                                                OutputProcessor::Unit::m3_s,
                                                ZnAirRpt(Ventilation(Loop).ZonePtr).VentilVdotCurDensity,
                                                "System",
                                                "Average",
                                                Zone(Ventilation(Loop).ZonePtr).Name);
                            SetupOutputVariable(state, "Zone Ventilation Standard Density Volume Flow Rate",
                                                OutputProcessor::Unit::m3_s,
                                                ZnAirRpt(Ventilation(Loop).ZonePtr).VentilVdotStdDensity,
                                                "System",
                                                "Average",
                                                Zone(Ventilation(Loop).ZonePtr).Name);
                            SetupOutputVariable(state, "Zone Ventilation Current Density Volume",
                                                OutputProcessor::Unit::m3,
                                                ZnAirRpt(Ventilation(Loop).ZonePtr).VentilVolumeCurDensity,
                                                "System",
                                                "Sum",
                                                Zone(Ventilation(Loop).ZonePtr).Name);
                            SetupOutputVariable(state, "Zone Ventilation Standard Density Volume",
                                                OutputProcessor::Unit::m3,
                                                ZnAirRpt(Ventilation(Loop).ZonePtr).VentilVolumeStdDensity,
                                                "System",
                                                "Sum",
                                                Zone(Ventilation(Loop).ZonePtr).Name);
                            SetupOutputVariable(state, "Zone Ventilation Mass",
                                                OutputProcessor::Unit::kg,
                                                ZnAirRpt(Ventilation(Loop).ZonePtr).VentilMass,
                                                "System",
                                                "Sum",
                                                Zone(Ventilation(Loop).ZonePtr).Name);
                            SetupOutputVariable(state, "Zone Ventilation Mass Flow Rate",
                                                OutputProcessor::Unit::kg_s,
                                                ZnAirRpt(Ventilation(Loop).ZonePtr).VentilMdot,
                                                "System",
                                                "Average",
                                                Zone(Ventilation(Loop).ZonePtr).Name);
                            SetupOutputVariable(state, "Zone Ventilation Air Change Rate",
                                                OutputProcessor::Unit::ach,
                                                ZnAirRpt(Ventilation(Loop).ZonePtr).VentilAirChangeRate,
                                                "System",
                                                "Average",
                                                Zone(Ventilation(Loop).ZonePtr).Name);
                            SetupOutputVariable(state, "Zone Ventilation Fan Electricity Energy",
                                                OutputProcessor::Unit::J,
                                                ZnAirRpt(Ventilation(Loop).ZonePtr).VentilFanElec,
                                                "System",
                                                "Sum",
                                                Zone(Ventilation(Loop).ZonePtr).Name,
                                                _,
                                                "Electricity",
                                                "Fans",
                                                "Ventilation (simple)",
                                                "Building",
                                                Zone(Ventilation(Loop).ZonePtr).Name);
                            SetupOutputVariable(state, "Zone Ventilation Air Inlet Temperature",
                                                OutputProcessor::Unit::C,
                                                ZnAirRpt(Ventilation(Loop).ZonePtr).VentilAirTemp,
                                                "System",
                                                "Average",
                                                Zone(Ventilation(Loop).ZonePtr).Name);
                        }
                    }

                    if (AnyEnergyManagementSystemInModel) {
                        SetupEMSActuator("Zone Ventilation",
                                         Ventilation(Loop).Name,
                                         "Air Exchange Flow Rate",
                                         "[m3/s]",
                                         Ventilation(Loop).EMSSimpleVentOn,
                                         Ventilation(Loop).EMSimpleVentFlowRate);
                    }
                }
            }
        }

        cCurrentModuleObject = "ZoneVentilation:WindandStackOpenArea";
        VentiCount = TotDesignFlowVentilation;
        for (Loop = 1; Loop <= TotWindAndStackVentilation; ++Loop) {

            inputProcessor->getObjectItem(state,
                                          cCurrentModuleObject,
                                          Loop,
                                          cAlphaArgs,
                                          NumAlpha,
                                          rNumericArgs,
                                          NumNumber,
                                          IOStat,
                                          lNumericFieldBlanks,
                                          lAlphaFieldBlanks,
                                          cAlphaFieldNames,
                                          cNumericFieldNames);

            VentiCount = TotDesignFlowVentilation + Loop;
            UtilityRoutines::IsNameEmpty(cAlphaArgs(1), cCurrentModuleObject, ErrorsFound);

            Ventilation(VentiCount).Name = cAlphaArgs(1);
            Ventilation(VentiCount).ModelType = VentilationWindAndStack;

            Ventilation(VentiCount).ZonePtr = UtilityRoutines::FindItemInList(cAlphaArgs(2), Zone);
            if (Ventilation(VentiCount).ZonePtr == 0) {
                ShowSevereError(RoutineName + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\", invalid (not found) " + cAlphaFieldNames(2) +
                                "=\"" + cAlphaArgs(2) + "\".");
                ErrorsFound = true;
            }

            // setup a flag if the outdoor air balance method is applied
            if (Ventilation(VentiCount).ZonePtr > 0 && TotZoneAirBalance > 0) {
                for (i = 1; i <= TotZoneAirBalance; ++i) {
                    if (Ventilation(VentiCount).ZonePtr == ZoneAirBalance(i).ZonePtr) {
                        if (ZoneAirBalance(i).BalanceMethod == AirBalanceQuadrature) {
                            Ventilation(VentiCount).QuadratureSum = true;
                            Ventilation(VentiCount).OABalancePtr = i;
                            break;
                        }
                    }
                }
            }

            Ventilation(VentiCount).OpenArea = rNumericArgs(1);
            if (Ventilation(VentiCount).OpenArea < 0.0) {
                ShowSevereError(RoutineName + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\", " + cNumericFieldNames(1) + " must be positive.");
                ErrorsFound = true;
            }

            Ventilation(VentiCount).OpenAreaSchedPtr = GetScheduleIndex(state, cAlphaArgs(3));
            if (Ventilation(VentiCount).OpenAreaSchedPtr == 0) {
                if (lAlphaFieldBlanks(3)) {
                    ShowSevereError(RoutineName + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\", " + cAlphaFieldNames(3) +
                                    " is required but field is blank.");
                } else {
                    ShowSevereError(RoutineName + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\", invalid (not found) " + cAlphaFieldNames(3) +
                                    "=\"" + cAlphaArgs(3) + "\".");
                }
                ErrorsFound = true;
            }

            Ventilation(VentiCount).OpenEff = rNumericArgs(2);
            if (Ventilation(VentiCount).OpenEff != AutoCalculate &&
                (Ventilation(VentiCount).OpenEff < 0.0 || Ventilation(VentiCount).OpenEff > 1.0)) {
                ShowSevereError(RoutineName + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\", " + cNumericFieldNames(2) +
                                " must be between 0 and 1.");
                ErrorsFound = true;
            }

            Ventilation(VentiCount).EffAngle = rNumericArgs(3);
            if (Ventilation(VentiCount).EffAngle < 0.0 || Ventilation(VentiCount).EffAngle >= 360.0) {
                ShowSevereError(RoutineName + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\", " + cNumericFieldNames(3) +
                                " must be between 0 and 360.");
                ErrorsFound = true;
            }

            Ventilation(VentiCount).DH = rNumericArgs(4);
            if (Ventilation(VentiCount).DH < 0.0) {
                ShowSevereError(RoutineName + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\", " + cNumericFieldNames(4) + " must be positive.");
                ErrorsFound = true;
            }

            Ventilation(VentiCount).DiscCoef = rNumericArgs(5);
            if (Ventilation(VentiCount).DiscCoef != AutoCalculate &&
                (Ventilation(VentiCount).DiscCoef < 0.0 || Ventilation(VentiCount).DiscCoef > 1.0)) {
                ShowSevereError(RoutineName + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\", " + cNumericFieldNames(5) +
                                " must be between 0 and 1.");
                ErrorsFound = true;
            }

            if (!lNumericFieldBlanks(6)) {
                Ventilation(VentiCount).MinIndoorTemperature = rNumericArgs(6);
            } else {
                Ventilation(VentiCount).MinIndoorTemperature = -VentilTempLimit;
            }
            if ((Ventilation(VentiCount).MinIndoorTemperature < -VentilTempLimit) ||
                (Ventilation(VentiCount).MinIndoorTemperature > VentilTempLimit)) {
                ShowSevereError(RoutineName + cCurrentModuleObject + " statement = " + cAlphaArgs(1) + " must have " + cNumericFieldNames(6) +
                                " between -100C and 100C");
                ErrorsFound = true;
            }

            Ventilation(VentiCount).MinIndoorTempSchedPtr = GetScheduleIndex(state, cAlphaArgs(4));
            if (Ventilation(VentiCount).MinIndoorTempSchedPtr > 0) {
                if (!lNumericFieldBlanks(6))
                    ShowWarningError(
                        RoutineName +
                        "The Minimum Indoor Temperature value and schedule are provided. The scheduled temperature will be used in the " +
                        cCurrentModuleObject + " object = " + cAlphaArgs(1));
                // Check min and max values in the schedule to ensure both values are within the range
                if (!CheckScheduleValueMinMax(Ventilation(VentiCount).MinIndoorTempSchedPtr, ">=", -VentilTempLimit, "<=", VentilTempLimit)) {
                    ShowSevereError(RoutineName + cCurrentModuleObject + " statement = " + cAlphaArgs(1) +
                                    " must have a minimum indoor temperature between -100C and 100C defined in the schedule = " + cAlphaArgs(4));
                    ErrorsFound = true;
                }
            }
            if (Ventilation(VentiCount).MinIndoorTempSchedPtr == 0 && lNumericFieldBlanks(6) && (!lAlphaFieldBlanks(4))) {
                ShowWarningError(RoutineName + cNumericFieldNames(6) +
                                 ": the value field is blank and schedule field is invalid. The default value will be used (" +
                                 RoundSigDigits(-VentilTempLimit, 1) + ") ");
                ShowContinueError("in the " + cCurrentModuleObject + " object = " + cAlphaArgs(1) + " and the simulation continues...");
            }
            // Check Minimum indoor temperature value and schedule fields
            if (!lNumericFieldBlanks(6) && (!cAlphaArgs(4).empty() && Ventilation(VentiCount).MinIndoorTempSchedPtr == 0)) {
                ShowWarningError(RoutineName + cAlphaFieldNames(4) + " = " + cAlphaArgs(4) + " is invalid. The constant value will be used at " +
                                 RoundSigDigits(rNumericArgs(11), 1) + " degrees C ");
                ShowContinueError("in the " + cCurrentModuleObject + " object = " + cAlphaArgs(1) + " and the simulation continues...");
            }

            if (!lNumericFieldBlanks(7)) {
                Ventilation(VentiCount).MaxIndoorTemperature = rNumericArgs(7);
            } else {
                Ventilation(VentiCount).MaxIndoorTemperature = VentilTempLimit;
            }
            if ((Ventilation(VentiCount).MaxIndoorTemperature < -VentilTempLimit) ||
                (Ventilation(VentiCount).MaxIndoorTemperature > VentilTempLimit)) {
                ShowSevereError(RoutineName + cCurrentModuleObject + "=\"" + cAlphaArgs(1) +
                                "\" must have a maximum indoor temperature between -100C and 100C");
                ErrorsFound = true;
            }

            Ventilation(VentiCount).MaxIndoorTempSchedPtr = GetScheduleIndex(state, cAlphaArgs(5));
            if (Ventilation(VentiCount).MaxIndoorTempSchedPtr > 0) {
                if (!lNumericFieldBlanks(7))
                    ShowWarningError(
                        RoutineName +
                        "The Maximum Indoor Temperature value and schedule are provided. The scheduled temperature will be used in the " +
                        cCurrentModuleObject + " object = " + cAlphaArgs(1));
                // Check min and max values in the schedule to ensure both values are within the range
                if (!CheckScheduleValueMinMax(Ventilation(VentiCount).MaxIndoorTempSchedPtr, ">=", -VentilTempLimit, "<=", VentilTempLimit)) {
                    ShowSevereError(cCurrentModuleObject + " = " + cAlphaArgs(1) +
                                    " must have a maximum indoor temperature between -100C and 100C defined in the schedule = " + cAlphaArgs(5));
                    ErrorsFound = true;
                }
            }
            if (Ventilation(VentiCount).MaxIndoorTempSchedPtr == 0 && lNumericFieldBlanks(7) && (!lAlphaFieldBlanks(5))) {
                ShowWarningError(RoutineName + cNumericFieldNames(7) +
                                 ": the value field is blank and schedule field is invalid. The default value will be used (" +
                                 RoundSigDigits(VentilTempLimit, 1) + ") ");
                ShowContinueError("in the " + cCurrentModuleObject + " object = " + cAlphaArgs(1) + " and the simulation continues...");
            }
            // Check Maximum indoor temperature value and schedule fields
            if (!lNumericFieldBlanks(7) && ((!lAlphaFieldBlanks(5)) && Ventilation(VentiCount).MaxIndoorTempSchedPtr == 0)) {
                ShowWarningError(RoutineName + cAlphaFieldNames(7) + " = " + cAlphaArgs(5) + " is invalid. The constant value will be used at " +
                                 RoundSigDigits(rNumericArgs(7), 1) + " degrees C ");
                ShowContinueError("in the " + cCurrentModuleObject + " object = " + cAlphaArgs(1) + " and the simulation continues...");
            }

            if (!lNumericFieldBlanks(8)) {
                Ventilation(VentiCount).DelTemperature = rNumericArgs(8);
            } else {
                Ventilation(VentiCount).DelTemperature = -VentilTempLimit;
            }

            Ventilation(VentiCount).DeltaTempSchedPtr = GetScheduleIndex(state, cAlphaArgs(6));
            if (Ventilation(VentiCount).DeltaTempSchedPtr > 0) {
                if (!lNumericFieldBlanks(8))
                    ShowWarningError(RoutineName +
                                     "The Delta Temperature value and schedule are provided. The scheduled temperature will be used in the " +
                                     cCurrentModuleObject + " object = " + cAlphaArgs(1));
                // Check min value in the schedule to ensure both values are within the range
                if (GetScheduleMinValue(Ventilation(VentiCount).DeltaTempSchedPtr) < -VentilTempLimit) {
                    ShowSevereError(RoutineName + cCurrentModuleObject + " statement = " + cAlphaArgs(1) +
                                    " must have a delta temperature equal to or above -100C defined in the schedule = " + cAlphaArgs(8));
                    ErrorsFound = true;
                }
            }
            if (Ventilation(VentiCount).DeltaTempSchedPtr == 0 && lNumericFieldBlanks(8) && (!lAlphaFieldBlanks(6))) {
                ShowWarningError(RoutineName + cNumericFieldNames(8) +
                                 ": the value field is blank and schedule field is invalid. The default value will be used (" +
                                 RoundSigDigits(VentilTempLimit, 1) + ") ");
                ShowContinueError("in the " + cCurrentModuleObject + " object = " + cAlphaArgs(1) + " and the simulation continues...");
            }
            if (!lNumericFieldBlanks(8) && ((!lAlphaFieldBlanks(6)) && Ventilation(VentiCount).DeltaTempSchedPtr == 0)) {
                ShowWarningError(RoutineName + cAlphaFieldNames(6) + " = " + cAlphaArgs(6) + " is invalid. The constant value will be used at " +
                                 RoundSigDigits(rNumericArgs(8), 1) + " degrees C ");
                ShowContinueError("in the " + cCurrentModuleObject + " object = " + cAlphaArgs(1) + " and the simulation continues...");
            }

            if (!lNumericFieldBlanks(9)) {
                Ventilation(VentiCount).MinOutdoorTemperature = rNumericArgs(9);
            } else {
                Ventilation(VentiCount).MinOutdoorTemperature = -VentilTempLimit;
            }
            if ((Ventilation(VentiCount).MinOutdoorTemperature < -VentilTempLimit) ||
                (Ventilation(VentiCount).MinOutdoorTemperature > VentilTempLimit)) {
                ShowSevereError(RoutineName + cCurrentModuleObject + " statement = " + cAlphaArgs(1) + " must have " + cNumericFieldNames(9) +
                                " between -100C and 100C");
                ErrorsFound = true;
            }

            Ventilation(VentiCount).MinOutdoorTempSchedPtr = GetScheduleIndex(state, cAlphaArgs(7));
            if (Ventilation(VentiCount).MinOutdoorTempSchedPtr > 0) {
                if (!lNumericFieldBlanks(9))
                    ShowWarningError(
                        RoutineName +
                        "The Minimum Outdoor Temperature value and schedule are provided. The scheduled temperature will be used in the " +
                        cCurrentModuleObject + " object = " + cAlphaArgs(1));
                // Check min and max values in the schedule to ensure both values are within the range
                if (!CheckScheduleValueMinMax(Ventilation(VentiCount).MinOutdoorTempSchedPtr, ">=", -VentilTempLimit, "<=", VentilTempLimit)) {
                    ShowSevereError(RoutineName + cCurrentModuleObject + " statement = " + cAlphaArgs(1) +
                                    " must have a minimum outdoor temperature between -100C and 100C defined in the schedule = " + cAlphaArgs(7));
                    ErrorsFound = true;
                }
            }
            if (Ventilation(VentiCount).MinOutdoorTempSchedPtr == 0 && lNumericFieldBlanks(9) && (!lAlphaFieldBlanks(7))) {
                ShowWarningError(
                    RoutineName +
                    "Minimum Outdoor Temperature: the value field is blank and schedule field is invalid. The default value will be used (" +
                    RoundSigDigits(-VentilTempLimit, 1) + ") ");
                ShowContinueError("in the " + cCurrentModuleObject + " object = " + cAlphaArgs(1) + " and the simulation continues...");
            }
            // Check Minimum outdoor temperature value and schedule fields
            if (!lNumericFieldBlanks(9) && ((!lAlphaFieldBlanks(7)) && Ventilation(VentiCount).MinOutdoorTempSchedPtr == 0)) {
                ShowWarningError(RoutineName + cAlphaFieldNames(7) + " = " + cAlphaArgs(7) + " is invalid. The constant value will be used at " +
                                 RoundSigDigits(rNumericArgs(14), 1) + " degrees C ");
                ShowContinueError("in the " + cCurrentModuleObject + " object = " + cAlphaArgs(1) + " and the simulation continues...");
            }

            if (!lNumericFieldBlanks(10)) {
                Ventilation(VentiCount).MaxOutdoorTemperature = rNumericArgs(10);
            } else {
                Ventilation(VentiCount).MaxOutdoorTemperature = VentilTempLimit;
            }
            if ((Ventilation(VentiCount).MaxOutdoorTemperature < -VentilTempLimit) ||
                (Ventilation(VentiCount).MaxOutdoorTemperature > VentilTempLimit)) {
                ShowSevereError(RoutineName + cCurrentModuleObject + " statement = " + cAlphaArgs(1) + " must have a " + cNumericFieldNames(10) +
                                " between -100C and 100C");
                ErrorsFound = true;
            }

            Ventilation(VentiCount).MaxOutdoorTempSchedPtr = GetScheduleIndex(state, cAlphaArgs(8));
            if (Ventilation(VentiCount).MaxOutdoorTempSchedPtr > 0) {
                if (!lNumericFieldBlanks(10))
                    ShowWarningError(
                        RoutineName +
                        "The Maximum Outdoor Temperature value and schedule are provided. The scheduled temperature will be used in the " +
                        cCurrentModuleObject + " object = " + cAlphaArgs(1));
                if (!CheckScheduleValueMinMax(Ventilation(VentiCount).MaxOutdoorTempSchedPtr, ">=", -VentilTempLimit, "<=", VentilTempLimit)) {
                    ShowSevereError(RoutineName + cCurrentModuleObject + " statement = " + cAlphaArgs(1) +
                                    " must have a maximum outdoor temperature between -100C and 100C defined in the schedule = " + cAlphaArgs(8));
                    ErrorsFound = true;
                }
            }
            if (Ventilation(VentiCount).MaxOutdoorTempSchedPtr == 0 && lNumericFieldBlanks(10) && (!lAlphaFieldBlanks(8))) {
                ShowWarningError(RoutineName + cNumericFieldNames(10) +
                                 ": the value field is blank and schedule field is invalid. The default value will be used (" +
                                 RoundSigDigits(VentilTempLimit, 1) + ") ");
                ShowContinueError("in the " + cCurrentModuleObject + " object = " + cAlphaArgs(1) + " and the simulation continues...");
            }
            // Check Maximum outdoor temperature value and schedule fields
            if (!lNumericFieldBlanks(10) && ((!lAlphaFieldBlanks(8)) && Ventilation(VentiCount).MaxOutdoorTempSchedPtr == 0)) {
                ShowWarningError(RoutineName + cAlphaFieldNames(8) + " = " + cAlphaArgs(8) + "is invalid. The constant value will be used at " +
                                 RoundSigDigits(rNumericArgs(10), 1) + " degrees C ");
                ShowContinueError("in the " + cCurrentModuleObject + " object = " + cAlphaArgs(1) + " and the simulation continues...");
            }

            if (!lNumericFieldBlanks(11)) {
                Ventilation(VentiCount).MaxWindSpeed = rNumericArgs(11);
            } else {
                Ventilation(VentiCount).MaxWindSpeed = VentilWSLimit;
            }
            if ((Ventilation(VentiCount).MaxWindSpeed < -VentilWSLimit) || (Ventilation(VentiCount).MaxWindSpeed > VentilWSLimit)) {
                ShowSevereError(RoutineName + cCurrentModuleObject + " statement = " + cAlphaArgs(1) +
                                " must have a maximum wind speed between 0 m/s and 40 m/s");
                ErrorsFound = true;
            }

            // Report variables should be added for individual VENTILATION objects, in addition to zone totals below

            if (Ventilation(VentiCount).ZonePtr > 0) {
                if (RepVarSet(Ventilation(VentiCount).ZonePtr) && !Ventilation(Loop).QuadratureSum) {
                    RepVarSet(Ventilation(VentiCount).ZonePtr) = false;
                    SetupOutputVariable(state, "Zone Ventilation Sensible Heat Loss Energy",
                                        OutputProcessor::Unit::J,
                                        ZnAirRpt(Ventilation(VentiCount).ZonePtr).VentilHeatLoss,
                                        "System",
                                        "Sum",
                                        Zone(Ventilation(VentiCount).ZonePtr).Name);
                    SetupOutputVariable(state, "Zone Ventilation Sensible Heat Gain Energy",
                                        OutputProcessor::Unit::J,
                                        ZnAirRpt(Ventilation(VentiCount).ZonePtr).VentilHeatGain,
                                        "System",
                                        "Sum",
                                        Zone(Ventilation(VentiCount).ZonePtr).Name);
                    SetupOutputVariable(state, "Zone Ventilation Latent Heat Loss Energy",
                                        OutputProcessor::Unit::J,
                                        ZnAirRpt(Ventilation(VentiCount).ZonePtr).VentilLatentLoss,
                                        "System",
                                        "Sum",
                                        Zone(Ventilation(VentiCount).ZonePtr).Name);
                    SetupOutputVariable(state, "Zone Ventilation Latent Heat Gain Energy",
                                        OutputProcessor::Unit::J,
                                        ZnAirRpt(Ventilation(VentiCount).ZonePtr).VentilLatentGain,
                                        "System",
                                        "Sum",
                                        Zone(Ventilation(VentiCount).ZonePtr).Name);
                    SetupOutputVariable(state, "Zone Ventilation Total Heat Loss Energy",
                                        OutputProcessor::Unit::J,
                                        ZnAirRpt(Ventilation(VentiCount).ZonePtr).VentilTotalLoss,
                                        "System",
                                        "Sum",
                                        Zone(Ventilation(VentiCount).ZonePtr).Name);
                    SetupOutputVariable(state, "Zone Ventilation Total Heat Gain Energy",
                                        OutputProcessor::Unit::J,
                                        ZnAirRpt(Ventilation(VentiCount).ZonePtr).VentilTotalGain,
                                        "System",
                                        "Sum",
                                        Zone(Ventilation(VentiCount).ZonePtr).Name);
                    SetupOutputVariable(state, "Zone Ventilation Current Density Volume Flow Rate",
                                        OutputProcessor::Unit::m3_s,
                                        ZnAirRpt(Ventilation(VentiCount).ZonePtr).VentilVdotCurDensity,
                                        "System",
                                        "Average",
                                        Zone(Ventilation(VentiCount).ZonePtr).Name);
                    SetupOutputVariable(state, "Zone Ventilation Standard Density Volume Flow Rate",
                                        OutputProcessor::Unit::m3_s,
                                        ZnAirRpt(Ventilation(VentiCount).ZonePtr).VentilVdotStdDensity,
                                        "System",
                                        "Average",
                                        Zone(Ventilation(VentiCount).ZonePtr).Name);
                    SetupOutputVariable(state, "Zone Ventilation Current Density Volume",
                                        OutputProcessor::Unit::m3,
                                        ZnAirRpt(Ventilation(VentiCount).ZonePtr).VentilVolumeCurDensity,
                                        "System",
                                        "Sum",
                                        Zone(Ventilation(VentiCount).ZonePtr).Name);
                    SetupOutputVariable(state, "Zone Ventilation Standard Density Volume",
                                        OutputProcessor::Unit::m3,
                                        ZnAirRpt(Ventilation(VentiCount).ZonePtr).VentilVolumeStdDensity,
                                        "System",
                                        "Sum",
                                        Zone(Ventilation(VentiCount).ZonePtr).Name);
                    SetupOutputVariable(state, "Zone Ventilation Mass",
                                        OutputProcessor::Unit::kg,
                                        ZnAirRpt(Ventilation(VentiCount).ZonePtr).VentilMass,
                                        "System",
                                        "Sum",
                                        Zone(Ventilation(VentiCount).ZonePtr).Name);
                    SetupOutputVariable(state, "Zone Ventilation Mass Flow Rate",
                                        OutputProcessor::Unit::kg_s,
                                        ZnAirRpt(Ventilation(VentiCount).ZonePtr).VentilMdot,
                                        "System",
                                        "Average",
                                        Zone(Ventilation(VentiCount).ZonePtr).Name);
                    SetupOutputVariable(state, "Zone Ventilation Air Change Rate",
                                        OutputProcessor::Unit::ach,
                                        ZnAirRpt(Ventilation(VentiCount).ZonePtr).VentilAirChangeRate,
                                        "System",
                                        "Average",
                                        Zone(Ventilation(VentiCount).ZonePtr).Name);
                    SetupOutputVariable(state, "Zone Ventilation Fan Electricity Energy",
                                        OutputProcessor::Unit::J,
                                        ZnAirRpt(Ventilation(VentiCount).ZonePtr).VentilFanElec,
                                        "System",
                                        "Sum",
                                        Zone(Ventilation(VentiCount).ZonePtr).Name,
                                        _,
                                        "Electricity",
                                        "Fans",
                                        "Ventilation (simple)",
                                        "Building",
                                        Zone(Ventilation(VentiCount).ZonePtr).Name);
                    SetupOutputVariable(state, "Zone Ventilation Air Inlet Temperature",
                                        OutputProcessor::Unit::C,
                                        ZnAirRpt(Ventilation(VentiCount).ZonePtr).VentilAirTemp,
                                        "System",
                                        "Average",
                                        Zone(Ventilation(VentiCount).ZonePtr).Name);
                }
            }

            if (AnyEnergyManagementSystemInModel) {
                SetupEMSActuator("Zone Ventilation",
                                 Ventilation(VentiCount).Name,
                                 "Air Exchange Flow Rate",
                                 "[m3/s]",
                                 Ventilation(VentiCount).EMSSimpleVentOn,
                                 Ventilation(VentiCount).EMSimpleVentFlowRate);
            }
        }

        RepVarSet = true;

        cCurrentModuleObject = "ZoneMixing";
        TotMixing = inputProcessor->getNumObjectsFound(cCurrentModuleObject);
        Mixing.allocate(TotMixing);

        for (Loop = 1; Loop <= TotMixing; ++Loop) {

            inputProcessor->getObjectItem(state,
                                          cCurrentModuleObject,
                                          Loop,
                                          cAlphaArgs,
                                          NumAlpha,
                                          rNumericArgs,
                                          NumNumber,
                                          IOStat,
                                          lNumericFieldBlanks,
                                          lAlphaFieldBlanks,
                                          cAlphaFieldNames,
                                          cNumericFieldNames);
            UtilityRoutines::IsNameEmpty(cAlphaArgs(1), cCurrentModuleObject, ErrorsFound);

            Mixing(Loop).Name = cAlphaArgs(1);

            Mixing(Loop).ZonePtr = UtilityRoutines::FindItemInList(cAlphaArgs(2), Zone);
            if (Mixing(Loop).ZonePtr == 0) {
                ShowSevereError(RoutineName + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\", invalid (not found) " + cAlphaFieldNames(2) +
                                "=\"" + cAlphaArgs(2) + "\".");
                ErrorsFound = true;
            }

            Mixing(Loop).SchedPtr = GetScheduleIndex(state, cAlphaArgs(3));

            if (Mixing(Loop).SchedPtr == 0) {
                if (lAlphaFieldBlanks(3)) {
                    ShowSevereError(RoutineName + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\"," + cAlphaFieldNames(3) +
                                    " is required but field is blank.");
                } else {
                    ShowSevereError(RoutineName + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\", invalid (not found) " + cAlphaFieldNames(3) +
                                    "=\"" + cAlphaArgs(3) + "\".");
                }
                ErrorsFound = true;
            }

            // Mixing equipment design level calculation method
            {
                auto const SELECT_CASE_var(cAlphaArgs(4));
                if ((SELECT_CASE_var == "FLOW/ZONE") || (SELECT_CASE_var == "FLOW")) {
                    Mixing(Loop).DesignLevel = rNumericArgs(1);
                    if (lNumericFieldBlanks(1)) {
                        ShowWarningError(RoutineName + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\", " + cAlphaFieldNames(4) + " specifies " +
                                         cNumericFieldNames(1) + ", but that field is blank.  0 Mixing will result.");
                    }

                } else if (SELECT_CASE_var == "FLOW/AREA") {
                    if (Mixing(Loop).ZonePtr != 0) {
                        if (rNumericArgs(2) >= 0.0) {
                            Mixing(Loop).DesignLevel = rNumericArgs(2) * Zone(Mixing(Loop).ZonePtr).FloorArea;
                            if (Zone(Mixing(Loop).ZonePtr).FloorArea <= 0.0) {
                                ShowWarningError(RoutineName + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\", " + cAlphaFieldNames(4) +
                                                 " specifies " + cNumericFieldNames(2) + ", but Zone Floor Area = 0.  0 Mixing will result.");
                            }
                        } else {
                            ShowSevereError(RoutineName + cCurrentModuleObject + "=\"" + cAlphaArgs(1) +
                                            "\", invalid flow/person specification [<0.0]=" + RoundSigDigits(rNumericArgs(2), 3));
                            ErrorsFound = true;
                        }
                    }
                    if (lNumericFieldBlanks(2)) {
                        ShowWarningError(RoutineName + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\", " + cAlphaFieldNames(4) + " specifies " +
                                         cNumericFieldNames(2) + ", but that field is blank.  0 Mixing will result.");
                    }

                } else if (SELECT_CASE_var == "FLOW/PERSON") {
                    if (Mixing(Loop).ZonePtr != 0) {
                        if (rNumericArgs(3) >= 0.0) {
                            Mixing(Loop).DesignLevel = rNumericArgs(3) * Zone(Mixing(Loop).ZonePtr).TotOccupants;
                            if (Zone(Mixing(Loop).ZonePtr).TotOccupants <= 0.0) {
                                ShowWarningError(RoutineName + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\", " + cAlphaFieldNames(4) +
                                                 " specifies " + cNumericFieldNames(3) + ", but Zone Total Occupants = 0.  0 Mixing will result.");
                            }
                        } else {
                            ShowSevereError(RoutineName + cCurrentModuleObject + "=\"" + cAlphaArgs(1) +
                                            "\", invalid flow/person specification [<0.0]=" + RoundSigDigits(rNumericArgs(3), 3));
                            ErrorsFound = true;
                        }
                    }
                    if (lNumericFieldBlanks(3)) {
                        ShowWarningError(RoutineName + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\", " + cAlphaFieldNames(4) + " specifies " +
                                         cNumericFieldNames(3) + ", but that field is blank.  0 Mixing will result.");
                    }

                } else if (SELECT_CASE_var == "AIRCHANGES/HOUR") {
                    if (Mixing(Loop).ZonePtr != 0) {
                        if (rNumericArgs(4) >= 0.0) {
                            Mixing(Loop).DesignLevel = rNumericArgs(4) * Zone(Mixing(Loop).ZonePtr).Volume / SecInHour;
                            if (Zone(Mixing(Loop).ZonePtr).Volume <= 0.0) {
                                ShowWarningError(RoutineName + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\", " + cAlphaFieldNames(4) +
                                                 " specifies " + cNumericFieldNames(4) + ", but Zone Volume = 0.  0 Mixing will result.");
                            }
                        } else {
                            ShowSevereError(RoutineName + cCurrentModuleObject + "=\"" + cAlphaArgs(1) +
                                            "\", invalid flow/person specification [<0.0]=" + RoundSigDigits(rNumericArgs(4), 3));
                            ErrorsFound = true;
                        }
                    }
                    if (lNumericFieldBlanks(4)) {
                        ShowWarningError(RoutineName + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\", " + cAlphaFieldNames(4) + " specifies " +
                                         cNumericFieldNames(4) + ", but that field is blank.  0 Mixing will result.");
                    }

                } else {
                    ShowSevereError(RoutineName + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\", invalid calculation method=" + cAlphaArgs(4));
                    ErrorsFound = true;
                }
            }

            Mixing(Loop).FromZone = UtilityRoutines::FindItemInList(cAlphaArgs(5), Zone);
            if (Mixing(Loop).FromZone == 0) {
                ShowSevereError(RoutineName + cAlphaFieldNames(5) + " not found=" + cAlphaArgs(5) + " for " + cCurrentModuleObject + '=' +
                                cAlphaArgs(1));
                ErrorsFound = true;
            }
            Mixing(Loop).DeltaTemperature = rNumericArgs(5);

            if (NumAlpha > 5) {
                Mixing(Loop).DeltaTempSchedPtr = GetScheduleIndex(state, cAlphaArgs(6));
                if (Mixing(Loop).DeltaTempSchedPtr > 0) {
                    if (!lNumericFieldBlanks(5))
                        ShowWarningError(RoutineName +
                                         "The Delta Temperature value and schedule are provided. The scheduled temperature will be used in the " +
                                         cCurrentModuleObject + " object = " + cAlphaArgs(1));
                    if (GetScheduleMinValue(Mixing(Loop).DeltaTempSchedPtr) < -MixingTempLimit) {
                        ShowSevereError(RoutineName + cCurrentModuleObject + " statement = " + cAlphaArgs(1) +
                                        " must have a delta temperature equal to or above -100C defined in the schedule = " + cAlphaArgs(6));
                        ErrorsFound = true;
                    }
                }
            }
            if (Mixing(Loop).DeltaTempSchedPtr == 0 && lNumericFieldBlanks(5) && (!lAlphaFieldBlanks(6))) {
                ShowWarningError(RoutineName + cNumericFieldNames(5) +
                                 ": the value field is blank and schedule field is invalid. The default value will be used (" +
                                 RoundSigDigits(rNumericArgs(5), 1) + ") ");
                ShowContinueError("in the " + cCurrentModuleObject + " object = " + cAlphaArgs(1) + " and the simulation continues...");
            }
            if (!lNumericFieldBlanks(5) && ((!lAlphaFieldBlanks(6)) && Mixing(Loop).DeltaTempSchedPtr == 0)) {
                ShowWarningError(RoutineName + cAlphaFieldNames(6) + " = " + cAlphaArgs(6) + " is invalid. The constant value will be used at " +
                                 RoundSigDigits(rNumericArgs(5), 1) + " degrees C ");
                ShowContinueError("in the " + cCurrentModuleObject + " object = " + cAlphaArgs(1) + " and the simulation continues...");
            }

            if (NumAlpha > 6) {
                Mixing(Loop).MinIndoorTempSchedPtr = GetScheduleIndex(state, cAlphaArgs(7));
                if (Mixing(Loop).MinIndoorTempSchedPtr == 0) {
                    if ((!lAlphaFieldBlanks(7))) {
                        ShowSevereError(RoutineName + cAlphaFieldNames(7) + " not found=" + cAlphaArgs(7) + " for " + cCurrentModuleObject + '=' +
                                        cAlphaArgs(1));
                        ErrorsFound = true;
                    }
                }
                if (Mixing(Loop).MinIndoorTempSchedPtr > 0) {
                    // Check min and max values in the schedule to ensure both values are within the range
                    if (!CheckScheduleValueMinMax(Mixing(Loop).MinIndoorTempSchedPtr, ">=", -MixingTempLimit, "<=", MixingTempLimit)) {
                        ShowSevereError(RoutineName + cCurrentModuleObject + " statement = " + cAlphaArgs(1) +
                                        " must have a minimum zone temperature between -100C and 100C defined in the schedule = " + cAlphaArgs(7));
                        ErrorsFound = true;
                    }
                }
            }

            if (NumAlpha > 7) {
                Mixing(Loop).MaxIndoorTempSchedPtr = GetScheduleIndex(state, cAlphaArgs(8));
                if (Mixing(Loop).MaxIndoorTempSchedPtr == 0) {
                    if ((!lAlphaFieldBlanks(8))) {
                        ShowSevereError(RoutineName + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\", " + cAlphaFieldNames(8) + " not found=\"" +
                                        cAlphaArgs(8) + "\".");
                        ErrorsFound = true;
                    }
                }
                if (Mixing(Loop).MaxIndoorTempSchedPtr > 0) {
                    // Check min and max values in the schedule to ensure both values are within the range
                    if (!CheckScheduleValueMinMax(Mixing(Loop).MaxIndoorTempSchedPtr, ">=", -MixingTempLimit, "<=", MixingTempLimit)) {
                        ShowSevereError(RoutineName + cCurrentModuleObject + "=\"" + cAlphaArgs(1) +
                                        "\" must have a maximum zone temperature between -100C and 100C defined in the schedule = " + cAlphaArgs(8));
                        ErrorsFound = true;
                    }
                }
            }

            if (NumAlpha > 8) {
                Mixing(Loop).MinSourceTempSchedPtr = GetScheduleIndex(state, cAlphaArgs(9));
                if (Mixing(Loop).MinSourceTempSchedPtr == 0) {
                    if ((!lAlphaFieldBlanks(9))) {
                        ShowSevereError(RoutineName + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\", " + cAlphaFieldNames(9) + " not found=\"" +
                                        cAlphaArgs(9) + "\".");
                        ErrorsFound = true;
                    }
                }
                if (Mixing(Loop).MinSourceTempSchedPtr > 0) {
                    // Check min and max values in the schedule to ensure both values are within the range
                    if (!CheckScheduleValueMinMax(Mixing(Loop).MinSourceTempSchedPtr, ">=", -MixingTempLimit, "<=", MixingTempLimit)) {
                        ShowSevereError(
                            RoutineName + cCurrentModuleObject + "=\"" + cAlphaArgs(1) +
                            "\" must have a minimum source temperature between -100C and 100C defined in the schedule = " + cAlphaArgs(9));
                        ErrorsFound = true;
                    }
                }
            }

            if (NumAlpha > 9) {
                Mixing(Loop).MaxSourceTempSchedPtr = GetScheduleIndex(state, cAlphaArgs(10));
                if (Mixing(Loop).MaxSourceTempSchedPtr == 0) {
                    if ((!lAlphaFieldBlanks(10))) {
                        ShowSevereError(RoutineName + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\", " + cAlphaFieldNames(10) + " not found=\"" +
                                        cAlphaArgs(10) + "\".");
                        ErrorsFound = true;
                    }
                }
                if (Mixing(Loop).MaxSourceTempSchedPtr > 0) {
                    // Check min and max values in the schedule to ensure both values are within the range
                    if (!CheckScheduleValueMinMax(Mixing(Loop).MaxSourceTempSchedPtr, ">=", -MixingTempLimit, "<=", MixingTempLimit)) {
                        ShowSevereError(
                            RoutineName + cCurrentModuleObject + " statement =\"" + cAlphaArgs(1) +
                            "\" must have a maximum source temperature between -100C and 100C defined in the schedule = " + cAlphaArgs(10));
                        ErrorsFound = true;
                    }
                }
            }

            if (NumAlpha > 10) {
                Mixing(Loop).MinOutdoorTempSchedPtr = GetScheduleIndex(state, cAlphaArgs(11));
                if (Mixing(Loop).MinOutdoorTempSchedPtr == 0) {
                    if ((!lAlphaFieldBlanks(11))) {
                        ShowSevereError(RoutineName + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\", " + cAlphaFieldNames(11) + " not found=\"" +
                                        cAlphaArgs(11) + "\".");
                        ErrorsFound = true;
                    }
                }
                if (Mixing(Loop).MinOutdoorTempSchedPtr > 0) {
                    // Check min and max values in the schedule to ensure both values are within the range
                    if (!CheckScheduleValueMinMax(Mixing(Loop).MinOutdoorTempSchedPtr, ">=", -MixingTempLimit, "<=", MixingTempLimit)) {
                        ShowSevereError(
                            RoutineName + cCurrentModuleObject + " =\"" + cAlphaArgs(1) +
                            "\" must have a minimum outdoor temperature between -100C and 100C defined in the schedule = " + cAlphaArgs(11));
                        ErrorsFound = true;
                    }
                }
            }

            if (NumAlpha > 11) {
                Mixing(Loop).MaxOutdoorTempSchedPtr = GetScheduleIndex(state, cAlphaArgs(12));
                if (Mixing(Loop).MaxOutdoorTempSchedPtr == 0) {
                    if ((!lAlphaFieldBlanks(12))) {
                        ShowSevereError(RoutineName + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\", " + cAlphaFieldNames(12) + " not found=\"" +
                                        cAlphaArgs(12) + "\".");
                        ErrorsFound = true;
                    }
                }
                if (Mixing(Loop).MaxOutdoorTempSchedPtr > 0) {
                    // Check min and max values in the schedule to ensure both values are within the range
                    if (!CheckScheduleValueMinMax(Mixing(Loop).MaxOutdoorTempSchedPtr, ">=", -MixingTempLimit, "<=", MixingTempLimit)) {
                        ShowSevereError(
                            RoutineName + cCurrentModuleObject + " =\"" + cAlphaArgs(1) +
                            "\" must have a maximum outdoor temperature between -100C and 100C defined in the schedule = " + cAlphaArgs(12));
                        ErrorsFound = true;
                    }
                }
            }

            if (Mixing(Loop).ZonePtr > 0) {
                if (RepVarSet(Mixing(Loop).ZonePtr)) {
                    RepVarSet(Mixing(Loop).ZonePtr) = false;
                    SetupOutputVariable(state, "Zone Mixing Volume",
                                        OutputProcessor::Unit::m3,
                                        ZnAirRpt(Mixing(Loop).ZonePtr).MixVolume,
                                        "System",
                                        "Sum",
                                        Zone(Mixing(Loop).ZonePtr).Name);
                    SetupOutputVariable(state, "Zone Mixing Current Density Volume Flow Rate",
                                        OutputProcessor::Unit::m3_s,
                                        ZnAirRpt(Mixing(Loop).ZonePtr).MixVdotCurDensity,
                                        "System",
                                        "Average",
                                        Zone(Mixing(Loop).ZonePtr).Name);
                    SetupOutputVariable(state, "Zone Mixing Standard Density Volume Flow Rate",
                                        OutputProcessor::Unit::m3_s,
                                        ZnAirRpt(Mixing(Loop).ZonePtr).MixVdotStdDensity,
                                        "System",
                                        "Average",
                                        Zone(Mixing(Loop).ZonePtr).Name);
                    SetupOutputVariable(state, "Zone Mixing Mass",
                                        OutputProcessor::Unit::kg,
                                        ZnAirRpt(Mixing(Loop).ZonePtr).MixMass,
                                        "System",
                                        "Sum",
                                        Zone(Mixing(Loop).ZonePtr).Name);
                    SetupOutputVariable(state, "Zone Mixing Mass Flow Rate",
                                        OutputProcessor::Unit::kg_s,
                                        ZnAirRpt(Mixing(Loop).ZonePtr).MixMdot,
                                        "System",
                                        "Average",
                                        Zone(Mixing(Loop).ZonePtr).Name);
                    SetupOutputVariable(state, "Zone Mixing Sensible Heat Loss Energy",
                                        OutputProcessor::Unit::J,
                                        ZnAirRpt(Mixing(Loop).ZonePtr).MixHeatLoss,
                                        "System",
                                        "Sum",
                                        Zone(Mixing(Loop).ZonePtr).Name);
                    SetupOutputVariable(state, "Zone Mixing Sensible Heat Gain Energy",
                                        OutputProcessor::Unit::J,
                                        ZnAirRpt(Mixing(Loop).ZonePtr).MixHeatGain,
                                        "System",
                                        "Sum",
                                        Zone(Mixing(Loop).ZonePtr).Name);
                    SetupOutputVariable(state, "Zone Mixing Latent Heat Loss Energy",
                                        OutputProcessor::Unit::J,
                                        ZnAirRpt(Mixing(Loop).ZonePtr).MixLatentLoss,
                                        "System",
                                        "Sum",
                                        Zone(Mixing(Loop).ZonePtr).Name);
                    SetupOutputVariable(state, "Zone Mixing Latent Heat Gain Energy",
                                        OutputProcessor::Unit::J,
                                        ZnAirRpt(Mixing(Loop).ZonePtr).MixLatentGain,
                                        "System",
                                        "Sum",
                                        Zone(Mixing(Loop).ZonePtr).Name);
                    SetupOutputVariable(state, "Zone Mixing Total Heat Loss Energy",
                                        OutputProcessor::Unit::J,
                                        ZnAirRpt(Mixing(Loop).ZonePtr).MixTotalLoss,
                                        "System",
                                        "Sum",
                                        Zone(Mixing(Loop).ZonePtr).Name);
                    SetupOutputVariable(state, "Zone Mixing Total Heat Gain Energy",
                                        OutputProcessor::Unit::J,
                                        ZnAirRpt(Mixing(Loop).ZonePtr).MixTotalGain,
                                        "System",
                                        "Sum",
                                        Zone(Mixing(Loop).ZonePtr).Name);
                }
            }
            if (AnyEnergyManagementSystemInModel) {
                SetupEMSActuator("ZoneMixing",
                                 Mixing(Loop).Name,
                                 "Air Exchange Flow Rate",
                                 "[m3/s]",
                                 Mixing(Loop).EMSSimpleMixingOn,
                                 Mixing(Loop).EMSimpleMixingFlowRate);
            }
        }

        // allocate MassConservation
        MassConservation.allocate(NumOfZones);

        // added by BAN, 02/14
        if (TotMixing > 0) {
            ZoneMixingNum.allocate(TotMixing);
            // get source zones mixing objects index
            for (ZoneNum = 1; ZoneNum <= NumOfZones; ++ZoneNum) {
                SourceCount = 0;
                for (Loop = 1; Loop <= TotMixing; ++Loop) {
                    if (ZoneNum == Mixing(Loop).FromZone) {
                        SourceCount += 1;
                        ZoneMixingNum(SourceCount) = Loop;
                    }
                }
                // save mixing objects index for zones which serve as a source zone
                MassConservation(ZoneNum).NumSourceZonesMixingObject = SourceCount;
                if (SourceCount > 0) {
                    MassConservation(ZoneNum).ZoneMixingSourcesPtr.allocate(SourceCount);
                    for (Loop = 1; Loop <= SourceCount; ++Loop) {
                        MassConservation(ZoneNum).ZoneMixingSourcesPtr(Loop) = ZoneMixingNum(Loop);
                    }
                }
            }

            // check zones which are used only as a source zones
            for (ZoneNum = 1; ZoneNum <= NumOfZones; ++ZoneNum) {
                IsSourceZone = false;
                for (Loop = 1; Loop <= TotMixing; ++Loop) {
                    if (ZoneNum != Mixing(Loop).FromZone) continue;
                    MassConservation(ZoneNum).IsOnlySourceZone = true;
                    for (Loop1 = 1; Loop1 <= TotMixing; ++Loop1) {
                        if (ZoneNum == Mixing(Loop1).ZonePtr) {
                            MassConservation(ZoneNum).IsOnlySourceZone = false;
                            break;
                        }
                    }
                }
            }
            // get receiving zones mixing objects index
            ZoneMixingNum = 0;
            for (ZoneNum = 1; ZoneNum <= NumOfZones; ++ZoneNum) {
                ReceivingCount = 0;
                for (Loop = 1; Loop <= TotMixing; ++Loop) {
                    if (ZoneNum == Mixing(Loop).ZonePtr) {
                        ReceivingCount += 1;
                        ZoneMixingNum(ReceivingCount) = Loop;
                    }
                }
                // save mixing objects index for zones which serve as a receiving zone
                MassConservation(ZoneNum).NumReceivingZonesMixingObject = ReceivingCount;
                if (ReceivingCount > 0) {
                    MassConservation(ZoneNum).ZoneMixingReceivingPtr.allocate(ReceivingCount);
                    MassConservation(ZoneNum).ZoneMixingReceivingFr.allocate(ReceivingCount);
                    for (Loop = 1; Loop <= ReceivingCount; ++Loop) {
                        MassConservation(ZoneNum).ZoneMixingReceivingPtr(Loop) = ZoneMixingNum(Loop);
                    }
                }
            }
            if (allocated(ZoneMixingNum)) ZoneMixingNum.deallocate();
        }

        // zone mass conservation calculation order starts with receiving zones
        // and then proceeds to source zones
        Loop = 0;
        for (ZoneNum = 1; ZoneNum <= NumOfZones; ++ZoneNum) {
            if (!MassConservation(ZoneNum).IsOnlySourceZone) {
                Loop += 1;
                ZoneReOrder(Loop) = ZoneNum;
            }
        }
        for (ZoneNum = 1; ZoneNum <= NumOfZones; ++ZoneNum) {
            if (MassConservation(ZoneNum).IsOnlySourceZone) {
                Loop += 1;
                ZoneReOrder(Loop) = ZoneNum;
            }
        }

        cCurrentModuleObject = "ZoneCrossMixing";
        int inputCrossMixing = inputProcessor->getNumObjectsFound(cCurrentModuleObject);
        TotCrossMixing = inputCrossMixing + DataHeatBalance::NumAirBoundaryMixing;
        CrossMixing.allocate(TotCrossMixing);

        for (Loop = 1; Loop <= TotCrossMixing; ++Loop) {

            if (Loop > inputCrossMixing) {
                // Create CrossMixing object from air boundary info
                int airBoundaryIndex = Loop - inputCrossMixing - 1; //zero-based
                int zone1 = DataHeatBalance::AirBoundaryMixingZone1[airBoundaryIndex];
                int zone2 = DataHeatBalance::AirBoundaryMixingZone2[airBoundaryIndex];
                CrossMixing(Loop).Name = "Air Boundary Mixing Zones " + General::RoundSigDigits(zone1) + " and " + General::RoundSigDigits(zone2);
                CrossMixing(Loop).ZonePtr = zone1;
                CrossMixing(Loop).SchedPtr = DataHeatBalance::AirBoundaryMixingSched[airBoundaryIndex];
                CrossMixing(Loop).DesignLevel = DataHeatBalance::AirBoundaryMixingVol[airBoundaryIndex];
                CrossMixing(Loop).FromZone = zone2;
            }
            else {
                inputProcessor->getObjectItem(state,
                                              cCurrentModuleObject,
                                              Loop,
                                              cAlphaArgs,
                                              NumAlpha,
                                              rNumericArgs,
                                              NumNumber,
                                              IOStat,
                                              lNumericFieldBlanks,
                                              lAlphaFieldBlanks,
                                              cAlphaFieldNames,
                                              cNumericFieldNames);
                UtilityRoutines::IsNameEmpty(cAlphaArgs(1), cCurrentModuleObject, ErrorsFound);

                CrossMixing(Loop).Name = cAlphaArgs(1);

                CrossMixing(Loop).ZonePtr = UtilityRoutines::FindItemInList(cAlphaArgs(2), Zone);
                if (CrossMixing(Loop).ZonePtr == 0) {
                    ShowSevereError(RoutineName + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\", invalid (not found) " + cAlphaFieldNames(2) +
                        "=\"" + cAlphaArgs(2) + "\".");
                    ErrorsFound = true;
                }

                CrossMixing(Loop).SchedPtr = GetScheduleIndex(state, cAlphaArgs(3));
                if (CrossMixing(Loop).SchedPtr == 0) {
                    if (lAlphaFieldBlanks(3)) {
                        ShowSevereError(RoutineName + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\"," + cAlphaFieldNames(3) +
                            " is required but field is blank.");
                    }
                    else {
                        ShowSevereError(RoutineName + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\", invalid (not found) " + cAlphaFieldNames(3) +
                            "=\"" + cAlphaArgs(3) + "\".");
                    }
                    ErrorsFound = true;
                }

                // Mixing equipment design level calculation method.
                {
                    auto const SELECT_CASE_var(cAlphaArgs(4));
                    if ((SELECT_CASE_var == "FLOW/ZONE") || (SELECT_CASE_var == "FLOW")) {
                        CrossMixing(Loop).DesignLevel = rNumericArgs(1);
                        if (lNumericFieldBlanks(1)) {
                            ShowWarningError(RoutineName + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\", " + cAlphaFieldNames(4) + " specifies " +
                                cNumericFieldNames(1) + ", but that field is blank.  0 Cross Mixing will result.");
                        }

                    }
                    else if (SELECT_CASE_var == "FLOW/AREA") {
                        if (CrossMixing(Loop).ZonePtr != 0) {
                            if (rNumericArgs(2) >= 0.0) {
                                CrossMixing(Loop).DesignLevel = rNumericArgs(2) * Zone(CrossMixing(Loop).ZonePtr).FloorArea;
                                if (Zone(CrossMixing(Loop).ZonePtr).FloorArea <= 0.0) {
                                    ShowWarningError(RoutineName + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\", " + cAlphaFieldNames(4) +
                                        " specifies " + cNumericFieldNames(2) + ", but Zone Floor Area = 0.  0 Cross Mixing will result.");
                                }
                            }
                            else {
                                ShowSevereError(RoutineName + cCurrentModuleObject + "=\"" + cAlphaArgs(1) +
                                    "\", invalid flow/person specification [<0.0]=" + RoundSigDigits(rNumericArgs(2), 3));
                                ErrorsFound = true;
                            }
                        }
                        if (lNumericFieldBlanks(2)) {
                            ShowWarningError(RoutineName + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\", " + cAlphaFieldNames(4) + " specifies " +
                                cNumericFieldNames(2) + ", but that field is blank.  0 Cross Mixing will result.");
                        }

                    }
                    else if (SELECT_CASE_var == "FLOW/PERSON") {
                        if (CrossMixing(Loop).ZonePtr != 0) {
                            if (rNumericArgs(3) >= 0.0) {
                                CrossMixing(Loop).DesignLevel = rNumericArgs(3) * Zone(CrossMixing(Loop).ZonePtr).TotOccupants;
                                if (Zone(CrossMixing(Loop).ZonePtr).TotOccupants <= 0.0) {
                                    ShowWarningError(RoutineName + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\", " + cAlphaFieldNames(4) +
                                        " specifies " + cNumericFieldNames(3) +
                                        ", but Zone Total Occupants = 0.  0 Cross Mixing will result.");
                                }
                            }
                            else {
                                ShowSevereError(RoutineName + cCurrentModuleObject + "=\"" + cAlphaArgs(1) +
                                    "\", invalid flow/person specification [<0.0]=" + RoundSigDigits(rNumericArgs(3), 3));
                                ErrorsFound = true;
                            }
                        }
                        if (lNumericFieldBlanks(3)) {
                            ShowWarningError(RoutineName + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\", " + cAlphaFieldNames(4) + " specifies " +
                                cNumericFieldNames(3) + ", but that field is blank.  0 Cross Mixing will result.");
                        }

                    }
                    else if (SELECT_CASE_var == "AIRCHANGES/HOUR") {
                        if (CrossMixing(Loop).ZonePtr != 0) {
                            if (rNumericArgs(4) >= 0.0) {
                                CrossMixing(Loop).DesignLevel = rNumericArgs(4) * Zone(CrossMixing(Loop).ZonePtr).Volume / SecInHour;
                                if (Zone(CrossMixing(Loop).ZonePtr).Volume <= 0.0) {
                                    ShowWarningError(RoutineName + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\", " + cAlphaFieldNames(4) +
                                        " specifies " + cNumericFieldNames(4) + ", but Zone Volume = 0.  0 Cross Mixing will result.");
                                }
                            }
                            else {
                                ShowSevereError(RoutineName + cCurrentModuleObject + "=\"" + cAlphaArgs(1) +
                                    "\", invalid flow/person specification [<0.0]=" + RoundSigDigits(rNumericArgs(4), 3));
                                ErrorsFound = true;
                            }
                        }
                        if (lNumericFieldBlanks(4)) {
                            ShowWarningError(RoutineName + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\", " + cAlphaFieldNames(4) + " specifies " +
                                cNumericFieldNames(4) + ", but that field is blank.  0 Cross Mixing will result.");
                        }

                    }
                    else {
                        ShowSevereError(RoutineName + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\", invalid calculation method=" + cAlphaArgs(4));
                        ErrorsFound = true;
                    }
                }

                CrossMixing(Loop).FromZone = UtilityRoutines::FindItemInList(cAlphaArgs(5), Zone);
                if (CrossMixing(Loop).FromZone == 0) {
                    ShowSevereError(RoutineName + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\", invalid (not found) " + cAlphaFieldNames(5) +
                        "=\"" + cAlphaArgs(5) + "\".");
                    ErrorsFound = true;
                }
                CrossMixing(Loop).DeltaTemperature = rNumericArgs(5);

                if (NumAlpha > 5) {
                    CrossMixing(Loop).DeltaTempSchedPtr = GetScheduleIndex(state, cAlphaArgs(6));
                    if (CrossMixing(Loop).DeltaTempSchedPtr > 0) {
                        if (!lNumericFieldBlanks(5))
                            ShowWarningError(RoutineName +
                                "The Delta Temperature value and schedule are provided. The scheduled temperature will be used in the " +
                                cCurrentModuleObject + " object = " + cAlphaArgs(1));
                        if (GetScheduleMinValue(CrossMixing(Loop).DeltaTempSchedPtr) < 0.0) {
                            ShowSevereError(RoutineName + cCurrentModuleObject + " = " + cAlphaArgs(1) +
                                " must have a delta temperature equal to or above 0 C defined in the schedule = " + cAlphaArgs(6));
                            ErrorsFound = true;
                        }
                    }
                }
                if (CrossMixing(Loop).DeltaTempSchedPtr == 0 && lNumericFieldBlanks(5) && (!lAlphaFieldBlanks(6))) {
                    ShowWarningError(RoutineName + cNumericFieldNames(5) +
                        ": the value field is blank and schedule field is invalid. The default value will be used (" +
                        RoundSigDigits(rNumericArgs(5), 1) + ") ");
                    ShowContinueError("in " + cCurrentModuleObject + " = " + cAlphaArgs(1) + " and the simulation continues...");
                }
                if (!lNumericFieldBlanks(5) && ((!lAlphaFieldBlanks(6)) && CrossMixing(Loop).DeltaTempSchedPtr == 0)) {
                    ShowWarningError(RoutineName + cAlphaFieldNames(6) + " = " + cAlphaArgs(6) + " is invalid. The constant value will be used at " +
                        RoundSigDigits(rNumericArgs(5), 1) + " degrees C ");
                    ShowContinueError("in the " + cCurrentModuleObject + " object = " + cAlphaArgs(1) + " and the simulation continues...");
                }

                if (NumAlpha > 6) {
                    CrossMixing(Loop).MinIndoorTempSchedPtr = GetScheduleIndex(state, cAlphaArgs(7));
                    if (CrossMixing(Loop).MinIndoorTempSchedPtr == 0) {
                        if ((!lAlphaFieldBlanks(7))) {
                            ShowSevereError(RoutineName + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\"," + cAlphaFieldNames(7) +
                                " not found=" + cAlphaArgs(7) + "\".");
                            ErrorsFound = true;
                        }
                    }
                    if (CrossMixing(Loop).MinIndoorTempSchedPtr > 0) {
                        // Check min and max values in the schedule to ensure both values are within the range
                        if (!CheckScheduleValueMinMax(CrossMixing(Loop).MinIndoorTempSchedPtr, ">=", -MixingTempLimit, "<=", MixingTempLimit)) {
                            ShowSevereError(RoutineName + cCurrentModuleObject + " = " + cAlphaArgs(1) +
                                " must have a minimum zone temperature between -100C and 100C defined in the schedule = " + cAlphaArgs(7));
                            ErrorsFound = true;
                        }
                    }
                }

                if (NumAlpha > 7) {
                    CrossMixing(Loop).MaxIndoorTempSchedPtr = GetScheduleIndex(state, cAlphaArgs(8));
                    if (CrossMixing(Loop).MaxIndoorTempSchedPtr == 0) {
                        if ((!lAlphaFieldBlanks(8))) {
                            ShowSevereError(RoutineName + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\"," + cAlphaFieldNames(8) + " not found=\"" +
                                cAlphaArgs(8) + "\".");
                            ErrorsFound = true;
                        }
                    }
                    if (CrossMixing(Loop).MaxIndoorTempSchedPtr > 0) {
                        // Check min and max values in the schedule to ensure both values are within the range
                        if (!CheckScheduleValueMinMax(CrossMixing(Loop).MaxIndoorTempSchedPtr, ">=", -MixingTempLimit, "<=", MixingTempLimit)) {
                            ShowSevereError(RoutineName + cCurrentModuleObject + " = " + cAlphaArgs(1) +
                                " must have a maximum zone temperature between -100C and 100C defined in the schedule = " + cAlphaArgs(8));
                            ErrorsFound = true;
                        }
                    }
                }

                if (NumAlpha > 8) {
                    CrossMixing(Loop).MinSourceTempSchedPtr = GetScheduleIndex(state, cAlphaArgs(9));
                    if (CrossMixing(Loop).MinSourceTempSchedPtr == 0) {
                        if ((!lAlphaFieldBlanks(9))) {
                            ShowSevereError(RoutineName + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\"," + cAlphaFieldNames(9) + " not found=\"" +
                                cAlphaArgs(9) + "\".");
                            ErrorsFound = true;
                        }
                    }
                    if (CrossMixing(Loop).MinSourceTempSchedPtr > 0) {
                        // Check min and max values in the schedule to ensure both values are within the range
                        if (!CheckScheduleValueMinMax(CrossMixing(Loop).MinSourceTempSchedPtr, ">=", -MixingTempLimit, "<=", MixingTempLimit)) {
                            ShowSevereError(RoutineName + cCurrentModuleObject + " = " + cAlphaArgs(1) +
                                " must have a minimum source temperature between -100C and 100C defined in the schedule = " + cAlphaArgs(9));
                            ErrorsFound = true;
                        }
                    }
                }

                if (NumAlpha > 9) {
                    CrossMixing(Loop).MaxSourceTempSchedPtr = GetScheduleIndex(state, cAlphaArgs(10));
                    if (CrossMixing(Loop).MaxSourceTempSchedPtr == 0) {
                        if ((!lAlphaFieldBlanks(10))) {
                            ShowSevereError(RoutineName + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\"," + cAlphaFieldNames(10) + " not found=\"" +
                                cAlphaArgs(9) + "\".");
                            ErrorsFound = true;
                        }
                    }
                    if (CrossMixing(Loop).MaxSourceTempSchedPtr > 0) {
                        // Check min and max values in the schedule to ensure both values are within the range
                        if (!CheckScheduleValueMinMax(CrossMixing(Loop).MaxSourceTempSchedPtr, ">=", -MixingTempLimit, "<=", MixingTempLimit)) {
                            ShowSevereError(RoutineName + cCurrentModuleObject + " = " + cAlphaArgs(1) +
                                " must have a maximum source temperature between -100C and 100C defined in the schedule = " + cAlphaArgs(10));
                            ErrorsFound = true;
                        }
                    }
                }

                if (NumAlpha > 10) {
                    CrossMixing(Loop).MinOutdoorTempSchedPtr = GetScheduleIndex(state, cAlphaArgs(11));
                    if (CrossMixing(Loop).MinOutdoorTempSchedPtr == 0) {
                        if ((!lAlphaFieldBlanks(11))) {
                            ShowSevereError(RoutineName + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\"," + cAlphaFieldNames(11) + " not found=\"" +
                                cAlphaArgs(9) + "\".");
                            ErrorsFound = true;
                        }
                    }
                    if (CrossMixing(Loop).MinOutdoorTempSchedPtr > 0) {
                        // Check min and max values in the schedule to ensure both values are within the range
                        if (!CheckScheduleValueMinMax(CrossMixing(Loop).MinOutdoorTempSchedPtr, ">=", -MixingTempLimit, "<=", MixingTempLimit)) {
                            ShowSevereError(
                                RoutineName + cCurrentModuleObject + " = " + cAlphaArgs(1) +
                                " must have a minimum outdoor temperature between -100C and 100C defined in the schedule = " + cAlphaArgs(11));
                            ErrorsFound = true;
                        }
                    }
                }

                if (NumAlpha > 11) {
                    CrossMixing(Loop).MaxOutdoorTempSchedPtr = GetScheduleIndex(state, cAlphaArgs(12));
                    if (CrossMixing(Loop).MaxOutdoorTempSchedPtr == 0) {
                        if ((!lAlphaFieldBlanks(12))) {
                            ShowSevereError(RoutineName + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\"," + cAlphaFieldNames(12) + " not found=\"" +
                                cAlphaArgs(9) + "\".");
                            ErrorsFound = true;
                        }
                    }
                    if (CrossMixing(Loop).MaxOutdoorTempSchedPtr > 0) {
                        // Check min and max values in the schedule to ensure both values are within the range
                        if (!CheckScheduleValueMinMax(CrossMixing(Loop).MaxOutdoorTempSchedPtr, ">=", -MixingTempLimit, "<=", MixingTempLimit)) {
                            ShowSevereError(
                                RoutineName + cCurrentModuleObject + " = " + cAlphaArgs(1) +
                                " must have a maximum outdoor temperature between -100C and 100C defined in the schedule = " + cAlphaArgs(12));
                            ErrorsFound = true;
                        }
                    }
                }
            }

            if (CrossMixing(Loop).ZonePtr > 0) {
                if (RepVarSet(CrossMixing(Loop).ZonePtr)) {
                    RepVarSet(CrossMixing(Loop).ZonePtr) = false;
                    SetupOutputVariable(state, "Zone Mixing Volume",
                                        OutputProcessor::Unit::m3,
                                        ZnAirRpt(CrossMixing(Loop).ZonePtr).MixVolume,
                                        "System",
                                        "Sum",
                                        Zone(CrossMixing(Loop).ZonePtr).Name);
                    SetupOutputVariable(state, "Zone Mixing Current Density Volume Flow Rate",
                                        OutputProcessor::Unit::m3_s,
                                        ZnAirRpt(CrossMixing(Loop).ZonePtr).MixVdotCurDensity,
                                        "System",
                                        "Average",
                                        Zone(CrossMixing(Loop).ZonePtr).Name);
                    SetupOutputVariable(state, "Zone Mixing Standard Density Volume Flow Rate",
                                        OutputProcessor::Unit::m3_s,
                                        ZnAirRpt(CrossMixing(Loop).ZonePtr).MixVdotStdDensity,
                                        "System",
                                        "Average",
                                        Zone(CrossMixing(Loop).ZonePtr).Name);
                    SetupOutputVariable(state, "Zone Mixing Mass",
                                        OutputProcessor::Unit::kg,
                                        ZnAirRpt(CrossMixing(Loop).ZonePtr).MixMass,
                                        "System",
                                        "Sum",
                                        Zone(CrossMixing(Loop).ZonePtr).Name);
                    SetupOutputVariable(state, "Zone Mixing Mass Flow Rate",
                                        OutputProcessor::Unit::kg_s,
                                        ZnAirRpt(CrossMixing(Loop).ZonePtr).MixMdot,
                                        "System",
                                        "Average",
                                        Zone(CrossMixing(Loop).ZonePtr).Name);
                    SetupOutputVariable(state, "Zone Mixing Sensible Heat Loss Energy",
                                        OutputProcessor::Unit::J,
                                        ZnAirRpt(CrossMixing(Loop).ZonePtr).MixHeatLoss,
                                        "System",
                                        "Sum",
                                        Zone(CrossMixing(Loop).ZonePtr).Name);
                    SetupOutputVariable(state, "Zone Mixing Sensible Heat Gain Energy",
                                        OutputProcessor::Unit::J,
                                        ZnAirRpt(CrossMixing(Loop).ZonePtr).MixHeatGain,
                                        "System",
                                        "Sum",
                                        Zone(CrossMixing(Loop).ZonePtr).Name);
                    SetupOutputVariable(state, "Zone Mixing Latent Heat Loss Energy",
                                        OutputProcessor::Unit::J,
                                        ZnAirRpt(CrossMixing(Loop).ZonePtr).MixLatentLoss,
                                        "System",
                                        "Sum",
                                        Zone(CrossMixing(Loop).ZonePtr).Name);
                    SetupOutputVariable(state, "Zone Mixing Latent Heat Gain Energy",
                                        OutputProcessor::Unit::J,
                                        ZnAirRpt(CrossMixing(Loop).ZonePtr).MixLatentGain,
                                        "System",
                                        "Sum",
                                        Zone(CrossMixing(Loop).ZonePtr).Name);
                    SetupOutputVariable(state, "Zone Mixing Total Heat Loss Energy",
                                        OutputProcessor::Unit::J,
                                        ZnAirRpt(CrossMixing(Loop).ZonePtr).MixTotalLoss,
                                        "System",
                                        "Sum",
                                        Zone(CrossMixing(Loop).ZonePtr).Name);
                    SetupOutputVariable(state, "Zone Mixing Total Heat Gain Energy",
                                        OutputProcessor::Unit::J,
                                        ZnAirRpt(CrossMixing(Loop).ZonePtr).MixTotalGain,
                                        "System",
                                        "Sum",
                                        Zone(CrossMixing(Loop).ZonePtr).Name);
                }
            }
            if (CrossMixing(Loop).FromZone > 0) {
                if (RepVarSet(CrossMixing(Loop).FromZone)) {
                    RepVarSet(CrossMixing(Loop).FromZone) = false;
                    SetupOutputVariable(state, "Zone Mixing Volume",
                                        OutputProcessor::Unit::m3,
                                        ZnAirRpt(CrossMixing(Loop).FromZone).MixVolume,
                                        "System",
                                        "Sum",
                                        Zone(CrossMixing(Loop).FromZone).Name);
                    SetupOutputVariable(state, "Zone Mixing Current Density Volume Flow Rate",
                                        OutputProcessor::Unit::m3_s,
                                        ZnAirRpt(CrossMixing(Loop).FromZone).MixVdotCurDensity,
                                        "System",
                                        "Average",
                                        Zone(CrossMixing(Loop).FromZone).Name);
                    SetupOutputVariable(state, "Zone Mixing Standard Density Volume Flow Rate",
                                        OutputProcessor::Unit::m3_s,
                                        ZnAirRpt(CrossMixing(Loop).FromZone).MixVdotStdDensity,
                                        "System",
                                        "Average",
                                        Zone(CrossMixing(Loop).FromZone).Name);
                    SetupOutputVariable(state, "Zone Mixing Mass",
                                        OutputProcessor::Unit::kg,
                                        ZnAirRpt(CrossMixing(Loop).FromZone).MixMass,
                                        "System",
                                        "Sum",
                                        Zone(CrossMixing(Loop).FromZone).Name);
                    SetupOutputVariable(state, "Zone Mixing Mass Flow Rate",
                                        OutputProcessor::Unit::kg_s,
                                        ZnAirRpt(CrossMixing(Loop).FromZone).MixMdot,
                                        "System",
                                        "Average",
                                        Zone(CrossMixing(Loop).FromZone).Name);
                    SetupOutputVariable(state, "Zone Mixing Sensible Heat Loss Energy",
                                        OutputProcessor::Unit::J,
                                        ZnAirRpt(CrossMixing(Loop).FromZone).MixHeatLoss,
                                        "System",
                                        "Sum",
                                        Zone(CrossMixing(Loop).FromZone).Name);
                    SetupOutputVariable(state, "Zone Mixing Sensible Heat Gain Energy",
                                        OutputProcessor::Unit::J,
                                        ZnAirRpt(CrossMixing(Loop).FromZone).MixHeatGain,
                                        "System",
                                        "Sum",
                                        Zone(CrossMixing(Loop).FromZone).Name);
                    SetupOutputVariable(state, "Zone Mixing Latent Heat Loss Energy",
                                        OutputProcessor::Unit::J,
                                        ZnAirRpt(CrossMixing(Loop).FromZone).MixLatentLoss,
                                        "System",
                                        "Sum",
                                        Zone(CrossMixing(Loop).FromZone).Name);
                    SetupOutputVariable(state, "Zone Mixing Latent Heat Gain Energy",
                                        OutputProcessor::Unit::J,
                                        ZnAirRpt(CrossMixing(Loop).FromZone).MixLatentGain,
                                        "System",
                                        "Sum",
                                        Zone(CrossMixing(Loop).FromZone).Name);
                    SetupOutputVariable(state, "Zone Mixing Total Heat Loss Energy",
                                        OutputProcessor::Unit::J,
                                        ZnAirRpt(CrossMixing(Loop).FromZone).MixTotalLoss,
                                        "System",
                                        "Sum",
                                        Zone(CrossMixing(Loop).FromZone).Name);
                    SetupOutputVariable(state, "Zone Mixing Total Heat Gain Energy",
                                        OutputProcessor::Unit::J,
                                        ZnAirRpt(CrossMixing(Loop).FromZone).MixTotalGain,
                                        "System",
                                        "Sum",
                                        Zone(CrossMixing(Loop).FromZone).Name);
                }
            }

            if (AnyEnergyManagementSystemInModel) {
                SetupEMSActuator("ZoneCrossMixing",
                                 CrossMixing(Loop).Name,
                                 "Air Exchange Flow Rate",
                                 "[m3/s]",
                                 CrossMixing(Loop).EMSSimpleMixingOn,
                                 CrossMixing(Loop).EMSimpleMixingFlowRate);
            }
        }

        cCurrentModuleObject = "ZoneRefrigerationDoorMixing";
        TotRefDoorMixing = inputProcessor->getNumObjectsFound(cCurrentModuleObject);
        if (TotRefDoorMixing > 0) {
            RefDoorMixing.allocate(NumOfZones);
            for (auto &e : RefDoorMixing)
                e.NumRefDoorConnections = 0;

            for (Loop = 1; Loop <= TotRefDoorMixing; ++Loop) {

                inputProcessor->getObjectItem(state,
                                              cCurrentModuleObject,
                                              Loop,
                                              cAlphaArgs,
                                              NumAlpha,
                                              rNumericArgs,
                                              NumNumber,
                                              IOStat,
                                              lNumericFieldBlanks,
                                              lAlphaFieldBlanks,
                                              cAlphaFieldNames,
                                              cNumericFieldNames);
                UtilityRoutines::IsNameEmpty(cAlphaArgs(1), cCurrentModuleObject, ErrorsFound);

                NameThisObject = cAlphaArgs(1);

                AlphaNum = 2;
                Zone1Num = UtilityRoutines::FindItemInList(cAlphaArgs(AlphaNum), Zone);
                if (Zone1Num == 0) {
                    ShowSevereError(RoutineName + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\", invalid (not found) " +
                                    cAlphaFieldNames(AlphaNum) + "=\"" + cAlphaArgs(AlphaNum) + "\".");
                    ErrorsFound = true;
                }

                ++AlphaNum; // 3
                Zone2Num = UtilityRoutines::FindItemInList(cAlphaArgs(AlphaNum), Zone);
                if (Zone2Num == 0) {
                    ShowSevereError(RoutineName + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\", invalid (not found) " +
                                    cAlphaFieldNames(AlphaNum) + "=\"" + cAlphaArgs(AlphaNum) + "\".");
                    ErrorsFound = true;
                }
                if (Zone1Num == Zone2Num) {
                    ShowSevereError(RoutineName + cCurrentModuleObject + "=\"" + cAlphaArgs(1) +
                                    "\", The same zone name has been entered for both sides of a refrigerated door " + cAlphaFieldNames(AlphaNum) +
                                    "=\"" + cAlphaArgs(AlphaNum) + "\".");
                    ErrorsFound = true;
                } else if (Zone1Num < Zone2Num) { // zone 1 will come first in soln loop, id zone 2 as mate zone
                    ZoneNumA = Zone1Num;
                    ZoneNumB = Zone2Num;
                } else if (Zone2Num < Zone1Num) { // zone 2 will come first in soln loop, id zone 1 as mate zone
                    ZoneNumA = Zone2Num;
                    ZoneNumB = Zone1Num;
                }

                if (!allocated(RefDoorMixing(ZoneNumA).OpenSchedPtr)) {
                    RefDoorMixing(ZoneNumA).DoorMixingObjectName.allocate(NumOfZones);
                    RefDoorMixing(ZoneNumA).OpenSchedPtr.allocate(NumOfZones);
                    RefDoorMixing(ZoneNumA).DoorHeight.allocate(NumOfZones);
                    RefDoorMixing(ZoneNumA).DoorArea.allocate(NumOfZones);
                    RefDoorMixing(ZoneNumA).Protection.allocate(NumOfZones);
                    RefDoorMixing(ZoneNumA).MateZonePtr.allocate(NumOfZones);
                    RefDoorMixing(ZoneNumA).EMSRefDoorMixingOn.allocate(NumOfZones);
                    RefDoorMixing(ZoneNumA).EMSRefDoorFlowRate.allocate(NumOfZones);
                    RefDoorMixing(ZoneNumA).VolRefDoorFlowRate.allocate(NumOfZones);
                    RefDoorMixing(ZoneNumA).DoorProtTypeName.allocate(NumOfZones);
                    RefDoorMixing(ZoneNumA).DoorMixingObjectName = "";
                    RefDoorMixing(ZoneNumA).OpenSchedPtr = 0;
                    RefDoorMixing(ZoneNumA).DoorHeight = 0.0;
                    RefDoorMixing(ZoneNumA).DoorArea = 0.0;
                    RefDoorMixing(ZoneNumA).Protection = RefDoorNone;
                    RefDoorMixing(ZoneNumA).MateZonePtr = 0;
                    RefDoorMixing(ZoneNumA).EMSRefDoorMixingOn = false;
                    RefDoorMixing(ZoneNumA).EMSRefDoorFlowRate = 0.0;
                    RefDoorMixing(ZoneNumA).VolRefDoorFlowRate = 0.0;
                    RefDoorMixing(ZoneNumA).DoorProtTypeName = "";
                } // First refrigeration mixing in this zone

                if (!allocated(RefDoorMixing(ZoneNumB).OpenSchedPtr)) {
                    RefDoorMixing(ZoneNumB).DoorMixingObjectName.allocate(NumOfZones);
                    RefDoorMixing(ZoneNumB).OpenSchedPtr.allocate(NumOfZones);
                    RefDoorMixing(ZoneNumB).DoorHeight.allocate(NumOfZones);
                    RefDoorMixing(ZoneNumB).DoorArea.allocate(NumOfZones);
                    RefDoorMixing(ZoneNumB).Protection.allocate(NumOfZones);
                    RefDoorMixing(ZoneNumB).MateZonePtr.allocate(NumOfZones);
                    RefDoorMixing(ZoneNumB).EMSRefDoorMixingOn.allocate(NumOfZones);
                    RefDoorMixing(ZoneNumB).EMSRefDoorFlowRate.allocate(NumOfZones);
                    RefDoorMixing(ZoneNumB).VolRefDoorFlowRate.allocate(NumOfZones);
                    RefDoorMixing(ZoneNumB).DoorProtTypeName.allocate(NumOfZones);
                    RefDoorMixing(ZoneNumB).DoorMixingObjectName = "";
                    RefDoorMixing(ZoneNumB).OpenSchedPtr = 0;
                    RefDoorMixing(ZoneNumB).DoorHeight = 0.0;
                    RefDoorMixing(ZoneNumB).DoorArea = 0.0;
                    RefDoorMixing(ZoneNumB).Protection = RefDoorNone;
                    RefDoorMixing(ZoneNumB).MateZonePtr = 0;
                    RefDoorMixing(ZoneNumB).EMSRefDoorMixingOn = false;
                    RefDoorMixing(ZoneNumB).EMSRefDoorFlowRate = 0.0;
                    RefDoorMixing(ZoneNumB).VolRefDoorFlowRate = 0.0;
                    RefDoorMixing(ZoneNumB).DoorProtTypeName = "";
                } // First refrigeration mixing in this zone

                ConnectionNumber = RefDoorMixing(ZoneNumA).NumRefDoorConnections + 1;
                RefDoorMixing(ZoneNumA).NumRefDoorConnections = ConnectionNumber;
                RefDoorMixing(ZoneNumA).ZonePtr = ZoneNumA;
                RefDoorMixing(ZoneNumA).MateZonePtr(ConnectionNumber) = ZoneNumB;
                RefDoorMixing(ZoneNumA).DoorMixingObjectName(ConnectionNumber) = NameThisObject;
                // need to make sure same pair of zones is only entered once.
                if (RefDoorMixing(ZoneNumA).RefDoorMixFlag && RefDoorMixing(ZoneNumB).RefDoorMixFlag) {
                    if (RefDoorMixing(ZoneNumA).NumRefDoorConnections > 1) {
                        for (ConnectTest = 1; ConnectTest <= (ConnectionNumber - 1); ++ConnectTest) {
                            if (RefDoorMixing(ZoneNumA).MateZonePtr(ConnectTest) != RefDoorMixing(ZoneNumA).MateZonePtr(ConnectionNumber)) continue;
                            ShowSevereError(RoutineName + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\", and " +
                                            RefDoorMixing(ZoneNumA).DoorMixingObjectName(ConnectTest));
                            ShowContinueError(" Share same pair of zones: \"" + Zone(ZoneNumA).Name + "\" and \"" + Zone(ZoneNumB).Name +
                                              "\". Only one RefrigerationDoorMixing object is allowed for any unique pair of zones.");
                            ErrorsFound = true;
                        } // ConnectTest
                    }     // NumRefDoorconnections > 1
                } else {  // Both zones need to be flagged with ref doors
                    RefDoorMixing(ZoneNumA).RefDoorMixFlag = true;
                    RefDoorMixing(ZoneNumB).RefDoorMixFlag = true;
                } // Both zones already flagged with ref doors

                ++AlphaNum; // 4
                if (lAlphaFieldBlanks(AlphaNum)) {
                    ShowSevereError(RoutineName + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\"," + cAlphaFieldNames(AlphaNum) +
                                    " is required but field is blank.");
                    ErrorsFound = true;
                } else { //(lAlphaFieldBlanks(AlphaNum)) THEN
                    RefDoorMixing(ZoneNumA).OpenSchedPtr(ConnectionNumber) = GetScheduleIndex(state, cAlphaArgs(AlphaNum));
                    if (RefDoorMixing(ZoneNumA).OpenSchedPtr(ConnectionNumber) == 0) {
                        ShowSevereError(RoutineName + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\", invalid (not found) " +
                                        cAlphaFieldNames(AlphaNum) + "=\"" + cAlphaArgs(AlphaNum) + "\".");
                        ErrorsFound = true;
                    } else { // OpenSchedPtr(ConnectionNumber) ne 0)
                        if (!CheckScheduleValueMinMax(RefDoorMixing(ZoneNumA).OpenSchedPtr(ConnectionNumber), ">=", 0.0, "<=", 1.0)) {
                            ShowSevereError(RoutineName + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\"," + cAlphaFieldNames(AlphaNum) + "=\"" +
                                            cAlphaArgs(AlphaNum) + "\" has schedule values < 0 or > 1.");
                            ErrorsFound = true;
                        } // check door opening schedule values between 0 and 1
                    }     // OpenSchedPtr(ConnectionNumber) == 0)
                }         //(lAlphaFieldBlanks(AlphaNum)) THEN

                NumbNum = 1;
                if (lNumericFieldBlanks(NumbNum)) {
                    RefDoorMixing(ZoneNumA).DoorHeight(ConnectionNumber) = 3.0; // default height of 3 meters
                    ShowWarningError(RoutineName + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + cNumericFieldNames(NumbNum) +
                                     " is blank and the default value of 3.0 will be used.");
                } else {
                    RefDoorMixing(ZoneNumA).DoorHeight(ConnectionNumber) = rNumericArgs(NumbNum);
                    if ((RefDoorMixing(ZoneNumA).DoorHeight(ConnectionNumber) < 0) || (RefDoorMixing(ZoneNumA).DoorHeight(ConnectionNumber) > 50.0)) {
                        ShowSevereError(RoutineName + cCurrentModuleObject + " = " + cAlphaArgs(1) +
                                        " must have a door height between 0 and 50 meters. ");
                        ErrorsFound = true;
                    }
                }

                ++NumbNum; // 2
                if (lNumericFieldBlanks(NumbNum)) {
                    RefDoorMixing(ZoneNumA).DoorArea(ConnectionNumber) = 9.0; // default area of 9 m2
                    ShowWarningError(RoutineName + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + cNumericFieldNames(NumbNum) +
                                     " is blank and the default value of 9 m2 will be used.");
                } else {
                    RefDoorMixing(ZoneNumA).DoorArea(ConnectionNumber) = rNumericArgs(NumbNum);
                    if ((RefDoorMixing(ZoneNumA).DoorArea(ConnectionNumber) < 0) || (RefDoorMixing(ZoneNumA).DoorArea(ConnectionNumber) > 400.0)) {
                        ShowSevereError(RoutineName + cCurrentModuleObject + " = " + cAlphaArgs(1) +
                                        " must have a door height between 0 and 400 square meters. ");
                        ErrorsFound = true;
                    }
                }

                ++AlphaNum; // 5
                // Door protection type.
                if (lAlphaFieldBlanks(AlphaNum)) {
                    RefDoorMixing(ZoneNumA).Protection(ConnectionNumber) = RefDoorNone;  // Default
                    RefDoorMixing(ZoneNumA).DoorProtTypeName(ConnectionNumber) = "None"; // Default
                    ShowWarningError(RoutineName + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\"  " + cAlphaFieldNames(AlphaNum) +
                                     " is blank. Default of no door protection will be used");
                } else {
                    if (cAlphaArgs(AlphaNum) == "NONE") {
                        RefDoorMixing(ZoneNumA).Protection(ConnectionNumber) = RefDoorNone;
                        RefDoorMixing(ZoneNumA).DoorProtTypeName(ConnectionNumber) = "None";
                    } else if (cAlphaArgs(AlphaNum) == "AIRCURTAIN") {
                        RefDoorMixing(ZoneNumA).Protection(ConnectionNumber) = RefDoorAirCurtain;
                        RefDoorMixing(ZoneNumA).DoorProtTypeName(ConnectionNumber) = "AirCurtain";
                    } else if (cAlphaArgs(AlphaNum) == "STRIPCURTAIN") {
                        RefDoorMixing(ZoneNumA).Protection(ConnectionNumber) = RefDoorStripCurtain;
                        RefDoorMixing(ZoneNumA).DoorProtTypeName(ConnectionNumber) = "StripCurtain";
                    } else {
                        ShowSevereError(RoutineName + cCurrentModuleObject + "=\"" + cAlphaArgs(1) +
                                        "\", invalid calculation method=" + cAlphaArgs(AlphaNum) + " with alphanum of 5: " + cAlphaArgs(5));
                        ErrorsFound = true;
                    } // =none, etc.
                }     // Blank

                if (ZoneNumA > 0) {
                    if (RepVarSet(ZoneNumA)) {
                        RepVarSet(ZoneNumA) = false;
                        SetupOutputVariable(state,
                            "Zone Mixing Volume", OutputProcessor::Unit::m3, ZnAirRpt(ZoneNumA).MixVolume, "System", "Sum", Zone(ZoneNumA).Name);
                        SetupOutputVariable(state, "Zone Mixing Current Density Volume Flow Rate",
                                            OutputProcessor::Unit::m3_s,
                                            ZnAirRpt(ZoneNumA).MixVdotCurDensity,
                                            "System",
                                            "Average",
                                            Zone(ZoneNumA).Name);
                        SetupOutputVariable(state, "Zone Mixing Standard Density Volume Flow Rate",
                                            OutputProcessor::Unit::m3_s,
                                            ZnAirRpt(ZoneNumA).MixVdotStdDensity,
                                            "System",
                                            "Average",
                                            Zone(ZoneNumA).Name);
                        SetupOutputVariable(state,
                            "Zone Mixing Mass", OutputProcessor::Unit::kg, ZnAirRpt(ZoneNumA).MixMass, "System", "Sum", Zone(ZoneNumA).Name);
                        SetupOutputVariable(state, "Zone Mixing Mass Flow Rate",
                                            OutputProcessor::Unit::kg_s,
                                            ZnAirRpt(ZoneNumA).MixMdot,
                                            "System",
                                            "Average",
                                            Zone(ZoneNumA).Name);
                        SetupOutputVariable(state, "Zone Mixing Sensible Heat Loss Energy",
                                            OutputProcessor::Unit::J,
                                            ZnAirRpt(ZoneNumA).MixHeatLoss,
                                            "System",
                                            "Sum",
                                            Zone(ZoneNumA).Name);
                        SetupOutputVariable(state, "Zone Mixing Sensible Heat Gain Energy",
                                            OutputProcessor::Unit::J,
                                            ZnAirRpt(ZoneNumA).MixHeatGain,
                                            "System",
                                            "Sum",
                                            Zone(ZoneNumA).Name);
                        SetupOutputVariable(state, "Zone Mixing Latent Heat Loss Energy",
                                            OutputProcessor::Unit::J,
                                            ZnAirRpt(ZoneNumA).MixLatentLoss,
                                            "System",
                                            "Sum",
                                            Zone(ZoneNumA).Name);
                        SetupOutputVariable(state, "Zone Mixing Latent Heat Gain Energy",
                                            OutputProcessor::Unit::J,
                                            ZnAirRpt(ZoneNumA).MixLatentGain,
                                            "System",
                                            "Sum",
                                            Zone(ZoneNumA).Name);
                        SetupOutputVariable(state, "Zone Mixing Total Heat Loss Energy",
                                            OutputProcessor::Unit::J,
                                            ZnAirRpt(ZoneNumA).MixTotalLoss,
                                            "System",
                                            "Sum",
                                            Zone(ZoneNumA).Name);
                        SetupOutputVariable(state, "Zone Mixing Total Heat Gain Energy",
                                            OutputProcessor::Unit::J,
                                            ZnAirRpt(ZoneNumA).MixTotalGain,
                                            "System",
                                            "Sum",
                                            Zone(ZoneNumA).Name);
                    }
                }
                if (AnyEnergyManagementSystemInModel) {
                    SetupEMSActuator("ZoneRefDoorMixing",
                                     RefDoorMixing(ZoneNumA).Name,
                                     "Air Exchange Flow Rate",
                                     "[m3/s]",
                                     RefDoorMixing(ZoneNumA).EMSRefDoorMixingOn(ConnectionNumber),
                                     RefDoorMixing(ZoneNumA).EMSRefDoorFlowRate(ConnectionNumber));
                }

                if (ZoneNumB > 0) {
                    if (RepVarSet(ZoneNumB)) {
                        RepVarSet(ZoneNumB) = false;
                        SetupOutputVariable(state,
                            "Zone Mixing Volume", OutputProcessor::Unit::m3, ZnAirRpt(ZoneNumB).MixVolume, "System", "Sum", Zone(ZoneNumB).Name);
                        SetupOutputVariable(state, "Zone Mixing Current Density Volume Flow Rate",
                                            OutputProcessor::Unit::m3_s,
                                            ZnAirRpt(ZoneNumB).MixVdotCurDensity,
                                            "System",
                                            "Average",
                                            Zone(ZoneNumB).Name);
                        SetupOutputVariable(state, "Zone Mixing Standard Density Volume Flow Rate",
                                            OutputProcessor::Unit::m3_s,
                                            ZnAirRpt(ZoneNumB).MixVdotStdDensity,
                                            "System",
                                            "Average",
                                            Zone(ZoneNumB).Name);
                        SetupOutputVariable(state,
                            "Zone Mixing Mass", OutputProcessor::Unit::kg, ZnAirRpt(ZoneNumB).MixMass, "System", "Sum", Zone(ZoneNumB).Name);
                        SetupOutputVariable(state, "Zone Mixing Mass Flow Rate",
                                            OutputProcessor::Unit::kg_s,
                                            ZnAirRpt(ZoneNumB).MixMdot,
                                            "System",
                                            "Average",
                                            Zone(ZoneNumB).Name);
                        SetupOutputVariable(state, "Zone Mixing Sensible Heat Loss Energy",
                                            OutputProcessor::Unit::J,
                                            ZnAirRpt(ZoneNumB).MixHeatLoss,
                                            "System",
                                            "Sum",
                                            Zone(ZoneNumB).Name);
                        SetupOutputVariable(state, "Zone Mixing Sensible Heat Gain Energy",
                                            OutputProcessor::Unit::J,
                                            ZnAirRpt(ZoneNumB).MixHeatGain,
                                            "System",
                                            "Sum",
                                            Zone(ZoneNumB).Name);
                        SetupOutputVariable(state, "Zone Mixing Latent Heat Loss Energy",
                                            OutputProcessor::Unit::J,
                                            ZnAirRpt(ZoneNumB).MixLatentLoss,
                                            "System",
                                            "Sum",
                                            Zone(ZoneNumB).Name);
                        SetupOutputVariable(state, "Zone Mixing Latent Heat Gain Energy",
                                            OutputProcessor::Unit::J,
                                            ZnAirRpt(ZoneNumB).MixLatentGain,
                                            "System",
                                            "Sum",
                                            Zone(ZoneNumB).Name);
                        SetupOutputVariable(state, "Zone Mixing Total Heat Loss Energy",
                                            OutputProcessor::Unit::J,
                                            ZnAirRpt(ZoneNumB).MixTotalLoss,
                                            "System",
                                            "Sum",
                                            Zone(ZoneNumB).Name);
                        SetupOutputVariable(state, "Zone Mixing Total Heat Gain Energy",
                                            OutputProcessor::Unit::J,
                                            ZnAirRpt(ZoneNumB).MixTotalGain,
                                            "System",
                                            "Sum",
                                            Zone(ZoneNumB).Name);
                    }
                }
                if (AnyEnergyManagementSystemInModel) {
                    SetupEMSActuator("ZoneRefDoorMixing",
                                     RefDoorMixing(ZoneNumB).Name,
                                     "Air Exchange Flow Rate",
                                     "[m3/s]",
                                     RefDoorMixing(ZoneNumA).EMSRefDoorMixingOn(ConnectionNumber),
                                     RefDoorMixing(ZoneNumA).EMSRefDoorFlowRate(ConnectionNumber));
                }

            } // DO Loop=1,TotRefDoorMixing
        }     // TotRefDoorMixing > 0)

        RepVarSet.deallocate();
        cAlphaArgs.deallocate();
        cAlphaFieldNames.deallocate();
        cNumericFieldNames.deallocate();
        rNumericArgs.deallocate();
        lAlphaFieldBlanks.deallocate();
        lNumericFieldBlanks.deallocate();

        TotInfilVentFlow.dimension(NumOfZones, 0.0);


        auto divide_and_print_if_greater_than_zero = [&](const Real64 denominator, const Real64 numerator){
            if (denominator > 0.0) {
                print(state.files.eio, "{:.3R},", numerator / denominator);
            } else {
                print(state.files.eio, "N/A,");
            }
        };

        for (Loop = 1; Loop <= TotInfiltration; ++Loop) {
            if (Loop == 1)
                print(state.files.eio, Format_721,
                    "ZoneInfiltration",
                     "Design Volume Flow Rate {m3/s},Volume Flow Rate/Floor Area {m3/s-m2},Volume Flow Rate/Exterior Surface Area {m3/s-m2},ACH - "
                       "Air Changes per Hour,Equation A - Constant Term Coefficient {},Equation B - Temperature Term Coefficient {1/C},Equation C - "
                       "Velocity Term Coefficient {s/m}, Equation D - Velocity Squared Term Coefficient {s2/m2}");

            ZoneNum = Infiltration(Loop).ZonePtr;
            if (ZoneNum == 0) {
                print(state.files.eio, Format_722, "Infiltration-Illegal Zone specified", Infiltration(Loop).Name);
                continue;
            }
            TotInfilVentFlow(ZoneNum) += Infiltration(Loop).DesignLevel;
            print(state.files.eio, Format_720, "ZoneInfiltration", Infiltration(Loop).Name, GetScheduleName(state, Infiltration(Loop).SchedPtr),
                Zone(ZoneNum).Name, Zone(ZoneNum).FloorArea, Zone(ZoneNum).TotOccupants);
            print(state.files.eio, "{:.3R},", Infiltration(Loop).DesignLevel);

            divide_and_print_if_greater_than_zero(Zone(ZoneNum).FloorArea, Infiltration(Loop).DesignLevel);
            divide_and_print_if_greater_than_zero(Zone(ZoneNum).ExteriorTotalSurfArea, Infiltration(Loop).DesignLevel);
            divide_and_print_if_greater_than_zero(Zone(ZoneNum).Volume, Infiltration(Loop).DesignLevel * SecInHour);

            print(state.files.eio, "{:.3R},", Infiltration(Loop).ConstantTermCoef);
            print(state.files.eio, "{:.3R},", Infiltration(Loop).TemperatureTermCoef);
            print(state.files.eio, "{:.3R},", Infiltration(Loop).VelocityTermCoef);
            print(state.files.eio, "{:.3R}\n", Infiltration(Loop).VelocitySQTermCoef);
        }

        if (ZoneAirMassFlow.EnforceZoneMassBalance) {
            for (Loop = 1; Loop <= TotInfiltration; ++Loop) {
                ZoneNum = Infiltration(Loop).ZonePtr;
                MassConservation(ZoneNum).InfiltrationPtr = Loop;
            }
        }

        for (Loop = 1; Loop <= TotVentilation; ++Loop) {
            if (Loop == 1) {
                print(state.files.eio, Format_721,
                    "ZoneVentilation",
                    "Design Volume Flow Rate {m3/s},Volume Flow Rate/Floor Area {m3/s-m2},Volume Flow Rate/person Area {m3/s-person},ACH - Air "
                       "Changes per Hour,Fan Type {Exhaust;Intake;Natural},Fan Pressure Rise {Pa},Fan Efficiency {},Equation A - Constant Term "
                       "Coefficient {},Equation B - Temperature Term Coefficient {1/C},Equation C - Velocity Term Coefficient {s/m}, Equation D - "
                       "Velocity Squared Term Coefficient {s2/m2},Minimum Indoor Temperature{C}/Schedule,Maximum Indoor "
                       "Temperature{C}/Schedule,Delta Temperature{C}/Schedule,Minimum Outdoor Temperature{C}/Schedule,Maximum Outdoor "
                       "Temperature{C}/Schedule,Maximum WindSpeed{m/s}");
            }

            ZoneNum = Ventilation(Loop).ZonePtr;
            if (ZoneNum == 0) {
                print(state.files.eio, Format_722, "Ventilation-Illegal Zone specified", Ventilation(Loop).Name);
                continue;
            }
            TotInfilVentFlow(ZoneNum) += Ventilation(Loop).DesignLevel;
            print(state.files.eio, Format_720,
                   "ZoneVentilation", Ventilation(Loop).Name, GetScheduleName(state, Ventilation(Loop).SchedPtr), Zone(ZoneNum).Name
                    , Zone(ZoneNum).FloorArea, Zone(ZoneNum).TotOccupants);

            print(state.files.eio, "{:.3R},", Ventilation(Loop).DesignLevel);

            divide_and_print_if_greater_than_zero(Zone(ZoneNum).FloorArea, Ventilation(Loop).DesignLevel);
            divide_and_print_if_greater_than_zero(Zone(ZoneNum).TotOccupants, Ventilation(Loop).DesignLevel);
            divide_and_print_if_greater_than_zero(Zone(ZoneNum).Volume, Ventilation(Loop).DesignLevel * SecInHour);

            if (Ventilation(Loop).FanType == ExhaustVentilation) {
                print(state.files.eio, "Exhaust,");
            } else if (Ventilation(Loop).FanType == IntakeVentilation) {
                print(state.files.eio, "Intake,");
            } else if (Ventilation(Loop).FanType == NaturalVentilation) {
                print(state.files.eio, "Natural,");
            } else if (Ventilation(Loop).FanType == BalancedVentilation) {
                print(state.files.eio, "Balanced,");
            } else {
                print(state.files.eio, "UNKNOWN,");
            }
            print(state.files.eio, "{:.3R},", Ventilation(Loop).FanPressure);
            print(state.files.eio, "{:.1R},", Ventilation(Loop).FanEfficiency);
            print(state.files.eio, "{:.3R},", Ventilation(Loop).ConstantTermCoef);
            print(state.files.eio, "{:.3R},", Ventilation(Loop).TemperatureTermCoef);
            print(state.files.eio, "{:.3R},", Ventilation(Loop).VelocityTermCoef);
            print(state.files.eio, "{:.3R},", Ventilation(Loop).VelocitySQTermCoef);

            // TODO Should this also be prefixed with "Schedule: " like the following ones are?
            if (Ventilation(Loop).MinIndoorTempSchedPtr > 0) {
                print(state.files.eio, "{},", GetScheduleName(state, Ventilation(Loop).MinIndoorTempSchedPtr));
            } else {
                print(state.files.eio, "{:.2R},", Ventilation(Loop).MinIndoorTemperature);
            }

            const auto print_temperature = [&](const int ptr, const Real64 value) {
                if (ptr > 0) {
                    print(state.files.eio, "Schedule: {},", GetScheduleName(state, ptr));
                } else {
                    print(state.files.eio, "{:.2R},", value);
                }
            };

            print_temperature(Ventilation(Loop).MaxIndoorTempSchedPtr, Ventilation(Loop).MaxIndoorTemperature);
            print_temperature(Ventilation(Loop).DeltaTempSchedPtr, Ventilation(Loop).DelTemperature);
            print_temperature(Ventilation(Loop).MinOutdoorTempSchedPtr, Ventilation(Loop).MinOutdoorTemperature);
            print_temperature(Ventilation(Loop).MaxOutdoorTempSchedPtr, Ventilation(Loop).MaxOutdoorTemperature);

            print(state.files.eio, "{:.2R}\n", Ventilation(Loop).MaxWindSpeed);
        }

        TotMixingFlow.dimension(NumOfZones, 0.0);
        for (Loop = 1; Loop <= TotMixing; ++Loop) {
            if (Loop == 1)
                print(state.files.eio, Format_721, "Mixing",
                    "Design Volume Flow Rate {m3/s},Volume Flow Rate/Floor Area {m3/s-m2},Volume Flow Rate/person Area {m3/s-person},ACH - Air "
                       "Changes per Hour,From/Source Zone,Delta Temperature {C}");

            ZoneNum = Mixing(Loop).ZonePtr;
            if (ZoneNum == 0) {
                print(state.files.eio, Format_722, "Mixing-Illegal Zone specified", Mixing(Loop).Name);
                continue;
            }
            TotMixingFlow(ZoneNum) += Mixing(Loop).DesignLevel;
            print(state.files.eio,
                  Format_720,
                  "Mixing",
                  Mixing(Loop).Name,
                  GetScheduleName(state, Mixing(Loop).SchedPtr),
                  Zone(ZoneNum).Name,
                  Zone(ZoneNum).FloorArea,
                  Zone(ZoneNum).TotOccupants);
            print(state.files.eio, "{:.3R},", Mixing(Loop).DesignLevel);
            divide_and_print_if_greater_than_zero(Zone(ZoneNum).FloorArea, Mixing(Loop).DesignLevel);
            divide_and_print_if_greater_than_zero(Zone(ZoneNum).TotOccupants, Mixing(Loop).DesignLevel);
            divide_and_print_if_greater_than_zero(Zone(ZoneNum).Volume, Mixing(Loop).DesignLevel * SecInHour);

            print(state.files.eio, "{},", Zone(Mixing(Loop).FromZone).Name);
            print(state.files.eio, "{:.2R}\n", Mixing(Loop).DeltaTemperature);
        }

        for (Loop = 1; Loop <= TotCrossMixing; ++Loop) {
            if (Loop == 1) {
                print(state.files.eio,
                      Format_721,
                      "CrossMixing",
                      "Design Volume Flow Rate {m3/s},Volume Flow Rate/Floor Area {m3/s-m2},Volume Flow Rate/person Area {m3/s-person},ACH - Air "
                      "Changes per Hour,From/Source Zone,Delta Temperature {C}");
            }

            ZoneNum = CrossMixing(Loop).ZonePtr;
            if (ZoneNum == 0) {
                print(state.files.eio, Format_722, "CrossMixing-Illegal Zone specified", CrossMixing(Loop).Name);
                continue;
            }
            TotMixingFlow(ZoneNum) += CrossMixing(Loop).DesignLevel;
            print(state.files.eio,
                  Format_720,
                  "CrossMixing",
                  CrossMixing(Loop).Name,
                  GetScheduleName(state, CrossMixing(Loop).SchedPtr),
                  Zone(ZoneNum).Name,
                  Zone(ZoneNum).FloorArea,
                  Zone(ZoneNum).TotOccupants);

            print(state.files.eio,"{:.3R},",CrossMixing(Loop).DesignLevel);

            divide_and_print_if_greater_than_zero(Zone(ZoneNum).FloorArea, CrossMixing(Loop).DesignLevel);
            divide_and_print_if_greater_than_zero(Zone(ZoneNum).TotOccupants, CrossMixing(Loop).DesignLevel);
            divide_and_print_if_greater_than_zero(Zone(ZoneNum).Volume, CrossMixing(Loop).DesignLevel * SecInHour);

            print(state.files.eio, "{},", Zone(CrossMixing(Loop).FromZone).Name);
            print(state.files.eio, "{:.2R}\n", CrossMixing(Loop).DeltaTemperature);
        }

        if (TotRefDoorMixing > 0) {
            static constexpr auto Format_724("! <{} Airflow Stats Nominal>, {}\n");
            print(state.files.eio, Format_724,
                "RefrigerationDoorMixing ",
                "Name, Zone 1 Name,Zone 2 Name,Door Opening Schedule Name,Door Height {m},Door Area {m2},Door Protection Type");
            for (ZoneNumA = 1; ZoneNumA <= (NumOfZones - 1); ++ZoneNumA) {
                if (!RefDoorMixing(ZoneNumA).RefDoorMixFlag) continue;
                for (ConnectionNumber = 1; ConnectionNumber <= RefDoorMixing(ZoneNumA).NumRefDoorConnections; ++ConnectionNumber) {
                    ZoneNumB = RefDoorMixing(ZoneNumA).MateZonePtr(ConnectionNumber);
                    // TotMixingFlow(ZoneNum)=TotMixingFlow(ZoneNum)+RefDoorMixing(Loop)%!DesignLevel
                    static constexpr auto Format_723(" {} Airflow Stats Nominal, {},{},{},{},{:.3R},{:.3R},{}\n");
                    print(state.files.eio,
                          Format_723,
                          "RefrigerationDoorMixing",
                          RefDoorMixing(ZoneNumA).DoorMixingObjectName(ConnectionNumber),
                          Zone(ZoneNumA).Name,
                          Zone(ZoneNumB).Name,
                          GetScheduleName(state, RefDoorMixing(ZoneNumA).OpenSchedPtr(ConnectionNumber)),
                          RefDoorMixing(ZoneNumA).DoorHeight(ConnectionNumber),
                          RefDoorMixing(ZoneNumA).DoorArea(ConnectionNumber),
                          RefDoorMixing(ZoneNumA).DoorProtTypeName(ConnectionNumber));
                } // ConnectionNumber
            }     // ZoneNumA
        }         //(TotRefDoorMixing .GT. 0)

        for (ZoneNum = 1; ZoneNum <= NumOfZones; ++ZoneNum) {
            Zone(ZoneNum).NominalInfilVent = TotInfilVentFlow(ZoneNum);
            Zone(ZoneNum).NominalMixing = TotMixingFlow(ZoneNum);
        }

        if (ZoneAirMassFlow.EnforceZoneMassBalance) {
            // Check for infiltration in zone which are only a mixing source zone
            for (ZoneNum = 1; ZoneNum <= NumOfZones; ++ZoneNum) {
                if ((ZoneAirMassFlow.BalanceMixing && MassConservation(ZoneNum).IsOnlySourceZone) &&
                    (ZoneAirMassFlow.InfiltrationTreatment != NoInfiltrationFlow)) {
                    if (MassConservation(ZoneNum).InfiltrationPtr == 0) {
                        ShowSevereError(RoutineName + ": Infiltration object is not defined for zone = " + Zone(ZoneNum).Name);
                        ShowContinueError("Zone air mass flow balance requires infiltration object for source zones of mixing objects");
                    }
                }
            }
            // Set up zone air mass balance output variables
            for (ZoneNum = 1; ZoneNum <= NumOfZones; ++ZoneNum) {
                SetupOutputVariable(state, "Zone Air Mass Balance Supply Mass Flow Rate",
                                    OutputProcessor::Unit::kg_s,
                                    MassConservation(ZoneNum).InMassFlowRate,
                                    "System",
                                    "Average",
                                    Zone(ZoneNum).Name);
                SetupOutputVariable(state, "Zone Air Mass Balance Exhaust Mass Flow Rate",
                                    OutputProcessor::Unit::kg_s,
                                    MassConservation(ZoneNum).ExhMassFlowRate,
                                    "System",
                                    "Average",
                                    Zone(ZoneNum).Name);
                SetupOutputVariable(state, "Zone Air Mass Balance Return Mass Flow Rate",
                                    OutputProcessor::Unit::kg_s,
                                    MassConservation(ZoneNum).RetMassFlowRate,
                                    "System",
                                    "Average",
                                    Zone(ZoneNum).Name);
                if (ZoneAirMassFlow.BalanceMixing &&
                    ((MassConservation(ZoneNum).NumSourceZonesMixingObject + MassConservation(ZoneNum).NumReceivingZonesMixingObject) > 0)) {
                    SetupOutputVariable(state, "Zone Air Mass Balance Mixing Receiving Mass Flow Rate",
                                        OutputProcessor::Unit::kg_s,
                                        MassConservation(ZoneNum).MixingMassFlowRate,
                                        "System",
                                        "Average",
                                        Zone(ZoneNum).Name);
                    SetupOutputVariable(state, "Zone Air Mass Balance Mixing Source Mass Flow Rate",
                                        OutputProcessor::Unit::kg_s,
                                        MassConservation(ZoneNum).MixingSourceMassFlowRate,
                                        "System",
                                        "Average",
                                        Zone(ZoneNum).Name);
                }
                if (ZoneAirMassFlow.InfiltrationTreatment != NoInfiltrationFlow) {
                    if (ZoneAirMassFlow.InfiltrationZoneType == AllZones || (MassConservation(ZoneNum).NumSourceZonesMixingObject > 0)) {
                        if (MassConservation(ZoneNum).InfiltrationPtr > 0) {
                            SetupOutputVariable(state, "Zone Air Mass Balance Infiltration Mass Flow Rate",
                                                OutputProcessor::Unit::kg_s,
                                                MassConservation(ZoneNum).InfiltrationMassFlowRate,
                                                "System",
                                                "Average",
                                                Zone(ZoneNum).Name);
                            SetupOutputVariable(state, "Zone Air Mass Balance Infiltration Status",
                                                OutputProcessor::Unit::None,
                                                MassConservation(ZoneNum).IncludeInfilToZoneMassBal,
                                                "System",
                                                "Average",
                                                Zone(ZoneNum).Name);
                        }
                    }
                }
            }
        }

        TotInfilVentFlow.deallocate();
        TotMixingFlow.deallocate();
        //           ' Area per Occupant {m2/person}, Occupant per Area {person/m2}, Interior Lighting {W/m2}, ',  &
        //           'Electric Load {W/m2}, Gas Load {W/m2}, Other Load {W/m2}, Hot Water Eq {W/m2}, Outdoor Controlled Baseboard Heat')
    }

    //*****************************************************************************************
    // This subroutine was moved from 'RoomAirManager' Module

    void GetRoomAirModelParameters(EnergyPlusData &state, bool &errFlag) // True if errors found during this input routine
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Brent Griffith
        //       DATE WRITTEN   August 2001
        //       MODIFIED       na
        //       RE-ENGINEERED  April 2003, Weixiu Kong
        //                      December 2003, CC

        // PURPOSE OF THIS SUBROUTINE:
        //     Get room air model parameters for all zones at once

        // METHODOLOGY EMPLOYED:
        //     Use input processer to get input from idf file

        // Using/Aliasing
        using namespace DataIPShortCuts;
        using DataGlobals::NumOfZones;
        using DataHeatBalance::Zone;
        using DataRoomAirModel::AirModel;
        using DataRoomAirModel::ChAirModel;
        using DataRoomAirModel::DirectCoupling;
        using DataRoomAirModel::IndirectCoupling;
        using DataRoomAirModel::MundtModelUsed;
        using DataRoomAirModel::RoomAirModel_AirflowNetwork;
        using DataRoomAirModel::RoomAirModel_Mixing;
        using DataRoomAirModel::RoomAirModel_Mundt;
        using DataRoomAirModel::RoomAirModel_UCSDCV;
        using DataRoomAirModel::RoomAirModel_UCSDDV;
        using DataRoomAirModel::RoomAirModel_UCSDUFE;
        using DataRoomAirModel::RoomAirModel_UCSDUFI;
        using DataRoomAirModel::RoomAirModel_UserDefined;
        using DataRoomAirModel::UCSDModelUsed;
        using DataRoomAirModel::UserDefinedUsed;


        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int NumAlphas; // States which alpha value to read from a
        // "Number" line
        int NumNumbers; // Number of numbers encountered
        int Status;     // Notes if there was an error in processing the input
        int AirModelNum;
        int NumOfAirModels;
        int ZoneNum;
        bool ErrorsFound;
        bool IsNotOK;

        // FLOW:

        // Initialize default values for air model parameters
        AirModel.allocate(NumOfZones);

        ErrorsFound = false;

        cCurrentModuleObject = "RoomAirModelType";
        NumOfAirModels = inputProcessor->getNumObjectsFound(cCurrentModuleObject);
        if (NumOfAirModels > NumOfZones) {
            ShowSevereError("Too many " + cCurrentModuleObject + ".  Cannot exceed the number of Zones.");
            ErrorsFound = true;
        }

        for (AirModelNum = 1; AirModelNum <= NumOfAirModels; ++AirModelNum) {
            inputProcessor->getObjectItem(state,
                                          cCurrentModuleObject,
                                          AirModelNum,
                                          cAlphaArgs,
                                          NumAlphas,
                                          rNumericArgs,
                                          NumNumbers,
                                          Status,
                                          _,
                                          _,
                                          cAlphaFieldNames,
                                          cNumericFieldNames);
            ZoneNum = UtilityRoutines::FindItemInList(cAlphaArgs(2), Zone);
            if (ZoneNum != 0) {
                if (!AirModel(ZoneNum).AirModelName.empty()) {
                    ShowSevereError("Invalid " + cAlphaFieldNames(2) + " = " + cAlphaArgs(2));
                    ShowContinueError("Entered in " + cCurrentModuleObject + " = " + cAlphaArgs(1));
                    ShowContinueError("Duplicate zone name, only one type of roomair model is allowed per zone");
                    ShowContinueError("Zone " + cAlphaArgs(2) + " was already assigned a roomair model by " + cCurrentModuleObject + " = " +
                                      AirModel(ZoneNum).AirModelName);
                    ShowContinueError("Air Model Type for zone already set to " + ChAirModel(AirModel(ZoneNum).AirModelType));
                    ShowContinueError("Trying to overwrite with model type = " + cAlphaArgs(3));
                    ErrorsFound = true;
                }
                AirModel(ZoneNum).AirModelName = cAlphaArgs(1);
                AirModel(ZoneNum).ZoneName = cAlphaArgs(2);

                {
                    auto const SELECT_CASE_var(cAlphaArgs(3));
                    if (SELECT_CASE_var == "MIXING") {
                        AirModel(ZoneNum).AirModelType = RoomAirModel_Mixing;
                    } else if (SELECT_CASE_var == "ONENODEDISPLACEMENTVENTILATION") {
                        AirModel(ZoneNum).AirModelType = RoomAirModel_Mundt;
                        AirModel(ZoneNum).SimAirModel = true;
                        MundtModelUsed = true;
                        IsNotOK = false;
                        ValidateComponent(
                            state, "RoomAirSettings:OneNodeDisplacementVentilation", "zone_name", cAlphaArgs(2), IsNotOK, "GetRoomAirModelParameters");
                        if (IsNotOK) {
                            ShowContinueError("In " + cCurrentModuleObject + '=' + cAlphaArgs(1) + '.');
                            ErrorsFound = true;
                        }
                    } else if (SELECT_CASE_var == "THREENODEDISPLACEMENTVENTILATION") {
                        AirModel(ZoneNum).AirModelType = RoomAirModel_UCSDDV;
                        AirModel(ZoneNum).SimAirModel = true;
                        UCSDModelUsed = true;
                        IsNotOK = false;
                        ValidateComponent(
                            state, "RoomAirSettings:ThreeNodeDisplacementVentilation", "zone_name", cAlphaArgs(2), IsNotOK, "GetRoomAirModelParameters");
                        if (IsNotOK) {
                            ShowContinueError("In " + cCurrentModuleObject + '=' + cAlphaArgs(1) + '.');
                            ErrorsFound = true;
                        }
                    } else if (SELECT_CASE_var == "CROSSVENTILATION") {
                        AirModel(ZoneNum).AirModelType = RoomAirModel_UCSDCV;
                        AirModel(ZoneNum).SimAirModel = true;
                        UCSDModelUsed = true;
                        IsNotOK = false;
                        ValidateComponent(state, "RoomAirSettings:CrossVentilation", "zone_name", cAlphaArgs(2), IsNotOK, "GetRoomAirModelParameters");
                        if (IsNotOK) {
                            ShowContinueError("In " + cCurrentModuleObject + '=' + cAlphaArgs(1) + '.');
                            ErrorsFound = true;
                        }
                    } else if (SELECT_CASE_var == "UNDERFLOORAIRDISTRIBUTIONINTERIOR") {
                        AirModel(ZoneNum).AirModelType = RoomAirModel_UCSDUFI;
                        AirModel(ZoneNum).SimAirModel = true;
                        UCSDModelUsed = true;
                        ValidateComponent(state,
                            "RoomAirSettings:UnderFloorAirDistributionInterior", "zone_name", cAlphaArgs(2), IsNotOK, "GetRoomAirModelParameters");
                        if (IsNotOK) {
                            ShowContinueError("In " + cCurrentModuleObject + '=' + cAlphaArgs(1) + '.');
                            ErrorsFound = true;
                        }
                    } else if (SELECT_CASE_var == "UNDERFLOORAIRDISTRIBUTIONEXTERIOR") {
                        AirModel(ZoneNum).AirModelType = RoomAirModel_UCSDUFE;
                        AirModel(ZoneNum).SimAirModel = true;
                        UCSDModelUsed = true;
                        ValidateComponent(state,
                            "RoomAirSettings:UnderFloorAirDistributionExterior", "zone_name", cAlphaArgs(2), IsNotOK, "GetRoomAirModelParameters");
                        if (IsNotOK) {
                            ShowContinueError("In " + cCurrentModuleObject + '=' + cAlphaArgs(1) + '.');
                            ErrorsFound = true;
                        }
                    } else if (SELECT_CASE_var == "USERDEFINED") {
                        AirModel(ZoneNum).AirModelType = RoomAirModel_UserDefined;
                        AirModel(ZoneNum).SimAirModel = true;
                        UserDefinedUsed = true;
                    } else if (SELECT_CASE_var == "AIRFLOWNETWORK") {
                        AirModel(ZoneNum).AirModelType = RoomAirModel_AirflowNetwork;
                        AirModel(ZoneNum).SimAirModel = true;
                        if (inputProcessor->getNumObjectsFound("AirflowNetwork:SimulationControl") == 0) {
                            ShowSevereError("In " + cCurrentModuleObject + " = " + cAlphaArgs(1) + ": " + cAlphaFieldNames(3) + " = AIRFLOWNETWORK.");
                            ShowContinueError("This model requires AirflowNetwork:* objects to form a complete network, including "
                                              "AirflowNetwork:Intrazone:Node and AirflowNetwork:Intrazone:Linkage.");
                            ShowContinueError("AirflowNetwork:SimulationControl not found.");
                            ErrorsFound = true;
                        }
                    } else {
                        ShowWarningError("Invalid " + cAlphaFieldNames(3) + " = " + cAlphaArgs(3));
                        ShowContinueError("Entered in " + cCurrentModuleObject + " = " + cAlphaArgs(1));
                        ShowContinueError("The mixing air model will be used for Zone =" + cAlphaArgs(2));
                        AirModel(ZoneNum).AirModelType = RoomAirModel_Mixing;
                    }
                }

                {
                    auto const SELECT_CASE_var(cAlphaArgs(4));
                    if (SELECT_CASE_var == "DIRECT") {
                        AirModel(ZoneNum).TempCoupleScheme = DirectCoupling;
                    } else if (SELECT_CASE_var == "INDIRECT") {
                        AirModel(ZoneNum).TempCoupleScheme = IndirectCoupling;
                    } else {
                        ShowWarningError("Invalid " + cAlphaFieldNames(4) + " = " + cAlphaArgs(4));
                        ShowContinueError("Entered in " + cCurrentModuleObject + " = " + cAlphaArgs(1));
                        ShowContinueError("The direct coupling scheme will be used for Zone =" + cAlphaArgs(2));
                        AirModel(ZoneNum).TempCoupleScheme = DirectCoupling;
                    }
                }
            } else { // Zone Not Found
                ShowSevereError(cCurrentModuleObject + ", Zone not found=" + cAlphaArgs(2));
                ShowContinueError("occurs in " + cCurrentModuleObject + '=' + cAlphaArgs(1));
                ErrorsFound = true;
            }
        } // AirModel_Param_Loop

        for (ZoneNum = 1; ZoneNum <= NumOfZones; ++ZoneNum) {
            if (NumOfAirModels == 0) {
                AirModel(ZoneNum).AirModelName = "MIXING AIR MODEL FOR " + Zone(ZoneNum).Name;
                AirModel(ZoneNum).ZoneName = Zone(ZoneNum).Name;
            } else if (AirModel(ZoneNum).ZoneName == BlankString) {
                // no 'select air model' object for this zone so the mixing model is used for this zone
                AirModel(ZoneNum).AirModelName = "MIXING AIR MODEL FOR " + Zone(ZoneNum).Name;
                AirModel(ZoneNum).ZoneName = Zone(ZoneNum).Name;
            }
        }

        // Write RoomAir Model details onto EIO file
        static constexpr auto RoomAirHeader("! <RoomAir Model>, Zone Name, Mixing/Mundt/UCSDDV/UCSDCV/UCSDUFI/UCSDUFE/User Defined\n");
        print(state.files.eio, RoomAirHeader);
        for (ZoneNum = 1; ZoneNum <= NumOfZones; ++ZoneNum) {
            {
                static constexpr auto RoomAirZoneFmt("RoomAir Model,{},{}\n");

                auto const SELECT_CASE_var(AirModel(ZoneNum).AirModelType);
                if (SELECT_CASE_var == RoomAirModel_Mixing) {
                    print(state.files.eio, RoomAirZoneFmt, Zone(ZoneNum).Name, "Mixing/Well-Stirred");
                } else if (SELECT_CASE_var == RoomAirModel_Mundt) {
                    print(state.files.eio, RoomAirZoneFmt, Zone(ZoneNum).Name, "OneNodeDisplacementVentilation");
                } else if (SELECT_CASE_var == RoomAirModel_UCSDDV) {
                    print(state.files.eio, RoomAirZoneFmt, Zone(ZoneNum).Name, "ThreeNodeDisplacementVentilation");
                } else if (SELECT_CASE_var == RoomAirModel_UCSDCV) {
                    print(state.files.eio, RoomAirZoneFmt, Zone(ZoneNum).Name, "CrossVentilation");
                } else if (SELECT_CASE_var == RoomAirModel_UCSDUFI) {
                    print(state.files.eio, RoomAirZoneFmt, Zone(ZoneNum).Name, "UnderFloorAirDistributionInterior");
                } else if (SELECT_CASE_var == RoomAirModel_UCSDUFE) {
                    print(state.files.eio, RoomAirZoneFmt, Zone(ZoneNum).Name, "UnderFloorAirDistributionExterior");
                } else if (SELECT_CASE_var == RoomAirModel_UserDefined) {
                    print(state.files.eio, RoomAirZoneFmt, Zone(ZoneNum).Name, "UserDefined");
                } else if (SELECT_CASE_var == RoomAirModel_AirflowNetwork) {
                    print(state.files.eio, RoomAirZoneFmt, Zone(ZoneNum).Name, "AirflowNetwork");
                }
            }
        }

        if (ErrorsFound) {
            ShowSevereError("Errors found in processing input for " + cCurrentModuleObject);
            errFlag = true;
        }
    }

    // END of Get Input subroutines for the HBAir Module
    //******************************************************************************

    // Beginning Initialization Section of the Module
    //******************************************************************************

    void InitAirHeatBalance()
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Richard J. Liesen
        //       DATE WRITTEN   February 1998

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine is for  initializations within the
        // air heat balance.

        // METHODOLOGY EMPLOYED:
        // Uses the status flags to trigger events.

        // Do the Begin Day initializations
        if (BeginDayFlag) {
        }

        // Do the following initializations (every time step):
        InitSimpleMixingConvectiveHeatGains();
    }

    void InitSimpleMixingConvectiveHeatGains()
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Richard Liesen
        //       DATE WRITTEN   February 1998
        //       MODIFIED       March 2003, FCW: allow individual window/door venting control
        //       DATE MODIFIED  April 2000
        //                      May 2009, Brent Griffith added EMS override to mixing and cross mixing flows
        //                      renamed routine and did some cleanup
        //                      August 2011, Therese Stovall added refrigeration door mixing flows
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine sets up the mixing and cross mixing flows

        // METHODOLOGY EMPLOYED:
        // na

        // REFERENCES:
        // na

        // Using/Aliasing
        using General::RoundSigDigits;
        using ScheduleManager::GetCurrentScheduleValue;
        using ScheduleManager::GetScheduleIndex;

        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:
        // na

        // SUBROUTINE PARAMETER DEFINITIONS:

        // INTERFACE BLOCK SPECIFICATIONS:
        // na

        // DERIVED TYPE DEFINITIONS:
        // na

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int Loop; // local loop index
        int NZ;   // local index for zone number
        int J;    // local index for second zone in refrig door pair

        int ZoneNum;              // zone counter
        Real64 ZoneMixingFlowSum; // sum of zone mixing flows for a zone
        int NumOfMixingObjects;   // number of mixing objects for a receiving zone

        // Select type of airflow calculation

        {
            auto const SELECT_CASE_var(AirFlowFlag);

            if (SELECT_CASE_var == UseSimpleAirFlow) { // Simplified airflow calculation
                // Process the scheduled Mixing for air heat balance
                for (Loop = 1; Loop <= TotMixing; ++Loop) {
                    NZ = Mixing(Loop).ZonePtr;
                    Mixing(Loop).DesiredAirFlowRate = Mixing(Loop).DesignLevel * GetCurrentScheduleValue(Mixing(Loop).SchedPtr);
                    if (Mixing(Loop).EMSSimpleMixingOn) Mixing(Loop).DesiredAirFlowRate = Mixing(Loop).EMSimpleMixingFlowRate;
                    Mixing(Loop).DesiredAirFlowRateSaved = Mixing(Loop).DesiredAirFlowRate;
                }

                // if zone air mass flow balance enforced calculate the fraction of
                // contribution of each mixing object to a zone mixed flow rate, BAN Feb 2014
                if (ZoneAirMassFlow.EnforceZoneMassBalance) {
                    for (ZoneNum = 1; ZoneNum <= NumOfZones; ++ZoneNum) {
                        ZoneMixingFlowSum = 0.0;
                        NumOfMixingObjects = MassConservation(ZoneNum).NumReceivingZonesMixingObject;
                        for (Loop = 1; Loop <= NumOfMixingObjects; ++Loop) {
                            ZoneMixingFlowSum = ZoneMixingFlowSum + Mixing(Loop).DesignLevel;
                        }
                        if (ZoneMixingFlowSum > 0.0) {
                            for (Loop = 1; Loop <= NumOfMixingObjects; ++Loop) {
                                MassConservation(ZoneNum).ZoneMixingReceivingFr(Loop) = Mixing(Loop).DesignLevel / ZoneMixingFlowSum;
                            }
                        }
                    }
                }

                // Process the scheduled CrossMixing for air heat balance
                for (Loop = 1; Loop <= TotCrossMixing; ++Loop) {
                    NZ = CrossMixing(Loop).ZonePtr;
                    CrossMixing(Loop).DesiredAirFlowRate = CrossMixing(Loop).DesignLevel * GetCurrentScheduleValue(CrossMixing(Loop).SchedPtr);
                    if (CrossMixing(Loop).EMSSimpleMixingOn) CrossMixing(Loop).DesiredAirFlowRate = CrossMixing(Loop).EMSimpleMixingFlowRate;
                }

                // Note - do each Pair a Single time, so must do increment reports for both zones
                //       Can't have a pair that has ZoneA zone number = NumOfZones because organized
                //       in input with lowest zone # first no matter how input in idf

                // Process the scheduled Refrigeration Door mixing for air heat balance
                if (TotRefDoorMixing > 0) {
                    for (NZ = 1; NZ <= (NumOfZones - 1);
                         ++NZ) { // Can't have %ZonePtr==NumOfZones because lesser zone # of pair placed in ZonePtr in input
                        if (!RefDoorMixing(NZ).RefDoorMixFlag) continue;
                        if (RefDoorMixing(NZ).ZonePtr == NZ) {
                            for (J = 1; J <= RefDoorMixing(NZ).NumRefDoorConnections; ++J) {
                                RefDoorMixing(NZ).VolRefDoorFlowRate(J) = 0.0;
                                if (RefDoorMixing(NZ).EMSRefDoorMixingOn(J))
                                    RefDoorMixing(NZ).VolRefDoorFlowRate(J) = RefDoorMixing(NZ).EMSRefDoorFlowRate(J);
                            }
                        }
                    }
                } // TotRefDoorMixing

                // Infiltration and ventilation calculations have been moved to a subroutine of CalcAirFlowSimple in HVAC Manager

            } else {
            }
        }
    }

    // END Initialization Section of the Module
    //******************************************************************************

    // Begin Algorithm Section of the Module
    //******************************************************************************

    void CalcHeatBalanceAir(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Legacy Code
        //       DATE WRITTEN   na
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine calculates the air component of the heat balance.

        // METHODOLOGY EMPLOYED:
        // na

        // REFERENCES:
        // na

        // Using/Aliasing
        using HVACManager::ManageHVAC;

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

        if(DataGlobals::externalHVACManager) {
          if (!DataGlobals::externalHVACManagerInitialized) {
              initializeForExternalHVACManager(state);
          }
          DataGlobals::externalHVACManager(&state);
        } else {
          ManageHVAC(state);
        }

        // Do Final Temperature Calculations for Heat Balance before next Time step
        SumHmAW = 0.0;
        SumHmARa = 0.0;
        SumHmARaW = 0.0;
    }

    // END Algorithm Section of the Module

    void initializeForExternalHVACManager(EnergyPlusData &state) {
        // this function will ultimately provide a nice series of calls that initialize all the hvac stuff needed
        // to allow an external hvac manager to play nice with E+
        EnergyPlus::ZoneTempPredictorCorrector::InitZoneAirSetPoints(state);
        if (!EnergyPlus::DataZoneEquipment::ZoneEquipInputsFilled) {
            EnergyPlus::DataZoneEquipment::GetZoneEquipmentData(state);
            EnergyPlus::DataZoneEquipment::ZoneEquipInputsFilled = true;
        }
    }

    void ReportZoneMeanAirTemp()
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda Lawrie
        //       DATE WRITTEN   July 2000
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine updates the report variables for the AirHeatBalance.

        // METHODOLOGY EMPLOYED:
        // na

        // REFERENCES:
        // na

        // Using/Aliasing
        using DataEnvironment::OutBaroPress;
        using DataHeatBalance::MRT;
        using DataZoneControls::AnyOpTempControl;
        using DataZoneControls::TempControlledZone;
        using Psychrometrics::PsyTdpFnWPb;
        using ScheduleManager::GetCurrentScheduleValue;

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
        int ZoneLoop;             // Counter for the # of zones (nz)
        int TempControlledZoneID; // index for zone in TempConrolled Zone structure
        Real64 thisMRTFraction;   // temp working value for radiative fraction/weight

        for (ZoneLoop = 1; ZoneLoop <= NumOfZones; ++ZoneLoop) {
            // The mean air temperature is actually ZTAV which is the average
            // temperature of the air temperatures at the system time step for the
            // entire zone time step.
            ZnAirRpt(ZoneLoop).MeanAirTemp = ZTAV(ZoneLoop);
            ZnAirRpt(ZoneLoop).MeanAirHumRat = ZoneAirHumRatAvg(ZoneLoop);
            ZnAirRpt(ZoneLoop).OperativeTemp = 0.5 * (ZTAV(ZoneLoop) + MRT(ZoneLoop));
            ZnAirRpt(ZoneLoop).MeanAirDewPointTemp = PsyTdpFnWPb(ZnAirRpt(ZoneLoop).MeanAirHumRat, OutBaroPress);

            // if operative temperature control is being used, then radiative fraction/weighting
            //  might be defined by user to be something different than 0.5, even scheduled over simulation period
            if (AnyOpTempControl) { // dig further...
                // find TempControlledZoneID from ZoneLoop index
                TempControlledZoneID = Zone(ZoneLoop).TempControlledZoneIndex;
                if (Zone(ZoneLoop).IsControlled) {
                    if ((TempControlledZone(TempControlledZoneID).OperativeTempControl)) {
                        // is operative temp radiative fraction scheduled or fixed?
                        if (TempControlledZone(TempControlledZoneID).OpTempCntrlModeScheduled) {
                            thisMRTFraction = GetCurrentScheduleValue(TempControlledZone(TempControlledZoneID).OpTempRadiativeFractionSched);
                        } else {
                            thisMRTFraction = TempControlledZone(TempControlledZoneID).FixedRadiativeFraction;
                        }
                        ZnAirRpt(ZoneLoop).ThermOperativeTemp = (1.0 - thisMRTFraction) * ZTAV(ZoneLoop) + thisMRTFraction * MRT(ZoneLoop);
                    }
                }
            }
        }
    }

} // namespace HeatBalanceAirManager

} // namespace EnergyPlus
