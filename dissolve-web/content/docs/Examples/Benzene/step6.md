---
title: Step 6 - Set up Refinement
grand_parent: Examples
---

## 6. Set up Refinement

Time to refine. We'll need two processing layers - one to calculate the g(r) and S(Q) from the simulation, and one to refine it against experimental data.

As noted in the introduction we have neutron-weighted experimental data, so we need a layer to calculate g(r) and neutron-weighted S(Q) for our three experimental datasets:

{{< action type="menu" text="Layer &#8680; Create... &#8680; Calculate... &#8680; RDF and Neutron S(Q)" >}}
{{< action type="mouse" text="Show the module palette for the layer by clicking the _Show Palette_{: .text-green-100} button at the top left of the tab" >}}
{{< step text="Drag two additional [`NeutronSQ`](../../userguide/modules/neutronsq) modules from the `Correlation Functions` in the _Module Palette_{: .text-green-100} on the left, placing them after the existing [`RDF`](../../userguide/modules/rdf) module">}}

Let's set up the three [`NeutronSQ`](../../userguide/modules/neutronsq) modules to calculate what we want:

### C<sub>6</sub>H<sub>6</sub>

{{< action type="mouse" text="Click on the first the [`NeutronSQ`](../../userguide/modules/neutronsq) module to display its options" >}}
{{< action type="edit" text="Change its name to `C6H6`" >}}
{{< action type="groups" text="Open the **Calculation** settings group" >}}
{{< step text="Set the **QBroadening** to `OmegaDependentGaussian` with a FWHM of 0.02">}}
{{< action type="groups" text="Open the **Neutron Isotopes** settings group" >}}
{{< step text="Click the _Edit..._{: .text-green-100} button for the **Isotopologue** option">}}
{{< step text="Press the _Auto_{: .text-green-100} button to populate the list with the default isotopic selection for each species" >}}
{{< action type="groups" text="Open the **Reference Data** settings group" >}}
{{< step text="For the **Reference** keyword select the file `C6H6.mint01` and set the format of the data to `mint` instead of `xy`">}}


### C<sub>6</sub>D<sub>6</sub>

{{< action type="mouse" text="Click on the second the [`NeutronSQ`](../../userguide/modules/neutronsq) module to display its options" >}}
{{< action type="edit" text="Change its name to `C6D6`" >}}
{{< action type="groups" text="Open the **Calculation** settings group" >}}
{{< step text="Set the **QBroadening** to `OmegaDependentGaussian` with a FWHM of 0.02" >}}
{{< action type="groups" text="Open the **Neutron Isotopes** settings group" >}}
{{< step text="Click the _Edit..._{: .text-green-100} button for the **Isotopologue** option" >}}
{{< step text=" Press the _Auto_{: .text-green-100} button to add the natural isotopologue for each species present" >}}
{{< step text="Change the isotopologue from `Natural` to `C6D6`" >}}
{{< action type="groups" text="Open the **Reference Data** settings group" >}}
{{< step text="For the **Reference** keyword select the file `C6D6.mint01` and set the format of the data to `mint`" >}}

### 50:50 Mix

{{< action type="mouse" text="Click on the third the [`NeutronSQ`](../../userguide/modules/neutronsq) module to display its options" >}}
{{< action type="edit" text="Change its name to `5050`" >}}
{{< action type="settings" text="Open the **Calculation** settings group" >}}
{{< step text="Set the **QBroadening** to `OmegaDependentGaussian` with a FWHM of 0.02">}}
{{< action type="settings" text=" Open the **Neutron Isotopes** settings group" >}}
{{< step text="Click the _Edit..._{: .text-green-100} button for the **Isotopologue** option">}}
{{< step text="Press the _Auto_{: .text-green-100} button to add the natural isotopologue for each species present">}}
{{< step text="Select the entry for the benzene species, or its isotopologue, and click the _Add_{: .text-green-100} button to insert the next \"unused\" isotopologue (i.e. the deuterated analogue)" >}}
{{< action type="settings" text="Open the **Reference Data** settings group" >}}
{{< step text="For the **Reference** keyword select the file `5050.mint01` and set the format of the data to `mint`">}}


All that remains is to add our EPSR refinement layer:

{{< action type="menu" text="Layer &#8680; Create... &#8680; Refinement... &#8680; Standard EPSR" >}}
{{< step text=" Select the [`EPSR`](../../userguide/modules/epsr) module to display its options">}}
{{< action type="edit" text="Change the **EReq** value to 10.0 - the default of 3.0 will not be enough to promote the necessary changes in structure" >}}


{{< button pos="left" text="Previous Step" path= "/docs/examples/benzene/step5/">}}
{{< button pos="right" text="Next Step" path= "/docs/examples/benzene/step7/">}}