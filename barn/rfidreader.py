import serial
import threading
import time

class RfidReader(threading.Thread):
    
    def __init__(self, serial_port = "/dev/ttyUSB0"):
        threading.Thread.__init__(self)
        self.daemon = True
        self.lock = threading.Lock()
        self.tag_id = ''
        self.last = time.time()
        # Open port with default baud rate
        self.ser = serial.Serial(serial_port)
        # Reset Arduino
        self.ser.setDTR(False)
        time.sleep(1)
        self.ser.flushInput()
        self.ser.setDTR(True)
        # Reopen with proper baud rate
        self.ser = serial.Serial(port = serial_port,
            baudrate = 115200,
            timeout = 1.0,
            rtscts = 1,
            dsrdtr = False)
            
    def getid(self):
        id = ''
        self.lock.acquire()
        # Code expires after 10 seconds
        if time.time() - self.last < 10:
            id = self.tag_id
            self.tag_id = ''
        self.lock.release()
        return id
    
    def read_id(self):
        self.ser.write(b"C\n")
        line = self.ser.read_until().decode("utf-8") 
        line = line.strip()
        if line[0:2] == 'ID':
            line = line[2:]
        return line

    def run(self):
        while True:
            i = self.read_id()
            if len(i) > 0:
                self.lock.acquire()
                self.tag_id = i
                self.last = time.time()
                self.lock.release()
            time.sleep(0.1)
            
if __name__ == "__main__":
    print("init")
    l = RfidReader()
    l.start()
    
    for x in range(0, 20):
        print("ID %s" % l.getid())
        time.sleep(2)
        
        
