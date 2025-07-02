#ifndef LKG4DETECTORCREATOR_HH
#define LKG4DETECTORCREATOR_HH

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

// nptool
#include "DetectorConstruction.hh"
#include "MaterialManager.hh"
#include "BeamReaction.hh"
#include "Decay.hh"

class LKG4DetectorCreator
{
    public:
        LKG4DetectorCreator() {}
        virtual ~LKG4DetectorCreator() {}
        G4VPhysicalVolume* AddDetector(LKParameterContainer* par);
        G4VPhysicalVolume* GetWorld();
};

#endif
