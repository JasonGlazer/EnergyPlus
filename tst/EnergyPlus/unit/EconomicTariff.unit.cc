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

// Google Test Headers
#include <gtest/gtest.h>

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>

// EnergyPlus Headers
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/EconomicTariff.hh>
#include <EnergyPlus/ExteriorEnergyUse.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/OutputReportTabular.hh>
#include <EnergyPlus/OutputReportPredefined.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/SimulationManager.hh>

#include "Fixtures/EnergyPlusFixture.hh"

using namespace EnergyPlus;
using namespace EnergyPlus::EconomicTariff;
using namespace EnergyPlus::OutputProcessor;
using namespace EnergyPlus::OutputReportPredefined;

TEST_F(EnergyPlusFixture, EconomicTariff_GetInput_Test)
{
    std::string const idf_objects = delimited_string({
        "  UtilityCost:Tariff,                                                       ",
        "    ExampleFmc,              !- Name                                        ",
        "    ElectricityPurchased:Facility,  !- Output Meter Name                    ",
        "    kWh,                     !- Conversion Factor Choice                    ",
        "    ,                        !- Energy Conversion Factor                    ",
        "    ,                        !- Demand Conversion Factor                    ",
        "    TimeOfDaySchedule-Fmc,   !- Time of Use Period Schedule Name            ",
        "    TwoSeasonSchedule-Fmc,   !- Season Schedule Name                        ",
        "    ,                        !- Month Schedule Name                         ",
        "    ,                        !- Demand Window Length                        ",
        "    37.75;                   !- Monthly Charge or Variable Name             ",
        "                                                                            ",
        "  UtilityCost:Charge:Simple,                                                ",
        "    SummerOnPeak,            !- Name                                        ",
        "    ExampleFmc,              !- Tariff Name                                 ",
        "    peakEnergy,              !- Source Variable                             ",
        "    summer,                  !- Season                                      ",
        "    EnergyCharges,           !- Category Variable Name                      ",
        "    0.14009;                 !- Cost per Unit Value or Variable Name        ",
        "                                                                            ",
        "  UtilityCost:Charge:Simple,                                                ",
        "    SummerOffPeak,           !- Name                                        ",
        "    ExampleFmc,              !- Tariff Name                                 ",
        "    offPeakEnergy,           !- Source Variable                             ",
        "    summer,                  !- Season                                      ",
        "    EnergyCharges,           !- Category Variable Name                      ",
        "    0.06312;                 !- Cost per Unit Value or Variable Name        ",
        "                                                                            ",
        "  UtilityCost:Charge:Block,                                                 ",
        "    WinterOnPeak,            !- Name                                        ",
        "    ExampleFmc,              !- Tariff Name                                 ",
        "    peakEnergy,              !- Source Variable                             ",
        "    winter,                  !- Season                                      ",
        "    EnergyCharges,           !- Category Variable Name                      ",
        "    ,                        !- Remaining Into Variable                     ",
        "    ,                        !- Block Size Multiplier Value or Variable Name",
        "    650,                     !- Block Size 1 Value or Variable Name         ",
        "    0.04385,                 !- Block 1 Cost per Unit Value or Variable Name",
        "    350,                     !- Block Size 2 Value or Variable Name         ",
        "    0.03763,                 !- Block 2 Cost per Unit Value or Variable Name",
        "    remaining,               !- Block Size 3 Value or Variable Name         ",
        "    0.03704;                 !- Block 3 Cost per Unit Value or Variable Name",
        "                                                                            ",
        "  UtilityCost:Charge:Simple,                                                ",
        "    WinterOffPeak,           !- Name                                        ",
        "    ExampleFmc,              !- Tariff Name                                 ",
        "    offPeakEnergy,           !- Source Variable                             ",
        "    winter,                  !- Season                                      ",
        "    EnergyCharges,           !- Category Variable Name                      ",
        "    0.02420;                 !- Cost per Unit Value or Variable Name        ",
        "                                                                            ",
        "  UtilityCost:Qualify,                                                      ",
        "    MinDemand,               !- Name                                        ",
        "    ExampleFmc,              !- Tariff Name                                 ",
        "    TotalDemand,             !- Variable Name                               ",
        "    Minimum,                 !- Qualify Type                                ",
        "    12,                      !- Threshold Value or Variable Name            ",
        "    Annual,                  !- Season                                      ",
        "    Count,                   !- Threshold Test                              ",
        "    2;                       !- Number of Months                            ",
        "                                                                            ",
        "  UtilityCost:Computation,                                                  ",
        "    ManualExample,           !- Name                                        ",
        "    ExampleFmc,              !- Tariff Name                                 ",
        "    SumEneCharges SUM SUMMERONPEAK SUMMEROFFPEAK,  !- Compute Step 1        ",
        "    WinEneCharges SUM WINTERONPEAK WINTEROFFPEAK,  !- Compute Step 2        ",
        "    EnergyCharges SUM SumEneCharges WinEneCharges,  !- Compute Step 3       ",
        "    Basis SUM EnergyCharges DemandCharges ServiceCharges,  !- Compute Step 4",
        "    Subtotal SUM Basis Adjustment Surcharge,  !- Compute Step 5             ",
        "    Total SUM Subtotal Taxes;!- Compute Step 6                              ",
        "                                                                            ",
        "  UtilityCost:Ratchet,                                                      ",
        "    BillingDemand1,          !- Name                                        ",
        "    ExampleFmc,              !- Tariff Name                                 ",
        "    TotalDemand,             !- Baseline Source Variable                    ",
        "    TotalDemand,             !- Adjustment Source Variable                  ",
        "    Summer,                  !- Season From                                 ",
        "    Annual,                  !- Season To                                   ",
        "    0.80,                    !- Multiplier Value or Variable Name           ",
        "    0;                       !- Offset Value or Variable Name               ",
        "                                                                            ",
        "  Schedule:Compact,                                                         ",
        "    TwoSeasonSchedule-Fmc,   !- Name                                        ",
        "    number,                  !- Schedule Type Limits Name                   ",
        "    Through: 5/31,           !- Field 1                                     ",
        "    For: AllDays,            !- Field 2                                     ",
        "    Until: 24:00,1,          !- Field 3                                     ",
        "    Through: 9/30,           !- Field 5                                     ",
        "    For: AllDays,            !- Field 6                                     ",
        "    Until: 24:00,3,          !- Field 7                                     ",
        "    Through: 12/31,          !- Field 9                                     ",
        "    For: AllDays,            !- Field 10                                    ",
        "    Until: 24:00,1;          !- Field 11                                    ",
        "                                                                            ",
        "  Schedule:Compact,                                                         ",
        "    TimeOfDaySchedule-Fmc,   !- Name                                        ",
        "    number,                  !- Schedule Type Limits Name                   ",
        "    Through: 5/31,           !- Field 1                                     ",
        "    For: AllDays,            !- Field 2                                     ",
        "    Until: 15:00,3,          !- Field 3                                     ",
        "    Until: 22:00,1,          !- Field 5                                     ",
        "    Until: 24:00,3,          !- Field 7                                     ",
        "    Through: 9/30,           !- Field 9                                     ",
        "    For: AllDays,            !- Field 10                                    ",
        "    Until: 10:00,3,          !- Field 11                                    ",
        "    Until: 19:00,1,          !- Field 13                                    ",
        "    Until: 24:00,3,          !- Field 15                                    ",
        "    Through: 12/31,          !- Field 17                                    ",
        "    For: AllDays,            !- Field 18                                    ",
        "    Until: 15:00,3,          !- Field 19                                    ",
        "    Until: 22:00,1,          !- Field 21                                    ",
        "    Until: 24:00,3;          !- Field 23                                    ",
        "                                                                            ",

    });

    ASSERT_TRUE(process_idf(idf_objects));

    UpdateUtilityBills(state);

    // tariff
    EXPECT_EQ(1, numTariff);
    EXPECT_EQ("EXAMPLEFMC", tariff(1).tariffName);
    EXPECT_EQ(conversionKWH, tariff(1).convChoice);
    EXPECT_EQ(37.75, tariff(1).monthChgVal);

    // qualify
    EXPECT_EQ(1, numQualify);
    EXPECT_FALSE(qualify(1).isMaximum);
    EXPECT_EQ(12, qualify(1).thresholdVal);
    EXPECT_EQ(seasonAnnual, qualify(1).season);
    EXPECT_FALSE(qualify(1).isConsecutive);
    EXPECT_EQ(2, qualify(1).numberOfMonths);

    // ChargeSimple
    EXPECT_EQ(3, numChargeSimple);
    EXPECT_EQ(seasonWinter, chargeSimple(3).season);
    EXPECT_EQ(0.02420, chargeSimple(3).costPerVal);

    // ChargeBlock
    EXPECT_EQ(1, numChargeBlock);
    EXPECT_EQ(seasonWinter, chargeBlock(1).season);
    EXPECT_EQ(3, chargeBlock(1).numBlk);
    EXPECT_EQ(350, chargeBlock(1).blkSzVal(2));
    EXPECT_EQ(0.03763, chargeBlock(1).blkCostVal(2));

    // Ratchet
    EXPECT_EQ(1, numRatchet);
    EXPECT_EQ(seasonSummer, ratchet(1).seasonFrom);
    EXPECT_EQ(seasonAnnual, ratchet(1).seasonTo);
    EXPECT_EQ(0.80, ratchet(1).multiplierVal);
    EXPECT_EQ(0.0, ratchet(1).offsetVal);

    // Computation
    EXPECT_EQ(1, numComputation);
}

/** Test that if a meter is a water meter, and no conversion choice is give, it defaults to m3 **/
TEST_F(EnergyPlusFixture, EconomicTariff_Water_DefaultConv_Test)
{
    std::string const idf_objects = delimited_string({
        "  UtilityCost:Tariff,                                                       ",
        "    ExampleWaterTariff,      !- Name                                        ",
        "    Water:Facility,          !- Output Meter Name                           ",
        "    ,                        !- Conversion Factor Choice                    ",
        "    ,                        !- Energy Conversion Factor                    ",
        "    ,                        !- Demand Conversion Factor                    ",
        "    ,                        !- Time of Use Period Schedule Name            ",
        "    ,                        !- Season Schedule Name                        ",
        "    ,                        !- Month Schedule Name                         ",
        "    ,                        !- Demand Window Length                        ",
        "    10;                      !- Monthly Charge or Variable Name             ",
        "                                                                            ",
        "  UtilityCost:Charge:Simple,                                                ",
        "    FlatWaterChargePerm3,    !- Name                                        ",
        "    ExampleWaterTariff,      !- Tariff Name                                 ",
        "    totalEnergy,             !- Source Variable                             ",
        "    Annual,                  !- Season                                      ",
        "    EnergyCharges,           !- Category Variable Name                      ",
        "    3.3076;                  !- Cost per Unit Value or Variable Name        ",
    });

    ASSERT_TRUE(process_idf(idf_objects));

    // Create a water meter
    NumEnergyMeters = 1;
    EnergyMeters.allocate(NumEnergyMeters);
    EnergyMeters(1).Name = "WATER:FACILITY";
    EnergyMeters(1).ResourceType = "WATER";

    UpdateUtilityBills(state);

    // tariff
    EXPECT_EQ(1, numTariff);
    EXPECT_EQ("EXAMPLEWATERTARIFF", tariff(1).tariffName);

    // Check that it correctly assesses the meter type
    EXPECT_EQ(kindMeterWater, tariff(1).kindWaterMtr);
    EXPECT_EQ(kindMeterNotElectric, tariff(1).kindElectricMtr);
    EXPECT_EQ(kindMeterNotGas, tariff(1).kindGasMtr);

    // Check that if defaults the conversion choice correctly
    EXPECT_EQ(conversionM3, tariff(1).convChoice);
    EXPECT_EQ(1, tariff(1).energyConv);
    EXPECT_EQ(3600, tariff(1).demandConv);
    EXPECT_EQ(10, tariff(1).monthChgVal);
}

/** Test that if a meter is a water meter, and CCF is used, it uses the right conversion (not the gas one) **/
TEST_F(EnergyPlusFixture, EconomicTariff_Water_CCF_Test)
{
    std::string const idf_objects = delimited_string({
        "  UtilityCost:Tariff,                                                       ",
        "    ExampleWaterTariff,      !- Name                                        ",
        "    Water:Facility,          !- Output Meter Name                           ",
        "    CCF,                     !- Conversion Factor Choice                    ",
        "    ,                        !- Energy Conversion Factor                    ",
        "    ,                        !- Demand Conversion Factor                    ",
        "    ,                        !- Time of Use Period Schedule Name            ",
        "    ,                        !- Season Schedule Name                        ",
        "    ,                        !- Month Schedule Name                         ",
        "    ,                        !- Demand Window Length                        ",
        "    10;                      !- Monthly Charge or Variable Name             ",
    });

    ASSERT_TRUE(process_idf(idf_objects));

    // Create a water meter
    NumEnergyMeters = 1;
    EnergyMeters.allocate(NumEnergyMeters);
    EnergyMeters(1).Name = "WATER:FACILITY";
    EnergyMeters(1).ResourceType = "WATER";

    UpdateUtilityBills(state);;

    // tariff
    EXPECT_EQ(1, numTariff);

    // Check that it correctly assesses the meter type (water)
    EXPECT_EQ(kindMeterWater, tariff(1).kindWaterMtr);
    EXPECT_EQ(kindMeterNotElectric, tariff(1).kindElectricMtr);
    EXPECT_EQ(kindMeterNotGas, tariff(1).kindGasMtr);

    // Check conversion choice
    EXPECT_EQ(conversionCCF, tariff(1).convChoice);
    ASSERT_FLOAT_EQ(0.35314666721488586, tariff(1).energyConv);
}

/** Test that if a meter is a gas meter, and CCF is used, it uses the right conversion (not the water one) **/
TEST_F(EnergyPlusFixture, EconomicTariff_Gas_CCF_Test)
{
    std::string const idf_objects = delimited_string({
        "  UtilityCost:Tariff,                                                       ",
        "    ExampleTariff,           !- Name                                        ",
        "    NaturalGas:Facility,     !- Output Meter Name                           ",
        "    CCF,                     !- Conversion Factor Choice                    ",
        "    ,                        !- Energy Conversion Factor                    ",
        "    ,                        !- Demand Conversion Factor                    ",
        "    ,                        !- Time of Use Period Schedule Name            ",
        "    ,                        !- Season Schedule Name                        ",
        "    ,                        !- Month Schedule Name                         ",
        "    ,                        !- Demand Window Length                        ",
        "    10;                      !- Monthly Charge or Variable Name             ",
    });

    ASSERT_TRUE(process_idf(idf_objects));

    // Create a water meter
    NumEnergyMeters = 1;
    EnergyMeters.allocate(NumEnergyMeters);
    EnergyMeters(1).Name = "NATURALGAS:FACILITY";
    EnergyMeters(1).ResourceType = "NATURALGAS";

    UpdateUtilityBills(state);;

    // tariff
    EXPECT_EQ(1, numTariff);

    // Check that it correctly assesses the meter type (gas)
    EXPECT_EQ(kindMeterNotWater, tariff(1).kindWaterMtr);
    EXPECT_EQ(kindMeterNotElectric, tariff(1).kindElectricMtr);
    EXPECT_EQ(kindMeterGas, tariff(1).kindGasMtr);

    // Check conversion choice

    EXPECT_EQ(conversionCCF, tariff(1).convChoice);
    ASSERT_FLOAT_EQ(9.4781712e-9, tariff(1).energyConv);
}

/** Test that if a meter is an Electric meter, and CCF is used, it still defaults to kWh (not allowed) **/
TEST_F(EnergyPlusFixture, EconomicTariff_Electric_CCF_Test)
{
    std::string const idf_objects = delimited_string({
        "  UtilityCost:Tariff,                                                       ",
        "    ExampleTariff,           !- Name                                        ",
        "    Electricity:Facility,    !- Output Meter Name                           ",
        "    CCF,                     !- Conversion Factor Choice                    ",
        "    ,                        !- Energy Conversion Factor                    ",
        "    ,                        !- Demand Conversion Factor                    ",
        "    ,                        !- Time of Use Period Schedule Name            ",
        "    ,                        !- Season Schedule Name                        ",
        "    ,                        !- Month Schedule Name                         ",
        "    ,                        !- Demand Window Length                        ",
        "    10;                      !- Monthly Charge or Variable Name             ",
    });

    ASSERT_TRUE(process_idf(idf_objects));

    // Create a water meter
    NumEnergyMeters = 1;
    EnergyMeters.allocate(NumEnergyMeters);
    EnergyMeters(1).Name = "ELECTRICITY:FACILITY";
    EnergyMeters(1).ResourceType = "ELECTRICITY";

    UpdateUtilityBills(state);;

    // tariff
    EXPECT_EQ(1, numTariff);

    // Check that it correctly assesses the meter type (electricity, and electric simple in particular)
    EXPECT_EQ(kindMeterNotWater, tariff(1).kindWaterMtr);
    EXPECT_NE(kindMeterNotElectric, tariff(1).kindElectricMtr);
    EXPECT_EQ(kindMeterElecSimple, tariff(1).kindElectricMtr);
    EXPECT_EQ(kindMeterNotGas, tariff(1).kindGasMtr);

    // Check conversion choice, should force back to kWh
    EXPECT_EQ(conversionKWH, tariff(1).convChoice);
    ASSERT_FLOAT_EQ(0.0000002778, tariff(1).energyConv);
    ASSERT_FLOAT_EQ(0.001, tariff(1).demandConv);
}

TEST_F(EnergyPlusFixture, EconomicTariff_LEEDtariffReporting_Test)
{
    NumEnergyMeters = 4;
    EnergyMeters.allocate(NumEnergyMeters);
    EnergyMeters(1).Name = "ELECTRICITY:FACILITY";
    EnergyMeters(2).Name = "NATURALGAS:FACILITY";
    EnergyMeters(3).Name = "DISTRICTCOOLING:FACILITY";
    EnergyMeters(4).Name = "DISTRICTHEATING:FACILITY";

    numTariff = 4;
    tariff.allocate(numTariff);
    tariff(1).tariffName = "SecondaryGeneralUnit";
    tariff(1).isSelected = true;
    tariff(1).totalAnnualCost = 4151.45;
    tariff(1).totalAnnualEnergy = 4855.21;
    tariff(1).kindElectricMtr = 3;
    tariff(1).reportMeterIndx = 1;

    tariff(2).tariffName = "SmallCGUnit";
    tariff(2).isSelected = true;
    tariff(2).totalAnnualCost = 415.56;
    tariff(2).totalAnnualEnergy = 0.00;
    tariff(2).reportMeterIndx = 2;

    tariff(3).tariffName = "DistrictCoolingUnit";
    tariff(3).isSelected = true;
    tariff(3).totalAnnualCost = 55.22;
    tariff(3).totalAnnualEnergy = 8.64;
    tariff(3).reportMeterIndx = 3;

    tariff(4).tariffName = "DistrictHeatingUnit";
    tariff(4).isSelected = true;
    tariff(4).totalAnnualCost = 15.98;
    tariff(4).totalAnnualEnergy = 1.47;
    tariff(4).reportMeterIndx = 4;

    SetPredefinedTables(); // need to setup the predefined table entry numbers

    LEEDtariffReporting();

    EXPECT_EQ("SecondaryGeneralUnit", RetrievePreDefTableEntry(pdchLeedEtsRtNm, "Electricity"));
    EXPECT_EQ("SmallCGUnit", RetrievePreDefTableEntry(pdchLeedEtsRtNm, "Natural Gas"));
    EXPECT_EQ("DistrictCoolingUnit", RetrievePreDefTableEntry(pdchLeedEtsRtNm, "District Cooling"));
    EXPECT_EQ("DistrictHeatingUnit", RetrievePreDefTableEntry(pdchLeedEtsRtNm, "District Heating"));

    EXPECT_EQ("0.855", RetrievePreDefTableEntry(pdchLeedEtsVirt, "Electricity"));
    EXPECT_EQ("6.391", RetrievePreDefTableEntry(pdchLeedEtsVirt, "District Cooling"));
    EXPECT_EQ("10.871", RetrievePreDefTableEntry(pdchLeedEtsVirt, "District Heating"));
}

TEST_F(EnergyPlusFixture, EconomicTariff_GatherForEconomics)
{
    // Test for #7814 to ensure that Tariff seasons are calculated properly

    std::string const idf_objects = delimited_string({

        "RunPeriodControl:DaylightSavingTime,",
        "  2nd Sunday in March,     !- Start Date",
        "  1st Sunday in November;  !- End Date",

        "SimulationControl,",
        "  Yes,                     !- Do Zone Sizing Calculation",
        "  Yes,                     !- Do System Sizing Calculation",
        "  No,                      !- Do Plant Sizing Calculation",
        "  No,                      !- Run Simulation for Sizing Periods",
        "  YES;                     !- Run Simulation for Weather File Run Periods",

        "Building,",
        "  Mid-Rise Apartment,      !- Name",
        "  0,                       !- North Axis {deg}",
        "  City,                    !- Terrain",
        "  0.04,                    !- Loads Convergence Tolerance Value",
        "  0.4,                     !- Temperature Convergence Tolerance Value {deltaC}",
        "  FullExterior,            !- Solar Distribution",
        "  25,                      !- Maximum Number of Warmup Days",
        "  6;                       !- Minimum Number of Warmup Days",

        "Timestep,",
        "  4;                       !- Number of Timesteps per Hour",

        "RunPeriod,",
        "  Annual,                  !- Name",
        "  1,                       !- Begin Month",
        "  1,                       !- Begin Day of Month",
        "  ,                        !- Begin Year",
        "  12,                      !- End Month",
        "  31,                      !- End Day of Month",
        "  ,                        !- End Year",
        "  Sunday,                  !- Day of Week for Start Day",
        "  No,                      !- Use Weather File Holidays and Special Days",
        "  No,                      !- Use Weather File Daylight Saving Period",
        "  Yes,                     !- Apply Weekend Holiday Rule",
        "  Yes,                     !- Use Weather File Rain Indicators",
        "  Yes;                     !- Use Weather File Snow Indicators",

        "GlobalGeometryRules,",
        "  LowerLeftCorner,         !- Starting Vertex Position",
        "  Clockwise,               !- Vertex Entry Direction",
        "  Relative;                !- Coordinate System",

        "ScheduleTypeLimits,",
        "  Any Number;              !- Name",

        "Schedule:Constant,",
        "  Always On Discrete,      !- Name",
        "  Any Number,              !- Schedule Type Limits Name",
        "  1;                       !- Hourly Value",

        "Exterior:Lights,",
        "  Exterior Facade Lighting,!- Name",
        "  Always On Discrete,      !- Schedule Name",
        "  1000.00,                 !- Design Level {W}",
        "  ScheduleNameOnly,        !- Control Option",
        "  Exterior Facade Lighting;!- End-Use Subcategory",

        "Schedule:Compact,",
        "  Electricity Season Schedule,  !- Name",
        "  Any Number,              !- Schedule Type Limits Name",
        "  Through: 5/31,           !- Field 1",
        "  For: AllDays,            !- Field 2",
        "  Until: 24:00,            !- Field 3",
        "  1,                       !- Field 4",
        "  Through: 9/30,           !- Field 5",
        "  For: AllDays,            !- Field 6",
        "  Until: 24:00,            !- Field 7",
        "  3,                       !- Field 8",
        "  Through: 12/31,          !- Field 9",
        "  For: AllDays,            !- Field 10",
        "  Until: 24:00,            !- Field 11",
        "  1;                       !- Field 12",

        "UtilityCost:Tariff,",
        "  Seasonal_Tariff,         !- Name",
        "  ElectricityNet:Facility, !- Output Meter Name",
        "  kWh,                     !- Conversion Factor Choice",
        "  ,                        !- Energy Conversion Factor",
        "  ,                        !- Demand Conversion Factor",
        "  ,                        !- Time of Use Period Schedule Name",
        "  Electricity Season Schedule,  !- Season Schedule Name",
        "  ,                        !- Month Schedule Name",
        "  ,                        !- Demand Window Length",
        "  0,                       !- Monthly Charge or Variable Name",
        "  ,                        !- Minimum Monthly Charge or Variable Name",
        "  ,                        !- Real Time Pricing Charge Schedule Name",
        "  ,                        !- Customer Baseline Load Schedule Name",
        "  ,                        !- Group Name",
        "  NetMetering;             !- Buy Or Sell",

        "UtilityCost:Charge:Simple,",
        "  Seasonal_Tariff_Winter_Charge, !- Utility Cost Charge Simple Name",
        "  Seasonal_Tariff,         !- Tariff Name",
        "  totalEnergy,             !- Source Variable",
        "  Winter,                  !- Season",
        "  EnergyCharges,           !- Category Variable Name",
        "  0.02;                    !- Cost per Unit Value or Variable Name",

        "UtilityCost:Charge:Simple,",
        "  Seasonal_Tariff_Summer_Charge, !- Utility Cost Charge Simple Name",
        "  Seasonal_Tariff,         !- Tariff Name",
        "  totalEnergy,             !- Source Variable",
        "  Summer,                  !- Season",
        "  EnergyCharges,           !- Category Variable Name",
        "  0.04;                    !- Cost per Unit Value or Variable Name",

        "Output:Table:SummaryReports,",
        "  TariffReport;            !- Report 1 Name",

        "OutputControl:Table:Style,",
        "  HTML;                                   !- Column Separator",

        "Output:SQLite,",
        "  SimpleAndTabular;                       !- Option Type",

        "Output:Meter,Electricity:Facility,timestep;"

    });

    ASSERT_TRUE(process_idf(idf_objects));

    DataGlobals::NumOfTimeStepInHour = 4;    // must initialize this to get schedules initialized
    DataGlobals::MinutesPerTimeStep = 15;    // must initialize this to get schedules initialized
    DataGlobals::TimeStepZone = 0.25;
    DataGlobals::TimeStepZoneSec = DataGlobals::TimeStepZone * DataGlobals::SecInHour;

    ScheduleManager::ProcessScheduleInput(state); // read schedules
    ExteriorEnergyUse::ManageExteriorEnergyUse(state);
    EXPECT_EQ(1, state.dataExteriorEnergyUse->NumExteriorLights);
    EXPECT_EQ(1000, state.dataExteriorEnergyUse->ExteriorLights(1).DesignLevel);


    // This will only do the get input routines
    EconomicTariff::UpdateUtilityBills(state);;

    // tariff
    EXPECT_EQ(1, EconomicTariff::numTariff);
    EXPECT_EQ("SEASONAL_TARIFF", EconomicTariff::tariff(1).tariffName);
    EXPECT_EQ(conversionKWH, EconomicTariff::tariff(1).convChoice);
    EXPECT_EQ(0, EconomicTariff::tariff(1).monthChgVal);
    EXPECT_EQ("ELECTRICITY SEASON SCHEDULE", EconomicTariff::tariff(1).seasonSchedule);

    int seasonSchPtr = EconomicTariff::tariff(1).seasonSchIndex;
    EXPECT_GT(seasonSchPtr, 0);
    EXPECT_EQ("ELECTRICITY SEASON SCHEDULE", ScheduleManager::Schedule(seasonSchPtr).Name);

    // Two Simple Charges
    EXPECT_EQ(2, EconomicTariff::numChargeSimple);

    EXPECT_EQ(seasonWinter, EconomicTariff::chargeSimple(1).season);
    EXPECT_EQ(0.02, EconomicTariff::chargeSimple(1).costPerVal);

    EXPECT_EQ(EconomicTariff::seasonSummer, EconomicTariff::chargeSimple(2).season);
    EXPECT_EQ(0.04, EconomicTariff::chargeSimple(2).costPerVal);

    DataGlobals::KindOfSim = DataGlobals::ksRunPeriodWeather; // fake a weather run

    // Unitialized: default initialized to 0
    EXPECT_EQ(0, EconomicTariff::tariff(1).seasonForMonth(5));
    EXPECT_EQ(0, EconomicTariff::tariff(1).seasonForMonth(6));

    DataEnvironment::Month = 5;
    DataEnvironment::DayOfMonth = 31;
    DataGlobals::HourOfDay = 23;
    DataEnvironment::DSTIndicator = 1; // DST IS ON
    DataEnvironment::MonthTomorrow = 6;
    DataEnvironment::DayOfWeek = 4;
    DataEnvironment::DayOfWeekTomorrow = 5;
    DataEnvironment::HolidayIndex = 0;
    DataGlobals::TimeStep = 4;
    DataEnvironment::DayOfYear_Schedule = General::OrdinalDay(DataEnvironment::Month, DataEnvironment::DayOfMonth, 1);

    ScheduleManager::UpdateScheduleValues(state);
    EXPECT_EQ(1.0, ScheduleManager::LookUpScheduleValue(state, 1, DataGlobals::HourOfDay, DataGlobals::TimeStep));
    EXPECT_EQ(1.0, ScheduleManager::GetCurrentScheduleValue(tariff(1).seasonSchIndex));
    EXPECT_EQ(1.0, ScheduleManager::Schedule(seasonSchPtr).CurrentValue);

    ExteriorEnergyUse::ManageExteriorEnergyUse(state);

    EXPECT_EQ(1000.0, state.dataExteriorEnergyUse->ExteriorLights(1).Power);
    EXPECT_EQ(state.dataExteriorEnergyUse->ExteriorLights(1).Power * DataGlobals::TimeStepZoneSec, state.dataExteriorEnergyUse->ExteriorLights(1).CurrentUse);

    int curPeriod = 1;
    EXPECT_EQ(0, EconomicTariff::tariff(1).gatherEnergy(DataEnvironment::Month, curPeriod));

    // This Should now call GatherForEconomics
    DataGlobals::DoOutputReporting = true;
    EconomicTariff::UpdateUtilityBills(state);;
    EXPECT_EQ(1, EconomicTariff::tariff(1).seasonForMonth(5));
    EXPECT_EQ(0, EconomicTariff::tariff(1).seasonForMonth(6));


    DataEnvironment::Month = 5;
    DataEnvironment::DayOfMonth = 31;
    DataGlobals::HourOfDay = 24;
    DataEnvironment::DSTIndicator = 1; // DST IS ON
    DataEnvironment::MonthTomorrow = 6;
    DataEnvironment::DayOfWeek = 4;
    DataEnvironment::DayOfWeekTomorrow = 5;
    DataEnvironment::HolidayIndex = 0;
    DataGlobals::TimeStep = 1;
    DataEnvironment::DayOfYear_Schedule = General::OrdinalDay(DataEnvironment::Month, DataEnvironment::DayOfMonth, 1);

    ScheduleManager::UpdateScheduleValues(state);
    EXPECT_EQ(3.0, ScheduleManager::GetCurrentScheduleValue(tariff(1).seasonSchIndex));

    ExteriorEnergyUse::ManageExteriorEnergyUse(state);

    EXPECT_EQ(1000.0, state.dataExteriorEnergyUse->ExteriorLights(1).Power);
    EXPECT_EQ(state.dataExteriorEnergyUse->ExteriorLights(1).Power * DataGlobals::TimeStepZoneSec, state.dataExteriorEnergyUse->ExteriorLights(1).CurrentUse);

    // This Should now call GatherForEconomics
    EconomicTariff::UpdateUtilityBills(state);;
    EXPECT_EQ(1, EconomicTariff::tariff(1).seasonForMonth(5));
    EXPECT_EQ(3, EconomicTariff::tariff(1).seasonForMonth(6));

}
