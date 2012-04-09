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
    
    sonar_wall_hough::Config configuration = _config.get();
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
  
  int centerX = linesFrame->getWidth()/2;
  int centerY = linesFrame->getHeight()/2;
  
  for(int i = 0; i < (int)lines->size(); i++)
  {
    //find end points for line
    int x0 = centerX + lines->at(i).d*cos(lines->at(i).alpha)+linesFrame->getWidth()*sin(lines->at(i).alpha);
    int y0 = centerY - lines->at(i).d*sin(lines->at(i).alpha)+linesFrame->getWidth()*cos(lines->at(i).alpha);
    int x1 = centerX + lines->at(i).d*cos(lines->at(i).alpha)-linesFrame->getWidth()*sin(lines->at(i).alpha);
    int y1 = centerY - lines->at(i).d*sin(lines->at(i).alpha)-linesFrame->getWidth()*cos(lines->at(i).alpha);
    //std::cout << "x0="<<x0<<", y0="<<y0<<", x1="<<x1<<", y1="<<y1<<std::endl;
    
    drawLine(linesFrame, x0, y0, x1, y1);
  }
  
  
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
  peaksFrame->reset();
  std::vector<sonar_wall_hough::SonarPeak>* allPeaks = hough->getAllPeaks();
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