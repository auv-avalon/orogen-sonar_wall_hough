#library for displaying data
require 'vizkit'

include Orocos
Orocos.initialize

#load log file 
log = Orocos::Log::Replay.open("~/work/logdaten/sauce12_sonar/day2/sonar.0.log", "~/work/logdaten/sauce12_sonar/day2/avalon_back_base_control.0.log")

#now you can access all logged data by 
#addressing them by their task and port name
#log.task_name.port_name

#start deployment
#Orocos.run 'sonar_wall_hough_deployment', 'sonar_feature_estimator' do
Orocos.run 'sonar_wall_hough_deployment' do
    #Orocos.log_all
    
    hough = Orocos::TaskContext.get 'sonar_wall_hough'
    hough.angleDelta = 0.0
    hough.show_debug = true
    hough.continous_write = false
    hough.sensorAngularResolution = 2.5
    hough.basinHeight = 50
    hough.basinWidth = 120
    hough.minDistance = 10
#    hough.filterThreshold = 30
#    hough.minLineVotesRatio = 0.05
    hough.configure
    
    sonar = log.task 'sonar'
    compass = log.task 'depth_orientation_fusion'
    
    sonar.sonar_beam.connect_to hough.sonar_samples
    compass.pose_samples.connect_to hough.orientation_samples
    
    Vizkit.display hough
    
    if hough.show_debug != 0
      Vizkit.display hough.peaks
      Vizkit.display hough.houghspace
      Vizkit.display hough.lines
      Vizkit.display sonar.sonar_beam
    end
    
    hough.start

    #open control widget and start replay
    Vizkit.control log
    Vizkit.exec
end
