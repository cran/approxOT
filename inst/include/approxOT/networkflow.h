#ifndef NETWORKFLOW_H
#define NETWORKFLOW_H
// from https://github.com/cran/transport/blob/f2cea92550cc15634a38e8163ebc68fc30ec6816/src/networksimplex.cpp
#include "../approxOT_types.h"
#ifdef _OPENMP
# include <omp.h>
#endif
#include <iostream>
#include <vector>
#include "network_simplex_simple.h"
#include <stdio.h>

using namespace Rcpp;
using namespace std;
using namespace lemon;

static inline void trans_networkflow(const Eigen::VectorXd & a, const Eigen::VectorXd & b, const matrix & C, 
                       matrix & Tplan, int threads, bool acc, int niter){
#ifdef _OPENMP
  omp_set_num_threads(threads); //check whether this still causes trouble for people without openmp
#endif
  struct TsFlow {
    int from, to;
    double amount;
  };
  typedef FullBipartiteDigraph Digraph;
  //DIGRAPH_TYPEDEFS(FullBipartiteDigraph);
  
  
  int64_t n1 = a.rows();
  int64_t n2 = b.rows(); 
  std::vector<double> weights1(n1), weights2(n2);
  size_t maxiter = niter;
  
  Digraph di(n1, n2);
  NetworkSimplexSimple<Digraph, double, double, long long> net(di, acc, n1 + n2, n1*n2, maxiter); 
  /// \param acc Indicate if the arcs have to be stored in a
  /// mixed order in the internal data structure.
  /// In special cases, it could lead to better overall performance,
  /// but it is usually slower. Therefore it is disabled by default.
  // (const GR& graph, bool arc_mixing, int nbnodes, ArcsType nb_arcs, size_t maxiters = 0)
  int64_t idarc = 0;
  for (int i = 0; i < n1; i++) {
    for (int j = 0; j < n2; j++) {
      double d =C(i,j);
      net.setCost(di.arcFromId(idarc), d);
      idarc++;
    }
  }
  
  for (int i = 0; i < n1; i++) {
    weights1[di.nodeFromId(i)] = a(i,0);
  }
  for (int i = 0; i < n2; i++) {
    weights2[di.nodeFromId(i)] = (-1)*b(i,0);
  }
  net.supplyMap(&weights1[0], n1, &weights2[0], n2);
  net.run();
  // double resultdist = net.totalCost(); 
  
  std::vector<TsFlow> flow;
  flow.reserve(n1 + n2 - 1);
  // Eigen::MatrixXd Tplan=Eigen::MatrixXd::Constant(n1,n2,  0);
  // Eigen::MatrixXd Tframe=Eigen::MatrixXd::Constant(n1*n2,3,  0);
  // Eigen::MatrixXd Tpot=Eigen::MatrixXd::Constant(n1+n2,1,  0);
  // int count=0;
  for (int64_t i = 0; i < n1; i++) {
    for (int64_t j = 0; j < n2; j++)
    {
      TsFlow f;
      f.amount = net.flow(di.arcFromId(i*n2 + j));
      Tplan(i,j)=f.amount;
      // Tframe(count,0)=i+1;
      // Tframe(count,1)=j+1;
      // Tframe(count,2)=f.amount;
      // count+=1;
    }
  }
  // for (int64_t i = 0; i < (n1); i++) {
  //   Tpot(i,0)=(-1)*net.potential(i);
  // }
  // for (int64_t i = n1; i < (n1+n2); i++) {
  //   Tpot(i,0)=net.potential(i);
  // }
  // return  Rcpp::List::create(Rcpp::Named("dist")=resultdist,Rcpp::Named("plan")=Tplan,Rcpp::Named("frame")=Tframe,Rcpp::Named("potential")=Tpot);
  // return  Tplan;
  if((1.0 - Tplan.sum()) > 1e-5) Rcpp::warning("networkflow hasn't found feasible solution yet. increase number of iterations!");
}

#endif //SHORTSIMPLEX_H
