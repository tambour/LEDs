from flask import Flask
import serial
import time

app = Flask(__name__)
default_return = '<h2>Success!</h2>'
boom = True

def reconnect_loop(port=0):
    asd = None
    print('>> attempting connection to board on port {}...'.format(port))
    while asd is None:
        try:
          asd = serial.Serial(port='/dev/ttyACM{}'.format(port),baudrate=9600)
          print('>> connected to board on /dev/ttyACM{}'.format(port))
          return asd
        except Exception as ex:
            print('!! connection exception: {}'.format(str(ex)))
            if (port <= 3):
                time.sleep(1)
                return reconnect_loop(port=(port+1))
            else:
                print('-- board connection failed, sleeping...')
                time.sleep(5)
                port = 0

asd = reconnect_loop(0)

def send_letter(letter):
    retries = 10
    while retries > 0:
        try:
            asd.write('{}{}{}'.format(letter, letter, letter))
            print('++ wrote {} to board'.format(letter))
            return
        except Exception as ex:
            print('!! send exception: {}'.format(str(ex)))
            retries -= 1

    print('-- ran out of retries, resetting board on serial reconnect')
    reconnect_loop()

@app.route('/brightness/<path:choice>/')
def brightness(choice):
    if int(choice) == 0:
        send_letter(' ')
        print('---- sent space')
    elif int(choice) == 1:
        send_letter('!')
        print('---- sent exclamation')
    elif int(choice) == 2:
        send_letter('\"')
    elif int(choice) == 3:
        send_letter('#')
    elif int(choice) == 4:
        send_letter('$')
    elif int(choice) == 5:
        send_letter('%')
    return '<h2>Sent {}</h2>'.format(choice)

@app.route('/saturation/<choice>/')
def saturation(choice):
    if int(choice) == 0:
        send_letter('&')
    elif int(choice) == 1:
        send_letter('(')
    elif int(choice) == 2:
        send_letter(')')
    elif int(choice) == 3:
        send_letter('*')
    elif int(choice) == 4:
        send_letter('+')
    elif int(choice) == 5:
        send_letter(',')
    return '<h2>Sent {}</h2>'.format(choice)

@app.route('/mode/<choice>/')
def action_key(choice):
    if choice == 'left-bracket':
        send_letter('[')
    elif choice == 'right-bracket':
        send_letter(']')
    elif choice == 'semi-colon':
        send_letter('];')
    elif choice == 'apostrophe':
        send_letter('\'')
    elif choice == 'colon':
        send_letter(':')
    elif choice == 'quotation':
        send_letter('\"')
    elif choice == 'comma':
        send_letter(',')
    elif choice == 'period':
        send_letter('.')
    elif choice == 'greater-than':
        send_letter('>')
    elif choice == 'less-than':
        send_letter('<')
    elif choice == 'forward-slash':
        send_letter('/')
    elif choice == 'question-mark':
        send_letter('?')
    elif choice == 'back-slash':
        send_letter('\\')
    elif choice == 'pipe':
        send_letter('|')
    elif choice == 'left-curly':
        send_letter('{')
    elif choice == 'right-curly':
        send_letter('}')
    elif choice == 'plus':
        send_letter('+')
    elif choice == 'equals':
        send_letter('=')
    elif choice == 'hyphen':
        send_letter('-')
    elif choice == 'underscore':
        send_letter('_')
    else:
        send_letter(choice)
    return '<h2>Sent {}</h2>'.format(choice)

if __name__ == '__main__':
    app.run(host='10.10.10.172', debug=True)
    #app.run(host='192.168.1.172', debug=True)
