void read_data()
{
    ifstream file("data/cssu_alpha_test.dat");
    int event, branch_id, step_id, track_id;
    double x, y, z, t, e;
    while (file >> event >> branch_id >> step_id >> track_id >> x >> y >> z >> t >> e)
    {
        cout << event << " " << branch_id << " " << z << " " << e << endl;
    }
}
