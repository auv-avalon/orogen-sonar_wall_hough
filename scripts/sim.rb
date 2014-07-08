require 'vizkit'
include Orocos

Orocos.initialize

widget = Vizkit.load "simulator.ui"

#Orocos.run "AvalonSimulation" ,:wait => 10000, :valgrind => false, :valgrind_options => ['--undef-value-errors=no'] do 
Orocos.run "AvalonSimulation", "sonar_wall_hough", "auv_control::AccelerationController" => "acceleration_controller", :wait => 100, :valgrind => false, :valgrind_options => ['--undef-value-errors=no'] do 
  #Orocos.log_all  
  simulation = TaskContext.get 'avalon_simulation'
    
      white_light = TaskContext.get 'white_light'
#     white_light.interval_mode = 1
#     white_light.constantInterval = 3000
      white_light.interval_mode = 2
      white_light.randomInterval_min = 1;
      white_light.randomInterval_max = 5000;
      white_light.start
      

    #simulation.debug_sonar = false 
    #simulation.use_osg_ocean = false 
    #simulation.enable_gui = true
    #simulation.laserOpeningAngle = 0.3
    #simulation.laserFrontAngle = 0.5
    #simulation.laserColor = [[0.0, 1.0, 0.0]]
    simulation.apply_conf_file("/home/fabio/avalon/bundles/avalon/config/orogen/simulation::Mars.yml")
    simulation.configure
    simulation.start
    
    laser = TaskContext.get 'lineLaser'
    laser.laserColor = [[0.0, 1.0, 0.0]]
    laser.frontAngle = 0.3
    laser.configure
    laser.start

# Konfiguration der Aktuatoren für alle Fahrzeuge:    
require 'actuators'
values = ActuatorsConfig.new()
    
    actuators = TaskContext.get 'avalon_actuators'
    actuators.node_name = "avalon"
    actuators.amount_of_actuators = values.avalon_amount_of_actuators
    actuators.maximum_thruster_force = values.avalon_maximum_thruster_force    
    actuators.thruster_position = values.avalon_thruster_position    
    actuators.thruster_direction = values.avalon_thruster_direction    
    actuators.linear_damp = [[8, 24, 24, 10, 10, 10]]
    actuators.square_damp = [[0, 0, 0, 0, 0, 0]]
    actuators.thruster_coefficients = [0.005, 0.005, 0.005, 0.005, 0.005, 0.005]
    actuators.buoyancy_force = 72
    actuators.cob = [[0, 0, 0.08]]
    actuators.voltage = 28
    actuators.configure
    actuators.start
    

# Camera configuration

    front_cam = TaskContext.get 'front_camera'
    front_cam.name = 'front_cam'
    #front_cam.configure
    #front_cam.start
    
    bottom_cam = TaskContext.get 'bottom_camera'
    bottom_cam.name = 'bottom_cam'
    #bottom_cam.configure
    #bottom_cam.start
    
    #top_cam = TaskContext.get 'top_camera'
    #top_cam.name = 'top_cam'
    #top_cam.configure
    #top_cam.start


    sonar = TaskContext.get 'sonar'
    sonar.node_name = "sonar_top_sensor"
    sonar.left_limit = Math::PI
    sonar.right_limit = -Math::PI
    sonar.resolution = 0.1
    sonar.maximum_distance = 100.0
    sonar.ping_pong_mode = false
    sonar.configure
    sonar.start
    
    sonar_rear = TaskContext.get 'sonar_rear'
    sonar_rear.node_name = "sonar_rear_sensor"
    sonar_rear.left_limit = 0.7*Math::PI
    sonar_rear.right_limit = 0.3*Math::PI
    sonar_rear.resolution = 0.1
    sonar_rear.maximum_distance = 50.0
    sonar_rear.ping_pong_mode = true
    sonar_rear.configure
    sonar_rear.start
        
    ground_distance = TaskContext.get 'ground_distance'
    ground_distance.node_name = "ground_distance_sensor"
    ground_distance.configure
    ground_distance.start

    imu = TaskContext.get 'imu'
    imu.name = "avalon"
    imu.configure
    imu.start
    
    hough = TaskContext.get 'sonar_wall_hough'
    hough.usePositionSamples =false
    hough.correctToFirstPosition=true
    hough.filterThreshold=70
    hough.withMinimumFilter=false
    hough.show_debug=true
    hough.basinHeight = 19 #20
    hough.basinWidth = 23 #24
    hough.angleDelta = 0.0
    hough.minDistance = 1.0
    hough.maxDistance=700
    #hough.anglesPerBin = 1#2
    #hough.distancesPerBin = 1
    hough.minLineVotesRatio = 0.01
    hough.gain = 5    
    
    hough.ignoreOrientation = true
    hough.sensorAngularTolerance = 20     
      
    sonar.sonar_beam.connect_to hough.sonar_samples, :type => :buffer, :size => 1000
    imu.pose_samples.connect_to hough.orientation_samples
    #state_estimator.pose_samples.connect_to hough.pose_samples #Ersatz für UW-Lokalisierung  
    

    hough.configure
    hough.start
    
#Control Draft
    
###########################ACCELERATION_CONTROLLER
    acceleration_controller = TaskContext.get 'acceleration_controller'
    
    expected = acceleration_controller.expected_inputs
    expected.linear[0] = true
    expected.linear[1] = true
    expected.linear[2] = true
    expected.angular[0] = true
    expected.angular[1] = true
    expected.angular[2] = true
    acceleration_controller.expected_inputs = expected
    
    acceleration_controller.timeout_in = 0
    acceleration_controller.timeout_cascade = 0

    matrix = acceleration_controller.matrix
    
    matrix.resize(6,6)
    matrix[0, 0] = 0
    matrix[0, 1] = 0
    matrix[0, 2] = -1 #-0.6
    matrix[0, 3] = -1 #-0.65
    matrix[0, 4] = 0
    matrix[0, 5] = 0

    matrix[1, 0] = 0
    matrix[1, 1] = 0
    matrix[1, 2] = 0
    matrix[1, 3] = 0
    matrix[1, 4] = 0.0001 #0.2
    matrix[1, 5] = -0.8 #-0.55

    matrix[2, 0] = 0.02 #0.3
    matrix[2, 1] = 1.0 #-0.6
    matrix[2, 2] = 0
    matrix[2, 3] = 0
    matrix[2, 4] = 0
    matrix[2, 5] = 0

    matrix[3, 0] = 0
    matrix[3, 1] = 0
    matrix[3, 2] = 0
    matrix[3, 3] = 0
    matrix[3, 4] = 0
    matrix[3, 5] = 0

    matrix[4, 0] = 1.0 #0.45
    matrix[4, 1] = -0.1 #0.15
    matrix[4, 2] = 0
    matrix[4, 3] = 0
    matrix[4, 4] = 0
    matrix[4, 5] = 0

    matrix[5, 0] = 0
    matrix[5, 1] = 0
    matrix[5, 2] = 0
    matrix[5, 3] = 0
    matrix[5, 4] = -1.0 #-0.6
    matrix[5, 5] = -0.2 #-0.4
    acceleration_controller.matrix = matrix

    acceleration_controller.cmd_out.connect_to(actuators.command) 

    acceleration_controller.configure
    acceleration_controller.start

    
    writer_ac_in = acceleration_controller.cmd_in.writer
    writer_ac_cascade = acceleration_controller.cmd_cascade.writer

   
        sample = writer_ac_cascade.new_sample
        sample.time = Time.now
        sample.linear[0] = NaN
        sample.linear[1] = NaN
        sample.linear[2] = NaN
        sample.angular[0] = 0
        sample.angular[1] = 0
        sample.angular[2] = NaN
        writer_ac_cascade.write(sample)

        sample = writer_ac_in.new_sample
        sample.time = Time.now
        sample.linear[0] = 0.1
        sample.linear[1] = 0
        sample.linear[2] = 0.0
        sample.angular[0] = NaN
        sample.angular[1] = NaN
        sample.angular[2] = 0
        writer_ac_in.write(sample)

    widget.joystick1.connect(SIGNAL('axisChanged(double,double)'))do |x,y|
        sample.time = Time.now
        sample.linear[0] = x
        sample.angular[2] = y
        writer_ac_in.write(sample)
    end

    widget.joystick2.connect(SIGNAL('axisChanged(double,double)'))do |x,y|
        sample.linear[2] = x
        sample.linear[1] = y - 0.4
        writer_ac_in.write sample
    end
    
    #widget.horizontalSlider_1.connect(SIGNAL('valueChanged(int)'))do |x|
    #    sample.target[1] = x/100.0
#        writer.write sample
#    end


    widget.show     
    
    #Vizkit.display sonar
    Vizkit.display hough
    Vizkit.display hough.lines
    Vizkit.exec

end

