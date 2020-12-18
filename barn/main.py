from gpiozero import LED
from time import sleep
from rfidreader import RfidReader
from rest import RestClient

reader = RfidReader()
reader.start()

restclient = RestClient()

lock = LED(4)

while True:
    card_id = reader.getid()
    print("Card ID %s" % card_id)
    if len(card_id) > 0:
        r = restclient.check_card(card_id)
        if r['id'] == 0:
            print("Card not found")
        else:
            print("Card found")
            username = r['name']
            if r['allowed']:
                user_approved = True
            else:
                user_approved = False
            if user_approved:
                lock.on()
                sleep(5)
                lock.off()
                
