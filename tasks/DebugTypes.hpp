#ifndef SONAR_WALL_HOUGH_TASK_DEBUGTYPES_HPP
#define SONAR_WALL_HOUGH_TASK_DEBUGTYPES_HPP

#include <base/samples/RigidBodyState.hpp>

namespace sonar_wall_hough{

  struct PositionQuality{
    
    base::Time time;
    double orientation_drift;
    double basin_width_diff;
    double basin_height_diff;
    double mean_sq_error;
    double support_ratio;    
  };
}

#endif
