import serial
import time
import rtmidi



def reconnect_loop(port=0):
    asd = None
    print('>> attempting connection to board on port {}...'.format(port))
    while asd is None:
        try:
          asd = serial.Serial(port='/dev/tty.usbmodem1421'.format(port),baudrate=9600)
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
            asd.write('{}{}'.format(letter,letter).encode())
            print('++ wrote {} to board'.format(letter))
            return
        except Exception as ex:
            print('!! send exception: {}'.format(str(ex)))
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

def main():
    # open midi device for listening
    midi_in = rtmidi.RtMidiIn()

    ports = range(midi_in.getPortCount())
    if ports:
        print('available ports:')
        for i in ports:
            print(midi_in.getPortName(i))
            midi_in.openPort(i)

        # create midi device for output

        # poll for midi events
        # note on/off: calculate led index and send to board
        while True:
            m = midi_in.getMessage(250) # some timeout in ms
            if m:
                print_message(m)
                send_letter('r')
    else:
        print('NO MIDI INPUT PORTS! headshotting')
        return



if __name__ == '__main__':
    main()

