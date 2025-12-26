#include <iostream>
#include <string>
#include <vector>
#include <cstdint>
#include <cmath>
#include <fstream>
#pragma pack (push, 1)// Ensure no padding is added to the structures


struct WavHeader {//contains various important details of the file
	char riff[4];                // "RIFF"
	uint32_t chunkSize;         // Size of the entire file minus 8 bytes
	char wave[4];               // "WAVE"
	char fmt[4];                // "fmt "
	uint32_t subchunk1Size;     // Size of the fmt chunk (16 for PCM)
	uint16_t audioFormat;       // Audio format (1 for PCM)
	uint16_t numChannels;       // Number of channels
	uint32_t sampleRate;        // Sample rate
	uint32_t byteRate;          // Byte rate
	uint16_t blockAlign;        // Block align
	uint16_t bitsPerSample;     // Bits per sample
	char data[4];               // "data"
	uint32_t subchunk2Size;     // Size of the data section
};

// Update the increase_volume function to accept audioData as a parameter
void increase_volume(float factor, std::vector<int16_t>& audioData) {
	for (int i = 0; i < audioData.size(); i++)
	{
		audioData[i] = audioData[i] * factor;

	}
	std::cout << "Volume increased by " << factor << std::endl;
}

// Robot effect function
void robot_effect(int sampleRate, std::vector<int16_t>& audioData, float frequency) {
	float PI = 3.14159265f;
	for (int i = 0; i < audioData.size(); i++) {
		float modulator = sinf(2.0f * PI * frequency * (i / static_cast<float>(sampleRate)));
		audioData[i] = static_cast<int16_t>(audioData[i] * modulator);
	}
};

void echo(int sampleRate, std::vector<int16_t>& audioData, float delaySeconds) {
	float delayInSamples = delaySeconds * sampleRate;
	std::vector<int16_t> echoData(audioData.size(), 0);
	for (int i = 0; i < audioData.size(); i++) {
		echoData[i] = audioData[i];
		if (i >= delayInSamples) {
			echoData[i] += static_cast<int16_t>(0.6f * audioData[i - static_cast<int>(delayInSamples)]);
		}
	}
	audioData.swap(echoData);
};

// Function to write modified audio data to a new WAV file
void write_wav(const std::string& outputPath, const WavHeader& header, const std::vector<int16_t>& audioData) {
	std::ofstream outFile(outputPath, std::ios::binary);
	outFile.write(reinterpret_cast<const char*>(&header), sizeof(WavHeader));
	outFile.write(reinterpret_cast<const char*>(audioData.data()), header.subchunk2Size);
	std::cout << "Wrote file: " << outputPath << std::endl;
}

void choose_effect(int sampleRate, std::vector<int16_t>& audioData) {
	std::cout << "Choose an effect to apply:\n";
	std::cout << "1. Increase Volume\n";
	std::cout << "2. Robot Effect\n";
	std::cout << "3. Echo Effect\n";
	std::cout << "Enter the number of your choice: ";
	int choice;
	std::cin >> choice;
	switch (choice) {
	case 1: {
		float factor;
		std::cout << "Enter volume factor (e.g., 1.5 for 50% louder): ";
		std::cin >> factor;
		increase_volume(factor, audioData);
		break;
	}
	case 2: {
		float frequency;
		std::cout << "Enter modulation frequency in Hz (e.g., 30): ";
		std::cin >> frequency;
		robot_effect(sampleRate, audioData, frequency);
		break;
	}
	case 3: {
		float delaySeconds;
		std::cout << "Enter echo delay in seconds (e.g., 0.5): ";
		std::cin >> delaySeconds;
		echo(sampleRate, audioData, delaySeconds);
		break;
	}
	default:
		std::cout << "Invalid choice. No effect will be applied.\n";
		break;
	}


};

int main() {
	std::ifstream file("voice-sample.wav", std::ios::binary);//opening .wav file in binary format

	if (!file) {
		std::cerr <<"Could not open the file!"<<std::endl;
		return 1;
	}
	WavHeader header;//making instance of WavHeader struct (object creation)
	file.read(reinterpret_cast<char*>(&header), sizeof(WavHeader));//reading the header from the file into the struct

	if (std::string(header.riff,4)!="RIFF" || std::string(header.wave,4)!="WAVE") { //making sure that it is a .wav file
		std::cerr << "Error: Not a valid WAV file\n";
		return 1;
	}
	std::cout << "Sucessfully read header (WAV file detected!)" << std::endl;
	std::cout << "Sample Rate: " << header.sampleRate << " Hz"<<std::endl;
	std::cout << "Channels: " << header.numChannels << " (1=Mono,2=Stereo fyi)" << std::endl;
	std::cout << "Bit Depth: " << header.bitsPerSample << "-bit" << std::endl;
	
	int numSamples = header.subchunk2Size / sizeof(int16_t);
	std::vector<int16_t> audioData;//storing the actual audio data in a vector
	audioData.resize(numSamples);
	file.read(reinterpret_cast<char*>(audioData.data()), header.subchunk2Size);//reading the audio data from the file into the vector

	choose_effect(header.sampleRate, audioData);
	write_wav("voice-sample-modified.wav", header, audioData);


	std::cin.get();
	return 0;
}