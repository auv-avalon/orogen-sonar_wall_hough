/* Generated from orogen/lib/orogen/templates/tasks/Task.cpp */

#include "Task.hpp"

using namespace sonar_wall_hough;

Task::Task(std::string const& name)
    : TaskBase(name)
    , hough(0)
    , peaksFrame(0)
    , houghspaceFrame(0)
    , linesFrame(0)
    , lastPeakCount(0)
    , peaksFrameOld(0)
{
}

Task::Task(std::string const& name, RTT::ExecutionEngine* engine)
    : TaskBase(name, engine)
    , hough(0)
    , peaksFrame(0)
    , houghspaceFrame(0)
    , linesFrame(0)
    , lastPeakCount(0)
    , peaksFrameOld(0)
{
}

Task::~Task()
{
}



/// The following lines are template definitions for the various state machine
// hooks defined by Orocos::RTT. See Task.hpp for more detailed
// documentation about them.

bool Task::configureHook()
{
    if (! TaskBase::configureHook())
        return false;
    
    sonar_wall_hough::Config configuration;
    configuration.angleDelta = _angleDelta.get();
    configuration.anglesPerBin = _anglesPerBin.get();
    configuration.basinHeight = _basinHeight.get();
    configuration.basinWidth = _basinWidth.get();
    configuration.distancesPerBin = _distancesPerBin.get();
    configuration.filterThreshold = _filterThreshold.get();
    configuration.withMinimumFilter = _withMinimumFilter.get();
    configuration.maxDistance = _maxDistance.get();
    configuration.minDistance = _minDistance.get();
    configuration.minLineVotesRatio = _minLineVotesRatio.get();
    configuration.sensorAngularResolution = _sensorAngularResolution.get();
    configuration.gain = _gain.get();
    
    //std::cout << "angleDelta = " << configuration.angleDelta << std::endl;
    
    hough = new sonar_wall_hough::Hough(configuration);
    
    peaksFrame = new base::samples::frame::Frame(2*configuration.maxDistance, 2*configuration.maxDistance);
    peaksFrameOld = new base::samples::frame::Frame(2*configuration.maxDistance, 2*configuration.maxDistance);
    linesFrame = new base::samples::frame::Frame(2*configuration.maxDistance, 2*configuration.maxDistance);
    houghspaceFrame = new base::samples::frame::Frame(hough->getHoughspace()->getWidth(), hough->getHoughspace()->getHeight());
    
    return true;
}

bool Task::startHook()
{    
    if (! TaskBase::startHook())
        return false;
    return true;
}

void Task::updateHook()
{
    //std::cout << "size= " << (hough->getAllPeaks()->size()) << std::endl;
    TaskBase::updateHook();
    
    base::samples::RigidBodyState rbs;
    if(_orientation.read(rbs) == RTT::NewData)
    {
      hough->setOrientation(rbs.getYaw());
    }
    
    base::samples::SonarBeam sonarBeam;
    if(_input.read(sonarBeam) == RTT::NewData)
    {
      hough->registerBeam(sonarBeam);
    }
    
    //write orientation drift
    _orientation_drift.write(hough->getOrientationDrift());
    
    //write position to output port
    base::samples::RigidBodyState rbs_out(rbs);
    base::Pose pose = rbs_out.getPose();
    std::pair<double,double> xyPos = hough->getActualPosition();
    pose.position(0,0) = xyPos.first;
    pose.position(1,0) = xyPos.second;
    rbs_out.setPose(pose);
    rbs_out.time.microseconds = rbs.time.microseconds;
    _position.write(rbs_out);
    
    if(!_show_debug.get())
      return;
    
    //save old peaks image
    if(lastPeakCount > hough->getAllPeaks()->size())
    {
      peaksFrameOld->image.clear();
      peaksFrameOld->image.insert(peaksFrameOld->image.begin(), peaksFrame->image.begin(), peaksFrame->image.end());
      makeLinesFrame();
    }
    lastPeakCount = hough->getAllPeaks()->size();
    
    makePeaksFrame();
    makeHoughspaceFrame();
}

// void Task::errorHook()
// {
//     TaskBase::errorHook();
// }
// void Task::stopHook()
// {
//     TaskBase::stopHook();
// }
void Task::cleanupHook()
{
    TaskBase::cleanupHook();
    delete hough;
}

void Task::makeHoughspaceFrame()
{
  std::vector<uint8_t>::iterator imageCenterY = houghspaceFrame->image.begin() + houghspaceFrame->getHeight()/2 * houghspaceFrame->getWidth();
  for(int y = -houghspaceFrame->getHeight()/2; y <= houghspaceFrame->getHeight()/2; y++)
  {
    for(int x = 0; x < houghspaceFrame->getWidth(); x++)
    {
      *(imageCenterY + y*houghspaceFrame->getWidth() + x) = *(hough->getHoughspace()->uncheckedAt(x,y)); 
    }
  }
  
  _houghspace.write(*houghspaceFrame);
}

void Task::makeLinesFrame()
{
  linesFrame->reset();
    
  std::vector<Line>* lines = hough->getActualLines();
  
  //make LineMarks
  lineMarks.clear();
  if(lines->size() != 4)
    return;
  
  uw_localization::Linemark lm0, lm1, lm2, lm3;
  
  std::pair<base::Vector3d, base::Vector3d> limits0 = lines->at(0).toCartesian(lines->at(2), lines->at(3));
  lm0.from = limits0.first;
  lm0.to = limits0.second;
  lm0.height = 100.0;
  
  std::pair<base::Vector3d, base::Vector3d> limits1 = lines->at(1).toCartesian(lines->at(2), lines->at(3));
  lm1.from = limits1.first;
  lm1.to = limits1.second;
  lm1.height = lm0.height;
  
  std::pair<base::Vector3d, base::Vector3d> limits2 = lines->at(2).toCartesian(lines->at(0), lines->at(1));
  lm2.from = limits2.first;
  lm2.to = limits2.second;
  lm2.height = lm0.height;
  
  std::pair<base::Vector3d, base::Vector3d> limits3 = lines->at(3).toCartesian(lines->at(0), lines->at(1));
  lm3.from = limits3.first;
  lm3.to = limits3.second;
  lm3.height = lm0.height;
  
  lineMarks.push_back(lm0);
  lineMarks.push_back(lm1);
  lineMarks.push_back(lm2);
  lineMarks.push_back(lm3);
  
  //_map_wall_lines.write(lineMarks);
  
  //make frame
  int centerX = linesFrame->getWidth()/2;
  int centerY = linesFrame->getHeight()/2;
  
  drawLine(linesFrame, centerX+lm0.from[0], centerY-lm0.from[1], centerX+lm0.to[0], centerY-lm0.to[1]);
  drawLine(linesFrame, centerX+lm1.from[0], centerY-lm1.from[1], centerX+lm1.to[0], centerY-lm1.to[1]);
  drawLine(linesFrame, centerX+lm2.from[0], centerY-lm2.from[1], centerX+lm2.to[0], centerY-lm2.to[1]);
  drawLine(linesFrame, centerX+lm3.from[0], centerY-lm3.from[1], centerX+lm3.to[0], centerY-lm3.to[1]);
  
  
  //draw x and y axis
  drawLine(linesFrame, centerX, centerY, centerX+30*cos(hough->getOrientation().getRad()), centerY+30*sin(hough->getOrientation().getRad())); //x-axis
  //drawLine(linesFrame, centerX, centerY, centerX+30*cos(hough->getOrientation().getRad()+M_PI/2), centerY+30*sin(hough->getOrientation().getRad()+M_PI/2)); //y-axis
  std::cout << "orientation was " << hough->getOrientation() << std::endl;
  
  //copy old peaks on linesFrame
  for(int i = 0; i < linesFrame->image.size(); i++)
    linesFrame->image.at(i) = std::max(linesFrame->image.at(i), peaksFrameOld->image.at(i));
  
  _lines.write(*linesFrame);
}

void Task::drawLine(base::samples::frame::Frame* frame, int x0, int y0, int x1, int y1)
{
  //bresenham line drawing (mostly taken from Wikipedia)
    int dx =  abs(x1-x0), sx = x0<x1 ? 1 : -1;
    int dy = -abs(y1-y0), sy = y0<y1 ? 1 : -1; 
    int err = dx+dy, e2; /* error value e_xy */
    
    for(;;)
    {
      if(x0 >= 0 && x0 < frame->getWidth() && y0 >= 0 && y0 < frame->getHeight())
      {
	frame->image[y0 * frame->getWidth() + x0] = 255;//lines->at(i).votes;
      }
      if (x0==x1 && y0==y1) break;
      e2 = 2*err;
      if (e2 > dy) { err += dy; x0 += sx; } /* e_xy+e_x > 0 */
      if (e2 < dx) { err += dx; y0 += sy; } /* e_xy+e_y < 0 */
    }
}

void Task::makePeaksFrame()
{
  std::vector<sonar_wall_hough::SonarPeak>* allPeaks = hough->getAllPeaks();
  /*
  //make Particles
  for(int i = 0; i < (int)allPeaks->size(); i++)
  {
    base::samples::LaserScan ls;
    ls.start_angle = allPeaks->at(i).alpha;
    ls.ranges.push_back(allPeaks->at(i).distance);
    //uw_localization::Particle p;
    //p.position = allPeaks->at(i).toCartesian();
    //ppeaks.particles.push_back(p);
  }
  _ppeaks.write(ppeaks);
  */
  //make frame
  peaksFrame->reset();
  for(int i = 0; i < (int)allPeaks->size(); i++)
  {
    int x = peaksFrame->getWidth()/2 + allPeaks->at(i).distance * cos(allPeaks->at(i).alpha.rad);
    int y = peaksFrame->getHeight()/2 - allPeaks->at(i).distance * sin(allPeaks->at(i).alpha.rad);
    peaksFrame->image[y * peaksFrame->getWidth() + x] = 255;
    //and some around there
    if(x >= 1 && x < peaksFrame->getWidth()-1 && y >= 1 && y < peaksFrame->getHeight()-1)
    {
      peaksFrame->image[(y+0) * peaksFrame->getWidth() + x+1] = 255;
      peaksFrame->image[(y+0) * peaksFrame->getWidth() + x-1] = 255;
      peaksFrame->image[(y+1) * peaksFrame->getWidth() + x+0] = 255;
      peaksFrame->image[(y-1) * peaksFrame->getWidth() + x+0] = 255;
    }
  }
  _peaks.write(*peaksFrame);
}