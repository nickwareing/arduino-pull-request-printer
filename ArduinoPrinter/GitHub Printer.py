from time import sleep
import requests
import json
import argparse
import ConfigParser

# GitHub
headers = {'Authorization': ''}
baseUrl = ''
owner = ''

# Arduino
url = 'http://10.1.1.11'

# TeamX Devs
teamXdevs = {
    'dev-joem': 'Joe',
    'dev-mingl': 'Ming',
    'dev-elisac': 'Elisa',
    'dev-markc': 'Mark',
    'dev-dano': 'Dan',
    'dev-fayp': 'Fay',
    'dev-jamesm': 'James',
    'dev-regang': 'Regan'
}

latestPR = {}


def post_to_printer(pr, repo):
    title = pr['title']
    user = teamXdevs[pr['head']['user']['login']]
    message = "**{0} opened PR in {2}:** {1}".format(user, title, repo)
    #print json.dumps(pr, indent=4, sort_keys=True)
    r = requests.get(url + '?p=' + message)
    print(r.status_code)


def post_new_line():
    r = requests.get(url + '?p= ')
    print(r.status_code)


def parse_and_print(PRs, latest):
    temp_num = None
    for PR in PRs:
        user = PR['head']['user']['login']
        number = PR['number']
        repo = PR['head']['repo']['name']
        if user in teamXdevs and number > latest:
            post_to_printer(PR, repo)
            temp_num = max(number, temp_num)
    if temp_num:
        latest = temp_num
    return latest


def poll(repos):
    changed = False
    global latestPR

    for repo in repos:
        if repo not in latestPR:
            latestPR[repo] = 0

        PRs = requests.get(baseUrl + 'repos/' + owner + '/' + repo +'/pulls', headers=headers, verify=False)

        if PRs.ok:
            PRsText = json.loads(PRs.text or PRs.content)
            tempNum = parse_and_print(PRsText, latestPR[repo])
            if tempNum > latestPR[repo]:
                changed = True
                latestPR[repo] = tempNum

    if changed:
        post_new_line()


if __name__ == "__main__":
    # Read values from config file
    Config = ConfigParser.ConfigParser()
    Config.read("config.ini")
    section = 'GitHub'
    try:
        baseUrl = Config.get(section, 'baseUrl')
        owner = Config.get(section, 'owner')
        token = Config.get(section, 'token')
        headers['Authorization'] = 'token ' + token
    except KeyError:
        print("Unable to find config option")

    parser = argparse.ArgumentParser()
    parser.add_argument("-period", type=int,
                    help="period at which to poll")
    parser.add_argument('repositories', metavar='R', nargs='+',
                       help='a list of repositories to poll')

    args = parser.parse_args()
    reposToPoll = args.repositories

    while True:
        poll(reposToPoll)
        sleep(args.period)