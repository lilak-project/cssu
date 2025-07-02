#include "analysis_header.h"

TH1D *histe[4];
TH1D *histz[4];

void analysis_1_read_data(TString file_name="")
{
    if (file_name.IsNull()) file_name = default_file_name1;
    cout << "Reading " << file_name << endl;

    ifstream data_file(file_name);
    int event, branch_id, step_id, track_id;
    double x, y, z, t, e;
    double energy_sum[4] = {0};

    histe[0] = new TH1D("energysum1",";energy sum (MeV);",300,0,6);
    histe[1] = new TH1D("energysum2",";energy sum (MeV);",300,0,6);
    histe[2] = new TH1D("energysum3",";energy sum (MeV);",300,0,6);
    histe[3] = new TH1D("energysum4",";energy sum (MeV);",300,0,6);
    histz[0] = new TH1D("zPosition1",";z-position (MeV);",50,-250,250);
    histz[1] = new TH1D("zPosition2",";z-position (MeV);",50,-250,250);
    histz[2] = new TH1D("zPosition3",";z-position (MeV);",50,-250,250);
    histz[3] = new TH1D("zPosition4",";z-position (MeV);",50,-250,250);

    auto end_of_event = [&energy_sum]()
    {
        if (energy_sum[0]>0) histe[0] -> Fill(gRandom->Gaus(energy_sum[0],energy_sum[0]*energy_resolution));
        if (energy_sum[1]>0) histe[1] -> Fill(gRandom->Gaus(energy_sum[1],energy_sum[1]*energy_resolution));
        if (energy_sum[2]>0) histe[2] -> Fill(gRandom->Gaus(energy_sum[2],energy_sum[2]*energy_resolution));
        if (energy_sum[3]>0) histe[3] -> Fill(gRandom->Gaus(energy_sum[3],energy_sum[3]*energy_resolution));
        energy_sum[0] = 0;
        energy_sum[1] = 0;
        energy_sum[2] = 0;
        energy_sum[3] = 0;
    };

    int current_event = 0;
    while (data_file >> event >> branch_id >> step_id >> track_id >> x >> y >> z >> t >> e)
    {
        if (current_event!=event) {
            end_of_event();
            current_event = event;
        }
        if (branch_id==0) { energy_sum[0] += e; histz[0] -> Fill(z); }
        if (branch_id==1) { energy_sum[1] += e; histz[1] -> Fill(z); }
        if (branch_id==2) { energy_sum[2] += e; histz[2] -> Fill(z); }
        if (branch_id==3) { energy_sum[3] += e; histz[3] -> Fill(z); }
    }
    end_of_event();
    cout << "number of events = " << current_event+1 << endl;

    auto cvs = LKPainter::GetPainter() -> CanvasFull("cvs1",0.8);
    cvs -> Divide(4,2);
    cvs -> cd(1); histe[0] -> Draw();
    cvs -> cd(2); histe[1] -> Draw();
    cvs -> cd(3); histe[2] -> Draw();
    cvs -> cd(4); histe[3] -> Draw();
    cvs -> cd(5); histz[0] -> Draw();
    cvs -> cd(6); histz[1] -> Draw();
    cvs -> cd(7); histz[2] -> Draw();
    cvs -> cd(8); histz[3] -> Draw();
}
