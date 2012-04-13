#library for displaying data
require 'vizkit'

include Orocos
Orocos.initialize

#load log file 
#log = Orocos::Log::Replay.open("~/work/logdaten/slam/sonar.19.log")
log = Orocos::Log::Replay.open("~/work/logdaten/sauc-e_first_day/20110704-1551/sonar.0.0.log", "~/work/logdaten/sauc-e_first_day/20110704-1551/orientation_estimator.0.log")

#now you can access all logged data by 
#addressing them by their task and port name
#log.task_name.port_name

#start deployment
#Orocos.run 'sonar_wall_hough_deployment', 'sonar_feature_estimator' do
Orocos.run 'sonar_wall_hough_deployment' do
    #Orocos.log_all
    #feature_estimator = Orocos::TaskContext.get 'sonar_feature_estimator'
    #feature_estimator.proportional_value_threshold = 0.2
    
    hough = Orocos::TaskContext.get 'sonar_wall_hough'
    hough.configure
    
    sonar = log.task 'sonar'
    compass = log.task 'orientation_estimator'
    #sonar.BaseScan.connect_to feature_estimator.sonar_input
    #sonar.SonarScan.connect_to feature_estimator.sonar_input
    #feature_estimator.filtered_sonarbeam.connect_to hough.input
    #sonar.SonarScan.connect_to hough.input
    sonar.BaseScan.connect_to hough.input
    compass.orientation_samples.connect_to hough.orientation
    
    Vizkit.display hough.peaks
    Vizkit.display hough.houghspace
    Vizkit.display hough.lines
    
    viewer = Vizkit.default_loader.StructViewer
    hough.position.connect_to viewer
    viewer.show
    
    #feature_estimator.start
    hough.start

    #open control widget and start replay
    #Vizkit.display sonar.SonarScan
    #Vizkit.display sonar.BaseScan
    Vizkit.control log
    Vizkit.exec
end
