#include "tetris.h"

void drawBoard(Board board) {
	
	for (int i=0; i<21; i++) {
		for (int j=0; j<12; j++) { 
			if (board[i]%2) putchar('O'); else putchar(' ');
			board[i]>>=1;
		}
		putchar('\n');
	}
	std::cout << "Score: " << board.score << std::endl;
	std::cout << "Pieces: " << board.pieces << std::endl;
	std::cout << "Lines: " << board.lines << std::endl;
	std::cout << "Avg Score / line: " << board.score/(board.lines+1e-10) << std::endl;
}

Board initBoard() {
	
	Board board;
	
	board.fill(0);
	
	for (int i=0; i<20; i++) 
		board[i] = 0b100000000001;
		
	board[20] = 0b111111111111;
	
	return board;
}


std::vector<Board> findAllPossibleDestinations(Piece pieceIndex, const Board &board) {
	
	auto &piece = pieces[pieceIndex];

	std::vector<Board> ret;

	std::unordered_set<int> visitedPositions;
	std::vector<int> toVisit;
	
	auto posToHash = [](int x, int y, int r){return r+4*y+4*32*x;};
	auto hashToPos = [](int hash, int &x, int &y, int &r){
		
		r = hash&3; hash>>=2;
		y = hash&31; hash>>=5;
		x = hash;
	};
	
	toVisit.push_back( posToHash(1+5-int(piece[0].size()/2), 0, 0) );
	visitedPositions.insert(toVisit.back());
	
	while (toVisit.empty() == false) {
		
		int x, y, r;
		hashToPos(toVisit.back(), x, y, r);
		toVisit.pop_back();

		bool collision = false;
		for (int i=0; not collision and i<piece[r].size(); i++) 
			if ( (piece[r][i]<<x) & board[i+y] )
				collision = true;
		
				
		if (collision == true) continue;
		
		if (r<piece.size()-1) {
			int testPosition = posToHash(x, y, r+1);
			if (visitedPositions.insert(testPosition).second) {
				toVisit.push_back(testPosition);
			}
		}

		if (r>1) {
			int testPosition = posToHash(x, y, r-1);
			if (visitedPositions.insert(testPosition).second) {
				toVisit.push_back(testPosition);
			}
		}

		{
			int testPosition = posToHash(x+1, y, r);
			if (visitedPositions.insert(testPosition).second) {
				toVisit.push_back(testPosition);
			}
		}

		if (x) {
			int testPosition = posToHash(x-1, y, r);
			if (visitedPositions.insert(testPosition).second) {
				toVisit.push_back(testPosition);
			}
		}

		{
			int testPosition = posToHash(x, y+1, r);
			if (visitedPositions.insert(testPosition).second) {
				toVisit.push_back(testPosition);
			}
		}

		bool collisionBottom = false;
		for (int i=0; not collisionBottom and i<piece[r].size(); i++)
			if ( (piece[r][i]<<x) & board[i+y+1] )
				collisionBottom = true;
		
		if (collisionBottom) {
			
			Board newBoard = board;
			for (int i=0; i<piece[r].size(); i++)
				newBoard[i+y] = newBoard[i+y] | (piece[r][i]<<x);
				
				
			int countLines = 0;
			for (int i=0; i<20; i++) {
				if (newBoard[i]==0b111111111111) {
					for (int j=i; j>0; j--) {
						newBoard[j] = newBoard[j-1];
					}
					newBoard[0] = 0b100000000001;
					countLines++;
				}
			}
			if (countLines==1) newBoard.score += 40;
			if (countLines==2) newBoard.score += 100;
			if (countLines==3) newBoard.score += 300;
			if (countLines==4) newBoard.score += 1200;
			
			newBoard.pieces++;
			newBoard.lines+=countLines;
			newBoard.lastMoveLines = countLines;
			ret.push_back(newBoard);
		}
	}
	
	return ret;
	
}

double scoreBoard(const Board &board) {

	double aggregateHeight = 0;
	for (int i=1; i<11; i++) {
	
		int j;
		for (j=0; (board[j] & (1<<i))==0 and j<21; j++);
		aggregateHeight += 20 - j;
	}
	
	double holes = 0;
	for (int i=1; i<11; i++) {
	
		int j;
		for (j=0; (board[j] & (1<<i))==0 and j<21; j++);

		for (int k=j; k<21; k++)
			if ((board[k] & (1<<i))==0)
				holes++;
	}

	double bumpiness = 0;
	int previousColumnHeight;
	for (int i=1; i<11; i++) {
	
		int j;
		for (j=0; (board[j] & (1<<i))==0 and j<21; j++);
		int columnHeight = 20 - j;
		
		if (i>1) bumpiness += std::abs(columnHeight - previousColumnHeight);
		previousColumnHeight = columnHeight;
	}
	
	// If we have holes, we enter recovery mode.
	if (holes>0 or aggregateHeight>60) 
		return -100 + -0.51*aggregateHeight - 0.76*board.lastMoveLines -0.356*holes -0.18*bumpiness;
	
	// We try to avoid making 1-3 lines.
	if (board.lastMoveLines>0 and board.lastMoveLines<4) 
		return -50 + -0.51*aggregateHeight - 0.76*board.lastMoveLines -0.356*holes -0.18*bumpiness;


	bool haveWellLeft = true;
	{
		for (int j=0; j<20; j++) 
			if ((board[j] & (1<<10))!=0)
				haveWellLeft = false;
	}

	if (haveWellLeft)
		return 20 + -0.51*aggregateHeight - 0.76*board.lastMoveLines -0.356*holes -0.18*bumpiness;

	if (board.lastMoveLines==4) 
		return 50 + -0.51*aggregateHeight - 0.76*board.lastMoveLines -0.356*holes -0.18*bumpiness;

	
	return -0.51*aggregateHeight - 0.76*board.lastMoveLines -0.356*holes -0.18*bumpiness;
}

double meanScoreAllPieces(int depth, const Board &board) {
	
	if (depth==0) return scoreBoard(board);
	
	double minScore = 1e10;
	for (int i=0; i<7; i++) {
		
		std::vector<Board> allPiecesBoards = findAllPossibleDestinations(Piece(i), board);
		double bestScore = -1e10;
		for (auto &allPiecesBoard : allPiecesBoards) {
			double score = meanScoreAllPieces(depth-1, allPiecesBoard);
			if (score>bestScore) 
				bestScore = score;
		}
		if (bestScore < minScore)
			minScore = bestScore;
	}
	
	if (minScore == 1e10) minScore = 0;
	
	return minScore;
}

static Piece getSample() {
	
	static std::vector<int> bag;
	while (bag.size() < 28) 
		for (int i=0; i<7; i++)
			bag.push_back(i);
	
	std::random_shuffle(bag.begin(), bag.end());
	int ret = bag.back();
	bag.pop_back();
	return Piece(ret);
}

Board startGame() {
	
	Piece next = getSample();
	
	Board board = initBoard();
	
	drawBoard(board);
	
	while (true) {
		
		Piece piece = next;
		next = getSample();
		
		std::vector<Board> allFirstPieceBoards = findAllPossibleDestinations(piece, board);
		
		double bestBoardScore = -1e10;
		Board bestBoard;
		for (auto &firstPieceBoard : allFirstPieceBoards) {
			
			std::vector<Board> allSecondPieceBoards = findAllPossibleDestinations(next, firstPieceBoard);

			for (auto &secondPieceBoard : allSecondPieceBoards) {
				
				double boardScore = meanScoreAllPieces(1, secondPieceBoard);
				if (boardScore > bestBoardScore) {
					bestBoardScore = boardScore;
					bestBoard = firstPieceBoard;
				} 
			}
		}
		
		if (bestBoardScore==-1e10) return board;
		board = bestBoard;

		drawBoard(board);
		//std::this_thread::sleep_for(100ms);
	}
}



int main() {
	
	startGame();
}
