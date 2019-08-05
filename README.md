## Package for CombineHarvester to constrain uncertainties for MET recoil corrections

### Setup

In order to setup the software, follow the commands in the [checkout.sh](https://github.com/KIT-CMS/FitMETRecoilUnc/blob/master/scripts/checkout.sh) or simply download and execute it:

```bash
wget https://raw.githubusercontent.com/KIT-CMS/FitMETRecoilUnc/master/scripts/checkout.sh
bash checkout.sh
```

### Preparing inputs

Before creating datacards, please copy root files with input shapes into the [shapes](https://github.com/KIT-CMS/FitMETRecoilUnc/blob/master/shapes/) folder of this package. The root files should have the following naming scheme:

```bash
htt_mm.inputs-sm-Run${YEAR}-ML-${VARIABLE}.root
```
Prepare inputs for `YEAR` in `2016 2017 2018` and the MET component parallel to Z $`\vec{p}_{T}`$, called e.g. `metParToZ`.

The files must contain the following categories as folders:
 * `mm_0jet`
 * `mm_1jet`
 * `mm_ge2jet`

Futhermore, distriutions of the following processes must be provided:

 * `ZL` (signal process)
 * `ZTT`
 * `TTT`
 * `VVT`
 * `TTL`
 * `VVL`
 * `W`
 * `QCD`

Only for the `ZL` process, systematic variations are introduced being 10% shifts in the scale and resolution of the hadronic recoil. These variations should have the following names:

 * `CMS_htt_boson_scale_met_Run${YEAR}Up`
 * `CMS_htt_boson_scale_met_Run${YEAR}Down`
 * `CMS_htt_boson_reso_met_Run${YEAR}Up`
 * `CMS_htt_boson_reso_met_Run${YEAR}Down`

Feel free to extend the uncertainty model, since this one is far from complete.

### Creating datacards

The first step of datacard creation is covered by the [MorphingMETRecoil](https://github.com/KIT-CMS/FitMETRecoilUnc/blob/master/bin/MorphingMETRecoil.cpp) executable. Asuming, that datacards for all Run II years are needed, using `metParToZ` and `puppimetParToZ` as variables, the following shell command applies:

```bash
for var in metParToZ puppimetParToZ;
do
    for year in 2016 2017 2018;
    do
        MorphingMETRecoil --era=${year} --variable=${var}
    done
done
```

### Creating workspaces

This is done with the usual text-to-workspace method of CombineHarvester separately for each category:

```bash
combineTool.py -M T2W -i output_*/Run201*/mm*/ --parallel 10
```

### Performing the fits

To constrain the MET recoil uncertainties, a maximum likelihood fit is performed, with `ZL` as signal with a floating signal strength `r` in order to cover normalization effects. The recoil uncertainties are treated as nuisances. Execute the following command for the fit:

```bash
combineTool.py -M FitDiagnostics -d output_*/Run201*/*/combined.txt.cmb.root --there
```


### Calculating total values of the uncertainties

As a final step, the total uncertainty for scale and resolution is determined for each category. The computation follows this formula: $`10\% \cdot \sqrt{\text{pull}^2 + \text{constraint}^2}`$.
The calulcation is performed with the [determine_recoiluncs.py](https://github.com/KIT-CMS/FitMETRecoilUnc/blob/master/scripts/determine_recoiluncs.py) script. To perform the computation for all years and MET definitions, execute:

```bash
for var in metParToZ puppimetParToZ;
do
    for year in 2016 2017 2018;
    do
        python $CMSSW_BASE/src/CombineHarvester/METRecoil/scripts/determine_recoiluncs.py --initial_shift 0.1 --input_dir output_${var}/Run${year}/
    done
done
```

This will create `.json` files in the `output_${var}/Run${year}/` directories, which can be used further to define final recoil correction uncertainties
