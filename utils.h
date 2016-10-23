#ifndef __UTILS_H__
#define __UTILS_H__

#define DEBUG_FLAG 0

#define BITS_PER_CHAR 8
#define RUNLENGTH_MASK 0x1
#define HUFFMAN_MASK 0x2
#define DIFFERENCE_MASK 0x4

// *** References ***
// http://soundfile.sapp.org/doc/WaveFormat/
// http://stackoverflow.com/questions/13660777/c-reading-the-data-part-of-a-wav-file

// Struct que representa o header dos arquivos .wav
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

/* Headers abaixo ja estao alinhados de 8 em 8 bytes */

/* Header geral para o arquivo comprimido */
typedef struct encodeHeader {
    /* 00000DHR (D - Diferença; H - Huffman; R - Runlength) */
    unsigned int encodeType;
    unsigned long long totalLength;
    unsigned int originalFileSize;
} enc_hdr;

/******* INCLUIR HEADER DE DIFERENCA *******/
typedef struct differenceHeader {
    // TO DO
} dif_hdr;

/* Header presente no caso de haver codificacao Runlength */
typedef struct runlengthHeader {
    unsigned int runlengthNumBits;
    unsigned int runlengthPadding;
} run_hdr;

/* Header presente no caso de haver codificacao Huffman */
typedef struct huffmanHeader {
    unsigned int huffmanFrequenciesCount;
    unsigned int huffmanMaxValue;
} huf_hdr;

#endif
