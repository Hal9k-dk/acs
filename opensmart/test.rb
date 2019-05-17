#!/usr/bin/ruby

require 'serialport'

sp = SerialPort.new('/dev/ttyUSB0',
                    { 'baud' => 115200,
                      'data_bits' => 8,
                      'parity' => SerialPort::NONE
                    })
line = 0;
color = 0;
while true
  sleep 1
  puts line
  sp.printf "T%02d%02d0TEST TEST TEST%d\n" % [line, color, line]
  line = line + 1
  if line >= 8
    line = 0
  end
  color = color + 1
  if color >= 15
    color = 0
  end
  sp.printf "c%s\n" % Time.now.strftime("%H:%M")
end
