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
    runManager -> SetSensitiveDetector(placeGas);
    regionNPSimulation -> AddRootLogicalVolume(logicGas);
    logicGas -> SetVisAttributes(visGas);

    if (sensitiveRanges.size()>1)
    {
        for (auto iRange=0; iRange<sensitiveRanges.size()-1; ++iRange)
        {
            int z1 = sensitiveRanges[0+iRange];
            int z2 = sensitiveRanges[1+iRange];
            auto solidBlock = new G4Box(Form("solidBlock_%d_%d",z1,z2), dmGasVolume.x()*0.5, dmGasVolume.y()*0.5, 0.5*double(abs(z2-z1)));
            auto logicBlock = new G4LogicalVolume(solidBlock, materialGas, Form("logicBlock_%d_%d",z1,z2), 0, 0, 0);
            G4Transform3D transformBlock(G4RotationMatrix(), G4ThreeVector(0, 0, -0.5*dmGasVolume.z() + 0.5*double(z1+z2)));
            auto placeBlock = new G4PVPlacement(transformBlock, logicBlock, Form("SensitiveBlock_%d_%d",z1,z2), logicGas, false, 0);
            runManager -> SetSensitiveDetector(placeBlock);
            regionNPSimulation -> AddRootLogicalVolume(logicBlock);
            logicBlock -> SetVisAttributes(visBlock);
        }
    }

    if (1)
    {
        const G4double X6_PCBX    = 45.20*mm; 
        const G4double X6_PCBY    = 93.10*mm;
        const G4double X6_PCBZ    =  2.40*mm;
        const G4double X6_PCBSub1X = 43.60*mm;
        const G4double X6_PCBSub1Y = 78.30*mm;
        const G4double X6_PCBSub1Z =  1.20*mm;
        const G4double X6_PCBSub1XOffset = 0.0*mm;
        const G4double X6_PCBSub1YOffset = 6.2*mm;
        const G4double X6_PCBSub1ZOffset = 0.6*mm;
        const G4double X6_PCBSub2X = 42.20*mm;
        const G4double X6_PCBSub2Y = 76.90*mm;
        const G4double X6_PCBSub2Z =  1.20*mm;
        const G4double X6_PCBSub2XOffset = 0.0*mm;
        const G4double X6_PCBSub2YOffset = 6.2*mm;
        const G4double X6_PCBSub2ZOffset = -0.6*mm;

        const G4double X6_SiX = 43.30*mm;
        const G4double X6_SiY = 78.00*mm;
        const G4double X6_SiZ =  1.00*mm; 
        const G4double X6_SiXOffset =  0.0*mm;
        const G4double X6_SiYOffset =  6.2*mm;
        const G4double X6_SiZOffset =  0.5*mm; 
        const G4double X6_SiActiveX = 40.30*mm; 
        const G4double X6_SiActiveY = 75.00*mm;
        const G4double X6_SiActiveZ =  1.00*mm; // 1000 um

        const G4int X6_NFrontStrips = 8;
        const G4int X6_NBackStrips  = 4;

        const G4double Conn_X = 40.0*mm;
        const G4double Conn_Y =  5.0*mm;
        const G4double Conn_Z =  5.0*mm;

        ////////////////////////////////////////////////////////////
        // material definition
        G4Material* matSi  = MaterialManager::getInstance()->GetMaterialFromLibrary("Si");
        G4Material* matPCB = MaterialManager::getInstance()->GetMaterialFromLibrary("PCB");
        ////////////////////////////////////////////////////////////

        G4Box* solidX6PCBAll  = new G4Box("solidX6PCBAll",  X6_PCBX/2.,X6_PCBY/2.,X6_PCBZ/2.);
        G4Box* solidX6PCBSub1 = new G4Box("solidX6PCBSub1", X6_PCBSub1X/2.,X6_PCBSub1Y/2.,X6_PCBSub1Z/2.+0.01*mm); // +0.01 mm for the perfect subtraction of solid
        G4Box* solidX6PCBSub2 = new G4Box("solidX6PCBSub2", X6_PCBSub2X/2.,X6_PCBSub2Y/2.,X6_PCBSub2Z/2.+0.01*mm); // +0.01 mm for the perfect subtraction of solid

        G4VSolid* solidX6PCBTemp = new G4SubtractionSolid("solidX6PCBTemp", solidX6PCBAll,  solidX6PCBSub1, 0, G4ThreeVector(X6_PCBSub1XOffset, X6_PCBSub1YOffset, X6_PCBSub1ZOffset));
        G4VSolid* solidX6PCB     = new G4SubtractionSolid("solidX6PCB",     solidX6PCBTemp, solidX6PCBSub2, 0, G4ThreeVector(X6_PCBSub2XOffset, X6_PCBSub2YOffset, X6_PCBSub2ZOffset));

        auto m_VisX6    = new G4VisAttributes(G4Colour(0., 0.5, 0.5));
        auto m_VisX6PCB = new G4VisAttributes(G4Colour(0.8, 0.5, 0.5));
        auto m_VisConn  = new G4VisAttributes(G4Colour(0.8, 0.8, 0.8));

        G4LogicalVolume* logicX6PCB = new G4LogicalVolume(solidX6PCB, matPCB,"logicX6PCB",0,0,0);
        logicX6PCB->SetVisAttributes(m_VisX6PCB);

        G4Box* solidX6Conn = new G4Box("solidX6Conn", Conn_X/2.,Conn_Y/2.,Conn_Z/2.);
        G4LogicalVolume* logicX6Conn = new G4LogicalVolume(solidX6Conn, matPCB,"logicX6Conn",0,0,0);
        logicX6Conn->SetVisAttributes(m_VisConn);

        G4Box* solidX6Si = new G4Box("solidX6Si", X6_SiX/2.,X6_SiY/2.,X6_SiZ/2.);
        G4LogicalVolume* logicX6Si = new G4LogicalVolume(solidX6Si, matSi,"logicX6Si",0,0,0);
        logicX6Si->SetVisAttributes(m_VisX6);

        auto zPositionSi = par -> InitPar(490, "CSSUDetectorConstruction/SiZPosition ?? # mm, position of si-detetor in z in local gas coordinate starting from 0");
        zPositionSi = zPositionSi * mm;
        zPositionSi = zPositionSi -0.5*dmGasVolume.z();
        G4Transform3D transformX6Si  (G4RotationMatrix(), G4ThreeVector(0,0,zPositionSi));
        G4Transform3D transformX6PCB (G4RotationMatrix(), G4ThreeVector(-X6_SiXOffset, -X6_SiYOffset, -X6_SiZOffset+zPositionSi));
        G4Transform3D transformX6Conn(G4RotationMatrix(), G4ThreeVector(-X6_SiXOffset, -X6_SiYOffset - X6_PCBY/2. + Conn_Y/2., X6_PCBZ/2. + Conn_Z/2. + zPositionSi));
        auto placeX6Si   = new G4PVPlacement(transformX6Si  , logicX6Si  , "X6Si",   logicGas, false, 6);
        auto placeX6PCB  = new G4PVPlacement(transformX6PCB , logicX6PCB , "X6PCB",  logicGas, false, 7);
        auto placeX6Conn = new G4PVPlacement(transformX6Conn, logicX6Conn, "X6Conn", logicGas, false, 8);

        runManager -> SetSensitiveDetector(placeX6Si);
    }

    return placeWorld;
}
