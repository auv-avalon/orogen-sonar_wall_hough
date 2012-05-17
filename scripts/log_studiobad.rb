#library for displaying data
require 'vizkit'

include Orocos
Orocos.initialize

#load log file 
log = Orocos::Log::Replay.open("~/work/logdaten/avalonhd/20111118-1540/sonar.0.log", "~/work/logdaten/avalonhd/20111118-1540/orientation_estimator.0.log")

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
    hough.sensorAngularResolution = 4.950773558368496
    hough.maxDistance = 150
    hough.withMinimumFilter = false
    hough.angleDelta = 0
    hough.basinHeight = 10.0
    hough.basinWidth = 17.3
    hough.minLineVotesRatio = 0.2
    hough.configure
    
    sonar = log.task 'sonar'
    compass = log.task 'orientation_estimator'
    
    sonar.BaseScan.connect_to hough.input
    compass.orientation_samples.connect_to hough.orientation
    
    #view3d = Vizkit.default_loader.create_widget 'vizkit::Vizkit3DWidget'
    #sonar = view3d.createPlugin("uw_localization_particle", "ParticleVisualization")
    #walls = view3d.createPlugin("uw_localization_mapfeature", "MapFeatureVisualization")
    #view3d.show
    
    #Vizkit.connect_port_to 'sonar_wall_hough', 'map_wall_lines', :type => :buffer, :size => 100, :pull => false, :update_frequency => 33 do |sample, _|
    #    walls.updateLinemarks(sample)
    #    sample
    #end
    
    #Vizkit.connect_port_to 'uw_particle_localization', 'ppeaks', :type => :buffer, :size => 100, :pull => false, :update_frequency => 33 do |sample, _|
    #    sonar.updateParticles(sample)
    #    sample
    #end
    
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
