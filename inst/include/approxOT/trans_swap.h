#ifndef TRANS_SWAP_H
#define TRANS_SWAP_H

#include "../approxOT_types.h"
#include "cost.h"


template<typename Derived> double wass_cost_p(const Eigen::MatrixBase<Derived>& A, const Eigen::MatrixBase<Derived>& B, double p, double ground_p) {
  double cost = std::pow((A - B).array().pow(ground_p).sum(), 1.0/ground_p);
  return( std::pow(cost, p) );
}

//' Transport plan based on swapping
//'
//' @param A An Eigen::MatrixXd of the data in sample A
//' @param B An Eigen::MatrixXd of the data in sample B
//' @param N The columns of A
//' @param M The columns of B
//' @param idx A two column Eigen::MatrixXi giving the paired
//' indexes between samples.
//' @param mass An Eigen::VectorXd giving the mass between pairs
//' of observations.
//' @param ground_p The power for the Lp norm (double >=1)
//' @param p The power for the Wasserstein distance
//' @param tol The tolerance to use for stopping
//' @param niter The number of iterations
//' @return void
//' @keywords internal
 static inline void trans_swap(const matrix & A, const matrix & B, int N, int M,
                 matrixI & idx, Eigen::VectorXd &  mass, double ground_p,
                 double p, double tol, int niter)
 {
   int perm_i, perm_j;
   double cost = 0.0, previous_cost = 0.0;
   double proposed_cost = 0.0, cur_cost = 0.0;
   if(N != M) {
     Rcpp::stop("Number of atoms of A and B must match for current implementation of swapping distance!");
   }
   
   for(int i = 0; i < idx.rows(); i ++) {
     cost += wass_cost_p(A.col(idx(i,0)), B.col(idx(i,1)), p, ground_p) * mass(i);
   }
   previous_cost = cost;
   
   for(int iter = 0; iter < niter; iter++) {
     // Rcpp::Rcout << iter << ", " << std::endl;
     for(int i = 0; i < (N-1); i ++) {
       perm_i = idx(i,0);
       if(i % 10) Rcpp::checkUserInterrupt();
       for(int j = i+1; j < N; j ++) {
         perm_j = idx(j,0);
         
         cur_cost = wass_cost_p(A.col(perm_i), B.col(idx(i,1)), p, ground_p) * mass(i);
         cur_cost += wass_cost_p(A.col(perm_j), B.col(idx(j,1)), p, ground_p) * mass(j);
         proposed_cost = wass_cost_p(A.col(perm_j), B.col(idx(i,1)), p, ground_p) * mass(j);
         proposed_cost += wass_cost_p(A.col(perm_i), B.col(idx(j,1)), p, ground_p) * mass(i);
         
         if(proposed_cost < cur_cost) {
           idx(i,0) = perm_j;
           idx(j,0) = perm_i;
           perm_i = perm_j;
           cost += (proposed_cost - cur_cost);
         }
       }
     }
     double error = std::fabs(cost - previous_cost) /double(N);
     if (error < tol) {
       break;
     }
     previous_cost = cost;
   }
 }
 

#endif //TRANS_SWAP_H
