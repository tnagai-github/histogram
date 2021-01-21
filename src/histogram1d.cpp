/* 
 * Copyright (c) 2021, Tetsuro Nagai
 */

#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <numeric>
#include <cmath>
#include <cstring>
#include <cassert>
#include <string>
#include <iomanip>
#include <boost/program_options.hpp>
#include <boost/format.hpp>
#include <prettyprint.hpp>
#include "time_string.hpp"

constexpr double kB=1.380649e-23  ;
constexpr double NA=6.02214076e23 ;
constexpr double kB_kJ_per_mol=kB*NA/1000 ;
constexpr int BUF_MAX=100000;

constexpr int DIM_HIST=1 ;

using std::string;
using std::vector;
using std::array;
using std::cout;
using std::cerr;
using std::endl;
namespace po=boost::program_options;


int main(int argc, char *argv[])
{

  string starting_date;
  misc::put_string_of_time(starting_date);
  cout << "Execution start at " << starting_date <<endl;
  
  //get command line argments
  double min,max,dx, T;
  string fout_prefix;
  bool bStrict;
  
  // making command line options
  po::options_description opt("generic histogram and free energy. Data should be supplemented from the standard input; the first column will be analysed.");
  opt.add_options()
    ("help,h" ,                                                   "show help")
    ("dx"     ,      po::value<double>()->default_value(0.1),    "dx, width of histogram")
    ("min"     ,     po::value<double>()->default_value(-10),    "min of histogram")
    ("max"     ,     po::value<double>()->default_value(10),     "max of histogram")
    ("T"     ,       po::value<double>()->default_value(300),     "temperature for free energy convertion")
    ("bStrictOutOfRange"   , po::value<bool>()->default_value(false),     "if true, abort when a sample out of min and max is found")
    ("fout_prefix",  po::value<string>(),                         "fout_prefix, a number of files will be made with this prefix");
  
  // analyze argc and argv and results are stored in vm
  try{
    po::variables_map vm;
    store(parse_command_line(argc, argv, opt), vm);
    notify(vm);
    if(vm.count("help")){
      cout << opt << endl; // show help
      exit(1);
    }
    else if(!vm.count("fout_prefix")){
      cerr << vm.count("fout_prefix") << endl;
      cerr << "fout_prefix is mandatory " << endl;
      cerr << "exit!!" << endl;
      exit(1);
    }
    else
    {
      dx = vm["dx"].as<double>();
      min = vm["min"].as<double>();
      max = vm["max"].as<double>();
      T = vm["T"].as<double>();
      bStrict = vm["bStrictOutOfRange"].as<bool>();
      fout_prefix = vm["fout_prefix"].as<string>();
		}
	}	
  catch (boost::bad_any_cast &e) {
    cout << e.what() << endl;
  	cout <<"something wrong and buggy happend!!"  << endl;
  	cout <<"exit!!"  << endl;
  	exit(1);
  }
  catch (std::exception  &e) {
    cout << e.what() << endl;
    cout <<"exit!!"  << endl;
    exit(2);
  }

  cout << "**** Input parameters ****"  << endl;
  cout << "dx: " << dx << endl;
  cout << "min: " << min << endl;
  cout << "max: " << max << endl;
  cout << "T: " << T << endl;
  cout << "bStrictOutOfRange: " << bStrict << endl;
  cout << "fout_prefix: " << fout_prefix << endl;
  cout << "*************************\n"  << endl;

	const string fname_hist = fout_prefix+"_hist.dat";
	const string fname_FE   = fout_prefix+"_free_energy.dat";

  cout << "Files to be created: " << fname_hist << endl;
  cout << "Files to be created: " << fname_FE  << endl;

  std::ofstream ofs_hist(fname_hist.c_str());
  std::ofstream ofs_FE(fname_FE.c_str());

  if(!ofs_hist){
    cerr << "cannot open " << fname_hist << endl;
    return -1;
  }
  if(!ofs_FE){
    cerr << "cannot open " << fname_FE << endl;
    return -1;
  }


  if(max<=min){
    cout << "ERROR: max < min" <<endl;
    cerr << "ERROR: max < min" <<endl;
    return -1;
  }

  const int  nbins = std::ceil((max-min)/dx);
  vector<double>  edges(nbins+1,0);
  vector<int>     counts(nbins);
  vector<double>  pdf(nbins);
  vector<double>  free_energy(nbins);
  
  for (int i = 0 ; i < nbins+1; i++){
    edges[i] = min + dx*i ;
  }


  cout << "**** parameters determined ****"  << endl;
  cout << "nbins: " << nbins <<endl;
  cout << "edges: " << edges <<endl;
  cout << "*******************************"  << endl;

  double val;
  string buf;
  char tmp[BUF_MAX];
  char* err;

  auto get_ibin = [](double val,double min, double dx){return std::floor((val-min)/dx);};
  int ibin;

  while (std::getline(std::cin, buf))
  {
    auto bsscanf = std::sscanf(buf.c_str(), "%s" , tmp );
    if(bsscanf != DIM_HIST){
        cout << "ERROR: not enough columns\n" << "Exit!!" <<endl;
        cerr << "ERROR: not enough columns\n" << "Exit!!" <<endl;
        return -1;
    }


    val=std::strtod(tmp, &err);
    if(*err!='\0'){
      cout << "ERROR: error to convert: " << tmp << "\nExit!!" <<endl;
      cerr << "ERROR: error to convert: " << tmp << "\nExit!!" <<endl;
      //continue;
      return -1;
    }
    
    ibin = get_ibin(val, min, dx);
    if(val < min){
      cout << "min is too large: set min such that min < " << val << endl;
      cerr << "min is too large: set min such that min < " << val << endl;
      ibin = 0;
      if(bStrict){
        cout << "Abort at parsing: " << buf << endl;
        cerr << "Abort at parsing: " << buf << endl;
        return -1;
      }
    }
    if(val >= max){
      cout << "max is too small: set max such that max> " << val << endl;
      cerr << "max is too small: set max such that max> " << val << endl;
      ibin = nbins-1;
      if(bStrict){
        cout << "As bStrictOutOfRange is on, abort at parsing: " << buf << endl;
        cerr << "As bStrictOutOfRange is on, abort at parsing: " << buf << endl;
        return -1;
      }
    }
    counts[ibin]++;
  }

  int ncounts = std::accumulate(counts.begin(), counts.end(), 0);
  cout << boost::format("In total, %d samples have been considered.") % ncounts << endl;

  for(int i=0; i < nbins; i++){
    pdf[i] = counts[i]/double(ncounts)/dx;
    free_energy[i] = -kB_kJ_per_mol*T*std::log(pdf[i]);
  }

  double min_free_energy = *std::min_element(free_energy.begin(),free_energy.end());

  for(auto  &v1: free_energy){
    v1 -= min_free_energy;
  }

  cout << counts << endl;
  cout << free_energy << endl;


  for(int i= 0; i<nbins;i++){
  ofs_hist << boost::format("%10.5f %12.6g %12d\n") % (edges[i]+0.5*dx) % pdf[i]  %counts[i] ;
  ofs_FE << boost::format("%10.5f %12.6g\n") % (edges[i]+0.5*dx) % free_energy[i] ;
  }
  ofs_hist.close();
  ofs_FE.close();

  return EXIT_SUCCESS ;
}
