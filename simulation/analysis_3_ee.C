#include "analysis_header.h"

TH2D* hist_ee;

void analysis_3_ee()
{
    int event, branch_id, step_id, track_id;
    double x, y, z, t, e;
    double energy_sum[4] = {0};

    hist_ee = new TH2D("ee",";enery loss from block-0 (MeV);enery loss from block-1 (MeV)",100,0,3.5,100,0,3.5);

    auto end_of_event = [&energy_sum]()
    {
        if (energy_sum[0]>0&&energy_sum[1]>0)
        {
            double energy_sum_0 = gRandom->Gaus(energy_sum[0],energy_sum[0]*energy_resolution);
            double energy_sum_1 = gRandom->Gaus(energy_sum[1],energy_sum[1]*energy_resolution);
            hist_ee-> Fill(energy_sum_0,energy_sum_1);
        }
        energy_sum[0] = 0;
        energy_sum[1] = 0;
        energy_sum[2] = 0;
        energy_sum[3] = 0;
    };

    int current_event = 0;
    for (auto file_name : {default_file_name1, default_file_name2})
    {
        if (file_name.IsNull()) break;
        cout << "Reading " << file_name << endl;
        ifstream data_file(file_name);
        while (data_file >> event >> branch_id >> step_id >> track_id >> x >> y >> z >> t >> e)
        {
            if (current_event!=event) {
                end_of_event();
                current_event = event;
            }
            if (branch_id==0) { energy_sum[0] += e; }
            if (branch_id==1) { energy_sum[1] += e; }
            if (branch_id==2) { energy_sum[2] += e; }
            if (branch_id==3) { energy_sum[3] += e; }
        }
        end_of_event();
        cout << "number of events = " << current_event+1 << endl;
    }

    auto cvs = LKPainter::GetPainter() -> CanvasSquare("cvs3");
    cvs -> SetMargin(.12,.15,.12,.12);
    hist_ee -> Draw("colz");
}
