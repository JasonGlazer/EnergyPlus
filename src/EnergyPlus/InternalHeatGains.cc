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
#include <map>
#include <string>

// ObjexxFCL Headers
#include <ObjexxFCL/Array.functions.hh>
#include <ObjexxFCL/Array1D.hh>
#include <ObjexxFCL/Fmath.hh>

// EnergyPlus Headers
#include <EnergyPlus/CurveManager.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataContaminantBalance.hh>
#include <EnergyPlus/DataDaylighting.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataHeatBalFanSys.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataIPShortCuts.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/DataPrecisionGlobals.hh>
#include <EnergyPlus/DataRoomAirModel.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/DataSurfaces.hh>
#include <EnergyPlus/DataViewFactorInformation.hh>
#include <EnergyPlus/DataZoneEquipment.hh>
#include <EnergyPlus/DaylightingDevices.hh>
#include <EnergyPlus/EMSManager.hh>
#include <EnergyPlus/ElectricPowerServiceManager.hh>
#include <EnergyPlus/ExteriorEnergyUse.hh>
#include <EnergyPlus/FuelCellElectricGenerator.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/HeatBalanceInternalHeatGains.hh>
#include <EnergyPlus/HybridModel.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/InternalHeatGains.hh>
#include <EnergyPlus/MicroCHPElectricGenerator.hh>
#include <EnergyPlus/NodeInputManager.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/OutputReportPredefined.hh>
#include <EnergyPlus/OutputReportTabular.hh>
#include <EnergyPlus/PipeHeatTransfer.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/RefrigeratedCase.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/SetPointManager.hh>
#include <EnergyPlus/UtilityRoutines.hh>
#include <EnergyPlus/WaterThermalTanks.hh>
#include <EnergyPlus/WaterUse.hh>
#include <EnergyPlus/ZonePlenum.hh>

namespace EnergyPlus {

namespace InternalHeatGains {
    // Module containing the routines dealing with the internal heat gains

    // MODULE INFORMATION:
    //       AUTHOR         Rick Strand
    //       DATE WRITTEN   August 2000
    //       MODIFIED       Aug 2005, PGE (Added object names and report variables)
    //                      Feb 2006, PGE (Added end-use subcategories)
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS MODULE:
    // Part of the heat balance modularization/re-engineering.  Purpose of this
    // module is to contain the internal heat gain routines in a single location.

    // METHODOLOGY EMPLOYED:
    // Routines are called as subroutines to supply the data-only module structures
    // with the proper values.

    // REFERENCES:
    // Legacy BLAST code

    // OTHER NOTES: none

    // Using/Aliasing
    using namespace DataPrecisionGlobals;
    using namespace DataGlobals;
    using namespace DataEnvironment;
    using namespace DataHeatBalance;
    using namespace DataSurfaces;

    // Data
    // MODULE PARAMETER DEFINITIONS:
    int const ITEClassNone(0);
    int const ITEClassA1(1);
    int const ITEClassA2(2);
    int const ITEClassA3(3);
    int const ITEClassA4(4);
    int const ITEClassB(5);
    int const ITEClassC(6);
    int const ITEInletAdjustedSupply(0);
    int const ITEInletZoneAirNode(1);
    int const ITEInletRoomAirModel(2);

    bool GetInternalHeatGainsInputFlag(true); // Controls the GET routine calling (limited to first time)
    bool ErrorsFound(false);                  // if errors were found in the input

    static std::string const BlankString;

    // Functions
    void clear_state()
    {
        GetInternalHeatGainsInputFlag = true;
        ErrorsFound = false;
    }

    void ManageInternalHeatGains(EnergyPlusData &state, Optional_bool_const InitOnly) // when true, just calls the get input, if appropriate and returns.
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Rick Strand
        //       DATE WRITTEN   May 2000
        //       MODIFIED       Mar 2004, FCW: move call to DayltgElecLightingControl from InitSurfaceHeatBalance
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This is the main driver subroutine for the internal heat gains.

        // FLOW:
        if (GetInternalHeatGainsInputFlag) {
            GetInternalHeatGainsInput(state);
            GetInternalHeatGainsInputFlag = false;
        }

        if (present(InitOnly)) {
            if (InitOnly) return;
        }

        InitInternalHeatGains(state);

        ReportInternalHeatGains();

        CheckReturnAirHeatGain();

        // for the load component report, gather the load components for each timestep but not when doing pulse
        if (ZoneSizingCalc) GatherComponentLoadsIntGain();
    }

    void GetInternalHeatGainsInput(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda K. Lawrie
        //       DATE WRITTEN   September 1997
        //       MODIFIED       September 1998, FW
        //                      May 2009, BG: added calls to setup for possible EMS override
        //       RE-ENGINEERED  August 2000, RKS

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine gets the Internal Heat Gain Data for the Zones.
        // Sets up the various data that will be used later with the
        // schedulemanager to determine the actual values.

        // METHODOLOGY EMPLOYED:
        // The GetObjectItem routines are employed to retrieve the data.

        // REFERENCES:
        // IDD Objects:
        // People
        // Lights
        // ElectricEquipment
        // GasEquipment
        // SteamEquipment
        // HotWaterEquipment
        // OtherEquipment
        // ElectricEquipment:ITE:AirCooled
        // ZoneBaseboard:OutdoorTemperatureControlled

        // Using/Aliasing
        using namespace DataIPShortCuts;
        using namespace ScheduleManager;
        using General::CheckCreatedZoneItemName;
        using General::RoundSigDigits;
        using namespace OutputReportPredefined;
        using namespace DataLoopNode;
        using CurveManager::GetCurveIndex;
        using NodeInputManager::GetOnlySingleNode;

        // SUBROUTINE PARAMETER DEFINITIONS:
        static ObjexxFCL::gio::Fmt fmtA("(A)");
        static std::string const RoutineName("GetInternalHeatGains: ");

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Array1D_string AlphaName;
        Array1D<Real64> IHGNumbers;
        int IOStat;
        int Loop;
        bool MustInpSch;
        int NumAlpha;
        int NumNumber;
        int MaxAlpha;
        int MaxNumber;
        int OptionNum(0); // Autodesk:Init Initialization added to elim poss use uninitialized
        int lastOption;
        Array1D_bool RepVarSet;
        //   Variables for reporting nominal internal gains
        Real64 LightTot;       // Total Lights for calculating lights per square meter
        Real64 ElecTot;        // Total Electric Load for calculating electric per square meter
        Real64 GasTot;         // Total Gas load for calculating gas per square meter
        Real64 OthTot;         // Total Other load for calculating other load per square meter
        Real64 HWETot;         // Total Hot Water Equipment for calculating HWE per square meter
        Real64 StmTot;         // Total Steam for calculating Steam per square meter
        std::string BBHeatInd; // Yes if BBHeat in zone, no if not.
        int Loop1;
        Real64 SchMin;
        Real64 SchMax;
        static bool UsingThermalComfort(false);
        std::string liteName;
        int zonePt;
        Real64 mult;
        static Real64 sumArea(0.0);
        static Real64 sumPower(0.0);
        int ZoneNum;
        Real64 maxOccupLoad;
        std::string CurrentModuleObject;
        bool errFlag;
        int Item;
        int ZLItem;
        int Item1;

        // Formats
        static constexpr auto Format_720(" Zone Internal Gains Nominal, {},{:.2R},{:.1R},");

        static constexpr auto Format_722(" {} Internal Gains Nominal, {},{},{},{:.2R},{:.1R},");
        static constexpr auto Format_723("! <{} Internal Gains Nominal>,Name,Schedule Name,Zone Name,Zone Floor Area {{m2}},# Zone Occupants,{}");
        static constexpr auto Format_724(" {}, {}\n");

        auto print_and_divide_if_greater_than_zero = [&](const Real64 numerator, const Real64 denominator) {
            if (denominator > 0.0) {
                print(state.files.eio, "{:.3R},", numerator / denominator);
            } else {
                print(state.files.eio, "N/A,");
            }
        };

        // FLOW:
        ZoneIntGain.allocate(NumOfZones);
        ZnRpt.allocate(NumOfZones);
        ZoneIntEEuse.allocate(NumOfZones);
        RefrigCaseCredit.allocate(NumOfZones);

        RepVarSet.dimension(NumOfZones, true);

        // Determine argument length of objects gotten by this routine
        MaxAlpha = -100;
        MaxNumber = -100;
        CurrentModuleObject = "People";
        inputProcessor->getObjectDefMaxArgs(CurrentModuleObject, Loop, NumAlpha, NumNumber);
        MaxAlpha = max(MaxAlpha, NumAlpha);
        MaxNumber = max(MaxNumber, NumNumber);
        CurrentModuleObject = "Lights";
        inputProcessor->getObjectDefMaxArgs(CurrentModuleObject, Loop, NumAlpha, NumNumber);
        MaxAlpha = max(MaxAlpha, NumAlpha);
        MaxNumber = max(MaxNumber, NumNumber);
        CurrentModuleObject = "ElectricEquipment";
        inputProcessor->getObjectDefMaxArgs(CurrentModuleObject, Loop, NumAlpha, NumNumber);
        MaxAlpha = max(MaxAlpha, NumAlpha);
        MaxNumber = max(MaxNumber, NumNumber);
        CurrentModuleObject = "GasEquipment";
        inputProcessor->getObjectDefMaxArgs(CurrentModuleObject, Loop, NumAlpha, NumNumber);
        MaxAlpha = max(MaxAlpha, NumAlpha);
        MaxNumber = max(MaxNumber, NumNumber);
        CurrentModuleObject = "HotWaterEquipment";
        inputProcessor->getObjectDefMaxArgs(CurrentModuleObject, Loop, NumAlpha, NumNumber);
        MaxAlpha = max(MaxAlpha, NumAlpha);
        MaxNumber = max(MaxNumber, NumNumber);
        CurrentModuleObject = "SteamEquipment";
        inputProcessor->getObjectDefMaxArgs(CurrentModuleObject, Loop, NumAlpha, NumNumber);
        MaxAlpha = max(MaxAlpha, NumAlpha);
        MaxNumber = max(MaxNumber, NumNumber);
        CurrentModuleObject = "OtherEquipment";
        inputProcessor->getObjectDefMaxArgs(CurrentModuleObject, Loop, NumAlpha, NumNumber);
        MaxAlpha = max(MaxAlpha, NumAlpha);
        MaxNumber = max(MaxNumber, NumNumber);
        CurrentModuleObject = "ElectricEquipment:ITE:AirCooled";
        inputProcessor->getObjectDefMaxArgs(CurrentModuleObject, Loop, NumAlpha, NumNumber);
        MaxAlpha = max(MaxAlpha, NumAlpha);
        MaxNumber = max(MaxNumber, NumNumber);
        CurrentModuleObject = "ZoneBaseboard:OutdoorTemperatureControlled";
        inputProcessor->getObjectDefMaxArgs(CurrentModuleObject, Loop, NumAlpha, NumNumber);
        MaxAlpha = max(MaxAlpha, NumAlpha);
        MaxNumber = max(MaxNumber, NumNumber);
        CurrentModuleObject = "ZoneContaminantSourceAndSink:CarbonDioxide";
        inputProcessor->getObjectDefMaxArgs(CurrentModuleObject, Loop, NumAlpha, NumNumber);
        MaxAlpha = max(MaxAlpha, NumAlpha);
        MaxNumber = max(MaxNumber, NumNumber);

        IHGNumbers.allocate(MaxNumber);
        AlphaName.allocate(MaxAlpha);
        IHGNumbers = 0.0;
        AlphaName = "";

        // CurrentModuleObject='Zone'
        for (Loop = 1; Loop <= NumOfZones; ++Loop) {
            // Overall Zone Variables
            SetupOutputVariable(state,
                "Zone Total Internal Radiant Heating Energy", OutputProcessor::Unit::J, ZnRpt(Loop).TotRadiantGain, "Zone", "Sum", Zone(Loop).Name);
            SetupOutputVariable(state, "Zone Total Internal Radiant Heating Rate",
                                OutputProcessor::Unit::W,
                                ZnRpt(Loop).TotRadiantGainRate,
                                "Zone",
                                "Average",
                                Zone(Loop).Name);
            SetupOutputVariable(state, "Zone Total Internal Visible Radiation Heating Energy",
                                OutputProcessor::Unit::J,
                                ZnRpt(Loop).TotVisHeatGain,
                                "Zone",
                                "Sum",
                                Zone(Loop).Name);
            SetupOutputVariable(state, "Zone Total Internal Visible Radiation Heating Rate",
                                OutputProcessor::Unit::W,
                                ZnRpt(Loop).TotVisHeatGainRate,
                                "Zone",
                                "Average",
                                Zone(Loop).Name);
            SetupOutputVariable(state, "Zone Total Internal Convective Heating Energy",
                                OutputProcessor::Unit::J,
                                ZnRpt(Loop).TotConvectiveGain,
                                "Zone",
                                "Sum",
                                Zone(Loop).Name);
            SetupOutputVariable(state, "Zone Total Internal Convective Heating Rate",
                                OutputProcessor::Unit::W,
                                ZnRpt(Loop).TotConvectiveGainRate,
                                "Zone",
                                "Average",
                                Zone(Loop).Name);
            SetupOutputVariable(state,
                "Zone Total Internal Latent Gain Energy", OutputProcessor::Unit::J, ZnRpt(Loop).TotLatentGain, "Zone", "Sum", Zone(Loop).Name);
            SetupOutputVariable(state,
                "Zone Total Internal Latent Gain Rate", OutputProcessor::Unit::W, ZnRpt(Loop).TotLatentGainRate, "Zone", "Average", Zone(Loop).Name);
            SetupOutputVariable(state,
                "Zone Total Internal Total Heating Energy", OutputProcessor::Unit::J, ZnRpt(Loop).TotTotalHeatGain, "Zone", "Sum", Zone(Loop).Name);
            SetupOutputVariable(state, "Zone Total Internal Total Heating Rate",
                                OutputProcessor::Unit::W,
                                ZnRpt(Loop).TotTotalHeatGainRate,
                                "Zone",
                                "Average",
                                Zone(Loop).Name);
        }

        // PEOPLE: Includes both information related to the heat balance and thermal comfort
        // First, allocate and initialize the People derived type
        CurrentModuleObject = "People";
        NumPeopleStatements = inputProcessor->getNumObjectsFound(CurrentModuleObject);
        PeopleObjects.allocate(NumPeopleStatements);

        TotPeople = 0;
        errFlag = false;
        for (Item = 1; Item <= NumPeopleStatements; ++Item) {
            inputProcessor->getObjectItem(state,
                                          CurrentModuleObject,
                                          Item,
                                          AlphaName,
                                          NumAlpha,
                                          IHGNumbers,
                                          NumNumber,
                                          IOStat,
                                          lNumericFieldBlanks,
                                          lAlphaFieldBlanks,
                                          cAlphaFieldNames,
                                          cNumericFieldNames);
            UtilityRoutines::IsNameEmpty(AlphaName(1), CurrentModuleObject, ErrorsFound);
            errFlag = ErrorsFound;

            PeopleObjects(Item).Name = AlphaName(1);

            Item1 = UtilityRoutines::FindItemInList(AlphaName(2), Zone);
            ZLItem = 0;
            if (Item1 == 0 && NumOfZoneLists > 0) ZLItem = UtilityRoutines::FindItemInList(AlphaName(2), ZoneList);
            if (Item1 > 0) {
                PeopleObjects(Item).StartPtr = TotPeople + 1;
                ++TotPeople;
                PeopleObjects(Item).NumOfZones = 1;
                PeopleObjects(Item).ZoneListActive = false;
                PeopleObjects(Item).ZoneOrZoneListPtr = Item1;
            } else if (ZLItem > 0) {
                PeopleObjects(Item).StartPtr = TotPeople + 1;
                TotPeople += ZoneList(ZLItem).NumOfZones;
                PeopleObjects(Item).NumOfZones = ZoneList(ZLItem).NumOfZones;
                PeopleObjects(Item).ZoneListActive = true;
                PeopleObjects(Item).ZoneOrZoneListPtr = ZLItem;
            } else {
                ShowSevereError(CurrentModuleObject + "=\"" + AlphaName(1) + "\" invalid " + cAlphaFieldNames(2) + "=\"" + AlphaName(2) +
                                "\" not found.");
                ErrorsFound = true;
                errFlag = true;
            }
        }

        if (errFlag) {
            ShowSevereError(RoutineName + "Errors with invalid names in " + CurrentModuleObject + " objects.");
            ShowContinueError("...These will not be read in.  Other errors may occur.");
            TotPeople = 0;
        }

        People.allocate(TotPeople);

        if (TotPeople > 0) {
            Loop = 0;
            for (Item = 1; Item <= NumPeopleStatements; ++Item) {
                AlphaName = BlankString;
                IHGNumbers = 0.0;

                inputProcessor->getObjectItem(state,
                                              CurrentModuleObject,
                                              Item,
                                              AlphaName,
                                              NumAlpha,
                                              IHGNumbers,
                                              NumNumber,
                                              IOStat,
                                              lNumericFieldBlanks,
                                              lAlphaFieldBlanks,
                                              cAlphaFieldNames,
                                              cNumericFieldNames);

                for (Item1 = 1; Item1 <= PeopleObjects(Item).NumOfZones; ++Item1) {
                    ++Loop;
                    if (!PeopleObjects(Item).ZoneListActive) {
                        People(Loop).Name = AlphaName(1);
                        People(Loop).ZonePtr = PeopleObjects(Item).ZoneOrZoneListPtr;
                    } else {
                        CheckCreatedZoneItemName(RoutineName,
                                                 CurrentModuleObject,
                                                 Zone(ZoneList(PeopleObjects(Item).ZoneOrZoneListPtr).Zone(Item1)).Name,
                                                 ZoneList(PeopleObjects(Item).ZoneOrZoneListPtr).MaxZoneNameLength,
                                                 PeopleObjects(Item).Name,
                                                 People,
                                                 Loop - 1,
                                                 People(Loop).Name,
                                                 errFlag);
                        People(Loop).ZonePtr = ZoneList(PeopleObjects(Item).ZoneOrZoneListPtr).Zone(Item1);
                        if (errFlag) ErrorsFound = true;
                    }

                    People(Loop).NumberOfPeoplePtr = GetScheduleIndex(state, AlphaName(3));
                    SchMin = 0.0;
                    SchMax = 0.0;
                    if (People(Loop).NumberOfPeoplePtr == 0) {
                        if (Item1 == 1) { // only show error on first one
                            if (lAlphaFieldBlanks(3)) {
                                ShowSevereError(RoutineName + CurrentModuleObject + "=\"" + AlphaName(1) + "\", " + cAlphaFieldNames(3) +
                                                " is required.");
                            } else {
                                ShowSevereError(RoutineName + CurrentModuleObject + "=\"" + AlphaName(1) + "\", invalid " + cAlphaFieldNames(3) +
                                                " entered=" + AlphaName(3));
                            }
                            ErrorsFound = true;
                        }
                    } else { // check min/max on schedule
                        SchMin = GetScheduleMinValue(People(Loop).NumberOfPeoplePtr);
                        SchMax = GetScheduleMaxValue(People(Loop).NumberOfPeoplePtr);
                        if (SchMin < 0.0 || SchMax < 0.0) {
                            if (Item1 == 1) {
                                if (SchMin < 0.0) {
                                    ShowSevereError(RoutineName + CurrentModuleObject + "=\"" + AlphaName(1) + "\", " + cAlphaFieldNames(3) +
                                                    ", minimum is < 0.0");
                                    ShowContinueError("Schedule=\"" + AlphaName(3) + "\". Minimum is [" + RoundSigDigits(SchMin, 1) +
                                                      "]. Values must be >= 0.0.");
                                    ErrorsFound = true;
                                }
                            }
                            if (Item1 == 1) {
                                if (SchMax < 0.0) {
                                    ShowSevereError(RoutineName + CurrentModuleObject + "=\"" + AlphaName(1) + "\", " + cAlphaFieldNames(3) +
                                                    ", maximum is < 0.0");
                                    ShowContinueError("Schedule=\"" + AlphaName(3) + "\". Maximum is [" + RoundSigDigits(SchMax, 1) +
                                                      "]. Values must be >= 0.0.");
                                    ErrorsFound = true;
                                }
                            }
                        }
                    }

                    // Number of people calculation method.
                    {
                        auto const peopleMethod(AlphaName(4));
                        if (peopleMethod == "PEOPLE") {
                            People(Loop).NumberOfPeople = IHGNumbers(1);
                            if (lNumericFieldBlanks(1)) {
                                ShowWarningError(RoutineName + CurrentModuleObject + "=\"" + People(Loop).Name + "\", specifies " +
                                                 cNumericFieldNames(1) + ", but that field is blank.  0 People will result.");
                            }

                        } else if (peopleMethod == "PEOPLE/AREA") {
                            if (People(Loop).ZonePtr != 0) {
                                if (IHGNumbers(2) >= 0.0) {
                                    People(Loop).NumberOfPeople = IHGNumbers(2) * Zone(People(Loop).ZonePtr).FloorArea;
                                    if (Zone(People(Loop).ZonePtr).FloorArea <= 0.0) {
                                        ShowWarningError(RoutineName + CurrentModuleObject + "=\"" + People(Loop).Name + "\", specifies " +
                                                         cNumericFieldNames(2) + ", but Zone Floor Area = 0.  0 People will result.");
                                    }
                                } else {
                                    ShowSevereError(RoutineName + CurrentModuleObject + "=\"" + People(Loop).Name + "\", invalid " +
                                                    cNumericFieldNames(2) + ", value  [<0.0]=" + RoundSigDigits(IHGNumbers(2), 3));
                                    ErrorsFound = true;
                                }
                            }
                            if (lNumericFieldBlanks(2)) {
                                ShowWarningError(RoutineName + CurrentModuleObject + "=\"" + People(Loop).Name + "\", specifies " +
                                                 cNumericFieldNames(2) + ", but that field is blank.  0 People will result.");
                            }

                        } else if (peopleMethod == "AREA/PERSON") {
                            if (People(Loop).ZonePtr != 0) {
                                if (IHGNumbers(3) > 0.0) {
                                    People(Loop).NumberOfPeople = Zone(People(Loop).ZonePtr).FloorArea / IHGNumbers(3);
                                    if (Zone(People(Loop).ZonePtr).FloorArea <= 0.0) {
                                        ShowWarningError(RoutineName + CurrentModuleObject + "=\"" + People(Loop).Name + "\", specifies " +
                                                         cNumericFieldNames(2) + ", but Zone Floor Area = 0.  0 People will result.");
                                    }
                                } else {
                                    ShowSevereError(RoutineName + CurrentModuleObject + "=\"" + People(Loop).Name + "\", invalid " +
                                                    cNumericFieldNames(3) + ", value  [<0.0]=" + RoundSigDigits(IHGNumbers(3), 3));
                                    ErrorsFound = true;
                                }
                            }
                            if (lNumericFieldBlanks(3)) {
                                ShowWarningError(RoutineName + CurrentModuleObject + "=\"" + People(Loop).Name + "\", specifies " +
                                                 cNumericFieldNames(3) + ", but that field is blank.  0 People will result.");
                            }

                        } else {
                            if (Item1 == 1) {
                                ShowSevereError(RoutineName + CurrentModuleObject + "=\"" + AlphaName(1) + "\", invalid " + cAlphaFieldNames(4) +
                                                ", value  =" + AlphaName(4));
                                ShowContinueError("...Valid values are \"People\", \"People/Area\", \"Area/Person\".");
                                ErrorsFound = true;
                            }
                        }
                    }

                    // Calculate nominal min/max people
                    People(Loop).NomMinNumberPeople = People(Loop).NumberOfPeople * SchMin;
                    People(Loop).NomMaxNumberPeople = People(Loop).NumberOfPeople * SchMax;

                    if (People(Loop).ZonePtr > 0) {
                        Zone(People(Loop).ZonePtr).TotOccupants += People(Loop).NumberOfPeople;
                    }

                    People(Loop).FractionRadiant = IHGNumbers(4);
                    People(Loop).FractionConvected = 1.0 - People(Loop).FractionRadiant;
                    if (Item1 == 1) {
                        if (People(Loop).FractionConvected < 0.0) {
                            ShowSevereError(RoutineName + CurrentModuleObject + "=\"" + AlphaName(1) + "\", " + cNumericFieldNames(4) +
                                            " < 0.0, value =" + RoundSigDigits(IHGNumbers(4), 2));
                            ErrorsFound = true;
                        }
                    }

                    if (NumNumber >= 5 && !lNumericFieldBlanks(5)) {
                        People(Loop).UserSpecSensFrac = IHGNumbers(5);
                    } else {
                        People(Loop).UserSpecSensFrac = AutoCalculate;
                    }

                    if (NumNumber == 6 && !lNumericFieldBlanks(6)) {
                        People(Loop).CO2RateFactor = IHGNumbers(6);
                    } else {
                        People(Loop).CO2RateFactor = 3.82e-8; // m3/s-W
                    }
                    if (People(Loop).CO2RateFactor < 0.0) {
                        ShowSevereError(RoutineName + CurrentModuleObject + "=\"" + AlphaName(1) + "\", " + cNumericFieldNames(6) +
                                        " < 0.0, value =" + RoundSigDigits(IHGNumbers(6), 2));
                        ErrorsFound = true;
                    }

                    People(Loop).ActivityLevelPtr = GetScheduleIndex(state, AlphaName(5));
                    if (People(Loop).ActivityLevelPtr == 0) {
                        if (Item1 == 1) {
                            if (lAlphaFieldBlanks(5)) {
                                ShowSevereError(RoutineName + CurrentModuleObject + "=\"" + AlphaName(1) + "\", " + cAlphaFieldNames(5) +
                                                " is required.");
                            } else {
                                ShowSevereError(RoutineName + CurrentModuleObject + "=\"" + AlphaName(1) + "\", invalid " + cAlphaFieldNames(5) +
                                                " entered=" + AlphaName(5));
                            }
                            ErrorsFound = true;
                        }
                    } else { // Check values in Schedule
                        SchMin = GetScheduleMinValue(People(Loop).ActivityLevelPtr);
                        SchMax = GetScheduleMaxValue(People(Loop).ActivityLevelPtr);
                        if (SchMin < 0.0 || SchMax < 0.0) {
                            if (Item1 == 1) {
                                if (SchMin < 0.0) {
                                    ShowSevereError(RoutineName + CurrentModuleObject + "=\"" + AlphaName(1) + "\", " + cAlphaFieldNames(5) +
                                                    " minimum is < 0.0");
                                    ShowContinueError("Schedule=\"" + AlphaName(5) + "\". Minimum is [" + RoundSigDigits(SchMin, 1) +
                                                      "]. Values must be >= 0.0.");
                                    ErrorsFound = true;
                                }
                            }
                            if (Item1 == 1) {
                                if (SchMax < 0.0) {
                                    ShowSevereError(RoutineName + CurrentModuleObject + "=\"" + AlphaName(1) + "\", " + cAlphaFieldNames(5) +
                                                    " maximum is < 0.0");
                                    ShowContinueError("Schedule=\"" + AlphaName(5) + "\". Maximum is [" + RoundSigDigits(SchMax, 1) +
                                                      "]. Values must be >= 0.0.");
                                    ErrorsFound = true;
                                }
                            }
                        } else if (SchMin < 70.0 || SchMax > 1000.0) {
                            if (Item1 == 1) {
                                ShowWarningError(RoutineName + CurrentModuleObject + "=\"" + AlphaName(1) + "\", " + cAlphaFieldNames(5) + " values");
                                ShowContinueError("fall outside typical range [70,1000] W/person for Thermal Comfort Reporting.");
                                ShowContinueError("Odd comfort values may result; Schedule=\"" + AlphaName(5) + "\".");
                                ShowContinueError("Entered min/max range=[" + RoundSigDigits(SchMin, 1) + ',' + RoundSigDigits(SchMax, 1) +
                                                  "] W/person.");
                            }
                        }
                    }

                    // Following is an optional parameter (ASHRAE 55 warnings
                    if (NumAlpha >= 6) {
                        if (UtilityRoutines::SameString(AlphaName(6), "Yes")) {
                            People(Loop).Show55Warning = true;
                        } else if (!UtilityRoutines::SameString(AlphaName(6), "No") && !lAlphaFieldBlanks(6)) {
                            if (Item1 == 1) {
                                ShowSevereError(RoutineName + CurrentModuleObject + "=\"" + AlphaName(1) + "\", " + cAlphaFieldNames(6) +
                                                " field should be Yes or No");
                                ShowContinueError("...Field value=\"" + AlphaName(6) + "\" is invalid.");
                                ErrorsFound = true;
                            }
                        }
                    }

                    if (NumAlpha > 6) { // Optional parameters present--thermal comfort data follows...
                        MustInpSch = false;
                        UsingThermalComfort = false;
                        lastOption = NumAlpha;

                        // check to see if the user has specified schedules for air velocity, clothing insulation, and/or work efficiency
                        // but have NOT made a selection for a thermal comfort model.  If so, then the schedules are reported as unused
                        // which could cause confusion.  The solution is for the user to either remove those schedules or pick a thermal
                        // comfort model.
                        int const NumFirstTCModel = 14;
                        if (NumAlpha < NumFirstTCModel) {
                            bool NoTCModelSelectedWithSchedules = false;
                            NoTCModelSelectedWithSchedules =
                                CheckThermalComfortSchedules(lAlphaFieldBlanks(9), lAlphaFieldBlanks(12), lAlphaFieldBlanks(13));
                            if (NoTCModelSelectedWithSchedules) {
                                ShowWarningError(RoutineName + CurrentModuleObject + "=\"" + AlphaName(1) +
                                                 "\" has comfort related schedules but no thermal comfort model selected.");
                                ShowContinueError("If schedules are specified for air velocity, clothing insulation, and/or work efficiency but no "
                                                  "thermal comfort");
                                ShowContinueError(
                                    "thermal comfort model is selected, the schedules will be listed as unused schedules in the .err file.");
                                ShowContinueError(
                                    "To avoid these errors, select a valid thermal comfort model or eliminate these schedules in the PEOPLE input.");
                            }
                        }

                        for (OptionNum = NumFirstTCModel; OptionNum <= lastOption; ++OptionNum) {

                            {
                                auto const thermalComfortType(AlphaName(OptionNum));

                                if (thermalComfortType == "FANGER") {
                                    People(Loop).Fanger = true;
                                    MustInpSch = true;
                                    UsingThermalComfort = true;

                                } else if (thermalComfortType == "PIERCE") {
                                    People(Loop).Pierce = true;
                                    MustInpSch = true;
                                    UsingThermalComfort = true;

                                } else if (thermalComfortType == "KSU") {
                                    People(Loop).KSU = true;
                                    MustInpSch = true;
                                    UsingThermalComfort = true;

                                } else if (thermalComfortType == "ADAPTIVEASH55") {
                                    People(Loop).AdaptiveASH55 = true;
                                    AdaptiveComfortRequested_ASH55 = true;
                                    MustInpSch = true;
                                    UsingThermalComfort = true;

                                } else if (thermalComfortType == "ADAPTIVECEN15251") {
                                    People(Loop).AdaptiveCEN15251 = true;
                                    AdaptiveComfortRequested_CEN15251 = true;
                                    MustInpSch = true;
                                    UsingThermalComfort = true;

                                } else if (thermalComfortType == "") { // Blank input field--just ignore this

                                } else { // An invalid keyword was entered--warn but ignore
                                    if (Item1 == 1) {
                                        ShowWarningError(RoutineName + CurrentModuleObject + "=\"" + AlphaName(1) + "\", invalid " +
                                                         cAlphaFieldNames(OptionNum) + " Option=" + AlphaName(OptionNum));
                                        ShowContinueError(
                                            "Valid Values are \"Fanger\", \"Pierce\", \"KSU\", \"AdaptiveASH55\", \"AdaptiveCEN15251\"");
                                    }
                                }
                            }
                        }

                        if (UsingThermalComfort) {

                            // Set the default value of MRTCalcType as 'ZoneAveraged'
                            People(Loop).MRTCalcType = ZoneAveraged;

                            // MRT Calculation Type and Surface Name
                            {
                                auto const mrtType(AlphaName(7));

                                if (mrtType == "ZONEAVERAGED") {
                                    People(Loop).MRTCalcType = ZoneAveraged;

                                } else if (mrtType == "SURFACEWEIGHTED") {
                                    People(Loop).MRTCalcType = SurfaceWeighted;
                                    People(Loop).SurfacePtr = UtilityRoutines::FindItemInList(AlphaName(8), Surface);
                                    if (People(Loop).SurfacePtr == 0 && (People(Loop).Fanger || People(Loop).Pierce || People(Loop).KSU)) {
                                        if (Item1 == 1) {
                                            ShowSevereError(RoutineName + CurrentModuleObject + "=\"" + AlphaName(1) + "\", " + cAlphaFieldNames(7) +
                                                            '=' + AlphaName(7) + " invalid Surface Name=" + AlphaName(8));
                                            ErrorsFound = true;
                                        }
                                    } else if (Surface(People(Loop).SurfacePtr).Zone != People(Loop).ZonePtr &&
                                               (People(Loop).Fanger || People(Loop).Pierce || People(Loop).KSU)) {
                                        ShowSevereError(RoutineName + CurrentModuleObject + "=\"" + AlphaName(1) + "\", Surface referenced in " +
                                                        cAlphaFieldNames(7) + '=' + AlphaName(8) + " in different zone.");
                                        ShowContinueError("Surface is in Zone=" + Zone(Surface(People(Loop).SurfacePtr).Zone).Name + " and " +
                                                          CurrentModuleObject + " is in Zone=" + AlphaName(2));
                                        ErrorsFound = true;
                                    }

                                } else if (mrtType == "ANGLEFACTOR") {
                                    People(Loop).MRTCalcType = AngleFactor;
                                    People(Loop).AngleFactorListName = AlphaName(8);

                                } else if (mrtType == "") { // Blank input field--just ignore this
                                    if (MustInpSch && Item1 == 1 && (People(Loop).Fanger || People(Loop).Pierce || People(Loop).KSU))
                                        ShowWarningError(RoutineName + CurrentModuleObject + "=\"" + AlphaName(1) + "\", blank " +
                                                         cAlphaFieldNames(7));

                                } else { // An invalid keyword was entered--warn but ignore
                                    if (MustInpSch && Item1 == 1 && (People(Loop).Fanger || People(Loop).Pierce || People(Loop).KSU)) {
                                        ShowWarningError(RoutineName + CurrentModuleObject + "=\"" + AlphaName(1) + "\", invalid " +
                                                         cAlphaFieldNames(7) + '=' + AlphaName(7));
                                        ShowContinueError("...Valid values are \"ZoneAveraged\", \"SurfaceWeighted\", \"AngleFactor\".");
                                    }
                                }
                            }

                            if (!lAlphaFieldBlanks(9)) {
                                People(Loop).WorkEffPtr = GetScheduleIndex(state, AlphaName(9));
                                if (People(Loop).WorkEffPtr == 0) {
                                    if (Item1 == 1) {
                                        ShowSevereError(RoutineName + CurrentModuleObject + "=\"" + AlphaName(1) + "\", invalid " +
                                                        cAlphaFieldNames(9) + " entered=" + AlphaName(9));
                                        ErrorsFound = true;
                                    }
                                } else { // check min/max on schedule
                                    SchMin = GetScheduleMinValue(People(Loop).WorkEffPtr);
                                    SchMax = GetScheduleMaxValue(People(Loop).WorkEffPtr);
                                    if (SchMin < 0.0 || SchMax < 0.0) {
                                        if (SchMin < 0.0) {
                                            if (Item1 == 1) {
                                                ShowSevereError(RoutineName + CurrentModuleObject + "=\"" + AlphaName(1) + "\", " +
                                                                cAlphaFieldNames(9) + ", minimum is < 0.0");
                                                ShowContinueError("Schedule=\"" + AlphaName(9) + "\". Minimum is [" + RoundSigDigits(SchMin, 1) +
                                                                  "]. Values must be >= 0.0.");
                                                ErrorsFound = true;
                                            }
                                        }
                                        if (SchMax < 0.0) {
                                            if (Item1 == 1) {
                                                ShowSevereError(RoutineName + CurrentModuleObject + "=\"" + AlphaName(1) + "\", " +
                                                                cAlphaFieldNames(9) + ", maximum is < 0.0");
                                                ShowContinueError("Schedule=\"" + AlphaName(9) + "\". Maximum is [" + RoundSigDigits(SchMax, 1) +
                                                                  "]. Values must be >= 0.0.");
                                                ErrorsFound = true;
                                            }
                                        }
                                    }
                                    if (SchMax > 1.0) {
                                        if (Item1 == 1) {
                                            ShowWarningError(RoutineName + CurrentModuleObject + "=\"" + AlphaName(1) + "\", " + cAlphaFieldNames(9) +
                                                             ", maximum is > 1.0");
                                            ShowContinueError("Schedule=\"" + AlphaName(9) + "\"; Entered min/max range=[" +
                                                              RoundSigDigits(SchMin, 1) + ',' + RoundSigDigits(SchMax, 1) + "] Work Efficiency.");
                                        }
                                    }
                                }
                            } else if (MustInpSch && (People(Loop).Fanger || People(Loop).Pierce || People(Loop).KSU)) {
                                if (Item1 == 1) {
                                    ShowSevereError(RoutineName + CurrentModuleObject + "=\"" + AlphaName(1) + "\", blank " + cAlphaFieldNames(9) +
                                                    " is required for this item.");
                                    ErrorsFound = true;
                                }
                            }

                            if (!lAlphaFieldBlanks(10) || AlphaName(10) != "") {
                                {
                                    auto const clothingType(AlphaName(10));
                                    if (clothingType == "CLOTHINGINSULATIONSCHEDULE") {
                                        People(Loop).ClothingType = 1;
                                        People(Loop).ClothingPtr = GetScheduleIndex(state, AlphaName(12));
                                        if (People(Loop).ClothingPtr == 0 && (People(Loop).Fanger || People(Loop).Pierce || People(Loop).KSU)) {
                                            if (Item1 == 1) {
                                                ShowSevereError(RoutineName + CurrentModuleObject + "=\"" + AlphaName(1) + "\", invalid " +
                                                                cAlphaFieldNames(12) + " entered=" + AlphaName(12));
                                                ErrorsFound = true;
                                            }
                                        } else { // check min/max on schedule
                                            SchMin = GetScheduleMinValue(People(Loop).ClothingPtr);
                                            SchMax = GetScheduleMaxValue(People(Loop).ClothingPtr);
                                            if (SchMin < 0.0 || SchMax < 0.0) {
                                                if (SchMin < 0.0) {
                                                    if (Item1 == 1) {
                                                        ShowSevereError(RoutineName + CurrentModuleObject + "=\"" + AlphaName(1) + "\", " +
                                                                        cAlphaFieldNames(12) + ", minimum is < 0.0");
                                                        ShowContinueError("Schedule=\"" + AlphaName(12) + "\". Minimum is [" +
                                                                          RoundSigDigits(SchMin, 1) + "]. Values must be >= 0.0.");
                                                        ErrorsFound = true;
                                                    }
                                                }
                                                if (SchMax < 0.0) {
                                                    if (Item1 == 1) {
                                                        ShowSevereError(RoutineName + CurrentModuleObject + "=\"" + AlphaName(1) + "\", " +
                                                                        cAlphaFieldNames(12) + ", maximum is < 0.0");
                                                        ShowContinueError("Schedule=\"" + AlphaName(12) + "\". Maximum is [" +
                                                                          RoundSigDigits(SchMax, 1) + "]. Values must be >= 0.0.");
                                                        ErrorsFound = true;
                                                    }
                                                }
                                            }
                                            if (SchMax > 2.0) {
                                                if (Item1 == 1) {
                                                    ShowWarningError(RoutineName + CurrentModuleObject + "=\"" + AlphaName(1) + "\", " +
                                                                     cAlphaFieldNames(12) + ", maximum is > 2.0");
                                                    ShowContinueError("Schedule=\"" + AlphaName(12) + "\"; Entered min/max range=[" +
                                                                      RoundSigDigits(SchMin, 1) + ',' + RoundSigDigits(SchMax, 1) + "] Clothing.");
                                                }
                                            }
                                        }

                                    } else if (clothingType == "DYNAMICCLOTHINGMODELASHRAE55") {
                                        People(Loop).ClothingType = 2;

                                    } else if (clothingType == "CALCULATIONMETHODSCHEDULE") {
                                        People(Loop).ClothingType = 3;
                                        People(Loop).ClothingMethodPtr = GetScheduleIndex(state, AlphaName(11));
                                        if (People(Loop).ClothingMethodPtr == 0) {
                                            if (Item1 == 1) {
                                                ShowSevereError(RoutineName + CurrentModuleObject + "=\"" + AlphaName(1) + "\", invalid " +
                                                                cAlphaFieldNames(11) + " entered=" + AlphaName(11));
                                                ErrorsFound = true;
                                            }
                                        }
                                        if (CheckScheduleValue(People(Loop).ClothingMethodPtr, 1)) {
                                            People(Loop).ClothingPtr = GetScheduleIndex(state, AlphaName(12));
                                            if (People(Loop).ClothingPtr == 0) {
                                                if (Item1 == 1) {
                                                    ShowSevereError(RoutineName + CurrentModuleObject + "=\"" + AlphaName(1) + "\", invalid " +
                                                                    cAlphaFieldNames(12) + " entered=" + AlphaName(12));
                                                    ErrorsFound = true;
                                                }
                                            }
                                        }

                                    } else {
                                        ShowSevereError(RoutineName + CurrentModuleObject + "=\"" + People(Loop).Name + "\", invalid " +
                                                        cAlphaFieldNames(10) + ", value  =" + AlphaName(10));
                                        ShowContinueError("...Valid values are \"ClothingInsulationSchedule\",\"DynamicClothingModelASHRAE55a\", "
                                                          "\"CalculationMethodSchedule\".");
                                        ErrorsFound = true;
                                    }
                                }
                            }

                            if (!lAlphaFieldBlanks(13)) {
                                People(Loop).AirVelocityPtr = GetScheduleIndex(state, AlphaName(13));
                                if (People(Loop).AirVelocityPtr == 0) {
                                    if (Item1 == 1) {
                                        ShowSevereError(RoutineName + CurrentModuleObject + "=\"" + AlphaName(1) + "\", invalid " +
                                                        cAlphaFieldNames(13) + " entered=" + AlphaName(13));
                                        ErrorsFound = true;
                                    }
                                } else { // check min/max on schedule
                                    SchMin = GetScheduleMinValue(People(Loop).AirVelocityPtr);
                                    SchMax = GetScheduleMaxValue(People(Loop).AirVelocityPtr);
                                    if (SchMin < 0.0 || SchMax < 0.0) {
                                        if (SchMin < 0.0) {
                                            if (Item1 == 1) {
                                                ShowSevereError(RoutineName + CurrentModuleObject + "=\"" + AlphaName(1) + "\", " +
                                                                cAlphaFieldNames(13) + ", minimum is < 0.0");
                                                ShowContinueError("Schedule=\"" + AlphaName(13) + "\". Minimum is [" + RoundSigDigits(SchMin, 1) +
                                                                  "]. Values must be >= 0.0.");
                                                ErrorsFound = true;
                                            }
                                        }
                                        if (SchMax < 0.0) {
                                            if (Item1 == 1) {
                                                ShowSevereError(RoutineName + CurrentModuleObject + "=\"" + AlphaName(1) + "\", " +
                                                                cAlphaFieldNames(13) + ", maximum is < 0.0");
                                                ShowContinueError("Schedule=\"" + AlphaName(13) + "\". Maximum is [" + RoundSigDigits(SchMax, 1) +
                                                                  "]. Values must be >= 0.0.");
                                                ErrorsFound = true;
                                            }
                                        }
                                    }
                                }
                            } else if (MustInpSch && (People(Loop).Fanger || People(Loop).Pierce || People(Loop).KSU)) {
                                if (Item1 == 1) {
                                    ShowSevereError(RoutineName + CurrentModuleObject + "=\"" + AlphaName(1) + "\", blank " + cAlphaFieldNames(13) +
                                                    " is required for this item.");
                                    ErrorsFound = true;
                                }
                            }

                        } // usingthermalcomfort block

                    } // ...end of thermal comfort data IF-THEN block  (NumAlphas > 6)

                    if (People(Loop).ZonePtr <= 0) continue; // Error, will be caught and terminated later

                    // Object report variables
                    SetupOutputVariable(state,
                        "People Occupant Count", OutputProcessor::Unit::None, People(Loop).NumOcc, "Zone", "Average", People(Loop).Name);
                    SetupOutputVariable(state,
                        "People Radiant Heating Energy", OutputProcessor::Unit::J, People(Loop).RadGainEnergy, "Zone", "Sum", People(Loop).Name);
                    SetupOutputVariable(state,
                        "People Radiant Heating Rate", OutputProcessor::Unit::W, People(Loop).RadGainRate, "Zone", "Average", People(Loop).Name);
                    SetupOutputVariable(state,
                        "People Convective Heating Energy", OutputProcessor::Unit::J, People(Loop).ConGainEnergy, "Zone", "Sum", People(Loop).Name);
                    SetupOutputVariable(state,
                        "People Convective Heating Rate", OutputProcessor::Unit::W, People(Loop).ConGainRate, "Zone", "Average", People(Loop).Name);
                    SetupOutputVariable(state,
                        "People Sensible Heating Energy", OutputProcessor::Unit::J, People(Loop).SenGainEnergy, "Zone", "Sum", People(Loop).Name);
                    SetupOutputVariable(state,
                        "People Sensible Heating Rate", OutputProcessor::Unit::W, People(Loop).SenGainRate, "Zone", "Average", People(Loop).Name);
                    SetupOutputVariable(state,
                        "People Latent Gain Energy", OutputProcessor::Unit::J, People(Loop).LatGainEnergy, "Zone", "Sum", People(Loop).Name);
                    SetupOutputVariable(state,
                        "People Latent Gain Rate", OutputProcessor::Unit::W, People(Loop).LatGainRate, "Zone", "Average", People(Loop).Name);
                    SetupOutputVariable(state,
                        "People Total Heating Energy", OutputProcessor::Unit::J, People(Loop).TotGainEnergy, "Zone", "Sum", People(Loop).Name);
                    SetupOutputVariable(state,
                        "People Total Heating Rate", OutputProcessor::Unit::W, People(Loop).TotGainRate, "Zone", "Average", People(Loop).Name);
                    SetupOutputVariable(state,
                        "People Air Temperature", OutputProcessor::Unit::C, People(Loop).TemperatureInZone, "Zone", "Average", People(Loop).Name);
                    SetupOutputVariable(state, "People Air Relative Humidity",
                                        OutputProcessor::Unit::Perc,
                                        People(Loop).RelativeHumidityInZone,
                                        "Zone",
                                        "Average",
                                        People(Loop).Name);

                    // Zone total report variables
                    if (RepVarSet(People(Loop).ZonePtr)) {
                        RepVarSet(People(Loop).ZonePtr) = false;
                        SetupOutputVariable(state, "Zone People Occupant Count",
                                            OutputProcessor::Unit::None,
                                            ZnRpt(People(Loop).ZonePtr).PeopleNumOcc,
                                            "Zone",
                                            "Average",
                                            Zone(People(Loop).ZonePtr).Name);
                        SetupOutputVariable(state, "Zone People Radiant Heating Energy",
                                            OutputProcessor::Unit::J,
                                            ZnRpt(People(Loop).ZonePtr).PeopleRadGain,
                                            "Zone",
                                            "Sum",
                                            Zone(People(Loop).ZonePtr).Name);
                        SetupOutputVariable(state, "Zone People Radiant Heating Rate",
                                            OutputProcessor::Unit::W,
                                            ZnRpt(People(Loop).ZonePtr).PeopleRadGainRate,
                                            "Zone",
                                            "Average",
                                            Zone(People(Loop).ZonePtr).Name);
                        SetupOutputVariable(state, "Zone People Convective Heating Energy",
                                            OutputProcessor::Unit::J,
                                            ZnRpt(People(Loop).ZonePtr).PeopleConGain,
                                            "Zone",
                                            "Sum",
                                            Zone(People(Loop).ZonePtr).Name);
                        SetupOutputVariable(state, "Zone People Convective Heating Rate",
                                            OutputProcessor::Unit::W,
                                            ZnRpt(People(Loop).ZonePtr).PeopleConGainRate,
                                            "Zone",
                                            "Average",
                                            Zone(People(Loop).ZonePtr).Name);
                        SetupOutputVariable(state, "Zone People Sensible Heating Energy",
                                            OutputProcessor::Unit::J,
                                            ZnRpt(People(Loop).ZonePtr).PeopleSenGain,
                                            "Zone",
                                            "Sum",
                                            Zone(People(Loop).ZonePtr).Name);
                        SetupOutputVariable(state, "Zone People Sensible Heating Rate",
                                            OutputProcessor::Unit::W,
                                            ZnRpt(People(Loop).ZonePtr).PeopleSenGainRate,
                                            "Zone",
                                            "Average",
                                            Zone(People(Loop).ZonePtr).Name);
                        SetupOutputVariable(state, "Zone People Latent Gain Energy",
                                            OutputProcessor::Unit::J,
                                            ZnRpt(People(Loop).ZonePtr).PeopleLatGain,
                                            "Zone",
                                            "Sum",
                                            Zone(People(Loop).ZonePtr).Name);
                        SetupOutputVariable(state, "Zone People Latent Gain Rate",
                                            OutputProcessor::Unit::W,
                                            ZnRpt(People(Loop).ZonePtr).PeopleLatGainRate,
                                            "Zone",
                                            "Average",
                                            Zone(People(Loop).ZonePtr).Name);
                        SetupOutputVariable(state, "Zone People Total Heating Energy",
                                            OutputProcessor::Unit::J,
                                            ZnRpt(People(Loop).ZonePtr).PeopleTotGain,
                                            "Zone",
                                            "Sum",
                                            Zone(People(Loop).ZonePtr).Name);
                        SetupOutputVariable(state, "Zone People Total Heating Rate",
                                            OutputProcessor::Unit::W,
                                            ZnRpt(People(Loop).ZonePtr).PeopleTotGainRate,
                                            "Zone",
                                            "Average",
                                            Zone(People(Loop).ZonePtr).Name);
                    }

                    if (AnyEnergyManagementSystemInModel) {
                        SetupEMSActuator(
                            "People", People(Loop).Name, "Number of People", "[each]", People(Loop).EMSPeopleOn, People(Loop).EMSNumberOfPeople);
                        SetupEMSInternalVariable("People Count Design Level", People(Loop).Name, "[each]", People(Loop).NumberOfPeople);
                    }

                    // setup internal gains
                    if (!ErrorsFound)
                        SetupZoneInternalGain(People(Loop).ZonePtr,
                                              "People",
                                              People(Loop).Name,
                                              IntGainTypeOf_People,
                                              &People(Loop).ConGainRate,
                                              nullptr,
                                              &People(Loop).RadGainRate,
                                              &People(Loop).LatGainRate,
                                              nullptr,
                                              &People(Loop).CO2GainRate);

                } // Item1 - number of zones
            }     // Item - number of people statements
        }         // TotPeople > 0

        // transfer the nominal number of people in a zone to the tabular reporting
        for (Loop = 1; Loop <= NumOfZones; ++Loop) {
            if (Zone(Loop).TotOccupants > 0.0) {
                if (Zone(Loop).FloorArea > 0.0 && Zone(Loop).FloorArea / Zone(Loop).TotOccupants < 0.1) {
                    ShowWarningError(RoutineName + "Zone=\"" + Zone(Loop).Name + "\" occupant density is extremely high.");
                    if (Zone(Loop).FloorArea > 0.0) {
                        ShowContinueError("Occupant Density=[" + RoundSigDigits(Zone(Loop).TotOccupants / Zone(Loop).FloorArea, 0) + "] person/m2.");
                    }
                    ShowContinueError("Occupant Density=[" + RoundSigDigits(Zone(Loop).FloorArea / Zone(Loop).TotOccupants, 3) +
                                      "] m2/person. Problems in Temperature Out of Bounds may result.");
                }
                maxOccupLoad = 0.0;
                for (Loop1 = 1; Loop1 <= TotPeople; ++Loop1) {
                    if (People(Loop1).ZonePtr != Loop) continue;
                    if (maxOccupLoad < GetScheduleMaxValue(People(Loop1).NumberOfPeoplePtr) * People(Loop1).NumberOfPeople) {
                        maxOccupLoad = GetScheduleMaxValue(People(Loop1).NumberOfPeoplePtr) * People(Loop1).NumberOfPeople;
                        MaxNumber = People(Loop1).NumberOfPeoplePtr;
                        OptionNum = Loop1;
                    }
                }
                if (maxOccupLoad > Zone(Loop).TotOccupants) {
                    if (Zone(Loop).FloorArea > 0.0 && Zone(Loop).FloorArea / maxOccupLoad < 0.1) {
                        ShowWarningError(RoutineName + "Zone=\"" + Zone(Loop).Name +
                                         "\" occupant density at a maximum schedule value is extremely high.");
                        if (Zone(Loop).FloorArea > 0.0) {
                            ShowContinueError("Occupant Density=[" + RoundSigDigits(maxOccupLoad / Zone(Loop).FloorArea, 0) + "] person/m2.");
                        }
                        ShowContinueError("Occupant Density=[" + RoundSigDigits(Zone(Loop).FloorArea / maxOccupLoad, 3) +
                                          "] m2/person. Problems in Temperature Out of Bounds may result.");
                        ShowContinueError("Check values in People=" + People(OptionNum).Name +
                                          ", Number of People Schedule=" + GetScheduleName(state, MaxNumber));
                    }
                }
            }

            if (Zone(Loop).isNominalControlled) { // conditioned zones only
                if (Zone(Loop).TotOccupants > 0.0) {
                    Zone(Loop).isNominalOccupied = true;
                    PreDefTableEntry(pdchOaoNomNumOcc1, Zone(Loop).Name, Zone(Loop).TotOccupants);
                    PreDefTableEntry(pdchOaoNomNumOcc2, Zone(Loop).Name, Zone(Loop).TotOccupants);
                }
            }
        }

        RepVarSet = true;
        CurrentModuleObject = "Lights";
        NumLightsStatements = inputProcessor->getNumObjectsFound(CurrentModuleObject);
        LightsObjects.allocate(NumLightsStatements);

        TotLights = 0;
        errFlag = false;
        for (Item = 1; Item <= NumLightsStatements; ++Item) {
            inputProcessor->getObjectItem(state,
                                          CurrentModuleObject,
                                          Item,
                                          AlphaName,
                                          NumAlpha,
                                          IHGNumbers,
                                          NumNumber,
                                          IOStat,
                                          lNumericFieldBlanks,
                                          lAlphaFieldBlanks,
                                          cAlphaFieldNames,
                                          cNumericFieldNames);
            UtilityRoutines::IsNameEmpty(AlphaName(1), CurrentModuleObject, ErrorsFound);
            errFlag = ErrorsFound;

            LightsObjects(Item).Name = AlphaName(1);

            Item1 = UtilityRoutines::FindItemInList(AlphaName(2), Zone);
            ZLItem = 0;
            if (Item1 == 0 && NumOfZoneLists > 0) ZLItem = UtilityRoutines::FindItemInList(AlphaName(2), ZoneList);
            if (Item1 > 0) {
                LightsObjects(Item).StartPtr = TotLights + 1;
                ++TotLights;
                LightsObjects(Item).NumOfZones = 1;
                LightsObjects(Item).ZoneListActive = false;
                LightsObjects(Item).ZoneOrZoneListPtr = Item1;
            } else if (ZLItem > 0) {
                LightsObjects(Item).StartPtr = TotLights + 1;
                TotLights += ZoneList(ZLItem).NumOfZones;
                LightsObjects(Item).NumOfZones = ZoneList(ZLItem).NumOfZones;
                LightsObjects(Item).ZoneListActive = true;
                LightsObjects(Item).ZoneOrZoneListPtr = ZLItem;
            } else {
                ShowSevereError(CurrentModuleObject + "=\"" + AlphaName(1) + "\" invalid " + cAlphaFieldNames(2) + "=\"" + AlphaName(2) +
                                "\" not found.");
                ErrorsFound = true;
                errFlag = true;
            }
        }

        if (errFlag) {
            ShowSevereError(RoutineName + "Errors with invalid names in " + CurrentModuleObject + " objects.");
            ShowContinueError("...These will not be read in.  Other errors may occur.");
            TotLights = 0;
        }

        Lights.allocate(TotLights);

        if (TotLights > 0) {
            Loop = 0;
            for (Item = 1; Item <= NumLightsStatements; ++Item) {
                AlphaName = BlankString;
                IHGNumbers = 0.0;

                inputProcessor->getObjectItem(state,
                                              CurrentModuleObject,
                                              Item,
                                              AlphaName,
                                              NumAlpha,
                                              IHGNumbers,
                                              NumNumber,
                                              IOStat,
                                              lNumericFieldBlanks,
                                              lAlphaFieldBlanks,
                                              cAlphaFieldNames,
                                              cNumericFieldNames);

                for (Item1 = 1; Item1 <= LightsObjects(Item).NumOfZones; ++Item1) {
                    ++Loop;
                    if (!LightsObjects(Item).ZoneListActive) {
                        Lights(Loop).Name = AlphaName(1);
                        Lights(Loop).ZonePtr = LightsObjects(Item).ZoneOrZoneListPtr;
                    } else {
                        CheckCreatedZoneItemName(RoutineName,
                                                 CurrentModuleObject,
                                                 Zone(ZoneList(LightsObjects(Item).ZoneOrZoneListPtr).Zone(Item1)).Name,
                                                 ZoneList(LightsObjects(Item).ZoneOrZoneListPtr).MaxZoneNameLength,
                                                 LightsObjects(Item).Name,
                                                 Lights,
                                                 Loop - 1,
                                                 Lights(Loop).Name,
                                                 errFlag);
                        Lights(Loop).ZonePtr = ZoneList(LightsObjects(Item).ZoneOrZoneListPtr).Zone(Item1);
                        if (errFlag) ErrorsFound = true;
                    }

                    Lights(Loop).SchedPtr = GetScheduleIndex(state, AlphaName(3));
                    SchMin = 0.0;
                    SchMax = 0.0;
                    if (Lights(Loop).SchedPtr == 0) {
                        if (Item1 == 1) {
                            if (lAlphaFieldBlanks(3)) {
                                ShowSevereError(RoutineName + CurrentModuleObject + "=\"" + AlphaName(1) + "\", " + cAlphaFieldNames(3) +
                                                " is required.");
                            } else {
                                ShowSevereError(RoutineName + CurrentModuleObject + "=\"" + AlphaName(1) + "\", invalid " + cAlphaFieldNames(3) +
                                                " entered=" + AlphaName(3));
                            }
                            ErrorsFound = true;
                        }
                    } else { // check min/max on schedule
                        SchMin = GetScheduleMinValue(Lights(Loop).SchedPtr);
                        SchMax = GetScheduleMaxValue(Lights(Loop).SchedPtr);
                        if (SchMin < 0.0 || SchMax < 0.0) {
                            if (Item1 == 1) {
                                if (SchMin < 0.0) {
                                    ShowSevereError(RoutineName + CurrentModuleObject + "=\"" + AlphaName(1) + "\", " + cAlphaFieldNames(3) +
                                                    ", minimum is < 0.0");
                                    ShowContinueError("Schedule=\"" + AlphaName(3) + "\". Minimum is [" + RoundSigDigits(SchMin, 1) +
                                                      "]. Values must be >= 0.0.");
                                    ErrorsFound = true;
                                }
                            }
                            if (Item1 == 1) {
                                if (SchMax < 0.0) {
                                    ShowSevereError(RoutineName + CurrentModuleObject + "=\"" + AlphaName(1) + "\", " + cAlphaFieldNames(3) +
                                                    ", maximum is < 0.0");
                                    ShowContinueError("Schedule=\"" + AlphaName(3) + "\". Maximum is [" + RoundSigDigits(SchMax, 1) +
                                                      "]. Values must be >= 0.0.");
                                    ErrorsFound = true;
                                }
                            }
                        }
                    }

                    // Lights Design Level calculation method.
                    {
                        auto const lightingLevel(AlphaName(4));
                        if (lightingLevel == "LIGHTINGLEVEL") {
                            Lights(Loop).DesignLevel = IHGNumbers(1);
                            if (lNumericFieldBlanks(1)) {
                                ShowWarningError(RoutineName + CurrentModuleObject + "=\"" + Lights(Loop).Name + "\", specifies " +
                                                 cNumericFieldNames(1) + ", but that field is blank.  0 Lights will result.");
                            }

                        } else if (lightingLevel == "WATTS/AREA") {
                            if (Lights(Loop).ZonePtr != 0) {
                                if (IHGNumbers(2) >= 0.0) {
                                    Lights(Loop).DesignLevel = IHGNumbers(2) * Zone(Lights(Loop).ZonePtr).FloorArea;
                                    if (Zone(Lights(Loop).ZonePtr).FloorArea <= 0.0) {
                                        ShowWarningError(RoutineName + CurrentModuleObject + "=\"" + Lights(Loop).Name + "\", specifies " +
                                                         cNumericFieldNames(2) + ", but Zone Floor Area = 0.  0 Lights will result.");
                                    }
                                } else {
                                    ShowSevereError(RoutineName + CurrentModuleObject + "=\"" + Lights(Loop).Name + "\", invalid " +
                                                    cNumericFieldNames(2) + ", value  [<0.0]=" + RoundSigDigits(IHGNumbers(2), 3));
                                    ErrorsFound = true;
                                }
                            }
                            if (lNumericFieldBlanks(2)) {
                                ShowWarningError(RoutineName + CurrentModuleObject + "=\"" + Lights(Loop).Name + "\", specifies " +
                                                 cNumericFieldNames(2) + ", but that field is blank.  0 Lights will result.");
                            }

                        } else if (lightingLevel == "WATTS/PERSON") {
                            if (Lights(Loop).ZonePtr != 0) {
                                if (IHGNumbers(3) >= 0.0) {
                                    Lights(Loop).DesignLevel = IHGNumbers(3) * Zone(Lights(Loop).ZonePtr).TotOccupants;
                                    if (Zone(Lights(Loop).ZonePtr).TotOccupants <= 0.0) {
                                        ShowWarningError(RoutineName + CurrentModuleObject + "=\"" + Lights(Loop).Name + "\", specifies " +
                                                         cNumericFieldNames(2) + ", but Total Occupants = 0.  0 Lights will result.");
                                    }
                                } else {
                                    ShowSevereError(RoutineName + CurrentModuleObject + "=\"" + Lights(Loop).Name + "\", invalid " +
                                                    cNumericFieldNames(3) + ", value  [<0.0]=" + RoundSigDigits(IHGNumbers(3), 3));
                                    ErrorsFound = true;
                                }
                            }
                            if (lNumericFieldBlanks(3)) {
                                ShowWarningError(RoutineName + CurrentModuleObject + "=\"" + Lights(Loop).Name + "\", specifies " +
                                                 cNumericFieldNames(3) + ", but that field is blank.  0 Lights will result.");
                            }

                        } else {
                            if (Item1 == 1) {
                                ShowSevereError(RoutineName + CurrentModuleObject + "=\"" + AlphaName(1) + "\", invalid " + cAlphaFieldNames(4) +
                                                ", value  =" + AlphaName(4));
                                ShowContinueError("...Valid values are \"LightingLevel\", \"Watts/Area\", \"Watts/Person\".");
                                ErrorsFound = true;
                            }
                        }
                    }

                    // Calculate nominal min/max lighting level
                    Lights(Loop).NomMinDesignLevel = Lights(Loop).DesignLevel * SchMin;
                    Lights(Loop).NomMaxDesignLevel = Lights(Loop).DesignLevel * SchMax;

                    Lights(Loop).FractionReturnAir = IHGNumbers(4);
                    Lights(Loop).FractionRadiant = IHGNumbers(5);
                    Lights(Loop).FractionShortWave = IHGNumbers(6);
                    Lights(Loop).FractionReplaceable = IHGNumbers(7);
                    Lights(Loop).FractionReturnAirPlenTempCoeff1 = IHGNumbers(8);
                    Lights(Loop).FractionReturnAirPlenTempCoeff2 = IHGNumbers(9);

                    Lights(Loop).FractionConvected =
                        1.0 - (Lights(Loop).FractionReturnAir + Lights(Loop).FractionRadiant + Lights(Loop).FractionShortWave);
                    if (std::abs(Lights(Loop).FractionConvected) <= 0.001) Lights(Loop).FractionConvected = 0.0;
                    if (Lights(Loop).FractionConvected < 0.0) {
                        if (Item1 == 1) {
                            ShowSevereError(RoutineName + CurrentModuleObject + "=\"" + AlphaName(1) + "\", Sum of Fractions > 1.0");
                            ErrorsFound = true;
                        }
                    }

                    // Note: if FractionReturnAirIsCalculated = Yes and there is a return-air plenum:
                    // (1) The input values of FractionReturnAir, FractionRadiant and FractionShortWave, and the
                    // value of FractionConvected calculated from these are used in the zone sizing calculations;
                    // (2) in the regular calculation, FractionReturnAir is calculated each time step in
                    // Subr. InitInternalHeatGains as a function of the zone's return plenum air temperature
                    // using FractionReturnAirPlenTempCoeff1 and FractionReturnAirPlenTempCoeff2; then
                    // FractionRadiant and FractionConvected are adjusted from their input values such that
                    // FractionReturnAir + FractionRadiant + FractionShortWave + FractionConvected = 1.0, assuming
                    // FractionShortWave is constant and equal to its input value.

                    if (NumAlpha > 4) {
                        Lights(Loop).EndUseSubcategory = AlphaName(5);
                    } else {
                        Lights(Loop).EndUseSubcategory = "General";
                    }

                    if (lAlphaFieldBlanks(6)) {
                        Lights(Loop).FractionReturnAirIsCalculated = false;
                    } else if (AlphaName(6) != "YES" && AlphaName(6) != "NO") {
                        if (Item1 == 1) {
                            ShowWarningError(RoutineName + CurrentModuleObject + "=\"" + AlphaName(1) + "\", invalid " + cAlphaFieldNames(6) +
                                             ", value  =" + AlphaName(6));
                            ShowContinueError(".. Return Air Fraction from Plenum will NOT be calculated.");
                        }
                        Lights(Loop).FractionReturnAirIsCalculated = false;
                    } else {
                        Lights(Loop).FractionReturnAirIsCalculated = (AlphaName(6) == "YES");
                    }

                    // Set return air node number
                    Lights(Loop).ZoneReturnNum = 0;
                    std::string retNodeName = "";
                    if (!lAlphaFieldBlanks(7)) {
                        if (LightsObjects(Item).ZoneListActive) {
                            ShowSevereError(RoutineName + CurrentModuleObject + "=\"" + Lights(Loop).Name + "\": " + cAlphaFieldNames(7) +
                                            " must be blank when using a ZoneList.");
                            ErrorsFound = true;
                        } else {
                            retNodeName = AlphaName(7);
                        }
                    }
                    if (Lights(Loop).ZonePtr > 0) {
                        Lights(Loop).ZoneReturnNum = DataZoneEquipment::GetReturnNumForZone(state, Zone(Lights(Loop).ZonePtr).Name, retNodeName);
                    }

                    if ((Lights(Loop).ZoneReturnNum == 0) && (Lights(Loop).FractionReturnAir > 0.0) && (!lAlphaFieldBlanks(7))) {
                        ShowSevereError(RoutineName + CurrentModuleObject + "=\"" + AlphaName(1) + "\", invalid " + cAlphaFieldNames(7) + " =" +
                                        AlphaName(7));
                        ShowContinueError("No matching Zone Return Air Node found.");
                        ErrorsFound = true;
                    }
                    if (Lights(Loop).ZonePtr <= 0) continue; // Error, will be caught and terminated later

                    // Object report variables
                    SetupOutputVariable(state, "Lights Electricity Rate", OutputProcessor::Unit::W, Lights(Loop).Power, "Zone", "Average", Lights(Loop).Name);

                    SetupOutputVariable(state,
                        "Lights Radiant Heating Energy", OutputProcessor::Unit::J, Lights(Loop).RadGainEnergy, "Zone", "Sum", Lights(Loop).Name);
                    SetupOutputVariable(state,
                        "Lights Radiant Heating Rate", OutputProcessor::Unit::W, Lights(Loop).RadGainRate, "Zone", "Average", Lights(Loop).Name);
                    SetupOutputVariable(state, "Lights Visible Radiation Heating Energy",
                                        OutputProcessor::Unit::J,
                                        Lights(Loop).VisGainEnergy,
                                        "Zone",
                                        "Sum",
                                        Lights(Loop).Name);

                    SetupOutputVariable(state, "Lights Visible Radiation Heating Rate",
                                        OutputProcessor::Unit::W,
                                        Lights(Loop).VisGainRate,
                                        "Zone",
                                        "Average",
                                        Lights(Loop).Name);
                    SetupOutputVariable(state,
                        "Lights Convective Heating Energy", OutputProcessor::Unit::J, Lights(Loop).ConGainEnergy, "Zone", "Sum", Lights(Loop).Name);
                    SetupOutputVariable(state,
                        "Lights Convective Heating Rate", OutputProcessor::Unit::W, Lights(Loop).ConGainRate, "Zone", "Average", Lights(Loop).Name);
                    SetupOutputVariable(state, "Lights Return Air Heating Energy",
                                        OutputProcessor::Unit::J,
                                        Lights(Loop).RetAirGainEnergy,
                                        "Zone",
                                        "Sum",
                                        Lights(Loop).Name);
                    SetupOutputVariable(state, "Lights Return Air Heating Rate",
                                        OutputProcessor::Unit::W,
                                        Lights(Loop).RetAirGainRate,
                                        "Zone",
                                        "Average",
                                        Lights(Loop).Name);
                    SetupOutputVariable(state,
                        "Lights Total Heating Energy", OutputProcessor::Unit::J, Lights(Loop).TotGainEnergy, "Zone", "Sum", Lights(Loop).Name);
                    SetupOutputVariable(state,
                        "Lights Total Heating Rate", OutputProcessor::Unit::W, Lights(Loop).TotGainRate, "Zone", "Average", Lights(Loop).Name);
                    SetupOutputVariable(state, "Lights Electricity Energy",
                                        OutputProcessor::Unit::J,
                                        Lights(Loop).Consumption,
                                        "Zone",
                                        "Sum",
                                        Lights(Loop).Name,
                                        _,
                                        "Electricity",
                                        "InteriorLights",
                                        Lights(Loop).EndUseSubcategory,
                                        "Building",
                                        Zone(Lights(Loop).ZonePtr).Name,
                                        Zone(Lights(Loop).ZonePtr).Multiplier,
                                        Zone(Lights(Loop).ZonePtr).ListMultiplier);

                    // Zone total report variables
                    if (RepVarSet(Lights(Loop).ZonePtr)) {
                        RepVarSet(Lights(Loop).ZonePtr) = false;
                        SetupOutputVariable(state, "Zone Lights Electricity Rate",
                                            OutputProcessor::Unit::W,
                                            ZnRpt(Lights(Loop).ZonePtr).LtsPower,
                                            "Zone",
                                            "Average",
                                            Zone(Lights(Loop).ZonePtr).Name);
                        SetupOutputVariable(state, "Zone Lights Electricity Energy",
                                            OutputProcessor::Unit::J,
                                            ZnRpt(Lights(Loop).ZonePtr).LtsElecConsump,
                                            "Zone",
                                            "Sum",
                                            Zone(Lights(Loop).ZonePtr).Name);
                        SetupOutputVariable(state, "Zone Lights Radiant Heating Energy",
                                            OutputProcessor::Unit::J,
                                            ZnRpt(Lights(Loop).ZonePtr).LtsRadGain,
                                            "Zone",
                                            "Sum",
                                            Zone(Lights(Loop).ZonePtr).Name);
                        SetupOutputVariable(state, "Zone Lights Radiant Heating Rate",
                                            OutputProcessor::Unit::W,
                                            ZnRpt(Lights(Loop).ZonePtr).LtsRadGainRate,
                                            "Zone",
                                            "Average",
                                            Zone(Lights(Loop).ZonePtr).Name);
                        SetupOutputVariable(state, "Zone Lights Visible Radiation Heating Energy",
                                            OutputProcessor::Unit::J,
                                            ZnRpt(Lights(Loop).ZonePtr).LtsVisGain,
                                            "Zone",
                                            "Sum",
                                            Zone(Lights(Loop).ZonePtr).Name);
                        SetupOutputVariable(state, "Zone Lights Visible Radiation Heating Rate",
                                            OutputProcessor::Unit::W,
                                            ZnRpt(Lights(Loop).ZonePtr).LtsVisGainRate,
                                            "Zone",
                                            "Average",
                                            Zone(Lights(Loop).ZonePtr).Name);
                        SetupOutputVariable(state, "Zone Lights Convective Heating Energy",
                                            OutputProcessor::Unit::J,
                                            ZnRpt(Lights(Loop).ZonePtr).LtsConGain,
                                            "Zone",
                                            "Sum",
                                            Zone(Lights(Loop).ZonePtr).Name);
                        SetupOutputVariable(state, "Zone Lights Convective Heating Rate",
                                            OutputProcessor::Unit::W,
                                            ZnRpt(Lights(Loop).ZonePtr).LtsConGainRate,
                                            "Zone",
                                            "Average",
                                            Zone(Lights(Loop).ZonePtr).Name);
                        SetupOutputVariable(state, "Zone Lights Return Air Heating Energy",
                                            OutputProcessor::Unit::J,
                                            ZnRpt(Lights(Loop).ZonePtr).LtsRetAirGain,
                                            "Zone",
                                            "Sum",
                                            Zone(Lights(Loop).ZonePtr).Name);
                        SetupOutputVariable(state, "Zone Lights Return Air Heating Rate",
                                            OutputProcessor::Unit::W,
                                            ZnRpt(Lights(Loop).ZonePtr).LtsRetAirGainRate,
                                            "Zone",
                                            "Average",
                                            Zone(Lights(Loop).ZonePtr).Name);
                        SetupOutputVariable(state, "Zone Lights Total Heating Energy",
                                            OutputProcessor::Unit::J,
                                            ZnRpt(Lights(Loop).ZonePtr).LtsTotGain,
                                            "Zone",
                                            "Sum",
                                            Zone(Lights(Loop).ZonePtr).Name);
                        SetupOutputVariable(state, "Zone Lights Total Heating Rate",
                                            OutputProcessor::Unit::W,
                                            ZnRpt(Lights(Loop).ZonePtr).LtsTotGainRate,
                                            "Zone",
                                            "Average",
                                            Zone(Lights(Loop).ZonePtr).Name);
                    }

                    if (AnyEnergyManagementSystemInModel) {
                        SetupEMSActuator(
                            "Lights", Lights(Loop).Name, "Electricity Rate", "[W]", Lights(Loop).EMSLightsOn, Lights(Loop).EMSLightingPower);
                        SetupEMSInternalVariable("Lighting Power Design Level", Lights(Loop).Name, "[W]", Lights(Loop).DesignLevel);
                    } // EMS
                    // setup internal gains
                    int returnNodeNum = 0;
                    if ((Lights(Loop).ZoneReturnNum > 0) &&
                        (Lights(Loop).ZoneReturnNum <= DataZoneEquipment::ZoneEquipConfig(Lights(Loop).ZonePtr).NumReturnNodes)) {
                        returnNodeNum = DataZoneEquipment::ZoneEquipConfig(Lights(Loop).ZonePtr).ReturnNode(Lights(Loop).ZoneReturnNum);
                    }
                    if (!ErrorsFound)
                        SetupZoneInternalGain(Lights(Loop).ZonePtr,
                                              "Lights",
                                              Lights(Loop).Name,
                                              IntGainTypeOf_Lights,
                                              &Lights(Loop).ConGainRate,
                                              &Lights(Loop).RetAirGainRate,
                                              &Lights(Loop).RadGainRate,
                                              nullptr,
                                              nullptr,
                                              nullptr,
                                              nullptr,
                                              returnNodeNum);

                    if (Lights(Loop).FractionReturnAir > 0) Zone(Lights(Loop).ZonePtr).HasLtsRetAirGain = true;
                    // send values to predefined lighting summary report
                    liteName = Lights(Loop).Name;
                    zonePt = Lights(Loop).ZonePtr;
                    mult = Zone(zonePt).Multiplier * Zone(zonePt).ListMultiplier;
                    sumArea += Zone(zonePt).FloorArea * mult;
                    sumPower += Lights(Loop).DesignLevel * mult;
                    PreDefTableEntry(pdchInLtZone, liteName, Zone(zonePt).Name);
                    if (Zone(zonePt).FloorArea > 0.0) {
                        PreDefTableEntry(pdchInLtDens, liteName, Lights(Loop).DesignLevel / Zone(zonePt).FloorArea, 4);
                    } else {
                        PreDefTableEntry(pdchInLtDens, liteName, constant_zero, 4);
                    }
                    PreDefTableEntry(pdchInLtArea, liteName, Zone(zonePt).FloorArea * mult);
                    PreDefTableEntry(pdchInLtPower, liteName, Lights(Loop).DesignLevel * mult);
                    PreDefTableEntry(pdchInLtEndUse, liteName, Lights(Loop).EndUseSubcategory);
                    PreDefTableEntry(pdchInLtSchd, liteName, GetScheduleName(state, Lights(Loop).SchedPtr));
                    PreDefTableEntry(pdchInLtRetAir, liteName, Lights(Loop).FractionReturnAir, 4);
                } // Item1 - zones
            }     // Item = Number of Lights Objects
        }         // TotLights > 0 check
        // add total line to lighting summary table
        if (sumArea > 0.0) {
            PreDefTableEntry(pdchInLtDens, "Interior Lighting Total", sumPower / sumArea, 4); //** line 792
        } else {
            PreDefTableEntry(pdchInLtDens, "Interior Lighting Total", constant_zero, 4);
        }
        PreDefTableEntry(pdchInLtArea, "Interior Lighting Total", sumArea);
        PreDefTableEntry(pdchInLtPower, "Interior Lighting Total", sumPower);

        RepVarSet = true;
        CurrentModuleObject = "ElectricEquipment";
        NumZoneElectricStatements = inputProcessor->getNumObjectsFound(CurrentModuleObject);
        ZoneElectricObjects.allocate(NumZoneElectricStatements);

        TotElecEquip = 0;
        errFlag = false;
        for (Item = 1; Item <= NumZoneElectricStatements; ++Item) {
            inputProcessor->getObjectItem(state,
                                          CurrentModuleObject,
                                          Item,
                                          AlphaName,
                                          NumAlpha,
                                          IHGNumbers,
                                          NumNumber,
                                          IOStat,
                                          lNumericFieldBlanks,
                                          lAlphaFieldBlanks,
                                          cAlphaFieldNames,
                                          cNumericFieldNames);
            UtilityRoutines::IsNameEmpty(AlphaName(1), CurrentModuleObject, ErrorsFound);
            errFlag = ErrorsFound;

            ZoneElectricObjects(Item).Name = AlphaName(1);

            Item1 = UtilityRoutines::FindItemInList(AlphaName(2), Zone);
            ZLItem = 0;
            if (Item1 == 0 && NumOfZoneLists > 0) ZLItem = UtilityRoutines::FindItemInList(AlphaName(2), ZoneList);
            if (Item1 > 0) {
                ZoneElectricObjects(Item).StartPtr = TotElecEquip + 1;
                ++TotElecEquip;
                ZoneElectricObjects(Item).NumOfZones = 1;
                ZoneElectricObjects(Item).ZoneListActive = false;
                ZoneElectricObjects(Item).ZoneOrZoneListPtr = Item1;
            } else if (ZLItem > 0) {
                ZoneElectricObjects(Item).StartPtr = TotElecEquip + 1;
                TotElecEquip += ZoneList(ZLItem).NumOfZones;
                ZoneElectricObjects(Item).NumOfZones = ZoneList(ZLItem).NumOfZones;
                ZoneElectricObjects(Item).ZoneListActive = true;
                ZoneElectricObjects(Item).ZoneOrZoneListPtr = ZLItem;
            } else {
                ShowSevereError(CurrentModuleObject + "=\"" + AlphaName(1) + "\" invalid " + cAlphaFieldNames(2) + "=\"" + AlphaName(2) +
                                "\" not found.");
                ErrorsFound = true;
                errFlag = true;
            }
        }

        if (errFlag) {
            ShowSevereError(RoutineName + "Errors with invalid names in " + CurrentModuleObject + " objects.");
            ShowContinueError("...These will not be read in.  Other errors may occur.");
            TotElecEquip = 0;
        }

        ZoneElectric.allocate(TotElecEquip);

        if (TotElecEquip > 0) {
            Loop = 0;
            for (Item = 1; Item <= NumZoneElectricStatements; ++Item) {
                AlphaName = BlankString;
                IHGNumbers = 0.0;

                inputProcessor->getObjectItem(state,
                                              CurrentModuleObject,
                                              Item,
                                              AlphaName,
                                              NumAlpha,
                                              IHGNumbers,
                                              NumNumber,
                                              IOStat,
                                              lNumericFieldBlanks,
                                              lAlphaFieldBlanks,
                                              cAlphaFieldNames,
                                              cNumericFieldNames);

                for (Item1 = 1; Item1 <= ZoneElectricObjects(Item).NumOfZones; ++Item1) {
                    ++Loop;
                    if (!ZoneElectricObjects(Item).ZoneListActive) {
                        ZoneElectric(Loop).Name = AlphaName(1);
                        ZoneElectric(Loop).ZonePtr = ZoneElectricObjects(Item).ZoneOrZoneListPtr;
                    } else {
                        CheckCreatedZoneItemName(RoutineName,
                                                 CurrentModuleObject,
                                                 Zone(ZoneList(ZoneElectricObjects(Item).ZoneOrZoneListPtr).Zone(Item1)).Name,
                                                 ZoneList(ZoneElectricObjects(Item).ZoneOrZoneListPtr).MaxZoneNameLength,
                                                 ZoneElectricObjects(Item).Name,
                                                 ZoneElectric,
                                                 Loop - 1,
                                                 ZoneElectric(Loop).Name,
                                                 errFlag);
                        ZoneElectric(Loop).ZonePtr = ZoneList(ZoneElectricObjects(Item).ZoneOrZoneListPtr).Zone(Item1);
                        if (errFlag) ErrorsFound = true;
                    }

                    ZoneElectric(Loop).SchedPtr = GetScheduleIndex(state, AlphaName(3));
                    SchMin = 0.0;
                    SchMax = 0.0;
                    if (ZoneElectric(Loop).SchedPtr == 0) {
                        if (lAlphaFieldBlanks(3)) {
                            ShowSevereError(RoutineName + CurrentModuleObject + "=\"" + AlphaName(1) + "\", " + cAlphaFieldNames(3) +
                                            " is required.");
                        } else {
                            ShowSevereError(RoutineName + CurrentModuleObject + "=\"" + AlphaName(1) + "\", invalid " + cAlphaFieldNames(3) +
                                            " entered=" + AlphaName(3));
                        }
                        ErrorsFound = true;
                    } else { // check min/max on schedule
                        SchMin = GetScheduleMinValue(ZoneElectric(Loop).SchedPtr);
                        SchMax = GetScheduleMaxValue(ZoneElectric(Loop).SchedPtr);
                        if (SchMin < 0.0 || SchMax < 0.0) {
                            if (SchMin < 0.0) {
                                ShowSevereError(RoutineName + CurrentModuleObject + "=\"" + AlphaName(1) + "\", " + cAlphaFieldNames(3) +
                                                ", minimum is < 0.0");
                                ShowContinueError("Schedule=\"" + AlphaName(3) + "\". Minimum is [" + RoundSigDigits(SchMin, 1) +
                                                  "]. Values must be >= 0.0.");
                                ErrorsFound = true;
                            }
                            if (SchMax < 0.0) {
                                ShowSevereError(RoutineName + CurrentModuleObject + "=\"" + AlphaName(1) + "\", " + cAlphaFieldNames(3) +
                                                ", maximum is < 0.0");
                                ShowContinueError("Schedule=\"" + AlphaName(3) + "\". Maximum is [" + RoundSigDigits(SchMax, 1) +
                                                  "]. Values must be >= 0.0.");
                                ErrorsFound = true;
                            }
                        }
                    }

                    // Electric equipment design level calculation method.
                    {
                        auto const equipmentLevel(AlphaName(4));
                        if (equipmentLevel == "EQUIPMENTLEVEL") {
                            ZoneElectric(Loop).DesignLevel = IHGNumbers(1);
                            if (lNumericFieldBlanks(1)) {
                                ShowWarningError(RoutineName + CurrentModuleObject + "=\"" + AlphaName(1) + "\", specifies " + cNumericFieldNames(1) +
                                                 ", but that field is blank.  0 Electric Equipment will result.");
                            }

                        } else if (equipmentLevel == "WATTS/AREA") {
                            if (ZoneElectric(Loop).ZonePtr != 0) {
                                if (IHGNumbers(2) >= 0.0) {
                                    ZoneElectric(Loop).DesignLevel = IHGNumbers(2) * Zone(ZoneElectric(Loop).ZonePtr).FloorArea;
                                    if (Zone(ZoneElectric(Loop).ZonePtr).FloorArea <= 0.0) {
                                        ShowWarningError(RoutineName + CurrentModuleObject + "=\"" + AlphaName(1) + "\", specifies " +
                                                         cNumericFieldNames(2) + ", but Zone Floor Area = 0.  0 Electric Equipment will result.");
                                    }
                                } else {
                                    ShowSevereError(RoutineName + CurrentModuleObject + "=\"" + AlphaName(1) + "\", invalid " +
                                                    cNumericFieldNames(2) + ", value  [<0.0]=" + RoundSigDigits(IHGNumbers(2), 3));
                                    ErrorsFound = true;
                                }
                            }
                            if (lNumericFieldBlanks(2)) {
                                ShowWarningError(RoutineName + CurrentModuleObject + "=\"" + AlphaName(1) + "\", specifies " + cNumericFieldNames(2) +
                                                 ", but that field is blank.  0 Electric Equipment will result.");
                            }

                        } else if (equipmentLevel == "WATTS/PERSON") {
                            if (ZoneElectric(Loop).ZonePtr != 0) {
                                if (IHGNumbers(3) >= 0.0) {
                                    ZoneElectric(Loop).DesignLevel = IHGNumbers(3) * Zone(ZoneElectric(Loop).ZonePtr).TotOccupants;
                                    if (Zone(ZoneElectric(Loop).ZonePtr).TotOccupants <= 0.0) {
                                        ShowWarningError(RoutineName + CurrentModuleObject + "=\"" + AlphaName(1) + "\", specifies " +
                                                         cNumericFieldNames(2) + ", but Total Occupants = 0.  0 Electric Equipment will result.");
                                    }
                                } else {
                                    ShowSevereError(RoutineName + CurrentModuleObject + "=\"" + AlphaName(1) + "\", invalid " +
                                                    cNumericFieldNames(3) + ", value  [<0.0]=" + RoundSigDigits(IHGNumbers(3), 3));
                                    ErrorsFound = true;
                                }
                            }
                            if (lNumericFieldBlanks(3)) {
                                ShowWarningError(RoutineName + CurrentModuleObject + "=\"" + AlphaName(1) + "\", specifies " + cNumericFieldNames(3) +
                                                 ", but that field is blank.  0 Electric Equipment will result.");
                            }

                        } else {
                            if (Item1 == 1) {
                                ShowSevereError(RoutineName + CurrentModuleObject + "=\"" + AlphaName(1) + "\", invalid " + cAlphaFieldNames(4) +
                                                ", value  =" + AlphaName(4));
                                ShowContinueError("...Valid values are \"EquipmentLevel\", \"Watts/Area\", \"Watts/Person\".");
                                ErrorsFound = true;
                            }
                        }
                    }

                    // Calculate nominal min/max equipment level
                    ZoneElectric(Loop).NomMinDesignLevel = ZoneElectric(Loop).DesignLevel * SchMin;
                    ZoneElectric(Loop).NomMaxDesignLevel = ZoneElectric(Loop).DesignLevel * SchMax;

                    ZoneElectric(Loop).FractionLatent = IHGNumbers(4);
                    ZoneElectric(Loop).FractionRadiant = IHGNumbers(5);
                    ZoneElectric(Loop).FractionLost = IHGNumbers(6);
                    // FractionConvected is a calculated field
                    ZoneElectric(Loop).FractionConvected =
                        1.0 - (ZoneElectric(Loop).FractionLatent + ZoneElectric(Loop).FractionRadiant + ZoneElectric(Loop).FractionLost);
                    if (std::abs(ZoneElectric(Loop).FractionConvected) <= 0.001) ZoneElectric(Loop).FractionConvected = 0.0;
                    if (ZoneElectric(Loop).FractionConvected < 0.0) {
                        ShowSevereError(RoutineName + CurrentModuleObject + "=\"" + AlphaName(1) + "\", Sum of Fractions > 1.0");
                        ErrorsFound = true;
                    }

                    if (NumAlpha > 4) {
                        ZoneElectric(Loop).EndUseSubcategory = AlphaName(5);
                    } else {
                        ZoneElectric(Loop).EndUseSubcategory = "General";
                    }

                    if (ZoneElectric(Loop).ZonePtr <= 0) continue; // Error, will be caught and terminated later

                    // Object report variables
                    SetupOutputVariable(state, "Electric Equipment Electricity Rate",
                                        OutputProcessor::Unit::W,
                                        ZoneElectric(Loop).Power,
                                        "Zone",
                                        "Average",
                                        ZoneElectric(Loop).Name);
                    SetupOutputVariable(state, "Electric Equipment Electricity Energy",
                                        OutputProcessor::Unit::J,
                                        ZoneElectric(Loop).Consumption,
                                        "Zone",
                                        "Sum",
                                        ZoneElectric(Loop).Name,
                                        _,
                                        "Electricity",
                                        "InteriorEquipment",
                                        ZoneElectric(Loop).EndUseSubcategory,
                                        "Building",
                                        Zone(ZoneElectric(Loop).ZonePtr).Name,
                                        Zone(ZoneElectric(Loop).ZonePtr).Multiplier,
                                        Zone(ZoneElectric(Loop).ZonePtr).ListMultiplier);

                    SetupOutputVariable(state, "Electric Equipment Radiant Heating Energy",
                                        OutputProcessor::Unit::J,
                                        ZoneElectric(Loop).RadGainEnergy,
                                        "Zone",
                                        "Sum",
                                        ZoneElectric(Loop).Name);
                    SetupOutputVariable(state, "Electric Equipment Radiant Heating Rate",
                                        OutputProcessor::Unit::W,
                                        ZoneElectric(Loop).RadGainRate,
                                        "Zone",
                                        "Average",
                                        ZoneElectric(Loop).Name);
                    SetupOutputVariable(state, "Electric Equipment Convective Heating Energy",
                                        OutputProcessor::Unit::J,
                                        ZoneElectric(Loop).ConGainEnergy,
                                        "Zone",
                                        "Sum",
                                        ZoneElectric(Loop).Name);
                    SetupOutputVariable(state, "Electric Equipment Convective Heating Rate",
                                        OutputProcessor::Unit::W,
                                        ZoneElectric(Loop).ConGainRate,
                                        "Zone",
                                        "Average",
                                        ZoneElectric(Loop).Name);
                    SetupOutputVariable(state, "Electric Equipment Latent Gain Energy",
                                        OutputProcessor::Unit::J,
                                        ZoneElectric(Loop).LatGainEnergy,
                                        "Zone",
                                        "Sum",
                                        ZoneElectric(Loop).Name);
                    SetupOutputVariable(state, "Electric Equipment Latent Gain Rate",
                                        OutputProcessor::Unit::W,
                                        ZoneElectric(Loop).LatGainRate,
                                        "Zone",
                                        "Average",
                                        ZoneElectric(Loop).Name);
                    SetupOutputVariable(state, "Electric Equipment Lost Heat Energy",
                                        OutputProcessor::Unit::J,
                                        ZoneElectric(Loop).LostEnergy,
                                        "Zone",
                                        "Sum",
                                        ZoneElectric(Loop).Name);
                    SetupOutputVariable(state, "Electric Equipment Lost Heat Rate",
                                        OutputProcessor::Unit::W,
                                        ZoneElectric(Loop).LostRate,
                                        "Zone",
                                        "Average",
                                        ZoneElectric(Loop).Name);
                    SetupOutputVariable(state, "Electric Equipment Total Heating Energy",
                                        OutputProcessor::Unit::J,
                                        ZoneElectric(Loop).TotGainEnergy,
                                        "Zone",
                                        "Sum",
                                        ZoneElectric(Loop).Name);
                    SetupOutputVariable(state, "Electric Equipment Total Heating Rate",
                                        OutputProcessor::Unit::W,
                                        ZoneElectric(Loop).TotGainRate,
                                        "Zone",
                                        "Average",
                                        ZoneElectric(Loop).Name);

                    // Zone total report variables
                    if (RepVarSet(ZoneElectric(Loop).ZonePtr)) {
                        RepVarSet(ZoneElectric(Loop).ZonePtr) = false;
                        SetupOutputVariable(state, "Zone Electric Equipment Electricity Rate",
                                            OutputProcessor::Unit::W,
                                            ZnRpt(ZoneElectric(Loop).ZonePtr).ElecPower,
                                            "Zone",
                                            "Average",
                                            Zone(ZoneElectric(Loop).ZonePtr).Name);
                        SetupOutputVariable(state, "Zone Electric Equipment Electricity Energy",
                                            OutputProcessor::Unit::J,
                                            ZnRpt(ZoneElectric(Loop).ZonePtr).ElecConsump,
                                            "Zone",
                                            "Sum",
                                            Zone(ZoneElectric(Loop).ZonePtr).Name);

                        SetupOutputVariable(state, "Zone Electric Equipment Radiant Heating Energy",
                                            OutputProcessor::Unit::J,
                                            ZnRpt(ZoneElectric(Loop).ZonePtr).ElecRadGain,
                                            "Zone",
                                            "Sum",
                                            Zone(ZoneElectric(Loop).ZonePtr).Name);
                        SetupOutputVariable(state, "Zone Electric Equipment Radiant Heating Rate",
                                            OutputProcessor::Unit::W,
                                            ZnRpt(ZoneElectric(Loop).ZonePtr).ElecRadGainRate,
                                            "Zone",
                                            "Average",
                                            Zone(ZoneElectric(Loop).ZonePtr).Name);
                        SetupOutputVariable(state, "Zone Electric Equipment Convective Heating Energy",
                                            OutputProcessor::Unit::J,
                                            ZnRpt(ZoneElectric(Loop).ZonePtr).ElecConGain,
                                            "Zone",
                                            "Sum",
                                            Zone(ZoneElectric(Loop).ZonePtr).Name);
                        SetupOutputVariable(state, "Zone Electric Equipment Convective Heating Rate",
                                            OutputProcessor::Unit::W,
                                            ZnRpt(ZoneElectric(Loop).ZonePtr).ElecConGainRate,
                                            "Zone",
                                            "Average",
                                            Zone(ZoneElectric(Loop).ZonePtr).Name);
                        SetupOutputVariable(state, "Zone Electric Equipment Latent Gain Energy",
                                            OutputProcessor::Unit::J,
                                            ZnRpt(ZoneElectric(Loop).ZonePtr).ElecLatGain,
                                            "Zone",
                                            "Sum",
                                            Zone(ZoneElectric(Loop).ZonePtr).Name);
                        SetupOutputVariable(state, "Zone Electric Equipment Latent Gain Rate",
                                            OutputProcessor::Unit::W,
                                            ZnRpt(ZoneElectric(Loop).ZonePtr).ElecLatGainRate,
                                            "Zone",
                                            "Average",
                                            Zone(ZoneElectric(Loop).ZonePtr).Name);
                        SetupOutputVariable(state, "Zone Electric Equipment Lost Heat Energy",
                                            OutputProcessor::Unit::J,
                                            ZnRpt(ZoneElectric(Loop).ZonePtr).ElecLost,
                                            "Zone",
                                            "Sum",
                                            Zone(ZoneElectric(Loop).ZonePtr).Name);
                        SetupOutputVariable(state, "Zone Electric Equipment Lost Heat Rate",
                                            OutputProcessor::Unit::W,
                                            ZnRpt(ZoneElectric(Loop).ZonePtr).ElecLostRate,
                                            "Zone",
                                            "Average",
                                            Zone(ZoneElectric(Loop).ZonePtr).Name);
                        SetupOutputVariable(state, "Zone Electric Equipment Total Heating Energy",
                                            OutputProcessor::Unit::J,
                                            ZnRpt(ZoneElectric(Loop).ZonePtr).ElecTotGain,
                                            "Zone",
                                            "Sum",
                                            Zone(ZoneElectric(Loop).ZonePtr).Name);
                        SetupOutputVariable(state, "Zone Electric Equipment Total Heating Rate",
                                            OutputProcessor::Unit::W,
                                            ZnRpt(ZoneElectric(Loop).ZonePtr).ElecTotGainRate,
                                            "Zone",
                                            "Average",
                                            Zone(ZoneElectric(Loop).ZonePtr).Name);
                    }

                    if (AnyEnergyManagementSystemInModel) {
                        SetupEMSActuator("ElectricEquipment",
                                         ZoneElectric(Loop).Name,
                                         "Electricity Rate",
                                         "[W]",
                                         ZoneElectric(Loop).EMSZoneEquipOverrideOn,
                                         ZoneElectric(Loop).EMSEquipPower);
                        SetupEMSInternalVariable(
                            "Plug and Process Power Design Level", ZoneElectric(Loop).Name, "[W]", ZoneElectric(Loop).DesignLevel);
                    } // EMS

                    if (!ErrorsFound)
                        SetupZoneInternalGain(ZoneElectric(Loop).ZonePtr,
                                              "ElectricEquipment",
                                              ZoneElectric(Loop).Name,
                                              IntGainTypeOf_ElectricEquipment,
                                              &ZoneElectric(Loop).ConGainRate,
                                              nullptr,
                                              &ZoneElectric(Loop).RadGainRate,
                                              &ZoneElectric(Loop).LatGainRate);

                } // Item1
            }     // Item - Number of ZoneElectric objects
        }         // Check on number of ZoneElectric

        RepVarSet = true;
        CurrentModuleObject = "GasEquipment";
        NumZoneGasStatements = inputProcessor->getNumObjectsFound(CurrentModuleObject);
        ZoneGasObjects.allocate(NumZoneGasStatements);

        TotGasEquip = 0;
        errFlag = false;
        for (Item = 1; Item <= NumZoneGasStatements; ++Item) {
            inputProcessor->getObjectItem(state,
                                          CurrentModuleObject,
                                          Item,
                                          AlphaName,
                                          NumAlpha,
                                          IHGNumbers,
                                          NumNumber,
                                          IOStat,
                                          lNumericFieldBlanks,
                                          lAlphaFieldBlanks,
                                          cAlphaFieldNames,
                                          cNumericFieldNames);
            UtilityRoutines::IsNameEmpty(AlphaName(1), CurrentModuleObject, ErrorsFound);
            errFlag = ErrorsFound;

            ZoneGasObjects(Item).Name = AlphaName(1);

            Item1 = UtilityRoutines::FindItemInList(AlphaName(2), Zone);
            ZLItem = 0;
            if (Item1 == 0 && NumOfZoneLists > 0) ZLItem = UtilityRoutines::FindItemInList(AlphaName(2), ZoneList);
            if (Item1 > 0) {
                ZoneGasObjects(Item).StartPtr = TotGasEquip + 1;
                ++TotGasEquip;
                ZoneGasObjects(Item).NumOfZones = 1;
                ZoneGasObjects(Item).ZoneListActive = false;
                ZoneGasObjects(Item).ZoneOrZoneListPtr = Item1;
            } else if (ZLItem > 0) {
                ZoneGasObjects(Item).StartPtr = TotGasEquip + 1;
                TotGasEquip += ZoneList(ZLItem).NumOfZones;
                ZoneGasObjects(Item).NumOfZones = ZoneList(ZLItem).NumOfZones;
                ZoneGasObjects(Item).ZoneListActive = true;
                ZoneGasObjects(Item).ZoneOrZoneListPtr = ZLItem;
            } else {
                ShowSevereError(CurrentModuleObject + "=\"" + AlphaName(1) + "\" invalid " + cAlphaFieldNames(2) + "=\"" + AlphaName(2) +
                                "\" not found.");
                ErrorsFound = true;
                errFlag = true;
            }
        }

        if (errFlag) {
            ShowSevereError(RoutineName + "Errors with invalid names in " + CurrentModuleObject + " objects.");
            ShowContinueError("...These will not be read in.  Other errors may occur.");
            TotGasEquip = 0;
        }

        ZoneGas.allocate(TotGasEquip);

        if (TotGasEquip > 0) {
            Loop = 0;
            for (Item = 1; Item <= NumZoneGasStatements; ++Item) {
                AlphaName = BlankString;
                IHGNumbers = 0.0;

                inputProcessor->getObjectItem(state,
                                              CurrentModuleObject,
                                              Item,
                                              AlphaName,
                                              NumAlpha,
                                              IHGNumbers,
                                              NumNumber,
                                              IOStat,
                                              lNumericFieldBlanks,
                                              lAlphaFieldBlanks,
                                              cAlphaFieldNames,
                                              cNumericFieldNames);

                for (Item1 = 1; Item1 <= ZoneGasObjects(Item).NumOfZones; ++Item1) {
                    ++Loop;
                    if (!ZoneGasObjects(Item).ZoneListActive) {
                        ZoneGas(Loop).Name = AlphaName(1);
                        ZoneGas(Loop).ZonePtr = ZoneGasObjects(Item).ZoneOrZoneListPtr;
                    } else {
                        CheckCreatedZoneItemName(RoutineName,
                                                 CurrentModuleObject,
                                                 Zone(ZoneList(ZoneGasObjects(Item).ZoneOrZoneListPtr).Zone(Item1)).Name,
                                                 ZoneList(ZoneGasObjects(Item).ZoneOrZoneListPtr).MaxZoneNameLength,
                                                 ZoneGasObjects(Item).Name,
                                                 ZoneGas,
                                                 Loop - 1,
                                                 ZoneGas(Loop).Name,
                                                 errFlag);
                        ZoneGas(Loop).ZonePtr = ZoneList(ZoneGasObjects(Item).ZoneOrZoneListPtr).Zone(Item1);
                        if (errFlag) ErrorsFound = true;
                    }

                    ZoneGas(Loop).SchedPtr = GetScheduleIndex(state, AlphaName(3));
                    SchMin = 0.0;
                    SchMax = 0.0;
                    if (ZoneGas(Loop).SchedPtr == 0) {
                        if (Item1 == 1) {
                            if (lAlphaFieldBlanks(3)) {
                                ShowSevereError(RoutineName + CurrentModuleObject + "=\"" + AlphaName(1) + "\", " + cAlphaFieldNames(3) +
                                                " is required.");
                            } else {
                                ShowSevereError(RoutineName + CurrentModuleObject + "=\"" + AlphaName(1) + "\", invalid " + cAlphaFieldNames(3) +
                                                " entered=" + AlphaName(3));
                            }
                            ErrorsFound = true;
                        }
                    } else { // check min/max on schedule
                        SchMin = GetScheduleMinValue(ZoneGas(Loop).SchedPtr);
                        SchMax = GetScheduleMaxValue(ZoneGas(Loop).SchedPtr);
                        if (SchMin < 0.0 || SchMax < 0.0) {
                            if (Item1 == 1) {
                                if (SchMin < 0.0) {
                                    ShowSevereError(RoutineName + CurrentModuleObject + "=\"" + AlphaName(1) + "\", " + cAlphaFieldNames(3) +
                                                    ", minimum is < 0.0");
                                    ShowContinueError("Schedule=\"" + AlphaName(3) + "\". Minimum is [" + RoundSigDigits(SchMin, 1) +
                                                      "]. Values must be >= 0.0.");
                                    ErrorsFound = true;
                                }
                            }
                            if (Item1 == 1) {
                                if (SchMax < 0.0) {
                                    ShowSevereError(RoutineName + CurrentModuleObject + "=\"" + AlphaName(1) + "\", " + cAlphaFieldNames(3) +
                                                    ", maximum is < 0.0");
                                    ShowContinueError("Schedule=\"" + AlphaName(3) + "\". Maximum is [" + RoundSigDigits(SchMax, 1) +
                                                      "]. Values must be >= 0.0.");
                                    ErrorsFound = true;
                                }
                            }
                        }
                    }

                    // equipment design level calculation method.
                    {
                        auto const equipmentLevel(AlphaName(4));
                        if (equipmentLevel == "EQUIPMENTLEVEL") {
                            ZoneGas(Loop).DesignLevel = IHGNumbers(1);
                            if (lNumericFieldBlanks(1)) {
                                ShowWarningError(RoutineName + CurrentModuleObject + "=\"" + ZoneGas(Loop).Name + "\", specifies " +
                                                 cNumericFieldNames(1) + ", but that field is blank.  0 Gas Equipment will result.");
                            }

                        } else if (equipmentLevel == "WATTS/AREA" || equipmentLevel == "POWER/AREA") {
                            if (ZoneGas(Loop).ZonePtr != 0) {
                                if (IHGNumbers(2) >= 0.0) {
                                    ZoneGas(Loop).DesignLevel = IHGNumbers(2) * Zone(ZoneGas(Loop).ZonePtr).FloorArea;
                                    if (Zone(ZoneGas(Loop).ZonePtr).FloorArea <= 0.0) {
                                        ShowWarningError(RoutineName + CurrentModuleObject + "=\"" + ZoneGas(Loop).Name + "\", specifies " +
                                                         cNumericFieldNames(2) + ", but Zone Floor Area = 0.  0 Gas Equipment will result.");
                                    }
                                } else {
                                    ShowSevereError(RoutineName + CurrentModuleObject + "=\"" + ZoneGas(Loop).Name + "\", invalid " +
                                                    cNumericFieldNames(2) + ", value  [<0.0]=" + RoundSigDigits(IHGNumbers(2), 3));
                                    ErrorsFound = true;
                                }
                            }
                            if (lNumericFieldBlanks(2)) {
                                ShowWarningError(RoutineName + CurrentModuleObject + "=\"" + ZoneGas(Loop).Name + "\", specifies " +
                                                 cNumericFieldNames(2) + ", but that field is blank.  0 Gas Equipment will result.");
                            }

                        } else if (equipmentLevel == "WATTS/PERSON" || equipmentLevel == "POWER/PERSON") {
                            if (ZoneGas(Loop).ZonePtr != 0) {
                                if (IHGNumbers(3) >= 0.0) {
                                    ZoneGas(Loop).DesignLevel = IHGNumbers(3) * Zone(ZoneGas(Loop).ZonePtr).TotOccupants;
                                    if (Zone(ZoneGas(Loop).ZonePtr).TotOccupants <= 0.0) {
                                        ShowWarningError(RoutineName + CurrentModuleObject + "=\"" + ZoneGas(Loop).Name + "\", specifies " +
                                                         cNumericFieldNames(2) + ", but Total Occupants = 0.  0 Gas Equipment will result.");
                                    }
                                } else {
                                    ShowSevereError(RoutineName + CurrentModuleObject + "=\"" + ZoneGas(Loop).Name + "\", invalid " +
                                                    cNumericFieldNames(3) + ", value  [<0.0]=" + RoundSigDigits(IHGNumbers(3), 3));
                                    ErrorsFound = true;
                                }
                            }
                            if (lNumericFieldBlanks(3)) {
                                ShowWarningError(RoutineName + CurrentModuleObject + "=\"" + ZoneGas(Loop).Name + "\", specifies " +
                                                 cNumericFieldNames(3) + ", but that field is blank.  0 Gas Equipment will result.");
                            }

                        } else {
                            if (Item1 == 1) {
                                ShowSevereError(RoutineName + CurrentModuleObject + "=\"" + AlphaName(1) + "\", invalid " + cAlphaFieldNames(4) +
                                                ", value  =" + AlphaName(4));
                                ShowContinueError("...Valid values are \"EquipmentLevel\", \"Watts/Area\", \"Watts/Person\".");
                                ErrorsFound = true;
                            }
                        }
                    }

                    // Calculate nominal min/max equipment level
                    ZoneGas(Loop).NomMinDesignLevel = ZoneGas(Loop).DesignLevel * SchMin;
                    ZoneGas(Loop).NomMaxDesignLevel = ZoneGas(Loop).DesignLevel * SchMax;

                    ZoneGas(Loop).FractionLatent = IHGNumbers(4);
                    ZoneGas(Loop).FractionRadiant = IHGNumbers(5);
                    ZoneGas(Loop).FractionLost = IHGNumbers(6);

                    if ((NumNumber == 7) || (!lNumericFieldBlanks(7))) {
                        ZoneGas(Loop).CO2RateFactor = IHGNumbers(7);
                    }
                    if (ZoneGas(Loop).CO2RateFactor < 0.0) {
                        ShowSevereError(RoutineName + CurrentModuleObject + "=\"" + AlphaName(1) + "\", " + cNumericFieldNames(7) +
                                        " < 0.0, value =" + RoundSigDigits(IHGNumbers(7), 2));
                        ErrorsFound = true;
                    }
                    if (ZoneGas(Loop).CO2RateFactor > 4.0e-7) {
                        ShowSevereError(RoutineName + CurrentModuleObject + "=\"" + AlphaName(1) + "\", " + cNumericFieldNames(7) +
                                        " > 4.0E-7, value =" + RoundSigDigits(IHGNumbers(7), 2));
                        ErrorsFound = true;
                    }
                    // FractionConvected is a calculated field
                    ZoneGas(Loop).FractionConvected =
                        1.0 - (ZoneGas(Loop).FractionLatent + ZoneGas(Loop).FractionRadiant + ZoneGas(Loop).FractionLost);
                    if (std::abs(ZoneGas(Loop).FractionConvected) <= 0.001) ZoneGas(Loop).FractionConvected = 0.0;
                    if (ZoneGas(Loop).FractionConvected < 0.0) {
                        if (Item1 == 1) {
                            ShowSevereError(RoutineName + CurrentModuleObject + "=\"" + AlphaName(1) + "\", Sum of Fractions > 1.0");
                            ErrorsFound = true;
                        }
                    }

                    if (NumAlpha > 4) {
                        ZoneGas(Loop).EndUseSubcategory = AlphaName(5);
                    } else {
                        ZoneGas(Loop).EndUseSubcategory = "General";
                    }

                    if (ZoneGas(Loop).ZonePtr <= 0) continue; // Error, will be caught and terminated later

                    // Object report variables
                    SetupOutputVariable(state,
                        "Gas Equipment NaturalGas Rate", OutputProcessor::Unit::W, ZoneGas(Loop).Power, "Zone", "Average", ZoneGas(Loop).Name);
                    SetupOutputVariable(state, "Gas Equipment NaturalGas Energy",
                                        OutputProcessor::Unit::J,
                                        ZoneGas(Loop).Consumption,
                                        "Zone",
                                        "Sum",
                                        ZoneGas(Loop).Name,
                                        _,
                                        "NaturalGas",
                                        "InteriorEquipment",
                                        ZoneGas(Loop).EndUseSubcategory,
                                        "Building",
                                        Zone(ZoneGas(Loop).ZonePtr).Name,
                                        Zone(ZoneGas(Loop).ZonePtr).Multiplier,
                                        Zone(ZoneGas(Loop).ZonePtr).ListMultiplier);

                    SetupOutputVariable(state, "Gas Equipment Radiant Heating Energy",
                                        OutputProcessor::Unit::J,
                                        ZoneGas(Loop).RadGainEnergy,
                                        "Zone",
                                        "Sum",
                                        ZoneGas(Loop).Name);
                    SetupOutputVariable(state, "Gas Equipment Convective Heating Energy",
                                        OutputProcessor::Unit::J,
                                        ZoneGas(Loop).ConGainEnergy,
                                        "Zone",
                                        "Sum",
                                        ZoneGas(Loop).Name);
                    SetupOutputVariable(state,
                        "Gas Equipment Latent Gain Energy", OutputProcessor::Unit::J, ZoneGas(Loop).LatGainEnergy, "Zone", "Sum", ZoneGas(Loop).Name);
                    SetupOutputVariable(state,
                        "Gas Equipment Lost Heat Energy", OutputProcessor::Unit::J, ZoneGas(Loop).LostEnergy, "Zone", "Sum", ZoneGas(Loop).Name);
                    SetupOutputVariable(state, "Gas Equipment Total Heating Energy",
                                        OutputProcessor::Unit::J,
                                        ZoneGas(Loop).TotGainEnergy,
                                        "Zone",
                                        "Sum",
                                        ZoneGas(Loop).Name);
                    SetupOutputVariable(state, "Gas Equipment Radiant Heating Rate",
                                        OutputProcessor::Unit::W,
                                        ZoneGas(Loop).RadGainRate,
                                        "Zone",
                                        "Average",
                                        ZoneGas(Loop).Name);
                    SetupOutputVariable(state, "Gas Equipment Convective Heating Rate",
                                        OutputProcessor::Unit::W,
                                        ZoneGas(Loop).ConGainRate,
                                        "Zone",
                                        "Average",
                                        ZoneGas(Loop).Name);
                    SetupOutputVariable(state,
                        "Gas Equipment Latent Gain Rate", OutputProcessor::Unit::W, ZoneGas(Loop).LatGainRate, "Zone", "Average", ZoneGas(Loop).Name);
                    SetupOutputVariable(state,
                        "Gas Equipment Lost Heat Rate", OutputProcessor::Unit::W, ZoneGas(Loop).LostRate, "Zone", "Average", ZoneGas(Loop).Name);
                    SetupOutputVariable(state, "Gas Equipment Total Heating Rate",
                                        OutputProcessor::Unit::W,
                                        ZoneGas(Loop).TotGainRate,
                                        "Zone",
                                        "Average",
                                        ZoneGas(Loop).Name);

                    // Zone total report variables
                    if (RepVarSet(ZoneGas(Loop).ZonePtr)) {
                        RepVarSet(ZoneGas(Loop).ZonePtr) = false;

                        SetupOutputVariable(state, "Zone Gas Equipment NaturalGas Rate",
                                            OutputProcessor::Unit::W,
                                            ZnRpt(ZoneGas(Loop).ZonePtr).GasPower,
                                            "Zone",
                                            "Average",
                                            Zone(ZoneGas(Loop).ZonePtr).Name);
                        SetupOutputVariable(state, "Zone Gas Equipment NaturalGas Energy",
                                            OutputProcessor::Unit::J,
                                            ZnRpt(ZoneGas(Loop).ZonePtr).GasConsump,
                                            "Zone",
                                            "Sum",
                                            Zone(ZoneGas(Loop).ZonePtr).Name);

                        SetupOutputVariable(state, "Zone Gas Equipment Radiant Heating Energy",
                                            OutputProcessor::Unit::J,
                                            ZnRpt(ZoneGas(Loop).ZonePtr).GasRadGain,
                                            "Zone",
                                            "Sum",
                                            Zone(ZoneGas(Loop).ZonePtr).Name);
                        SetupOutputVariable(state, "Zone Gas Equipment Radiant Heating Rate",
                                            OutputProcessor::Unit::W,
                                            ZnRpt(ZoneGas(Loop).ZonePtr).GasRadGainRate,
                                            "Zone",
                                            "Average",
                                            Zone(ZoneGas(Loop).ZonePtr).Name);
                        SetupOutputVariable(state, "Zone Gas Equipment Convective Heating Energy",
                                            OutputProcessor::Unit::J,
                                            ZnRpt(ZoneGas(Loop).ZonePtr).GasConGain,
                                            "Zone",
                                            "Sum",
                                            Zone(ZoneGas(Loop).ZonePtr).Name);
                        SetupOutputVariable(state, "Zone Gas Equipment Convective Heating Rate",
                                            OutputProcessor::Unit::W,
                                            ZnRpt(ZoneGas(Loop).ZonePtr).GasConGainRate,
                                            "Zone",
                                            "Average",
                                            Zone(ZoneGas(Loop).ZonePtr).Name);
                        SetupOutputVariable(state, "Zone Gas Equipment Latent Gain Energy",
                                            OutputProcessor::Unit::J,
                                            ZnRpt(ZoneGas(Loop).ZonePtr).GasLatGain,
                                            "Zone",
                                            "Sum",
                                            Zone(ZoneGas(Loop).ZonePtr).Name);
                        SetupOutputVariable(state, "Zone Gas Equipment Latent Gain Rate",
                                            OutputProcessor::Unit::W,
                                            ZnRpt(ZoneGas(Loop).ZonePtr).GasLatGainRate,
                                            "Zone",
                                            "Average",
                                            Zone(ZoneGas(Loop).ZonePtr).Name);
                        SetupOutputVariable(state, "Zone Gas Equipment Lost Heat Energy",
                                            OutputProcessor::Unit::J,
                                            ZnRpt(ZoneGas(Loop).ZonePtr).GasLost,
                                            "Zone",
                                            "Sum",
                                            Zone(ZoneGas(Loop).ZonePtr).Name);
                        SetupOutputVariable(state, "Zone Gas Equipment Lost Heat Rate",
                                            OutputProcessor::Unit::W,
                                            ZnRpt(ZoneGas(Loop).ZonePtr).GasLostRate,
                                            "Zone",
                                            "Average",
                                            Zone(ZoneGas(Loop).ZonePtr).Name);
                        SetupOutputVariable(state, "Zone Gas Equipment Total Heating Energy",
                                            OutputProcessor::Unit::J,
                                            ZnRpt(ZoneGas(Loop).ZonePtr).GasTotGain,
                                            "Zone",
                                            "Sum",
                                            Zone(ZoneGas(Loop).ZonePtr).Name);
                        SetupOutputVariable(state, "Zone Gas Equipment Total Heating Rate",
                                            OutputProcessor::Unit::W,
                                            ZnRpt(ZoneGas(Loop).ZonePtr).GasTotGainRate,
                                            "Zone",
                                            "Average",
                                            Zone(ZoneGas(Loop).ZonePtr).Name);
                    }

                    if (AnyEnergyManagementSystemInModel) {
                        SetupEMSActuator("GasEquipment",
                                         ZoneGas(Loop).Name,
                                         "NaturalGas Rate",
                                         "[W]",
                                         ZoneGas(Loop).EMSZoneEquipOverrideOn,
                                         ZoneGas(Loop).EMSEquipPower);
                        SetupEMSInternalVariable("Gas Process Power Design Level", ZoneGas(Loop).Name, "[W]", ZoneGas(Loop).DesignLevel);
                    } // EMS

                    if (!ErrorsFound)
                        SetupZoneInternalGain(ZoneGas(Loop).ZonePtr,
                                              "GasEquipment",
                                              ZoneGas(Loop).Name,
                                              IntGainTypeOf_GasEquipment,
                                              &ZoneGas(Loop).ConGainRate,
                                              nullptr,
                                              &ZoneGas(Loop).RadGainRate,
                                              &ZoneGas(Loop).LatGainRate,
                                              nullptr,
                                              &ZoneGas(Loop).CO2GainRate);

                } // Item1
            }     // Item - number of gas statements
        }         // check for number of gas statements

        RepVarSet = true;
        CurrentModuleObject = "HotWaterEquipment";
        NumHotWaterEqStatements = inputProcessor->getNumObjectsFound(CurrentModuleObject);
        HotWaterEqObjects.allocate(NumHotWaterEqStatements);

        TotHWEquip = 0;
        errFlag = false;
        for (Item = 1; Item <= NumHotWaterEqStatements; ++Item) {
            inputProcessor->getObjectItem(state,
                                          CurrentModuleObject,
                                          Item,
                                          AlphaName,
                                          NumAlpha,
                                          IHGNumbers,
                                          NumNumber,
                                          IOStat,
                                          lNumericFieldBlanks,
                                          lAlphaFieldBlanks,
                                          cAlphaFieldNames,
                                          cNumericFieldNames);
            UtilityRoutines::IsNameEmpty(AlphaName(1), CurrentModuleObject, ErrorsFound);
            errFlag = ErrorsFound;

            HotWaterEqObjects(Item).Name = AlphaName(1);

            Item1 = UtilityRoutines::FindItemInList(AlphaName(2), Zone);
            ZLItem = 0;
            if (Item1 == 0 && NumOfZoneLists > 0) ZLItem = UtilityRoutines::FindItemInList(AlphaName(2), ZoneList);
            if (Item1 > 0) {
                HotWaterEqObjects(Item).StartPtr = TotHWEquip + 1;
                ++TotHWEquip;
                HotWaterEqObjects(Item).NumOfZones = 1;
                HotWaterEqObjects(Item).ZoneListActive = false;
                HotWaterEqObjects(Item).ZoneOrZoneListPtr = Item1;
            } else if (ZLItem > 0) {
                HotWaterEqObjects(Item).StartPtr = TotHWEquip + 1;
                TotHWEquip += ZoneList(ZLItem).NumOfZones;
                HotWaterEqObjects(Item).NumOfZones = ZoneList(ZLItem).NumOfZones;
                HotWaterEqObjects(Item).ZoneListActive = true;
                HotWaterEqObjects(Item).ZoneOrZoneListPtr = ZLItem;
            } else {
                ShowSevereError(CurrentModuleObject + "=\"" + AlphaName(1) + "\" invalid " + cAlphaFieldNames(2) + "=\"" + AlphaName(2) +
                                "\" not found.");
                ErrorsFound = true;
                errFlag = true;
            }
        }

        if (errFlag) {
            ShowSevereError(RoutineName + "Errors with invalid names in " + CurrentModuleObject + " objects.");
            ShowContinueError("...These will not be read in.  Other errors may occur.");
            TotHWEquip = 0;
        }

        ZoneHWEq.allocate(TotHWEquip);

        if (TotHWEquip > 0) {
            Loop = 0;
            for (Item = 1; Item <= NumHotWaterEqStatements; ++Item) {
                AlphaName = BlankString;
                IHGNumbers = 0.0;

                inputProcessor->getObjectItem(state,
                                              CurrentModuleObject,
                                              Item,
                                              AlphaName,
                                              NumAlpha,
                                              IHGNumbers,
                                              NumNumber,
                                              IOStat,
                                              lNumericFieldBlanks,
                                              lAlphaFieldBlanks,
                                              cAlphaFieldNames,
                                              cNumericFieldNames);

                for (Item1 = 1; Item1 <= HotWaterEqObjects(Item).NumOfZones; ++Item1) {
                    ++Loop;
                    if (!HotWaterEqObjects(Item).ZoneListActive) {
                        ZoneHWEq(Loop).Name = AlphaName(1);
                        ZoneHWEq(Loop).ZonePtr = HotWaterEqObjects(Item).ZoneOrZoneListPtr;
                    } else {
                        CheckCreatedZoneItemName(RoutineName,
                                                 CurrentModuleObject,
                                                 Zone(ZoneList(HotWaterEqObjects(Item).ZoneOrZoneListPtr).Zone(Item1)).Name,
                                                 ZoneList(HotWaterEqObjects(Item).ZoneOrZoneListPtr).MaxZoneNameLength,
                                                 HotWaterEqObjects(Item).Name,
                                                 ZoneHWEq,
                                                 Loop - 1,
                                                 ZoneHWEq(Loop).Name,
                                                 errFlag);
                        ZoneHWEq(Loop).ZonePtr = ZoneList(HotWaterEqObjects(Item).ZoneOrZoneListPtr).Zone(Item1);
                        if (errFlag) ErrorsFound = true;
                    }

                    ZoneHWEq(Loop).SchedPtr = GetScheduleIndex(state, AlphaName(3));
                    SchMin = 0.0;
                    SchMax = 0.0;
                    if (ZoneHWEq(Loop).SchedPtr == 0) {
                        if (lAlphaFieldBlanks(3)) {
                            ShowSevereError(RoutineName + CurrentModuleObject + "=\"" + AlphaName(1) + "\", " + cAlphaFieldNames(3) +
                                            " is required.");
                        } else {
                            ShowSevereError(RoutineName + CurrentModuleObject + "=\"" + AlphaName(1) + "\", invalid " + cAlphaFieldNames(3) +
                                            " entered=" + AlphaName(3));
                        }
                        ErrorsFound = true;
                    } else { // check min/max on schedule
                        SchMin = GetScheduleMinValue(ZoneHWEq(Loop).SchedPtr);
                        SchMax = GetScheduleMaxValue(ZoneHWEq(Loop).SchedPtr);
                        if (SchMin < 0.0 || SchMax < 0.0) {
                            if (SchMin < 0.0) {
                                ShowSevereError(RoutineName + CurrentModuleObject + "=\"" + AlphaName(1) + "\", " + cAlphaFieldNames(3) +
                                                ", minimum is < 0.0");
                                ShowContinueError("Schedule=\"" + AlphaName(3) + "\". Minimum is [" + RoundSigDigits(SchMin, 1) +
                                                  "]. Values must be >= 0.0.");
                                ErrorsFound = true;
                            }
                            if (SchMax < 0.0) {
                                ShowSevereError(RoutineName + CurrentModuleObject + "=\"" + AlphaName(1) + "\", " + cAlphaFieldNames(3) +
                                                ", maximum is < 0.0");
                                ShowContinueError("Schedule=\"" + AlphaName(3) + "\". Maximum is [" + RoundSigDigits(SchMax, 1) +
                                                  "]. Values must be >= 0.0.");
                                ErrorsFound = true;
                            }
                        }
                    }

                    // Hot Water equipment design level calculation method.
                    {
                        auto const equipmentLevel(AlphaName(4));
                        if (equipmentLevel == "EQUIPMENTLEVEL") {
                            ZoneHWEq(Loop).DesignLevel = IHGNumbers(1);
                            if (lNumericFieldBlanks(1)) {
                                ShowWarningError(RoutineName + CurrentModuleObject + "=\"" + AlphaName(1) + "\", specifies " + cNumericFieldNames(1) +
                                                 ", but that field is blank.  0 Hot Water Equipment will result.");
                            }

                        } else if (equipmentLevel == "WATTS/AREA" || equipmentLevel == "POWER/AREA") {
                            if (ZoneHWEq(Loop).ZonePtr != 0) {
                                if (IHGNumbers(2) >= 0.0) {
                                    ZoneHWEq(Loop).DesignLevel = IHGNumbers(2) * Zone(ZoneHWEq(Loop).ZonePtr).FloorArea;
                                    if (Zone(ZoneHWEq(Loop).ZonePtr).FloorArea <= 0.0) {
                                        ShowWarningError(RoutineName + CurrentModuleObject + "=\"" + AlphaName(1) + "\", specifies " +
                                                         cNumericFieldNames(2) + ", but Zone Floor Area = 0.  0 Hot Water Equipment will result.");
                                    }
                                } else {
                                    ShowSevereError(RoutineName + CurrentModuleObject + "=\"" + AlphaName(1) + "\", invalid " +
                                                    cNumericFieldNames(2) + ", value  [<0.0]=" + RoundSigDigits(IHGNumbers(2), 3));
                                    ErrorsFound = true;
                                }
                            }
                            if (lNumericFieldBlanks(2)) {
                                ShowWarningError(RoutineName + CurrentModuleObject + "=\"" + AlphaName(1) + "\", specifies " + cNumericFieldNames(2) +
                                                 ", but that field is blank.  0 Hot Water Equipment will result.");
                            }

                        } else if (equipmentLevel == "WATTS/PERSON" || equipmentLevel == "POWER/PERSON") {
                            if (ZoneHWEq(Loop).ZonePtr != 0) {
                                if (IHGNumbers(3) >= 0.0) {
                                    ZoneHWEq(Loop).DesignLevel = IHGNumbers(3) * Zone(ZoneHWEq(Loop).ZonePtr).TotOccupants;
                                    if (Zone(ZoneHWEq(Loop).ZonePtr).TotOccupants <= 0.0) {
                                        ShowWarningError(RoutineName + CurrentModuleObject + "=\"" + AlphaName(1) + "\", specifies " +
                                                         cNumericFieldNames(2) + ", but Total Occupants = 0.  0 Hot Water Equipment will result.");
                                    }
                                } else {
                                    ShowSevereError(RoutineName + CurrentModuleObject + "=\"" + AlphaName(1) + "\", invalid " +
                                                    cNumericFieldNames(3) + ", value  [<0.0]=" + RoundSigDigits(IHGNumbers(3), 3));
                                    ErrorsFound = true;
                                }
                            }
                            if (lNumericFieldBlanks(3)) {
                                ShowWarningError(RoutineName + CurrentModuleObject + "=\"" + AlphaName(1) + "\", specifies " + cNumericFieldNames(3) +
                                                 ", but that field is blank.  0 Hot Water Equipment will result.");
                            }

                        } else {
                            if (Item1 == 1) {
                                ShowSevereError(RoutineName + CurrentModuleObject + "=\"" + AlphaName(1) + "\", invalid " + cAlphaFieldNames(4) +
                                                ", value  =" + AlphaName(4));
                                ShowContinueError("...Valid values are \"EquipmentLevel\", \"Watts/Area\", \"Watts/Person\".");
                                ErrorsFound = true;
                            }
                        }
                    }

                    // Calculate nominal min/max equipment level
                    ZoneHWEq(Loop).NomMinDesignLevel = ZoneHWEq(Loop).DesignLevel * SchMin;
                    ZoneHWEq(Loop).NomMaxDesignLevel = ZoneHWEq(Loop).DesignLevel * SchMax;

                    ZoneHWEq(Loop).FractionLatent = IHGNumbers(4);
                    ZoneHWEq(Loop).FractionRadiant = IHGNumbers(5);
                    ZoneHWEq(Loop).FractionLost = IHGNumbers(6);
                    // FractionConvected is a calculated field
                    ZoneHWEq(Loop).FractionConvected =
                        1.0 - (ZoneHWEq(Loop).FractionLatent + ZoneHWEq(Loop).FractionRadiant + ZoneHWEq(Loop).FractionLost);
                    if (std::abs(ZoneHWEq(Loop).FractionConvected) <= 0.001) ZoneHWEq(Loop).FractionConvected = 0.0;
                    if (ZoneHWEq(Loop).FractionConvected < 0.0) {
                        ShowSevereError(RoutineName + CurrentModuleObject + "=\"" + AlphaName(1) + "\", Sum of Fractions > 1.0");
                        ErrorsFound = true;
                    }

                    if (NumAlpha > 4) {
                        ZoneHWEq(Loop).EndUseSubcategory = AlphaName(5);
                    } else {
                        ZoneHWEq(Loop).EndUseSubcategory = "General";
                    }

                    if (ZoneHWEq(Loop).ZonePtr <= 0) continue; // Error, will be caught and terminated later

                    // Object report variables
                    SetupOutputVariable(state, "Hot Water Equipment District Heating Rate",
                                        OutputProcessor::Unit::W,
                                        ZoneHWEq(Loop).Power,
                                        "Zone",
                                        "Average",
                                        ZoneHWEq(Loop).Name);
                    SetupOutputVariable(state, "Hot Water Equipment District Heating Energy",
                                        OutputProcessor::Unit::J,
                                        ZoneHWEq(Loop).Consumption,
                                        "Zone",
                                        "Sum",
                                        ZoneHWEq(Loop).Name,
                                        _,
                                        "DistrictHeating",
                                        "InteriorEquipment",
                                        ZoneHWEq(Loop).EndUseSubcategory,
                                        "Building",
                                        Zone(ZoneHWEq(Loop).ZonePtr).Name,
                                        Zone(ZoneHWEq(Loop).ZonePtr).Multiplier,
                                        Zone(ZoneHWEq(Loop).ZonePtr).ListMultiplier);

                    SetupOutputVariable(state, "Hot Water Equipment Radiant Heating Energy",
                                        OutputProcessor::Unit::J,
                                        ZoneHWEq(Loop).RadGainEnergy,
                                        "Zone",
                                        "Sum",
                                        ZoneHWEq(Loop).Name);
                    SetupOutputVariable(state, "Hot Water Equipment Radiant Heating Rate",
                                        OutputProcessor::Unit::W,
                                        ZoneHWEq(Loop).RadGainRate,
                                        "Zone",
                                        "Average",
                                        ZoneHWEq(Loop).Name);
                    SetupOutputVariable(state, "Hot Water Equipment Convective Heating Energy",
                                        OutputProcessor::Unit::J,
                                        ZoneHWEq(Loop).ConGainEnergy,
                                        "Zone",
                                        "Sum",
                                        ZoneHWEq(Loop).Name);
                    SetupOutputVariable(state, "Hot Water Equipment Convective Heating Rate",
                                        OutputProcessor::Unit::W,
                                        ZoneHWEq(Loop).ConGainRate,
                                        "Zone",
                                        "Average",
                                        ZoneHWEq(Loop).Name);
                    SetupOutputVariable(state, "Hot Water Equipment Latent Gain Energy",
                                        OutputProcessor::Unit::J,
                                        ZoneHWEq(Loop).LatGainEnergy,
                                        "Zone",
                                        "Sum",
                                        ZoneHWEq(Loop).Name);
                    SetupOutputVariable(state, "Hot Water Equipment Latent Gain Rate",
                                        OutputProcessor::Unit::W,
                                        ZoneHWEq(Loop).LatGainRate,
                                        "Zone",
                                        "Average",
                                        ZoneHWEq(Loop).Name);
                    SetupOutputVariable(state, "Hot Water Equipment Lost Heat Energy",
                                        OutputProcessor::Unit::J,
                                        ZoneHWEq(Loop).LostEnergy,
                                        "Zone",
                                        "Sum",
                                        ZoneHWEq(Loop).Name);
                    SetupOutputVariable(state, "Hot Water Equipment Lost Heat Rate",
                                        OutputProcessor::Unit::W,
                                        ZoneHWEq(Loop).LostRate,
                                        "Zone",
                                        "Average",
                                        ZoneHWEq(Loop).Name);
                    SetupOutputVariable(state, "Hot Water Equipment Total Heating Energy",
                                        OutputProcessor::Unit::J,
                                        ZoneHWEq(Loop).TotGainEnergy,
                                        "Zone",
                                        "Sum",
                                        ZoneHWEq(Loop).Name);
                    SetupOutputVariable(state, "Hot Water Equipment Total Heating Rate",
                                        OutputProcessor::Unit::W,
                                        ZoneHWEq(Loop).TotGainRate,
                                        "Zone",
                                        "Average",
                                        ZoneHWEq(Loop).Name);

                    // Zone total report variables
                    if (RepVarSet(ZoneHWEq(Loop).ZonePtr)) {
                        RepVarSet(ZoneHWEq(Loop).ZonePtr) = false;
                        SetupOutputVariable(state, "Zone Hot Water Equipment District Heating Rate",
                                            OutputProcessor::Unit::W,
                                            ZnRpt(ZoneHWEq(Loop).ZonePtr).HWPower,
                                            "Zone",
                                            "Average",
                                            Zone(ZoneHWEq(Loop).ZonePtr).Name);
                        SetupOutputVariable(state, "Zone Hot Water Equipment District Heating Energy",
                                            OutputProcessor::Unit::J,
                                            ZnRpt(ZoneHWEq(Loop).ZonePtr).HWConsump,
                                            "Zone",
                                            "Sum",
                                            Zone(ZoneHWEq(Loop).ZonePtr).Name);

                        SetupOutputVariable(state, "Zone Hot Water Equipment Radiant Heating Energy",
                                            OutputProcessor::Unit::J,
                                            ZnRpt(ZoneHWEq(Loop).ZonePtr).HWRadGain,
                                            "Zone",
                                            "Sum",
                                            Zone(ZoneHWEq(Loop).ZonePtr).Name);
                        SetupOutputVariable(state, "Zone Hot Water Equipment Radiant Heating Rate",
                                            OutputProcessor::Unit::W,
                                            ZnRpt(ZoneHWEq(Loop).ZonePtr).HWRadGainRate,
                                            "Zone",
                                            "Average",
                                            Zone(ZoneHWEq(Loop).ZonePtr).Name);
                        SetupOutputVariable(state, "Zone Hot Water Equipment Convective Heating Energy",
                                            OutputProcessor::Unit::J,
                                            ZnRpt(ZoneHWEq(Loop).ZonePtr).HWConGain,
                                            "Zone",
                                            "Sum",
                                            Zone(ZoneHWEq(Loop).ZonePtr).Name);
                        SetupOutputVariable(state, "Zone Hot Water Equipment Convective Heating Rate",
                                            OutputProcessor::Unit::W,
                                            ZnRpt(ZoneHWEq(Loop).ZonePtr).HWConGainRate,
                                            "Zone",
                                            "Average",
                                            Zone(ZoneHWEq(Loop).ZonePtr).Name);
                        SetupOutputVariable(state, "Zone Hot Water Equipment Latent Gain Energy",
                                            OutputProcessor::Unit::J,
                                            ZnRpt(ZoneHWEq(Loop).ZonePtr).HWLatGain,
                                            "Zone",
                                            "Sum",
                                            Zone(ZoneHWEq(Loop).ZonePtr).Name);
                        SetupOutputVariable(state, "Zone Hot Water Equipment Latent Gain Rate",
                                            OutputProcessor::Unit::W,
                                            ZnRpt(ZoneHWEq(Loop).ZonePtr).HWLatGainRate,
                                            "Zone",
                                            "Average",
                                            Zone(ZoneHWEq(Loop).ZonePtr).Name);
                        SetupOutputVariable(state, "Zone Hot Water Equipment Lost Heat Energy",
                                            OutputProcessor::Unit::J,
                                            ZnRpt(ZoneHWEq(Loop).ZonePtr).HWLost,
                                            "Zone",
                                            "Sum",
                                            Zone(ZoneHWEq(Loop).ZonePtr).Name);
                        SetupOutputVariable(state, "Zone Hot Water Equipment Lost Heat Rate",
                                            OutputProcessor::Unit::W,
                                            ZnRpt(ZoneHWEq(Loop).ZonePtr).HWLostRate,
                                            "Zone",
                                            "Average",
                                            Zone(ZoneHWEq(Loop).ZonePtr).Name);
                        SetupOutputVariable(state, "Zone Hot Water Equipment Total Heating Energy",
                                            OutputProcessor::Unit::J,
                                            ZnRpt(ZoneHWEq(Loop).ZonePtr).HWTotGain,
                                            "Zone",
                                            "Sum",
                                            Zone(ZoneHWEq(Loop).ZonePtr).Name);
                        SetupOutputVariable(state, "Zone Hot Water Equipment Total Heating Rate",
                                            OutputProcessor::Unit::W,
                                            ZnRpt(ZoneHWEq(Loop).ZonePtr).HWTotGainRate,
                                            "Zone",
                                            "Average",
                                            Zone(ZoneHWEq(Loop).ZonePtr).Name);
                    }

                    if (AnyEnergyManagementSystemInModel) {
                        SetupEMSActuator("HotWaterEquipment",
                                         ZoneHWEq(Loop).Name,
                                         "District Heating Power Level",
                                         "[W]",
                                         ZoneHWEq(Loop).EMSZoneEquipOverrideOn,
                                         ZoneHWEq(Loop).EMSEquipPower);
                        SetupEMSInternalVariable("Process District Heat Design Level", ZoneHWEq(Loop).Name, "[W]", ZoneHWEq(Loop).DesignLevel);
                    } // EMS

                    if (!ErrorsFound)
                        SetupZoneInternalGain(ZoneHWEq(Loop).ZonePtr,
                                              "HotWaterEquipment",
                                              ZoneHWEq(Loop).Name,
                                              IntGainTypeOf_HotWaterEquipment,
                                              &ZoneHWEq(Loop).ConGainRate,
                                              nullptr,
                                              &ZoneHWEq(Loop).RadGainRate,
                                              &ZoneHWEq(Loop).LatGainRate);

                } // Item1
            }     // Item - number of hot water statements
        }

        RepVarSet = true;
        CurrentModuleObject = "SteamEquipment";
        NumSteamEqStatements = inputProcessor->getNumObjectsFound(CurrentModuleObject);
        SteamEqObjects.allocate(NumSteamEqStatements);

        TotStmEquip = 0;
        errFlag = false;
        for (Item = 1; Item <= NumSteamEqStatements; ++Item) {
            inputProcessor->getObjectItem(state,
                                          CurrentModuleObject,
                                          Item,
                                          AlphaName,
                                          NumAlpha,
                                          IHGNumbers,
                                          NumNumber,
                                          IOStat,
                                          lNumericFieldBlanks,
                                          lAlphaFieldBlanks,
                                          cAlphaFieldNames,
                                          cNumericFieldNames);
            UtilityRoutines::IsNameEmpty(AlphaName(1), CurrentModuleObject, ErrorsFound);
            errFlag = ErrorsFound;

            SteamEqObjects(Item).Name = AlphaName(1);

            Item1 = UtilityRoutines::FindItemInList(AlphaName(2), Zone);
            ZLItem = 0;
            if (Item1 == 0 && NumOfZoneLists > 0) ZLItem = UtilityRoutines::FindItemInList(AlphaName(2), ZoneList);
            if (Item1 > 0) {
                SteamEqObjects(Item).StartPtr = TotStmEquip + 1;
                ++TotStmEquip;
                SteamEqObjects(Item).NumOfZones = 1;
                SteamEqObjects(Item).ZoneListActive = false;
                SteamEqObjects(Item).ZoneOrZoneListPtr = Item1;
            } else if (ZLItem > 0) {
                SteamEqObjects(Item).StartPtr = TotStmEquip + 1;
                TotStmEquip += ZoneList(ZLItem).NumOfZones;
                SteamEqObjects(Item).NumOfZones = ZoneList(ZLItem).NumOfZones;
                SteamEqObjects(Item).ZoneListActive = true;
                SteamEqObjects(Item).ZoneOrZoneListPtr = ZLItem;
            } else {
                ShowSevereError(CurrentModuleObject + "=\"" + AlphaName(1) + "\" invalid " + cAlphaFieldNames(2) + "=\"" + AlphaName(2) +
                                "\" not found.");
                ErrorsFound = true;
                errFlag = true;
            }
        }

        if (errFlag) {
            ShowSevereError(RoutineName + "Errors with invalid names in " + CurrentModuleObject + " objects.");
            ShowContinueError("...These will not be read in.  Other errors may occur.");
            TotStmEquip = 0;
        }

        ZoneSteamEq.allocate(TotStmEquip);

        if (TotStmEquip > 0) {
            Loop = 0;
            for (Item = 1; Item <= NumSteamEqStatements; ++Item) {
                AlphaName = BlankString;
                IHGNumbers = 0.0;

                inputProcessor->getObjectItem(state,
                                              CurrentModuleObject,
                                              Item,
                                              AlphaName,
                                              NumAlpha,
                                              IHGNumbers,
                                              NumNumber,
                                              IOStat,
                                              lNumericFieldBlanks,
                                              lAlphaFieldBlanks,
                                              cAlphaFieldNames,
                                              cNumericFieldNames);

                for (Item1 = 1; Item1 <= SteamEqObjects(Item).NumOfZones; ++Item1) {
                    ++Loop;
                    if (!SteamEqObjects(Item).ZoneListActive) {
                        ZoneSteamEq(Loop).Name = AlphaName(1);
                        ZoneSteamEq(Loop).ZonePtr = SteamEqObjects(Item).ZoneOrZoneListPtr;
                    } else {
                        CheckCreatedZoneItemName(RoutineName,
                                                 CurrentModuleObject,
                                                 Zone(ZoneList(SteamEqObjects(Item).ZoneOrZoneListPtr).Zone(Item1)).Name,
                                                 ZoneList(SteamEqObjects(Item).ZoneOrZoneListPtr).MaxZoneNameLength,
                                                 SteamEqObjects(Item).Name,
                                                 ZoneSteamEq,
                                                 Loop - 1,
                                                 ZoneSteamEq(Loop).Name,
                                                 errFlag);
                        ZoneSteamEq(Loop).ZonePtr = ZoneList(SteamEqObjects(Item).ZoneOrZoneListPtr).Zone(Item1);
                        if (errFlag) ErrorsFound = true;
                    }

                    ZoneSteamEq(Loop).SchedPtr = GetScheduleIndex(state, AlphaName(3));
                    SchMin = 0.0;
                    SchMax = 0.0;
                    if (ZoneSteamEq(Loop).SchedPtr == 0) {
                        if (lAlphaFieldBlanks(3)) {
                            ShowSevereError(RoutineName + CurrentModuleObject + "=\"" + AlphaName(1) + "\", " + cAlphaFieldNames(3) +
                                            " is required.");
                        } else {
                            ShowSevereError(RoutineName + CurrentModuleObject + "=\"" + AlphaName(1) + "\", invalid " + cAlphaFieldNames(3) +
                                            " entered=" + AlphaName(3));
                        }
                        ErrorsFound = true;
                    } else { // check min/max on schedule
                        SchMin = GetScheduleMinValue(ZoneSteamEq(Loop).SchedPtr);
                        SchMax = GetScheduleMaxValue(ZoneSteamEq(Loop).SchedPtr);
                        if (SchMin < 0.0 || SchMax < 0.0) {
                            if (SchMin < 0.0) {
                                ShowSevereError(RoutineName + CurrentModuleObject + "=\"" + AlphaName(1) + "\", " + cAlphaFieldNames(3) +
                                                ", minimum is < 0.0");
                                ShowContinueError("Schedule=\"" + AlphaName(3) + "\". Minimum is [" + RoundSigDigits(SchMin, 1) +
                                                  "]. Values must be >= 0.0.");
                                ErrorsFound = true;
                            }
                            if (SchMax < 0.0) {
                                ShowSevereError(RoutineName + CurrentModuleObject + "=\"" + AlphaName(1) + "\", " + cAlphaFieldNames(3) +
                                                ", maximum is < 0.0");
                                ShowContinueError("Schedule=\"" + AlphaName(3) + "\". Maximum is [" + RoundSigDigits(SchMax, 1) +
                                                  "]. Values must be >= 0.0.");
                                ErrorsFound = true;
                            }
                        }
                    }

                    // Hot Water equipment design level calculation method.
                    {
                        auto const equipmentLevel(AlphaName(4));
                        if (equipmentLevel == "EQUIPMENTLEVEL") {
                            ZoneSteamEq(Loop).DesignLevel = IHGNumbers(1);
                            if (lNumericFieldBlanks(1)) {
                                ShowWarningError(RoutineName + CurrentModuleObject + "=\"" + AlphaName(1) + "\", specifies " + cNumericFieldNames(1) +
                                                 ", but that field is blank.  0 Hot Water Equipment will result.");
                            }

                        } else if (equipmentLevel == "WATTS/AREA" || equipmentLevel == "POWER/AREA") {
                            if (ZoneSteamEq(Loop).ZonePtr != 0) {
                                if (IHGNumbers(2) >= 0.0) {
                                    ZoneSteamEq(Loop).DesignLevel = IHGNumbers(2) * Zone(ZoneSteamEq(Loop).ZonePtr).FloorArea;
                                    if (Zone(ZoneSteamEq(Loop).ZonePtr).FloorArea <= 0.0) {
                                        ShowWarningError(RoutineName + CurrentModuleObject + "=\"" + AlphaName(1) + "\", specifies " +
                                                         cNumericFieldNames(2) + ", but Zone Floor Area = 0.  0 Hot Water Equipment will result.");
                                    }
                                } else {
                                    ShowSevereError(RoutineName + CurrentModuleObject + "=\"" + AlphaName(1) + "\", invalid " +
                                                    cNumericFieldNames(2) + ", value  [<0.0]=" + RoundSigDigits(IHGNumbers(2), 3));
                                    ErrorsFound = true;
                                }
                            }
                            if (lNumericFieldBlanks(2)) {
                                ShowWarningError(RoutineName + CurrentModuleObject + "=\"" + AlphaName(1) + "\", specifies " + cNumericFieldNames(2) +
                                                 ", but that field is blank.  0 Hot Water Equipment will result.");
                            }

                        } else if (equipmentLevel == "WATTS/PERSON" || equipmentLevel == "POWER/PERSON") {
                            if (ZoneSteamEq(Loop).ZonePtr != 0) {
                                if (IHGNumbers(3) >= 0.0) {
                                    ZoneSteamEq(Loop).DesignLevel = IHGNumbers(3) * Zone(ZoneSteamEq(Loop).ZonePtr).TotOccupants;
                                    if (Zone(ZoneSteamEq(Loop).ZonePtr).TotOccupants <= 0.0) {
                                        ShowWarningError(RoutineName + CurrentModuleObject + "=\"" + AlphaName(1) + "\", specifies " +
                                                         cNumericFieldNames(2) + ", but Total Occupants = 0.  0 Hot Water Equipment will result.");
                                    }
                                } else {
                                    ShowSevereError(RoutineName + CurrentModuleObject + "=\"" + AlphaName(1) + "\", invalid " +
                                                    cNumericFieldNames(3) + ", value  [<0.0]=" + RoundSigDigits(IHGNumbers(3), 3));
                                    ErrorsFound = true;
                                }
                            }
                            if (lNumericFieldBlanks(3)) {
                                ShowWarningError(RoutineName + CurrentModuleObject + "=\"" + AlphaName(1) + "\", specifies " + cNumericFieldNames(3) +
                                                 ", but that field is blank.  0 Hot Water Equipment will result.");
                            }

                        } else {
                            if (Item1 == 1) {
                                ShowSevereError(RoutineName + CurrentModuleObject + "=\"" + AlphaName(1) + "\", invalid " + cAlphaFieldNames(4) +
                                                ", value  =" + AlphaName(4));
                                ShowContinueError("...Valid values are \"EquipmentLevel\", \"Watts/Area\", \"Watts/Person\".");
                                ErrorsFound = true;
                            }
                        }
                    }

                    // Calculate nominal min/max equipment level
                    ZoneSteamEq(Loop).NomMinDesignLevel = ZoneSteamEq(Loop).DesignLevel * SchMin;
                    ZoneSteamEq(Loop).NomMaxDesignLevel = ZoneSteamEq(Loop).DesignLevel * SchMax;

                    ZoneSteamEq(Loop).FractionLatent = IHGNumbers(4);
                    ZoneSteamEq(Loop).FractionRadiant = IHGNumbers(5);
                    ZoneSteamEq(Loop).FractionLost = IHGNumbers(6);
                    // FractionConvected is a calculated field
                    ZoneSteamEq(Loop).FractionConvected =
                        1.0 - (ZoneSteamEq(Loop).FractionLatent + ZoneSteamEq(Loop).FractionRadiant + ZoneSteamEq(Loop).FractionLost);
                    if (std::abs(ZoneSteamEq(Loop).FractionConvected) <= 0.001) ZoneSteamEq(Loop).FractionConvected = 0.0;
                    if (ZoneSteamEq(Loop).FractionConvected < 0.0) {
                        ShowSevereError(RoutineName + CurrentModuleObject + "=\"" + AlphaName(1) + "\", Sum of Fractions > 1.0");
                        ErrorsFound = true;
                    }

                    if (NumAlpha > 4) {
                        ZoneSteamEq(Loop).EndUseSubcategory = AlphaName(5);
                    } else {
                        ZoneSteamEq(Loop).EndUseSubcategory = "General";
                    }

                    if (ZoneSteamEq(Loop).ZonePtr <= 0) continue; // Error, will be caught and terminated later

                    // Object report variables
                    SetupOutputVariable(state, "Steam Equipment District Heating Rate",
                                        OutputProcessor::Unit::W,
                                        ZoneSteamEq(Loop).Power,
                                        "Zone",
                                        "Average",
                                        ZoneSteamEq(Loop).Name);
                    SetupOutputVariable(state, "Steam Equipment District Heating Energy",
                                        OutputProcessor::Unit::J,
                                        ZoneSteamEq(Loop).Consumption,
                                        "Zone",
                                        "Sum",
                                        ZoneSteamEq(Loop).Name,
                                        _,
                                        "DistrictHeating",
                                        "InteriorEquipment",
                                        ZoneSteamEq(Loop).EndUseSubcategory,
                                        "Building",
                                        Zone(ZoneSteamEq(Loop).ZonePtr).Name,
                                        Zone(ZoneSteamEq(Loop).ZonePtr).Multiplier,
                                        Zone(ZoneSteamEq(Loop).ZonePtr).ListMultiplier);

                    SetupOutputVariable(state, "Steam Equipment Radiant Heating Energy",
                                        OutputProcessor::Unit::J,
                                        ZoneSteamEq(Loop).RadGainEnergy,
                                        "Zone",
                                        "Sum",
                                        ZoneSteamEq(Loop).Name);
                    SetupOutputVariable(state, "Steam Equipment Radiant Heating Rate",
                                        OutputProcessor::Unit::W,
                                        ZoneSteamEq(Loop).RadGainRate,
                                        "Zone",
                                        "Average",
                                        ZoneSteamEq(Loop).Name);
                    SetupOutputVariable(state, "Steam Equipment Convective Heating Energy",
                                        OutputProcessor::Unit::J,
                                        ZoneSteamEq(Loop).ConGainEnergy,
                                        "Zone",
                                        "Sum",
                                        ZoneSteamEq(Loop).Name);
                    SetupOutputVariable(state, "Steam Equipment Convective Heating Rate",
                                        OutputProcessor::Unit::W,
                                        ZoneSteamEq(Loop).ConGainRate,
                                        "Zone",
                                        "Average",
                                        ZoneSteamEq(Loop).Name);
                    SetupOutputVariable(state, "Steam Equipment Latent Gain Energy",
                                        OutputProcessor::Unit::J,
                                        ZoneSteamEq(Loop).LatGainEnergy,
                                        "Zone",
                                        "Sum",
                                        ZoneSteamEq(Loop).Name);
                    SetupOutputVariable(state, "Steam Equipment Latent Gain Rate",
                                        OutputProcessor::Unit::W,
                                        ZoneSteamEq(Loop).LatGainRate,
                                        "Zone",
                                        "Average",
                                        ZoneSteamEq(Loop).Name);
                    SetupOutputVariable(state, "Steam Equipment Lost Heat Energy",
                                        OutputProcessor::Unit::J,
                                        ZoneSteamEq(Loop).LostEnergy,
                                        "Zone",
                                        "Sum",
                                        ZoneSteamEq(Loop).Name);
                    SetupOutputVariable(state, "Steam Equipment Lost Heat Rate",
                                        OutputProcessor::Unit::W,
                                        ZoneSteamEq(Loop).LostRate,
                                        "Zone",
                                        "Average",
                                        ZoneSteamEq(Loop).Name);
                    SetupOutputVariable(state, "Steam Equipment Total Heating Energy",
                                        OutputProcessor::Unit::J,
                                        ZoneSteamEq(Loop).TotGainEnergy,
                                        "Zone",
                                        "Sum",
                                        ZoneSteamEq(Loop).Name);
                    SetupOutputVariable(state, "Steam Equipment Total Heating Rate",
                                        OutputProcessor::Unit::W,
                                        ZoneSteamEq(Loop).TotGainRate,
                                        "Zone",
                                        "Average",
                                        ZoneSteamEq(Loop).Name);

                    // Zone total report variables
                    if (RepVarSet(ZoneSteamEq(Loop).ZonePtr)) {
                        RepVarSet(ZoneSteamEq(Loop).ZonePtr) = false;
                        SetupOutputVariable(state, "Zone Steam Equipment District Heating Rate",
                                            OutputProcessor::Unit::W,
                                            ZnRpt(ZoneSteamEq(Loop).ZonePtr).SteamPower,
                                            "Zone",
                                            "Average",
                                            Zone(ZoneSteamEq(Loop).ZonePtr).Name);
                        SetupOutputVariable(state, "Zone Steam Equipment District Heating Energy",
                                            OutputProcessor::Unit::J,
                                            ZnRpt(ZoneSteamEq(Loop).ZonePtr).SteamConsump,
                                            "Zone",
                                            "Sum",
                                            Zone(ZoneSteamEq(Loop).ZonePtr).Name);

                        SetupOutputVariable(state, "Zone Steam Equipment Radiant Heating Energy",
                                            OutputProcessor::Unit::J,
                                            ZnRpt(ZoneSteamEq(Loop).ZonePtr).SteamRadGain,
                                            "Zone",
                                            "Sum",
                                            Zone(ZoneSteamEq(Loop).ZonePtr).Name);
                        SetupOutputVariable(state, "Zone Steam Equipment Radiant Heating Rate",
                                            OutputProcessor::Unit::W,
                                            ZnRpt(ZoneSteamEq(Loop).ZonePtr).SteamRadGainRate,
                                            "Zone",
                                            "Average",
                                            Zone(ZoneSteamEq(Loop).ZonePtr).Name);
                        SetupOutputVariable(state, "Zone Steam Equipment Convective Heating Energy",
                                            OutputProcessor::Unit::J,
                                            ZnRpt(ZoneSteamEq(Loop).ZonePtr).SteamConGain,
                                            "Zone",
                                            "Sum",
                                            Zone(ZoneSteamEq(Loop).ZonePtr).Name);
                        SetupOutputVariable(state, "Zone Steam Equipment Convective Heating Rate",
                                            OutputProcessor::Unit::W,
                                            ZnRpt(ZoneSteamEq(Loop).ZonePtr).SteamConGainRate,
                                            "Zone",
                                            "Average",
                                            Zone(ZoneSteamEq(Loop).ZonePtr).Name);
                        SetupOutputVariable(state, "Zone Steam Equipment Latent Gain Energy",
                                            OutputProcessor::Unit::J,
                                            ZnRpt(ZoneSteamEq(Loop).ZonePtr).SteamLatGain,
                                            "Zone",
                                            "Sum",
                                            Zone(ZoneSteamEq(Loop).ZonePtr).Name);
                        SetupOutputVariable(state, "Zone Steam Equipment Latent Gain Rate",
                                            OutputProcessor::Unit::W,
                                            ZnRpt(ZoneSteamEq(Loop).ZonePtr).SteamLatGainRate,
                                            "Zone",
                                            "Average",
                                            Zone(ZoneSteamEq(Loop).ZonePtr).Name);
                        SetupOutputVariable(state, "Zone Steam Equipment Lost Heat Energy",
                                            OutputProcessor::Unit::J,
                                            ZnRpt(ZoneSteamEq(Loop).ZonePtr).SteamLost,
                                            "Zone",
                                            "Sum",
                                            Zone(ZoneSteamEq(Loop).ZonePtr).Name);
                        SetupOutputVariable(state, "Zone Steam Equipment Lost Heat Rate",
                                            OutputProcessor::Unit::W,
                                            ZnRpt(ZoneSteamEq(Loop).ZonePtr).SteamLostRate,
                                            "Zone",
                                            "Average",
                                            Zone(ZoneSteamEq(Loop).ZonePtr).Name);
                        SetupOutputVariable(state, "Zone Steam Equipment Total Heating Energy",
                                            OutputProcessor::Unit::J,
                                            ZnRpt(ZoneSteamEq(Loop).ZonePtr).SteamTotGain,
                                            "Zone",
                                            "Sum",
                                            Zone(ZoneSteamEq(Loop).ZonePtr).Name);
                        SetupOutputVariable(state, "Zone Steam Equipment Total Heating Rate",
                                            OutputProcessor::Unit::W,
                                            ZnRpt(ZoneSteamEq(Loop).ZonePtr).SteamTotGainRate,
                                            "Zone",
                                            "Average",
                                            Zone(ZoneSteamEq(Loop).ZonePtr).Name);
                    }

                    if (AnyEnergyManagementSystemInModel) {
                        SetupEMSActuator("SteamEquipment",
                                         ZoneSteamEq(Loop).Name,
                                         "District Heating Power Level",
                                         "[W]",
                                         ZoneSteamEq(Loop).EMSZoneEquipOverrideOn,
                                         ZoneSteamEq(Loop).EMSEquipPower);
                        SetupEMSInternalVariable(
                            "Process Steam District Heat Design Level", ZoneSteamEq(Loop).Name, "[W]", ZoneSteamEq(Loop).DesignLevel);
                    } // EMS

                    if (!ErrorsFound)
                        SetupZoneInternalGain(ZoneSteamEq(Loop).ZonePtr,
                                              "SteamEquipment",
                                              ZoneSteamEq(Loop).Name,
                                              IntGainTypeOf_SteamEquipment,
                                              &ZoneSteamEq(Loop).ConGainRate,
                                              nullptr,
                                              &ZoneSteamEq(Loop).RadGainRate,
                                              &ZoneSteamEq(Loop).LatGainRate);

                } // Item1
            }     // Item - number of hot water statements
        }

        RepVarSet = true;
        CurrentModuleObject = "OtherEquipment";
        NumOtherEqStatements = inputProcessor->getNumObjectsFound(CurrentModuleObject);
        OtherEqObjects.allocate(NumOtherEqStatements);

        TotOthEquip = 0;
        errFlag = false;
        for (Item = 1; Item <= NumOtherEqStatements; ++Item) {
            inputProcessor->getObjectItem(state,
                                          CurrentModuleObject,
                                          Item,
                                          AlphaName,
                                          NumAlpha,
                                          IHGNumbers,
                                          NumNumber,
                                          IOStat,
                                          lNumericFieldBlanks,
                                          lAlphaFieldBlanks,
                                          cAlphaFieldNames,
                                          cNumericFieldNames);
            UtilityRoutines::IsNameEmpty(AlphaName(1), CurrentModuleObject, ErrorsFound);
            errFlag = ErrorsFound;

            OtherEqObjects(Item).Name = AlphaName(1);

            Item1 = UtilityRoutines::FindItemInList(AlphaName(3), Zone);
            ZLItem = 0;
            if (Item1 == 0 && NumOfZoneLists > 0) ZLItem = UtilityRoutines::FindItemInList(AlphaName(3), ZoneList);
            if (Item1 > 0) {
                OtherEqObjects(Item).StartPtr = TotOthEquip + 1;
                ++TotOthEquip;
                OtherEqObjects(Item).NumOfZones = 1;
                OtherEqObjects(Item).ZoneListActive = false;
                OtherEqObjects(Item).ZoneOrZoneListPtr = Item1;
            } else if (ZLItem > 0) {
                OtherEqObjects(Item).StartPtr = TotOthEquip + 1;
                TotOthEquip += ZoneList(ZLItem).NumOfZones;
                OtherEqObjects(Item).NumOfZones = ZoneList(ZLItem).NumOfZones;
                OtherEqObjects(Item).ZoneListActive = true;
                OtherEqObjects(Item).ZoneOrZoneListPtr = ZLItem;
            } else {
                ShowSevereError(CurrentModuleObject + "=\"" + AlphaName(1) + "\" invalid " + cAlphaFieldNames(3) + "=\"" + AlphaName(3) +
                                "\" not found.");
                ErrorsFound = true;
                errFlag = true;
            }
        }

        if (errFlag) {
            ShowSevereError(RoutineName + "Errors with invalid names in " + CurrentModuleObject + " objects.");
            ShowContinueError("...These will not be read in.  Other errors may occur.");
            TotOthEquip = 0;
        }

        ZoneOtherEq.allocate(TotOthEquip);

        if (TotOthEquip > 0) {
            Loop = 0;
            for (Item = 1; Item <= NumOtherEqStatements; ++Item) {
                AlphaName = BlankString;
                IHGNumbers = 0.0;

                inputProcessor->getObjectItem(state,
                                              CurrentModuleObject,
                                              Item,
                                              AlphaName,
                                              NumAlpha,
                                              IHGNumbers,
                                              NumNumber,
                                              IOStat,
                                              lNumericFieldBlanks,
                                              lAlphaFieldBlanks,
                                              cAlphaFieldNames,
                                              cNumericFieldNames);

                for (Item1 = 1; Item1 <= OtherEqObjects(Item).NumOfZones; ++Item1) {
                    ++Loop;
                    if (!OtherEqObjects(Item).ZoneListActive) {
                        ZoneOtherEq(Loop).Name = AlphaName(1);
                        ZoneOtherEq(Loop).ZonePtr = OtherEqObjects(Item).ZoneOrZoneListPtr;
                    } else {
                        CheckCreatedZoneItemName(RoutineName,
                                                 CurrentModuleObject,
                                                 Zone(ZoneList(OtherEqObjects(Item).ZoneOrZoneListPtr).Zone(Item1)).Name,
                                                 ZoneList(OtherEqObjects(Item).ZoneOrZoneListPtr).MaxZoneNameLength,
                                                 OtherEqObjects(Item).Name,
                                                 ZoneOtherEq,
                                                 Loop - 1,
                                                 ZoneOtherEq(Loop).Name,
                                                 errFlag);
                        ZoneOtherEq(Loop).ZonePtr = ZoneList(OtherEqObjects(Item).ZoneOrZoneListPtr).Zone(Item1);
                        if (errFlag) ErrorsFound = true;
                    }

                    std::string FuelTypeString("");
                    if (AlphaName(2) == "NONE") {
                        ZoneOtherEq(Loop).OtherEquipFuelType = ExteriorEnergyUse::ExteriorFuelUsage::Unknown;
                        FuelTypeString = AlphaName(2);
                    } else {
                        ExteriorEnergyUse::ValidateFuelType(ZoneOtherEq(Loop).OtherEquipFuelType,
                                                            AlphaName(2),
                                                            FuelTypeString,
                                                            CurrentModuleObject,
                                                            cAlphaFieldNames(2),
                                                            AlphaName(2));
                        if (ZoneOtherEq(Loop).OtherEquipFuelType == ExteriorEnergyUse::ExteriorFuelUsage::Unknown ||
                            ZoneOtherEq(Loop).OtherEquipFuelType == ExteriorEnergyUse::ExteriorFuelUsage::WaterUse) {
                            ShowSevereError(RoutineName + CurrentModuleObject + ": invalid " + cAlphaFieldNames(2) + " entered=" + AlphaName(2) +
                                            " for " + cAlphaFieldNames(1) + '=' + AlphaName(1));
                            ErrorsFound = true;
                        }
                    }

                    ZoneOtherEq(Loop).SchedPtr = GetScheduleIndex(state, AlphaName(4));
                    SchMin = 0.0;
                    SchMax = 0.0;
                    if (ZoneOtherEq(Loop).SchedPtr == 0) {
                        if (lAlphaFieldBlanks(4)) {
                            ShowSevereError(RoutineName + CurrentModuleObject + "=\"" + AlphaName(1) + "\", " + cAlphaFieldNames(4) +
                                            " is required.");
                        } else {
                            ShowSevereError(RoutineName + CurrentModuleObject + "=\"" + AlphaName(1) + "\", invalid " + cAlphaFieldNames(4) +
                                            " entered=" + AlphaName(4));
                        }
                        ErrorsFound = true;
                    } else { // check min/max on schedule
                        SchMin = GetScheduleMinValue(ZoneOtherEq(Loop).SchedPtr);
                        SchMax = GetScheduleMaxValue(ZoneOtherEq(Loop).SchedPtr);
                    }

                    // equipment design level calculation method.
                    unsigned int DesignLevelFieldNumber;
                    {
                        auto const equipmentLevel(AlphaName(5));
                        if (equipmentLevel == "EQUIPMENTLEVEL") {
                            DesignLevelFieldNumber = 1;
                            ZoneOtherEq(Loop).DesignLevel = IHGNumbers(DesignLevelFieldNumber);
                            if (lNumericFieldBlanks(DesignLevelFieldNumber)) {
                                ShowWarningError(RoutineName + CurrentModuleObject + "=\"" + AlphaName(1) + "\", specifies " +
                                                 cNumericFieldNames(DesignLevelFieldNumber) +
                                                 ", but that field is blank.  0 Other Equipment will result.");
                            }

                        } else if (equipmentLevel == "WATTS/AREA" || equipmentLevel == "POWER/AREA") {
                            DesignLevelFieldNumber = 2;
                            if (ZoneOtherEq(Loop).ZonePtr != 0) {
                                ZoneOtherEq(Loop).DesignLevel = IHGNumbers(DesignLevelFieldNumber) * Zone(ZoneOtherEq(Loop).ZonePtr).FloorArea;
                                if (Zone(ZoneOtherEq(Loop).ZonePtr).FloorArea <= 0.0) {
                                    ShowWarningError(RoutineName + CurrentModuleObject + "=\"" + AlphaName(1) + "\", specifies " +
                                                     cNumericFieldNames(DesignLevelFieldNumber) +
                                                     ", but Zone Floor Area = 0.  0 Other Equipment will result.");
                                }
                            }
                            if (lNumericFieldBlanks(DesignLevelFieldNumber)) {
                                ShowWarningError(RoutineName + CurrentModuleObject + "=\"" + AlphaName(1) + "\", specifies " +
                                                 cNumericFieldNames(DesignLevelFieldNumber) +
                                                 ", but that field is blank.  0 Other Equipment will result.");
                            }

                        } else if (equipmentLevel == "WATTS/PERSON" || equipmentLevel == "POWER/PERSON") {
                            DesignLevelFieldNumber = 3;
                            if (ZoneOtherEq(Loop).ZonePtr != 0) {
                                ZoneOtherEq(Loop).DesignLevel = IHGNumbers(3) * Zone(ZoneOtherEq(Loop).ZonePtr).TotOccupants;
                                if (Zone(ZoneOtherEq(Loop).ZonePtr).TotOccupants <= 0.0) {
                                    ShowWarningError(RoutineName + CurrentModuleObject + "=\"" + AlphaName(1) + "\", specifies " +
                                                     cNumericFieldNames(DesignLevelFieldNumber) +
                                                     ", but Total Occupants = 0.  0 Other Equipment will result.");
                                }
                            }
                            if (lNumericFieldBlanks(DesignLevelFieldNumber)) {
                                ShowWarningError(RoutineName + CurrentModuleObject + "=\"" + AlphaName(1) + "\", specifies " +
                                                 cNumericFieldNames(DesignLevelFieldNumber) +
                                                 ", but that field is blank.  0 Other Equipment will result.");
                            }

                        } else {
                            if (Item1 == 1) {
                                ShowSevereError(RoutineName + CurrentModuleObject + "=\"" + AlphaName(1) + "\", invalid " + cAlphaFieldNames(5) +
                                                ", value  =" + AlphaName(5));
                                ShowContinueError("...Valid values are \"EquipmentLevel\", \"Watts/Area\", \"Watts/Person\".");
                                ErrorsFound = true;
                            }
                        }
                    }

                    // Throw an error if the design level is negative and we have a fuel type
                    if (ZoneOtherEq(Loop).DesignLevel < 0.0 && ZoneOtherEq(Loop).OtherEquipFuelType != ExteriorEnergyUse::ExteriorFuelUsage::Unknown) {
                        ShowSevereError(RoutineName + CurrentModuleObject + "=\"" + AlphaName(1) + "\", " +
                                        cNumericFieldNames(DesignLevelFieldNumber) + " is not allowed to be negative");
                        ShowContinueError("... when a fuel type of " + FuelTypeString + " is specified.");
                        ErrorsFound = true;
                    }

                    // Calculate nominal min/max equipment level
                    ZoneOtherEq(Loop).NomMinDesignLevel = ZoneOtherEq(Loop).DesignLevel * SchMin;
                    ZoneOtherEq(Loop).NomMaxDesignLevel = ZoneOtherEq(Loop).DesignLevel * SchMax;

                    ZoneOtherEq(Loop).FractionLatent = IHGNumbers(4);
                    ZoneOtherEq(Loop).FractionRadiant = IHGNumbers(5);
                    ZoneOtherEq(Loop).FractionLost = IHGNumbers(6);

                    if ((NumNumber == 7) || (!lNumericFieldBlanks(7))) {
                        ZoneOtherEq(Loop).CO2RateFactor = IHGNumbers(7);
                    }
                    if (ZoneOtherEq(Loop).CO2RateFactor < 0.0) {
                        ShowSevereError(RoutineName + CurrentModuleObject + "=\"" + AlphaName(1) + "\", " + cNumericFieldNames(7) +
                                        " < 0.0, value =" + RoundSigDigits(IHGNumbers(7), 2));
                        ErrorsFound = true;
                    }
                    if (ZoneOtherEq(Loop).CO2RateFactor > 4.0e-7) {
                        ShowSevereError(RoutineName + CurrentModuleObject + "=\"" + AlphaName(1) + "\", " + cNumericFieldNames(7) +
                                        " > 4.0E-7, value =" + RoundSigDigits(IHGNumbers(7), 2));
                        ErrorsFound = true;
                    }

                    // FractionConvected is a calculated field
                    ZoneOtherEq(Loop).FractionConvected =
                        1.0 - (ZoneOtherEq(Loop).FractionLatent + ZoneOtherEq(Loop).FractionRadiant + ZoneOtherEq(Loop).FractionLost);
                    if (std::abs(ZoneOtherEq(Loop).FractionConvected) <= 0.001) ZoneOtherEq(Loop).FractionConvected = 0.0;
                    if (ZoneOtherEq(Loop).FractionConvected < 0.0) {
                        ShowSevereError(RoutineName + CurrentModuleObject + "=\"" + AlphaName(1) + "\", Sum of Fractions > 1.0");
                        ErrorsFound = true;
                    }

                    if (NumAlpha > 5) {
                        ZoneOtherEq(Loop).EndUseSubcategory = AlphaName(6);
                    } else {
                        ZoneOtherEq(Loop).EndUseSubcategory = "General";
                    }

                    if (ZoneOtherEq(Loop).ZonePtr <= 0) continue; // Error, will be caught and terminated later

                    // Object report variables
                    if (ZoneOtherEq(Loop).OtherEquipFuelType != ExteriorEnergyUse::ExteriorFuelUsage::Unknown) {
                        SetupOutputVariable(state, "Other Equipment " + FuelTypeString + " Rate",
                                            OutputProcessor::Unit::W,
                                            ZoneOtherEq(Loop).Power,
                                            "Zone",
                                            "Average",
                                            ZoneOtherEq(Loop).Name);
                        SetupOutputVariable(state, "Other Equipment " + FuelTypeString + " Energy",
                                            OutputProcessor::Unit::J,
                                            ZoneOtherEq(Loop).Consumption,
                                            "Zone",
                                            "Sum",
                                            ZoneOtherEq(Loop).Name,
                                            _,
                                            FuelTypeString,
                                            "InteriorEquipment",
                                            ZoneOtherEq(Loop).EndUseSubcategory,
                                            "Building",
                                            Zone(ZoneOtherEq(Loop).ZonePtr).Name,
                                            Zone(ZoneOtherEq(Loop).ZonePtr).Multiplier,
                                            Zone(ZoneOtherEq(Loop).ZonePtr).ListMultiplier);
                    }

                    SetupOutputVariable(state, "Other Equipment Radiant Heating Energy",
                                        OutputProcessor::Unit::J,
                                        ZoneOtherEq(Loop).RadGainEnergy,
                                        "Zone",
                                        "Sum",
                                        ZoneOtherEq(Loop).Name);
                    SetupOutputVariable(state, "Other Equipment Radiant Heating Rate",
                                        OutputProcessor::Unit::W,
                                        ZoneOtherEq(Loop).RadGainRate,
                                        "Zone",
                                        "Average",
                                        ZoneOtherEq(Loop).Name);
                    SetupOutputVariable(state, "Other Equipment Convective Heating Energy",
                                        OutputProcessor::Unit::J,
                                        ZoneOtherEq(Loop).ConGainEnergy,
                                        "Zone",
                                        "Sum",
                                        ZoneOtherEq(Loop).Name);
                    SetupOutputVariable(state, "Other Equipment Convective Heating Rate",
                                        OutputProcessor::Unit::W,
                                        ZoneOtherEq(Loop).ConGainRate,
                                        "Zone",
                                        "Average",
                                        ZoneOtherEq(Loop).Name);
                    SetupOutputVariable(state, "Other Equipment Latent Gain Energy",
                                        OutputProcessor::Unit::J,
                                        ZoneOtherEq(Loop).LatGainEnergy,
                                        "Zone",
                                        "Sum",
                                        ZoneOtherEq(Loop).Name);
                    SetupOutputVariable(state, "Other Equipment Latent Gain Rate",
                                        OutputProcessor::Unit::W,
                                        ZoneOtherEq(Loop).LatGainRate,
                                        "Zone",
                                        "Average",
                                        ZoneOtherEq(Loop).Name);
                    SetupOutputVariable(state, "Other Equipment Lost Heat Energy",
                                        OutputProcessor::Unit::J,
                                        ZoneOtherEq(Loop).LostEnergy,
                                        "Zone",
                                        "Sum",
                                        ZoneOtherEq(Loop).Name);
                    SetupOutputVariable(state, "Other Equipment Lost Heat Rate",
                                        OutputProcessor::Unit::W,
                                        ZoneOtherEq(Loop).LostRate,
                                        "Zone",
                                        "Average",
                                        ZoneOtherEq(Loop).Name);
                    SetupOutputVariable(state, "Other Equipment Total Heating Energy",
                                        OutputProcessor::Unit::J,
                                        ZoneOtherEq(Loop).TotGainEnergy,
                                        "Zone",
                                        "Sum",
                                        ZoneOtherEq(Loop).Name);
                    SetupOutputVariable(state, "Other Equipment Total Heating Rate",
                                        OutputProcessor::Unit::W,
                                        ZoneOtherEq(Loop).TotGainRate,
                                        "Zone",
                                        "Average",
                                        ZoneOtherEq(Loop).Name);

                    // Zone total report variables
                    if (RepVarSet(ZoneOtherEq(Loop).ZonePtr)) {
                        RepVarSet(ZoneOtherEq(Loop).ZonePtr) = false;

                        if (ZoneOtherEq(Loop).OtherEquipFuelType != ExteriorEnergyUse::ExteriorFuelUsage::Unknown) {
                            SetupOutputVariable(state, "Zone Other Equipment " + FuelTypeString + " Rate",
                                                OutputProcessor::Unit::W,
                                                ZnRpt(ZoneOtherEq(Loop).ZonePtr).OtherPower,
                                                "Zone",
                                                "Average",
                                                Zone(ZoneOtherEq(Loop).ZonePtr).Name);
                            SetupOutputVariable(state, "Zone Other Equipment " + FuelTypeString + " Energy",
                                                OutputProcessor::Unit::J,
                                                ZnRpt(ZoneOtherEq(Loop).ZonePtr).OtherConsump,
                                                "Zone",
                                                "Sum",
                                                Zone(ZoneOtherEq(Loop).ZonePtr).Name);
                        }

                        SetupOutputVariable(state, "Zone Other Equipment Radiant Heating Energy",
                                            OutputProcessor::Unit::J,
                                            ZnRpt(ZoneOtherEq(Loop).ZonePtr).OtherRadGain,
                                            "Zone",
                                            "Sum",
                                            Zone(ZoneOtherEq(Loop).ZonePtr).Name);
                        SetupOutputVariable(state, "Zone Other Equipment Radiant Heating Rate",
                                            OutputProcessor::Unit::W,
                                            ZnRpt(ZoneOtherEq(Loop).ZonePtr).OtherRadGainRate,
                                            "Zone",
                                            "Average",
                                            Zone(ZoneOtherEq(Loop).ZonePtr).Name);
                        SetupOutputVariable(state, "Zone Other Equipment Convective Heating Energy",
                                            OutputProcessor::Unit::J,
                                            ZnRpt(ZoneOtherEq(Loop).ZonePtr).OtherConGain,
                                            "Zone",
                                            "Sum",
                                            Zone(ZoneOtherEq(Loop).ZonePtr).Name);
                        SetupOutputVariable(state, "Zone Other Equipment Convective Heating Rate",
                                            OutputProcessor::Unit::W,
                                            ZnRpt(ZoneOtherEq(Loop).ZonePtr).OtherConGainRate,
                                            "Zone",
                                            "Average",
                                            Zone(ZoneOtherEq(Loop).ZonePtr).Name);
                        SetupOutputVariable(state, "Zone Other Equipment Latent Gain Energy",
                                            OutputProcessor::Unit::J,
                                            ZnRpt(ZoneOtherEq(Loop).ZonePtr).OtherLatGain,
                                            "Zone",
                                            "Sum",
                                            Zone(ZoneOtherEq(Loop).ZonePtr).Name);
                        SetupOutputVariable(state, "Zone Other Equipment Latent Gain Rate",
                                            OutputProcessor::Unit::W,
                                            ZnRpt(ZoneOtherEq(Loop).ZonePtr).OtherLatGainRate,
                                            "Zone",
                                            "Average",
                                            Zone(ZoneOtherEq(Loop).ZonePtr).Name);
                        SetupOutputVariable(state, "Zone Other Equipment Lost Heat Energy",
                                            OutputProcessor::Unit::J,
                                            ZnRpt(ZoneOtherEq(Loop).ZonePtr).OtherLost,
                                            "Zone",
                                            "Sum",
                                            Zone(ZoneOtherEq(Loop).ZonePtr).Name);
                        SetupOutputVariable(state, "Zone Other Equipment Lost Heat Rate",
                                            OutputProcessor::Unit::W,
                                            ZnRpt(ZoneOtherEq(Loop).ZonePtr).OtherLostRate,
                                            "Zone",
                                            "Average",
                                            Zone(ZoneOtherEq(Loop).ZonePtr).Name);
                        SetupOutputVariable(state, "Zone Other Equipment Total Heating Energy",
                                            OutputProcessor::Unit::J,
                                            ZnRpt(ZoneOtherEq(Loop).ZonePtr).OtherTotGain,
                                            "Zone",
                                            "Sum",
                                            Zone(ZoneOtherEq(Loop).ZonePtr).Name);
                        SetupOutputVariable(state, "Zone Other Equipment Total Heating Rate",
                                            OutputProcessor::Unit::W,
                                            ZnRpt(ZoneOtherEq(Loop).ZonePtr).OtherTotGainRate,
                                            "Zone",
                                            "Average",
                                            Zone(ZoneOtherEq(Loop).ZonePtr).Name);
                    }
                    if (AnyEnergyManagementSystemInModel) {
                        SetupEMSActuator("OtherEquipment",
                                         ZoneOtherEq(Loop).Name,
                                         "Power Level",
                                         "[W]",
                                         ZoneOtherEq(Loop).EMSZoneEquipOverrideOn,
                                         ZoneOtherEq(Loop).EMSEquipPower);
                        SetupEMSInternalVariable("Other Equipment Design Level", ZoneOtherEq(Loop).Name, "[W]", ZoneOtherEq(Loop).DesignLevel);
                    } // EMS

                    if (!ErrorsFound)
                        SetupZoneInternalGain(ZoneOtherEq(Loop).ZonePtr,
                                              "OtherEquipment",
                                              ZoneOtherEq(Loop).Name,
                                              IntGainTypeOf_OtherEquipment,
                                              &ZoneOtherEq(Loop).ConGainRate,
                                              nullptr,
                                              &ZoneOtherEq(Loop).RadGainRate,
                                              &ZoneOtherEq(Loop).LatGainRate);

                } // Item1
            }     // Item - number of other equipment statements
        }

        RepVarSet = true;
        CurrentModuleObject = "ElectricEquipment:ITE:AirCooled";
        NumZoneITEqStatements = inputProcessor->getNumObjectsFound(CurrentModuleObject);
        errFlag = false;

        // Note that this object type does not support ZoneList due to node names in input fields
        ZoneITEq.allocate(NumZoneITEqStatements);

        if (NumZoneITEqStatements > 0) {
            Loop = 0;
            for (Loop = 1; Loop <= NumZoneITEqStatements; ++Loop) {
                AlphaName = BlankString;
                IHGNumbers = 0.0;

                inputProcessor->getObjectItem(state,
                                              CurrentModuleObject,
                                              Loop,
                                              AlphaName,
                                              NumAlpha,
                                              IHGNumbers,
                                              NumNumber,
                                              IOStat,
                                              lNumericFieldBlanks,
                                              lAlphaFieldBlanks,
                                              cAlphaFieldNames,
                                              cNumericFieldNames);

                ZoneITEq(Loop).Name = AlphaName(1);
                ZoneITEq(Loop).ZonePtr = UtilityRoutines::FindItemInList(AlphaName(2), Zone);

                // IT equipment design level calculation method.
                if (lAlphaFieldBlanks(3)) {
                    ZoneITEq(Loop).FlowControlWithApproachTemps = false;
                } else {
                    if (UtilityRoutines::SameString(AlphaName(3), "FlowFromSystem")) {
                        ZoneITEq(Loop).FlowControlWithApproachTemps = false;
                    } else if (UtilityRoutines::SameString(AlphaName(3), "FlowControlWithApproachTemperatures")) {
                        ZoneITEq(Loop).FlowControlWithApproachTemps = true;
                        Zone(ZoneITEq(Loop).ZonePtr).HasAdjustedReturnTempByITE = true;
                        Zone(ZoneITEq(Loop).ZonePtr).NoHeatToReturnAir = false;
                    } else {
                        ShowSevereError(RoutineName + CurrentModuleObject + "=\"" + AlphaName(1) + "\": invalid calculation method: " + AlphaName(3));
                        ErrorsFound = true;
                    }
                }

                {
                    auto const equipmentLevel(AlphaName(4));
                    if (equipmentLevel == "WATTS/UNIT") {
                        ZoneITEq(Loop).DesignTotalPower = IHGNumbers(1) * IHGNumbers(2);
                        if (lNumericFieldBlanks(1)) {
                            ShowWarningError(RoutineName + CurrentModuleObject + "=\"" + AlphaName(1) + "\", specifies " + cNumericFieldNames(1) +
                                             ", but that field is blank.  0 IT Equipment will result.");
                        }
                        if (lNumericFieldBlanks(2)) {
                            ShowWarningError(RoutineName + CurrentModuleObject + "=\"" + AlphaName(1) + "\", specifies " + cNumericFieldNames(2) +
                                             ", but that field is blank.  0 IT Equipment will result.");
                        }

                    } else if (equipmentLevel == "WATTS/AREA") {
                        if (ZoneITEq(Loop).ZonePtr != 0) {
                            if (IHGNumbers(3) >= 0.0) {
                                ZoneITEq(Loop).DesignTotalPower = IHGNumbers(3) * Zone(ZoneITEq(Loop).ZonePtr).FloorArea;
                                if (Zone(ZoneITEq(Loop).ZonePtr).FloorArea <= 0.0) {
                                    ShowWarningError(RoutineName + CurrentModuleObject + "=\"" + AlphaName(1) + "\", specifies " +
                                                     cNumericFieldNames(3) + ", but Zone Floor Area = 0.  0 IT Equipment will result.");
                                }
                            } else {
                                ShowSevereError(RoutineName + CurrentModuleObject + "=\"" + AlphaName(1) + "\", invalid " + cNumericFieldNames(3) +
                                                ", value  [<0.0]=" + RoundSigDigits(IHGNumbers(3), 3));
                                ErrorsFound = true;
                            }
                        }
                        if (lNumericFieldBlanks(3)) {
                            ShowWarningError(RoutineName + CurrentModuleObject + "=\"" + AlphaName(1) + "\", specifies " + cNumericFieldNames(3) +
                                             ", but that field is blank.  0 IT Equipment will result.");
                        }

                    } else {
                        ShowSevereError(RoutineName + CurrentModuleObject + "=\"" + AlphaName(1) + "\", invalid " + cAlphaFieldNames(4) +
                                        ", value  =" + AlphaName(4));
                        ShowContinueError("...Valid values are \"Watts/Unit\" or \"Watts/Area\".");
                        ErrorsFound = true;
                    }
                }

                if (lAlphaFieldBlanks(5)) {
                    ZoneITEq(Loop).OperSchedPtr = ScheduleAlwaysOn;
                } else {
                    ZoneITEq(Loop).OperSchedPtr = GetScheduleIndex(state, AlphaName(5));
                }
                SchMin = 0.0;
                SchMax = 0.0;
                if (ZoneITEq(Loop).OperSchedPtr == 0) {
                    ShowSevereError(RoutineName + CurrentModuleObject + "=\"" + AlphaName(1) + "\", invalid " + cAlphaFieldNames(5) +
                                    " entered=" + AlphaName(5));
                    ErrorsFound = true;
                } else { // check min/max on schedule
                    SchMin = GetScheduleMinValue(ZoneITEq(Loop).OperSchedPtr);
                    SchMax = GetScheduleMaxValue(ZoneITEq(Loop).OperSchedPtr);
                    if (SchMin < 0.0 || SchMax < 0.0) {
                        if (SchMin < 0.0) {
                            ShowSevereError(RoutineName + CurrentModuleObject + "=\"" + AlphaName(1) + "\", " + cAlphaFieldNames(5) +
                                            ", minimum is < 0.0");
                            ShowContinueError("Schedule=\"" + AlphaName(5) + "\". Minimum is [" + RoundSigDigits(SchMin, 1) +
                                              "]. Values must be >= 0.0.");
                            ErrorsFound = true;
                        }
                        if (SchMax < 0.0) {
                            ShowSevereError(RoutineName + CurrentModuleObject + "=\"" + AlphaName(1) + "\", " + cAlphaFieldNames(5) +
                                            ", maximum is < 0.0");
                            ShowContinueError("Schedule=\"" + AlphaName(5) + "\". Maximum is [" + RoundSigDigits(SchMax, 1) +
                                              "]. Values must be >= 0.0.");
                            ErrorsFound = true;
                        }
                    }
                }

                if (lAlphaFieldBlanks(6)) {
                    ZoneITEq(Loop).CPULoadSchedPtr = ScheduleAlwaysOn;
                } else {
                    ZoneITEq(Loop).CPULoadSchedPtr = GetScheduleIndex(state, AlphaName(6));
                }
                SchMin = 0.0;
                SchMax = 0.0;
                if (ZoneITEq(Loop).CPULoadSchedPtr == 0) {
                    ShowSevereError(RoutineName + CurrentModuleObject + "=\"" + AlphaName(1) + "\", invalid " + cAlphaFieldNames(6) +
                                    " entered=" + AlphaName(6));
                    ErrorsFound = true;
                } else { // check min/max on schedule
                    SchMin = GetScheduleMinValue(ZoneITEq(Loop).CPULoadSchedPtr);
                    SchMax = GetScheduleMaxValue(ZoneITEq(Loop).CPULoadSchedPtr);
                    if (SchMin < 0.0 || SchMax < 0.0) {
                        if (SchMin < 0.0) {
                            ShowSevereError(RoutineName + CurrentModuleObject + "=\"" + AlphaName(1) + "\", " + cAlphaFieldNames(6) +
                                            ", minimum is < 0.0");
                            ShowContinueError("Schedule=\"" + AlphaName(6) + "\". Minimum is [" + RoundSigDigits(SchMin, 1) +
                                              "]. Values must be >= 0.0.");
                            ErrorsFound = true;
                        }
                        if (SchMax < 0.0) {
                            ShowSevereError(RoutineName + CurrentModuleObject + "=\"" + AlphaName(1) + "\", " + cAlphaFieldNames(6) +
                                            ", maximum is < 0.0");
                            ShowContinueError("Schedule=\"" + AlphaName(6) + "\". Maximum is [" + RoundSigDigits(SchMax, 1) +
                                              "]. Values must be >= 0.0.");
                            ErrorsFound = true;
                        }
                    }
                }

                // Calculate nominal min/max equipment level
                ZoneITEq(Loop).NomMinDesignLevel = ZoneITEq(Loop).DesignTotalPower * SchMin;
                ZoneITEq(Loop).NomMaxDesignLevel = ZoneITEq(Loop).DesignTotalPower * SchMax;

                ZoneITEq(Loop).DesignFanPowerFrac = IHGNumbers(4);
                ZoneITEq(Loop).DesignFanPower = ZoneITEq(Loop).DesignFanPowerFrac * ZoneITEq(Loop).DesignTotalPower;
                ZoneITEq(Loop).DesignCPUPower = (1.0 - ZoneITEq(Loop).DesignFanPowerFrac) * ZoneITEq(Loop).DesignTotalPower;
                ZoneITEq(Loop).DesignAirVolFlowRate = IHGNumbers(5) * ZoneITEq(Loop).DesignTotalPower;
                ZoneITEq(Loop).DesignTAirIn = IHGNumbers(6);
                ZoneITEq(Loop).DesignRecircFrac = IHGNumbers(7);
                ZoneITEq(Loop).DesignUPSEfficiency = IHGNumbers(8);
                ZoneITEq(Loop).UPSLossToZoneFrac = IHGNumbers(9);
                ZoneITEq(Loop).SupplyApproachTemp = IHGNumbers(10);
                ZoneITEq(Loop).ReturnApproachTemp = IHGNumbers(11);

                bool hasSupplyApproachTemp = !lNumericFieldBlanks(10);
                bool hasReturnApproachTemp = !lNumericFieldBlanks(11);

                // Performance curves
                ZoneITEq(Loop).CPUPowerFLTCurve = GetCurveIndex(state, AlphaName(7));
                if (ZoneITEq(Loop).CPUPowerFLTCurve == 0) {
                    ShowSevereError(RoutineName + CurrentModuleObject + " \"" + AlphaName(1) + "\"");
                    ShowContinueError("Invalid " + cAlphaFieldNames(7) + '=' + AlphaName(7));
                    ErrorsFound = true;
                }

                ZoneITEq(Loop).AirFlowFLTCurve = GetCurveIndex(state, AlphaName(8));
                if (ZoneITEq(Loop).AirFlowFLTCurve == 0) {
                    ShowSevereError(RoutineName + CurrentModuleObject + " \"" + AlphaName(1) + "\"");
                    ShowContinueError("Invalid " + cAlphaFieldNames(8) + '=' + AlphaName(8));
                    ErrorsFound = true;
                }

                ZoneITEq(Loop).FanPowerFFCurve = GetCurveIndex(state, AlphaName(9));
                if (ZoneITEq(Loop).FanPowerFFCurve == 0) {
                    ShowSevereError(RoutineName + CurrentModuleObject + " \"" + AlphaName(1) + "\"");
                    ShowContinueError("Invalid " + cAlphaFieldNames(9) + '=' + AlphaName(9));
                    ErrorsFound = true;
                }

                if (!lAlphaFieldBlanks(15)) {
                    // If this field isn't blank, it must point to a valid curve
                    ZoneITEq(Loop).RecircFLTCurve = GetCurveIndex(state, AlphaName(15));
                    if (ZoneITEq(Loop).RecircFLTCurve == 0) {
                        ShowSevereError(RoutineName + CurrentModuleObject + " \"" + AlphaName(1) + "\"");
                        ShowContinueError("Invalid " + cAlphaFieldNames(15) + '=' + AlphaName(15));
                        ErrorsFound = true;
                    }
                } else {
                    // If this curve is left blank, then the curve is assumed to always equal 1.0.
                    ZoneITEq(Loop).RecircFLTCurve = 0;
                }

                if (!lAlphaFieldBlanks(16)) {
                    // If this field isn't blank, it must point to a valid curve
                    ZoneITEq(Loop).UPSEfficFPLRCurve = GetCurveIndex(state, AlphaName(16));
                    if (ZoneITEq(Loop).UPSEfficFPLRCurve == 0) {
                        ShowSevereError(RoutineName + CurrentModuleObject + " \"" + AlphaName(1) + "\"");
                        ShowContinueError("Invalid " + cAlphaFieldNames(16) + '=' + AlphaName(16));
                        ErrorsFound = true;
                    }
                } else {
                    // If this curve is left blank, then the curve is assumed to always equal 1.0.
                    ZoneITEq(Loop).UPSEfficFPLRCurve = 0;
                }

                // Environmental class
                if (UtilityRoutines::SameString(AlphaName(10), "None")) {
                    ZoneITEq(Loop).Class = ITEClassNone;
                } else if (UtilityRoutines::SameString(AlphaName(10), "A1")) {
                    ZoneITEq(Loop).Class = ITEClassA1;
                } else if (UtilityRoutines::SameString(AlphaName(10), "A2")) {
                    ZoneITEq(Loop).Class = ITEClassA2;
                } else if (UtilityRoutines::SameString(AlphaName(10), "A3")) {
                    ZoneITEq(Loop).Class = ITEClassA3;
                } else if (UtilityRoutines::SameString(AlphaName(10), "A4")) {
                    ZoneITEq(Loop).Class = ITEClassA4;
                } else if (UtilityRoutines::SameString(AlphaName(10), "B")) {
                    ZoneITEq(Loop).Class = ITEClassB;
                } else if (UtilityRoutines::SameString(AlphaName(10), "C")) {
                    ZoneITEq(Loop).Class = ITEClassC;
                } else {
                    ShowSevereError(RoutineName + CurrentModuleObject + ": " + AlphaName(1));
                    ShowContinueError("Invalid " + cAlphaFieldNames(10) + '=' + AlphaName(10));
                    ShowContinueError("Valid entries are None, A1, A2, A3, A4, B or C.");
                    ErrorsFound = true;
                }

                // Air and supply inlet connections
                if (UtilityRoutines::SameString(AlphaName(11), "AdjustedSupply")) {
                    ZoneITEq(Loop).AirConnectionType = ITEInletAdjustedSupply;
                } else if (UtilityRoutines::SameString(AlphaName(11), "ZoneAirNode")) {
                    ZoneITEq(Loop).AirConnectionType = ITEInletZoneAirNode;
                } else if (UtilityRoutines::SameString(AlphaName(11), "RoomAirModel")) {
                    // ZoneITEq( Loop ).AirConnectionType = ITEInletRoomAirModel;
                    ShowWarningError(RoutineName + CurrentModuleObject + "=\"" + AlphaName(1) +
                                     "Air Inlet Connection Type = RoomAirModel is not implemented yet, using ZoneAirNode");
                    ZoneITEq(Loop).AirConnectionType = ITEInletZoneAirNode;
                } else {
                    ShowSevereError(RoutineName + CurrentModuleObject + ": " + AlphaName(1));
                    ShowContinueError("Invalid " + cAlphaFieldNames(11) + '=' + AlphaName(11));
                    ShowContinueError("Valid entries are AdjustedSupply, ZoneAirNode, or RoomAirModel.");
                    ErrorsFound = true;
                }
                if (lAlphaFieldBlanks(14)) {
                    if (ZoneITEq(Loop).AirConnectionType == ITEInletAdjustedSupply) {
                        ShowSevereError(RoutineName + CurrentModuleObject + ": " + AlphaName(1));
                        ShowContinueError("For " + cAlphaFieldNames(11) + "= AdjustedSupply, " + cAlphaFieldNames(14) +
                                          " is required, but this field is blank.");
                        ErrorsFound = true;
                    } else if (ZoneITEq(Loop).FlowControlWithApproachTemps) {
                        ShowSevereError(RoutineName + CurrentModuleObject + ": " + AlphaName(1));
                        ShowContinueError("For " + cAlphaFieldNames(3) + "= FlowControlWithApproachTemperatures, " + cAlphaFieldNames(14) +
                                          " is required, but this field is blank.");
                        ErrorsFound = true;
                    }
                } else {
                    ZoneITEq(Loop).SupplyAirNodeNum = GetOnlySingleNode(state,
                        AlphaName(14), ErrorsFound, CurrentModuleObject, AlphaName(1), NodeType_Air, NodeConnectionType_Sensor, 1, ObjectIsNotParent);
                }

                // check supply air node for matches with zone equipment supply air node
                int zoneEqIndex = DataZoneEquipment::GetControlledZoneIndex(state, Zone(ZoneITEq(Loop).ZonePtr).Name);
                auto itStart = DataZoneEquipment::ZoneEquipConfig(zoneEqIndex).InletNode.begin();
                auto itEnd = DataZoneEquipment::ZoneEquipConfig(zoneEqIndex).InletNode.end();
                auto key = ZoneITEq(Loop).SupplyAirNodeNum;
                bool supplyNodeFound = false;
                if (std::find(itStart, itEnd, key) != itEnd) {
                    supplyNodeFound = true;
                }

                if (ZoneITEq(Loop).AirConnectionType == ITEInletAdjustedSupply && !supplyNodeFound) {
                    // supply air node must match zone equipment supply air node for these conditions
                    ShowSevereError(RoutineName + ": ElectricEquipment:ITE:AirCooled " + ZoneITEq(Loop).Name);
                    ShowContinueError("Air Inlet Connection Type = AdjustedSupply but no Supply Air Node is specified.");
                    ErrorsFound = true;
                } else if (ZoneITEq(Loop).FlowControlWithApproachTemps && !supplyNodeFound) {
                    // supply air node must match zone equipment supply air node for these conditions
                    ShowSevereError(RoutineName + ": ElectricEquipment:ITE:AirCooled " + ZoneITEq(Loop).Name);
                    ShowContinueError("Air Inlet Connection Type = AdjustedSupply but no Supply Air Node is specified.");
                    ErrorsFound = true;
                } else if (ZoneITEq(Loop).SupplyAirNodeNum != 0 && !supplyNodeFound) {
                    // the given supply air node does not match any zone equipment supply air nodes
                    ShowWarningError(CurrentModuleObject + "name: '" + AlphaName(1) + ". " + "Supply Air Node Name '" + AlphaName(14) + "' does not match any ZoneHVAC:EquipmentConnections objects.");
                }

                // End-Use subcategories
                if (NumAlpha > 16) {
                    ZoneITEq(Loop).EndUseSubcategoryCPU = AlphaName(17);
                } else {
                    ZoneITEq(Loop).EndUseSubcategoryCPU = "ITE-CPU";
                }

                if (NumAlpha > 17) {
                    ZoneITEq(Loop).EndUseSubcategoryFan = AlphaName(18);
                } else {
                    ZoneITEq(Loop).EndUseSubcategoryFan = "ITE-Fans";
                }
                if (ZoneITEq(Loop).ZonePtr <= 0) continue; // Error, will be caught and terminated later

                if (NumAlpha > 18) {
                    ZoneITEq(Loop).EndUseSubcategoryUPS = AlphaName(19);
                } else {
                    ZoneITEq(Loop).EndUseSubcategoryUPS = "ITE-UPS";
                }
                if (ZoneITEq(Loop).FlowControlWithApproachTemps) {
                    if (!lAlphaFieldBlanks(20)) {
                        ZoneITEq(Loop).SupplyApproachTempSch = GetScheduleIndex(state, AlphaName(20));
                        if (ZoneITEq(Loop).SupplyApproachTempSch == 0) {
                            ShowSevereError(RoutineName + CurrentModuleObject + "=\"" + AlphaName(1) + "\", invalid " + cAlphaFieldNames(20) +
                                            " entered=" + AlphaName(20));
                            ErrorsFound = true;
                        }
                    } else {
                        if (!hasSupplyApproachTemp) {
                            ShowSevereError(RoutineName + CurrentModuleObject + " \"" + AlphaName(1) + "\"");
                            ShowContinueError("For " + cAlphaFieldNames(3) + "= FlowControlWithApproachTemperatures, either " +
                                              cNumericFieldNames(10) + " or " + cAlphaFieldNames(20) + " is required, but both are left blank.");
                            ErrorsFound = true;
                        }
                    }

                    if (!lAlphaFieldBlanks(21)) {
                        ZoneITEq(Loop).ReturnApproachTempSch = GetScheduleIndex(state, AlphaName(21));
                        if (ZoneITEq(Loop).ReturnApproachTempSch == 0) {
                            ShowSevereError(RoutineName + CurrentModuleObject + "=\"" + AlphaName(1) + "\", invalid " + cAlphaFieldNames(20) +
                                            " entered=" + AlphaName(20));
                            ErrorsFound = true;
                        }
                    } else {
                        if (!hasReturnApproachTemp) {
                            ShowSevereError(RoutineName + CurrentModuleObject + " \"" + AlphaName(1) + "\"");
                            ShowContinueError("For " + cAlphaFieldNames(3) + "= FlowControlWithApproachTemperatures, either " +
                                              cNumericFieldNames(11) + " or " + cAlphaFieldNames(21) + " is required, but both are left blank.");
                            ErrorsFound = true;
                        }
                    }
                }

                if (ZoneITEq(Loop).FlowControlWithApproachTemps) {
                    Real64 TAirInSizing = 0.0;
                    // Set the TAirInSizing to the maximun setpoint value to do sizing based on the maximum fan and cpu power of the ite object
                    SetPointManager::GetSetPointManagerInputData(state, ErrorsFound);
                    for (int SetPtMgrNum = 1; SetPtMgrNum <= SetPointManager::NumSZClSetPtMgrs; ++SetPtMgrNum) {
                        if (SetPointManager::SingZoneClSetPtMgr(SetPtMgrNum).ControlZoneNum == Loop) {
                            TAirInSizing = SetPointManager::SingZoneClSetPtMgr(SetPtMgrNum).MaxSetTemp;
                        }
                    }

                    ZoneITEq(Loop).SizingTAirIn = max(TAirInSizing, ZoneITEq(Loop).DesignTAirIn);
                }

                // Object report variables
                SetupOutputVariable(state,
                    "ITE CPU Electricity Rate", OutputProcessor::Unit::W, ZoneITEq(Loop).CPUPower, "Zone", "Average", ZoneITEq(Loop).Name);
                SetupOutputVariable(state,
                    "ITE Fan Electricity Rate", OutputProcessor::Unit::W, ZoneITEq(Loop).FanPower, "Zone", "Average", ZoneITEq(Loop).Name);
                SetupOutputVariable(state,
                    "ITE UPS Electricity Rate", OutputProcessor::Unit::W, ZoneITEq(Loop).UPSPower, "Zone", "Average", ZoneITEq(Loop).Name);
                SetupOutputVariable(state, "ITE CPU Electricity Rate at Design Inlet Conditions",
                                    OutputProcessor::Unit::W,
                                    ZoneITEq(Loop).CPUPowerAtDesign,
                                    "Zone",
                                    "Average",
                                    ZoneITEq(Loop).Name);
                SetupOutputVariable(state, "ITE Fan Electricity Rate at Design Inlet Conditions",
                                    OutputProcessor::Unit::W,
                                    ZoneITEq(Loop).FanPowerAtDesign,
                                    "Zone",
                                    "Average",
                                    ZoneITEq(Loop).Name);
                SetupOutputVariable(state, "ITE UPS Heat Gain to Zone Rate",
                                    OutputProcessor::Unit::W,
                                    ZoneITEq(Loop).UPSGainRateToZone,
                                    "Zone",
                                    "Average",
                                    ZoneITEq(Loop).Name);
                SetupOutputVariable(state, "ITE Total Heat Gain to Zone Rate",
                                    OutputProcessor::Unit::W,
                                    ZoneITEq(Loop).ConGainRateToZone,
                                    "Zone",
                                    "Average",
                                    ZoneITEq(Loop).Name);

                SetupOutputVariable(state, "ITE CPU Electricity Energy",
                                    OutputProcessor::Unit::J,
                                    ZoneITEq(Loop).CPUConsumption,
                                    "Zone",
                                    "Sum",
                                    ZoneITEq(Loop).Name,
                                    _,
                                    "Electricity",
                                    "InteriorEquipment",
                                    ZoneITEq(Loop).EndUseSubcategoryCPU,
                                    "Building",
                                    Zone(ZoneITEq(Loop).ZonePtr).Name,
                                    Zone(ZoneITEq(Loop).ZonePtr).Multiplier,
                                    Zone(ZoneITEq(Loop).ZonePtr).ListMultiplier);
                SetupOutputVariable(state, "ITE Fan Electricity Energy",
                                    OutputProcessor::Unit::J,
                                    ZoneITEq(Loop).FanConsumption,
                                    "Zone",
                                    "Sum",
                                    ZoneITEq(Loop).Name,
                                    _,
                                    "Electricity",
                                    "InteriorEquipment",
                                    ZoneITEq(Loop).EndUseSubcategoryFan,
                                    "Building",
                                    Zone(ZoneITEq(Loop).ZonePtr).Name,
                                    Zone(ZoneITEq(Loop).ZonePtr).Multiplier,
                                    Zone(ZoneITEq(Loop).ZonePtr).ListMultiplier);
                SetupOutputVariable(state, "ITE UPS Electricity Energy",
                                    OutputProcessor::Unit::J,
                                    ZoneITEq(Loop).UPSConsumption,
                                    "Zone",
                                    "Sum",
                                    ZoneITEq(Loop).Name,
                                    _,
                                    "Electricity",
                                    "InteriorEquipment",
                                    ZoneITEq(Loop).EndUseSubcategoryUPS,
                                    "Building",
                                    Zone(ZoneITEq(Loop).ZonePtr).Name,
                                    Zone(ZoneITEq(Loop).ZonePtr).Multiplier,
                                    Zone(ZoneITEq(Loop).ZonePtr).ListMultiplier);
                SetupOutputVariable(state, "ITE CPU Electricity Energy at Design Inlet Conditions",
                                    OutputProcessor::Unit::J,
                                    ZoneITEq(Loop).CPUEnergyAtDesign,
                                    "Zone",
                                    "Sum",
                                    ZoneITEq(Loop).Name);
                SetupOutputVariable(state, "ITE Fan Electricity Energy at Design Inlet Conditions",
                                    OutputProcessor::Unit::J,
                                    ZoneITEq(Loop).FanEnergyAtDesign,
                                    "Zone",
                                    "Sum",
                                    ZoneITEq(Loop).Name);
                SetupOutputVariable(state, "ITE UPS Heat Gain to Zone Energy",
                                    OutputProcessor::Unit::J,
                                    ZoneITEq(Loop).UPSGainEnergyToZone,
                                    "Zone",
                                    "Sum",
                                    ZoneITEq(Loop).Name);
                SetupOutputVariable(state, "ITE Total Heat Gain to Zone Energy",
                                    OutputProcessor::Unit::J,
                                    ZoneITEq(Loop).ConGainEnergyToZone,
                                    "Zone",
                                    "Sum",
                                    ZoneITEq(Loop).Name);

                SetupOutputVariable(state, "ITE Standard Density Air Volume Flow Rate",
                                    OutputProcessor::Unit::m3_s,
                                    ZoneITEq(Loop).AirVolFlowStdDensity,
                                    "Zone",
                                    "Average",
                                    ZoneITEq(Loop).Name);
                SetupOutputVariable(state, "ITE Current Density Air Volume Flow Rate",
                                    OutputProcessor::Unit::m3_s,
                                    ZoneITEq(Loop).AirVolFlowCurDensity,
                                    "Zone",
                                    "Average",
                                    ZoneITEq(Loop).Name);
                SetupOutputVariable(state,
                    "ITE Air Mass Flow Rate", OutputProcessor::Unit::kg_s, ZoneITEq(Loop).AirMassFlow, "Zone", "Average", ZoneITEq(Loop).Name);
                SetupOutputVariable(state, "ITE Air Inlet Dry-Bulb Temperature",
                                    OutputProcessor::Unit::C,
                                    ZoneITEq(Loop).AirInletDryBulbT,
                                    "Zone",
                                    "Average",
                                    ZoneITEq(Loop).Name);
                SetupOutputVariable(state, "ITE Air Inlet Dewpoint Temperature",
                                    OutputProcessor::Unit::C,
                                    ZoneITEq(Loop).AirInletDewpointT,
                                    "Zone",
                                    "Average",
                                    ZoneITEq(Loop).Name);
                SetupOutputVariable(state, "ITE Air Inlet Relative Humidity",
                                    OutputProcessor::Unit::Perc,
                                    ZoneITEq(Loop).AirInletRelHum,
                                    "Zone",
                                    "Average",
                                    ZoneITEq(Loop).Name);
                SetupOutputVariable(state, "ITE Air Outlet Dry-Bulb Temperature",
                                    OutputProcessor::Unit::C,
                                    ZoneITEq(Loop).AirOutletDryBulbT,
                                    "Zone",
                                    "Average",
                                    ZoneITEq(Loop).Name);
                if (ZoneITEq(Loop).SupplyAirNodeNum != 0) {
                    SetupOutputVariable(state,
                        "ITE Supply Heat Index", OutputProcessor::Unit::None, ZoneITEq(Loop).SHI, "Zone", "Average", ZoneITEq(Loop).Name);
                }
                SetupOutputVariable(state, "ITE Air Inlet Operating Range Exceeded Time",
                                    OutputProcessor::Unit::hr,
                                    ZoneITEq(Loop).TimeOutOfOperRange,
                                    "Zone",
                                    "Sum",
                                    ZoneITEq(Loop).Name);
                SetupOutputVariable(state, "ITE Air Inlet Dry-Bulb Temperature Above Operating Range Time",
                                    OutputProcessor::Unit::hr,
                                    ZoneITEq(Loop).TimeAboveDryBulbT,
                                    "Zone",
                                    "Sum",
                                    ZoneITEq(Loop).Name);
                SetupOutputVariable(state, "ITE Air Inlet Dry-Bulb Temperature Below Operating Range Time",
                                    OutputProcessor::Unit::hr,
                                    ZoneITEq(Loop).TimeBelowDryBulbT,
                                    "Zone",
                                    "Sum",
                                    ZoneITEq(Loop).Name);
                SetupOutputVariable(state, "ITE Air Inlet Dewpoint Temperature Above Operating Range Time",
                                    OutputProcessor::Unit::hr,
                                    ZoneITEq(Loop).TimeAboveDewpointT,
                                    "Zone",
                                    "Sum",
                                    ZoneITEq(Loop).Name);
                SetupOutputVariable(state, "ITE Air Inlet Dewpoint Temperature Below Operating Range Time",
                                    OutputProcessor::Unit::hr,
                                    ZoneITEq(Loop).TimeBelowDewpointT,
                                    "Zone",
                                    "Sum",
                                    ZoneITEq(Loop).Name);
                SetupOutputVariable(state, "ITE Air Inlet Relative Humidity Above Operating Range Time",
                                    OutputProcessor::Unit::hr,
                                    ZoneITEq(Loop).TimeAboveRH,
                                    "Zone",
                                    "Sum",
                                    ZoneITEq(Loop).Name);
                SetupOutputVariable(state, "ITE Air Inlet Relative Humidity Below Operating Range Time",
                                    OutputProcessor::Unit::hr,
                                    ZoneITEq(Loop).TimeBelowRH,
                                    "Zone",
                                    "Sum",
                                    ZoneITEq(Loop).Name);
                SetupOutputVariable(state, "ITE Air Inlet Dry-Bulb Temperature Difference Above Operating Range",
                                    OutputProcessor::Unit::deltaC,
                                    ZoneITEq(Loop).DryBulbTAboveDeltaT,
                                    "Zone",
                                    "Average",
                                    ZoneITEq(Loop).Name);
                SetupOutputVariable(state, "ITE Air Inlet Dry-Bulb Temperature Difference Below Operating Range",
                                    OutputProcessor::Unit::deltaC,
                                    ZoneITEq(Loop).DryBulbTBelowDeltaT,
                                    "Zone",
                                    "Average",
                                    ZoneITEq(Loop).Name);
                SetupOutputVariable(state, "ITE Air Inlet Dewpoint Temperature Difference Above Operating Range",
                                    OutputProcessor::Unit::deltaC,
                                    ZoneITEq(Loop).DewpointTAboveDeltaT,
                                    "Zone",
                                    "Average",
                                    ZoneITEq(Loop).Name);
                SetupOutputVariable(state, "ITE Air Inlet Dewpoint Temperature Difference Below Operating Range",
                                    OutputProcessor::Unit::deltaC,
                                    ZoneITEq(Loop).DewpointTBelowDeltaT,
                                    "Zone",
                                    "Average",
                                    ZoneITEq(Loop).Name);
                SetupOutputVariable(state, "ITE Air Inlet Relative Humidity Difference Above Operating Range",
                                    OutputProcessor::Unit::Perc,
                                    ZoneITEq(Loop).RHAboveDeltaRH,
                                    "Zone",
                                    "Average",
                                    ZoneITEq(Loop).Name);
                SetupOutputVariable(state, "ITE Air Inlet Relative Humidity Difference Below Operating Range",
                                    OutputProcessor::Unit::Perc,
                                    ZoneITEq(Loop).RHBelowDeltaRH,
                                    "Zone",
                                    "Average",
                                    ZoneITEq(Loop).Name);

                // Zone total report variables
                if (RepVarSet(ZoneITEq(Loop).ZonePtr)) {
                    RepVarSet(ZoneITEq(Loop).ZonePtr) = false;
                    SetupOutputVariable(state, "Zone ITE CPU Electricity Rate",
                                        OutputProcessor::Unit::W,
                                        ZnRpt(ZoneITEq(Loop).ZonePtr).ITEqCPUPower,
                                        "Zone",
                                        "Average",
                                        Zone(ZoneITEq(Loop).ZonePtr).Name);
                    SetupOutputVariable(state, "Zone ITE Fan Electricity Rate",
                                        OutputProcessor::Unit::W,
                                        ZnRpt(ZoneITEq(Loop).ZonePtr).ITEqFanPower,
                                        "Zone",
                                        "Average",
                                        Zone(ZoneITEq(Loop).ZonePtr).Name);
                    SetupOutputVariable(state, "Zone ITE UPS Electricity Rate",
                                        OutputProcessor::Unit::W,
                                        ZnRpt(ZoneITEq(Loop).ZonePtr).ITEqUPSPower,
                                        "Zone",
                                        "Average",
                                        Zone(ZoneITEq(Loop).ZonePtr).Name);
                    SetupOutputVariable(state, "Zone ITE CPU Electricity Rate at Design Inlet Conditions",
                                        OutputProcessor::Unit::W,
                                        ZnRpt(ZoneITEq(Loop).ZonePtr).ITEqCPUPowerAtDesign,
                                        "Zone",
                                        "Average",
                                        Zone(ZoneITEq(Loop).ZonePtr).Name);
                    SetupOutputVariable(state, "Zone ITE Fan Electricity Rate at Design Inlet Conditions",
                                        OutputProcessor::Unit::W,
                                        ZnRpt(ZoneITEq(Loop).ZonePtr).ITEqFanPowerAtDesign,
                                        "Zone",
                                        "Average",
                                        Zone(ZoneITEq(Loop).ZonePtr).Name);
                    SetupOutputVariable(state, "Zone ITE UPS Heat Gain to Zone Rate",
                                        OutputProcessor::Unit::W,
                                        ZnRpt(ZoneITEq(Loop).ZonePtr).ITEqUPSGainRateToZone,
                                        "Zone",
                                        "Average",
                                        Zone(ZoneITEq(Loop).ZonePtr).Name);
                    SetupOutputVariable(state, "Zone ITE Total Heat Gain to Zone Rate",
                                        OutputProcessor::Unit::W,
                                        ZnRpt(ZoneITEq(Loop).ZonePtr).ITEqConGainRateToZone,
                                        "Zone",
                                        "Average",
                                        Zone(ZoneITEq(Loop).ZonePtr).Name);
                    SetupOutputVariable(state, "Zone ITE Adjusted Return Air Temperature",
                                        OutputProcessor::Unit::W,
                                        ZnRpt(ZoneITEq(Loop).ZonePtr).ITEAdjReturnTemp,
                                        "Zone",
                                        "Average",
                                        Zone(ZoneITEq(Loop).ZonePtr).Name);

                    SetupOutputVariable(state, "Zone ITE CPU Electricity Energy",
                                        OutputProcessor::Unit::J,
                                        ZnRpt(ZoneITEq(Loop).ZonePtr).ITEqCPUConsumption,
                                        "Zone",
                                        "Sum",
                                        Zone(ZoneITEq(Loop).ZonePtr).Name);
                    SetupOutputVariable(state, "Zone ITE Fan Electricity Energy",
                                        OutputProcessor::Unit::J,
                                        ZnRpt(ZoneITEq(Loop).ZonePtr).ITEqFanConsumption,
                                        "Zone",
                                        "Sum",
                                        Zone(ZoneITEq(Loop).ZonePtr).Name);
                    SetupOutputVariable(state, "Zone ITE UPS Electricity Energy",
                                        OutputProcessor::Unit::J,
                                        ZnRpt(ZoneITEq(Loop).ZonePtr).ITEqUPSConsumption,
                                        "Zone",
                                        "Sum",
                                        Zone(ZoneITEq(Loop).ZonePtr).Name);
                    SetupOutputVariable(state, "Zone ITE CPU Electricity Energy at Design Inlet Conditions",
                                        OutputProcessor::Unit::J,
                                        ZnRpt(ZoneITEq(Loop).ZonePtr).ITEqCPUEnergyAtDesign,
                                        "Zone",
                                        "Sum",
                                        Zone(ZoneITEq(Loop).ZonePtr).Name);
                    SetupOutputVariable(state, "Zone ITE Fan Electricity Energy at Design Inlet Conditions",
                                        OutputProcessor::Unit::J,
                                        ZnRpt(ZoneITEq(Loop).ZonePtr).ITEqFanEnergyAtDesign,
                                        "Zone",
                                        "Sum",
                                        Zone(ZoneITEq(Loop).ZonePtr).Name);
                    SetupOutputVariable(state, "Zone ITE UPS Heat Gain to Zone Energy",
                                        OutputProcessor::Unit::J,
                                        ZnRpt(ZoneITEq(Loop).ZonePtr).ITEqUPSGainEnergyToZone,
                                        "Zone",
                                        "Sum",
                                        Zone(ZoneITEq(Loop).ZonePtr).Name);
                    SetupOutputVariable(state, "Zone ITE Total Heat Gain to Zone Energy",
                                        OutputProcessor::Unit::J,
                                        ZnRpt(ZoneITEq(Loop).ZonePtr).ITEqConGainEnergyToZone,
                                        "Zone",
                                        "Sum",
                                        Zone(ZoneITEq(Loop).ZonePtr).Name);

                    SetupOutputVariable(state, "Zone ITE Standard Density Air Volume Flow Rate",
                                        OutputProcessor::Unit::m3_s,
                                        ZnRpt(ZoneITEq(Loop).ZonePtr).ITEqAirVolFlowStdDensity,
                                        "Zone",
                                        "Average",
                                        Zone(ZoneITEq(Loop).ZonePtr).Name);
                    SetupOutputVariable(state, "Zone ITE Air Mass Flow Rate",
                                        OutputProcessor::Unit::kg_s,
                                        ZnRpt(ZoneITEq(Loop).ZonePtr).ITEqAirMassFlow,
                                        "Zone",
                                        "Average",
                                        Zone(ZoneITEq(Loop).ZonePtr).Name);
                    SetupOutputVariable(state, "Zone ITE Average Supply Heat Index",
                                        OutputProcessor::Unit::None,
                                        ZnRpt(ZoneITEq(Loop).ZonePtr).ITEqSHI,
                                        "Zone",
                                        "Average",
                                        Zone(ZoneITEq(Loop).ZonePtr).Name);
                    SetupOutputVariable(state, "Zone ITE Any Air Inlet Operating Range Exceeded Time",
                                        OutputProcessor::Unit::hr,
                                        ZnRpt(ZoneITEq(Loop).ZonePtr).ITEqTimeOutOfOperRange,
                                        "Zone",
                                        "Sum",
                                        Zone(ZoneITEq(Loop).ZonePtr).Name);
                    SetupOutputVariable(state, "Zone ITE Any Air Inlet Dry-Bulb Temperature Above Operating Range Time",
                                        OutputProcessor::Unit::hr,
                                        ZnRpt(ZoneITEq(Loop).ZonePtr).ITEqTimeAboveDryBulbT,
                                        "Zone",
                                        "Sum",
                                        Zone(ZoneITEq(Loop).ZonePtr).Name);
                    SetupOutputVariable(state, "Zone ITE Any Air Inlet Dry-Bulb Temperature Below Operating Range Time",
                                        OutputProcessor::Unit::hr,
                                        ZnRpt(ZoneITEq(Loop).ZonePtr).ITEqTimeBelowDryBulbT,
                                        "Zone",
                                        "Sum",
                                        Zone(ZoneITEq(Loop).ZonePtr).Name);
                    SetupOutputVariable(state, "Zone ITE Any Air Inlet Dewpoint Temperature Above Operating Range Time",
                                        OutputProcessor::Unit::hr,
                                        ZnRpt(ZoneITEq(Loop).ZonePtr).ITEqTimeAboveDewpointT,
                                        "Zone",
                                        "Sum",
                                        Zone(ZoneITEq(Loop).ZonePtr).Name);
                    SetupOutputVariable(state, "Zone ITE Any Air Inlet Dewpoint Temperature Below Operating Range Time",
                                        OutputProcessor::Unit::hr,
                                        ZnRpt(ZoneITEq(Loop).ZonePtr).ITEqTimeBelowDewpointT,
                                        "Zone",
                                        "Sum",
                                        Zone(ZoneITEq(Loop).ZonePtr).Name);
                    SetupOutputVariable(state, "Zone ITE Any Air Inlet Relative Humidity Above Operating Range Time",
                                        OutputProcessor::Unit::hr,
                                        ZnRpt(ZoneITEq(Loop).ZonePtr).ITEqTimeAboveRH,
                                        "Zone",
                                        "Sum",
                                        Zone(ZoneITEq(Loop).ZonePtr).Name);
                    SetupOutputVariable(state, "Zone ITE Any Air Inlet Relative Humidity Below Operating Range Time",
                                        OutputProcessor::Unit::hr,
                                        ZnRpt(ZoneITEq(Loop).ZonePtr).ITEqTimeBelowRH,
                                        "Zone",
                                        "Sum",
                                        Zone(ZoneITEq(Loop).ZonePtr).Name);
                }

                // MJW - EMS Not in place yet
                // if ( AnyEnergyManagementSystemInModel ) {
                // SetupEMSActuator( "ElectricEquipment", ZoneITEq( Loop ).Name, "Electric Power Level", "[W]", ZoneITEq( Loop
                // ).EMSZoneEquipOverrideOn, ZoneITEq( Loop ).EMSEquipPower ); SetupEMSInternalVariable( "Plug and Process Power Design Level",
                // ZoneITEq( Loop ).Name, "[W]", ZoneITEq( Loop ).DesignTotalPower ); } // EMS

                if (!ErrorsFound)
                    SetupZoneInternalGain(ZoneITEq(Loop).ZonePtr,
                                          "ElectricEquipment:ITE:AirCooled",
                                          ZoneITEq(Loop).Name,
                                          IntGainTypeOf_ElectricEquipmentITEAirCooled,
                                          &ZoneITEq(Loop).ConGainRateToZone);

            } // Item - Number of ZoneITEq objects
            for (Loop = 1; Loop <= NumZoneITEqStatements; ++Loop) {
                if (Zone(ZoneITEq(Loop).ZonePtr).HasAdjustedReturnTempByITE && (!ZoneITEq(Loop).FlowControlWithApproachTemps)) {
                    ShowSevereError(RoutineName + CurrentModuleObject + "=\"" + AlphaName(1) + "\": invalid calculation method " + AlphaName(3) +
                                    " for Zone: " + AlphaName(2));
                    ShowContinueError("...Multiple flow control methods apply to one zone. ");
                    ErrorsFound = true;
                }
            }
        } // Check on number of ZoneITEq

        RepVarSet = true;
        CurrentModuleObject = "ZoneBaseboard:OutdoorTemperatureControlled";
        TotBBHeat = inputProcessor->getNumObjectsFound(CurrentModuleObject);
        ZoneBBHeat.allocate(TotBBHeat);

        for (Loop = 1; Loop <= TotBBHeat; ++Loop) {
            AlphaName = "";
            IHGNumbers = 0.0;
            inputProcessor->getObjectItem(state,
                                          CurrentModuleObject,
                                          Loop,
                                          AlphaName,
                                          NumAlpha,
                                          IHGNumbers,
                                          NumNumber,
                                          IOStat,
                                          lNumericFieldBlanks,
                                          lAlphaFieldBlanks,
                                          cAlphaFieldNames,
                                          cNumericFieldNames);
            UtilityRoutines::IsNameEmpty(AlphaName(1), CurrentModuleObject, ErrorsFound);

            ZoneBBHeat(Loop).Name = AlphaName(1);

            ZoneBBHeat(Loop).ZonePtr = UtilityRoutines::FindItemInList(AlphaName(2), Zone);
            if (ZoneBBHeat(Loop).ZonePtr == 0) {
                ShowSevereError(RoutineName + CurrentModuleObject + "=\"" + AlphaName(1) + "\", invalid " + cAlphaFieldNames(2) +
                                " entered=" + AlphaName(2));
                ErrorsFound = true;
            }

            ZoneBBHeat(Loop).SchedPtr = GetScheduleIndex(state, AlphaName(3));
            if (ZoneBBHeat(Loop).SchedPtr == 0) {
                if (lAlphaFieldBlanks(3)) {
                    ShowSevereError(RoutineName + CurrentModuleObject + "=\"" + AlphaName(1) + "\", " + cAlphaFieldNames(3) + " is required.");
                } else {
                    ShowSevereError(RoutineName + CurrentModuleObject + "=\"" + AlphaName(1) + "\", invalid " + cAlphaFieldNames(3) +
                                    " entered=" + AlphaName(3));
                }
                ErrorsFound = true;
            } else { // check min/max on schedule
                SchMin = GetScheduleMinValue(ZoneBBHeat(Loop).SchedPtr);
                SchMax = GetScheduleMaxValue(ZoneBBHeat(Loop).SchedPtr);
                if (SchMin < 0.0 || SchMax < 0.0) {
                    if (SchMin < 0.0) {
                        ShowSevereError(RoutineName + CurrentModuleObject + "=\"" + AlphaName(1) + "\", " + cAlphaFieldNames(3) +
                                        ", minimum is < 0.0");
                        ShowContinueError("Schedule=\"" + AlphaName(3) + "\". Minimum is [" + RoundSigDigits(SchMin, 1) +
                                          "]. Values must be >= 0.0.");
                        ErrorsFound = true;
                    }
                    if (SchMax < 0.0) {
                        ShowSevereError(RoutineName + CurrentModuleObject + "=\"" + AlphaName(1) + "\", " + cAlphaFieldNames(3) +
                                        ", maximum is < 0.0");
                        ShowContinueError("Schedule=\"" + AlphaName(3) + "\". Maximum is [" + RoundSigDigits(SchMax, 1) +
                                          "]. Values must be >= 0.0.");
                        ErrorsFound = true;
                    }
                }
            }

            if (NumAlpha > 3) {
                ZoneBBHeat(Loop).EndUseSubcategory = AlphaName(4);
            } else {
                ZoneBBHeat(Loop).EndUseSubcategory = "General";
            }

            ZoneBBHeat(Loop).CapatLowTemperature = IHGNumbers(1);
            ZoneBBHeat(Loop).LowTemperature = IHGNumbers(2);
            ZoneBBHeat(Loop).CapatHighTemperature = IHGNumbers(3);
            ZoneBBHeat(Loop).HighTemperature = IHGNumbers(4);
            ZoneBBHeat(Loop).FractionRadiant = IHGNumbers(5);
            ZoneBBHeat(Loop).FractionConvected = 1.0 - ZoneBBHeat(Loop).FractionRadiant;
            if (ZoneBBHeat(Loop).FractionConvected < 0.0) {
                ShowSevereError(RoutineName + CurrentModuleObject + "=\"" + AlphaName(1) + "\", Sum of Fractions > 1.0");
                ErrorsFound = true;
            }

            if (ZoneBBHeat(Loop).ZonePtr <= 0) continue; // Error, will be caught and terminated later

            // Object report variables
            SetupOutputVariable(state,
                "Baseboard Electricity Rate", OutputProcessor::Unit::W, ZoneBBHeat(Loop).Power, "Zone", "Average", ZoneBBHeat(Loop).Name);
            SetupOutputVariable(state, "Baseboard Electricity Energy",
                                OutputProcessor::Unit::J,
                                ZoneBBHeat(Loop).Consumption,
                                "Zone",
                                "Sum",
                                ZoneBBHeat(Loop).Name,
                                _,
                                "Electricity",
                                "InteriorEquipment",
                                ZoneBBHeat(Loop).EndUseSubcategory,
                                "Building",
                                Zone(ZoneBBHeat(Loop).ZonePtr).Name,
                                Zone(ZoneBBHeat(Loop).ZonePtr).Multiplier,
                                Zone(ZoneBBHeat(Loop).ZonePtr).ListMultiplier);

            SetupOutputVariable(state,
                "Baseboard Radiant Heating Energy", OutputProcessor::Unit::J, ZoneBBHeat(Loop).RadGainEnergy, "Zone", "Sum", ZoneBBHeat(Loop).Name);
            SetupOutputVariable(state,
                "Baseboard Radiant Heating Rate", OutputProcessor::Unit::W, ZoneBBHeat(Loop).RadGainRate, "Zone", "Average", ZoneBBHeat(Loop).Name);
            SetupOutputVariable(state, "Baseboard Convective Heating Energy",
                                OutputProcessor::Unit::J,
                                ZoneBBHeat(Loop).ConGainEnergy,
                                "Zone",
                                "Sum",
                                ZoneBBHeat(Loop).Name);
            SetupOutputVariable(state, "Baseboard Convective Heating Rate",
                                OutputProcessor::Unit::W,
                                ZoneBBHeat(Loop).ConGainRate,
                                "Zone",
                                "Average",
                                ZoneBBHeat(Loop).Name);
            SetupOutputVariable(state,
                "Baseboard Total Heating Energy", OutputProcessor::Unit::J, ZoneBBHeat(Loop).TotGainEnergy, "Zone", "Sum", ZoneBBHeat(Loop).Name);
            SetupOutputVariable(state,
                "Baseboard Total Heating Rate", OutputProcessor::Unit::W, ZoneBBHeat(Loop).TotGainRate, "Zone", "Average", ZoneBBHeat(Loop).Name);

            // Zone total report variables
            if (RepVarSet(ZoneBBHeat(Loop).ZonePtr)) {
                RepVarSet(ZoneBBHeat(Loop).ZonePtr) = false;
                SetupOutputVariable(state, "Zone Baseboard Electricity Rate",
                                    OutputProcessor::Unit::W,
                                    ZnRpt(ZoneBBHeat(Loop).ZonePtr).BaseHeatPower,
                                    "Zone",
                                    "Average",
                                    Zone(ZoneBBHeat(Loop).ZonePtr).Name);
                SetupOutputVariable(state, "Zone Baseboard Electricity Energy",
                                    OutputProcessor::Unit::J,
                                    ZnRpt(ZoneBBHeat(Loop).ZonePtr).BaseHeatElecCons,
                                    "Zone",
                                    "Sum",
                                    Zone(ZoneBBHeat(Loop).ZonePtr).Name);

                SetupOutputVariable(state, "Zone Baseboard Radiant Heating Energy",
                                    OutputProcessor::Unit::J,
                                    ZnRpt(ZoneBBHeat(Loop).ZonePtr).BaseHeatRadGain,
                                    "Zone",
                                    "Sum",
                                    Zone(ZoneBBHeat(Loop).ZonePtr).Name);
                SetupOutputVariable(state, "Zone Baseboard Radiant Heating Rate",
                                    OutputProcessor::Unit::W,
                                    ZnRpt(ZoneBBHeat(Loop).ZonePtr).BaseHeatRadGainRate,
                                    "Zone",
                                    "Average",
                                    Zone(ZoneBBHeat(Loop).ZonePtr).Name);
                SetupOutputVariable(state, "Zone Baseboard Convective Heating Energy",
                                    OutputProcessor::Unit::J,
                                    ZnRpt(ZoneBBHeat(Loop).ZonePtr).BaseHeatConGain,
                                    "Zone",
                                    "Sum",
                                    Zone(ZoneBBHeat(Loop).ZonePtr).Name);
                SetupOutputVariable(state, "Zone Baseboard Convective Heating Rate",
                                    OutputProcessor::Unit::W,
                                    ZnRpt(ZoneBBHeat(Loop).ZonePtr).BaseHeatConGainRate,
                                    "Zone",
                                    "Average",
                                    Zone(ZoneBBHeat(Loop).ZonePtr).Name);
                SetupOutputVariable(state, "Zone Baseboard Total Heating Energy",
                                    OutputProcessor::Unit::J,
                                    ZnRpt(ZoneBBHeat(Loop).ZonePtr).BaseHeatTotGain,
                                    "Zone",
                                    "Sum",
                                    Zone(ZoneBBHeat(Loop).ZonePtr).Name);
                SetupOutputVariable(state, "Zone Baseboard Total Heating Rate",
                                    OutputProcessor::Unit::W,
                                    ZnRpt(ZoneBBHeat(Loop).ZonePtr).BaseHeatTotGainRate,
                                    "Zone",
                                    "Average",
                                    Zone(ZoneBBHeat(Loop).ZonePtr).Name);
            }

            if (AnyEnergyManagementSystemInModel) {
                SetupEMSActuator("ZoneBaseboard:OutdoorTemperatureControlled",
                                 ZoneBBHeat(Loop).Name,
                                 "Power Level",
                                 "[W]",
                                 ZoneBBHeat(Loop).EMSZoneBaseboardOverrideOn,
                                 ZoneBBHeat(Loop).EMSZoneBaseboardPower);
                SetupEMSInternalVariable(
                    "Simple Zone Baseboard Capacity At Low Temperature", ZoneBBHeat(Loop).Name, "[W]", ZoneBBHeat(Loop).CapatLowTemperature);
                SetupEMSInternalVariable(
                    "Simple Zone Baseboard Capacity At High Temperature", ZoneBBHeat(Loop).Name, "[W]", ZoneBBHeat(Loop).CapatHighTemperature);
            } // EMS

            SetupZoneInternalGain(ZoneBBHeat(Loop).ZonePtr,
                                  "ZoneBaseboard:OutdoorTemperatureControlled",
                                  ZoneBBHeat(Loop).Name,
                                  IntGainTypeOf_ZoneBaseboardOutdoorTemperatureControlled,
                                  &ZoneBBHeat(Loop).ConGainRate,
                                  nullptr,
                                  &ZoneBBHeat(Loop).RadGainRate);
        }

        RepVarSet = true;
        CurrentModuleObject = "ZoneContaminantSourceAndSink:CarbonDioxide";
        TotCO2Gen = inputProcessor->getNumObjectsFound(CurrentModuleObject);
        ZoneCO2Gen.allocate(TotCO2Gen);

        for (Loop = 1; Loop <= TotCO2Gen; ++Loop) {
            AlphaName = "";
            IHGNumbers = 0.0;
            inputProcessor->getObjectItem(state,
                                          CurrentModuleObject,
                                          Loop,
                                          AlphaName,
                                          NumAlpha,
                                          IHGNumbers,
                                          NumNumber,
                                          IOStat,
                                          lNumericFieldBlanks,
                                          lAlphaFieldBlanks,
                                          cAlphaFieldNames,
                                          cNumericFieldNames);
            UtilityRoutines::IsNameEmpty(AlphaName(1), CurrentModuleObject, ErrorsFound);

            ZoneCO2Gen(Loop).Name = AlphaName(1);

            ZoneCO2Gen(Loop).ZonePtr = UtilityRoutines::FindItemInList(AlphaName(2), Zone);
            if (ZoneCO2Gen(Loop).ZonePtr == 0) {
                ShowSevereError(RoutineName + CurrentModuleObject + "=\"" + AlphaName(1) + "\", invalid " + cAlphaFieldNames(2) +
                                " entered=" + AlphaName(2));
                ErrorsFound = true;
            }

            ZoneCO2Gen(Loop).SchedPtr = GetScheduleIndex(state, AlphaName(3));
            if (ZoneCO2Gen(Loop).SchedPtr == 0) {
                if (lAlphaFieldBlanks(3)) {
                    ShowSevereError(RoutineName + CurrentModuleObject + "=\"" + AlphaName(1) + "\", " + cAlphaFieldNames(3) + " is required.");
                } else {
                    ShowSevereError(RoutineName + CurrentModuleObject + "=\"" + AlphaName(1) + "\", invalid " + cAlphaFieldNames(3) +
                                    " entered=" + AlphaName(3));
                }
                ErrorsFound = true;
            } else { // check min/max on schedule
                SchMin = GetScheduleMinValue(ZoneCO2Gen(Loop).SchedPtr);
                SchMax = GetScheduleMaxValue(ZoneCO2Gen(Loop).SchedPtr);
                if (SchMin < 0.0 || SchMax < 0.0) {
                    if (SchMin < 0.0) {
                        ShowSevereError(RoutineName + CurrentModuleObject + "=\"" + AlphaName(1) + "\", " + cAlphaFieldNames(3) +
                                        ", minimum is < 0.0");
                        ShowContinueError("Schedule=\"" + AlphaName(3) + "\". Minimum is [" + RoundSigDigits(SchMin, 1) +
                                          "]. Values must be >= 0.0.");
                        ErrorsFound = true;
                    }
                    if (SchMax < 0.0) {
                        ShowSevereError(RoutineName + CurrentModuleObject + "=\"" + AlphaName(1) + "\", " + cAlphaFieldNames(3) +
                                        ", maximum is < 0.0");
                        ShowContinueError("Schedule=\"" + AlphaName(3) + "\". Maximum is [" + RoundSigDigits(SchMax, 1) +
                                          "]. Values must be >= 0.0.");
                        ErrorsFound = true;
                    }
                }
            }

            ZoneCO2Gen(Loop).CO2DesignRate = IHGNumbers(1);

            if (ZoneCO2Gen(Loop).ZonePtr <= 0) continue; // Error, will be caught and terminated later

            // Object report variables
            SetupOutputVariable(state, "Contaminant Source or Sink CO2 Gain Volume Flow Rate",
                                OutputProcessor::Unit::m3_s,
                                ZoneCO2Gen(Loop).CO2GainRate,
                                "Zone",
                                "Average",
                                ZoneCO2Gen(Loop).Name);

            // Zone total report variables
            if (RepVarSet(ZoneCO2Gen(Loop).ZonePtr)) {
                RepVarSet(ZoneCO2Gen(Loop).ZonePtr) = false;

                SetupOutputVariable(state, "Zone Contaminant Source or Sink CO2 Gain Volume Flow Rate",
                                    OutputProcessor::Unit::m3_s,
                                    ZnRpt(ZoneCO2Gen(Loop).ZonePtr).CO2Rate,
                                    "Zone",
                                    "Average",
                                    Zone(ZoneCO2Gen(Loop).ZonePtr).Name);
            }

            SetupZoneInternalGain(ZoneCO2Gen(Loop).ZonePtr,
                                  "ZoneContaminantSourceAndSink:CarbonDioxide",
                                  ZoneCO2Gen(Loop).Name,
                                  IntGainTypeOf_ZoneContaminantSourceAndSinkCarbonDioxide,
                                  nullptr,
                                  nullptr,
                                  nullptr,
                                  nullptr,
                                  nullptr,
                                  &ZoneCO2Gen(Loop).CO2GainRate);
        }

        RepVarSet.deallocate();
        IHGNumbers.deallocate();
        AlphaName.deallocate();

        if (ErrorsFound) {
            ShowFatalError(RoutineName + "Errors found in Getting Internal Gains Input, Program Stopped");
        }

        static constexpr auto Format_721(
            "! <Zone Internal Gains Nominal>,Zone Name, Floor Area {{m2}},# Occupants,Area per Occupant "
            "{{m2/person}},Occupant per Area {{person/m2}},Interior Lighting {{W/m2}},Electric Load {{W/m2}},Gas Load {{W/m2}},Other "
            "Load {{W/m2}},Hot Water Eq {{W/m2}},Steam Equipment {{W/m2}},Sum Loads per Area {{W/m2}},Outdoor Controlled Baseboard "
            "Heat\n");

        print(state.files.eio, Format_721);
        for (Loop = 1; Loop <= NumOfZones; ++Loop) {
            LightTot = 0.0;
            ElecTot = 0.0;
            GasTot = 0.0;
            OthTot = 0.0;
            HWETot = 0.0;
            StmTot = 0.0;
            BBHeatInd = "No";
            for (Loop1 = 1; Loop1 <= TotLights; ++Loop1) {
                if (Lights(Loop1).ZonePtr != Loop) continue;
                LightTot += Lights(Loop1).DesignLevel;
            }
            for (Loop1 = 1; Loop1 <= TotElecEquip; ++Loop1) {
                if (ZoneElectric(Loop1).ZonePtr != Loop) continue;
                ElecTot += ZoneElectric(Loop1).DesignLevel;
            }
            for (Loop1 = 1; Loop1 <= NumZoneITEqStatements; ++Loop1) {
                if (ZoneITEq(Loop1).ZonePtr != Loop) continue;
                ElecTot += ZoneITEq(Loop1).DesignTotalPower;
            }
            for (Loop1 = 1; Loop1 <= TotGasEquip; ++Loop1) {
                if (ZoneGas(Loop1).ZonePtr != Loop) continue;
                GasTot += ZoneGas(Loop1).DesignLevel;
            }
            for (Loop1 = 1; Loop1 <= TotOthEquip; ++Loop1) {
                if (ZoneOtherEq(Loop1).ZonePtr != Loop) continue;
                OthTot += ZoneOtherEq(Loop1).DesignLevel;
            }
            for (Loop1 = 1; Loop1 <= TotStmEquip; ++Loop1) {
                if (ZoneSteamEq(Loop1).ZonePtr != Loop) continue;
                StmTot += ZoneSteamEq(Loop1).DesignLevel;
            }
            for (Loop1 = 1; Loop1 <= TotHWEquip; ++Loop1) {
                if (ZoneHWEq(Loop1).ZonePtr != Loop) continue;
                HWETot += ZoneHWEq(Loop1).DesignLevel;
            }
            for (Loop1 = 1; Loop1 <= TotBBHeat; ++Loop1) {
                if (ZoneBBHeat(Loop1).ZonePtr != Loop) continue;
                BBHeatInd = "Yes";
            }
            Zone(Loop).InternalHeatGains = LightTot + ElecTot + GasTot + OthTot + HWETot + StmTot;
            if (Zone(Loop).FloorArea > 0.0) {
                print(state.files.eio, Format_720, Zone(Loop).Name, Zone(Loop).FloorArea, Zone(Loop).TotOccupants);
                print_and_divide_if_greater_than_zero(Zone(Loop).FloorArea, Zone(Loop).TotOccupants);
                print(state.files.eio, "{:.3R},", Zone(Loop).TotOccupants / Zone(Loop).FloorArea);
                print(state.files.eio, "{:.3R},", LightTot / Zone(Loop).FloorArea);
                print(state.files.eio, "{:.3R},", ElecTot / Zone(Loop).FloorArea);
                print(state.files.eio, "{:.3R},", GasTot / Zone(Loop).FloorArea);
                print(state.files.eio, "{:.3R},", OthTot / Zone(Loop).FloorArea);
                print(state.files.eio, "{:.3R},", HWETot / Zone(Loop).FloorArea);
                print(state.files.eio, "{:.3R},", StmTot / Zone(Loop).FloorArea);
                print(state.files.eio, "{:.3R},{}\n", Zone(Loop).InternalHeatGains / Zone(Loop).FloorArea, BBHeatInd);
            } else {
                print(state.files.eio, Format_720, Zone(Loop).Name, Zone(Loop).FloorArea, Zone(Loop).TotOccupants);
                print(state.files.eio, "0.0,N/A,N/A,N/A,N/A,N/A,N/A,N/A,N/A,{}\n", BBHeatInd);
            }
        }
        for (Loop = 1; Loop <= TotPeople; ++Loop) {
            if (Loop == 1) {
                print(state.files.eio,
                      Format_723,
                      "People",
                      "Number of People {},People/Floor Area {person/m2},Floor Area per person {m2/person},Fraction Radiant,Fraction "
                      "Convected,Sensible Fraction Calculation,Activity level,ASHRAE 55 Warnings,Carbon Dioxide Generation Rate,Nominal Minimum "
                      "Number of People,Nominal Maximum Number of People");
                if (People(Loop).Fanger || People(Loop).Pierce || People(Loop).KSU) {
                    print(state.files.eio,
                          ",MRT Calculation Type,Work Efficiency, Clothing Insulation Calculation Method,Clothing "
                          "Insulation Calculation Method Schedule,Clothing,Air Velocity,Fanger Calculation,Pierce "
                          "Calculation,KSU Calculation\n");
                } else {
                    print(state.files.eio, "\n");
                }
            }

            ZoneNum = People(Loop).ZonePtr;

            if (ZoneNum == 0) {
                print(state.files.eio, Format_724, "People-Illegal Zone specified", People(Loop).Name);
                continue;
            }

            print(state.files.eio,
                  Format_722,
                  "People",
                  People(Loop).Name,
                  GetScheduleName(state, People(Loop).NumberOfPeoplePtr),
                  Zone(ZoneNum).Name,
                  Zone(ZoneNum).FloorArea,
                  Zone(ZoneNum).TotOccupants);

            print(state.files.eio, "{:.1R},", People(Loop).NumberOfPeople);

            print_and_divide_if_greater_than_zero(People(Loop).NumberOfPeople, Zone(ZoneNum).FloorArea);

            if (People(Loop).NumberOfPeople > 0.0){
                print_and_divide_if_greater_than_zero(Zone(ZoneNum).FloorArea, People(Loop).NumberOfPeople);
            } else {
                print(state.files.eio, "N/A,");
            }

            print(state.files.eio, "{:.3R},", People(Loop).FractionRadiant);
            print(state.files.eio, "{:.3R},", People(Loop).FractionConvected);
            if (People(Loop).UserSpecSensFrac == AutoCalculate) {
                print(state.files.eio, "AutoCalculate,");
            } else {
                print(state.files.eio, "{:.3R},", People(Loop).UserSpecSensFrac);
            }
            print(state.files.eio, "{},", GetScheduleName(state, People(Loop).ActivityLevelPtr));

            if (People(Loop).Show55Warning) {
                print(state.files.eio, "Yes,");
            } else {
                print(state.files.eio, "No,");
            }
            print(state.files.eio, "{:.4R},", People(Loop).CO2RateFactor);
            print(state.files.eio, "{:.0R},", People(Loop).NomMinNumberPeople);

            if (People(Loop).Fanger || People(Loop).Pierce || People(Loop).KSU) {
                print(state.files.eio, "{:.0R},", People(Loop).NomMaxNumberPeople);

                if (People(Loop).MRTCalcType == ZoneAveraged) {
                    print(state.files.eio, "Zone Averaged,");
                } else if (People(Loop).MRTCalcType == SurfaceWeighted) {
                    print(state.files.eio, "Surface Weighted,");
                } else if (People(Loop).MRTCalcType == AngleFactor) {
                    print(state.files.eio, "Angle Factor,");
                } else {
                    print(state.files.eio, "N/A,");
                }
                print(state.files.eio, "{},", GetScheduleName(state, People(Loop).WorkEffPtr));

                if (People(Loop).ClothingType == 1) {
                    print(state.files.eio, "Clothing Insulation Schedule,");
                } else if (People(Loop).ClothingType == 2) {
                    print(state.files.eio, "Dynamic Clothing Model ASHRAE55,");
                } else if (People(Loop).ClothingType == 3) {
                    print(state.files.eio, "Calculation Method Schedule,");
                } else {
                    print(state.files.eio, "N/A,");
                }

                if (People(Loop).ClothingType == 3) {
                    print(state.files.eio, "{},", GetScheduleName(state, People(Loop).ClothingMethodPtr));
                } else {
                    print(state.files.eio, "N/A,");
                }

                print(state.files.eio, "{},", GetScheduleName(state, People(Loop).ClothingPtr));
                print(state.files.eio, "{},", GetScheduleName(state, People(Loop).AirVelocityPtr));

                if (People(Loop).Fanger) {
                    print(state.files.eio, "Yes,");
                } else {
                    print(state.files.eio, "No,");
                }
                if (People(Loop).Pierce) {
                    print(state.files.eio, "Yes,");
                } else {
                    print(state.files.eio, "No,");
                }
                if (People(Loop).KSU) {
                    print(state.files.eio, "Yes\n");
                } else {
                    print(state.files.eio, "No\n");
                }
            } else {
                print(state.files.eio, "{:.0R}\n", People(Loop).NomMaxNumberPeople);
            }
        }
        for (Loop = 1; Loop <= TotLights; ++Loop) {
            if (Loop == 1) {
                print(state.files.eio,
                      Format_723,
                      "Lights",
                      "Lighting Level {W},Lights/Floor Area {W/m2},Lights per person {W/person},Fraction Return "
                      "Air,Fraction Radiant,Fraction Short Wave,Fraction Convected,Fraction Replaceable,End-Use "
                      "Category,Nominal Minimum Lighting Level {W},Nominal Maximum Lighting Level {W}\n");
            }

            ZoneNum = Lights(Loop).ZonePtr;

            if (ZoneNum == 0) {
                print(state.files.eio, "Lights-Illegal Zone specified", Lights(Loop).Name);
                continue;
            }
            print(state.files.eio,
                  Format_722,
                  "Lights",
                  Lights(Loop).Name,
                  GetScheduleName(state, Lights(Loop).SchedPtr),
                  Zone(ZoneNum).Name,
                  Zone(ZoneNum).FloorArea,
                  Zone(ZoneNum).TotOccupants);

            print(state.files.eio, "{:.3R},", Lights(Loop).DesignLevel);

            print_and_divide_if_greater_than_zero(Lights(Loop).DesignLevel, Zone(ZoneNum).FloorArea);
            print_and_divide_if_greater_than_zero(Lights(Loop).DesignLevel, Zone(ZoneNum).TotOccupants);

            print(state.files.eio, "{:.3R},", Lights(Loop).FractionReturnAir);
            print(state.files.eio, "{:.3R},", Lights(Loop).FractionRadiant);
            print(state.files.eio, "{:.3R},", Lights(Loop).FractionShortWave);
            print(state.files.eio, "{:.3R},", Lights(Loop).FractionConvected);
            print(state.files.eio, "{:.3R},", Lights(Loop).FractionReplaceable);
            print(state.files.eio, "{},", Lights(Loop).EndUseSubcategory);
            print(state.files.eio, "{:.3R},", Lights(Loop).NomMinDesignLevel);
            print(state.files.eio, "{:.3R}\n", Lights(Loop).NomMaxDesignLevel);
        }
        for (Loop = 1; Loop <= TotElecEquip; ++Loop) {
            if (Loop == 1) {
                print(state.files.eio,
                      Format_723,
                      "ElectricEquipment",
                      "Equipment Level {W},Equipment/Floor Area {W/m2},Equipment per person {W/person},Fraction Latent,Fraction Radiant,Fraction "
                      "Lost,Fraction Convected,End-Use SubCategory,Nominal Minimum Equipment Level {W},Nominal Maximum Equipment Level {W}\n");
            }

            ZoneNum = ZoneElectric(Loop).ZonePtr;

            if (ZoneNum == 0) {
                print(state.files.eio, Format_724, "Electric Equipment-Illegal Zone specified", ZoneElectric(Loop).Name);
                continue;
            }
            print(state.files.eio,
                  Format_722,
                  "ElectricEquipment",
                  ZoneElectric(Loop).Name,
                  GetScheduleName(state, ZoneElectric(Loop).SchedPtr),
                  Zone(ZoneNum).Name,
                  Zone(ZoneNum).FloorArea,
                  Zone(ZoneNum).TotOccupants);

            print(state.files.eio, "{:.3R},", ZoneElectric(Loop).DesignLevel);

            print_and_divide_if_greater_than_zero(ZoneElectric(Loop).DesignLevel, Zone(ZoneNum).FloorArea);
            print_and_divide_if_greater_than_zero(ZoneElectric(Loop).DesignLevel, Zone(ZoneNum).TotOccupants);

            print(state.files.eio, "{:.3R},", ZoneElectric(Loop).FractionLatent);
            print(state.files.eio, "{:.3R},", ZoneElectric(Loop).FractionRadiant);
            print(state.files.eio, "{:.3R},", ZoneElectric(Loop).FractionLost);
            print(state.files.eio, "{:.3R},", ZoneElectric(Loop).FractionConvected);
            print(state.files.eio, "{},", ZoneElectric(Loop).EndUseSubcategory);
            print(state.files.eio, "{:.3R},", ZoneElectric(Loop).NomMinDesignLevel);
            print(state.files.eio, "{:.3R}\n", ZoneElectric(Loop).NomMaxDesignLevel);
        }
        for (Loop = 1; Loop <= TotGasEquip; ++Loop) {
            if (Loop == 1) {
                print(state.files.eio,
                      Format_723,
                      "GasEquipment",
                      "Equipment Level {W},Equipment/Floor Area {W/m2},Equipment per person {W/person},Fraction Latent,Fraction Radiant,Fraction "
                      "Lost,Fraction Convected,End-Use SubCategory,Nominal Minimum Equipment Level {W},Nominal Maximum Equipment Level {W}\n");
            }

            ZoneNum = ZoneGas(Loop).ZonePtr;

            if (ZoneNum == 0) {
                print(state.files.eio, Format_724, "Gas Equipment-Illegal Zone specified", ZoneGas(Loop).Name);
                continue;
            }

            print(state.files.eio,
                  Format_722,
                  "GasEquipment",
                  ZoneGas(Loop).Name,
                  GetScheduleName(state, ZoneGas(Loop).SchedPtr),
                  Zone(ZoneNum).Name,
                  Zone(ZoneNum).FloorArea,
                  Zone(ZoneNum).TotOccupants);

            print(state.files.eio, "{:.3R},", ZoneGas(Loop).DesignLevel);

            print_and_divide_if_greater_than_zero(ZoneGas(Loop).DesignLevel, Zone(ZoneNum).FloorArea);
            print_and_divide_if_greater_than_zero(ZoneGas(Loop).DesignLevel, Zone(ZoneNum).TotOccupants);

            print(state.files.eio, "{:.3R},", ZoneGas(Loop).FractionLatent);
            print(state.files.eio, "{:.3R},", ZoneGas(Loop).FractionRadiant);
            print(state.files.eio, "{:.3R},", ZoneGas(Loop).FractionLost);
            print(state.files.eio, "{:.3R},", ZoneGas(Loop).FractionConvected);
            print(state.files.eio, "{},", ZoneGas(Loop).EndUseSubcategory);
            print(state.files.eio, "{:.3R},", ZoneGas(Loop).NomMinDesignLevel);
            print(state.files.eio, "{:.3R}\n", ZoneGas(Loop).NomMaxDesignLevel);
        }

        for (Loop = 1; Loop <= TotHWEquip; ++Loop) {
            if (Loop == 1) {
                print(state.files.eio,
                      Format_723,
                      "HotWaterEquipment",
                      "Equipment Level {W},Equipment/Floor Area {W/m2},Equipment per person {W/person},Fraction Latent,Fraction Radiant,Fraction "
                      "Lost,Fraction Convected,End-Use SubCategory,Nominal Minimum Equipment Level {W},Nominal Maximum Equipment Level {W}\n");
            }

            ZoneNum = ZoneHWEq(Loop).ZonePtr;

            if (ZoneNum == 0) {
                print(state.files.eio, Format_724, "Hot Water Equipment-Illegal Zone specified", ZoneHWEq(Loop).Name);
                continue;
            }

            print(state.files.eio,
                  Format_722,
                  "HotWaterEquipment",
                  ZoneHWEq(Loop).Name,
                  GetScheduleName(state, ZoneHWEq(Loop).SchedPtr),
                  Zone(ZoneNum).Name,
                  Zone(ZoneNum).FloorArea,
                  Zone(ZoneNum).TotOccupants);

            print(state.files.eio, "{:.3R},", ZoneHWEq(Loop).DesignLevel);

            print_and_divide_if_greater_than_zero(ZoneHWEq(Loop).DesignLevel, Zone(ZoneNum).FloorArea);
            print_and_divide_if_greater_than_zero(ZoneHWEq(Loop).DesignLevel, Zone(ZoneNum).TotOccupants);

            print(state.files.eio, "{:.3R},", ZoneHWEq(Loop).FractionLatent);
            print(state.files.eio, "{:.3R},", ZoneHWEq(Loop).FractionRadiant);
            print(state.files.eio, "{:.3R},", ZoneHWEq(Loop).FractionLost);
            print(state.files.eio, "{:.3R},", ZoneHWEq(Loop).FractionConvected);
            print(state.files.eio, "{},", ZoneHWEq(Loop).EndUseSubcategory);
            print(state.files.eio, "{:.3R},", ZoneHWEq(Loop).NomMinDesignLevel);
            print(state.files.eio, "{:.3R}\n", ZoneHWEq(Loop).NomMaxDesignLevel);
        }

        for (Loop = 1; Loop <= TotStmEquip; ++Loop) {
            if (Loop == 1) {
                print(state.files.eio,
                      Format_723,
                      "SteamEquipment",
                      "Equipment Level {W},Equipment/Floor Area {W/m2},Equipment per person {W/person},Fraction Latent,Fraction Radiant,Fraction "
                      "Lost,Fraction Convected,End-Use SubCategory,Nominal Minimum Equipment Level {W},Nominal Maximum Equipment Level {W}\n");
            }

            ZoneNum = ZoneSteamEq(Loop).ZonePtr;

            if (ZoneNum == 0) {
                print(state.files.eio, Format_724, "Steam Equipment-Illegal Zone specified", ZoneSteamEq(Loop).Name);
                continue;
            }

            print(state.files.eio,
                  Format_722,
                  "SteamEquipment",
                  ZoneSteamEq(Loop).Name,
                  GetScheduleName(state, ZoneSteamEq(Loop).SchedPtr),
                  Zone(ZoneNum).Name,
                  Zone(ZoneNum).FloorArea,
                  Zone(ZoneNum).TotOccupants);

            print(state.files.eio, "{:.3R},", ZoneSteamEq(Loop).DesignLevel);

            print_and_divide_if_greater_than_zero(ZoneSteamEq(Loop).DesignLevel, Zone(ZoneNum).FloorArea);
            print_and_divide_if_greater_than_zero(ZoneSteamEq(Loop).DesignLevel, Zone(ZoneNum).TotOccupants);

            print(state.files.eio, "{:.3R},", ZoneSteamEq(Loop).FractionLatent);
            print(state.files.eio, "{:.3R},", ZoneSteamEq(Loop).FractionRadiant);
            print(state.files.eio, "{:.3R},", ZoneSteamEq(Loop).FractionLost);
            print(state.files.eio, "{:.3R},", ZoneSteamEq(Loop).FractionConvected);
            print(state.files.eio, "{},", ZoneSteamEq(Loop).EndUseSubcategory);
            print(state.files.eio, "{:.3R},", ZoneSteamEq(Loop).NomMinDesignLevel);
            print(state.files.eio, "{:.3R}\n", ZoneSteamEq(Loop).NomMaxDesignLevel);
        }

        for (Loop = 1; Loop <= TotOthEquip; ++Loop) {
            if (Loop == 1) {
                print(state.files.eio,
                      Format_723,
                      "OtherEquipment",
                      "Equipment Level {W},Equipment/Floor Area {W/m2},Equipment per person {W/person},Fraction Latent,Fraction Radiant,Fraction "
                      "Lost,Fraction Convected,Nominal Minimum Equipment Level {W},Nominal Maximum Equipment Level {W}\n");
            }

            ZoneNum = ZoneOtherEq(Loop).ZonePtr;

            if (ZoneNum == 0) {
                print(state.files.eio, Format_724, "Other Equipment-Illegal Zone specified", ZoneOtherEq(Loop).Name);
                continue;
            }

            print(state.files.eio,
                  Format_722,
                  "OtherEquipment",
                  ZoneOtherEq(Loop).Name,
                  GetScheduleName(state, ZoneOtherEq(Loop).SchedPtr),
                  Zone(ZoneNum).Name,
                  Zone(ZoneNum).FloorArea,
                  Zone(ZoneNum).TotOccupants);

            print(state.files.eio, "{:.3R},", ZoneOtherEq(Loop).DesignLevel);

            print_and_divide_if_greater_than_zero(ZoneOtherEq(Loop).DesignLevel, Zone(ZoneNum).FloorArea);
            print_and_divide_if_greater_than_zero(ZoneOtherEq(Loop).DesignLevel, Zone(ZoneNum).TotOccupants);

            print(state.files.eio, "{:.3R},", ZoneOtherEq(Loop).FractionLatent);
            print(state.files.eio, "{:.3R},", ZoneOtherEq(Loop).FractionRadiant);
            print(state.files.eio, "{:.3R},", ZoneOtherEq(Loop).FractionLost);
            print(state.files.eio, "{:.3R},", ZoneOtherEq(Loop).FractionConvected);
            print(state.files.eio, "{:.3R},", ZoneOtherEq(Loop).NomMinDesignLevel);
            print(state.files.eio, "{:.3R}\n", ZoneOtherEq(Loop).NomMaxDesignLevel);
        }

        for (Loop = 1; Loop <= NumZoneITEqStatements; ++Loop) {
            if (Loop == 1) {
                print(state.files.eio,
                      Format_723,
                      "ElectricEquipment:ITE:AirCooled",
                      "Equipment Level {W},"
                      "Equipment/Floor Area {W/m2},Equipment per person {W/person},"
                      "Fraction Convected,CPU End-Use SubCategory,Fan End-Use SubCategory,UPS End-Use SubCategory,"
                      "Nominal Minimum Equipment Level {W},Nominal Maximum Equipment Level {W}, Design Air Volume Flow Rate {m3/s}\n");
            }

            ZoneNum = ZoneITEq(Loop).ZonePtr;

            if (ZoneNum == 0) {
                print(state.files.eio, Format_724, "ElectricEquipment:ITE:AirCooled-Illegal Zone specified", ZoneITEq(Loop).Name);
                continue;
            }
            print(state.files.eio,
                  Format_722,
                  "ElectricEquipment:ITE:AirCooled",
                  ZoneITEq(Loop).Name,
                  GetScheduleName(state, ZoneITEq(Loop).OperSchedPtr),
                  Zone(ZoneNum).Name,
                  Zone(ZoneNum).FloorArea,
                  Zone(ZoneNum).TotOccupants);

            print(state.files.eio, "{:.3R},", ZoneITEq(Loop).DesignTotalPower);

            print_and_divide_if_greater_than_zero(ZoneITEq(Loop).DesignTotalPower, Zone(ZoneNum).FloorArea);

            // ElectricEquipment:ITE:AirCooled is 100% convective
            print(state.files.eio, "1.0,");

            print(state.files.eio, "{},", ZoneITEq(Loop).EndUseSubcategoryCPU);
            print(state.files.eio, "{},", ZoneITEq(Loop).EndUseSubcategoryFan);
            print(state.files.eio, "{},", ZoneITEq(Loop).EndUseSubcategoryUPS);
            print(state.files.eio, "{:.3R},", ZoneITEq(Loop).NomMinDesignLevel);
            print(state.files.eio, "{:.3R},", ZoneITEq(Loop).NomMaxDesignLevel);
            print(state.files.eio, "{:.10R}\n", ZoneITEq(Loop).DesignAirVolFlowRate);
        }

        for (Loop = 1; Loop <= TotBBHeat; ++Loop) {
            if (Loop == 1) {
                print(state.files.eio,
                      Format_723,
                      "Outdoor Controlled Baseboard Heat",
                      "Capacity at Low Temperature {W},Low Temperature {C},Capacity at High Temperature "
                      "{W},High Temperature {C},Fraction Radiant,Fraction Convected,End-Use Subcategory\n");
            }

            ZoneNum = ZoneBBHeat(Loop).ZonePtr;

            if (ZoneNum == 0) {
                print(state.files.eio, Format_724, "Outdoor Controlled Baseboard Heat-Illegal Zone specified", ZoneBBHeat(Loop).Name);
                continue;
            }
            print(state.files.eio,
                  Format_722,
                  "Outdoor Controlled Baseboard Heat",
                  ZoneBBHeat(Loop).Name,
                  GetScheduleName(state, ZoneBBHeat(Loop).SchedPtr),
                  Zone(ZoneNum).Name,
                  Zone(ZoneNum).FloorArea,
                  Zone(ZoneNum).TotOccupants);

            print(state.files.eio, "{:.3R},", ZoneBBHeat(Loop).CapatLowTemperature);
            print(state.files.eio, "{:.3R},", ZoneBBHeat(Loop).LowTemperature);
            print(state.files.eio, "{:.3R},", ZoneBBHeat(Loop).CapatHighTemperature);
            print(state.files.eio, "{:.3R},", ZoneBBHeat(Loop).HighTemperature);
            print(state.files.eio, "{:.3R},", ZoneBBHeat(Loop).FractionRadiant);
            print(state.files.eio, "{:.3R},", ZoneBBHeat(Loop).FractionConvected);
            print(state.files.eio, "{}\n", ZoneBBHeat(Loop).EndUseSubcategory);
        }
    }

    void InitInternalHeatGains(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda K. Lawrie
        //       DATE WRITTEN   September 1997
        //       MODIFIED       November 1998, FW: add adjustment to elec lights for dayltg controls
        //                      August 2003, FCW: add optional calculation of light-to-return fraction
        //                       as a function of return plenum air temperature.
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine sets up the zone internal heat gains
        // that are independent of the zone air temperature.

        // Using/Aliasing
        using namespace ScheduleManager;
        using DataHeatBalFanSys::MAT;
        using DataHeatBalFanSys::SumConvHTRadSys;
        using DataHeatBalFanSys::ZoneLatentGain;
        using DataHeatBalFanSys::ZoneLatentGainExceptPeople;
        using namespace DataDaylighting;
        using DataGlobals::CompLoadReportIsReq;
        using DataRoomAirModel::IsZoneDV;
        using DataRoomAirModel::IsZoneUI;
        using DataRoomAirModel::TCMF;
        using DataSizing::CurOverallSimDay;
        using DataZoneEquipment::ZoneEquipConfig;
        using DaylightingDevices::FigureTDDZoneGains;
        using FuelCellElectricGenerator::FigureFuelCellZoneGains;
        using MicroCHPElectricGenerator::FigureMicroCHPZoneGains;
        using OutputReportTabular::AllocateLoadComponentArrays;
        using OutputReportTabular::radiantPulseReceived;
        using OutputReportTabular::radiantPulseTimestep;
        using Psychrometrics::PsyRhoAirFnPbTdbW;
        using RefrigeratedCase::FigureRefrigerationZoneGains;
        using WaterThermalTanks::CalcWaterThermalTankZoneGains;
        using WaterUse::CalcWaterUseZoneGains;
        //using ZonePlenum::ZoneRetPlenCond;

        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:
        // na

        // SUBROUTINE PARAMETER DEFINITIONS:
        static Array1D<Real64> const C(
            9, {6.4611027, 0.946892, 0.0000255737, 7.139322, -0.0627909, 0.0000589271, -0.198550, 0.000940018, -0.00000149532});
        static ZoneCatEUseData const zeroZoneCatEUse; // For initialization

        // INTERFACE BLOCK SPECIFICATIONS:
        // na

        // DERIVED TYPE DEFINITIONS:
        // na

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 ActivityLevel_WperPerson; // Units on Activity Level (Schedule)
        Real64 NumberOccupants;          // Number of occupants
        int Loop;
        Real64 Q;                  // , QR
        Real64 TotalPeopleGain;    // Total heat gain from people (intermediate calculational variable)
        Real64 SensiblePeopleGain; // Sensible heat gain from people (intermediate calculational variable)
        Real64 FractionConvected;  // For general lighting, fraction of heat from lights convected to zone air
        Real64 FractionReturnAir;  // For general lighting, fraction of heat from lights convected to zone's return air
        Real64 FractionRadiant;    // For general lighting, fraction of heat from lights to zone that is long wave

        Real64 ReturnPlenumTemp;  // Air temperature of a zone's return air plenum (C)
        Real64 pulseMultipler;    // use to create a pulse for the load component report computations
        static Real64 curQL(0.0); // radiant value prior to adjustment for pulse for load component report
        static Real64 adjQL(0.0); // radiant value including adjustment for pulse for load component report

        //  REAL(r64), ALLOCATABLE, SAVE, DIMENSION(:) :: QSA

        //  IF (.NOT. ALLOCATED(QSA)) ALLOCATE(QSA(NumOfZones))

        //  Zero out time step variables
        for (auto &e : ZoneIntGain) {
            e.NOFOCC = 0.0;
            e.QOCTOT = 0.0;
            e.QOCSEN = 0.0;
            e.QOCLAT = 0.0;
            e.QOCRAD = 0.0;
            e.QOCCON = 0.0;
            e.QLTSW = 0.0;
            e.QLTCRA = 0.0;
            e.QLTRAD = 0.0;
            e.QLTCON = 0.0;
            e.QLTTOT = 0.0;

            e.QEELAT = 0.0;
            e.QEERAD = 0.0;
            e.QEECON = 0.0;
            e.QEELost = 0.0;
            e.QGELAT = 0.0;
            e.QGERAD = 0.0;
            e.QGECON = 0.0;
            e.QGELost = 0.0;
            e.QBBRAD = 0.0;
            e.QBBCON = 0.0;
            e.QOELAT = 0.0;
            e.QOERAD = 0.0;
            e.QOECON = 0.0;
            e.QOELost = 0.0;
            e.QHWLAT = 0.0;
            e.QHWRAD = 0.0;
            e.QHWCON = 0.0;
            e.QHWLost = 0.0;
            e.QSELAT = 0.0;
            e.QSERAD = 0.0;
            e.QSECON = 0.0;
            e.QSELost = 0.0;
        }

        ZoneIntEEuse = zeroZoneCatEUse; // Set all member arrays to zeros

        for (auto &e : ZnRpt) {
            e.LtsPower = 0.0;
            e.ElecPower = 0.0;
            e.GasPower = 0.0;
            e.HWPower = 0.0;
            e.SteamPower = 0.0;
            e.BaseHeatPower = 0.0;
            e.CO2Rate = 0.0;
        }

        //  QSA = 0.0

        // Process Internal Heat Gains, People done below
        // Occupant Stuff
        //   METHOD:
        //       The function is based on a curve fit to data presented in
        //       Table 48 'Heat Gain From People' of Chapter 1 of the 'Carrier
        //       Handbook of Air Conditioning System Design', 1965.  Values of
        //       Sensible gain were obtained from the table at average adjusted
        //       metabolic rates 350, 400, 450, 500, 750, 850, 1000, and
        //       1450 Btu/hr each at temperatures 82, 80, 78, 75, and 70F.
        //       Sensible gains of 0.0 at 96F and equal to the metabolic rate
        //       at 30F were assumed in order to give reasonable values beyond
        //       The reported temperature range.
        for (Loop = 1; Loop <= TotPeople; ++Loop) {
            int NZ = People(Loop).ZonePtr;
            NumberOccupants = People(Loop).NumberOfPeople * GetCurrentScheduleValue(People(Loop).NumberOfPeoplePtr);
            if (People(Loop).EMSPeopleOn) NumberOccupants = People(Loop).EMSNumberOfPeople;

            TotalPeopleGain = 0.0;
            SensiblePeopleGain = 0.0;

            if (NumberOccupants > 0.0) {
                ActivityLevel_WperPerson = GetCurrentScheduleValue(People(Loop).ActivityLevelPtr);
                TotalPeopleGain = NumberOccupants * ActivityLevel_WperPerson;
                // if the user did not specify a sensible fraction, calculate the sensible heat gain
                if (People(Loop).UserSpecSensFrac == AutoCalculate) {
                    if (!(IsZoneDV(NZ) || IsZoneUI(NZ))) {
                        SensiblePeopleGain =
                            NumberOccupants * (C(1) + ActivityLevel_WperPerson * (C(2) + ActivityLevel_WperPerson * C(3)) +
                                               MAT(NZ) * ((C(4) + ActivityLevel_WperPerson * (C(5) + ActivityLevel_WperPerson * C(6))) +
                                                          MAT(NZ) * (C(7) + ActivityLevel_WperPerson * (C(8) + ActivityLevel_WperPerson * C(9)))));
                    } else { // UCSD - DV or UI
                        SensiblePeopleGain =
                            NumberOccupants * (C(1) + ActivityLevel_WperPerson * (C(2) + ActivityLevel_WperPerson * C(3)) +
                                               TCMF(NZ) * ((C(4) + ActivityLevel_WperPerson * (C(5) + ActivityLevel_WperPerson * C(6))) +
                                                           TCMF(NZ) * (C(7) + ActivityLevel_WperPerson * (C(8) + ActivityLevel_WperPerson * C(9)))));
                    }
                } else { // if the user did specify a sensible fraction, use it
                    SensiblePeopleGain = TotalPeopleGain * People(Loop).UserSpecSensFrac;
                }

                if (SensiblePeopleGain > TotalPeopleGain) SensiblePeopleGain = TotalPeopleGain;
                if (SensiblePeopleGain < 0.0) SensiblePeopleGain = 0.0;

                // For predefined tabular reports related to outside air ventilation
                ZonePreDefRep(NZ).isOccupied = true; // set flag to occupied to be used in tabular reporting for ventilation
                ZonePreDefRep(NZ).NumOccAccum += NumberOccupants * TimeStepZone;
                ZonePreDefRep(NZ).NumOccAccumTime += TimeStepZone;
            } else {
                ZonePreDefRep(NZ).isOccupied = false; // set flag to occupied to be used in tabular reporting for ventilation
            }

            People(Loop).NumOcc = NumberOccupants;
            People(Loop).RadGainRate = SensiblePeopleGain * People(Loop).FractionRadiant;
            People(Loop).ConGainRate = SensiblePeopleGain * People(Loop).FractionConvected;
            People(Loop).SenGainRate = SensiblePeopleGain;
            People(Loop).LatGainRate = TotalPeopleGain - SensiblePeopleGain;
            People(Loop).TotGainRate = TotalPeopleGain;
            People(Loop).CO2GainRate = TotalPeopleGain * People(Loop).CO2RateFactor;

            ZoneIntGain(NZ).NOFOCC += People(Loop).NumOcc;
            ZoneIntGain(NZ).QOCRAD += People(Loop).RadGainRate;
            ZoneIntGain(NZ).QOCCON += People(Loop).ConGainRate;
            ZoneIntGain(NZ).QOCSEN += People(Loop).SenGainRate;
            ZoneIntGain(NZ).QOCLAT += People(Loop).LatGainRate;
            ZoneIntGain(NZ).QOCTOT += People(Loop).TotGainRate;
        }

        for (Loop = 1; Loop <= TotLights; ++Loop) {
            int NZ = Lights(Loop).ZonePtr;
            Q = Lights(Loop).DesignLevel * GetCurrentScheduleValue(Lights(Loop).SchedPtr);

            if (ZoneDaylight(NZ).DaylightMethod == SplitFluxDaylighting || ZoneDaylight(NZ).DaylightMethod == DElightDaylighting) {

                if (Lights(Loop).FractionReplaceable > 0.0) { // FractionReplaceable can only be 0 or 1 for these models
                    Q *= ZoneDaylight(NZ).ZonePowerReductionFactor;
                }
            }

            // Reduce lighting power due to demand limiting
            if (Lights(Loop).ManageDemand && (Q > Lights(Loop).DemandLimit)) Q = Lights(Loop).DemandLimit;

            // Set Q to EMS override if being called for by EMs
            if (Lights(Loop).EMSLightsOn) Q = Lights(Loop).EMSLightingPower;

            FractionConvected = Lights(Loop).FractionConvected;
            FractionReturnAir = Lights(Loop).FractionReturnAir;
            FractionRadiant = Lights(Loop).FractionRadiant;
            if (Lights(Loop).FractionReturnAirIsCalculated && !ZoneSizingCalc && SimTimeSteps > 1) {
                // Calculate FractionReturnAir based on conditions in the zone's return air plenum, if there is one.
                if (Zone(NZ).IsControlled) {
                    int retNum = Lights(Loop).ZoneReturnNum;
                    int ReturnZonePlenumCondNum = ZoneEquipConfig(NZ).ReturnNodePlenumNum(retNum);
                    if (ReturnZonePlenumCondNum > 0) {
                        ReturnPlenumTemp = state.dataZonePlenum->ZoneRetPlenCond(ReturnZonePlenumCondNum).ZoneTemp;
                        FractionReturnAir =
                            Lights(Loop).FractionReturnAirPlenTempCoeff1 - Lights(Loop).FractionReturnAirPlenTempCoeff2 * ReturnPlenumTemp;
                        FractionReturnAir = max(0.0, min(1.0, FractionReturnAir));
                        if (FractionReturnAir >= (1.0 - Lights(Loop).FractionShortWave)) {
                            FractionReturnAir = 1.0 - Lights(Loop).FractionShortWave;
                            FractionRadiant = 0.0;
                            FractionConvected = 0.0;
                        } else {
                            FractionRadiant = ((1.0 - FractionReturnAir - Lights(Loop).FractionShortWave) /
                                               (Lights(Loop).FractionRadiant + Lights(Loop).FractionConvected)) *
                                              Lights(Loop).FractionRadiant;
                            FractionConvected = 1.0 - (FractionReturnAir + FractionRadiant + Lights(Loop).FractionShortWave);
                        }
                    }
                }
            }

            Lights(Loop).Power = Q;
            Lights(Loop).RadGainRate = Q * FractionRadiant;
            Lights(Loop).VisGainRate = Q * Lights(Loop).FractionShortWave;
            Lights(Loop).ConGainRate = Q * FractionConvected;
            Lights(Loop).RetAirGainRate = Q * FractionReturnAir;
            Lights(Loop).TotGainRate = Q;

            ZnRpt(NZ).LtsPower += Lights(Loop).Power;
            ZoneIntGain(NZ).QLTRAD += Lights(Loop).RadGainRate;
            ZoneIntGain(NZ).QLTSW += Lights(Loop).VisGainRate;
            ZoneIntGain(NZ).QLTCON += Lights(Loop).ConGainRate;
            ZoneIntGain(NZ).QLTCRA += Lights(Loop).RetAirGainRate;
            ZoneIntGain(NZ).QLTTOT += Lights(Loop).TotGainRate;
        }

        for (Loop = 1; Loop <= TotElecEquip; ++Loop) {
            Q = ZoneElectric(Loop).DesignLevel * GetCurrentScheduleValue(ZoneElectric(Loop).SchedPtr);

            // Reduce equipment power due to demand limiting
            if (ZoneElectric(Loop).ManageDemand && (Q > ZoneElectric(Loop).DemandLimit)) Q = ZoneElectric(Loop).DemandLimit;

            // Set Q to EMS override if being called for by EMs
            if (ZoneElectric(Loop).EMSZoneEquipOverrideOn) Q = ZoneElectric(Loop).EMSEquipPower;

            ZoneElectric(Loop).Power = Q;
            ZoneElectric(Loop).RadGainRate = Q * ZoneElectric(Loop).FractionRadiant;
            ZoneElectric(Loop).ConGainRate = Q * ZoneElectric(Loop).FractionConvected;
            ZoneElectric(Loop).LatGainRate = Q * ZoneElectric(Loop).FractionLatent;
            ZoneElectric(Loop).LostRate = Q * ZoneElectric(Loop).FractionLost;
            ZoneElectric(Loop).TotGainRate = Q - ZoneElectric(Loop).LostRate;

            int NZ = ZoneElectric(Loop).ZonePtr;
            ZnRpt(NZ).ElecPower += ZoneElectric(Loop).Power;
            ZoneIntGain(NZ).QEERAD += ZoneElectric(Loop).RadGainRate;
            ZoneIntGain(NZ).QEECON += ZoneElectric(Loop).ConGainRate;
            ZoneIntGain(NZ).QEELAT += ZoneElectric(Loop).LatGainRate;
            ZoneIntGain(NZ).QEELost += ZoneElectric(Loop).LostRate;
        }

        for (Loop = 1; Loop <= TotGasEquip; ++Loop) {
            Q = ZoneGas(Loop).DesignLevel * GetCurrentScheduleValue(ZoneGas(Loop).SchedPtr);

            // Set Q to EMS override if being called for by EMs
            if (ZoneGas(Loop).EMSZoneEquipOverrideOn) Q = ZoneGas(Loop).EMSEquipPower;

            ZoneGas(Loop).Power = Q;
            ZoneGas(Loop).RadGainRate = Q * ZoneGas(Loop).FractionRadiant;
            ZoneGas(Loop).ConGainRate = Q * ZoneGas(Loop).FractionConvected;
            ZoneGas(Loop).LatGainRate = Q * ZoneGas(Loop).FractionLatent;
            ZoneGas(Loop).LostRate = Q * ZoneGas(Loop).FractionLost;
            ZoneGas(Loop).TotGainRate = Q - ZoneGas(Loop).LostRate;
            ZoneGas(Loop).CO2GainRate = Q * ZoneGas(Loop).CO2RateFactor;

            int NZ = ZoneGas(Loop).ZonePtr;
            ZnRpt(NZ).GasPower += ZoneGas(Loop).Power;
            ZoneIntGain(NZ).QGERAD += ZoneGas(Loop).RadGainRate;
            ZoneIntGain(NZ).QGECON += ZoneGas(Loop).ConGainRate;
            ZoneIntGain(NZ).QGELAT += ZoneGas(Loop).LatGainRate;
            ZoneIntGain(NZ).QGELost += ZoneGas(Loop).LostRate;
        }

        for (Loop = 1; Loop <= TotOthEquip; ++Loop) {
            Q = ZoneOtherEq(Loop).DesignLevel * GetCurrentScheduleValue(ZoneOtherEq(Loop).SchedPtr);

            // Set Q to EMS override if being called for by EMs
            if (ZoneOtherEq(Loop).EMSZoneEquipOverrideOn) Q = ZoneOtherEq(Loop).EMSEquipPower;

            ZoneOtherEq(Loop).Power = Q;
            ZoneOtherEq(Loop).RadGainRate = Q * ZoneOtherEq(Loop).FractionRadiant;
            ZoneOtherEq(Loop).ConGainRate = Q * ZoneOtherEq(Loop).FractionConvected;
            ZoneOtherEq(Loop).LatGainRate = Q * ZoneOtherEq(Loop).FractionLatent;
            ZoneOtherEq(Loop).LostRate = Q * ZoneOtherEq(Loop).FractionLost;
            ZoneOtherEq(Loop).TotGainRate = Q - ZoneOtherEq(Loop).LostRate;

            int NZ = ZoneOtherEq(Loop).ZonePtr;
            ZoneIntGain(NZ).QOERAD += ZoneOtherEq(Loop).RadGainRate;
            ZoneIntGain(NZ).QOECON += ZoneOtherEq(Loop).ConGainRate;
            ZoneIntGain(NZ).QOELAT += ZoneOtherEq(Loop).LatGainRate;
            ZoneIntGain(NZ).QOELost += ZoneOtherEq(Loop).LostRate;
        }

        for (Loop = 1; Loop <= TotHWEquip; ++Loop) {
            Q = ZoneHWEq(Loop).DesignLevel * GetCurrentScheduleValue(ZoneHWEq(Loop).SchedPtr);

            // Set Q to EMS override if being called for by EMs
            if (ZoneHWEq(Loop).EMSZoneEquipOverrideOn) Q = ZoneHWEq(Loop).EMSEquipPower;

            ZoneHWEq(Loop).Power = Q;
            ZoneHWEq(Loop).RadGainRate = Q * ZoneHWEq(Loop).FractionRadiant;
            ZoneHWEq(Loop).ConGainRate = Q * ZoneHWEq(Loop).FractionConvected;
            ZoneHWEq(Loop).LatGainRate = Q * ZoneHWEq(Loop).FractionLatent;
            ZoneHWEq(Loop).LostRate = Q * ZoneHWEq(Loop).FractionLost;
            ZoneHWEq(Loop).TotGainRate = Q - ZoneHWEq(Loop).LostRate;

            int NZ = ZoneHWEq(Loop).ZonePtr;
            ZnRpt(NZ).HWPower += ZoneHWEq(Loop).Power;
            ZoneIntGain(NZ).QHWRAD += ZoneHWEq(Loop).RadGainRate;
            ZoneIntGain(NZ).QHWCON += ZoneHWEq(Loop).ConGainRate;
            ZoneIntGain(NZ).QHWLAT += ZoneHWEq(Loop).LatGainRate;
            ZoneIntGain(NZ).QHWLost += ZoneHWEq(Loop).LostRate;
        }

        for (Loop = 1; Loop <= TotStmEquip; ++Loop) {
            Q = ZoneSteamEq(Loop).DesignLevel * GetCurrentScheduleValue(ZoneSteamEq(Loop).SchedPtr);

            // Set Q to EMS override if being called for by EMs
            if (ZoneSteamEq(Loop).EMSZoneEquipOverrideOn) Q = ZoneSteamEq(Loop).EMSEquipPower;

            ZoneSteamEq(Loop).Power = Q;
            ZoneSteamEq(Loop).RadGainRate = Q * ZoneSteamEq(Loop).FractionRadiant;
            ZoneSteamEq(Loop).ConGainRate = Q * ZoneSteamEq(Loop).FractionConvected;
            ZoneSteamEq(Loop).LatGainRate = Q * ZoneSteamEq(Loop).FractionLatent;
            ZoneSteamEq(Loop).LostRate = Q * ZoneSteamEq(Loop).FractionLost;
            ZoneSteamEq(Loop).TotGainRate = Q - ZoneSteamEq(Loop).LostRate;

            int NZ = ZoneSteamEq(Loop).ZonePtr;
            ZnRpt(NZ).SteamPower += ZoneSteamEq(Loop).Power;
            ZoneIntGain(NZ).QSERAD += ZoneSteamEq(Loop).RadGainRate;
            ZoneIntGain(NZ).QSECON += ZoneSteamEq(Loop).ConGainRate;
            ZoneIntGain(NZ).QSELAT += ZoneSteamEq(Loop).LatGainRate;
            ZoneIntGain(NZ).QSELost += ZoneSteamEq(Loop).LostRate;
        }

        for (Loop = 1; Loop <= TotBBHeat; ++Loop) {
            int NZ = ZoneBBHeat(Loop).ZonePtr;
            if (Zone(NZ).OutDryBulbTemp >= ZoneBBHeat(Loop).HighTemperature) {
                Q = 0.0;
            } else if (Zone(NZ).OutDryBulbTemp > ZoneBBHeat(Loop).LowTemperature) {
                Q = (Zone(NZ).OutDryBulbTemp - ZoneBBHeat(Loop).LowTemperature) *
                        (ZoneBBHeat(Loop).CapatHighTemperature - ZoneBBHeat(Loop).CapatLowTemperature) /
                        (ZoneBBHeat(Loop).HighTemperature - ZoneBBHeat(Loop).LowTemperature) +
                    ZoneBBHeat(Loop).CapatLowTemperature;
            } else {
                Q = ZoneBBHeat(Loop).CapatLowTemperature;
            }
            Q *= GetCurrentScheduleValue(ZoneBBHeat(Loop).SchedPtr);

            // set with EMS value if being called for.
            if (ZoneBBHeat(Loop).EMSZoneBaseboardOverrideOn) Q = ZoneBBHeat(Loop).EMSZoneBaseboardPower;

            ZoneBBHeat(Loop).Power = Q;
            ZoneBBHeat(Loop).RadGainRate = Q * ZoneBBHeat(Loop).FractionRadiant;
            ZoneBBHeat(Loop).ConGainRate = Q * ZoneBBHeat(Loop).FractionConvected;
            ZoneBBHeat(Loop).TotGainRate = Q;

            NZ = ZoneBBHeat(Loop).ZonePtr;
            ZnRpt(NZ).BaseHeatPower += ZoneBBHeat(Loop).Power;
            ZoneIntGain(NZ).QBBRAD += ZoneBBHeat(Loop).RadGainRate;
            ZoneIntGain(NZ).QBBCON += ZoneBBHeat(Loop).ConGainRate;
        }

        for (Loop = 1; Loop <= TotCO2Gen; ++Loop) {
            int NZ = ZoneCO2Gen(Loop).ZonePtr;
            ZoneCO2Gen(Loop).CO2GainRate = ZoneCO2Gen(Loop).CO2DesignRate * GetCurrentScheduleValue(ZoneCO2Gen(Loop).SchedPtr);
            ZnRpt(NZ).CO2Rate += ZoneCO2Gen(Loop).CO2GainRate;
        }

        if (NumZoneITEqStatements > 0) CalcZoneITEq(state);

        CalcWaterThermalTankZoneGains(state);
        PipeHeatTransfer::PipeHTData::CalcZonePipesHeatGain();
        CalcWaterUseZoneGains(state);
        FigureFuelCellZoneGains();
        FigureMicroCHPZoneGains();
        initializeElectricPowerServiceZoneGains();
        FigureTDDZoneGains();
        FigureRefrigerationZoneGains(state);

        // store pointer values to hold generic internal gain values constant for entire timestep
        UpdateInternalGainValues();

        for (int NZ = 1; NZ <= NumOfZones; ++NZ) {

            SumAllInternalLatentGains(NZ, ZoneLatentGain(NZ));
            // Added for hybrid model
            if (HybridModel::FlagHybridModel_PC) {
                SumAllInternalLatentGainsExceptPeople(NZ, ZoneLatentGainExceptPeople(NZ));
            }
        }

        // QL is per radiant enclosure (one or more zones if grouped by air boundaries)
        for (int enclosureNum = 1; enclosureNum <= DataViewFactorInformation::NumOfRadiantEnclosures; ++enclosureNum) {
            auto &thisEnclosure(DataViewFactorInformation::ZoneRadiantInfo(enclosureNum));
            QL(enclosureNum) = 0.0;
            for (int const zoneNum : thisEnclosure.ZoneNums) {
                Real64 zoneQL;
                SumAllInternalRadiationGains(zoneNum, zoneQL);
                QL(enclosureNum) += zoneQL;
            }
        }

        SumConvHTRadSys = 0.0;

        pulseMultipler = 0.01; // the W/sqft pulse for the zone
        if (CompLoadReportIsReq) {
            AllocateLoadComponentArrays();
        }
        for (int zoneNum = 1; zoneNum <= NumOfZones; ++zoneNum) {// Loop through all surfaces...
            int const firstSurf = Zone(zoneNum).SurfaceFirst;
            int const lastSurf = Zone(zoneNum).SurfaceLast;
            if (firstSurf <= 0) continue;
            for (int SurfNum = firstSurf; SurfNum <= lastSurf; ++SurfNum) {
                if (!Surface(SurfNum).HeatTransSurf) continue; // Skip non-heat transfer surfaces
                int const radEnclosureNum = Zone(zoneNum).RadiantEnclosureNum;
                if (!doLoadComponentPulseNow) {
                    SurfQRadThermInAbs(SurfNum) = QL(radEnclosureNum) * TMULT(radEnclosureNum) * ITABSF(SurfNum);
                } else {
                    curQL = QL(radEnclosureNum);
                    // for the loads component report during the special sizing run increase the radiant portion
                    // a small amount to create a "pulse" of heat that is used for the delayed loads
                    adjQL = curQL +
                            DataViewFactorInformation::ZoneRadiantInfo(radEnclosureNum).FloorArea * pulseMultipler;
                    // ITABSF is the Inside Thermal Absorptance
                    // TMULT is a multiplier for each zone
                    // QRadThermInAbs is the thermal radiation absorbed on inside surfaces
                    SurfQRadThermInAbs(SurfNum) = adjQL * TMULT(radEnclosureNum) * ITABSF(SurfNum);
                    // store the magnitude and time of the pulse
                    radiantPulseTimestep(CurOverallSimDay, zoneNum) = (HourOfDay - 1) * NumOfTimeStepInHour + TimeStep;
                    radiantPulseReceived(CurOverallSimDay, SurfNum) =
                            (adjQL - curQL) * TMULT(radEnclosureNum) * ITABSF(SurfNum) * Surface(SurfNum).Area;
                }
            }
        }
    }

    void CheckReturnAirHeatGain()
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Xuan Luo
        //       DATE WRITTEN   Jan 2018

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine currently creates the values for standard "zone loads" reporting
        // from the heat balance module.

        // Using/Aliasing
        using DataHeatBalance::Zone;
        using DataZoneEquipment::ZoneEquipConfig;

        for (int ZoneNum = 1; ZoneNum <= NumOfZones; ++ZoneNum) {
            if (Zone(ZoneNum).HasAdjustedReturnTempByITE && Zone(ZoneNum).HasLtsRetAirGain) {
                ShowFatalError("Return air heat gains from lights are not allowed when Air Flow Calculation Method = "
                               "FlowControlWithApproachTemperatures in zones with ITE objects.");
            }
            if (Zone(ZoneNum).HasAdjustedReturnTempByITE && Zone(ZoneNum).HasAirFlowWindowReturn) {
                ShowFatalError("Return air heat gains from windows are not allowed when Air Flow Calculation Method = "
                               "FlowControlWithApproachTemperatures in zones with ITE objects.");
            }
        }
    }

    void CalcZoneITEq(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         M.J. Witte
        //       DATE WRITTEN   October 2014

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine calculates the gains and other results for ElectricEquipment:ITE:AirCooled.
        // This broken into a separate subroutine, because the calculations are more detailed than the other
        // types of internal gains.

        using DataHeatBalFanSys::MAT;
        using DataHeatBalFanSys::ZoneAirHumRat;
        using DataZoneEquipment::ZoneEquipConfig;
        using ScheduleManager::GetCurrentScheduleValue;
        using namespace Psychrometrics;
        using CurveManager::CurveValue;
        using DataHeatBalance::Zone;
        using DataHVACGlobals::SmallAirVolFlow;
        using DataHVACGlobals::SmallTempDiff;
        using DataLoopNode::Node;
        using DataRoomAirModel::IsZoneDV;
        using DataRoomAirModel::IsZoneUI;
        using DataRoomAirModel::TCMF;

        // Operating Limits for environmental class: None, A1, A2, A3, A4, B, C
        // From ASHRAE 2011 Thermal Guidelines environmental classes for Air-Cooled ITE
        static Array1D<Real64> const DBMin(7, {-99.0, 15.0, 10.0, 5.0, 5.0, 5.0, 5.0});           // Minimum dry-bulb temperature [C]
        static Array1D<Real64> const DBMax(7, {99.0, 32.0, 35.0, 40.0, 45.0, 35.0, 40.0});        // Maximum dry-bulb temperature [C]
        static Array1D<Real64> const DPMax(7, {99.0, 17.0, 21.0, 24.0, 24.0, 28.0, 28.0});        // Maximum dewpoint temperature [C]
        static Array1D<Real64> const DPMin(7, {-99.0, -99.0, -99.0, -12.0, -12.0, -99.0, -99.0}); // Minimum dewpoint temperature [C]
        static Array1D<Real64> const RHMin(7, {0.0, 20.0, 20.0, 8.0, 8.0, 8.0, 8.0});             // Minimum relative humidity [%]
        static Array1D<Real64> const RHMax(7, {99.0, 80.0, 80.0, 85.0, 90.0, 80.0, 80.0});        // Maximum relative humidity [%]

        static std::string const RoutineName("CalcZoneITEq");
        int Loop;
        int NZ;
        int SupplyNodeNum;                                // Supply air node number (if zero, then not specified)
        Real64 OperSchedFrac;                             // Operating schedule fraction
        Real64 CPULoadSchedFrac;                          // CPU loading schedule fraction
        Real64 AirConnection;                             // Air connection type
        Real64 TSupply(0.0);                              // Supply air temperature [C]
        Real64 WSupply;                                   // Supply air humidity ratio [kgWater/kgDryAir]
        Real64 RecircFrac;                                // Recirulation fraction - current
        Real64 TRecirc;                                   // Recirulation air temperature [C]
        Real64 WRecirc;                                   // Recirulation air humidity ratio [kgWater/kgDryAir]
        Real64 TAirIn;                                    // Entering air dry-bulb temperature [C]
        Real64 TAirInDesign;                              // Design entering air dry-bulb temperature [C]
        Real64 WAirIn;                                    // Entering air humidity ratio [kgWater/kgDryAir]
        Real64 TDPAirIn;                                  // Entering air dewpoint temperature [C]
        Real64 RHAirIn;                                   // Entering air relative humidity [%]
        Real64 SupplyHeatIndex;                           // Supply heat index
        Real64 TAirOut;                                   // Leaving air temperature [C]
        Real64 AirVolFlowFrac;                            // Air volume flow fraction
        Real64 AirVolFlowFracDesignT;                     // Air volume flow fraction at design entering air temperature
        Real64 AirVolFlowRate;                            // Air volume flow rate at current density [m3/s]
        Real64 AirMassFlowRate;                           // Air mass flow rate [kg/s]
        Real64 CPUPower;                                  // CPU power input [W]
        Real64 FanPower;                                  // Fan power input [W]
        Real64 UPSPower;                                  // UPS new power input (losses) [W]
        Real64 UPSPartLoadRatio;                          // UPS part load ratio (current total power input / design total power input)
        Real64 UPSHeatGain;                               // UPS convective heat gain to zone [W]
        int EnvClass;                                     // Index for environmental class (None=0, A1=1, A2=2, A3=3, A4=4, B=5, C=6)

        std::map<int, std::vector<int>> ZoneITEMap;

        //  Zero out time step variables
        // Object report variables
        for (Loop = 1; Loop <= NumZoneITEqStatements; ++Loop) {
            ZoneITEq(Loop).CPUPower = 0.0;
            ZoneITEq(Loop).FanPower = 0.0;
            ZoneITEq(Loop).UPSPower = 0.0;
            ZoneITEq(Loop).CPUPowerAtDesign = 0.0;
            ZoneITEq(Loop).FanPowerAtDesign = 0.0;
            ZoneITEq(Loop).UPSGainRateToZone = 0.0;
            ZoneITEq(Loop).ConGainRateToZone = 0.0;

            ZoneITEq(Loop).CPUConsumption = 0.0;
            ZoneITEq(Loop).FanConsumption = 0.0;
            ZoneITEq(Loop).UPSConsumption = 0.0;
            ZoneITEq(Loop).CPUEnergyAtDesign = 0.0;
            ZoneITEq(Loop).FanEnergyAtDesign = 0.0;
            ZoneITEq(Loop).UPSGainEnergyToZone = 0.0;
            ZoneITEq(Loop).ConGainEnergyToZone = 0.0;

            ZoneITEq(Loop).AirVolFlowStdDensity = 0.0;
            ZoneITEq(Loop).AirVolFlowCurDensity = 0.0;
            ZoneITEq(Loop).AirMassFlow = 0.0;
            ZoneITEq(Loop).AirInletDryBulbT = 0.0;
            ZoneITEq(Loop).AirInletDewpointT = 0.0;
            ZoneITEq(Loop).AirInletRelHum = 0.0;
            ZoneITEq(Loop).AirOutletDryBulbT = 0.0;
            ZoneITEq(Loop).SHI = 0.0;
            ZoneITEq(Loop).TimeOutOfOperRange = 0.0;
            ZoneITEq(Loop).TimeAboveDryBulbT = 0.0;
            ZoneITEq(Loop).TimeBelowDryBulbT = 0.0;
            ZoneITEq(Loop).TimeAboveDewpointT = 0.0;
            ZoneITEq(Loop).TimeBelowDewpointT = 0.0;
            ZoneITEq(Loop).TimeAboveRH = 0.0;
            ZoneITEq(Loop).TimeBelowRH = 0.0;
            ZoneITEq(Loop).DryBulbTAboveDeltaT = 0.0;
            ZoneITEq(Loop).DryBulbTBelowDeltaT = 0.0;
            ZoneITEq(Loop).DewpointTAboveDeltaT = 0.0;
            ZoneITEq(Loop).DewpointTBelowDeltaT = 0.0;
            ZoneITEq(Loop).RHAboveDeltaRH = 0.0;
            ZoneITEq(Loop).RHBelowDeltaRH = 0.0;
        } // ZoneITEq init loop

        // Zone total report variables
        for (Loop = 1; Loop <= NumOfZones; ++Loop) {
            ZnRpt(Loop).ITEqCPUPower = 0.0;
            ZnRpt(Loop).ITEqFanPower = 0.0;
            ZnRpt(Loop).ITEqUPSPower = 0.0;
            ZnRpt(Loop).ITEqCPUPowerAtDesign = 0.0;
            ZnRpt(Loop).ITEqFanPowerAtDesign = 0.0;
            ZnRpt(Loop).ITEqUPSGainRateToZone = 0.0;
            ZnRpt(Loop).ITEqConGainRateToZone = 0.0;

            ZnRpt(Loop).ITEAdjReturnTemp = 0.0;
            ZnRpt(Loop).ITEqCPUConsumption = 0.0;
            ZnRpt(Loop).ITEqFanConsumption = 0.0;
            ZnRpt(Loop).ITEqUPSConsumption = 0.0;
            ZnRpt(Loop).ITEqCPUEnergyAtDesign = 0.0;
            ZnRpt(Loop).ITEqFanEnergyAtDesign = 0.0;
            ZnRpt(Loop).ITEqUPSGainEnergyToZone = 0.0;
            ZnRpt(Loop).ITEqConGainEnergyToZone = 0.0;

            ZnRpt(Loop).ITEqAirVolFlowStdDensity = 0.0;
            ZnRpt(Loop).ITEqAirMassFlow = 0.0;
            ZnRpt(Loop).ITEqSHI = 0.0;
            ZnRpt(Loop).ITEqTimeOutOfOperRange = 0.0;
            ZnRpt(Loop).ITEqTimeAboveDryBulbT = 0.0;
            ZnRpt(Loop).ITEqTimeBelowDryBulbT = 0.0;
            ZnRpt(Loop).ITEqTimeAboveDewpointT = 0.0;
            ZnRpt(Loop).ITEqTimeBelowDewpointT = 0.0;
            ZnRpt(Loop).ITEqTimeAboveRH = 0.0;
            ZnRpt(Loop).ITEqTimeBelowRH = 0.0;

            ZnRpt(Loop).SumTinMinusTSup = 0.0;
            ZnRpt(Loop).SumToutMinusTSup = 0.0;
        } // Zone init loop

        for (Loop = 1; Loop <= NumZoneITEqStatements; ++Loop) {
            // Get schedules
            NZ = ZoneITEq(Loop).ZonePtr;
            OperSchedFrac = GetCurrentScheduleValue(ZoneITEq(Loop).OperSchedPtr);
            CPULoadSchedFrac = GetCurrentScheduleValue(ZoneITEq(Loop).CPULoadSchedPtr);

            // Determine inlet air temperature and humidity
            AirConnection = ZoneITEq(Loop).AirConnectionType;
            RecircFrac = 0.0;
            SupplyNodeNum = ZoneITEq(Loop).SupplyAirNodeNum;
            if (ZoneITEq(Loop).FlowControlWithApproachTemps) {
                TSupply = Node(SupplyNodeNum).Temp;
                WSupply = Node(SupplyNodeNum).HumRat;
                if (ZoneITEq(Loop).SupplyApproachTempSch != 0) {
                    TAirIn = TSupply + GetCurrentScheduleValue(ZoneITEq(Loop).SupplyApproachTempSch);
                } else {
                    TAirIn = TSupply + ZoneITEq(Loop).SupplyApproachTemp;
                }
                WAirIn = Node(SupplyNodeNum).HumRat;
            } else {
                if (AirConnection == ITEInletAdjustedSupply) {
                    TSupply = Node(SupplyNodeNum).Temp;
                    WSupply = Node(SupplyNodeNum).HumRat;
                    if (ZoneITEq(Loop).RecircFLTCurve != 0) {
                        RecircFrac = ZoneITEq(Loop).DesignRecircFrac * CurveValue(state, ZoneITEq(Loop).RecircFLTCurve, CPULoadSchedFrac, TSupply);
                    } else {
                        RecircFrac = ZoneITEq(Loop).DesignRecircFrac;
                    }
                    TRecirc = MAT(NZ);
                    WRecirc = ZoneAirHumRat(NZ);
                    TAirIn = TRecirc * RecircFrac + TSupply * (1.0 - RecircFrac);
                    WAirIn = WRecirc * RecircFrac + WSupply * (1.0 - RecircFrac);
                } else if (AirConnection == ITEInletRoomAirModel) {
                    // Room air model option: TAirIn=TAirZone, according to EngineeringRef 17.1.4
                    TAirIn = MAT(NZ);
                    TSupply = TAirIn;
                    WAirIn = ZoneAirHumRat(NZ);
                } else {
                    // TAirIn = TRoomAirNodeIn, according to EngineeringRef 17.1.4
                    int ZoneAirInletNode = DataZoneEquipment::ZoneEquipConfig(NZ).InletNode(1);
                    TSupply = Node(ZoneAirInletNode).Temp;
                    TAirIn = MAT(NZ);
                    WAirIn = ZoneAirHumRat(NZ);
                }
            }
            TDPAirIn = PsyTdpFnWPb(WAirIn, StdBaroPress, RoutineName);
            RHAirIn = 100.0 * PsyRhFnTdbWPb(TAirIn, WAirIn, StdBaroPress, RoutineName); // RHAirIn is %

            // Calculate power input and airflow
            TAirInDesign = ZoneITEq(Loop).DesignTAirIn;

            if (DoingSizing && ZoneITEq(Loop).FlowControlWithApproachTemps) {

                TAirInDesign = ZoneITEq(Loop).SizingTAirIn;
                if (ZoneITEq(Loop).SupplyApproachTempSch != 0) {
                    TAirInDesign = TAirInDesign + GetCurrentScheduleValue(ZoneITEq(Loop).SupplyApproachTempSch);
                } else {
                    TAirInDesign = TAirInDesign + ZoneITEq(Loop).SupplyApproachTemp;
                }
                OperSchedFrac = GetCurrentScheduleValue(ZoneITEq(Loop).OperSchedPtr);
                CPULoadSchedFrac = GetCurrentScheduleValue(ZoneITEq(Loop).CPULoadSchedPtr);

            }

            CPUPower =
                max(ZoneITEq(Loop).DesignCPUPower * OperSchedFrac * CurveValue(state, ZoneITEq(Loop).CPUPowerFLTCurve, CPULoadSchedFrac, TAirIn), 0.0);
            ZoneITEq(Loop).CPUPowerAtDesign =
                max(ZoneITEq(Loop).DesignCPUPower * OperSchedFrac * CurveValue(state, ZoneITEq(Loop).CPUPowerFLTCurve, CPULoadSchedFrac, TAirInDesign), 0.0);

            AirVolFlowFrac = max(CurveValue(state, ZoneITEq(Loop).AirFlowFLTCurve, CPULoadSchedFrac, TAirIn), 0.0);
            AirVolFlowRate = ZoneITEq(Loop).DesignAirVolFlowRate * OperSchedFrac * AirVolFlowFrac;
            if (AirVolFlowRate < SmallAirVolFlow) {
                AirVolFlowRate = 0.0;
            }
            AirVolFlowFracDesignT = max(CurveValue(state, ZoneITEq(Loop).AirFlowFLTCurve, CPULoadSchedFrac, TAirInDesign), 0.0);

            FanPower = max(ZoneITEq(Loop).DesignFanPower * OperSchedFrac * CurveValue(state, ZoneITEq(Loop).FanPowerFFCurve, AirVolFlowFrac), 0.0);
            ZoneITEq(Loop).FanPowerAtDesign =
                max(ZoneITEq(Loop).DesignFanPower * OperSchedFrac * CurveValue(state, ZoneITEq(Loop).FanPowerFFCurve, AirVolFlowFracDesignT), 0.0);

            // Calculate UPS net power input (power in less power to ITEquip) and UPS heat gain to zone
            if (ZoneITEq(Loop).DesignTotalPower > 0.0) {
                UPSPartLoadRatio = (CPUPower + FanPower) / ZoneITEq(Loop).DesignTotalPower;
            } else {
                UPSPartLoadRatio = 0.0;
            }
            if (ZoneITEq(Loop).UPSEfficFPLRCurve != 0) {
                UPSPower = (CPUPower + FanPower) *
                           max((1.0 - ZoneITEq(Loop).DesignUPSEfficiency * CurveValue(state, ZoneITEq(Loop).UPSEfficFPLRCurve, UPSPartLoadRatio)), 0.0);
            } else {
                UPSPower = (CPUPower + FanPower) * max((1.0 - ZoneITEq(Loop).DesignUPSEfficiency), 0.0);
            }
            UPSHeatGain = UPSPower * ZoneITEq(Loop).UPSLossToZoneFrac;

            // Calculate air outlet conditions and convective heat gain to zone

            AirMassFlowRate = AirVolFlowRate * PsyRhoAirFnPbTdbW(StdBaroPress, TAirIn, WAirIn, RoutineName);
            if (AirMassFlowRate > 0.0) {
                TAirOut = TAirIn + (CPUPower + FanPower) / AirMassFlowRate / PsyCpAirFnW(WAirIn);
            } else {
                TAirOut = TAirIn;
            }

            if (std::abs(TAirOut - TSupply) < SmallTempDiff) {
                TAirOut = TSupply;
            }

            if ((SupplyNodeNum != 0) && (TAirOut != TSupply)) {
                SupplyHeatIndex = (TAirIn - TSupply) / (TAirOut - TSupply);
            } else {
                SupplyHeatIndex = 0.0;
            }

            if (AirConnection == ITEInletAdjustedSupply || AirConnection == ITEInletZoneAirNode) {
                // If not a room air model, then all ITEquip power input is a convective heat gain to the zone heat balance, plus UPS heat gain
                ZoneITEq(Loop).ConGainRateToZone = CPUPower + FanPower + UPSHeatGain;
            } else if (AirConnection == ITEInletRoomAirModel) {
                // Room air model option not implemented yet - set room air model outlet node conditions here
                // If a room air model, then the only convective heat gain to the zone heat balance is the UPS heat gain
                ZoneITEq(Loop).ConGainRateToZone = UPSHeatGain;
            }
            if (Zone(ZoneITEq(Loop).ZonePtr).HasAdjustedReturnTempByITE) {
                ZoneITEMap[ZoneITEq(Loop).ZonePtr].push_back(Loop);
            }
            if (DoingSizing && ZoneITEq(Loop).FlowControlWithApproachTemps) {
                if (ZoneITEq(Loop).FanPowerAtDesign + ZoneITEq(Loop).CPUPowerAtDesign > ZoneITEq(Loop).DesignTotalPower) {
                    ZoneITEq(Loop).ConGainRateToZone = ZoneITEq(Loop).FanPowerAtDesign + ZoneITEq(Loop).CPUPowerAtDesign;
                }
            }
            // Object report variables
            ZoneITEq(Loop).CPUPower = CPUPower;
            ZoneITEq(Loop).FanPower = FanPower;
            ZoneITEq(Loop).UPSPower = UPSPower;
            // ZoneITEq( Loop ).CPUPowerAtDesign = set above
            // ZoneITEq( Loop ).FanPowerAtDesign = set above
            ZoneITEq(Loop).UPSGainRateToZone = UPSHeatGain;
            // ZoneITEq( Loop ).ConGainRateToZone = set above

            ZnRpt(NZ).ITEqCPUPower += ZoneITEq(Loop).CPUPower;
            ZnRpt(NZ).ITEqFanPower += ZoneITEq(Loop).FanPower;
            ZnRpt(NZ).ITEqUPSPower += ZoneITEq(Loop).UPSPower;
            ZnRpt(NZ).ITEqCPUPowerAtDesign += ZoneITEq(Loop).CPUPowerAtDesign;
            ZnRpt(NZ).ITEqFanPowerAtDesign += ZoneITEq(Loop).FanPowerAtDesign;
            ZnRpt(NZ).ITEqUPSGainRateToZone += ZoneITEq(Loop).UPSGainRateToZone;
            ZnRpt(NZ).ITEqConGainRateToZone += ZoneITEq(Loop).ConGainRateToZone;

            ZoneITEq(Loop).CPUConsumption = CPUPower * TimeStepZoneSec;
            ZoneITEq(Loop).FanConsumption = FanPower * TimeStepZoneSec;
            ZoneITEq(Loop).UPSConsumption = UPSPower * TimeStepZoneSec;
            ZoneITEq(Loop).CPUEnergyAtDesign = ZoneITEq(Loop).CPUPowerAtDesign * TimeStepZoneSec;
            ZoneITEq(Loop).FanEnergyAtDesign = ZoneITEq(Loop).FanPowerAtDesign * TimeStepZoneSec;
            ZoneITEq(Loop).UPSGainEnergyToZone = UPSHeatGain * TimeStepZoneSec;
            ZoneITEq(Loop).ConGainEnergyToZone = ZoneITEq(Loop).ConGainRateToZone * TimeStepZoneSec;

            ZnRpt(NZ).ITEqCPUConsumption += ZoneITEq(Loop).CPUConsumption;
            ZnRpt(NZ).ITEqFanConsumption += ZoneITEq(Loop).FanConsumption;
            ZnRpt(NZ).ITEqUPSConsumption += ZoneITEq(Loop).UPSConsumption;
            ZnRpt(NZ).ITEqCPUEnergyAtDesign += ZoneITEq(Loop).CPUEnergyAtDesign;
            ZnRpt(NZ).ITEqFanEnergyAtDesign += ZoneITEq(Loop).FanEnergyAtDesign;
            ZnRpt(NZ).ITEqUPSGainEnergyToZone += ZoneITEq(Loop).UPSGainEnergyToZone;
            ZnRpt(NZ).ITEqConGainEnergyToZone += ZoneITEq(Loop).ConGainEnergyToZone;

            ZoneITEq(Loop).AirVolFlowStdDensity = AirMassFlowRate * StdRhoAir;
            ZoneITEq(Loop).AirVolFlowCurDensity = AirVolFlowRate;
            ZoneITEq(Loop).AirMassFlow = AirMassFlowRate;
            ZoneITEq(Loop).AirInletDryBulbT = TAirIn;
            ZoneITEq(Loop).AirInletDewpointT = TDPAirIn;
            ZoneITEq(Loop).AirInletRelHum = RHAirIn;
            ZoneITEq(Loop).AirOutletDryBulbT = TAirOut;
            ZoneITEq(Loop).SHI = SupplyHeatIndex;

            ZnRpt(NZ).ITEqAirVolFlowStdDensity += ZoneITEq(Loop).AirVolFlowStdDensity;
            ZnRpt(NZ).ITEqAirMassFlow += ZoneITEq(Loop).AirMassFlow;
            ZnRpt(NZ).SumTinMinusTSup += (TAirIn - TSupply) * AirVolFlowRate;
            ZnRpt(NZ).SumToutMinusTSup += (TAirOut - TSupply) * AirVolFlowRate;

            // Check environmental class operating range limits (defined as parameters in this subroutine)
            EnvClass = ZoneITEq(Loop).Class;
            if (EnvClass > 0) {
                if (TAirIn > DBMax(EnvClass)) {
                    ZoneITEq(Loop).TimeAboveDryBulbT = TimeStepZone;
                    ZoneITEq(Loop).TimeOutOfOperRange = TimeStepZone;
                    ZoneITEq(Loop).DryBulbTAboveDeltaT = TAirIn - DBMax(EnvClass);
                    ZnRpt(NZ).ITEqTimeAboveDryBulbT = TimeStepZone;
                    ZnRpt(NZ).ITEqTimeOutOfOperRange = TimeStepZone;
                }
                if (TAirIn < DBMin(EnvClass)) {
                    ZoneITEq(Loop).TimeBelowDryBulbT = TimeStepZone;
                    ZoneITEq(Loop).TimeOutOfOperRange = TimeStepZone;
                    ZoneITEq(Loop).DryBulbTBelowDeltaT = TAirIn - DBMin(EnvClass);
                    ZnRpt(NZ).ITEqTimeBelowDryBulbT = TimeStepZone;
                    ZnRpt(NZ).ITEqTimeOutOfOperRange = TimeStepZone;
                }
                if (TDPAirIn > DPMax(EnvClass)) {
                    ZoneITEq(Loop).TimeAboveDewpointT = TimeStepZone;
                    ZoneITEq(Loop).TimeOutOfOperRange = TimeStepZone;
                    ZoneITEq(Loop).DewpointTAboveDeltaT = TDPAirIn - DPMax(EnvClass);
                    ZnRpt(NZ).ITEqTimeAboveDewpointT = TimeStepZone;
                    ZnRpt(NZ).ITEqTimeOutOfOperRange = TimeStepZone;
                }
                if (TDPAirIn < DPMin(EnvClass)) {
                    ZoneITEq(Loop).TimeBelowDewpointT = TimeStepZone;
                    ZoneITEq(Loop).TimeOutOfOperRange = TimeStepZone;
                    ZoneITEq(Loop).DewpointTBelowDeltaT = TDPAirIn - DPMin(EnvClass);
                    ZnRpt(NZ).ITEqTimeBelowDewpointT = TimeStepZone;
                    ZnRpt(NZ).ITEqTimeOutOfOperRange = TimeStepZone;
                }
                if (RHAirIn > RHMax(EnvClass)) {
                    ZoneITEq(Loop).TimeAboveRH = TimeStepZone;
                    ZoneITEq(Loop).TimeOutOfOperRange = TimeStepZone;
                    ZoneITEq(Loop).RHAboveDeltaRH = RHAirIn - RHMax(EnvClass);
                    ZnRpt(NZ).ITEqTimeAboveRH = TimeStepZone;
                    ZnRpt(NZ).ITEqTimeOutOfOperRange = TimeStepZone;
                }
                if (RHAirIn < RHMin(EnvClass)) {
                    ZoneITEq(Loop).TimeBelowRH = TimeStepZone;
                    ZoneITEq(Loop).TimeOutOfOperRange = TimeStepZone;
                    ZoneITEq(Loop).RHBelowDeltaRH = RHAirIn - RHMin(EnvClass);
                    ZnRpt(NZ).ITEqTimeBelowRH = TimeStepZone;
                    ZnRpt(NZ).ITEqTimeOutOfOperRange = TimeStepZone;
                }
            }


        } // ZoneITEq calc loop

        // Zone-level sensible heat index
        for (Loop = 1; Loop <= NumZoneITEqStatements; ++Loop) {
            int ZN = ZoneITEq(Loop).ZonePtr;
            if (ZnRpt(NZ).SumToutMinusTSup != 0.0) {
                ZnRpt(ZN).ITEqSHI = ZnRpt(NZ).SumTinMinusTSup / ZnRpt(NZ).SumToutMinusTSup;
            }
        }

        std::map<int, std::vector<int>>::iterator it = ZoneITEMap.begin();
        Real64 totalGain;
        Real64 totalRate;
        Real64 TAirReturn;
        while (it != ZoneITEMap.end()) {
            if (Zone(it->first).HasAdjustedReturnTempByITE) {
                totalGain = 0;
                totalRate = 0;
                for (int i : it->second) {
                    if (ZoneITEq(i).ReturnApproachTempSch != 0) {
                        TAirReturn = ZoneITEq(i).AirOutletDryBulbT + GetCurrentScheduleValue(ZoneITEq(i).ReturnApproachTempSch);
                    } else {
                        TAirReturn = ZoneITEq(i).AirOutletDryBulbT + ZoneITEq(i).ReturnApproachTemp;
                    }
                    totalRate += ZoneITEq(i).AirMassFlow;
                    totalGain += ZoneITEq(i).AirMassFlow * TAirReturn;
                }
                if (totalRate != 0) {
                    Zone(it->first).AdjustedReturnTempByITE = totalGain / totalRate;
                    ZnRpt(it->first).ITEAdjReturnTemp = Zone(it->first).AdjustedReturnTempByITE;
                }
            }
            it++;
        }

    } // End CalcZoneITEq

    void ReportInternalHeatGains()
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Richard Liesen
        //       DATE WRITTEN   June 1997
        //       MODIFIED       July 1997 RKS
        //       RE-ENGINEERED  December 1998 LKL

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine currently creates the values for standard "zone loads" reporting
        // from the heat balance module.

        // METHODOLOGY EMPLOYED:
        // The reporting methodology is described in the OutputDataStructure.doc
        // as the "modified modular" format.

        // REFERENCES:
        // OutputDataStructure.doc (EnergyPlus documentation)

        // Using/Aliasing
        using OutputReportTabular::WriteTabularFiles;

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int Loop;
        int ZoneLoop; // Counter for the # of zones (nz)
        static Array1D_int TradIntGainTypes(8,
                                            {IntGainTypeOf_People,
                                             IntGainTypeOf_Lights,
                                             IntGainTypeOf_ElectricEquipment,
                                             IntGainTypeOf_ElectricEquipmentITEAirCooled,
                                             IntGainTypeOf_GasEquipment,
                                             IntGainTypeOf_HotWaterEquipment,
                                             IntGainTypeOf_SteamEquipment,
                                             IntGainTypeOf_OtherEquipment});

        // FLOW:
        for (Loop = 1; Loop <= TotPeople; ++Loop) {
            People(Loop).RadGainEnergy = People(Loop).RadGainRate * TimeStepZoneSec;
            People(Loop).ConGainEnergy = People(Loop).ConGainRate * TimeStepZoneSec;
            People(Loop).SenGainEnergy = People(Loop).SenGainRate * TimeStepZoneSec;
            People(Loop).LatGainEnergy = People(Loop).LatGainRate * TimeStepZoneSec;
            People(Loop).TotGainEnergy = People(Loop).TotGainRate * TimeStepZoneSec;
        }

        for (Loop = 1; Loop <= TotLights; ++Loop) {
            Lights(Loop).Consumption = Lights(Loop).Power * TimeStepZoneSec;
            Lights(Loop).RadGainEnergy = Lights(Loop).RadGainRate * TimeStepZoneSec;
            Lights(Loop).VisGainEnergy = Lights(Loop).VisGainRate * TimeStepZoneSec;
            Lights(Loop).ConGainEnergy = Lights(Loop).ConGainRate * TimeStepZoneSec;
            Lights(Loop).RetAirGainEnergy = Lights(Loop).RetAirGainRate * TimeStepZoneSec;
            Lights(Loop).TotGainEnergy = Lights(Loop).TotGainRate * TimeStepZoneSec;
            if (!WarmupFlag) {
                if (DoOutputReporting && WriteTabularFiles && (KindOfSim == ksRunPeriodWeather)) { // for weather simulations only
                    // for tabular report, accumulate the total electricity used for each Light object
                    Lights(Loop).SumConsumption += Lights(Loop).Consumption;
                    // for tabular report, accumulate the time when each Light has consumption (using a very small threshold instead of zero)
                    if (Lights(Loop).Power > 0.01 * Lights(Loop).DesignLevel) {
                        Lights(Loop).SumTimeNotZeroCons += TimeStepZone;
                    }
                }
            }
        }

        for (Loop = 1; Loop <= TotElecEquip; ++Loop) {
            ZoneElectric(Loop).Consumption = ZoneElectric(Loop).Power * TimeStepZoneSec;
            ZoneElectric(Loop).RadGainEnergy = ZoneElectric(Loop).RadGainRate * TimeStepZoneSec;
            ZoneElectric(Loop).ConGainEnergy = ZoneElectric(Loop).ConGainRate * TimeStepZoneSec;
            ZoneElectric(Loop).LatGainEnergy = ZoneElectric(Loop).LatGainRate * TimeStepZoneSec;
            ZoneElectric(Loop).LostEnergy = ZoneElectric(Loop).LostRate * TimeStepZoneSec;
            ZoneElectric(Loop).TotGainEnergy = ZoneElectric(Loop).TotGainRate * TimeStepZoneSec;
        }

        for (Loop = 1; Loop <= TotGasEquip; ++Loop) {
            ZoneGas(Loop).Consumption = ZoneGas(Loop).Power * TimeStepZoneSec;
            ZoneGas(Loop).RadGainEnergy = ZoneGas(Loop).RadGainRate * TimeStepZoneSec;
            ZoneGas(Loop).ConGainEnergy = ZoneGas(Loop).ConGainRate * TimeStepZoneSec;
            ZoneGas(Loop).LatGainEnergy = ZoneGas(Loop).LatGainRate * TimeStepZoneSec;
            ZoneGas(Loop).LostEnergy = ZoneGas(Loop).LostRate * TimeStepZoneSec;
            ZoneGas(Loop).TotGainEnergy = ZoneGas(Loop).TotGainRate * TimeStepZoneSec;
        }

        for (Loop = 1; Loop <= TotOthEquip; ++Loop) {
            ZoneOtherEq(Loop).Consumption = ZoneOtherEq(Loop).Power * TimeStepZoneSec;
            ZoneOtherEq(Loop).RadGainEnergy = ZoneOtherEq(Loop).RadGainRate * TimeStepZoneSec;
            ZoneOtherEq(Loop).ConGainEnergy = ZoneOtherEq(Loop).ConGainRate * TimeStepZoneSec;
            ZoneOtherEq(Loop).LatGainEnergy = ZoneOtherEq(Loop).LatGainRate * TimeStepZoneSec;
            ZoneOtherEq(Loop).LostEnergy = ZoneOtherEq(Loop).LostRate * TimeStepZoneSec;
            ZoneOtherEq(Loop).TotGainEnergy = ZoneOtherEq(Loop).TotGainRate * TimeStepZoneSec;
        }

        for (Loop = 1; Loop <= TotHWEquip; ++Loop) {
            ZoneHWEq(Loop).Consumption = ZoneHWEq(Loop).Power * TimeStepZoneSec;
            ZoneHWEq(Loop).RadGainEnergy = ZoneHWEq(Loop).RadGainRate * TimeStepZoneSec;
            ZoneHWEq(Loop).ConGainEnergy = ZoneHWEq(Loop).ConGainRate * TimeStepZoneSec;
            ZoneHWEq(Loop).LatGainEnergy = ZoneHWEq(Loop).LatGainRate * TimeStepZoneSec;
            ZoneHWEq(Loop).LostEnergy = ZoneHWEq(Loop).LostRate * TimeStepZoneSec;
            ZoneHWEq(Loop).TotGainEnergy = ZoneHWEq(Loop).TotGainRate * TimeStepZoneSec;
        }

        for (Loop = 1; Loop <= TotStmEquip; ++Loop) {
            ZoneSteamEq(Loop).Consumption = ZoneSteamEq(Loop).Power * TimeStepZoneSec;
            ZoneSteamEq(Loop).RadGainEnergy = ZoneSteamEq(Loop).RadGainRate * TimeStepZoneSec;
            ZoneSteamEq(Loop).ConGainEnergy = ZoneSteamEq(Loop).ConGainRate * TimeStepZoneSec;
            ZoneSteamEq(Loop).LatGainEnergy = ZoneSteamEq(Loop).LatGainRate * TimeStepZoneSec;
            ZoneSteamEq(Loop).LostEnergy = ZoneSteamEq(Loop).LostRate * TimeStepZoneSec;
            ZoneSteamEq(Loop).TotGainEnergy = ZoneSteamEq(Loop).TotGainRate * TimeStepZoneSec;
        }

        for (Loop = 1; Loop <= TotBBHeat; ++Loop) {
            ZoneBBHeat(Loop).Consumption = ZoneBBHeat(Loop).Power * TimeStepZoneSec;
            ZoneBBHeat(Loop).RadGainEnergy = ZoneBBHeat(Loop).RadGainRate * TimeStepZoneSec;
            ZoneBBHeat(Loop).ConGainEnergy = ZoneBBHeat(Loop).ConGainRate * TimeStepZoneSec;
            ZoneBBHeat(Loop).TotGainEnergy = ZoneBBHeat(Loop).TotGainRate * TimeStepZoneSec;
        }

        for (ZoneLoop = 1; ZoneLoop <= NumOfZones; ++ZoneLoop) {
            // People
            ZnRpt(ZoneLoop).PeopleNumOcc = ZoneIntGain(ZoneLoop).NOFOCC;
            ZnRpt(ZoneLoop).PeopleRadGain = ZoneIntGain(ZoneLoop).QOCRAD * TimeStepZoneSec;
            ZnRpt(ZoneLoop).PeopleConGain = ZoneIntGain(ZoneLoop).QOCCON * TimeStepZoneSec;
            ZnRpt(ZoneLoop).PeopleSenGain = ZoneIntGain(ZoneLoop).QOCSEN * TimeStepZoneSec;
            ZnRpt(ZoneLoop).PeopleLatGain = ZoneIntGain(ZoneLoop).QOCLAT * TimeStepZoneSec;
            ZnRpt(ZoneLoop).PeopleTotGain = ZoneIntGain(ZoneLoop).QOCTOT * TimeStepZoneSec;
            ZnRpt(ZoneLoop).PeopleRadGainRate = ZoneIntGain(ZoneLoop).QOCRAD;
            ZnRpt(ZoneLoop).PeopleConGainRate = ZoneIntGain(ZoneLoop).QOCCON;
            ZnRpt(ZoneLoop).PeopleSenGainRate = ZoneIntGain(ZoneLoop).QOCSEN;
            ZnRpt(ZoneLoop).PeopleLatGainRate = ZoneIntGain(ZoneLoop).QOCLAT;
            ZnRpt(ZoneLoop).PeopleTotGainRate = ZoneIntGain(ZoneLoop).QOCTOT;

            // General Lights
            ZnRpt(ZoneLoop).LtsRetAirGain = ZoneIntGain(ZoneLoop).QLTCRA * TimeStepZoneSec;
            ZnRpt(ZoneLoop).LtsRadGain = ZoneIntGain(ZoneLoop).QLTRAD * TimeStepZoneSec;
            ZnRpt(ZoneLoop).LtsTotGain = ZoneIntGain(ZoneLoop).QLTTOT * TimeStepZoneSec;
            ZnRpt(ZoneLoop).LtsConGain = ZoneIntGain(ZoneLoop).QLTCON * TimeStepZoneSec;
            ZnRpt(ZoneLoop).LtsVisGain = ZoneIntGain(ZoneLoop).QLTSW * TimeStepZoneSec;
            ZnRpt(ZoneLoop).LtsRetAirGainRate = ZoneIntGain(ZoneLoop).QLTCRA;
            ZnRpt(ZoneLoop).LtsRadGainRate = ZoneIntGain(ZoneLoop).QLTRAD;
            ZnRpt(ZoneLoop).LtsTotGainRate = ZoneIntGain(ZoneLoop).QLTTOT;
            ZnRpt(ZoneLoop).LtsConGainRate = ZoneIntGain(ZoneLoop).QLTCON;
            ZnRpt(ZoneLoop).LtsVisGainRate = ZoneIntGain(ZoneLoop).QLTSW;
            ZnRpt(ZoneLoop).LtsElecConsump = ZnRpt(ZoneLoop).LtsTotGain;

            // Electric Equipment
            ZnRpt(ZoneLoop).ElecConGain = ZoneIntGain(ZoneLoop).QEECON * TimeStepZoneSec;
            ZnRpt(ZoneLoop).ElecRadGain = ZoneIntGain(ZoneLoop).QEERAD * TimeStepZoneSec;
            ZnRpt(ZoneLoop).ElecLatGain = ZoneIntGain(ZoneLoop).QEELAT * TimeStepZoneSec;
            ZnRpt(ZoneLoop).ElecLost = ZoneIntGain(ZoneLoop).QEELost * TimeStepZoneSec;
            ZnRpt(ZoneLoop).ElecConGainRate = ZoneIntGain(ZoneLoop).QEECON;
            ZnRpt(ZoneLoop).ElecRadGainRate = ZoneIntGain(ZoneLoop).QEERAD;
            ZnRpt(ZoneLoop).ElecLatGainRate = ZoneIntGain(ZoneLoop).QEELAT;
            ZnRpt(ZoneLoop).ElecLostRate = ZoneIntGain(ZoneLoop).QEELost;
            ZnRpt(ZoneLoop).ElecConsump =
                ZnRpt(ZoneLoop).ElecConGain + ZnRpt(ZoneLoop).ElecRadGain + ZnRpt(ZoneLoop).ElecLatGain + ZnRpt(ZoneLoop).ElecLost;
            ZnRpt(ZoneLoop).ElecTotGain = ZnRpt(ZoneLoop).ElecConGain + ZnRpt(ZoneLoop).ElecRadGain + ZnRpt(ZoneLoop).ElecLatGain;
            ZnRpt(ZoneLoop).ElecTotGainRate = ZnRpt(ZoneLoop).ElecConGainRate + ZnRpt(ZoneLoop).ElecRadGainRate + ZnRpt(ZoneLoop).ElecLatGainRate;

            // Gas Equipment
            ZnRpt(ZoneLoop).GasConGain = ZoneIntGain(ZoneLoop).QGECON * TimeStepZoneSec;
            ZnRpt(ZoneLoop).GasRadGain = ZoneIntGain(ZoneLoop).QGERAD * TimeStepZoneSec;
            ZnRpt(ZoneLoop).GasLatGain = ZoneIntGain(ZoneLoop).QGELAT * TimeStepZoneSec;
            ZnRpt(ZoneLoop).GasLost = ZoneIntGain(ZoneLoop).QGELost * TimeStepZoneSec;
            ZnRpt(ZoneLoop).GasConGainRate = ZoneIntGain(ZoneLoop).QGECON;
            ZnRpt(ZoneLoop).GasRadGainRate = ZoneIntGain(ZoneLoop).QGERAD;
            ZnRpt(ZoneLoop).GasLatGainRate = ZoneIntGain(ZoneLoop).QGELAT;
            ZnRpt(ZoneLoop).GasLostRate = ZoneIntGain(ZoneLoop).QGELost;
            ZnRpt(ZoneLoop).GasConsump =
                ZnRpt(ZoneLoop).GasConGain + ZnRpt(ZoneLoop).GasRadGain + ZnRpt(ZoneLoop).GasLatGain + ZnRpt(ZoneLoop).GasLost;
            ZnRpt(ZoneLoop).GasTotGain = ZnRpt(ZoneLoop).GasConGain + ZnRpt(ZoneLoop).GasRadGain + ZnRpt(ZoneLoop).GasLatGain;
            ZnRpt(ZoneLoop).GasTotGainRate = ZnRpt(ZoneLoop).GasConGainRate + ZnRpt(ZoneLoop).GasRadGainRate + ZnRpt(ZoneLoop).GasLatGainRate;

            // Hot Water Equipment
            ZnRpt(ZoneLoop).HWConGain = ZoneIntGain(ZoneLoop).QHWCON * TimeStepZoneSec;
            ZnRpt(ZoneLoop).HWRadGain = ZoneIntGain(ZoneLoop).QHWRAD * TimeStepZoneSec;
            ZnRpt(ZoneLoop).HWLatGain = ZoneIntGain(ZoneLoop).QHWLAT * TimeStepZoneSec;
            ZnRpt(ZoneLoop).HWLost = ZoneIntGain(ZoneLoop).QHWLost * TimeStepZoneSec;
            ZnRpt(ZoneLoop).HWConGainRate = ZoneIntGain(ZoneLoop).QHWCON;
            ZnRpt(ZoneLoop).HWRadGainRate = ZoneIntGain(ZoneLoop).QHWRAD;
            ZnRpt(ZoneLoop).HWLatGainRate = ZoneIntGain(ZoneLoop).QHWLAT;
            ZnRpt(ZoneLoop).HWLostRate = ZoneIntGain(ZoneLoop).QHWLost;
            ZnRpt(ZoneLoop).HWConsump = ZnRpt(ZoneLoop).HWConGain + ZnRpt(ZoneLoop).HWRadGain + ZnRpt(ZoneLoop).HWLatGain + ZnRpt(ZoneLoop).HWLost;
            ZnRpt(ZoneLoop).HWTotGain = ZnRpt(ZoneLoop).HWConGain + ZnRpt(ZoneLoop).HWRadGain + ZnRpt(ZoneLoop).HWLatGain;
            ZnRpt(ZoneLoop).HWTotGainRate = ZnRpt(ZoneLoop).HWConGainRate + ZnRpt(ZoneLoop).HWRadGainRate + ZnRpt(ZoneLoop).HWLatGainRate;

            // Steam Equipment
            ZnRpt(ZoneLoop).SteamConGain = ZoneIntGain(ZoneLoop).QSECON * TimeStepZoneSec;
            ZnRpt(ZoneLoop).SteamRadGain = ZoneIntGain(ZoneLoop).QSERAD * TimeStepZoneSec;
            ZnRpt(ZoneLoop).SteamLatGain = ZoneIntGain(ZoneLoop).QSELAT * TimeStepZoneSec;
            ZnRpt(ZoneLoop).SteamLost = ZoneIntGain(ZoneLoop).QSELost * TimeStepZoneSec;
            ZnRpt(ZoneLoop).SteamConGainRate = ZoneIntGain(ZoneLoop).QSECON;
            ZnRpt(ZoneLoop).SteamRadGainRate = ZoneIntGain(ZoneLoop).QSERAD;
            ZnRpt(ZoneLoop).SteamLatGainRate = ZoneIntGain(ZoneLoop).QSELAT;
            ZnRpt(ZoneLoop).SteamLostRate = ZoneIntGain(ZoneLoop).QSELost;
            ZnRpt(ZoneLoop).SteamConsump =
                ZnRpt(ZoneLoop).SteamConGain + ZnRpt(ZoneLoop).SteamRadGain + ZnRpt(ZoneLoop).SteamLatGain + ZnRpt(ZoneLoop).SteamLost;
            ZnRpt(ZoneLoop).SteamTotGain = ZnRpt(ZoneLoop).SteamConGain + ZnRpt(ZoneLoop).SteamRadGain + ZnRpt(ZoneLoop).SteamLatGain;
            ZnRpt(ZoneLoop).SteamTotGainRate = ZnRpt(ZoneLoop).SteamConGainRate + ZnRpt(ZoneLoop).SteamRadGainRate + ZnRpt(ZoneLoop).SteamLatGainRate;

            // Other Equipment
            ZnRpt(ZoneLoop).OtherConGain = ZoneIntGain(ZoneLoop).QOECON * TimeStepZoneSec;
            ZnRpt(ZoneLoop).OtherRadGain = ZoneIntGain(ZoneLoop).QOERAD * TimeStepZoneSec;
            ZnRpt(ZoneLoop).OtherLatGain = ZoneIntGain(ZoneLoop).QOELAT * TimeStepZoneSec;
            ZnRpt(ZoneLoop).OtherLost = ZoneIntGain(ZoneLoop).QOELost * TimeStepZoneSec;
            ZnRpt(ZoneLoop).OtherConGainRate = ZoneIntGain(ZoneLoop).QOECON;
            ZnRpt(ZoneLoop).OtherRadGainRate = ZoneIntGain(ZoneLoop).QOERAD;
            ZnRpt(ZoneLoop).OtherLatGainRate = ZoneIntGain(ZoneLoop).QOELAT;
            ZnRpt(ZoneLoop).OtherLostRate = ZoneIntGain(ZoneLoop).QOELost;
            ZnRpt(ZoneLoop).OtherConsump =
                ZnRpt(ZoneLoop).OtherConGain + ZnRpt(ZoneLoop).OtherRadGain + ZnRpt(ZoneLoop).OtherLatGain + ZnRpt(ZoneLoop).OtherLost;
            ZnRpt(ZoneLoop).OtherTotGain = ZnRpt(ZoneLoop).OtherConGain + ZnRpt(ZoneLoop).OtherRadGain + ZnRpt(ZoneLoop).OtherLatGain;
            ZnRpt(ZoneLoop).OtherTotGainRate = ZnRpt(ZoneLoop).OtherConGainRate + ZnRpt(ZoneLoop).OtherRadGainRate + ZnRpt(ZoneLoop).OtherLatGainRate;

            // Baseboard Heat
            ZnRpt(ZoneLoop).BaseHeatConGain = ZoneIntGain(ZoneLoop).QBBCON * TimeStepZoneSec;
            ZnRpt(ZoneLoop).BaseHeatRadGain = ZoneIntGain(ZoneLoop).QBBRAD * TimeStepZoneSec;
            ZnRpt(ZoneLoop).BaseHeatConGainRate = ZoneIntGain(ZoneLoop).QBBCON;
            ZnRpt(ZoneLoop).BaseHeatRadGainRate = ZoneIntGain(ZoneLoop).QBBRAD;
            ZnRpt(ZoneLoop).BaseHeatTotGain = ZnRpt(ZoneLoop).BaseHeatConGain + ZnRpt(ZoneLoop).BaseHeatRadGain;
            ZnRpt(ZoneLoop).BaseHeatTotGainRate = ZnRpt(ZoneLoop).BaseHeatConGainRate + ZnRpt(ZoneLoop).BaseHeatRadGainRate;
            ZnRpt(ZoneLoop).BaseHeatElecCons = ZnRpt(ZoneLoop).BaseHeatTotGain;

            // Overall Zone Variables

            // these overalls include component gains from devices like water heater, water use, and generators
            //   working vars QFCConv QGenConv QFCRad QGenRad  WaterUseLatentGain WaterThermalTankGain WaterUseSensibleGain

            ZnRpt(ZoneLoop).TotVisHeatGain = ZnRpt(ZoneLoop).LtsVisGain;
            ZnRpt(ZoneLoop).TotVisHeatGainRate = ZnRpt(ZoneLoop).LtsVisGainRate;

            SumInternalRadiationGainsByTypes(ZoneLoop, TradIntGainTypes, ZnRpt(ZoneLoop).TotRadiantGainRate);
            ZnRpt(ZoneLoop).TotRadiantGain = ZnRpt(ZoneLoop).TotRadiantGainRate * TimeStepZoneSec;

            SumInternalConvectionGainsByTypes(ZoneLoop, TradIntGainTypes, ZnRpt(ZoneLoop).TotConvectiveGainRate);
            ZnRpt(ZoneLoop).TotConvectiveGain = ZnRpt(ZoneLoop).TotConvectiveGainRate * TimeStepZoneSec;

            SumInternalLatentGainsByTypes(ZoneLoop, TradIntGainTypes, ZnRpt(ZoneLoop).TotLatentGainRate);
            ZnRpt(ZoneLoop).TotLatentGain = ZnRpt(ZoneLoop).TotLatentGainRate * TimeStepZoneSec;

            ZnRpt(ZoneLoop).TotTotalHeatGainRate = ZnRpt(ZoneLoop).TotLatentGainRate + ZnRpt(ZoneLoop).TotRadiantGainRate +
                                                   ZnRpt(ZoneLoop).TotConvectiveGainRate + ZnRpt(ZoneLoop).TotVisHeatGainRate;
            ZnRpt(ZoneLoop).TotTotalHeatGain = ZnRpt(ZoneLoop).TotTotalHeatGainRate * TimeStepZoneSec;
        }
    }

    Real64 GetDesignLightingLevelForZone(int const WhichZone) // name of zone
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         Linda Lawrie
        //       DATE WRITTEN   April 2007; January 2008 - moved to InternalGains
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS FUNCTION:
        // This routine sums the Lighting Level for a zone.
        // Will issue a severe error for illegal zone.
        // Must be called after InternalHeatGains get input.

        // Using/Aliasing
        using namespace DataHeatBalance;
        using namespace DataGlobals;

        // Return value
        Real64 DesignLightingLevelSum; // Sum of design lighting level for this zone

        // FUNCTION LOCAL VARIABLE DECLARATIONS:
        int Loop;

        if (GetInternalHeatGainsInputFlag) {
            ShowFatalError("GetDesignLightingLevelForZone: Function called prior to Getting Lights Input.");
        }

        DesignLightingLevelSum = 0.0;

        for (Loop = 1; Loop <= TotLights; ++Loop) {
            if (Lights(Loop).ZonePtr == WhichZone) {
                DesignLightingLevelSum += Lights(Loop).DesignLevel;
            }
        }

        return DesignLightingLevelSum;
    }

    bool CheckThermalComfortSchedules(bool const WorkEffSch, // Blank work efficiency schedule = true
                                      bool const CloInsSch,  // Blank clothing insulation schedule = true
                                      bool const AirVeloSch) // Blank air velocity schedule = true
    {
        bool TCSchedsPresent = false;

        if (!WorkEffSch || !CloInsSch || !AirVeloSch) {
            TCSchedsPresent = true;
        }

        return TCSchedsPresent;
    }

    void CheckLightsReplaceableMinMaxForZone(int const WhichZone) // Zone Number
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda Lawrie
        //       DATE WRITTEN   April 2007
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Daylighting is not available unless Lights (replaceable) is 0.0 or 1.0.  No dimming will be done
        // unless the lights replaceable fraction is 1.0.  This is documented in the InputOutputReference but
        // not warned about.  Also, this will sum the Zone Design Lighting level, in case the calling routine
        // would like to have an error if the lights is zero and daylighting is requested.

        // METHODOLOGY EMPLOYED:
        // Traverse the LIGHTS structure and get fraction replaceable - min/max as well as lighting
        // level for a zone.

        // Using/Aliasing
        using namespace DataDaylighting;

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int Loop;
        Real64 LightsRepMin; // Minimum Lighting replacement fraction for any lights statement for this zone
        Real64 LightsRepMax; // Maximum Lighting replacement fraction for any lights statement for this zone
        int NumLights;       // Number of Lights statement for that zone.

        if (GetInternalHeatGainsInputFlag) {
            ShowFatalError("CheckLightsReplaceableMinMaxForZone: Function called prior to Getting Lights Input.");
        }

        LightsRepMin = 99999.0;
        LightsRepMax = -99999.0;
        NumLights = 0;

        for (Loop = 1; Loop <= TotLights; ++Loop) {
            if (Lights(Loop).ZonePtr != WhichZone) continue;
            LightsRepMin = min(LightsRepMin, Lights(Loop).FractionReplaceable);
            LightsRepMax = max(LightsRepMax, Lights(Loop).FractionReplaceable);
            ++NumLights;
            if ((ZoneDaylight(Lights(Loop).ZonePtr).DaylightMethod == SplitFluxDaylighting ||
                 ZoneDaylight(Lights(Loop).ZonePtr).DaylightMethod == DElightDaylighting) &&
                (Lights(Loop).FractionReplaceable > 0.0 && Lights(Loop).FractionReplaceable < 1.0)) {
                ShowWarningError("CheckLightsReplaceableMinMaxForZone: Fraction Replaceable must be 0.0 or 1.0 if used with daylighting.");
                ShowContinueError("..Lights=\"" + Lights(Loop).Name + "\", Fraction Replaceable will be reset to 1.0 to allow dimming controls");
                ShowContinueError("..in Zone=" + Zone(WhichZone).Name);
                Lights(Loop).FractionReplaceable = 1.0;
            }
        }

        if (ZoneDaylight(WhichZone).DaylightMethod == SplitFluxDaylighting) {
            if (LightsRepMax == 0.0) {
                ShowWarningError("CheckLightsReplaceable: Zone \"" + Zone(WhichZone).Name + "\" has Daylighting:Controls.");
                ShowContinueError("but all of the LIGHTS object in that zone have zero Fraction Replaceable.");
                ShowContinueError("The daylighting controls will have no effect.");
            }
            if (NumLights == 0) {
                ShowWarningError("CheckLightsReplaceable: Zone \"" + Zone(WhichZone).Name + "\" has Daylighting:Controls.");
                ShowContinueError("but there are no LIGHTS objects in that zone.");
                ShowContinueError("The daylighting controls will have no effect.");
            }
        } else if (ZoneDaylight(WhichZone).DaylightMethod == DElightDaylighting) {
            if (LightsRepMax == 0.0) {
                ShowWarningError("CheckLightsReplaceable: Zone \"" + Zone(WhichZone).Name + "\" has Daylighting:Controls.");
                ShowContinueError("but all of the LIGHTS object in that zone have zero Fraction Replaceable.");
                ShowContinueError("The daylighting controls will have no effect.");
            }
            if (NumLights == 0) {
                ShowWarningError("CheckLightsReplaceable: Zone \"" + Zone(WhichZone).Name + "\" has Daylighting:Controls.");
                ShowContinueError("but there are no LIGHTS objects in that zone.");
                ShowContinueError("The daylighting controls will have no effect.");
            }
        }
    }

    void UpdateInternalGainValues(Optional_bool_const SuppressRadiationUpdate, Optional_bool_const SumLatentGains)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         B. Griffith
        //       DATE WRITTEN   Dec. 2011
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // Using/Aliasing
        using DataContaminantBalance::Contaminant;
        using DataContaminantBalance::ZoneGCGain;
        using DataHeatBalFanSys::ZoneLatentGain;
        using DataHeatBalFanSys::ZoneLatentGainExceptPeople; // Added for hybrid model

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int Loop;
        int NZ;
        bool DoRadiationUpdate;
        bool ReSumLatentGains;

        DoRadiationUpdate = true;
        ReSumLatentGains = false;

        if (present(SuppressRadiationUpdate)) {
            if (SuppressRadiationUpdate) DoRadiationUpdate = false;
        }

        if (present(SumLatentGains)) {
            if (SumLatentGains) ReSumLatentGains = true;
        }

        // store pointer values to hold generic internal gain values constant for entire timestep
        for (NZ = 1; NZ <= NumOfZones; ++NZ) {
            for (Loop = 1; Loop <= ZoneIntGain(NZ).NumberOfDevices; ++Loop) {
                ZoneIntGain(NZ).Device(Loop).ConvectGainRate = *ZoneIntGain(NZ).Device(Loop).PtrConvectGainRate;
                ZoneIntGain(NZ).Device(Loop).ReturnAirConvGainRate = *ZoneIntGain(NZ).Device(Loop).PtrReturnAirConvGainRate;
                if (DoRadiationUpdate) ZoneIntGain(NZ).Device(Loop).RadiantGainRate = *ZoneIntGain(NZ).Device(Loop).PtrRadiantGainRate;
                ZoneIntGain(NZ).Device(Loop).LatentGainRate = *ZoneIntGain(NZ).Device(Loop).PtrLatentGainRate;
                ZoneIntGain(NZ).Device(Loop).ReturnAirLatentGainRate = *ZoneIntGain(NZ).Device(Loop).PtrReturnAirLatentGainRate;
                ZoneIntGain(NZ).Device(Loop).CarbonDioxideGainRate = *ZoneIntGain(NZ).Device(Loop).PtrCarbonDioxideGainRate;
                ZoneIntGain(NZ).Device(Loop).GenericContamGainRate = *ZoneIntGain(NZ).Device(Loop).PtrGenericContamGainRate;
            }
            if (ReSumLatentGains) {
                SumAllInternalLatentGains(NZ, ZoneLatentGain(NZ));
                // Added for the hybrid model
                if (HybridModel::FlagHybridModel_PC) {
                    SumAllInternalLatentGainsExceptPeople(NZ, ZoneLatentGainExceptPeople(NZ));
                }
            }
        }

        if (Contaminant.GenericContamSimulation && allocated(ZoneGCGain)) {
            for (NZ = 1; NZ <= NumOfZones; ++NZ) {
                SumAllInternalGenericContamGains(NZ, ZoneGCGain(NZ));
                ZnRpt(NZ).GCRate = ZoneGCGain(NZ);
            }
        }
    }

    void SumAllInternalConvectionGains(int const ZoneNum, // zone index pointer for which zone to sum gains for
                                       Real64 &SumConvGainRate)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         B. Griffith
        //       DATE WRITTEN   Nov. 2011
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // worker routine for summing all the internal gain types

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 tmpSumConvGainRate;
        int DeviceNum;

        tmpSumConvGainRate = 0.0;
        if (ZoneIntGain(ZoneNum).NumberOfDevices == 0) {
            SumConvGainRate = 0.0;
            return;
        }

        for (DeviceNum = 1; DeviceNum <= ZoneIntGain(ZoneNum).NumberOfDevices; ++DeviceNum) {
            tmpSumConvGainRate += ZoneIntGain(ZoneNum).Device(DeviceNum).ConvectGainRate;
        }

        SumConvGainRate = tmpSumConvGainRate;
    }

    // For HybridModel
    void SumAllInternalConvectionGainsExceptPeople(int const ZoneNum, Real64 &SumConvGainRateExceptPeople)
    {
        Real64 tmpSumConvGainRateExceptPeople;
        int DeviceNum;
        std::string str_people = "PEOPLE";
        tmpSumConvGainRateExceptPeople = 0.0;

        if (ZoneIntGain(ZoneNum).NumberOfDevices == 0) {
            SumConvGainRateExceptPeople = 0.0;
            return;
        }

        for (DeviceNum = 1; DeviceNum <= ZoneIntGain(ZoneNum).NumberOfDevices; ++DeviceNum) {
            if (ZoneIntGain(ZoneNum).Device(DeviceNum).CompObjectType != str_people) {
                tmpSumConvGainRateExceptPeople += ZoneIntGain(ZoneNum).Device(DeviceNum).ConvectGainRate;
            }
        }

        SumConvGainRateExceptPeople = tmpSumConvGainRateExceptPeople;
    }

    void SumInternalConvectionGainsByTypes(int const ZoneNum,             // zone index pointer for which zone to sum gains for
                                           const Array1D_int &GainTypeARR, // variable length 1-d array of integer valued gain types
                                           Real64 &SumConvGainRate)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         B. Griffith
        //       DATE WRITTEN   Nov. 2011cl
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // worker routine for summing a subset of the internal gain types

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int NumberOfTypes;
        Real64 tmpSumConvGainRate;
        int DeviceNum;
        int TypeNum;

        NumberOfTypes = size(GainTypeARR);
        tmpSumConvGainRate = 0.0;

        if (ZoneIntGain(ZoneNum).NumberOfDevices == 0) {
            SumConvGainRate = 0.0;
            return;
        }

        for (DeviceNum = 1; DeviceNum <= ZoneIntGain(ZoneNum).NumberOfDevices; ++DeviceNum) {
            for (TypeNum = 1; TypeNum <= NumberOfTypes; ++TypeNum) {

                if (ZoneIntGain(ZoneNum).Device(DeviceNum).CompTypeOfNum == GainTypeARR(TypeNum)) {
                    tmpSumConvGainRate += ZoneIntGain(ZoneNum).Device(DeviceNum).ConvectGainRate;
                }
            }
        }

        SumConvGainRate = tmpSumConvGainRate;
    }

    void SumAllReturnAirConvectionGains(int const ZoneNum, // zone index pointer for which zone to sum gains for
                                        Real64 &SumReturnAirGainRate,
                                        int const ReturnNodeNum // return air node number
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         B. Griffith
        //       DATE WRITTEN   Dec. 2011

        // PURPOSE OF THIS SUBROUTINE:
        // worker routine for summing all the internal gain types

        Real64 tmpSumRetAirGainRate;
        int DeviceNum;

        tmpSumRetAirGainRate = 0.0;

        if (ZoneIntGain(ZoneNum).NumberOfDevices == 0) {
            SumReturnAirGainRate = 0.0;
            return;
        }

        for (DeviceNum = 1; DeviceNum <= ZoneIntGain(ZoneNum).NumberOfDevices; ++DeviceNum) {
            // If ReturnNodeNum is zero, sum for entire zone, otherwise sum only for specified ReturnNodeNum
            if ((ReturnNodeNum == 0) || (ReturnNodeNum == ZoneIntGain(ZoneNum).Device(DeviceNum).ReturnAirNodeNum)) {
                tmpSumRetAirGainRate += ZoneIntGain(ZoneNum).Device(DeviceNum).ReturnAirConvGainRate;
            }
        }

        SumReturnAirGainRate = tmpSumRetAirGainRate;
    }

    void SumReturnAirConvectionGainsByTypes(int const ZoneNum,             // zone index pointer for which zone to sum gains for
                                            const Array1D_int &GainTypeARR, // variable length 1-d array of integer valued gain types
                                            Real64 &SumReturnAirGainRate)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         B. Griffith
        //       DATE WRITTEN   Nov. 2011
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // worker routine for summing a subset of the internal gain types

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int NumberOfTypes;
        Real64 tmpSumRetAirConvGainRate;
        int DeviceNum;
        int TypeNum;

        NumberOfTypes = size(GainTypeARR);
        tmpSumRetAirConvGainRate = 0.0;

        if (ZoneIntGain(ZoneNum).NumberOfDevices == 0) {
            SumReturnAirGainRate = 0.0;
            return;
        }

        for (DeviceNum = 1; DeviceNum <= ZoneIntGain(ZoneNum).NumberOfDevices; ++DeviceNum) {
            for (TypeNum = 1; TypeNum <= NumberOfTypes; ++TypeNum) {

                if (ZoneIntGain(ZoneNum).Device(DeviceNum).CompTypeOfNum == GainTypeARR(TypeNum)) {
                    tmpSumRetAirConvGainRate += ZoneIntGain(ZoneNum).Device(DeviceNum).ReturnAirConvGainRate;
                }
            }
        }

        SumReturnAirGainRate = tmpSumRetAirConvGainRate;
    }

    void SumAllInternalRadiationGains(int const ZoneNum, // zone index pointer for which zone to sum gains for
                                      Real64 &SumRadGainRate)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         B. Griffith
        //       DATE WRITTEN   Nov. 2011
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // worker routine for summing all the internal gain types

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 tmpSumRadGainRate;
        int DeviceNum;

        tmpSumRadGainRate = 0.0;

        if (ZoneIntGain(ZoneNum).NumberOfDevices == 0) {
            SumRadGainRate = 0.0;
            return;
        }

        for (DeviceNum = 1; DeviceNum <= ZoneIntGain(ZoneNum).NumberOfDevices; ++DeviceNum) {
            tmpSumRadGainRate += ZoneIntGain(ZoneNum).Device(DeviceNum).RadiantGainRate;
        }

        SumRadGainRate = tmpSumRadGainRate;
    }

    void SumInternalRadiationGainsByTypes(int const ZoneNum,             // zone index pointer for which zone to sum gains for
                                          const Array1D_int &GainTypeARR, // variable length 1-d array of integer valued gain types
                                          Real64 &SumRadiationGainRate)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         B. Griffith
        //       DATE WRITTEN   Dec. 2011
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // worker routine for summing a subset of the internal gain types

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int NumberOfTypes;
        Real64 tmpSumRadiationGainRate;
        int DeviceNum;
        int TypeNum;

        NumberOfTypes = size(GainTypeARR);
        tmpSumRadiationGainRate = 0.0;

        if (ZoneIntGain(ZoneNum).NumberOfDevices == 0) {
            SumRadiationGainRate = 0.0;
            return;
        }

        for (DeviceNum = 1; DeviceNum <= ZoneIntGain(ZoneNum).NumberOfDevices; ++DeviceNum) {
            for (TypeNum = 1; TypeNum <= NumberOfTypes; ++TypeNum) {

                if (ZoneIntGain(ZoneNum).Device(DeviceNum).CompTypeOfNum == GainTypeARR(TypeNum)) {
                    tmpSumRadiationGainRate += ZoneIntGain(ZoneNum).Device(DeviceNum).RadiantGainRate;
                }
            }
        }

        SumRadiationGainRate = tmpSumRadiationGainRate;
    }

    void SumAllInternalLatentGains(int const ZoneNum, // zone index pointer for which zone to sum gains for
                                   Real64 &SumLatentGainRate)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         B. Griffith
        //       DATE WRITTEN   Nov. 2011
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // worker routine for summing all the internal gain types

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 tmpSumLatentGainRate;
        int DeviceNum;

        tmpSumLatentGainRate = 0.0;

        if (ZoneIntGain(ZoneNum).NumberOfDevices == 0) {
            SumLatentGainRate = 0.0;
            return;
        }

        for (DeviceNum = 1; DeviceNum <= ZoneIntGain(ZoneNum).NumberOfDevices; ++DeviceNum) {
            tmpSumLatentGainRate += ZoneIntGain(ZoneNum).Device(DeviceNum).LatentGainRate;
        }

        SumLatentGainRate = tmpSumLatentGainRate;
    }

    // Added for hybrid model -- calculate the latent gain from all sources except for people
    void SumAllInternalLatentGainsExceptPeople(int const ZoneNum, // zone index pointer for which zone to sum gains for
                                               Real64 &SumLatentGainRateExceptPeople)
    {
        if (ZoneIntGain(ZoneNum).NumberOfDevices == 0) {
            SumLatentGainRateExceptPeople = 0.0;
            return;
        }
        for (int DeviceNum = 1; DeviceNum <= ZoneIntGain(ZoneNum).NumberOfDevices; ++DeviceNum) {
            if (ZoneIntGain(ZoneNum).Device(DeviceNum).CompTypeOfNum != IntGainTypeOf_People) {
                SumLatentGainRateExceptPeople += ZoneIntGain(ZoneNum).Device(DeviceNum).LatentGainRate;
            }
        }
    }

    void SumInternalLatentGainsByTypes(int const ZoneNum,              // zone index pointer for which zone to sum gains for
                                       const Array1D_int &GainTypeARR, // variable length 1-d array of integer valued gain types
                                       Real64 &SumLatentGainRate)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         B. Griffith
        //       DATE WRITTEN   Dec. 2011
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // worker routine for summing a subset of the internal gain types

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int NumberOfTypes;
        Real64 tmpSumLatentGainRate;
        int DeviceNum;
        int TypeNum;

        NumberOfTypes = size(GainTypeARR);
        tmpSumLatentGainRate = 0.0;

        if (ZoneIntGain(ZoneNum).NumberOfDevices == 0) {
            SumLatentGainRate = 0.0;
            return;
        }

        for (DeviceNum = 1; DeviceNum <= ZoneIntGain(ZoneNum).NumberOfDevices; ++DeviceNum) {
            for (TypeNum = 1; TypeNum <= NumberOfTypes; ++TypeNum) {

                if (ZoneIntGain(ZoneNum).Device(DeviceNum).CompTypeOfNum == GainTypeARR(TypeNum)) {
                    tmpSumLatentGainRate += ZoneIntGain(ZoneNum).Device(DeviceNum).LatentGainRate;
                }
            }
        }

        SumLatentGainRate = tmpSumLatentGainRate;
    }

    void SumAllReturnAirLatentGains(int const ZoneNum, // zone index pointer for which zone to sum gains for
                                    Real64 &SumRetAirLatentGainRate,
                                    int const ReturnNodeNum // return air node number
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         B. Griffith
        //       DATE WRITTEN   Nov. 2011
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // worker routine for summing all the internal gain types

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 tmpSumLatentGainRate;
        int DeviceNum;

        tmpSumLatentGainRate = 0.0;

        if (ZoneIntGain(ZoneNum).NumberOfDevices == 0) {
            SumRetAirLatentGainRate = 0.0;
            return;
        }

        for (DeviceNum = 1; DeviceNum <= ZoneIntGain(ZoneNum).NumberOfDevices; ++DeviceNum) {
            // If ReturnNodeNum is zero, sum for entire zone, otherwise sum only for specified ReturnNodeNum
            if ((ReturnNodeNum == 0) || (ReturnNodeNum == ZoneIntGain(ZoneNum).Device(DeviceNum).ReturnAirNodeNum)) {
                tmpSumLatentGainRate += ZoneIntGain(ZoneNum).Device(DeviceNum).ReturnAirLatentGainRate;
            }
        }

        SumRetAirLatentGainRate = tmpSumLatentGainRate;
    }

    void SumAllInternalCO2Gains(int const ZoneNum, // zone index pointer for which zone to sum gains for
                                Real64 &SumCO2GainRate)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         B. Griffith
        //       DATE WRITTEN   Dec. 2011
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // worker routine for summing all the internal gain types

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 tmpSumCO2GainRate;
        int DeviceNum;

        tmpSumCO2GainRate = 0.0;

        if (ZoneIntGain(ZoneNum).NumberOfDevices == 0) {
            SumCO2GainRate = 0.0;
            return;
        }

        for (DeviceNum = 1; DeviceNum <= ZoneIntGain(ZoneNum).NumberOfDevices; ++DeviceNum) {
            tmpSumCO2GainRate += ZoneIntGain(ZoneNum).Device(DeviceNum).CarbonDioxideGainRate;
        }

        SumCO2GainRate = tmpSumCO2GainRate;
    }

    // Added for hybrid model -- function for calculating CO2 gains except people
    void SumAllInternalCO2GainsExceptPeople(int const ZoneNum, // zone index pointer for which zone to sum gains for
                                            Real64 &SumCO2GainRateExceptPeople)
    {
        if (ZoneIntGain(ZoneNum).NumberOfDevices == 0) {
            SumCO2GainRateExceptPeople = 0.0;
            return;
        }

        for (int DeviceNum = 1; DeviceNum <= ZoneIntGain(ZoneNum).NumberOfDevices; ++DeviceNum) {
            if (ZoneIntGain(ZoneNum).Device(DeviceNum).CompTypeOfNum != IntGainTypeOf_People) {
                SumCO2GainRateExceptPeople += ZoneIntGain(ZoneNum).Device(DeviceNum).CarbonDioxideGainRate;
            }
        }
    }

    void SumInternalCO2GainsByTypes(int const ZoneNum,              // zone index pointer for which zone to sum gains for
                                    const Array1D_int &GainTypeARR, // variable length 1-d array of integer valued gain types
                                    Real64 &SumCO2GainRate)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         B. Griffith
        //       DATE WRITTEN   Dec. 2011
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // worker routine for summing a subset of the internal gain types

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int NumberOfTypes;
        Real64 tmpSumCO2GainRate;
        int DeviceNum;
        int TypeNum;

        NumberOfTypes = size(GainTypeARR);
        tmpSumCO2GainRate = 0.0;

        if (ZoneIntGain(ZoneNum).NumberOfDevices == 0) {
            SumCO2GainRate = 0.0;
            return;
        }

        for (DeviceNum = 1; DeviceNum <= ZoneIntGain(ZoneNum).NumberOfDevices; ++DeviceNum) {
            for (TypeNum = 1; TypeNum <= NumberOfTypes; ++TypeNum) {

                if (ZoneIntGain(ZoneNum).Device(DeviceNum).CompTypeOfNum == GainTypeARR(TypeNum)) {
                    tmpSumCO2GainRate += ZoneIntGain(ZoneNum).Device(DeviceNum).CarbonDioxideGainRate;
                }
            }
        }

        SumCO2GainRate = tmpSumCO2GainRate;
    }

    void SumAllInternalGenericContamGains(int const ZoneNum, // zone index pointer for which zone to sum gains for
                                          Real64 &SumGCGainRate)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         L. Gu
        //       DATE WRITTEN   Feb. 2012
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // worker routine for summing all the internal gain types based on the existing subrotine SumAllInternalCO2Gains

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 tmpSumGCGainRate;
        int DeviceNum;

        tmpSumGCGainRate = 0.0;

        if (ZoneIntGain(ZoneNum).NumberOfDevices == 0) {
            SumGCGainRate = 0.0;
            return;
        }

        for (DeviceNum = 1; DeviceNum <= ZoneIntGain(ZoneNum).NumberOfDevices; ++DeviceNum) {
            tmpSumGCGainRate += ZoneIntGain(ZoneNum).Device(DeviceNum).GenericContamGainRate;
        }

        SumGCGainRate = tmpSumGCGainRate;
    }

    void GatherComponentLoadsIntGain()
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Jason Glazer
        //       DATE WRITTEN   September 2012
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        //   Gather values during sizing used for loads component report.

        // METHODOLOGY EMPLOYED:
        //   Save sequence of values for report during sizing.

        // Using/Aliasing
        using namespace DataHeatBalance;
        using DataGlobals::CompLoadReportIsReq;
        using DataGlobals::isPulseZoneSizing;
        using DataGlobals::NumOfTimeStepInHour;
        using DataSizing::CurOverallSimDay;
        using OutputReportTabular::equipInstantSeq;
        using OutputReportTabular::equipLatentSeq;
        using OutputReportTabular::equipRadSeq;
        using OutputReportTabular::hvacLossInstantSeq;
        using OutputReportTabular::hvacLossRadSeq;
        using OutputReportTabular::lightInstantSeq;
        using OutputReportTabular::lightLWRadSeq;
        using OutputReportTabular::lightRetAirSeq;
        using OutputReportTabular::peopleInstantSeq;
        using OutputReportTabular::peopleLatentSeq;
        using OutputReportTabular::peopleRadSeq;
        using OutputReportTabular::powerGenInstantSeq;
        using OutputReportTabular::powerGenRadSeq;
        using OutputReportTabular::refrigInstantSeq;
        using OutputReportTabular::refrigLatentSeq;
        using OutputReportTabular::refrigRetAirSeq;
        using OutputReportTabular::waterUseInstantSeq;
        using OutputReportTabular::waterUseLatentSeq;

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        static int iZone(0);
        static int TimeStepInDay(0);
        static Array1D_int IntGainTypesPeople(1, {IntGainTypeOf_People});
        static Array1D_int IntGainTypesLight(1, {IntGainTypeOf_Lights});
        static Array1D_int IntGainTypesEquip(6,
                                             {IntGainTypeOf_ElectricEquipment,
                                              IntGainTypeOf_ElectricEquipmentITEAirCooled,
                                              IntGainTypeOf_GasEquipment,
                                              IntGainTypeOf_HotWaterEquipment,
                                              IntGainTypeOf_SteamEquipment,
                                              IntGainTypeOf_OtherEquipment});
        static Array1D_int IntGainTypesRefrig(10,
                                              {IntGainTypeOf_RefrigerationCase,
                                               IntGainTypeOf_RefrigerationCompressorRack,
                                               IntGainTypeOf_RefrigerationSystemAirCooledCondenser,
                                               IntGainTypeOf_RefrigerationSystemSuctionPipe,
                                               IntGainTypeOf_RefrigerationSecondaryReceiver,
                                               IntGainTypeOf_RefrigerationSecondaryPipe,
                                               IntGainTypeOf_RefrigerationWalkIn,
                                               IntGainTypeOf_RefrigerationTransSysAirCooledGasCooler,
                                               IntGainTypeOf_RefrigerationTransSysSuctionPipeMT,
                                               IntGainTypeOf_RefrigerationTransSysSuctionPipeLT});
        static Array1D_int IntGainTypesWaterUse(
            3, {IntGainTypeOf_WaterUseEquipment, IntGainTypeOf_WaterHeaterMixed, IntGainTypeOf_WaterHeaterStratified});
        static Array1D_int IntGainTypesHvacLoss(20,
                                                {IntGainTypeOf_ZoneBaseboardOutdoorTemperatureControlled,
                                                 IntGainTypeOf_ThermalStorageChilledWaterMixed,
                                                 IntGainTypeOf_ThermalStorageChilledWaterStratified,
                                                 IntGainTypeOf_PipeIndoor,
                                                 IntGainTypeOf_Pump_VarSpeed,
                                                 IntGainTypeOf_Pump_ConSpeed,
                                                 IntGainTypeOf_Pump_Cond,
                                                 IntGainTypeOf_PumpBank_VarSpeed,
                                                 IntGainTypeOf_PumpBank_ConSpeed,
                                                 IntGainTypeOf_PlantComponentUserDefined,
                                                 IntGainTypeOf_CoilUserDefined,
                                                 IntGainTypeOf_ZoneHVACForcedAirUserDefined,
                                                 IntGainTypeOf_AirTerminalUserDefined,
                                                 IntGainTypeOf_PackagedTESCoilTank,
                                                 IntGainTypeOf_FanSystemModel,
                                                 IntGainTypeOf_SecCoolingDXCoilSingleSpeed,
                                                 IntGainTypeOf_SecHeatingDXCoilSingleSpeed,
                                                 IntGainTypeOf_SecCoolingDXCoilTwoSpeed,
                                                 IntGainTypeOf_SecCoolingDXCoilMultiSpeed,
                                                 IntGainTypeOf_SecHeatingDXCoilMultiSpeed});
        static Array1D_int IntGainTypesPowerGen(9,
                                                {IntGainTypeOf_GeneratorFuelCell,
                                                 IntGainTypeOf_GeneratorMicroCHP,
                                                 IntGainTypeOf_ElectricLoadCenterTransformer,
                                                 IntGainTypeOf_ElectricLoadCenterInverterSimple,
                                                 IntGainTypeOf_ElectricLoadCenterInverterFunctionOfPower,
                                                 IntGainTypeOf_ElectricLoadCenterInverterLookUpTable,
                                                 IntGainTypeOf_ElectricLoadCenterStorageBattery,
                                                 IntGainTypeOf_ElectricLoadCenterStorageSimple,
                                                 IntGainTypeOf_ElectricLoadCenterConverter});

        if (CompLoadReportIsReq && !isPulseZoneSizing) {
            TimeStepInDay = (HourOfDay - 1) * NumOfTimeStepInHour + TimeStep;
            for (iZone = 1; iZone <= NumOfZones; ++iZone) {
                SumInternalConvectionGainsByTypes(iZone, IntGainTypesPeople, peopleInstantSeq(CurOverallSimDay, TimeStepInDay, iZone));
                SumInternalLatentGainsByTypes(iZone, IntGainTypesPeople, peopleLatentSeq(CurOverallSimDay, TimeStepInDay, iZone));
                SumInternalRadiationGainsByTypes(iZone, IntGainTypesPeople, peopleRadSeq(CurOverallSimDay, TimeStepInDay, iZone));

                SumInternalConvectionGainsByTypes(iZone, IntGainTypesLight, lightInstantSeq(CurOverallSimDay, TimeStepInDay, iZone));
                SumReturnAirConvectionGainsByTypes(iZone, IntGainTypesLight, lightRetAirSeq(CurOverallSimDay, TimeStepInDay, iZone));
                SumInternalRadiationGainsByTypes(iZone, IntGainTypesLight, lightLWRadSeq(CurOverallSimDay, TimeStepInDay, iZone));

                SumInternalConvectionGainsByTypes(iZone, IntGainTypesEquip, equipInstantSeq(CurOverallSimDay, TimeStepInDay, iZone));
                SumInternalLatentGainsByTypes(iZone, IntGainTypesEquip, equipLatentSeq(CurOverallSimDay, TimeStepInDay, iZone));
                SumInternalRadiationGainsByTypes(iZone, IntGainTypesEquip, equipRadSeq(CurOverallSimDay, TimeStepInDay, iZone));

                SumInternalConvectionGainsByTypes(iZone, IntGainTypesRefrig, refrigInstantSeq(CurOverallSimDay, TimeStepInDay, iZone));
                SumReturnAirConvectionGainsByTypes(iZone, IntGainTypesRefrig, refrigRetAirSeq(CurOverallSimDay, TimeStepInDay, iZone));
                SumInternalLatentGainsByTypes(iZone, IntGainTypesRefrig, refrigLatentSeq(CurOverallSimDay, TimeStepInDay, iZone));

                SumInternalConvectionGainsByTypes(iZone, IntGainTypesWaterUse, waterUseInstantSeq(CurOverallSimDay, TimeStepInDay, iZone));
                SumInternalLatentGainsByTypes(iZone, IntGainTypesWaterUse, waterUseLatentSeq(CurOverallSimDay, TimeStepInDay, iZone));

                SumInternalConvectionGainsByTypes(iZone, IntGainTypesHvacLoss, hvacLossInstantSeq(CurOverallSimDay, TimeStepInDay, iZone));
                SumInternalRadiationGainsByTypes(iZone, IntGainTypesHvacLoss, hvacLossRadSeq(CurOverallSimDay, TimeStepInDay, iZone));

                SumInternalConvectionGainsByTypes(iZone, IntGainTypesPowerGen, powerGenInstantSeq(CurOverallSimDay, TimeStepInDay, iZone));
                SumInternalRadiationGainsByTypes(iZone, IntGainTypesPowerGen, powerGenRadSeq(CurOverallSimDay, TimeStepInDay, iZone));
            }
        }
    }

    void GetInternalGainDeviceIndex(int const ZoneNum,              // zone index pointer for which zone to sum gains for
                                    int const IntGainTypeOfNum,     // zone internal gain type number
                                    std::string const &IntGainName, // Internal gain name
                                    int &DeviceIndex,               // Device index
                                    bool &ErrorFound)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         B. Griffith
        //       DATE WRITTEN   June 2012
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // utility to retrieve index pointer to a specific internal gain

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        bool Found;
        int DeviceNum;

        Found = false;

        if (ZoneIntGain(ZoneNum).NumberOfDevices == 0) {
            DeviceIndex = -1;
            ErrorFound = true;
            return;
        }

        for (DeviceNum = 1; DeviceNum <= ZoneIntGain(ZoneNum).NumberOfDevices; ++DeviceNum) {
            if (UtilityRoutines::SameString(ZoneIntGain(ZoneNum).Device(DeviceNum).CompObjectName, IntGainName)) {
                if (ZoneIntGain(ZoneNum).Device(DeviceNum).CompTypeOfNum != IntGainTypeOfNum) {
                    ErrorFound = true;
                } else {
                    ErrorFound = false;
                }
                Found = true;
                DeviceIndex = DeviceNum;
                break;
            }
        }
    }

    void SumInternalConvectionGainsByIndices(
        int const ZoneNum,                 // zone index pointer for which zone to sum gains for
        const Array1D_int &DeviceIndexARR,  // variable length 1-d array of integer device index pointers to include in summation
        const Array1D<Real64> &FractionARR, // array of fractional multipliers to apply to devices
        Real64 &SumConvGainRate)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         B. Griffith
        //       DATE WRITTEN   June 2012
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // worker routine for summing a subset of the internal gains by index

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int NumberOfIndices;
        int NumberOfFractions;
        Real64 tmpSumConvGainRate;
        int loop;
        int DeviceNum;
        Real64 DeviceFraction;

        NumberOfIndices = isize(DeviceIndexARR);
        NumberOfFractions = isize(FractionARR);
        tmpSumConvGainRate = 0.0;

        // remove this next safety check after testing code
        if (NumberOfIndices != NumberOfFractions) { // throw error
            ShowSevereError("SumInternalConvectionGainsByIndices: bad arguments, sizes do not match");
        }

        if (ZoneIntGain(ZoneNum).NumberOfDevices == 0) {
            SumConvGainRate = 0.0;
            return;
        }

        for (loop = 1; loop <= NumberOfIndices; ++loop) {
            DeviceNum = DeviceIndexARR(loop);
            DeviceFraction = FractionARR(loop);
            tmpSumConvGainRate = tmpSumConvGainRate + ZoneIntGain(ZoneNum).Device(DeviceNum).ConvectGainRate * DeviceFraction;
        }
        SumConvGainRate = tmpSumConvGainRate;
    }

    void SumInternalLatentGainsByIndices(
        int const ZoneNum,                  // zone index pointer for which zone to sum gains for
        const Array1D_int &DeviceIndexARR,  // variable length 1-d array of integer device index pointers to include in summation
        const Array1D<Real64> &FractionARR, // array of fractional multipliers to apply to devices
        Real64 &SumLatentGainRate)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         B. Griffith
        //       DATE WRITTEN   June 2012
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // worker routine for summing a subset of the internal gains by index

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int NumberOfIndices;
        int NumberOfFractions;
        Real64 tmpSumLatentGainRate;
        int loop;
        int DeviceNum;
        Real64 DeviceFraction;

        NumberOfIndices = isize(DeviceIndexARR);
        NumberOfFractions = isize(FractionARR);
        tmpSumLatentGainRate = 0.0;

        // remove this next safety check after testing code
        if (NumberOfIndices != NumberOfFractions) { // throw error
            ShowSevereError("SumInternalLatentGainsByIndices: bad arguments, sizes do not match");
        }

        if (ZoneIntGain(ZoneNum).NumberOfDevices == 0) {
            SumLatentGainRate = 0.0;
            return;
        }

        for (loop = 1; loop <= NumberOfIndices; ++loop) {
            DeviceNum = DeviceIndexARR(loop);
            DeviceFraction = FractionARR(loop);
            tmpSumLatentGainRate = tmpSumLatentGainRate + ZoneIntGain(ZoneNum).Device(DeviceNum).LatentGainRate * DeviceFraction;
        }
        SumLatentGainRate = tmpSumLatentGainRate;
    }

    void SumReturnAirConvectionGainsByIndices(
        int const ZoneNum,                  // zone index pointer for which zone to sum gains for
        const Array1D_int &DeviceIndexARR,  // variable length 1-d array of integer device index pointers to include in summation
        const Array1D<Real64> &FractionARR, // array of fractional multipliers to apply to devices
        Real64 &SumReturnAirGainRate)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         B. Griffith
        //       DATE WRITTEN   June 2012
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // worker routine for summing a subset of the internal gains by index

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int NumberOfIndices;
        int NumberOfFractions;
        Real64 tmpSumReturnAirGainRate;
        int loop;
        int DeviceNum;
        Real64 DeviceFraction;

        NumberOfIndices = isize(DeviceIndexARR);
        NumberOfFractions = isize(FractionARR);
        tmpSumReturnAirGainRate = 0.0;

        // remove this next safety check after testing code
        if (NumberOfIndices != NumberOfFractions) { // throw error
            ShowSevereError("SumReturnAirConvectionGainsByIndice: bad arguments, sizes do not match");
        }

        if (ZoneIntGain(ZoneNum).NumberOfDevices == 0) {
            SumReturnAirGainRate = 0.0;
            return;
        }

        for (loop = 1; loop <= NumberOfIndices; ++loop) {
            DeviceNum = DeviceIndexARR(loop);
            DeviceFraction = FractionARR(loop);
            tmpSumReturnAirGainRate = tmpSumReturnAirGainRate + ZoneIntGain(ZoneNum).Device(DeviceNum).ReturnAirConvGainRate * DeviceFraction;
        }
        SumReturnAirGainRate = tmpSumReturnAirGainRate;
    }

} // namespace InternalHeatGains

} // namespace EnergyPlus
