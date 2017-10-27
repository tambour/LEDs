import serial
import time
import rtmidi
import random

control_down = False

def reconnect_loop(port=0):
    asd = None
    print('>> attempting connection to board on port {}...'.format(port))
    while asd is None:
        try:
          asd = serial.Serial(port='/dev/tty.usbmodem14121'.format(port),baudrate=9600)
          print('>> connected to board on /dev/ttyACM{}'.format(port))
          return asd
        except Exception as ex:
            print('!! connection exception: {}'.format(str(ex)))
            if (port <= 3):
                time.sleep(1)
                return reconnect_loop(port=(port+1))
            else:
                print('-- board connection failed, sleeping...')
                time.sleep(2)
                port = 0

asd = reconnect_loop(0)

def send_letter(letter):
    retries = 10
    while retries > 0:
        try:
            asd.write('{}{}{}'.format(str(chr(letter)),str(chr(letter)),str(chr(letter))))
            #print('++ wrote {} to board'.format(letter))
            return
        except Exception as ex:
            print(' !! send exception: {}'.format(str(ex)))
            retries -= 1

    print('-- ran out of retries, resetting board on serial reconnect')
    reconnect_loop()

def print_message(midi):
    if midi.isNoteOn():
        print('ON: ', midi.getMidiNoteName(midi.getNoteNumber()), midi.getVelocity())
    elif midi.isNoteOff():
        print('OFF:', midi.getMidiNoteName(midi.getNoteNumber()))
    elif midi.isController():
        print('CONTROLLER', midi.getControllerNumber(), midi.getControllerValue())

def test():
    time.sleep(2)
    delay = 0.05
    value = 230
    dir = 1
    while True:
        send_letter(value)
        time.sleep(delay)
        value += 1

        if value == 256:
            value = 60
        if value == 86:
            value = 230

def main():
    # open midi device for listening
    midi_in = rtmidi.RtMidiIn()

    global control_down

    ports = range(midi_in.getPortCount())
    if ports:
        print('available ports:')
        for i in ports:
            print(midi_in.getPortName(i))
            midi_in.openPort(i)

        CONST = 47 # up one octave
        #CONST = 35 # normal octave

        # poll for midi events
        # note on/off: calculate led index and send to board
        while True:
            m = midi_in.getMessage(250) # some timeout in ms
            if m:
                #print_message(m)
                if m.isNoteOn():
                    num = int((m.getNoteNumber()-CONST)*0.5)+230
                    #print(num)
                    if not control_down:
                        send_letter(num)
                    n = m.getNoteNumber()
                    note = m.getMidiNoteName(m.getNoteNumber())
                    v = m.getVelocity()
                    

                    if control_down:

                        # modes
                        if n == 48:
                            send_letter(97) # mode 0
                            print(' 00 setting mode 0 (solid)')
                        elif n == 49:
                            send_letter(115) # mode 1
                            print(' 01 setting mode 1 (solid moving)')
                        elif n == 50:
                            send_letter(100) # mode 2
                            print(' 02 setting mode 2 (gradient)')
                        elif n == 51:
                            send_letter(102) # mode 3
                            print(' 03 setting mode 3 (gradient moving)')
                        elif n == 52:
                            send_letter(103) # mode 4
                            print(' 04 setting mode 4 (gradient moving palette)')
                        elif n == 57:
                            send_letter(104) # mode 5
                            print(' 05 setting mode 5 (gradient moving palette tracers)')
                        elif n == 53:
                            send_letter(106) # mode 6
                            print(' 06 setting mode 6 (gradient moving palette glows')
                        elif n == 55:
                            send_letter(107) # mode 7
                            print(' 07 setting mode 7 (gradient moving palette snek)')
                        # elif n == 56:
                        #     send_letter(108) # mode 8
                        # elif n == 57:
                        #     send_letter(59) # mode 9
                        # elif n == 58:
                        #     send_letter(39) # mode 10
                        elif n == 59:
                            send_letter(122) # mode 11
                            print(' 08 setting mode 8 (random rainbow)')

                        # actions
                        elif n in [54, 56, 58]:
                            send_letter(113)
                            print(' ** action 0 (mode action)') # mode action
                        elif n == 84:
                            send_letter(119)
                            print(' ** action 1 (burst slow)') # action 0
                        elif n == 85:
                            send_letter(101)
                            print(' ** action 2 (burst medium)') # action 1
                        elif n == 86:
                            send_letter(114)
                            print(' ** action 3 (burst fast)') # action 2
                        elif n == 87:
                            send_letter(116)
                            print(' ** action 4 (swell timely)') # action 3
                        elif n == 88:
                            send_letter(121)
                            print(' ** action 5 (burst reset)') # action 4

                        # palettes
                        elif n == 60:
                            send_letter(117)
                            print(' ** gradient 0 (ROY)') # action 5 (ROY palette)
                        elif n == 61:
                            send_letter(105)
                            print(' ** gradient 1 (YGB)') # action 6 (YGB palette)
                        elif n == 62:
                            send_letter(111)
                            print(' ** gradient 2 (GBV)') # action 7 (GBV palette)
                        elif n == 63:
                            send_letter(112)
                            print(' ** gradient 3 (BVR)') # action 8 (BVR palette)
                        elif n == 64:
                            send_letter(91)
                            print(' ** gradient 4 (ROYGBIV)') # action 9 (ROYGBIV)

                        # brightness
                        elif n == 72:
                            send_letter(32)
                            print(' %% brightness 0 (off)')
                        elif n == 73:
                            send_letter(33)
                            print(' %% brightness 1 (51)')
                        elif n == 74:
                            send_letter(34)
                            print(' %% brightness 2 (102)')
                        elif n == 75:
                            send_letter(35)
                            print(' %% brightness 3 (153)')
                        elif n == 76:
                            send_letter(36)
                            print(' %% brightness 4 (204)')
                        elif n == 77:
                            send_letter(36)
                            print(' %% brightness 5 (max)')

                        # saturation
                        elif n == 89:
                            send_letter(38)
                            print(' %% saturation 0 (off)')
                        elif n == 90:
                            send_letter(40)
                            print(' %% saturation 1 (51)')
                        elif n == 91:
                            send_letter(41)
                            print(' %% saturation 2 (102)')
                        elif n == 92:
                            send_letter(42)
                            print(' %% saturation 3 (153)')
                        elif n == 93:
                            send_letter(43)
                            print(' %% saturation 4 (204)')
                        elif n == 94:
                            send_letter(44)
                            print(' %% saturation 5 (max)')

                    else:
                        print(' ++ note on ({}, {}): {}'.format(note, v, n))
                        

                        
                elif m.isNoteOff():
                    num = int((m.getNoteNumber()-CONST)*0.5)+60
                    #send_letter(num)
                    n = m.getNoteNumber()
                    v = m.getControllerValue()
                    if not control_down:
                        print(' -- note off ({}): {}'.format(v, n))

                elif m.isController():
                    n = m.getControllerNumber()
                    v = m.getControllerValue()
                    print(' @@ control ({}): {}'.format(n, v))
                    if n == 93:
                        if v > 5:
                            control_down = True
                        elif v <= 5:
                            control_down = False

    else:
        print(' !! NO MIDI INPUT PORTS! headshotting')
        return

if __name__ == '__main__':
    main()

