#include "CSSUDetectorConstruction.h"
#include "MaterialManager.hh"
#include "BeamReaction.hh"
#include "Decay.hh"

#include "G4SubtractionSolid.hh"
#include "G4AssemblyVolume.hh"

CSSUDetectorConstruction::CSSUDetectorConstruction()
{
}

G4VPhysicalVolume* CSSUDetectorConstruction::Construct()
{
    auto runManager = (LKG4RunManager *) G4RunManager::GetRunManager();
    auto par = runManager -> GetParameterContainer();

    /////////////////////////////////////////////////////////
    // General parameters
    /////////////////////////////////////////////////////////
    auto dmWorld         = par -> InitPar(TVector3(200,200,500),            "CSSUDetectorConstruction/WorldSize       ?? # mm");
    auto dmGasVolume     = par -> InitPar(TVector3(200,200,400),            "CSSUDetectorConstruction/GasVolumeSize   ?? # mm");
    auto lfGasMatName    = par -> InitPar(std::vector<TString>{"He","CO2"}, "CSSUDetectorConstruction/GasMaterial     ?? # gas material name. \"vacuum\" for vacuum");
    auto lfGasFraction   = par -> InitPar(std::vector<double>{.97,.03},     "CSSUDetectorConstruction/GasFraction     ?? # gas material factction in must add upto 1");
    auto gasTemperature  = par -> InitPar(295,                              "#CSSUDetectorConstruction/GasTemperture   ?? # kelvin");
    auto stepLimitInGas  = par -> InitPar(5,                                "CSSUDetectorConstruction/StepLimitInGas  ?? # mm");
    auto sensitiveRanges = par -> InitPar(std::vector<int>{},               "CSSUDetectorConstruction/SensitiveRanges ?? # mm, int, z range of sensitive volume in local gas coordinate starting from 0. ex) 0, 20, 40, 60");

    par -> Require("CSSUDetectorConstruction/GasPressure", "50 torr", "gas pressure with unit (bar or torr)");
    auto gasPressure = 0.1 * bar;
    TString gasPressureUnit = "bar";
    static constexpr double unit_torr = atmosphere / 760.;
    bool good_gas_parameter = false;
    if (par -> CheckPar("CSSUDetectorConstruction/GasPressure") && par->GetParN("CSSUDetectorConstruction/GasPressure")==2)
    {
        good_gas_parameter = true;
        par -> UpdatePar(gasPressure, "CSSUDetectorConstruction/GasPressure", 0);
        par -> UpdatePar(gasPressureUnit, "CSSUDetectorConstruction/GasPressure", 1);
        if      (gasPressureUnit=="torr") gasPressure = gasPressure * unit_torr;
        else if (gasPressureUnit=="bar")  gasPressure = gasPressure * bar;
        else
            good_gas_parameter = false;
    }
    if (good_gas_parameter==false)
        e_info << "Pressure parameter should have value and unit(bar or torr): " << par -> GetParString("CSSUDetectorConstruction/GasPressure") << endl;

    dmWorld     = dmWorld * mm;
    dmGasVolume = dmGasVolume * mm;
    gasTemperature = gasTemperature * kelvin;
    stepLimitInGas = stepLimitInGas * mm;

    /////////////////////////////////////////////////////////
    // Material
    /////////////////////////////////////////////////////////
    G4NistManager *nist = G4NistManager::Instance();
    G4Material *materialVacuum = nist -> FindOrBuildMaterial("G4_Galactic");

    G4Material* materialGas = nullptr;
    if (lfGasMatName.size()==1 && lfGasMatName[0]=="vacuum") {
        materialGas = nist -> FindOrBuildMaterial("G4_Galactic");
        e_info << "Gas volume is vacuum" << endl;
    }
    else {
        double density = 0;
        double density_sum = 0;
        vector<G4Material*> GasComponent;
        vector<double> FractionMass;
        int numberOfGasMix = lfGasMatName.size();
        if (lfGasMatName.size()>0&&lfGasFraction.size()==lfGasMatName.size())
        {
            if (numberOfGasMix==1) lfGasFraction[0] = 1;
            double total_fraction = 0;
            for (auto fraction : lfGasFraction) total_fraction += fraction;
            if (abs(1.-total_fraction)>0.01)
                e_error << "Sum of fraction of gas material should sum up to 1! (now=" << total_fraction << ")" << endl;
        }
        else {
            e_warning << "Something wrong with gas-material-names and -fractions: " << endl;
            for (unsigned int i = 0; i < lfGasMatName.size(); i++)  e_info << "mat-" << i << ": " << lfGasMatName[i] << endl;
            for (unsigned int i = 0; i < lfGasFraction.size(); i++) e_info << "frc-" << i << ": " << lfGasFraction[i] << endl;
            e_info << "Using CF4!" << endl;
            lfGasMatName.clear(); lfGasMatName.push_back("CF4");
            lfGasFraction.clear(); lfGasFraction.push_back(1);
        }
        e_info << "Gas gasPressure is " << gasPressure/bar << " bar (" << gasPressure/unit_torr << " torr)" << endl;
        for (unsigned int i = 0; i < numberOfGasMix; i++) {
            GasComponent.push_back(MaterialManager::getInstance()->GetGasFromLibrary(lfGasMatName[i].Data(), gasPressure, gasTemperature));
            density += lfGasFraction[i] * GasComponent[i]->GetDensity();
            density_sum += GasComponent[i]->GetDensity();
        }
        for (unsigned int i = 0; i < numberOfGasMix; i++) FractionMass.push_back(GasComponent[i]->GetDensity() / density_sum);
        materialGas = new G4Material("GasMix", density, numberOfGasMix, kStateGas, gasTemperature, gasPressure);
        for (unsigned int i = 0; i < numberOfGasMix; i++) materialGas -> AddMaterial(GasComponent[i], FractionMass[i]);
    }

    /////////////////////////////////////////////////////////
    // Region
    /////////////////////////////////////////////////////////
    G4ProductionCuts* ecut = new G4ProductionCuts();
    ecut -> SetProductionCut(1000*mm,"e-");
    auto regionNPSimulation = new G4Region("NPSimulationProcess");
    regionNPSimulation -> SetProductionCuts(ecut);
    regionNPSimulation -> SetUserLimits(new G4UserLimits(stepLimitInGas));    
    G4FastSimulationManager* simulationManager = regionNPSimulation -> GetFastSimulationManager();
    vector<G4VFastSimulationModel*> listSimulationModel;
    listSimulationModel.push_back(new NPS::BeamReaction("BeamReaction", regionNPSimulation));
    listSimulationModel.push_back(new NPS::Decay("Decay", regionNPSimulation));

    /////////////////////////////////////////////////////////
    // Attributes
    /////////////////////////////////////////////////////////
    auto visWorld = new G4VisAttributes(G4Colour::Grey());
    auto visGas   = new G4VisAttributes(G4Colour(0, 0.5, 0.5, 0.3));
    auto visBlock = new G4VisAttributes(G4Colour::Yellow());
    visWorld -> SetForceWireframe(true);
    visGas   -> SetForceWireframe(true);
    visBlock -> SetForceWireframe(true);

    /////////////////////////////////////////////////////////
    // Physical volume
    /////////////////////////////////////////////////////////
    auto solidWorld = new G4Box("solidWorld", dmWorld.x()*0.5, dmWorld.y()*0.5, dmWorld.z()*0.5);
    auto logicWorld = new G4LogicalVolume(solidWorld, materialVacuum, "logicWorld", 0, 0, 0);
    auto placeWorld = new G4PVPlacement(0, G4ThreeVector(0., 0., 0.), logicWorld, "World", 0, false, 0, true);
    logicWorld  -> SetVisAttributes(visWorld);

    auto solidGas = new G4Box("solidGas", dmGasVolume.x()*0.5, dmGasVolume.y()*0.5, dmGasVolume.z()*0.5);
    auto logicGas = new G4LogicalVolume(solidGas, materialGas, "logicGas", 0, 0, 0);
    auto placeGas = new G4PVPlacement(G4Transform3D(), logicGas, "Gas", logicWorld, false, 0);
    regionNPSimulation -> AddRootLogicalVolume(logicGas);
    logicGas -> SetVisAttributes(visGas);

    if (sensitiveRanges.size()>2)
    {
        for (auto iRange=0; iRange<sensitiveRanges.size()/2.; ++iRange)
        {
            int z1 = sensitiveRanges[0+2*iRange];
            int z2 = sensitiveRanges[1+2*iRange];
            auto solidBlock = new G4Box(Form("solidBlock_%d_%d",z1,z2), dmGasVolume.x()*0.5, dmGasVolume.y()*0.5, 0.5*double(abs(z2-z1)));
            auto logicBlock = new G4LogicalVolume(solidBlock, materialGas, Form("logicBlock_%d_%d",z1,z2), 0, 0, 0);
            G4Transform3D transformBlock(G4RotationMatrix(), G4ThreeVector(0, 0, -0.5*dmGasVolume.z() + 0.5*double(z1+z2)));
            auto placeBlock = new G4PVPlacement(transformBlock, logicBlock, Form("SensitiveBlock_%d_%d",z1,z2), logicGas, false, 0);
            runManager -> SetSensitiveDetector(placeBlock);
            regionNPSimulation -> AddRootLogicalVolume(logicBlock);
            logicBlock -> SetVisAttributes(visBlock);
        }
    }

    double zPositionSi = par -> InitPar(390,  "CSSUDetectorConstruction/SiZPosition ?? # mm, position of si-detetor in z in local gas coordinate starting from 0");
    double SidX        = par -> InitPar(50.0, "CSSUDetectorConstruction/SidX ?? # mm") * mm;
    double SidY        = par -> InitPar(50.0, "CSSUDetectorConstruction/SidY ?? # mm") * mm;
    double SiThickness = par -> InitPar(0.30, "CSSUDetectorConstruction/SiThickness ?? # mm") * mm;

    G4Material* matSi  = MaterialManager::getInstance()->GetMaterialFromLibrary("Si");

    G4Box* solidSi = new G4Box("solidSi", SidX/2.,SidY/2.,SiThickness/2.);
    G4LogicalVolume* logicSi = new G4LogicalVolume(solidSi, matSi,"logicSi",0,0,0);
    auto m_VisSi = new G4VisAttributes(G4Colour(0., 0.5, 0.5));
    logicSi -> SetVisAttributes(m_VisSi);

    zPositionSi = zPositionSi * mm;
    zPositionSi = zPositionSi -0.5*dmGasVolume.z();
    G4Transform3D transformSi(G4RotationMatrix(), G4ThreeVector(0,0,zPositionSi));
    auto placeSi = new G4PVPlacement(transformSi, logicSi, "Si", logicGas, false, 6);

    runManager -> SetSensitiveDetector(placeSi);

    return placeWorld;
}
