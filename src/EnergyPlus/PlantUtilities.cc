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

// ObjexxFCL Headers
#include <ObjexxFCL/Array.functions.hh>
#include <ObjexxFCL/Array1D.hh>
#include <ObjexxFCL/Fmath.hh>

// EnergyPlus Headers
#include <EnergyPlus/BranchInputManager.hh>
#include <EnergyPlus/DataBranchAirLoopPlant.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/Plant/DataPlant.hh>
#include <EnergyPlus/DataPrecisionGlobals.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/FluidProperties.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/PlantUtilities.hh>
#include <EnergyPlus/UtilityRoutines.hh>

namespace EnergyPlus {

namespace PlantUtilities {

    // Module containing the routines dealing with the <module_name>

    // MODULE INFORMATION:
    //       AUTHOR         <author>
    //       DATE WRITTEN   <date_written>
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS MODULE:
    // <description>

    // METHODOLOGY EMPLOYED:
    // <description>

    // REFERENCES:
    // na

    // OTHER NOTES:
    // na

    // USE STATEMENTS:
    // <use statements for data only modules>
    // <use statements for access to subroutines in other modules>
    // Using/Aliasing
    using namespace DataPrecisionGlobals;

    namespace {
        struct CriteriaData
        {
            // Members
            int CallingCompLoopNum;        // for debug error handling
            int CallingCompLoopSideNum;    // for debug error handling
            int CallingCompBranchNum;      // for debug error handling
            int CallingCompCompNum;        // for debug error handling
            Real64 ThisCriteriaCheckValue; // the previous value, to check the current against

            // Default Constructor
            CriteriaData()
                : CallingCompLoopNum(0), CallingCompLoopSideNum(0), CallingCompBranchNum(0), CallingCompCompNum(0), ThisCriteriaCheckValue(0.0)
            {
            }
        };

        // Object Data
        Array1D<CriteriaData> CriteriaChecks; // stores criteria information
    }                                         // namespace
    // MODULE VARIABLE DECLARATIONS:
    // na

    // SUBROUTINE SPECIFICATIONS FOR MODULE <module_name>:

    // Functions
    void clear_state()
    {
        CriteriaChecks.deallocate();
    }

    void InitComponentNodes(Real64 const MinCompMdot,
                            Real64 const MaxCompMdot,
                            int const InletNode,              // component's inlet node index in node structure
                            int const OutletNode,             // component's outlet node index in node structure
                            int const EP_UNUSED(LoopNum),     // plant loop index for PlantLoop structure
                            int const EP_UNUSED(LoopSideNum), // Loop side index for PlantLoop structure
                            int const EP_UNUSED(BranchIndex), // branch index for PlantLoop
                            int const EP_UNUSED(CompIndex)    // component index for PlantLoop
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Brent Griffith
        //       DATE WRITTEN   Sept 2010
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        //  Central routine for initializing plant nodes connected to components
        //  typically used for BeginEnvrnFlag

        // METHODOLOGY EMPLOYED:
        // set MassFlowRate variables on inlet node
        //  reset inlet node if more restrictive

        // REFERENCES:
        // na

        // Using/Aliasing
        using DataLoopNode::Node;
        using DataLoopNode::NodeID;
        using DataPlant::DemandOpSchemeType;

        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:
        // SUBROUTINE PARAMETER DEFINITIONS:
        // na

        // INTERFACE BLOCK SPECIFICATIONS:
        // na

        // DERIVED TYPE DEFINITIONS:
        // na

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 tmpMinCompMdot; // local value
        Real64 tmpMaxCompMdot; // local value

        tmpMinCompMdot = MinCompMdot;
        tmpMaxCompMdot = MaxCompMdot;
        // trap bad values that can happen before all the setup is done
        if (tmpMinCompMdot < 0.0) tmpMinCompMdot = 0.0;
        if (tmpMaxCompMdot < 0.0) tmpMaxCompMdot = 0.0;

        // reset outlet node
        Node(OutletNode).MassFlowRate = 0.0;
        //  Node(OutletNode)%MassFlowRateMin      = MinCompMdot
        //  Node(OutletNode)%MassFlowRateMinAvail = MinCompMdot
        //  Node(OutletNode)%MassFlowRateMax      = MaxCompMdot
        //  Node(OutletNode)%MassFlowRateMaxAvail = MaxCompMdot

        Node(InletNode).MassFlowRateMin = tmpMinCompMdot;
        Node(InletNode).MassFlowRateMinAvail = tmpMinCompMdot;
        Node(InletNode).MassFlowRateMax = tmpMaxCompMdot;
        Node(InletNode).MassFlowRateMaxAvail = tmpMaxCompMdot;
        // reset inlet node, but only change from inlet setting if set and more restrictive
        Node(InletNode).MassFlowRate = 0.0;
        Node(InletNode).MassFlowRateRequest = 0.0;
        //  IF (Node(InletNode)%MassFlowRateMax > 0.0d0) THEN !if inlet has been set, only change it if more restrictive
        //    Node(InletNode)%MassFlowRateMax       = MIN(tmpMaxCompMdot, Node(InletNode)%MassFlowRateMax)
        //  ELSE
        //    Node(InletNode)%MassFlowRateMax       = tmpMaxCompMdot
        //  ENDIF
        //  IF (Node(InletNode)%MassFlowRateMaxAvail> 0.0d0) THEN !if inlet has been set, only change it if more restrictive
        //    Node(InletNode)%MassFlowRateMaxAvail  = MIN(tmpMaxCompMdot, Node(InletNode)%MassFlowRateMaxAvail)
        //  ELSE
        //    Node(InletNode)%MassFlowRateMaxAvail  = tmpMaxCompMdot
        //  ENDIF
        //  IF (Node(InletNode)%MassFlowRateMin > 0.0d0) THEN
        //    Node(InletNode)%MassFlowRateMin       = MAX(tmpMinCompMdot, Node(InletNode)%MassFlowRateMin)
        //  ELSE
        //    Node(InletNode)%MassFlowRateMin       = tmpMinCompMdot
        //  ENDIF
        //  IF (Node(InletNode)%MassFlowRateMinAvail > 0.0d0) THEN
        //    Node(InletNode)%MassFlowRateMinAvail  = MAX(tmpMinCompMdot, Node(InletNode)%MassFlowRateMinAvail)
        //  ELSE
        //    Node(InletNode)%MassFlowRateMinAvail  = tmpMinCompMdot
        //  ENDIF
    }

    void SetComponentFlowRate(Real64 &CompFlow,      // [kg/s]
                              int const InletNode,   // component's inlet node index in node structure
                              int const OutletNode,  // component's outlet node index in node structure
                              int const LoopNum,     // plant loop index for PlantLoop structure
                              int const LoopSideNum, // Loop side index for PlantLoop structure
                              int const BranchIndex, // branch index for PlantLoop
                              int const CompIndex    // component index for PlantLoop
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Dan Fisher
        //       DATE WRITTEN   August 2009
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // General purpose worker routine to set flows for a component model

        if (LoopNum == 0) {                 // protect from hard crash below
            if (InletNode > 0) {
                ShowSevereError("SetComponentFlowRate: trapped plant loop index = 0, check component with inlet node named=" + DataLoopNode::NodeID(InletNode));
            } else {
                ShowSevereError("SetComponentFlowRate: trapped plant loop node id = 0");
            }
            return;
            // this crashes during ManageSizing, maybe it's just an init thing...
            // ShowFatalError("Preceding loop index error causes program termination");
        }

        Real64 const MdotOldRequest = DataLoopNode::Node(InletNode).MassFlowRateRequest;
        auto &loop_side(DataPlant::PlantLoop(LoopNum).LoopSide(LoopSideNum));
        auto &comp(loop_side.Branch(BranchIndex).Comp(CompIndex));

        if (comp.CurOpSchemeType == DataPlant::DemandOpSchemeType) {
            // store flow request on inlet node
            DataLoopNode::Node(InletNode).MassFlowRateRequest = CompFlow;
            DataLoopNode::Node(OutletNode).MassFlowRateMinAvail = max(DataLoopNode::Node(InletNode).MassFlowRateMinAvail, DataLoopNode::Node(InletNode).MassFlowRateMin);
            DataLoopNode::Node(OutletNode).MassFlowRateMaxAvail = min(DataLoopNode::Node(InletNode).MassFlowRateMaxAvail, DataLoopNode::Node(InletNode).MassFlowRateMax);
            // virtual 2-way valve (was tried but it clamps down demand side component's flow options so they can't find proper solutions)
            //  Node(OutletNode)%MassFlowRateMinAvail = MAX(Node(InletNode)%MassFlowRateMinAvail , CompFlow)
            //  Node(OutletNode)%MassFlowRateMaxAvail = MIN(Node(InletNode)%MassFlowRateMaxAvail , CompFlow)
        } else {
            // DSU lodge the original request for all types
            DataLoopNode::Node(InletNode).MassFlowRateRequest = CompFlow;
        }

        // Update Min/Max Avail

        DataLoopNode::Node(OutletNode).MassFlowRateMinAvail = max(DataLoopNode::Node(InletNode).MassFlowRateMinAvail, DataLoopNode::Node(InletNode).MassFlowRateMin);
        if (DataLoopNode::Node(InletNode).MassFlowRateMax >= 0.0) {
            DataLoopNode::Node(OutletNode).MassFlowRateMaxAvail = min(DataLoopNode::Node(InletNode).MassFlowRateMaxAvail, DataLoopNode::Node(InletNode).MassFlowRateMax);
        } else {
            if (!DataGlobals::SysSizingCalc && DataPlant::PlantFirstSizesOkayToFinalize) {
                // throw error for developers, need to change a component model to set hardware limits on inlet
                if (!DataLoopNode::Node(InletNode).plantNodeErrorMsgIssued) {
                    ShowSevereError("SetComponentFlowRate: check component model implementation for component with inlet node named=" +
                                            DataLoopNode::NodeID(InletNode));
                    ShowContinueError("Inlet node MassFlowRatMax = " + General::RoundSigDigits(DataLoopNode::Node(InletNode).MassFlowRateMax, 8));
                    DataLoopNode::Node(InletNode).plantNodeErrorMsgIssued = true;
                }
            }
        }

        // Set loop flow rate
        if (loop_side.FlowLock == DataPlant::FlowUnlocked) {
            if (DataPlant::PlantLoop(LoopNum).MaxVolFlowRate == DataSizing::AutoSize) { // still haven't sized the plant loop
                DataLoopNode::Node(OutletNode).MassFlowRate = CompFlow;
                DataLoopNode::Node(InletNode).MassFlowRate = DataLoopNode::Node(OutletNode).MassFlowRate;
            } else { // bound the flow by Min/Max available and hardware limits
                if (comp.FlowCtrl == DataBranchAirLoopPlant::ControlType_SeriesActive) {
                    // determine highest flow request for all the components on the branch
                    Real64 SeriesBranchHighFlowRequest = 0.0;
                    Real64 SeriesBranchHardwareMaxLim = DataLoopNode::Node(InletNode).MassFlowRateMax;
                    Real64 SeriesBranchHardwareMinLim = 0.0;
                    Real64 SeriesBranchMaxAvail = DataLoopNode::Node(InletNode).MassFlowRateMaxAvail;
                    Real64 SeriesBranchMinAvail = 0.0;

                    // inserting EMS On/Off Supervisory control here to series branch constraint and assuming EMS should shut off flow completely
                    // action here means EMS will not impact the FlowLock == FlowLocked condition (which should still show EMS intent)
                    bool EMSLoadOverride = false;

                    for (int CompNum = 1; CompNum <= loop_side.Branch(BranchIndex).TotalComponents; ++CompNum) {
                        auto &thisComp(loop_side.Branch(BranchIndex).Comp(CompNum));
                        int const CompInletNodeNum = thisComp.NodeNumIn;
                        auto &thisInletNode(DataLoopNode::Node(CompInletNodeNum));
                        SeriesBranchHighFlowRequest = max(thisInletNode.MassFlowRateRequest, SeriesBranchHighFlowRequest);
                        SeriesBranchHardwareMaxLim = min(thisInletNode.MassFlowRateMax, SeriesBranchHardwareMaxLim);
                        SeriesBranchHardwareMinLim = max(thisInletNode.MassFlowRateMin, SeriesBranchHardwareMinLim);
                        SeriesBranchMaxAvail = min(thisInletNode.MassFlowRateMaxAvail, SeriesBranchMaxAvail);
                        SeriesBranchMinAvail = max(thisInletNode.MassFlowRateMinAvail, SeriesBranchMinAvail);
                        // check to see if any component on branch uses EMS On/Off Supervisory control to shut down flow
                        if (thisComp.EMSLoadOverrideOn && thisComp.EMSLoadOverrideValue == 0.0) EMSLoadOverride = true;
                    }

                    if (EMSLoadOverride) { // actuate EMS controlled components to 0 if On/Off Supervisory control is active off
                        SeriesBranchHardwareMaxLim = 0.0;
                    }

                    // take higher of branch max flow request and this new flow request
                    CompFlow = max(CompFlow, SeriesBranchHighFlowRequest);

                    // apply constraints on component flow
                    CompFlow = max(CompFlow, SeriesBranchHardwareMinLim);
                    CompFlow = max(CompFlow, SeriesBranchMinAvail);
                    CompFlow = min(CompFlow, SeriesBranchHardwareMaxLim);
                    CompFlow = min(CompFlow, SeriesBranchMaxAvail);

                    if (CompFlow < DataBranchAirLoopPlant::MassFlowTolerance) CompFlow = 0.0;
                    DataLoopNode::Node(OutletNode).MassFlowRate = CompFlow;
                    DataLoopNode::Node(InletNode).MassFlowRate = DataLoopNode::Node(OutletNode).MassFlowRate;
                    for (int CompNum = 1; CompNum <= loop_side.Branch(BranchIndex).TotalComponents; ++CompNum) {
                        auto &thisComp(loop_side.Branch(BranchIndex).Comp(CompNum));
                        int const CompInletNodeNum = thisComp.NodeNumIn;
                        int const CompOutletNodeNum = thisComp.NodeNumOut;
                        DataLoopNode::Node(CompInletNodeNum).MassFlowRate = DataLoopNode::Node(OutletNode).MassFlowRate;
                        DataLoopNode::Node(CompOutletNodeNum).MassFlowRate = DataLoopNode::Node(OutletNode).MassFlowRate;
                    }

                } else { // not series active
                    DataLoopNode::Node(OutletNode).MassFlowRate = max(DataLoopNode::Node(OutletNode).MassFlowRateMinAvail, CompFlow);
                    DataLoopNode::Node(OutletNode).MassFlowRate = max(DataLoopNode::Node(InletNode).MassFlowRateMin, DataLoopNode::Node(OutletNode).MassFlowRate);
                    DataLoopNode::Node(OutletNode).MassFlowRate = min(DataLoopNode::Node(OutletNode).MassFlowRateMaxAvail, DataLoopNode::Node(OutletNode).MassFlowRate);
                    DataLoopNode::Node(OutletNode).MassFlowRate = min(DataLoopNode::Node(InletNode).MassFlowRateMax, DataLoopNode::Node(OutletNode).MassFlowRate);

                    // inserting EMS On/Off Supervisory control here to override min constraint assuming EMS should shut off flow completely
                    // action here means EMS will not impact the FlowLock == FlowLocked condition (which should still show EMS intent)
                    bool EMSLoadOverride = false;

                    for (int CompNum = 1; CompNum <= loop_side.Branch(BranchIndex).TotalComponents; ++CompNum) {
                        // check to see if any component on branch uses EMS On/Off Supervisory control to shut down flow
                        auto &thisComp(loop_side.Branch(BranchIndex).Comp(CompNum));
                        if (thisComp.EMSLoadOverrideOn && thisComp.EMSLoadOverrideValue == 0.0) EMSLoadOverride = true;
                    }

                    if (EMSLoadOverride) { // actuate EMS controlled components to 0 if On/Off Supervisory control is active off
                        DataLoopNode::Node(OutletNode).MassFlowRate = 0.0;
                    }

                    if (DataLoopNode::Node(OutletNode).MassFlowRate < DataBranchAirLoopPlant::MassFlowTolerance) DataLoopNode::Node(OutletNode).MassFlowRate = 0.0;
                    CompFlow = DataLoopNode::Node(OutletNode).MassFlowRate;
                    DataLoopNode::Node(InletNode).MassFlowRate = DataLoopNode::Node(OutletNode).MassFlowRate;
                }
            }
        } else if (loop_side.FlowLock == DataPlant::FlowLocked) {
            DataLoopNode::Node(OutletNode).MassFlowRate = DataLoopNode::Node(InletNode).MassFlowRate;
            CompFlow = DataLoopNode::Node(OutletNode).MassFlowRate;
        } else {
            ShowFatalError("SetComponentFlowRate: Flow lock out of range"); // DEBUG error...should never get here LCOV_EXCL_LINE
        }

        if (comp.CurOpSchemeType == DataPlant::DemandOpSchemeType) {
            if ((MdotOldRequest > 0.0) && (CompFlow > 0.0)) { // sure that not coming back from a no flow reset
                if (std::abs(MdotOldRequest - DataLoopNode::Node(InletNode).MassFlowRateRequest) > DataBranchAirLoopPlant::MassFlowTolerance) { // demand comp changed its flow request
                    loop_side.SimLoopSideNeeded = true;
                }
            }
        }
    }

    void SetActuatedBranchFlowRate(Real64 &CompFlow,
                                   int const ActuatedNode,
                                   int const LoopNum,
                                   int const LoopSideNum,
                                   int const BranchNum,
                                   bool const ResetMode // flag to indicate if this is a real flow set, or a reset flow setting.
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         B. Griffith
        //       DATE WRITTEN   Feb 2010
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // general purpse worker routine to set plant node variables for node
        // and all nodes on the branch.  Used by HVAC water coil controller, that do not
        //  distinguish single component and have no inlet-outlet pair
        //  only a actuated noded of no clear position.  set flow on entire branch

        // METHODOLOGY EMPLOYED:
        // Set flow on node and branch while honoring constraints on actuated node

        auto &a_node(DataLoopNode::Node(ActuatedNode));
        if (LoopNum == 0 || LoopSideNum == 0) {
            // early in simulation before plant loops are setup and found
            a_node.MassFlowRate = CompFlow;
            return;
        }

        auto &loop_side(DataPlant::PlantLoop(LoopNum).LoopSide(LoopSideNum));

        // store original flow
        Real64 const MdotOldRequest = a_node.MassFlowRateRequest;
        a_node.MassFlowRateRequest = CompFlow;
        if (LoopNum > 0 && LoopSideNum > 0 && (!ResetMode)) {
            if ((MdotOldRequest > 0.0) && (CompFlow > 0.0)) { // sure that not coming back from a no flow reset
                if ((std::abs(MdotOldRequest - a_node.MassFlowRateRequest) > DataBranchAirLoopPlant::MassFlowTolerance) && (loop_side.FlowLock == DataPlant::FlowUnlocked)) {
                    loop_side.SimLoopSideNeeded = true;
                }
            }
        }
        // Set loop flow rate

        if (LoopNum > 0 && LoopSideNum > 0) {
            auto const &branch(loop_side.Branch(BranchNum));
            if (loop_side.FlowLock == DataPlant::FlowUnlocked) {
                if (DataPlant::PlantLoop(LoopNum).MaxVolFlowRate == DataSizing::AutoSize) { // still haven't sized the plant loop
                    a_node.MassFlowRate = CompFlow;
                } else { // bound the flow by Min/Max available across entire branch

                    a_node.MassFlowRate = max(a_node.MassFlowRateMinAvail, CompFlow);
                    a_node.MassFlowRate = max(a_node.MassFlowRateMin, a_node.MassFlowRate);
                    // add MassFlowRateMin hardware constraints

                    // inserting EMS On/Off Supervisory control here to override min constraint assuming EMS should shut off flow completely
                    // action here means EMS will not impact the FlowLock == FlowLocked condition (which should still show EMS intent)
                    bool EMSLoadOverride = false;
                    // check to see if any component on branch uses EMS On/Off Supervisory control to shut down flow
                    for (int CompNum = 1, CompNum_end = branch.TotalComponents; CompNum <= CompNum_end; ++CompNum) {
                        auto const &comp(branch.Comp(CompNum));
                        if (comp.EMSLoadOverrideOn && comp.EMSLoadOverrideValue == 0.0) EMSLoadOverride = true;
                    }
                    if (EMSLoadOverride) { // actuate EMS controlled components to 0 if On/Off Supervisory control is active off
                        a_node.MassFlowRate = 0.0;
                        a_node.MassFlowRateRequest = 0.0;
                    }

                    a_node.MassFlowRate = min(a_node.MassFlowRateMaxAvail, a_node.MassFlowRate);
                    a_node.MassFlowRate = min(a_node.MassFlowRateMax, a_node.MassFlowRate);
                    if (a_node.MassFlowRate < DataBranchAirLoopPlant::MassFlowTolerance) a_node.MassFlowRate = 0.0;
                    for (int CompNum = 1, CompNum_end = branch.TotalComponents; CompNum <= CompNum_end; ++CompNum) {
                        auto const &comp(branch.Comp(CompNum));
                        if (ActuatedNode == comp.NodeNumIn) {
                            //            ! found controller set to inlet of a component.  now set that component's outlet
                            int const NodeNum = comp.NodeNumOut;
                            //            Node(ActuatedNode)%MassFlowRate = MAX( Node(ActuatedNode)%MassFlowRate , Node(NodeNum)%MassFlowRateMinAvail)
                            //            Node(ActuatedNode)%MassFlowRate = MAX( Node(ActuatedNode)%MassFlowRate , Node(ActuatedNode)%MassFlowRateMin)
                            //            Node(ActuatedNode)%MassFlowRate = MIN( Node(ActuatedNode)%MassFlowRate , Node(NodeNum)%MassFlowRateMaxAvail)
                            //            Node(ActuatedNode)%MassFlowRate = MIN( Node(ActuatedNode)%MassFlowRate , Node(ActuatedNode)%MassFlowRateMax)

                            // virtual 2-way valve
                            //     Node(NodeNum)%MassFlowRateMinAvail = MAX(Node(ActuatedNode)%MassFlowRateMinAvail
                            //     ,Node(ActuatedNode)%MassFlowRateMin) Node(NodeNum)%MassFlowRateMinAvail =
                            //     MAX(Node(ActuatedNode)%MassFlowRateMinAvail , CompFlow)
                            DataLoopNode::Node(NodeNum).MassFlowRateMinAvail = max(a_node.MassFlowRateMinAvail, a_node.MassFlowRateMin);
                            //      Node(NodeNum)%MassFlowRateMaxAvail =
                            //      MIN(Node(ActuatedNode)%MassFlowRateMaxAvail,Node(ActuatedNode)%MassFlowRateMax) Node(NodeNum)%MassFlowRateMaxAvail
                            //      = MIN(Node(ActuatedNode)%MassFlowRateMaxAvail , CompFlow)
                            DataLoopNode::Node(NodeNum).MassFlowRateMaxAvail = min(a_node.MassFlowRateMaxAvail, a_node.MassFlowRateMax);
                            DataLoopNode::Node(NodeNum).MassFlowRate = a_node.MassFlowRate;
                        }
                    }
                }

            } else if (loop_side.FlowLock == DataPlant::FlowLocked) {

                CompFlow = a_node.MassFlowRate;
                // do not change requested flow rate either
                a_node.MassFlowRateRequest = MdotOldRequest;
                if ((CompFlow - a_node.MassFlowRateMaxAvail > DataBranchAirLoopPlant::MassFlowTolerance) || (a_node.MassFlowRateMinAvail - CompFlow > DataBranchAirLoopPlant::MassFlowTolerance)) {
                    ShowSevereError("SetActuatedBranchFlowRate: Flow rate is out of range"); // DEBUG error...should never get here
                    ShowContinueErrorTimeStamp("");
                    ShowContinueError("Component flow rate [kg/s] = " + General::RoundSigDigits(CompFlow, 8));
                    ShowContinueError("Node maximum flow rate available [kg/s] = " + General::RoundSigDigits(a_node.MassFlowRateMaxAvail, 8));
                    ShowContinueError("Node minimum flow rate available [kg/s] = " + General::RoundSigDigits(a_node.MassFlowRateMinAvail, 8));
                }
            } else {
                ShowFatalError("SetActuatedBranchFlowRate: Flowlock out of range, value=" +
                                       General::RoundSigDigits(loop_side.FlowLock)); // DEBUG error...should never get here LCOV_EXCL_LINE
            }

            Real64 const a_node_MasFlowRate(a_node.MassFlowRate);
            Real64 const a_node_MasFlowRateRequest(a_node.MassFlowRateRequest);
            for (int CompNum = 1, CompNum_end = branch.TotalComponents; CompNum <= CompNum_end; ++CompNum) {
                auto const &comp(branch.Comp(CompNum));
                int NodeNum = comp.NodeNumIn;
                DataLoopNode::Node(NodeNum).MassFlowRate = a_node_MasFlowRate;
                DataLoopNode::Node(NodeNum).MassFlowRateRequest = a_node_MasFlowRateRequest;
                NodeNum = comp.NodeNumOut;
                DataLoopNode::Node(NodeNum).MassFlowRate = a_node_MasFlowRate;
                DataLoopNode::Node(NodeNum).MassFlowRateRequest = a_node_MasFlowRateRequest;
            }
        }
    }

    Real64 RegulateCondenserCompFlowReqOp(
        int const LoopNum, int const LoopSideNum, int const BranchNum, int const CompNum, Real64 const TentativeFlowRequest)
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         Edwin Lee
        //       DATE WRITTEN   April 2012
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS FUNCTION:
        // This function will do some intelligent flow request logic for condenser equipment.
        // Some condenser equipment (ground heat exchangers, etc.) may not have a meaningful load value
        //  since this is an environment heat transfer component.
        // The runflag is set, but may not be properly set, and the component may still request flow even
        //  when it doesn't need to.
        // This function will do a little more advanced logic than just checking runflag to determine whether
        //  to request any flow

        // METHODOLOGY EMPLOYED:
        // Query run flag and MyLoad
        // If run flag is OFF, then the component should actually be OFF, and tentative flow request will be zeroed
        // If the run flag is ON, then check the control type to determine if MyLoad is a meaningful value
        // If it is meaningful then determine whether to do flow request based on MyLoad
        // If not then we will have no choice but to leave the flow request alone (uncontrolled operation?)

        // Using/Aliasing
        using DataPlant::CompSetPtBasedSchemeType;
        using DataPlant::CoolingRBOpSchemeType;
        using DataPlant::HeatingRBOpSchemeType;
        using DataPlant::PlantLoop;

        // Return value
        Real64 FlowVal;

        // FUNCTION PARAMETER DEFINITIONS:
        Real64 const ZeroLoad(0.0001);

        // FUNCTION LOCAL VARIABLE DECLARATIONS:
        Real64 CompCurLoad;
        bool CompRunFlag;
        int CompOpScheme;

        CompCurLoad = PlantLoop(LoopNum).LoopSide(LoopSideNum).Branch(BranchNum).Comp(CompNum).MyLoad;
        CompRunFlag = PlantLoop(LoopNum).LoopSide(LoopSideNum).Branch(BranchNum).Comp(CompNum).ON;
        CompOpScheme = PlantLoop(LoopNum).LoopSide(LoopSideNum).Branch(BranchNum).Comp(CompNum).CurOpSchemeType;

        if (CompRunFlag) {

            {
                auto const SELECT_CASE_var(CompOpScheme);

                if ((SELECT_CASE_var == HeatingRBOpSchemeType) || (SELECT_CASE_var == CoolingRBOpSchemeType) ||
                    (SELECT_CASE_var == CompSetPtBasedSchemeType)) { // These provide meaningful MyLoad values
                    if (std::abs(CompCurLoad) > ZeroLoad) {
                        FlowVal = TentativeFlowRequest;
                    } else { // no load
                        FlowVal = 0.0;
                    }

                } else { // Types that don't provide meaningful MyLoad values
                    FlowVal = TentativeFlowRequest;
                }
            }

        } else { // runflag OFF

            FlowVal = 0.0;
        }

        return FlowVal;
    }

    bool AnyPlantSplitterMixerLacksContinuity()
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         B. Griffith
        //       DATE WRITTEN   April 2012
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS FUNCTION:
        // Similar to CheckPlantMixerSplitterConsistency, but used to decide if plant needs to iterate again
        for (int LoopNum = 1; LoopNum <= DataPlant::TotNumLoops; ++LoopNum) {
            for (int LoopSide = DataPlant::DemandSide; LoopSide <= DataPlant::SupplySide; ++LoopSide) {
                if (DataPlant::PlantLoop(LoopNum).LoopSide(LoopSide).Splitter.Exists) {
                    int const SplitterInletNode = DataPlant::PlantLoop(LoopNum).LoopSide(LoopSide).Splitter.NodeNumIn;
                    // loop across branch outlet nodes and check mass continuity
                    int const NumSplitterOutlets = DataPlant::PlantLoop(LoopNum).LoopSide(LoopSide).Splitter.TotalOutletNodes;
                    Real64 SumOutletFlow = 0.0;
                    for (int OutletNum = 1; OutletNum <= NumSplitterOutlets; ++OutletNum) {
                        int const BranchNum = DataPlant::PlantLoop(LoopNum).LoopSide(LoopSide).Splitter.BranchNumOut(OutletNum);
                        int const LastNodeOnBranch = DataPlant::PlantLoop(LoopNum).LoopSide(LoopSide).Branch(BranchNum).NodeNumOut;
                        SumOutletFlow += DataLoopNode::Node(LastNodeOnBranch).MassFlowRate;
                    }
                    Real64 const AbsDifference = std::abs(DataLoopNode::Node(SplitterInletNode).MassFlowRate - SumOutletFlow);
                    if (AbsDifference > DataPlant::CriteriaDelta_MassFlowRate) {
                        return true;
                    }
                }
            }
        }
        return false;
    }

    void
    CheckPlantMixerSplitterConsistency(int const LoopNum, int const LoopSideNum, bool const FirstHVACIteration)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Brent Griffith
        //       DATE WRITTEN   Oct 2007
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Check for plant flow resolver errors

        // METHODOLOGY EMPLOYED:
        // compare flow rate of splitter inlet to flow rate of mixer outlet

        // Using/Aliasing
        using DataBranchAirLoopPlant::MassFlowTolerance;
        using DataGlobals::DoingSizing;
        using DataGlobals::WarmupFlag;
        using DataLoopNode::Node;
        using DataPlant::CriteriaDelta_MassFlowRate;
        using DataPlant::DemandSide;
        using DataPlant::PlantLoop;
        using DataPlant::SupplySide;
        using General::RoundSigDigits;

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int MixerOutletNode;
        int SplitterInletNode;
        Real64 AbsDifference;
        int NumSplitterOutlets;
        Real64 SumOutletFlow;
        int OutletNum;
        int BranchNum;
        int LastNodeOnBranch;

        if (!PlantLoop(LoopNum).LoopHasConnectionComp) {
            if (!DoingSizing && !WarmupFlag && PlantLoop(LoopNum).LoopSide(LoopSideNum).Mixer.Exists && !FirstHVACIteration) {
                // Find mixer outlet node number
                MixerOutletNode = PlantLoop(LoopNum).LoopSide(LoopSideNum).Mixer.NodeNumOut;
                // Find splitter inlet node number
                SplitterInletNode = PlantLoop(LoopNum).LoopSide(LoopSideNum).Splitter.NodeNumIn;

                AbsDifference = std::abs(Node(SplitterInletNode).MassFlowRate - Node(MixerOutletNode).MassFlowRate);
                if (AbsDifference > MassFlowTolerance) {
                    if (PlantLoop(LoopNum).MFErrIndex1 == 0) {
                        ShowSevereMessage("Plant flows do not resolve -- splitter inlet flow does not match mixer outlet flow ");
                        ShowContinueErrorTimeStamp("");
                        ShowContinueError("PlantLoop name= " + PlantLoop(LoopNum).Name);
                        ShowContinueError("Plant Connector:Mixer name= " + PlantLoop(LoopNum).LoopSide(LoopSideNum).Mixer.Name);
                        ShowContinueError("Mixer outlet mass flow rate= " + RoundSigDigits(Node(MixerOutletNode).MassFlowRate, 6) + " {kg/s}");
                        ShowContinueError("Plant Connector:Splitter name= " + PlantLoop(LoopNum).LoopSide(LoopSideNum).Splitter.Name);
                        ShowContinueError("Splitter inlet mass flow rate= " + RoundSigDigits(Node(SplitterInletNode).MassFlowRate, 6) + " {kg/s}");
                        ShowContinueError("Difference in two mass flow rates= " + RoundSigDigits(AbsDifference, 6) + " {kg/s}");
                    }
                    ShowRecurringSevereErrorAtEnd("Plant Flows (Loop=" + PlantLoop(LoopNum).Name +
                                                      ") splitter inlet flow not match mixer outlet flow",
                                                  PlantLoop(LoopNum).MFErrIndex1,
                                                  AbsDifference,
                                                  AbsDifference,
                                                  _,
                                                  "kg/s",
                                                  "kg/s");
                    if (AbsDifference > MassFlowTolerance * 10.0) {
                        ShowSevereError("Plant flows do not resolve -- splitter inlet flow does not match mixer outlet flow ");
                        ShowContinueErrorTimeStamp("");
                        ShowContinueError("PlantLoop name= " + PlantLoop(LoopNum).Name);
                        ShowContinueError("Plant Connector:Mixer name= " + PlantLoop(LoopNum).LoopSide(LoopSideNum).Mixer.Name);
                        ShowContinueError("Mixer outlet mass flow rate= " + RoundSigDigits(Node(MixerOutletNode).MassFlowRate, 6) + " {kg/s}");
                        ShowContinueError("Plant Connector:Splitter name= " + PlantLoop(LoopNum).LoopSide(LoopSideNum).Splitter.Name);
                        ShowContinueError("Splitter inlet mass flow rate= " + RoundSigDigits(Node(SplitterInletNode).MassFlowRate, 6) + " {kg/s}");
                        ShowContinueError("Difference in two mass flow rates= " + RoundSigDigits(AbsDifference, 6) + " {kg/s}");
                        ShowFatalError("CheckPlantMixerSplitterConsistency: Simulation terminated because of problems in plant flow resolver");
                    }
                }

                // now check inside s/m to see if there are problems

                // loop across branch outlet nodes and check mass continuity
                NumSplitterOutlets = PlantLoop(LoopNum).LoopSide(LoopSideNum).Splitter.TotalOutletNodes;
                SumOutletFlow = 0.0;
                //  SumInletFlow = 0.0;
                for (OutletNum = 1; OutletNum <= NumSplitterOutlets; ++OutletNum) {
                    BranchNum = PlantLoop(LoopNum).LoopSide(LoopSideNum).Splitter.BranchNumOut(OutletNum);
                    LastNodeOnBranch = PlantLoop(LoopNum).LoopSide(LoopSideNum).Branch(BranchNum).NodeNumOut;
                    SumOutletFlow += Node(LastNodeOnBranch).MassFlowRate;
                    //  FirstNodeOnBranch= PlantLoop(LoopNum)%LoopSide(LoopSideNum)%Branch(BranchNum)%NodeNumIn
                    //  SumInletFlow = SumInletFlow + Node(FirstNodeOnBranch)%MassFlowRate
                }
                AbsDifference = std::abs(Node(SplitterInletNode).MassFlowRate - SumOutletFlow);
                if (AbsDifference > CriteriaDelta_MassFlowRate) {
                    if (PlantLoop(LoopNum).MFErrIndex2 == 0) {
                        ShowSevereMessage("Plant flows do not resolve -- splitter inlet flow does not match branch outlet flows");
                        ShowContinueErrorTimeStamp("");
                        ShowContinueError("PlantLoop name= " + PlantLoop(LoopNum).Name);
                        ShowContinueError("Plant Connector:Mixer name= " + PlantLoop(LoopNum).LoopSide(LoopSideNum).Mixer.Name);
                        ShowContinueError("Sum of Branch outlet mass flow rates= " + RoundSigDigits(SumOutletFlow, 6) + " {kg/s}");
                        ShowContinueError("Plant Connector:Splitter name= " + PlantLoop(LoopNum).LoopSide(LoopSideNum).Splitter.Name);
                        ShowContinueError("Splitter inlet mass flow rate= " + RoundSigDigits(Node(SplitterInletNode).MassFlowRate, 6) + " {kg/s}");
                        ShowContinueError("Difference in two mass flow rates= " + RoundSigDigits(AbsDifference, 6) + " {kg/s}");
                    }
                    ShowRecurringSevereErrorAtEnd("Plant Flows (Loop=" + PlantLoop(LoopNum).Name +
                                                      ") splitter inlet flow does not match branch outlet flows",
                                                  PlantLoop(LoopNum).MFErrIndex2,
                                                  AbsDifference,
                                                  AbsDifference,
                                                  _,
                                                  "kg/s",
                                                  "kg/s");

                }
            }
        }
    }

    void CheckForRunawayPlantTemps(int const LoopNum, int const LoopSideNum)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Brent Griffith
        //       DATE WRITTEN   Sept 2010
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Check for plant control errors revealed as run away fluid temps
        //  halt program so it won't siliently run in out of control state

        // METHODOLOGY EMPLOYED:
        //  compare plant temps to plant min and max and halt if things run away
        //  sensitivity can be adjusted with parameters, picked somewhat arbitrary

        // REFERENCES:
        // na

        // Using/Aliasing
        using DataGlobals::DoingSizing;
        using DataGlobals::WarmupFlag;
        using DataLoopNode::Node;
        using DataLoopNode::NodeID;
        using DataPlant::DemandSide;
        using DataPlant::PlantLoop;
        using DataPlant::SupplySide;
        using General::RoundSigDigits;

        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:

        // SUBROUTINE PARAMETER DEFINITIONS:
        Real64 const OverShootOffset(5.0);
        Real64 const UnderShootOffset(5.0);
        Real64 const FatalOverShootOffset(200.0);
        Real64 const FatalUnderShootOffset(100.0);
        // INTERFACE BLOCK SPECIFICATIONS:
        // na

        // DERIVED TYPE DEFINITIONS:
        // na

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        std::string hotcold;
        bool makefatalerror;
        std::string DemandSupply;
        int LSN;
        int BrN;
        int CpN;
        Real64 LoopCapacity;
        Real64 LoopDemandSideCapacity;
        Real64 LoopSupplySideCapacity;
        Real64 DispatchedCapacity;
        Real64 LoopDemandSideDispatchedCapacity;
        Real64 LoopSupplySideDispatchedCapacity;

        makefatalerror = false;
        if (Node(PlantLoop(LoopNum).LoopSide(LoopSideNum).NodeNumOut).Temp > (PlantLoop(LoopNum).MaxTemp + OverShootOffset)) {

            // first stage, throw recurring warning that plant loop is getting out of control
            ShowRecurringWarningErrorAtEnd("Plant loop exceeding upper temperature limit, PlantLoop=\"" + PlantLoop(LoopNum).Name + "\"",
                                           PlantLoop(LoopNum).MaxTempErrIndex,
                                           Node(PlantLoop(LoopNum).LoopSide(LoopSideNum).NodeNumOut).Temp);

            if (Node(PlantLoop(LoopNum).LoopSide(LoopSideNum).NodeNumOut).Temp > (PlantLoop(LoopNum).MaxTemp + FatalOverShootOffset)) {
                hotcold = "hot";
                makefatalerror = true;
            }
        }

        if (Node(PlantLoop(LoopNum).LoopSide(LoopSideNum).NodeNumOut).Temp < (PlantLoop(LoopNum).MinTemp - UnderShootOffset)) {

            // first stage, throw recurring warning that plant loop is getting out of control
            ShowRecurringWarningErrorAtEnd("Plant loop falling below lower temperature limit, PlantLoop=\"" + PlantLoop(LoopNum).Name + "\"",
                                           PlantLoop(LoopNum).MinTempErrIndex,
                                           _,
                                           Node(PlantLoop(LoopNum).LoopSide(LoopSideNum).NodeNumOut).Temp);

            if (Node(PlantLoop(LoopNum).LoopSide(LoopSideNum).NodeNumOut).Temp < (PlantLoop(LoopNum).MinTemp - FatalUnderShootOffset)) {
                hotcold = "cold";
                makefatalerror = true;
            }
        }

        if (makefatalerror) {
            ShowSevereError("Plant temperatures are getting far too " + hotcold + ", check controls and relative loads and capacities");
            ShowContinueErrorTimeStamp("");
            if (LoopSideNum == DemandSide) {
                DemandSupply = "Demand";
            } else if (LoopSideNum == SupplySide) {
                DemandSupply = "Supply";
            } else {
                DemandSupply = "Unknown";
            }
            ShowContinueError("PlantLoop Name (" + DemandSupply + "Side)= " + PlantLoop(LoopNum).Name);
            ShowContinueError("PlantLoop Setpoint Temperature=" + RoundSigDigits(Node(PlantLoop(LoopNum).TempSetPointNodeNum).TempSetPoint, 1) +
                              " {C}");
            if (PlantLoop(LoopNum).LoopSide(SupplySide).InletNodeSetPt) {
                ShowContinueError("PlantLoop Inlet Node (SupplySide) has a Setpoint.");
            } else {
                ShowContinueError("PlantLoop Inlet Node (SupplySide) does not have a Setpoint.");
            }
            if (PlantLoop(LoopNum).LoopSide(DemandSide).InletNodeSetPt) {
                ShowContinueError("PlantLoop Inlet Node (DemandSide) has a Setpoint.");
            } else {
                ShowContinueError("PlantLoop Inlet Node (DemandSide) does not have a Setpoint.");
            }
            if (PlantLoop(LoopNum).LoopSide(SupplySide).OutletNodeSetPt) {
                ShowContinueError("PlantLoop Outlet Node (SupplySide) has a Setpoint.");
            } else {
                ShowContinueError("PlantLoop Outlet Node (SupplySide) does not have a Setpoint.");
            }
            if (PlantLoop(LoopNum).LoopSide(DemandSide).OutletNodeSetPt) {
                ShowContinueError("PlantLoop Outlet Node (DemandSide) has a Setpoint.");
            } else {
                ShowContinueError("PlantLoop Outlet Node (DemandSide) does not have a Setpoint.");
            }
            ShowContinueError("PlantLoop Outlet Node (" + DemandSupply + "Side) \"" + NodeID(PlantLoop(LoopNum).LoopSide(LoopSideNum).NodeNumOut) +
                              "\" has temperature=" + RoundSigDigits(Node(PlantLoop(LoopNum).LoopSide(LoopSideNum).NodeNumOut).Temp, 1) + " {C}");
            ShowContinueError("PlantLoop Inlet Node (" + DemandSupply + "Side) \"" + NodeID(PlantLoop(LoopNum).LoopSide(LoopSideNum).NodeNumIn) +
                              "\" has temperature=" + RoundSigDigits(Node(PlantLoop(LoopNum).LoopSide(LoopSideNum).NodeNumIn).Temp, 1) + " {C}");
            ShowContinueError("PlantLoop Minimum Temperature=" + RoundSigDigits(PlantLoop(LoopNum).MinTemp, 1) + " {C}");
            ShowContinueError("PlantLoop Maximum Temperature=" + RoundSigDigits(PlantLoop(LoopNum).MaxTemp, 1) + " {C}");
            ShowContinueError("PlantLoop Flow Request (SupplySide)=" + RoundSigDigits(PlantLoop(LoopNum).LoopSide(SupplySide).FlowRequest, 1) +
                              " {kg/s}");
            ShowContinueError("PlantLoop Flow Request (DemandSide)=" + RoundSigDigits(PlantLoop(LoopNum).LoopSide(DemandSide).FlowRequest, 1) +
                              " {kg/s}");
            ShowContinueError("PlantLoop Node (" + DemandSupply + "Side) \"" + NodeID(PlantLoop(LoopNum).LoopSide(LoopSideNum).NodeNumOut) +
                              "\" has mass flow rate =" + RoundSigDigits(Node(PlantLoop(LoopNum).LoopSide(LoopSideNum).NodeNumOut).MassFlowRate, 1) +
                              " {kg/s}");
            ShowContinueError("PlantLoop PumpHeat (SupplySide)=" + RoundSigDigits(PlantLoop(LoopNum).LoopSide(SupplySide).TotalPumpHeat, 1) + " {W}");
            ShowContinueError("PlantLoop PumpHeat (DemandSide)=" + RoundSigDigits(PlantLoop(LoopNum).LoopSide(DemandSide).TotalPumpHeat, 1) + " {W}");
            ShowContinueError("PlantLoop Cooling Demand=" + RoundSigDigits(PlantLoop(LoopNum).CoolingDemand, 1) + " {W}");
            ShowContinueError("PlantLoop Heating Demand=" + RoundSigDigits(PlantLoop(LoopNum).HeatingDemand, 1) + " {W}");
            ShowContinueError("PlantLoop Demand not Dispatched=" + RoundSigDigits(PlantLoop(LoopNum).DemandNotDispatched, 1) + " {W}");
            ShowContinueError("PlantLoop Unmet Demand=" + RoundSigDigits(PlantLoop(LoopNum).UnmetDemand, 1) + " {W}");

            LoopCapacity = 0.0;
            DispatchedCapacity = 0.0;
            for (LSN = DemandSide; LSN <= SupplySide; ++LSN) {
                for (BrN = 1; BrN <= PlantLoop(LoopNum).LoopSide(LSN).TotalBranches; ++BrN) {
                    for (CpN = 1; CpN <= PlantLoop(LoopNum).LoopSide(LSN).Branch(BrN).TotalComponents; ++CpN) {
                        LoopCapacity += PlantLoop(LoopNum).LoopSide(LSN).Branch(BrN).Comp(CpN).MaxLoad;
                        DispatchedCapacity += std::abs(PlantLoop(LoopNum).LoopSide(LSN).Branch(BrN).Comp(CpN).MyLoad);
                    }
                }
                if (LSN == DemandSide) {
                    LoopDemandSideCapacity = LoopCapacity;
                    LoopDemandSideDispatchedCapacity = DispatchedCapacity;
                } else {
                    LoopSupplySideCapacity = LoopCapacity - LoopDemandSideCapacity;
                    LoopSupplySideDispatchedCapacity = DispatchedCapacity - LoopDemandSideDispatchedCapacity;
                }
            }
            ShowContinueError("PlantLoop Capacity=" + RoundSigDigits(LoopCapacity, 1) + " {W}");
            ShowContinueError("PlantLoop Capacity (SupplySide)=" + RoundSigDigits(LoopSupplySideCapacity, 1) + " {W}");
            ShowContinueError("PlantLoop Capacity (DemandSide)=" + RoundSigDigits(LoopDemandSideCapacity, 1) + " {W}");
            ShowContinueError("PlantLoop Operation Scheme=" + PlantLoop(LoopNum).OperationScheme);
            ShowContinueError("PlantLoop Operation Dispatched Load = " + RoundSigDigits(DispatchedCapacity, 1) + " {W}");
            ShowContinueError("PlantLoop Operation Dispatched Load (SupplySide)= " + RoundSigDigits(LoopSupplySideDispatchedCapacity, 1) + " {W}");
            ShowContinueError("PlantLoop Operation Dispatched Load (DemandSide)= " + RoundSigDigits(LoopDemandSideDispatchedCapacity, 1) + " {W}");
            ShowContinueError("Branches on the Loop.");
            ShowBranchesOnLoop(LoopNum);
            ShowContinueError("*************************");
            ShowContinueError("Possible things to look for to correct this problem are:");
            ShowContinueError("  Capacity, Operation Scheme, Mass flow problems, Pump Heat building up over time.");
            ShowContinueError("  Try a shorter runperiod to stop before it fatals and look at");
            ShowContinueError("    lots of node time series data to see what is going wrong.");
            ShowContinueError("  If this is happening during Warmup, you can use Output:Diagnostics,ReportDuringWarmup;");
            ShowContinueError("  This is detected at the loop level, but the typical problems are in the components.");
            ShowFatalError("CheckForRunawayPlantTemps: Simulation terminated because of run away plant temperatures, too " + hotcold);
        }
    }

    void SetAllFlowLocks(int const Value)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Edwin Lee
        //       DATE WRITTEN   November 2009
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine will set both LoopSide flowlocks on all plant loops to the input value (0 or 1)
        // Initially this routine is used as a quick replacement for the FlowLock=0 and FlowLock=1 statements
        //  in order to provide the same behavior through phase I of the demand side rewrite
        // Eventually this routine may be employed again to quickly initialize all loops once phase III is complete
        for (int LoopNum = 1; LoopNum <= DataPlant::TotNumLoops; ++LoopNum) {
            for (int LoopSideNum = 1; LoopSideNum <= isize(DataPlant::PlantLoop(LoopNum).LoopSide); ++LoopSideNum) {
                DataPlant::PlantLoop(LoopNum).LoopSide(LoopSideNum).FlowLock = Value;
            }
        }
    }

    void ResetAllPlantInterConnectFlags()
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Edwin Lee
        //       DATE WRITTEN   September 2010
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine will reset all interconnected (air, zone, etc.) sim flags for both loopsides of all loops

        for (int LoopNum = 1; LoopNum <= DataPlant::TotNumLoops; ++LoopNum) {
            for (auto &e : DataPlant::PlantLoop(LoopNum).LoopSide) {
                e.SimAirLoopsNeeded = false;
                e.SimZoneEquipNeeded = false;
                e.SimNonZoneEquipNeeded = false;
                e.SimElectLoadCentrNeeded = false;
            }
        }
    }

    void PullCompInterconnectTrigger(int const LoopNum,             // component's loop index
                                     int const LoopSide,            // component's loop side number
                                     int const BranchNum,           // Component's branch number
                                     int const CompNum,             // Component's comp number
                                     int &UniqueCriteriaCheckIndex, // An integer given to this particular check
                                     int const ConnectedLoopNum,    // Component's interconnected loop number
                                     int const ConnectedLoopSide,   // Component's interconnected loop side number
                                     int const CriteriaType,        // The criteria check to use, see DataPlant: SimFlagCriteriaTypes
                                     Real64 const CriteriaValue     // The value of the criteria check to evaluate
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Edwin Lee
        //       DATE WRITTEN   September 2010
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Provides a generic means for components to trigger interconnected loop sides sim flags

        // METHODOLOGY EMPLOYED:
        // Determine convergence criteria based on *CriteriaType* variable.  This routine only turns
        //  the loop side sim flag ON, it doesn't turn it OFF.
        // The convergence value history was originally going to be put at the Branch()%Comp()%...
        //  level, but this would be quite difficult if we had multiple convergence checks for the
        //  same component, such as if a chiller was trying to turn on the condenser side and the
        //  heat recovery side.
        // It was determined to use a local array, which is only reallocated during the first stages
        //  of the simulation when components are first calling their sim flag requests.  After that
        //  the INOUT index variable will be used to avoid reallocation and string compares.
        // Error handling will be put in to ensure unique identifiers are used for debugging purposes.
        // A single component may have multiple check indeces, but a single index will only have one
        //  associated component.  Therefore whenever we come in with a non-zero index, we will just
        //  verify that the stored loop/side/branch/comp matches

        // Using/Aliasing
        using DataPlant::CriteriaDelta_HeatTransferRate;
        using DataPlant::CriteriaDelta_MassFlowRate;
        using DataPlant::CriteriaDelta_Temperature;
        using DataPlant::CriteriaType_HeatTransferRate;
        using DataPlant::CriteriaType_MassFlowRate;
        using DataPlant::CriteriaType_Temperature;
        using DataPlant::PlantLoop;

        CriteriaData CurCriteria; // for convenience

        if (UniqueCriteriaCheckIndex <= 0) { // If we don't yet have an index, we need to initialize

            // We need to start by allocating, or REallocating the array
            int const CurrentNumChecksStored(CriteriaChecks.size() + 1);
            CriteriaChecks.redimension(CurrentNumChecksStored);

            // Store the unique name and location
            CriteriaChecks(CurrentNumChecksStored).CallingCompLoopNum = LoopNum;
            CriteriaChecks(CurrentNumChecksStored).CallingCompLoopSideNum = LoopSide;
            CriteriaChecks(CurrentNumChecksStored).CallingCompBranchNum = BranchNum;
            CriteriaChecks(CurrentNumChecksStored).CallingCompCompNum = CompNum;

            // Since this was the first pass, it is safe to assume something has changed!
            // Therefore we'll set the sim flag to true
            PlantLoop(ConnectedLoopNum).LoopSide(ConnectedLoopSide).SimLoopSideNeeded = true;

            // Make sure we return the proper value of index
            UniqueCriteriaCheckIndex = CurrentNumChecksStored;

        } else { // We already have an index

            // If we have an index, we need to do a brief error handling, then determine
            //  sim flag status based on the criteria type

            // First store the current check in a single variable instead of array for readability
            CurCriteria = CriteriaChecks(UniqueCriteriaCheckIndex);

            // Check to make sure we didn't reuse the index in multiple components
            if (CurCriteria.CallingCompLoopNum != LoopNum || CurCriteria.CallingCompLoopSideNum != LoopSide ||
                CurCriteria.CallingCompBranchNum != BranchNum || CurCriteria.CallingCompCompNum != CompNum) {
                // Diagnostic fatal: component does not properly utilize unique indexing
            }

            // Initialize, then check if we are out of range
            {
                auto const SELECT_CASE_var(CriteriaType);
                if (SELECT_CASE_var == CriteriaType_MassFlowRate) {
                    if (std::abs(CurCriteria.ThisCriteriaCheckValue - CriteriaValue) > CriteriaDelta_MassFlowRate) {
                        PlantLoop(ConnectedLoopNum).LoopSide(ConnectedLoopSide).SimLoopSideNeeded = true;
                    }

                } else if (SELECT_CASE_var == CriteriaType_Temperature) {
                    if (std::abs(CurCriteria.ThisCriteriaCheckValue - CriteriaValue) > CriteriaDelta_Temperature) {
                        PlantLoop(ConnectedLoopNum).LoopSide(ConnectedLoopSide).SimLoopSideNeeded = true;
                    }

                } else if (SELECT_CASE_var == CriteriaType_HeatTransferRate) {
                    if (std::abs(CurCriteria.ThisCriteriaCheckValue - CriteriaValue) > CriteriaDelta_HeatTransferRate) {
                        PlantLoop(ConnectedLoopNum).LoopSide(ConnectedLoopSide).SimLoopSideNeeded = true;
                    }

                } else {
                    // Diagnostic fatal: improper criteria type
                }
            }

        } // if we have an index or not

        // Store the value for the next pass
        CriteriaChecks(UniqueCriteriaCheckIndex).ThisCriteriaCheckValue = CriteriaValue;
    }

    void UpdateChillerComponentCondenserSide(EnergyPlusData &state,
                                             int const LoopNum,                   // component's loop index
                                             int const LoopSide,                  // component's loop side number
                                             int const EP_UNUSED(TypeOfNum),      // Component's type index
                                             int const InletNodeNum,              // Component's inlet node pointer
                                             int const OutletNodeNum,             // Component's outlet node pointer
                                             Real64 const ModelCondenserHeatRate, // model's heat rejection rate at condenser (W)
                                             Real64 const ModelInletTemp,         // model's inlet temperature (C)
                                             Real64 const ModelOutletTemp,        // model's outlet temperature (C)
                                             Real64 const ModelMassFlowRate,      // model's condenser water mass flow rate (kg/s)
                                             bool const FirstHVACIteration)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Brent Griffith
        //       DATE WRITTEN   February 2010
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // provides reusable update routine for water cooled chiller's condenser water
        // connection to plant loops

        // METHODOLOGY EMPLOYED:
        // check if anything changed or doesn't agree and set simulation flags.
        // update outlet conditions if needed or possible

        // Using/Aliasing
        using DataBranchAirLoopPlant::MassFlowTolerance;
        using DataLoopNode::Node;
        using DataPlant::PlantLoop;
        using FluidProperties::GetSpecificHeatGlycol;

        // SUBROUTINE PARAMETER DEFINITIONS:
        static std::string const RoutineName("UpdateChillerComponentCondenserSide");

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        bool DidAnythingChange(false); // set to true if conditions changed
        int OtherLoopNum;                     // local loop pointer for remote connected loop
        int OtherLoopSide;                    // local loop side pointer for remote connected loop
        int ConnectLoopNum;                   // local do loop counter
        Real64 Cp;

        // check if any conditions have changed
        if (Node(InletNodeNum).MassFlowRate != ModelMassFlowRate) DidAnythingChange = true;

        if (Node(OutletNodeNum).MassFlowRate != ModelMassFlowRate) DidAnythingChange = true;

        if (Node(InletNodeNum).Temp != ModelInletTemp) DidAnythingChange = true;

        if (Node(OutletNodeNum).Temp != ModelOutletTemp) DidAnythingChange = true;

        // could also check heat rate agains McDeltaT from node data

        if ((Node(InletNodeNum).MassFlowRate == 0.0) && (ModelCondenserHeatRate > 0.0)) {

            // DSU3 TODO also send a request that condenser loop be made available, interlock message infrastructure??

            DidAnythingChange = true;
        }

        if (DidAnythingChange || FirstHVACIteration) {
            // use current mass flow rate and inlet temp from Node and recalculate outlet temp
            if (Node(InletNodeNum).MassFlowRate > MassFlowTolerance) {
                // update node outlet conditions
                Cp = GetSpecificHeatGlycol(state, PlantLoop(LoopNum).FluidName, ModelInletTemp, PlantLoop(LoopNum).FluidIndex, RoutineName);
                Node(OutletNodeNum).Temp = Node(InletNodeNum).Temp + ModelCondenserHeatRate / (Node(InletNodeNum).MassFlowRate * Cp);
            }

            // set sim flag for this loop
            PlantLoop(LoopNum).LoopSide(LoopSide).SimLoopSideNeeded = true;

            // set sim flag on connected loops to true because this side changed
            if (PlantLoop(LoopNum).LoopSide(LoopSide).TotalConnected > 0) {
                for (ConnectLoopNum = 1; ConnectLoopNum <= PlantLoop(LoopNum).LoopSide(LoopSide).TotalConnected; ++ConnectLoopNum) {
                    if (PlantLoop(LoopNum).LoopSide(LoopSide).Connected(ConnectLoopNum).LoopDemandsOnRemote) {
                        OtherLoopNum = PlantLoop(LoopNum).LoopSide(LoopSide).Connected(ConnectLoopNum).LoopNum;
                        OtherLoopSide = PlantLoop(LoopNum).LoopSide(LoopSide).Connected(ConnectLoopNum).LoopSideNum;
                        PlantLoop(OtherLoopNum).LoopSide(OtherLoopSide).SimLoopSideNeeded = true;
                    }
                }
            }

        } else { // nothing changed so turn off sim flag
            PlantLoop(LoopNum).LoopSide(LoopSide).SimLoopSideNeeded = false;
        }
    }

    void UpdateComponentHeatRecoverySide(EnergyPlusData &state,
                                         int const LoopNum,                  // component's loop index
                                         int const LoopSide,                 // component's loop side number
                                         int const EP_UNUSED(TypeOfNum),     // Component's type index
                                         int const InletNodeNum,             // Component's inlet node pointer
                                         int const OutletNodeNum,            // Component's outlet node pointer
                                         Real64 const ModelRecoveryHeatRate, // model's heat rejection rate at recovery (W)
                                         Real64 const ModelInletTemp,        // model's inlet temperature (C)
                                         Real64 const ModelOutletTemp,       // model's outlet temperature (C)
                                         Real64 const ModelMassFlowRate,     // model's condenser water mass flow rate (kg/s)
                                         bool const FirstHVACIteration)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Brent Griffith
        //       DATE WRITTEN   Sept 2010
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // provides reusable update routine for heat recovery type
        // connection to plant loops

        // METHODOLOGY EMPLOYED:
        // check if anything changed or doesn't agree and set simulation flags.
        // update outlet conditions if needed or possible

        // Using/Aliasing
        using DataBranchAirLoopPlant::MassFlowTolerance;
        using DataLoopNode::Node;
        using DataPlant::PlantLoop;
        using FluidProperties::GetSpecificHeatGlycol;

        // SUBROUTINE PARAMETER DEFINITIONS:
        static std::string const RoutineName("UpdateComponentHeatRecoverySide");

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        bool DidAnythingChange(false); // set to true if conditions changed
        int OtherLoopNum;                     // local loop pointer for remote connected loop
        int OtherLoopSide;                    // local loop side pointer for remote connected loop
        int ConnectLoopNum;                   // local do loop counter
        Real64 Cp;                            // local fluid specific heat

        // check if any conditions have changed
        if (Node(InletNodeNum).MassFlowRate != ModelMassFlowRate) DidAnythingChange = true;

        if (Node(OutletNodeNum).MassFlowRate != ModelMassFlowRate) DidAnythingChange = true;

        if (Node(InletNodeNum).Temp != ModelInletTemp) DidAnythingChange = true;

        if (Node(OutletNodeNum).Temp != ModelOutletTemp) DidAnythingChange = true;

        // could also check heat rate against McDeltaT from node data

        if ((Node(InletNodeNum).MassFlowRate == 0.0) && (ModelRecoveryHeatRate > 0.0)) {
            // no flow but trying to move heat to this loop problem!

            DidAnythingChange = true;
        }

        if (DidAnythingChange || FirstHVACIteration) {
            // use current mass flow rate and inlet temp from Node and recalculate outlet temp
            if (Node(InletNodeNum).MassFlowRate > MassFlowTolerance) {
                // update node outlet conditions
                Cp = GetSpecificHeatGlycol(state, PlantLoop(LoopNum).FluidName, ModelInletTemp, PlantLoop(LoopNum).FluidIndex, RoutineName);
                Node(OutletNodeNum).Temp = Node(InletNodeNum).Temp + ModelRecoveryHeatRate / (Node(InletNodeNum).MassFlowRate * Cp);
            }

            // set sim flag for this loop
            PlantLoop(LoopNum).LoopSide(LoopSide).SimLoopSideNeeded = true;

            // set sim flag on connected loops to true because this side changed
            if (PlantLoop(LoopNum).LoopSide(LoopSide).TotalConnected > 0) {
                for (ConnectLoopNum = 1; ConnectLoopNum <= PlantLoop(LoopNum).LoopSide(LoopSide).TotalConnected; ++ConnectLoopNum) {
                    if (PlantLoop(LoopNum).LoopSide(LoopSide).Connected(ConnectLoopNum).LoopDemandsOnRemote) {
                        OtherLoopNum = PlantLoop(LoopNum).LoopSide(LoopSide).Connected(ConnectLoopNum).LoopNum;
                        OtherLoopSide = PlantLoop(LoopNum).LoopSide(LoopSide).Connected(ConnectLoopNum).LoopSideNum;
                        PlantLoop(OtherLoopNum).LoopSide(OtherLoopSide).SimLoopSideNeeded = true;
                    }
                }
            }

        } else { // nothing changed so turn off sim flag
            PlantLoop(LoopNum).LoopSide(LoopSide).SimLoopSideNeeded = false;
        }
    }

    void UpdateAbsorberChillerComponentGeneratorSide(int const LoopNum,                   // component's loop index
                                                     int const LoopSide,                  // component's loop side number
                                                     int const EP_UNUSED(TypeOfNum),      // Component's type index
                                                     int const InletNodeNum,              // Component's inlet node pointer
                                                     int const EP_UNUSED(OutletNodeNum),  // Component's outlet node pointer
                                                     int const EP_UNUSED(HeatSourceType),            // Type of fluid in Generator loop
                                                     Real64 const ModelGeneratorHeatRate, // model's generator heat rate (W)
                                                     Real64 const ModelMassFlowRate,      // model's generator mass flow rate (kg/s)
                                                     bool const FirstHVACIteration)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Brent Griffith
        //       DATE WRITTEN   February 2010
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // provides reusable update routine for absoption chiller's generator
        // connection to plant loops

        // METHODOLOGY EMPLOYED:
        // check if anything changed or doesn't agree and set simulation flags.
        // update outlet conditions if needed or possible

        // Using/Aliasing
        using DataLoopNode::Node;
        using DataPlant::PlantLoop;

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        bool DidAnythingChange(false); // set to true if conditions changed
        int OtherLoopNum;                     // local loop pointer for remote connected loop
        int OtherLoopSide;                    // local loop side pointer for remote connected loop
        int ConnectLoopNum;                   // local do loop counter

//        TODO: Umm, this block seems like it doesn't do much...
//        // check if node heat rate compares well with generator heat rate
//        if (HeatSourceType == NodeType_Water) {
//
//        } else if (HeatSourceType == NodeType_Steam) {
//
//        } else {
//            // throw error
//        }

        // check if any conditions have changed
        if (Node(InletNodeNum).MassFlowRate != ModelMassFlowRate) DidAnythingChange = true;

        if ((Node(InletNodeNum).MassFlowRate == 0.0) && (ModelGeneratorHeatRate > 0.0)) {

            // DSU3 TODO also send a request that generator loop be made available, interlock message infrastructure??

            DidAnythingChange = true;
        }

        if (DidAnythingChange || FirstHVACIteration) {

            // set sim flag for this loop
            PlantLoop(LoopNum).LoopSide(LoopSide).SimLoopSideNeeded = true;

            // set sim flag on connected loops to true because this side changed
            if (PlantLoop(LoopNum).LoopSide(LoopSide).TotalConnected > 0) {
                for (ConnectLoopNum = 1; ConnectLoopNum <= PlantLoop(LoopNum).LoopSide(LoopSide).TotalConnected; ++ConnectLoopNum) {
                    if (PlantLoop(LoopNum).LoopSide(LoopSide).Connected(ConnectLoopNum).LoopDemandsOnRemote) {
                        OtherLoopNum = PlantLoop(LoopNum).LoopSide(LoopSide).Connected(ConnectLoopNum).LoopNum;
                        OtherLoopSide = PlantLoop(LoopNum).LoopSide(LoopSide).Connected(ConnectLoopNum).LoopSideNum;
                        PlantLoop(OtherLoopNum).LoopSide(OtherLoopSide).SimLoopSideNeeded = true;
                    }
                }
            }

        } else { // nothing changed so turn off sim flag
            PlantLoop(LoopNum).LoopSide(LoopSide).SimLoopSideNeeded = false;
        }
    }

    void InterConnectTwoPlantLoopSides(int const Loop1Num,
                                       int const Loop1LoopSideNum,
                                       int const Loop2Num,
                                       int const Loop2LoopSideNum,
                                       int const PlantComponentTypeOfNum,
                                       bool const Loop1DemandsOnLoop2)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         B. Griffith
        //       DATE WRITTEN   February 2010
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Setup PlantLoop data structure pointers to direct interacting loops

        // Using/Aliasing
        using DataPlant::ConnectedLoopData;
        using DataPlant::PlantLoop;

        if (Loop1Num == 0 || Loop1LoopSideNum == 0 || Loop2Num == 0 || Loop2LoopSideNum == 0) {
            return; // Associated ScanPlantLoopsForObject couldn't find the component in the the plant loop structure...
        }           // This is a Fatal error condition

        bool const Loop2DemandsOnLoop1(!Loop1DemandsOnLoop2);

        int TotalConnected;

        auto &loop_side_1(PlantLoop(Loop1Num).LoopSide(Loop1LoopSideNum));
        auto &connected_1(loop_side_1.Connected);
        if (allocated(connected_1)) {
            TotalConnected = ++loop_side_1.TotalConnected;
            connected_1.redimension(TotalConnected);
        } else {
            TotalConnected = loop_side_1.TotalConnected = 1;
            connected_1.allocate(1);
        }
        connected_1(TotalConnected).LoopNum = Loop2Num;
        connected_1(TotalConnected).LoopSideNum = Loop2LoopSideNum;
        connected_1(TotalConnected).ConnectorTypeOf_Num = PlantComponentTypeOfNum;
        connected_1(TotalConnected).LoopDemandsOnRemote = Loop1DemandsOnLoop2;

        auto &loop_side_2(PlantLoop(Loop2Num).LoopSide(Loop2LoopSideNum));
        auto &connected_2(loop_side_2.Connected);
        if (allocated(connected_2)) {
            TotalConnected = ++loop_side_2.TotalConnected;
            connected_2.redimension(TotalConnected);
        } else {
            TotalConnected = loop_side_2.TotalConnected = 1;
            connected_2.allocate(1);
        }
        connected_2(TotalConnected).LoopNum = Loop1Num;
        connected_2(TotalConnected).LoopSideNum = Loop1LoopSideNum;
        connected_2(TotalConnected).ConnectorTypeOf_Num = PlantComponentTypeOfNum;
        connected_2(TotalConnected).LoopDemandsOnRemote = Loop2DemandsOnLoop1;
    }

    void ShiftPlantLoopSideCallingOrder(int const OldIndex, int const NewIndex)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         B. Griffith
        //       DATE WRITTEN   <April 2011
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // re-arrange the calling order, move one loop side from an old index to a new one

        // Using/Aliasing
        using namespace DataPlant;
        using General::RoundSigDigits;

        // Object Data
        PlantCallingOrderInfoStruct RecordToMoveInPlantCallingOrderInfo;

        if (OldIndex == 0) {
            ShowSevereError("ShiftPlantLoopSideCallingOrder: developer error notice of invalid index, Old Index=0");
        }
        if (NewIndex == 0) {
            ShowSevereError("ShiftPlantLoopSideCallingOrder: developer error notice of invalid index, New Index=1");
        }
        if ((OldIndex == 0) || (NewIndex == 0)) {
            return;
        }

        // store copy of prior structure
        Array1D<PlantCallingOrderInfoStruct> TempPlantCallingOrderInfo(PlantCallingOrderInfo);

        RecordToMoveInPlantCallingOrderInfo = PlantCallingOrderInfo(OldIndex);

        if (OldIndex == NewIndex) {
            // do nothing, no shift needed.
        } else if ((OldIndex == 1) && (NewIndex > OldIndex) && (NewIndex < TotNumHalfLoops)) {
            // example was:      1  2  3  4  5  6  7  8 (with OI = 1, NI = 5)
            // example shifted:  2  3  4  5  1  6  7  8

            PlantCallingOrderInfo({1, NewIndex - 1}) = TempPlantCallingOrderInfo({2, NewIndex});
            PlantCallingOrderInfo(NewIndex) = RecordToMoveInPlantCallingOrderInfo;
            PlantCallingOrderInfo({NewIndex + 1, TotNumHalfLoops}) = TempPlantCallingOrderInfo({NewIndex + 1, TotNumHalfLoops});

        } else if ((OldIndex == 1) && (NewIndex > OldIndex) && (NewIndex == TotNumHalfLoops)) {
            // example was:      1  2  3  4  5  6  7  8 (with OI = 1, NI = 8)
            // example shifted:  2  3  4  5  6  7  8  1

            PlantCallingOrderInfo({1, NewIndex - 1}) = TempPlantCallingOrderInfo({2, NewIndex});
            PlantCallingOrderInfo(NewIndex) = RecordToMoveInPlantCallingOrderInfo;
        } else if ((OldIndex > 1) && (NewIndex > OldIndex) && (NewIndex < TotNumHalfLoops)) {
            // example was:      1  2  3  4  5  6  7  8 (with OI = 3, NI = 6)
            // example shifted:  1  2  4  5  6  3  7  8
            PlantCallingOrderInfo({1, OldIndex - 1}) = TempPlantCallingOrderInfo({1, OldIndex - 1});
            PlantCallingOrderInfo({OldIndex, NewIndex - 1}) = TempPlantCallingOrderInfo({OldIndex + 1, NewIndex});
            PlantCallingOrderInfo(NewIndex) = RecordToMoveInPlantCallingOrderInfo;
            PlantCallingOrderInfo({NewIndex + 1, TotNumHalfLoops}) = TempPlantCallingOrderInfo({NewIndex + 1, TotNumHalfLoops});
        } else if ((OldIndex > 1) && (NewIndex > OldIndex) && (NewIndex == TotNumHalfLoops)) {
            // example was:      1  2  3  4  5  6  7  8 (with OI = 3, NI = 8)
            // example shifted:  1  2  4  5  6  7  8  3
            PlantCallingOrderInfo({1, OldIndex - 1}) = TempPlantCallingOrderInfo({1, OldIndex - 1});
            PlantCallingOrderInfo({OldIndex, NewIndex - 1}) = TempPlantCallingOrderInfo({OldIndex + 1, NewIndex});
            PlantCallingOrderInfo(NewIndex) = RecordToMoveInPlantCallingOrderInfo;
        } else if ((OldIndex > 1) && (NewIndex < OldIndex) && (NewIndex == 1)) {
            // example was:      1  2  3  4  5  6  7  8 (with OI = 3, NI = 1)
            // example shifted:  3  1  2  4  5  6  7  8
            PlantCallingOrderInfo(NewIndex) = RecordToMoveInPlantCallingOrderInfo;
            PlantCallingOrderInfo({NewIndex + 1, OldIndex}) = TempPlantCallingOrderInfo({1, OldIndex - 1});
            PlantCallingOrderInfo({OldIndex + 1, TotNumHalfLoops}) = TempPlantCallingOrderInfo({OldIndex + 1, TotNumHalfLoops});

        } else if ((OldIndex > 1) && (NewIndex < OldIndex) && (NewIndex > 1)) {
            // example was:      1  2  3  4  5  6  7  8 (with OI = 3, NI = 2)
            // example shifted:  1  3  2  4  5  6  7  8
            PlantCallingOrderInfo({1, NewIndex - 1}) = TempPlantCallingOrderInfo({1, NewIndex - 1});
            PlantCallingOrderInfo(NewIndex) = RecordToMoveInPlantCallingOrderInfo;
            PlantCallingOrderInfo({NewIndex + 1, OldIndex}) = TempPlantCallingOrderInfo({NewIndex, NewIndex + (OldIndex - NewIndex) - 1});
            PlantCallingOrderInfo({OldIndex + 1, TotNumHalfLoops}) = TempPlantCallingOrderInfo({OldIndex + 1, TotNumHalfLoops});

        } else {
            ShowSevereError("ShiftPlantLoopSideCallingOrder: developer error notice, caught unexpected logical case in "
                            "ShiftPlantLoopSideCallingOrder PlantUtilities");
        }
    }

    void RegisterPlantCompDesignFlow(int const ComponentInletNodeNum, // the component's water inlet node number
                                     Real64 const DesPlantFlow        // the component's design fluid volume flow rate [m3/s]
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Fred Buhl(previosly SaveCompDesWaterFlow in General.cc)
        //       DATE WRITTEN   January 2004
        //       MODIFIED
        //       RE-ENGINEERED  B. Griffith April 2011, allow to enter repeatedly

        // PURPOSE OF THIS SUBROUTINE:
        // Regester the design fluid flow rates of plant components for sizing purposes
        // in an array that can be accessed by the plant manager routines
        // allows sizing routines to iterate by safely processing repeated calls from the same component

        // METHODOLOGY EMPLOYED:
        // Derived from SaveCompDesWaterFlow but changed to allow re entry with the same node just update
        // the information at the same location in the structure
        // The design flow rate is stored in a dynamic structure array along with the plant component's inlet node number
        // (which is used by plant as a component identifier instead if name and type).

        // Using/Aliasing
        using namespace DataSizing;

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int NumPlantComps;
        int PlantCompNum; // component do loop index
        bool Found;
        int thisCallNodeIndex;

        NumPlantComps = SaveNumPlantComps;

        if (NumPlantComps == 0) { // first time in, fill and return
            NumPlantComps = 1;
            CompDesWaterFlow.allocate(NumPlantComps);
            // save the new data
            CompDesWaterFlow(NumPlantComps).SupNode = ComponentInletNodeNum;
            CompDesWaterFlow(NumPlantComps).DesVolFlowRate = DesPlantFlow;
            SaveNumPlantComps = NumPlantComps;
            return;
        }

        Found = false;
        // find node num index in structure if any
        for (PlantCompNum = 1; PlantCompNum <= NumPlantComps; ++PlantCompNum) {
            if (ComponentInletNodeNum == CompDesWaterFlow(PlantCompNum).SupNode) {
                Found = true;
                thisCallNodeIndex = PlantCompNum;
            }
            if (Found) break;
        }

        if (!Found) {        // grow structure and add new node at the end
            ++NumPlantComps; // increment the number of components that use water as a source of heat or coolth
            CompDesWaterFlow.emplace_back(ComponentInletNodeNum, DesPlantFlow); // Append the new element
            SaveNumPlantComps = NumPlantComps;
        } else {
            CompDesWaterFlow(thisCallNodeIndex).SupNode = ComponentInletNodeNum;
            CompDesWaterFlow(thisCallNodeIndex).DesVolFlowRate = DesPlantFlow;
        }
    }

    void SafeCopyPlantNode(int const InletNodeNum,
                           int const OutletNodeNum,
                           Optional_int_const LoopNum,
                           Optional<Real64 const> EP_UNUSED(OutletTemp) // set on outlet node if present and water.
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         B.  Griffith
        //       DATE WRITTEN   February, 2010
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Provide a safer alternative for Node(outlet) = Node(inlet)
        // Intended just for plant

        // METHODOLOGY EMPLOYED:
        // Copy over state variables but not setpoints
        // derived from adiabatic Pipes

        // Using/Aliasing
        using DataLoopNode::Node;
        using DataPlant::PlantLoop;

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Node(OutletNodeNum).FluidType = Node(InletNodeNum).FluidType;

        Node(OutletNodeNum).Temp = Node(InletNodeNum).Temp;
        Node(OutletNodeNum).MassFlowRate = Node(InletNodeNum).MassFlowRate;
        Node(OutletNodeNum).Quality = Node(InletNodeNum).Quality;
        Node(OutletNodeNum).Enthalpy = Node(InletNodeNum).Enthalpy; // should have routines that keep this current with temp?

        Node(OutletNodeNum).TempMin = Node(InletNodeNum).TempMin;
        Node(OutletNodeNum).TempMax = Node(InletNodeNum).TempMax;
        // DSU3 not don't do this, upstream components outlet might stomp on this components inlet
        //  so don't propagate hardware limits downstream.  Node(OutletNodeNum)%MassFlowRateMin      = Node(InletNodeNum)%MassFlowRateMin
        // DSU3 not don't do this                            Node(OutletNodeNum)%MassFlowRateMax      = Node(InletNodeNum)%MassFlowRateMax
        // DSU3 hopefully these next two go away once changes are broadly implemented...
        Node(OutletNodeNum).MassFlowRateMinAvail = max(Node(InletNodeNum).MassFlowRateMin, Node(InletNodeNum).MassFlowRateMinAvail);
        Node(OutletNodeNum).MassFlowRateMaxAvail = min(Node(InletNodeNum).MassFlowRateMax, Node(InletNodeNum).MassFlowRateMaxAvail);

        Node(OutletNodeNum).HumRat = Node(InletNodeNum).HumRat; // air only?

        // Only pass pressure if we aren't doing a pressure simulation
        if (present(LoopNum)) {
            if (PlantLoop(LoopNum).PressureSimType > 1) {
                // Don't do anything
            } else {
                Node(OutletNodeNum).Press = Node(InletNodeNum).Press;
            }
        }
    }

    Real64 BoundValueToNodeMinMaxAvail(Real64 const ValueToBound, int const NodeNumToBoundWith)
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         Edwin Lee
        //       DATE WRITTEN   September 2010
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS FUNCTION:
        // Provides a clean way to quickly bound a generic value to within any node's minavail and maxavail range

        // METHODOLOGY EMPLOYED:
        // Bound up to min avail, down to max avail

        // Using/Aliasing
        using DataLoopNode::Node;

        // Return value
        Real64 BoundedValue;

        BoundedValue = ValueToBound;
        BoundedValue = max(BoundedValue, Node(NodeNumToBoundWith).MassFlowRateMinAvail);
        BoundedValue = min(BoundedValue, Node(NodeNumToBoundWith).MassFlowRateMaxAvail);

        return BoundedValue;
    }

    void TightenNodeMinMaxAvails(int const NodeNum, Real64 const NewMinAvail, Real64 const NewMaxAvail)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Edwin Lee
        //       DATE WRITTEN   January, 2011
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Provides a means of tightening up min/max avail on a node if possible

        // METHODOLOGY EMPLOYED:
        // Bring up node min avail to new min avail if it doesn't violate any other node conditions
        // Pull down node max avail to new max avail if it doesn't violate any other node conditions
        // Assumes that current min/max avails are already honoring hardware min/max values, so they aren't checked here

        // Using/Aliasing
        using DataLoopNode::Node;

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 OldMinAvail;
        Real64 OldMaxAvail;

        OldMinAvail = Node(NodeNum).MassFlowRateMinAvail;
        OldMaxAvail = Node(NodeNum).MassFlowRateMaxAvail;

        // If the new min avail is higher than previous, and it isn't higher than the max avail, update MIN AVAIL
        if ((NewMinAvail > OldMinAvail) && (NewMinAvail <= OldMaxAvail)) Node(NodeNum).MassFlowRateMinAvail = NewMinAvail;

        // If the new max avail is lower than previous, and it isn't lower than the min avail, update MAX AVAIL
        if ((NewMaxAvail < OldMaxAvail) && (NewMaxAvail >= OldMinAvail)) Node(NodeNum).MassFlowRateMaxAvail = NewMaxAvail;
    }

    Real64 BoundValueToWithinTwoValues(Real64 const ValueToBound, Real64 const LowerBound, Real64 const UpperBound)
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         Edwin Lee
        //       DATE WRITTEN   September 2010
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS FUNCTION:
        // Provides a clean way to quickly bound a generic value to within any two other values

        // METHODOLOGY EMPLOYED:
        // Bound up to min and down to max

        // Using/Aliasing
        using DataLoopNode::Node;

        // Return value
        Real64 BoundedValue;

        BoundedValue = ValueToBound;
        BoundedValue = max(BoundedValue, LowerBound);
        BoundedValue = min(BoundedValue, UpperBound);

        return BoundedValue;
    }

    bool IntegerIsWithinTwoValues(int const ValueToCheck, int const LowerBound, int const UpperBound)
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         Edwin Lee
        //       DATE WRITTEN   September 2010
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS FUNCTION:
        // Provides a clean way to quickly check if an integer is within two values

        // METHODOLOGY EMPLOYED:
        // TRUE if ValueToCheck = [LowerBound, UpperBound]
        // in other words, it returns true if ValueToCheck=LowerBound, or if ValueToCheck=UpperBound

        // Return value
        return (ValueToCheck >= LowerBound) && (ValueToCheck <= UpperBound);
    }

    // In-Place Right Shift by 1 of Array Elements
    void rshift1(Array1D<Real64> &a, Real64 const a_l)
    {
        assert(a.size_bounded());
        for (int i = a.u(), e = a.l(); i > e; --i) {
            a(i) = a(i - 1);
        }
        a(a.l()) = a_l;
    }

    void LogPlantConvergencePoints(bool const FirstHVACIteration)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Edwin Lee
        //       DATE WRITTEN   Summer 2011
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This routine stores the history of the plant convergence to check for stuck (max iteration) conditions

        // METHODOLOGY EMPLOYED:
        // Loop across all loops and loopsides
        //   On first hvac, reset the history arrays to begin anew
        //   Pick up the LoopSide inlet and outlet temp and flow rate
        //   Store this in the history array of each node using EOSHIFT

        for (int ThisLoopNum = 1; ThisLoopNum <= isize(DataPlant::PlantLoop); ++ThisLoopNum) {
            auto &loop(DataPlant::PlantLoop(ThisLoopNum));
            for (int ThisLoopSide = 1; ThisLoopSide <= isize(loop.LoopSide); ++ThisLoopSide) {
                auto &loop_side(loop.LoopSide(ThisLoopSide));

                if (FirstHVACIteration) {
                    loop_side.InletNode.TemperatureHistory = 0.0;
                    loop_side.InletNode.MassFlowRateHistory = 0.0;
                    loop_side.OutletNode.TemperatureHistory = 0.0;
                    loop_side.OutletNode.MassFlowRateHistory = 0.0;
                }

                int InletNodeNum = loop_side.NodeNumIn;
                Real64 InletNodeTemp = DataLoopNode::Node(InletNodeNum).Temp;
                Real64 InletNodeMdot = DataLoopNode::Node(InletNodeNum).MassFlowRate;

                int OutletNodeNum = loop_side.NodeNumOut;
                Real64 OutletNodeTemp = DataLoopNode::Node(OutletNodeNum).Temp;
                Real64 OutletNodeMdot = DataLoopNode::Node(OutletNodeNum).MassFlowRate;

                rshift1(loop_side.InletNode.TemperatureHistory, InletNodeTemp);
                rshift1(loop_side.InletNode.MassFlowRateHistory, InletNodeMdot);
                rshift1(loop_side.OutletNode.TemperatureHistory, OutletNodeTemp);
                rshift1(loop_side.OutletNode.MassFlowRateHistory, OutletNodeMdot);
            }
        }
    }

    void ScanPlantLoopsForObject(EnergyPlusData &state,
                                 std::string const &CompName,
                                 int const CompType,
                                 int &LoopNum,
                                 int &LoopSideNum,
                                 int &BranchNum,
                                 int &CompNum,
                                 bool &errFlag,
                                 Optional<Real64 const> LowLimitTemp,
                                 Optional<Real64 const> HighLimitTemp,
                                 Optional_int CountMatchPlantLoops,
                                 Optional_int_const InletNodeNumber,
                                 Optional_int_const SingleLoopSearch
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Edwin Lee
        //       DATE WRITTEN   November 2009
        //       MODIFIED       B. Griffith, changes to help with single component one multiple plant loops
        //       RE-ENGINEERED  na
        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine scans the plant loop structure trying to find the component by type then name.
        // If there are more than one match, it counts them up and returns count using an optional output arg
        // If the option input declaring the component inlet's node name, then the matching is more specific.
        // An optional input, lowlimittemp, can be passed in to be used in the PlantCondLoopOperation routines
        //  when distributing loads to components
        // METHODOLOGY EMPLOYED:
        // Standard EnergyPlus methodology.

        // Using/Aliasing
        using namespace DataGlobals;
        using BranchInputManager::AuditBranches;
        using General::RoundSigDigits;

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int LoopCtr;
        int LoopSideCtr;
        int BranchCtr;
        int CompCtr;
        bool FoundComponent;
        int FoundCount;
        bool FoundCompName;
        int StartingLoopNum;
        int EndingLoopNum;

        FoundCount = 0;

        FoundComponent = false;
        FoundCompName = false;
        StartingLoopNum = 1;
        EndingLoopNum = DataPlant::TotNumLoops;
        if (present(SingleLoopSearch)) {
            StartingLoopNum = SingleLoopSearch;
            EndingLoopNum = SingleLoopSearch;
        }

        for (LoopCtr = StartingLoopNum; LoopCtr <= EndingLoopNum; ++LoopCtr) {
            auto &this_loop(DataPlant::PlantLoop(LoopCtr));
            for (LoopSideCtr = 1; LoopSideCtr <= 2; ++LoopSideCtr) {
                auto &this_loop_side(this_loop.LoopSide(LoopSideCtr));
                for (BranchCtr = 1; BranchCtr <= this_loop_side.TotalBranches; ++BranchCtr) {
                    auto &this_branch(this_loop_side.Branch(BranchCtr));
                    for (CompCtr = 1; CompCtr <= this_branch.TotalComponents; ++CompCtr) {
                        auto &this_component(this_branch.Comp(CompCtr));
                        if (this_component.TypeOf_Num == CompType) {
                            if (UtilityRoutines::SameString(CompName, this_component.Name)) {
                                FoundCompName = true;
                                if (present(InletNodeNumber)) {
                                    if (InletNodeNumber > 0) {
                                        // check if inlet nodes agree
                                        if (InletNodeNumber == this_component.NodeNumIn) {
                                            FoundComponent = true;
                                            ++FoundCount;
                                            LoopNum = LoopCtr;
                                            LoopSideNum = LoopSideCtr;
                                            BranchNum = BranchCtr;
                                            CompNum = CompCtr;
                                        }
                                    }
                                } else {
                                    FoundComponent = true;
                                    ++FoundCount;
                                    LoopNum = LoopCtr;
                                    LoopSideNum = LoopSideCtr;
                                    BranchNum = BranchCtr;
                                    CompNum = CompCtr;
                                }
                                if (present(LowLimitTemp)) {
                                    this_component.MinOutletTemp = LowLimitTemp;
                                }
                                if (present(HighLimitTemp)) {
                                    this_component.MaxOutletTemp = HighLimitTemp;
                                }
                            }
                        }
                    }
                }
            }
        }

        if (!FoundComponent) {
            if (CompType >= 1 && CompType <= DataPlant::NumSimPlantEquipTypes) {
                if (!present(SingleLoopSearch)) {
                    ShowSevereError("Plant Component " + DataPlant::ccSimPlantEquipTypes(CompType) + " called \"" + CompName +
                                    "\" was not found on any plant loops.");
                    AuditBranches(state, true, DataPlant::ccSimPlantEquipTypes(CompType), CompName);
                } else {
                    ShowSevereError("Plant Component " + DataPlant::ccSimPlantEquipTypes(CompType) + " called \"" + CompName +
                                    "\" was not found on plant loop=\"" + DataPlant::PlantLoop(SingleLoopSearch).Name + "\".");
                }
                if (present(InletNodeNumber)) {
                    if (FoundCompName) {
                        ShowContinueError("Looking for matching inlet Node=\"" + DataLoopNode::NodeID(InletNodeNumber) + "\".");
                    }
                }
                if (present(SingleLoopSearch)) {
                    ShowContinueError("Look at Operation Scheme=\"" + DataPlant::PlantLoop(SingleLoopSearch).OperationScheme + "\".");
                    ShowContinueError("Look at Branches and Components on the Loop.");
                    ShowBranchesOnLoop(SingleLoopSearch);
                }
                errFlag = true;
            } else {
                ShowSevereError("ScanPlantLoopsForObject: Invalid CompType passed [" + RoundSigDigits(CompType) + "], Name=" + CompName);
                ShowContinueError("Valid CompTypes are in the range [1 - " + RoundSigDigits(DataPlant::NumSimPlantEquipTypes) + "].");
                ShowFatalError("Previous error causes program termination");
            }
        }

        if (present(CountMatchPlantLoops)) {
            CountMatchPlantLoops = FoundCount;
        }
    }

    void ScanPlantLoopsForNodeNum(std::string const &CallerName, // really used for error messages
                                  int const NodeNum,             // index in Node structure of node to be scanned
                                  int &LoopNum,                  // return value for plant loop
                                  int &LoopSideNum,              // return value for plant loop side
                                  int &BranchNum,
                                  Optional_int CompNum)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         B. Griffith
        //       DATE WRITTEN   Feb. 2010
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Get routine to return plant loop index and plant loop side
        // based on node number.  for one time init routines only.

        // METHODOLOGY EMPLOYED:
        // Loop thru plant data structure and find matching node.

        // REFERENCES:
        // na

        // Using/Aliasing
        using namespace DataGlobals;
        using General::RoundSigDigits;

        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:

        // SUBROUTINE PARAMETER DEFINITIONS:
        // na

        // INTERFACE BLOCK SPECIFICATIONS:
        // na

        // DERIVED TYPE DEFINITIONS:
        // na

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:

        int LoopCtr;
        int LoopSideCtr;
        int BranchCtr;
        int CompCtr;
        bool FoundNode;
        int inFoundCount;
        int outFoundCount;

        inFoundCount = 0;
        outFoundCount = 0;
        if (present(CompNum)) {
            CompNum = 0;
        }
        FoundNode = false;

        for (LoopCtr = 1; LoopCtr <= DataPlant::TotNumLoops; ++LoopCtr) {
            auto &this_loop(DataPlant::PlantLoop(LoopCtr));
            for (LoopSideCtr = 1; LoopSideCtr <= 2; ++LoopSideCtr) {
                auto &this_loop_side(this_loop.LoopSide(LoopSideCtr));
                for (BranchCtr = 1; BranchCtr <= this_loop_side.TotalBranches; ++BranchCtr) {
                    auto &this_branch(this_loop_side.Branch(BranchCtr));
                    for (CompCtr = 1; CompCtr <= this_branch.TotalComponents; ++CompCtr) {
                        auto &this_comp(this_branch.Comp(CompCtr));
                        if (NodeNum == this_comp.NodeNumIn) {
                            FoundNode = true;
                            ++inFoundCount;
                            LoopNum = LoopCtr;
                            LoopSideNum = LoopSideCtr;
                            BranchNum = BranchCtr;
                            if (present(CompNum)) {
                                CompNum = CompCtr;
                            }
                        }

                        if (NodeNum == this_comp.NodeNumOut) {
                            ++outFoundCount;
                            LoopNum = LoopCtr;
                            LoopSideNum = LoopSideCtr;
                            BranchNum = BranchCtr;
                        }
                    }
                }
            }
        }

        if (!FoundNode) {
            ShowSevereError("ScanPlantLoopsForNodeNum: Plant Node was not found as inlet node (for component) on any plant loops");
            ShowContinueError("Node Name=\"" + DataLoopNode::NodeID(NodeNum) + "\"");
            if (!DoingSizing) {
                ShowContinueError("called by " + CallerName);
            } else {
                ShowContinueError("during sizing: called by " + CallerName);
            }
            if (outFoundCount > 0) ShowContinueError("Node was found as outlet node (for component) " + RoundSigDigits(outFoundCount) + " time(s).");
            ShowContinueError("Possible error in Branch inputs.  For more information, look for other error messages related to this node name.");
            // fatal?
        }
    }

    bool AnyPlantLoopSidesNeedSim()
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         Edwin Lee
        //       DATE WRITTEN   November 2009
        //       MODIFIED       na
        //       RE-ENGINEERED  na
        // PURPOSE OF THIS FUNCTION:
        // This subroutine scans the plant LoopSide simflags and returns if any of them are still true
        // METHODOLOGY EMPLOYED:
        // Standard EnergyPlus methodology.
        // REFERENCES:
        // na
        // USE STATEMENTS:
        // na

        // Return value
        bool AnyPlantLoopSidesNeedSim;

        // Locals
        // FUNCTION ARGUMENT DEFINITIONS:
        // na

        // FUNCTION PARAMETER DEFINITIONS:
        // na

        // INTERFACE BLOCK SPECIFICATIONS
        // na

        // DERIVED TYPE DEFINITIONS
        // na

        // FUNCTION LOCAL VARIABLE DECLARATIONS:
        int LoopCtr;
        int LoopSideCtr;

        // Assume that there aren't any
        AnyPlantLoopSidesNeedSim = false;

        // Then check if there are any
        for (LoopCtr = 1; LoopCtr <= DataPlant::TotNumLoops; ++LoopCtr) {
            for (LoopSideCtr = 1; LoopSideCtr <= 2; ++LoopSideCtr) {
                if (DataPlant::PlantLoop(LoopCtr).LoopSide(LoopSideCtr).SimLoopSideNeeded) {
                    AnyPlantLoopSidesNeedSim = true;
                    return AnyPlantLoopSidesNeedSim;
                }
            }
        }

        return AnyPlantLoopSidesNeedSim;
    }

    void SetAllPlantSimFlagsToValue(bool const Value)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Edwin Lee
        //       DATE WRITTEN   November 2009
        //       MODIFIED       na
        //       RE-ENGINEERED  B. Griffith Feb 2009 DSU3
        // PURPOSE OF THIS SUBROUTINE:
        // Quickly sets all sim flags of a certain type (loop type/side) to a value
        // USE STATEMENTS:
        // na

        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int LoopCtr;

        // Loop over all loops
        for (LoopCtr = 1; LoopCtr <= DataPlant::TotNumLoops; ++LoopCtr) {
            auto &this_loop(DataPlant::PlantLoop(LoopCtr));
            this_loop.LoopSide(DataPlant::DemandSide).SimLoopSideNeeded = Value;
            this_loop.LoopSide(DataPlant::SupplySide).SimLoopSideNeeded = Value;
        }
    }

    void ShowBranchesOnLoop(int const LoopNum) // Loop number of loop
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda Lawrie
        //       DATE WRITTEN   November 2011
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This routine will display (with continue error messages) the branch/component
        // structure of the given loop.

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        std::string DemandSupply;
        int LSN; // LoopSide counter
        int BrN; // Branch counter
        int CpN; // Component (on branch) counter

        for (LSN = DataPlant::DemandSide; LSN <= DataPlant::SupplySide; ++LSN) {
            if (LSN == DataPlant::DemandSide) {
                DemandSupply = "Demand";
            } else if (LSN == DataPlant::SupplySide) {
                DemandSupply = "Supply";
            } else {
                DemandSupply = "Unknown";
            }
            ShowContinueError(DemandSupply + " Branches:");
            for (BrN = 1; BrN <= DataPlant::PlantLoop(LoopNum).LoopSide(LSN).TotalBranches; ++BrN) {
                ShowContinueError("  " + DataPlant::PlantLoop(LoopNum).LoopSide(LSN).Branch(BrN).Name);
                ShowContinueError("    Components on Branch:");
                for (CpN = 1; CpN <= DataPlant::PlantLoop(LoopNum).LoopSide(LSN).Branch(BrN).TotalComponents; ++CpN) {
                    ShowContinueError("      " + DataPlant::PlantLoop(LoopNum).LoopSide(LSN).Branch(BrN).Comp(CpN).TypeOf + ':' +
                                      DataPlant::PlantLoop(LoopNum).LoopSide(LSN).Branch(BrN).Comp(CpN).Name);
                }
            }
        }
    }

    int MyPlantSizingIndex(std::string const &CompType,      // component description
                           std::string const &CompName,      // user name of component
                           int const NodeNumIn,              // component water inlet node
                           int const EP_UNUSED(NodeNumOut),  // component water outlet node
                           bool &ErrorsFound,                // set to true if there's an error, unchanged otherwise
                           Optional_bool_const SupressErrors // used for WSHP's where condenser loop may not be on a plant loop
    )
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         Fred Buhl
        //       DATE WRITTEN   July 2008
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS FUNCTION:
        // Identify the correct Plant Sizing object for demand-side components such as heating and
        // cooling coils.

        // METHODOLOGY EMPLOYED:
        // This function searches all plant loops for a component whose input and
        // output nodes match the desired input & output nodes. This plant loop index is then used
        // to search the Plant Sizing array for the matching Plant Sizing object.

        // Using/Aliasing
        using DataSizing::NumPltSizInput;
        using DataSizing::PlantSizData;
        using DataSizing::PlantSizingData;
        //  USE DataPlant, ONLY: PlantLoop, ScanPlantLoopsForNodeNum

        // Return value
        int MyPltSizNum; // returned plant sizing index

        // FUNCTION LOCAL VARIABLE DECLARATIONS:

        int MyPltLoopNum;
        int PlantLoopNum;
        int DummyLoopSideNum;
        int DummyBranchNum;
        bool PrintErrorFlag;

        MyPltLoopNum = 0;
        MyPltSizNum = 0;
        if (present(SupressErrors)) {
            PrintErrorFlag = SupressErrors;
        } else {
            PrintErrorFlag = true;
        }

        ScanPlantLoopsForNodeNum("MyPlantSizingIndex", NodeNumIn, PlantLoopNum, DummyLoopSideNum, DummyBranchNum);

        if (PlantLoopNum > 0) {
            MyPltLoopNum = PlantLoopNum;
        } else {
            MyPltLoopNum = 0;
        }

        if (MyPltLoopNum > 0) {
            if (NumPltSizInput > 0) {
                MyPltSizNum = UtilityRoutines::FindItemInList(DataPlant::PlantLoop(MyPltLoopNum).Name, PlantSizData, &PlantSizingData::PlantLoopName);
            }
            if (MyPltSizNum == 0) {
                if (PrintErrorFlag) {
                    ShowSevereError("MyPlantSizingIndex: Could not find " + DataPlant::PlantLoop(MyPltLoopNum).Name + " in Sizing:Plant objects.");
                    ShowContinueError("...reference Component Type=\"" + CompType + "\", Name=\"" + CompName + "\".");
                }
                ErrorsFound = true;
            }
        } else {
            if (PrintErrorFlag) {
                ShowWarningError("MyPlantSizingIndex: Could not find " + CompType + " with name " + CompName + " on any plant loop");
            }
            ErrorsFound = true;
        }

        return MyPltSizNum;
    }

    bool verifyTwoNodeNumsOnSamePlantLoop(int const nodeIndexA, int const nodeIndexB)
    {
        // this function simply searches across plant loops looking for node numbers
        // it returns true if the two nodes are found to be on the same loop
        // it returns false otherwise
        // because this is a nested loop, there's no reason it should be called except in one-time fashion
        int matchedIndexA = 0;
        int matchedIndexB = 0;
        for (int loopNum = 1; loopNum <= DataPlant::TotNumLoops; loopNum++) {
            for (auto &loopSide : DataPlant::PlantLoop(loopNum).LoopSide) {
                for (auto &branch : loopSide.Branch) {
                    for (auto &comp : branch.Comp) {
                        if (comp.NodeNumIn == nodeIndexA || comp.NodeNumOut == nodeIndexA) {
                            matchedIndexA = loopNum;
                        }
                        if (comp.NodeNumIn == nodeIndexB || comp.NodeNumOut == nodeIndexB) {
                            matchedIndexB = loopNum;
                        }
                    }
                }
            }
        }
        return (matchedIndexA == matchedIndexB) && (matchedIndexA != 0); // only return true if both are equal and non-zero
    }

} // namespace PlantUtilities

} // namespace EnergyPlus
