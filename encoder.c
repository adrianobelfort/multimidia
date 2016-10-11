#include <stdio.h>
#include <stdlib.h>

#define BITS_PER_CHAR 8

// *** References ***
// http://soundfile.sapp.org/doc/WaveFormat/
// http://stackoverflow.com/questions/13660777/c-reading-the-data-part-of-a-wav-file

// Struc que representa o header dos arquivos .wav
typedef struct  WAV_HEADER{
    char                RIFF[4];        // RIFF Header      Magic header
    unsigned int        ChunkSize;      // RIFF Chunk Size
    char                WAVE[4];        // WAVE Header
    char                fmt[4];         // FMT header
    unsigned int        Subchunk1Size;  // Size of the fmt chunk
    unsigned short      AudioFormat;    // Audio format 1=PCM,6=mulaw,7=alaw, 257=IBM Mu-Law, 258=IBM A-Law, 259=ADPCM
    unsigned short      NumOfChan;      // Number of channels 1=Mono 2=Sterio
    unsigned int        SamplesPerSec;  // Sampling Frequency in Hz
    unsigned int        bytesPerSec;    // bytes per second
    unsigned short      blockAlign;     // 2=16-bit mono, 4=16-bit stereo
    unsigned short      bitsPerSample;  // Number of bits per sample
    char                Subchunk2ID[4]; // "data"  string
    unsigned int        Subchunk2Size;  // Sampled data length
    
} wav_hdr;

// Le o header do arquivo .wav e encontra o tamanho do arquivo
wav_hdr *readHeader(FILE *input, int *fileSize) {
    wav_hdr* header = (wav_hdr *) malloc(sizeof(wav_hdr));
    
    fseek(input, 0, SEEK_END);
    
    *fileSize = (int) ftell(input);
    
    fseek(input, 0, SEEK_SET);
    
    fread(header, sizeof(wav_hdr), 1, input);
    
    printf("File size: %d bytes\n", *fileSize);
    printf("ChunkID: %s\n", header->RIFF);
    printf("ChunkSize: %u\n", header->ChunkSize);
    printf("AudioFormat: %d\n", header->AudioFormat);
    printf("NumOfChan: %d\n", header->NumOfChan);
    printf("SamplesPerSec: %u\n", header->SamplesPerSec);
    printf("bytesPerSec: %u\n", header->bytesPerSec);
    printf("bitsPerSample: %hu\n", header->bitsPerSample);
    printf("SubChunk2Size (data): %u\n", header->Subchunk2Size);
    
    return header;
}

char *readData(FILE *input, wav_hdr *header) {
    
    // Aloca um vetor de chars, sendo que cada um representa um bit.
    // Por isso o tamanho é Subchunk2Size * BITS_PER_CHAR, já que
    // um char tem um byte.
    char *dataBits = (char *) malloc(header->Subchunk2Size * BITS_PER_CHAR);
    
    // Vetor dos bytes lidos originalmente do arquivo
    char *originalData = (char *) malloc(header->Subchunk2Size);
    
    // Prepara o offset para leitura dos dados do arquivo,
    // pulando o header
    fseek(input, sizeof(wav_hdr), SEEK_SET);
    
    
    fread(originalData, header->Subchunk2Size, 1, input);
    
    // Mascara utilizada para isolar bits
    char mask = 0x1;
    int i;
    int currentBitPosition; // Atual posicao do bit dentro de um byte
    int dataBitsPosition = 0; // Posicao no vetor dataBits
    int shift; // Quantidade a ser deslocada na operacao bitshift
    
    // Para todos os bytes lidos do arquivo
    for(i = 0; i < header->Subchunk2Size; i++) {
        // Seleciona o byte atual
        char currValue = originalData[i];
        // Inicializa a posicao de bit no byte atual
        currentBitPosition = 0;
        // Para todos os bits no byte
        while(currentBitPosition < BITS_PER_CHAR) {
            // Calcula o shift que deve ser feito
            shift = BITS_PER_CHAR - 1 - currentBitPosition++;
            // Isola o bit do byte e atribui a posicao do vetor dataBits
            dataBits[dataBitsPosition++] = (currValue & (mask << shift)) >> shift;
        }
    }
    
    return dataBits;
}

int main(int argc, char **argv) {
    FILE *f; // descritor do arquivo de entrada a ser processado
    int fileSize; // tamanho total do arquivo
    
    if(argc <= 1) {
        printf("Usage: ./wave_reader file.wav\n");
        return EXIT_FAILURE;
    }
    
    f = fopen(argv[1], "r");
    if(f == NULL) {
        printf("Could not open wave file %s\n", argv[1]);
        return EXIT_FAILURE;
    }
    
    wav_hdr *header = readHeader(f, &fileSize);
    
    char *dataBits = readData(f, header);

    free(dataBits);
    free(header);
    fclose(f);
    
    return EXIT_SUCCESS;
}
