#library for displaying data
require 'vizkit'

include Orocos
Orocos.initialize

#load log file 
log = Orocos::Log::Replay.open("~/work/logdaten/localization/sonar.0.log", "~/work/logdaten/localization/state_estimator.0.log")

view3d = Vizkit.default_loader.create_widget 'vizkit::Vizkit3DWidget'
view3d.show_grid = true
view3d.show
ep = view3d.createPlugin("RigidBodyStateVisualization")

#start deployment
#Orocos.run 'sonar_wall_hough_deployment', 'sonar_feature_estimator' do
Orocos.run 'sonar_wall_hough_deployment' do
    #Orocos.log_all
    
    hough = Orocos::TaskContext.get 'sonar_wall_hough'
    hough.sensorAngularResolution = 5
    hough.maxDistance = 173
    hough.withMinimumFilter = true
    hough.angleDelta = 0
    hough.basinHeight = 10.0
    hough.basinWidth = 17.3
    hough.minLineVotesRatio = 0.1
    hough.gain = 10
    hough.show_debug = true
    hough.filterThreshold = 30
    hough.configure
    
    sonar = log.task 'sonar'
    compass = log.task 'state_estimator'
    
    sonar.sonar_beam.connect_to hough.sonar_samples
    compass.orientation_samples.connect_to hough.orientation_samples
    
    #Vizkit.connect_port_to 'sonar_wall_hough', 'position', :pull => false, :update_frequency => 33 do |sample, _|
    #    ep.updateRigidBodyState(sample)
    #    sample
    #end
    
    if hough.show_debug != 0
      #Vizkit.display hough.peaks
      Vizkit.display hough.houghspace
      Vizkit.display hough.lines
    end
    
    Vizkit.display hough
    hough.start

    Vizkit.control log
    Vizkit.exec
end
