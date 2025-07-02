#include "analysis_3_ee.C"

void analysis_4_pid()
{
    gStyle -> SetOptFit();

    analysis_3_ee();

    double bin1 = 40;
    double bin2 = 40;
    cout << bin1 << " low -> " << hist_ee -> GetXaxis() -> GetBinLowEdge(bin1) << endl;
    cout << bin1 << "  up -> " << hist_ee -> GetXaxis() -> GetBinUpEdge(bin2) << endl;
    auto hist_projection = hist_ee -> ProjectionY("projection",bin1,bin2);

    auto cvs = LKPainter::GetPainter() -> Canvas("cvs4");
    hist_projection-> Draw();

    TF1 *fit = new TF1("fit","gaus(0) + gaus(3)", 0,6);
    fit -> SetParameters(15,0.8,0.1,15,1.5,0.1);
    hist_projection -> Fit(fit);
}
