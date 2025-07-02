#ifndef CSSUDETECTORCONSTRUCTION_HH
#define CSSUDETECTORCONSTRUCTION_HH

#include "LKG4RunManager.h"
#include "LKLogger.h"
#include "LKParameterContainer.h"

#include "G4UnionSolid.hh"
#include "G4Box.hh"
#include "G4Colour.hh"
#include "G4FieldManager.hh"
#include "G4LogicalVolume.hh"
#include "G4NistManager.hh"
#include "G4PVPlacement.hh"
#include "G4RunManager.hh"
#include "G4SystemOfUnits.hh"
#include "G4TransportationManager.hh"
#include "G4Tubs.hh"
#include "G4UniformMagField.hh"
#include "G4UserLimits.hh"
#include "G4VUserDetectorConstruction.hh"
#include "G4VisAttributes.hh"
#include "G4Region.hh"
#include "G4RegionStore.hh"
#include "G4FastSimulationManager.hh"
#include "G4VFastSimulationModel.hh"
#include "G4ProductionCuts.hh"
#include "globals.hh"

#include "DetectorConstruction.hh"

class CSSUDetectorConstruction : public DetectorConstruction
{
    public:
        CSSUDetectorConstruction();
        virtual ~CSSUDetectorConstruction(){}
        G4VPhysicalVolume* Construct();
};

#endif
