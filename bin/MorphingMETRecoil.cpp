#include <string>
#include <map>
#include <set>
#include <iostream>
#include <utility>
#include <vector>
#include <cstdlib>
#include "boost/algorithm/string/predicate.hpp"
#include "boost/program_options.hpp"
#include "boost/lexical_cast.hpp"
#include "boost/filesystem.hpp"
#include "CombineHarvester/CombineTools/interface/CombineHarvester.h"
#include "CombineHarvester/CombineTools/interface/Observation.h"
#include "CombineHarvester/CombineTools/interface/Process.h"
#include "CombineHarvester/CombineTools/interface/Utilities.h"
#include "CombineHarvester/CombineTools/interface/Systematics.h"
#include "CombineHarvester/CombineTools/interface/BinByBin.h"
#include "CombineHarvester/CombineTools/interface/AutoRebin.h"
#include "CombineHarvester/CombinePdfs/interface/MorphFunctions.h"
#include "CombineHarvester/CombineTools/interface/HttSystematics.h"
#include "RooWorkspace.h"
#include "RooRealVar.h"
#include "TH2.h"

using namespace std;
using boost::starts_with;
namespace po = boost::program_options;
using ch::syst::SystMap;
using ch::syst::SystMapAsymm;
using ch::syst::era;
using ch::syst::channel;
using ch::syst::bin_id;
using ch::syst::process;
using ch::JoinStr;
using namespace ch;

int main(int argc, char** argv) {
    string base_path = string(getenv("CMSSW_BASE")) + "/src/CombineHarvester/METRecoil/shapes/";
    string variable = "metParToZ";
    string postfix = "-ML";
    bool verbose = false;
    int era = 2016; // 2016, 2017, 2018
    po::variables_map vm;
    po::options_description config("configuration");
    config.add_options()
        ("base_path", po::value<string>(&base_path)->default_value(base_path))
        ("variable", po::value<string>(&variable)->default_value(variable))
        ("postfix", po::value<string>(&postfix)->default_value(postfix))
        ("verbose", po::value<bool>(&verbose)->default_value(verbose))
        ("era", po::value<int>(&era)->default_value(era));
    po::store(po::command_line_parser(argc, argv).options(config).run(), vm);
    po::notify(vm);

    typedef vector<pair<int, string>> Categories;

    std::vector<std::string> bkg_procs = {"ZTT", "TTT", "TTL", "VVT", "VVL", "W", "QCD"};
    std::vector<std::string> sig_procs = {"ZL"};
    Categories cats = {{1, "mm_0jet"}, {2, "mm_1jet"}, {3, "mm_ge2jet"}};

    ch::CombineHarvester cb;

    std::string era_tag;
    if (era == 2016) era_tag = "Run2016";
    else if (era == 2017) era_tag = "Run2017";
    else if (era == 2018) era_tag = "Run2018";

    cb.AddObservations({"*"}, {"htt"}, {era_tag}, {"mm"}, cats);
    cb.AddProcesses({"*"}, {"htt"}, {era_tag}, {"mm"}, bkg_procs, cats, false);
    cb.AddProcesses({""}, {"htt"}, {era_tag}, {"mm"}, sig_procs, cats, true);

    cb.cp()
        .channel({"mm"})
        .process({"ZL"})
        .AddSyst(cb, "CMS_htt_boson_scale_met_$ERA", "shape", SystMap<>::init(1.00));
    cb.cp()
        .channel({"mm"})
        .process({"ZL"})
        .AddSyst(cb, "CMS_htt_boson_reso_met_$ERA", "shape", SystMap<>::init(1.00));

    cb.cp().channel({"mm"}).backgrounds().ExtractShapes(
        base_path + "htt_mm.inputs-sm-" + era_tag + postfix + "-" + variable + ".root",
        "$BIN/$PROCESS", "$BIN/$PROCESS_$SYSTEMATIC");
    cb.cp().channel({"mm"}).process(sig_procs).ExtractShapes(
        base_path + "htt_mm.inputs-sm-" + era_tag + postfix + "-" + variable + ".root",
        "$BIN/$PROCESS$MASS", "$BIN/$PROCESS$MASS_$SYSTEMATIC");

    std::string output_prefix = "output_" + variable + "/";
    std::string output_dir = output_prefix + era_tag + "/common/";
    if (!boost::filesystem::exists(output_dir))
    {
        boost::filesystem::create_directories(output_dir);
    }

    TFile output((output_dir + "htt_input_" + era_tag + ".root").c_str(), "recreate");

    std::set<std::string> bins = cb.bin_set();
    for (auto b : bins){
        std::string datacard_dir = output_prefix + "/" + era_tag + "/" + b + "/";
        if (!boost::filesystem::exists(datacard_dir))
        {
            boost::filesystem::create_directories(datacard_dir);
        }
        cb.cp().bin({b}).WriteDatacard(datacard_dir + b + ".txt", output);
    }
}
