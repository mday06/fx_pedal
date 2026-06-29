#include <driver/adc.h>
#include <driver/dac.h>
#include <string.h>

#define AUDIO_IN ADC1_CHANNEL_6   // GPIO34
#define SAMPLE_RATE 22050
#define CHORUS_BUF_SIZE 2048
#define MAX_DELAY 20000 

uint8_t delayFlag, tremoloFlag, softDistortionFlag, hardDistortionFlag, chorusFlag, phaserFlag;
// Delay Settings
float delayFeedback = 0.5, delayMix = 0.7, delaySamples = 500;
// Distortion Settings
float distGain = 1.0;
// Tremolo Settings
float tremFreq = 5.0, tremDepth = 0.5;
// Chorus Settings
float chorusRate = 1.5, chorusDepth = 30.0, chorusMix = 0.5;
// Phaser Settings
float phaserRate = 0.5, phaserDepth = 0.7;

void handleSerial() {
    if (Serial.available() > 0) {
        String input = Serial.readStringUntil('\n');
        input.trim();
        if (input.length() == 0) return;

        char cstr[64]; 
        input.toCharArray(cstr, 64);
        char *p = strtok(cstr, " ");
        if (p == NULL) return;

        String cmd = String(p);
        cmd.toUpperCase();

        if (cmd == "DELAY") {
            p = strtok(NULL, " "); if (p) { delayFeedback = atof(p); delayFlag = (delayFeedback > 0); }
            p = strtok(NULL, " "); if (p) delayMix = atof(p);
            p = strtok(NULL, " "); if (p) delaySamples = atoi(p);
        }
        else if (cmd == "SDIST") {
            p = strtok(NULL, " "); 
            if (p) { 
                distGain = atof(p); 
                softDistortionFlag = (distGain > 1.0);
                hardDistortionFlag = 0;
            }
        }
        else if (cmd == "HDIST") {
            p = strtok(NULL, " "); 
            if (p) { 
                distGain = atof(p); 
                hardDistortionFlag = (distGain > 1.0);
                softDistortionFlag = 0;
            }
        }
        else if (cmd == "TREM") {
            p = strtok(NULL, " "); if (p) tremFreq = atof(p);
            p = strtok(NULL, " "); if (p) { tremDepth = atof(p); tremoloFlag = (tremDepth > 0); }
        }
        else if (cmd == "CHORUS") {
            p = strtok(NULL, " "); if (p) chorusRate = atof(p);
            p = strtok(NULL, " "); if (p) chorusDepth = atof(p);
            p = strtok(NULL, " "); if (p) { chorusMix = atof(p); chorusFlag = (chorusMix > 0); }
        }
        else if (cmd == "PHASER") {
            p = strtok(NULL, " "); if (p) phaserRate = atof(p);
            p = strtok(NULL, " "); if (p) { phaserDepth = atof(p); phaserFlag = (phaserDepth > 0); }
        }
        else if (cmd == "OFF") {
            delayFlag = tremoloFlag = softDistortionFlag = hardDistortionFlag = chorusFlag = phaserFlag = 0;
        }
    }
}

void handleFlags(int &input) {
    if(delayFlag) input = applyDelay(input, delayFeedback, delayMix, delaySamples);
    if(tremoloFlag) input = applyTremolo(input, tremFreq, tremDepth);
    if(softDistortionFlag) input = softDistortion(input, distGain);
    if(hardDistortionFlag) input = hardDistortion(input, distGain);
    if(chorusFlag) input = applyChorus(input, chorusRate, chorusDepth, chorusMix);
    if(phaserFlag) input = applyPhaser(input, phaserRate, phaserDepth);
}

void setup() {
    Serial.begin(115200);
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(AUDIO_IN, ADC_ATTEN_DB_0);

    dac_output_enable(DAC_CHANNEL_1); // GPIO25
}

int hardDistortion(int input, float gain) {
    long boosted = (long)(input * gain);
    
    if (boosted > 2047) boosted = 2047;
    if (boosted < -2047) boosted = -2047;
    
    return (int)boosted;
}

int softDistortion(int input, float gain) {
    float x = (float)input / 2048.0; 
    x *= gain;

    float processed;
    if (x > 1.0) processed = 0.66;
    else if (x < -1.0) processed = -0.66;
    else processed = x - (x * x * x / 3.0);

    return (int)(processed * 2048.0);
}

int delayBuffer[MAX_DELAY];
int writePtr = 0;

int applyDelay(int input, float feedback, float mix, int delaySamples) {

    int readPtr = writePtr - delaySamples;
    if (readPtr < 0) readPtr += MAX_DELAY; 
    
    int delayedSample = delayBuffer[readPtr];

    delayBuffer[writePtr] = input + (int)(delayedSample * feedback);
    writePtr++;
    if (writePtr >= MAX_DELAY) writePtr = 0;

    return (int)(input * (1.0 - mix) + delayedSample * mix);
}

float lfoPhase = 0;

int applyTremolo(int input, float freq, float depth) {
    float lfo = sin(lfoPhase);
    
    float volume = 1.0 - (depth * 0.5 * (lfo + 1.0));

    lfoPhase += (2.0 * PI * freq) / SAMPLE_RATE;
    if (lfoPhase >= 2.0 * PI) lfoPhase -= 2.0 * PI;

    return (int)(input * volume);
}

int chorusBuffer[CHORUS_BUF_SIZE];
int chorusPtr = 0;
float chorusPhase = 0;

int applyChorus(int input, float rate, float depth, float mix) {
    chorusBuffer[chorusPtr] = input;

    float offset = (CHORUS_BUF_SIZE / 4) + (sin(chorusPhase) * depth);
    
    float readPtr = (float)chorusPtr - offset;
    while (readPtr < 0) readPtr += CHORUS_BUF_SIZE;
    
    int delayedSample = chorusBuffer[(int)readPtr % CHORUS_BUF_SIZE];

    chorusPhase += (2.0 * PI * rate) / SAMPLE_RATE;
    if (chorusPhase >= 2.0 * PI) chorusPhase -= 2.0 * PI;
    
    chorusPtr = (chorusPtr + 1) % CHORUS_BUF_SIZE;

    return (int)(input * (1.0 - mix) + delayedSample * mix);
}

float phaserLFO = 0;
float oldX[4] = {0,0,0,0};
float oldY[4] = {0,0,0,0};

int applyPhaser(int input, float rate, float depth) {
    float x = (float)input / 2048.0;

    float lfo = (sin(phaserLFO) + 1.0) / 2.0;
    float a = 0.3 + (lfo * 0.6);

    float y = x;
    for(int i = 0; i < 4; i++) {
        float currentY = a * y + oldX[i] - a * oldY[i];
        oldX[i] = y;
        oldY[i] = currentY;
        y = currentY;
    }

    phaserLFO += (2.0 * PI * rate) / SAMPLE_RATE;
    if (phaserLFO >= 2.0 * PI) phaserLFO -= 2.0 * PI;

    float finalOut = (x + y * depth) * 0.5;

    return (int)(finalOut * 2048.0);
}

void loop() {
    int sample = adc1_get_raw(AUDIO_IN);
    int audio = sample - 2048;

    handleSerial();
    handleFlags(audio);
  
    if (audio > 2047) audio = 2047;
    if (audio < -2047) audio = -2047;

    int output = (audio >> 4) + 128;

    if (output < 0) output = 0;
    if (output > 255) output = 255;
    dac_output_voltage(DAC_CHANNEL_1, output);
}