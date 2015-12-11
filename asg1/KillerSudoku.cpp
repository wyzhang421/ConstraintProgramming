#include<gecode/int.hh>
#include<gecode/search.hh>
#include<gecode/minimodel.hh>

using namespace Gecode;

class KillerSudoku: public Space{
protected:
	IntVarArray l;

public:
	KillerSudoku(void): l(*this, 9*9, 1, 9){
		Matrix<IntVarArray> Mat(l,9,9);

        // all_different elements in each row
		for(int i=0; i<9; i++)
			distinct(*this, Mat.row(i));
		
        // all_different elements in each column
		for(int i=0; i<9; i++)
			distinct(*this, Mat.col(i));
		
        // all_different elements in each grid
		for(int i=0; i<9; i+=3)
			for(int j=0; j<9; j+=3)
				distinct(*this, Mat.slice(i,i+3,j,j+3));
			
        // the sum of cells in one cage equals to a total
		rel(*this, Mat(0,0)+Mat(0,1)+Mat(1,1)==20);

		rel(*this, Mat(0,2)+Mat(0,3)+Mat(0,4)+Mat(1,2)==19);

		rel(*this, Mat(0,5)+Mat(0,6)+Mat(0,7)==18);

		rel(*this, Mat(0,8)+Mat(1,8)+Mat(2,8)==12);

		rel(*this, Mat(1,0)+Mat(2,0)+Mat(3,0)==7);

		rel(*this, Mat(1,3)+Mat(2,1)+Mat(2,2)+Mat(2,3)==28);

		rel(*this, Mat(1,4)+Mat(1,5)+Mat(1,6)==9);

		rel(*this, Mat(1,7)+Mat(2,6)+Mat(2,7)==16);

		rel(*this, Mat(2,4)+Mat(2,5)==10);	

		rel(*this, Mat(3,1)+Mat(4,0)+Mat(4,1)==16);

		rel(*this, Mat(3,2)+Mat(3,3)+Mat(3,4)+Mat(4,2)==23);

		rel(*this, Mat(3,5)+Mat(4,3)+Mat(4,4)+Mat(4,5)+Mat(5,3)==32);

		rel(*this, Mat(3,6)+Mat(3,7)+Mat(3,8)==12);

		rel(*this, Mat(4,6)+Mat(5,4)+Mat(5,5)+Mat(5,6)==17);

		rel(*this, Mat(4,7)+Mat(4,8)+Mat(5,7)==17);

		rel(*this, Mat(5,0)+Mat(5,1)+Mat(5,2)==8);

		rel(*this, Mat(5,8)+Mat(6,8)+Mat(7,8)==23);

		rel(*this, Mat(6,0)+Mat(7,0)+Mat(8,0)==23);

		rel(*this, Mat(6,1)+Mat(6,2)+Mat(7,1)==16);

		rel(*this, Mat(6,3)+Mat(6,4)==11);

		rel(*this, Mat(6,5)+Mat(6,6)+Mat(6,7)+Mat(7,5)==15);

		rel(*this, Mat(7,2)+Mat(7,3)+Mat(7,4)==9);

		rel(*this, Mat(7,6)+Mat(8,4)+Mat(8,5)+Mat(8,6)==26);

		rel(*this, Mat(7,7)+Mat(8,7)+Mat(8,8)==10);

		rel(*this, Mat(8,1)+Mat(8,2)+Mat(8,3)==8);

		//post branching
		branch(*this, l, INT_VAR_SIZE_MIN(), INT_VAL_MIN());

	}

	//search support
	KillerSudoku(bool share, KillerSudoku& s):Space(share, s){
		l.update(*this, share, s.l);
	}
	virtual Space* copy(bool share){
		return new KillerSudoku(share, *this);
	}
	//print solution
	void print(void) const{

		Matrix<IntVarArray> Mat(l,9,9);
        for (int i = 0; i<9; i++) {
            for (int j = 0; j<9; j++) {
                std::cout << Mat(i,j) << " ";
            }
            std::cout << std::endl;
        }
	}
};

//main function
int main(int argc, char* argv[]){
	//create model and search engine
	KillerSudoku* m = new KillerSudoku;
	DFS<KillerSudoku> e(m);
	delete m;

	//search and print all solutions
	while (KillerSudoku* s = e.next()){
		s->print();
		delete s;
	}
	return 0;
}

/*
please use the following commands for compiling & linking

setenv LD_LIBRARY_PATH <dir>/lib
g++ -I<dir>/include -c KillerSudoku.cpp
g++ -o KillerSudoku -L<dir>/lib KillerSudoku.o -lgecodekernel -lgecodesupport -lgecodesearch -lgecodeint -lgecodeminimodel


Output:
linux1:/uac/msc/wyzhang> ./KillerSudoku
3 8 7 6 2 4 5 9 1 
2 9 4 8 5 1 3 6 7 
1 5 6 9 3 7 8 2 4 
4 6 9 5 1 8 2 7 3 
7 3 8 2 9 6 1 4 5 
5 2 1 7 4 3 9 8 6 
6 4 5 3 8 2 7 1 9 
9 7 2 1 6 5 4 3 8 
8 1 3 4 7 9 6 5 2 

*/