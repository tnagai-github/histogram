# histogram

- histograms in 1d 2d, and 3d can be made quickly in terminal.

## how to install 

- <https://github.com/louisdx/cxx-prettyprint/> and boost are used.
- cmake is used and thus you may follow the ordinary procedure with cmake 
  - make a directory for example named build
  - `mv build`
  - `cmake  -DPRETTYPRINT=/path_to/cxx-prettyprint/ path_to/src && make `

- you will get three executables, namely, histogram1d, histogram2d, and histogram3d


## how to use

- data are supplied from from standard input and options are specified in command line argument. 
- to make a histogram of the second column of data.txt, for example, execute
```
cat data.txt  | awk '{print $2}'  \
/path_to/histogram1d --max 100 --min 1 --dx 5 --fout_prefix prefix  
```

- for details, check `/path_to/histogram1d  --help`

