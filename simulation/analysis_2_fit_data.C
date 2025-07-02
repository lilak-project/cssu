#include "analysis_1_read_data.C"

void analysis_2_fit_data(TString file_name="")
{
    gStyle -> SetOptFit();

    if (file_name.IsNull()) file_name = default_file_name1;
    analysis_1_read_data(file_name);

    TF1 *fit[4];
    fit[0] = new TF1("fit0","gaus(0)",0,6); fit[0] -> SetNpx(500);
    fit[1] = new TF1("fit1","gaus(0)",0,6); fit[1] -> SetNpx(500);
    fit[2] = new TF1("fit2","gaus(0)",0,6); fit[2] -> SetNpx(500);
    fit[3] = new TF1("fit3","gaus(0)",0,6); fit[3] -> SetNpx(500);

    histe[0] -> Fit(fit[0],"0");
    histe[1] -> Fit(fit[1],"0");
    histe[2] -> Fit(fit[2],"0");
    histe[3] -> Fit(fit[3],"0");

    histe[0] -> GetXaxis() -> SetRangeUser(fit[0]->GetParameter(1)-10*fit[0]->GetParameter(2),fit[0]->GetParameter(1)+10*fit[0]->GetParameter(2));
    histe[1] -> GetXaxis() -> SetRangeUser(fit[1]->GetParameter(1)-10*fit[1]->GetParameter(2),fit[1]->GetParameter(1)+10*fit[1]->GetParameter(2));
    histe[2] -> GetXaxis() -> SetRangeUser(fit[2]->GetParameter(1)-10*fit[2]->GetParameter(2),fit[2]->GetParameter(1)+10*fit[2]->GetParameter(2));
    histe[3] -> GetXaxis() -> SetRangeUser(fit[3]->GetParameter(1)-10*fit[3]->GetParameter(2),fit[3]->GetParameter(1)+10*fit[3]->GetParameter(2));
    
    auto cvs = LKPainter::GetPainter() -> CanvasDefault("cvs2");
    cvs -> Divide(2,2);
    cvs -> cd(1); histe[0] -> Draw(); fit[0] -> Draw("samel");
    cvs -> cd(2); histe[1] -> Draw(); fit[1] -> Draw("samel");
    cvs -> cd(3); histe[2] -> Draw(); fit[2] -> Draw("samel");
    cvs -> cd(4); histe[3] -> Draw(); fit[3] -> Draw("samel");
}
