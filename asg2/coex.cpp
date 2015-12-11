#include <gecode/int.hh>
#include <gecode/search.hh>
#include <gecode/minimodel.hh>
#include <gecode/driver.hh>
#include <cstdlib>

using namespace Gecode;

int N;
int result[100][100];


class test : public IntMaximizeSpace {
protected:
    IntVarArray white;
    IntVarArray black;
    IntVar sumw;
	IntVar sumb;

public:

	test() :white(*this, N * N, 0, 1), black(*this, N * N, 0, 2), sumw(*this, 0, N * N), sumb(*this, 0, N * N) {   

		Matrix<IntVarArray> W(white, N, N);
		Matrix<IntVarArray> B(black, N, N);
		

		count(*this, white, 1, IRT_EQ, sumw);
		count(*this, black, 2, IRT_EQ, sumb);
		
		rel(*this, black, IRT_NQ ,1); //domain of black queens is {0,2}
		
		//constraint 1: same size constraint
		rel(*this, sumw == sumb); 
		
		//constraint 2: each row only include one type queen
		for(int i = 0; i < N; ++i)
			for(int j = 0; j < N; ++j)
			{
				for(int k = 0; k < N; ++k)
					rel( *this, W(i,j) * B(i,k) == 0);
			}
			
		//constraint 3: each column only include one type queen
		for(int j = 0; j < N; ++j)
			for(int i = 0; i < N; ++i)
			{
				for(int k = 0; k < N; ++k)
					rel( *this, W(i,j)* B(k,j) == 0 );
			}
		
		//constraint 4: each diagonal only include one type queen
	    for(int i = 0; i < N; ++i)
			for(int j = 0; j < N; ++j)
			{	
				for(int x = 0; x < N; ++x)
					for(int y = 0; y < N; ++y)
					{
						if((i - j) == (x - y) || (i+j) == (x+y))
							rel( *this, W(i,j) * B(x,y)==0 );
					}
			}
		
		branch(*this, white, INT_VAR_SIZE_MIN(), INT_VAL_MIN());
		branch(*this, black, INT_VAR_SIZE_MIN(), INT_VAL_MIN());

	}

  // search support
	test(bool share, test& s) : IntMaximizeSpace(share, s) {
		white.update(*this, share, s.white);
		black.update(*this, share, s.black);
		sumw.update(*this, share, s.sumw); 
		sumb.update(*this, share, s.sumb);
	}
    virtual IntMaximizeSpace* copy(bool share) {
		return new test(share,*this);
	}

  // print solution
    void print() const {     
		Matrix<IntVarArray> W(white, N, N);
		Matrix<IntVarArray> B(black, N, N);
		
		for( int i = 0; i < N; ++i )
			for(int j = 0; j < N; ++j )
			{
				result[i][j] = 0;
				if( W(i,j).val() == 1 )
					result[i][j] = 1;
				else if ( B(i,j).val() == 2 )
					result[i][j] = 2;
			}
    }

	virtual IntVar cost(void) const{
		return sumw;
	}

};


// main function
int main(int argc, char* argv[]) {

	N = atoi(argv[1]);
	//std::cout<<N<<"\n";
	
	test* s = new test;
	BAB<test> e(s);
	delete s;
	test* m;

	while(e.next()){
		m = e.next();
	}
	m->print();
	delete m;
	
	int total=0;
	for( int i = 0; i < N; ++i ){
		for(int j = 0; j < N; ++j ){
			std::cout.width(2);
			if( result[i][j] == 0 )
				std::cout<<'#';
			else if( result[i][j] == 1 )
			{
				std::cout<<'0';
				++total;
			}
				 else  std::cout<<'1';
		}
		std::cout<<std::endl;
	}
	
	std::cout<<"The max number of queens for size "<<N <<" is "<<total<<std::endl;
	return 0;

}



/*

please use the following commands for compiling & linking

setenv LD_LIBRARY_PATH <dir>/lib
g++ -I<dir>/include -c coex.cpp
g++ -o coex -L<dir>/lib coex.o -lgecodekernel -lgecodesupport -lgecodesearch -lgecodeint -lgecodeminimodel -lgecodedriver -lgecodegist
./coex 7

output:
linux14:/uac/msc/wyzhang/CSP/asg2/asg2f> ./coex 7
 # # 1 1 # # #
 # # # # # # 0
 # # # # # # 0
 1 1 # # # # #
 1 1 1 # # # #
 # # # # 0 0 0
 # # # # # 0 0
The max number of queens for size 7 is 7


*/