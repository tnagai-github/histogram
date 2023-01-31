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

constexpr int DIM_HIST=3 ;

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
  double min[DIM_HIST],max[DIM_HIST],dx[DIM_HIST], T;
  string fout_prefix;
  bool bStrict;
  
  // making command line options
  po::options_description opt("generic histogram and free energy. Data should be supplemented from the standard input; the first and second columns will be analysed.");
  opt.add_options()
    ("help,h" ,                                                   "show help")
    ("dx"     ,      po::value<double>()->default_value(0.1),    "dx, width of histogram")
    ("minx"     ,    po::value<double>()->default_value(-10),    "minx of histogram")
    ("maxx"     ,    po::value<double>()->default_value(10),     "maxx of histogram")
    ("dy"     ,      po::value<double>()->default_value(0.1),    "dy, width of histogram")
    ("miny"     ,    po::value<double>()->default_value(-10),    "miny of histogram")
    ("maxy"     ,    po::value<double>()->default_value(10),     "maxy of histogram")
    ("dz"     ,      po::value<double>()->default_value(0.1),    "dz, width of histogram")
    ("minz"     ,    po::value<double>()->default_value(-10),    "minz of histogram")
    ("maxz"     ,    po::value<double>()->default_value(10),     "maxz of histogram")
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
      dx[0]  = vm["dx"].as<double>();
      min[0] = vm["minx"].as<double>();
      max[0] = vm["maxx"].as<double>();
      dx[1]  = vm["dy"].as<double>();
      min[1] = vm["miny"].as<double>();
      max[1] = vm["maxy"].as<double>();
      dx[2]  = vm["dz"].as<double>();
      min[2] = vm["minz"].as<double>();
      max[2] = vm["maxz"].as<double>();
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
  cout << "dx: " << dx[0] << endl;
  cout << "minx: " << min[0] << endl;
  cout << "maxx: " << max[0] << endl;
  cout << "dy: " << dx[1] << endl;
  cout << "miny: " << min[1] << endl;
  cout << "maxy: " << max[1] << endl;
  cout << "dz: " << dx[2] << endl;
  cout << "minz: " << min[2] << endl;
  cout << "maxz: " << max[2] << endl;
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

  for(int i=0; i<DIM_HIST; i++){
    if(max[i]<=min[i]){
      cout << boost::format("ERROR: max < min for %2d-th dimenstion") % i <<endl;
      cerr << boost::format("ERROR: max < min for %2d-th dimenstion") % i <<endl;
      cout << boost::format("I am exitting.") % i <<endl;
      cerr << boost::format("I am exitting.") % i <<endl;
      return -1;
    }
  }

  const int  nbins[DIM_HIST] = {(int)std::ceil((max[0]-min[0])/dx[0]), (int)std::ceil((max[1]-min[1])/dx[1]), (int)std::ceil((max[2]-min[2])/dx[2])};
  vector<vector<double> >  edges;
  vector< vector< vector<int>>>  counts(nbins[0],vector<vector<int>>(nbins[1],vector<int>(nbins[2],0)));
  vector< vector<vector<double>>> pdf(nbins[0],vector<vector<double>>(nbins[1],vector<double>(nbins[2], 0.0)));
  vector<vector<vector<double>>> free_energy(nbins[0],vector<vector<double>>(nbins[1],vector<double>(nbins[2], 0.0)));
  
  edges.push_back(vector<double>(nbins[0]+1,0));
  edges.push_back(vector<double>(nbins[1]+1,0));
  edges.push_back(vector<double>(nbins[2]+1,0));
  for (int i = 0 ; i < DIM_HIST; i++){
    for (int j = 0 ; j < nbins[i]+1; j++){
      edges[i][j] = min[i] + dx[i]*j ;
    }
  }


  cout << "**** parameters determined ****"  << endl;
  cout << "nbins_x: " << nbins[0] <<endl;
  cout << "nbins_y: " << nbins[1] <<endl;
  cout << "nbins_z: " << nbins[2] <<endl;
  cout << "edges_x: " << edges[0] <<endl;
  cout << "edges_y: " << edges[1] <<endl;
  cout << "edges_z: " << edges[2] <<endl;
  cout << "*******************************"  << endl;

  double val[DIM_HIST];
  string buf;
  char tmp[DIM_HIST][BUF_MAX/3];
  char* err[DIM_HIST];

  auto get_ibin = [](double val,double min, double dx){return std::floor((val-min)/dx);};
  int ibin[DIM_HIST];

  while (std::getline(std::cin, buf))
  {
    auto bsscanf = std::sscanf(buf.c_str(), "%s %s %s" , tmp[0],tmp[1] ,tmp[2] );
    if(bsscanf != DIM_HIST){
        cout << "ERROR: not enough columns\n" << "Exit!!" <<endl;
        cerr << "ERROR: not enough columns\n" << "Exit!!" <<endl;
        return -1;
    }

    for(int i=0; i<DIM_HIST; i++){
      val[i]=std::strtod(tmp[i], &err[i]);
      if(*err[i]!='\0'){
        cout << "error to convert: " << tmp[i] << "in line "<<buf <<"\nExit!!" <<endl;
        cerr << "error to convert: " << tmp[i] << "in line "<<buf <<"\nExit!!" <<endl;
        return -1;
      }
    }
    
    for(int i=0; i<DIM_HIST; i++){
      ibin[i] = get_ibin(val[i],min[i],dx[i]);
      if(val[i] < min[i]){
        cout << "min is too large: set min < " << val[i] << endl;
        cerr << "min is too large: set min < " << val[i] << endl;
        ibin[i] = 0;
        if(bStrict){
          cout << "As bStrictOutOfRange is on, abort at parsing: " << buf << endl;
          cerr << "As bStrictOutOfRange is on, abort at parsing: " << buf << endl;
          return -1;
        }
      }
      if(val[i] >= max[i]){
        cout << "max is too small: set max > " << val[i] << endl;
        cerr << "max is too small: set max > " << val[i] << endl;
        ibin[i] = nbins[i]-1;
        if(bStrict){
          cout << "As bStrictOutOfRange is on, abort at parsing: " << buf << endl;
          cerr << "As bStrictOutOfRange is on, abort at parsing: " << buf << endl;
          return -1;
        }
      }
    }
    counts[ibin[0]][ibin[1]][ibin[2]]++;
  }

  int ncounts = 0;
  for(const auto  &v1: counts){
    for(const auto &v2: v1){
      for(const auto &v3: v2){
        ncounts+=v3;
      }
    }
  } 

  cout << boost::format("In total, %d samples have been considered.") % ncounts << endl;

  for(int i=0; i < nbins[0]; i++){
    for(int j=0; j < nbins[1]; j++){
      for(int k=0; k < nbins[2]; k++){
        pdf[i][j][k] = counts[i][j][k]/double(ncounts)/(dx[0]*dx[1]*dx[2]);
        free_energy[i][j][k] = -kB_kJ_per_mol*T*std::log(pdf[i][j][k]);
      }
    }
  }

  double min_free_energy = free_energy[0][0][0];
  for(const auto  &v1: free_energy){
    for(const auto &v2: v1){
      for(const auto &v3: v2){
        min_free_energy=std::min(min_free_energy, v3);
      }
    }
  } 

  for(auto  &v1: free_energy){
    for(auto &v2: v1){
      for(auto &v3: v2){
        v3 -= min_free_energy;
      }
    }
  }

  cout << counts << endl;
  cout << free_energy << endl;


  for(int i= 0; i<nbins[0];i++){
    for(int j= 0; j<nbins[1];j++){
      for(int k= 0; k<nbins[2];k++){
        ofs_hist << boost::format("%10.5f %10.5f %10.5f %12.6g %12d\n") % (edges[0][i]+0.5*dx[0]) % (edges[1][j]+0.5*dx[1]) % (edges[2][k]+0.5*dx[2]) % pdf[i][j][k]  %counts[i][j][k] ;
        ofs_FE   << boost::format("%10.5f %10.5f %10.5f %12.6g\n")      % (edges[0][i]+0.5*dx[0]) % (edges[1][j]+0.5*dx[1]) % (edges[2][k]+0.5*dx[2]) % free_energy[i][j][k] ;
      }
    }
  }
  ofs_hist.close();
  ofs_FE.close();

  return EXIT_SUCCESS ;
}
