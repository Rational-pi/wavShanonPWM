#include "WaveFile.h"

int main(int argc, char* argv[]){
    ////file opening
    const char* intputFilePath;
    const char* outputFilePath;
    if (argc <= 2){
        std::cout << "Specify a wave file and a outpute file!" << std::endl;
        return 1;
    }else{
        intputFilePath = argv[1];
        outputFilePath = argv[2];
    }

    FILE *inputWavFile = fopen(intputFilePath, "r");
    FILE *outputFile = fopen(outputFilePath, "w");
    WaveFile file;


    file.ReadData(inputWavFile);
    file.AddEntropyChannel(500,4000);
    file.ConvertHtoPWM(1);
    file.WrightData(outputFile);


    fclose(inputWavFile);
    fclose(outputFile);






}
