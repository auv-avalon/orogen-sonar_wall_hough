/* Generated from orogen/lib/orogen/templates/tasks/Task.cpp */

#include "Task.hpp"

using namespace sonar_wall_hough;

Task::Task(std::string const& name)
    : TaskBase(name)
    , hough(0)
    , peaksFrame(0)
    , houghspaceFrame(0)
    , linesFrame(0)
    , jpegConverter(50)
{
}

Task::Task(std::string const& name, RTT::ExecutionEngine* engine)
    : TaskBase(name, engine)
    , hough(0)
    , peaksFrame(0)
    , houghspaceFrame(0)
    , linesFrame(0)
    , jpegConverter(50)
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
    
    peaksFrame = new base::samples::frame::Frame(2*configuration.maxDistance, 2*configuration.maxDistance, 8, base::samples::frame::MODE_JPEG);
    linesFrame = new base::samples::frame::Frame(2*configuration.maxDistance, 2*configuration.maxDistance, 8, base::samples::frame::MODE_JPEG);
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
    if(_orientation_samples.read(rbs) == RTT::NewData)
    {
      hough->setOrientation(rbs.getYaw());
    }
    
    base::samples::SonarBeam sonarBeam;
    if(_sonar_samples.read(sonarBeam) == RTT::NewData)
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
 
    if(_continous_write.get())
         _position.write(rbs_out);
   
    //save old peaks image
    if(oldPeaks.size() > hough->getAllPeaks()->size())
    {
      //write out quality values
      _basinWidthDiff.write(hough->getBasinWidthDiff());
      _basinHeightDiff.write(hough->getBasinHeightDiff());
      _meanSqError.write(hough->getMeanSqErr());
      _supportRatio.write(hough->getSupportRatio());
      
      if(_show_debug.get()) {
          makeLinesFrame();
          _lines.write(*linesFrame);
          _houghspace.write(*houghspaceFrame);
      }

      if(!_continous_write.get())
          _position.write(rbs_out);
    }
    oldPeaks.clear();
    oldPeaks.insert(oldPeaks.begin(), hough->getAllPeaks()->begin(), hough->getAllPeaks()->end());
    
    makeHoughspaceFrame();
    /*
    //only for real-time peak updates
    makePeaksFrame(peaksFrame, oldPeaks);
    */
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
  houghspaceFrame->reset();
    
  std::vector<uint8_t>::iterator imageCenterY = houghspaceFrame->image.begin() + houghspaceFrame->getHeight()/2 * houghspaceFrame->getWidth();
  for(int y = -houghspaceFrame->getHeight()/2; y <= houghspaceFrame->getHeight()/2; y++)
  {
    for(int x = 0; x < houghspaceFrame->getWidth(); x++)
    {
      *(imageCenterY + y*houghspaceFrame->getWidth() + x) = *(hough->getHoughspace()->uncheckedAt(x,y));
    }
  }
}

void Task::makeLinesFrame()
{
  //make clean grayscale frame
  linesFrame->init(linesFrame->getWidth(), linesFrame->getHeight());
    
  std::vector<Line>* lines = hough->getActualLines();
  if(lines->size() == 4)
  {
    std::pair<base::Vector3d, base::Vector3d> limits0 = lines->at(0).toCartesian(lines->at(2), lines->at(3));
    std::pair<base::Vector3d, base::Vector3d> limits1 = lines->at(1).toCartesian(lines->at(2), lines->at(3));
    std::pair<base::Vector3d, base::Vector3d> limits2 = lines->at(2).toCartesian(lines->at(0), lines->at(1));
    std::pair<base::Vector3d, base::Vector3d> limits3 = lines->at(3).toCartesian(lines->at(0), lines->at(1));
    
    //make frame
    int centerX = linesFrame->getWidth()/2;
    int centerY = linesFrame->getHeight()/2;
    
    drawLine(linesFrame, centerX+limits0.first[0], centerY-limits0.first[1], centerX+limits0.second[0], centerY-limits0.second[1]);
    drawLine(linesFrame, centerX+limits1.first[0], centerY-limits1.first[1], centerX+limits1.second[0], centerY-limits1.second[1]);
    drawLine(linesFrame, centerX+limits2.first[0], centerY-limits2.first[1], centerX+limits2.second[0], centerY-limits2.second[1]);
    drawLine(linesFrame, centerX+limits3.first[0], centerY-limits3.first[1], centerX+limits3.second[0], centerY-limits3.second[1]);
    
    
    //draw x and y axis
    drawLine(linesFrame, centerX, centerY, centerX+30*cos(hough->getOrientation().getRad()), centerY+30*sin(hough->getOrientation().getRad())); //x-axis
    //drawLine(linesFrame, centerX, centerY, centerX+30*cos(hough->getOrientation().getRad()+M_PI/2), centerY+30*sin(hough->getOrientation().getRad()+M_PI/2)); //y-axis
    std::cout << "orientation was " << hough->getOrientation() << std::endl;
  }
  
  //copy old peaks on linesFrame
  makePeaksFrame(linesFrame, &oldPeaks, false);
  
  //compress frame
  //jpegConverter.compress(*linesFrame, *linesFrame);
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
	//frame->image[y0 * frame->getWidth() + x0] = 255;//lines->at(i).votes;
	frame->at<unsigned char>(x0, y0) = 255;
      }
      if (x0==x1 && y0==y1) break;
      e2 = 2*err;
      if (e2 > dy) { err += dy; x0 += sx; } /* e_xy+e_x > 0 */
      if (e2 < dx) { err += dx; y0 += sy; } /* e_xy+e_y < 0 */
    }
}

void Task::makePeaksFrame(base::samples::frame::Frame* frame, std::vector<SonarPeak>* peaks, bool clear)
{
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
  if(clear)
    frame->reset();
  
  for(int i = 0; i < (int)peaks->size(); i++)
  {
    int x = frame->getWidth()/2 + peaks->at(i).distance * cos(peaks->at(i).alpha.rad);
    int y = frame->getHeight()/2 - peaks->at(i).distance * sin(peaks->at(i).alpha.rad);
    
    if(x >= 1 && x < frame->getWidth()-1 && y >= 1 && y < frame->getHeight()-1)
    {
      //frame->image[y * frame->getWidth() + x] = 255;
      frame->at<unsigned char>(x,y) = 255;
      //and some around there
      //frame->image[(y+0) * frame->getWidth() + x+1] = 255;
      frame->at<unsigned char>(x+1,y) = 255;
      //frame->image[(y+0) * frame->getWidth() + x-1] = 255;
      frame->at<unsigned char>(x-1,y) = 255;
      //frame->image[(y+1) * frame->getWidth() + x+0] = 255;
      frame->at<unsigned char>(x,y+1) = 255;
      //frame->image[(y-1) * frame->getWidth() + x+0] = 255;
      frame->at<unsigned char>(x,y-1) = 255;
    }
  }
}
