#include <iostream>
#include <fstream>
#include <random>
#include <cassert>
#include <chrono>

constexpr uint64_t BOARD_SIZE_X = 24;
constexpr uint64_t BOARD_SIZE_Y = 20;
constexpr int NUM_FLAGS = 99;
constexpr uint64_t NUM_GAMES = 2000000;
constexpr char MINE = 1;
constexpr char NO_MINE = 0;

struct xorshift128p_state {

	uint64_t x[2];
};

xorshift128p_state rngState;

char access2DArray(char* arr, int x, int y);
void set2DArray(char* arr, int x, int y, char value);
void initializeRNG();
uint64_t xorshift128p();

int main() {
	
	uint64_t numTilesFound[9]{ 0 };
	auto tilesDistribution = new uint64_t[BOARD_SIZE_X * BOARD_SIZE_Y * 9];
	memset(tilesDistribution, 0, sizeof(uint64_t) * BOARD_SIZE_X * BOARD_SIZE_Y * 9);

	uint64_t numTilesSearched = 0;
	char* currentBoard = new char[(BOARD_SIZE_X + 2) * (BOARD_SIZE_Y + 2)];
	uint64_t tileCounts[9]{ 0 };
	initializeRNG();

	assert(BOARD_SIZE_X > 0 && BOARD_SIZE_Y > 0);
	assert(BOARD_SIZE_X * BOARD_SIZE_Y > 0);
	assert(BOARD_SIZE_X * BOARD_SIZE_Y >= NUM_FLAGS);

	auto begin = std::chrono::steady_clock::now();

	for (uint64_t i = 0; i < NUM_GAMES; i++) {
		
		memset(currentBoard, 0, static_cast<size_t>((BOARD_SIZE_X + 2) * (BOARD_SIZE_Y + 2)));
		memset(tileCounts, 0, sizeof(tileCounts));

		{
			int temp1 = 0, temp2 = 0;
			for (int j = 0; j < NUM_FLAGS; j++) {

				set2DArray(currentBoard, temp1 + 1, temp2 + 1, MINE);
				temp1++;
				if (temp1 == BOARD_SIZE_X - 1) { temp2++; temp1 = 0; }
			}
		}

		for (int j = BOARD_SIZE_X * BOARD_SIZE_Y - 1; j > 0; j--) {

			xorshift128p();
			int k = ((unsigned int)rngState.x[0]) % (j + 1);
			char temp = access2DArray(currentBoard, j % BOARD_SIZE_X + 1, j / BOARD_SIZE_X + 1);
			set2DArray(currentBoard, j % BOARD_SIZE_X + 1, j / BOARD_SIZE_X + 1, access2DArray(currentBoard, k % BOARD_SIZE_X + 1, k / BOARD_SIZE_X + 1));
			set2DArray(currentBoard, k % BOARD_SIZE_X + 1, k / BOARD_SIZE_X + 1, temp);
		}

		for (int x = 1; x < BOARD_SIZE_X + 1; x++) {

			for (int y = 1; y < BOARD_SIZE_Y + 1; y++) {

				if (access2DArray(currentBoard, x, y) == MINE) continue;
				int surroundingMineCount = 0;

				surroundingMineCount += access2DArray(currentBoard, x - 1, y - 1);
				surroundingMineCount += access2DArray(currentBoard, x - 1, y);
				surroundingMineCount += access2DArray(currentBoard, x - 1, y + 1);
				surroundingMineCount += access2DArray(currentBoard, x, y - 1);
				surroundingMineCount += access2DArray(currentBoard, x, y + 1);
				surroundingMineCount += access2DArray(currentBoard, x + 1, y - 1);
				surroundingMineCount += access2DArray(currentBoard, x + 1, y);
				surroundingMineCount += access2DArray(currentBoard, x + 1, y + 1);

				tileCounts[surroundingMineCount]++;
				numTilesSearched++;
				numTilesFound[surroundingMineCount]++;
			}
		}

		tilesDistribution[tileCounts[0]]++;
		tilesDistribution[BOARD_SIZE_X * BOARD_SIZE_Y + tileCounts[1]]++;
		tilesDistribution[BOARD_SIZE_X * BOARD_SIZE_Y * 2 + tileCounts[2]]++;
		tilesDistribution[BOARD_SIZE_X * BOARD_SIZE_Y * 3 + tileCounts[3]]++;
		tilesDistribution[BOARD_SIZE_X * BOARD_SIZE_Y * 4 + tileCounts[4]]++;
		tilesDistribution[BOARD_SIZE_X * BOARD_SIZE_Y * 5 + tileCounts[5]]++;
		tilesDistribution[BOARD_SIZE_X * BOARD_SIZE_Y * 6 + tileCounts[6]]++;
		tilesDistribution[BOARD_SIZE_X * BOARD_SIZE_Y * 7 + tileCounts[7]]++;
		tilesDistribution[BOARD_SIZE_X * BOARD_SIZE_Y * 8 + tileCounts[8]]++;

		if ((i - 1) % 100000 == 0 && (i - 1) != 0) std::cout << i - 1 << " games completed" << std::endl;
	}

	delete[] currentBoard;

	auto end = std::chrono::steady_clock::now();
	float timeSpent = std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() / 1000000.0f;

	std::wcout << L"Finished in " << timeSpent << L" seconds with an average of " << timeSpent / NUM_GAMES * 1000000 << L" µs per game\n";
	for (int i = 0; i < sizeof(numTilesFound) / sizeof(uint64_t); i++) {

		std::cout << i << ": " << numTilesFound[i] << " / " << numTilesSearched << " or " << numTilesFound[i] / (float)numTilesSearched * 100 << "%, on average "
			<< numTilesFound[i] / (float)NUM_GAMES << " per game\n";
	}

	std::cout.flush();

	std::ofstream distributionDataFile;
	distributionDataFile.open("distributionData.csv");

	for (int i = 0; i < BOARD_SIZE_X * BOARD_SIZE_Y; i++) distributionDataFile << i << ',';

	distributionDataFile << '\n';
	for (int i = 0; i < 9; i++) {
		
		for (int j = 0; j < BOARD_SIZE_X * BOARD_SIZE_Y; j++) {

			distributionDataFile << tilesDistribution[BOARD_SIZE_X * BOARD_SIZE_Y * i + j] << ',';
		}

		distributionDataFile << '\n';
	}

	delete[] tilesDistribution;
	distributionDataFile.flush();
	distributionDataFile.close();

	char a = 0;
	std::cin >> a;
}

char access2DArray(char* arr, int x, int y) {
		
	return arr[x + y * (BOARD_SIZE_X + 2)];
}

void set2DArray(char* arr, int x, int y, char value) {

	arr[x + y * (BOARD_SIZE_X + 2)] = value;
}

//this up to xorshift128p are all borrowed from from https://en.wikipedia.org/wiki/Xorshift
uint64_t splitmix64(uint64_t seed) {

	uint64_t result = (seed += 0x9E3779B97f4A7C15);
	result = (result ^ (result >> 30)) * 0xBF58476D1CE4E5B9;
	result = (result ^ (result >> 27)) * 0x94D049BB133111EB;
	return result ^ (result >> 31);
}

void initializeRNG() {

	std::random_device rand;
	std::mt19937_64 gen(rand());
	std::uniform_int_distribution<uint64_t> distribution(0, UINT64_MAX);
	uint64_t seed = distribution(gen);

	uint64_t tmp = splitmix64(seed);
	rngState.x[0] = tmp;

	tmp = splitmix64(seed);
	rngState.x[1] = tmp;
}

uint64_t xorshift128p() {

	uint64_t t = rngState.x[0];
	uint64_t const s = rngState.x[1];
	rngState.x[0] = s;
	t ^= t << 23;
	t ^= t >> 18;
	t ^= s ^ (s >> 5);
	rngState.x[1] = t;
	return t + s;
}
