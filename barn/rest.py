import requests, json, yaml

class RestClient:
    def __init__(self):
        contents = open('settings.yml')
        yml = yaml.load(contents, Loader=yaml.SafeLoader)
        self.url = yml['settings']['resturl']
        self.token = yml['secrets']['apikey']
        print("URL %s" % self.url)
    
    def check_card(self, card_id):
        data = '{ "api_token": "%s", "card_id": "%s" }' % (self.token, card_id)
        response = requests.post(self.url + 'permissions', data=data, headers={"Content-Type": "application/json"}, verify=False)
        if response.status_code != 200:
            return { 'id': 0 }
        return response.json

    def log(self, id, msg):
        if id:
            data = '{ "api_token": "%s", "log": { "user_id": %d, "message": "%s" } }' % (self.token, id, msg)
        else:
            data = '{ "api_token": "%s", "log": { "message": "%s" } }' % (self.token, msg)
        response = requests.post(self.url + 'logs', data=data, headers={"Content-Type": "application/json"}, verify=False)
        return response.status_code == 200
        
if __name__ == "__main__":
    r = RestClient()
    cc = r.check_card("0000BB96C5")
    print(cc)
    print(r.log(cc['id'], 'Dummy log entry'))
