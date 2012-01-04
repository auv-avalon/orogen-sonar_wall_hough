#library for displaying data
require 'vizkit'

include Orocos
Orocos.initialize


#load log file 
log = Orocos::Log::Replay.open("~/work/logdaten/slam/sonar.19.log")

#now you can access all logged data by 
#addressing them by their task and port name
#log.task_name.port_name

#start deployment
Orocos.run 'sonar_wall_hough_deployment', 'sonar_feature_estimator' do 
    feature_estimator = Orocos::TaskContext.get 'sonar_feature_estimator'
    hough = Orocos::TaskContext.get 'sonar_wall_hough'
    hough.config do |c|
	c.sensorAngularResolution = 1.8
	c.anglesPerBin = 2
	c.maxDistance = 600
	c.distancesPerBin = 4
	c.minLineVotes = 20
    end    
    hough.configure
    
    sonar = log.task 'sonar'
    sonar.SonarScan.connect_to feature_estimator.sonar_input
    feature_estimator.filtered_sonarbeam.connect_to hough.input
    
    Vizkit.display hough.peaks
    Vizkit.display hough.houghspace
    Vizkit.display hough.lines
    
    feature_estimator.start
    hough.start

    #open control widget and start replay
    #Vizkit.display sonar.SonarScan
    Vizkit.control log
    Vizkit.exec
end
