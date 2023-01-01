Backlog
======
Todo
----
+ extend ```./idle``` to watch for several files
+ forecast rainfall
+ add rainfall as an additional dimension for forcast
+ add predecessing days weather data as an additional dimension for forcasts
+ provide start script for forecast
+ start script shoud pick current wether data from internet
+ boosting missing measurements, try to solve the issue of missing probability mass in the distributions

    - Probe 2: create a complete probability distribution and precomute missing values -> maybe lots of memory needed
    - each Probe is a branch

Done
----
+ automatize picking weather data from DWD

    -  pick current data from DWD site
    -  generate trainigs and validation set
+ boosting missing measurements, try to solve the issue of missing probability mass in the distributions
  ("Vicinity Sampling")

    - Probe 1: estimate missing probability mass during prediction -> maybe slow


Quickstart
==========
Getting Data
------------
in order to get data use
```console
./getdata.sh
```
Data is stored in ```data/``` folder.

Build an Run
------------
use ```./build```script.

Continous Build and Run
-----------------------

use ```./idle <file to watch>``` script. It currently only can watch for a single file.

Remarks
=======

General Idea
------------

Use historical data e.g. temperature of one or more recent days (the evidence), air pressure etc. to predict the temperature of the current day (the proposition).

From the joint evidence and proposition create a histogram (probability distribution).

The distribution can be treated as a markov chain, where for each evidence there ist a probability distribution of propositions. The predicted value is the expected value (sic average) of the  ditstribution of the propositions. 

Instead of expected value using maximal probability value or a random sample could also make sense.

Beware the distribution of joint eveidence and proposition will most probably have holes.

Vicinity Sampling
-----------------

Observation: the trainig data set has holes, means there are buckets in the histogram with probability 0.

If this is the case one can try to estimate the probability including the buckets in vicinity of the measurement.