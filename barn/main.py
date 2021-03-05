from gpiozero import LED
from time import sleep
from rfidreader import RfidReader
from rest import RestClient
from display import Display

reader = RfidReader()
reader.start()

restclient = RestClient()

disp = Display()
disp.println("BACS 0.1 ready")

lock = LED(4)

while True:
    card_id = reader.getid()
    if len(card_id) > 0:
        disp.println("Checking card...")
        print("Card ID %s" % card_id)
        try:
            r = restclient.check_card(card_id)
        except:
            disp.println("Error accessing ACS")
            sleep(1)
            continue
        if r['id'] == 0:
            print("Card not found")
            disp.println("Card %s not found" % card_id)
        else:
            print("Card found")
            username = r['name']
            disp.println("User: %s" % username)
            if r['allowed']:
                user_approved = True
            else:
                user_approved = False
                disp.println("Not allowed")
            if user_approved:
                disp.println("Opening")
                lock.on()
                sleep(5)
                lock.off()
            else:
                sleep(5)
                
