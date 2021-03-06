
#ifndef __DEF_GRAPHMATCHINGCGAL_H__
# define __DEF_GRAPHMATCHINGCGAL_H__

#include <CGAL/QP_models.h>    // quadratic programming
#include <CGAL/QP_functions.h>
#include <CGAL/MP_Float.h>

#include "cpr_main.h"
#include "cpr_matrices.h"
#include "cpr_graphmatching.h"

/**
A graph matching algorithm inspired by Zaslavskiy and Huang

	@author Stephan Kunne <stephan.kunne@univ-nantes.fr>
*/
/*
** cpr_graphmatching.cpp
*/
class GraphMatchingCgal : GraphMatching
{
private:
  //MatrixDouble const *vsim;    // vertex similarity matrix
  //EdgeSimilarityMatrix const *esim; // edge similarity matrix

  //MatrixInt const *g_adj;   // adjacency matrix of first graph
  //MatrixInt const *h_adj;   // adjacency matrix of second graph

  //MatrixDouble sol_concav;   // doubly stochastic, actually a permutation matrix
  //MatrixDouble sol_convex;   // doubly stochastic
  //MatrixDouble sol_smooth;   // doubly stochastic

public:
  //MatrixInt matching;          // permutation matrix
  // include matching as an std::map as well as a permutation matrix?

protected:
  int **A;    // stochasticity constraints, columnwise
  std::size_t nbconstraints;
  double **D; // objective, rowwise, on/below diagonal only, multiplied by a factor 2
  std::size_t nbvar;
  CGAL::Const_oneset_iterator<CGAL::Comparison_result> r; // constraints are "=="
  CGAL::Const_oneset_iterator<int> b;   // rhs of constraints is 1
  CGAL::Const_oneset_iterator<bool> fl; // all variables lowerbounded by 0
  CGAL::Const_oneset_iterator<int> l;   // all variables lowerbounded by 0
  CGAL::Const_oneset_iterator<bool> fu; // all variables upperbounded by 1 (redundant with sum = 1)
  CGAL::Const_oneset_iterator<int> u;   // all variables upperbounded by 1 (redundant with sum = 1)
  CGAL::Const_oneset_iterator<int> c;   // no linear term in objective
  int c0;                               // no constant term in objective

public:
  GraphMatchingCgal(MatrixDouble const *vsim, EdgeSimilarityMatrix const *esim, MatrixInt const *g_adj, MatrixInt const *h_adj);
  void build(void);
  virtual void run(void);
  void fillMpsStream(std::stringstream &in) const;
  void fillQuadraticObjective(void);       // objective, rowwise, on/below diagonal only, multiplied by a factor 2
  void fillStochasticityConstraints(void); // make constraints, columnwise
  //unsigned int mappedVertex(unsigned int) const;
};

#endif /* __DEF_GRAPHMATCHINGCGAL_H__ */
