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

asd = reconnect_loop(0)

def send_letter(letter):
    retries = 100
    while retries > 0:
        try:
            asd.write('{}'.format(letter))
            print('++ wrote {} to board'.format(letter))
            return
        except Exception as ex:
            print('!! send exception: {}'.format(str(ex)))
            retries -= 1

    reconnect_loop()

@app.route('/q/')
def action_key():
    send_letter('q')
    return default_return

@app.route('/w/')
def slow_fade():
    send_letter('w')
    return default_return

@app.route('/e/')
def medium_fade():
    send_letter('e')
    return default_return

@app.route('/r/')
def quick_fade():
    send_letter('r')
    return default_return

@app.route('/t/')
def swell():
    send_letter('t')
    return default_return

@app.route('/y/')
def stahp():
    send_letter('y')
    return default_return

@app.route('/u/')
def pal1():
    send_letter('u')
    return default_return

@app.route('/i/')
def pal2():
    send_letter(i)
    return default_return

@app.route('/o/')
def pal3():
    send_letter('o')
    return default_return

@app.route('/p/')
def pal4():
    send_letter('p')
    return default_return

@app.route('/left-bracket/')
def pal5():
    send_letter('[')
    return default_return

@app.route('/right-bracket/')
def pal6():
    send_letter(']')
    return default_return

@app.route('/a/')
def mode0():
    send_letter('a')
    return default_return

@app.route('/s/')
def mode1():
    send_letter('s')
    return default_return

@app.route('/d/')
def mode2():
    send_letter('d')
    return default_return

@app.route('/f/')
def mode3():
    send_letter('f')
    return default_return

@app.route('/g/')
def mode4():
    send_letter('g')
    return default_return

@app.route('/h/')
def mode5():
    send_letter('h')
    return default_return

@app.route('/j/')
def mode6():
    send_letter('j')
    return default_return

@app.route('/k/')
def mode7():
    send_letter('k')
    return default_return

@app.route('/l/')
def mode8():
    send_letter('l')
    return default_return

@app.route('/semi-colon/')
def mode9():
    send_letter(';')
    return default_return

@app.route('/apostrophe/')
def mode10():
    send_letter('\'')
    return default_return


if __name__ == '__main__':
    app.run(host='192.168.1.172', debug=True)
