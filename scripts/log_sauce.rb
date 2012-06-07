#library for displaying data
require 'vizkit'

include Orocos
Orocos.initialize

#load log file 
log = Orocos::Log::Replay.open("~/work/logdaten/sauc-e_first_day/20110704-1551/sonar.0.0.log", "~/work/logdaten/sauc-e_first_day/20110704-1551/orientation_estimator.0.log")

#now you can access all logged data by 
#addressing them by their task and port name
#log.task_name.port_name

#start deployment
#Orocos.run 'sonar_wall_hough_deployment', 'sonar_feature_estimator' do
Orocos.run 'sonar_wall_hough_deployment' do
    #Orocos.log_all
    
    hough = Orocos::TaskContext.get 'sonar_wall_hough'
    hough.angleDelta = -40.0
    hough.configure
    
    sonar = log.task 'sonar'
    compass = log.task 'orientation_estimator'
    
    sonar.BaseScan.connect_to hough.input
    compass.orientation_samples.connect_to hough.orientation
    
    Vizkit.display hough
    
    if hough.show_debug != 0
      Vizkit.display hough.peaks
      Vizkit.display hough.houghspace
      Vizkit.display hough.lines
      Vizkit.display sonar.BaseScan
    end
    
    hough.start

    #open control widget and start replay
    Vizkit.control log
    Vizkit.exec
end