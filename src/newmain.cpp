#include "carChecker.h"
#include "bmcChecker.h"
#include "statistics.h"
#include "data_structure.h"
#include "implysolver.h"
#include "model.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <signal.h>
#include <assert.h>
using namespace std;
using namespace car;

namespace car
{
    Statistics CARStats;
    ofstream dot_file;
    ofstream dive_file;
    Checker *chk;
    bool verbose = false;
    bool verbose_ = false;
    Model *model;
    const Model *State::model_;
    const aiger *State::aig_;
}

void signal_handler(int sig_num)
{
    CARStats.stop_everything();
    if (dot_file.is_open())
    {
        if (chk)
        {
            chk->draw_graph();
        }
        dot_file.close();
    }
    if (dive_file.is_open())
    {
        if (chk)
        {
            chk->dive_draw_tail();
        }
        dive_file.close();
    }
    CARStats.print();
    exit(0);
}

void print_usage()
{
    printf("Usage: simplecar <-f|-b|-p|-e|-v|-h> <aiger file> <output directory>\n");
    printf("       -f          forward checking (Default = backward checking)\n");
    printf("       -b          backward checking \n");
    printf("       -p          enable propagation (Default = off)\n");
    printf("       -e          print witness (Default = off)\n");
    printf("       -v          print verbose information (Default = off)\n");
    printf("       -h          print help information\n");
    printf("       -vb         reproduce the mUC result\n");
    exit(1);
}

string get_file_name(string &s)
{
    size_t start_pos = s.find_last_of("/");
    if (start_pos == string::npos)
        start_pos = 0;
    else
        start_pos += 1;

    string tmp_res = s.substr(start_pos);

    string res = "";
    // remove .aig

    size_t end_pos = tmp_res.find(".aig");
    assert(end_pos != string::npos);

    for (int i = 0; i < end_pos; i++)
        res += tmp_res.at(i);

    return res;
}

void check_aiger(int argc, char **argv)
{
    bool forward = false;
    bool evidence = false;
    bool propagate = false;
    bool enable_draw = false;
    bool enable_dive = false;
    bool bmc = false;
    bool enable_conv=false;
    bool enable_rotate=false;
    bool inv_incomplete = false;
    bool raw_uc = false;
    int inter_cnt=0;
    int convMode=-1;
    int convParam=0;
    int impMethod=0;
    int time_limit_to_restart = -1;
    int rememOption = 0;

    string input;
    string output_dir;
    bool input_set = false;
    bool output_dir_set = false;
    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "--vb") ==0)
        {
            inv_incomplete = true;
            enable_rotate = true;
            inter_cnt = 1;
            raw_uc = true;
            forward = false;
            evidence = true;
            impMethod = 5;
        }
        else if (strcmp(argv[i], "-f") == 0)
            forward = true;
        else if (strcmp(argv[i], "-b") == 0)
            forward = false;
        else if (strcmp(argv[i], "-p") == 0)
            propagate = true;
        else if (strcmp(argv[i], "-v") == 0)
        {
            verbose = true;  // used outside checker
            verbose_ = true; // used inside checker
        }
        else if (strcmp(argv[i], "-e") == 0)
            evidence = true;
        else if (strcmp(argv[i], "-h") == 0)
            print_usage();
        else if (strcmp(argv[i], "--rotate") == 0)
        {
            enable_rotate=true;
        }
        else if (strcmp(argv[i], "--inter") == 0)
        {
            assert(i+1<argc);
            ++i;
            inter_cnt = atoi(argv[i]);
        }
        else if (strcmp(argv[i], "--restart") == 0)
        {
            assert(i+1<argc);
            ++i;
            time_limit_to_restart = atoi(argv[i]);
        }
        else if (strcmp(argv[i], "--rem") == 0)
        {
            assert(i+1<argc);
            ++i;
            rememOption = atoi(argv[i]);
        }
        else if (strcmp(argv[i], "--draw") == 0)
        {
            evidence = true;
            enable_draw = true;
        }
        else if (strcmp(argv[i], "--dive") == 0)
        {
            enable_dive = true;
        }
        else if (strcmp(argv[i], "--bmc") == 0)
        {
            bmc = true;
        }
        else if (strcmp(argv[i], "--imp") == 0)
        {
            assert(i+1<argc);
            ++i;
            impMethod = atoi(argv[i]);
        }
        else if (strcmp(argv[i], "--incomplete") == 0)
        {
            inv_incomplete = true;
        }
        else if (strcmp(argv[i], "--raw") == 0)
        {
            raw_uc = true;
        }
        else if (strcmp(argv[i], "--convMode") == 0)
        {
            enable_conv = true;
            assert(i+1<argc);
            ++i;
            convMode = atoi(argv[i]);
        }
        else if (strcmp(argv[i], "--convParam") == 0)
        {
            assert(i+1<argc);
            ++i;
            convParam = atoi(argv[i]);
        }
        else if (!input_set)
        {
            input = string(argv[i]);
            input_set = true;
        }
        else if (!output_dir_set)
        {
            output_dir = string(argv[i]);
            output_dir_set = true;
        }
        else
            print_usage();
    }
    if (!input_set || !output_dir_set)
        print_usage();

    if (output_dir.at(output_dir.size() - 1) != '/')
        output_dir += "/";
    std::string filename = get_file_name(input);

    std::string stdout_filename = output_dir + filename + ".log";
    std::string stderr_filename = output_dir + filename + ".err";
    std::string res_file_name = output_dir + filename + ".res";
    std::string trial_name = output_dir + filename + ".trail";
    std::string dot_name = output_dir + filename + ".dot";
    std::string dive_name = output_dir + filename + ".dive.dot";
    if (!verbose)
        auto fs = freopen(stdout_filename.c_str(), "w", stdout);
    ofstream res_file;
    res_file.open(res_file_name.c_str());
    ofstream trail_file;
#ifdef TRAIL
    trail_file.open(trial_name.c_str());
#endif

    if (enable_draw)
        dot_file.open(dot_name.c_str());
    if (enable_dive)
        dive_file.open(dive_name.c_str());

    // get aiger object
    aiger *aig = aiger_init();
    aiger_open_and_read_from_file(aig, input.c_str());
    const char *err = aiger_error(aig);
    if (err)
    {
        printf("read aiger file error!\n");
        // throw InputError(err);
        exit(0);
    }
    if (!aiger_is_reencoded(aig))
        aiger_reencode(aig);

    Model *model = new Model(aig);
    // FIXME: collect all these static members. unify them.
    car::model = model;
    State::model_ = model;
    State::aig_ = aig;

    if (verbose)
        model->print();

    State::set_num_inputs_and_latches(model->num_inputs(), model->num_latches());

    // assume that there is only one output needs to be checked in each aiger model,
    // which is consistent with the HWMCC format
    assert(model->num_outputs() == 1);

    if (bmc)
    {
        auto bchker = new bmc::BMCChecker(model);
        bchker->check();
        bchker->printEvidence(res_file);
        return;
    }
    std::set<car::Checker *> to_clean;

    if(time_limit_to_restart > 0)
    {
        assert(convMode >=0);
        CARStats.count_whole_begin();

        // construct the checker
        // cout << "strategy is : convParam = " << convParam << endl;
        chk = new Checker(time_limit_to_restart, model, res_file, trail_file, dot_file, dive_file, enable_dive, forward, evidence, 0, convMode, convParam,enable_rotate, inter_cnt, inv_incomplete, raw_uc, impMethod);
        auto clear_delay = chk;// last checker may be used to pass information.
        bool res = chk->check();
        while (chk->ppstoped)
        {
            ++convParam;
            // see what happens with implySolver
            ImplySolver::reset_all();
            CARStats.reset_imply_cnter(); // reset

            chk = new Checker(time_limit_to_restart, clear_delay, rememOption, model, res_file, trail_file, dot_file, dive_file, enable_dive, forward, evidence, 0, convMode, convParam,enable_rotate, inter_cnt, inv_incomplete, raw_uc, impMethod);
            
            // cout << "strategy is : convParam = " << convParam << endl;
            res = chk->check();
            delete clear_delay;
            clear_delay = chk;
        }
        
        CARStats.count_whole_end();
        delete chk;

    }
    else{
        chk = new Checker(model, res_file, trail_file, dot_file, dive_file, enable_dive, forward, evidence, 0, convMode, convParam,enable_rotate, inter_cnt, inv_incomplete, raw_uc, impMethod);
        CARStats.count_whole_begin();
        chk->check();
        CARStats.count_whole_end();
        delete chk;
    }
    

    // cleaning work
    aiger_reset(aig);
    delete model;
    res_file.close();
#ifdef TRAIL
    trail_file.close();
#endif
    if (enable_draw)
        dot_file.close();
    if (enable_dive)
        dive_file.close();
    CARStats.print();
    return;
}

int main(int argc, char **argv)
{
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    check_aiger(argc, argv);

    return 0;
}
