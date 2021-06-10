from gpiozero import LED
from time import sleep
from rfidreader import RfidReader
from slack import Slack
from rest import RestClient
from display import Display
import urllib3

VERSION = "0.2"

# Yes, we are using a self-signed cert
urllib3.disable_warnings(urllib3.exceptions.InsecureRequestWarning)

reader = RfidReader()
reader.start()

restclient = RestClient()

disp = Display()
disp.println("BACS %s ready" % VERSION)

lock = LED(4)

slack = Slack()

last_card_id = None
print("Ready")
slack.set_status("BACS version %s starting" % VERSION)
while True:
    card_id = reader.getid()
    if len(card_id) > 0:
        if card_id != last_card_id:
            print("Card ID %s" % card_id)
            disp.println("Checking card...")
            last_card_id = card_id
            try:
                r = restclient.check_card(card_id)
            except:
                disp.println("Error accessing ACS")
                sleep(1)
                continue
            if r['id'] == 0:
                print("Card not found")
                disp.println("Card %s not found" % card_id)
                slack.set_status("BACS: Unrecognized card %s" % card)
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
                    sleep(20)
                    disp.println("Closing")
                    lock.off()
                
