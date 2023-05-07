


# Free FA only: Summary

The following graphs are calculated with a feed FA concentration 50 times less than the bound case. No other changes were made. Hence explanations below may not be consistent with the graphs. 

Two models are set-up and analyzed for the adsorption column. Only the loading
step is considered. The first model could apply to any situation, but neglects
dispersion. The second applies only to the situation where concentration
profiles have a tendency to sharpen, but has the benefit of also considering
dispersion.

An optimization is done with the second yielding decisions for the column
length and cross-sectional area, cycle time and resin particle size.

Several sensitivity analyses are done. Results tentatively suggest that a
cooler upstream of the column may be unnecessary, or at least that it is unnecessary for it to be large enough to cool the feed all the way down to 20 °C. A
second analysis suggests that the particle diameter is not very important so
long as it is within rather wide bounds.


+----------------------------------+-------------+-------------------------------------------------------------------------------+
| Variable                         | unit        | Description                                                                   |
+==================================+=============+===============================================================================+
| $a = 6(1-\alpha)/D_p$            | 1/m         | particle surface area per unit volume                                         |
+----------------------------------+-------------+-------------------------------------------------------------------------------+
| $c$                              | kg / m3     | ferulic acid concentration in the liquid phase                                |
+----------------------------------+-------------+-------------------------------------------------------------------------------+
| $c_0$                            | kg / m3     | ferulic acid concentration in the feed liquid                                 |
+----------------------------------+-------------+-------------------------------------------------------------------------------+
| $f(c) = K'Qc/(1 + K'c)$          | kg/m3       | ferulic acid concentration in the solid at equilibrium with a liquid at $c$   |
+----------------------------------+-------------+-------------------------------------------------------------------------------+
| $k_La$                           | 1/s         | mass transfer coefficient (including surface area per volume term)            |
+----------------------------------+-------------+-------------------------------------------------------------------------------+
| $k' = K'k_La$                    | m3 / kg / s | mass transfer coefficient                                                     |
+----------------------------------+-------------+-------------------------------------------------------------------------------+
| $q$                              | kg / m3     | ferulic acid concentration in the solid                                       |
+----------------------------------+-------------+-------------------------------------------------------------------------------+
| $r$                              | 1           | deviation from isotherm (see eq 9 below)                                      |
+----------------------------------+-------------+-------------------------------------------------------------------------------+
| $t$                              | s           | time                                                                          |
+----------------------------------+-------------+-------------------------------------------------------------------------------+
| $u_s = R / \alpha$               | m/s         | superficial velocity                                                          |
+----------------------------------+-------------+-------------------------------------------------------------------------------+
| $x$                              | 1           | dimensionless bed length (see equation (9) below)                             |
+----------------------------------+-------------+-------------------------------------------------------------------------------+
| $z = X/L$                        | 1           | dimensionless bed length (from 0 to 1)                                        |
+----------------------------------+-------------+-------------------------------------------------------------------------------+
| $A_c$                            | m^2         | empty bed cross section                                                       |
+----------------------------------+-------------+-------------------------------------------------------------------------------+
| $D$                              | m^2/s       | mass diffusivity (of FA in water)                                             |
+----------------------------------+-------------+-------------------------------------------------------------------------------+
| $D_p$                            | m           | particle diameter                                                             |
+----------------------------------+-------------+-------------------------------------------------------------------------------+
| $E$                              | m2/s        | dispersion                                                                    |
+----------------------------------+-------------+-------------------------------------------------------------------------------+
| $K'$                             | m3 / kg     | adsorption isotherm parameter                                                 |
+----------------------------------+-------------+-------------------------------------------------------------------------------+
| $L$                              | m           | bed length (maximum X)                                                        |
+----------------------------------+-------------+-------------------------------------------------------------------------------+
| $L_0$                            | m           | reference length (currently the minimum bed length)                           |
+----------------------------------+-------------+-------------------------------------------------------------------------------+
| $Q$                              | kg / m3     | adsorbent maximum capacity                                                    |
+----------------------------------+-------------+-------------------------------------------------------------------------------+
| $\mathbf{Pe} = RL_0/E\alpha$     | 1           | Peclet number                                                                 |
+----------------------------------+-------------+-------------------------------------------------------------------------------+
| $R$                              | m/s         | interstitial velocity                                                         |
+----------------------------------+-------------+-------------------------------------------------------------------------------+
| $\mathbf{Re} = D_p u_s / \nu$    | 1           | Reynolds number                                                               |
+----------------------------------+-------------+-------------------------------------------------------------------------------+
| $\mathbf{Sc} = \eta / D$         | 1           | Schmidt number (external)                                                     |
+----------------------------------+-------------+-------------------------------------------------------------------------------+
| $\mathbf{St} = k_La L_0\alpha/R$ | 1           | Stanton number                                                                |
+----------------------------------+-------------+-------------------------------------------------------------------------------+
| $\mathbf{Sh} = k_L D_p / D$      | 1           | Sherwood number (external mass transfer resistance)                           |
+----------------------------------+-------------+-------------------------------------------------------------------------------+
| $X$                              | m           | distance from bed feed end                                                    |
+----------------------------------+-------------+-------------------------------------------------------------------------------+
| $1/\lambda=1+\nu f(c_0)/c_0$     | 1           | dimensionless speed of the shock Rhee equation (7.10.12)                      |
+----------------------------------+-------------+-------------------------------------------------------------------------------+
| $\alpha$                         | 1           | ratio of liquid volume in the bed to total volume                             |
+----------------------------------+-------------+-------------------------------------------------------------------------------+
| $\eta$                           | m^2/s       | kinematic viscosity of water                                                  |
+----------------------------------+-------------+-------------------------------------------------------------------------------+
| $\theta$                         | 1           | variable added to convert a 2nd order ODE into a system of two 1st order ODEs |
+----------------------------------+-------------+-------------------------------------------------------------------------------+
| $\nu = (1-\alpha)/\alpha$        | 1           | ratio of solid volume to liquid volume in the column                          |
+----------------------------------+-------------+-------------------------------------------------------------------------------+
| $\xi = z - \lambda\tau$          | 1           | coordinate moving at the same speed as the shock wave                         |
+----------------------------------+-------------+-------------------------------------------------------------------------------+
| $\tau = Rt/\alpha L$             | 1           | dimensionless time                                                            |
+----------------------------------+-------------+-------------------------------------------------------------------------------+

Table: Nomenclature

# Model 1
## Governing Equations
Equations are based on S. Goldstein "On the mathematics of exchange processes in
fixed columns 1. Mathematical solutions and asymptotic expansions".
Equations present with numbers like (x) are copied from the paper.

Conservation of mass in a packed bed considering accumulation in solid and liquid phases as well as transport by the liquid moving with velocity $R$ is given by the equation below. Note that this equation neglects dispersion either due to
a velocity profile in the bed or axial mixing.

![](img/eq5_goldstein.png)

The boundary conditions selected are those for loading an initially clean column with a constant feed concentration:

$c|_{X > 0, t=0} = 0$

$q|_{X > 0, t=0} = 0$

$c|_{X=0, t} = c_0$

The rate of transport between phases where the adsorption follows a Langmuir isotherm $q_e / Q = c K' / (1 + K' c)$, and has a mass transfer coefficient $k'$ which is related to the $k_La$ which has units of $1/s$ by the equation $k' = K'k_La$.

![](img/eq7_goldstein.png)

The following dimensionless variables will be substituted to simplify the above equations:

![](img/eq9_goldstein.png)

The dimensionless version of equation 5 is: $\frac{\partial u}{\partial x} + \frac{\partial v}{\partial y} = 0$, while equation 7 becomes: $\frac{\partial v}{\partial y} = u - rv + (r-1)uv$.

After a lengthly derivation, which can be found in the reference, the concentrations in the solid and liquid can be recovered from the following equations. Note that these are vaild for a constant $u_0$. Substituting $u_0=1$, which is the natural dimensionless inlet concentration, allows some simplifications but these have not been done below.

$J(x,y) = \int_0^x I_0(2\sqrt{\tau y}) e^{-(y+\tau)} d\tau$, where $I_0$ is a modified Bessel function (Abramowitz & Stegun section 9.6)

![](img/beta.png)

![](img/MNF.png)

$u = N/F$

$v = M/F$

The actual concentrations of FA in the solid $q(X, t)$ and liquid
$c(X, t)$ are recovered by substituting the values of $u$ and $v$ into equation
9 above. Numerical integration, both for the $J$ as well as  calculating the
total mass of solute to leave the column $R \alpha A_c \int_0^t c(X, t) dt$ is done in
[Jfun.f90](Jfun.f90.html). That fortran code is called from some [R code](Jversion.R.html) which does the rest of the analysis.


## Correlations for parameters

### Diffusivity
From RE Treybal "Mass transfer operations" 1980, the Wilke-Chang correlation is given using

![](img/wilke_chang1.png)

![](img/wilke_chang2.png)

This leads to value of around $2\times 10^{-9}$ m$^2$/s. Note that the units for atomic volume are assumed to be m3 / kmol atoms × 1000, not a volume per 1000 atoms as listed, since this makes the two columns a comparable order of magnitude.

### Mass transfer coefficient
From N Wakao and S Kaguei "Heat and mass transfer in packed beds" 1982, a correlation for a mass transfer coefficient $k$ which considers a wide range of experimental data is reported below

![](img/transferB.png)



```
## Loading required package: foreach
```

```
## foreach: simple, scalable parallel programming from Revolution Analytics
## Use Revolution R for scalability, fault tolerance and more.
## http://www.revolutionanalytics.com
```

```
## Loading required package: iterators
```

```
## Loading required package: parallel
```


## Model 






![The longer column (green) takes longer to be saturated as expected.](figure_free/unnamed-chunk-5.svg) 



![breakthrough times](figure_free/unnamed-chunk-6.svg) 

```
## Error: 0 (non-NA) cases
```




```
##    user  system elapsed 
##   2.724   0.092   3.405
```


![For a fixed time at which to claim the resin is used up, there is a certain optimal design. The design with the lowest cost is the cycle time (1.5 days) that saturates a minimum length column (L = 0.1 m)](figure_free/unnamed-chunk-8.svg) 


------------------ ---------------------------------------
panel              description
------------------ ---------------------------------------
C.total            the total cost of the unit, less revenues in k\$ 

C.valve            the cost of replacing valves (k\$) which have an assumed cost and number of cycles before they need to be replaced. Longer cycle times reduce this cost, but it is small compared with other prices.

C.resin            this cost is independent of the cycle time, except when a constraint on the minimum bed length forces the bed to be oversized

fraction.recovery  in most cases nearly 99% of the inlet FA is to be adsorbed on the resin. This might not be practical for real operation, where a detector for breakthrough may not be sensitive enough. For example, absorbance at 280 nm which is expected to occur from the aromatic ring in FA may not be reliabily measured in the plant environment, since other minor components, such as protein, will also absorb at such a wavelength.

capacity.frac      fraction of the total capacity used. In most situations this is around 99.2%

L                  bed length. This is directly proportional to the cycle time.
------------------ ---------------------------------------

Resin is also assumed to have a finite number of cycles before it must be replaced. This cost does not vary much with the cycle time since the longer cycle times which lead to a longer resin lifetime is offset by the requirement for more resin. Loss of resin capacity due to irreversible adsorption might be a mechanism that follows the above method, if the poisons are present in low quantities and completely removed by the resin. On the other hand, some mechanisms might involve reactants (oxidisers for example) which are not reactive enough that a short bed will completly remove it from the stream. In that situation, it would appear that it would be better to have a smaller bed, since the total rate of resin deactivation will be faster when more resin is present.





![operating at higher temperatures improves kinetics, but reduces the equilibrium capacity of the resin. Overall higher temperatures are worse, but not by much.](figure_free/unnamed-chunk-10.svg) 


------------------ ---------------------------------------
panel              description
------------------ ---------------------------------------
C.total            total present value including revenue (1000 $)

C.resin            contribution to C.total by resin replacement (1000 $)

C.valve            contribution to C.total by valve replacement (1000 $)

Qsat/Q             equilibrium loading on the resin (kg/m^3) given the feed temperature and concentration divided by the value at infinite feed concentration

bedvol.loading     how many bed volumes of liquid are processed to load the column (dimensionless)

L                  bed length (m)

fraction.recovery  fraction of the FA fed that ends up on the resin

capacity.frac      fraction of the maximum capacity of the resin that is actually used when the column is deemed "saturated"

k/K                combined mass transfer resistance (1/s). 

k_liquid           liquid-side mass transfer coefficient including surface area (1/s)

k_solid            solid-side coeficient intraparticle diffusion (1/s)

seconds/3600       hours for each cycle
------------------ ---------------------------------------

Based on the above figure, it appears that the is only a small penalty for
running the adsorption at a higher temperature. The difference is only about
$1000. This means that the total project cost could be reduced by leaving out
the cooler upstream of the adsorption column.

This is because it was assumed
that the internal and external mass transfer are goverened by diffusivities
which decrease with the ratio $\mu /T$. There is a significant reduction in
the strength of adsorption as temperature increases: the $K$ which governs the
slope of the isotherm at low concentrations drops from above 12 m3/kg to 6
m3 / kg. However, because the product of feed concentration (c0) and $K$ is
still much greater than one, there is only a small impact on the maximum
capacity of the sorbent. If it was not assumed that there is a 10x enrichment
in FA concentration by the membrane filtration, then the higher temperature
will have a much larger impact, since the capacity of the resin given the feed concentration (*Qsat*) will drop much more at higher temperatures.

Note that this type of result is not backed by experiment,
and for example in the paper referenced to find the heat of
adsorption
[10.1016/j.jhazmat.2007.12.102](http://dx.doi.org/10.1016/j.jhazmat.2007.12.102)
fitted their equilibrium data with lower maximum capacities
as temperature increased.

Additional reasons to keep the cooler upstream are:

* reduce oxidation damage to the resin which may be a limiting factor for resin lifetime
* reduce polymerization of adsorbed FA, which will reduce recovery depending on the end-use of FA
* reduce the need for additional experiments
* improve safety, since the column must operates at low pH (2-4) and high pressure, so leaks will be less of a hazard if they are not hot

# Model 2
Assuming that the breakthrough curve's shape no longer varies with the distance traveled through the bed is a common simplification [McCabe Smith Harriott 1993]. Note that some combinations of feed concentration and isotherm do not allow this combination. In short the requirement is that on a graph of the isotherm $q = f(c)$, when tracing a straight line from the feed composition to the initial bed concentration, $f(c)$ has to be on the right (section 7.4 H-K Rhee cited below). Therefore this model is unlikely to be suitable for the eultion step, which has not been considered below.

## Justification
![Values of x from 0 to 120, which are much smaller than values used in the previous section (1000s) give very similarly shaped breakthrough curves, especially as $r \rightarrow 0$. In the above section, $r=0.019$, so it is apparent that the breakthrough curves no longer change shape as they pass through the columns.](figure_free/unnamed-chunk-11.svg) 


![breakthrough times ($x$ such that $u(x,y;r)=0.5$) given r.](figure_free/unnamed-chunk-12.svg) 


Further support for assuming a constant-pattern behavior can be found in the graphs calculated with it below. It is found that the mass transfer occurs within a zone whose length is much less than the length of the column.

## Equations Solved
Numerical concentrations both in the liquid and solids are only given graphically in general references such as [McCabe Smith Harriott 1993]. To have numerical values of concentrations, the analysis done in section 7.10 H-K Rhee, R Aris, NR Amundson "First Order Partial Differential Equations" volume 1 is followed. The equation that is used in this section to model the column behavior includes dispersion in addition to the mechanisms considered in the above section. Note that some variable names have been changed from the original source to avoid conflicts with the previous section.

$\frac{1}{\mathbf{Pe}} \frac{\partial^2 c}{\partial z^2} = \frac{\partial c}{\partial z} + \frac{\partial c}{\partial \tau} + \nu \frac{\partial q}{\partial \tau}$

$\frac{\partial q}{\partial \tau} = \mathbf{St} \left( f(c) - q \right)$


The assumption that the shape of the breakthrough curve becomes constant (in a
moving frame of reference), allows the above PDE to be reduced to an ODE. 
Equation numbers are those in the reference cited above.

(7.10.15)

$\frac{\partial c}{\partial \xi} = \theta$

$\frac{\partial \theta}{\partial \xi} = \left[ \left( 1-\lambda \right)\mathbf{Pe} + \mathbf{St}/\lambda \right] - \nu \mathbf{Pe} \mathbf{St} F$

(7.10.14)

$F = (c - c_0)f(c_0)/c_0 - f(c) + f(c_0)$

exercise 7.10.4

$n = F - \theta /(\nu\lambda\mathbf{Pe}) + f(c)$

The above definitions are provided to an ODE solver (lsodar) which integrates backwards from initial conditions $c \approx 0$ and $\partial c / \partial \xi =0$.  The $\xi$ coordinate at which the ODE solver starts is arbitrary. The feed is introduced as a step, but the profile spreads out eventually becoming the profile that is calculated with the above equations. The centre of the step has a certain value of $\xi$, refered to as $\xi_r$ below, which needs to be known in order to calculate how long the loading step will take. One way to define that centre is that there is no net transport of solute across the centre as the curve spreads out.

![areas A and B are equal at $\xi_r$](img/18.svg)

$A = \int^{0}_{\xi_r} c d\xi$

$B = \int_{-\infty}^{\xi_r} (c_0 - c) d\xi$

Combining and re-arranging gives:

$\xi_r = \frac{1}{c_0} \int_{-\infty}^{0} (c_0 - c) d\xi$

For numerical calculations, the infinities are instead $\xi$ values such that the integrand (before combining the integrals) is close to 0. One additional modification to the above equations is that the average concentration considering both phases ($\alpha c + (1-\alpha)q$) is used.

If mass transfer happened instantly, and there was no dispersion, then the profile found would be a step centred around $\xi_r$, which would travel from the inlet of the bed to the outlet in a time equal to $t_r = L\alpha / R \lambda$. If the cycle is stopped at a time $\xi_e$, the step will actually take $t = t_r (1 + \xi_r - \xi_e)$. Graphically this looks like:

![Variables relating to the time at which the cycle ends](img/18b.svg)

If the cycle is stopped at a low $c(\xi_e)$ then not much product is lost, but then the cycle time will be shorter. Below it is found that the optimal design has a $c(\xi_e)$ at a minimum value, which will be the lowest concentration that can be detected reliably without too much expense.

## Dispersion
![Empirical correlation for dispersion in packed beds chosen from Levenspiel 1999 pg311](img/dispersion_small.png)

![fit of the above curve giving the mean, max and minimum. The mean value is used in the following calculations.](figure_free/unnamed-chunk-13.svg) 


## Numerical solution

```
## Error: object 'deFun' not found
```

```
## Timing stopped at: 0.012 0 0.012
```





![As the interstitial velocity $R$ is increased the number of bedvolumes processed decreased. This is because the internal mass transfer resistance dominates, so any improvement in the external mass transfer coefficient barely changes the St, while it has increases dispersion. Longer beds have a more efficient use of resin in that more bed volumes can be processed before the effluent rises to the cutoff concentration. This is because the breakthrough curve does not spread out as it passes through the bed, so there is a constant amount of resin unused at the end. This constant amount makes up a smaller fraction for longer beds.](figure_free/unnamed-chunk-16.svg) 



```
## Error: object 'L' not found
```


![profiles calculated with the equations for a constant-pattern behavior. The solid concentration profile is sharper than the liquid concentration profile because the feed concentration is high enough to put the resin very close to its maximum capacity. deltaN is the driving force for mass transfer, and intC is an integral of C from the right.](figure_free/unnamed-chunk-18.svg) 





## Optimization
### Graphical: 2 variables
To make sure that the model has been set up to include competing costs which will keep the decision variables within ranges, first explore two variables graphically. The first variable is *cutoff*, this is outlet concentration at the end of the cycle. It is assumed that solutes in the liquid inside the vessel is also recovered. The other variable is the interstitial velocity $R$.




![For constant bed length, there is an optimal point at which to stop the loading step and move on. If the cycle is made too short, then the unused resin at the end of the column costs are too high. If the cycle is too long, the lost revenue from product which could have been recovered exceed the previous savings. Hence there is an optimal cost in the centre. Note that fraction.recovery includes FA in the liquid in the column.](figure_free/unnamed-chunk-20.svg) 


![variables impacted by flow velocity only, not the cutoff time. seconds.mtz13 is the time it takes for the concentration to rise from 1% to 99% of the inlet concentration. This decreases with increasing velocity because 1. the liquid is moving faster so that a larger area is exposed in a shorter time, and 2. the external mass transfer resistance is slightly decreased as the k/K (mass transfer coefficient in 1/s) panel shows. Finally a higher feed velocity reduces the vessel cost because the required cross-section area decreases.](figure_free/unnamed-chunk-21.svg) 


### Numerical: 4 Variables

Below the results are for an optimization that simultaneously considers 4 variables:

* feed velocity
* liquid concentration to cut-off. The optimum is the minimum allowed, which will depend on instrument limitations (not in scope). Below the minimum is assumed to be 50% of the feed concentration.
* particle diameter
* bed length

Besides the outlet concentration at which to end the cycle, all other variables at the optimum fall within the bounds set. Important limitations of the current model include:

* elution part of the model
* treatment of the hold-up in the column is an oversimplification

    * mixing of elution solvent (EtOH) with water
    * liquid hold-up is currently considered to be lost product

* detection limits may be unrealistic for deciding cycle duration. The column design may change slightly if an on-line sensor is unavailble or inaccurate, and instead the column operates with predetermined cycle times.





![keeping all other parameters besides bed length (L) constant, the optimum length can be seen as a balance between improved recovery which a larger column can provide (loss of FA is pretty much constant per cycle, so longer step times reduces the time-averaged loss) and higher costs resulting from larger equipment.](figure_free/unnamed-chunk-23.svg) 


![same data as above, but scales have equal tick spacing (k$)](figure_free/unnamed-chunk-24.svg) 


![same as above, except scales are different in each panel. The seconds/3600 panel is the hour-duration of the step.](figure_free/unnamed-chunk-25.svg) 



------------------------
 name     value    unit 
------- --------- ------
$c/c_0$ 0.5007633   1   

  $R$   0.0050000  m/s  

 $d_p$  0.0001505   m   

  $L$   0.2217881   m   
------------------------


Table: decision variables at optimum


-----------------------------------
      name          value     unit 
----------------- ---------- ------
     C.total      -3.565e+01   k$  

    C.deltaP      1.509e-01    k$  

     C.valve      1.675e+00    k$  

     C.resin      5.865e+00    k$  

    C.vessel      1.538e+01    k$  

    C.product     -5.872e+01   k$  

     deltaP       1.379e+05    Pa  

 bedvol.loading   1.861e+03    1   

       St         1.239e+00    1   

       Pe         3.441e+02    1   

   discharged     3.591e-03    kg  

       fed        5.645e-01    kg  

fraction.recovery 9.936e-01    1   

     seconds      2.476e+04    s   

      Dbed        2.200e-01    m   

      Nbed        2.000e+00    1   
-----------------------------------


Table: other variables at optimum

### Impact of particle diameter

![smaller particle diameters reduce the internal mass transfer resistance. Because a short bed length has been chosen, there is no concern over pressure drop unless the diameter is reduced 100x below the literature value of 0.635 mm.](figure_free/unnamed-chunk-28.svg) 

