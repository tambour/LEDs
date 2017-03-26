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

def send_value(value):
    retries = 100
    while retries > 0:
        try:
            asd.write(value)
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

@app.route('/b0/')
def brightness0():
    send_value(238)
    return default_return
@app.route('/b1/')
def brightness1():
    send_value(239)
    return default_return
@app.route('/b2/')
def brightness2():
    send_value(240)
    return default_return
@app.route('/b3/')
def brightness3():
    send_value(241)
    return default_return
@app.route('/b4/')
def brightness4():
    send_value(242)
    return default_return
@app.route('/b5/')
def brightness5():
    send_value(243)
    return default_return
@app.route('/b6/')
def brightness6():
    send_value(244)
    return default_return
@app.route('/b7/')
def brightness7():
    send_value(245)
    return default_return
@app.route('/b8/')
def brightness8():
    send_value(246)
    return default_return
@app.route('/b9/')
def brightness9():
    send_value(247)
    return default_return
@app.route('/b10/')
def brightness10():
    send_value(248)
    return default_return
@app.route('/b11/')
def brightness11():
    send_value(249)
    return default_return
@app.route('/b12/')
def brightness12():
    send_value(250)
    return default_return
@app.route('/b13/')
def brightness13():
    send_value(251)
    return default_return
@app.route('/b14/')
def brightness14():
    send_value(252)
    return default_return
@app.route('/b15/')
def brightness15():
    send_value(253)
    return default_return
@app.route('/b16/')
def brightness16():
    send_value(254)
    return default_return
@app.route('/b17/')
def brightness17():
    send_value(255)
    return default_return

@app.route('/s0/')
def saturation0():
    send_value(220)
    return default_return
@app.route('/s1/')
def saturation1():
    send_value(221)
    return default_return
@app.route('/s2/')
def saturation2():
    send_value(222)
    return default_return
@app.route('/s3/')
def saturation3():
    send_value(223)
    return default_return
@app.route('/s4/')
def saturation4():
    send_value(224)
    return default_return
@app.route('/s5/')
def saturation5():
    send_value(225)
    return default_return
@app.route('/s6/')
def saturation6():
    send_value(226)
    return default_return
@app.route('/s7/')
def saturation7():
    send_value(227)
    return default_return
@app.route('/s8/')
def saturation8():
    send_value(228)
    return default_return
@app.route('/s9/')
def saturation9():
    send_value(229)
    return default_return
@app.route('/s10/')
def saturation10():
    send_value(230)
    return default_return
@app.route('/s11/')
def saturation11():
    send_value(231)
    return default_return
@app.route('/s12/')
def saturation12():
    send_value(232)
    return default_return
@app.route('/s13/')
def saturation13():
    send_value(233)
    return default_return
@app.route('/s14/')
def saturation14():
    send_value(234)
    return default_return
@app.route('/s15/')
def saturation15():
    send_value(235)
    return default_return
@app.route('/s16/')
def saturation16():
    send_value(236)
    return default_return
@app.route('/s17/')
def saturation17():
    send_value(237)
    return default_return


if __name__ == '__main__':
    app.run(host='10.10.10.172', debug=True)
