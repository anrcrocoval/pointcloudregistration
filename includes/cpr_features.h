
#ifndef __DEF_FEATURES_H__
# define __DEF_FEATURES_H__

#include "cpr_main.h"


/*
** features.cpp
*/

double esfDistance(ESFHist const &a, ESFHist const &b);

double edgeDistance(EdgeFeature const &a, EdgeFeature const &b);


void calculateAnglesAndLength(
  PointT const &p1, PointT const &p2,
  double &angle_x, double &angle_y, double &angle_z, double &length,
  double const voxelres);

void calculateESFDescriptors(SupervoxelClusters const &sv_clusters, ESFDescriptors &esf_descriptors);

void calculateEdgesDescriptors(
  SupervoxelClusters const &sv_clusters,
  SupervoxelAdjacency const &sv_adjacency,
  double voxelres,
  EdgeDescriptors &edge_descriptors);

void calculateDescriptors(
  SupervoxelClusters const &sv_clusters,
  SupervoxelAdjacency const &sv_adjacency,
  double voxelres, ESFDescriptors &esf_descriptors,
  EdgeDescriptors &edge_descriptors);

void writeDescriptorsToCSV(char const *name, ESFDescriptors esf_descriptors, EdgeDescriptors edge_descriptors);



#endif /* __DEF_FEATURES_H__ */
