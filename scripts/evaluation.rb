require 'orocos/log'
include Orocos

Result = Struct.new(:real, :estimate, :error, :time)
result = []
g = []
e = []


log = Log::Replay.open("~/work/logdaten/groundtruth/pose_estimator.3.0.log", "~/work/Avalon/avalon/orogen/sonar_wall_hough/scripts/sonar_wall_hough_deployment.0.log")

log.pose_estimator.pose_samples do |sample|
  g << sample
  sample
end
log.sonar_wall_hough.position do |sample|
  e << sample
  sample
end

log.run

e.shift(5)
puts e[0].time
puts g[0].time
ptr = 0

error = 0.0
var = 0.0
min = 1_000_000.0
max = 0.0

puts ""

start = nil
last_sample = nil

for pos in e
    data = Result.new
    start = pos.time unless start

#    next if last_sample and (pos.time - last_sample.time).abs < 5.0

    while ptr < g.size and (pos.time - g[ptr].time).abs > 0.01
        ptr += 1
    end

    if g[ptr] and (pos.time - g[ptr].time).abs <= 0.01
      data.real = g[ptr].position
      data.estimate = pos.position
      
      x_error = (data.real[0] - data.estimate[0]) * (data.real[0] - data.estimate[0])
      y_error = (data.real[1] - data.estimate[1]) * (data.real[1] - data.estimate[1])
      data.error = Math.sqrt(x_error + y_error)
      data.time = pos.time - start

      max = data.error if data.error > max
      min = data.error if data.error < min

      error += data.error

      result << data

      last_sample = pos
    end

    printf "\r Processing [#{ptr}/#{e.size}]"
end

avg = error / result.size

File.open("evaluation.data", "w") do |fd|
    for pos in result
        fd.printf "#{pos.real[0]} #{pos.real[1]} #{pos.real[2]} "
        fd.printf "#{pos.estimate[0]} #{pos.estimate[1]} #{pos.estimate[2]} "
        fd.printf "#{pos.error} "
        fd.printf "#{pos.time} "
        fd.printf "\n"

        var += (avg - pos.error) * (avg - pos.error)
    end
end

puts "\nAverage Error: #{error / result.size}"
puts "Max Error: #{max}"
puts "Min Error: #{min}"
puts "Variance: #{var / (result.size - 1)}"
puts "Samples: #{result.size}"

system 'gnuplot ./templates/stats.plot'
