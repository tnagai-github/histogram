# histogram

- histograms in 1d 2d, and 3d can be made quickly.
- For computational chemistry, free energy profile is also made

## how to install 

- <https://github.com/louisdx/cxx-prettyprint/> and boost are used.
- cmake is used and thus you may follow the ordinary procedure with cmake 
  - make a directory for example named build
  - `cd build`
  - `cmake  -DPRETTYPRINT=/path_to/cxx-prettyprint/ path_to/src && make `

- you will get three executables, namely, histogram1d, histogram2d, and histogram3d


## how to use

- data are supplied from the standard input and options are specified in the command line argument. 
- to make a (one-dimensional) histogram of the second column of data.txt spanning from 1 to 100 with binwidth 5, for example, execute
```
cat data.txt  | awk '{print $2}'  \
/path_to/histogram1d --max 100 --min 1 --dx 5 --fout_prefix prefix_of_filename  
```
- For computational chemists, the free energy profile, converted from the histogram, will be also created. 

- for details, check `/path_to/histogram1d  --help`

