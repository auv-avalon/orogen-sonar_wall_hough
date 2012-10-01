#library for displaying data
require 'vizkit'

include Orocos
Orocos.initialize

#load log file 
log = Orocos::Log::Replay.open("~/work/logdaten/groundtruth/sonar.6.0.log", "~/work/logdaten/groundtruth/pose_estimator.6.0.log")
#log = Orocos::Log::Replay.open("~/work/logdaten/groundtruth/sonar.3.0.log", "~/work/logdaten/groundtruth/pose_estimator.3.0.log")

#now you can access all logged data by 
#addressing them by their task and port name
#log.task_name.port_name

view3d = Vizkit.default_loader.create_widget 'vizkit::Vizkit3DWidget'
view3d.show_grid = false
view3d.show
ep = view3d.createPlugin("RigidBodyStateVisualization")
gt = view3d.createPlugin("RigidBodyStateVisualization")

#start deployment
Orocos.run 'sonar_wall_hough_deployment' do
    Orocos.log_all
    
    hough = Orocos::TaskContext.get 'sonar_wall_hough'
    hough.sensorAngularResolution = 4.950773558368496
    hough.maxDistance = 600
    hough.withMinimumFilter = true
    hough.angleDelta = 0
    hough.basinHeight = 10.0
    hough.basinWidth = 17.3
    hough.minLineVotesRatio = 0.1
    hough.show_debug = false
    hough.continous_write = false
    hough.configure
    
    sonar = log.task 'sonar'
    compass = log.task 'pose_estimator'
    
    sonar.SonarScan.connect_to hough.sonar_samples
    compass.pose_samples.connect_to hough.orientation_samples
    
    Vizkit.connect_port_to 'sonar_wall_hough', 'position', :pull => false, :update_frequency => 33 do |sample, _|
        ep.updateRigidBodyState(sample)
        sample
    end
    Vizkit.display hough
    
    log.pose_estimator.pose_samples do |sample|
        gt.updateRigidBodyState(sample)
        sample
    end
    
    if hough.show_debug != 0
      Vizkit.display hough.peaks
      Vizkit.display hough.houghspace
      Vizkit.display hough.lines
    end
    
    hough.start

    #open control widget and start replay
    Vizkit.control log
    Vizkit.exec
end
