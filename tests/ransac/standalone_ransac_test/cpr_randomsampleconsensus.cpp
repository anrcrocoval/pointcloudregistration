
// g++ -c -o cpr_randomsampleconsensus.o -Wall -Wextra -Werror cpr_randomsampleconsensus.cpp -I /usr/include/pcl-1.9/ -I /usr/include/eigen3/ -I /usr/include/vtk/
// g++ cpr_randomsampleconsensus.o -lvtkCommonDataModel -lvtkCommonMath -lvtkCommonCore -lvtkRenderingCore -lpcl_common -lpcl_filters -lpcl_io -lpcl_sample_consensus -lpcl_visualization

#include <iostream>
#include <thread>

#include <fstream>  // opening and reading files
#include <vector>   // vector of indices of inlier points in the cloud

#include <pcl/console/parse.h>
#include <pcl/filters/extract_indices.h>
#include <pcl/io/pcd_io.h>
#include <pcl/point_types.h>
#include <pcl/sample_consensus/ransac.h>
#include <pcl/sample_consensus/sac_model_plane.h>   // TODO remove this header
#include <pcl/sample_consensus/sac_model_sphere.h>  // TODO remove this header
#include <pcl/sample_consensus/sac_model_registration.h>
#include <pcl/visualization/pcl_visualizer.h>

using namespace std::chrono_literals;

pcl::visualization::PCLVisualizer::Ptr
simpleVis (pcl::PointCloud<pcl::PointXYZ>::ConstPtr cloud)
{
  // --------------------------------------------
  // -----Open 3D viewer and add point cloud-----
  // --------------------------------------------
  pcl::visualization::PCLVisualizer::Ptr viewer (new pcl::visualization::PCLVisualizer ("3D Viewer"));
  viewer->setBackgroundColor (0, 0, 0);
  viewer->addPointCloud<pcl::PointXYZ> (cloud, "sample cloud");
  viewer->setPointCloudRenderingProperties (pcl::visualization::PCL_VISUALIZER_POINT_SIZE, 3, "sample cloud");
  //viewer->addCoordinateSystem (1.0, "global");
  viewer->initCameraParameters ();
  return (viewer);
}

void populateWithHemispheres(pcl::PointCloud<pcl::PointXYZ>::Ptr cloud_model,
  pcl::PointCloud<pcl::PointXYZ>::Ptr cloud_data,
  double threshold)
{
  double pi = 3.141592653589793;

  // populate our PointClouds with points
  cloud_model->width    = 250;
  cloud_model->height   = 1;
  cloud_model->is_dense = false;
  cloud_model->points.resize (cloud_model->width * cloud_model->height);
  cloud_data->width    = 250;
  cloud_data->height   = 1;
  cloud_data->is_dense = false;
  cloud_data->points.resize (cloud_data->width * cloud_data->height);
  for (std::size_t i = 0; i < cloud_model->points.size(); ++i)
  {
    // source: theta in [0, pi], phi in [0, pi]     ==> hemisphere y > 0
    double theta = pi * (static_cast<float>(rand()) / RAND_MAX);
    double u = -1.0 + 2.0 * (static_cast<float>(rand()) / RAND_MAX);
    double phi = acos(u);
    cloud_model->points[i].x = sin(phi) * cos(theta);
    cloud_model->points[i].y = sin(phi) * sin(theta);
    cloud_model->points[i].z = cos(phi);
  }
  for (std::size_t i = 0; i < cloud_data->points.size(); i += 2)
  {
    // inliers:                                     ==> hemisphere y < 0
    // double theta = pi * (1.0 + static_cast<float>(rand()) / RAND_MAX);
    // double u = -1.0 + 2.0 * (static_cast<float>(rand()) / RAND_MAX);
    // double phi = acos(u);
    // cloud_data->points[i].x = sin(phi) * cos(theta);
    // cloud_data->points[i].y = sin(phi) * sin(theta);
    // cloud_data->points[i].z = cos(phi);
    cloud_data->points[i].x = cloud_model->points[i].x + (static_cast<float>(rand()) / RAND_MAX - 0.5) * threshold;
    cloud_data->points[i].y = -cloud_model->points[i].y + (static_cast<float>(rand()) / RAND_MAX - 0.5) * threshold;
    cloud_data->points[i].z = -cloud_model->points[i].z + (static_cast<float>(rand()) / RAND_MAX - 0.5) * threshold;

    // outliers: x in [-1,1], y in [-1,0], z in [-1,1]
    cloud_data->points[i+1].x = -1 + 2 * (static_cast<float>(rand()) / RAND_MAX);
    cloud_data->points[i+1].y = -1 + (static_cast<float>(rand()) / RAND_MAX);
    cloud_data->points[i+1].z = -1 + 2 * (static_cast<float>(rand()) / RAND_MAX);
  }
}


int test_loadCSVPointcloud(char const *filename, pcl::PointCloud<pcl::PointXYZ>::Ptr cloud)
{
  std::cout << "Loading point cloud from csv file " << filename << std::endl;
  std::ifstream file(filename);
  std::string line;
  std::getline(file, line);   // header: "id,dimension_1,dimension_2,dimension_3"
  while (std::getline(file, line))
  {
    std::istringstream iss(line);
    int id;
    float x,y,z;
    char separator;
    if (!(iss >> id >> separator >> x >> separator >> y >> separator >> z))
    {
      std::cout << "Wrong line when reading .csv point cloud:" << std::endl;
      std::cout << "    " << line << std::endl;
      return 1;
    }
    cloud->push_back(pcl::PointXYZ(x,y,z));
  }
  cloud->width = cloud->points.size();
  cloud->height = 1;
  cloud->is_dense = false;
  return 0;
}

void printUsageAndExit(char const *cmd)
{
  std::cout << "SYNOPSIS" << std::endl << std::endl;
  std::cout << cmd << " -h" << std::endl;
  std::cout << "    Print this help message and exit." << std::endl << std::endl;
  std::cout << cmd << " -s [-f]" << std::endl;
  std::cout << "    Use a hemisphere as model, and a hemisphere + 100% noise as data." << std::endl << std::endl;
  std::cout << cmd << " [-f] <pc0> <pc1>" << std::endl;
  std::cout << "    Use csv file <pc0> as model, and <pc1> as data." << std::endl << std::endl;
  std::cout << "If option -f is specified, apply RanSaC to remove outliers from the data." << std::endl << std::endl;
  exit(1);
}

void updateFlags(bool &flagSphere, bool &flagCompute, bool &flagShowmodel, int argc, char **argv)
{
  flagSphere    = false;
  flagCompute   = false;
  flagShowmodel = false;
  if (pcl::console::find_argument (argc, argv, "-f") >= 0 )
    flagCompute = true;
  if (pcl::console::find_argument (argc, argv, "-s") >= 0 )
    flagSphere = true;
  if (pcl::console::find_argument(argc, argv, "-m") >= 0)
    flagShowmodel = true;
  if ((pcl::console::find_argument (argc, argv, "-sf") >= 0) || (pcl::console::find_argument (argc, argv, "-fs") >= 0) )
  {
    flagCompute = true;
    flagSphere  = true;
  }
  if ((!flagSphere && argc != 3 && argc != 4) || pcl::console::find_argument (argc, argv, "-h") >= 0)
    printUsageAndExit(argv[0]);
}

// void test_writeCSVPointcloud(char const *filename, pcl::PointCloud<pcl::PointXYZ>::Ptr const cloud)
// {
//   std::ofstream file(filename, std::ofstream::trunc);
//   file << "id,dimension_1,dimension_2,dimension_3" << '\n';
//   std::size_t i = 0;
//   for (auto p : cloud->points)
//   {
//     file << i << ',' << p.x << ',' << p.y << ',' << p.z << '\n';
//     ++i;
//     //note that it is a bit stupid to discard the indices that were read in the input csv file
//     //we might want to keep the same indices, so it is apparent which points were outliers
//   }
// }

void test_writeCSVPointcloud(char const *filename,
  pcl::PointCloud<pcl::PointXYZ>::Ptr const cloud,
  std::vector<int> indices)
{
  std::ofstream file(filename, std::ofstream::trunc);
  file << "id,dimension_1,dimension_2,dimension_3" << '\n';
  for (int i : indices)
  {
    file << i << ',' << cloud->points[i].x
              << ',' << cloud->points[i].y
              << ',' << cloud->points[i].z << '\n';
  }
}

int
main(int argc, char** argv)
{
  bool flagSphere, flagCompute, flagShowmodel;
  updateFlags(flagSphere, flagCompute, flagShowmodel, argc, argv);
  double threshold = 0.01;
  // initialize PointClouds
  pcl::PointCloud<pcl::PointXYZ>::Ptr cloud_model (new pcl::PointCloud<pcl::PointXYZ>);
  pcl::PointCloud<pcl::PointXYZ>::Ptr cloud_data (new pcl::PointCloud<pcl::PointXYZ>);
  pcl::PointCloud<pcl::PointXYZ>::Ptr final_model (new pcl::PointCloud<pcl::PointXYZ>);
  pcl::PointCloud<pcl::PointXYZ>::Ptr final_data (new pcl::PointCloud<pcl::PointXYZ>);

  if (flagSphere)
    populateWithHemispheres(cloud_model, cloud_data, threshold);
  else
  {
    assert(argc == 3 || argc == 4);
    test_loadCSVPointcloud(argv[argc - 2], cloud_model);
    test_loadCSVPointcloud(argv[argc - 1], cloud_data);
  }

  std::vector<int> inliers;

  // created RandomSampleConsensus object and compute the appropriated model
  pcl::SampleConsensusModelRegistration<pcl::PointXYZ>::Ptr
    model_r(new pcl::SampleConsensusModelRegistration<pcl::PointXYZ>(cloud_data));
  model_r->setInputTarget(cloud_model);
  if (flagCompute)
  {
    pcl::RandomSampleConsensus<pcl::PointXYZ> ransac (model_r);
    ransac.setDistanceThreshold (threshold);
    ransac.computeModel();
    ransac.getInliers(inliers);
  }

  // copy all inliers of the model computed to another PointCloud
  pcl::copyPointCloud (*cloud_model, inliers, *final_model);
  pcl::copyPointCloud (*cloud_data, inliers, *final_data);
  std::cout << "Writing resulting point cloud to file final_data.csv and final_model." << std::endl;
  test_writeCSVPointcloud("final_data.csv", cloud_data, inliers);
  test_writeCSVPointcloud("final_model.csv", cloud_model, inliers);

  // creates the visualization object and adds either our original cloud or all of the inliers
  // depending on the command line arguments specified.
  pcl::visualization::PCLVisualizer::Ptr viewer;
  if (flagCompute)
    viewer = simpleVis(final_data);
  else if (flagShowmodel)
    viewer = simpleVis(cloud_model);
  else
    viewer = simpleVis(cloud_data);
  while (!viewer->wasStopped ())
  {
    viewer->spinOnce (100);
    std::this_thread::sleep_for(100ms);
  }
  return 0;
 }
