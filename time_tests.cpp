//#include <jemalloc/jemalloc.h>
#include <stdlib.h> // for atoi
#include <string> // for stoi
#include "parallel.h"
#include "sequence.h"
#include "get_time.h"
#include "time_operations.h"
#include "parse_command_line.h"
#include <iostream>
#include <ctype.h>
#include <math.h>
#include <limits>
#include <vector>
#include <algorithm>
#include <atomic>


size_t str_to_int(char* str) {
    return strtol(str, NULL, 10);
}

void report_time(double t, std::string name) {
  cout << name << " : " << t << endl;
}

template<typename F>
std::vector<double> repeat(size_t n, size_t rounds, bool check, F test, const std::string distribution, const std::string para) {
  if (check) test(n, distribution, para, true);
  std::vector<double> R;
  for (size_t i=0; i < rounds; i++) {
    // drop the first one and take the median of the rest 5
    if (i > 0) R.push_back(test(n, distribution, para, false));
  }
  return R;
}

template<typename F>
double reduce(std::vector<double> V, F f) {
  double x = V[0];
  for (size_t i=1; i < V.size(); i++) x = f(x,V[i]);
  return x;
}

double median(std::vector<double> V) {
  std::sort(V.begin(),V.end());
  if (V.size()%2 == 1)
    return V[V.size()/2];
  else
    return (V[V.size()/2] + V[V.size()/2 - 1])/2.0;
}

double sumf(double a, double b) {return a+ b;};
double minf(double a, double b) {return (a < b) ? a : b;};
double maxf(double a, double b) {return (a > b) ? a : b;};

bool global_check = false;

template<typename F>
bool run_multiple(size_t n, size_t rounds, float bytes_per_elt,
		  std::string name, F test, bool half_length=1, std::string x="bw",
      const std::string distribution="uniform", const std::string para="1000000") {
  std::vector<double> t = repeat(n, rounds, global_check, test, distribution, para);

  double mint = reduce(t, minf);
  double maxt = reduce(t, maxf);
  double med = median(t);
  double rate = n/mint;
  double l=n;
  double tt;
  if (half_length)
    do {
      l = round(l * .8);
      tt = reduce(repeat(l, rounds, global_check, test, distribution, para),minf);
    } while (tt != 0.0 && l/tt > rate/2 && l > 1);

  double bandwidth = rate * bytes_per_elt / 1e9;

  cout << name << std::setprecision(3)
       << ": r=" << rounds
       << ", med=" << med
       << " (" << mint << "," << maxt << "), "
       << "hlen=" << round(l) << ", "
       << x << " = " << bandwidth
       << endl;
  return 1;
}

float bytes_per_read = 1.0;
float bytes_per_write_back = .70;

// the effective number of bytes assuming assymetry
float ebytes(int reads, int write_backs) {
  return reads * bytes_per_read + write_backs * bytes_per_write_back;
}

double pick_test(size_t id, size_t n, size_t rounds, bool half_length) {
  pbbs::allocator_clear();
  
  switch (id) {
  case 0:
    cout << "Testing out-of-place and stable sample-sort ... " << endl;
    return run_multiple(n,rounds,1,"sample-sort (outplace and stable), uniform 10, pair<uint64, uint64>", t_sort_outplace_stable<uint64_t>, half_length, "Gelts/sec", "uniform", "10");
  case 1:
    return run_multiple(n,rounds,1,"sample-sort (outplace and stable), uniform 100, pair<uint64, uint64>", t_sort_outplace_stable<uint64_t>, half_length, "Gelts/sec", "uniform", "100");
  case 2:
    return run_multiple(n,rounds,1,"sample-sort (outplace and stable), uniform 1000, pair<uint64, uint64>", t_sort_outplace_stable<uint64_t>, half_length, "Gelts/sec", "uniform", "1000");
  case 3:
    return run_multiple(n,rounds,1,"sample-sort (outplace and stable), uniform 5000, pair<uint64, uint64>", t_sort_outplace_stable<uint64_t>, half_length, "Gelts/sec", "uniform", "5000");
  case 4:
    return run_multiple(n,rounds,1,"sample-sort (outplace and stable), uniform 7000, pair<uint64, uint64>", t_sort_outplace_stable<uint64_t>, half_length, "Gelts/sec", "uniform", "7000");
  case 5:
    return run_multiple(n,rounds,1,"sample-sort (outplace and stable), uniform 8000, pair<uint64, uint64>", t_sort_outplace_stable<uint64_t>, half_length, "Gelts/sec", "uniform", "8000");
  case 6:
    return run_multiple(n,rounds,1,"sample-sort (outplace and stable), uniform 10000, pair<uint64, uint64>", t_sort_outplace_stable<uint64_t>, half_length, "Gelts/sec", "uniform", "10000");
  case 7:
    return run_multiple(n,rounds,1,"sample-sort (outplace and stable), uniform 15000, pair<uint64, uint64>", t_sort_outplace_stable<uint64_t>, half_length, "Gelts/sec", "uniform", "15000");
  case 8:
    return run_multiple(n,rounds,1,"sample-sort (outplace and stable), uniform 20000, pair<uint64, uint64>", t_sort_outplace_stable<uint64_t>, half_length, "Gelts/sec", "uniform", "20000");
  case 9:
    return run_multiple(n,rounds,1,"sample-sort (outplace and stable), uniform 50000, pair<uint64, uint64>", t_sort_outplace_stable<uint64_t>, half_length, "Gelts/sec", "uniform", "50000");
  case 10:
    return run_multiple(n,rounds,1,"sample-sort (outplace and stable), uniform 100000, pair<uint64, uint64>", t_sort_outplace_stable<uint64_t>, half_length, "Gelts/sec", "uniform", "100000");
  case 11:
    return run_multiple(n,rounds,1,"sample-sort (outplace and stable), uniform 1000000, pair<uint64, uint64>", t_sort_outplace_stable<uint64_t>, half_length, "Gelts/sec", "uniform", "1000000");
  case 12:
    return run_multiple(n,rounds,1,"sample-sort (outplace and stable), uniform 10000000, pair<uint64, uint64>", t_sort_outplace_stable<uint64_t>, half_length, "Gelts/sec", "uniform", "10000000");
  case 13:
    return run_multiple(n,rounds,1,"sample-sort (outplace and stable), uniform 100000000, pair<uint64, uint64>", t_sort_outplace_stable<uint64_t>, half_length, "Gelts/sec", "uniform", "100000000");
  case 14:
    return run_multiple(n,rounds,1,"sample-sort (outplace and stable), uniform 1000000000, pair<uint64, uint64>", t_sort_outplace_stable<uint64_t>, half_length, "Gelts/sec", "uniform", "1000000000");
  case 15:
    return run_multiple(n,rounds,1,"sample-sort (outplace and stable), exponential 1, pair<uint64, uint64>", t_sort_outplace_stable<uint64_t>, half_length, "Gelts/sec", "exponential", "1");
  case 16:
    return run_multiple(n,rounds,1,"sample-sort (outplace and stable), exponential 0.001, pair<uint64, uint64>", t_sort_outplace_stable<uint64_t>, half_length, "Gelts/sec", "exponential", "0.001");
  case 17:
    return run_multiple(n,rounds,1,"sample-sort (outplace and stable), exponential 0.0003, pair<uint64, uint64>", t_sort_outplace_stable<uint64_t>, half_length, "Gelts/sec", "exponential", "0.0003");
  case 18:
    return run_multiple(n,rounds,1,"sample-sort (outplace and stable), exponential 0.0002, pair<uint64, uint64>", t_sort_outplace_stable<uint64_t>, half_length, "Gelts/sec", "exponential", "0.0002");
  case 19:
    return run_multiple(n,rounds,1,"sample-sort (outplace and stable), exponential 0.00015, pair<uint64, uint64>", t_sort_outplace_stable<uint64_t>, half_length, "Gelts/sec", "exponential", "0.00015");
  case 20:
    return run_multiple(n,rounds,1,"sample-sort (outplace and stable), exponential 0.0001, pair<uint64, uint64>", t_sort_outplace_stable<uint64_t>, half_length, "Gelts/sec", "exponential", "0.0001");
  case 21:
    return run_multiple(n,rounds,1,"sample-sort (outplace and stable), exponential 0.00001, pair<uint64, uint64>", t_sort_outplace_stable<uint64_t>, half_length, "Gelts/sec", "exponential", "0.00001");
  case 22:
    return run_multiple(n,rounds,1,"sample-sort (outplace and stable), zipfian 10000, pair<uint64, uint64>", t_sort_outplace_stable<uint64_t>, half_length, "Gelts/sec", "zipfian", "10000");
  case 23:
    return run_multiple(n,rounds,1,"sample-sort (outplace and stable), zipfian 100000, pair<uint64, uint64>", t_sort_outplace_stable<uint64_t>, half_length, "Gelts/sec", "zipfian", "100000");
  case 24:
    return run_multiple(n,rounds,1,"sample-sort (outplace and stable), zipfian 1000000, pair<uint64, uint64>", t_sort_outplace_stable<uint64_t>, half_length, "Gelts/sec", "zipfian", "1000000");
  case 25:
    return run_multiple(n,rounds,1,"sample-sort (outplace and stable), zipfian 10000000, pair<uint64, uint64>", t_sort_outplace_stable<uint64_t>, half_length, "Gelts/sec", "zipfian", "10000000");
  case 26:
    return run_multiple(n,rounds,1,"sample-sort (outplace and stable), zipfian 100000000, pair<uint64, uint64>", t_sort_outplace_stable<uint64_t>, half_length, "Gelts/sec", "zipfian", "100000000");
  case 27:
    return run_multiple(n,rounds,1,"sample-sort (outplace and stable), zipfian 1000000000, pair<uint64, uint64>", t_sort_outplace_stable<uint64_t>, half_length, "Gelts/sec", "zipfian", "1000000000");
  
  case 28:
    cout << "Testing out-of-place and unstable sample-sort ... " << endl;
    return run_multiple(n,rounds,1,"sample-sort (outplace and unstable), uniform 10, pair<uint64, uint64>", t_sort_outplace_unstable<uint64_t>, half_length, "Gelts/sec", "uniform", "10");
  case 29:
    return run_multiple(n,rounds,1,"sample-sort (outplace and unstable), uniform 100, pair<uint64, uint64>", t_sort_outplace_unstable<uint64_t>, half_length, "Gelts/sec", "uniform", "100");
  case 30:
    return run_multiple(n,rounds,1,"sample-sort (outplace and unstable), uniform 1000, pair<uint64, uint64>", t_sort_outplace_unstable<uint64_t>, half_length, "Gelts/sec", "uniform", "1000");
  case 31:
    return run_multiple(n,rounds,1,"sample-sort (outplace and unstable), uniform 5000, pair<uint64, uint64>", t_sort_outplace_unstable<uint64_t>, half_length, "Gelts/sec", "uniform", "5000");
  case 32:
    return run_multiple(n,rounds,1,"sample-sort (outplace and unstable), uniform 7000, pair<uint64, uint64>", t_sort_outplace_unstable<uint64_t>, half_length, "Gelts/sec", "uniform", "7000");
  case 33:
    return run_multiple(n,rounds,1,"sample-sort (outplace and unstable), uniform 8000, pair<uint64, uint64>", t_sort_outplace_unstable<uint64_t>, half_length, "Gelts/sec", "uniform", "8000");
  case 34:
    return run_multiple(n,rounds,1,"sample-sort (outplace and unstable), uniform 10000, pair<uint64, uint64>", t_sort_outplace_unstable<uint64_t>, half_length, "Gelts/sec", "uniform", "10000");
  case 35:
    return run_multiple(n,rounds,1,"sample-sort (outplace and unstable), uniform 15000, pair<uint64, uint64>", t_sort_outplace_unstable<uint64_t>, half_length, "Gelts/sec", "uniform", "15000");
  case 36:
    return run_multiple(n,rounds,1,"sample-sort (outplace and unstable), uniform 20000, pair<uint64, uint64>", t_sort_outplace_unstable<uint64_t>, half_length, "Gelts/sec", "uniform", "20000");
  case 37:
    return run_multiple(n,rounds,1,"sample-sort (outplace and unstable), uniform 50000, pair<uint64, uint64>", t_sort_outplace_unstable<uint64_t>, half_length, "Gelts/sec", "uniform", "50000");
  case 38:
    return run_multiple(n,rounds,1,"sample-sort (outplace and unstable), uniform 100000, pair<uint64, uint64>", t_sort_outplace_unstable<uint64_t>, half_length, "Gelts/sec", "uniform", "100000");
  case 39:
    return run_multiple(n,rounds,1,"sample-sort (outplace and unstable), uniform 1000000, pair<uint64, uint64>", t_sort_outplace_unstable<uint64_t>, half_length, "Gelts/sec", "uniform", "1000000");
  case 40:
    return run_multiple(n,rounds,1,"sample-sort (outplace and unstable), uniform 10000000, pair<uint64, uint64>", t_sort_outplace_unstable<uint64_t>, half_length, "Gelts/sec", "uniform", "10000000");
  case 41:
    return run_multiple(n,rounds,1,"sample-sort (outplace and unstable), uniform 100000000, pair<uint64, uint64>", t_sort_outplace_unstable<uint64_t>, half_length, "Gelts/sec", "uniform", "100000000");
  case 42:
    return run_multiple(n,rounds,1,"sample-sort (outplace and unstable), uniform 1000000000, pair<uint64, uint64>", t_sort_outplace_unstable<uint64_t>, half_length, "Gelts/sec", "uniform", "1000000000");
  case 43:
    return run_multiple(n,rounds,1,"sample-sort (outplace and unstable), exponential 1, pair<uint64, uint64>", t_sort_outplace_unstable<uint64_t>, half_length, "Gelts/sec", "exponential", "1");
  case 44:
    return run_multiple(n,rounds,1,"sample-sort (outplace and unstable), exponential 0.001, pair<uint64, uint64>", t_sort_outplace_unstable<uint64_t>, half_length, "Gelts/sec", "exponential", "0.001");
  case 45:
    return run_multiple(n,rounds,1,"sample-sort (outplace and unstable), exponential 0.0003, pair<uint64, uint64>", t_sort_outplace_unstable<uint64_t>, half_length, "Gelts/sec", "exponential", "0.0003");
  case 46:
    return run_multiple(n,rounds,1,"sample-sort (outplace and unstable), exponential 0.0002, pair<uint64, uint64>", t_sort_outplace_unstable<uint64_t>, half_length, "Gelts/sec", "exponential", "0.0002");
  case 47:
    return run_multiple(n,rounds,1,"sample-sort (outplace and unstable), exponential 0.00015, pair<uint64, uint64>", t_sort_outplace_unstable<uint64_t>, half_length, "Gelts/sec", "exponential", "0.00015");
  case 48:
    return run_multiple(n,rounds,1,"sample-sort (outplace and unstable), exponential 0.0001, pair<uint64, uint64>", t_sort_outplace_unstable<uint64_t>, half_length, "Gelts/sec", "exponential", "0.0001");
  case 49:
    return run_multiple(n,rounds,1,"sample-sort (outplace and unstable), exponential 0.00001, pair<uint64, uint64>", t_sort_outplace_unstable<uint64_t>, half_length, "Gelts/sec", "exponential", "0.00001");
  case 50:
    return run_multiple(n,rounds,1,"sample-sort (outplace and unstable), zipfian 10000, pair<uint64, uint64>", t_sort_outplace_unstable<uint64_t>, half_length, "Gelts/sec", "zipfian", "10000");
  case 51:
    return run_multiple(n,rounds,1,"sample-sort (outplace and unstable), zipfian 100000, pair<uint64, uint64>", t_sort_outplace_unstable<uint64_t>, half_length, "Gelts/sec", "zipfian", "100000");
  case 52:
    return run_multiple(n,rounds,1,"sample-sort (outplace and unstable), zipfian 1000000, pair<uint64, uint64>", t_sort_outplace_unstable<uint64_t>, half_length, "Gelts/sec", "zipfian", "1000000");
  case 53:
    return run_multiple(n,rounds,1,"sample-sort (outplace and unstable), zipfian 10000000, pair<uint64, uint64>", t_sort_outplace_unstable<uint64_t>, half_length, "Gelts/sec", "zipfian", "10000000");
  case 54:
    return run_multiple(n,rounds,1,"sample-sort (outplace and unstable), zipfian 100000000, pair<uint64, uint64>", t_sort_outplace_unstable<uint64_t>, half_length, "Gelts/sec", "zipfian", "100000000");
  case 55:
    return run_multiple(n,rounds,1,"sample-sort (outplace and unstable), zipfian 1000000000, pair<uint64, uint64>", t_sort_outplace_unstable<uint64_t>, half_length, "Gelts/sec", "zipfian", "1000000000");
  
  case 56:
    cout << "Testing in-place and stable sample-sort ... " << endl;
    return run_multiple(n,rounds,1,"sample-sort (inplace and stable), uniform 10, pair<uint64, uint64>", t_sort_inplace_stable<uint64_t>, half_length, "Gelts/sec", "uniform", "10");
  case 57:
    return run_multiple(n,rounds,1,"sample-sort (inplace and stable), uniform 100, pair<uint64, uint64>", t_sort_inplace_stable<uint64_t>, half_length, "Gelts/sec", "uniform", "100");
  case 58:
    return run_multiple(n,rounds,1,"sample-sort (inplace and stable), uniform 1000, pair<uint64, uint64>", t_sort_inplace_stable<uint64_t>, half_length, "Gelts/sec", "uniform", "1000");
  case 59:
    return run_multiple(n,rounds,1,"sample-sort (inplace and stable), uniform 5000, pair<uint64, uint64>", t_sort_inplace_stable<uint64_t>, half_length, "Gelts/sec", "uniform", "5000");
  case 60:
    return run_multiple(n,rounds,1,"sample-sort (inplace and stable), uniform 7000, pair<uint64, uint64>", t_sort_inplace_stable<uint64_t>, half_length, "Gelts/sec", "uniform", "7000");
  case 61:
    return run_multiple(n,rounds,1,"sample-sort (inplace and stable), uniform 8000, pair<uint64, uint64>", t_sort_inplace_stable<uint64_t>, half_length, "Gelts/sec", "uniform", "8000");
  case 62:
    return run_multiple(n,rounds,1,"sample-sort (inplace and stable), uniform 10000, pair<uint64, uint64>", t_sort_inplace_stable<uint64_t>, half_length, "Gelts/sec", "uniform", "10000");
  case 63:
    return run_multiple(n,rounds,1,"sample-sort (inplace and stable), uniform 15000, pair<uint64, uint64>", t_sort_inplace_stable<uint64_t>, half_length, "Gelts/sec", "uniform", "15000");
  case 64: 
    return run_multiple(n,rounds,1,"sample-sort (inplace and stable), uniform 20000, pair<uint64, uint64>", t_sort_inplace_stable<uint64_t>, half_length, "Gelts/sec", "uniform", "20000");
  case 65:
    return run_multiple(n,rounds,1,"sample-sort (inplace and stable), uniform 50000, pair<uint64, uint64>", t_sort_inplace_stable<uint64_t>, half_length, "Gelts/sec", "uniform", "50000");
  case 66:
    return run_multiple(n,rounds,1,"sample-sort (inplace and stable), uniform 100000, pair<uint64, uint64>", t_sort_inplace_stable<uint64_t>, half_length, "Gelts/sec", "uniform", "100000");
  case 67:
    return run_multiple(n,rounds,1,"sample-sort (inplace and stable), uniform 1000000, pair<uint64, uint64>", t_sort_inplace_stable<uint64_t>, half_length, "Gelts/sec", "uniform", "1000000");
  case 68:
    return run_multiple(n,rounds,1,"sample-sort (inplace and stable), uniform 10000000, pair<uint64, uint64>", t_sort_inplace_stable<uint64_t>, half_length, "Gelts/sec", "uniform", "10000000");
  case 69:
    return run_multiple(n,rounds,1,"sample-sort (inplace and stable), uniform 100000000, pair<uint64, uint64>", t_sort_inplace_stable<uint64_t>, half_length, "Gelts/sec", "uniform", "100000000");
  case 70:
    return run_multiple(n,rounds,1,"sample-sort (inplace and stable), uniform 1000000000, pair<uint64, uint64>", t_sort_inplace_stable<uint64_t>, half_length, "Gelts/sec", "uniform", "1000000000");
  case 71:
    return run_multiple(n,rounds,1,"sample-sort (inplace and stable), exponential 1, pair<uint64, uint64>", t_sort_inplace_stable<uint64_t>, half_length, "Gelts/sec", "exponential", "1");
  case 72:
    return run_multiple(n,rounds,1,"sample-sort (inplace and stable), exponential 0.001, pair<uint64, uint64>", t_sort_inplace_stable<uint64_t>, half_length, "Gelts/sec", "exponential", "0.001");
  case 73:
    return run_multiple(n,rounds,1,"sample-sort (inplace and stable), exponential 0.0003, pair<uint64, uint64>", t_sort_inplace_stable<uint64_t>, half_length, "Gelts/sec", "exponential", "0.0003");
  case 74:
    return run_multiple(n,rounds,1,"sample-sort (inplace and stable), exponential 0.0002, pair<uint64, uint64>", t_sort_inplace_stable<uint64_t>, half_length, "Gelts/sec", "exponential", "0.0002");
  case 75:
    return run_multiple(n,rounds,1,"sample-sort (inplace and stable), exponential 0.00015, pair<uint64, uint64>", t_sort_inplace_stable<uint64_t>, half_length, "Gelts/sec", "exponential", "0.00015");
  case 76:
    return run_multiple(n,rounds,1,"sample-sort (inplace and stable), exponential 0.0001, pair<uint64, uint64>", t_sort_inplace_stable<uint64_t>, half_length, "Gelts/sec", "exponential", "0.0001");
  case 77:
    return run_multiple(n,rounds,1,"sample-sort (inplace and stable), exponential 0.00001, pair<uint64, uint64>", t_sort_inplace_stable<uint64_t>, half_length, "Gelts/sec", "exponential", "0.00001");
  case 78:
    return run_multiple(n,rounds,1,"sample-sort (inplace and stable), zipfian 10000, pair<uint64, uint64>", t_sort_inplace_stable<uint64_t>, half_length, "Gelts/sec", "zipfian", "10000");
  case 79:
    return run_multiple(n,rounds,1,"sample-sort (inplace and stable), zipfian 100000, pair<uint64, uint64>", t_sort_inplace_stable<uint64_t>, half_length, "Gelts/sec", "zipfian", "100000");
  case 80:
    return run_multiple(n,rounds,1,"sample-sort (inplace and stable), zipfian 1000000, pair<uint64, uint64>", t_sort_inplace_stable<uint64_t>, half_length, "Gelts/sec", "zipfian", "1000000");
  case 81:
    return run_multiple(n,rounds,1,"sample-sort (inplace and stable), zipfian 10000000, pair<uint64, uint64>", t_sort_inplace_stable<uint64_t>, half_length, "Gelts/sec", "zipfian", "10000000");
  case 82:
    return run_multiple(n,rounds,1,"sample-sort (inplace and stable), zipfian 100000000, pair<uint64, uint64>", t_sort_inplace_stable<uint64_t>, half_length, "Gelts/sec", "zipfian", "100000000");
  case 83:
    return run_multiple(n,rounds,1,"sample-sort (inplace and stable), zipfian 1000000000, pair<uint64, uint64>", t_sort_inplace_stable<uint64_t>, half_length, "Gelts/sec", "zipfian", "1000000000");
  
  case 84:
    cout << "Testing in-place and unstable sample-sort ... " << endl;
    return run_multiple(n,rounds,1,"sample-sort (inplace and unstable), uniform 10, pair<uint64, uint64>", t_sort_inplace_unstable<uint64_t>, half_length, "Gelts/sec", "uniform", "10");
  case 85:
    return run_multiple(n,rounds,1,"sample-sort (inplace and unstable), uniform 100, pair<uint64, uint64>", t_sort_inplace_unstable<uint64_t>, half_length, "Gelts/sec", "uniform", "100");
  case 86:
    return run_multiple(n,rounds,1,"sample-sort (inplace and unstable), uniform 1000, pair<uint64, uint64>", t_sort_inplace_unstable<uint64_t>, half_length, "Gelts/sec", "uniform", "1000");
  case 87:
    return run_multiple(n,rounds,1,"sample-sort (inplace and unstable), uniform 5000, pair<uint64, uint64>", t_sort_inplace_unstable<uint64_t>, half_length, "Gelts/sec", "uniform", "5000");
  case 88:
    return run_multiple(n,rounds,1,"sample-sort (inplace and unstable), uniform 7000, pair<uint64, uint64>", t_sort_inplace_unstable<uint64_t>, half_length, "Gelts/sec", "uniform", "7000");
  case 89:
    return run_multiple(n,rounds,1,"sample-sort (inplace and unstable), uniform 8000, pair<uint64, uint64>", t_sort_inplace_unstable<uint64_t>, half_length, "Gelts/sec", "uniform", "8000");
  case 90:
    return run_multiple(n,rounds,1,"sample-sort (inplace and unstable), uniform 10000, pair<uint64, uint64>", t_sort_inplace_unstable<uint64_t>, half_length, "Gelts/sec", "uniform", "10000");
  case 91:
    return run_multiple(n,rounds,1,"sample-sort (inplace and unstable), uniform 15000, pair<uint64, uint64>", t_sort_inplace_unstable<uint64_t>, half_length, "Gelts/sec", "uniform", "15000");
  case 92: 
    return run_multiple(n,rounds,1,"sample-sort (inplace and unstable), uniform 20000, pair<uint64, uint64>", t_sort_inplace_unstable<uint64_t>, half_length, "Gelts/sec", "uniform", "20000");
  case 93:
    return run_multiple(n,rounds,1,"sample-sort (inplace and unstable), uniform 50000, pair<uint64, uint64>", t_sort_inplace_unstable<uint64_t>, half_length, "Gelts/sec", "uniform", "50000");
  case 94:
    return run_multiple(n,rounds,1,"sample-sort (inplace and unstable), uniform 100000, pair<uint64, uint64>", t_sort_inplace_unstable<uint64_t>, half_length, "Gelts/sec", "uniform", "100000");
  case 95:
    return run_multiple(n,rounds,1,"sample-sort (inplace and unstable), uniform 1000000, pair<uint64, uint64>", t_sort_inplace_unstable<uint64_t>, half_length, "Gelts/sec", "uniform", "1000000");
  case 96:
    return run_multiple(n,rounds,1,"sample-sort (inplace and unstable), uniform 10000000, pair<uint64, uint64>", t_sort_inplace_unstable<uint64_t>, half_length, "Gelts/sec", "uniform", "10000000");
  case 97:
    return run_multiple(n,rounds,1,"sample-sort (inplace and unstable), uniform 100000000, pair<uint64, uint64>", t_sort_inplace_unstable<uint64_t>, half_length, "Gelts/sec", "uniform", "100000000");
  case 98:
    return run_multiple(n,rounds,1,"sample-sort (inplace and unstable), uniform 1000000000, pair<uint64, uint64>", t_sort_inplace_unstable<uint64_t>, half_length, "Gelts/sec", "uniform", "1000000000");
  case 99:
    return run_multiple(n,rounds,1,"sample-sort (inplace and unstable), exponential 1, pair<uint64, uint64>", t_sort_inplace_unstable<uint64_t>, half_length, "Gelts/sec", "exponential", "1");
  case 100:
    return run_multiple(n,rounds,1,"sample-sort (inplace and unstable), exponential 0.001, pair<uint64, uint64>", t_sort_inplace_unstable<uint64_t>, half_length, "Gelts/sec", "exponential", "0.001");
  case 101:
    return run_multiple(n,rounds,1,"sample-sort (inplace and unstable), exponential 0.0003, pair<uint64, uint64>", t_sort_inplace_unstable<uint64_t>, half_length, "Gelts/sec", "exponential", "0.0003");
  case 102:
    return run_multiple(n,rounds,1,"sample-sort (inplace and unstable), exponential 0.0002, pair<uint64, uint64>", t_sort_inplace_unstable<uint64_t>, half_length, "Gelts/sec", "exponential", "0.0002");
  case 103:
    return run_multiple(n,rounds,1,"sample-sort (inplace and unstable), exponential 0.00015, pair<uint64, uint64>", t_sort_inplace_unstable<uint64_t>, half_length, "Gelts/sec", "exponential", "0.00015");
  case 104:
    return run_multiple(n,rounds,1,"sample-sort (inplace and unstable), exponential 0.0001, pair<uint64, uint64>", t_sort_inplace_unstable<uint64_t>, half_length, "Gelts/sec", "exponential", "0.0001");
  case 105:
    return run_multiple(n,rounds,1,"sample-sort (inplace and unstable), exponential 0.00001, pair<uint64, uint64>", t_sort_inplace_unstable<uint64_t>, half_length, "Gelts/sec", "exponential", "0.00001");
  case 106:
    return run_multiple(n,rounds,1,"sample-sort (inplace and unstable), zipfian 10000, pair<uint64, uint64>", t_sort_inplace_unstable<uint64_t>, half_length, "Gelts/sec", "zipfian", "10000");
  case 107:
    return run_multiple(n,rounds,1,"sample-sort (inplace and unstable), zipfian 100000, pair<uint64, uint64>", t_sort_inplace_unstable<uint64_t>, half_length, "Gelts/sec", "zipfian", "100000");
  case 108:
    return run_multiple(n,rounds,1,"sample-sort (inplace and unstable), zipfian 1000000, pair<uint64, uint64>", t_sort_inplace_unstable<uint64_t>, half_length, "Gelts/sec", "zipfian", "1000000");
  case 109:
    return run_multiple(n,rounds,1,"sample-sort (inplace and unstable), zipfian 10000000, pair<uint64, uint64>", t_sort_inplace_unstable<uint64_t>, half_length, "Gelts/sec", "zipfian", "10000000");
  case 110:
    return run_multiple(n,rounds,1,"sample-sort (inplace and unstable), zipfian 100000000, pair<uint64, uint64>", t_sort_inplace_unstable<uint64_t>, half_length, "Gelts/sec", "zipfian", "100000000");
  case 111:
    return run_multiple(n,rounds,1,"sample-sort (inplace and unstable), zipfian 1000000000, pair<uint64, uint64>", t_sort_inplace_unstable<uint64_t>, half_length, "Gelts/sec", "zipfian", "1000000000");
  

  /*
  case 0:
    return run_multiple(n,rounds,ebytes(16,8),"map long", t_map<long>, half_length);
  case 1:
    return run_multiple(n,rounds,ebytes(8,8),"tabulate long",t_tabulate<long>, half_length);
  case 2:
    return run_multiple(n,rounds,ebytes(8,0),"reduce add long", t_reduce_add<long>, half_length);
  case 3:
    return run_multiple(n,rounds,ebytes(24,8),"scan add long", t_scan_add<long>, half_length);
  case 4:
    return run_multiple(n,rounds,ebytes(14,4),"pack long", t_pack<long>, half_length);
  case 5:
    return run_multiple(n,rounds,ebytes(80,8),"gather long", t_gather<long>, half_length);
  case 6:
    return run_multiple(n,rounds,ebytes(72,64),"scatter long", t_scatter<long>, half_length);
  case 7:
    return run_multiple(n,rounds,ebytes(72,64),"write add long", t_write_add<long>, half_length);
  case 8:
    return run_multiple(n,rounds,ebytes(72,64),"write min long", t_write_min<long>, half_length);
  case 9:
    return run_multiple(n,rounds,1,"count sort 8bit long", t_count_sort_8<long>, half_length, "Gelts/sec");
  case 10:
    return run_multiple(n,rounds,1,"random shuffle long", t_shuffle<long>, half_length, "Gelts/sec");
  case 11:
    return run_multiple(n,rounds,1,"histogram uint", t_histogram<uint>, half_length, "Gelts/sec");
  case 12:
    return run_multiple(n,rounds,1,"histogram same uint", t_histogram_same<uint>, half_length, "Gelts/sec");
  case 13:
    return run_multiple(n,rounds,1,"histogram few uint", t_histogram_few<uint>, half_length, "Gelts/sec");
  case 14:
    return run_multiple(n,rounds,1,"integer sort<uint,uint>", t_integer_sort_pair<uint>, half_length, "Gelts/sec");
  case 15:
    return run_multiple(n,rounds,1,"integer sort uint", t_integer_sort<uint>, half_length, "Gelts/sec");
  case 16:
    return run_multiple(n,rounds,1,"integer sort 128 bits", t_integer_sort_128, half_length, "Gelts/sec");
  case 17:
    return run_multiple(n,rounds,1,"sort long", t_sort<long>, half_length, "Gelts/sec");
  case 18:
    return run_multiple(n,rounds,1,"sort uint", t_sort<uint>, half_length, "Gelts/sec");
  case 19:
    return run_multiple(n,rounds,1,"sort 128 bits", t_sort<__int128>, half_length, "Gelts/sec");
  case 20:
    return run_multiple(n,rounds,ebytes(16,8),"merge long", t_merge<long>, half_length);
  case 21:
    return run_multiple(n,rounds,ebytes(16 + 5 * 80, 8),"mat vect mult", t_mat_vec_mult<size_t,double>, half_length);
  case 22:
    return run_multiple(n,rounds,ebytes(68,64),"scatter int", t_scatter<uint>, half_length);
  case 23:
    return run_multiple(n,rounds,1,"merge sort long", t_merge_sort<long>, half_length, "Gelts/sec");
  case 24:
    return run_multiple(n,rounds,1,"count sort 2bit long", t_count_sort_2<long>, half_length, "Gelts/sec");
  case 25:
    return run_multiple(n,rounds,ebytes(24,8),"split3 long", t_split3<long>, half_length);
  case 26:
    return run_multiple(n,rounds,1,"quicksort long", t_quicksort<long>, half_length, "Gelts/sec");
  case 27:
    return run_multiple(n,rounds,1,"collect reduce 256 buckets uint", t_collect_reduce_8<uint>, half_length,"Gelts/sec");
  case 28:
    return run_multiple(n,rounds,ebytes(64,0),"strided read, 128 bytes", t_map_reduce_128, half_length);
  case 29:
    return run_multiple(n,rounds,1,"collect reduce sparse uint", t_collect_reduce_pair_sparse<uint>, half_length, "Gelts/sec");
  case 30:
    return run_multiple(n,rounds,1,"remove duplicates", t_remove_duplicates<long>, half_length, "Gelts/sec");
  case 31:
    return run_multiple(n,rounds,1,"add to bag long", t_bag<long>, half_length, "Gelts/sec");
  case 32:
    return run_multiple(n,rounds,1,"collect reduce dense uint", t_collect_reduce_pair_dense<uint>, half_length, "Gelts/sec");
  case 33:
    return run_multiple(n,rounds,ebytes(4,0),"find mid long", t_find_mid<long>, half_length);



    // these are not part of standard suite
  case 50:
    return run_multiple(n,rounds,1,"histogram reducer", t_histogram_reducer, half_length, "Gelts/sec");
  case 51:
    return run_multiple(n,rounds,ebytes(24,8),"scan add long seq", t_scan_add_seq<long>, half_length);
  case 52:
    return run_multiple(n,rounds,1, "range_min long", t_range_min<long>, half_length, "Gelts/sec");
  */
  default:
    assert(false);
    return 0.0 ;
  }
}

int main (int argc, char *argv[]) {
  commandLine P(argc, argv,
		"[-n <size>] [-r <rounds>] [-halflen] [-t <testid>]");
  size_t n = P.getOptionLongValue("-n", 1000000000);
  int rounds = P.getOptionIntValue("-r", 6);
  int test_num = P.getOptionIntValue("-t", -1);
  bool half_length = P.getOption("-halflen");
  global_check = P.getOption("-check");
  int num_tests = 112;//33;

  cout << "n = " << n << endl;
  cout << "rounds = " << rounds << endl;
  cout << "num threads = " << num_workers() << endl;
  if (half_length) cout << "half length on" << endl;
  else cout << "half length off" << endl;

  if (test_num == -1)
    for (int i=0; i < num_tests; i++)
      pick_test(i,n,rounds,half_length);
  else pick_test(test_num,n,rounds,half_length);
  //my_mem_pool.sizes();
}
