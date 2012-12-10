require 'vizkit'
include Orocos

Orocos.initialize

#AvalonSimulation.0.log: Geradeaus fahren
#AvalonSimulation.1.log: Kurven fahren
#AvalonSimulation.2.log: Drehen
log = Orocos::Log::Replay.open("AvalonSimulation.0.log")

Orocos.run "sonar_wall_hough","uw_particle_localization_test"  ,:wait => 10000, :valgrind => false, :valgrind_options => ['--undef-value-errors=no'] do
  
  sonar = log.task 'sonar'
  state_estimator = log.task 'state_estimator'
  sim = log.task 'avalon_simulation'
  actuators = log.task 'actuators'
  
   
  hough = TaskContext.get 'sonar_wall_hough'
  hough.usePositionSamples =true
  hough.correctToFirstPosition=true
  hough.filterThreshold=70
  hough.withMinimumFilter=false
  hough.show_debug=true
  hough.basinHeight = 50
  hough.basinWidth = 120
  hough.angleDelta = 0.0
  hough.maxDistance=700
  
  pos = TaskContext.get 'uw_particle_localization'
    pos.init_position = [0.0, 0.0, 0.0]
    pos.init_variance = [1.0, 1.0, 0.0]

    pos.static_motion_covariance = [0.00001,0.0,0.0, 0.0,0.00001,0.0, 0.0,0.0,0.0]
    pos.pure_random_motion = true
    pos.particle_number = 50
    pos.minimum_depth = 0.0
    pos.minimum_perceptions = 3
    pos.effective_sample_size_threshold = 0.9
    pos.hough_interspersal_ratio = 0.1
    pos.sonar_maximum_distance = 12.0
    pos.sonar_covariance = 4.0
    pos.yaml_map = File.join("nurc.yml")
  
  sonar.sonar_beam.connect_to hough.sonar_samples, :type => :buffer, :size => 100
  state_estimator.pose_samples.connect_to hough.orientation_samples
  state_estimator.pose_samples.connect_to pos.orientation_samples
  #pos.dead_reckoning_samples.connect_to hough.pose_samples
  state_estimator.pose_samples.connect_to hough.pose_samples #Ersatz für UW-Lokalisierung
  actuators.status.connect_to pos.thruster_samples
  
  pos.configure
  pos.start
  
  hough.configure
  hough.start 

  
  Vizkit.display sim
  Vizkit.display hough
  Vizkit.display sonar
  Vizkit.display pos 
  Vizkit.display state_estimator
  Vizkit.control log
  Vizkit.exec
end  