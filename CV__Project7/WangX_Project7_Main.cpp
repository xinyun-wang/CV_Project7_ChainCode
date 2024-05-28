#include <iostream>
#include <fstream>
#include <string>
using namespace std;

class chainCode {
public:
	struct point {
		int r;
		int c;
		point() {}
		point(int a, int b) {
			r = a;
			c = b;
		}

		bool equal(point b) {
			return this->r == b.r && this->c == b.c;
		}
	};

	struct CCproperty {
		int label;
		int numPixels;
		int minRow, minCol, maxRow, maxCol;
		CCproperty() {
		}
	};

	int numCC;
	CCproperty CC;
	int numR;
	int numC;
	int minV;
	int maxV;
	int label;
	int** imgAry;
	int** boundaryAry;
	int** CCAry;
	point coordOffset[8] = { point(0,1), point(-1,1), point(-1,0), point(-1,-1), point(0,-1), point(1,-1), point(1,0), point(1,1) };
	int zeroTable[8] = { 6, 0, 0, 2, 2, 4, 4, 6 };
	point startP;
	point currentP;
	point nextP;
	int lastQ;
	int nextDir;
	int chainDir;

	chainCode() {
	}

	void zero2DAry(int** ary) {
		for (int i = 0; i < numR + 2; i++) {
			for (int j = 0; j < numC + 2; j++) {
				ary[i][j] = 0;
			}
		}
	}

	void loadImage(ifstream& in) {
		for (int i = 1; i < numR + 1; i++) {
			for (int j = 1; j < numC + 1; j++) {
				in >> imgAry[i][j];
			}
		}
	}

	void loadCCAry(int ccLabel) {
		for (int i = 1; i < numR + 1; i++) {
			for (int j = 1; j < numC + 1; j++) {
				if (imgAry[i][j] == ccLabel)
					CCAry[i][j] = imgAry[i][j];
			}
		}
	}

	void imgReformat(int** inAry, string d, ofstream& out) {
		out << numR << " " << numC << " " << minV << " " << maxV << endl;
		string s = to_string(maxV);

		int w = s.length();
		int r = 1;
		while (r < numR + 1) {
			int c = 1;
			while (c < numC + 1) {
				if (inAry[r][c] == 0) {
					out << d;
				}
				else {
					out << inAry[r][c];
				}
				s = to_string(inAry[r][c]);
				int ww = s.length();
				while (ww <= w) {
					out << " ";
					ww++;
				}
				c++;
			}
			out << endl;
			r++;
		}
	}

	int findNextP(ofstream& debug) {
		debug << "entering findNextP method" << endl;
		int index = lastQ;
		bool found = false;

		while (!found)
		{
			int ir = currentP.r + coordOffset[index].r;
			int jc = currentP.c + coordOffset[index].c;

			if (imgAry[ir][jc] == label) {
				chainDir = index;
				found = true;
			}
			else
				index = fmod(index + 1, 8);
		}

		debug << "leaving findNextP method" << endl;
		debug << "chainDir = " << chainDir << endl;

		return chainDir;
	}

	void getChainCode(ofstream& chainCodeFile, ofstream& debug) {
		debug << "entering getChainCode method" << endl;
		label = CC.label;
		for (int i = 1; i < numR + 1; i++) {
			for (int j = 1; j < numC + 1; j++) {
				if (CCAry[i][j] == label)
				{
					chainCodeFile << i << " " << j << " " << label << endl;
					startP.r = i;
					startP.c = j;
					currentP.r = i;
					currentP.c = j;
					lastQ = 4;
					break;
				}
			}
			if (CCAry[i][startP.c] == label)
				break;
		}
		while (!(nextP.r == startP.r && nextP.c == startP.c))
		{
			lastQ = fmod(lastQ + 1, 8);
			chainDir = findNextP(debug);

			chainCodeFile << chainDir << " ";

			nextP.r = currentP.r + coordOffset[chainDir].r;
			nextP.c = currentP.c + coordOffset[chainDir].c;
			currentP = nextP;

			if (chainDir == 0)
				lastQ = zeroTable[7];
			else
				lastQ = zeroTable[chainDir - 1];

			debug << "lastQ = " << lastQ << " currentP.row = " << currentP.r << " currentP.col = " << currentP.c << " nextP.row = " << nextP.r << " nextP.col = " << nextP.c << endl;
		}
		chainCodeFile << endl;
		debug << "leaving getChainCode" << endl;
	}

	void constructBoundary(ifstream& chainCodeFile, ofstream& debug) {
		debug << "Entering constructBoundary" << endl;
		int i, j;
		chainCodeFile >> i >> j >> label;
		startP.r = i;
		startP.c = j;
		currentP.r = i;
		currentP.c = j;
		lastQ = 4;
		boundaryAry[startP.r][startP.c] = label;
		nextP.r = -1;
		nextP.c = 0;


		while (!nextP.equal(startP))
		{
			lastQ = fmod(lastQ + 1, 8);
			chainCodeFile >> chainDir;
			nextP.r = currentP.r + coordOffset[chainDir].r;
			nextP.c = currentP.c + coordOffset[chainDir].c;
			boundaryAry[nextP.r][nextP.c] = label;
			currentP = nextP;

			if (chainDir == 0)
				lastQ = zeroTable[7];
			else
				lastQ = zeroTable[chainDir - 1];

			debug << "lastQ = " << lastQ << " currentP.row = " << currentP.r << " currentP.col = " << currentP.c << " nextP.row = " << nextP.r << " nextP.col = " << nextP.c << endl;
		}

		debug << "Leaving constructBoundary" << endl;
	}
};


int main(int argc, char* argv[]) {
	ifstream in(argv[1]);
	ifstream propFile(argv[2]);
	ofstream out(argv[3]);
	ofstream debug(argv[4]);
	ofstream chainCodeFile(argv[5]);
	ofstream BoundaryFile(argv[6]);

	chainCode e;
	in >> e.numR >> e.numC >> e.minV >> e.maxV;
	propFile >> e.numR >> e.numC >> e.minV >> e.maxV;
	propFile >> e.numCC;

	e.imgAry = new int* [e.numR + 2];
	e.CCAry = new int* [e.numR + 2];
	e.boundaryAry = new int* [e.numR + 2];

	for (int j = 0; j < e.numR + 2; j++) {
		e.imgAry[j] = new int[e.numC + 2];
		e.CCAry[j] = new int[e.numC + 2];
		e.boundaryAry[j] = new int[e.numC + 2];
	}

	e.zero2DAry(e.imgAry);
	e.zero2DAry(e.boundaryAry);

	e.loadImage(in);
	out << "After loadImage; imgAry as below" << endl;
	e.imgReformat(e.imgAry, ".", out);

	chainCodeFile << e.numR << " " << e.numC << " " << e.minV << " " << e.maxV << endl;
	chainCodeFile << e.numCC << endl;

	while (propFile >> e.CC.label >> e.CC.numPixels >> e.CC.minRow >> e.CC.minCol >> e.CC.maxRow >> e.CC.maxCol)
	{
		e.zero2DAry(e.CCAry);

		e.loadCCAry(e.CC.label);
		out << "After loadCCAry; CCAry as below" << endl;
		e.imgReformat(e.CCAry, ".", out);

		e.getChainCode(chainCodeFile, debug);
		debug << "After getChainCode; CCAry as below" << endl;
	}
	chainCodeFile.close();

	ifstream chainCodeFile1(argv[5]);
	chainCodeFile1 >> e.numR >> e.numC >> e.minV >> e.maxV;
	chainCodeFile1 >> e.numCC;

	for(int i = 0; i <e.numCC; i++)
	{
		e.constructBoundary(chainCodeFile1, debug);
	}
	out << "After constructBoundary; boundaryAry as below" << endl;
	e.imgReformat(e.boundaryAry, ".", out);
	e.imgReformat(e.boundaryAry, "0", BoundaryFile);

	chainCodeFile1.close();
	in.close();
	propFile.close();
	out.close();
	debug.close();
	BoundaryFile.close();
}