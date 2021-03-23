#!/usr/bin/ruby

require 'serialport'

sp = SerialPort.new('/dev/ttyUSB0',
                    { 'baud' => 115200,
                      'data_bits' => 8,
                      'parity' => SerialPort::NONE
                    })
sleep 5
line = 0
color = 0
n = 0
erase = false
while true
  sleep 1
  sp.flush_input
  sp.puts "S"
  puts sp.gets
  if erase
    sp.printf "E%02d\n" % line
  else
    sp.printf "T%02d%02d1TEST TEST TEST%d\n" % [line, color, n]
  end
  #puts sp.gets
  line = line + (erase ? 2 : 1)
  n = n + 1
  if line >= 6
    line = 0
    erase = !erase
  end
  color = color + 1
  if color >= 15
    color = 0
  end
  sp.printf "c%s\n" % Time.now.strftime("%H:%M")
end
