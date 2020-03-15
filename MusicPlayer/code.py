from board import *
from audioio import WaveFile, Mixer, AudioOut
from digitalio import DigitalInOut
from bitbangio import I2C
from time import sleep

speaker_enable = DigitalInOut(SPEAKER_ENABLE)
speaker_enable.switch_to_output(value=False) 

# sound filename postfixes
notesE = [1, 2, 3, 4, 5, 6, 7, 8, 9]
notesA = [6, 7, 8, 9, 10, 11, 12, 13, 14]
notesD = [11, 12, 13, 14, 15, 16, 17, 18, 19]
notesG = [16, 17, 18, 19, 20, 21, 22, 23, 24]

states = [False, False, False, False];

instrument = "violin" # one of {'violin', 'guitar'}

fretChipAddresses = [0b01110000, 0b00100000, 0b01000000, 0b01100000] # E, A, D, G

mixer = Mixer(voice_count=4, sample_rate=16000, channel_count=1, bits_per_sample=16, samples_signed=True, buffer_size=(6000))
a = AudioOut(A0)

def playNote(scale, note):
    # discard not allowed note
    if note not in range(0, 9):
        print("Note must be 1-9, not playing anything")
        return;
    
    # different scales play a different set of notes on a different voice
    if(scale == 0):
        noteIndex = notesE[note]
        channel = 0;
    elif(scale == 1):
        noteIndex = notesA[note]
        channel = 1;
    elif(scale == 2):
        noteIndex = notesD[note]
        channel = 2;
    elif(scale == 3):
        noteIndex = notesG[note]
        channel = 3;
    else:
        print("Invalid scale, must be 0-3")

    formattedNote = str(noteIndex);

    note = WaveFile(open(instrument + "/Marker #" + formattedNote + ".wav", "rb"));
    
    mixer.play(note, voice=channel);

a.play(mixer)
#print("audio setup done")
note = 0
'''
while True:
    print("play " + str(note))
    playNote("G", note);
    
    note += 1;
  
    if(note >= 9):
        note = 0;
        '''

i2c = I2C(SCL, SDA, frequency=100000, timeout=20000)

def getNoteHeld(scale):
    global i2c
    
    # We need to wait until the I2C bus is available when using it from CP
    while not i2c.try_lock():
        pass
    
    writeAddress = 0;
    
    # send fret chip address
    if(scale == 0):
        writeAddress = fretChipAddresses[0];
    elif(scale == 1):
        writeAddress = fretChipAddresses[1];
    elif(scale == 2):
        writeAddress = fretChipAddresses[2];
    elif(scale == 3):
        writeAddress = fretChipAddresses[3];
    else:
        print("Invalid scale, must be 0-3")
        return
    
    #print("SCAN");
    sr = i2c.scan()
    
    if(len(sr) != 4):
        print("Not receiving 4 I2C devices! (Receiving: ", sr, ")");

    result = 0;
    
    for i in sr:
        address = i & 0b01110000

        if(address == writeAddress):
            result = i & 0b00001111
            break
    
    i2c.unlock()
    
    return result

stringInputs = [DigitalInOut(A1), DigitalInOut(A2), DigitalInOut(A3), DigitalInOut(A7)]

while True:
    for i in range(4):
        if(stringInputs[i].value != states[i]):
            states[i] = stringInputs[i].value
            
            if(states[i]):
                #print("String " + str(i) + " was just pressed");
                
                # This string was just pressed.
                # We now check what fret here is pressed
                fret = getNoteHeld(i)
                
                print(i, fret);
                
                # now we can play a note
                playNote(i, fret);