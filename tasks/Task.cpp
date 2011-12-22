/* Generated from orogen/lib/orogen/templates/tasks/Task.cpp */

#include "Task.hpp"

using namespace sonar_wall_hough;

Task::Task(std::string const& name)
    : TaskBase(name)
    , hough(0)
    , peaksFrame(0)
    , houghspaceFrame(0)
    , linesFrame(0)
{
}

Task::Task(std::string const& name, RTT::ExecutionEngine* engine)
    : TaskBase(name, engine)
    , hough(0)
    , peaksFrame(0)
    , houghspaceFrame(0)
    , linesFrame(0)
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
    
    sonarWallHough::Config configuration = _config.get();
    hough = new sonarWallHough::Hough(configuration);
    
    peaksFrame = new base::samples::frame::Frame(2*configuration.maxDistance, 2*configuration.maxDistance);
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
    
    base::samples::SonarBeam sonarBeam;
    if(_input.read(sonarBeam) == RTT::NewData)
    {
      hough->registerBeam(sonarBeam);
    }
    
    makePeaksFrame();
    
    /*bool interpretFirst;
    if(_reset.read(interpretFirst) == RTT::NewData)
    {
      if(interpretFirst)
	hough->getLines();
      hough->clearHoughspace();
    }
    else
    {
      base::samples::SonarBeam sonarBeam;
      _input.read(sonarBeam);
      hough->accumulate(sonarBeam);
    }
    
    if(hough->hasNewResults())
    {
      base::samples::frame::Frame frame(1200, 1200, 8, base::samples::frame::MODE_GRAYSCALE, 0, 0);
	
      int d, x, xPrime, yPrime;
      double alpha;
      for(int i = 0; i < hough->getResult().size(); i++)
      {
	d = hough->getResult()[i].second;
	alpha = hough->getResult()[i].first;
	if(abs(d) < 50)
	  continue;
	
	for(int y = 0; y < frame.getHeight(); y++)
	{
	  d = hough->getResult()[i].second;
	  alpha = hough->getResult()[i].first - (M_PI/2); //0° ist to the right
	  yPrime = frame.getHeight()/2 - y;
	  xPrime = (int)((d - yPrime * sin(alpha)) / (cos(alpha)) + 0.5);
	  x = xPrime + frame.getWidth()/2;
	  if(x >= 0 && x < frame.getWidth())
	  {
	    frame.image[(y*frame.getWidth() + x)] = 255;
	  }
	}
      }
      _lines.write(frame);
    }
    _preprocessed.write(hough->preprocessedBeam);
    
    //write houghspace as image
      base::samples::frame::Frame houghimage(hough->getNumberOfAngles(), 601, 8, base::samples::frame::MODE_GRAYSCALE, 0, 0);
      for(int d = -300; d <= 300; d++)
      {
	for(int alphaIdx = 0; alphaIdx < hough->getNumberOfAngles(); alphaIdx++)
	{
	  houghimage.image[hough->getNumberOfAngles()* (d+300) + alphaIdx] = 10 * hough->houghspace[d * hough->getNumberOfAngles() + alphaIdx];
	}
      }
      _houghimage.write(houghimage);*/
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

}

void Task::makelinesFrame()
{

}

void Task::makePeaksFrame()
{
  std::vector<sonarWallHough::SonarPeak>* allPeaks = hough->getAllPeaks();
  for(int i = 0; i < (int)allPeaks->size(); i++)
  {
    int x = peaksFrame->getWidth()/2 + allPeaks->at(i).distance * cos(allPeaks->at(i).alpha.rad);
    int y = peaksFrame->getHeight()/2 - allPeaks->at(i).distance * sin(allPeaks->at(i).alpha.rad);
    peaksFrame->image[y * peaksFrame->getHeight() + x] = 255;
  }
  _peaks.write(*peaksFrame);
}