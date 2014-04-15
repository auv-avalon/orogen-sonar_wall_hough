/* Generated from orogen/lib/orogen/templates/tasks/Task.hpp */

#ifndef SONAR_WALL_HOUGH_TASK_TASK_HPP
#define SONAR_WALL_HOUGH_TASK_TASK_HPP

#include "sonar_wall_hough/TaskBase.hpp"
#include "sonar_wall_hough/Hough.hpp"
#include "jpeg_conversion/jpeg_conversion.hpp"

namespace sonar_wall_hough {
    class Task : public TaskBase
    {
	friend class TaskBase;
	
    private:
	/**
	* This method draws the sonar peaks (points in polar coordinates) into the debug image frame
	* @param frame the frame to be drawn in
	* @param peaks the vector of peaks to be drawn
	* @param clear if the frame should be cleared before drawing
	*/
	void makePeaksFrame(base::samples::frame::Frame* frame, std::vector<SonarPeak>* peaks, bool clear=true);
	
	/**
	 * This method draws the current houghspace into the debug image frame
	 */
	void makeHoughspaceFrame();
	
	/**
	 * This method draws the actual detected lines into the debug image frame
	 */
	void makeLinesFrame();
	
	/**
	 * This method draws a line into an image frame
	 * @param x0 x-coordinate of the starting point
	 * @param y0 y-coordinate of the starting point
	 * @param x1 x-coordinate of the end points
	 * @param y1 y-coordinate of the end point
	 */
	void drawLine(base::samples::frame::Frame* frame, int x0, int y0, int x1, int y1);
	
	base::samples::frame::Frame* peaksFrame;
	std::vector<SonarPeak> oldPeaks;
	base::samples::frame::Frame* houghspaceFrame;
	base::samples::frame::Frame* linesFrame;
	conversion::JpegConversion jpegConverter;
        base::Time lastPerception;
	
    protected:
	sonar_wall_hough::Hough* hough;

    public:
        Task(std::string const& name = "message_producer::Task");
	Task(std::string const& name, RTT::ExecutionEngine* engine);

	~Task();

        /** This hook is called by Orocos when the state machine transitions
         * from PreOperational to Stopped. If it returns false, then the
         * component will stay in PreOperational. Otherwise, it goes into
         * Stopped.
         *
         * It is meaningful only if the #needs_configuration has been specified
         * in the task context definition with (for example):
         *
         *   task_context "TaskName" do
         *     needs_configuration
         *     ...
         *   end
         */
        bool configureHook();

        /** This hook is called by Orocos when the state machine transitions
         * from Stopped to Running. If it returns false, then the component will
         * stay in Stopped. Otherwise, it goes into Running and updateHook()
         * will be called.
         */
        bool startHook();

        /** This hook is called by Orocos when the component is in the Running
         * state, at each activity step. Here, the activity gives the "ticks"
         * when the hook should be called.
         *
         * The error(), exception() and fatal() calls, when called in this hook,
         * allow to get into the associated RunTimeError, Exception and
         * FatalError states. 
         *
         * In the first case, updateHook() is still called, and recover() allows
         * you to go back into the Running state.  In the second case, the
         * errorHook() will be called instead of updateHook(). In Exception, the
         * component is stopped and recover() needs to be called before starting
         * it again. Finally, FatalError cannot be recovered.
         */
        void updateHook();

        /** This hook is called by Orocos when the state machine transitions
         * from Stopped to PreOperational, requiring the call to configureHook()
         * before calling start() again.
         */
        void cleanupHook();
    };
}

#endif

