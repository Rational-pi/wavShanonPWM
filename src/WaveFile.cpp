#include "WaveFile.h"


WaveFile::~WaveFile(){
    for(std::vector<uint8_t*>::iterator it = Chanels.begin();it!=Chanels.end();++it)
        delete *it;
}

bool WaveFile::ReadHeader(FILE *inputFile){
    if (inputFile==NULL) return false;
    fread(&fileHeader, sizeof(FileHeader), 1, inputFile);
    blockCount=(fileHeader.dataHeader.DataSize-sizeof(FileHeader))/fileHeader.audioHeader.BytePerBloc;
    return true;
}

bool WaveFile::ReadData(FILE *inputFile){
    if (inputFile==NULL) return false;
    ReadHeader(inputFile);

    uint8_t *data=new uint8_t[blockCount*fileHeader.audioHeader.CnNumber*fileHeader.audioHeader.BitsPerSample/8];
    fread(data, 1, blockCount*fileHeader.audioHeader.CnNumber*fileHeader.audioHeader.BitsPerSample/8, inputFile);

    std::vector<uint8_t*>::iterator it = Chanels.begin();
    for(;it != Chanels.end();it++)delete *it;
    Chanels.clear();

    for (int chanel = 0; chanel < fileHeader.audioHeader.CnNumber; ++chanel) {
        Chanels.push_back(new uint8_t[blockCount*fileHeader.audioHeader.BitsPerSample/8]);
    }

    for (uint32_t blockIndex = 0; blockIndex < blockCount; ++blockIndex) {
        for (uint32_t chanelIndex = 0; chanelIndex < fileHeader.audioHeader.CnNumber; ++chanelIndex) {
            ((int16_t*)Chanels[chanelIndex])[blockIndex]=
                    ((int16_t*)data)[blockIndex*fileHeader.audioHeader.CnNumber+chanelIndex];
        }
    }
    delete data;
    return true;
}

void WaveFile::CoutData(){
    for (uint32_t blockIndex = 0; blockIndex < blockCount; ++blockIndex) {
        std::cout << blockIndex;
        for (int chanelIndex = 0; chanelIndex < fileHeader.audioHeader.CnNumber; ++chanelIndex) {
            switch (fileHeader.audioHeader.BitsPerSample) {

            case 8:
                std::cout << ", "<< ((int8_t*)Chanels[chanelIndex])[blockIndex];
                break;

            case 16:
                std::cout << ", "<< ((int16_t*)Chanels[chanelIndex])[blockIndex];
                break;
            }
        }
        std::cout << std::endl;
    }
}

template <typename T> float shannonEntropy(T data[],int elements,int mod){
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
void WaveFile::AddEntropyChannel(int elements, int mod){

    uint32_t backward=elements/2;
    uint32_t forward=elements-backward;


    Chanels.push_back(new uint8_t[blockCount*fileHeader.audioHeader.BitsPerSample/8]);
    fileHeader.audioHeader.CnNumber++;
    for (uint32_t blockIndex = 0; blockIndex < blockCount; ++blockIndex) {
        if (blockIndex>backward && blockIndex<(blockCount-forward)){

            float entropy=shannonEntropy(&(((int16_t*)Chanels[fileHeader.audioHeader.CnNumber-2])[blockIndex-backward]), elements,mod);

            ((int16_t*)Chanels[fileHeader.audioHeader.CnNumber-1])[blockIndex]=entropy*16000;

        }
    }
    fileHeader.waveHeader.FileSize=
            sizeof(FileHeader)
            +fileHeader.audioHeader.CnNumber*blockCount*fileHeader.audioHeader.BitsPerSample/8
            -8;
    fileHeader.dataHeader.DataSize=fileHeader.waveHeader.FileSize-sizeof(FileHeader);
}

template <typename T> float mean(T data[],uint32_t elements){
    int64_t total=0;
    for (uint32_t index = 0; index < elements; ++index) {
        total+=data[index];
    }
    return total/elements;
}
void WaveFile::ConvertHtoPWM(int chanelIndex){
    uint8_t *PWM=new uint8_t[blockCount*fileHeader.audioHeader.BitsPerSample/8];

    int f=fileHeader.audioHeader.Frequency;
    int periode=(15.0f*fileHeader.audioHeader.Frequency)/1000; //pulse
    int minPulsW=(1.0f*fileHeader.audioHeader.Frequency)/1000; //pulse
    int maxPulsW=(2.0f*fileHeader.audioHeader.Frequency)/1000; //pulse
    int maxPulsDuration=maxPulsW-minPulsW;

    for (int periodeIndex = 0; periodeIndex < (blockCount-periode)/periode; ++periodeIndex) {
        float factor=mean(&(((int16_t*)Chanels[chanelIndex])[periodeIndex*periode]),maxPulsDuration)/15000;
        factor = factor>1?1:factor;
        uint16_t pulsW=minPulsW+maxPulsDuration*factor;
        for (int pulseSubPeriodeIndex = 0; pulseSubPeriodeIndex < periode; ++pulseSubPeriodeIndex) {
            if (pulseSubPeriodeIndex<pulsW) ((int16_t*)PWM)[periodeIndex*periode+pulseSubPeriodeIndex]=INT16_MAX/4;
            else ((int16_t*)PWM)[periodeIndex*periode+pulseSubPeriodeIndex]=0;
        }
    }

    //AddEntropyChannel(1,1);
    //delete signales[chanelIndex+1];
    //signales[chanelIndex+1]=PWM;

    delete Chanels[chanelIndex];
    Chanels[chanelIndex]=PWM;
}

void WaveFile::WrightData(FILE *outputFile){
    if (outputFile==NULL) return;
    fwrite(&fileHeader, sizeof(FileHeader), 1, outputFile);

    for (uint32_t blockIndex = 0; blockIndex < blockCount; ++blockIndex) {
        for (uint32_t chanelIndex = 0; chanelIndex < fileHeader.audioHeader.CnNumber; ++chanelIndex) {
            fwrite(&((int16_t*)Chanels[chanelIndex])[blockIndex], sizeof(int16_t), 1, outputFile);
        }
    }
}


