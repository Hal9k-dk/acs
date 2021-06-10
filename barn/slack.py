import requests

class Slack:

    def __init__(self):
        file = open('slack-token')
        self.token = file.read()
        self.last_status = None

    def set_status(self, status):
        if status != self.last_status:
            self.send_message(status)
            self.last_status = status

    def get_status(self):
        return self.last_status
  
    def send_message(self, msg):
        print("SLACK: %s" % msg)
        body = { 'channel': "monitoring", 'icon_emoji': ":panopticon:", 'parse': "full", "text": msg }
        headers = {
            'content_type': "application/json",
            "Authorization": "Bearer %s" % self.token
        }
        r = requests.post(url = "https://slack.com/api/chat.postMessage", data = body, headers = headers)

if __name__ == "__main__":
    s = Slack()
    s.send_message("Hej fra laden")
