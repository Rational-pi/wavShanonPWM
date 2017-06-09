#ifndef WAVEFILE_H
#define WAVEFILE_H

#include <stdint.h>  // integers
#include <stdio.h>   // files read

#include <iostream>  // cout
#include <map>       // map
#include <math.h>    // log
#include <vector>    // vector

class WaveFile
{
public:
    WaveFile(){}
    ~WaveFile(){
        std::vector<uint8_t*>::iterator it = signales.begin();
        while(it != signales.end()){
            delete *it;
            it++;
        }
    }

    bool ReadHeader(FILE *inputFile){
        if (inputFile==NULL) return false;
        fread(&fileHeader, sizeof(FileHeader), 1, inputFile);
        blockCount=(fileHeader.dataHeader.DataSize-sizeof(FileHeader))/fileHeader.audioHeader.BytePerBloc;
        return true;
    }

    bool ReadData(FILE *inputFile){
        if (inputFile==NULL) return false;
        ReadHeader(inputFile);

        uint8_t *data=new uint8_t[blockCount*fileHeader.audioHeader.CnNumber*fileHeader.audioHeader.BitsPerSample/8];
        fread(data, 1, blockCount*fileHeader.audioHeader.CnNumber*fileHeader.audioHeader.BitsPerSample/8, inputFile);

        std::vector<uint8_t*>::iterator it = signales.begin();
        while(it != signales.end()){
            delete *it;
            it++;
        }
        signales.clear();

        for (int chanel = 0; chanel < fileHeader.audioHeader.CnNumber; ++chanel) {
            signales.push_back(new uint8_t[blockCount*fileHeader.audioHeader.BitsPerSample/8]);
        }

        for (uint32_t blockIndex = 0; blockIndex < blockCount; ++blockIndex) {
            for (uint32_t chanelIndex = 0; chanelIndex < fileHeader.audioHeader.CnNumber; ++chanelIndex) {
                ((int16_t*)signales[chanelIndex])[blockIndex]=
                        ((int16_t*)data)[blockIndex*fileHeader.audioHeader.CnNumber+chanelIndex];
            }
        }
        delete data;
        return true;
    }

    void CoutData(){
        for (uint32_t blockIndex = 0; blockIndex < blockCount; ++blockIndex) {
            std::cout << blockIndex;
            for (int chanelIndex = 0; chanelIndex < fileHeader.audioHeader.CnNumber; ++chanelIndex) {
                switch (fileHeader.audioHeader.BitsPerSample) {

                case 8:
                    std::cout << ", "<< ((int8_t*)signales[chanelIndex])[blockIndex];
                    break;

                case 16:
                    std::cout << ", "<< ((int16_t*)signales[chanelIndex])[blockIndex];
                    break;
                }
            }
            std::cout << std::endl;
        }
    }

    void AddEntropyChannel(int elements,int mod){

        uint32_t backward=elements/2;
        uint32_t forward=elements-backward;


        signales.push_back(new uint8_t[blockCount*fileHeader.audioHeader.BitsPerSample/8]);
        fileHeader.audioHeader.CnNumber++;
        for (uint32_t blockIndex = 0; blockIndex < blockCount; ++blockIndex) {
            if (blockIndex>backward && blockIndex<(blockCount-forward)){

                float entropy=shannonEntropy(&(((int16_t*)signales[fileHeader.audioHeader.CnNumber-2])[blockIndex-backward]), elements,mod);

                ((int16_t*)signales[fileHeader.audioHeader.CnNumber-1])[blockIndex]=entropy*16000;

            }
        }
        fileHeader.waveHeader.FileSize=
                sizeof(FileHeader)
                +fileHeader.audioHeader.CnNumber*blockCount*fileHeader.audioHeader.BitsPerSample/8
                -8;
        fileHeader.dataHeader.DataSize=fileHeader.waveHeader.FileSize-sizeof(FileHeader);
    }

    void ConvertHtoPWM(int chanelIndex){
        uint8_t *PWM=new uint8_t[blockCount*fileHeader.audioHeader.BitsPerSample/8];

        int f=fileHeader.audioHeader.Frequency;
        int periode=18*fileHeader.audioHeader.Frequency/1000; //pulse
        int minPulsW=1*fileHeader.audioHeader.Frequency/1000; //pulse
        int maxPulsW=2*fileHeader.audioHeader.Frequency/1000; //pulse
        int maxPulsDuration=maxPulsW-minPulsW;

        for (int periodeIndex = 0; periodeIndex < (blockCount-periode)/periode; ++periodeIndex) {
            float factor=mean(&(((int16_t*)signales[chanelIndex])[periodeIndex*periode]),maxPulsDuration)/INT16_MAX;
            uint16_t pulsW=minPulsW+maxPulsDuration*factor;
            for (int pulseSubPeriodeIndex = 0; pulseSubPeriodeIndex < periode; ++pulseSubPeriodeIndex) {
                if (pulseSubPeriodeIndex<pulsW) ((int16_t*)PWM)[periodeIndex*periode+pulseSubPeriodeIndex]=-INT16_MAX;
                else ((int16_t*)PWM)[periodeIndex*periode+pulseSubPeriodeIndex]=0;
            }
        }

        //AddEntropyChannel(1,1);
        //delete signales[chanelIndex+1];
        //signales[chanelIndex+1]=PWM;

        delete signales[chanelIndex];
        signales[chanelIndex]=PWM;
    }

    void WrightData(FILE *outputFile){
        if (outputFile==NULL) return;
        fwrite(&fileHeader, sizeof(FileHeader), 1, outputFile);

        for (uint32_t blockIndex = 0; blockIndex < blockCount; ++blockIndex) {
            for (uint32_t chanelIndex = 0; chanelIndex < fileHeader.audioHeader.CnNumber; ++chanelIndex) {
                fwrite(&((int16_t*)signales[chanelIndex])[blockIndex], sizeof(int16_t), 1, outputFile);
            }
        }
    }

private:
    struct WaveHeader{
        char        FileTypeBlocID[4]; // (4 bytes) Constante «RIFF»  (0x52,0x49,0x46,0x46)
        uint32_t    FileSize;          // (4 bytes) Taille du fichier moins 8 bytes
        char        FileFormatID[4];   // (4 bytes) Format = «WAVE»  (0x57,0x41,0x56,0x45)
    };
    struct AudioHeader{
        char        FormatBlocID[4];   // (4 bytes) Identifiant «fmt »  (0x66,0x6D, 0x74,0x20)
        uint32_t    BlocSize;          // (4 bytes) Nombre de bytes du bloc - 16  (0x10)
        uint16_t    AudioFormat;       // (2 bytes) Format du stockage dans le fichier (1: PCM, ...)
        uint16_t    CnNumber;          // (2 bytes) Nombre de canaux
        uint32_t    Frequency;         // (4 bytes) Fréquence d'échantillonnage (en hertz) [Valeurs standardisées : 11025, 22050, 44100 et éventuellement 48000 et 96000]
        uint32_t    BytePerSec;        // (4 bytes) Nombre d'octets à lire par seconde (c.-à-d., Frequence * BytePerBloc).
        uint16_t    BytePerBloc;       // (2 bytes) Nombre d'octets par bloc d'échantillonnage (c.-à-d., tous canaux confondus : NbrCanaux * BitsPerSample/8).
        uint16_t    BitsPerSample;     // (2 bytes) Nombre de bits utilisés pour le codage de chaque échantillon (8, 16, 24)
    };
    struct DataHeader {
        char        DataBlocID[4];     // (4 bytes) Constante «data»  (0x64,0x61,0x74,0x61)
        uint32_t    DataSize;          // (4 bytes) Nombre d'octets des données (c.-à-d. "Data[]", c.-à-d. taille_du_fichier - taille_de_l'entête  (qui fait 44 octets normalement).
    };
    struct FileHeader{
        WaveHeader waveHeader;
        AudioHeader audioHeader;
        DataHeader dataHeader;
    };

    FileHeader fileHeader;
    std::vector<uint8_t*> signales;
    uint32_t blockCount; //number of sample (file duration-ish)

    template <typename T> static float shannonEntropy(T data[],int elements,int mod){
        float entropy=0;
        std::map<T,int> counts;
        typename std::map<T,int>::iterator it;
        //
        for (int dataIndex = 0; dataIndex < elements; ++dataIndex) {
            counts[data[dataIndex]-(data[dataIndex]%mod)]++;
        }
        //
        it = counts.begin();
        while(it != counts.end()){
            float p_x = (float)it->second/elements;
            if (p_x>0) entropy-=p_x*log(p_x)/log(2);
            it++;
        }
        return entropy;
    }
    template <typename T> static float mean(T data[],uint32_t elements){
        int64_t total=0;
        for (uint32_t index = 0; index < elements; ++index) {
            total+=data[index];
        }
        return total/elements;
    }
};

#endif
