#!/usr/bin/ruby
require 'optparse'
require 'serialport'
require 'rest-client'
require 'pg'

HOST = 'https://127.0.0.1'

LED_ENTER = 'P250R8SGN'
LED_NO_ENTRY = 'P100R30SRN'
LED_WAIT = 'P10R0SGNN'
LED_ERROR = 'P5R10SGX10NX100RX100N'
# Constant green
LED_OPEN = 'P200R10SG'
LED_CLOSING = 'P5R5SGX10NX100R'
LED_LOW_INTEN = 'I20'
LED_MED_INTEN = 'I50'
LED_HIGH_INTEN = 'I100'

# How many seconds green key must be held down to activate timed unlock
UNLOCK_KEY_TIME = 0.1
# How many seconds green key must be held down to activate Thursday mode
THURSDAY_KEY_TIME = 2

TEMP_STATUS_SHOWN_FOR = 5

UNLOCK_PERIOD_S = 15*60
UNLOCK_WARN_S = 5*60

$q = Queue.new
$api_key = File.read('apikey.txt').strip()
$db_pass = File.read('dbpass.txt').strip()
$opensmart = false

log_thread = Thread.new do
  puts "Thread start"
  while true
    e = $q.pop
    rest_start = Time.now
    begin
      url = "#{HOST}/api/v1/logs"
      response = RestClient::Request.execute(method: :post,
                                             url: url,
                                             timeout: 60,
                                             payload: { api_token: $api_key,
                                                        log: {
                                                          user_id: e["id"],
                                                          message: e["msg"]
                                                        }
                                                      }.to_json(),
                                             headers: {
                                               'Content-Type': 'application/json',
                                               'Accept': 'application/json'
                                             },
					     :verify_ssl => false)
      puts("log_thread: Got server reply in #{Time.now - rest_start} s")
    rescue Exception => e  
      puts "log_thread: #{e.class} Failed to connect to server"
    end
  end
end

def find_ports()
  r = {}
  for p in 0..1
    port = "/dev/ttyUSB#{p}"
    begin
      sp = SerialPort.new(port,
                          { 'baud' => 115200,
                            'data_bits' => 8,
                            'parity' => SerialPort::NONE,
                            'read_timeout' => 1
                          })
      if sp
        puts "Found #{port}"
        sp.read_timeout = 10
        begin
          while true
            sleep 5
            sp.puts("V")
            sleep 1
            begin
              line = sp.gets
            end while !line || line.empty?
            line.strip!
            reply = line.gsub(/[^[:print:]]/i, '')
            puts "Got #{line} -> #{reply}"
            if reply.empty?
              break
            end
            if reply.include? "ACS"
              puts("Version: #{reply}")
              if reply.include? "UI"
                r['ui'] = sp
                version = reply.gsub(/.* v /, '')
                $opensmart = version[0] == '1'
                break
              elsif reply.include? "cardreader"
                r['reader'] = sp
                break
              end
            end
          end
        end
      end
    rescue
      # No port here
    end
  end
  return r
end

def is_it_thursday?
  return (Date.today.strftime("%A") == 'Thursday') && (Time.now.hour >= 15);
end

def get_led_inten_cmd
  h = Time.now.hour
  if h < 5 || h > 20
    return LED_LOW_INTEN
  end
  if h < 8 || h > 16
    return LED_MED_INTEN
  end
  return LED_HIGH_INTEN
end

class Ui
  # Display lines for lock status
  STATUS_1 = 2
  STATUS_2 = 4
  ENTER_TIME_SECS = 5 # How long to keep the door open after valid card is presented

  def initialize(port)
    @port = port
    @port.flush_input
    @lock_state = :locked
    @unlock_time = nil
    if !$opensmart
      @color_map = [
        'white',
        'blue',
        'green',
        'red',
        'navy',
        'darkblue',
        'darkgreen',
        'darkcyan',
        'cyan',
        'turquoise',
        'indigo',
        'darkred',
        'olive',
        'gray',
        'grey',
        'skyblue',
        'blueviolet',
        'lightgreen',
        'darkviolet',
        'yellowgreen',
        'brown',
        'darkgray',
        'darkgrey',
        'sienna',
        'lightblue',
        'greenyellow',
        'silver',
        'lightgray',
        'lightgrey',
        'lightcyan',
        'violet',
        'azure',
        'beige',
        'magenta',
        'tomato',
        'gold',
        'orange',
        'snow',
        'yellow'
      ]
    else
      @color_map = [
        'white',
        'blue',
        'green',
        'red',
        'navy',
        'darkgreen',
        'darkcyan',
        'cyan',
        'maroon',
        'olive',
        'gray',
        'grey',
        'magenta',
        'orange',
        'yellow'
      ]
    end
    @last_time = ''
    @green_pressed_at = nil
    @unlocked_at = nil
    @last_status_1 = nil
    @last_status_2 = nil
    @reader = nil
    @temp_status_1 = ''
    @temp_status_2 = ''
    @temp_status_colour = ''
    @temp_status_at = nil
    @who = nil
  end

  def set_reader(reader)
    @reader = reader
  end
  
  def clear()
    send_and_wait("C")
  end

  def clear_line(large, line)
    send_and_wait(sprintf("#{large ? 'E' :'e'}%02d", line))
  end

  def write(large, erase, line, text, col = 'white')
    s = sprintf("#{large ? 'T' :'t'}%02d%02d%s%s",
                line, @color_map.find_index(col), erase ? '1' : '0', text)
    send_and_wait(s)
  end

  def unlock(who)
    if @lock_state == :timed_unlock || @lock_state == :unlocked
      return
    end
    @who = who
    @lock_state = :unlocking
    @unlock_time = Time.now
  end

  def set_temp_status(s1, s2 = '', colour = '')
    @temp_status_1 = s1
    @temp_status_2 = s2
    @temp_status_colour = colour
    @temp_status_at = Time.now
  end
  
  def wait_response(s)
    reply = ''
    while true
      c = @port.getc
      if c
        if c.ord == 13
          next
        end
        if c.ord == 10 && !reply.empty?
          break
        end
        reply = reply + c
      end
    end
    #puts "Reply: #{reply}"
    if reply != "OK #{s[0]}"
      puts "ERROR: Expected 'OK #{s[0]}', got '#{reply.inspect}' (in response to #{s})"
      Process.exit()
    end
  end

  def send_and_wait(s)
    #puts("Sending #{s}")
    @port.flush_input()
    @port.puts(s)
    wait_response(s)
  end

  def read_keys()
    @port.flush_input()
    @port.puts("S")
    reply = ''
    while true
      c = @port.getc
      if c
        if c.ord == 13
          next
        end
        if c.ord == 10 && !reply.empty?
          break
        end
        reply = reply + c
      end
    end
    #puts "Reply: #{reply}"
    if reply[0] != "S"
      puts "ERROR: Expected 'Sxx', got '#{reply.inspect}'"
      Process.exit()
    end
    if $opensmart
      return reply[1] == '1', reply[2] == '1', reply[3] == '1'
    else
      return reply[1] == '1', reply[2] == '1'
    end
  end
  
  def update()
    # Lock state
    col = 'green'
    s1 = ''
    s2 = ''
    if @lock_state == :unlocking
      elapsed = Time.now - @unlock_time
      if elapsed > ENTER_TIME_SECS
        puts "Lock again"
        @lock_state = :locked
      else
        send_and_wait("LT")
        col = 'blue'
        s1 = 'Enter'
        s2 = @who
      end
    end
    
    if @lock_state == :unlocking
      # nop
    elsif @lock_state == :locked
      send_and_wait("L0")
      col = 'orange'
      s1 = 'Locked'
    elsif @lock_state == :unlocked
      if !is_it_thursday?
        puts "Locking, no longer Thursday"
        @lock_state = :locked
      else
        send_and_wait("L1")
        col = 'green'
        s1 = 'Open'
        @reader.advertise_open()
      end
    elsif @lock_state == :timed_unlock
      send_and_wait("L1")
      col = 'green'
      s1 = 'Open for'
      locking_at = @unlocked_at + UNLOCK_PERIOD_S
      secs_left = (locking_at - Time.now).to_i
      mins_left = (secs_left/60.0).ceil
      #puts "Left: #{mins_left}m #{secs_left}s"
      if mins_left > 1
        s2 = "#{mins_left} minutes"
      else
        s2 = "#{secs_left} seconds"
      end
      if secs_left <= UNLOCK_WARN_S
        col = 'orange'
        @reader.warn_closing()
      else
        @reader.advertise_open()
      end
    else
      ui.clear();
      ui.write(true, false, 2, 'FATAL ERROR:', 'red')
      ui.write(true, false, 4, 'UNKNOWN LOCK STATE', 'red')
      puts("Fatal error: Unknown lock state")
      Process.exit
    end
    if !@temp_status_1.empty?
      shown_for = Time.now - @temp_status_at
      if shown_for > TEMP_STATUS_SHOWN_FOR
        @temp_status_1 = ''
      else
        s1 = @temp_status_1
        s2 = @temp_status_2
	if @temp_status_colour && @temp_status_colour != ''
          col = @temp_status_colour
        end
      end
    end
    if s1 != @last_status_1
      write(true, true, STATUS_1, s1, col)
      @last_status_1 = s1
    end
    if s2 != @last_status_2
      write(true, true, STATUS_2, s2, col)
      @last_status_2 = s2
    end
    # Buttons
    if $opensmart
      green, white, red = read_keys()
      if red
        puts "Red pressed at #{Time.now}"
        if @lock_state != :locked
          @lock_state = :locked
          @unlocked_at = nil
          @reader.add_log(nil, 'Door locked')
        end
      elsif green
        puts "Green pressed"
        if @lock_state != :timed_unlock
          @lock_state = :timed_unlock
          @unlocked_at = Time.now
          @reader.add_log(nil, "Door unlocked for #{UNLOCK_PERIOD_S} s")
          puts("Unlocked at #{@unlocked_at}")
        end
      elsif white
        puts "White pressed"
        if @lock_state != :unlocked
          if is_it_thursday?
            @lock_state = :unlocked
            @unlocked_at = nil
            @reader.add_log(nil, 'Door unlocked')
          else
            @temp_status_1 = 'It is not'
            @temp_status_2 = 'Thursday yet'
            @temp_status_at = Time.now
          end
        end
      end
    else
      # Not OpenSmart
      red, green = read_keys()
      if red
        if @lock_state != :locked
          @reader.add_log(nil, 'Door locked')
        end
        @lock_state = :locked
        @unlocked_at = nil
      elsif green && @lock_state != :unlocked
        if !@green_pressed_at
          @green_pressed_at = Time.now
        end
      else
        if @green_pressed_at
          # Release
          green_pressed_for = Time.now - @green_pressed_at
          if green_pressed_for >= THURSDAY_KEY_TIME
            if is_it_thursday?
              @lock_state = :unlocked
              @reader.add_log(nil, 'Door unlocked')
            else
              @temp_status_1 = 'It is not'
              @temp_status_2 = 'Thursday yet'
              @temp_status_at = Time.now
            end
          elsif green_pressed_for >= UNLOCK_KEY_TIME && !@unlocked_at
            @lock_state = :timed_unlock
            @unlocked_at = Time.now
            @reader.add_log(nil, "Door unlocked for #{UNLOCK_PERIOD_S} s")
            puts("Unlocked at #{@unlocked_at}")
          end
        end
        @green_pressed_at = nil
      end
    end
    # Automatic locking
    if @unlocked_at
      unlocked_for = Time.now - @unlocked_at
      if unlocked_for >= UNLOCK_PERIOD_S
        puts "Unlocked for #{unlocked_for}"
        @unlocked_at = nil
        @lock_state = :locked
      end
    end
    # Time display
    ct = DateTime.now.to_time.strftime("%H:%M")
    if ct != @last_time
      write(false, true, $opensmart ? 10 : 12, ct, 'blue')
      @last_time = ct
      @reader.send(get_led_inten_cmd())
    end
  end
end

class CardReader
  def initialize(port)
    @port = port
    @port.flush_input
    @last_card = ''
    @last_card_read_at = Time.now()
    @last_card_seen_at = Time.now()
    @last_led_write_at = Time.now()
  end

  def set_ui(ui)
    @ui = ui
  end

  def send(s)
    @port.flush_input
    #puts "Send(#{Time.now}): #{s}"
    @port.puts(s)
    @port.flush_output
    begin
      line = @port.gets
    end while !line || line.empty?
    line.strip!
    #puts "Reply: #{line}"
    if line != "OK"
      puts "ERROR: Expected 'OK', got '#{line}' (in response to #{s})"
      Process.exit()
    end
  end
  
  def warn_closing()
    if Time.now - @last_led_write_at < 1
      return
    end
    @last_led_write_at = Time.now
    send(LED_CLOSING)
  end
  
  def advertise_open()
    if Time.now - @last_led_write_at < 1
      return
    end
    @last_led_write_at = Time.now
    send(LED_OPEN)
  end

  def check_permission(id)
    db_start = Time.now
    allowed = false
    error = false
    who = ''
    begin
      conn = PG.connect(host: 'localhost', dbname: 'acs_production', user: 'acs', password: $db_pass)
      res = conn.exec("SELECT u.id, u.name FROM users u join machines_users mu on mu.user_id = u.id join machines m on m.id = mu.machine_id where u.card_id = '#{id}' and m.name='Door' and u.active = true")
      puts("Got #{res.ntuples()} tuples from DB in #{Time.now - db_start} s")
      if res && res.ntuples() > 0
        allowed = true
        user_id = res[0]["id"]
        who = res[0]["name"]
        puts("User: #{user_id} #{who}")
      end
    rescue Exception => e  
      puts "#{e.class} Failed to connect to DB"
      error = true
    end
    return allowed, error, who, user_id
  end
  
  def add_log(id, msg)
    $q << { 'id' => id, 'msg' => msg }
  end
  
  def add_unknown_card(id)
    rest_start = Time.now
    error = false
    begin
      url = "#{HOST}/api/v1/unknown_cards"
      response = RestClient::Request.execute(method: :post,
                                             url: url,
                                             timeout: 60,
                                             payload: { api_token: $api_key,
                                                        card_id: id
                                                      }.to_json(),
                                             headers: {
                                               'Content-Type': 'application/json',
                                               'Accept': 'application/json'
                                             },
					     :verify_ssl => false)
      puts("Got server reply in #{Time.now - rest_start} s")
    rescue Exception => e  
      puts "#{e.class} Failed to connect to server"
      error = true
    end
    return !error
  end
  
  def update()
    if Time.now - @last_card_read_at < 1
      return
    end
    @last_card_read_at = Time.now
    @port.flush_input
    @port.puts("C")
    #puts "Send(#{Time.now}): C"
    begin
      line = @port.gets
    end while !line || line.empty?
    line.strip!
    #puts "Reply: #{line}"
    if !line || line.empty? || line[0..1] != "ID"
      puts "ERROR: Expected 'IDxxxxxx', got '#{line}' (in response to C)"
      return
    end
    line = line[2..-1]
    if !line.empty? && line.length != 10
      puts "Invalid card ID: #{line}"
      line = ''
    end
    now = Time.now()
    if !line.empty? && ((line != @last_card) || (now - @last_card_seen_at > 5))
      puts "Card ID: #{line}"
      @last_card = line
      @last_card_seen_at = now
      # Let user know we are doing something
      send(LED_WAIT)
      allowed, error, who, user_id = check_permission(@last_card)
      if error
        send(LED_ERROR)
      else
        if allowed == true
          send(LED_ENTER)
          @ui.unlock(who)
          add_log(user_id, 'Granted entry')
        elsif allowed == false
          send(LED_NO_ENTRY)
          if user_id
            add_log(user_id, 'Denied entry')
            @ui.set_temp_status('Denied entry:', who, 'red')
          else
            add_log(user_id, "Denied entry for #{@last_card}")
            add_unknown_card(@last_card)
            @ui.set_temp_status('Unknown card', @last_card, 'yellow')
          end
        else
          puts("Impossible! allowed is neither true nor false: #{allowed}")
          send(LED_ERROR)
        end
      end
    end
  end

end # end CardReader

ports = find_ports()
if !ports['ui']
  puts("Fatal error: No UI found")
  Process.exit
end

ui = Ui.new(ports['ui'])
ui.clear();

if !ports['reader']
  ui.write(true, false, 2, 'FATAL ERROR:', 'red')
  ui.write(true, false, 4, 'NO READER FOUND', 'red')
  puts("Fatal error: No card reader found")
  Process.exit
end

reader = CardReader.new(ports['reader'])
reader.set_ui(ui)
ui.set_reader(reader)

puts("----\nReady")
ui.clear();

while true
  ui.update()
  reader.update()
  sleep 0.1
  #puts "loop"
end
