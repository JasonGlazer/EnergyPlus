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

#include <EnergyPlus/Autosizing/WaterHeatingCapacitySizing.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/FluidProperties.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/Psychrometrics.hh>

namespace EnergyPlus {

Real64 WaterHeatingCapacitySizer::size(EnergyPlusData &state, Real64 _originalValue, bool &errorsFound)
{
    if (!this->checkInitialized(errorsFound)) {
        return 0.0;
    }
    this->preSize(_originalValue);

    if (this->curZoneEqNum > 0) {
        if (!this->wasAutoSized && !this->sizingDesRunThisZone) {
            this->autoSizedValue = _originalValue;
        } else {
            Real64 DesMassFlow = 0.0;
            Real64 NominalCapacityDes = 0.0;
            Real64 CoilInTemp = 0.0;
            Real64 CoilOutTemp = 0.0;
            Real64 CoilOutHumRat = 0.0;
            if ((this->termUnitSingDuct || this->termUnitPIU || this->termUnitIU) && (this->curTermUnitSizingNum > 0)) {
                DesMassFlow = this->termUnitSizing(this->curTermUnitSizingNum).MaxHWVolFlow;
                Real64 Cp = FluidProperties::GetSpecificHeatGlycol(state,
                                                                   DataPlant::PlantLoop(this->dataWaterLoopNum).FluidName,
                                                                   DataGlobals::HWInitConvTemp,
                                                                   DataPlant::PlantLoop(this->dataWaterLoopNum).FluidIndex,
                                                                   this->callingRoutine);
                Real64 rho = FluidProperties::GetDensityGlycol(state,
                                                               DataPlant::PlantLoop(this->dataWaterLoopNum).FluidName,
                                                               DataGlobals::HWInitConvTemp,
                                                               DataPlant::PlantLoop(this->dataWaterLoopNum).FluidIndex,
                                                               this->callingRoutine);
                NominalCapacityDes = DesMassFlow * this->dataWaterCoilSizHeatDeltaT * Cp * rho;
            } else if (this->zoneEqFanCoil || this->zoneEqUnitHeater) {
                DesMassFlow = this->zoneEqSizing(this->curZoneEqNum).MaxHWVolFlow;
                Real64 Cp = FluidProperties::GetSpecificHeatGlycol(state,
                                                                   DataPlant::PlantLoop(this->dataWaterLoopNum).FluidName,
                                                                   DataGlobals::HWInitConvTemp,
                                                                   DataPlant::PlantLoop(this->dataWaterLoopNum).FluidIndex,
                                                                   this->callingRoutine);
                Real64 rho = FluidProperties::GetDensityGlycol(state,
                                                               DataPlant::PlantLoop(this->dataWaterLoopNum).FluidName,
                                                               DataGlobals::HWInitConvTemp,
                                                               DataPlant::PlantLoop(this->dataWaterLoopNum).FluidIndex,
                                                               this->callingRoutine);
                NominalCapacityDes = DesMassFlow * this->dataWaterCoilSizHeatDeltaT * Cp * rho;
                // if coil is part of a zonal unit, calc coil load to get hot water flow rate
            } else {
                if (this->zoneEqSizing(this->curZoneEqNum).SystemAirFlow) {
                    DesMassFlow = this->zoneEqSizing(this->curZoneEqNum).AirVolFlow * DataEnvironment::StdRhoAir;
                } else if (this->zoneEqSizing(this->curZoneEqNum).HeatingAirFlow) {
                    DesMassFlow = this->zoneEqSizing(this->curZoneEqNum).HeatingAirVolFlow * DataEnvironment::StdRhoAir;
                } else {
                    DesMassFlow = this->finalZoneSizing(this->curZoneEqNum).DesHeatMassFlow;
                }
                CoilInTemp =
                    this->setHeatCoilInletTempForZoneEqSizing(this->setOAFracForZoneEqSizing(DesMassFlow, this->zoneEqSizing(this->curZoneEqNum)),
                                                              this->zoneEqSizing(this->curZoneEqNum),
                                                              this->finalZoneSizing(this->curZoneEqNum));
                // Real64 CoilInHumRat =
                //    this->setHeatCoilInletHumRatForZoneEqSizing(this->setOAFracForZoneEqSizing(DesMassFlow, this->zoneEqSizing(this->curZoneEqNum)),
                //                                                this->zoneEqSizing(this->curZoneEqNum),
                //                                                this->finalZoneSizing(this->curZoneEqNum));
                CoilOutTemp = this->finalZoneSizing(this->curZoneEqNum).HeatDesTemp;
                CoilOutHumRat = this->finalZoneSizing(this->curZoneEqNum).HeatDesHumRat;
                NominalCapacityDes = Psychrometrics::PsyCpAirFnW(CoilOutHumRat) * DesMassFlow * (CoilOutTemp - CoilInTemp);
            }
            this->autoSizedValue = NominalCapacityDes * this->dataHeatSizeRatio;
            if (DataGlobals::DisplayExtraWarnings && this->autoSizedValue <= 0.0) {
                std::string msg = this->callingRoutine + ": Potential issue with equipment sizing for " + this->compType + ' ' + this->compName;
                this->addErrorMessage(msg);
                ShowWarningMessage(msg);
                msg = "...Rated Total Heating Capacity = " + General::TrimSigDigits(this->autoSizedValue, 2) + " [W]";
                this->addErrorMessage(msg);
                ShowContinueError(msg);
                msg = "...Air flow rate used for sizing = " + General::TrimSigDigits(DesMassFlow / DataEnvironment::StdRhoAir, 5) + " [m3/s]";
                this->addErrorMessage(msg);
                ShowContinueError(msg);
                if (this->termUnitSingDuct || this->termUnitPIU || this->termUnitIU || this->zoneEqFanCoil || this->zoneEqUnitHeater) {
                    msg = "...Air flow rate used for sizing = " + General::TrimSigDigits(DesMassFlow / DataEnvironment::StdRhoAir, 5) + " [m3/s]";
                    this->addErrorMessage(msg);
                    ShowContinueError(msg);
                    msg = "...Plant loop temperature difference = " + General::TrimSigDigits(this->dataWaterCoilSizHeatDeltaT, 2) + " [C]";
                    this->addErrorMessage(msg);
                    ShowContinueError(msg);
                } else {
                    msg = "...Coil inlet air temperature used for sizing = " + General::TrimSigDigits(CoilInTemp, 2) + " [C]";
                    this->addErrorMessage(msg);
                    ShowContinueError(msg);
                    msg = "...Coil outlet air temperature used for sizing = " + General::TrimSigDigits(CoilOutTemp, 2) + " [C]";
                    this->addErrorMessage(msg);
                    ShowContinueError(msg);
                    msg = "...Coil outlet air humidity ratio used for sizing = " + General::TrimSigDigits(CoilOutHumRat, 2) + " [kgWater/kgDryAir]";
                    this->addErrorMessage(msg);
                    ShowContinueError(msg);
                }
            }
        }
    } else if (this->curSysNum > 0) {
        if (!this->wasAutoSized && !this->sizingDesRunThisAirSys) {
            this->autoSizedValue = _originalValue;
        } else {
            // not implemented for air loop equipment
        }
    }
    if (this->overrideSizeString) {
        if (this->isEpJSON) this->sizingString = "rated_capacity";
    }
    this->selectSizerOutput(errorsFound);
    if (this->isCoilReportObject)
        coilSelectionReportObj->setCoilWaterHeaterCapacityPltSizNum(
            this->compName, this->compType, this->autoSizedValue, this->wasAutoSized, this->dataPltSizHeatNum, this->dataWaterLoopNum);
    return this->autoSizedValue;
}

} // namespace EnergyPlus
