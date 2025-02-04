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

// ObjexxFCL Headers
#include <ObjexxFCL/Array.functions.hh>
#include <ObjexxFCL/Array2D.hh>
#include <ObjexxFCL/Fmath.hh>
#include <ObjexxFCL/gio.hh>
#include <ObjexxFCL/numeric.hh>
#include <ObjexxFCL/string.functions.hh>

// EnergyPlus Headers
#include <EnergyPlus/Autosizing/Base.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataAirLoop.hh>
#include <EnergyPlus/DataAirSystems.hh>
#include <EnergyPlus/DataConvergParams.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/DataPrecisionGlobals.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/DataSystemVariables.hh>
#include <EnergyPlus/EMSManager.hh>
#include <EnergyPlus/FaultsManager.hh>
#include <EnergyPlus/FluidProperties.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/HVACControllers.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/MixedAir.hh>
#include <EnergyPlus/NodeInputManager.hh>
#include <EnergyPlus/Plant/DataPlant.hh>
#include <EnergyPlus/PlantUtilities.hh>
#include <EnergyPlus/RootFinder.hh>
#include <EnergyPlus/SetPointManager.hh>
#include <EnergyPlus/UtilityRoutines.hh>
#include <EnergyPlus/WaterCoils.hh>

namespace EnergyPlus {

namespace HVACControllers {
    // Module containing the controller simulation routines for the air loop

    // MODULE INFORMATION:
    //       AUTHOR         Richard J. Liesen
    //       DATE WRITTEN   July 1998
    //       MODIFIED       Feb 2006, Dimitri Curtil (LBNL)
    //                      - Added tracing mechanism for debugging convergence process.
    //                        - Trace operation of each individual controller in a file named
    //                          'controller.<Controller Name>.csv'
    //                        - Trace operation of all controllers per air loop in a file named
    //                          'controller.<Air Loop Name>.csv'
    //                      - Added operations to enable cold start/speculative warm restart
    //                        and final check.
    //       MODIFIED       March 2006, Dimitri Curtil (LBNL)
    //                      - Added mechanism to track runtime performance statistics.
    //                      - Added routine to dump controller statistics to a file named
    //                        'statistics.HVACControllers.csv'
    //                      - Integrated smart root finder from MODULE RootFinder implemented in
    //                        file RootFinder.cc.
    //       MODIFIED       April 2006, Dimitri Curtil (LBNL)
    //                      - Added speedup optimization scheme to reuse air loop solution
    //                        obtained at the current HVAC iteration from solving the previous controller
    //                        on the loop (see ReuseIntermediateSolutionFlag). Of course this works only
    //                        if there are 2 or more controllers on the same air loop.
    //                      - Added speedup optimization scheme to reuse solution obtained
    //                        at the previous HVAC iteration for this controller during the
    //                        bracketing phase (see ReusePreviousSolutionFlag).
    //       MODIFIED       May 2006, Dimitri Curtil (LBNL)
    //                      - Added mechanism to monitor min/max bounds to ensure that they remain invariant
    //                        between successive controller iterations.
    //                      - Modified setpoint calculation to force the setpoint to be computed only once.
    //                      - Modified setpoint calculation for TEMPandHUMRAT control strategy to
    //                        force the setpoint to be computed once the air loop has been evaluated with
    //                        the max actuated value.
    //       MODIFIED       June 2006, Dimitri Curtil (LBNL)
    //                      - Renamed parameter variables so as to use lower caps.
    //                      - Replaced $ edit descriptor in WRITE statements with ADVANCE='No'
    //                      - Replaced the preprocessing directives TRACK_AIRLOOP, TRACE_AIRLOOP,
    //                        TRACE_CONTROLLER with corresponding environment variables defined
    //                        in DataSystemVariables.cc.
    //       MODIFIED       Feb. 2010, Brent Griffith (NREL)
    //                       - changed plant loop interactions, Demand Side Update Phase 3
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS MODULE:
    // To encapsulate the data and algorithms required to
    // manage the Controller System Component.

    // METHODOLOGY EMPLOYED:
    // The main entry point if the SUBROUTINE ManageControllers().
    // 1. For proper operation, the subroutine must first be called with either the
    //    iControllerOpColdStart or iControllerOpWarmRestart operation code to initialize
    //    the various controllers.
    // 2. Then the actuated variable for each controller is computed iteratively using
    //    root finding techniques that aim at forcing the sensed variable to be
    //    "equal" (within the user-specified tolerance) to the desired setpoint.
    //    This step is achieved by calling ManageController() with the iControllerOpIterate
    //    operation code.
    // 3. Finally, after all controllers have been successfully simulated,  the subroutine has
    //    to be called one last time with the iControllerOpEnd operation code to ensure that
    //    the sequential solution indeed represents a valid global solution across all controllers
    //    simultaneously.
    // The following pseudo-code shows the typical calling sequence for the SUBROUTINE
    // ManageControllers :
    // - for each controller on air loop
    //   - CALL ManageControllers( Operation=<iControllerOpColdStart or iControllerOpWarmRestart> )
    // - simulate air loop components with the initial values for all actuated variables
    // - for each controller on air loop
    //   - CALL ManageControllers( Operation=iControllerOpIterate, IsConvergedFlag )
    //   - if NOT IsConvergedFlag then
    //     - exit loop with error if too many iterations performed
    //     - simulate air loop components with the new candidate value for the actuated variable of
    //       the current controller
    // - simulate air loop components with the final values for all actuated variables
    // - for each controller on air loop
    //   - CALL ManageControllers( Operation=iControllerOpEnd, IsConvergedFlag )
    //   - if NOT IsConvergedFlag then
    //     - exit loop with error indicating no "global" convergence with final solution.
    // Check the subroutines SolveAirLoopControllers() and ReSolveAirLoopControllers()
    // invoked in the subroutine SimAirLoop() for the actual calling sequences.

    // REFERENCES:
    // na

    // OTHER NOTES:
    // To enable runtime statistics tracking for each air loop, define the environment variable
    // TRACK_AIRLOOP=YES or TRACK_AIRLOOP=Y.
    // To enable generating a trace file with the converged solution for all controllers on each air loop,
    // define the environment variable TRACE_AIRLOOP=YES or TRACE_AIRLOOP=Y.
    // To enable generating an individual, detailed trace file for each controller, define the
    // environment variable TRACE_CONTROLLER=YES or TRACE_CONTROLLER=Y.
    // See DataSystemVariables.cc for the definitions of the environment variables used to debug
    // the air loop simulation.

    // USE STATEMENTS:
    // Use statements for data only modules
    // Using/Aliasing
    using namespace DataPrecisionGlobals;
    using namespace DataLoopNode;
    using namespace DataGlobals;
    using DataHVACGlobals::SetPointErrorFlag;
    using DataHVACGlobals::SmallWaterVolFlow;
    using namespace DataHVACControllers;
    using namespace DataRootFinder;

    // Data
    // MODULE PARAMETER DEFINITIONS
    // Number of significant digits to display in error messages for floating-point numbers
    Real64 const SomeFloatingPoint(1.0);
    int const NumSigDigits(PRECISION(SomeFloatingPoint));

    static std::string const BlankString;

    // Parameters for controls used here
    int const iNoControlVariable(0);
    int const iTemperature(1);
    int const iHumidityRatio(2);
    int const iTemperatureAndHumidityRatio(3);
    int const iFlow(4);

    int const CoilType_Cooling(1);
    int const CoilType_Heating(2);

    Array1D_string const
        ControlVariableTypes({0, 4}, {"No control variable", "Temperature", "Humidity ratio", "Temperature and humidity ratio", "Flow rate"});

    // DERIVED TYPE DEFINITIONS

    // Type describing a controller's properties

    // Type describing a controller's runtime statistics over the course of the simulation

    // Type describing an air loop's runtime statistics over the course of the simulation

    // MODULE VARIABLE DECLARATIONS:
    int NumControllers(0);  // The number of controllers found in the Input
    int NumAirLoopStats(0); // Same size as NumPrimaryAirSys if controllers
    // are defined, 0 otherwise.
    // all controllers per air loop
    Array1D_bool CheckEquipName;

    // Flag set to make sure you get input once
    bool GetControllerInputFlag(true);

    // SUBROUTINE Specifications for the Module
    // Driver/Manager Routines

    // Get Input routines for module

    // Initialization routines for module

    // Algorithms for the module

    // Update routine to check convergence and update nodes

    // Reporting routines for module

    // Algorithms for the Simple Controller

    // Statistics routines

    // Trace routines for all controllers on each air loop

    // Trace routines for each individual controller

    // Misc routines

    // Object Data
    Array1D<ControllerPropsType> ControllerProps;
    Array1D<RootFinderDataType> RootFinders;
    Array1D<AirLoopStatsType> AirLoopStats; // Statistics array to analyze computational profile for

    namespace {
        bool InitControllerOneTimeFlag(true);
        bool InitControllerSetPointCheckFlag(true);
    } // namespace

    // MODULE SUBROUTINES:
    //*************************************************************************

    // Functions

    // Needed for unit tests, should not be normally called.
    void clear_state()
    {
        NumControllers = 0;
        NumAirLoopStats = 0;
        GetControllerInputFlag = true;
        InitControllerOneTimeFlag = true;
        InitControllerSetPointCheckFlag = true;

        ControllerProps.deallocate();
        RootFinders.deallocate();
        AirLoopStats.deallocate();
        CheckEquipName.deallocate();
    }

    void ManageControllers(EnergyPlusData &state,
                           std::string const &ControllerName,
                           int &ControllerIndex,
                           bool const FirstHVACIteration,
                           int const AirLoopNum,
                           int const Operation,
                           bool &IsConvergedFlag,
                           bool &IsUpToDateFlag,
                           bool &BypassOAController,
                           Optional_bool AllowWarmRestartFlag)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Richard Liesen
        //       DATE WRITTEN   July 1998
        //       MODIFIED       Dimitri Curtil, February 2006
        //                      - Added air loop information
        //                      - Added tracing to csv files
        //                      - Added primitive operations to replace mixed
        //                        bag of ResetController, FirstCallConvergenceTest, ...
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine manages Controller component simulation.

        // Using/Aliasing
        using namespace DataSystemVariables;
        using DataPlant::FlowLocked;
        using DataPlant::PlantLoop;
        using General::TrimSigDigits;

        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:
        // TRUE if first full HVAC iteration in an HVAC time step
        // Current air loop num 1...NumPrimaryAirSys
        // Current pass counter in SimAirLoop()
        // Operation to execute
        // TRUE if controller is converged
        // TRUE if air loop is up-to-date meaning that the current node values are consistent (air loop evaluated)
        // Only used within the Calc routines
        // TRUE if speculative warm restart is supported by this controller

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        // The Controller that you are currently loading input into
        int ControlNum;
        int ControllerType;

        // Obtains and Allocates Controller related parameters from input file
        if (GetControllerInputFlag) { // First time subroutine has been entered
            GetControllerInput(state);
            GetControllerInputFlag = false;
        }

        if (ControllerIndex == 0) {
            ControlNum = UtilityRoutines::FindItemInList(ControllerName, ControllerProps, &ControllerPropsType::ControllerName);
            if (ControlNum == 0) {
                ShowFatalError("ManageControllers: Invalid controller=" + ControllerName +
                               ". The only valid controller type for an AirLoopHVAC is Controller:WaterCoil.");
            }
            ControllerIndex = ControlNum;
        } else {
            ControlNum = ControllerIndex;
            if (ControlNum > NumControllers || ControlNum < 1) {
                ShowFatalError("ManageControllers: Invalid ControllerIndex passed=" + TrimSigDigits(ControlNum) +
                               ", Number of controllers=" + TrimSigDigits(NumControllers) + ", Controller name=" + ControllerName);
            }
            if (CheckEquipName(ControlNum)) {
                if (ControllerName != ControllerProps(ControlNum).ControllerName) {
                    ShowFatalError("ManageControllers: Invalid ControllerIndex passed=" + TrimSigDigits(ControlNum) + ", Controller name=" +
                                   ControllerName + ", stored Controller Name for that index=" + ControllerProps(ControlNum).ControllerName);
                }
                CheckEquipName(ControlNum) = false;
            }
        }

        if (ControllerProps(ControlNum).BypassControllerCalc && BypassOAController) {
            IsUpToDateFlag = true;
            IsConvergedFlag = true;
            if (present(AllowWarmRestartFlag)) AllowWarmRestartFlag = true;
            return;
        }

        // Find the correct ControllerNumber with the AirLoop & CompNum from AirLoop Derived Type
        // ControlNum = AirLoopEquip(AirLoopNum)%ComponentOfTypeNum(CompNum)

        // detect if plant is locked and flow cannot change
        if (ControllerProps(ControlNum).ActuatedNodePlantLoopNum > 0) {

            if (PlantLoop(ControllerProps(ControlNum).ActuatedNodePlantLoopNum)
                    .LoopSide(ControllerProps(ControlNum).ActuatedNodePlantLoopSide)
                    .FlowLock == FlowLocked) {
                // plant is rigid so controller cannot change anything.
                // Update the current Controller to the outlet nodes
                UpdateController(ControlNum);

                IsConvergedFlag = true;
                return;
            }
        }

        // Detect if speculative warm restart is supported by this computer
        if (present(AllowWarmRestartFlag)) {
            // NOTE: Never allow speculative warm restart with dual humidity ratio and temperature control
            //       because the actual setpoint depends on the current temperature and max hum ratio at
            //       the sensed node, and therefore might not be known until after one air loop simulation.
            if (ControllerProps(ControlNum).ControlVar == iTemperatureAndHumidityRatio) {
                AllowWarmRestartFlag = false;
            } else {
                AllowWarmRestartFlag = true;
            }
        }

        if (ControllerProps(ControlNum).InitFirstPass) {
            // Coil must first be sized to:
            // Initialize ControllerProps(ControlNum)%MinActuated and ControllerProps(ControlNum)%MaxActuated
            InitController(state, ControlNum, IsConvergedFlag);
            ControllerProps(ControlNum).InitFirstPass = false;
        }

        // Perform requested operation
        // Note that InitController() is not called upon START/RESTART ops in order to avoid
        // side-effects on the calculation of Node(ActuatedNode)%MassFlowRateMaxAvail used to
        // determine ControllerProps(ControlNum)%MaxAvailActuated.
        // Plant upgrades for V7 added init to these cases because MassFlowRateMaxAvail is better controlled
        {
            auto const SELECT_CASE_var(Operation);
            if (SELECT_CASE_var == iControllerOpColdStart) {
                // For temperature and humidity control reset humidity control override if it was set
                if (HVACControllers::ControllerProps(ControlNum).HumRatCtrlOverride) {
                    HVACControllers::ControllerProps(ControlNum).HumRatCtrlOverride = false;
                    // Put the controller tolerance (offset) back to it's original value
                    RootFinder::SetupRootFinder(
                        RootFinders(ControlNum), iSlopeDecreasing, iMethodBrent, constant_zero, 1.0e-6, ControllerProps(ControlNum).Offset);
                }

                // If a iControllerOpColdStart call, reset the actuator inlet flows
                ResetController(ControlNum, false, IsConvergedFlag);
                // Update the current Controller to the outlet nodes
                UpdateController(ControlNum);

            } else if (SELECT_CASE_var == iControllerOpWarmRestart) {
                // If a iControllerOpWarmRestart call, set the actuator inlet flows to previous solution
                ResetController(ControlNum, true, IsConvergedFlag);
                // Update the current Controller to the outlet nodes
                UpdateController(ControlNum);

            } else if (SELECT_CASE_var == iControllerOpIterate) {
                // With the correct ControlNum Initialize all Controller related parameters
                InitController(state, ControlNum, IsConvergedFlag);

                // No initialization needed: should have been done before
                // Simulate the correct Controller with the current ControlNum
                ControllerType = ControllerProps(ControlNum).ControllerType_Num;

                {
                    auto const SELECT_CASE_var1(ControllerType);
                    if (SELECT_CASE_var1 == ControllerSimple_Type) { // 'Controller:WaterCoil'
                        CalcSimpleController(ControlNum, FirstHVACIteration, IsConvergedFlag, IsUpToDateFlag, ControllerName);
                    } else {
                        ShowFatalError("Invalid controller type in ManageControllers=" + ControllerProps(ControlNum).ControllerType);
                    }
                }

                // Update the current Controller to the outlet nodes
                UpdateController(ControlNum);

                CheckTempAndHumRatCtrl(ControlNum, IsConvergedFlag);

            } else if (SELECT_CASE_var == iControllerOpEnd) {
                // With the correct ControlNum Initialize all Controller related parameters
                InitController(state, ControlNum, IsConvergedFlag);

                // No initialization needed: should have been done before
                // Check convergence for the correct Controller with the current ControlNum
                ControllerType = ControllerProps(ControlNum).ControllerType_Num;

                {
                    auto const SELECT_CASE_var1(ControllerType);
                    if (SELECT_CASE_var1 == ControllerSimple_Type) { // 'Controller:WaterCoil'
                        CheckSimpleController(ControlNum, IsConvergedFlag);
                        SaveSimpleController(ControlNum, FirstHVACIteration, IsConvergedFlag);
                    } else {
                        ShowFatalError("Invalid controller type in ManageControllers=" + ControllerProps(ControlNum).ControllerType);
                    }
                }

            } else {
                ShowFatalError("ManageControllers: Invalid Operation passed=" + TrimSigDigits(Operation) + ", Controller name=" + ControllerName);
            }
        }

        // Write detailed diagnostic for individual controller
        // To enable generating an individual, detailed trace file for each controller on each air loop,
        // define the environment variable TRACE_CONTROLLER=YES or TRACE_CONTROLLER=Y
        if (TraceHVACControllerEnvFlag) {
            TraceIndividualController(ControlNum, FirstHVACIteration, state.dataAirLoop->AirLoopControlInfo(AirLoopNum).AirLoopPass, Operation, IsConvergedFlag);
        }
    }

    // Get Input Section of the Module
    //******************************************************************************

    void GetControllerInput(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Richard Liesen
        //       DATE WRITTEN   July 1998
        //       MODIFIED       February 2006, Dimitri Curtil
        //                      - Added processing for air loop controller stats
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine is the main routine to call other input routines and Get routines

        // METHODOLOGY EMPLOYED:
        // Uses the status flags to trigger events.

        // REFERENCES:
        // Gets the object:
        // Controller:WaterCoil,
        //   \min-fields 9
        //   A1 , \field Name
        //        \type alpha
        //        \required-field
        //        \reference AirLoopControllers
        //   A2 , \field Control Variable
        //        \type choice
        //        \key Temperature
        //        \key HumidityRatio
        //        \key TemperatureAndHumidityRatio
        //        \key Flow
        //        \note TemperatureAndHumidityRatio requires a SetpointManager:SingleZone:Humidity:Maximum object
        //   A3 , \field Action
        //        \type choice
        //        \key Normal
        //        \key Reverse
        //   A4 , \field Actuator Variable
        //        \type choice
        //        \key Flow
        //   A5 , \field Sensor Node Name
        //        \type alpha
        //   A6 , \field Actuator Node Name
        //        \type alpha
        //   N1 , \field Controller Convergence Tolerance
        //        \units deltaC
        //        \type real
        //        \default AutoSize
        //        \autosizable
        //   N2 , \field Maximum Actuated Flow
        //        \type real
        //        \units m3/s
        //        \autosizable
        //   N3 ; \field Minimum Actuated Flow
        //        \type real
        //        \default 0.0000001
        //        \units m3/s

        // Using/Aliasing
        using DataAirSystems::PrimaryAirSystem;
        using DataHVACGlobals::NumPrimaryAirSys;
        using DataSystemVariables::TraceAirLoopEnvFlag;
        using DataSystemVariables::TraceHVACControllerEnvFlag;
        using DataSystemVariables::TrackAirLoopEnvFlag;
        using EMSManager::CheckIfNodeSetPointManagedByEMS;
        using EMSManager::iHumidityRatioMaxSetPoint;
        using EMSManager::iTemperatureSetPoint;
        using MixedAir::CheckForControllerWaterCoil;
        using NodeInputManager::GetOnlySingleNode;
        using SetPointManager::iCtrlVarType_MaxHumRat;
        using SetPointManager::iCtrlVarType_Temp;
        using SetPointManager::NodeHasSPMCtrlVarType;
        using SetPointManager::ResetHumidityRatioCtrlVarType;
        using WaterCoils::CheckActuatorNode;
        using WaterCoils::CheckForSensorAndSetPointNode;

        // SUBROUTINE PARAMETER DEFINITIONS:
        static std::string const RoutineName("HVACControllers: GetControllerInput: "); // include trailing blank space

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int Num; // The Controller that you are currently loading input into
        int NumSimpleControllers;
        int NumAlphas;
        int NumNums;
        int NumArgs;
        int IOStat;
        int AirLoopNum;            // DO index for each air loop
        bool ActuatorNodeNotFound; // true if no water coil inlet node match for actuator node
        Array1D<Real64> NumArray;
        Array1D_string AlphArray;
        Array1D_string cAlphaFields;     // Alpha field names
        Array1D_string cNumericFields;   // Numeric field names
        Array1D_bool lAlphaBlanks;       // Logical array, alpha field input BLANK = .TRUE.
        Array1D_bool lNumericBlanks;     // Logical array, numeric field input BLANK = .TRUE.
        std::string CurrentModuleObject; // for ease in getting objects
        static bool ErrorsFound(false);
        int iNodeType;             // for checking actuator node type
        bool NodeNotFound;         // flag true if the sensor node is on the coil air outlet node
        bool EMSSetPointErrorFlag; // flag true is EMS is used to set node setpoints

        // All the controllers are loaded into the same derived type, both the PI and Limit
        // These controllers are separate objects and loaded sequentially, but will
        // be retrieved by name as they are needed.

        CurrentModuleObject = "Controller:WaterCoil";
        NumSimpleControllers = inputProcessor->getNumObjectsFound(CurrentModuleObject);
        NumControllers = NumSimpleControllers;

        // Allocate stats data structure for each air loop and controller if needed
        if (TrackAirLoopEnvFlag || TraceAirLoopEnvFlag || TraceHVACControllerEnvFlag) {
            if (NumPrimaryAirSys > 0) {
                NumAirLoopStats = NumPrimaryAirSys;
                AirLoopStats.allocate(NumAirLoopStats);

                // Allocate controller statistics data for each controller on each air loop
                for (AirLoopNum = 1; AirLoopNum <= NumPrimaryAirSys; ++AirLoopNum) {
                    AirLoopStats(AirLoopNum).ControllerStats.allocate(PrimaryAirSystem(AirLoopNum).NumControllers);
                }
            }
        }

        if (NumControllers == 0) return;
        // Condition of no controllers will be taken care of elsewhere, if necessary

        ControllerProps.allocate(NumControllers);
        RootFinders.allocate(NumControllers);
        CheckEquipName.dimension(NumControllers, true);

        inputProcessor->getObjectDefMaxArgs(CurrentModuleObject, NumArgs, NumAlphas, NumNums);
        AlphArray.allocate(NumAlphas);
        cAlphaFields.allocate(NumAlphas);
        cNumericFields.allocate(NumNums);
        NumArray.dimension(NumNums, 0.0);
        lAlphaBlanks.dimension(NumAlphas, true);
        lNumericBlanks.dimension(NumNums, true);

        // Now find and load all of the simple controllers.
        if (NumSimpleControllers > 0) {
            for (Num = 1; Num <= NumSimpleControllers; ++Num) {
                inputProcessor->getObjectItem(state,
                                              CurrentModuleObject,
                                              Num,
                                              AlphArray,
                                              NumAlphas,
                                              NumArray,
                                              NumNums,
                                              IOStat,
                                              lNumericBlanks,
                                              lAlphaBlanks,
                                              cAlphaFields,
                                              cNumericFields);
                UtilityRoutines::IsNameEmpty(AlphArray(1), CurrentModuleObject, ErrorsFound);

                ControllerProps(Num).ControllerName = AlphArray(1);
                ControllerProps(Num).ControllerType = CurrentModuleObject;
                {
                    auto const SELECT_CASE_var(AlphArray(2));
                    if (SELECT_CASE_var == "TEMPERATURE") {
                        ControllerProps(Num).ControlVar = iTemperature;
                    } else if (SELECT_CASE_var == "HUMIDITYRATIO") {
                        ControllerProps(Num).ControlVar = iHumidityRatio;
                    } else if (SELECT_CASE_var == "TEMPERATUREANDHUMIDITYRATIO") {
                        ControllerProps(Num).ControlVar = iTemperatureAndHumidityRatio;
                        //        CASE ('FLOW')
                        //          ControllerProps(Num)%ControlVar  = iFlow
                    } else {
                        ShowSevereError(RoutineName + CurrentModuleObject + "=\"" + AlphArray(1) + "\".");
                        ShowSevereError("...Invalid " + cAlphaFields(2) + "=\"" + AlphArray(2) +
                                        "\", must be Temperature, HumidityRatio, or TemperatureAndHumidityRatio.");
                        ErrorsFound = true;
                    }
                }
                if (UtilityRoutines::SameString(AlphArray(3), "Normal")) {
                    ControllerProps(Num).Action = iNormalAction;
                } else if (UtilityRoutines::SameString(AlphArray(3), "Reverse")) {
                    ControllerProps(Num).Action = iReverseAction;
                } else if (lAlphaBlanks(3)) {
                    ControllerProps(Num).Action = 0;
                } else {
                    ShowSevereError(RoutineName + CurrentModuleObject + "=\"" + AlphArray(1) + "\".");
                    ShowSevereError("...Invalid " + cAlphaFields(3) + "=\"" + AlphArray(3) + "\", must be \"Normal\", \"Reverse\" or blank.");
                    ErrorsFound = true;
                }
                if (AlphArray(4) == "FLOW") {
                    ControllerProps(Num).ActuatorVar = iFlow;
                } else {
                    ShowSevereError(RoutineName + CurrentModuleObject + "=\"" + AlphArray(1) + "\".");
                    ShowContinueError("...Invalid " + cAlphaFields(4) + "=\"" + AlphArray(4) + "\", only FLOW is allowed.");
                    ErrorsFound = true;
                }
                ControllerProps(Num).SensedNode = GetOnlySingleNode(state,
                    AlphArray(5), ErrorsFound, CurrentModuleObject, AlphArray(1), NodeType_Unknown, NodeConnectionType_Sensor, 1, ObjectIsNotParent);
                ControllerProps(Num).ActuatedNode = GetOnlySingleNode(state, AlphArray(6),
                                                                      ErrorsFound,
                                                                      CurrentModuleObject,
                                                                      AlphArray(1),
                                                                      NodeType_Unknown,
                                                                      NodeConnectionType_Actuator,
                                                                      1,
                                                                      ObjectIsNotParent);
                ControllerProps(Num).Offset = NumArray(1);
                ControllerProps(Num).MaxVolFlowActuated = NumArray(2);
                ControllerProps(Num).MinVolFlowActuated = NumArray(3);

                if (!CheckForControllerWaterCoil(state, CurrentModuleObject, AlphArray(1))) {
                    ShowSevereError(RoutineName + CurrentModuleObject + "=\"" + AlphArray(1) + "\" not found on any AirLoopHVAC:ControllerList.");
                    ErrorsFound = true;
                }

                if (ControllerProps(Num).SensedNode > 0) {

                    if (ControllerProps(Num).ControlVar == iHumidityRatio || ControllerProps(Num).ControlVar == iTemperatureAndHumidityRatio) {
                        ResetHumidityRatioCtrlVarType(state, ControllerProps(Num).SensedNode);
                    }
                    CheckForSensorAndSetPointNode(state, ControllerProps(Num).SensedNode, ControllerProps(Num).ControlVar, NodeNotFound);

                    if (NodeNotFound) {
                        // the sensor node is not on the water coil air outlet node
                        ShowWarningError(RoutineName + ControllerProps(Num).ControllerType + "=\"" + ControllerProps(Num).ControllerName + "\". ");
                        ShowContinueError(" ..Sensor node not found on water coil air outlet node.");
                        ShowContinueError(" ..The sensor node may have been placed on a node downstream of the coil or on an airloop outlet node.");
                    } else {
                        // check if the setpoint is also on the same node where the sensor is placed on
                        EMSSetPointErrorFlag = false;
                        {
                            auto const SELECT_CASE_var(ControllerProps(Num).ControlVar);
                            if (SELECT_CASE_var == iTemperature) {
                                CheckIfNodeSetPointManagedByEMS(ControllerProps(Num).SensedNode, iTemperatureSetPoint, EMSSetPointErrorFlag);
                                DataLoopNode::NodeSetpointCheck(ControllerProps(Num).SensedNode).needsSetpointChecking = false;
                                if (EMSSetPointErrorFlag) {
                                    if (!NodeHasSPMCtrlVarType(state, ControllerProps(Num).SensedNode, iCtrlVarType_Temp)) {
                                        ShowContinueError(" ..Temperature setpoint not found on coil air outlet node.");
                                        ShowContinueError(
                                            " ..The setpoint may have been placed on a node downstream of the coil or on an airloop outlet node.");
                                        ShowContinueError(" ..Specify the setpoint and the sensor on the coil air outlet node when possible.");
                                    }
                                }
                            } else if (SELECT_CASE_var == iHumidityRatio) {
                                CheckIfNodeSetPointManagedByEMS(ControllerProps(Num).SensedNode, iHumidityRatioMaxSetPoint, EMSSetPointErrorFlag);
                                DataLoopNode::NodeSetpointCheck(ControllerProps(Num).SensedNode).needsSetpointChecking = false;
                                if (EMSSetPointErrorFlag) {
                                    if (!NodeHasSPMCtrlVarType(state, ControllerProps(Num).SensedNode, iCtrlVarType_MaxHumRat)) {
                                        ShowContinueError(" ..Humidity ratio setpoint not found on coil air outlet node.");
                                        ShowContinueError(
                                            " ..The setpoint may have been placed on a node downstream of the coil or on an airloop outlet node.");
                                        ShowContinueError(" ..Specify the setpoint and the sensor on the coil air outlet node when possible.");
                                    }
                                }
                            } else if (SELECT_CASE_var == iTemperatureAndHumidityRatio) {
                                CheckIfNodeSetPointManagedByEMS(ControllerProps(Num).SensedNode, iTemperatureSetPoint, EMSSetPointErrorFlag);
                                DataLoopNode::NodeSetpointCheck(ControllerProps(Num).SensedNode).needsSetpointChecking = false;
                                if (EMSSetPointErrorFlag) {
                                    if (!NodeHasSPMCtrlVarType(state, ControllerProps(Num).SensedNode, iCtrlVarType_Temp)) {
                                        ShowContinueError(" ..Temperature setpoint not found on coil air outlet node.");
                                        ShowContinueError(
                                            " ..The setpoint may have been placed on a node downstream of the coil or on an airloop outlet node.");
                                        ShowContinueError(" ..Specify the setpoint and the sensor on the coil air outlet node when possible.");
                                    }
                                }
                                EMSSetPointErrorFlag = false;
                                CheckIfNodeSetPointManagedByEMS(ControllerProps(Num).SensedNode, iHumidityRatioMaxSetPoint, EMSSetPointErrorFlag);
                                DataLoopNode::NodeSetpointCheck(ControllerProps(Num).SensedNode).needsSetpointChecking = false;
                                if (EMSSetPointErrorFlag) {
                                    if (!NodeHasSPMCtrlVarType(state, ControllerProps(Num).SensedNode, iCtrlVarType_MaxHumRat)) {
                                        ShowContinueError(" ..Humidity ratio setpoint not found on coil air outlet node.");
                                        ShowContinueError(
                                            " ..The setpoint may have been placed on a node downstream of the coil or on an airloop outlet node.");
                                        ShowContinueError(" ..Specify the setpoint and the sensor on the coil air outlet node when possible.");
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        // check that actuator nodes are matched by a water coil inlet node
        for (Num = 1; Num <= NumSimpleControllers; ++Num) {
            CheckActuatorNode(state, ControllerProps(Num).ActuatedNode, iNodeType, ActuatorNodeNotFound);
            if (ActuatorNodeNotFound) {
                ErrorsFound = true;
                ShowSevereError(RoutineName + CurrentModuleObject + "=\"" + ControllerProps(Num).ControllerName + "\":");
                ShowContinueError("...the actuator node must also be a water inlet node of a water coil");
            } else { // Node found, check type and action
                if (iNodeType == CoilType_Cooling) {
                    if (ControllerProps(Num).Action == 0) {
                        ControllerProps(Num).Action = iReverseAction;
                    } else if (ControllerProps(Num).Action == iNormalAction) {
                        ShowWarningError(RoutineName + CurrentModuleObject + "=\"" + ControllerProps(Num).ControllerName + "\":");
                        ShowContinueError("...Normal action has been specified for a cooling coil - should be Reverse.");
                        ShowContinueError("...overriding user input action with Reverse Action.");
                        ControllerProps(Num).Action = iReverseAction;
                    }
                } else if (iNodeType == CoilType_Heating) {
                    if (ControllerProps(Num).Action == 0) {
                        ControllerProps(Num).Action = iNormalAction;
                    } else if (ControllerProps(Num).Action == iReverseAction) {
                        ShowWarningError(RoutineName + CurrentModuleObject + "=\"" + ControllerProps(Num).ControllerName + "\":");
                        ShowContinueError("...Reverse action has been specified for a heating coil - should be Normal.");
                        ShowContinueError("...overriding user input action with Normal Action.");
                        ControllerProps(Num).Action = iNormalAction;
                    }
                }
            }
        }

        AlphArray.deallocate();
        cAlphaFields.deallocate();
        cNumericFields.deallocate();
        NumArray.deallocate();
        lAlphaBlanks.deallocate();
        lNumericBlanks.deallocate();

        // CR 8253 check that the sensed nodes in the controllers are in flow order in controller List
        CheckControllerListOrder();

        if (ErrorsFound) {
            ShowFatalError(RoutineName + "Errors found in getting " + CurrentModuleObject + " input.");
        }
    }

    // End of Get Input subroutines for the Module
    //******************************************************************************

    // Beginning Initialization Section of the Module
    //******************************************************************************

    void ResetController(int const ControlNum, bool const DoWarmRestartFlag, bool &IsConvergedFlag)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Fred Buhl
        //       DATE WRITTEN   April 2004
        //       MODIFIED       Dimitri Curtil (LBNL), Feb 2006
        //                      - Added capability for speculative warm restart
        //                      Brent Griffith (NREL), Feb 2010
        //                      - use SetActuatedBranchFlowRate in Plant Utilities (honor hardware min > 0.0)
        //                      - add FirstHVACIteration logic, don't reset if false,
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine resets the actuator inlet flows.

        // METHODOLOGY EMPLOYED:
        // na

        // REFERENCES:
        // na

        // Using/Aliasing
        using PlantUtilities::SetActuatedBranchFlowRate;

        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:

        // SUBROUTINE PARAMETER DEFINITIONS:
        // na

        // INTERFACE BLOCK SPECIFICATIONS
        // na

        // DERIVED TYPE DEFINITIONS
        // na

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int ActuatedNode;
        int SensedNode;
        Real64 NoFlowResetValue;

        ActuatedNode = ControllerProps(ControlNum).ActuatedNode;
        SensedNode = ControllerProps(ControlNum).SensedNode;

        // Set again in ReportController() to ControllerProps(ControlNum)%NextActuatedValue
        //  IF (FirstHVACIteration) THEN
        // DSU3    Node(ActuatedNode)%MassFlowRate = 0.0d0
        NoFlowResetValue = 0.0;
        SetActuatedBranchFlowRate(NoFlowResetValue,
                                  ControllerProps(ControlNum).ActuatedNode,
                                  ControllerProps(ControlNum).ActuatedNodePlantLoopNum,
                                  ControllerProps(ControlNum).ActuatedNodePlantLoopSide,
                                  ControllerProps(ControlNum).ActuatedNodePlantLoopBranchNum,
                                  true);

        //  ENDIF

        // Reset iteration counter and internal variables
        ControllerProps(ControlNum).NumCalcCalls = 0;

        ControllerProps(ControlNum).DeltaSensed = 0.0;
        ControllerProps(ControlNum).SensedValue = 0.0;
        ControllerProps(ControlNum).ActuatedValue = 0.0;

        // Reset setpoint-related quantities
        ControllerProps(ControlNum).SetPointValue = 0.0;
        ControllerProps(ControlNum).IsSetPointDefinedFlag = false;

        // MinAvailActuated and MaxAvailActuated set in InitController()
        ControllerProps(ControlNum).MinAvailActuated = 0.0;
        ControllerProps(ControlNum).MinAvailSensed = 0.0;
        ControllerProps(ControlNum).MaxAvailActuated = 0.0;
        ControllerProps(ControlNum).MaxAvailSensed = 0.0;

        // Restart from previous solution if speculative warm restart flag set
        // Keep same mode and next actuated value unchanged from last controller simulation.
        if (DoWarmRestartFlag) {
            ControllerProps(ControlNum).DoWarmRestartFlag = true;
        } else {
            ControllerProps(ControlNum).DoWarmRestartFlag = false;
            // If no speculative warm restart then reset stored mode and actucated value
            ControllerProps(ControlNum).Mode = iModeNone;
            ControllerProps(ControlNum).NextActuatedValue = 0.0;
        }

        // Only set once per HVAC iteration.
        // Might be overwritten in the InitController() routine.
        // Allow reusing the previous solution while identifying brackets if
        // this is not the first HVAC step of the environment
        ControllerProps(ControlNum).ReusePreviousSolutionFlag = true;
        // Always reset to false by default. Set in CalcSimpleController() on the first controller iteration.
        ControllerProps(ControlNum).ReuseIntermediateSolutionFlag = false;
        // By default not converged
        IsConvergedFlag = false;

        // Reset root finder
        // This is independent of the processing in InitializeRootFinder() performed in Calc() routine.
        RootFinders(ControlNum).StatusFlag = iStatusNone;
        RootFinders(ControlNum).CurrentMethodType = iMethodNone;

        RootFinders(ControlNum).CurrentPoint.DefinedFlag = false;
        RootFinders(ControlNum).CurrentPoint.X = 0.0;
        RootFinders(ControlNum).CurrentPoint.Y = 0.0;

        RootFinders(ControlNum).MinPoint.DefinedFlag = false;
        RootFinders(ControlNum).MaxPoint.DefinedFlag = false;
        RootFinders(ControlNum).LowerPoint.DefinedFlag = false;
        RootFinders(ControlNum).UpperPoint.DefinedFlag = false;
    }

    void InitController(EnergyPlusData &state, int const ControlNum, bool &IsConvergedFlag)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Richard J. Liesen
        //       DATE WRITTEN   July 1998
        //       MODIFIED       Jan. 2004, Shirey/Raustad (FSEC),
        //       MODIFIED       Feb. 2006, Dimitri Curtil (LBNL), Moved first call convergence test code to ResetController()
        //                      Jul. 2016, R. Zhang (LBNL), Applied the water coil supply air temperature sensor offset fault model

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine is for  initializations of the Controller Components.

        // METHODOLOGY EMPLOYED:
        // Uses the status flags to trigger events.

        using DataGlobals::DoingSizing;
        using DataGlobals::KickOffSimulation;
        using DataGlobals::WarmupFlag;
        using DataHVACGlobals::DoSetPointTest;
        using DataPlant::PlantLoop;
        using EMSManager::CheckIfNodeSetPointManagedByEMS;
        using EMSManager::iHumidityRatioMaxSetPoint;
        using EMSManager::iHumidityRatioSetPoint;
        using EMSManager::iMassFlowRateSetPoint;
        using EMSManager::iTemperatureSetPoint;
        using FaultsManager::FaultsCoilSATSensor;
        using FluidProperties::GetDensityGlycol;
        using PlantUtilities::ScanPlantLoopsForNodeNum;
        using PlantUtilities::SetActuatedBranchFlowRate;
        using RootFinder::SetupRootFinder;
        using SetPointManager::GetHumidityRatioVariableType;
        using SetPointManager::iCtrlVarType_HumRat;
        using SetPointManager::iCtrlVarType_MaxHumRat;
        using SetPointManager::iCtrlVarType_MinHumRat;

        static std::string const RoutineName("InitController");

        int ActuatedNode;
        int SensedNode;
        int ControllerIndex;
        static Array1D_bool MyEnvrnFlag;
        static Array1D_bool MySizeFlag;
        static Array1D_bool MyPlantIndexsFlag;
        Real64 rho; // local fluid density

        if (InitControllerOneTimeFlag) {

            MyEnvrnFlag.allocate(NumControllers);
            MySizeFlag.allocate(NumControllers);
            MyPlantIndexsFlag.allocate(NumControllers);
            MyEnvrnFlag = true;
            MySizeFlag = true;
            MyPlantIndexsFlag = true;
            InitControllerOneTimeFlag = false;
        }

        if (!SysSizingCalc && InitControllerSetPointCheckFlag && DoSetPointTest) {
            // check for missing setpoints
            for (ControllerIndex = 1; ControllerIndex <= NumControllers; ++ControllerIndex) {
                SensedNode = ControllerProps(ControllerIndex).SensedNode;
                {
                    auto const SELECT_CASE_var(ControllerProps(ControllerIndex).ControlVar);
                    if (SELECT_CASE_var == iTemperature) { // 'Temperature'
                        if (Node(SensedNode).TempSetPoint == SensedNodeFlagValue) {
                            if (!AnyEnergyManagementSystemInModel) {
                                ShowSevereError("HVACControllers: Missing temperature setpoint for controller type=" +
                                                ControllerProps(ControllerIndex).ControllerType + " Name=\"" +
                                                ControllerProps(ControllerIndex).ControllerName + "\"");
                                ShowContinueError("Node Referenced (by Controller)=" + NodeID(SensedNode));
                                ShowContinueError("  use a Setpoint Manager with Control Variable = \"Temperature\" to establish a setpoint at the "
                                                  "controller sensed node.");
                                SetPointErrorFlag = true;
                            } else {
                                // call to check node is actuated by EMS
                                CheckIfNodeSetPointManagedByEMS(SensedNode, iTemperatureSetPoint, SetPointErrorFlag);
                                if (SetPointErrorFlag) {
                                    ShowSevereError("HVACControllers: Missing temperature setpoint for controller type=" +
                                                    ControllerProps(ControllerIndex).ControllerType + " Name=\"" +
                                                    ControllerProps(ControllerIndex).ControllerName + "\"");
                                    ShowContinueError("Node Referenced (by Controller)=" + NodeID(SensedNode));
                                    ShowContinueError("  use a Setpoint Manager with Control Variable = \"Temperature\" to establish a setpoint at "
                                                      "the controller sensed node.");
                                    ShowContinueError("Or add EMS Actuator to provide temperature setpoint at this node");
                                }
                            }
                        } else {
                            //           Warn if humidity setpoint is detected (only for cooling coils) and control varible is TEMP.
                            if (Node(SensedNode).HumRatMax != SensedNodeFlagValue && ControllerProps(ControllerIndex).Action == iReverseAction) {
                                ShowWarningError("HVACControllers: controller type=" + ControllerProps(ControllerIndex).ControllerType + " Name=\"" +
                                                 ControllerProps(ControllerIndex).ControllerName +
                                                 "\" has detected a maximum humidity ratio setpoint at the control node.");
                                ShowContinueError("Node referenced (by controller)=" + NodeID(SensedNode));
                                ShowContinueError(
                                    "  set the controller control variable to TemperatureAndHumidityRatio if humidity control is desired.");
                                //              SetPointErrorFlag = .TRUE.
                            }
                        }
                    } else if (SELECT_CASE_var == iHumidityRatio) { // 'HumidityRatio'
                        ControllerProps(ControllerIndex).HumRatCntrlType = GetHumidityRatioVariableType(state, SensedNode);
                        if ((ControllerProps(ControlNum).HumRatCntrlType == iCtrlVarType_HumRat &&
                             Node(SensedNode).HumRatSetPoint == SensedNodeFlagValue) ||
                            (ControllerProps(ControlNum).HumRatCntrlType == iCtrlVarType_MaxHumRat &&
                             Node(SensedNode).HumRatMax == SensedNodeFlagValue)) {
                            if (!AnyEnergyManagementSystemInModel) {
                                ShowSevereError("HVACControllers: Missing humidity ratio setpoint for controller type=" +
                                                ControllerProps(ControllerIndex).ControllerType + " Name=\"" +
                                                ControllerProps(ControllerIndex).ControllerName + "\"");
                                ShowContinueError("Node referenced (by controller)=" + NodeID(SensedNode));
                                ShowContinueError("  use a SetpointManager with the field Control Variable = \"MaximumHumidityRatio\" to establish a "
                                                  "setpoint at the controller sensed node.");
                                SetPointErrorFlag = true;
                            } else {
                                CheckIfNodeSetPointManagedByEMS(SensedNode, iHumidityRatioSetPoint, SetPointErrorFlag);
                                if (SetPointErrorFlag) {
                                    ShowSevereError("HVACControllers: Missing humidity ratio setpoint for controller type=" +
                                                    ControllerProps(ControllerIndex).ControllerType + " Name=\"" +
                                                    ControllerProps(ControllerIndex).ControllerName + "\"");
                                    ShowContinueError("Node referenced (by controller)=" + NodeID(SensedNode));
                                    ShowContinueError("  use a SetpointManager with the field Control Variable = \"MaximumHumidityRatio\" to "
                                                      "establish a setpoint at the controller sensed node.");
                                    ShowContinueError("Or add EMS Actuator to provide Humidity Ratio setpoint at this node");
                                }
                            }

                        } else if (ControllerProps(ControlNum).HumRatCntrlType == iCtrlVarType_MinHumRat) {
                            ShowSevereError("HVACControllers: incorrect humidity ratio setpoint for controller type=" +
                                            ControllerProps(ControllerIndex).ControllerType + " Name=\"" +
                                            ControllerProps(ControllerIndex).ControllerName + "\"");
                            ShowContinueError("Node referenced (by controller)=" + NodeID(SensedNode));
                            ShowContinueError("  use a SetpointManager with the field Control Variable = \"MaximumHumidityRatio\" to establish a "
                                              "setpoint at the controller sensed node.");
                            SetPointErrorFlag = true;
                        }
                    } else if (SELECT_CASE_var == iTemperatureAndHumidityRatio) { // 'TemperatureAndHumidityRatio'
                        if (Node(SensedNode).TempSetPoint == SensedNodeFlagValue) {
                            if (!AnyEnergyManagementSystemInModel) {
                                ShowSevereError("HVACControllers: Missing temperature setpoint for controller type=" +
                                                ControllerProps(ControllerIndex).ControllerType + " Name=\"" +
                                                ControllerProps(ControllerIndex).ControllerName + "\"");
                                ShowContinueError("Node Referenced (by Controller)=" + NodeID(SensedNode));
                                ShowContinueError("  use a Setpoint Manager with Control Variable = \"Temperature\" to establish a setpoint at the "
                                                  "controller sensed node.");
                                SetPointErrorFlag = true;
                            } else {
                                // call to check node is actuated by EMS
                                CheckIfNodeSetPointManagedByEMS(SensedNode, iTemperatureSetPoint, SetPointErrorFlag);
                                if (SetPointErrorFlag) {
                                    ShowSevereError("HVACControllers: Missing temperature setpoint for controller type=" +
                                                    ControllerProps(ControllerIndex).ControllerType + " Name=\"" +
                                                    ControllerProps(ControllerIndex).ControllerName + "\"");
                                    ShowContinueError("Node Referenced (by Controller)=" + NodeID(SensedNode));
                                    ShowContinueError("  use a Setpoint Manager with Control Variable = \"Temperature\" to establish a setpoint at "
                                                      "the controller sensed node.");
                                    ShowContinueError("Or add EMS Actuator to provide temperature setpoint at this node");
                                }
                            }
                        }
                        if (Node(SensedNode).HumRatMax == SensedNodeFlagValue) {
                            if (!AnyEnergyManagementSystemInModel) {
                                ShowSevereError("HVACControllers: Missing maximum humidity ratio setpoint for controller type=" +
                                                ControllerProps(ControllerIndex).ControllerType + " Name=\"" +
                                                ControllerProps(ControllerIndex).ControllerName + "\"");
                                ShowContinueError("Node Referenced (by Controller)=" + NodeID(SensedNode));
                                ShowContinueError("  use a SetpointManager with the field Control Variable = \"MaximumHumidityRatio\" to establish a "
                                                  "setpoint at the controller sensed node.");
                                SetPointErrorFlag = true;
                            } else {
                                // call to check node is actuated by EMS
                                CheckIfNodeSetPointManagedByEMS(SensedNode, iHumidityRatioMaxSetPoint, SetPointErrorFlag);
                                if (SetPointErrorFlag) {
                                    ShowSevereError("HVACControllers: Missing maximum humidity ratio setpoint for controller type=" +
                                                    ControllerProps(ControllerIndex).ControllerType + " Name=\"" +
                                                    ControllerProps(ControllerIndex).ControllerName + "\"");
                                    ShowContinueError("Node Referenced (by Controller)=" + NodeID(SensedNode));
                                    ShowContinueError("  use a SetpointManager with the field Control Variable = \"MaximumHumidityRatio\" to "
                                                      "establish a setpoint at the controller sensed node.");
                                    ShowContinueError("Or add EMS Actuator to provide maximum Humidity Ratio setpoint at this node");
                                }
                            }
                        }
                    } else if (SELECT_CASE_var == iFlow) { // 'Flow'
                        if (Node(SensedNode).MassFlowRateSetPoint == SensedNodeFlagValue) {
                            if (!AnyEnergyManagementSystemInModel) {
                                ShowSevereError("HVACControllers: Missing mass flow rate setpoint for controller type=" +
                                                ControllerProps(ControllerIndex).ControllerType + " Name=\"" +
                                                ControllerProps(ControllerIndex).ControllerName + "\"");
                                ShowContinueError("Node Referenced (in Controller)=" + NodeID(SensedNode));
                                ShowContinueError("  use a SetpointManager with the field Control Variable = \"MassFlowRate\" to establish a "
                                                  "setpoint at the controller sensed node.");
                                SetPointErrorFlag = true;
                            } else {
                                // call to check node is actuated by EMS
                                CheckIfNodeSetPointManagedByEMS(SensedNode, iMassFlowRateSetPoint, SetPointErrorFlag);
                                if (SetPointErrorFlag) {
                                    ShowSevereError("HVACControllers: Missing mass flow rate setpoint for controller type=" +
                                                    ControllerProps(ControllerIndex).ControllerType + " Name=\"" +
                                                    ControllerProps(ControllerIndex).ControllerName + "\"");
                                    ShowContinueError("Node Referenced (in Controller)=" + NodeID(SensedNode));
                                    ShowContinueError("  use a SetpointManager with the field Control Variable = \"MassFlowRate\" to establish a "
                                                      "setpoint at the controller sensed node.");
                                    ShowContinueError("Or add EMS Actuator to provide Mass Flow Rate setpoint at this node");
                                }
                            }
                        }
                    }
                }
            }

            InitControllerSetPointCheckFlag = false;
        }

        if (allocated(PlantLoop) && MyPlantIndexsFlag(ControlNum)) {
            ScanPlantLoopsForNodeNum(ControllerProps(ControlNum).ControllerName,
                                     ControllerProps(ControlNum).ActuatedNode,
                                     ControllerProps(ControlNum).ActuatedNodePlantLoopNum,
                                     ControllerProps(ControlNum).ActuatedNodePlantLoopSide,
                                     ControllerProps(ControlNum).ActuatedNodePlantLoopBranchNum);
            MyPlantIndexsFlag(ControlNum) = false;
        }

        if (!SysSizingCalc && MySizeFlag(ControlNum)) {

            SizeController(ControlNum);

            // Check to make sure that the Minimum Flow rate is less than the max.
            if (ControllerProps(ControlNum).MaxVolFlowActuated == 0.0) {
                ShowWarningError(RoutineName + ": Controller:WaterCoil=\"" + ControllerProps(ControlNum).ControllerName +
                                 "\", Maximum Actuated Flow is zero.");
                ControllerProps(ControlNum).MinVolFlowActuated = 0.0;
            } else if (ControllerProps(ControlNum).MinVolFlowActuated >= ControllerProps(ControlNum).MaxVolFlowActuated) {
                ShowFatalError(RoutineName + ": Controller:WaterCoil=\"" + ControllerProps(ControlNum).ControllerName +
                               "\", Minimum control flow is > or = Maximum control flow.");
            }

            // Setup root finder after sizing calculation
            {
                auto const SELECT_CASE_var(ControllerProps(ControlNum).Action);
                if (SELECT_CASE_var == iNormalAction) {
                    SetupRootFinder(RootFinders(ControlNum),
                                    iSlopeIncreasing,
                                    iMethodBrent,
                                    constant_zero,
                                    1.0e-6,
                                    ControllerProps(ControlNum).Offset); // Slope type | Method type | TolX: no relative tolerance for X variables |
                                                                         // ATolX: absolute tolerance for X variables | ATolY: absolute tolerance for
                                                                         // Y variables

                } else if (SELECT_CASE_var == iReverseAction) {
                    SetupRootFinder(RootFinders(ControlNum),
                                    iSlopeDecreasing,
                                    iMethodBrent,
                                    constant_zero,
                                    1.0e-6,
                                    ControllerProps(ControlNum).Offset); // Slope type | Method type | TolX: no relative tolerance for X variables |
                                                                         // ATolX: absolute tolerance for X variables | ATolY: absolute tolerance for
                                                                         // Y variables
                } else {
                    ShowFatalError("InitController: Invalid controller action. Valid choices are \"Normal\" or \"Reverse\"");
                }
            }

            MySizeFlag(ControlNum) = false;
        }

        // Set the sensed and actuated node numbers
        ActuatedNode = ControllerProps(ControlNum).ActuatedNode;
        SensedNode = ControllerProps(ControlNum).SensedNode;

        // Do the Begin Environment initializations
        if (BeginEnvrnFlag && MyEnvrnFlag(ControlNum)) {

            rho = GetDensityGlycol(state,
                                   PlantLoop(ControllerProps(ControlNum).ActuatedNodePlantLoopNum).FluidName,
                                   CWInitConvTemp,
                                   PlantLoop(ControllerProps(ControlNum).ActuatedNodePlantLoopNum).FluidIndex,
                                   RoutineName);

            ControllerProps(ControlNum).MinActuated = rho * ControllerProps(ControlNum).MinVolFlowActuated;
            ControllerProps(ControlNum).MaxActuated = rho * ControllerProps(ControlNum).MaxVolFlowActuated;

            // Turn off scheme to reuse previous solution obtained at last SimAirLoop() call
            ControllerProps(ControlNum).ReusePreviousSolutionFlag = false;
            // Reset solution trackers
            for (auto &e : ControllerProps(ControlNum).SolutionTrackers) {
                e.DefinedFlag = false;
                e.Mode = iModeNone;
                e.ActuatedValue = 0.0;
            }

            MyEnvrnFlag(ControlNum) = false;
        }

        if (!BeginEnvrnFlag) {
            MyEnvrnFlag(ControlNum) = true;
        }

        SetActuatedBranchFlowRate(ControllerProps(ControlNum).NextActuatedValue,
                                  ActuatedNode,
                                  ControllerProps(ControlNum).ActuatedNodePlantLoopNum,
                                  ControllerProps(ControlNum).ActuatedNodePlantLoopSide,
                                  ControllerProps(ControlNum).ActuatedNodePlantLoopBranchNum,
                                  false);

        // Do the following initializations (every time step): This should be the info from
        // the previous components outlets or the node data in this section.
        // Load the node data in this section for the component simulation
        IsConvergedFlag = false;

        {
            auto const SELECT_CASE_var(ControllerProps(ControlNum).ControlVar);
            if (SELECT_CASE_var == iTemperature) { // 'Temperature'
                ControllerProps(ControlNum).SensedValue = Node(SensedNode).Temp;
                // Done once per HVAC step
                if (!ControllerProps(ControlNum).IsSetPointDefinedFlag) {
                    ControllerProps(ControlNum).SetPointValue = Node(SensedNode).TempSetPoint;
                    ControllerProps(ControlNum).IsSetPointDefinedFlag = true;

                    // If there is a fault of water coil SAT sensor (zrp_Jul2016)
                    if (ControllerProps(ControlNum).FaultyCoilSATFlag && (!WarmupFlag) && (!DoingSizing) && (!KickOffSimulation)) {
                        // calculate the sensor offset using fault information
                        int FaultIndex = ControllerProps(ControlNum).FaultyCoilSATIndex;
                        ControllerProps(ControlNum).FaultyCoilSATOffset = FaultsCoilSATSensor(FaultIndex).CalFaultOffsetAct();
                        // update the SetPointValue
                        ControllerProps(ControlNum).SetPointValue = Node(SensedNode).TempSetPoint - ControllerProps(ControlNum).FaultyCoilSATOffset;
                    }
                }

            } else if (SELECT_CASE_var == iTemperatureAndHumidityRatio) { // 'TemperatureAndHumidityRatio'
                if (ControllerProps(ControlNum).HumRatCtrlOverride) {
                    // Humidity ratio control
                    ControllerProps(ControlNum).SensedValue = Node(SensedNode).HumRat;
                } else {
                    // Temperature control
                    ControllerProps(ControlNum).SensedValue = Node(SensedNode).Temp;
                }
                if (!ControllerProps(ControlNum).IsSetPointDefinedFlag) {
                    if (ControllerProps(ControlNum).HumRatCtrlOverride) {
                        // Humidity ratio control
                        ControllerProps(ControlNum).SetPointValue = Node(SensedNode).HumRatMax;
                    } else {
                        // Pure temperature setpoint control strategy
                        ControllerProps(ControlNum).SetPointValue = Node(SensedNode).TempSetPoint;
                    }
                    // Finally indicate thate the setpoint has been computed
                    ControllerProps(ControlNum).IsSetPointDefinedFlag = true;
                }

            } else if (SELECT_CASE_var == iHumidityRatio) { // 'HumidityRatio'
                ControllerProps(ControlNum).SensedValue = Node(SensedNode).HumRat;
                // Done once per HVAC step
                if (!ControllerProps(ControlNum).IsSetPointDefinedFlag) {
                    {
                        auto const SELECT_CASE_var1(ControllerProps(ControlNum).HumRatCntrlType);
                        if (SELECT_CASE_var1 == iCtrlVarType_MaxHumRat) {
                            ControllerProps(ControlNum).SetPointValue = Node(SensedNode).HumRatMax;
                        } else {
                            ControllerProps(ControlNum).SetPointValue = Node(SensedNode).HumRatSetPoint;
                        }
                    }
                    ControllerProps(ControlNum).IsSetPointDefinedFlag = true;
                }

            } else if (SELECT_CASE_var == iFlow) { // 'Flow'
                ControllerProps(ControlNum).SensedValue = Node(SensedNode).MassFlowRate;
                // Done once per HVAC step
                if (!ControllerProps(ControlNum).IsSetPointDefinedFlag) {
                    ControllerProps(ControlNum).SetPointValue = Node(SensedNode).MassFlowRateSetPoint;
                    ControllerProps(ControlNum).IsSetPointDefinedFlag = true;
                }

            } else {
                ShowFatalError("Invalid Controller Variable Type=" + ControlVariableTypes(ControllerProps(ControlNum).ControlVar));
            }
        }

        {
            auto const SELECT_CASE_var(ControllerProps(ControlNum).ActuatorVar);
            if (SELECT_CASE_var == iFlow) { // 'Flow'
                // At the beginning of every time step the value is reset to the User Input
                // The interface managers can reset the Max or Min to available values during the time step
                // and these will then be the new setpoint limits for the controller to work within.
                ControllerProps(ControlNum).ActuatedValue = Node(ActuatedNode).MassFlowRate;
                // Compute the currently available min and max bounds for controller.
                // Done only once per HVAC step, as it would not make any sense to modify the min/max
                // bounds during successive iterations of the root finder.
                if (ControllerProps(ControlNum).NumCalcCalls == 0) {
                    ControllerProps(ControlNum).MinAvailActuated =
                        max(Node(ActuatedNode).MassFlowRateMinAvail, ControllerProps(ControlNum).MinActuated);
                    ControllerProps(ControlNum).MaxAvailActuated =
                        min(Node(ActuatedNode).MassFlowRateMaxAvail, ControllerProps(ControlNum).MaxActuated);
                    // MinActuated is user input for minimum actuated flow, use that value if allowed
                    // (i.e., reset MinAvailActuated based on Node%MassFlowRateMaxAvail)
                    ControllerProps(ControlNum).MinAvailActuated =
                        min(ControllerProps(ControlNum).MinAvailActuated, ControllerProps(ControlNum).MaxAvailActuated);
                }

            } else {
                ShowFatalError("Invalid Actuator Variable Type=" + ControlVariableTypes(ControllerProps(ControlNum).ActuatorVar));
            }
        }

        // Compute residual for control function using desired setpoint value and current sensed value
        // NOTE: The delta sensed value might be wrong if the setpoint has not yet been computed.
        //       Make sure not to use it until the setpoint has been computed.
        if (ControllerProps(ControlNum).IsSetPointDefinedFlag) {
            ControllerProps(ControlNum).DeltaSensed = ControllerProps(ControlNum).SensedValue - ControllerProps(ControlNum).SetPointValue;
        } else {
            ControllerProps(ControlNum).DeltaSensed = 0.0;
        }
    }

    void SizeController(int const ControlNum)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Fred Buhl
        //       DATE WRITTEN   November 2001
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine is for sizing Controller Components for which max flow rates have not been
        // specified in the input.

        // METHODOLOGY EMPLOYED:
        // Obtains flow rates from the actuated node. Should have been set by the water coils.

        // REFERENCES:
        // na

        // Using/Aliasing
        using namespace DataSizing;
        using DataConvergParams::HVACEnergyToler;
        using DataConvergParams::HVACTemperatureToler;

        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:

        // SUBROUTINE PARAMETER DEFINITIONS:
        // na

        // INTERFACE BLOCK SPECIFICATIONS
        // na

        // DERIVED TYPE DEFINITIONS
        // na

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int ActuatedNode; // node number of actuated node
        int WaterCompNum;

        ActuatedNode = ControllerProps(ControlNum).ActuatedNode;

        if (ControllerProps(ControlNum).MaxVolFlowActuated == AutoSize) {
            for (WaterCompNum = 1; WaterCompNum <= SaveNumPlantComps; ++WaterCompNum) {
                if (CompDesWaterFlow(WaterCompNum).SupNode == ActuatedNode) {
                    ControllerProps(ControlNum).MaxVolFlowActuated = CompDesWaterFlow(WaterCompNum).DesVolFlowRate;
                }
            }

            if (ControllerProps(ControlNum).MaxVolFlowActuated < SmallWaterVolFlow) {
                ControllerProps(ControlNum).MaxVolFlowActuated = 0.0;
            }
            BaseSizer::reportSizerOutput(ControllerProps(ControlNum).ControllerType,
                                         ControllerProps(ControlNum).ControllerName,
                                         "Maximum Actuated Flow [m3/s]",
                                         ControllerProps(ControlNum).MaxVolFlowActuated);
        }

        if (ControllerProps(ControlNum).Offset == AutoSize) {
            // 2100 = 0.5 * 4.2 * 1000/1.2 * 1.2 where 0.5 is the ratio of chilled water delta T to supply air delta T,
            //   4.2 is the ratio of water density to air density, 1000/1.2 is the ratio of water specific heat to
            //   air specific heat, and 1.2 converts the result from air volumetric flow rate to air mass flow rate.
            //   The assumption is that a temperatute tolerance of 0.001 C is good for an air mass flow rate of 1 kg/s.
            //   So we divide .001 by the air mass flow rate estimated from the water volumetric flow rate to come up
            //   with a temperature tolerance that won't exceed the loop energy error tolerance (10 W).
            // Finally we need to take into account the fact that somebody might change the energy tolerance.
            ControllerProps(ControlNum).Offset =
                (0.001 / (2100.0 * max(ControllerProps(ControlNum).MaxVolFlowActuated, SmallWaterVolFlow))) * (HVACEnergyToler / 10.0);
            // do not let the controller tolerance exceed 1/10 of the loop temperature tolerance.
            ControllerProps(ControlNum).Offset = min(0.1 * HVACTemperatureToler, ControllerProps(ControlNum).Offset);
            BaseSizer::reportSizerOutput(ControllerProps(ControlNum).ControllerType,
                                         ControllerProps(ControlNum).ControllerName,
                                         "Controller Convergence Tolerance",
                                         ControllerProps(ControlNum).Offset);
        }
    }

    // End Initialization Section of the Module
    //******************************************************************************

    // Begin Algorithm Section of the Module
    //******************************************************************************

    void CalcSimpleController(int const ControlNum,
                              bool const FirstHVACIteration,
                              bool &IsConvergedFlag,
                              bool &IsUpToDateFlag,
                              std::string const &ControllerName // used when errors occur
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Dimitri Curtil
        //       DATE WRITTEN   May 2006
        //       MODIFIED       Dimitri Curtil (LBNL), May 2006
        //                      - Added IsPointFlagDefinedFlag to control when the setpoiont should be
        //                        computed depending on the control strategy. This was needed to
        //                        trigger the setpoint calculation for the dual temperature and
        //                        humidity ratio control strategy only once the air loop has been
        //                        evaluated with the max actuated flow.
        //                        See the routine InitController() for more details on the setpoint
        //                        calculation.
        //       MODIFIED       Dimitri Curtil (LBNL), March 2006
        //                      - Added IsUpToDateFlag to detect whether or not the air loop
        //                        has been evaluated prior the first iteration, which allows
        //                        to use the current node values as the first iterate for the root
        //                        finder (for COLD RESTART ONLY).
        //       MODIFIED       Dimitri Curtil (LBNL), Feb 2006
        //                      - Added mode detection capability.
        //                      - Now trying min actuated variable first to
        //                        detect min-constrained cases in 1 iteration.
        //                      - Trying max actuated variable second.
        //                        Checks for max-constrained here instead of in
        //                        NormActuatedCalc mode.
        //                      - Checking for inactive mode as soon as min and max
        //                        support points are known instead of in NormActuatedCalc
        //                        mode.
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine needs a description.

        // METHODOLOGY EMPLOYED:
        // na

        // REFERENCES:
        // na

        // Using/Aliasing
        using General::TrimSigDigits;
        using RootFinder::CheckRootFinderCandidate;
        using RootFinder::InitializeRootFinder;

        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:
        // Set to TRUE if current controller is converged; FALSE if more iteration are needed.
        // Note that an error in the root finding process can be mapped onto IsConvergedFlag=TRUE
        // to avoid continue iterating.
        // TRUE if air loop is up-to-date meaning that the current node values are consistent (air loop evaluated)
        // Only used within the Calc routines

        // SUBROUTINE PARAMETER DEFINITIONS:
        // na

        // INTERFACE BLOCK SPECIFICATIONS
        // na

        // DERIVED TYPE DEFINITIONS
        // na

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int ActuatedNode;
        int SensedNode;

        // Increment counter
        ++ControllerProps(ControlNum).NumCalcCalls;

        // Obtain actuated and sensed nodes
        ActuatedNode = ControllerProps(ControlNum).ActuatedNode;
        SensedNode = ControllerProps(ControlNum).SensedNode;

        // Check to see if the component is running; if not converged and return.  This check will be done
        // by looking at the component mass flow rate at the sensed node.
        if (Node(SensedNode).MassFlowRate == 0.0) {
            ExitCalcController(ControlNum, constant_zero, iModeOff, IsConvergedFlag, IsUpToDateFlag);
            return;
        }

        // Intialize root finder
        if (ControllerProps(ControlNum).NumCalcCalls == 1) {
            // Set min/max boundaries for root finder on first iteration
            InitializeRootFinder(RootFinders(ControlNum),
                                 ControllerProps(ControlNum).MinAvailActuated,
                                 ControllerProps(ControlNum).MaxAvailActuated); // XMin | XMax

            // Only allow to reuse initial evaluation if the air loop is up-to-date.
            // Set in SolveAirLoopControllers()
            // Only reuse initial evaluation if setpoint is already available for the current controller
            // Note that in the case of dual temperature and humidity ratio control strategy since the
            // setpoint at a later iteration, the initial solution cannot be reused.
            // Make sure that the initial candidate value lies within range
            ControllerProps(ControlNum).ReuseIntermediateSolutionFlag =
                IsUpToDateFlag && ControllerProps(ControlNum).IsSetPointDefinedFlag &&
                CheckRootFinderCandidate(RootFinders(ControlNum), ControllerProps(ControlNum).ActuatedValue);

            if (ControllerProps(ControlNum).ReuseIntermediateSolutionFlag) {

                // Reuse intermediate solution obtained with previous controller for the current HVAC step
                // and fire root finder to get next root candidate
                FindRootSimpleController(ControlNum, FirstHVACIteration, IsConvergedFlag, IsUpToDateFlag, ControllerName);

            } else {
                // Always start with min point by default
                ControllerProps(ControlNum).NextActuatedValue = RootFinders(ControlNum).MinPoint.X;
            }

            // Process current iterate and compute next candidate if needed
            // We assume that after the first controller iteration:
            // - the setpoint is defined
            // - the min and max available bounds are defined
            // NOTE: Not explicitly checked but the air mass flow rate must remain constant across successive
            //       controller iterations to ensure that the root finder converges.
        } else {
            // Check that the setpoint is defined
            if (!ControllerProps(ControlNum).IsSetPointDefinedFlag) {
                ShowSevereError("CalcSimpleController: Root finder failed at " + CreateHVACStepFullString());
                ShowContinueError(" Controller name=\"" + ControllerName + "\"");
                ShowContinueError(" Setpoint is not available/defined.");
                ShowFatalError("Preceding error causes program termination.");
            }
            // Monitor invariants across successive controller iterations
            // - min bound
            // - max bound
            if (RootFinders(ControlNum).MinPoint.X != ControllerProps(ControlNum).MinAvailActuated) {
                ShowSevereError("CalcSimpleController: Root finder failed at " + CreateHVACStepFullString());
                ShowContinueError(" Controller name=\"" + ControllerName + "\"");
                ShowContinueError(" Minimum bound must remain invariant during successive iterations.");
                ShowContinueError(" Minimum root finder point=" + TrimSigDigits(RootFinders(ControlNum).MinPoint.X, NumSigDigits));
                ShowContinueError(" Minimum avail actuated=" + TrimSigDigits(ControllerProps(ControlNum).MinAvailActuated, NumSigDigits));
                ShowFatalError("Preceding error causes program termination.");
            }
            if (RootFinders(ControlNum).MaxPoint.X != ControllerProps(ControlNum).MaxAvailActuated) {
                ShowSevereError("CalcSimpleController: Root finder failed at " + CreateHVACStepFullString());
                ShowContinueError(" Controller name=\"" + ControllerName + "\"");
                ShowContinueError(" Maximum bound must remain invariant during successive iterations.");
                ShowContinueError(" Maximum root finder point=" + TrimSigDigits(RootFinders(ControlNum).MaxPoint.X, NumSigDigits));
                ShowContinueError(" Maximum avail actuated=" + TrimSigDigits(ControllerProps(ControlNum).MaxAvailActuated, NumSigDigits));
                ShowFatalError("Preceding error causes program termination.");
            }

            // Updates root finder with current iterate and computes next one if needed
            FindRootSimpleController(ControlNum, FirstHVACIteration, IsConvergedFlag, IsUpToDateFlag, ControllerName);
        }
    }

    void FindRootSimpleController(int const ControlNum,
                                  bool const FirstHVACIteration,
                                  bool &IsConvergedFlag,
                                  bool &IsUpToDateFlag,
                                  std::string const &ControllerName // used when errors occur
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Dimitri Curtil (LBNL)
        //       DATE WRITTEN   March 2006
        //       MODIFIED       na
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // New routine to fire the root finder using the current actuated and sensed values.
        // - Updates IsConvergedFlag depending ou iteration status.
        // - Sets next actuated value to try in ControllerProps(ControlNum)%NextActuatedValue

        // METHODOLOGY EMPLOYED:

        // REFERENCES:

        // Using/Aliasing
        using General::TrimSigDigits;
        using RootFinder::CheckRootFinderCandidate;
        using RootFinder::IterateRootFinder;

        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:

        // SUBROUTINE PARAMETER DEFINITIONS:
        // na

        // INTERFACE BLOCK SPECIFICATIONS
        // na

        // DERIVED TYPE DEFINITIONS
        // na

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int ActuatedNode;
        int SensedNode;
        bool IsDoneFlag; // TRUE if root finder needs to continue iterating, FALSE otherwise.
        bool ReusePreviousSolutionFlag;
        int PreviousSolutionIndex;
        bool PreviousSolutionDefinedFlag;
        int PreviousSolutionMode;
        Real64 PreviousSolutionValue;

        // Obtain actuated and sensed nodes
        ActuatedNode = ControllerProps(ControlNum).ActuatedNode;
        SensedNode = ControllerProps(ControlNum).SensedNode;

        // Update root finder with latest solution point
        // Check for unconstrained/constrained convergence
        // Compute next candidate if not converged yet.
        IterateRootFinder(RootFinders(ControlNum),
                          ControllerProps(ControlNum).ActuatedValue,
                          ControllerProps(ControlNum).DeltaSensed,
                          IsDoneFlag); // root finder's data | X | Y | not used

        // Process root finder if converged or error
        // Map root finder status onto controller mode
        {
            auto const SELECT_CASE_var(RootFinders(ControlNum).StatusFlag);
            if ((SELECT_CASE_var == iStatusNone) || (SELECT_CASE_var == iStatusWarningNonMonotonic) || (SELECT_CASE_var == iStatusWarningSingular)) {
                // We need to keep iterating...
                IsConvergedFlag = false;

                if (FirstHVACIteration) {
                    PreviousSolutionIndex = 1;
                } else {
                    PreviousSolutionIndex = 2;
                }

                PreviousSolutionDefinedFlag = ControllerProps(ControlNum).SolutionTrackers(PreviousSolutionIndex).DefinedFlag;
                PreviousSolutionMode = ControllerProps(ControlNum).SolutionTrackers(PreviousSolutionIndex).Mode;
                PreviousSolutionValue = ControllerProps(ControlNum).SolutionTrackers(PreviousSolutionIndex).ActuatedValue;

                // Attempt to use root at previous HVAC step in place of the candidate produced by the
                // root finder.
                // Set in InitController() depending on controller mode at previous HVAC step iteration
                // Only attempted during bracketing phase of root finder.
                // Check that a previous solution is available
                // Make sure that mode of previous solution was active
                // Make sure that proposed candidate does not conflict with current min/max range and lower/upper brackets
                ReusePreviousSolutionFlag = ControllerProps(ControlNum).ReusePreviousSolutionFlag &&
                                            (RootFinders(ControlNum).CurrentMethodType == iMethodBracket) && PreviousSolutionDefinedFlag &&
                                            (PreviousSolutionMode == iModeActive) &&
                                            CheckRootFinderCandidate(RootFinders(ControlNum), PreviousSolutionValue);

                if (ReusePreviousSolutionFlag) {
                    // Try to reuse saved solution from previous call to SolveAirLoopControllers()
                    // instead of candidate proposed by the root finder
                    ControllerProps(ControlNum).NextActuatedValue = PreviousSolutionValue;

                    // Turn off flag since we can only use the previous solution once per HVAC iteration
                    ControllerProps(ControlNum).ReusePreviousSolutionFlag = false;
                } else {
                    // By default, use candidate value computed by root finder
                    ControllerProps(ControlNum).NextActuatedValue = RootFinders(ControlNum).XCandidate;
                }

            } else if ((SELECT_CASE_var == iStatusOK) || (SELECT_CASE_var == iStatusOKRoundOff)) {
                // Indicate convergence with base value (used to obtain DeltaSensed!)
                ExitCalcController(ControlNum, RootFinders(ControlNum).XCandidate, iModeActive, IsConvergedFlag, IsUpToDateFlag);

            } else if (SELECT_CASE_var == iStatusOKMin) {
                // Indicate convergence with min value
                // Should be the same as ControllerProps(ControlNum)%MinAvailActuated
                ExitCalcController(ControlNum, RootFinders(ControlNum).MinPoint.X, iModeMinActive, IsConvergedFlag, IsUpToDateFlag);

            } else if (SELECT_CASE_var == iStatusOKMax) {
                // Indicate convergence with max value
                // Should be the same as ControllerProps(ControlNum)%MaxAvailActuated
                ExitCalcController(ControlNum, RootFinders(ControlNum).MaxPoint.X, iModeMaxActive, IsConvergedFlag, IsUpToDateFlag);

            } else if (SELECT_CASE_var == iStatusErrorSingular) {
                // Indicate inactive mode with min actuated value
                // NOTE: Original code returned Node(ActuatedNode)%MassFlowRateMinAvail
                //       This was not portable in case the actuated variable was NOT a mass flow rate!
                //       Replaced   Node(ActuatedNode)%MassFlowRateMinAvail
                //       with       RootFinders(ControlNum)%MinPoint%X
                //       which is the same as (see SUBROUTINE InitController)
                //                  ControllerProps(ControlNum)%MinAvailActuated
                ExitCalcController(ControlNum, RootFinders(ControlNum).MinPoint.X, iModeInactive, IsConvergedFlag, IsUpToDateFlag);

                // Abnormal case: should never happen
            } else if (SELECT_CASE_var == iStatusErrorRange) {
                ShowSevereError("FindRootSimpleController: Root finder failed at " + CreateHVACStepFullString());
                ShowContinueError(" Controller name=\"" + ControllerName + "\"");
                ShowContinueError(" Root candidate x=" + TrimSigDigits(ControllerProps(ControlNum).ActuatedValue, NumSigDigits) +
                                  " does not lie within the min/max bounds.");
                ShowContinueError(" Min bound is x=" + TrimSigDigits(RootFinders(ControlNum).MinPoint.X, NumSigDigits));
                ShowContinueError(" Max bound is x=" + TrimSigDigits(RootFinders(ControlNum).MaxPoint.X, NumSigDigits));
                ShowFatalError("Preceding error causes program termination.");

                // Abnormal case: should never happen
            } else if (SELECT_CASE_var == iStatusErrorBracket) {
                ShowSevereError("FindRootSimpleController: Root finder failed at " + CreateHVACStepFullString());
                ShowContinueError(" Controller name=" + ControllerProps(ControlNum).ControllerName);
                ShowContinueError(" Controller action=" + ActionTypes(ControllerProps(ControlNum).Action));
                ShowContinueError(" Root candidate x=" + TrimSigDigits(ControllerProps(ControlNum).ActuatedValue, NumSigDigits) +
                                  " does not lie within the lower/upper brackets.");
                if (RootFinders(ControlNum).LowerPoint.DefinedFlag) {
                    ShowContinueError(" Lower bracket is x=" + TrimSigDigits(RootFinders(ControlNum).LowerPoint.X, NumSigDigits));
                }
                if (RootFinders(ControlNum).UpperPoint.DefinedFlag) {
                    ShowContinueError(" Upper bracket is x=" + TrimSigDigits(RootFinders(ControlNum).UpperPoint.X, NumSigDigits));
                }
                ShowFatalError("Preceding error causes program termination.");

                // Detected control function with wrong action between the min and max points.
                // Should never happen: probably indicative of some serious problems in IDFs
                // NOTE: This approach is more robust and consistent than what was done in version 1.3.
                //       Indeed, such a function with the wrong action characteristic would have silently returned
                //       either of the following values depending on the specified action:
                //       - NORMAL ACTION:
                //         - If y(xMin) > ySetPoint && y(xMax) < y(xMin), then  x = xMin
                //         - If y(xMin) < ySetPoint && y(xMax) < y(xMin), then  x = xMax
                //       - REVERSE ACTION:
                //         - If y(xMin) < ySetPoint && y(xMax) > y(xMin), then  x = xMin
                //         - If y(xMin) > ySetPoint && y(xMax) > y(xMin), then  x = xMax
            } else if (SELECT_CASE_var == iStatusErrorSlope) {
                // CALL ShowSevereError('FindRootSimpleController: Root finder failed at '//TRIM(CreateHVACStepFullString()))
                // CALL ShowContinueError( &
                //   'FindRootSimpleController: Controller name='//TRIM(ControllerProps(ControlNum)%ControllerName) &
                //  )
                //  CALL ShowContinueError( &
                //    'FindRootSimpleController: Controller action='//TRIM(ActionTypes(ControllerProps(ControlNum)%Action)) &
                //  )
                //  CALL ShowContinueError( &
                //    'FindRootSimpleController: Controller setpoint='// &
                //    TRIM(TrimSigDigits(ControllerProps(ControlNum)%SetPointValue,NumSigDigits)) &
                //  )
                //  CALL ShowContinueError( &
                //    'FindRootSimpleController: Controller function is inconsistent with the specified action.' &
                //  )
                //  CALL ShowContinueError( &
                //    'FindRootSimpleController: Min bound is '// &
                //    'x='//TRIM(TrimSigDigits(RootFinders(ControlNum)%MinPoint%X,NumSigDigits))//','// &
                //    'y='//TRIM(TrimSigDigits(RootFinders(ControlNum)%MinPoint%Y,NumSigDigits)) &
                //  )
                //  CALL ShowContinueError( &
                //    'FindRootSimpleController: Max bound is '// &
                //    'x='//TRIM(TrimSigDigits(RootFinders(ControlNum)%MaxPoint%X,NumSigDigits))//','// &
                //    'y='//TRIM(TrimSigDigits(RootFinders(ControlNum)%MaxPoint%Y,NumSigDigits)) &
                //  )
                //  CALL ShowFatalError('FindRootSimpleController: Preceding error causes program termination.')
                if (!WarmupFlag && ControllerProps(ControlNum).BadActionErrCount == 0) {
                    ++ControllerProps(ControlNum).BadActionErrCount;
                    ShowSevereError("FindRootSimpleController: Controller error for controller = \"" + ControllerName + "\"");
                    ShowContinueErrorTimeStamp("");
                    ShowContinueError("  Controller function is inconsistent with user specified controller action = " +
                                      ActionTypes(ControllerProps(ControlNum).Action));
                    ShowContinueError("  Actuator will be set to maximum action");
                    ShowContinueError("Controller control type=" + ControlVariableTypes(ControllerProps(ControlNum).ControlVar));
                    if (ControllerProps(ControlNum).ControlVar == iTemperature) {
                        ShowContinueError("Controller temperature setpoint = " + TrimSigDigits(ControllerProps(ControlNum).SetPointValue, 2) +
                                          " [C]");
                        ShowContinueError("Controller sensed temperature = " + TrimSigDigits(ControllerProps(ControlNum).SensedValue, 2) + " [C]");
                    } else if (ControllerProps(ControlNum).ControlVar == iHumidityRatio) {
                        ShowContinueError("Controller humidity ratio setpoint = " + TrimSigDigits(ControllerProps(ControlNum).SetPointValue, 2) +
                                          " [kgWater/kgDryAir]");
                        ShowContinueError("Controller sensed humidity ratio = " + TrimSigDigits(ControllerProps(ControlNum).SensedValue, 2) +
                                          " [kgWater/kgDryAir]");
                    } else if (ControllerProps(ControlNum).ControlVar == iTemperatureAndHumidityRatio) {
                        ShowContinueError("Controller temperature setpoint = " + TrimSigDigits(ControllerProps(ControlNum).SetPointValue, 2) +
                                          " [C]");
                        ShowContinueError("Controller sensed temperature = " + TrimSigDigits(ControllerProps(ControlNum).SensedValue, 2) + " [C]");
                        ShowContinueError("Controller humidity ratio setpoint = " +
                                          TrimSigDigits(Node(ControllerProps(ControlNum).SensedNode).HumRatMax, 2) + " [kgWater/kgDryAir]");
                        ShowContinueError("Controller sensed humidity ratio = " +
                                          TrimSigDigits(Node(ControllerProps(ControlNum).SensedNode).HumRat, 2) + " [kgWater/kgDryAir]");
                    } else if (ControllerProps(ControlNum).ControlVar == iFlow) {
                        ShowContinueError("Controller mass flow rate setpoint = " + TrimSigDigits(ControllerProps(ControlNum).SetPointValue, 2) +
                                          " [kg/s]");
                        ShowContinueError("Controller sensed mass flow rate = " + TrimSigDigits(ControllerProps(ControlNum).SensedValue, 2) +
                                          " [kg/s]");
                    } else {
                        // bad control variable input checked in input routine
                    }
                    if (ControllerProps(ControlNum).ActuatorVar == iFlow) {
                        ShowContinueError("Controller actuator mass flow rate set to " +
                                          TrimSigDigits(ControllerProps(ControlNum).MaxAvailActuated, 2) + " [kg/s]");
                        if (ControllerProps(ControlNum).ControlVar == iTemperature) {
                            ShowContinueError("Controller actuator temperature = " +
                                              TrimSigDigits(Node(ControllerProps(ControlNum).ActuatedNode).Temp, 2) + " [C]");
                            ShowContinueError("  Note: Chilled water coils should be reverse action and the entering chilled");
                            ShowContinueError("        water temperature (controller actuator temperature) should be below the setpoint temperature");
                            ShowContinueError("  Note: Hot water coils should be normal action and the entering hot");
                            ShowContinueError("        water temperature (controller actuator temperature) should be above the setpoint temperature");
                        }
                    } else {
                        // bad actuator variable input checked in input routine
                    }
                } else if (!WarmupFlag) {
                    ++ControllerProps(ControlNum).BadActionErrCount;
                    ShowRecurringSevereErrorAtEnd("FindRootSimpleController: Previous controller action error continues for controller = " +
                                                      ControllerName,
                                                  ControllerProps(ControlNum).BadActionErrIndex);
                } else {
                    // do nothing
                }
                // Indicate convergence with min value
                // Should be the same as ControllerProps(ControlNum)%MaxAvailActuated
                ExitCalcController(ControlNum, RootFinders(ControlNum).MaxPoint.X, iModeMaxActive, IsConvergedFlag, IsUpToDateFlag);

            } else {
                // Should never happen
                ShowSevereError("FindRootSimpleController: Root finder failed at " + CreateHVACStepFullString());
                ShowContinueError(" Controller name=" + ControllerName);
                ShowContinueError(" Unrecognized root finder status flag=" + TrimSigDigits(RootFinders(ControlNum).StatusFlag));
                ShowFatalError("Preceding error causes program termination.");
            }
        }
    }

    void CheckSimpleController(int const ControlNum, bool &IsConvergedFlag)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Dimitri Curtil (LBNL)
        //       DATE WRITTEN   Feb 2006
        //       MODIFIED       na
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // New routine used to detect whether controller can be considered converged
        // depending on its mode of operation.
        // Used after all controllers on an air loop have been solved in order
        // to make sure that final air loop state still represents a converged
        // state.
        // PRECONDITION: Setpoint must be known. See ControllerProps%IsSetPointDefinedFlag

        // METHODOLOGY EMPLOYED:

        // REFERENCES:

        // Using/Aliasing
        using General::TrimSigDigits;
        using RootFinder::CheckRootFinderConvergence;

        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:

        // SUBROUTINE PARAMETER DEFINITIONS:
        // na

        // INTERFACE BLOCK SPECIFICATIONS
        // na

        // DERIVED TYPE DEFINITIONS
        // na

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int ActuatedNode;
        int SensedNode;

        // Obtain actuated and sensed nodes
        ActuatedNode = ControllerProps(ControlNum).ActuatedNode;
        SensedNode = ControllerProps(ControlNum).SensedNode;

        // Default initialization: assuming no convergence unless detected in the following code!
        IsConvergedFlag = false;

        {
            auto const SELECT_CASE_var(ControllerProps(ControlNum).Mode);
            if (SELECT_CASE_var == iModeOff) {
                // Check whether the component is running
                // This check is perfomed by looking at the component mass flow rate at the sensed node.
                // Since the components have been simulated before getting here, if they are zero they should be OFF.
                if (Node(SensedNode).MassFlowRate == 0.0) {
                    if (ControllerProps(ControlNum).ActuatedValue == 0.0) {
                        IsConvergedFlag = true;
                        return;
                    }
                }

            } else if (SELECT_CASE_var == iModeInactive) {
                // Controller component NOT available (ie, inactive)
                // Make sure that the actuated variable is still equal to the node min avail
                // NOTE: Replaced Node(ActuatedNode)%MassFlowRateMinAvail         in release 1.3
                //       with     ControllerProps(ControlNum)%MinAvailActuated    in release 1.4
                if (ControllerProps(ControlNum).ActuatedValue == ControllerProps(ControlNum).MinAvailActuated) {
                    IsConvergedFlag = true;
                    return;
                }

            } else if (SELECT_CASE_var == iModeMinActive) {
                // Check for min constrained convergence
                if (CheckMinActiveController(ControlNum)) {
                    IsConvergedFlag = true;
                    return;
                }
                // Check for unconstrained convergence assuming that there is more than one controller controlling
                // the same sensed node and that the other controller was able to meet the setpoint although this one
                // was min-constrained.
                if (CheckRootFinderConvergence(RootFinders(ControlNum), ControllerProps(ControlNum).DeltaSensed)) {
                    // Indicate convergence with base value (used to compute DeltaSensed!)
                    IsConvergedFlag = true;
                    return;
                }

            } else if (SELECT_CASE_var == iModeMaxActive) {
                // Check for max constrained convergence
                if (CheckMaxActiveController(ControlNum)) {
                    IsConvergedFlag = true;
                    return;
                }
                // Check for unconstrained convergence assuming that there is more than one controller controlling
                // the same sensed node and that the other controller was able to meet the setpoint although this one
                // was max-constrained.
                if (CheckRootFinderConvergence(RootFinders(ControlNum), ControllerProps(ControlNum).DeltaSensed)) {
                    // Indicate convergence with base value (used to compute DeltaSensed!)
                    IsConvergedFlag = true;
                    return;
                }

            } else if (SELECT_CASE_var == iModeActive) {
                // Check min constraint on actuated variable
                if (ControllerProps(ControlNum).ActuatedValue < ControllerProps(ControlNum).MinAvailActuated) {
                    IsConvergedFlag = false;
                    return;
                }
                // Check max constraint on actuated variable
                if (ControllerProps(ControlNum).ActuatedValue > ControllerProps(ControlNum).MaxAvailActuated) {
                    IsConvergedFlag = false;
                    return;
                }

                // Check for unconstrained convergence
                // Equivalent to:
                // IF ((ABS(ControllerProps(ControlNum)%DeltaSensed) .LE. ControllerProps(ControlNum)%Offset)) THEN
                // NOTE: If setpoint has changed since last call, then the following test will most likely fail.
                if (CheckRootFinderConvergence(RootFinders(ControlNum), ControllerProps(ControlNum).DeltaSensed)) {
                    // Indicate convergence with base value (used to compute DeltaSensed!)
                    IsConvergedFlag = true;
                    return;
                }
                // Check for min constrained convergence
                if (CheckMinActiveController(ControlNum)) {
                    IsConvergedFlag = true;
                    return;
                }
                // Check for max constrained convergence
                if (CheckMaxActiveController(ControlNum)) {
                    IsConvergedFlag = true;
                    return;
                }

            } else {
                // Can only happen if controller is not converged after MaxIter in SolveAirLoopControllers()
                // which will produce ControllerProps(ControlNum)%Mode = iModeNone
                IsConvergedFlag = false;
            }
        }
    }

    bool CheckMinActiveController(int const ControlNum)
    {
        // FUNCTION INFORMATION:
        //       AUTHOR         Dimitri Curtil
        //       DATE WRITTEN   May 2006
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS FUNCTION:
        // Returns true if controller is min-constrained. false otherwise.

        // METHODOLOGY EMPLOYED:
        // na

        // REFERENCES:
        // na

        // USE STATEMENTS:
        // na

        // Return value
        bool CheckMinActiveController;

        // Locals
        // FUNCTION ARGUMENT DEFINITIONS:

        // FUNCTION PARAMETER DEFINITIONS:
        // na

        // INTERFACE BLOCK SPECIFICATIONS
        // na

        // DERIVED TYPE DEFINITIONS
        // na

        // FUNCTION LOCAL VARIABLE DECLARATIONS:
        // na

        CheckMinActiveController = false;

        // Check that actuated value is the min avail actuated value
        if (ControllerProps(ControlNum).ActuatedValue != ControllerProps(ControlNum).MinAvailActuated) {
            CheckMinActiveController = false;
            return CheckMinActiveController;
        }

        {
            auto const SELECT_CASE_var(ControllerProps(ControlNum).Action);
            if (SELECT_CASE_var == iNormalAction) { // "NORMAL"
                // Check for min constrained convergence
                if (ControllerProps(ControlNum).SetPointValue <= ControllerProps(ControlNum).SensedValue) {
                    CheckMinActiveController = true;
                    return CheckMinActiveController;
                }

            } else if (SELECT_CASE_var == iReverseAction) { // "REVERSE"
                // Check for min constrained convergence
                if (ControllerProps(ControlNum).SetPointValue >= ControllerProps(ControlNum).SensedValue) {
                    CheckMinActiveController = true;
                    return CheckMinActiveController;
                }

            } else {
                // Should never happen
                ShowSevereError("CheckMinActiveController: Invalid controller action during " + CreateHVACStepFullString() + '.');
                ShowContinueError("CheckMinActiveController: Controller name=" + ControllerProps(ControlNum).ControllerName);
                ShowContinueError("CheckMinActiveController: Valid choices are \"NORMAL\" or \"REVERSE\"");
                ShowFatalError("CheckMinActiveController: Preceding error causes program termination.");
            }
        }

        return CheckMinActiveController;
    }

    bool CheckMaxActiveController(int const ControlNum)
    {
        // FUNCTION INFORMATION:
        //       AUTHOR         Dimitri Curtil
        //       DATE WRITTEN   May 2006
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS FUNCTION:
        // Returns true if controller is max-constrained. false otherwise.

        // METHODOLOGY EMPLOYED:
        // na

        // REFERENCES:
        // na

        // USE STATEMENTS:
        // na

        // Return value
        bool CheckMaxActiveController;

        // Locals
        // FUNCTION ARGUMENT DEFINITIONS:

        // FUNCTION PARAMETER DEFINITIONS:
        // na

        // INTERFACE BLOCK SPECIFICATIONS
        // na

        // DERIVED TYPE DEFINITIONS
        // na

        // FUNCTION LOCAL VARIABLE DECLARATIONS:
        // na

        CheckMaxActiveController = false;

        // Check that actuated value is the max avail actuated value
        if (ControllerProps(ControlNum).ActuatedValue != ControllerProps(ControlNum).MaxAvailActuated) {
            CheckMaxActiveController = false;
            return CheckMaxActiveController;
        }

        {
            auto const SELECT_CASE_var(ControllerProps(ControlNum).Action);
            if (SELECT_CASE_var == iNormalAction) { // "NORMAL"
                // Check for max constrained convergence
                if (ControllerProps(ControlNum).SetPointValue >= ControllerProps(ControlNum).SensedValue) {
                    CheckMaxActiveController = true;
                    return CheckMaxActiveController;
                }

            } else if (SELECT_CASE_var == iReverseAction) { // "REVERSE"
                // Check for max constrained convergence
                if (ControllerProps(ControlNum).SetPointValue <= ControllerProps(ControlNum).SensedValue) {
                    CheckMaxActiveController = true;
                    return CheckMaxActiveController;
                }

            } else {
                // Should never happen
                ShowSevereError("CheckMaxActiveController: Invalid controller action during " + CreateHVACStepFullString() + '.');
                ShowContinueError("CheckMaxActiveController: Controller name=" + ControllerProps(ControlNum).ControllerName);
                ShowContinueError("CheckMaxActiveController: Valid choices are \"NORMAL\" or \"REVERSE\"");
                ShowFatalError("CheckMaxActiveController: Preceding error causes program termination.");
            }
        }

        return CheckMaxActiveController;
    }

    void SaveSimpleController(int const ControlNum, bool const FirstHVACIteration, bool const IsConvergedFlag)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Dimitri Curtil
        //       DATE WRITTEN   April 2006
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Updates solution trackers if simple controller is converged.

        // METHODOLOGY EMPLOYED:

        // REFERENCES:

        // USE STATEMENTS:
        // na

        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:

        // SUBROUTINE PARAMETER DEFINITIONS:
        // na

        // INTERFACE BLOCK SPECIFICATIONS
        // na

        // DERIVED TYPE DEFINITIONS
        // na

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int PreviousSolutionIndex;

        // FLOW

        // Save solution and mode for next call only if converged
        if (IsConvergedFlag) {
            if (FirstHVACIteration) {
                PreviousSolutionIndex = 1;
            } else {
                PreviousSolutionIndex = 2;
            }

            if (ControllerProps(ControlNum).Mode == iModeActive) {
                ControllerProps(ControlNum).SolutionTrackers(PreviousSolutionIndex).DefinedFlag = true;
                ControllerProps(ControlNum).SolutionTrackers(PreviousSolutionIndex).Mode = ControllerProps(ControlNum).Mode;
                ControllerProps(ControlNum).SolutionTrackers(PreviousSolutionIndex).ActuatedValue = ControllerProps(ControlNum).NextActuatedValue;
            } else {
                ControllerProps(ControlNum).SolutionTrackers(PreviousSolutionIndex).DefinedFlag = false;
                ControllerProps(ControlNum).SolutionTrackers(PreviousSolutionIndex).Mode = ControllerProps(ControlNum).Mode;
                ControllerProps(ControlNum).SolutionTrackers(PreviousSolutionIndex).ActuatedValue = ControllerProps(ControlNum).NextActuatedValue;
            }
        }
    }

    void UpdateController(int const ControlNum)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         <author>
        //       DATE WRITTEN   <date_written>
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine updates the actuated node with the next candidate value.

        // METHODOLOGY EMPLOYED:
        // Needs description, as appropriate.

        // REFERENCES:
        // na

        // Using/Aliasing
        using PlantUtilities::SetActuatedBranchFlowRate;

        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:

        // SUBROUTINE PARAMETER DEFINITIONS:
        // na

        // INTERFACE BLOCK SPECIFICATIONS
        // na

        // DERIVED TYPE DEFINITIONS
        // na

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int ActuatedNode;
        int SensedNode;

        // Set the sensed and actuated node numbers
        ActuatedNode = ControllerProps(ControlNum).ActuatedNode;
        SensedNode = ControllerProps(ControlNum).SensedNode;

        // Set the actuated node of the Controller
        {
            auto const SELECT_CASE_var(ControllerProps(ControlNum).ActuatorVar);
            if (SELECT_CASE_var == iFlow) { // 'Flow'
                SetActuatedBranchFlowRate(ControllerProps(ControlNum).NextActuatedValue,
                                          ControllerProps(ControlNum).ActuatedNode,
                                          ControllerProps(ControlNum).ActuatedNodePlantLoopNum,
                                          ControllerProps(ControlNum).ActuatedNodePlantLoopSide,
                                          ControllerProps(ControlNum).ActuatedNodePlantLoopBranchNum,
                                          false);
                //     Node(ActuatedNode)%MassFlowRate = ControllerProps(ControlNum)%NextActuatedValue

            } else {
                ShowFatalError("UpdateController: Invalid Actuator Variable Type=" + ControlVariableTypes(ControllerProps(ControlNum).ActuatorVar));
            }
        }
    }

    //        End of Update subroutines for the Controller Module
    // *****************************************************************************

    void CheckTempAndHumRatCtrl(int const ControlNum, bool &IsConvergedFlag)
    {

        {
            auto &thisController(ControllerProps(ControlNum));
            if (IsConvergedFlag) {
                if (thisController.ControlVar == iTemperatureAndHumidityRatio) {
                    // For temperature and humidity control, after temperature control is converged, check if humidity setpoint is met
                    if (!thisController.HumRatCtrlOverride) {
                        // For humidity control tolerance, always use 0.0001 which is roughly equivalent to a 0.015C change in dewpoint
                        if (Node(thisController.SensedNode).HumRat > (Node(thisController.SensedNode).HumRatMax + 1.0e-5)) {
                            // Turn on humdity control and restart controller
                            IsConvergedFlag = false;
                            thisController.HumRatCtrlOverride = true;
                            if (thisController.Action == iReverseAction) {
                                // Cooling coil controller should always be ReverseAction, but skip this if not
                                RootFinder::SetupRootFinder(
                                    RootFinders(ControlNum), iSlopeDecreasing, iMethodFalsePosition, constant_zero, 1.0e-6, 1.0e-5);
                            }
                            // Do a cold start reset, same as iControllerOpColdStart
                            ResetController(ControlNum, false, IsConvergedFlag);
                        }
                    }
                }
            }
        }
    }

    void ExitCalcController(int const ControlNum, Real64 const NextActuatedValue, int const Mode, bool &IsConvergedFlag, bool &IsUpToDateFlag)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Dimitri Curtil
        //       DATE WRITTEN   February 06
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Only called when controller is considered as "converged", meaning that we do no longer
        // need to continue iterating.

        // METHODOLOGY EMPLOYED:
        // Updates:
        // - next actuated value
        // - controller mode
        // - IsConvergedFlag
        // - IsUpToDateFlag

        // REFERENCES:
        // na

        // USE STATEMENTS:
        // na

        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:

        // SUBROUTINE PARAMETER DEFINITIONS:
        // na

        // INTERFACE BLOCK SPECIFICATIONS
        // na

        // DERIVED TYPE DEFINITIONS
        // na

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        // na

        ControllerProps(ControlNum).NextActuatedValue = NextActuatedValue;
        ControllerProps(ControlNum).Mode = Mode;
        IsConvergedFlag = true;

        // Set IsUpToDateFlag upon exiting to indicate caller whether or not the air loop needs to be
        // re-simulated with the current candidate value, ie ControllerProps(ControlNum)%NextActuatedValue
        if (ControllerProps(ControlNum).ActuatedValue != ControllerProps(ControlNum).NextActuatedValue) {
            IsUpToDateFlag = false;
        } else {
            IsUpToDateFlag = true;
        }
    }

    // Beginning of Statistics subroutines for the Controller Module
    // *****************************************************************************

    void TrackAirLoopControllers(
        int const AirLoopNum, int const WarmRestartStatus, int const AirLoopIterMax, int const AirLoopIterTot, int const AirLoopNumCalls)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Dimitri Curtil
        //       DATE WRITTEN   April 2006
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Updates runtime statistics for controllers on the specified air loop.
        // Used to produce objective metrics when analyzing runtime performance
        // of HVAC controllers for different implementations.

        // METHODOLOGY EMPLOYED:
        // Needs description, as appropriate.

        // REFERENCES:
        // na

        // Using/Aliasing
        using DataAirSystems::PrimaryAirSystem;
        using DataHVACGlobals::NumPrimaryAirSys;

        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:
        // na

        // SUBROUTINE PARAMETER DEFINITIONS:
        // See CONTROLLER_WARM_RESTART_<> parameters in DataHVACControllers.cc
        // If Status<0, no speculative warm restart.
        // If Status==0, speculative warm restart failed.
        // If Status>0, speculative warm restart succeeded.
        // Max number of iterations performed by controllers on this air loop (per call to SimAirLoop)
        // Aggregated number of iterations performed by controllers on this air loop (per call to SimAirLoop)
        // Number of times SimAirLoopComponents() has been invoked

        // INTERFACE BLOCK SPECIFICATIONS
        // na

        // DERIVED TYPE DEFINITIONS
        // na

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int ControllerNum;

        // FLOW

        // If no controllers on this air loop then we have nothig to do
        if (PrimaryAirSystem(AirLoopNum).NumControllers == 0) return;
        // To avoid tracking statistics in case of no air loop or no HVAC controllers are defined
        if (NumAirLoopStats == 0) return;

        // Update performance statistics for air loop
        ++AirLoopStats(AirLoopNum).NumCalls;

        {
            auto const SELECT_CASE_var(WarmRestartStatus);
            if (SELECT_CASE_var == iControllerWarmRestartSuccess) {
                ++AirLoopStats(AirLoopNum).NumSuccessfulWarmRestarts;
            } else if (SELECT_CASE_var == iControllerWarmRestartFail) {
                ++AirLoopStats(AirLoopNum).NumFailedWarmRestarts;
            } else {
                // Nothing to do if no speculative warm restart used
            }
        }

        AirLoopStats(AirLoopNum).TotSimAirLoopComponents += AirLoopNumCalls;

        AirLoopStats(AirLoopNum).MaxSimAirLoopComponents = max(AirLoopStats(AirLoopNum).MaxSimAirLoopComponents, AirLoopNumCalls);

        AirLoopStats(AirLoopNum).TotIterations += AirLoopIterTot;

        AirLoopStats(AirLoopNum).MaxIterations = max(AirLoopStats(AirLoopNum).MaxIterations, AirLoopIterMax);

        // Update performance statistics for each controller on air loop
        for (ControllerNum = 1; ControllerNum <= PrimaryAirSystem(AirLoopNum).NumControllers; ++ControllerNum) {
            TrackAirLoopController(AirLoopNum, ControllerNum);
        }
    }

    void TrackAirLoopController(int const AirLoopNum,       // Air loop index
                                int const AirLoopControlNum // Controller index on this air loop
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Dimitri Curtil
        //       DATE WRITTEN   April 2006
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Updates runtime statistics for the specified controller.
        // Used to produce objective metrics when analyzing runtime performance
        // of HVAC controllers for different implementations.

        // METHODOLOGY EMPLOYED:
        // Needs description, as appropriate.

        // REFERENCES:
        // na

        // Using/Aliasing
        using DataAirSystems::PrimaryAirSystem;
        using DataHVACGlobals::NumPrimaryAirSys;

        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:
        // na

        // SUBROUTINE PARAMETER DEFINITIONS:

        // INTERFACE BLOCK SPECIFICATIONS
        // na

        // DERIVED TYPE DEFINITIONS
        // na

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        // Corresponding index in ControllerProps array
        int ControlIndex;
        // Number of iterations needed to solve this controller
        int IterationCount;
        // Current operating mode
        int Mode;

        // FLOW

        ControlIndex = PrimaryAirSystem(AirLoopNum).ControllerIndex(AirLoopControlNum);

        // We use NumCalcCalls instead of the iteration counter used in SolveAirLoopControllers()
        // to avoid having to call TrackAirLoopController() directly from SolveAirLoopControllers().
        // The 2 counters should be the same anyway as NumCalcCalls is first reset to zero and
        // incremented each time ManageControllers() is invoked with iControllerOpIterate
        IterationCount = ControllerProps(ControlIndex).NumCalcCalls;
        Mode = ControllerProps(ControlIndex).Mode;

        if (Mode != iModeNone) {

            ++AirLoopStats(AirLoopNum).ControllerStats(AirLoopControlNum).NumCalls(Mode);

            AirLoopStats(AirLoopNum).ControllerStats(AirLoopControlNum).TotIterations(Mode) += IterationCount;

            AirLoopStats(AirLoopNum).ControllerStats(AirLoopControlNum).MaxIterations(Mode) =
                max(AirLoopStats(AirLoopNum).ControllerStats(AirLoopControlNum).MaxIterations(Mode), IterationCount);
        }
    }

    void DumpAirLoopStatistics()
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Dimitri Curtil
        //       DATE WRITTEN   April 2006
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Writes runtime statistics for controllers on all air loops
        // to a CSV file named "statistics.HVACControllers.csv".

        // METHODOLOGY EMPLOYED:
        // Needs description, as appropriate.

        // REFERENCES:
        // na

        // Using/Aliasing
        using DataAirSystems::PrimaryAirSystem;
        using DataHVACGlobals::NumPrimaryAirSys;
        using DataSystemVariables::TrackAirLoopEnvFlag;

        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:
        // na

        // SUBROUTINE PARAMETER DEFINITIONS:
        // na

        // INTERFACE BLOCK SPECIFICATIONS

        // DERIVED TYPE DEFINITIONS
        // na

        // FLOW

        // Detect if statistics have been generated or not for this run
        if (!TrackAirLoopEnvFlag) {
            return;
        }

        InputOutputFileName StatisticsFileName{"statistics.HVACControllers.csv"};
        auto statisticsFile = StatisticsFileName.open("DumpAirLoopStatistics");

        // note that the AirLoopStats object does not seem to be initialized when this code
        // is executed and it causes a crash here
        for (int AirLoopNum = 1; AirLoopNum <= NumPrimaryAirSys; ++AirLoopNum) {
            WriteAirLoopStatistics(statisticsFile, PrimaryAirSystem(AirLoopNum), AirLoopStats(AirLoopNum));
        }
    }

    void WriteAirLoopStatistics(InputOutputFile &statisticsFile,
                                DefinePrimaryAirSystem const &ThisPrimaryAirSystem,
                                AirLoopStatsType const &ThisAirLoopStats)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Dimitri Curtil
        //       DATE WRITTEN   April 2006
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Writes runtime statistics for controllers on the specified air loop
        // to the specified file.

        // METHODOLOGY EMPLOYED:
        // Needs description, as appropriate.

        // REFERENCES:
        // na

        // Using/Aliasing
        using namespace DataAirSystems;
        using General::TrimSigDigits;

        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:
        // na

        // SUBROUTINE PARAMETER DEFINITIONS:

        // INTERFACE BLOCK SPECIFICATIONS
        // na

        // DERIVED TYPE DEFINITIONS
        // na

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int AirLoopControlNum;
        int NumWarmRestarts;
        Real64 WarmRestartSuccessRatio;
        int NumCalls;
        int TotIterations;
        int MaxIterations;
        Real64 AvgIterations;
        int iModeNum;

        // FLOW

        print(statisticsFile, "{},\n", ThisPrimaryAirSystem.Name);

        // Number of calls to SimAirLoop() has been invoked over the course of the simulation
        // to simulate the specified air loop
        print(statisticsFile, "NumCalls,{}\n", ThisAirLoopStats.NumCalls);

        // Warm restart success ratio
        NumWarmRestarts = ThisAirLoopStats.NumSuccessfulWarmRestarts + ThisAirLoopStats.NumFailedWarmRestarts;
        if (NumWarmRestarts == 0) {
            WarmRestartSuccessRatio = 0.0;
        } else {
            WarmRestartSuccessRatio = double(ThisAirLoopStats.NumSuccessfulWarmRestarts) / double(NumWarmRestarts);
        }

        print(statisticsFile, "NumWarmRestarts,{}\n", NumWarmRestarts);
        print(statisticsFile, "NumSuccessfulWarmRestarts,{}\n", ThisAirLoopStats.NumSuccessfulWarmRestarts);
        print(statisticsFile, "NumFailedWarmRestarts,{}\n", ThisAirLoopStats.NumFailedWarmRestarts);
        print(statisticsFile, "WarmRestartSuccessRatio,{:.10T}\n", WarmRestartSuccessRatio);

        // Total number of times SimAirLoopComponents() has been invoked over the course of the simulation
        // to simulate the specified air loop
        print(statisticsFile, "TotSimAirLoopComponents,{}\n", ThisAirLoopStats.TotSimAirLoopComponents);
        // Maximum number of times SimAirLoopComponents() has been invoked over the course of the simulation
        // to simulate the specified air loop
        print(statisticsFile, "MaxSimAirLoopComponents,{}\n", ThisAirLoopStats.MaxSimAirLoopComponents);

        // Aggregated number of iterations needed by all controllers to simulate the specified air loop
        print(statisticsFile, "TotIterations,{}\n", ThisAirLoopStats.TotIterations);
        // Maximum number of iterations needed by controllers to simulate the specified air loop
        print(statisticsFile, "MaxIterations,{}\n", ThisAirLoopStats.MaxIterations);

        // Average number of iterations needed by controllers to simulate the specified air loop
        if (ThisAirLoopStats.NumCalls == 0) {
            AvgIterations = 0.0;
        } else {
            AvgIterations = double(ThisAirLoopStats.TotIterations) / double(ThisAirLoopStats.NumCalls);
        }

        print(statisticsFile, "AvgIterations,{:.10T}\n", AvgIterations);

        // Dump statistics for each controller on this air loop
        for (AirLoopControlNum = 1; AirLoopControlNum <= ThisPrimaryAirSystem.NumControllers; ++AirLoopControlNum) {

            print(statisticsFile, "{},\n", ThisPrimaryAirSystem.ControllerName(AirLoopControlNum));

            // Aggregate iteration trackers across all operating modes
            NumCalls = 0;
            TotIterations = 0;
            MaxIterations = 0;

            for (iModeNum = iFirstMode; iModeNum <= iLastMode; ++iModeNum) {
                NumCalls += ThisAirLoopStats.ControllerStats(AirLoopControlNum).NumCalls(iModeNum);

                TotIterations += ThisAirLoopStats.ControllerStats(AirLoopControlNum).TotIterations(iModeNum);

                MaxIterations = max(MaxIterations, ThisAirLoopStats.ControllerStats(AirLoopControlNum).MaxIterations(iModeNum));
            }

            // Number of times this controller was simulated (should match air loop num calls)
            print(statisticsFile, "NumCalls,{}\n", NumCalls);
            // Aggregated number of iterations needed by this controller
            print(statisticsFile, "TotIterations,{}\n", TotIterations);
            // Aggregated number of iterations needed by this controller
            print(statisticsFile, "MaxIterations,{}\n", MaxIterations);

            // Average number of iterations needed by controllers to simulate the specified air loop
            if (NumCalls == 0) {
                AvgIterations = 0.0;
            } else {
                AvgIterations = double(TotIterations) / double(NumCalls);
            }
            print(statisticsFile, "AvgIterations,{:.10T}\n", AvgIterations);

            // Dump iteration trackers for each operating mode
            for (iModeNum = iFirstMode; iModeNum <= iLastMode; ++iModeNum) {

                print(statisticsFile, "{},\n", ControllerModeTypes(iModeNum));

                // Number of times this controller operated in this mode
                print(statisticsFile, "NumCalls,{}\n", ThisAirLoopStats.ControllerStats(AirLoopControlNum).NumCalls(iModeNum));

                // Aggregated number of iterations needed by this controller
                print(statisticsFile, "TotIterations,{}\n", ThisAirLoopStats.ControllerStats(AirLoopControlNum).TotIterations(iModeNum));
                // Aggregated number of iterations needed by this controller
                print(statisticsFile, "MaxIterations,{}\n", ThisAirLoopStats.ControllerStats(AirLoopControlNum).MaxIterations(iModeNum));

                // Average number of iterations needed by controllers to simulate the specified air loop
                if (ThisAirLoopStats.ControllerStats(AirLoopControlNum).NumCalls(iModeNum) == 0) {
                    AvgIterations = 0.0;
                } else {
                    AvgIterations = double(ThisAirLoopStats.ControllerStats(AirLoopControlNum).TotIterations(iModeNum)) /
                                    double(ThisAirLoopStats.ControllerStats(AirLoopControlNum).NumCalls(iModeNum));
                }
                print(statisticsFile, "AvgIterations,{:.10T}\n", AvgIterations);
            }
        }
    }

    // Beginning of Tracing subroutines for the Controller Module
    // *****************************************************************************

    void SetupAirLoopControllersTracer(int const AirLoopNum)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Dimitri Curtil
        //       DATE WRITTEN   February 2006
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Opens main trace file for controllers on specific air loop
        // and writes header row with titles.

        // METHODOLOGY EMPLOYED:
        // Needs description, as appropriate.

        // REFERENCES:
        // na

        // Using/Aliasing
        using DataAirSystems::PrimaryAirSystem;
        using General::TrimSigDigits;

        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:
        // na

        // SUBROUTINE PARAMETER DEFINITIONS:

        // INTERFACE BLOCK SPECIFICATIONS

        // DERIVED TYPE DEFINITIONS
        // na

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int ControllerNum;

        // Open main controller trace file for each air loop
        const auto TraceFileName = "controller." + PrimaryAirSystem(AirLoopNum).Name + ".csv";

        // Store file unit in air loop stats
        AirLoopStats(AirLoopNum).TraceFile->fileName = TraceFileName;
        AirLoopStats(AirLoopNum).TraceFile->open();

        if (!AirLoopStats(AirLoopNum).TraceFile->good()) {
            ShowFatalError("SetupAirLoopControllersTracer: Failed to open air loop trace file \"" + TraceFileName + "\" for output (write).");
            return;
        }

        auto &TraceFile = *AirLoopStats(AirLoopNum).TraceFile;

        // List all controllers and their corresponding handles into main trace file
        print(TraceFile, "Num,Name,\n");

        for (ControllerNum = 1; ControllerNum <= PrimaryAirSystem(AirLoopNum).NumControllers; ++ControllerNum) {
            print(TraceFile, "{},{},\n", ControllerNum, PrimaryAirSystem(AirLoopNum).ControllerName(ControllerNum));
            // SAME AS ControllerProps(ControllerIndex)%ControllerName BUT NOT YET AVAILABLE
        }

        // Skip a bunch of lines
        print(TraceFile, "\n\n\n");

        // Write column header in main controller trace file
        print(TraceFile,
              "ZoneSizingCalc,SysSizingCalc,EnvironmentNum,WarmupFlag,SysTimeStamp,SysTimeInterval,BeginTimeStepFlag,FirstTimeStepSysFlag,"
              "FirstHVACIteration,AirLoopPass,AirLoopNumCallsTot,AirLoopConverged,");

        // Write headers for final state
        for (ControllerNum = 1; ControllerNum <= PrimaryAirSystem(AirLoopNum).NumControllers; ++ControllerNum) {
            print(TraceFile,
                  "Mode{},IterMax{},XRoot{},YRoot{},YSetPoint{},\n",
                  ControllerNum,
                  ControllerNum,
                  ControllerNum,
                  ControllerNum,
                  ControllerNum);
        }

        print(TraceFile, "\n");
    }

    void TraceAirLoopControllers(
        bool const FirstHVACIteration, int const AirLoopNum, int const AirLoopPass, bool const AirLoopConverged, int const AirLoopNumCalls)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Dimitri Curtil
        //       DATE WRITTEN   January 2006
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine writes diagnostic to the trace file attached to each air loop.

        // METHODOLOGY EMPLOYED:
        // na

        // REFERENCES:
        // na

        // Using/Aliasing
        using DataAirSystems::PrimaryAirSystem;

        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:
        // TRUE when primary air system & controllers simulation has converged;
        // Number of times SimAirLoopComponents() has been invoked

        // SUBROUTINE PARAMETER DEFINITIONS:
        // na

        // INTERFACE BLOCK SPECIFICATIONS
        // na

        // DERIVED TYPE DEFINITIONS
        // na

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int ControllerNum;

        // FLOW

        // IF no controllers on this air loop then we have nothing to do
        if (PrimaryAirSystem(AirLoopNum).NumControllers == 0) return;
        // To avoid tracking statistics in case of no air loop or no HVAC controllers are defined
        if (NumAirLoopStats == 0) return;

        // Setup trace file on first call only
        if (AirLoopStats(AirLoopNum).FirstTraceFlag) {
            SetupAirLoopControllersTracer(AirLoopNum);

            AirLoopStats(AirLoopNum).FirstTraceFlag = false;
        }

        auto &TraceFile = *AirLoopStats(AirLoopNum).TraceFile;

        if (!TraceFile.good()) return;

        // Write iteration stamp first
        TraceIterationStamp(TraceFile, FirstHVACIteration, AirLoopPass, AirLoopConverged, AirLoopNumCalls);

        // Loop over the air sys controllers and write diagnostic to trace file
        for (ControllerNum = 1; ControllerNum <= PrimaryAirSystem(AirLoopNum).NumControllers; ++ControllerNum) {
            TraceAirLoopController(TraceFile, PrimaryAirSystem(AirLoopNum).ControllerIndex(ControllerNum));
        }

        // Go to next line
        print(TraceFile, "\n");
    }

    void TraceIterationStamp(
        InputOutputFile &TraceFile, bool const FirstHVACIteration, int const AirLoopPass, bool const AirLoopConverged, int const AirLoopNumCalls)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Dimitri Curtil
        //       DATE WRITTEN   February 2006
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Writes current iteration time stamp to specified trace file.

        // METHODOLOGY EMPLOYED:
        // Needs description, as appropriate.

        // REFERENCES:
        // na

        // Using/Aliasing
        using DataEnvironment::CurEnvirNum;
        using DataEnvironment::CurMnDy;
        using DataGlobals::BeginTimeStepFlag;
        using DataGlobals::SysSizingCalc;
        using DataGlobals::WarmupFlag;
        using DataGlobals::ZoneSizingCalc;
        using DataHVACGlobals::FirstTimeStepSysFlag;
        using General::LogicalToInteger;
        using General::TrimSigDigits;

        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:
        // na

        // SUBROUTINE PARAMETER DEFINITIONS:
        // TRUE when primary air system and controllers simulation has converged;
        // Number of times SimAirLoopComponents() has been invoked

        // INTERFACE BLOCK SPECIFICATIONS
        // na

        // DERIVED TYPE DEFINITIONS
        // na

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        // na

        // Write step stamp to air loop trace file after reset
        // Note that we do not go to the next line
        print(TraceFile,
              "{},{},{},{},{},{},{},{},{},{},{},{},",
              LogicalToInteger(ZoneSizingCalc),
              LogicalToInteger(SysSizingCalc),
              CurEnvirNum,
              LogicalToInteger(WarmupFlag),
              CreateHVACTimeString(),
              MakeHVACTimeIntervalString(),
              LogicalToInteger(BeginTimeStepFlag),
              LogicalToInteger(FirstTimeStepSysFlag),
              LogicalToInteger(FirstHVACIteration),
              AirLoopPass,
              AirLoopNumCalls,
              LogicalToInteger(AirLoopConverged));
    }

    void TraceAirLoopController(InputOutputFile &TraceFile, int const ControlNum)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Dimitri Curtil
        //       DATE WRITTEN   January 2006
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine writes convergence diagnostic to the air loop trace file
        // for the specified controller index.

        // METHODOLOGY EMPLOYED:
        // na

        // REFERENCES:
        // na

        // Using/Aliasing
        using General::TrimSigDigits;

        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:

        // SUBROUTINE PARAMETER DEFINITIONS:
        // na

        // INTERFACE BLOCK SPECIFICATIONS
        // na

        // DERIVED TYPE DEFINITIONS
        // na

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int ActuatedNode;
        int SensedNode;

        // Set the sensed and actuated node numbers
        ActuatedNode = ControllerProps(ControlNum).ActuatedNode;
        SensedNode = ControllerProps(ControlNum).SensedNode;

        print(TraceFile,
              "{},{},{:.10T},{:.10T},{:.10T},",
              ControllerProps(ControlNum).Mode,
              ControllerProps(ControlNum).NumCalcCalls,
              Node(ActuatedNode).MassFlowRate,
              Node(SensedNode).Temp,
              Node(SensedNode).TempSetPoint);
    }

    void SetupIndividualControllerTracer(int const ControlNum)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Dimitri Curtil
        //       DATE WRITTEN   February 2006
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Opens individual controller trace file for the specified controller
        // and writes header row.

        // METHODOLOGY EMPLOYED:
        // Needs description, as appropriate.

        // REFERENCES:
        // na

        // Using/Aliasing
        using RootFinder::WriteRootFinderTraceHeader;

        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:

        // SUBROUTINE PARAMETER DEFINITIONS:
        // na

        // INTERFACE BLOCK SPECIFICATIONS

        // DERIVED TYPE DEFINITIONS
        // na

        const auto TraceFileName = "controller." + ControllerProps(ControlNum).ControllerName + ".csv";
        auto &TraceFile = *ControllerProps(ControlNum).TraceFile;
        TraceFile.fileName = TraceFileName;
        TraceFile.open();

        if (!TraceFile.good()) {
            ShowFatalError("SetupIndividualControllerTracer: Failed to open controller trace file \"" + TraceFileName + "\" for output (write).");
            return;
        }

        // Write header row
        // Masss flow rate
        // Convergence analysis
        print(TraceFile,
              "EnvironmentNum,WarmupFlag,SysTimeStamp,SysTimeInterval,AirLoopPass,FirstHVACIteration,Operation,NumCalcCalls,SensedNode%MassFlowRate,"
              "ActuatedNode%MassFlowRateMinAvail,ActuatedNode%MassFlowRateMaxAvail,X,Y,Setpoint,DeltaSensed,Offset,Mode,IsConvergedFlag,"
              "NextActuatedValue");

        WriteRootFinderTraceHeader(TraceFile);

        // Finally skip line
        print(TraceFile, "\n");
    }

    void TraceIndividualController(int const ControlNum,
                                   bool const FirstHVACIteration,
                                   int const AirLoopPass,
                                   int const Operation, // Operation to execute
                                   bool const IsConvergedFlag)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Dimitri Curtil
        //       DATE WRITTEN   January 2006
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine writes convergence diagnostic to the trace file for the specified
        // controller.

        // METHODOLOGY EMPLOYED:
        // na

        // REFERENCES:
        // na

        // Using/Aliasing
        using DataEnvironment::CurEnvirNum;
        using General::LogicalToInteger;
        using General::TrimSigDigits;
        using RootFinder::WriteRootFinderTrace;

        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:

        // SUBROUTINE PARAMETER DEFINITIONS:
        // na

        // INTERFACE BLOCK SPECIFICATIONS
        // na

        // DERIVED TYPE DEFINITIONS
        // na

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int ActuatedNode;
        int SensedNode;
        bool SkipLineFlag;

        // Setup individual trace file on first trace only
        if (ControllerProps(ControlNum).FirstTraceFlag) {
            SetupIndividualControllerTracer(ControlNum);

            ControllerProps(ControlNum).FirstTraceFlag = false;
            SkipLineFlag = false;
        } else {
            SkipLineFlag = FirstHVACIteration && (ControllerProps(ControlNum).NumCalcCalls == 0);
        }

        auto &TraceFile = *ControllerProps(ControlNum).TraceFile;

        // Nothing to do if trace file not registered
        if (!TraceFile.good()) return;

        // Skip a line before each new HVAC step
        if (SkipLineFlag) {
            print(TraceFile, "\n");
        }

        // Set the sensed and actuated node numbers
        ActuatedNode = ControllerProps(ControlNum).ActuatedNode;
        SensedNode = ControllerProps(ControlNum).SensedNode;

        // Write iteration stamp
        print(TraceFile,
              "{},{},{},{},{},{},{},{},",
              CurEnvirNum,
              LogicalToInteger(WarmupFlag),
              CreateHVACTimeString(),
              MakeHVACTimeIntervalString(),
              AirLoopPass,
              LogicalToInteger(FirstHVACIteration),
              Operation,
              ControllerProps(ControlNum).NumCalcCalls);

        // Write detailed diagnostic
        {
            auto const SELECT_CASE_var(Operation);
            if ((SELECT_CASE_var == iControllerOpColdStart) || (SELECT_CASE_var == iControllerOpWarmRestart)) {
                print(TraceFile,
                      "{:.10T},{:.10T},{:.10T},{:.10T},{:.10T},{:.10T},{},{},{},{},{:.10T},",
                      Node(SensedNode).MassFlowRate,
                      Node(ActuatedNode).MassFlowRateMinAvail,
                      Node(ActuatedNode).MassFlowRateMaxAvail,
                      ControllerProps(ControlNum).ActuatedValue,
                      Node(SensedNode).Temp,
                      ControllerProps(ControlNum).SetPointValue,
                      ' ',
                      ' ',
                      ControllerProps(ControlNum).Mode,
                      LogicalToInteger(IsConvergedFlag),
                      ControllerProps(ControlNum).NextActuatedValue);
                // X | Y | setpoint | DeltaSensed = Y - YRoot | Offset | Mode | IsConvergedFlag

                // No trace available for root finder yet
                // Skip call to WriteRootFinderTrace()

                // Finally skip line
                print(TraceFile, "\n");

            } else if (SELECT_CASE_var == iControllerOpIterate) {
                // Masss flow rate
                // Convergence analysis

                print(TraceFile,
                      "{:.10T},{:.10T},{:.10T},{:.10T},{:.10T},{:.10T},{:.10T},{:.10T},{},{},{:.10T},",
                      Node(SensedNode).MassFlowRate,
                      Node(ActuatedNode).MassFlowRateMinAvail,
                      Node(ActuatedNode).MassFlowRateMaxAvail,
                      ControllerProps(ControlNum).ActuatedValue,
                      Node(SensedNode).Temp,
                      ControllerProps(ControlNum).SetPointValue,
                      ControllerProps(ControlNum).DeltaSensed,
                      ControllerProps(ControlNum).Offset,
                      ControllerProps(ControlNum).Mode,
                      LogicalToInteger(IsConvergedFlag),
                      ControllerProps(ControlNum).NextActuatedValue);

                // X | Y | setpoint | DeltaSensed = Y - YRoot | Offset | Mode | IsConvergedFlag

                // Append trace for root finder
                WriteRootFinderTrace(TraceFile, RootFinders(ControlNum));

                // Finally skip line
                print(TraceFile, "\n");

            } else if (SELECT_CASE_var == iControllerOpEnd) {
                // Masss flow rate
                // Convergence analysis
                print(TraceFile,
                      "{:.10T},{:.10T},{:.10T},{:.10T},{:.10T},{:.10T},{:.10T},{:.10T},{},{},{:.10T},",
                      Node(SensedNode).MassFlowRate,
                      Node(ActuatedNode).MassFlowRateMinAvail,
                      Node(ActuatedNode).MassFlowRateMaxAvail,
                      ControllerProps(ControlNum).ActuatedValue,
                      Node(SensedNode).Temp,
                      ControllerProps(ControlNum).SetPointValue,
                      ControllerProps(ControlNum).DeltaSensed,
                      ControllerProps(ControlNum).Offset,
                      ControllerProps(ControlNum).Mode,
                      LogicalToInteger(IsConvergedFlag),
                      ControllerProps(ControlNum).NextActuatedValue);

                // X | Y | setpoint | DeltaSensed = Y - YRoot | Offset | Mode | IsConvergedFlag

                // No trace available for root finder yet
                // Skip call to WriteRootFinderTrace()

                // Finally skip line
                print(TraceFile, "\n");

                // Skip an additional line to indicate end of current HVAC step
                print(TraceFile, "\n");

            } else {
                // Should never happen
                ShowFatalError("TraceIndividualController: Invalid Operation passed=" + TrimSigDigits(Operation) +
                               ", Controller name=" + ControllerProps(ControlNum).ControllerName);
            }
        }

        TraceFile.flush();
    }

    std::string CreateHVACTimeString()
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         Dimitri Curtil
        //       DATE WRITTEN   January 2006
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS FUNCTION:
        // This function creates a string describing the current time stamp of the system
        // time step.

        // METHODOLOGY EMPLOYED:
        // na

        // REFERENCES:
        // na

        // Using/Aliasing
        using DataEnvironment::CurMnDy;
        using General::CreateTimeString;
        using General::GetCurrentHVACTime;

        // Return value
        std::string OutputString;

        // Locals
        // FUNCTION ARGUMENT DEFINITIONS:

        // FUNCTION PARAMETER DEFINITIONS:
        // na
        // INTERFACE BLOCK SPECIFICATIONS
        // na

        // DERIVED TYPE DEFINITIONS
        // na

        // FUNCTION LOCAL VARIABLE DECLARATIONS:
        std::string Buffer;

        Buffer = CreateTimeString(GetCurrentHVACTime());
        OutputString = CurMnDy + ' ' + stripped(Buffer);

        return OutputString;
    }

    std::string CreateHVACStepFullString()
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         Dimitri Curtil
        //       DATE WRITTEN   April 2006
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS FUNCTION:
        // This function creates a string describing the current HVAC step.
        // It includes the environment name, the current day/month and the current
        // time stamp for the system time step.
        // It is used in error messages only.

        // METHODOLOGY EMPLOYED:
        // na

        // REFERENCES:
        // na

        // Using/Aliasing
        using DataEnvironment::EnvironmentName;

        // Return value
        std::string OutputString;

        // Locals
        // FUNCTION ARGUMENT DEFINITIONS:

        // FUNCTION PARAMETER DEFINITIONS:
        // na
        // INTERFACE BLOCK SPECIFICATIONS
        // na

        // DERIVED TYPE DEFINITIONS
        // na

        // FUNCTION LOCAL VARIABLE DECLARATIONS:
        // na

        OutputString = EnvironmentName + ", " + MakeHVACTimeIntervalString();

        return OutputString;
    }

    std::string MakeHVACTimeIntervalString()
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         Dimitri Curtil
        //       DATE WRITTEN   January 2006
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS FUNCTION:
        // This function creates a string describing the current time interval of the system
        // time step.

        // METHODOLOGY EMPLOYED:
        // na

        // REFERENCES:
        // na

        // Using/Aliasing
        using General::CreateHVACTimeIntervalString;

        // Return value
        std::string OutputString;

        // Locals
        // FUNCTION ARGUMENT DEFINITIONS:

        // FUNCTION PARAMETER DEFINITIONS:
        // na
        // INTERFACE BLOCK SPECIFICATIONS
        // na

        // DERIVED TYPE DEFINITIONS
        // na

        // FUNCTION LOCAL VARIABLE DECLARATIONS:
        // na

        OutputString = stripped(CreateHVACTimeIntervalString());

        return OutputString;
    }

    //        End of Tracing subroutines for the Controller Module

    void CheckControllerListOrder()
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         B. Griffith
        //       DATE WRITTEN   Oct 10.
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // check that if multiple controllers on an air loop, that they aren't listed in a bad order
        // CR 8253

        // METHODOLOGY EMPLOYED:
        // setup data for sensed nodes and compare positions if on the same branch

        // Using/Aliasing
        using DataAirSystems::PrimaryAirSystem;
        using DataHVACGlobals::NumPrimaryAirSys;

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int AirSysNum;
        int ContrlNum;
        int WaterCoilContrlCount;
        Array2D_int ContrlSensedNodeNums; // array for storing sense node info
        int SensedNodeIndex;
        int BranchNodeIndex;
        int BranchNum;
        int foundControl;

        for (AirSysNum = 1; AirSysNum <= NumPrimaryAirSys; ++AirSysNum) {

            if (PrimaryAirSystem(AirSysNum).NumControllers > 1) {
                // first see how many are water coil controllers
                WaterCoilContrlCount = 0; // init
                for (ContrlNum = 1; ContrlNum <= PrimaryAirSystem(AirSysNum).NumControllers; ++ContrlNum) {
                    if (UtilityRoutines::SameString(PrimaryAirSystem(AirSysNum).ControllerType(ContrlNum), "CONTROLLER:WATERCOIL")) {
                        ++WaterCoilContrlCount;
                    }
                }

                if (WaterCoilContrlCount > 1) {
                    ContrlSensedNodeNums.allocate(3, WaterCoilContrlCount);
                    ContrlSensedNodeNums = 0;
                    SensedNodeIndex = 0;
                    for (ContrlNum = 1; ContrlNum <= PrimaryAirSystem(AirSysNum).NumControllers; ++ContrlNum) {
                        if (UtilityRoutines::SameString(PrimaryAirSystem(AirSysNum).ControllerType(ContrlNum), "CONTROLLER:WATERCOIL")) {
                            ++SensedNodeIndex;
                            foundControl = UtilityRoutines::FindItemInList(
                                PrimaryAirSystem(AirSysNum).ControllerName(ContrlNum), ControllerProps, &ControllerPropsType::ControllerName);
                            if (foundControl > 0) {
                                ContrlSensedNodeNums(1, SensedNodeIndex) = ControllerProps(foundControl).SensedNode;
                            }
                        }
                    }
                }

                // fill branch index for sensed nodes
                if (allocated(ContrlSensedNodeNums)) {
                    for (BranchNum = 1; BranchNum <= PrimaryAirSystem(AirSysNum).NumBranches; ++BranchNum) {
                        for (SensedNodeIndex = 1; SensedNodeIndex <= WaterCoilContrlCount; ++SensedNodeIndex) {
                            for (BranchNodeIndex = 1; BranchNodeIndex <= PrimaryAirSystem(AirSysNum).Branch(BranchNum).TotalNodes;
                                 ++BranchNodeIndex) {
                                if (ContrlSensedNodeNums(1, SensedNodeIndex) ==
                                    PrimaryAirSystem(AirSysNum).Branch(BranchNum).NodeNum(BranchNodeIndex)) {
                                    ContrlSensedNodeNums(2, SensedNodeIndex) = BranchNodeIndex;
                                    ContrlSensedNodeNums(3, SensedNodeIndex) = BranchNum;
                                }
                            }
                        }
                    }
                }
                // check if flow order doesn't agree with controller order
                if (allocated(ContrlSensedNodeNums)) {
                    for (SensedNodeIndex = 1; SensedNodeIndex <= WaterCoilContrlCount; ++SensedNodeIndex) {
                        if (SensedNodeIndex == 1) continue;
                        if (ContrlSensedNodeNums(2, SensedNodeIndex) < ContrlSensedNodeNums(2, SensedNodeIndex - 1)) {
                            // now see if on the same branch
                            if (ContrlSensedNodeNums(3, SensedNodeIndex) == ContrlSensedNodeNums(3, SensedNodeIndex - 1)) {
                                // we have a flow order problem with water coil controllers
                                ShowSevereError("CheckControllerListOrder: A water coil controller list has the wrong order");
                                ShowContinueError("Check the AirLoopHVAC:ControllerList for the air loop called \"" +
                                                  PrimaryAirSystem(AirSysNum).Name + "\"");
                                ShowContinueError("When there are multiple Controller:WaterCoil objects for the same air loop, they need to be "
                                                  "listed in the proper order.");
                                ShowContinueError("The controllers should be listed in natural flow order with those for upstream coils listed "
                                                  "before those for downstream coils.");
                                ShowContinueError("The sensed nodes specified for the respective controllers should also reflect this order.");
                            }
                        }
                    }
                }

                if (allocated(ContrlSensedNodeNums)) ContrlSensedNodeNums.deallocate();

            } // controllers > 1
        }
    }

    void CheckCoilWaterInletNode(EnergyPlusData &state,
                                 int const WaterInletNodeNum, // input actuator node number
                                 bool &NodeNotFound           // true if matching actuator node not found
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Heejin Cho
        //       DATE WRITTEN   November 2010
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS FUNCTION:
        // This subroutine checks that the water inlet node number is matched by
        // the actuator node number of some water coil

        // METHODOLOGY EMPLOYED:
        // na

        // REFERENCES:
        // na

        // USE STATEMENTS:

        // Locals
        // FUNCTION ARGUMENT DEFINITIONS:

        // FUNCTION PARAMETER DEFINITIONS:
        // na

        // INTERFACE BLOCK SPECIFICATIONS:
        // na

        // DERIVED TYPE DEFINITIONS:
        // na

        // FUNCTION LOCAL VARIABLE DECLARATIONS:
        int ControlNum;

        if (GetControllerInputFlag) {
            GetControllerInput(state);
            GetControllerInputFlag = false;
        }

        NodeNotFound = true;
        for (ControlNum = 1; ControlNum <= NumControllers; ++ControlNum) {
            if (ControllerProps(ControlNum).ActuatedNode == WaterInletNodeNum) {
                NodeNotFound = false;
            }
        }
    }

    void GetControllerNameAndIndex(EnergyPlusData &state,
                                   int const WaterInletNodeNum, // input actuator node number
                                   std::string &ControllerName, // controller name used by water coil
                                   int &ControllerIndex,        // controller index used by water coil
                                   bool &ErrorsFound            // true if matching actuator node not found
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Richard Raustad
        //       DATE WRITTEN   June 2017

        // PURPOSE OF THIS FUNCTION:
        // This subroutine checks that the water inlet node number is matched by
        // the actuator node number of some water coil and passed back controller name and index

        // FUNCTION LOCAL VARIABLE DECLARATIONS:
        int ControlNum;

        if (GetControllerInputFlag) {
            GetControllerInput(state);
            GetControllerInputFlag = false;
        }

        ControllerName = " ";
        ControllerIndex = 0;
        for (ControlNum = 1; ControlNum <= NumControllers; ++ControlNum) {
            if (ControllerProps(ControlNum).ActuatedNode == WaterInletNodeNum) {
                ControllerIndex = ControlNum;
                ControllerName = ControllerProps(ControlNum).ControllerName;
                break;
            }
        }

        if (ControllerIndex == 0) {
            ErrorsFound = true;
        }
    }

    void GetControllerActuatorNodeNum(EnergyPlusData &state,
                                      std::string const &ControllerName, // name of coil controller
                                      int &WaterInletNodeNum,            // input actuator node number
                                      bool &NodeNotFound                 // true if matching actuator node not found
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Richard Raustad, FSEC
        //       DATE WRITTEN   September 2013
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS FUNCTION:
        // This subroutine finds the controllers actuator node number

        // FUNCTION LOCAL VARIABLE DECLARATIONS:
        int ControlNum;

        if (GetControllerInputFlag) {
            GetControllerInput(state);
            GetControllerInputFlag = false;
        }

        NodeNotFound = true;
        ControlNum = UtilityRoutines::FindItemInList(ControllerName, ControllerProps, &ControllerPropsType::ControllerName);
        if (ControlNum > 0 && ControlNum <= NumControllers) {
            WaterInletNodeNum = ControllerProps(ControlNum).ActuatedNode;
            NodeNotFound = false;
        }
    }

    int GetControllerIndex(EnergyPlusData &state, std::string const &ControllerName // name of coil controller
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Richard Raustad, FSEC
        //       DATE WRITTEN   January 2018

        // This subroutine finds the controllers actuator node number

        if (GetControllerInputFlag) {
            GetControllerInput(state);
            GetControllerInputFlag = false;
        }

        int ControllerIndex = UtilityRoutines::FindItemInList(ControllerName, ControllerProps, &ControllerPropsType::ControllerName);
        if (ControllerIndex == 0) {
            ShowFatalError("ManageControllers: Invalid controller=" + ControllerName +
                           ". The only valid controller type for an AirLoopHVAC is Controller:WaterCoil.");
        }

        return ControllerIndex;
    }

} // namespace HVACControllers

} // namespace EnergyPlus
