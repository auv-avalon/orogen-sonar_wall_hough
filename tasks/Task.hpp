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
	void makePeaksFrame(base::samples::frame::Frame* frame, std::vector<SonarPeak>* peaks, bool clear=true);
	void makeHoughspaceFrame();
	void makeLinesFrame();
	void drawLine(base::samples::frame::Frame* frame, int x0, int y0, int x1, int y1);
	base::samples::frame::Frame* peaksFrame;
	std::vector<SonarPeak> oldPeaks;
	base::samples::frame::Frame* houghspaceFrame;
	base::samples::frame::Frame* linesFrame;
	conversion::JpegConversion jpegConverter;
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

        /** This hook is called by Orocos when the component is in the
         * RunTimeError state, at each activity step. See the discussion in
         * updateHook() about triggering options.
         *
         * Call recover() to go back in the Runtime state.
         */
        // void errorHook();

        /** This hook is called by Orocos when the state machine transitions
         * from Running to Stopped after stop() has been called.
         */
        // void stopHook();

        /** This hook is called by Orocos when the state machine transitions
         * from Stopped to PreOperational, requiring the call to configureHook()
         * before calling start() again.
         */
        void cleanupHook();
    };
}

#endif

