#ifndef WAVEFILE_H
#define WAVEFILE_H

#include <stdint.h>  // integers
#include <stdio.h>   // files read

#include <iostream>  // cout
#include <map>       // map
#include <math.h>    // log
#include <vector>    // vector

class WaveFile{
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
    std::vector<uint8_t*> Chanels;
    uint32_t blockCount; //number of sample (file duration-ish)

public:
    WaveFile(){}
    ~WaveFile();

    bool ReadHeader(FILE *inputFile);
    bool ReadData(FILE *inputFile);
    void CoutData();
    void AddEntropyChannel(int elements,int mod);
    void ConvertHtoPWM(int chanelIndex);
    void WrightData(FILE *outputFile);

};

#endif
