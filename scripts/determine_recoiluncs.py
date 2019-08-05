import ROOT as r
import argparse
import os
import numpy as np
import json


categories = ["mm_0jet", "mm_1jet", "mm_ge2jet"]

def parse_arguments():
    parser = argparse.ArgumentParser(
        description=""
    )
    parser.add_argument("--initial_shift", type=float, required=True, help="Initial systematic shift chosen before fitting (equal for all systs & categories)")
    parser.add_argument("--input_dir",type=str,required=True,help="Main directory for fits of a certain year, e.g. output_metParToZ/Run2017/")
    return parser.parse_args()


def main(args):
    out = open(os.path.join(args.input_dir,"uncertainties.json"),"w")
    print "Considering directory:",args.input_dir
    print
    json_dict = {}
    for cat in categories:
        json_dict[cat] = {}
        fitfilepath = os.path.join(args.input_dir,cat,"fitDiagnostics.Test.root")
        f = r.TFile(fitfilepath, "read")
        fit_s = f.Get("fit_s")
        #prefit  = f.Get("nuisances_prefit")
        fittedPars = fit_s.floatParsFinal()
        print "Examine category:",cat
        for i in range(fittedPars.getSize()):
            par = fittedPars.at(i)
            name = par.GetName()
            if name == "r":
                continue
            #prefitpar = prefit.find(name)
            pull = par.getVal()
            uncertainty = par.getError()
            total_uncertainty = np.sqrt(pull**2 + uncertainty**2)*args.initial_shift
            print "\t",name,"postfit value & uncertainty:",par.getVal(),par.getError(),"total determined uncertainty:",total_uncertainty
            json_dict[cat][name] = total_uncertainty
    out.write(json.dumps(json_dict, indent=2,sort_keys=True))
    print "---- computation finished ----"
    print

if __name__ == "__main__":
    args = parse_arguments()
    main(args)
