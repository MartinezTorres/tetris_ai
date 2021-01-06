#include <array>
#include <vector>
#include <set>
#include <unordered_set>
#include <iostream>
#include <algorithm>
#include <chrono>
#include <thread>

using namespace std::chrono_literals;

struct Board : public std::array<uint32_t, 21> {
	
	int score=0, pieces=0, lines=0, lastMoveLines=0;
};

enum Piece { I,J,L,O,S,T,Z };

static const std::vector<std::vector<std::vector<int>>> pieces = { 
	{
		{ 0b0000, 
		  0b1111,
		  0b0000,
		  0b0000},
		{ 0b0010, 
		  0b0010,
		  0b0010,
		  0b0010},
	}, 
	{
		{ 0b100, 
		  0b111,
		  0b000},
		{ 0b011, 
		  0b010,
		  0b010},
		{ 0b000, 
		  0b111,
		  0b001},
		{ 0b010, 
		  0b010,
		  0b110},
	}, 
	{
		{ 0b001, 
		  0b111,
		  0b000},
		{ 0b010, 
		  0b010,
		  0b011},
		{ 0b000, 
		  0b111,
		  0b100},
		{ 0b110, 
		  0b010,
		  0b010},
	}, 
	{
		{ 0b11, 
		  0b11},
	}, 
	{
		{ 0b000, 
		  0b011,
		  0b110},
		{ 0b010, 
		  0b011,
		  0b001},
	}, 
	{
		{ 0b010, 
		  0b111,
		  0b000},
		{ 0b010, 
		  0b011,
		  0b010},
		{ 0b000, 
		  0b111,
		  0b010},
		{ 0b010, 
		  0b110,
		  0b010},
	}, 
	{
		{ 0b000, 
		  0b110,
		  0b011},
		{ 0b001, 
		  0b011,
		  0b010},
	}
};	



void drawBoard(Board board);

Board initBoard();

std::vector<Board> findAllPossibleDestinations(Piece piece, const Board &board);

double scoreBoard(const Board &board);

Board startGame();
